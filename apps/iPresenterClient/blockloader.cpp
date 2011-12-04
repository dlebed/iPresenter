#include "blockloader.h"

#include <QMetaType>
#include <QDir>
#include <QPluginLoader>
#include <QDomElement>
#include <QDomDocument>
#include <QFile>

#include <qlogger.h>

#include "hash/hashcalculatorfactory.h"

BlockLoader::BlockLoader(const QString &hashType, QObject *parent) :
    QThread(parent),
    isLoading(false), hashQuery(NULL), hashCalculator(NULL)
{
	if (QMetaType::type("QDomDocument") == 0)
        qRegisterMetaType<QDomDocument>("QDomDocument");
    
    moveToThread(this);
    
    mediaBasePath = settings.value("storage/media_base_path", "/var/media").toString();
    
    hashCalculator = HashCalculatorFactory::hashCalculatorInstance(hashType);
    Q_ASSERT(hashCalculator);
    
    scheduleUpdateCheckTimer.setSingleShot(false);
    scheduleUpdateCheckTimer.setInterval(settings.value("loaders/update_check_interval", 180).toUInt() * 1000);
    
}

BlockLoader::~BlockLoader() {
    QHash<QString, IBlockLoader *>::const_iterator blockLoader = blockLoadersHash.constBegin();
    while (blockLoader != blockLoadersHash.constEnd()) {
        QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_TRACE) << __FUNCTION__ << "Removing content loader plugin ID:" << blockLoader.key();
        disconnect(&scheduleUpdateCheckTimer, SIGNAL(timeout()), blockLoader.value(), SLOT(scheduleUpdateCheck()));
        disconnect(blockLoader.value(), SIGNAL(scheduleUpdateAvailable(QString)), this, SLOT(updateSchedule(QString)));
        delete blockLoader.value();
        ++blockLoader;
    }
    
    if (hashQuery != NULL) {
        delete hashQuery;
        hashQuery = NULL;
    }
    
}

void BlockLoader::run() {
    QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_INFO) << __FUNCTION__ << "BlockLoader thread starting:" << hex << currentThreadId();
    
    if (hashQuery == NULL)
        hashQuery = new HashQuery();
    
    Q_ASSERT(hashQuery != NULL);
    
    initLoaders();
    
    exec();
}

quint8 BlockLoader::initLoaders() {
    if (!blockLoadersHash.isEmpty())
        return E_LOADER_ALREADY_LOADED;
    
    QString pluginsPath = settings.value("loaders/plugin_path").toString();
    
    QDir pluginsDir = QDir(pluginsPath);
    QStringList fileFilters;
    fileFilters << "*" ".so" << "*" ".so" ".*";

    QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_INFO) << __FUNCTION__ <<
            "Loading content loaders plugins from" << pluginsPath;
    
    foreach (QString fileName, pluginsDir.entryList(fileFilters, QDir::Files)) {
        QPluginLoader pluginLoader(pluginsDir.absoluteFilePath(fileName));

        if (!pluginLoader.load()) {
            QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_ERROR) << __FUNCTION__ <<
                    "Unable to load content loader plugin:" << fileName;
            
            continue;
        }

        QObject *plugin = pluginLoader.instance();

        if (plugin != NULL) {
            IBlockLoader * blockLoader = qobject_cast<IBlockLoader *>(plugin);

            if (blockLoader != NULL) {
                // Check if module with this id already exists
                if (!blockLoadersHash.contains(blockLoader->getID())) {
                    QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_INFO) << __FUNCTION__ <<
                            "Content loader loaded:" << fileName << "; ID:" << blockLoader->getID() <<
                            "; Description:" << blockLoader->description();

                    blockLoadersHash[blockLoader->getID()] = blockLoader;
                    connect(blockLoader, SIGNAL(scheduleUpdateAvailable(QString)), this, SLOT(updateSchedule(QString)));
                    connect(&scheduleUpdateCheckTimer, SIGNAL(timeout()), blockLoader, SLOT(scheduleUpdateCheck()));
                } else {
                    QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_INFO) << __FUNCTION__ <<
                            "Content loader with same ID already loaded" << fileName << "; ID:" << blockLoader->getID();
                }

            } else {
                QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_ERROR) << __FUNCTION__ <<
                        "Unable to cast to IBlockLoader * content loader plugin" << fileName <<
                        "; Error: " << pluginLoader.errorString();
            }
        } else {
            QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_ERROR) << __FUNCTION__ <<
                    "Unable to instance content loader:" << fileName;
        }
    }

    QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_INFO) << __FUNCTION__ <<
            "Total content loader plugins:" << blockLoadersHash.size();
    
    if (blockLoadersHash.isEmpty() == 0)
        return E_NO_PLUGINS_FOUND;
    
    return E_OK;
}

void BlockLoader::stop() {
    interruptLoading();
    exit();
    wait();
    
    delete hashQuery;
    hashQuery = NULL;
}

void BlockLoader::interruptLoading() {
    
    
    
}

void BlockLoader::loadBlock(const QDomDocument &blockDocument) {
    if (blockLoadersHash.size() == 0 && !settings.value("presentation/allow_incomplete_blocks", false).toBool()) {
        emit blockLoadingError(E_NO_LOADERS_LOADED);
        return;
    }
    
    QDomElement blockElements = blockDocument.documentElement().firstChildElement("elements");
    
    if (blockElements.isNull()) {
        QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_WARN) << __FUNCTION__ << "Attempt to load empty block:" << blockDocument.toText().data();
        emit blockLoaded(blockDocument);
        return;
    }
    
    QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_TRACE) << __FUNCTION__ << "Starting to load block images...";
    
    QDomElement imagesGroupElement = blockElements.firstChildElement("images");
    
    while (!imagesGroupElement.isNull()) {
    
        QDomElement imageElement = imagesGroupElement.firstChildElement("image");
        
        while (!imageElement.isNull()) {
            QString imageHash = imageElement.attribute("hash");
            
            if (!imageHash.isEmpty()) {
                Q_ASSERT(hashQuery != NULL);
                if (hashQuery->lookupFilePathByHash(imageHash, FILE_TYPE_IMAGE).isEmpty()) {
                    QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_INFO) << __FUNCTION__ << "Found new image to load:" << imageHash;
                    if (loadMediaFile(imageHash, FILE_TYPE_IMAGE) != E_OK) {
                        QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_ERROR) << __FUNCTION__ << "Image load failed:" << imageHash;
                    }
                }
            } else {
                QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_WARN) << __FUNCTION__ << "Found image tag with empty hash:" << imageElement.toText().data();
            }
            
            imageElement = imageElement.nextSiblingElement("image");
        }
        
        imagesGroupElement = imagesGroupElement.firstChildElement("images");
    }
    
    QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_TRACE) << __FUNCTION__ << "Starting to load block movies...";
    
    QDomElement movieElement = blockElements.firstChildElement("movie");
    
    while (!movieElement.isNull()) {
        QString movieHash = movieElement.attribute("hash");
        
        if (!movieHash.isEmpty()) {
            Q_ASSERT(hashQuery != NULL);
            if (hashQuery->lookupFilePathByHash(movieHash, FILE_TYPE_MOVIE).isEmpty()) {
                QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_INFO) << __FUNCTION__ << "Found new movie to load:" << movieHash;
                if (loadMediaFile(movieHash, FILE_TYPE_MOVIE) != E_OK) {
                    QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_ERROR) << __FUNCTION__ << "Movie load failed:" << movieHash;
                }
            }
        } else {
            QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_WARN) << __FUNCTION__ << "Found movie tag with empty hash:" << movieElement.toText().data();
        }
        
        movieElement = movieElement.nextSiblingElement("movie");
    }
    
    QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_TRACE) << __FUNCTION__ << "All media files is loaded.";
    
    // stub
    emit blockLoaded(blockDocument);
}

void BlockLoader::checkScheduleUpdate(schedule_version_t currentScheduleVersion) {
    if (blockLoadersHash.size() == 0) {
        return;
    }
    
    
}

void BlockLoader::updateSchedule(const QString & scheduleDocument) {
    
}

quint8 BlockLoader::loadMediaFile(const QString &hash, FILE_TYPE fileType) {
    QString loadedFilePath;
    QString fileTypeStr = "images";
    QString filePath;
    quint8 res;
    
    if (fileType == FILE_TYPE_MOVIE)
        fileTypeStr = "movies";
    
    filePath = mediaBasePath + "/" + fileTypeStr + "/" + hash;
    
    if (QFile::exists(filePath)) {
        QString fileHash = hashCalculator->getFileHash(filePath);
        
        if (fileHash == hash) {
            QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_WARN) << __FUNCTION__ << "Found existing file with same hash:" << filePath;
            Q_ASSERT(hashQuery != NULL);
            hashQuery->addFile(filePath, hash, fileType);
            return E_OK;    
        } else {
            QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_WARN) << __FUNCTION__ << "Found existing file with different hash:" << filePath;
            QFile::remove(filePath);
        }
    }
    
    QHash<QString, IBlockLoader *>::const_iterator i = blockLoadersHash.constBegin();
    while (i != blockLoadersHash.constEnd()) {
        res = i.value()->loadFile(hash, fileType, filePath);
        
        if (res == IBlockLoader::LOAD_SUCCESS) {
            if (!QFile::exists(filePath)) {
                QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_ERROR) << __FUNCTION__ << "Media file does not exist after loading.";
                ++i;
                continue;
            }
            
            QString fileHash = hashCalculator->getFileHash(filePath);
            
            if (fileHash != hash) {
                QFile::remove(filePath);
                QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_ERROR) << __FUNCTION__ << "File received. But hash is invalid." << hash;   
                continue;
            }
            
            QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_TRACE) << __FUNCTION__ << "File hash check ok. Hash:" << hash;
            
            Q_ASSERT(hashQuery != NULL);
            hashQuery->addFile(filePath, hash, fileType);
            
            QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_INFO) << __FUNCTION__ << "Media file loaded successfully. Hash:" << hash;
            return E_OK;
        }
        
        ++i;
    }

    return E_BLOCK_LOAD_ERROR;
}

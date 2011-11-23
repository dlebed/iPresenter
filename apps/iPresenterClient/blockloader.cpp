#include "blockloader.h"

#include <QMetaType>
#include <QDir>
#include <QPluginLoader>

#include <qlogger.h>

#include "hash/hashcalculatorfactory.h"

BlockLoader::BlockLoader(const QString &hashType, QObject *parent) :
    QThread(parent),
    isLoading(false), hashCalculator(NULL)
{
	if (QMetaType::type("QDomDocument") == 0)
        qRegisterMetaType<QDomDocument>("QDomDocument");
    
    moveToThread(this);
    
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
}

void BlockLoader::run() {
    QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_INFO) << __FUNCTION__ << "BlockLoader thread starting:" << hex << currentThreadId();
    
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
}

void BlockLoader::interruptLoading() {
    
    
    
}

void BlockLoader::loadBlock(const QDomDocument &blockDocument) {
    if (blockLoadersHash.size() == 0)
        return;
    
    
    
    // stub
    emit blockLoaded(blockDocument);
}


void BlockLoader::updateSchedule(const QString & scheduleDocument) {
    
}

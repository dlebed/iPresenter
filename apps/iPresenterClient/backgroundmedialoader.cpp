#include "backgroundmedialoader.h"

#include <QTimer>
#include <qlogger.h>
#include <QDomDocument>

#include <hashquery/hashqueryfactory.h>
#include <hashcalculatorfactory.h>

BackgroundMediaLoader::BackgroundMediaLoader(const QString &scheduleDocString, IBlockLoader *blockLoader) :
    scheduleDocString(scheduleDocString),
    blockLoader(blockLoader),
    hashQuery(NULL),
    hashCalculator(NULL),
    isExiting(false)
{
    moveToThread(this);
    Q_ASSERT(blockLoader != NULL);

    mediaBasePath = settings.value("storage/media_base_path", "/var/media").toString();

}

BackgroundMediaLoader::~BackgroundMediaLoader() {

    stop();


}

void BackgroundMediaLoader::run() {
    QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_TRACE) << __FUNCTION__ << "BackgroundMediaLoader thread started:" << hex << currentThreadId();

    QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_TRACE) << __FUNCTION__ << "BackgroundMediaLoader using loader" << blockLoader->getID() << ":" << blockLoader->description();

    if (hashQuery == NULL)
        hashQuery = HashQueryFactory::hashQuery();

    Q_ASSERT(hashQuery != NULL);

    if (hashCalculator == NULL)
        hashCalculator = HashCalculatorFactory::hashCalculatorInstance(DEFAULT_HASH_TYPE);

    Q_ASSERT(hashCalculator != NULL);

    isExiting = false;

    QTimer::singleShot(0, this, SLOT(presentationDataLoadLoop()));

    QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_TRACE) << __FUNCTION__ << "Starting BackgroundMediaLoader event loop";
    exec();
}

void BackgroundMediaLoader::stop() {
    isExiting = true;

    exit();
    wait();

    if (hashQuery != NULL) {
        delete hashQuery;
        hashQuery = NULL;
    }

    if (hashCalculator != NULL) {
        delete hashCalculator;
        hashCalculator = NULL;
    }
}

void BackgroundMediaLoader::presentationDataLoadLoop() {
    QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_TRACE) << __FUNCTION__ << hex << currentThreadId();

    if (scheduleDocString.isEmpty()) {
        QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_TRACE) << __FUNCTION__ << "Empty schedule doc to load";
        emit scheduleLoadFailed();
        return;
    }

    QDomDocument scheduleDoc;

    if (!scheduleDoc.setContent(scheduleDocString)) {
        QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_TRACE) << __FUNCTION__ << "Schedule doc parse error";
        emit scheduleLoadFailed();
        return;
    }

    QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_TRACE) << __FUNCTION__ << "Starting to load presentation data...";

    QDomElement blocksElement = scheduleDoc.documentElement().firstChildElement("blocks");

    QDomElement blockElement = blocksElement.firstChildElement("block");
    quint8 res;

    while (!blockElement.isNull()) {
        QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_TRACE) << __FUNCTION__ << "Processing block" << blockElement.attribute("id");

        if ((res = loadBlock(blockElement)) != E_OK) {
            QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_TRACE) << __FUNCTION__ << "Block load error:" << res;
            emit scheduleLoadFailed();
            return;
        }

        QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_TRACE) << __FUNCTION__ << "Block" << blockElement.attribute("id") << "successfully loaded";

        blockElement = blockElement.nextSiblingElement("block");
    }

    QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_INFO) << __FUNCTION__ << "All blocks successfully loaded!";

    emit scheduleLoaded();
}

quint8 BackgroundMediaLoader::loadBlock(const QDomElement &blockElement) {
    QDomElement blockElements = blockElement.firstChildElement("elements");

    if (blockElements.isNull()) {
        QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_INFO) << __FUNCTION__ << "Attempt to load empty block:" << blockElement.toText().data();
        return E_OK;
    }

    QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_TRACE) << __FUNCTION__ << "Starting to load block images...";

    QDomElement imagesGroupElement = blockElements.firstChildElement("images");

    while (!imagesGroupElement.isNull()) {

        QDomElement imageElement = imagesGroupElement.firstChildElement("image");

        while (!imageElement.isNull()) {
            if (isExiting) {
                return E_INTERRUPTED;
            }

            QString imageHash = imageElement.attribute("hash");

            if (!imageHash.isEmpty()) {
                Q_ASSERT(hashQuery != NULL);
                if (hashQuery->lookupFilePathByHash(imageHash, FILE_TYPE_IMAGE).isEmpty()) {
                    QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_INFO) << __FUNCTION__ << "Found new image to load:" << imageHash;
                    if (loadMediaFile(imageHash, FILE_TYPE_IMAGE) != E_OK) {
                        QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_ERROR) << __FUNCTION__ << "Image load failed:" << imageHash;

                        if (!settings.value("presentation/allow_incomplete_blocks", false).toBool()) {
                            return E_LOAD_ERROR;
                        }
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
        if (isExiting) {
            return E_INTERRUPTED;
        }

        QString movieHash = movieElement.attribute("hash");

        if (!movieHash.isEmpty()) {
            Q_ASSERT(hashQuery != NULL);
            if (hashQuery->lookupFilePathByHash(movieHash, FILE_TYPE_MOVIE).isEmpty()) {
                QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_INFO) << __FUNCTION__ << "Found new movie to load:" << movieHash;
                if (loadMediaFile(movieHash, FILE_TYPE_MOVIE) != E_OK) {
                    QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_ERROR) << __FUNCTION__ << "Movie load failed:" << movieHash;

                    if (!settings.value("presentation/allow_incomplete_blocks", false).toBool()) {
                        return E_LOAD_ERROR;
                    }
                }
            }
        } else {
            QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_WARN) << __FUNCTION__ << "Found movie tag with empty hash:" << movieElement.toText().data();
        }

        movieElement = movieElement.nextSiblingElement("movie");
    }

    QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_TRACE) << __FUNCTION__ << "All media files is loaded.";

    return E_OK;
}

quint8 BackgroundMediaLoader::loadMediaFile(const QString &hash, FILE_TYPE fileType) {
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

    res = blockLoader->loadFile(hash, fileType, filePath);

    if (res == IBlockLoader::LOAD_SUCCESS) {
        if (!QFile::exists(filePath)) {
            QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_ERROR) << __FUNCTION__ << "Media file does not exist after loading.";
            return E_LOAD_ERROR;
        }

        QString fileHash = hashCalculator->getFileHash(filePath);

        if (fileHash != hash) {
            QFile::remove(filePath);
            QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_ERROR) << __FUNCTION__ << "File received. But hash is invalid." << hash;
            return E_LOAD_ERROR;
        }

        QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_TRACE) << __FUNCTION__ << "File hash check ok. Hash:" << hash;

        Q_ASSERT(hashQuery != NULL);
        hashQuery->addFile(filePath, hash, fileType);

        QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_INFO) << __FUNCTION__ << "Media file loaded successfully. Hash:" << hash;
        return E_OK;
    }


    return E_LOAD_ERROR;
}

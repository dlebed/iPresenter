#include "mediafile.h"

#include <QFile>
#include <QFileInfo>



#include <qlogger.h>

MediaFile::MediaFile(bool serverStored, QObject *parent) :
    QObject(parent), serverStored(serverStored), hashCalculator(NULL), fileSize(0), fileType(FILE_TYPE_UNKNOWN), timeout(0)
{
    hashCalculator = HashCalculatorFactory::hashCalculatorInstance(settings.value("hash/type", "sha256").toString());
    Q_ASSERT(hashCalculator != NULL);

}


MediaFile::MediaFile(QString name, QString description, MEDIA_FILE_TYPE type, QString path, quint64 size, quint32 timeout, bool serverStored, QObject *parent):
    QObject(parent), serverStored(serverStored),
    name(name), description(description), fileType(type), mediaFilePath(path), fileSize(size), timeout(timeout)
{

}


bool MediaFile::setFile(QString fileName, QString name, QString description) {
    if (!QFile::exists(fileName)) {
        QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_ERROR) << __FUNCTION__ << "Can't create media file: file does not exist";
        return false;
    }

    QFileInfo fileInfo(fileName);

    if (fileInfo.suffix() == "jpg" || fileInfo.suffix() == "png") {
        fileType = FILE_TYPE_IMAGE;
    } else if (fileInfo.suffix() == "avi" || fileInfo.suffix() == "mkv" ||
               fileInfo.suffix() == "mov" || fileInfo.suffix() == "mp4") {
        fileType = FILE_TYPE_MOVIE;
    } else {
        QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_ERROR) << __FUNCTION__ << "Can't create media file: unknown file type";
        return false;
    }

    QString hash = hashCalculator->getFileHash(fileName);

    if (hash.isEmpty()) {
        QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_ERROR) << __FUNCTION__ << "Can't create media file: empty hash";
        return false;
    }

    fileHash = hash;

    QFile mediaFile(fileName);

    fileSize = mediaFile.size();

    this->name = name;
    this->description = description;
    mediaFilePath = fileName;
    timeout = 0;

    QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_INFO) << __FUNCTION__ << "Successfully created media file:" << fileName << name << description << hash << fileSize << fileType;

    return true;
}

bool MediaFile::isValid() {
    if (!mediaFilePath.isEmpty() && QFile::exists(mediaFilePath))
        return true;
    else
        return false;
}

QString MediaFile::getHash() {
    if (!fileHash.isEmpty())
        return fileHash;

    if (QFile::exists(mediaFilePath)) {
        IHashCalculator *hashCalculator = HashCalculatorFactory::hashCalculatorInstance(settings.value("hash/default", "sha256").toString());
        Q_ASSERT(hashCalculator != NULL);
        QString calcFileHash = hashCalculator->getFileHash(mediaFilePath);

        if (!calcFileHash.isEmpty()) {
            fileHash = calcFileHash;
            return fileHash;
        }
    }

    return QString();
}

quint64 MediaFile::getFileSize(bool *ok) {
    if (ok != NULL)
        *ok = false;

    if (fileSize == 0 && QFile::exists(mediaFilePath)) {
        QFile mediaFile(mediaFilePath);

        if (mediaFile.open(QIODevice::ReadOnly) && mediaFile.size() >= 0) {
            fileSize = mediaFile.size();
            if (ok != NULL)
                *ok = true;
            return mediaFile.size();
        }
    } else {
        if (ok != NULL)
            *ok = true;
    }

    return fileSize;
}

QString MediaFile::getFileTypeStr() const {
    switch (fileType) {
    case FILE_TYPE_UNKNOWN:
        return "unknown";
    case FILE_TYPE_IMAGE:
        return "image";
    case FILE_TYPE_MOVIE:
        return "movie";
    }
}

bool MediaFile::parseElement(const QDomElement &mediaElement) {
    if (mediaElement.tagName() != "image" && mediaElement.tagName() != "movie")
        return false;

    clear();

    if (mediaElement.attribute("id").isEmpty() || mediaElement.attribute("hash").isEmpty())
        return false;

    name = mediaElement.attribute("id");
    fileHash = mediaElement.attribute("hash");

    if (mediaElement.tagName() == "image")
        fileType = FILE_TYPE_IMAGE;
    else if (mediaElement.tagName() == "movie")
        fileType = FILE_TYPE_MOVIE;

    if (!mediaElement.attribute("size").isEmpty()) {
        fileSize = mediaElement.attribute("size").toULongLong();
    }

    if (!mediaElement.attribute("desc").isEmpty()) {
        description = mediaElement.attribute("desc");
    }

    if (!mediaElement.attribute("timeout").isEmpty()) {
        timeout = mediaElement.attribute("timeout").toULong();
    }

    return true;
}

QDomElement MediaFile::makeElement(QDomDocument &document) {
    QDomElement mediaElement;

    if (name.isEmpty() || getHash().isEmpty() || fileType == FILE_TYPE_UNKNOWN)
        return mediaElement;

    if (fileType == FILE_TYPE_IMAGE)
        mediaElement = document.createElement("image");
    else if (fileType == FILE_TYPE_MOVIE)
        mediaElement = document.createElement("movie");

    mediaElement.setAttribute("id", name);
    mediaElement.setAttribute("hash", getHash());

    if (!description.isEmpty())
        mediaElement.setAttribute("desc", description);

    if (fileSize > 0)
        mediaElement.setAttribute("size", fileSize);

    if (timeout > 0)
        mediaElement.setAttribute("timeout", timeout);

    return mediaElement;
}

void MediaFile::clear() {
    fileType = FILE_TYPE_UNKNOWN;
    mediaFilePath.clear();
    name.clear();
    description.clear();
    timeout = 0;
    fileSize = 0;
    fileHash.clear();
}

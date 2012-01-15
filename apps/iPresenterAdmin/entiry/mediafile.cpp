#include "mediafile.h"

#include <QFile>

#include <hashcalculatorfactory.h>

MediaFile::MediaFile(QObject *parent) :
    QObject(parent), fileSize(0), fileType(FILE_TYPE_UNKNOWN), timeout(0)
{
}


MediaFile::MediaFile(QString name, QString description, MEDIA_FILE_TYPE type, QString path, quint64 size, quint32 timeout, QObject *parent):
    QObject(parent),
    name(name), description(description), fileType(type), mediaFilePath(path), fileSize(size), timeout(timeout)
{

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

quint64 MediaFile::getFileSize(bool &ok) {
    ok = false;

    if (fileSize == 0 && QFile::exists(mediaFilePath)) {
        QFile mediaFile(mediaFilePath);

        if (mediaFile.open(QIODevice::ReadOnly) && mediaFile.size() >= 0) {
            fileSize = mediaFile.size();
            ok = true;
            return mediaFile.size();
        }
    } else {
        ok = true;
    }

    return fileSize;
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

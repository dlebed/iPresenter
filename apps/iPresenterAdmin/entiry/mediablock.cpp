#include "mediablock.h"

#include <QDomDocument>

MediaBlock::MediaBlock(QObject *parent) :
    QObject(parent), version(0)
{
}

MediaBlock::MediaBlock(QString name, QString description, QObject *parent):
    QObject(parent),
    name(name), description(description),
    version(0)
{

}

MediaBlock::~MediaBlock() {
    for (int i = 0; i < mediaFiles.size(); i++) {
        delete mediaFiles.at(i);
    }

    mediaFiles.clear();
}

bool MediaBlock::addMediaFile(MediaFile *mediaFile) {
    mediaFiles.append(mediaFile);

    return true;
}

bool MediaBlock::removeMediaFile(QString hash) {

    for (int i = 0; i < mediaFiles.size(); i++) {
        if (mediaFiles.at(i)->getHash() == hash) {
            delete mediaFiles.at(i);
            mediaFiles.removeAt(i);
            return true;
        }
    }

    return false;
}

bool MediaBlock::removeMediaFile(int pos) {
    if (pos < mediaFiles.size()) {
        mediaFiles.removeAt(pos);
        return true;
    }

    return false;
}

QList<MediaFile *> MediaBlock::getMediaFilesToUpload() const {
    QList<MediaFile *> result;

    for (int i = 0; i < mediaFiles.size(); i++) {
        if (!mediaFiles.at(i)->isServerStored()) {
            result.append(mediaFiles.at(i));
        }
    }

    return result;
}

QString MediaBlock::getBlockXml(bool &ok) {
    QDomDocument blockDocument;
    QDomElement blockElement = blockDocument.createElement("block");
    blockDocument.appendChild(blockElement);

    ok = false;

    blockElement.setAttribute("id", name);
    blockElement.setAttribute("desc", description);

    QDomElement blockElementsElement = blockDocument.createElement("elements");
    QDomElement currentImagesElement;
    blockElement.appendChild(blockElementsElement);

    for (int i = 0; i < mediaFiles.size(); i++) {
        MediaFile *mediaFile = mediaFiles.at(i);
        QDomElement mediaElement = mediaFile->makeElement(blockDocument);

        if (!mediaElement.isNull()) {
            if (mediaFile->getFileType() == MediaFile::FILE_TYPE_IMAGE) {
                if (currentImagesElement.isNull()) {
                    currentImagesElement = blockDocument.createElement("images");
                    blockElementsElement.appendChild(currentImagesElement);
                }

                currentImagesElement.appendChild(mediaElement);
            } else {
                currentImagesElement = QDomElement();
                blockElementsElement.appendChild(mediaElement);
            }
        }
    }

    ok = true;
    return blockDocument.toString();
}

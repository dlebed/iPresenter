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

QString MediaBlock::getBlockXml(bool &ok) {
    QDomDocument blockDocument;
    QDomElement blockElement = blockDocument.createElement("block");
    blockDocument.appendChild(blockElement);

    ok = false;

    blockElement.setAttribute("id", name);
    blockElement.setAttribute("desc", description);

    QDomElement blockElementsElement = blockDocument.createElement("elements");
    blockElement.appendChild(blockElementsElement);

    for (int i = 0; i < mediaFiles.size(); i++) {
        MediaFile *mediaFile = mediaFiles.at(i);
        QDomElement mediaElement = mediaFile->makeElement(blockDocument);

        if (!mediaElement.isNull())
            blockElementsElement.appendChild(mediaElement);
    }

    ok = true;
    return blockDocument.toString();
}

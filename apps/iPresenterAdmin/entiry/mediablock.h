#ifndef MEDIABLOCK_H
#define MEDIABLOCK_H

#include <QObject>
#include <QList>

#include <entiry/mediafile.h>

class MediaBlock : public QObject
{
    Q_OBJECT
public:
    explicit MediaBlock(QObject *parent = 0);
    
    MediaBlock(QString name, QString description, QObject *parent = 0);

    ~MediaBlock();

    bool addMediaFile(MediaFile *mediaFile);
    QList<MediaFile *> getMediaFilesList() const { return mediaFiles; }

    QList<MediaFile *> getMediaFilesToUpload() const;

    void setName(const QString &name) { this->name = name; }
    QString getName() const { return name; }

    void setDescription(const QString &description) { this->description = description; }
    QString getDescription() const { return description; }

    QString getBlockXml(bool &ok);

private:
    QList<MediaFile *> mediaFiles;

    QString name;
    QString description;

    quint32 version;
    
};

#endif // MEDIABLOCK_H

#ifndef MEDIAFILE_H
#define MEDIAFILE_H

#include <QObject>
#include <QSettings>

#include <QDomDocument>
#include <QDomElement>

class MediaFile : public QObject
{
    Q_OBJECT
public:

    enum MEDIA_FILE_TYPE {
        FILE_TYPE_UNKNOWN   =   0x00,
        FILE_TYPE_IMAGE     =   0x01,
        FILE_TYPE_MOVIE     =   0x02
    };

    explicit MediaFile(QObject *parent = 0);
    
    MediaFile(QString name, QString description, MEDIA_FILE_TYPE type, QString path, quint64 size = 0, quint32 timeout = 0, QObject *parent = 0);

    bool isValid();

    void setHash(const QString &hash) { this->fileHash = hash; }
    QString getHash();

    void setName(const QString &name) { this->name = name; }
    QString getName() const { return name; }

    void setDescription(const QString &description) { this->description = description; }
    QString getDescription() const { return description; }

    void setFilePath(const QString &path) { mediaFilePath = path; }
    QString getFilePath() const { return mediaFilePath; }

    void setFileSize(quint64 fileSize) { if (this->fileSize == 0) { this->fileSize = fileSize; } }
    quint64 getFileSize(bool &ok);

    void setFileType(MEDIA_FILE_TYPE type) { this->fileType = type; }
    MEDIA_FILE_TYPE getFileType() const { return fileType; }

    void setTimeout(quint32 timeout) { this->timeout = timeout; }
    quint32 getTimeout() const { return timeout; }

    bool parseElement(const QDomElement &mediaElement);
    QDomElement makeElement(QDomDocument &document);

    void clear();

private:
    QSettings settings;

    MEDIA_FILE_TYPE fileType;
    QString mediaFilePath;
    QString name;
    QString description;

    quint32 timeout;

    quint64 fileSize;
    QString fileHash;

    
};

#endif // MEDIAFILE_H

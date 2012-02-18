#ifndef TCPCLIENTMODULE_H
#define TCPCLIENTMODULE_H

#include <QObject>
#include <QSettings>
#include <QTcpSocket>

#include <types.h>

#define HASH_LEN  64

class TCPClientModule : public QObject
{
    Q_OBJECT
public:
    enum TCPCLIENT_ERRORS {
        E_OK                        =       0x01,
        UPLOAD_CONNECTION_FAILED    =       0x01,
		UPLOAD_NO_SUCH_FILE         =       0x02,
		UPLOAD_ERROR                =       0x03,
        FILE_READ_ERROR             =       0x04
    };

    enum ADMIN_RESULT_CODES {
        ADMIN_OK                =   0x00,
        ADMIN_NACK              =   0xFF
    };

    typedef struct {
        uint8_t hash[HASH_LEN];
        uint8_t mediaType;
        media_size_t size;
        media_size_t offset;
    } __attribute__((packed)) MediaUploadCmd;

    typedef struct {
        uint8_t hash[HASH_LEN];
        uint8_t mediaType;
    } __attribute__((packed)) MediaHashCmd;

    explicit TCPClientModule(QObject *parent = 0);
    
signals:
    void uploadProgress(int percent);


public slots:
    void setServerParameters(QString host, uint16_t port);

    quint8 uploadFile(const QString &fileHash, MEDIA_TYPES fileType, const QString &filePath);

    bool initDataUpload(QTcpSocket *tcpSocket, const QByteArray &hashData, MEDIA_TYPES fileType);
    bool uploadMediaFileData(QTcpSocket *tcpSocket, const QByteArray &hashData, MEDIA_TYPES fileType, const QString &filePath);
    bool mediaDataVerify(QTcpSocket *tcpSocket, const QByteArray &hashData, MEDIA_TYPES fileType, const QString &filePath, media_size_t fileSize);

private:
    QSettings settings;

    uint16_t serverPort;
    QString serverHost;

    uint32_t readTimeout;
    
};

#endif // TCPCLIENTMODULE_H

#ifndef TCPLOADER_H
#define TCPLOADER_H

#include <QObject>
#include <QSettings>
#include <QTcpSocket>

#include <iblockloader.h>

#define AGENT_HASH_LEN  64

class TCPLoader : public IBlockLoader {
	Q_OBJECT
	Q_INTERFACES(IBlockLoader)
public:
    
    typedef struct {
        uint8_t hash[AGENT_HASH_LEN];
        uint8_t mediaType;
        media_size_t size;
        media_size_t offset;
    } __attribute__((packed)) GetMediaDataCmd;
    
    typedef struct {
        uint8_t hash[AGENT_HASH_LEN];
        uint8_t mediaType;
    } __attribute__((packed)) GetMediaSizeCmd;
    
    TCPLoader();
    virtual ~TCPLoader();
    
    QString getID() const { return "tcploader"; }
    
    QString description() const;

public slots:
	
	bool initBlockLoader();
	
	quint8 loadFile(const QString &fileHash, FILE_TYPE fileType, QString &filePath);
	
	quint8 cleanTempDir();
	
	quint8 scheduleUpdateCheck();
    
signals:
    void scheduleUpdateAvailable(const QString &scheduleDocument);
    
protected:
    bool getMediaFileSize(QTcpSocket *socket, const QByteArray &fileHashData, FILE_TYPE fileType, media_size_t &fileSize);
    
    bool getMediaFileData(QTcpSocket *socket, const QByteArray &fileHashData, FILE_TYPE fileType, uint8_t *buf, 
                          media_size_t offset, media_size_t size, media_size_t &readedSize);
    
    
    
private:
    QSettings settings;
    QString tempDir;
    uint16_t serverPort;
    QString serverHost;
    uint32_t readTimeout;

    QString mediaBasePath;
};

#endif // TCPLOADER_H

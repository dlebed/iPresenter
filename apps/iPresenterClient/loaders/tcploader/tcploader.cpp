#include "tcploader.h"

#include <QByteArray>
#include <QtPlugin>
#include <QFile>

#include <qlogger.h>
#include <networkprotoparser.h>
#include <typedefs.h>

#define DEFAULT_READ_TIMEOUT        (1000 * 120)
#define MEDIA_READ_BLOCK_SIZE       (128 * 1024)

TCPLoader::TCPLoader():
    serverPort(5115), readTimeout(DEFAULT_READ_TIMEOUT)
{
	QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_INFO) <<
			"TCP content loader starting. Server:" <<
			settings.value("loaders/tcploader/host", "localhost").toString() <<
			"; port:" << settings.value("loaders/tcploader/port", 5115).toUInt();
    
    serverPort = settings.value("loaders/tcploader/port", 5115).toUInt();
    serverHost = settings.value("loaders/tcploader/host", "localhost").toString();
	
    tempDir = settings.value("loaders/tcploader/tmpdir", "/tmp").toString();
    mediaBasePath = settings.value("storage/media_base_path", "/var/media").toString();
    readTimeout = settings.value("network/read_timeout", DEFAULT_READ_TIMEOUT).toUInt();
}

TCPLoader::~TCPLoader() {
	
}

bool TCPLoader::initBlockLoader() {
	
	return true;
}

quint8 TCPLoader::loadFile(const QString &fileHash, FILE_TYPE fileType, QString &filePath) {
    QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_INFO) << "Loading file type" << fileType << "with hash" << fileHash;
    
    NetworkProtoParser protoParser;
    int64_t bytesReaded;
    uint64_t bytesToRead;
    media_size_t fileSize;
    QTcpSocket socket;
    
    socket.connectToHost(serverHost, serverPort);
    
    if (!socket.waitForConnected(readTimeout)) {
        QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_ERROR) << "Connection to server timeout";
        return LOAD_CONNECTION_FAILED;
    }
    
    if (!getMediaFileSize(&socket, fileHash.toUtf8(), fileType, fileSize)) {
        QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_ERROR) << "Unable to get filesize from server for hash:" << fileHash;
        return LOAD_NO_SUCH_FILE;
    }
    
    QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_INFO) << "File size readed from server:" << fileSize << "hash:" << fileHash;
    
    

    socket.disconnectFromHost();
    if (socket.state() != QAbstractSocket::UnconnectedState)
        socket.waitForDisconnected();
	
	return LOAD_SUCCESS;
}

quint8 TCPLoader::cleanTempDir() {
	
	return LOAD_SUCCESS;
}

quint8 TCPLoader::scheduleUpdateCheck() {
	QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_TRACE) << "TCP Loader: Schedule update check";
	
	return LOAD_SUCCESS;
}

bool TCPLoader::getMediaFileSize(QTcpSocket *socket, const QByteArray &fileHashData, FILE_TYPE fileType, media_size_t &fileSize) {
    NetworkProtoParser protoParser;
    uint64_t bytesToRead;
    int64_t bytesReaded;
    GetMediaSizeCmd getSizeCmd;
    QByteArray dataBuf;
    uint8_t res;
    media_size_t tmpMediaSize;
    
    memset(&getSizeCmd, 0, sizeof(getSizeCmd));
    
    protoParser.makeHeader(AGENT_GET_MEDIA_SIZE);
    
    memcpy(getSizeCmd.hash, fileHashData.data(), (fileHashData.size() > AGENT_HASH_LEN) ? AGENT_HASH_LEN : fileHashData.size());
    getSizeCmd.mediaType = fileType;
    
    if (protoParser.appendPayloadData((uint8_t *)&getSizeCmd, sizeof(getSizeCmd)) != NetworkProtoParser::E_OK)
        return false;
        
    if (protoParser.packetData(dataBuf) != NetworkProtoParser::E_OK)
        return false;
    
    if (socket->write(dataBuf) != dataBuf.size())
        return false;
    
    dataBuf.clear();
    
    if (!socket->waitForBytesWritten())
        return false;
    
    // Packet written. Waiting for reply
    
    protoParser.clear();
    
    while ((bytesToRead = protoParser.bytesToReadCount()) > 0) {
        if (socket->bytesAvailable() <= 0) {
            if (!socket->waitForReadyRead(readTimeout)) {
                QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_INFO) << __FUNCTION__ << "Timeout reading header data from socket.";
                return false;
            }
        }
        
        uint8_t *packetBuf = new uint8_t[bytesToRead];
        Q_ASSERT(packetBuf != NULL);
        
        bytesReaded = socket->read((char *)packetBuf, bytesToRead);
        
        if (bytesReaded <= 0) {
            QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_WARN) << __FUNCTION__ << "Error reading header data from socket.";
            delete [] packetBuf;
            return false;
        }
        
        if ((res = protoParser.bytesReaded(packetBuf, bytesReaded)) != NetworkProtoParser::E_OK) {
            QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_INFO) << __FUNCTION__ << "Error parsing packet:" << res;
            delete [] packetBuf;
            return false;
        }
        
        delete [] packetBuf;
        packetBuf = NULL;
    }
    
    packet_size_t payloadSize;
    
    if (protoParser.payloadSize(payloadSize) != NetworkProtoParser::E_OK)
        return false;
    
    if (payloadSize >= sizeof(tmpMediaSize)) {
        if (protoParser.payloadData(dataBuf) != NetworkProtoParser::E_OK)
            return false;
        
        memcpy(&tmpMediaSize, dataBuf.data(), sizeof(tmpMediaSize));
        fileSize = tmpMediaSize;
    } else {
        return false;
    }
        
    
    return true;
}

QString TCPLoader::description() const {
#ifdef REVISION
	return "Content loader for TCP." " Revision: " REVISION ".";
#else
	return "Content loader for TCP.";
#endif
}

Q_EXPORT_PLUGIN2(tcploader, TCPLoader)

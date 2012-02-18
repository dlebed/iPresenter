#include "tcploader.h"

#include <QByteArray>
#include <QtPlugin>
#include <QFile>

#include <qlogger.h>
#include <networkprotoparser.h>
#include <typedefs.h>

#define DEFAULT_READ_TIMEOUT        (1000 * 120)
#define MEDIA_READ_BLOCK_SIZE       (128 * 1024)

#define AGENT_ID_LEN                128

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

    agentID = settings.value("ids/agent_id", "0000000000").toString();
}

TCPLoader::~TCPLoader() {
	
}

bool TCPLoader::initBlockLoader() {
	
	return true;
}

quint8 TCPLoader::loadFile(const QString &fileHash, FILE_TYPE fileType, QString &filePath) {
    QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_INFO) << "Loading file type" << fileType << "with hash" << fileHash;
    
    int64_t totalBytesReaded = 0;
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
    
    QFile mediaFile(filePath);
    
    if (!mediaFile.open(QIODevice::WriteOnly)) {
        QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_ERROR) << "Unable to create media file:" << filePath << fileHash;
        return LOAD_FILE_CREATE_FAILED;
    }
    
    uint8_t *fileDataBuf = new uint8_t[MEDIA_READ_BLOCK_SIZE];
    Q_ASSERT(fileDataBuf != NULL);
    
    while (totalBytesReaded < fileSize) {
        bytesToRead = ((fileSize - totalBytesReaded) > MEDIA_READ_BLOCK_SIZE) ? MEDIA_READ_BLOCK_SIZE : (fileSize - totalBytesReaded);
        
        QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_TRACE) << "Requested" << bytesToRead << "bytes. Total read:" << totalBytesReaded << fileHash;
        
        media_size_t bytesReaded;
        
        if (!getMediaFileData(&socket, fileHash.toUtf8(), fileType, fileDataBuf, totalBytesReaded, bytesToRead, bytesReaded)) {
            QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_ERROR) << "Unable to fetch media file data:" << filePath << fileHash;
            delete [] fileDataBuf;
            mediaFile.remove();
            return LOAD_ERROR;
        }
        
        if (bytesToRead != bytesReaded) {
            QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_ERROR) << "Readed file size mismatch:" << bytesToRead << bytesToRead;
            delete [] fileDataBuf;
            mediaFile.remove();
            return LOAD_ERROR;
        }

        mediaFile.write((char *)fileDataBuf, bytesReaded);

        totalBytesReaded += bytesReaded;
    }
    
    delete [] fileDataBuf;

    socket.disconnectFromHost();
    if (socket.state() != QAbstractSocket::UnconnectedState)
        socket.waitForDisconnected();
	
	return LOAD_SUCCESS;
}

quint8 TCPLoader::cleanTempDir() {
	
	return LOAD_SUCCESS;
}

quint8 TCPLoader::scheduleUpdateCheck(schedule_version_t currentScheduleVersion, QString & scheduleDocument) {
	QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_TRACE) << __FUNCTION__ << "TCP Loader: Schedule update check";

    QTcpSocket socket;

    socket.connectToHost(serverHost, serverPort);

    if (!socket.waitForConnected(readTimeout)) {
        QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_ERROR) << __FUNCTION__ << "Connection to server timeout";
        return LOAD_CONNECTION_FAILED;
    }

    schedule_version_t serverScheduleVersion = 0;

    if (!getScheduleVersion(&socket, serverScheduleVersion)) {
        QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_ERROR) << __FUNCTION__ << "Unable to get current schedule version from server";
        return LOAD_CMD_FAILURE;
    }

    QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_INFO) << __FUNCTION__ << "Schedule version returned by server:" << serverScheduleVersion;

    if (currentScheduleVersion >= serverScheduleVersion) {
        QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_INFO) << __FUNCTION__ << "There is no new version of schedule on server";
        socket.disconnectFromHost();
        if (socket.state() != QAbstractSocket::UnconnectedState)
            socket.waitForDisconnected();
        return LOAD_NO_UPDATE_AVALIABLE;
    }

    // New version of schedule exist

    QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_INFO) << __FUNCTION__ << "Found new version of schedule on server:" << serverScheduleVersion;

    if (!getScheduleData(&socket, scheduleDocument)) {
        QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_ERROR) << __FUNCTION__ << "Unable to get current schedule data from server";
        return LOAD_CMD_FAILURE;
    }

    if (scheduleDocument.isEmpty()) {
        QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_ERROR) << __FUNCTION__ << "Received empty schedule data from server";
        return LOAD_CMD_FAILURE;
    }

    QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_INFO) << __FUNCTION__ << "Schedule data successfully fetched from server";

    socket.disconnectFromHost();
    if (socket.state() != QAbstractSocket::UnconnectedState)
        socket.waitForDisconnected();

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

bool TCPLoader::getMediaFileData(QTcpSocket *socket, const QByteArray &fileHashData, FILE_TYPE fileType, uint8_t *buf, media_size_t offset, media_size_t size, media_size_t &readedSize) {
    NetworkProtoParser protoParser;
    uint64_t bytesToRead;
    int64_t bytesReaded;
    GetMediaDataCmd getDataCmd;
    QByteArray dataBuf;
    uint8_t res;
    
    memset(&getDataCmd, 0, sizeof(getDataCmd));
    
    protoParser.makeHeader(AGENT_GET_MEDIA_DATA);
    
    memcpy(getDataCmd.hash, fileHashData.data(), (fileHashData.size() > AGENT_HASH_LEN) ? AGENT_HASH_LEN : fileHashData.size());
    getDataCmd.mediaType = fileType;
    getDataCmd.size = size;
    getDataCmd.offset = offset;
    
    if (protoParser.appendPayloadData((uint8_t *)&getDataCmd, sizeof(getDataCmd)) != NetworkProtoParser::E_OK)
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
                //return false;
            }
        }
        
        uint8_t *packetBuf = new uint8_t[bytesToRead];
        Q_ASSERT(packetBuf != NULL);
        
        bytesReaded = socket->read((char *)packetBuf, bytesToRead);
        
        QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_TRACE) << __FUNCTION__ << "Readed:" << bytesReaded << "To read:" << bytesToRead;
        
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
    
    QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_TRACE) << __FUNCTION__ << "Packet readed successfully";
    
    packet_size_t payloadSize;
    
    if (protoParser.payloadSize(payloadSize) != NetworkProtoParser::E_OK)
        return false;
    
    if (payloadSize > size) {
        QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_ERROR) << "Readed more data than requested!";
        return false;
    } else {
        if (protoParser.payloadData(dataBuf) != NetworkProtoParser::E_OK)
            return false;
        
        readedSize = payloadSize;
        memcpy(buf, dataBuf.data(), payloadSize);
    }
    
    return true;
}

bool TCPLoader::getScheduleVersion(QTcpSocket *socket, schedule_version_t &scheduleVersion) {
    NetworkProtoParser protoParser;
    uint64_t bytesToRead;
    int64_t bytesReaded;
    QByteArray dataBuf;
    QByteArray agentIDData = agentID.toUtf8();
    uint8_t res;

    agentIDData.truncate(AGENT_ID_LEN);

    protoParser.makeHeader(AGENT_GET_SCHEDULE_VERSION);

    if (protoParser.appendPayloadData((uint8_t *)agentIDData.data(), agentIDData.size()) != NetworkProtoParser::E_OK)
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
                //return false;
            }
        }

        uint8_t *packetBuf = new uint8_t[bytesToRead];
        Q_ASSERT(packetBuf != NULL);

        bytesReaded = socket->read((char *)packetBuf, bytesToRead);

        QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_TRACE) << __FUNCTION__ << "Readed:" << bytesReaded << "To read:" << bytesToRead;

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

    QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_TRACE) << __FUNCTION__ << "Packet readed successfully";

    packet_size_t payloadSize;

    if (protoParser.payloadSize(payloadSize) != NetworkProtoParser::E_OK)
        return false;

    if (protoParser.payloadData(dataBuf) != NetworkProtoParser::E_OK)
        return false;

    scheduleVersion = 0;
    memcpy(&scheduleVersion, dataBuf.data(), (sizeof(scheduleVersion) <= payloadSize) ? sizeof(scheduleVersion) : payloadSize);

    QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_INFO) << __FUNCTION__ << "Server returned schedule version:" << scheduleVersion;

    return true;
}

bool TCPLoader::getScheduleData(QTcpSocket *socket, QString &scheduleData) {
    NetworkProtoParser protoParser;
    uint64_t bytesToRead;
    int64_t bytesReaded;
    QByteArray dataBuf;
    QByteArray agentIDData = agentID.toUtf8();
    uint8_t res;

    agentIDData.truncate(AGENT_ID_LEN);

    protoParser.makeHeader(AGENT_GET_SCHEDULE_DATA);

    if (protoParser.appendPayloadData((uint8_t *)agentIDData.data(), agentIDData.size()) != NetworkProtoParser::E_OK)
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
                //return false;
            }
        }

        uint8_t *packetBuf = new uint8_t[bytesToRead];
        Q_ASSERT(packetBuf != NULL);

        bytesReaded = socket->read((char *)packetBuf, bytesToRead);

        QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_TRACE) << __FUNCTION__ << "Readed:" << bytesReaded << "To read:" << bytesToRead;

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

    QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_TRACE) << __FUNCTION__ << "Packet readed successfully";

    packet_size_t payloadSize;

    if (protoParser.payloadSize(payloadSize) != NetworkProtoParser::E_OK)
        return false;

    if (protoParser.payloadData(dataBuf) != NetworkProtoParser::E_OK)
        return false;

    scheduleData = QString::fromUtf8(dataBuf.data(), dataBuf.size());

    QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_INFO) << __FUNCTION__ << "Server returned schedule data:" << scheduleData;

    return true;
}

QString TCPLoader::description() const {
	return "Content loader for TCP.";
}

Q_EXPORT_PLUGIN2(tcploader, TCPLoader)

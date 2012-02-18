#include "tcpclientmodule.h"

#include <QByteArray>
#include <QFile>

#include <qlogger.h>
#include <networkprotoparser.h>

#define DEFAULT_READ_TIMEOUT        (1000 * 120)
#define MEDIA_READ_BLOCK_SIZE       (128 * 1024)

TCPClientModule::TCPClientModule(QObject *parent) :
    QObject(parent), serverPort(5116)
{
    readTimeout = settings.value("network/read_timeout", DEFAULT_READ_TIMEOUT).toUInt();
}

void TCPClientModule::setServerParameters(QString host, uint16_t port) {
    serverPort = port;
    serverHost = host;
}

quint8 TCPClientModule::uploadFile(const QString &fileHash, MEDIA_TYPES fileType, const QString &filePath) {
    QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_INFO) << "Uploading file type" << fileType << "with hash" << fileHash << filePath;

    media_size_t fileSize;
    QTcpSocket socket;
    quint8 res;

    QFile mediaFile(filePath);

    if (!mediaFile.open(QIODevice::ReadOnly)) {
        QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_ERROR) << "File read error:" << filePath;
        return FILE_READ_ERROR;
    }

    fileSize = mediaFile.size();

    mediaFile.close();

    socket.connectToHost(serverHost, serverPort);

    if (!socket.waitForConnected(readTimeout)) {
        QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_ERROR) << "Connection to server timeout";
        return UPLOAD_CONNECTION_FAILED;
    }

    if ((res = initDataUpload(&socket, fileHash.toAscii(), fileType)) != E_OK) {
        QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_ERROR) << "Unable to initiate data upload to server";
        return UPLOAD_ERROR;
    }

    if ((res = uploadMediaFileData(&socket, fileHash.toAscii(), fileType, filePath)) != E_OK) {
        QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_ERROR) << "Unable to upload data to server";
        return UPLOAD_ERROR;
    }

    if ((res = mediaDataVerify(&socket, fileHash.toAscii(), fileType, filePath, fileSize)) != E_OK) {
        QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_ERROR) << "Uploaded data verification error...";
        return UPLOAD_ERROR;
    }

    QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_ERROR) << "Media file uploaded successfully:" << fileSize << fileHash << filePath;

    return E_OK;
}

bool TCPClientModule::initDataUpload(QTcpSocket *tcpSocket, const QByteArray &hashData, MEDIA_TYPES fileType) {
    NetworkProtoParser protoParser;
    uint64_t bytesToRead;
    int64_t bytesReaded;
    MediaHashCmd mediaHashCmd;
    QByteArray dataBuf;
    uint8_t res;
    uint8_t cmdExecResult = ADMIN_NACK;

    memset(&mediaHashCmd, 0, sizeof(mediaHashCmd));

    protoParser.makeHeader(ADMIN_INIT_DATA_UPLOAD);

    memcpy(mediaHashCmd.hash, hashData.data(), (hashData.size() > HASH_LEN) ? HASH_LEN : hashData.size());
    mediaHashCmd.mediaType = fileType;

    if (protoParser.appendPayloadData((uint8_t *)&mediaHashCmd, sizeof(mediaHashCmd)) != NetworkProtoParser::E_OK)
        return false;

    if (protoParser.packetData(dataBuf) != NetworkProtoParser::E_OK)
        return false;

    if (tcpSocket->write(dataBuf) != dataBuf.size())
        return false;

    dataBuf.clear();

    if (!tcpSocket->waitForBytesWritten())
        return false;

    // Packet written. Waiting for reply

    protoParser.clear();

    while ((bytesToRead = protoParser.bytesToReadCount()) > 0) {
        if (tcpSocket->bytesAvailable() <= 0) {
            if (!tcpSocket->waitForReadyRead(readTimeout)) {
                QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_INFO) << __FUNCTION__ << "Timeout reading header data from socket.";
                return false;
            }
        }

        uint8_t *packetBuf = new uint8_t[bytesToRead];
        Q_ASSERT(packetBuf != NULL);

        bytesReaded = tcpSocket->read((char *)packetBuf, bytesToRead);

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

    if (payloadSize >= sizeof(cmdExecResult)) {
        if (protoParser.payloadData(dataBuf) != NetworkProtoParser::E_OK)
            return false;

        memcpy(&cmdExecResult, dataBuf.data(), sizeof(cmdExecResult));

        if (cmdExecResult != ADMIN_OK) {
            QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_ERROR) << __FUNCTION__ << "Nack received from server:" << cmdExecResult;
            return false;
        }

    } else {
        return false;
    }

    QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_INFO) << __FUNCTION__ << "File upload initiation succeed";

    return true;
}

bool TCPClientModule::uploadMediaFileData(QTcpSocket *tcpSocket, const QByteArray &hashData, MEDIA_TYPES fileType, const QString &filePath) {
    NetworkProtoParser protoParser;
    uint64_t bytesToRead;
    int64_t bytesReaded;
    MediaUploadCmd mediaUploadCmd;
    QByteArray dataBuf;
    uint8_t res;
    uint8_t cmdExecResult = ADMIN_NACK;
    uint8_t *read_buf;
    media_size_t fileBytesReaded, totalBytesTransferred = 0, fileSize;

    QFile mediaFile(filePath);

    if (!mediaFile.open(QIODevice::ReadOnly)) {
        QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_ERROR) << __FUNCTION__ << "Can't open file for reading:" << filePath;
        return FILE_READ_ERROR;
    }

    fileSize = mediaFile.size();

    read_buf = new uint8_t[MEDIA_READ_BLOCK_SIZE];

    while ((fileBytesReaded = mediaFile.read((char*)read_buf, MEDIA_READ_BLOCK_SIZE)) > 0) {
        memset(&mediaUploadCmd, 0, sizeof(mediaUploadCmd));
        protoParser.makeHeader(ADMIN_DATA_UPLOAD);
        memcpy(mediaUploadCmd.hash, hashData.data(), (hashData.size() > HASH_LEN) ? HASH_LEN : hashData.size());
        mediaUploadCmd.mediaType = fileType;
        mediaUploadCmd.size = fileBytesReaded;

        if (protoParser.appendPayloadData((uint8_t *)&mediaUploadCmd, sizeof(mediaUploadCmd)) != NetworkProtoParser::E_OK)
            return false;

        if (protoParser.appendPayloadData(read_buf, fileBytesReaded) != NetworkProtoParser::E_OK) {
            delete [] read_buf;
            return false;
        }

        if (protoParser.packetData(dataBuf) != NetworkProtoParser::E_OK) {
            delete [] read_buf;
            return false;
        }

        if (tcpSocket->write(dataBuf) != dataBuf.size()) {
            delete [] read_buf;
            return false;
        }

        dataBuf.clear();

        if (!tcpSocket->waitForBytesWritten()) {
            delete [] read_buf;
            return false;
        }

        // Packet written. Waiting for reply

        protoParser.clear();

        while ((bytesToRead = protoParser.bytesToReadCount()) > 0) {
            if (tcpSocket->bytesAvailable() <= 0) {
                if (!tcpSocket->waitForReadyRead(readTimeout)) {
                    QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_INFO) << __FUNCTION__ << "Timeout reading header data from socket.";
                    delete [] read_buf;
                    return false;
                }
            }

            uint8_t *packetBuf = new uint8_t[bytesToRead];
            Q_ASSERT(packetBuf != NULL);

            bytesReaded = tcpSocket->read((char *)packetBuf, bytesToRead);

            if (bytesReaded <= 0) {
                QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_WARN) << __FUNCTION__ << "Error reading header data from socket.";
                delete [] packetBuf;
                delete [] read_buf;
                return false;
            }

            if ((res = protoParser.bytesReaded(packetBuf, bytesReaded)) != NetworkProtoParser::E_OK) {
                QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_INFO) << __FUNCTION__ << "Error parsing packet:" << res;
                delete [] packetBuf;
                delete [] read_buf;
                return false;
            }

            delete [] packetBuf;
            packetBuf = NULL;
        }

        packet_size_t payloadSize;

        if (protoParser.payloadSize(payloadSize) != NetworkProtoParser::E_OK) {
            delete [] read_buf;
            return false;
        }

        if (payloadSize >= sizeof(cmdExecResult)) {
            if (protoParser.payloadData(dataBuf) != NetworkProtoParser::E_OK) {
                delete [] read_buf;
                return false;
            }

            memcpy(&cmdExecResult, dataBuf.data(), sizeof(cmdExecResult));

            if (cmdExecResult != ADMIN_OK) {
                QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_ERROR) << __FUNCTION__ << "Nack received from server:" << cmdExecResult;
                delete [] read_buf;
                return false;
            }

        } else {
            delete [] read_buf;
            return false;
        }

        totalBytesTransferred += fileBytesReaded;

        emit uploadProgress((totalBytesTransferred * 100) / fileSize);
    }

    mediaFile.close();
    delete [] read_buf;

    QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_INFO) << __FUNCTION__ << "File upload succeed";

    return true;
}

bool TCPClientModule::mediaDataVerify(QTcpSocket *tcpSocket, const QByteArray &hashData, MEDIA_TYPES fileType, const QString &filePath, media_size_t fileSize) {
    NetworkProtoParser protoParser;
    uint64_t bytesToRead;
    int64_t bytesReaded;
    MediaUploadCmd mediaUploadCmd;
    QByteArray dataBuf;
    uint8_t res;
    uint8_t cmdExecResult = ADMIN_NACK;

    memset(&mediaUploadCmd, 0, sizeof(mediaUploadCmd));

    protoParser.makeHeader(ADMIN_DATA_UPLOAD_VERIFY);

    memcpy(mediaUploadCmd.hash, hashData.data(), (hashData.size() > HASH_LEN) ? HASH_LEN : hashData.size());
    mediaUploadCmd.mediaType = fileType;
    mediaUploadCmd.size = fileSize;

    if (protoParser.appendPayloadData((uint8_t *)&mediaUploadCmd, sizeof(mediaUploadCmd)) != NetworkProtoParser::E_OK)
        return false;

    if (protoParser.packetData(dataBuf) != NetworkProtoParser::E_OK)
        return false;

    if (tcpSocket->write(dataBuf) != dataBuf.size())
        return false;

    dataBuf.clear();

    if (!tcpSocket->waitForBytesWritten())
        return false;

    // Packet written. Waiting for reply

    protoParser.clear();

    while ((bytesToRead = protoParser.bytesToReadCount()) > 0) {
        if (tcpSocket->bytesAvailable() <= 0) {
            if (!tcpSocket->waitForReadyRead(readTimeout)) {
                QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_INFO) << __FUNCTION__ << "Timeout reading header data from socket.";
                return false;
            }
        }

        uint8_t *packetBuf = new uint8_t[bytesToRead];
        Q_ASSERT(packetBuf != NULL);

        bytesReaded = tcpSocket->read((char *)packetBuf, bytesToRead);

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

    if (payloadSize >= sizeof(cmdExecResult)) {
        if (protoParser.payloadData(dataBuf) != NetworkProtoParser::E_OK)
            return false;

        memcpy(&cmdExecResult, dataBuf.data(), sizeof(cmdExecResult));

        if (cmdExecResult != ADMIN_OK) {
            QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_ERROR) << __FUNCTION__ << "Nack received from server:" << cmdExecResult;
            return false;
        }

    } else {
        return false;
    }

    QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_INFO) << __FUNCTION__ << "File verification succeed";

    return true;
}

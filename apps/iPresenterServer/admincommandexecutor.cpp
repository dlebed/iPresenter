#include "admincommandexecutor.h"

#include <QByteArray>
#include <QSettings>
#include <QFile>
#include <QDomElement>
#include <QDomDocument>

#include <qlogger.h>

#include "db/dbproxyfactory.h"


#define MEDIA_READ_BLOCK_SIZE       (4 * 1024)

AdminCommandExecutor::AdminCommandExecutor() :
    dbProxy(NULL), hashCalculator(NULL)
{
    dbProxy = DBProxyFactory::dbProxy();
    Q_ASSERT(dbProxy != NULL);
    
    QSettings settings;
    mediaBasePath = settings.value("storage/media_base_path", "/var/media").toString();

    hashCalculator = HashCalculatorFactory::hashCalculatorInstance(settings.value("hash/type", "sha256").toString());
    Q_ASSERT(hashCalculator != NULL);
}

AdminCommandExecutor::~AdminCommandExecutor() {
    
    dbProxy = NULL;
    DBProxyFactory::freeThreadDBProxy();
    
}

uint8_t AdminCommandExecutor::executeCommand(const NetworkProtoParser &protoParser, QTcpSocket *tcpSocket) {
    QByteArray payloadData;
    MediaHashCmd *mediaHashCmd = NULL;
    MediaUploadCmd *mediaUploadCmd = NULL;
    uint8_t res;
    packet_cmd_t cmd;
    
    if (protoParser.payloadData(payloadData) != NetworkProtoParser::E_OK)
        return E_PAYLOAD_LEN;
    
    if ((res = protoParser.cmd(cmd)) != NetworkProtoParser::E_OK)
        if ((res = protoParser.cmd(cmd)) != NetworkProtoParser::E_OK)
        return res;
    
    /*if (cmd == AGENT_GET_SCHEDULE_VERSION || cmd == AGENT_GET_SCHEDULE_DATA) {
        if (payloadData.size() > AGENT_ID_LEN || payloadData.size() == 0)
            return E_PAYLOAD_LEN;

        agentID = payloadData.left(AGENT_ID_LEN);
    }*/
    
    QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_TRACE) << __FUNCTION__ << "Starting to execute admin command" << cmd;
    
    switch (cmd) {
    case ADMIN_INIT_DATA_UPLOAD:
        if (payloadData.size() < sizeof(MediaHashCmd))
            return E_PAYLOAD_LEN;

        mediaHashCmd = new MediaHashCmd;
        Q_ASSERT(mediaHashCmd != NULL);

        memcpy(mediaHashCmd, payloadData.data(), sizeof(MediaHashCmd));

        res = initDataUpload(tcpSocket, mediaHashCmd);

        delete mediaHashCmd;

        return res;

        break;

    case ADMIN_DATA_UPLOAD:
        if (payloadData.size() < sizeof(MediaUploadCmd))
            return E_PAYLOAD_LEN;

        mediaUploadCmd = new MediaUploadCmd;
        Q_ASSERT(mediaUploadCmd != NULL);

        memcpy(mediaUploadCmd, payloadData.data(), sizeof(MediaUploadCmd));

        res = appendMediaFileData(tcpSocket, mediaUploadCmd, (uint8_t*)payloadData.data() + sizeof(MediaUploadCmd),
                                  payloadData.size() - sizeof(MediaUploadCmd));

        delete mediaUploadCmd;

        return res;

        break;

    case ADMIN_DATA_UPLOAD_VERIFY:
        if (payloadData.size() < sizeof(MediaUploadCmd))
            return E_PAYLOAD_LEN;

        mediaUploadCmd = new MediaUploadCmd;
        Q_ASSERT(mediaUploadCmd != NULL);

        memcpy(mediaUploadCmd, payloadData.data(), sizeof(MediaUploadCmd));

        res = mediaDataVerify(tcpSocket, mediaUploadCmd);

        delete mediaUploadCmd;

        return res;

        break;

    case ADMIN_GET_MEDIA_DATA:
        if (payloadData.size() < sizeof(MediaUploadCmd))
            return E_PAYLOAD_LEN;
        
        mediaUploadCmd = new MediaUploadCmd;
        Q_ASSERT(mediaUploadCmd != NULL);
        
        memcpy(mediaUploadCmd, payloadData.data(), sizeof(MediaUploadCmd));
        
        res = getMediaData(tcpSocket, mediaUploadCmd);
        
        delete mediaUploadCmd;
        
        return res;
        
        break;
    
    default:
        return E_UNKNOWN_CMD;
    }
 
    return E_OK;
    
}

uint8_t AdminCommandExecutor::initDataUpload(QTcpSocket *tcpSocket, MediaHashCmd *mediaHashCmd) {
    uint8_t res;
    NetworkProtoParser packetParser;
    QByteArray packetData;
    QString hash = QString::fromUtf8((char*)mediaHashCmd->hash, sizeof(mediaHashCmd->hash));
    media_size_t size;
    uint8_t resultCode = ADMIN_NACK;;
    QString mediaTypeDir;

    switch (mediaHashCmd->mediaType) {
    case MEDIA_IMAGE:
        mediaTypeDir = "images";
        break;
    case MEDIA_MOVIE:
        mediaTypeDir = "movies";
        break;
    default:
        QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_ERROR) << __FUNCTION__ << "Unknown media type:" <<mediaHashCmd->mediaType;
        return E_UNKNOWN_MEDIA_TYPE;
    }

    Q_ASSERT(dbProxy != NULL);

    res = dbProxy->getMediaSize(hash, (MEDIA_TYPES)mediaHashCmd->mediaType, size);

    if (res != IDBProxy::E_OK) {
        if (QFile::exists(mediaBasePath + "/" + mediaTypeDir + "/" + hash)) {
            if (QFile::remove(mediaBasePath + "/" + mediaTypeDir + "/" + hash)) {
                resultCode = ADMIN_OK;
            } else {
                QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_ERROR) << __FUNCTION__ << "Can't remove existing file:" << mediaBasePath + "/" + mediaTypeDir + "/" + hash;
            }
        } else {
            resultCode = ADMIN_OK;
        }
    } else {
        QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_ERROR) << __FUNCTION__ << "Media file exists in DB:" << hash;
    }

    if (resultCode == ADMIN_OK) {
        QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_INFO) << __FUNCTION__ << "Upload of media file initiated:" << hash;
    }

    packetParser.makeHeader(ADMIN_INIT_DATA_UPLOAD);

    if (packetParser.appendPayloadData((uint8_t*)&resultCode, sizeof(resultCode)) != NetworkProtoParser::E_OK)
        return E_MAKE_PACKET;

    if (packetParser.packetData(packetData) != NetworkProtoParser::E_OK)
        return E_GET_DATA;

    if (tcpSocket->write(packetData.data(), packetData.size()) != packetData.size())
        return E_SOCKET_WRITE_ERROR;

    if (!tcpSocket->waitForBytesWritten())
        return E_SOCKET_WRITE_ERROR;

    return E_OK;
}

uint8_t AdminCommandExecutor::appendMediaFileData(QTcpSocket *tcpSocket, MediaUploadCmd *mediaUploadCmd, uint8_t *data, media_size_t dataSize) {
    uint8_t res;
    NetworkProtoParser packetParser;
    QByteArray packetData;
    QString hash = QString::fromUtf8((char*)mediaUploadCmd->hash, sizeof(mediaUploadCmd->hash));
    media_size_t size;
    uint8_t resultCode = ADMIN_NACK;
    QString mediaTypeDir;

    switch (mediaUploadCmd->mediaType) {
    case MEDIA_IMAGE:
        mediaTypeDir = "images";
        break;
    case MEDIA_MOVIE:
        mediaTypeDir = "movies";
        break;
    default:
        QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_ERROR) << __FUNCTION__ << "Unknown media type:" << mediaUploadCmd->mediaType;
        return E_UNKNOWN_MEDIA_TYPE;
    }

    Q_ASSERT(dbProxy != NULL);

    res = dbProxy->getMediaSize(hash, (MEDIA_TYPES)mediaUploadCmd->mediaType, size);

    if (res != IDBProxy::E_OK) {
        QFile mediaFile(mediaBasePath + "/" + mediaTypeDir + "/" + hash);

        if (mediaFile.open(QIODevice::WriteOnly | QIODevice::Append)) {
            if (mediaFile.write((char*)data, dataSize) == dataSize) {
                resultCode = ADMIN_OK;
            } else {
                QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_ERROR) << __FUNCTION__ << "Data write mismatch:" << hash << size;
            }

            mediaFile.close();
        } else {
            QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_ERROR) << __FUNCTION__ << "Can't open media file:" << mediaFile.fileName();
        }
    } else {
        QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_ERROR) << __FUNCTION__ << "Media file exists in DB:" << hash;
    }

    if (resultCode == ADMIN_OK) {
        QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_INFO) << __FUNCTION__ << "Media file data append success:" << hash << size;
    }

    packetParser.makeHeader(ADMIN_INIT_DATA_UPLOAD);

    if (packetParser.appendPayloadData((uint8_t*)&resultCode, sizeof(resultCode)) != NetworkProtoParser::E_OK)
        return E_MAKE_PACKET;

    if (packetParser.packetData(packetData) != NetworkProtoParser::E_OK)
        return E_GET_DATA;

    if (tcpSocket->write(packetData.data(), packetData.size()) != packetData.size())
        return E_SOCKET_WRITE_ERROR;

    if (!tcpSocket->waitForBytesWritten())
        return E_SOCKET_WRITE_ERROR;

    return E_OK;
}

uint8_t AdminCommandExecutor::mediaDataVerify(QTcpSocket *tcpSocket, MediaUploadCmd *mediaUploadCmd) {
    uint8_t res;
    NetworkProtoParser packetParser;
    QByteArray packetData;
    QString hash = QString::fromUtf8((char*)mediaUploadCmd->hash, sizeof(mediaUploadCmd->hash));
    uint8_t resultCode = ADMIN_NACK;
    QString mediaTypeDir;

    switch (mediaUploadCmd->mediaType) {
    case MEDIA_IMAGE:
        mediaTypeDir = "images";
        break;
    case MEDIA_MOVIE:
        mediaTypeDir = "movies";
        break;
    default:
        QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_ERROR) << __FUNCTION__ << "Unknown media type:" << mediaUploadCmd->mediaType;
        return E_UNKNOWN_MEDIA_TYPE;
    }

    QFile mediaFile(mediaBasePath + "/" + mediaTypeDir + "/" + hash);

    if (mediaFile.open(QIODevice::ReadOnly)) {
        if (mediaFile.size() == mediaUploadCmd->size) {
            mediaFile.close();
            QString calculatedHash = hashCalculator->getFileHash(mediaBasePath + "/" + mediaTypeDir + "/" + hash);
            if (calculatedHash == hash) {
                resultCode = ADMIN_OK;
            } else {
                QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_ERROR) << __FUNCTION__ << "Media file hash mismatch" << hash << "!=" << calculatedHash;
            }
        } else {
            QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_ERROR) << __FUNCTION__ << "Media file size mismatch:" << mediaUploadCmd->size << "!=" << mediaFile.size();
            mediaFile.close();
        }
    } else {
        QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_ERROR) << __FUNCTION__ << "Media open error:" << mediaBasePath + "/" + mediaTypeDir + "/" + hash;
    }

    if (resultCode == ADMIN_OK) {
        QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_ERROR) << __FUNCTION__ << "Media file check succeed" << mediaBasePath + "/" + mediaTypeDir + "/" + hash;
    }

    packetParser.makeHeader(ADMIN_INIT_DATA_UPLOAD);

    if (packetParser.appendPayloadData((uint8_t*)&resultCode, sizeof(resultCode)) != NetworkProtoParser::E_OK)
        return E_MAKE_PACKET;

    if (packetParser.packetData(packetData) != NetworkProtoParser::E_OK)
        return E_GET_DATA;

    if (tcpSocket->write(packetData.data(), packetData.size()) != packetData.size())
        return E_SOCKET_WRITE_ERROR;

    if (!tcpSocket->waitForBytesWritten())
        return E_SOCKET_WRITE_ERROR;

    return E_OK;
}

uint8_t AdminCommandExecutor::getMediaData(QTcpSocket *tcpSocket, MediaUploadCmd *cmdData) {
    media_size_t mediaSize, totalBytes;
    int64_t bytesReaded;
    QString hash = QString::fromUtf8((char *)cmdData->hash, sizeof(cmdData->hash));
    QString mediaTypeDir;
    NetworkProtoParser packetParser;
    QByteArray packetData;
    uint8_t res;
    
    switch (cmdData->mediaType) {
    case MEDIA_IMAGE:
        mediaTypeDir = "images";
        break;
    case MEDIA_MOVIE:
        mediaTypeDir = "movies";
        break;
    default:
        return E_UNKNOWN_MEDIA_TYPE;
    }
    
    res = dbProxy->getMediaSize(hash, (MEDIA_TYPES)cmdData->mediaType, mediaSize);
    
    if (res == IDBProxy::E_EMPTY_SELECT_RESULT) {
        return E_MEDIA_FILE_NOT_FOUND;
    } else if (res != IDBProxy::E_OK) {
        return E_DB_ERROR;
    }
    
    QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_TRACE) << __FUNCTION__ << "Trying to read media file. Total file size:" << mediaSize <<
                                                           "; Requested offset:" << cmdData->offset << "; size:" << cmdData->size << "; hash:" << hash;
    
    QFile mediaFile(mediaBasePath + "/" + mediaTypeDir + "/" + hash);    
    
    if (!mediaFile.exists()) {
        QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_ERROR) << __FUNCTION__ << "Hash exist in DB, but it data does not exists:" << hash;
        return E_MEDIA_FILE_NOT_FOUND;
    }
    
    if (!mediaFile.open(QIODevice::ReadOnly)) {
        QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_ERROR) << __FUNCTION__ << "Can't read file for hash:" << hash;
        return E_FILE_READ_ERROR;
    }
    
    if (mediaFile.size() != mediaSize) {
        QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_ERROR) << __FUNCTION__ << "Media size mismatch! Size:" << 
                                                               mediaSize << "; file size:" << mediaFile.size() << "hash:" << hash;
        return E_MEDIA_SIZE_MISMATCH;
    }
    
    if ((cmdData->offset + cmdData->size) > mediaSize) {
        QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_ERROR) << __FUNCTION__ << "Requested size is too big! Size:" << 
                                                               cmdData->size << "; offset:" << cmdData->offset << "; file size:" << mediaFile.size() << "hash:" << hash;
        return E_PARAMETERS_INVALID;
    }
    
    packetParser.makeHeader(ADMIN_GET_MEDIA_DATA);
    if (packetParser.incPayloadSize(cmdData->size) != NetworkProtoParser::E_OK)
        return E_MAKE_PACKET;
    
    if (packetParser.packetData(packetData) != NetworkProtoParser::E_OK)
        return E_GET_DATA;
    
    tcpSocket->write(packetData);
    
    uint8_t dataBuf[MEDIA_READ_BLOCK_SIZE];
    
    totalBytes = 0;
    
    mediaFile.seek(cmdData->offset);
    
    while (totalBytes < cmdData->size) {
        bytesReaded = mediaFile.read((char *)dataBuf, (MEDIA_READ_BLOCK_SIZE > cmdData->size) ? cmdData->size : MEDIA_READ_BLOCK_SIZE);
        
        if (bytesReaded <= 0)
            break;
        
        totalBytes += bytesReaded;
        
        if (tcpSocket->write((char *)dataBuf, bytesReaded) != bytesReaded)
            return E_SOCKET_WRITE_ERROR;
    }
    
    mediaFile.close();
    
    if (!tcpSocket->waitForBytesWritten())
        return E_SOCKET_WRITE_ERROR;
    
    return E_OK;
}

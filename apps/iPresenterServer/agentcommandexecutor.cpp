#include "agentcommandexecutor.h"

#include <QByteArray>
#include <QSettings>
#include <QFile>
#include <QDomElement>
#include <QDomDocument>

#include <qlogger.h>

#include "db/dbproxyfactory.h"


#define MEDIA_READ_BLOCK_SIZE       (4 * 1024)

AgentCommandExecutor::AgentCommandExecutor() :
    dbProxy(NULL)
{
    dbProxy = DBProxyFactory::dbProxy();
    Q_ASSERT(dbProxy != NULL);
    
    QSettings settings;
    mediaBasePath = settings.value("storage/media_base_path", "/var/media").toString();
}

AgentCommandExecutor::~AgentCommandExecutor() {
    
    dbProxy = NULL;
    DBProxyFactory::freeThreadDBProxy();
    
}

uint8_t AgentCommandExecutor::executeCommand(const NetworkProtoParser &protoParser, QTcpSocket *tcpSocket) {
    QByteArray payloadData;
    QString agentID;
    GetMediaDataCmd *cmdGetData = NULL;
    GetMediaSizeCmd *cmdGetSize = NULL;
    uint8_t res;
    packet_cmd_t cmd;
    
    if (protoParser.payloadData(payloadData) != NetworkProtoParser::E_OK)
        return E_PAYLOAD_LEN;
    
    if ((res = protoParser.cmd(cmd)) != NetworkProtoParser::E_OK)
        if ((res = protoParser.cmd(cmd)) != NetworkProtoParser::E_OK)
        return res;
    
    if (cmd == AGENT_GET_SCHEDULE_VERSION || cmd == AGENT_GET_SCHEDULE_DATA) {
        if (payloadData.size() > AGENT_ID_LEN || payloadData.size() == 0)
            return E_PAYLOAD_LEN;

        agentID = payloadData.left(AGENT_ID_LEN);
    }
    
    QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_TRACE) << __FUNCTION__ << "Starting to execute command" << cmd;
    
    switch (cmd) {
    case AGENT_GET_SCHEDULE_VERSION:
        return getScheduleVersion(tcpSocket, agentID);
        break;
       
    case AGENT_GET_SCHEDULE_DATA:
        return getScheduleVersion(tcpSocket, agentID);
        break;
        
    case AGENT_GET_MEDIA_SIZE:
        if (payloadData.size() < sizeof(GetMediaSizeCmd))
            return E_PAYLOAD_LEN;
        
        cmdGetSize = new GetMediaSizeCmd;
        Q_ASSERT(cmdGetSize != NULL);
        
        memcpy(cmdGetSize, payloadData.data(), sizeof(GetMediaSizeCmd));
        
        res = getMediaSize(tcpSocket, cmdGetSize);
        
        delete cmdGetSize;
        
        return res;
            
        break;
        
    case AGENT_GET_MEDIA_DATA:
        if (payloadData.size() < sizeof(GetMediaDataCmd))
            return E_PAYLOAD_LEN;
        
        cmdGetData = new GetMediaDataCmd;
        Q_ASSERT(cmdGetData != NULL);
        
        memcpy(cmdGetData, payloadData.data(), sizeof(GetMediaDataCmd));
        
        res = getMediaData(tcpSocket, cmdGetData);
        
        delete cmdGetData;
        
        return res;
        
        break;
    
    default:
        return E_UNKNOWN_CMD;
    }
 
    return E_OK;
    
}

uint8_t AgentCommandExecutor::getScheduleVersion(QTcpSocket *tcpSocket, const QString &agentID) {
    uint8_t res;
    schedule_version_t version;
    NetworkProtoParser packetParser;
    QByteArray packetData;

    Q_ASSERT(dbProxy != NULL);
    
    res = dbProxy->getScheduleVersion(agentID, version);

    if (res == IDBProxy::E_EMPTY_SELECT_RESULT)
        return E_UNKNOWN_AGENT_ID;
    else if (res != IDBProxy::E_OK)
        return E_DB_ERROR;
    
    packetParser.makeHeader(AGENT_GET_SCHEDULE_VERSION);
    if (packetParser.appendPayloadData((uint8_t*)&version, sizeof(version)) != NetworkProtoParser::E_OK)
        return E_MAKE_PACKET;
    
    if (packetParser.packetData(packetData) != NetworkProtoParser::E_OK)
        return E_GET_DATA;
    
    if (tcpSocket->write(packetData.data(), packetData.size()) != packetData.size())
        return E_SOCKET_WRITE_ERROR;
    
    if (!tcpSocket->waitForBytesWritten())
        return E_SOCKET_WRITE_ERROR;
        
    return E_OK;
}

uint8_t AgentCommandExecutor::getScheduleData(QTcpSocket *tcpSocket, const QString &agentID) {
    uint8_t res;
    QString scheduleData;
    NetworkProtoParser packetParser;
    QByteArray packetData;
    QDomDocument scheduleDocument;
    Q_ASSERT(dbProxy != NULL);
    
    res = dbProxy->getScheduleData(agentID, scheduleData);
    
    if (res == IDBProxy::E_EMPTY_SELECT_RESULT)
        return E_UNKNOWN_AGENT_ID;
    else if (res != IDBProxy::E_OK)
        return E_DB_ERROR;
    
    if (!scheduleDocument.setContent(scheduleData))
        return E_XML_PARSE_ERROR;
    
    if ((res = fillScheduleBlocks(scheduleDocument)) != E_OK)
        return res;
    
    scheduleData = scheduleDocument.toString();
    
    packetParser.makeHeader(AGENT_GET_SCHEDULE_DATA);
    if (packetParser.appendPayloadData(scheduleData.toUtf8()) != NetworkProtoParser::E_OK)
        return E_MAKE_PACKET;
    
    if (packetParser.packetData(packetData) != NetworkProtoParser::E_OK)
        return E_GET_DATA;
    
    if (tcpSocket->write(packetData) != packetData.size())
        return E_SOCKET_WRITE_ERROR;
    
    if (!tcpSocket->waitForBytesWritten())
        return E_SOCKET_WRITE_ERROR;
    
    return E_OK;
}

uint8_t AgentCommandExecutor::getMediaSize(QTcpSocket *tcpSocket, GetMediaSizeCmd *cmdData) {
    media_size_t mediaSize;
    NetworkProtoParser packetParser;
    QByteArray packetData;
    uint8_t res;
    QString hash = QString::fromUtf8((char*)cmdData->hash, sizeof(cmdData->hash));
    Q_ASSERT(dbProxy != NULL);
    
    res = dbProxy->getMediaSize(hash, (MEDIA_TYPES)cmdData->mediaType, mediaSize);

    if (res == IDBProxy::E_EMPTY_SELECT_RESULT)
        return E_UNKNOWN_AGENT_ID;
    else if (res != IDBProxy::E_OK)
        return E_DB_ERROR;
    
    QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_TRACE) << __FUNCTION__ << "Media size:" << mediaSize << "hash:" << hash;
    
    packetParser.makeHeader(AGENT_GET_MEDIA_SIZE);
    if (packetParser.appendPayloadData((uint8_t*)&mediaSize, sizeof(mediaSize)) != NetworkProtoParser::E_OK)
        return E_MAKE_PACKET;
    
    if (packetParser.packetData(packetData) != NetworkProtoParser::E_OK)
        return E_GET_DATA;
    
    if (tcpSocket->write(packetData) != packetData.size())
        return E_SOCKET_WRITE_ERROR;
    
    if (!tcpSocket->waitForBytesWritten())
        return E_SOCKET_WRITE_ERROR;
    
    return E_OK;
}

uint8_t AgentCommandExecutor::getMediaData(QTcpSocket *tcpSocket, GetMediaDataCmd *cmdData) {
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
    
    packetParser.makeHeader(AGENT_GET_MEDIA_DATA);
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

uint8_t AgentCommandExecutor::fillScheduleBlocks(QDomDocument &scheduleDocument) {
    QDomNodeList blockNodes = scheduleDocument.elementsByTagName("block");
    QSet<QString> blockIDsList;
    
    //! Generating used block set
    for (int i = 0; i < blockNodes.size(); i++) {
        QDomElement blockElement = blockNodes.at(i).toElement();
        if (blockElement.isNull())
            continue;
        
        if (!blockElement.attribute("id").isEmpty()) {
            blockIDsList.insert(blockElement.attribute("id"));
        }
    }
    
    QDomElement blocksElement = scheduleDocument.createElement("blocks");
    scheduleDocument.documentElement().appendChild(blocksElement);
    
    foreach (const QString &blockID, blockIDsList) {
        QString blockData;
        QDomDocument blockDocument;
        uint8_t res;
        
        res = dbProxy->getBlockData(blockID, blockData);
        
        if (res == IDBProxy::E_EMPTY_SELECT_RESULT) {
            QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_ERROR) << __FUNCTION__ << "Unknown block id:" << blockID;
            continue;
        } else if (res != IDBProxy::E_OK) {
            return E_DB_ERROR;
        }
        
        if (!blockDocument.setContent(blockData)) {
            QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_ERROR) << __FUNCTION__ << "Unable to parse block:" << blockID << blockData;
            continue;
        }
        
        blocksElement.appendChild(blockDocument.documentElement());
    }
    
    return E_OK;
}

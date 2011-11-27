#include "agentsservertask.h"

#include <QTcpSocket>
#include <QHostAddress>
#include <QSettings>

#include <qlogger.h>

#define PACKET_HEADER_SIZE          6

#define DEFAULT_MAX_PACKET_SIZE     (1024 * 128 + PACKET_HEADER_SIZE)
#define DEFAULT_READ_TIMEOUT        (1000 * 120)

#define STX                         0x02
#define PACKET_PAYLOAD_LEN_OFFSET   1
#define PACKET_CMD_OFFSET           5
#define PACKET_PAYLOAD_OFFSET       6

#define AGENT_ID_LEN                128

#define PAYLOAD_AGENT_ID_OFFSET     0

AgentsServerTask::AgentsServerTask(int socketDescriptor) :
    taskSocketDescriptor(socketDescriptor)
{
    QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_INFO) << __FUNCTION__ << "Agents server task created. Socket:" << socketDescriptor;

    QSettings settings;
    maxPacketSize = settings.value("network/max_packet_size", DEFAULT_MAX_PACKET_SIZE).toULongLong();
    readTimeout = settings.value("network/read_timeout", DEFAULT_READ_TIMEOUT).toUInt();
    mediaBasePath = settings.value("storage/media_base_path", "/var/media").toString();
}

void AgentsServerTask::run() {
    QTcpSocket tcpSocket;
       
    if (!tcpSocket.setSocketDescriptor(taskSocketDescriptor)) {
        QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_ERROR) << __FUNCTION__ << "Error converting socket descriptor to socket:" << tcpSocket.error();
        return;
    }
    
    QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_INFO) << __FUNCTION__ << "Agent serving task is started. Agent ip:" << 
                                                          tcpSocket.peerAddress().toString() << ":" << tcpSocket.peerPort();

    // Read and parse commands from socket
    packetsReadLoop(&tcpSocket);
    
    // Ending connection
    socketDisconnect(&tcpSocket);
}

void AgentsServerTask::socketDisconnect(QTcpSocket *tcpSocket) {
    tcpSocket->disconnectFromHost();
    if (tcpSocket->state() != QAbstractSocket::UnconnectedState)
        tcpSocket->waitForDisconnected();
}

void AgentsServerTask::packetsReadLoop(QTcpSocket *tcpSocket) {
    uint8_t *packetBuf;
    qint64 bytesRead;
    
    packetBuf = new uint8_t[maxPacketSize];
    
    if (packetBuf == NULL) {
        QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_ERROR) << __FUNCTION__ << "Error allocating memory for packet buffer! Agent ip:" << 
                                                               socketAddressLine(tcpSocket);
        socketDisconnect(tcpSocket);
        return;
    }
    
    // Serving connection
    
    while (1) {
        size_t packetPos = 0;
        size_t packetPayloadLen = 0;
        size_t packetPayloadReadedBytes = 0;
        uint8_t packetCmd = 0;
        memset(packetBuf, 0, maxPacketSize);
        
        // Считываем заголовок
        while (packetPos < PACKET_HEADER_SIZE) {
            if (!tcpSocket->waitForReadyRead(readTimeout)) {
                QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_INFO) << __FUNCTION__ << "Timeout reading header data from socket. Agent ip:" << 
                                                                       socketAddressLine(tcpSocket);
                break;
            }
            
            bytesRead = tcpSocket->read((char *)(packetBuf + packetPos), PACKET_HEADER_SIZE - packetPos);
            
            if (bytesRead <= 0) {
                QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_WARN) << __FUNCTION__ << "Error reading header data from socket. Agent ip:" << 
                                                                       socketAddressLine(tcpSocket);
                break;
            }
            
            QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_TRACE) << __FUNCTION__ << "Header data readed:" << bytesRead << "; pos:" << packetPos <<  ". Agent ip:" << 
                                                                   socketAddressLine(tcpSocket);
            
            packetPos += bytesRead;
        }
        
        // If error occured
        if (packetPos < PACKET_HEADER_SIZE)
            break;
        
        // Header readed. Let's check it
        if (packetBuf[0] != STX) {
            QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_ERROR) << __FUNCTION__ << "STX error. Readed:" << hex << packetBuf[0] << ". Agent ip:" << 
                                                                   socketAddressLine(tcpSocket);
            break;
        }
        
        packetPayloadLen = payloadLen(packetBuf);
        
        if (packetPayloadLen > (maxPacketSize - PACKET_HEADER_SIZE)) {
            QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_ERROR) << __FUNCTION__ << "Packet payload len error (too big). Readed:" << packetPayloadLen << ". Agent ip:" << 
                                                                   socketAddressLine(tcpSocket);
            break;
        }
        
        packetCmd = packetBuf[PACKET_CMD_OFFSET];
        
        QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_TRACE) << __FUNCTION__ << "Packet received. Starting to read payload. Payload len:" << packetPayloadLen << "; CMD:" << packetCmd <<  ". Agent ip:" << 
                                                               socketAddressLine(tcpSocket);
        
        while (packetPayloadReadedBytes < packetPayloadLen) {
            if (!tcpSocket->waitForReadyRead(readTimeout)) {
                QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_INFO) << __FUNCTION__ << "Timeout reading payload data from socket. Agent ip:" << 
                                                                       socketAddressLine(tcpSocket);
                break;
            }
            
            bytesRead = tcpSocket->read((char *)(packetBuf + packetPos), PACKET_HEADER_SIZE - packetPos);
            
            if (bytesRead <= 0) {
                QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_WARN) << __FUNCTION__ << "Error reading payload data from socket. Agent ip:" << 
                                                                       socketAddressLine(tcpSocket);
                break;
            }
            
            QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_TRACE) << __FUNCTION__ << "Payload data readed:" << bytesRead << "; payload pos:" << packetPayloadReadedBytes <<  ". Agent ip:" << 
                                                                   socketAddressLine(tcpSocket);
            
            packetPayloadReadedBytes += bytesRead;
        }
        
        // If error occured
        if (packetPayloadReadedBytes < packetPayloadLen)
            break;
        
        QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_TRACE) << __FUNCTION__ << "Payload readed. Starting to execute command. Agent ip:" << 
                                                               socketAddressLine(tcpSocket);
        
        uint8_t res = executeCommand(tcpSocket, packetCmd, packetBuf + PACKET_PAYLOAD_OFFSET, packetPayloadLen);
        
        if (res != E_OK) {
            QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_WARN) << __FUNCTION__ << "Command execution error:" << res << ". Agent ip:" << 
                                                                   socketAddressLine(tcpSocket);
        }
        
        QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_TRACE) << __FUNCTION__ << "Command executed. Reading next... Agent ip:" << 
                                                               socketAddressLine(tcpSocket);
    }
    
    delete [] packetBuf;
}
                                                          

QString AgentsServerTask::socketAddressLine(QTcpSocket *tcpSocket) {
    QString addressLine;

    addressLine = tcpSocket->peerAddress().toString() + ":" + QString::number(tcpSocket->peerPort());
    
    return addressLine;
}

uint32_t AgentsServerTask::payloadLen(uint8_t *packetBuf) {
    uint32_t payloadLen = 0;
    
    payloadLen = packetBuf[PACKET_PAYLOAD_LEN_OFFSET];
    payloadLen |= ((uint32_t)packetBuf[PACKET_PAYLOAD_LEN_OFFSET]) << 8;
    payloadLen |= ((uint32_t)packetBuf[PACKET_PAYLOAD_LEN_OFFSET]) << 16;
    payloadLen |= ((uint32_t)packetBuf[PACKET_PAYLOAD_LEN_OFFSET]) << 24;
    
    return payloadLen;
}

uint8_t AgentsServerTask::executeCommand(QTcpSocket *tcpSocket, uint8_t cmd, uint8_t *payload, size_t payloadSize) {
    QString agentID;
    GetMediaCmd *cmdData = NULL;
    
    if (payloadSize < AGENT_ID_LEN)
        return E_INVALID_PACKET_SIZE;
    
    agentID = QString::fromUtf8((char *)(payload + PAYLOAD_AGENT_ID_OFFSET), AGENT_ID_LEN);
    
    switch (cmd) {
    case AGENT_GET_SCHEDULE_VERSION:
        getScheduleVersion(tcpSocket, agentID);
        break;
       
    case AGENT_GET_SCHEDULE_DATA:
        getScheduleVersion(tcpSocket, agentID);
        break;
        
    case AGENT_GET_MEDIA_DATA:
        if (payloadSize < sizeof(GetMediaCmd))
            return E_INVALID_PACKET_SIZE;
        
        cmdData = (GetMediaCmd *)payload;
        
        getMediaData(tcpSocket, cmdData);
        
        break;
    
    default:
        return E_INVALID_CMD;
    }
 
    return E_OK;
}

uint8_t AgentsServerTask::getScheduleVersion(QTcpSocket *tcpSocket, const QString &agentID) {
    
    return E_OK;
}

uint8_t AgentsServerTask::getScheduleData(QTcpSocket *tcpSocket, const QString &agentID) {
    
    return E_OK;
}

uint8_t AgentsServerTask::getMediaData(QTcpSocket *tcpSocket, GetMediaCmd *cmdData) {
    QString hash = QString::fromUtf8((char *)cmdData->hash, sizeof(cmdData->hash));
    
    
    
    
    return E_OK;
}

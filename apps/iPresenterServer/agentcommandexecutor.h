#ifndef AGENTCOMMANDEXECUTOR_H
#define AGENTCOMMANDEXECUTOR_H

#include <QTcpSocket>
#include <QString>

#include <stdint.h>

#include "db/idbproxy.h"
#include "networkprotoparser.h"

#include "types.h"

class AgentCommandExecutor
{
public:
    enum AGENT_EXECUTOR_ERRORS {
        E_OK                    =   0x00,
        E_PAYLOAD_LEN           =   0x01,
        E_UNKNOWN_CMD           =   0x02,
        E_MAKE_PACKET           =   0x03,
        E_GET_DATA              =   0x04,
        
        E_UNKNOWN_AGENT_ID      =   0x10,
        E_DB_ERROR              =   0x11,
        E_SOCKET_WRITE_ERROR    =   0x12,
        E_MEDIA_FILE_NOT_FOUND  =   0x13,
        E_UNKNOWN_MEDIA_TYPE    =   0x14,
        E_MEDIA_SIZE_MISMATCH   =   0x15,
        E_FILE_READ_ERROR       =   0x16
    };
    
    enum AGENT_COMMANDS  {
        AGENT_GET_SCHEDULE_VERSION      =   0x00,
        AGENT_GET_SCHEDULE_DATA         =   0x01,
        AGENT_GET_MEDIA_SIZE            =   0x02,
        AGENT_GET_MEDIA_DATA            =   0x03
    };
    
    AgentCommandExecutor();
    ~AgentCommandExecutor();
    
    uint8_t executeCommand(const NetworkProtoParser &protoParser, QTcpSocket *tcpSocket);
    
    uint8_t getScheduleVersion(QTcpSocket *tcpSocket, const QString &agentID);
    uint8_t getScheduleData(QTcpSocket *tcpSocket, const QString &agentID);
    uint8_t getMediaSize(QTcpSocket *tcpSocket, GetMediaSizeCmd *cmdData);
    uint8_t getMediaData(QTcpSocket *tcpSocket, GetMediaDataCmd *cmdData);
    
    
private:
    IDBProxy *dbProxy;
    
    // TODO: make IStorageProxy realization
    QString mediaBasePath;
};

#endif // AGENTCOMMANDEXECUTOR_H

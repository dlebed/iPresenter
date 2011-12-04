#ifndef AGENTSSERVERTASK_H
#define AGENTSSERVERTASK_H

#include <QRunnable>
#include <QTcpSocket>
#include <stdint.h>

#include <types.h>
#include "agentcommandexecutor.h"


class AgentsServerTask : public QRunnable
{
public:
    enum AGENT_TAST_ERRORS {
        E_OK                    =   0x00,
        E_UNKNOWN_CMD           =   0x01,
        E_INVALID_PACKET_SIZE   =   0x02,
        E_INVALID_MEDIA_TYPE    =   0x03,
        E_UNKNOWN_AGENT_ID      =   0x04,
        E_UNKNOWN_HASH          =   0x05,
        E_DB_ERROR              =   0x06,
        E_SOCKET_WRITE_ERROR    =   0x07
    };
    
    
    
    AgentsServerTask(int socketDescriptor);
    
    void run();

protected:
    void socketDisconnect(QTcpSocket *tcpSocket);
    void packetsReadLoop(QTcpSocket *tcpSocket);
    QString socketAddressLine(QTcpSocket *tcpSocket);
    
private:
    int taskSocketDescriptor;
    size_t maxPacketSize;
    uint32_t readTimeout;
    AgentCommandExecutor *commandExecutor;
    
    
};

#endif // AGENTSSERVERTASK_H

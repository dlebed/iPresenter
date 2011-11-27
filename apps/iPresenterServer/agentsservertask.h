#ifndef AGENTSSERVERTASK_H
#define AGENTSSERVERTASK_H

#include <QRunnable>
#include <QTcpSocket>
#include <stdint.h>

#include <types.h>

class AgentsServerTask : public QRunnable
{
public:
    enum AGENT_TAST_ERRORS {
        E_OK                    =   0x00,
        E_INVALID_CMD           =   0x01,
        E_INVALID_PACKET_SIZE   =   0x02,
        E_INVALID_MEDIA_TYPE    =   0x03
    };
    
    enum AGENT_COMMANDS  {
        AGENT_GET_SCHEDULE_VERSION      =   0x00,
        AGENT_GET_SCHEDULE_DATA         =   0x01,
        AGENT_GET_MEDIA_DATA            =   0x02
    };
    
    typedef struct {
        uint8_t hash[128];
        uint8_t mediaType;
        uint32_t size;
        uint64_t offset;
    } GetMediaCmd;
    
    AgentsServerTask(int socketDescriptor);
    
    void run();

protected:
    void socketDisconnect(QTcpSocket *tcpSocket);
    void packetsReadLoop(QTcpSocket *tcpSocket);
    QString socketAddressLine(QTcpSocket *tcpSocket);
    uint32_t payloadLen(uint8_t *packetBuf);
    uint8_t executeCommand(QTcpSocket *tcpSocket, uint8_t cmd, uint8_t *payload, size_t payloadSize);
    
    // Commands
    uint8_t getScheduleVersion(QTcpSocket *tcpSocket, const QString &agentID);
    uint8_t getScheduleData(QTcpSocket *tcpSocket, const QString &agentID);
    uint8_t getMediaData(QTcpSocket *tcpSocket, GetMediaCmd *cmdData);
    
private:
    int taskSocketDescriptor;
    size_t maxPacketSize;
    uint32_t readTimeout;
    
    // TODO: make IStorageProxy realization
    QString mediaBasePath;
};

#endif // AGENTSSERVERTASK_H

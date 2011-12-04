#ifndef NETWORKPROTOPARSER_H
#define NETWORKPROTOPARSER_H

#include <QByteArray>

#include <stdint.h>

#define PACKET_HEADER_SIZE          6

#define DEFAULT_MAX_PACKET_SIZE     (1024 * 256 + PACKET_HEADER_SIZE)

#define STX                         0x02
#define PACKET_PAYLOAD_LEN_OFFSET   1
#define PACKET_CMD_OFFSET           5
#define PACKET_PAYLOAD_OFFSET       6

#define PAYLOAD_AGENT_ID_OFFSET     0

typedef uint32_t packet_size_t;
typedef uint8_t packet_cmd_t;

class NetworkProtoParser
{
public:
    
    enum PROTO_PARSER_ERRORS {
        E_OK                =   0x00,
        E_LEN_INVALID       =   0x01,
        E_EMPTY_PACKET      =   0x02,
        E_PACKET_TOO_BIG    =   0x03,
        E_INVALID_STX       =   0x04,
        E_PARITALY_READ     =   0x05
    };
    
    typedef struct {
        uint8_t stx;
        packet_size_t payloadSize;
        packet_cmd_t cmd;        
    } __attribute__((packed)) NetworkPacketHeader;
    
    NetworkProtoParser();
    ~NetworkProtoParser();
    
    packet_size_t bytesToReadCount() const;
    
    uint8_t bytesReaded(uint8_t *data, uint8_t size);
    uint8_t cmd(packet_cmd_t &cmd) const;
    uint8_t payloadSize(packet_size_t &payloadSize) const;
    packet_size_t packetSize() const;
    packet_size_t maxPacketSize() const { return maxPacketSizeValue; }
    
    void clear();
    void makeHeader(packet_cmd_t cmd);
    uint8_t appendPayloadData(uint8_t *data, packet_size_t size);
    uint8_t appendPayloadData(const QByteArray &data);
    uint8_t incPayloadSize(packet_size_t size);
    
    uint8_t packetData(QByteArray &packetDataBuf) const;
    uint8_t payloadData(QByteArray &payloadDataBuf) const;
    
protected:
    uint8_t checkPacket();
    
private:
    QByteArray packetDataBuf;
    NetworkPacketHeader packetHeader;
    packet_size_t maxPacketSizeValue;
};

#endif // NETWORKPROTOPARSER_H

#include "networkprotoparser.h"

#include <QSettings>

#include <qlogger.h>

NetworkProtoParser::NetworkProtoParser() :
    maxPacketSizeValue(0)
{
    QSettings settings;
    maxPacketSizeValue = settings.value("network/max_packet_size", DEFAULT_MAX_PACKET_SIZE).toULongLong();
    
}

NetworkProtoParser::~NetworkProtoParser() {
    
}

packet_size_t NetworkProtoParser::bytesToReadCount() const {
    if (packetDataBuf.size() < sizeof(NetworkPacketHeader))
        return sizeof(NetworkPacketHeader) - packetDataBuf.size();
    
    if (packetDataBuf.size() < (sizeof(NetworkPacketHeader) + packetHeader.payloadSize))
        return (sizeof(NetworkPacketHeader) + packetHeader.payloadSize) - packetDataBuf.size();
    
    return 0;        
}

uint8_t NetworkProtoParser::bytesReaded(uint8_t *data, uint8_t size) {
    QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_TRACE) << __FUNCTION__ << "Data readed:" << size << "bytes";
    
    if (size > maxPacketSizeValue)
        return E_PACKET_TOO_BIG;
    
    if ((packetDataBuf.size() >= sizeof(NetworkPacketHeader)) && 
            ((packetHeader.payloadSize + sizeof(NetworkPacketHeader)) < (packetDataBuf.size() + size))) {
        return E_PACKET_TOO_BIG;
    }
    
    if (packetDataBuf.size() < sizeof(NetworkPacketHeader) && 
            (packetDataBuf.size() + size) < maxPacketSizeValue) {
        packetDataBuf.append((char*)data, size);
        
        if (packetDataBuf.size() >= sizeof(NetworkPacketHeader)) {
            memcpy(&packetHeader, packetDataBuf.data(), sizeof(packetHeader));
            return checkPacket();
        }
         
        return E_OK;
    }
    
    if ((packetDataBuf.size() + size) < maxPacketSizeValue) {
        packetDataBuf.append((char*)data, size);
    }
    
    return E_OK;
}

uint8_t NetworkProtoParser::checkPacket() {
    if (packetHeader.stx != STX)
        return E_INVALID_STX;
    
    return E_OK;
}

uint8_t NetworkProtoParser::cmd(packet_cmd_t &cmd) const {
    if (packetDataBuf.size() >= sizeof(NetworkPacketHeader)) {
        cmd = packetHeader.cmd;
        return E_OK;
    }
        
    return E_LEN_INVALID;
}

uint8_t NetworkProtoParser::payloadSize(packet_size_t &payloadSize) const {
    if (packetDataBuf.size() >= sizeof(NetworkPacketHeader)) {
        payloadSize = packetHeader.payloadSize;
        return E_OK;
    }
        
    return E_LEN_INVALID;
}

packet_size_t NetworkProtoParser::packetSize() const {
    return packetDataBuf.size();
}

void NetworkProtoParser::clear() {
    packetDataBuf.clear();
}

void NetworkProtoParser::makeHeader(packet_cmd_t cmd) {
    memset(&packetHeader, 0, sizeof(packetHeader));
    packetHeader.cmd = cmd;
    packetHeader.stx = STX;
    
    packetDataBuf.clear();
    packetDataBuf.append((char*)&packetHeader, sizeof(packetHeader));
}

uint8_t NetworkProtoParser::appendPayloadData(uint8_t *data, packet_size_t size) {
    if ((size + packetDataBuf.size()) > maxPacketSizeValue)
        return E_PACKET_TOO_BIG;
    
    packetHeader.payloadSize += size;
    packetDataBuf.replace(0, sizeof(packetHeader), (char*)&packetHeader, sizeof(packetHeader));
    packetDataBuf.append((char*)data, size);
    
    return E_OK;
}

uint8_t NetworkProtoParser::appendPayloadData(const QByteArray &data) {
    if ((data.size() + packetDataBuf.size()) > maxPacketSizeValue)
        return E_PACKET_TOO_BIG;     
    
    packetHeader.payloadSize += data.size();
    packetDataBuf.replace(0, sizeof(packetHeader), (char*)&packetHeader, sizeof(packetHeader));
    packetDataBuf.append(data);
    
    return E_OK;
}

uint8_t NetworkProtoParser::incPayloadSize(packet_size_t size) {
    if ((size + packetDataBuf.size()) > maxPacketSizeValue)
        return E_PACKET_TOO_BIG;   
    
    packetHeader.payloadSize += size;
    packetDataBuf.replace(0, sizeof(packetHeader), (char*)&packetHeader, sizeof(packetHeader));
    
    return E_OK;
}

uint8_t NetworkProtoParser::packetData(QByteArray &packetDataBuf) const {
    if (this->packetDataBuf.size() == 0)
        return E_EMPTY_PACKET;
    
    packetDataBuf = this->packetDataBuf;
    
    return E_OK;
}

uint8_t NetworkProtoParser::payloadData(QByteArray &payloadDataBuf) const {
    if (this->packetDataBuf.size() < sizeof(NetworkPacketHeader))
        return E_LEN_INVALID;
    
    if ((this->packetDataBuf.size() - sizeof(NetworkPacketHeader)) < packetHeader.payloadSize)
        return E_PARITALY_READ;
    
    payloadDataBuf = this->packetDataBuf.right(packetHeader.payloadSize);
    
    return E_OK;
}

#ifndef ISTORAGEPROXY_H
#define ISTORAGEPROXY_H

#include <QString>
#include <stdint.h>

class IStorageProxy {
public:
    
    enum STORAGE_PROXY_ERRORS   {
        E_OK                =   0x00,
        E_UNKNOWN_HASH      =   0x01,
        E_READ_ERROR        =   0x02
    };
    
    virtual ~IStorageProxy() {};
    
    virtual uint8_t getDataSize(const QString &hash, size_t &dataSize) = 0;
    virtual uint8_t readData(const QString &hash, size_t offset, size_t len, uint8_t *buf, size_t &readed) = 0;
    virtual uint8_t writeData(const QString &hash, size_t len, uint8_t buf) = 0;
};

#endif // ISTORAGEPROXY_H

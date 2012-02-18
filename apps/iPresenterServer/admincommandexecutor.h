#ifndef ADMINCOMMANDEXECUTOR_H
#define ADMINCOMMANDEXECUTOR_H

#include <QTcpSocket>
#include <QString>

#include <stdint.h>

#include <hashcalculatorfactory.h>

#include "db/idbproxy.h"
#include "networkprotoparser.h"

#include "types.h"

#define HASH_LEN  64

class AdminCommandExecutor
{
public:
    enum ADMIN_EXECUTOR_ERRORS {
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
        E_FILE_READ_ERROR       =   0x16,
        E_XML_PARSE_ERROR       =   0x17,
        E_PARAMETERS_INVALID    =   0x18
    };
    
    enum ADMIN_RESULT_CODES {
        ADMIN_OK                =   0x00,
        ADMIN_FILE_EXIST        =   0x01,
        ADMIN_NACK              =   0xFF
    };

    typedef struct {
        uint8_t hash[HASH_LEN];
        uint8_t mediaType;
        media_size_t size;
        media_size_t offset;
    } __attribute__((packed)) MediaUploadCmd;
    
    typedef struct {
        uint8_t hash[HASH_LEN];
        uint8_t mediaType;
    } __attribute__((packed)) MediaHashCmd;
    
    AdminCommandExecutor();
    ~AdminCommandExecutor();
    
    uint8_t executeCommand(const NetworkProtoParser &protoParser, QTcpSocket *tcpSocket);
    
    uint8_t initDataUpload(QTcpSocket *tcpSocket, MediaHashCmd *mediaHashCmd);
    uint8_t appendMediaFileData(QTcpSocket *tcpSocket, MediaUploadCmd *mediaUploadCmd, uint8_t *data, media_size_t dataSize);

    uint8_t mediaDataVerify(QTcpSocket *tcpSocket, MediaUploadCmd *mediaUploadCmd);

    uint8_t getMediaData(QTcpSocket *tcpSocket, MediaUploadCmd *cmdData);
    
    
private:
    IDBProxy *dbProxy;
    
    IHashCalculator *hashCalculator;

    // TODO: make IStorageProxy realization
    QString mediaBasePath;
};

#endif // ADMINCOMMANDEXECUTOR_H

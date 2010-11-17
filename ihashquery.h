#ifndef IHASHQUERY_H
#define IHASHQUERY_H

#include <QString>

class IHashQuery {
public:
    typedef enum {
                FILE_TYPE_IMAGE     =   0x01,
                FILE_TYPE_MOVIE     =   0x02,
                FILE_TYPE_BLOCK     =   0x03
    } FILE_TYPE;
    
    IHashQuery(const QString &hashName = "md5") {};
    
    virtual ~IHashQuery() {}; 
    
    virtual void addFilePathWithHash(const QString &filePath, FILE_TYPE fileType) = 0;
    virtual QString lookupFilePathByHash(const QString &fileHash, FILE_TYPE fileType) = 0;
    
};

#endif // IHASHQUERY_H

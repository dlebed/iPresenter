#ifndef IHASHQUERY_H
#define IHASHQUERY_H

#include <QString>

#include <typedefs.h>

class IHashQuery {
public:

    IHashQuery(const QString &hashName = "md5") {};
    
    virtual ~IHashQuery() {}; 
    
    virtual void addFilePathWithHash(const QString &filePath, FILE_TYPE fileType) = 0;
    virtual QString lookupFilePathByHash(const QString &fileHash, FILE_TYPE fileType) = 0;
    
};

#endif // IHASHQUERY_H

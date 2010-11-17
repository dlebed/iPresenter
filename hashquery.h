#ifndef HASHQUERY_H
#define HASHQUERY_H

#include <QSqlQuery>
#include <QSqlError>
#include <QSqlDatabase>
#include <QSettings>

#include "ihashquery.h"
#include "hash/ihashcalculator.h"

class HashQuery : public IHashQuery {
public:
    
    HashQuery(const QString &hashName = "md5");
    virtual ~HashQuery();
    
    void addFilePathWithHash(const QString &filePath, FILE_TYPE fileType);
    QString lookupFilePathByHash(const QString &fileHash, FILE_TYPE fileType);
    
protected:
    QString fileTypeStr(FILE_TYPE type);
    
private:
    QSettings settings;
    QSqlDatabase db;
    
    IHashCalculator * hashCalculator;
    
};

#endif // HASHQUERY_H

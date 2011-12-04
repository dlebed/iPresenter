#ifndef HASHQUERY_H
#define HASHQUERY_H

#include <QSqlQuery>
#include <QSqlError>
#include <QSqlDatabase>
#include <QSettings>

#include "ihashquery.h"
#include "hash/ihashcalculator.h"

#define HASH_QUERY_DB_NAME      "HashQuery"

class HashQuery : public IHashQuery {
public:
    
    HashQuery(const QString &hashName = "sha256");
    virtual ~HashQuery();
    
    void addFilePathWithHash(const QString &filePath, FILE_TYPE fileType);
    void addFile(const QString &filePath, const QString &fileHash, FILE_TYPE fileType);
    QString lookupFilePathByHash(const QString &fileHash, FILE_TYPE fileType);
    
protected:
    QString fileTypeStr(FILE_TYPE type);
    
private:
    QSettings settings;
    QSqlDatabase db;
    
    IHashCalculator * hashCalculator;
    
};

#endif // HASHQUERY_H

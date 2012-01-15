#ifndef HASHQUERY_H
#define HASHQUERY_H

#include <QSqlQuery>
#include <QSqlError>
#include <QSqlDatabase>
#include <QSettings>
#include <QMutex>

#include "ihashquery.h"
#include <ihashcalculator.h>

#define HASH_QUERY_DB_NAME      QString("HashQuery")

class SQLDBHashQuery : public IHashQuery {
public:
    
    SQLDBHashQuery(const QString &hashName = "sha256");
    virtual ~SQLDBHashQuery();
    
    void addFilePathWithHash(const QString &filePath, FILE_TYPE fileType);
    void addFile(const QString &filePath, const QString &fileHash, FILE_TYPE fileType);
    QString lookupFilePathByHash(const QString &fileHash, FILE_TYPE fileType);
    
protected:
    QString fileTypeStr(FILE_TYPE type);
    
    QString currentThreadID();

private:
    QSettings settings;
    QSqlDatabase db;
    QMutex hashQueryMutex;

    IHashCalculator * hashCalculator;
    
};

#endif // HASHQUERY_H

#ifndef HASHQUERY_H
#define HASHQUERY_H

#include <QSqlQuery>
#include <QSqlError>
#include <QSqlDatabase>
#include <QSettings>

#define HASH_BUFFER_SIZE    (4096 * 1024)

class HashQuery
{
public:
    typedef enum {
                FILE_TYPE_IMAGE     =   0x01,
                FILE_TYPE_MOVIE     =   0x02
    } FILE_TYPE;
    
    HashQuery();
    ~HashQuery();
    
    void addFilePathWithHash(const QString &filePath, FILE_TYPE fileType);
    QString lookupFilePathByHash(const QString &fileHash, FILE_TYPE fileType);
    
protected:
    QString getFileHash(const QString &filePath);
    
private:
    QSettings settings;
    QSqlDatabase db;
    
};

#endif // HASHQUERY_H

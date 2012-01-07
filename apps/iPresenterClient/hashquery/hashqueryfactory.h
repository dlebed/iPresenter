#ifndef HASHQUERYFACTORY_H
#define HASHQUERYFACTORY_H

#include <QObject>
#include <hashquery/ihashquery.h>

class HashQueryFactory : public QObject
{
    Q_OBJECT
public:
    enum HASH_QUERY_TYPE {
        HASH_QUERY_SQL_DB       =   0x00
    };

    HashQueryFactory(QObject *parent = 0);
    
    static IHashQuery * hashQuery();

signals:
    
public slots:
    
};

#endif // HASHQUERYFACTORY_H

#include "hashqueryfactory.h"

#include <hashquery/sqldbhashquery.h>

HashQueryFactory::HashQueryFactory(QObject *parent) :
    QObject(parent)
{
}

IHashQuery * HashQueryFactory::hashQuery() {
    return new SQLDBHashQuery();
}

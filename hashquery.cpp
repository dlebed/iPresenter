#include "hashquery.h"
#include <QApplication>
#include <QCryptographicHash>
#include <QByteArray>
#include <QFile>

#include <qlogger.h>

#include "hash/hashcalculatorfactory.h"

HashQuery::HashQuery(const QString &hashName) :
    hashCalculator(NULL)
{
    QString dbPath = settings.value("hash/db_path", "/etc/ads/hash.sqlite").toString();
    
    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(dbPath);

    if (!db.open()) {
        QLogger(QLogger::INFO_DATABASE, QLogger::LEVEL_ERROR) << __FUNCTION__ << "Cannot open hash DB";
        QApplication::exit(1);
    } else {
        QLogger(QLogger::INFO_DATABASE, QLogger::LEVEL_INFO) << __FUNCTION__ << "Database '" << dbPath << "' successfully opened";
    }
    
    hashCalculator = HashCalculatorFactory::hashCalculatorInstance(hashName);
    Q_ASSERT(hashCalculator != NULL);
    
    if (hashCalculator == NULL) {
        QLogger(QLogger::INFO_DATABASE, QLogger::LEVEL_ERROR) << __FUNCTION__ << "Cannot create IHashCalculator instance";
        QApplication::exit(1);
    }
}

HashQuery::~HashQuery() {
    if (hashCalculator != NULL) {
        delete hashCalculator;
        hashCalculator = NULL;
    }
}


void HashQuery::addFilePathWithHash(const QString &filePath, FILE_TYPE fileType) {
    if (!QFile::exists(filePath)) {
        QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_ERROR)  << "Attempt to add hash for non-existing file:" <<
                                                                filePath;
        return;
    }
    
    QSqlQuery query(db);
    bool ok = false;
    Q_ASSERT(hashCalculator != NULL);
    QString fileHash = hashCalculator->getFileHash(filePath);
    
    if (!fileHash.isEmpty()) {        
        ok = query.exec("INSERT INTO " + fileTypeStr(fileType) + " (hash, filepath) VALUES ('" + fileHash + "', '" + filePath + "');");
        
        if (!ok) {
            QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_ERROR)  << "Unable to add file hash into DB" << filePath << "; type:" <<
                                                                    fileTypeStr(fileType) <<
                                                                    "; hash:" << fileHash << ":" << db.lastError().text();
        }
        
    } else {
        QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_ERROR)  << "Unable to add file hash into DB" << filePath << "; type:" <<
                                                                fileTypeStr(fileType) <<
                                                                "; file hash is empty";
    }
}

QString HashQuery::lookupFilePathByHash(const QString &fileHash, FILE_TYPE fileType) {
    QString filePath;
    
    QSqlQuery query;
    
    query = db.exec("SELECT filepath FROM " + fileTypeStr(fileType) + " WHERE hash = '" + fileHash.toLower() + "'");
    
    if (query.next()) {
        filePath = query.value(0).toString();
        QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_INFO) << "Path for file:" << filePath << "; type:" <<
                                                              fileTypeStr(fileType) << "; hash:" << fileHash;
    } else {
        QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_ERROR) << "Unable to get file path for hash. File type:" << 
                                                               fileTypeStr(fileType) << ". File hash:" << fileHash;
    }
    
    return filePath;
}

QString HashQuery::fileTypeStr(FILE_TYPE type) {
    switch (type) {
    case FILE_TYPE_IMAGE:
        return "images";
    case FILE_TYPE_MOVIE:
        return "movies";
    case FILE_TYPE_BLOCK:
        return "blocks";
    }
    
    return QString();
}

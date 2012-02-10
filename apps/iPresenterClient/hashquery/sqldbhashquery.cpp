#include "sqldbhashquery.h"
#include <QApplication>
#include <QCryptographicHash>
#include <QByteArray>
#include <QFile>
#include <QMutexLocker>
#include <QThread>

#include <qlogger.h>

#include <hashcalculatorfactory.h>

SQLDBHashQuery::SQLDBHashQuery(const QString &hashName) :
    hashCalculator(NULL)
{
    QString dbPath = settings.value("hash/db_path", "/etc/ads/hash.sqlite").toString();
    
    if (!QSqlDatabase::contains(HASH_QUERY_DB_NAME + currentThreadID())) {
        db = QSqlDatabase::addDatabase("QSQLITE", HASH_QUERY_DB_NAME + currentThreadID());
        db.setDatabaseName(dbPath);
    } else {
        db = QSqlDatabase::database(HASH_QUERY_DB_NAME + currentThreadID());
    }

    QLogger(QLogger::INFO_DATABASE, QLogger::LEVEL_ERROR) << __FUNCTION__ << "Hash DB created connection" << HASH_QUERY_DB_NAME + currentThreadID();

    if (!QFile::exists(dbPath)) {
        QLogger(QLogger::INFO_DATABASE, QLogger::LEVEL_ERROR) << __FUNCTION__ << "Hash DB file does not exists:" << dbPath;
        QApplication::exit(1);
    }
    
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

SQLDBHashQuery::~SQLDBHashQuery() {
    if (hashCalculator != NULL) {
        delete hashCalculator;
        hashCalculator = NULL;
    }
    
    db.close();
}


void SQLDBHashQuery::addFilePathWithHash(const QString &filePath, FILE_TYPE fileType) {
    if (!QFile::exists(filePath)) {
        QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_ERROR)  << "Attempt to add hash for non-existing file:" <<
                                                                filePath;
        return;
    }
    
    QSqlQuery query(db);
    bool ok = false;
    Q_ASSERT(hashCalculator != NULL);
    QString fileHash = hashCalculator->getFileHash(filePath);
    
    QMutexLocker ml(&hashQueryMutex);

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

void SQLDBHashQuery::addFile(const QString &filePath, const QString &fileHash, FILE_TYPE fileType) {
    QMutexLocker ml(&hashQueryMutex);

    if (!QFile::exists(filePath)) {
        QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_ERROR)  << "Attempt to add hash for non-existing file:" <<
                                                                filePath;
        return;
    }
    
    QSqlQuery query(db);
    bool ok = false;
    
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

QString SQLDBHashQuery::lookupFilePathByHash(const QString &fileHash, FILE_TYPE fileType) {
    QMutexLocker ml(&hashQueryMutex);
    QString filePath;
    
    QSqlQuery query;
    
    query = db.exec("SELECT filepath FROM " + fileTypeStr(fileType) + " WHERE hash = '" + fileHash.toLower() + "'");
    
    if (query.next()) {
        filePath = query.value(0).toString();
        QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_INFO) << "Path for file:" << filePath << "; type:" <<
                                                              fileTypeStr(fileType) << "; hash:" << fileHash;

        // Update last access datetime
        db.exec("UPDATE " + fileTypeStr(fileType) + " SET last_access = datetime('now') where hash = '" + fileHash.toLower() + "'");
    } else {
        QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_INFO) << "Unable to get file path for hash. File type:" << 
                                                               fileTypeStr(fileType) << ". File hash:" << fileHash;
    }
    
    return filePath;
}

QString SQLDBHashQuery::fileTypeStr(FILE_TYPE type) {
    switch (type) {
    case FILE_TYPE_IMAGE:
        return "images";
    case FILE_TYPE_MOVIE:
        return "movies";
    }
    
    return QString();
}

QString SQLDBHashQuery::currentThreadID() {
	return QString::number((qptrdiff)QThread::currentThread(), 16).toUpper();
}

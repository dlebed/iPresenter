#include "hashquery.h"
#include <QApplication>
#include <QCryptographicHash>
#include <QByteArray>
#include <QFile>

#include <qlogger.h>

HashQuery::HashQuery()
{
    QString dbPath = settings.value("hash/db_path", "/etc/ads/hash.sqlite").toString();
    
    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(dbPath);

    if (!db.open()) {
        QLogger(QLogger::INFO_DATABASE, QLogger::LEVEL_ERROR) << __FUNCTION__ << "Cannot open hash DB";
        QApplication::exit(1);
    }
}

HashQuery::~HashQuery() {
    
}


void HashQuery::addFilePathWithHash(const QString &filePath, FILE_TYPE fileType) {
    if (!QFile::exists(filePath)) {
        QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_ERROR)  << "Attempt to add hash for non-existing file:" <<
                                                                filePath;
        return;
    }
    
    QSqlQuery query(db);
    bool ok = false;
    QString fileHash = getFileHash(filePath);
    
    if (!fileHash.isEmpty()) {        
        switch (fileType) {
        case FILE_TYPE_IMAGE:
            ok = query.exec("INSERT INTO images (hash, filepath) VALUES ('" + fileHash + "', '" + filePath + "');");
            break;
        case FILE_TYPE_MOVIE:
            ok = query.exec("INSERT INTO movies (hash, filepath) VALUES ('" + fileHash + "', '" + filePath + "');");
            break;
        }
    }
    
    if (!ok) {
        QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_ERROR)  << "Unable to add file hash into DB" << filePath <<
                                                                "hash:" << fileHash << ":" << db.lastError().text();
    }
}

QString HashQuery::lookupFilePathByHash(const QString &fileHash, FILE_TYPE fileType) {
    QString filePath;
    
    QSqlQuery query;
    
    switch (fileType) {
    case FILE_TYPE_IMAGE:
        query = db.exec("SELECT filepath FROM images WHERE hash = '" + fileHash.toLower() + "'");
        break;
    case FILE_TYPE_MOVIE:
        query = db.exec("SELECT filepath FROM movies WHERE hash = '" + fileHash.toLower() + "'");
        break;
    }

    if (query.next()) {
        filePath = query.value(0).toString();
        QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_INFO) << "Path for file:" << filePath << "; type:" <<
                                                              fileType << "; hash:" << fileHash;
    } else {
        QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_ERROR) << "Unable to get file path for hash. File type:" << 
                                                               fileType << ". File path:" << fileHash;
    }
    
    return filePath;
}

QString HashQuery::getFileHash(const QString &filePath) {
    QCryptographicHash hash(QCryptographicHash::Md5);
    QFile hashFile(filePath);
    QByteArray buffer;
    
    if (!hashFile.open(QIODevice::ReadOnly)) {
        QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_ERROR) << "Unable to open file for hash calculating:" << filePath;
        return QString();
    }
    
    while (!hashFile.atEnd()) {
        buffer = hashFile.readLine(HASH_BUFFER_SIZE);
        hash.addData(buffer);
    }
    
    return hash.result().toHex().toLower();
}

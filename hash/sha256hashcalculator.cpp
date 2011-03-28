#include "sha256hashcalculator.h"

#include <QCryptographicHash>

#include <openssl/sha.h>

#include "ihashcalculator.h"

#include <qlogger.h>


SHA256HashCalculator::SHA256HashCalculator() :
    bufferSize(HASH_BUFFER_SIZE)
{
}

SHA256HashCalculator::SHA256HashCalculator(quint32 fileBufferSize) :
    bufferSize(fileBufferSize)
{
    
}


QString SHA256HashCalculator::getBufferHash(const QByteArray &buf) {
    quint8 hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, buf.data(), buf.size());
    SHA256_Final(hash, &sha256);
    
    return QByteArray((char *)hash, sizeof(hash)).toHex().toLower();
}

QString SHA256HashCalculator::getFileHash(const QString &filePath) {
    QFile hashFile(filePath);
    
    if (!hashFile.open(QIODevice::ReadOnly)) {
        QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_ERROR) << "Unable to open file for hash calculating:" << filePath;
        return QString();
    }
    
    quint8 *buffer = new quint8[bufferSize];
    quint8 hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    
    while (!hashFile.atEnd()) {
        quint64 bytesRead = hashFile.read((char *)buffer, bufferSize);
        SHA256_Update(&sha256, buffer, bytesRead);
    }
    
    SHA256_Final(hash, &sha256);
    delete [] buffer;
    
    return QByteArray((char *)hash, sizeof(hash)).toHex().toLower();
}

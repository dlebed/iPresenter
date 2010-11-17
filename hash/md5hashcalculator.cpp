#include "md5hashcalculator.h"

#include <QCryptographicHash>

#include "ihashcalculator.h"

#include <qlogger.h>

MD5HashCalculator::MD5HashCalculator() :
    bufferSize(HASH_BUFFER_SIZE)
{
    
}

MD5HashCalculator::MD5HashCalculator(quint32 fileBufferSize) :
    bufferSize(fileBufferSize)
{
    
}


QString MD5HashCalculator::getBufferHash(const QByteArray &buf) {
    QCryptographicHash hash(QCryptographicHash::Md5);
    
    hash.addData(buf);
    
    return hash.result().toHex().toLower();
}

QString MD5HashCalculator::getFileHash(const QString &filePath) {
    QCryptographicHash hash(QCryptographicHash::Md5);
    QFile hashFile(filePath);
    QByteArray buffer;
    
    if (!hashFile.open(QIODevice::ReadOnly)) {
        QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_ERROR) << "Unable to open file for hash calculating:" << filePath;
        return QString();
    }
    
    while (!hashFile.atEnd()) {
        buffer = hashFile.readLine(bufferSize);
        hash.addData(buffer);
    }
    
    return hash.result().toHex().toLower();
}

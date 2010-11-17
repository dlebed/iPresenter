#ifndef MD5HASHCALCULATOR_H
#define MD5HASHCALCULATOR_H

#include "ihashcalculator.h"

#define HASH_BUFFER_SIZE    (4096 * 1024)

class MD5HashCalculator : public IHashCalculator {
public:
    MD5HashCalculator();
    MD5HashCalculator(quint32 fileBufferSize);
    virtual ~MD5HashCalculator() {};
    
    void setFileBufferSize(quint32 size) { bufferSize = size; }
    quint32 getFileBufferSize() { return bufferSize; }
    
    QString getBufferHash(const QByteArray &buf);
    QString getFileHash(const QString &filePath);
    
private:
    quint32 bufferSize;
    
};

#endif // MD5HASHCALCULATOR_H

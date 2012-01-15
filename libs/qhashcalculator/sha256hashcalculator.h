#ifndef SHA256HASHCALCULATOR_H
#define SHA256HASHCALCULATOR_H

#include "ihashcalculator.h"

#define HASH_BUFFER_SIZE    (4096 * 1024)

class SHA256HashCalculator : public IHashCalculator
{
public:
    SHA256HashCalculator();
    SHA256HashCalculator(quint32 fileBufferSize);
    virtual ~SHA256HashCalculator() {};
    
    void setFileBufferSize(quint32 size) { bufferSize = size; }
    quint32 getFileBufferSize() { return bufferSize; }
    
    QString getBufferHash(const QByteArray &buf);
    QString getFileHash(const QString &filePath);
    
private:
    quint32 bufferSize;
};

#endif // SHA256HASHCALCULATOR_H

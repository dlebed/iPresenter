#ifndef IHASHCALCULATOR_H
#define IHASHCALCULATOR_H

#include <QString>
#include <QByteArray>

class IHashCalculator {
public:
    
    IHashCalculator() {};
    IHashCalculator(quint32 fileBufferSize) {};
    virtual ~IHashCalculator() {}; 

    virtual void setFileBufferSize(quint32 size) = 0;
    virtual quint32 getFileBufferSize() = 0;
    
    virtual QString getBufferHash(const QByteArray &buf) = 0;
    virtual QString getFileHash(const QString &filePath) = 0;
    
};

#endif // IHASHCALCULATOR_H

#ifndef BLOCKLOADER_H
#define BLOCKLOADER_H

#include <QThread>
#include <QDomDocument>
#include <QSettings>

#include "hash/ihashcalculator.h"

class BlockLoader : public QThread
{
    Q_OBJECT
public:
    BlockLoader(const QString &hashType = "sha256", QObject *parent = 0);
    ~BlockLoader();

    void run();
    
public slots:
    
    void stop();
    void interruptLoading();
    void loadBlock(const QDomDocument &blockDocument);

signals:
    
    void loadingInterrupted();
    void blockLoaded(const QDomDocument &blockDocument);
    
    
private:
    bool isLoading;
    QSettings settings;
    
    IHashCalculator * hashCalculator;
};

#endif // BLOCKLOADER_H

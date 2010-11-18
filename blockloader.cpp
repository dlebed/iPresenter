#include "blockloader.h"

#include <QMetaType>
#include <QFtp>

#include <qlogger.h>

#include "hash/hashcalculatorfactory.h"

BlockLoader::BlockLoader(const QString &hashType, QObject *parent) :
    QThread(parent),
    isLoading(false), hashCalculator(NULL)
{
	if (QMetaType::type("QDomDocument") == 0)
        qRegisterMetaType<QDomDocument>("QDomDocument");
    
    moveToThread(this);
    
    hashCalculator = HashCalculatorFactory::hashCalculatorInstance(hashType);
    Q_ASSERT(hashCalculator);
}

BlockLoader::~BlockLoader() {
    
}


void BlockLoader::run() {
    QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_INFO) << __FUNCTION__ << "BlockLoader thread starting:" << hex << currentThreadId();
    
    exec();
}


void BlockLoader::stop() {
    exit();
    wait();
}

void BlockLoader::interruptLoading() {
    
    
    
}

void BlockLoader::loadBlock(const QDomDocument &blockDocument) {
    
    
    
}

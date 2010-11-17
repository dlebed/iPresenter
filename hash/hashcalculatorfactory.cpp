#include "hashcalculatorfactory.h"

#include "md5hashcalculator.h"

#include <qlogger.h>

HashCalculatorFactory::HashCalculatorFactory()
{
}


IHashCalculator * HashCalculatorFactory::hashCalculatorInstance(const QString &hashName) {
    
    if (hashName.compare("md5", Qt::CaseInsensitive) == 0) {
        return new MD5HashCalculator();
    } else {
        QLogger(QLogger::INFO_DATABASE, QLogger::LEVEL_ERROR) << __FUNCTION__ << "Invalid hash name:" << hashName;
        return NULL;
    }
    
}

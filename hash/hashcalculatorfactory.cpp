#include "hashcalculatorfactory.h"

#include "md5hashcalculator.h"
#include "sha256hashcalculator.h"

#include <qlogger.h>

HashCalculatorFactory::HashCalculatorFactory()
{
}


IHashCalculator * HashCalculatorFactory::hashCalculatorInstance(const QString &hashName) {
    IHashCalculator *hashCalculator;
    
    if (hashName.compare("md5", Qt::CaseInsensitive) == 0) {
        hashCalculator = new MD5HashCalculator();
        Q_ASSERT(hashCalculator != NULL);
        return hashCalculator;
    } else if (hashName.compare("sha256", Qt::CaseInsensitive) == 0) {
        hashCalculator = new SHA256HashCalculator();
        Q_ASSERT(hashCalculator != NULL);
        return hashCalculator;
    } else {
        QLogger(QLogger::INFO_DATABASE, QLogger::LEVEL_ERROR) << __FUNCTION__ << "Invalid hash name:" << hashName;
        return NULL;
    }
    
}

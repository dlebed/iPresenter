#ifndef HASHCALCULATORFACTORY_H
#define HASHCALCULATORFACTORY_H

#include <QString>
#include "ihashcalculator.h"

class HashCalculatorFactory
{
public:
    HashCalculatorFactory();
    
    static IHashCalculator * hashCalculatorInstance(const QString &hashName);
    
};

#endif // HASHCALCULATORFACTORY_H

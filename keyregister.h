#ifndef KEYREGISTER_H
#define KEYREGISTER_H

#include "baseregister.h"


class KeyRegister : public BaseRegister
{
public:
    KeyRegister(const PosInt& keySize);
    KeyRegister(const PosInt& keySize, const QString& initialKey);
    KeyRegister(const PosInt& keySize, const BitArray& initialKey);
    void rotateLeft();
    KeyRegister *clone() const;
};

#endif // KEYREGISTER_H

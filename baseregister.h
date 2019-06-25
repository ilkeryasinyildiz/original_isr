#ifndef BASEREGISTER_H
#define BASEREGISTER_H

#include "constants.h"

class BaseRegister
{
public:
    BaseRegister(const PosInt& stateSize);
    void loadRegisterValue( const BitArray& state);
    QVector<BitArray> collectInvolvedStates( const BitIndexArray& involvedStateBits );
    bool fillWithString( const QString& strVal);
    void printRegister();
    const BitArray &registerValue() const;
protected:
    PosInt m_registerSize;
    BitArray m_registerValue;
private:
    bool increment();
    void clear();
};

#endif // BASEREGISTER_H

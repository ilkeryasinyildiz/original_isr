#include "keyregister.h"

KeyRegister::KeyRegister(const PosInt &keySize)
    :BaseRegister (keySize)
{
    //Nothing yet
}

KeyRegister::KeyRegister(const PosInt &keySize, const QString &initialKey)
    :BaseRegister (keySize)
{
    fillWithString(initialKey);
}

KeyRegister::KeyRegister(const PosInt &keySize, const BitArray &initialKey)
    :BaseRegister (keySize)
{
    loadRegisterValue(initialKey);
}

void KeyRegister::rotateLeft()
{
    Bit lsb = m_registerValue[0];

    for( BitIndex bitIndex = 1; bitIndex < m_registerSize; bitIndex++)
    {
        m_registerValue[bitIndex-1] = m_registerValue[bitIndex];
    }

    m_registerValue[m_registerSize-1] = lsb;
}

KeyRegister *KeyRegister::clone() const
{
    KeyRegister *keyRegister = new KeyRegister(m_registerSize, m_registerValue);
    return keyRegister;
}

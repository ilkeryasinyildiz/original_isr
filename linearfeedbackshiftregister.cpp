#include "linearfeedbackshiftregister.h"

LinearFeedbackShiftRegister::LinearFeedbackShiftRegister(const PosInt& stateSize) : BaseRegister(stateSize)
{
    //Nothing yet
}

LinearFeedbackShiftRegister::LinearFeedbackShiftRegister(const PosInt &stateSize, const QString &initialState)
    : BaseRegister (stateSize)
{
    fillWithString(initialState);
}

LinearFeedbackShiftRegister::LinearFeedbackShiftRegister(const PosInt &stateSize, const BitArray &initialState)
    : BaseRegister(stateSize)
{
    loadRegisterValue(initialState);
}

Bit LinearFeedbackShiftRegister::shiftLeft(const Bit &newBit)
{
    Bit lsb = m_registerValue[0];

    for( BitIndex bitIndex = 1; bitIndex < m_registerSize; bitIndex++)
    {
        m_registerValue[bitIndex-1] = m_registerValue[bitIndex];
    }

    m_registerValue[m_registerSize-1] = newBit;

    return lsb;
}

LinearFeedbackShiftRegister* LinearFeedbackShiftRegister::clone() const
{
    LinearFeedbackShiftRegister *lfsr = new LinearFeedbackShiftRegister(m_registerSize, m_registerValue);
    return lfsr;
}

PosInt LinearFeedbackShiftRegister::getRegisterSize() const
{
    return m_registerSize;
}

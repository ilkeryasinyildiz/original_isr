#include "statewithmismatchcounter.h"



StateWithMismatchCounter::StateWithMismatchCounter() : BaseRegister (24)
{

}

StateWithMismatchCounter::StateWithMismatchCounter(const PosInt &stateSize)
    :BaseRegister (stateSize)
{

}

StateWithMismatchCounter::StateWithMismatchCounter(const PosInt &stateSize, const QString &initialState, const PosInt& rootStateIndex)
    :BaseRegister (stateSize)
{
    fillWithString(initialState);
    m_RootState = m_registerValue;
    m_rootStateIndex = rootStateIndex;
}

StateWithMismatchCounter::StateWithMismatchCounter(const PosInt &stateSize, const BitArray &initialState, const PosInt& rootStateIndex)
    :BaseRegister (stateSize)
{
    loadRegisterValue(initialState);
    m_RootState = initialState;
    m_rootStateIndex = rootStateIndex;
}

void StateWithMismatchCounter::setInitialState(const BitArray &initialState)
{
    loadRegisterValue(initialState);
    m_RootState = initialState;
}

void StateWithMismatchCounter::addToMismatch(const Real &value)
{
    m_mismatchCount += value;
}

void StateWithMismatchCounter::setMismatchValue(const Real &value)
{
    m_mismatchCount = value;
}

Real StateWithMismatchCounter::mismatchCount() const
{
    return m_mismatchCount;
}

bool StateWithMismatchCounter::suggestedValueExists() const
{
    return m_suggestedValueExists;
}

Bit StateWithMismatchCounter::getSuggestedValue() const
{
    return m_suggestedValue;
}

void StateWithMismatchCounter::setSuggestedValue(const Bit &bit)
{
    m_suggestedValue = bit;
    m_suggestedValueExists = true;
}

void StateWithMismatchCounter::clearSuggestedValue()
{
    m_suggestedValue = 0;
    m_suggestedValueExists = false;
}

StateWithMismatchCounter* StateWithMismatchCounter::clone() const
{
    StateWithMismatchCounter *state = new StateWithMismatchCounter(m_registerSize, m_registerValue, m_rootStateIndex);
    state->setMismatchValue(m_mismatchCount);
    state->setRootState(m_RootState);
    state->setRootStateIndex(m_rootStateIndex);
    #if defined(CLOCK_DEBUG)
        state->setClockCount(m_clockCount);//NEW!
    #endif
//    if (state->suggestedValueExists())
//    {
//        state->setSuggestedValue(m_suggestedValue);
//    }
    return state;
}

BitArray StateWithMismatchCounter::getRootState() const
{
    return m_RootState;
}

void StateWithMismatchCounter::setRootState(const BitArray &RootState)
{
    m_RootState = RootState;
}

PosInt StateWithMismatchCounter::getRootStateIndex() const
{
    return m_rootStateIndex;
}

void StateWithMismatchCounter::setRootStateIndex(const PosInt &rootStateIndex)
{
    m_rootStateIndex = rootStateIndex;
}

void StateWithMismatchCounter::resetToOrigin()
{
    m_registerValue = m_RootState;
    m_mismatchCount = 0;
    clearSuggestedValue();
}

PosInt StateWithMismatchCounter::getRegisterSize() const
{
    return m_registerSize;
}

#if defined(CLOCK_DEBUG)
PosInt StateWithMismatchCounter::getClockCount() const
{
    return m_clockCount;
}

void StateWithMismatchCounter::incrementClockCount()
{
    m_clockCount++;
}

void StateWithMismatchCounter::setClockCount(const PosInt &clockCount)
{
    m_clockCount = clockCount;
}
#endif

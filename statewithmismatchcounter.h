#ifndef STATEWITHMISMATCHCOUNTER_H
#define STATEWITHMISMATCHCOUNTER_H


#include "baseregister.h"

class StateWithMismatchCounter : public BaseRegister
{
public:
    StateWithMismatchCounter();
    StateWithMismatchCounter(const PosInt& stateSize);
    StateWithMismatchCounter(const PosInt& stateSize, const QString& initialState, const PosInt &rootStateIndex);
    StateWithMismatchCounter(const PosInt& stateSize, const BitArray& initialState, const PosInt &rootStateIndex);
    void setInitialState( const BitArray& initialState);
    void addToMismatch( const Real& value);
    void setMismatchValue( const Real& value);
    Real mismatchCount() const;

    bool suggestedValueExists() const;
    Bit getSuggestedValue() const;
    void setSuggestedValue( const Bit& bit);
    void clearSuggestedValue();
    StateWithMismatchCounter *clone() const;

    BitArray getRootState() const;
    void setRootState(const BitArray &RootState);

    PosInt getRootStateIndex() const;
    void setRootStateIndex(const PosInt &rootStateIndex);

    void resetToOrigin();
    PosInt getRegisterSize() const;

    #if defined(CLOCK_DEBUG)
    PosInt getClockCount() const;
    void incrementClockCount();
    void setClockCount(const PosInt &clockCount);
    #endif

private:
    PosInt m_rootStateIndex = 0;
    Real m_mismatchCount = 0;
    bool m_suggestedValueExists = false;
    #if defined(CLOCK_DEBUG)
    PosInt m_clockCount = 0; //NEW!!
    #endif
    Bit m_suggestedValue = 0;
    BitArray m_RootState;

};

#endif // STATEWITHMISMATCHCOUNTER_H

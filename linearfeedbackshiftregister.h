#ifndef LINEARFEEDBACKSHIFTREGISTER_H
#define LINEARFEEDBACKSHIFTREGISTER_H

#include "baseregister.h"

class LinearFeedbackShiftRegister : public BaseRegister
{
public:
    LinearFeedbackShiftRegister(const PosInt& stateSize);
    LinearFeedbackShiftRegister(const PosInt& stateSize, const QString& initialState);
    LinearFeedbackShiftRegister(const PosInt& stateSize, const BitArray& initialState);
    Bit shiftLeft( const Bit& newBit);
    LinearFeedbackShiftRegister *clone() const;
    PosInt getRegisterSize() const;

};

#endif // LINEARFEEDBACKSHIFTREGISTER_H

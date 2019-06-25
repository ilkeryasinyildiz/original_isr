#ifndef CALCULATIONUTILITY_H
#define CALCULATIONUTILITY_H

#include "constants.h"

class LinearFeedbackShiftRegister;
class KeyRegister;

class CalculationUtility
{
public:

    //Register Operations
    static Bit generateFeedbackLFSR(const BitArray& s, const BitArray& k, ClockIndex clock);
    static Bit generateFeedbackNFSR(const BitArray& n, const BitArray& l, const BitArray& k, ClockIndex clock);
    static Bit generateOutputBit(const BitArray& s, ClockIndex clock);

    //Math Calculations
    static Real calculateForwardGC(const BitArray& s, const QVector<BitArray> &keyList);
    static Real calculateAverageGC(const LinearFeedbackShiftRegister & p_lfsr, const KeyRegister& p_key);
    static Real calculateAlphaTerminate(Real averageGC, Real epsilon, PosInt stateSize, PosInt outputCapacity, PosInt d);
    static Real calculateEpsilonTerminate( Real alphaTerminate, Real epsilon);

    static Real calculateAlphaThreshold(Real averageGC, Real alphaTerminate, Real epsilonTerminate);
    static Real calculatePrgThreshold(Real averageGC, Real alphaThreshold);




private:
    static PosInt getNumOfZeroFeedbacks (const BitArray& s, const QVector<BitArray> &keyList);
    static Real calculateForwardGuessCapacity( PosInt zeroFeedbackCount, PosInt numOfKeyBitsInvolved );
    //Utility Functions
    inline static Real ln(Real value);
};

#endif // CALCULATIONUTILITY_H

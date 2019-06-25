#include "calculationutility.h"
#include <QDebug>
#include "constants.h"
#include "keyregister.h"
#include "linearfeedbackshiftregister.h"
#include <math.h>

Bit CalculationUtility::generateFeedbackLFSR(const BitArray& s, const BitArray& k, ClockIndex clock)
{
    #ifndef SPROUT
    Bit feedback = FEEDBACK(s,k);
    #else
    Bit feedback = FEEDBACK(s,k, clock % C_KEYSIZE);
    #endif
    #ifdef VERBOSE
        qDebug() << "feedback: " << feedback;
        QString feedbackFuncStr;
        feedbackFuncStr.sprintf(STR_FN_FEEDBACK_LFSR_W_ARGS, clock, s[0], s[2], s[21], s[22], k[5], feedback);
        qDebug() << feedbackFuncStr;
    #endif
    return feedback;
}

Bit CalculationUtility::generateFeedbackNFSR(const BitArray &n, const BitArray &l, const BitArray &k, ClockIndex clock)
{
    BitIndex t = clock % k.size();
    Bit feedback = l[0] ^ n[6] ^ k[t] ^ n[1] ^ n[2];
    #ifdef VERBOSE
        qDebug() << "feedback: " << feedback;
    #endif
        return feedback;
}

Bit CalculationUtility::generateOutputBit(const BitArray &s, ClockIndex clock)
{
    Bit output = OUTPUT(s);
    #ifdef VERBOSE
        qDebug() << "Output Function: " << STR_FN_OUTPUT;
        QString outputFuncStr;
        outputFuncStr.sprintf(STR_FN_OUTPUT_W_ARGS, clock, s[1], s[3], s[8], s[11], output);
        qDebug() << outputFuncStr;;
    #endif

        return output;
}

Real CalculationUtility::calculateForwardGC(const BitArray &s, const QVector<BitArray> &keyList)
{
   PosInt zeroFeedbackCount = getNumOfZeroFeedbacks(s, keyList);
   Real fgc = calculateForwardGuessCapacity(zeroFeedbackCount, keyBits.size());
    #ifdef VERBOSE
        qDebug() << "Guess Capacity: " << fgc;
    #endif
        return fgc;
}

Real CalculationUtility::calculateAverageGC(const LinearFeedbackShiftRegister &p_lfsr, const KeyRegister &p_key)
{
    LinearFeedbackShiftRegister *lfsr = p_lfsr.clone();
    KeyRegister *key = p_key.clone();

    QVector<BitArray> possibleKeys = key->collectInvolvedStates(keyBits);
    QVector<BitArray> possibleStates = lfsr->collectInvolvedStates(stateBits);

    Real totalGC = 0.0;
    PosInt numOfCalculations = 0;

    for( int stateIndex = 0; stateIndex < possibleStates.size(); stateIndex++)
    {
        lfsr->loadRegisterValue(possibleStates[stateIndex]);
        Bit feedback = CalculationUtility::generateFeedbackLFSR(lfsr->registerValue(), key->registerValue(), 0);
        Real gcf = CalculationUtility::calculateForwardGC(lfsr->registerValue(), possibleKeys);

        totalGC += gcf;
        numOfCalculations++;

        lfsr->shiftLeft(feedback);
        key->rotateLeft();
    }

    Real avgGC = totalGC / numOfCalculations;
    delete lfsr;
    delete key;
    #ifdef VERBOSE
        qDebug() << "Average GC: " << avgGC;
    #endif

    return avgGC;
}

Real CalculationUtility::calculateAlphaTerminate(Real averageGC, Real epsilon, PosInt stateSize, PosInt outputCapacity, PosInt d)
{
    #if defined(CALCULATIONS_DEBUG)
    qDebug() << "Alpha Terminate:";
    Real m = 1 / pow(2 * averageGC - 1, 2);
    qDebug() << QString("m = 1 / pow(2 * averageGC - 1, 2)");
    qDebug() << QString().sprintf("m = 1 / pow(2 * %f - 1, 2) = %f", averageGC, m);
    Real n = sqrt(-2 * ln(epsilon));
    qDebug() << QString("n = sqrt(-2 * ln(epsilon))");
    qDebug() << QString().sprintf("n = sqrt(-2 * ln(%f)) = %f", epsilon, n);
    Real p = sqrt(2 * ln(2) * (stateSize - outputCapacity - d - 1));
    qDebug() << QString("p = sqrt(2 * ln(2) * (stateSize - outputCapacity - d - 1))");
    qDebug() << QString().sprintf("p = sqrt(2 * ln(2) * (%d - %d - %d - 1)) = %f", stateSize, outputCapacity, d, p);
    Real r = pow( n + p, 2);
    qDebug() << QString("r = pow( n + p, 2)");
    qDebug() << QString().sprintf("r = pow( %f + %f, 2) = %f", n, p, r);
    Real result = m * r;
    qDebug() << QString("Alpha Terminate = m * r");
    qDebug() << QString().sprintf("Alpha Terminate =  %f * %f = %f", m, r, result);
    #else
    Real result = (1 / pow(2 * averageGC - 1, 2)) * pow(sqrt(-2 * ln(epsilon)) + sqrt(2 * ln(2) * (stateSize - outputCapacity - d - 1)), 2);
    #endif
    return result;
}

Real CalculationUtility::calculateEpsilonTerminate(Real alphaTerminate, Real epsilon)
{
    #if defined(CALCULATIONS_DEBUG)
    qDebug() << "Epsilon Terminate:";
    Real result = sqrt(-1 * ln(epsilon) / (2 * alphaTerminate));
    qDebug() << QString("Epsilon Terminate = sqrt(-1 * ln(epsilon) / (2 * alphaTerminate))");
    qDebug() << QString().sprintf("Epsilon Terminate = sqrt(-1 * ln(%f) / (2 * %f)) = %f", epsilon, alphaTerminate, result);
    #else
    Real result = sqrt(-1 * ln(epsilon) / (2 * alphaTerminate));
    #endif
    return result;
}

Real CalculationUtility::calculateAlphaThreshold(Real averageGC, Real alphaTerminate, Real epsilonTerminate)
{
    #if defined(CALCULATIONS_DEBUG)
    qDebug() << "Alpha Threshold:";
    Real result = fabs(alphaTerminate * (1 - averageGC + epsilonTerminate));
    qDebug() << QString("Alpha Threshold = fabs(alphaTerminate * (1 - averageGC + epsilonTerminate))");
    qDebug() << QString().sprintf("Alpha Threshold = fabs(%f * (1 - %f + %f)) = %f", alphaTerminate, averageGC, epsilonTerminate, result);
    #else
    Real result = fabs(alphaTerminate * (1 - averageGC + epsilonTerminate));
    #endif
    return result;
}

Real CalculationUtility::calculatePrgThreshold(Real averageGC, Real alphaThreshold)
{
    return pow(1 - averageGC, alphaThreshold);
}

PosInt CalculationUtility::getNumOfZeroFeedbacks(const BitArray &s, const QVector<BitArray> &keyList)
{
    PosInt zeroFeedbackCount = 0;
    for (const BitArray& key: keyList )
    {
        #if defined(SPROUT)
        Bit feedback = FEEDBACK(s, key, 0);
        #else
        Bit feedback = FEEDBACK(s,key);
        #endif
        if (feedback == 0)
        {
            zeroFeedbackCount++;
        }
    }

    return zeroFeedbackCount;
}

Real CalculationUtility::calculateForwardGuessCapacity(PosInt zeroFeedbackCount, PosInt numOfKeyBitsInvolved)
{
    Real gc = 0.5 + fabs( ( static_cast<Real>(zeroFeedbackCount) / pow(2, numOfKeyBitsInvolved) ) - 0.5 );
    return gc;
}

Real CalculationUtility::ln(Real value)
{
    Real result = static_cast<Real>(logl(static_cast<long double>(value)));
    return result;
}



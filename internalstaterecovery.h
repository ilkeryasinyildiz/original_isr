#ifndef ALGORITHM21_H
#define ALGORITHM21_H

#include "constants.h"

class LinearFeedbackShiftRegister;
class StateWithMismatchCounter;

class InternalStateRecovery
{
public:
    static void run();
private:
    static bool determine(StateWithMismatchCounter *state, const Bit& determinedFeedback, LinearFeedbackShiftRegister& lfsr, QVector<StateWithMismatchCounter *> &NEW, const Real &alpha_threshold);
    static bool check_and_guess(const StateWithMismatchCounter *state, QVector<StateWithMismatchCounter *> &NEW, const Real &alpha_threshold, const Bit& currentOutput, const Bit& actualOutput);
    static QVector<StateWithMismatchCounter *> generateAllCandidates(LinearFeedbackShiftRegister &lfsr);
    static QVector<StateWithMismatchCounter *> fetchLimitedCandidates(const QVector<BitArray>& allCandidates, const PosInt& startIndex, const PosInt& candidateCount, const PosInt& registerSize);
    #if defined(SIEVE_MODE)
    static QVector<StateWithMismatchCounter *> fetchLimitedCandidates(const QVector<StateWithMismatchCounter *>& allCandidates, const PosInt& startIndex, const PosInt& candidateCount, const PosInt& registerSize);
    #endif
    static QVector<StateWithMismatchCounter *> reduceDuplicateRootCandidates(const QVector<StateWithMismatchCounter *> &CUR);
    static PosInt numOfRoots(const QVector<StateWithMismatchCounter *> &CUR);
    static void printStates( QVector<StateWithMismatchCounter *>& states);
};


#endif // ALGORITHM21_H

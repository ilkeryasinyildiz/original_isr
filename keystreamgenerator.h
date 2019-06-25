#ifndef KEYSTREAMGENERATOR_H
#define KEYSTREAMGENERATOR_H

#include "constants.h"

struct GeneratedStatesType{
    std::vector<BitArray> generated_states_lfsr;
    //std::vector<BitArray> generated_states_nfsr;
    BitArray generated_output;
};


class KeystreamGenerator
{
public:
    static GeneratedStatesType generate( const QString& initialStateLFSR, const QString & initialKey, const PosInt& clock_limit);
    static void print( const GeneratedStatesType& generatedStates);
private:
    static void printGeneratedStates( const  std::vector<BitArray>& generatedStates);
    static void printKeystream( const BitArray& keystream);
};

#endif // KEYSTREAMGENERATOR_H

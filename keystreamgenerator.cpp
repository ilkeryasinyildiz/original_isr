#include "keystreamgenerator.h"

#include "linearfeedbackshiftregister.h"
#include "keyregister.h"
#include "calculationutility.h"
#include "statewithmismatchcounter.h"
#include <QDebug>

#include <iostream>
#include <iomanip>

using namespace std;


GeneratedStatesType KeystreamGenerator::generate(const QString &initialStateLFSR, const QString &initialKey, const PosInt &clock_limit)
{
    LinearFeedbackShiftRegister lfsr(initialStateLFSR.length(), initialStateLFSR);
    KeyRegister key(initialKey.length(), initialKey);
    GeneratedStatesType generatedStates;

    for( PosInt clockIndex = 0; clockIndex < clock_limit; clockIndex++)
    {
         Bit feedback = CalculationUtility::generateFeedbackLFSR(lfsr.registerValue(), key.registerValue(), clockIndex);
         Bit output = CalculationUtility::generateOutputBit(lfsr.registerValue(), clockIndex);

         generatedStates.generated_states_lfsr.push_back(lfsr.registerValue());
         generatedStates.generated_output.push_back(output);

         lfsr.shiftLeft(feedback);
         key.rotateLeft();
    }

    return generatedStates;
}

void KeystreamGenerator::print(const GeneratedStatesType &generatedStates)
{
    printKeystream(generatedStates.generated_output);
    printGeneratedStates(generatedStates.generated_states_lfsr);
}

void KeystreamGenerator::printGeneratedStates(const std::vector<BitArray> &generatedStates)
{
    cout << "Generated Status (" << generatedStates.size() << " clocks): " << endl;

    for ( PosInt stateIndex = 0; stateIndex < generatedStates.size(); stateIndex++)
    {
         BitArray state = generatedStates[stateIndex];
         cout << setfill('0') << setw(4) << stateIndex; // leading zeros
         cout << " - ";
         for (PosInt bitIndex = 0; bitIndex < state.size(); bitIndex++)
         {
             cout << +state[bitIndex];
         }
         cout << endl;
    }
}

void KeystreamGenerator::printKeystream(const BitArray &keystream)
{
    cout << "Keystream (" << keystream.size() << " bits):" << endl;

    const PosInt maxBitsPerLine = 50;
    PosInt remainedBits = keystream.size();

    while( remainedBits > 0)
    {
        PosInt resumeBit = keystream.size() - remainedBits;
        cout << setfill('0') << setw(4) << resumeBit; // leading zeros
        cout << " - ";

        if (remainedBits < maxBitsPerLine)
        {
            for (PosInt bitIndex = resumeBit; bitIndex < keystream.size(); bitIndex++)
            {
                cout << +keystream[bitIndex];
            }

            remainedBits = 0;
        }
        else
        {
            PosInt startBit = resumeBit;
            PosInt endBit = resumeBit + maxBitsPerLine;

            for( PosInt bitIndex = startBit; bitIndex < endBit; bitIndex++)
            {
                 cout << +keystream[bitIndex];
            }

            cout << endl;

            remainedBits -= maxBitsPerLine;
        }


    }

    cout << endl;
}



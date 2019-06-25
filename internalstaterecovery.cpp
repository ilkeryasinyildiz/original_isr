#include "internalstaterecovery.h"

#include "linearfeedbackshiftregister.h"
#include "keyregister.h"
#include "calculationutility.h"
#include "statewithmismatchcounter.h"
#include "keystreamgenerator.h"
#include "keystreamgenerator.h"

#include <QString>
#include <QDebug>
#include <chrono>
#include <math.h>
using namespace std;
using namespace std::chrono;

void InternalStateRecovery::run()
{
    LinearFeedbackShiftRegister lfsr = LinearFeedbackShiftRegister(C_LFSR_SIZE, INITIAL_LFSR_STATE);
    KeyRegister key = KeyRegister(C_KEYSIZE, INITIAL_KEY);

    //************************************
    //      Algorithm Configuration
    //************************************
    #if defined(SIEVE_MODE)
    const PosInt max_sieve_count = 10;
    const Real alpha_terminate_divider = 10;
    const Real alpha_threshold_multiplier1 = 0.75;
    const Real alpha_threshold_multiplier2 = 0.75;
    const PosInt maxReqRootsToStop = 1;
    const Real C_ALPHA_TER = 13;
    const PosInt C_ALPHA_THR0 = 6;
    const PosInt C_ALPHA_THR1 = 5;
    struct Stats
    {
        PosInt sieveIndex = 0;
        PosInt globalAlphaTerminate = 0;
        PosInt alphaTerminate = 0;
        Real alphaThreshold = 0.0;
        Real averageMissTime = 0;
        PosInt sievedCandidateCount = 0;
        PosInt numOfRoots = 0;
        PosInt totalClockings = 0;
        PosInt totalTime = 0;
    };
    QList<Stats> statsList;
    PosInt totalTime = 0;
    #endif

    const Real average_guess_capacity = CalculationUtility::calculateAverageGC(lfsr, key);
    const PosInt d = 0;
    const Real epsilon = 1 - C_SUCCESS_RATE;
    const PosInt first_clock_t = 0;
    PosInt clock_offset = 0;
    const PosInt output_capacity_theta = lfsr.getRegisterSize() - outputBits.last();
    //const PosInt forward_output_capacity = output_capacity_theta - 1;
    PosInt alpha_terminate = static_cast<PosInt>(CalculationUtility::calculateAlphaTerminate(average_guess_capacity, epsilon, C_LFSR_SIZE, output_capacity_theta, d));
    Real epsilon_terminate = CalculationUtility::calculateEpsilonTerminate(alpha_terminate, epsilon);
    Real alpha_threshold = CalculationUtility::calculateAlphaThreshold(average_guess_capacity, alpha_terminate, epsilon_terminate);


    GeneratedStatesType generatedKeystream = KeystreamGenerator::generate(INITIAL_LFSR_STATE, INITIAL_KEY, alpha_terminate);
    KeystreamGenerator::print(generatedKeystream);
    //************************************
    //        Flow Configuration
    //************************************
    PosInt candidateBufferSize = 1000;
    PosInt startIndex = 0;
    PosInt endIndex = pow(2, lfsr.getRegisterSize());
    //PosInt endIndex = 10000;
    PosInt totalMissTime = 0;
    PosInt totalHitTime = 0;
    PosInt maxMissTime = 0;
    PosInt minMissTime = 1000000;
    PosInt misses = 0;
    PosInt hits = 0;
    PosInt totalClocks = 0;
    #if defined(CLOCK_DEBUG)
    PosInt minEliminationClock = 9999999;
    PosInt maxEliminationClock = 0;
    PosInt totalEliminationClock = 0;
    PosInt eliminatedStateCount = 0;
    #endif

    //Init time start
    auto init_time_start = high_resolution_clock::now();

    QVector<BitArray> generatedKeys = key.collectInvolvedStates(keyBits);

    // Fill Candidate States - Begin

    BitIndexArray allBits;
    for( BitIndex i = 0; i < lfsr.getRegisterSize(); i++)
    {
         allBits.append(i);
    }
    QVector<BitArray> candidateArrays = lfsr.collectInvolvedStates(allBits);

    auto init_time_end = high_resolution_clock::now();
    auto init_mseconds = duration_cast<milliseconds>(init_time_end - init_time_start).count();
    //qDebug() << "Initialization completed in " << init_mseconds << " mseconds";

    auto algorithmCore = [ &lfsr, &key, &clock_offset, &generatedKeys, &generatedKeystream, &alpha_terminate, &output_capacity_theta, &alpha_threshold, &totalClocks
        #if defined(CLOCK_DEBUG)
            , &minEliminationClock, &maxEliminationClock, &eliminatedStateCount, &totalEliminationClock
        #endif
            ] (QVector<StateWithMismatchCounter*>& CUR ) {

        QVector<StateWithMismatchCounter*> NEW;

//        //Experimental 2019-04-29
//        //Start lfsr from start till clock_offset
//        lfsr.fillWithString(INITIAL_LFSR_STATE);
//        key.fillWithString(INITIAL_KEY);
//        for( PosInt i = 0; i < clock_offset; i++)
//        {
//             Bit fb = CalculationUtility::generateFeedbackLFSR(lfsr.registerValue(), key.registerValue(), i);
//             lfsr.shiftLeft(fb);
//             key.rotateLeft();
//        }
//        //Experimental

        const PosInt firstClock = first_clock_t + clock_offset;
        const PosInt lastClock = alpha_terminate + clock_offset;

        for( PosInt clock = firstClock; clock < lastClock; clock++)
        {
//            qDebug() << "***************************************************************************";
//            qDebug() << QString("").sprintf("****************  START OF CYCLE (%d) -    ***************", clock);
//            qDebug() << "***************************************************************************";

            //qDebug() << clock << "," << CUR.size();

            for ( StateWithMismatchCounter* candidateState: CUR)
            {

                #if defined(CLOCK_DEBUG)
                PosInt clockCount = candidateState->getClockCount();
                #endif

//                if ( candidateState->getRootStateIndex() == 656057 )
//                {
//                    candidateState->printRegister();
//                }

                lfsr.loadRegisterValue(candidateState->registerValue());

                LinearFeedbackShiftRegister *lfsr_future = lfsr.clone();
                KeyRegister *key_future = key.clone();


                PosInt futureClockTarget = clock + output_capacity_theta - 1;

                for( PosInt futureClock = clock; futureClock <= futureClockTarget; futureClock++)
                {
                    Bit future_feedback = CalculationUtility::generateFeedbackLFSR(lfsr_future->registerValue(),key_future->registerValue(), futureClock);
                    key_future->rotateLeft();
                    lfsr_future->shiftLeft(future_feedback);
                }

                Bit currentFeedback = CalculationUtility::generateFeedbackLFSR(lfsr.registerValue(), key.registerValue(), clock);
                Bit currentOutput = CalculationUtility::generateOutputBit(lfsr.registerValue(), clock);
                Bit futureOutput = CalculationUtility::generateOutputBit(lfsr_future->registerValue(),futureClockTarget);

                delete lfsr_future;
                delete key_future;

                Real gcf = CalculationUtility::calculateForwardGC(lfsr.registerValue(), generatedKeys);

                if ( gcf == 0.5 )
                {
                    candidateState->clearSuggestedValue();
                    candidateState->addToMismatch(0.5);
                }
                else
                {
                    candidateState->setSuggestedValue(currentFeedback);
                }

                bool possibleToDetermine = POSSIBLE_TO_DETERMINE(lfsr.registerValue(), output_capacity_theta);

                if ( possibleToDetermine )
                {
                   BitArray s = lfsr.registerValue();
                   Bit determinedFeedback = DETERMINE(futureOutput, s, output_capacity_theta);
                   bool eliminated = determine(candidateState, determinedFeedback, lfsr, NEW, alpha_threshold);
                    #if defined(FIX_OF_DETERMINE_CLOCKING)
                    if(!eliminated)
                    {
                        totalClocks += 1;
                    }
                    #else
                   if(!eliminated)
                   {
                       totalClocks += 1;
                   }
                   else
                   {
                       totalClocks += 1;
                   }
                    #endif
                   #if defined(CLOCK_DEBUG)
                    if (eliminated){
                       if (clockCount > maxEliminationClock)
                           maxEliminationClock = clockCount;
                       else if(clockCount < minEliminationClock)
                          minEliminationClock = clockCount;

                       totalEliminationClock += clockCount;
                       eliminatedStateCount++;

                    }
                    #endif
                }
                else
                {
                   bool forked = check_and_guess(candidateState, NEW, alpha_threshold, currentOutput, generatedKeystream.generated_output[clock]);
                   if (forked)
                   {
                       totalClocks += 2;
                   }
                    #if defined(CLOCK_DEBUG)
                    else
                    {
                       if (clockCount > maxEliminationClock)
                           maxEliminationClock = clockCount;
                       else if(clockCount < minEliminationClock)
                          minEliminationClock = clockCount;

                       totalEliminationClock += clockCount;
                       eliminatedStateCount++;
                    }
                    #endif
                }

            }

            //qDebug() << "***************************************************************************";
            //qDebug() << QString("").sprintf("****************  END OF CYCLE (%d) -    ***************", clock);
            //qDebug() << "***************************************************************************";

            CUR.clear();
            if ( NEW.size() != 0)
            {
                //qDebug() << "Size of NEW: " << NEW.size();
                //qDebug() << "Check & Guess Count: " << checkAndGuessCount;
                //qDebug() << "Determine Count: " << determineCount;
                CUR = NEW;
                NEW.clear();
                key.rotateLeft();
            }
            else
            {
                break;
            }



        }

        return CUR;
    };

    //*******************************
    // ***  Algorithm Core - End ****
    //*******************************

    QVector<StateWithMismatchCounter*> hitStates;

    #if defined(SIEVE_MODE)
    qDebug() << "d: " << +d;
    qDebug() << QString("").sprintf("Epsilon: %f", epsilon);
    qDebug() << QString("").sprintf("AVG GC: %f", average_guess_capacity);
    qDebug() << QString("").sprintf("Output Cap: %d", output_capacity_theta);

    qDebug() << QString("").sprintf("Alpha Ter(Original): %d", alpha_terminate);
    qDebug() << QString("").sprintf("Alpha Thr(Original): %f", alpha_threshold);
    qDebug() << QString("").sprintf("Epsilon Ter(Original): %f", epsilon_terminate);
    //FIX 3
    //Real divided_alpha_terminate = alpha_terminate / (alpha_terminate / 50);
    Real divided_alpha_terminate = alpha_terminate / alpha_terminate_divider;
    alpha_terminate = static_cast<PosInt>(ceil(divided_alpha_terminate));
    //alpha_terminate = C_ALPHA_TER;
    epsilon_terminate = CalculationUtility::calculateEpsilonTerminate(alpha_terminate, epsilon);
    Real alpha_threshold_orig = CalculationUtility::calculateAlphaThreshold(average_guess_capacity, alpha_terminate, epsilon_terminate);
    //alpha_threshold = ((alpha_terminate / 2) + ((1 - average_guess_capacity) * alpha_terminate  ) ) / alpha_threshold_divider;
    alpha_threshold = alpha_threshold_orig * alpha_threshold_multiplier1;
    //alpha_threshold = C_ALPHA_THR0;
    qDebug() << QString("").sprintf("Alpha Ter(Fixed): %d", alpha_terminate);
    qDebug() << QString("").sprintf("Alpha Thr(Fixed-Auto): %f", alpha_threshold_orig);
    qDebug() << QString("").sprintf("Alpha Thr(Fixed-Multiplied)*: %f", alpha_threshold);
    qDebug() << QString("").sprintf("Epsilon Ter(Fixed): %f", epsilon_terminate);
    //FIX 3
    QVector<StateWithMismatchCounter*> hitStatesCumulative;

    for( PosInt sieveIndex = 0; sieveIndex < max_sieve_count; sieveIndex++){
        qDebug() << "***************************************************************************";
        qDebug() << "************  ALGORITHM 2.1 - INTERNAL STATE RECOVERY(SIEVE MODE)   ***********";
        qDebug() << "***************************************************************************";
        qDebug() << "Sieve Index: " << sieveIndex;
        qDebug() << "Max Number of Sieves:" << max_sieve_count;
        PosInt globalAlphaTerminate = alpha_terminate *(sieveIndex+1);

        if ( sieveIndex != 0 && !hitStatesCumulative.isEmpty()){
            qDebug() << "Updating EndIndex";
            endIndex = hitStatesCumulative.size();
//            qDebug() << "Extending AlphaTerminate value...";
//            alpha_terminate *= 2;
                #ifndef SIEVE_MODE_SPEEDUP
                epsilon_terminate = CalculationUtility::calculateEpsilonTerminate(globalAlphaTerminate, epsilon);
                alpha_threshold_orig = CalculationUtility::calculateAlphaThreshold(average_guess_capacity, globalAlphaTerminate, epsilon_terminate);
                alpha_threshold = alpha_threshold_orig * alpha_threshold_multiplier2;
                #endif
                //alpha_threshold += C_ALPHA_THR1;

              //alpha_threshold = ((alpha_terminate / 2) + ((1 - average_guess_capacity) * alpha_terminate * (sieveIndex+1) ) ) / alpha_threshold_divider;
            qDebug() << QString("").sprintf("Alpha Ter(Fixed): %d", alpha_terminate);
            qDebug() << QString("").sprintf("Alpha Thr(Fixed-Auto): %f", alpha_threshold_orig);
            qDebug() << QString("").sprintf("Alpha Thr(Fixed-Multiplied)*: %f", alpha_threshold);
            qDebug() << QString("").sprintf("Epsilon Ter(Fixed): %f", epsilon_terminate);
            qDebug() << "Alive Candidate Count:" << hitStatesCumulative.size();
            qDebug() << "Generating extended Keystream";
            generatedKeystream = KeystreamGenerator::generate(INITIAL_LFSR_STATE, INITIAL_KEY, globalAlphaTerminate);
        }
    #else
        qDebug() << "***************************************************************************";
        qDebug() << "****************  ALGORITHM 2.1 - INTERNAL STATE RECOVERY   ***************";
        qDebug() << "***************************************************************************";
        qDebug() << QString("").sprintf("Alpha Terminate: %d", alpha_terminate);
        qDebug() << QString("").sprintf("Alpha Threshold: %f", alpha_threshold);
        qDebug() << QString("").sprintf("Average Guess Capacity: %f", average_guess_capacity);
        qDebug() << QString("").sprintf("Epsilon: %f", epsilon);
        qDebug() << QString("").sprintf("Epsilon Terminate: %f", epsilon_terminate);
    #endif

    auto algorithm_time_start = high_resolution_clock::now();

    for ( PosInt stateIndex = startIndex; stateIndex < endIndex; stateIndex += candidateBufferSize  )
    {
        auto cycle_time_start = high_resolution_clock::now();
        #if defined(SHOW_PROGRESS)
            Real progress = (Real)(stateIndex * 100.0) / (Real)endIndex;
            qDebug() << QString("").sprintf("Progress: %.2f/100", progress);
        #endif

        QVector<StateWithMismatchCounter*> candidates;
        #if defined ( SIEVE_MODE)
                //qDebug() << "Cumulative Hit States:" << hitStatesCumulative.size();
                if ( sieveIndex == 0)
                {
                    candidates = fetchLimitedCandidates(candidateArrays, stateIndex, candidateBufferSize, C_LFSR_SIZE);
                }
                else
                {
                    candidates = fetchLimitedCandidates(hitStatesCumulative, stateIndex, candidateBufferSize, C_LFSR_SIZE);
                    //qDebug() << "Printing Sieved Candidates:";
                    //printStates(candidates);
                }
        #else
                candidates = fetchLimitedCandidates(candidateArrays, startIndex, candidateBufferSize, C_LFSR_SIZE);
        #endif
        //qDebug() << "Num of session candidates: " << candidates.size();
        QVector<StateWithMismatchCounter*> aliveStates = algorithmCore(candidates);
        //qDeleteAll(aliveStates);
        //qDebug() << "Reducing Results: ";
        //QVector<StateWithMismatchCounter*> reducedList = reduceDuplicateRootCandidates(aliveStates);
        //qDebug() << "Printing Reduced List:";
        //printStates(reducedList);
        //hitStates += reducedList;
        hitStates += aliveStates;
        auto cycle_time_end = high_resolution_clock::now();
        auto mseconds = duration_cast<milliseconds>(cycle_time_end - cycle_time_start).count();

        //misses += (endIndex - stateIndex > candidateBufferSize ) ? candidateBufferSize : (endIndex - stateIndex);
        if ( !aliveStates.isEmpty()){
            totalHitTime += mseconds;
            hits += 1;
            //misses -= 1;
        }
        else{
            totalMissTime += mseconds;
            misses += 1;
            if ( mseconds > maxMissTime)
                maxMissTime = mseconds;
            if ( mseconds < minMissTime)
                minMissTime = mseconds;
        }

        #if defined(SHOW_PROGRESS)
            qDebug() << "Algorithm completed in " << mseconds << " milliseconds";
            qDebug() << hitStates.count() << " states discovered.";
        #endif
    }

    auto algorithm_time_end = high_resolution_clock::now();
    auto algorithm_mseconds = duration_cast<milliseconds>(algorithm_time_end - algorithm_time_start).count();
    auto algorithm_seconds = algorithm_mseconds / 1000;

    Real avgMissTime = (misses == 0) ? 0 : (Real)totalMissTime / (Real)misses;
    Real avgHitTime = (hits == 0 ) ? 0 : (Real)totalHitTime / (Real)hits;
    #if defined(SIEVE_MODE)
        Stats stats;
        stats.sieveIndex = sieveIndex;
        stats.alphaTerminate = alpha_terminate;
        stats.totalTime = static_cast<PosInt>(algorithm_mseconds);
        stats.globalAlphaTerminate = globalAlphaTerminate;
        stats.alphaThreshold = alpha_threshold;
        stats.sievedCandidateCount = hitStates.size();
        stats.averageMissTime = avgMissTime;
        stats.totalClockings = totalClocks;
        stats.numOfRoots = numOfRoots(hitStates);
        statsList.append(stats);
        qDebug() << "SieveIndex :" << sieveIndex;
        totalClocks = 0;
    #endif
    qDebug() << "StartIndex :" << startIndex;
    qDebug() << "EndIndex :" << endIndex;
    qDebug() << QString("").sprintf("Alpha Terminate: %d", alpha_terminate);
    qDebug() << QString("").sprintf("Alpha Threshold: %f", alpha_threshold);
    qDebug() << QString("").sprintf("Average Guess Capacity: %f", average_guess_capacity);
    qDebug() << QString("").sprintf("Epsilon: %f", epsilon);
    qDebug() << QString("").sprintf("Epsilon Terminate: %f", epsilon_terminate);
    qDebug() << "Candidate Buffer Size: " << candidateBufferSize;
    qDebug() << QString("").sprintf("Average Miss Time: %f", avgMissTime);
    qDebug() << "Minimum Miss Time: " << minMissTime;
    qDebug() << "Maximum Miss Time: " << maxMissTime;
    qDebug() << "Number of Misses: " << misses;
    qDebug() << QString().sprintf("Average Hit Time: %f", avgHitTime);
    qDebug() << "Number of Hits: " << hits;
    qDebug() << "Total Time: " << algorithm_seconds << " seconds";
    qDebug() << "Total Time: " << algorithm_mseconds << " mseconds";
    qDebug() << "Total Clockings: " << totalClocks;
    #if defined(CLOCK_DEBUG)
            Real avgEliminatedClock = (Real)totalEliminationClock / (Real)eliminatedStateCount;
            qDebug() << QString("").sprintf("Elimination Clock(AVG): %f", avgEliminatedClock);
            qDebug() << QString("").sprintf("Elimination Clock(MAX): %d", maxEliminationClock);
            qDebug() << QString("").sprintf("Elimination Clock(MIN): %d", minEliminationClock);
            qDebug() << QString("").sprintf("Total Elimination Clock: %d", totalEliminationClock);
            qDebug() << QString("").sprintf("Eliminated State Count: %d", eliminatedStateCount);
    #endif
    //printStates(hitStates);

    #if defined(SIEVE_MODE)

            if( stats.numOfRoots > maxReqRootsToStop){
                //Call next sieve
                hitStatesCumulative.clear();
                #ifndef SIEVE_MODE_SPEEDUP
                for( StateWithMismatchCounter* s: hitStates)
                {
                    s->setMismatchValue(0);
                }
                #endif
                hitStatesCumulative += hitStates;
                hitStates.clear();
                //apply clock offset
                clock_offset += alpha_terminate;
                continue;
            }

            if( hitStates.size() == 0){
                //Show previous sieve output & exit
                qDebug() << "Algorithm Finished Completely 1!";
                qDebug() << "Nothing Left!";
                break;
            }
            else
            {
                //Show current sieve output & exit
                qDebug() << "Algorithm Finished Completely 2!";
                //printStates(hitStates);
//                Real minMismatch = 999999;
//                QList<StateWithMismatchCounter*> bestMatches;
//                for( StateWithMismatchCounter* state: hitStates)
//                {
//                    if ( state->mismatchCount() < minMismatch)
//                    {
//                        minMismatch = state->mismatchCount();
//                    }
//                }

//                for( StateWithMismatchCounter* state: hitStates)
//                {
//                    if ( state->mismatchCount() <= minMismatch)
//                    {
//                        bestMatches.append(state);
//                    }
//                }

//                for( StateWithMismatchCounter* bestMatch : bestMatches)
//                {
//                    qDebug() << "Last State:";
//                    bestMatch->printRegister();
//                    qDebug() << "Mismatch: " << bestMatch->mismatchCount();
//                    bestMatch->resetToOrigin();
//                    qDebug() << "Original State:";
//                    bestMatch->printRegister();
//                }

                qDebug() << "Reducing Results...";
                hitStates = reduceDuplicateRootCandidates(hitStates);

                for( StateWithMismatchCounter* state : hitStates)
                {
                    //qDebug() << "Last State:";
                    //state->printRegister();
                    qDebug() << "Mismatch: " << state->mismatchCount();
                    state->resetToOrigin();
                    qDebug() << "Original State:";
                    state->printRegister();
                }

                //printStates(hitStatesCumulative);
                qDebug() << "El finito";
                break;
            }


    }

    PosInt globalTotalClocks = 0;
    Real globalAverageMissTime = 0;
    PosInt totalNumOfScannedCandidates = 0;
    PosInt globalTotalTime = 0;



    for( Stats stats : statsList)
    {
        globalTotalClocks += stats.totalClockings;
        globalTotalTime += stats.totalTime;
        globalAverageMissTime += stats.averageMissTime * stats.sievedCandidateCount;
        totalNumOfScannedCandidates += stats.sievedCandidateCount;
    }
    globalAverageMissTime = globalAverageMissTime / totalNumOfScannedCandidates;

    qDebug() << "Sieved Candidate Counts:";
    qDebug() << "SieveIndex\t"
             << "AlphaTerminate(Local)\t"
             << "AlphaTer(Global)\t"
             << "AlphaThreshold\t\t"
             << "SievedCandidateCount\t\t"
             << "NumOfRoots\t"
             << "TotalTime(ms)";
    for( Stats stats : statsList)
    {
            qDebug() << stats.sieveIndex << "\t\t"
                     << stats.alphaTerminate << "\t\t\t"
                     << stats.globalAlphaTerminate << "\t\t\t"
                     << stats.alphaThreshold << "\t\t\t"
                     << stats.sievedCandidateCount << "\t\t\t"
                     << stats.numOfRoots << "\t\t\t"
                     << stats.totalTime;
    }

    qDebug() << "Global Clocks: " << globalTotalClocks << " pulses";
    qDebug() << "Global Avg Miss Time: " << globalAverageMissTime << " milliseconds";
    qDebug() << "Total Time: " << globalTotalTime << " milliseconds";


    #endif




}

bool InternalStateRecovery::determine(StateWithMismatchCounter *state, const Bit &determinedFeedback, LinearFeedbackShiftRegister &lfsr, QVector<StateWithMismatchCounter *> &NEW, const Real& alpha_threshold)
{
    bool still_alive = false;
    #ifndef FIX_OF_DETERMINE_CLOCKING
         lfsr.shiftLeft(determinedFeedback);
         #if defined(CLOCK_DEBUG)
             state->incrementClockCount();
         #endif
         state->loadRegisterValue(lfsr.registerValue());
     #endif

     if (state->suggestedValueExists() && state->getSuggestedValue() != determinedFeedback)
     {
         state->addToMismatch(1);
     }

    if ( state->mismatchCount() < alpha_threshold)
    {
        #if defined(FIX_OF_DETERMINE_CLOCKING)
            lfsr.shiftLeft(determinedFeedback);
            #if defined(CLOCK_DEBUG)
                state->incrementClockCount();
            #endif
            state->loadRegisterValue(lfsr.registerValue());
        #endif
        NEW.push_back(state);
        still_alive = true;
    }
    else
    {
        still_alive = false;
        delete state;
    }

    return still_alive;
}

bool InternalStateRecovery::check_and_guess(const StateWithMismatchCounter* state, QVector<StateWithMismatchCounter *> &NEW, const Real &alpha_threshold, const Bit &currentOutput, const Bit &actualOutput)
{
    bool forked = false;
    #ifdef VERBOSE
        qDebug() << "Check & Guess";
    #endif
    if ( currentOutput == actualOutput)
    {
        StateWithMismatchCounter* s0 = state->clone();
        StateWithMismatchCounter* s1 = state->clone();
        LinearFeedbackShiftRegister lfsr_s0(state->getRegisterSize(), state->registerValue());
        LinearFeedbackShiftRegister lfsr_s1(state->getRegisterSize(), state->registerValue());
        lfsr_s0.shiftLeft(0);
        lfsr_s1.shiftLeft(1);

        if (state->suggestedValueExists())
        {
            if( state->getSuggestedValue() == 0)
            {
                s1->addToMismatch(1.0);
            }
            else
            {
                s0->addToMismatch(1.0);
            }
        }

        if ( s0->mismatchCount() <= alpha_threshold)
        {
            s0->loadRegisterValue(lfsr_s0.registerValue());
            NEW.append(s0);
        }
        else
        {
            delete s0;
        }

        if ( s1->mismatchCount() <= alpha_threshold)
        {
            s1->loadRegisterValue(lfsr_s1.registerValue());
            NEW.append(s1);
        }
        else
        {
            delete s1;
        }

        delete state;
        forked = true;
    }
    else
    {
        delete state;
        #ifdef VERBOSE
            qDebug() << "Mismatch";
        #endif
        forked = false;
        //Mismatch
        //Do nothing
    }

    return forked;
}

QVector<StateWithMismatchCounter*> InternalStateRecovery::generateAllCandidates(LinearFeedbackShiftRegister& lfsr)
{
    PosInt stateIndex = 0;
    QVector<BitArray> candidateArrays;
    QVector<StateWithMismatchCounter*> candidateStates;

    // Method 1: Only involved bit combinations
    // candidates = LinearFeedbackShiftRegister::collectInvolvedStates(stateBits);
    //    for( const BitArray& state : stateList)
    //    {
    //        StateWithMismatchCounter candidate(state.size(), state);
    //        CUR.push_back(candidate);
    //    }

    // Method 2: Only single state
    // StateWithMismatchCounter candidate(lfsr.registerValue().size(), lfsr.registerValue());
    // CUR.push_back(candidate);

    // Method 3: All Bits
    BitIndexArray allBits;
    for( BitIndex i = 0; i < lfsr.getRegisterSize(); i++)
    {
         allBits.append(i);
    }

    candidateArrays = lfsr.collectInvolvedStates(allBits);

    PosInt size = lfsr.getRegisterSize();

    for( BitArray & candidateArray : candidateArrays )
    {
        stateIndex++;
        StateWithMismatchCounter* state = new StateWithMismatchCounter(size, candidateArray, stateIndex);
        candidateStates.append(state);
    }

    return candidateStates;
}

QVector<StateWithMismatchCounter *> InternalStateRecovery::fetchLimitedCandidates(const QVector<BitArray> &allCandidates, const PosInt &startIndex, const PosInt &candidateCount, const PosInt &registerSize)
{
    QVector<StateWithMismatchCounter *> limitedCandidates;

    if (startIndex >= allCandidates.size())
        return limitedCandidates;

    PosInt endIndex = startIndex + candidateCount;

    if (endIndex >= allCandidates.size())
    {
        endIndex = allCandidates.size();
    }

    for(PosInt i = startIndex; i< endIndex; i++)
    {
        const BitArray& candidateArray = allCandidates.at(i);
        StateWithMismatchCounter* state = new StateWithMismatchCounter(registerSize, candidateArray, i);
        limitedCandidates.append(state);
    }
    return limitedCandidates;
}


#if defined(SIEVE_MODE)
QVector<StateWithMismatchCounter *> InternalStateRecovery::fetchLimitedCandidates(const QVector<StateWithMismatchCounter *> &allCandidates, const PosInt &startIndex, const PosInt &candidateCount, const PosInt &registerSize)
{
    //qDebug() << "fetchLimitedCandidates - begin";

    QVector<StateWithMismatchCounter *> limitedCandidates;

    if (startIndex >= allCandidates.size())
        return limitedCandidates;

    PosInt endIndex = startIndex + candidateCount;

    if (endIndex >= allCandidates.size())
    {
        endIndex = allCandidates.size();
    }

    //qDebug() << "Start Index:" << startIndex;
    //qDebug() << "End Index:" << endIndex;

    for(PosInt i = startIndex; i< endIndex; i++)
    {
        StateWithMismatchCounter* state = allCandidates[i];
        limitedCandidates.append(state);
    }

    //qDebug() << "fetchLimitedCandidates - end";
    return limitedCandidates;
}
#endif

QVector<StateWithMismatchCounter *> InternalStateRecovery::reduceDuplicateRootCandidates( const QVector<StateWithMismatchCounter *> &CUR)
{
    QMap<PosInt, StateWithMismatchCounter*> mResultMap;

    for ( StateWithMismatchCounter * state : CUR)
    {
        PosInt stateIndex = state->getRootStateIndex();
        if ( !mResultMap.contains(stateIndex))
        {
            mResultMap.insert( stateIndex, state );
        }
        else
        {
            delete state;
        }
    }

    return mResultMap.values().toVector();
}

PosInt InternalStateRecovery::numOfRoots(const QVector<StateWithMismatchCounter *> &CUR)
{
    QMap<PosInt, StateWithMismatchCounter*> mResultMap;

    for ( StateWithMismatchCounter * state : CUR)
    {
        PosInt stateIndex = state->getRootStateIndex();
        if ( !mResultMap.contains(stateIndex))
        {
            mResultMap.insert( stateIndex, state );
        }
    }

    return static_cast<PosInt>(mResultMap.size());
}

void InternalStateRecovery::printStates(QVector<StateWithMismatchCounter *> &states)
{
    for( StateWithMismatchCounter* state : states){
        qDebug() << "Mismatch: " << state->mismatchCount();
        state->resetToOrigin();
        state->printRegister();
    }
}


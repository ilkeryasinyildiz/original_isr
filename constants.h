#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <QString>
#include <QList>
#include <QVector>
#include <QSet>
#include <QDebug>

#include <set>

//Type Definitions
using Bit = uint16_t;
//using BitArray = std::vector<Bit>;
using BitArray = QVector<Bit>;
using BitIndex = uint32_t;
//using BitIndexArray = std::set<BitIndex>;
using BitIndexArray = QVector<BitIndex>;
using ClockIndex = uint32_t;
using PosInt = uint32_t;
using Real = double;


///////////////////////////////
// Feedback Function Choice
// F1-F22 and SPROUT
#define SPROUT
///////////////////////////////

//Register Sizes
//Set keysize 10 for Sprout, 256 for F1-F22
#ifndef SPROUT
    const PosInt C_KEYSIZE = 256;
#else
    const PosInt C_KEYSIZE = 10;
#endif

const PosInt C_LFSR_SIZE = 20;
// Success rate of algorithm 99% for standard modes, 99.9% for sieve mode
const Real C_SUCCESS_RATE = 0.99;
//const Real C_SUCCESS_RATE = 0.999;

// Reference Sites for conversion:
// http://spellbackwards.com
// https://www.rapidtables.com/convert/number/binary-to-decimal.html
// https://www.random.org

//Test Sample: 10011101010000000101  Decimal: 656057
//Random 1: 11011100100010001001  Decimal: 594235
//Random 2: 11101111011001110011  Decimal: 845559
//Random 3: 00100110110110010010   Decimal: 301924
const QString INITIAL_LFSR_STATE = "10011101010000000101";

//Initial Key, 10 bits for Sprout, 256 bits for F1-F22
#ifndef SPROUT
const QString INITIAL_KEY = "01010110110010011110101110011101"
                            "01100011011010010001101100010110"
                            "00101100001110010011110010001000"
                            "01101001101001100000111000000111"
                            "01110000101110011101110111000010"
                            "00110101010000111010101011011010"
                            "11001010110010110100110100010010"
                            "01001100011001001111110101010110";
#else
const QString INITIAL_KEY = "1010101010";
#endif

// **********************
//    Logging Options
// **********************
//#define VERBOSE
//#define SHOW_PROGRESS
//#define CLOCK_DEBUG
#define CALCULATIONS_DEBUG

// **********************
//         Fixes
// **********************
//Fix 2
#define FIX_OF_DETERMINE_CLOCKING
//Fix 3
//#define SIEVE_MODE
//#define SIEVE_MODE_SPEEDUP

#if defined (F1)
const BitIndexArray stateBits = BitIndexArray() << 0 << 2 << 17 << 18;
const BitIndexArray keyBits = BitIndexArray() << 5;
#define FEEDBACK(S,K) S[0] ^ S[2] ^ ( S[17] & S[18] & K[5] )
#elif defined (F3)
const BitIndexArray stateBits = BitIndexArray() << 0 << 2 << 5 << 10 << 17;
const BitIndexArray keyBits = BitIndexArray() << 5;
#define FEEDBACK(S,K) S[0] ^ S[2] ^ ( S[5] & S[10] ) ^ ( S[17] &  K[5] )
#elif defined (F5)
const BitIndexArray stateBits = BitIndexArray() << 0 << 2 << 5 << 10 << 17 << 18 << 19;
const BitIndexArray keyBits = BitIndexArray() << 5 << 24;
#define FEEDBACK(S,K) S[0] ^ S[2] ^ ( S[5] & S[10] ) ^ ( S[17] & S[18] & K[5] ) ^ ( S[19] & K[24] )
#elif defined (F6)
const BitIndexArray stateBits = BitIndexArray() << 3 << 5 << 10 << 12;
const BitIndexArray keyBits = BitIndexArray() << 0 << 10 << 11;
#define FEEDBACK(S,K) S[10] ^ S[12] ^ (S[5] & K[0] ) ^ ( S[3] & K[10] & K[11] )
#elif defined (F7)
const BitIndexArray stateBits = BitIndexArray() << 3 << 5 << 10 << 11 << 12;
const BitIndexArray keyBits = BitIndexArray() << 0 << 10 << 11;
#define FEEDBACK(S,K) S[11] ^ S[12] ^ (S[5] & K[0] ) ^ ( S[3] & S[10] & K[10] & K[11] )
#elif defined (F8)
const BitIndexArray stateBits = BitIndexArray() << 4 << 5 << 11 << 12;
const BitIndexArray keyBits = BitIndexArray() << 0 << 1 << 3 << 10;
#define FEEDBACK(S,K) S[11] ^ S[12] ^ (S[5] & K[0] & K[3] & K[10] ) ^ ( S[4] & K[1] )
#elif defined (F10)
const BitIndexArray stateBits = BitIndexArray() << 0 << 2 << 15 << 16 << 17 << 18;
const BitIndexArray keyBits = BitIndexArray() << 5 << 10 << 15 << 20 << 25 << 30;
#define FEEDBACK(S,K) S[0] ^ S[2] ^ ( S[15] & S[16] & S[17] & S[18] & K[5] & K[10] & K[15] & K[20] & K[25] & K[30] )
#elif defined (F11)
const BitIndexArray stateBits = BitIndexArray() << 0 << 2 << 17 << 18;
const BitIndexArray keyBits = BitIndexArray() << 5 << 10 << 15 << 20;
#define FEEDBACK(S,K) S[0] ^ S[2] ^ ( S[17] & S[18] & K[5] & K[10] & K[15] & K[20] )
#elif defined (F12)
const BitIndexArray stateBits = BitIndexArray() << 0 << 2 << 17;
const BitIndexArray keyBits = BitIndexArray() << 5 << 10 << 15 << 20;
#define FEEDBACK(S,K) S[0] ^ S[2] ^ ( S[17] & K[5] & K[10] & K[15] & K[20] )
#elif defined (F13)//0.5625 / 1.0000
const BitIndexArray stateBits = BitIndexArray() << 5 << 10 << 11 << 15 << 17;
const BitIndexArray keyBits = BitIndexArray() << 5 << 10 << 15;
#define FEEDBACK(S,K) S[5] ^ S[10] ^ (S[11] & K[5] ) ^ (S[15] & K[10] ) ^ ( S[17] & K[15] )
#elif defined (F14)//0.59375 / 0.87500
const BitIndexArray stateBits = BitIndexArray() << 5 << 10 << 11 << 15 << 17;
const BitIndexArray keyBits = BitIndexArray() << 5 << 10 << 15 << 16;
#define FEEDBACK(S,K) S[5] ^ S[10] ^ (S[11] & K[5] ) ^ (S[15] & K[10] ) ^ ( S[17] & K[15] & K[16] )
#elif defined (F15)//0.609375 / 0.87500
const BitIndexArray stateBits = BitIndexArray() << 5 << 10 << 11 << 15 << 17 << 19;
const BitIndexArray keyBits = BitIndexArray() << 5 << 10 << 15 << 16;
#define FEEDBACK(S,K) S[5] ^ S[10] ^ (S[11] & K[5] ) ^ ( S[15] & K[10] ) ^ ( S[17] & S[19] & K[15] & K[16] )
#elif defined (F16)//0.71875 / 0.9375
const BitIndexArray stateBits = BitIndexArray() << 5 << 10 << 11 << 17 << 19;
const BitIndexArray keyBits = BitIndexArray() << 5 << 15 << 16;
#define FEEDBACK(S,K) S[5] ^ S[10] ^ (S[11] & K[5] ) ^ ( S[17] & S[19] & K[15] & K[16] )
#elif defined (F17)//0.9375 / 0.9375
const BitIndexArray stateBits = BitIndexArray() << 5 << 10 << 11 << 17 << 19;
const BitIndexArray keyBits = BitIndexArray() << 5 << 15 << 16;
#define FEEDBACK(S,K) S[5] ^ S[10] ^ (S[11] & K[5] & K[10] ) ^ ( S[17] & S[19] & K[15] & K[16] )
#elif defined (F18)//0.96875 / 0.96875
const BitIndexArray stateBits = BitIndexArray() << 5 << 10 << 11 << 17 << 19;
const BitIndexArray keyBits = BitIndexArray() << 5 << 11 << 15 << 16 << 17;
#define FEEDBACK(S,K) S[5] ^ S[10] ^ (S[11] & K[5] & K[10] & K[11] ) ^ ( S[17] & S[19] & K[15] & K[16] & K[17] )
#elif defined (F19)//0.875 / 1.00
const BitIndexArray stateBits = BitIndexArray() << 5 << 10 << 11 << 17 << 19;
const BitIndexArray keyBits = BitIndexArray() << 5 << 11 << 15;
#define FEEDBACK(S,K) S[5] ^ S[10] ^ (S[11] & K[5] & K[10] & K[11] ) ^ ( S[17] & S[19] & K[15] )
#elif defined (F20)//0.71875 / 0.9375
const BitIndexArray stateBits = BitIndexArray() << 5 << 10 << 11 << 12 << 17 << 19;
const BitIndexArray keyBits = BitIndexArray() << 5 << 15 << 16;
#define FEEDBACK(S,K) S[5] ^ S[10] ^ (S[11] & S[12] & K[5] ) ^ ( S[17] & S[19] & K[15] & K[16] )
#elif defined (F21)//0.71875 / 0.9375
const BitIndexArray stateBits = BitIndexArray() << 5 << 10 << 11 << 12 << 17 << 19;
const BitIndexArray keyBits = BitIndexArray() << 5 << 6 << 7 << 15 << 16;
#define FEEDBACK(S,K) S[5] ^ ( S[10] & K[7] ) ^ (S[11] & S[12] & K[5] & K[6] ) ^ ( S[17] & S[19] & K[15] & K[16] )
#elif defined (F22)//0.787109 /  0.882813
const BitIndexArray stateBits = BitIndexArray() << 5 << 10 << 11 << 12 << 13 << 17 << 19;
const BitIndexArray keyBits = BitIndexArray() << 105 << 106 << 107 << 115 << 116;
#define FEEDBACK(S,K) S[5] ^ ( S[10] & S[11] & K[107] ) ^ (S[12] & S[13] & K[105] & K[106] ) ^ ( S[17] & S[19] & K[115] & K[116] )
#elif defined (SPROUT)//0.75 /  1.0
const BitIndexArray stateBits = BitIndexArray() << 3 << 9 << 18;
//const BitIndexArray keyBits = BitIndexArray() << 0 << 1 << 2 << 3 << 4 << 5 << 6 << 7 << 8 << 9 << 10 << 11 << 12 << 13 << 14 << 15 << 16 << 17 << 18 << 19;
const BitIndexArray keyBits = BitIndexArray() << 0 << 1 << 2 << 3 << 4 << 5 << 6 << 7 << 8 << 9;
#define FEEDBACK(S,K,T) K[T] & (S[3] ^ S[9] ^ S[18])
#endif

#ifndef SPROUT
const BitIndexArray outputBits = BitIndexArray() << 0 << 1 << 3 << 8 << 11 << 13 << 19;
#define OUTPUT(S)   S[0] ^ S[1] ^ S[3] ^ S[8] ^ S[11] ^ S[13] ^ S[19];
#define POSSIBLE_TO_DETERMINE(S, theta) 0 == 1
#define DETERMINE(future_output, S, theta) future_output ^ S[0+theta] ^ S[1+theta] ^ S[3+theta] ^ S[8+theta] ^ S[11+theta] ^ S[13+theta]
#else
const BitIndexArray outputBits = BitIndexArray() << 0 << 1 << 3 << 5 << 9 << 11 << 13 << 17 << 19;
#define OUTPUT(S)   S[0] ^ (S[5] & S[19] ) ^ S[1] ^ S[3] ^  S[9] ^ S[11] ^ S[13] ^ S[17];
#define POSSIBLE_TO_DETERMINE(S, theta) 0 == 1
#define DETERMINE(future_output, S, theta) future_output ^ S[0+theta] ^ S[1+theta] ^ S[3+theta] ^ S[9+theta] ^ S[11+theta] ^ S[13+theta] ^ S[17+theta];
#endif

#endif // CONSTANTS_H


// The CodeWarrior integer-to-double idiom (high word 0x43300000, low word = the integer, minus 2^52 [+2^31])
// builds a double from two 32-bit words in memory; PowerPC stores the high word first.
#ifndef PPC_DOUBLE_WORDS_H
#define PPC_DOUBLE_WORDS_H
#define PPC_DW_HI 0
#define PPC_DW_LO 1
struct PpcDoubleWords {
    unsigned int hi, lo;
};
#endif

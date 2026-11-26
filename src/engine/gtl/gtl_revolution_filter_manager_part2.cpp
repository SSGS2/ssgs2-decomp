// Revolution filter manager, part 2: the freeing helper for a manager allocation, the parameter
// block size query, and the filter descriptor builder.

#include "classes/gtl_CRevolutionFilterManager.h"

extern "C" {

enum FilterParameterLayout {
    kFilterParameterWords = 24,
    kFilterParameterBytes = 0x60
};
enum ProviderFormatWords {
    kProviderSampleBufferBytes = 2,
    kProviderWorkspaceCount = 3,
    kProviderShowCounterParameter = 4,
    kProviderInitialReadBytes = 10,
    kProviderPacketHeaderWords = 12
};

void gtl_Free(void*);

// Free a filter-manager allocation if supplied.
void dtor_803F96CC(void* self) {
    if (self != 0) {
        gtl_Free(self);
    }
}

} // extern "C"

// Report the 24-word input parameter block size in bytes.
// size must be non-null; returns 1 and does not inspect this.
int gtl::CRevolutionFilterManager::GetParameterBlockSize(int* size) {
    *size = kFilterParameterBytes;
    return 1;
}

// The 24 words are the THP file header read from the disc (big-endian). They are cached in the
// manager, then projected into the output descriptor: word 4 is copied as float bits, words 17 and 18
// keep their low 16 bits, and the other output fields are plain copies. Output +0 is cleared
// and the rest of the output is left as it was.
// Input covers 0x60 bytes and output 0xA4. Returns 1 unconditionally.
int gtl::CRevolutionFilterManager::BuildDescriptor(unsigned int* input, RevolutionFilterOutput* output) {
    unsigned int* cached = cachedParameters;
    unsigned int firstWord, secondWord;
    // Cache the input. Pairs are loaded and stored in the original order, including
    // pairs loaded in reverse; this also fixes behavior for aliasing input.
    firstWord = input[1];
    secondWord = input[0];
    cached[0] = secondWord;
    cached[1] = firstWord;

    firstWord = input[2];
    secondWord = input[3];
    cached[2] = firstWord;
    cached[3] = secondWord;

    firstWord = input[4];
    secondWord = input[5];
    cached[4] = firstWord;
    cached[5] = secondWord;

    firstWord = input[6];
    secondWord = input[7];
    cached[6] = firstWord;
    cached[7] = secondWord;

    firstWord = input[8];
    secondWord = input[9];
    cached[8] = firstWord;
    cached[9] = secondWord;

    firstWord = input[10];
    secondWord = input[11];
    cached[10] = firstWord;
    cached[11] = secondWord;

    firstWord = input[12];
    secondWord = input[13];
    cached[12] = firstWord;
    cached[13] = secondWord;

    firstWord = input[14];
    secondWord = input[15];
    cached[14] = firstWord;
    cached[15] = secondWord;

    cached[16] = input[16];
    firstWord = input[17];
    secondWord = input[18];
    cached[17] = firstWord;
    cached[18] = secondWord;

    cached[19] = input[19];
    secondWord = input[21];
    firstWord = input[20];
    cached[20] = firstWord;
    cached[21] = secondWord;

    secondWord = input[23];
    firstWord = input[22];
    cached[22] = firstWord;
    cached[23] = secondWord;

    // Project the cached words into the descriptor.
    output->baseState = 0;
    output->showCounterFormatParameter = *(float*)&cached[kProviderShowCounterParameter];
    output->initialReadOffset = cached[kProviderInitialReadBytes];
    output->sampleBufferSize = cached[kProviderSampleBufferBytes];
    output->parameterWord6 = cached[6];
    output->parameterWord17 = (unsigned short)cached[17];
    output->parameterWord18 = (unsigned short)cached[18];
    output->transformWorkspaceCount = cached[kProviderWorkspaceCount];
    output->splitPacketHeaderWordCount = cached[kProviderPacketHeaderWords];
    return 1;
}

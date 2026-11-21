// Translation unit 0x803F96CC-0x803F97FC (3 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".


extern "C" {

enum FilterParameterLayout { kFilterParameterWords = 24, kFilterParameterBytes = 0x60 };
enum ProviderFormatWords {
    kProviderSampleBufferBytes = 2, kProviderWorkspaceCount = 3,
    kProviderShowCounterParameter = 4, kProviderInitialReadBytes = 10,
    kProviderPacketHeaderWords = 12
};
// Word numbers below identify the provider's opaque 0x60-byte input schema.
// This maps a provider block to the pipeline's GtlMediaFormat, rather than
// implementing a DSP coefficient formula. See classes/GtlMediaFormat.h and
// gtl_CFilterManager_CopyFormat in gtl_media_pipeline_filters.cpp for the same output byte map.
// Provider word -> output meaning (legacy parameterWord fields retained):
// 4 -> show-counter format parameter (units unresolved), 10 -> initial read
// offset (bytes), 2 -> sample buffer size (bytes), 3 -> transform workspace
// count, 12 -> split-packet header word count. Words 6/17/18 remain opaque.
// No frequency/gain interpretation is established for any of these words.
// To add support for a parameter, update both the cached-word projection and its
// output field; changing the cache alone does not change the output descriptor.
// Words 4/17/18 need special care: word 4 is a float BIT COPY (not numeric
// conversion), while words 17/18 intentionally discard their high 16 bits.
// Output +0 is cleared on every application; the format discriminator's code
// meaning is unresolved. This is not a pointer or a reset of the whole manager.
// Do not turn this into a whole-output clear: +4..+0x87 retain caller state.

void gtl_Free(void*);

// Free a filter-manager allocation if supplied.
void dtor_803F96CC(void* self) {
    if (self != 0) {
        gtl_Free(self);
    }
}

// Report the 24-word input parameter block size in bytes.
// size must be non-null; returns 1 and does not inspect self.
int CRevolutionFilterManager_GetParameterBlockSize(void* self, int* size) {
    *size = kFilterParameterBytes;
    return 1;
}

struct RevolutionFilterManagerData {
    char baseState[0x90];
    unsigned int cachedParameters[kFilterParameterWords];
};

struct RevolutionFilterOutput {
    unsigned int baseState;
    char reservedState[0x84];
    float showCounterFormatParameter;
    unsigned int initialReadOffset;
    unsigned int sampleBufferSize;
    unsigned int parameterWord6;
    unsigned short parameterWord17;
    unsigned short parameterWord18;
    unsigned int transformWorkspaceCount;
    unsigned int splitPacketHeaderWordCount;
};
// Output byte map: +0 is reset, +4..+0x87 are left untouched;
// +0x88 float bits from word 4, +0x8C word 10, +0x90 word 2, +0x94 word 6,
// +0x98/+0x9A low 16 bits of words 17/18, +0x9C word 3, +0xA0 word 12.

// Cache all 24 input words and copy the supported parameters into the output descriptor.
// All pointers are borrowed and required; input covers 0x60 bytes, output 0xA4.
// Returns 1 unconditionally. Only the listed output fields are written.
int gtl_CRevolutionFilterManager_BuildDescriptor(RevolutionFilterManagerData* self, unsigned int* input, RevolutionFilterOutput* output) {
    unsigned int* cached = self->cachedParameters;
    unsigned int firstWord, secondWord;
    // Phase 1: cache the input. Preserve paired load/store order, including
    // pairs loaded in reverse; this sequence also fixes behavior for aliasing input.
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

// The 24 words are the THP file header read from the disc (big-endian).
    // Phase 2: project the cached schema into the descriptor, preserving other bytes.
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

} // extern "C"

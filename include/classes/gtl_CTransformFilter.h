#ifndef CLASSES_GTL_CTRANSFORMFILTER_H
#define CLASSES_GTL_CTRANSFORMFILTER_H

#include "game/types.h"

namespace gtl {

// Filter that transforms an input sample into a borrowed output sample.
// The base filter occupies the first 4 bytes, so the vtable pointer sits at +4.
class CTransformFilter {
public:
    // Setup and teardown.
    CTransformFilter* Init(void* arg1, void* outputSample);
    void* Destroy(int deleting);

    // Processing and state.
    int Process();
    int ResumeOrExposeOutput();
    int SetRunning();
    int Reset();
    int ResetAndStart();

    // Samples and formats.
    int GetOutputSample(void* unused, unsigned int** output);
    int SetInputSample(unsigned int* value);
    int FormatsMatch(void* left, void* right);

    unsigned int unk_0;           // 0x0
    void* vtable;                 // 0x4
    char pad_008[0x8];            // 0x8
    int* inputSample;             // 0x10, retained reference
    unsigned int* outputSample;   // 0x14, borrowed
    unsigned char formatsChecked; // 0x18
};

#define GTL_CTRANSFORMFILTER_OFF(f, o) \
    typedef char gtl_CTransformFilter_off_##f[((int)&((CTransformFilter*)0)->f == (o)) ? 1 : -1]
GTL_CTRANSFORMFILTER_OFF(vtable, 0x4);
GTL_CTRANSFORMFILTER_OFF(inputSample, 0x10);
GTL_CTRANSFORMFILTER_OFF(outputSample, 0x14);
GTL_CTRANSFORMFILTER_OFF(formatsChecked, 0x18);
GTL_CTRANSFORMFILTER_OFF(unk_0, 0x0);
#undef GTL_CTRANSFORMFILTER_OFF

} // namespace gtl

#endif // CLASSES_GTL_CTRANSFORMFILTER_H

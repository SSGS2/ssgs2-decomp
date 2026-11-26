#ifndef CLASSES_GTL_CSPLITFILTER_H
#define CLASSES_GTL_CSPLITFILTER_H

#include "game/types.h"

struct RefObject;
struct CallbackObject;

namespace gtl {

// Splits one input sample into two output samples.
class CSplitFilter {
public:
    // Setup and teardown.
    // Initialize the base filter, the vtable and both output sample wrappers, which borrow input. arg2 is unused.
    CSplitFilter* Init(void* input, void* arg2);
    // Release the input and output references, destroy the base filter, and optionally free the object.
    void* Destroy(int deleting);

    // Processing.
    // Validate input/output formats once, split the input, and publish success or the failure state.
    int Process();
    // Compare the formats of two samples; reached through the filter vtable.
    int FormatsMatch(void* left, void* right);
    // Dispatch the input-reset and state-reset hooks of the given callback object in that order.
    static int ResetAndStart(CallbackObject* self);

    // Filter state.
    // Set the split filter to running.
    int SetRunning();
    // Stop the split filter.
    int Stop();
    // Release the input sample and reset the filter.
    int Reset();

    // Samples.
    // Add a reference to output zero or one; other selectors leave the output untouched.
    int GetOutputSample(int which, unsigned int** out);
    // Release the old input reference and retain the replacement sample.
    int SetInputSample(RefObject* value);

    unsigned int unk_0;           // 0x0
    void* vtable;                 // 0x4
    char pad_008[0x8];            // 0x8
    void* inputSample;            // 0x10
    void* primaryOutput;          // 0x14
    void* secondaryOutput;        // 0x18
    unsigned char formatsChecked; // 0x1c
};

} // namespace gtl

#define GTL_CSPLITFILTER_OFF(f, o) \
    typedef char gtl_CSplitFilter_off_##f[((int)&((gtl::CSplitFilter*)0)->f == (o)) ? 1 : -1]
GTL_CSPLITFILTER_OFF(vtable, 0x4);
GTL_CSPLITFILTER_OFF(inputSample, 0x10);
GTL_CSPLITFILTER_OFF(primaryOutput, 0x14);
GTL_CSPLITFILTER_OFF(secondaryOutput, 0x18);
GTL_CSPLITFILTER_OFF(formatsChecked, 0x1c);
GTL_CSPLITFILTER_OFF(unk_0, 0x0);
#undef GTL_CSPLITFILTER_OFF

#endif // CLASSES_GTL_CSPLITFILTER_H

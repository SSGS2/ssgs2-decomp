#ifndef CLASSES_GTL_CSOURCEFILTER_H
#define CLASSES_GTL_CSOURCEFILTER_H

#include "game/types.h"

struct SourceData;

namespace gtl {

// Reads a source into pool samples, one block per Process call.
class CSourceFilter {
public:
    // Setup and teardown.
    // Initialize the filter and start reading at the format's initial byte offset.
    void* Init(const SourceData* sourceData, void* callbackObject);
    // Destroy the base filter and optionally free the object.
    void* Destroy(int deleting);

    // Processing.
    // Read into an available pool sample, advance the byte offset, and preserve wait/error states.
    int Process();
    // Start processing unless the source has already reached output-ready state.
    int StartProcessing();
    // Resume processing unless the source has already reached output-ready state.
    int ResumeUnlessOutputReady();
    // Clear the pending read, restore the initial read offset, and reset the filter state.
    int ResetReadState();
    // Dispatch the input-reset and state-reset hooks in that order.
    int ResetAndStart();

    // Samples.
    // Return an added reference to the output sample when an output pointer is supplied.
    int GetOutputSample(void* unused, unsigned int** out);
    // Store the input sample and cache its data pointer.
    int StoreInputSample(void* input);
    // Accept a nonzero source-operation argument.
    int vfunc_8(int value);

    // Read position and pending-read latch.
    // Write the current read offset when an output pointer is supplied.
    int GetReadOffset(unsigned int* out);
    // Return the pending-read flag.
    int IsReadPending();
    // Latch a pending read.
    void MarkReadPending();
    // Clear the pending-read latch.
    void ClearReadPending();

    unsigned int unk_0;            // 0x0
    void* vtable;                  // 0x4
    char pad_008[0x8];             // 0x8
    void* outputSample;            // 0x10
    void* samplePool;              // 0x14
    unsigned char readPending;     // 0x18
    char pad_019[0x3];             // 0x19
    void* callback;                // 0x1c
    void* source;                  // 0x20
    unsigned int readOffset;       // 0x24
    unsigned char sourceValidated; // 0x28
    char pad_029[0x3];             // 0x29
};

} // namespace gtl

#define GTL_CSOURCEFILTER_OFF(f, o) \
    typedef char gtl_CSourceFilter_off_##f[((int)&((gtl::CSourceFilter*)0)->f == (o)) ? 1 : -1]
GTL_CSOURCEFILTER_OFF(vtable, 0x4);
GTL_CSOURCEFILTER_OFF(outputSample, 0x10);
GTL_CSOURCEFILTER_OFF(samplePool, 0x14);
GTL_CSOURCEFILTER_OFF(readPending, 0x18);
GTL_CSOURCEFILTER_OFF(callback, 0x1c);
GTL_CSOURCEFILTER_OFF(source, 0x20);
GTL_CSOURCEFILTER_OFF(readOffset, 0x24);
GTL_CSOURCEFILTER_OFF(sourceValidated, 0x28);
GTL_CSOURCEFILTER_OFF(unk_0, 0x0);
#undef GTL_CSOURCEFILTER_OFF
typedef char gtl_CSourceFilter_size[(sizeof(gtl::CSourceFilter) == 0x2C) ? 1 : -1];

#endif // CLASSES_GTL_CSOURCEFILTER_H

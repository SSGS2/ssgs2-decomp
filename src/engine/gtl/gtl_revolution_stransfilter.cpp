// Revolution S-transform filter: its Init around an output sample, its copied media format,
// and its Destroy, which frees the shared allocation once.

#include "classes/GtlMediaSample.h"
#include "classes/GtlMediaFormat.h"
#include "classes/GtlRevolutionSTransFilter.h"

extern "C" {

void gtl_Free(void*);
extern char __vt__Q23gtl23CRevolutionSTransFilter[];

struct CopiedMediaFormat {
    GtlMediaFormat format;
    char codecTail[8];
};

enum AudioTransformLayout {
    kAudioTransformFilterKind = 4,
    kWorkspaceBytesPerCount = 4,
    kWorkspaceHeaderBytes = 0x20
};

} // extern "C"

// Initialize the THP audio filter and compute the size published for each PCM segment.
// outputSample is stored by the base without acquiring a reference: the caller
// must keep it alive through filter use. Allocates nothing.
gtl::CRevolutionSTransFilter* gtl::CRevolutionSTransFilter::Init(void* outputSample) {
    // Construct the base and clear the segment state.
    CTransformFilter::Init((void*)kAudioTransformFilterKind, outputSample);
    segmentBytes = 0;
    vtable = &__vt__Q23gtl23CRevolutionSTransFilter;
    allocation = 0;
    currentBuffer = 0;
    buffers[0] = 0;
    buffers[1] = 0;
    // Derive the segment size from the output format's workspace count.
    volatile CopiedMediaFormat format;
    ((CMediaSample*)outputSample)->CopyFormat((void*)&format);
    unsigned int count = format.format.transformWorkspaceCount;
    if (count != 0)
        segmentBytes = count * kWorkspaceBytesPerCount + kWorkspaceHeaderBytes;
    return this;
}

// Release the shared segment allocation, destroy the base, and optionally free the object.
// Both segments share allocation; free it once. deleting > 0 frees this too.
void* gtl::CRevolutionSTransFilter::Destroy(int deleting) {
    if (this != 0) {
        void* allocationToFree = allocation;
        vtable = &__vt__Q23gtl23CRevolutionSTransFilter;
        if (allocationToFree != 0) {
            gtl_Free(allocationToFree);
            allocation = 0;
        }
        CTransformFilter::Destroy(0);
        if (deleting > 0 && this != 0)
            gtl_Free(this);
    }
    return this;
}

// Revolution V-transform filter, part 1: its Init around an output sample, and its Destroy.

#include "classes/GtlMediaSample.h"
#include "classes/GtlRevolutionVTransFilter.h"

extern "C" {
#include "sdk_protos.h"
void gtl_Free(void*);
extern void* __vt__Q23gtl23CRevolutionVTransFilter[];

// Format +0x98/+0x9A are unsigned pixel dimensions; CopyFormat writes 0xA4
// bytes into the original 0xA8-byte stack slot. Filter kind 3 is the value used
// by this THP video transform; other filter-kind values are outside this unit.
enum {
    kVideoTransformKind = 3,
    kFormatStackBytes = 0xA8,
    kFormatWidthOffset = 0x98,
    kFormatHeightOffset = 0x9A,
    kFrameHeaderBytes = 0x20
};

} // extern "C"

// Initialize a THP YUV420 transform using a borrowed output sample; return this.
// The base stores outputSample without taking a reference; this unit does not release it.
// Valid format storage is required.
gtl::CRevolutionVTransFilter* gtl::CRevolutionVTransFilter::Init(void* outputSample) {
    // Phase 1: initialize the base and clear all plane aliases and owned buffers.
    CTransformFilter::Init((void*)kVideoTransformKind, outputSample);
    yPlane = 0;
    vtable = __vt__Q23gtl23CRevolutionVTransFilter;
    uPlane = 0;
    vPlane = 0;
    decoderWorkBuffer = 0;
    frameBuffer = 0;
    // Phase 2: set full-resolution luma and half-width/half-height chroma planes.
    // Odd dimensions truncate on the shift. Plane storage is allocated later.
    unsigned char formatDescription[kFormatStackBytes];
    ((CMediaSample*)outputSample)->CopyFormat(formatDescription);
    frameHeaderSize = kFrameHeaderBytes;
    yPlaneWidth = *(unsigned short*)(formatDescription + kFormatWidthOffset);
    yPlaneHeight = *(unsigned short*)(formatDescription + kFormatHeightOffset);
    uPlaneWidth = *(unsigned short*)(formatDescription + kFormatWidthOffset) >> 1;
    uPlaneHeight = *(unsigned short*)(formatDescription + kFormatHeightOffset) >> 1;
    vPlaneWidth = *(unsigned short*)(formatDescription + kFormatWidthOffset) >> 1;
    vPlaneHeight = *(unsigned short*)(formatDescription + kFormatHeightOffset) >> 1;
    return this;
}

// Release decoder work and frame storage, destroy the base, optionally free this.
// Plane pointers alias the frame buffer and must not be freed individually.
// deleting > 0 frees this; the returned address may consequently be dangling.
void* gtl::CRevolutionVTransFilter::Destroy(int deleting) {
    if (this != 0) {
        vtable = __vt__Q23gtl23CRevolutionVTransFilter;
        // Phase 1: dispose the two allocations owned by AllocateFrameBuffers.
        void* ownedBuffer = decoderWorkBuffer;
        if (ownedBuffer != 0) {
            gtl_Free(ownedBuffer);
            decoderWorkBuffer = 0;
        }
        ownedBuffer = frameBuffer;
        if (ownedBuffer != 0) {
            gtl_Free(ownedBuffer);
            frameBuffer = 0;
        }
        // Phase 2: destroy the base without deleting, then honor the outer flag.
        CTransformFilter::Destroy(0);
        if (deleting > 0 && this != 0)
            gtl_Free(this);
    }
    return this;
}

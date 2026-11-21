#include "classes/GtlRevolutionVTransFilter.h"
// View existing filter storage without constructing or owning another object.
static inline GtlRevolutionVTransFilter* AsGtlRevolutionVTransFilter(void* object) {
    return (GtlRevolutionVTransFilter*)object;
}
// Translation unit 0x803FA01C-0x803FA17C (2 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

extern "C" {
#include "sdk_protos.h"
void gtl_CTransformFilter_Ctor(void*, int, int);
int gtl_CMediaSample_CopyFormat(void*, void*);
void gtl_Free(void*);
void* gtl_CTransformFilter_dtor(void*, int);
extern void* __vt__Q23gtl23CRevolutionVTransFilter[];

// Format +0x98/+0x9A are unsigned pixel dimensions; CopyFormat writes 0xA4
// bytes into the original 0xA8-byte stack slot. Filter kind 3 is the value used
// by this THP video transform; other filter-kind values are outside this unit.
enum { kVideoTransformKind = 3, kFormatStackBytes = 0xA8,
       kFormatWidthOffset = 0x98, kFormatHeightOffset = 0x9A,
       kFrameHeaderBytes = 0x20 };

// Initialize a THP YUV420 transform using a borrowed output sample; return self.
// outputSample holds a pointer in a 32-bit ABI integer. The base stores it;
// this unit does not release the sample. Valid format/self storage is required.
void* gtl_CRevolutionVTransFilter_Ctor(void* self, int outputSample) {
    // Phase 1: initialize the base and clear all plane aliases and owned buffers.
    gtl_CTransformFilter_Ctor(self, kVideoTransformKind, outputSample);
    AsGtlRevolutionVTransFilter(self)->yPlane = 0;
    AsGtlRevolutionVTransFilter(self)->vtbl = __vt__Q23gtl23CRevolutionVTransFilter;
    AsGtlRevolutionVTransFilter(self)->uPlane = 0;
    AsGtlRevolutionVTransFilter(self)->vPlane = 0;
    AsGtlRevolutionVTransFilter(self)->decoderWorkBuffer = 0; // 0x803FA01C: +0x48
    AsGtlRevolutionVTransFilter(self)->frameBuffer = 0; // 0x803FA01C: +0x4C
    // Phase 2: set full-resolution luma and half-width/half-height chroma planes.
    // Odd dimensions truncate on the shift. Plane storage is allocated by vfunc_9.
    unsigned char formatDescription[kFormatStackBytes];
    gtl_CMediaSample_CopyFormat((void*)outputSample, formatDescription);
    AsGtlRevolutionVTransFilter(self)->frameHeaderSize = kFrameHeaderBytes;
    AsGtlRevolutionVTransFilter(self)->yPlaneWidth = *(unsigned short*)(formatDescription + kFormatWidthOffset);
    AsGtlRevolutionVTransFilter(self)->yPlaneHeight = *(unsigned short*)(formatDescription + kFormatHeightOffset);
    AsGtlRevolutionVTransFilter(self)->uPlaneWidth = *(unsigned short*)(formatDescription + kFormatWidthOffset) >> 1;
    AsGtlRevolutionVTransFilter(self)->uPlaneHeight = *(unsigned short*)(formatDescription + kFormatHeightOffset) >> 1;
    AsGtlRevolutionVTransFilter(self)->vPlaneWidth = *(unsigned short*)(formatDescription + kFormatWidthOffset) >> 1;
    AsGtlRevolutionVTransFilter(self)->vPlaneHeight = *(unsigned short*)(formatDescription + kFormatHeightOffset) >> 1;
    return self;
}

// Release decoder work and frame storage, destroy the base, optionally free self.
// Plane pointers alias the frame buffer and must not be freed individually.
// deleting > 0 frees self; the returned address may consequently be dangling.
void* gtl_CRevolutionVTransFilter_dtor(void* self, int deleting) {
    if (self != 0) {
        AsGtlRevolutionVTransFilter(self)->vtbl = __vt__Q23gtl23CRevolutionVTransFilter;
        // Phase 1: dispose the two allocations owned by vfunc_9 (+0x48/+0x4C).
        void* ownedBuffer = AsGtlRevolutionVTransFilter(self)->decoderWorkBuffer;
        if (ownedBuffer != 0) {
            gtl_Free(ownedBuffer);
            AsGtlRevolutionVTransFilter(self)->decoderWorkBuffer = 0;
        }
        ownedBuffer = AsGtlRevolutionVTransFilter(self)->frameBuffer;
        if (ownedBuffer != 0) {
            gtl_Free(ownedBuffer);
            AsGtlRevolutionVTransFilter(self)->frameBuffer = 0;
        }
        // Phase 2: destroy the base without deleting, then honor the outer flag.
        gtl_CTransformFilter_dtor(self, 0);
        if (deleting > 0 && self != 0)
            gtl_Free(self);
    }
    return self;
}

} // extern "C"

// Translation unit 0x803F9DC0-0x803F9ED8 (2 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

#include "classes/gtl_CTransformFilter.h"
#include "classes/GtlMediaFormat.h"

extern "C" {

void gtl_CTransformFilter_Ctor(void*, int, void*);
void gtl_CMediaSample_CopyFormat(void*, void*);
void gtl_Free(void*);
void gtl_CTransformFilter_dtor(void*, int);
extern char __vt__Q23gtl23CRevolutionSTransFilter[];

struct CRevolutionSTransFilter {
    gtl_CTransformFilter base;
    void* outputSegmentBytes; // Published audio output byte count; pointer-sized raw storage.
    void* ownedBuffer; // Owns one allocation containing both alternating PCM buffers.
    void* unresolvedState24; // Only zero initialization is known in this unit.
    void* unresolvedState28;
    unsigned char unresolvedState2C;
};

struct CopiedMediaFormat {
    GtlMediaFormat format;
    char codecTail[8];
};

enum AudioTransformLayout {
    kAudioTransformFilterKind = 4,
    kWorkspaceBytesPerCount = 4,
    kWorkspaceHeaderBytes = 0x20
};

// Initialize the THP audio filter and compute the size published for each PCM buffer.
// outputSample is stored by the base without acquiring a reference: the caller
// must keep it alive through filter use. self is caller storage and is returned.
// This constructor copies the sample's format and computes a size, allocating nothing.
void* gtl_CRevolutionSTransFilter_Ctor(CRevolutionSTransFilter* self, void* outputSample) {
    // Phase 1: construct the base and clear owned/alternating-buffer state.
    gtl_CTransformFilter_Ctor(self, kAudioTransformFilterKind, outputSample);
    self->outputSegmentBytes = 0;
    self->base.vtable = &__vt__Q23gtl23CRevolutionSTransFilter;
    self->ownedBuffer = 0;
    self->unresolvedState2C = 0;
    self->unresolvedState24 = 0;
    self->unresolvedState28 = 0;
    // Phase 2: derive the output size. revolution_strans_audio_decode.cpp passes the +0x1C word to
    // CMediaSample_SetBuffer as its byte size; the pointer type here is a raw view.
    volatile CopiedMediaFormat format;
    gtl_CMediaSample_CopyFormat(outputSample, (void*)&format);
    unsigned int count = format.format.transformWorkspaceCount;
    if (count != 0)
        self->outputSegmentBytes = (void*)(count * kWorkspaceBytesPerCount + kWorkspaceHeaderBytes);
    return self;
}

// Release the shared audio-buffer allocation, destroy the base, and optionally free the object.
// Both alternating audio buffers share ownedBuffer; free the allocation once.
// Null self is accepted. deleting > 0 frees self too; return may be a freed address.
void* gtl_CRevolutionSTransFilter_dtor(CRevolutionSTransFilter* self, int deleting) {
    if (self != 0) {
        void* allocationToFree = self->ownedBuffer;
        self->base.vtable = &__vt__Q23gtl23CRevolutionSTransFilter;
        if (allocationToFree != 0) {
            gtl_Free(allocationToFree);
            self->ownedBuffer = 0;
        }
        gtl_CTransformFilter_dtor(self, 0);
        if (deleting > 0 && self != 0)
            gtl_Free(self);
    }
    return self;
}

} // extern "C"

#include "classes/GtlSamplingFilter.h"

// Translation unit 0x803F985C-0x803F98EC (2 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

extern "C" {

struct CRevolutionSamplingFilter {
    GtlSamplingFilter base;
    int initialRequestBytes; // +0x34: reset seed, copied from the format/configuration +0x94.
    int requestedBytes;      // +0x38: bytes to assemble for the next output sample.
    int completedBytes;      // +0x3C: size of the last fully assembled output sample.
};

// Only the seed word of the owner configuration is consumed here.
struct SamplingOwnerConfig {
    char padBeforeSeed[0x94];
    int initialRequestBytes;
};

extern void gtl_CSamplingFilter_Ctor(void*, void*);
extern void gtl_CSamplingFilter_ResetQueue(void*);
extern void __vt__Q23gtl25CRevolutionSamplingFilter();

// Construct the base queue and seed both Revolution byte counts from the owner.
// self is caller-provided storage; owner is required and is passed to the base
// constructor as its format/configuration. Its +0x94 word must describe a byte count.
// No allocation is performed by this derived portion; returns self.
CRevolutionSamplingFilter* gtl_CRevolutionSamplingFilter_Ctor(CRevolutionSamplingFilter* self, void* owner) {
    gtl_CSamplingFilter_Ctor(self, owner);
    int value = ((SamplingOwnerConfig*)owner)->initialRequestBytes;
    self->base.vtable = (void*)&__vt__Q23gtl25CRevolutionSamplingFilter;
    self->initialRequestBytes = value;
    self->requestedBytes = value;
    self->completedBytes = 0;
    return self;
}

// Restore the initial request size, clear the completed count, and reset the base queue.
// Returns 1 unconditionally. Queue cleanup/ownership is delegated to ResetQueue;
// completedBytes is a completed sample size, not an in-progress copy cursor.
int gtl_CRevolutionSamplingFilter_Reset(CRevolutionSamplingFilter* self) {
    self->requestedBytes = self->initialRequestBytes;
    self->completedBytes = 0;
    gtl_CSamplingFilter_ResetQueue(self);
    return 1;
}

} // extern "C"

// Translation unit 0x8040B210-0x8040B260 (1 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

#include "classes/FxReverbStdWork.h"

enum { kReverbChannelCount = 3 };

extern "C" {

struct ReverbBuffers {
    // Three borrowed sample-plane pointers; channel order is not established here.
    void* samples[kReverbChannelCount];
};

struct FxReverbStd {
    char baseEffect[0xc];
    unsigned char initializationAttempted; // +0x0C: initialization-attempt latch (legacy name), not a mixer-enable bit.
    char parameters[0x1f];
    // SDK work starts here; the complete shared view is FxReverbStdWork.
    char reverb[1];
};

void ReverbStd_ProcessAuxBuffers(ReverbBuffers*, void*);

// Process three auxiliary sample planes in place when the initialization latch is set.
// self/buffers must be valid; the second ABI argument is ignored. The SDK receives
// a stack copy of the pointer triple, retaining caller ownership of sample storage.
void gtl_nw4r_FxReverbStd_ProcessAuxBuffers(FxReverbStd* self, int, ReverbBuffers* buffers) {
    if (self->initializationAttempted) {
        ReverbBuffers samplePlanes;
        samplePlanes.samples[0] = buffers->samples[0];
        samplePlanes.samples[1] = buffers->samples[1];
        samplePlanes.samples[2] = buffers->samples[2];
        ReverbStd_ProcessAuxBuffers(&samplePlanes, ((FxReverbStdWork*)self)->sdkWork);
    }
}

} // extern "C"

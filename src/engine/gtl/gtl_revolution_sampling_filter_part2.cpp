// Translation unit 0x803F9A2C-0x803F9A3C (1 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

#include "classes/RevolutionSamplingState.h"
extern "C" {

// Write the next output sample's requested byte count and return success (1).
// self is a borrowed initialized filter; outRequestedBytes must point to writable
// u32 storage. Neither pointer is checked or retained, and no queue state changes.
// +0x38 is seeded/reset by gtl_revolution_sampling_filter_part1.cpp, then replaced with the completed sample's
// first u32 by revolution_sampling_filter_fill.cpp. This is the next target size, not bytes already queued.
int gtl_CRevolutionSamplingFilter_GetOutputBytes(void* self, unsigned int* outRequestedBytes) {
    *outRequestedBytes = AsRevolutionSamplingState(self)->requestedOutputBytes;
    return 1;
}

} // extern "C"

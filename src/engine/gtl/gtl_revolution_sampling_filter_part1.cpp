// Revolution sampling filter, part 1: setting the filter up for its owner and resetting its state.
// The owner config struct holds the initial request size read from the owner.

#include "classes/gtl_CRevolutionSamplingFilter.h"

extern "C" {

// Only the seed word of the owner configuration is consumed here.
struct SamplingOwnerConfig {
    char padBeforeSeed[0x94];
    int initialRequestBytes;
};

extern void __vt__Q23gtl25CRevolutionSamplingFilter();

} // extern "C"

// Construct the base queue and seed both byte counts from the owner. owner is passed to the base
// constructor as its format/configuration; its +0x94 word must describe a byte count.
// No allocation is performed here.
gtl::CRevolutionSamplingFilter* gtl::CRevolutionSamplingFilter::Init(void* owner) {
    CSamplingFilter::Init(owner);
    int value = ((SamplingOwnerConfig*)owner)->initialRequestBytes;
    vtable = (void*)&__vt__Q23gtl25CRevolutionSamplingFilter;
    initialRequestBytes = value;
    requestedBytes = value;
    completedBytes = 0;
    return this;
}

// Restore the seed request size, clear the completed count, and reset the base queue.
// completedBytes is the size of the last completed sample, not an in-progress copy cursor.
int gtl::CRevolutionSamplingFilter::Reset() {
    requestedBytes = initialRequestBytes;
    completedBytes = 0;
    ResetQueue();
    return 1;
}

// Translation unit 0x803F9144-0x803F9218 (1 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

#include "sdk_protos.h"
#include "classes/GtlShowCounter.h"

namespace gtl {
struct CRevolutionShowCounter {
    GtlShowCounter counter;
    unsigned long ticksPerHundredSeconds; // +0x14: display ticks per 100 seconds (5994 NTSC / 5000 PAL).
};
}

extern "C" {
extern unsigned long s_revolutionShowCounterTicks;
extern float kFloat_100_ShowCounterTimeScale;
void gtl_IShowCounter_GetCount(gtl::CRevolutionShowCounter*, unsigned long*);
void gtl_IShowCounter_SetPreviousCount(gtl::CRevolutionShowCounter*, unsigned long);
void gtl_IShowCounter_GetState(gtl::CRevolutionShowCounter*, int*);
void gtl_IShowCounter_SetCount(gtl::CRevolutionShowCounter*, unsigned long);

// Save the previous count, then convert elapsed time to scaled whole counter periods.
// Division occurs before scaling: preserve that truncation when changing the rate.
// self/period must be valid and nonzero. elapsed's unit is not established here;
// the exact formula is scale * (unsigned(100*elapsed) * displayTicks / period).
// Unsigned multiplication can wrap. This replaces count rather than adding to it.
// Always returns 1, including when paused/reset-armed; previousCount is still saved.
int gtl_CRevolutionShowCounter_UpdateCount(gtl::CRevolutionShowCounter* self, float elapsed, float scale) {
    unsigned long count;
    int state;
    gtl_IShowCounter_GetCount(self, &count);
    gtl_IShowCounter_SetPreviousCount(self, count);
    gtl_IShowCounter_GetState(self, &state);
    if (state) return 1; // State 0 counts; both 1 (paused) and 2 (reset armed) skip.
    unsigned long ticks = Runtime_ConvertDoubleToUnsigned(kFloat_100_ShowCounterTimeScale * elapsed);
    ticks *= s_revolutionShowCounterTicks;
    count = Runtime_ConvertDoubleToUnsigned(scale * (float)(ticks / self->ticksPerHundredSeconds));
    gtl_IShowCounter_SetCount(self, count);
    return 1;
}

} // extern "C"

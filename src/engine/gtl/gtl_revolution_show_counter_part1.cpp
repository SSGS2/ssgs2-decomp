// Revolution show counter, part 1: updating the count from elapsed time and a scale factor.

#include "sdk_protos.h"
#include "classes/gtl_CRevolutionShowCounter.h"

extern "C" {
extern unsigned long s_revolutionShowCounterTicks;
extern float kFloat_100_ShowCounterTimeScale;
} // extern "C"

// Save the previous count, then replace the count with the scaled number of whole counter periods
// elapsed. Division by ticksPerHundredSeconds happens before scaling, which keeps that truncation.
// Unsigned multiplication can wrap. Always returns 1, including when paused or reset-armed;
// the previous count is saved in every case.
int gtl::CRevolutionShowCounter::UpdateCount(float elapsed, float scale) {
    unsigned long count;
    int state;
    GetCount((int*)&count);
    SetPreviousCount(count);
    GetState((unsigned int*)&state);
    if (state)
        return 1; // State 0 counts; both 1 (paused) and 2 (reset armed) skip.
    unsigned long ticks = Runtime_ConvertDoubleToUnsigned(kFloat_100_ShowCounterTimeScale * elapsed);
    ticks *= s_revolutionShowCounterTicks;
    count = Runtime_ConvertDoubleToUnsigned(scale * (float)(ticks / ticksPerHundredSeconds));
    SetCount(count);
    return 1;
}

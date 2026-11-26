// gtl::CRevolutionShowCounter: the Revolution display counter. It is the show-counter base (gtl::IShowCounter,
// 0x14 bytes) followed by the display ticks per 100 seconds. Methods are in gtl_transform_filter_show_counter.cpp,
// gtl_revolution_show_counter_part1.cpp and gtl_revolution_show_counter_part2.cpp. No virtual functions are
// declared here; the vtable comes from the original data.
#ifndef CLASSES_GTL_CREVOLUTIONSHOWCOUNTER_H
#define CLASSES_GTL_CREVOLUTIONSHOWCOUNTER_H

#include "classes/GtlShowCounter.h"

namespace gtl {

class CRevolutionShowCounter : public IShowCounter {
public:
    // Initialize the base counter, set the vtable and select the NTSC (59.94 Hz) or PAL (50 Hz)
    // display timing from the runtime mode. Returns this.
    void* Init();
    // Start display counting. Returns 0 if already counting, leaving ticks and the callback unchanged.
    // Otherwise clears the tick count and registers the display-tick callback. Returns 1.
    int StartDisplayCounting();
    // Count one invocation of the registered display callback.
    static void Tick();
    // Replace the count with the scaled number of elapsed display periods. Always returns 1.
    int UpdateCount(float elapsed, float scale);
    // Free this object when deletingFlag is positive. Returns this.
    void* Destroy(int deletingFlag);

    unsigned long ticksPerHundredSeconds; // 0x14: display ticks per 100 seconds (5994 NTSC, 5000 PAL)
};

} // namespace gtl

#define GTL_CREVOLUTIONSHOWCOUNTER_OFF(f, o) \
    typedef char gtl_CRevolutionShowCounter_off_##f[((int)&((gtl::CRevolutionShowCounter*)0)->f == (o)) ? 1 : -1]
GTL_CREVOLUTIONSHOWCOUNTER_OFF(ticksPerHundredSeconds, 0x14);
#undef GTL_CREVOLUTIONSHOWCOUNTER_OFF

#endif

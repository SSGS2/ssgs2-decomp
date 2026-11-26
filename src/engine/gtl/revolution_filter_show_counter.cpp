// Revolution display counter: creates the manager's show counter the first time it is needed.
//
// EnsureShowCounter allocates the counter once and keeps it in the manager. Creating the counter
// does not start counting.

#include "classes/gtl_CRevolutionFilterManager.h"
#include "classes/gtl_CRevolutionShowCounter.h"

typedef unsigned long new_size_t;
// Revolution media display-counter creation (video presentation synchronization).
// API: manager slot 25 creates once; gtl::CFilterManager::Update (gtl_media_pipeline_filter_manager.cpp)
// consumes current/previous counts to suspend/start filters. No gameplay knobs.
// Counter timing: gtl::CRevolutionShowCounter::Init (gtl_transform_filter_show_counter.cpp) selects NTSC/PAL;
// StartDisplayCounting installs the tick callback; update is in gtl_revolution_show_counter_part1.cpp.

extern "C" void* gtl_Alloc(unsigned long);

// Allocate object storage through the gtl heap; construction follows separately.
inline void* operator new(new_size_t size) {
    return gtl_Alloc(size);
}
#define GTL_NEW new

struct RevolutionShowCounterStorage {
    char data[0x18];
    // Construct the display counter in this 0x18-byte layout view.
    RevolutionShowCounterStorage() {
        ((gtl::CRevolutionShowCounter*)this)->Init();
    }
};

// Lazily allocate the manager's display counter; repeated calls retain the existing counter.
// The manager retains the allocation at +0x4c. Return 1 unconditionally; this
// function provides no allocation-failure status.
// The constructor establishes refcount 1; that reference transfers to the manager.
// gtl::CFilterManager::ReleaseShowCounter (gtl_media_pipeline_filter_manager.cpp, cleanup slot 26) decrements
// it and clears +0x4C. At zero it calls counter destroy(1), whose implementation
// in gtl_revolution_show_counter_part2.cpp frees via gtl_Free. Use that release path, not plain delete/free.
// Creation itself neither starts display counting nor registers a callback.
int gtl::CRevolutionFilterManager::EnsureShowCounter() {
    if (!showCounter) {
        showCounter = GTL_NEW RevolutionShowCounterStorage;
    }
    return 1;
}

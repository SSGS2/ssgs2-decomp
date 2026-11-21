typedef unsigned long new_size_t;
// Revolution media display-counter creation (video presentation synchronization).
// API: manager slot 25 creates once; gtl_CFilterManager_Update (gtl_media_pipeline_filter_manager.cpp)
// consumes current/previous counts to suspend/start filters. No gameplay knobs.
// Counter timing: CRevolutionShowCounter_Ctor (gtl_transform_filter_show_counter.cpp) selects NTSC/PAL;
// StartDisplayCounting installs the tick callback; update is in gtl_revolution_show_counter_part1.cpp.
// Translation unit 0x803F9670-0x803F96CC (1 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

extern "C" void* gtl_Alloc(unsigned long);
extern "C" void gtl_CRevolutionShowCounter_Ctor(void*);

// Allocate object storage through the gtl heap; construction follows separately.
inline void* operator new(new_size_t size) { return gtl_Alloc(size); }
#define GTL_NEW new

struct RevolutionShowCounterStorage {
    char data[0x18];
    // Construct the display counter in this 0x18-byte layout view.
    RevolutionShowCounterStorage() { gtl_CRevolutionShowCounter_Ctor(this); }
};

namespace gtl {
struct CRevolutionFilterManager {
    char managerStorage[0x4c]; // Existing manager state precedes the optional counter.
    RevolutionShowCounterStorage* showCounter;
};
}

// Lazily allocate the manager's display counter; repeated calls retain the existing counter.
// The manager retains the allocation at +0x4c. Return 1 unconditionally; this
// function provides no allocation-failure status. self must be a valid manager.
// The constructor establishes refcount 1; that reference transfers to the manager.
// gtl_CFilterManager_ReleaseShowCounter (gtl_media_pipeline_filter_manager.cpp, cleanup slot 26) decrements
// it and clears +0x4C. At zero it calls counter destroy(1), whose implementation
// in gtl_revolution_show_counter_part2.cpp frees via gtl_Free. Use that release path, not plain delete/free.
// Creation itself neither starts display counting nor registers a callback.
extern "C" int gtl_CRevolutionFilterManager_EnsureShowCounter(gtl::CRevolutionFilterManager* self) {
    if (!self->showCounter) {
        self->showCounter = GTL_NEW RevolutionShowCounterStorage;
    }
    return 1;
}

// Translation unit 0x803F95E8-0x803F9670 (1 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

struct Filter {
    int references;
    // Destroy a filter through its dynamic type when its reference count reaches zero.
    virtual ~Filter();
};

struct FilterManager {
    char managerBaseStorage[0x50]; // Base state is not accessed by this release operation.
    Filter* filters[5];
};

extern "C" {

enum { kManagedFilterCount = 5 };

// Release each occupied filter slot, destroying filters whose last reference is removed.
// Each of the five slots owns one reference; repeated pointers require one count per slot.
// The manager remains borrowed and all slots are cleared, making a second call a no-op.
// Precondition: every occupied slot has a positive reference count; no underflow check occurs.
void CRevolutionFilterManager_ReleaseAllFilters(FilterManager* manager) {
    for (int slot = 0; slot < kManagedFilterCount; ++slot) {
        Filter* filter = manager->filters[slot];
        if (filter) {
            if (--filter->references == 0) {
                delete filter;
            }
            manager->filters[slot] = 0;
        }
    }
}

} // extern "C"

// Revolution filter manager: owns the five filter slots of the pipeline and releases them.
//
// ReleaseAllFilters drops one reference for each occupied slot and destroys a filter when its
// last reference is removed. Afterwards every slot is empty.

#include "classes/gtl_CRevolutionFilterManager.h"

struct Filter {
    int references;
    // Destroy a filter through its dynamic type when its reference count reaches zero.
    virtual ~Filter();
};

enum {
    kManagedFilterCount = 5
};

// Release each occupied filter slot, destroying filters whose last reference is removed.
// Each of the five slots owns one reference; repeated pointers require one count per slot.
// The slots are all cleared, making a second call a no-op.
// Precondition: every occupied slot has a positive reference count; no underflow check occurs.
void gtl::CRevolutionFilterManager::ReleaseAllFilters() {
    for (int slot = 0; slot < kManagedFilterCount; ++slot) {
        Filter* filter = (Filter*)filters[slot];
        if (filter) {
            if (--filter->references == 0) {
                delete filter;
            }
            filters[slot] = 0;
        }
    }
}

// Revolution split filter: its Destroy, which frees the storage with gtl_Free when asked to delete.

#include "classes/gtl_CRevolutionSplitFilter.h"

extern "C" void gtl_Free(void*);

// Destroy the base split-filter resources, then optionally free the derived storage.
// Null is accepted. Base destruction receives 0 so that it leaves this allocated;
// deleting > 0 frees that storage with gtl_Free. Returns this, possibly freed.
void* gtl::CRevolutionSplitFilter::Destroy(int deleting) {
    if (this != 0) {
        CSplitFilter::Destroy(0);
        if (deleting > 0) {
            if (this != 0)
                gtl_Free(this);
        }
    }
    return this;
}

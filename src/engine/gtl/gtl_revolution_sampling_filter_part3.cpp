// Revolution sampling filter, part 3: its Destroy, which frees the object when asked to delete it.

#include "classes/gtl_CRevolutionSamplingFilter.h"

extern "C" {
void gtl_Free(void*);
} // extern "C"

// Destroy the sampling-filter base and optionally release this object's storage.
// A null this is accepted; deleteFlag > 0 frees the object, otherwise the caller keeps the storage.
// The returned address is unchanged and may already have been freed.
void* gtl::CRevolutionSamplingFilter::Destroy(int deleteFlag) {
    if (this != 0) {
        CSamplingFilter::Destroy(0);
        if (deleteFlag > 0 && this != 0) {
            gtl_Free(this);
        }
    }
    return this;
}

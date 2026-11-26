// Revolution source filter destructor: CRevolutionSourceFilter::Destroy tears down the
// source-filter base and frees the object when asked to.

#include "classes/gtl_CRevolutionSourceFilter.h"

extern "C" {

void gtl_Free(void* self);

} // extern "C"

namespace gtl {

// Destroy the source-filter base, then release storage only when deleteFlag > 0.
// Null this is accepted; base cleanup receives 0 so it does not free the storage.
// Return the original address, which must not be dereferenced after a deleting call.
CRevolutionSourceFilter* CRevolutionSourceFilter::Destroy(int deleteFlag) {
    if (this) {
        CSourceFilter::Destroy(0);
        if (deleteFlag > 0) {
            if (this) {
                gtl_Free(this);
            }
        }
    }
    return this;
}

} // namespace gtl

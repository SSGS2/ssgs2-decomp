// Revolution show counter, part 2: its Destroy, which frees the object when asked to delete it.

#include "classes/gtl_CRevolutionShowCounter.h"

extern "C" {
void gtl_Free(void*, int);
} // extern "C"

// Optionally free the show-counter allocation through gtl_Free; return its old address.
// deletingFlag > 0 releases storage and is forwarded to the allocator unchanged.
// No members are released here; a returned freed address must not be dereferenced.
void* gtl::CRevolutionShowCounter::Destroy(int deletingFlag) {
    if (this != 0) {
        if (deletingFlag > 0 && this != 0) {
            gtl_Free(this, deletingFlag);
        }
    }
    return this;
}

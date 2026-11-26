// Revolution filter manager, part 3: its Destroy, which frees the object when asked to delete it.

#include "classes/GtlMediaPipeline.h"
#include "classes/gtl_CRevolutionFilterManager.h"

extern "C" {

void gtl_Free(void*);
}

// Destroy the base manager and free this object with gtl_Free only for deleting > 0.
// Accepts null. Returns the original address, which is dangling after a deleting call.
void* gtl::CRevolutionFilterManager::Destroy(int deleting) {
    if (this) {
        ((gtl::CFilterManager*)this)->Destroy(0);
        if (deleting > 0) {
            if (this) {
                gtl_Free(this);
            }
        }
    }
    return this;
}

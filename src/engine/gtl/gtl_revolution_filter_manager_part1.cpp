// Revolution filter manager and filter factory, part 1: the manager's Init and the factory's Destroy.
// Init sets the manager's vtable and starts the THP decoder. Destroy accepts null.

#include "classes/gtl_CRevolutionFilterFactory.h"
#include "classes/gtl_CRevolutionFilterManager.h"
#include "classes/GtlMediaPipeline.h"

extern "C" {

void THPInit(void);
void gtl_Free(void*);
extern void* __vt__Q23gtl24CRevolutionFilterManager[];
}

#pragma opt_propagation off
#pragma opt_common_subs off
#pragma opt_lifetimes off
#pragma opt_strength_reduction off
// Initialize the manager in caller-provided storage: base init from the config at input,
// set the manager vtable and start the THP decoder. Returns this.
void* gtl::CRevolutionFilterManager::Init(void* input) {
    ((gtl::CFilterManager*)this)->Init((const gtl::CFilterManagerConfig*)input);
    void* vtable;
    vtable = __vt__Q23gtl24CRevolutionFilterManager;
    vtbl = vtable;
    THPInit();
    return this;
}
#pragma opt_strength_reduction reset
#pragma opt_lifetimes reset
#pragma opt_common_subs reset
#pragma opt_propagation reset

// Optionally free Revolution filter-factory storage through the GTL allocator.
// This is the factory destructor slot (see its vtable), not the manager's destructor.
// Null is accepted. deleting > 0 frees this object; the returned address may therefore be stale.
void* gtl::CRevolutionFilterFactory::Destroy(int deleting) {
    if (this != 0) {
        if (deleting > 0) {
            if (this != 0) {
                gtl_Free(this);
            }
        }
    }
    return this;
}

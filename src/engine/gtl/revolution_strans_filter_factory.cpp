// Revolution filter factory: creates a 48-byte STrans filter object through the GTL allocator.

#include "classes/gtl_CRevolutionFilterFactory.h"
#include "classes/GtlRevolutionSTransFilter.h"

typedef unsigned long new_size_t;

extern "C" {
void* gtl_Alloc(unsigned long);

} // extern "C"

// Route this unit's filter allocation through the GTL allocator.
inline void* operator new(new_size_t size) {
    return gtl_Alloc(size);
}
#define GTL_NEW new

struct RevolutionFilter {
    char data[0x30]; // Opaque STrans-filter storage; keep the constructor's ABI size.
    // Forward the opaque constructor context unchanged, including null.
    RevolutionFilter(void* constructorContext) {
        ((gtl::CRevolutionSTransFilter*)this)->Init(constructorContext);
    }
};

// Create a 0x30-byte STrans filter; the factory receiver is unused.
// Null output returns 0; otherwise overwrite *output and return 1, even if the
// allocation yields null. A nonnull result is caller-owned; any prior *output
// is not released. Context may be null here; constructor retention is unresolved.
int gtl::CRevolutionFilterFactory::CreateSTransFilter(RevolutionFilter** output, void* constructorContext) {
    if (!output)
        return 0;
    *output = GTL_NEW RevolutionFilter(constructorContext);
    return 1;
}

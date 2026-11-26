// Revolution filter factory, VTrans filter: creates the VTrans filter and passes the source
// to its constructor.

#include "classes/gtl_CRevolutionFilterFactory.h"
#include "classes/GtlRevolutionVTransFilter.h"

typedef unsigned long new_size_t;

extern "C" void* gtl_Alloc(unsigned long);
extern "C" void dtor_803F92E8(void*);

namespace gtl {
struct CRevolutionFilter {
    char data[0x50]; // Opaque VTrans-filter storage; field layout is not reconstructed.
    // Allocate filter storage through the GTL allocator.
    static void* operator new(new_size_t size) {
        return gtl_Alloc(size);
    }
    // Construct the VTrans filter using the caller-supplied source/context.
    CRevolutionFilter(void* source) {
        ((gtl::CRevolutionVTransFilter*)this)->Init(source);
    }
    // Run the matching cleanup hook if construction unwinds.
    static void operator delete(void* pointer) {
        dtor_803F92E8(pointer);
    }
};
}

// Create a 0x50-byte VTrans filter.
// Return 0 for null output/source without touching output; otherwise store the
// new result and return 1 (not an allocation-success test). A nonnull filter is
// caller-owned; this wrapper does not release/replace any previous *output.
// source is forwarded to the constructor; its retention policy is unresolved.
int gtl::CRevolutionFilterFactory::CreateVTransFilter(gtl::CRevolutionFilter** output, void* source) {
    if (!output || !source)
        return 0;
    *output = new gtl::CRevolutionFilter(source);
    return 1;
}

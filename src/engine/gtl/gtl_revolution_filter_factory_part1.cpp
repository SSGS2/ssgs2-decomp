// Revolution filter factory: its constructor and destructor, and creating the source filter.
// Memory comes from the gtl allocator.

#include "classes/gtl_CRevolutionFilterFactory.h"
#include "classes/gtl_CRevolutionSourceFilter.h"

typedef unsigned long new_size_t;

extern "C" {
extern void* __vt__Q23gtl24CRevolutionFilterFactory[8];
void* gtl_Alloc(unsigned int);
void gtl_Free(void*);

struct RevolutionSourceFilter {
    // Allocate filter storage from the gtl heap; the factory does not check for allocation failure.
    static void* operator new(new_size_t size) {
        return gtl_Alloc(size);
    }
    // Forward the two creation words unchanged; their specific meaning/retention is unresolved here.
    RevolutionSourceFilter(void* creationPointer, unsigned int creationWord) {
        ((gtl::CRevolutionSourceFilter*)this)->Init(creationPointer, creationWord);
    }
    char padding[0x30];
};

} // extern "C"

// Install the Revolution filter-factory vtable into caller-provided non-null storage.
gtl::CRevolutionFilterFactory::CRevolutionFilterFactory() {
    vtbl = __vt__Q23gtl24CRevolutionFilterFactory;
}

extern "C" {

// Free non-null factory storage on the gtl heap; no child resources are released here.
void dtor_803F92E8(void* self) {
    if (self)
        gtl_Free(self);
}

} // extern "C"

// Create a source filter from {pointer, unsigned word} and publish it to the caller.
// creationArgs is a borrowed, word-aligned block of at least eight bytes. Caller owns
// the resulting filter. Returns 0 for a null argument/output (output untouched), else 1;
// allocation/constructor failure has no explicit status path here.
int gtl::CRevolutionFilterFactory::CreateSourceFilter(void** outFilter, void* creationArgs) {
    if (!outFilter || !creationArgs)
        return 0;
    RevolutionSourceFilter* filter =
        new RevolutionSourceFilter(*(void**)creationArgs, *((unsigned int*)creationArgs + 1));
    *outFilter = filter;
    return 1;
}

// Revolution filter factory: creates the split filter used in the filter pipeline.
//
// CreateSplitFilter allocates the splitter through the gtl heap and constructs it around the
// given filter. The caller owns the result it publishes through out.

#include "classes/gtl_CRevolutionFilterFactory.h"

extern "C" {

void* gtl_Alloc(unsigned int);
void __ct__Q23gtl22CRevolutionSplitFilterFPvPv(void*, void*);

enum {
    kRevolutionSplitFilterBytes = 0x20
};

} // extern "C"

// Allocate and construct a Revolution splitter, publishing its address through out.
// Returns 0 for null out/filter without writing out; returns 1 even if allocation
// fails (*out is then null). Caller owns a non-null result through its destruction API.
// filter is passed unchanged to the constructor as its output format. The constructor is called as a
// plain function with two arguments (the object and filter); its second parameter is not passed.
int gtl::CRevolutionFilterFactory::CreateSplitFilter(void** out, void* filter) {
    if (out == 0 || filter == 0) {
        return 0;
    }

    void* object = gtl_Alloc(kRevolutionSplitFilterBytes);
    if (object != 0) {
        __ct__Q23gtl22CRevolutionSplitFilterFPvPv(object, filter);
    }
    *out = object;
    return 1;
}

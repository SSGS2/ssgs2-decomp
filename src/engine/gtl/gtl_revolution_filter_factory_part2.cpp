// Revolution filter factory, sampling filter: allocates filter storage through the gtl
// allocator and creates a sampling filter for the caller.

#include "classes/gtl_CRevolutionFilterFactory.h"
#include "classes/gtl_CRevolutionSamplingFilter.h"

typedef unsigned long new_size_t;

extern "C" void* gtl_Alloc(unsigned long);

// Allocate filter storage through GTL; the factory preserves a null allocation result.
inline void* operator new(new_size_t size) {
    return gtl_Alloc(size);
}
#define GTL_NEW new

namespace gtl {
class CRevolutionFilter {
    // 0x40-byte sampling-filter allocation; concrete state is initialized by the callee.
    char data[0x40];

public:
    // Initialize caller-provided storage using the opaque borrowed constructor argument.
    CRevolutionFilter(void* settings) {
        ((gtl::CRevolutionSamplingFilter*)this)->Init(settings);
    }
};

}

// Allocate and construct a Revolution sampling filter using the GTL allocator.
// output/settings must be non-null; otherwise return 0 and leave output untouched.
// Return 1 means arguments were accepted: *output may still be null on allocation
// failure. A non-null filter transfers to the caller; settings retention is unresolved.
int gtl::CRevolutionFilterFactory::CreateSampling(gtl::CRevolutionFilter** output, void* settings) {
    if (!output || !settings)
        return 0;
    *output = GTL_NEW gtl::CRevolutionFilter(settings);
    return 1;
}

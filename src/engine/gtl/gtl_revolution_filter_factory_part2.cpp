typedef unsigned long new_size_t;
// Translation unit 0x803F92F8-0x803F9370 (1 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

extern "C" void gtl_CRevolutionSamplingFilter_Ctor(void*, void*);
extern "C" void* gtl_Alloc(unsigned long);

// Allocate filter storage through GTL; the factory preserves a null allocation result.
inline void* operator new(new_size_t size) { return gtl_Alloc(size); }
#define GTL_NEW new

namespace gtl {
class CRevolutionFilter {
    // 0x40-byte sampling-filter allocation; concrete state is initialized by the callee.
    char data[0x40];
public:
    // Initialize caller-provided storage using the opaque borrowed constructor argument.
    CRevolutionFilter(void* settings) { gtl_CRevolutionSamplingFilter_Ctor(this, settings); }
};

}

// Allocate and construct a Revolution sampling filter using the GTL allocator.
// output/settings must be non-null; otherwise return 0 and leave output untouched.
// Return 1 means arguments were accepted: *output may still be null on allocation
// failure. A non-null filter transfers to the caller; settings retention is unresolved.
extern "C" int gtl_CRevolutionFilterFactory_CreateSampling(void* unusedFactory, gtl::CRevolutionFilter** output, void* settings) {
    if (!output || !settings)
        return 0;
    *output = GTL_NEW gtl::CRevolutionFilter(settings);
    return 1;
}

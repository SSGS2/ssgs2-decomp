typedef unsigned long new_size_t;
// Translation unit 0x803F93E8-0x803F9460 (1 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

extern "C" void gtl_CRevolutionVTransFilter_Ctor(void*, void*);
extern "C" void* gtl_Alloc(unsigned long);
extern "C" void dtor_803F92E8(void*);

namespace gtl {
struct CRevolutionFilter {
    char data[0x50]; // Opaque VTrans-filter storage; field layout is not reconstructed.
    // Allocate filter storage through the GTL allocator.
    static void* operator new(new_size_t size) { return gtl_Alloc(size); }
    // Construct the VTrans filter using the caller-supplied source/context.
    CRevolutionFilter(void* source) { gtl_CRevolutionVTransFilter_Ctor(this, source); }
    // Run the matching cleanup hook if construction unwinds.
    static void operator delete(void* pointer) { dtor_803F92E8(pointer); }
};
}

extern "C" {

// Create a 0x50-byte VTrans filter; the factory receiver is unused.
// Return 0 for null output/source without touching output; otherwise store the
// new result and return 1 (not an allocation-success test). A nonnull filter is
// caller-owned; this wrapper does not release/replace any previous *output.
// source is forwarded to the constructor; its retention policy is unresolved.
int gtl_CRevolutionFilterFactory_CreateVTransFilter(void*, gtl::CRevolutionFilter** output, void* source)
{
    if (!output || !source)
        return 0;
    *output = new gtl::CRevolutionFilter(source);
    return 1;
}

} // extern "C"

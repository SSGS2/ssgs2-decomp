typedef unsigned long new_size_t;
// Translation unit 0x803F9460-0x803F94D0 (1 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

extern "C" {
void* gtl_Alloc(unsigned long);
void gtl_CRevolutionSTransFilter_Ctor(void*, void*);

} // extern "C"

// Route this unit's filter allocation through the GTL allocator.
inline void* operator new(new_size_t size) { return gtl_Alloc(size); }
#define GTL_NEW new

struct RevolutionFilter {
    char data[0x30]; // Opaque STrans-filter storage; keep the constructor's ABI size.
    // Forward the opaque constructor context unchanged, including null.
    RevolutionFilter(void* constructorContext) { gtl_CRevolutionSTransFilter_Ctor(this, constructorContext); }
};

// Create a 0x30-byte STrans filter; the factory receiver is unused.
// Null output returns 0; otherwise overwrite *output and return 1, even if the
// allocation yields null. A nonnull result is caller-owned; any prior *output
// is not released. Context may be null here; constructor retention is unresolved.
extern "C" int CRevolutionFilterFactory_CreateSTransFilter(void*, RevolutionFilter** output, void* constructorContext) {
    if (!output) return 0;
    *output = GTL_NEW RevolutionFilter(constructorContext);
    return 1;
}

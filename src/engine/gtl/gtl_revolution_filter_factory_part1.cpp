typedef unsigned long new_size_t;
// Translation unit 0x803F925C-0x803F92F8 (3 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

extern "C" {
extern void* __vt__Q23gtl24CRevolutionFilterFactory[8];
void* gtl_Alloc(unsigned int);
void gtl_Free(void*);
void gtl_CRevolutionSourceFilter_Ctor(void*, void*, unsigned int);
void gtl_CSourceFilter_Ctor(void*);

struct RevolutionSourceFilter {
    // Allocate filter storage from the gtl heap; the factory does not check for allocation failure.
    static void* operator new(new_size_t size) { return gtl_Alloc(size); }
    // Forward the two creation words unchanged; their specific meaning/retention is unresolved here.
    RevolutionSourceFilter(void* creationPointer, unsigned int creationWord) {
        gtl_CRevolutionSourceFilter_Ctor(this, creationPointer, creationWord);
    }
    char padding[0x30];
};

// Install the Revolution filter-factory vtable into caller-provided non-null storage.
void gtl_CRevolutionFilterFactory_Ctor(void* self) {
    *(void**)self = __vt__Q23gtl24CRevolutionFilterFactory;
}

// Free non-null factory storage on the gtl heap; no child resources are released here.
void dtor_803F92E8(void* self) {
    if (self) gtl_Free(self);
}

// Create a source filter from {pointer, unsigned word} and publish it to the caller.
// creationArgs is a borrowed, word-aligned block of at least eight bytes. Caller owns
// the resulting filter. Returns 0 for a null argument/output (output untouched), else 1;
// allocation/constructor failure has no explicit status path here. Factory self is unused.
int gtl_CRevolutionFilterFactory_CreateSourceFilter(void*, void** outFilter, void* creationArgs) {
    if (!outFilter || !creationArgs) return 0;
    RevolutionSourceFilter* filter = new RevolutionSourceFilter(*(void**)creationArgs, *((unsigned int*)creationArgs + 1));
    *outFilter = filter;
    return 1;
}

} // extern "C"

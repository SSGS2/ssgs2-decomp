// Translation unit 0x803F9370-0x803F93E8 (1 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

extern "C" {

void* gtl_Alloc(unsigned int);
void gtl_CRevolutionSplitFilter_Ctor(void*, void*);

enum { kRevolutionSplitFilterBytes = 0x20 };

// Allocate and construct a Revolution splitter, publishing its address through out.
// Returns 0 for null out/filter without writing out; returns 1 even if allocation
// fails (*out is then null). Caller owns a non-null result through its destruction API.
// filter is passed unchanged to the constructor; its retention contract is unresolved.
// The constructor definition in revolution_split_filter.cpp takes a third argument absent from this
// declaration. Keep this existing ABI discrepancy visible; do not infer that argument.
int gtl_CRevolutionFilterFactory_CreateSplitFilter(void* self, void** out, void* filter) {
    if (out == 0 || filter == 0) {
        return 0;
    }

    void* object = gtl_Alloc(kRevolutionSplitFilterBytes);
    if (object != 0) {
        gtl_CRevolutionSplitFilter_Ctor(object, filter);
    }
    *out = object;
    return 1;
}

} // extern "C"

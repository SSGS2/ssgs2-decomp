// Translation unit 0x803F94D0-0x803F9554 (2 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

extern "C" {

void gtl_CFilterManager_Ctor(void*, void*);
void THPInit(void);
void gtl_Free(void*);
extern void* __vt__Q23gtl24CRevolutionFilterManager[];

#pragma opt_propagation off
#pragma opt_common_subs off
#pragma opt_lifetimes off
#pragma opt_strength_reduction off
// Construct a Revolution media pipeline and initialize the THP decoder subsystem.
// self is caller-provided storage; input is the required (sample bytes, sample count)
// pair consumed by the base constructor. Returns self; neither argument is freed.
void* gtl_CRevolutionFilterManager_Ctor(void* self, void* input) {
    gtl_CFilterManager_Ctor(self, input);
    void* vtable;
    vtable = __vt__Q23gtl24CRevolutionFilterManager;
    *(void**)self = vtable;
    THPInit();
    return self;
}
#pragma opt_strength_reduction reset
#pragma opt_lifetimes reset
#pragma opt_common_subs reset
#pragma opt_propagation reset

// Optionally free Revolution filter-factory storage through the GTL allocator.
// This is the factory destructor slot (see its vtable), not the manager's destructor.
// Null is accepted. deleting > 0 frees self; the returned address may therefore be stale.
void* gtl_CRevolutionFilterFactory_dtor(void* self, int deleting) {
    if (self != 0) {
        if (deleting > 0) {
            if (self != 0) {
                gtl_Free(self);
            }
        }
    }
    return self;
}

} // extern "C"

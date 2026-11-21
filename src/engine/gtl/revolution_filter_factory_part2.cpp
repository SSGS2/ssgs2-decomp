// Translation unit 0x803F9554-0x803F95E8 (1 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

extern "C" {
void gtl_CRevolutionFilterFactory_Ctor(void*);
void gtl_CRevolutionFilterFactory_CreateSourceFilter(void*, void*, void*);
void gtl_CRevolutionFilterFactory_CreateSampling(void*, void*, void*);
void gtl_CRevolutionFilterFactory_CreateSplitFilter(void*, void*, void*);
void gtl_CRevolutionFilterFactory_CreateVTransFilter(void*, void*, void*);
void CRevolutionFilterFactory_CreateSTransFilter(void*, void*, void*);
}

struct RevolutionFilterFactory {
    void* vtable;
    // Install the factory vtable in this stack-local dispatch object.
    RevolutionFilterFactory() { gtl_CRevolutionFilterFactory_Ctor(this); }
    // The stack factory owns no filter storage to release.
    ~RevolutionFilterFactory() {}
};

namespace gtl {
struct CRevolutionFilterManager {
    void* vtable;
    void* context;
    char data[0x3C];
    void* field44;
    void* field48;
    void* field4C;
    // Slot order follows factory methods 1..5: source, sampling, split,
    // VTrans, STrans. Created filter pointers are stored here for later use.
    void* filters[5];
};
}

// Populate the manager's five filter slots using their respective borrowed settings.
// Returns 1 unconditionally: individual factory status values are ignored, so this
// does not prove every slot is non-null. Existing slots are not released here.
extern "C" int CRevolutionFilterManager_CreateFilterPipeline(gtl::CRevolutionFilterManager* self) {
    // Source construction receives a two-word argument block; the other methods
    // receive the context or the settings pointer at manager +0x44/+0x48 directly.
    RevolutionFilterFactory factory;
    struct Parameters { void* context; void* data; } parameters;
    parameters.context = self->context;
    parameters.data = self->data;
    // Build each stage in pipeline order and publish directly into the manager.
    gtl_CRevolutionFilterFactory_CreateSourceFilter(&factory, &self->filters[0], &parameters);
    gtl_CRevolutionFilterFactory_CreateSampling(&factory, &self->filters[1], self->context);
    gtl_CRevolutionFilterFactory_CreateSplitFilter(&factory, &self->filters[2], self->context);
    gtl_CRevolutionFilterFactory_CreateVTransFilter(&factory, &self->filters[3], self->field44);
    CRevolutionFilterFactory_CreateSTransFilter(&factory, &self->filters[4], self->field48);
    return 1;
}

extern "C" {

} // extern "C"

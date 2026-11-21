// Apply a gtl environment resource to the current renderer (no gameplay knobs).
// API lookup: gtl_GenericEnvironment_Apply in gtl_generic_environment.cpp scans archive chunks and
// sets lights, fog, ambient color, view and projection. Its third argument is
// the number of light slots to visit, including slots to disable when unused.
// s_genericEnvironmentLightSlotCount caches Renderer_InitializeDefaults' +0xA0 value during
// gtl_GenericResource_Initialize (gtl_generic_resource_loading.cpp), rather than an environment ID.
// Translation unit 0x803DBB54-0x803DBB84 (1 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

// Partial native Wii IGenericResourceFile prefix; archive is retained by the
// resource's construction/destruction, only borrowed by this forwarding call.
struct EnvironmentResourceView {
    int referenceCount;
    void* vtable;
    void* retainedArchive;
};

extern "C" {

extern unsigned int s_genericEnvironmentLightSlotCount;
void gtl_GenericEnvironment_Apply(void* obj, void* arg1, unsigned int* arg2);

// Receiver: required resource interface with refcount +0, vtable +4, retained
// archive pointer +8 (same IGenericResourceFile prefix as gtl_generic_resource_loading.cpp).
// overrides may be null; otherwise borrowed five-pointer record: +0 lights,
// +4 view matrix, +8 projection matrix, +C fog, +10 ambient. See the callee's
// exact admission tests (ambient is also gated by +C, not by +10).
// Archive/override storage must survive this synchronous call. No ownership is
// transferred and no status is returned; render state persists. Missing chunks
// leave their corresponding settings untouched, except unused light slots are
// disabled. The stack light-slot count is input-only in the current callee.
void gtl_IGenericEnvironmentResource_Apply(void* self, void* overrides) {
    unsigned int lightSlotCount = s_genericEnvironmentLightSlotCount;
    gtl_GenericEnvironment_Apply(((EnvironmentResourceView*)self)->retainedArchive, overrides, &lightSlotCount);
}

} // extern "C"

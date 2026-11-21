// Generic model renderer setup: publish the shared context, clear per-pass work,
// reserve scratch storage, and cache view/inverse-view matrices for rendering.
// Mod lookup: RendererScratchSizes controls allocation counts (not screen size);
// ResizeScratchBuffers in gtl_generic_renderer_scratch_buffers.cpp owns their allocation and limits. Shared
// default record s_rendererDefaults begins {512,512,64}; its fourth word is not read here.
// Transforms come from Engine_GetTransform, not the allocation request.
// Translation unit 0x803DCF8C-0x803DD064 (1 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

#include "sdk_protos.h"

struct GtlGenericRenderer {
    char reserved00[0xC0];
    float inverseViewTranspose[16]; // +C0: transpose of the legacy inverse result.
    float inverseView[16];          // +100: legacy inverse result (see contract).
    float view[16];                 // +140: world -> view, engine transform ID 0.
    int setupWord180;               // +180: set to one; further role unestablished.
};

// Three native u32 counts, four-byte alignment (gtl_generic_renderer_scratch_buffers.cpp's ResizeSizes).
// +0/+4 reserve word-slot vectors at context+194/+1A0; their sum sizes the
// 0x50-byte entry vector at +184. +8 reserves word slots at +1AC. Slots' higher
// level model roles are not established; they are counts, never byte sizes.
struct RendererScratchSizes { unsigned int slots194, slots1A0, slots1AC; };

extern "C" {

extern signed char s_genericRendererContextInitialized;
extern GtlGenericRenderer* g_genericRendererContext;
extern GtlGenericRenderer s_genericRendererContextStorage;
extern char s_genericRendererContextRegistration[];
extern char s_rendererDefaults[];

void gtl_GenericRenderer_ContextCtor(GtlGenericRenderer* self);
void gtl_GenericRenderer_ContextDtor(void);
void gtl_GenericRenderer_ClearScratchCounts(void);
void gtl_GenericRenderer_ClearDrawQueue(void);
void gtl_GenericRenderer_ResizeScratchBuffers(const void* sizes);
void Engine_GetTransform(unsigned int type, void* matrix);
void Mtx44_InvertLegacyCofactors(void* dst, int det, const void* src);
void PSMTX44Transpose(void* dst, const void* src);

// sizes is an optional borrowed 12-byte request, despite the old receiver-shaped
// parameter type. It must not overlap the shared mutable context. Null selects
// defaults; no pointer is retained. Sum of first
// two counts must not overflow and must fit 0x03333333 entries; each individual
// word-vector count must fit 0x3fffffff. Allocation/length errors propagate;
// return 1 means setup returned normally, not that matrix inversion succeeded.
// Cache matrices are row-major 4x4 float words (four rows of four); inverse and
// transpose preserve the existing call sequence/precision. Inversion requires
// a nonsingular view; the helper's legacy C22 cofactor anomaly is preserved
// (gtl_vector_matrix_math.cpp). A zero determinant leaves +100 unchanged, then that old cache
// is transposed into +C0; no success check or identity fallback occurs here.
// Matrices follow the engine's row-vector convention; +140 maps world to view.
// Global publication/reset can
// precede failure, so this is not transactional. Serialize on the render thread.
// The singleton is registered for shutdown; it owns its buffers, not this input.
int gtl_GenericRenderer_InitializeContext(const RendererScratchSizes* sizes) {
    if (s_genericRendererContextInitialized == 0) {
        gtl_GenericRenderer_ContextCtor(&s_genericRendererContextStorage);
        __register_global_object(&s_genericRendererContextStorage, (void*)gtl_GenericRenderer_ContextDtor, s_genericRendererContextRegistration);
        s_genericRendererContextInitialized = 1;
    }
    g_genericRendererContext = &s_genericRendererContextStorage;
    s_genericRendererContextStorage.setupWord180 = 1;
    gtl_GenericRenderer_ClearScratchCounts();
    gtl_GenericRenderer_ClearDrawQueue();
    gtl_GenericRenderer_ResizeScratchBuffers(sizes != 0 ? (const void*)sizes : (const void*)s_rendererDefaults);
    // Raw offsets retain the original address arithmetic; typed schema is above.
    Engine_GetTransform(0, (char*)g_genericRendererContext + 0x140);
    Mtx44_InvertLegacyCofactors((char*)g_genericRendererContext + 0x100, 0, (char*)g_genericRendererContext + 0x140);
    PSMTX44Transpose((char*)g_genericRendererContext + 0xC0, (char*)g_genericRendererContext + 0x100);
    return 1;
}

} // extern "C"

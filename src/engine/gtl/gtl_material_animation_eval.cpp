// GTL model material animation: initialize texture-coordinate transforms at key time zero.
// API: EvaluateMaterialAnimation -> Object_GetField8 -> EvaluateTextureTransformMatrices
// (gtl_texture_animation_matrices.cpp) -> BuildTextureMatrix (gtl_transform_frustum_stopwatch_input.cpp). No gameplay timing knob lives here.
// Mod lookup: texture scale/offset belong to the resource's DPT defaults and
// VCF/DK tracks; this wrapper selects the initial key time, not an update delta.
// kFloat_0_GtlResource is pooled float32 0.0, also used by EvaluateTransformAnimation
// (gtl_generic_modeling_resource_eval.cpp) and resource setup (gtl_generic_resource_loading.cpp); do not retune the pool globally.
// Translation unit 0x803D9550-0x803D9594 (1 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

extern "C" {

void* Object_GetField8(void* obj);
// ABI declaration retained: the definition's first pointer argument is unused.
// Zero is not a texture-animation flag mask. Its f64 time is narrowed to f32
// by the evaluator; this wrapper supplies the existing f32 zero unchanged.
void gtl_Animation_EvaluateTextureTransformMatrices(int unusedContext, void* matrices, void* resource, double keyTime);
extern float kFloat_0_GtlResource;

// Partial ABI view: +0 vtable/reference header, +8 retained buffer wrapper.
// Object_GetField8(buffer) borrows its parsed resource; the evaluator then reads
// its +0xC chunk root and resolves offsets relative to that resource's base.
struct MaterialAnimationModelView { char header[8]; void* buffer; };

// Required self/buffer/resource and writable matrices, all caller-owned/borrowed.
// matrices is an array of float[16] (0x40-byte slots), large enough for every
// PT matrix index other than 0xFF. Writes only referenced slots, in texture
// coordinate space, not model/world space. BuildTextureMatrix starts with identity,
// then applies resource scale/translation or mode-6 transformation. Keep its matrix
// convention with the renderer; no transpose or coordinate conversion occurs here.
// Missing PT/DPT returns from the evaluator without writing; missing VCF/DK skips
// affected slots. This void wrapper discards status; it neither clears outputs
// nor allocates/retains them. Output must not overlap resource storage.
void gtl_IGenericModelingResource_EvaluateMaterialAnimation(const void* self, void* matrices) {
    gtl_Animation_EvaluateTextureTransformMatrices(0, matrices, Object_GetField8(((const MaterialAnimationModelView*)self)->buffer), kFloat_0_GtlResource);
}

} // extern "C"

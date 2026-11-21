// Translation unit 0x803D94FC-0x803D9550 (1 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".
// Model transform-animation snapshot adapter; no clock is advanced here.
// Mod lookup: gtl_IGenericModelingResource_EvaluateTransformAnimation samples
// the resource at its fixed initial key time. Channel/key interpolation and
// output record construction live in gtl_animation_transform_channels.cpp; resource stream ownership
// is handled by the generic-resource retain/release API in gtl_generic_resource_loading.cpp.

extern "C" {

void* Object_GetField8(void* o);
int gtl_Animation_EvaluateModelTransformChannels(int unusedReceiver, float keyTime, void* transformsOut, void* trackCountWord, void* resourceFile);
extern const float kFloat_0_GtlResource; // 0.0f: initial resource key time, not a delta or interpolation weight.

struct GenericModelingResource {
    char reserved00[8];
    void* backingResource; // +8: retained backing resource wrapper; Object_GetField8 yields its parsed ResourceFile.
};

// Sample initial transforms into caller-owned records; return the evaluator's
// status (0 if its 'ft' transform chunk is absent, 1 after evaluation).
// self/parent/parsed file must be live and valid for the whole synchronous call;
// nothing is retained here. transformsOut must not overlap the resource data.
// trackCountWord carries an UNSIGNED INTEGER in a legacy pointer-shaped ABI slot,
// not a second input record. The callee processes min(count, 'trns'.count) tracks
// but writes by each track's u16 index, NOT by sequential processed-track number.
// Reserve 0x30 * 'ft'.count bytes; a small track count alone does not bound the
// largest output index. Unvisited output records remain unchanged.
// Each 0x30-byte record is eleven float words plus a mode word at +0x2C;
// channels write +0..+0x28 and mode 6 converts Euler components to a quaternion.
// Resource indices/channels/keys must be consistent: the evaluator validates no
// output capacity and assumes required chunks other than 'ft' exist.
// The leading 0 is the evaluator's unused receiver; the scalar is time=0 in the
// resource key domain. Changing it selects a different snapshot, not a speed.
// Evaluation updates the evaluator's shared interpolation scratch (gtl_animation_transform_channels.cpp):
// serialize calls. The wrapper adds no error recovery or output clearing.
int gtl_IGenericModelingResource_EvaluateTransformAnimation(const GenericModelingResource* self, void* transformsOut, void* trackCountWord) {
    return gtl_Animation_EvaluateModelTransformChannels(0, kFloat_0_GtlResource, transformsOut, trackCountWord, Object_GetField8(self->backingResource));
}

} // extern "C"

// Gtl model animation resource adapter: evaluate local scale/rotation/translation
// channels into pose records. CPetAnimation_EvaluateNamedTransforms (pet_animation.cpp)
// and WPuppet's bone sampling (wpuppet_model_bone_animation.cpp) call this; gtl_animation_transform_channels.cpp is the evaluator,
// gtl_transform_frustum_stopwatch_input.cpp converts poses to matrices. No playback clock is advanced here.
// Mod map: sample position is animationFrame; channel/key formats and output
// bounds belong to gtl_animation_transform_channels.cpp, not this resource-unwrapping boundary.
// Translation unit 0x803DB9F8-0x803DBA58 (1 function).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

extern "C" {

void* Object_GetField8(void* o);
// Float arguments use the separate PPC FPR argument bank. Keep this declaration's
// ordering: the GPR sequence is {0, poses, trackLimit, resource}, FPR1 is time;
// gtl_animation_transform_channels.cpp spells the same ABI with time second. Do not reorder GPR arguments.
int gtl_Animation_EvaluateModelTransformChannels(int flags, float animationFrame, void* poses, void* trackLimit, void* resource);

struct AnimationResourceHeader {
    int referenceCount;  // +00, IUnknownCustom convention
    int vtable;          // +04
    void* resourceFile;  // +08, retained resource wrapper, NOT serialized bytes
};

// Borrow required self, its live resource wrapper, and writable poses. Object_GetField8
// loads that wrapper's +08 word (gtl_render_state_and_primitive.cpp); in this context it is the ResourceFile
// consumed by the chunk evaluator. This is a second unwrap, not a vertex-buffer
// lookup despite that accessor's primitive-oriented comment in its defining unit.
// trackLimit carries an unsigned count as an ABI pointer word, not a pointer to
// count. It limits visited tracks, NOT the largest destination index: poses must
// cover every track.index < ft.count, each record 0x30 bytes. See gtl_animation_transform_channels.cpp schema.
// animationFrame is an absolute key-time sample (WPuppet passes frame), not a
// delta or seconds. Return 0 on missing ft chunk without writes; otherwise forward
// 1 even for zero visited tracks. Malformed/null resources are unchecked. No retain,
// release or allocation here; serialize evaluation because interpolation uses globals.
int gtl_IGenericAnimationResource_EvaluateTransforms(const AnimationResourceHeader* self, void* poses, void* trackLimit, float animationFrame) {
    return gtl_Animation_EvaluateModelTransformChannels(0, animationFrame, poses, trackLimit, Object_GetField8(self->resourceFile));
}

} // extern "C"

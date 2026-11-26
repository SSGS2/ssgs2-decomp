// Model transform-animation snapshot: samples the resource at its fixed initial key time. Channel and key
// interpolation live in gtl_animation_transform_channels.cpp; the backing resource file is owned by the
// generic-resource loader in gtl_generic_resource_loading.cpp.
#include "classes/GtlIGenericModelingResource.h"

extern "C" {

void* Object_GetField8(void* o);
int gtl_Animation_EvaluateModelTransformChannels(int unusedReceiver, float keyTime, void* transformsOut,
                                                 void* trackCountWord, void* resourceFile);
extern const float kFloat_0_GtlResource; // 0.0f: initial resource key time, not a delta or interpolation weight.
}

// Sample initial transforms into caller-owned records; return the evaluator's status (0 if its 'ft' transform
// chunk is absent, 1 after evaluation). self and the parsed file must stay valid for the whole synchronous call;
// nothing is retained here. transformsOut must not overlap the resource data.
// trackCountWord carries an unsigned count in a pointer-sized slot. The evaluator processes min(count, 'trns'.count)
// tracks but writes by each track's u16 index, so reserve 0x30 * 'ft'.count bytes. Unvisited records stay unchanged.
// The leading 0 passed to the evaluator is its unused receiver; the float is time 0 in the resource key domain.
// Evaluation shares interpolation scratch in the evaluator, so calls must be serialized.
int gtl::IGenericModelingResource::EvaluateTransformAnimation(void* transformsOut, void* trackCountWord) const {
    return gtl_Animation_EvaluateModelTransformChannels(0, kFloat_0_GtlResource, transformsOut, trackCountWord,
                                                        Object_GetField8(backingResource));
}

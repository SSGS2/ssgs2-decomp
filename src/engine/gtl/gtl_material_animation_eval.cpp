// Model material animation: initializes texture-coordinate transforms at key time zero. The evaluator chain is
// Object_GetField8 -> gtl_Animation_EvaluateTextureTransformMatrices (gtl_texture_animation_matrices.cpp).
#include "classes/GtlIGenericModelingResource.h"

extern "C" {

void* Object_GetField8(void* obj);
// ABI declaration retained: the definition's first pointer argument is unused. Zero is not a texture-animation
// flag mask. Its f64 time is narrowed to f32 by the evaluator.
void gtl_Animation_EvaluateTextureTransformMatrices(int unusedContext, void* matrices, void* resource, double keyTime);
extern float kFloat_0_GtlResource;
}

// matrices is an array of float[16] slots, one for every texture-matrix index that is used. Writes only the
// referenced slots, in texture-coordinate space. Missing resource tables return without writing; the result
// is discarded here. Output must not overlap resource storage.
void gtl::IGenericModelingResource::EvaluateMaterialAnimation(void* matrices) const {
    gtl_Animation_EvaluateTextureTransformMatrices(0, matrices, Object_GetField8(backingResource),
                                                   kFloat_0_GtlResource);
}

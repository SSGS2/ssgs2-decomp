// Generic animation resource, part 2: evaluates the material (texture transform) matrices of an
// animation resource from its retained buffer.
//
// The destructor and release code are in gtl_generic_animation_resource_part1.cpp.

#include "classes/GtlIGenericResourceFile.h"

extern "C" {

void* Object_GetField8(void* o);
void gtl_Animation_EvaluateTextureTransformMatrices(int flags, void* a, void* b, double dt);

} // extern "C"

// The evaluation returns 1 when the material chunks exist; the original leaves the result in r3.
void gtl::IGenericAnimationResource::EvaluateMaterials(void* matrices, float dt) const {
    gtl_Animation_EvaluateTextureTransformMatrices(0, matrices, Object_GetField8(resourceBuffer), dt);
}

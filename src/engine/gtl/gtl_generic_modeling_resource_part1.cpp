// Evaluates the model's transform hierarchy from the chunk directory of its backing resource file.
#include "classes/GtlIGenericModelingResource.h"

extern "C" {

void* Object_GetField8(void* obj);
void gtl_Animation_EvaluateModelMatrices(void* file, void* out, void* root, void* poses, unsigned int poseCount,
                                         unsigned int* flagsIn);
}

#pragma use_lmw_stmw on
void gtl::IGenericModelingResource::EvaluateTransforms(void* out, void* unused, void* root, void* poses,
                                                       unsigned int poseCount, unsigned int* flagsIn) const {
    gtl_Animation_EvaluateModelMatrices(Object_GetField8(backingResource), out, root, poses, poseCount, flagsIn);
}
#pragma use_lmw_stmw reset

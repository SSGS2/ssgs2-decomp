// Translation unit 0x803D94A4-0x803D94FC (1 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

extern "C" {

void* Object_GetField8(void* obj);
void gtl_Animation_EvaluateModelMatrices(void* file, void* out, void* root, void* poses, unsigned int poseCount, unsigned int* flagsIn);

#pragma use_lmw_stmw on
void gtl_IGenericModelingResource_EvaluateTransforms(const void* self, void* out, void* unused, void* root, void* poses, unsigned int poseCount, unsigned int* flagsIn) {
    gtl_Animation_EvaluateModelMatrices(Object_GetField8(((void* const*)self)[2]), out, root, poses, poseCount, flagsIn);
}
#pragma use_lmw_stmw reset

} // extern "C"

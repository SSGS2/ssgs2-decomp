// Translation unit 0x803DBA58-0x803DBAA8 (1 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

extern "C" {

void* Object_GetField8(void* o);
void gtl_Animation_EvaluateTextureTransformMatrices(int flags, void* a, void* b, double dt);

struct AnimationResourceHeader {
    int vt;
    int rc;
    void* buf;
};

// The evaluation returns 1 when the material chunks exist; the original leaves the result in r3.
void gtl_IGenericAnimationResource_EvaluateMaterials(const AnimationResourceHeader* self, void* a, float dt) {
    gtl_Animation_EvaluateTextureTransformMatrices(0, a, Object_GetField8(self->buf), dt);
}

} // extern "C"

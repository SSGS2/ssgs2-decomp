// Translation unit 0x803E0F0C-0x803E0F30 (1 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

extern "C" {

void Vec3_Lerp(void* out, const void* a, const void* b, float t);
int gtl_Animation_InterpolateLinearVec3(void* out, const void* a, const void* b, float t)
{
    Vec3_Lerp(out, a, b, t);
    return 1;
}

} // extern "C"

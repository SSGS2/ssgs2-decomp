// Linear interpolation for three-component animation values. Calls the Vec3 lerp helper
// and always reports success.

extern "C" {

void Vec3_Lerp(void* out, const void* a, const void* b, float t);
int gtl_Animation_InterpolateLinearVec3(void* out, const void* a, const void* b, float t) {
    Vec3_Lerp(out, a, b, t);
    return 1;
}

} // extern "C"

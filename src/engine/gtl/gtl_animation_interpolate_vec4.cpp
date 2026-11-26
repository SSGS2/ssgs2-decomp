// Linear interpolation for four-component animation values. Calls the Vec4 lerp helper
// and always reports success.

extern "C" {

void Vec4_Lerp(void* out, const void* a, const void* b, float t);

int gtl_Animation_InterpolateLinearVec4(void* out, const void* a, const void* b, float t) {
    Vec4_Lerp(out, a, b, t);
    return 1;
}

} // extern "C"

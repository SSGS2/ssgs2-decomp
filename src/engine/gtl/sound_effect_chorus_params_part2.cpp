// Chorus sound effect getter: CSoundEffectChorus::GetParameter reads the delay, depth or
// rate, or all three, back from the effect.

#include "sdk_protos.h"
#include "classes/SoundEffectFields803FB370.h"

extern "C" {
void* memcpy(void*, const void*, size_t);
float* gtl_nw4r_FxChorus_GetParams(void*);

enum ChorusParameterQuery {
    kChorusDelayMilliseconds,
    kChorusDepthRatio,
    kChorusRateHertz,
    kChorusAllParameters,
    kChorusParameterCount = 3
};

} // extern "C"

// Read a chorus parameter, or copy all three into the caller's pointed-to buffer.
// 0/1/2 require float* output; 3 requires void** output pointing to >=12 writable
// bytes in delay-ms/depth-ratio/rate-Hz order. All pointers are borrowed and unchecked.
// Reads raw requested values, not SDK-clamped settings. Unknown selectors leave
// output untouched. The third ABI argument is ignored; every selector returns 1.
int gtl::CSoundEffectChorus::GetParameter(int parameter, int unused, void* output) {
    float* values = gtl_nw4r_FxChorus_GetParams(effect);
    switch (parameter) {
    case kChorusDelayMilliseconds:
        *(float*)output = values[kChorusDelayMilliseconds];
        break;
    case kChorusDepthRatio:
        *(float*)output = values[kChorusDepthRatio];
        break;
    case kChorusRateHertz:
        *(float*)output = values[kChorusRateHertz];
        break;
    case kChorusAllParameters:
        memcpy(*(void**)output, values, kChorusParameterCount * sizeof(float));
        break;
    }
    return 1;
}

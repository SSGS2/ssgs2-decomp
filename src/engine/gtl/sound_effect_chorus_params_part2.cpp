// Translation unit 0x803FB2D4-0x803FB370 (1 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

#include "sdk_protos.h"

namespace gtl {
struct CSoundEffectChorus {
    char wrapperStorage[0x14]; // Sound-effect wrapper before the embedded chorus.
    char chorus[1]; // Legacy field name: embedded FxChorus begins here, not its float controls.
};
}

extern "C" {
void* memcpy(void*, const void*, size_t);
float* gtl_nw4r_FxChorus_GetParams(void*);

enum ChorusParameterQuery {
    kChorusDelayMilliseconds, kChorusDepthRatio, kChorusRateHertz, kChorusAllParameters,
    kChorusParameterCount = 3
};

// Read a chorus parameter, or copy all three into the caller's pointed-to buffer.
// 0/1/2 require float* output; 3 requires void** output pointing to >=12 writable
// bytes in delay-ms/depth-ratio/rate-Hz order. All pointers are borrowed and unchecked.
// Reads raw requested values, not SDK-clamped settings. Unknown selectors leave
// output untouched. The third ABI argument is ignored; every selector returns 1.
int CSoundEffectChorus_GetParameter(gtl::CSoundEffectChorus* self, int parameter, int unused, void* output) {
    float* values = gtl_nw4r_FxChorus_GetParams(self->chorus);
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

} // extern "C"

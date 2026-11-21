// Translation unit 0x803FA670-0x803FA728 (1 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

#include "sdk_protos.h"
#include "MSL/internal/mem.h"

namespace gtl {
class CSoundEffectReverbHi {
public:
    char ownerAndEffectLinks[0x14]; // Embedded FxReverbHi begins immediately after this prefix.
    // This is the embedded effect's prefix at +0x14; GetParams returns its
    // parameter array at effect +0x18 (wrapper +0x2C).
    char embeddedEffectPrefix[0x18];
};
}

extern "C" {
float* gtl_nw4r_FxReverbHi_GetParams(void*);

enum ReverbParameter {
    kPreDelay, kTime, kColoration, kDamping, kCrosstalk, kMix, kAllParameters
};

// IDs refer to the raw request block, not AXFX work-state field order:
// 0 pre-delay, 1 time, 2 coloration, 3 damping, 4 crosstalk, 5 mix, 6 all.
// gtl_fx_reverb_hi.cpp maps this raw block to the differently ordered SDK fields. Time units
// pass through to the backend; this getter performs no conversion or clamping.
// Return one reverb setting, or copy all six settings into the caller's pointed-to buffer.
// parameter 0..5 requires output = float*; parameter 6 requires output = float**
// whose pointee has room for six floats in the enum order. Both are borrowed
// writable destinations; no pointer is retained and no storage is allocated.
// The third ABI argument is ignored. Unknown indices leave output untouched.
// Returns 1 even for an unknown index; no output/null/buffer-size validation occurs.
int gtl_CSoundEffectReverbHi_GetParameter(gtl::CSoundEffectReverbHi* self, unsigned int parameter, unsigned int unused, void* output) {
    float* values = gtl_nw4r_FxReverbHi_GetParams(self->embeddedEffectPrefix);
    switch (parameter) {
    case kPreDelay: *(float*)output = values[kPreDelay]; break;
    case kTime: *(float*)output = values[kTime]; break;
    case kColoration: *(float*)output = values[kColoration]; break;
    case kDamping: *(float*)output = values[kDamping]; break;
    case kCrosstalk: *(float*)output = values[kCrosstalk]; break;
    case kMix: *(float*)output = values[kMix]; break;
    case kAllParameters: memcpy(*(void**)output, values, kAllParameters * sizeof(float)); break;
    }
    return 1;
}

} // extern "C"

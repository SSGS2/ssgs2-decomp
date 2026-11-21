// Translation unit 0x803FAEE8-0x803FAFA8 (1 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

#include "sdk_protos.h"
#include "classes/SoundEffectReverbStdState.h"

extern "C" {
void* memcpy(void*, const void*, unsigned long);
float* gtl_nw4r_FxReverbStd_GetParams(void*);

// Public block order verified against fx_reverb_std_params.cpp's getter/setter, not legacy field names.
// Time units are unresolved here; coloration/damping/wet mix are unitless controls.
enum ReverbParameter {
    kReverbPreDelay = 0, kReverbDecayTime = 1, kReverbColoration = 2,
    kReverbDamping = 3, kReverbWetMix = 4, kReverbAllParameters = 5,
    kReverbParameterBlockBytes = 5 * sizeof(float)
};

// Copy the requested (unclamped) reverb settings to caller-owned output storage.
// IDs 0..4: result points to a float. ID 5: result points to a POINTER to a
// writable five-float block (extra indirection). No output pointers are checked.
// Unknown IDs leave output untouched. unused is ignored; always returns 1.
int gtl_CSoundEffectReverbStd_GetParameter(void* self, int parameter, int unused, void* result)
{
    float* values = gtl_nw4r_FxReverbStd_GetParams(((SoundEffectReverbStdState*)self)->effect);
    switch (parameter) {
    case kReverbPreDelay: *(float*)result = values[kReverbPreDelay]; break;
    case kReverbDecayTime: *(float*)result = values[kReverbDecayTime]; break;
    case kReverbColoration: *(float*)result = values[kReverbColoration]; break;
    case kReverbDamping: *(float*)result = values[kReverbDamping]; break;
    case kReverbWetMix: *(float*)result = values[kReverbWetMix]; break;
    case kReverbAllParameters: memcpy(*(void**)result, values, kReverbParameterBlockBytes); break;
    }
    return 1;
}

} // extern "C"

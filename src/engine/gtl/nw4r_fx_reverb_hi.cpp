// High-quality reverb effect: processes the left, right and surround sample buffers while
// the effect is enabled.
//
// gtl_nw4r_FxReverbHi_ProcessBuffers passes the buffer addresses it is given to the SDK
// reverb callback. It does not copy the samples.

#include "sdk_protos.h"

namespace gtl {
namespace nw4r {
struct FxReverbHi {
    char effectBase[0xC];
    unsigned char enabled;
    char outputAndParameters[0x23];
    char reverb[1];
};
}
}

struct ReverbBuffers {
    // Three borrowed sample-buffer addresses, not samples or channel lengths.
    unsigned int left;
    unsigned int right;
    unsigned int surround;
};

extern "C" {

void AXFXReverbHiCallback(ReverbBuffers*, void*);

// Process the left, right and surround buffers through an enabled high-quality reverb.
// self owns the embedded SDK state at +0x30. buffers borrows three addresses for
// this call; the SDK modifies their sample data in place. Disabled calls do nothing.
// Volatile descriptor reads and their order are intentional; the local descriptor
// is temporary and does not own or copy the sample arrays.
void gtl_nw4r_FxReverbHi_ProcessBuffers(gtl::nw4r::FxReverbHi* self, void*, const volatile ReverbBuffers* buffers) {
    if (self->enabled) {
        volatile ReverbBuffers channelAddresses;
        channelAddresses.left = buffers->left;
        channelAddresses.right = buffers->right;
        channelAddresses.surround = buffers->surround;
        AXFXReverbHiCallback((ReverbBuffers*)&channelAddresses, self->reverb);
    }
}

} // extern "C"

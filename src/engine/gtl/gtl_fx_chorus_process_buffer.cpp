// Audio chorus callback adapter. Parameters/SDK initialization: gtl_voice_pool_basic_player_chorus.cpp,
// GtlFxChorus.h; processing: Chorus_ProcessThreeChannelBuffers (SDK thunk).
// There are no effect tuning constants here; delay/depth/rate belong to SetParams.
#include "classes/GtlFxChorus.h"
// Translation unit 0x8040A124-0x8040A174 (1 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

struct ChorusBuffer {
    // Three Wii bus-lane addresses, in the caller/SDK's lane order 0,1,2.
    // Each addresses 96 signed 32-bit samples (384 writable bytes per lane).
    // This callback forwards the bus order; it does not choose speaker routing.
    unsigned int channels[3];
};

extern "C" {

void Chorus_ProcessThreeChannelBuffers(ChorusBuffer*, void*);

// Process the three channel buffers after an SDK initialization attempt.
// buffer is a borrowed bus descriptor. The SDK processes it in place by default;
// its state can instead select alternate input/output buses (+0x90/+0x94).
// self owns the SDK state, while the stack descriptor owns no sample memory.
// initializationAttempted==0 skips processing. gtl_voice_pool_basic_player_chorus.cpp sets it even when
// AXFXChorusInit fails: do not replace this with a success/enabled test.
// The unnamed int ABI argument is unused (not a sample count). Descriptor holds
// three 32-bit Wii addresses, read once in order before calling the SDK.
// The SDK thunk at 0x8035E09C branches to 0x8035E4B4: that routine loops over
// 96 samples and three lanes, loads signed 32-bit samples and stores integer
// results. Thus every nonnull bus lane must cover 96 words, not 96 bytes.
// No sample rate is passed here; do not treat the unused argument as a rate.
// All addressed buffers and chorusState must remain valid through the call;
// serialize processing with initialization, parameter changes and destruction.
// This adapter allocates nothing and returns no status. Buffer reuse/free must
// wait for callback completion on the audio processing thread. It installs no
// locks; the owner must prevent concurrent parameter/reset/destruction calls.
void gtl_nw4r_FxChorus_ProcessBuffer(GtlFxChorus* self, int, const volatile ChorusBuffer* buffer) {
    if (self->initializationAttempted) {
        volatile ChorusBuffer channelBuffers;
        channelBuffers.channels[0] = buffer->channels[0];
        channelBuffers.channels[1] = buffer->channels[1];
        channelBuffers.channels[2] = buffer->channels[2];
        // The existing class view reserves the embedded AX chorus state at +0x24.
        Chorus_ProcessThreeChannelBuffers((ChorusBuffer*)&channelBuffers, self->chorusState);
    }
}

} // extern "C"

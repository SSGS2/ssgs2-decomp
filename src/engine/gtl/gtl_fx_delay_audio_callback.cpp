// AX audio delay callback bridge. API lookup: processing = vfunc_6 here;
// init/parameters = gtl_nw4r_fx_delay_chorus.cpp and gtl_fx_delay_audio_effect.cpp; SDK shutdown = gtl_fx_delay_shutdown.cpp.
// Layout: classes/GtlFxDelay.h; channel/block ABI: revolution/AX/AXAux.h
// (AX_STEREO_L/R/S, AX_SAMPLES_PER_FRAME=96, four bytes per sample).
// No frame-rate or gameplay timing setting is exposed here.
// Translation unit 0x8040A4F8-0x8040A548 (1 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

struct FxBuffer {
    unsigned int channel[3]; // Wii 32-bit addresses: left, right, surround; 96 s32 samples each.
};

#include "classes/GtlFxDelay.h"

extern "C" {

void AXFXDelayCallback(FxBuffer*, void*);

// Process one three-channel block if initialization has been ATTEMPTED. vfunc_4
// sets initializationAttempted even on AXFXDelayInit failure; SDK success is not
// checked here. A zero latch leaves all buffers/state untouched (buffers may then
// be null). self is always required; the middle ABI argument is ignored.
// When latched, buffers must point to three valid AX writable sample-buffer
// addresses in L/R/surround order, each spanning 96 signed 32-bit mix samples.
// Buffers remain borrowed for the synchronous SDK call; no pointer is retained by
// this wrapper. Caller provides SDK-valid delayState/work storage even after a
// failed init attempt; this function supplies no repair or failure status.
// Run in the caller's AX effect-processing context, with exclusive access to this
// effect and these buffers; serialize initialization, settings and shutdown with
// The volatile snapshot preserves three reads. The callback can alter audio in
// those buffers, but changes to its local address triplet do not reach the caller.
void gtl_nw4r_FxDelay_ProcessAudioBlock(GtlFxDelay* self, void*, const volatile FxBuffer* buffers) {
    if (self->initializationAttempted) {
        volatile FxBuffer callbackBuffers;
        callbackBuffers.channel[0] = buffers->channel[0];
        callbackBuffers.channel[1] = buffers->channel[1];
        callbackBuffers.channel[2] = buffers->channel[2];
        AXFXDelayCallback((FxBuffer*)&callbackBuffers, self->delayState);
    }
}

} // extern "C"

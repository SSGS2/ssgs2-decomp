// NW4R chorus lifecycle: release SDK work allocated by an initialization attempt.
// API map: gtl_voice_pool_basic_player_chorus.cpp vfunc_4 attempts AXFXChorusInit and sets the latch even
// on failure; gtl_fx_chorus_process_buffer.cpp vfunc_6 processes borrowed audio buffers while latched;
// this vfunc_5 consumes the latch. GtlFxChorus.h defines the embedded SDK state.
// Allocator map: gtl_audio_controller_effects.cpp installs/restores AXFX hooks and a global allocator
// context. Delay/depth/rate tuning belongs to the init/settings methods, not here.
#include "classes/GtlFxChorus.h"
// Translation unit 0x80409F8C-0x80409FEC (1 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".



extern "C" {

void gtl_AudioEffects_InstallAllocatorHooks(void*, unsigned int*, unsigned int*);
void AudioChorus_Shutdown(void*);
void gtl_AudioEffects_RestoreAllocatorHooks(void*, unsigned int, unsigned int);

// Consume an outstanding initialization attempt, including a failed attempt.
// Required self is borrowed. With a clear attempt latch this is a no-op. Otherwise
// clear it before SDK shutdown, release any SDK work through the installed hooks,
// then restore the saved process-wide hooks. No success result is returned.
// Caller must serialize this whole install/shutdown/restore interval against audio
// callbacks and all other effect hook mutations: these helpers take no lock, and
// restore clears the global context rather than restoring a nested prior context.
// chorusState is embedded storage; this neither deletes self nor outputBuffer.
void gtl_nw4r_FxChorus_Shutdown(GtlFxChorus* self) {
    if (self->initializationAttempted) {
        unsigned int savedAllocateHook;
        unsigned int savedFreeHook;
        // Consume the attempt before shutdown can invoke allocator callbacks.
        self->initializationAttempted = 0;
        gtl_AudioEffects_InstallAllocatorHooks(&self->initializationAttempted, &savedAllocateHook, &savedFreeHook);
        AudioChorus_Shutdown(&self->chorusState);
        gtl_AudioEffects_RestoreAllocatorHooks(&self->initializationAttempted, savedAllocateHook, savedFreeHook);
    }
}

} // extern "C"

// NW4R chorus lifecycle: release SDK work allocated by an initialization attempt.
// API map: gtl_nw4r_FxChorus_Initialize (gtl_voice_pool_basic_player_chorus.cpp) attempts AXFXChorusInit
// and sets the latch even on failure; gtl_nw4r_FxChorus_ProcessBuffer (gtl_fx_chorus_process_buffer.cpp)
// processes borrowed buffers while latched; this function consumes the latch. GtlFxChorus.h: SDK state.
// Allocator map: gtl_audio_controller_effects.cpp installs/restores AXFX hooks and a global allocator
// context. Delay/depth/rate tuning belongs to the init/settings methods, not here.
#include "classes/GtlFxChorus.h"

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

// Delay effect shutdown: releases the SDK delay's allocations for an initialized effect and clears
// its initialization latch, so a repeated call does nothing.
// The layout is GtlFxDelay (classes/GtlFxDelay.h).

#include "sdk_protos.h"
#include "classes/GtlFxDelay.h"

extern "C" {
void gtl_AudioEffects_InstallAllocatorHooks(void*, unsigned int*, unsigned int*);
void AXFXDelayShutdown(void*);
void gtl_AudioEffects_RestoreAllocatorHooks(void*, unsigned int, unsigned int);

// Shut down an initialized delay with the effect allocator hooks installed.
// self remains caller-owned; shutdown releases SDK-managed delay allocations only.
// initializationAttempted is an attempt latch: clear it before shutdown so repeated calls do
// no work. Both process-global allocator hooks are restored on normal return.
void gtl_nw4r_FxDelay_Shutdown(GtlFxDelay* self) {
    unsigned int savedAllocateHook;
    unsigned int savedFreeHook;
    if (self->initializationAttempted) {
        self->initializationAttempted = 0;
        gtl_AudioEffects_InstallAllocatorHooks(&self->initializationAttempted, &savedAllocateHook, &savedFreeHook);
        AXFXDelayShutdown(self->delayState);
        gtl_AudioEffects_RestoreAllocatorHooks(&self->initializationAttempted, savedAllocateHook, savedFreeHook);
    }
}

} // extern "C"

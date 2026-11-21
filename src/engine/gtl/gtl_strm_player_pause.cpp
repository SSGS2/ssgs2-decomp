// Translation unit 0x8040D32C-0x8040D400 (1 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

#include "sdk_protos.h"

extern "C" {
char* gtl_AudioWorker_GetInstance();
void StrmPlayer_SetPauseFlag(void*, unsigned int);
}

enum { kAudioWorkerMutexOffset = 0x354 };

struct LockBase {
    // Empty common guard base; preserves the existing unwind structure.
    ~LockBase() {}
};
struct Lock : LockBase {
    // Serialize access to stream-player state with the audio worker.
    Lock() { OSLockMutex((OSMutex*)(gtl_AudioWorker_GetInstance() + kAudioWorkerMutexOffset)); }
    // Release the worker mutex on normal return and exception unwinding.
    ~Lock() { OSUnlockMutex((OSMutex*)(gtl_AudioWorker_GetInstance() + kAudioWorkerMutexOffset)); }
};
struct InterruptLock : LockBase {
    int level;
    // Save the prior interrupt state, allowing nested interrupt guards.
    InterruptLock() : level(OSDisableInterrupts()) {}
    // Restore the saved state; do not unconditionally enable interrupts.
    ~InterruptLock() { OSRestoreInterrupts(level); }
};

// Partial view shared with gtl_strm_player_streaming.cpp's State::flags at +0xAC:
// active +0xAD = registered; paused +0xB0 = requested pause;
// applied +0xB2 = last voice pause; pending +0xB3 = waiting for reads.
// pending is a second reason to pause, not a queued change to the requested flag.

struct Player {
    char pad[0xad];
    unsigned char registered;
    char padAE[2];
    unsigned char pauseRequested;
    char padB1;
    unsigned char voicePaused;
    unsigned char waitingForReads;
    char padB4[0x4a0-0xb4];
    void* voice;
};

extern "C" {
// Store a pause request and update the borrowed voice only when effective pause changes.
// paused is normally 0/1. A pause also latches the read-wait flag; unpause alone
// does not clear it (streaming update clears it once pending reads have drained).
void gtl_nw4r_detail_StrmPlayer_SetPauseRequested(Player* self, int paused) {
    Lock lock;
    self->pauseRequested = paused;
    if (paused) self->waitingForReads = 1;
    if (self->registered) {
        // Hold interrupts off only for the effective-state comparison and voice update.
        InterruptLock interruptLock;
        unsigned int effectivePause = 0;
        if (self->pauseRequested) effectivePause = 1;
        if (self->waitingForReads) effectivePause = 1;
        if (effectivePause != self->voicePaused) {
            if (self->voice) StrmPlayer_SetPauseFlag(self->voice, effectivePause);
            self->voicePaused = effectivePause;
        }
    }
}

} // extern "C"

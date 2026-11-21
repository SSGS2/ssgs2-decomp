// Translation unit 0x8040E908-0x8040E9BC (1 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

#include "sdk_protos.h"

extern "C" void* gtl_nw4r_detail_StrmPlayer_ReleaseResources(void*);
extern "C" void IntrusiveList_UnlinkNode(void*, void*);
extern "C" void gtl_IntrusiveList_InsertBefore(void*, void*, void*);
extern "C" void* LockGuardBase_dtor(void*, int);

struct StreamLockBase {
    // Destroy this embedded helper without freeing the surrounding command.
    ~StreamLockBase() { LockGuardBase_dtor(this, 0); }
};

struct InterruptGuardBase {
    BOOL level;
    // Leave restoration to the derived guard, retaining the original EH shape.
    ~InterruptGuardBase() {}
};
struct InterruptGuard_u8040E908 : InterruptGuardBase {
    // Save the previous interrupt level and enter the list mutation section.
    InterruptGuard_u8040E908() { level = OSDisableInterrupts(); }
    // Restore the saved level on normal return or stack unwinding.
    ~InterruptGuard_u8040E908() { OSRestoreInterrupts(level); }
};

extern "C" {

// Only fields used by FinishAndRecycle are exposed; gaps retain the player ABI.
struct StreamPlayerCompletionState {
    char playbackState[0xAE];
    unsigned char loadComplete;
    char commandState[0x11];
    int pendingLoads;
    char decoderState[0x38];
    struct CommandList {
        unsigned int count;
        // These two words form the embedded circular-list sentinel; &first is
        // the end iterator, not a separately allocated command or payload.
        void* first;
        void* last;
    } freeCommands, activeCommands;
};
typedef char StreamPlayer_pendingLoads_offset[((int)&((StreamPlayerCompletionState*)0)->pendingLoads == 0xC0) ? 1 : -1];
typedef char StreamPlayer_freeCommands_offset[((int)&((StreamPlayerCompletionState*)0)->freeCommands == 0xFC) ? 1 : -1];
typedef char StreamPlayer_activeCommands_offset[((int)&((StreamPlayerCompletionState*)0)->activeCommands == 0x108) ? 1 : -1];

struct LoadCommand {
    void* vtable;
    StreamPlayerCompletionState* stream;
    int state;
    char requestState[8];
    StreamLockBase lock;
};
// Keep byte addressing for the embedded link so both list calls share its address.
static const int kLoadCommandLinkOffset = (int)&((LoadCommand*)0)->lock;
enum LoadCommandState { kLoadCountsTowardCompletion = 0 };

// Finish or cancel a stream load, then recycle its command while interrupts are disabled.
// self/stream must exist and the command must belong to activeCommands. Only the
// nullness of completedRequest is used: null releases stream resources, non-null
// decrements pendingLoads only for state 0. Other states just recycle the command.
// Neither the request nor command is freed; the player retains command ownership.
void gtl_StrmPlayerLoadCommand_FinishAndRecycle(LoadCommand* self, void* completedRequest) {
    // Phase 1: update load completion or release resources before entering the guard.
    if (completedRequest != 0) {
        if (self->state == kLoadCountsTowardCompletion) {
            int count = self->stream->pendingLoads - 1;
            self->stream->pendingLoads = count;
            if (count == 0) self->stream->loadComplete = 1;
        }
    } else {
        gtl_nw4r_detail_StrmPlayer_ReleaseResources(self->stream);
    }

    // Phase 2: atomically move the embedded link from active to free. InsertBefore
    // takes an iterator address (&position); position itself is the free sentinel.
    InterruptGuard_u8040E908 guard;
    IntrusiveList_UnlinkNode(&self->stream->activeCommands, (char*)self + kLoadCommandLinkOffset);
    void* list = &self->stream->freeCommands;
    void* position = &((StreamPlayerCompletionState::CommandList*)list)->first;
    gtl_IntrusiveList_InsertBefore(list, &position, (char*)self + kLoadCommandLinkOffset);
}

} // extern "C"

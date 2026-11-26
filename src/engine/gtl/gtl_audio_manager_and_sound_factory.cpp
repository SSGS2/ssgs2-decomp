// Audio manager singleton and sound factories: creates sound buffers, streams and sources,
// and the queues that track them. Also covers audio shutdown and the per-frame update.

#include "sdk_protos.h"
#include "classes/GtlSoundStreamInterface.h"
#include "classes/GtlSoundBuffer.h"
#include "classes/gtl_CSoundBufferSource.h"
#include "classes/SoundStreamSource.h"
#include "classes/gtl_ISoundResource.h"

extern "C" {

struct InterruptUnwindCleanup {
    BOOL level;
    bool active;
    // Save the interrupt level to restore if construction or queue work throws.
    explicit InterruptUnwindCleanup(BOOL saved) : level(saved), active(true) {}
    // Hand the saved level to explicit normal-path restoration and disarm this guard.
    BOOL release() {
        active = false;
        return level;
    }
    // Restore interrupts on exceptional exits only while this guard remains armed.
    ~InterruptUnwindCleanup() {
        if (active)
            OSRestoreInterrupts(level);
    }
};

extern unsigned int g_gtlAudioManager;
extern signed char s_audioStateInitialized;
extern unsigned int s_gtlEffectRegistry[2]; // 8-byte object (symbols.txt size:0x8)
extern signed char s_audioEffectRegistryInitialized;
extern unsigned int g_gtlEffectRegistry;
extern unsigned int kAudioPropertyDefaults[12];
extern unsigned int s_gtlAudioRuntime[24];
extern unsigned int s_gtlEffectRegistryDtorRegistration[4];
extern const float kFloat_1_GtlAudio;

void gtl_Audio_UpdateChannels(void*);
void* memset(void*, int, unsigned int);

// Runtime word map (s_gtlAudioRuntime is 24 32-bit words; these are not vtable slots):
// 0 active, 1 allocate(size), 2 release(pointer, manager), 3 buffer limit, 4 stream limit,
// 5 unresolved, 6 initialized to 2, 7 initialized to 0, 8 scalar initialized to 1.0,
// 9..20 copied property defaults, 21 buffer queue, 22 stream queue, 23 runtime memory.
// Word 23 owns the allocation passed to gtl_Audio_InitializeRuntime by
// gtl_Audio_InitializeState; shutdown releases it through word 2.
// Repeated singleton scaffolding initializes defaults once; it does not allocate queues.
// Allocation callbacks/queues must be installed by runtime initialization before factories run.
// Callback ABI: release(allocation, manager); the second argument is allocator context.
typedef void (*AudioReleaseCallback)(void* allocation, void* manager);
typedef void (*AudioDeletingDestructor)(void* object, int deleteStorage);
enum AudioRuntimeWord {
    kAudioAllocateWord = 1,
    kAudioReleaseWord = 2,
    kAudioBufferLimitWord = 3,
    kAudioStreamLimitWord = 4,
    kAudioBufferQueueWord = 21,
    kAudioStreamQueueWord = 22,
    kAudioRuntimeMemoryWord = 23
};
// Queue allocation: count at +0; an embedded circular sentinel starts at +4.
// QueueNode links are prev/next at +0/+4, value at +8. A queue retains an object
// reference; buffer/stream factories add one caller reference after linking the node.
// Nodes use the manager allocator/release callback; queue wrappers use operator delete.
// ListNode below is a DIFFERENT input-list layout (next/prev order), not this queue.
struct Allocator {
    void* unk0;
    void* (*allocate)(unsigned int);
};

struct RefObject {
    unsigned int refs;
    void* vtable;
};
struct QueueNode {
    QueueNode* prev;
    QueueNode* next;
    RefObject* value;
};
struct Queue {
    unsigned int count;
    QueueNode head;
};
struct Manager {
    unsigned int active;
    char pad[0x50];
    Queue* bufferQueue; // +0x54: owns tracked buffers.
    Queue* streamQueue; // +0x58: owns tracked streams.
    void* runtimeMemory;
};

void dtor_803FBAAC(void*);
void InterruptGuard_dtor(void*);
void GtlEffectRegistry_DeleteWrapper(void);
void gtl_EffectRegistry_Shutdown(void*);
void* gtl_AudioController_GetInstance(void);
void gtl_AudioController_BeginShutdownFade(void*);
void gtl_Audio_WaitForPendingUpdates(void);
void gtl_Audio_ShutdownRuntime(void);
void __dl__FPv(void*);

// Publish the lazy audio runtime and its default property block; does not allocate queues.
// Keep each call at its original site: constructors/callbacks can change the singleton.
static inline void EnsureAudioManagerDefaults() {
    if (g_gtlAudioManager == 0) {
        if (s_audioStateInitialized == 0) {
            unsigned int* src = kAudioPropertyDefaults;
            // Reverse declarations retain register allocation when this helper is inlined.
            unsigned int v11, v10, v9, v8, v7, v6, v5, v4, v3, v2, v1, v0;
            v0 = src[0];
            v1 = src[1];
            v2 = src[2];
            v3 = src[3];
            v4 = src[4];
            v5 = src[5];
            v6 = src[6];
            v7 = src[7];
            v8 = src[8];
            v9 = src[9];
            v10 = src[10];
            v11 = src[11];
            unsigned int* dst = s_gtlAudioRuntime;
            dst[0] = 0;
            dst[6] = 2;
            dst[7] = 0;
            *(float*)&dst[8] = kFloat_1_GtlAudio;
            dst[9] = v0;
            dst[10] = v1;
            dst[11] = v2;
            dst[12] = v3;
            dst[13] = v4;
            dst[14] = v5;
            dst[15] = v6;
            dst[16] = v7;
            dst[17] = v8;
            dst[18] = v9;
            dst[19] = v10;
            dst[20] = v11;
            dst[21] = 0;
            dst[22] = 0;
            dst[23] = 0;
            memset(dst + 1, 0, 0x14);
            s_audioStateInitialized = 1;
        }
        g_gtlAudioManager = (unsigned int)&s_gtlAudioRuntime;
    }
}

// The original takes no argument: it never reads r3 and works on the lazily
// built singleton g_gtlAudioManager (loaded into r31 at 0x803FC8E8 and used for
// +0x54/+0x58/+0x5C/+0x0 at 0x803FC95C, 0x803FCB7C, 0x803FCDB4, 0x803FCEAC).
// Callers (engine_startup_and_ani_sprite.cpp, system_reset_and_callbacks.cpp) pass nothing, so self was a stale r3.
// Force-destroy tracked buffers/streams, release queue storage, then stop the backend.
// PRECONDITION: clients and callbacks must have stopped using every tracked object.
// This is terminal invalidation: it drains ALL references, including references clients
// may still hold; it does not wait for client releases or prove they are quiescent.
// Interrupt masking protects queue mutation, not the lifetime of escaped client pointers.
// A caller retaining an object pointer after shutdown must treat it as invalid.
void gtl_Audio_Shutdown(void) {
    if (g_gtlAudioManager == 0) {
        if (s_audioStateInitialized == 0) {
            unsigned int* src = kAudioPropertyDefaults;
            unsigned int a0 = src[0], a1 = src[1], a2 = src[2], a3 = src[3];
            unsigned int a4 = src[4], a5 = src[5], a6 = src[6], a7 = src[7];
            unsigned int a8 = src[8], a9 = src[9], a10 = src[10], a11 = src[11];
            unsigned int* dst = s_gtlAudioRuntime;
            dst[0] = 0;
            dst[6] = 2;
            dst[7] = 0;
            *(float*)&dst[8] = kFloat_1_GtlAudio;
            dst[9] = a0;
            dst[10] = a1;
            dst[11] = a2;
            dst[12] = a3;
            dst[13] = a4;
            dst[14] = a5;
            dst[15] = a6;
            dst[16] = a7;
            dst[17] = a8;
            dst[18] = a9;
            dst[19] = a10;
            dst[20] = a11;
            dst[21] = dst[22] = dst[23] = 0;
            memset(dst + 1, 0, 0x14);
            s_audioStateInitialized = 1;
        }
        g_gtlAudioManager = (unsigned int)&s_gtlAudioRuntime;
    }
    Manager* self = (Manager*)g_gtlAudioManager;
    // Phase 1: flush channel state and shut down the effect registry.
    gtl_Audio_UpdateChannels(self);
    if (g_gtlEffectRegistry == 0) {
        if (s_audioEffectRegistryInitialized == 0) {
            // 0x803FC91C/0x803FC92C: both words of the 8-byte object are cleared
            s_gtlEffectRegistry[0] = 0;
            s_gtlEffectRegistry[1] = 0;
            __register_global_object(s_gtlEffectRegistry, (void*)GtlEffectRegistry_DeleteWrapper,
                                     (void*)s_gtlEffectRegistryDtorRegistration);
            s_audioEffectRegistryInitialized = 1;
        }
        g_gtlEffectRegistry = (unsigned int)s_gtlEffectRegistry;
    }
    gtl_EffectRegistry_Shutdown((void*)g_gtlEffectRegistry);
    // Phase 2: force-release buffer objects, then free their queue nodes and wrapper.
    BOOL level = OSDisableInterrupts();
    InterruptUnwindCleanup interruptCleanup0(level);
    Queue* queue = self->bufferQueue;
    for (QueueNode* n = queue->head.next; n != &queue->head; n = n->next) {
        RefObject* value = n->value;
        if (value) {
            int refs;
            do {
                refs = --value->refs;
                if (refs == 0) {
                    if (value)
                        ((AudioDeletingDestructor)((void**)value->vtable)[2])(value, 1);
                    refs = 0;
                } else
                    refs = value->refs;
            } while (refs > 0);
        }
        n->value = 0;
    }
    if (queue) {
        QueueNode* n = queue->head.next;
        if (n != &queue->head) {
            QueueNode* last = queue->head.prev;
            n->prev->next = last->next;
            last->next->prev = n->prev;
        }
        while (n != &queue->head) {
            QueueNode* next = n->next;
            EnsureAudioManagerDefaults();
            ((AudioReleaseCallback)((unsigned int*)g_gtlAudioManager)[kAudioReleaseWord])(n, (void*)g_gtlAudioManager);
            --queue->count;
            n = next;
        }
        __dl__FPv(queue);
    }
    self->bufferQueue = 0;
    // Phase 3: perform the same terminal invalidation for tracked streams.
    queue = self->streamQueue;
    for (QueueNode* n = queue->head.next; n != &queue->head; n = n->next) {
        RefObject* value = n->value;
        if (value) {
            int refs;
            do {
                refs = --value->refs;
                if (refs == 0) {
                    if (value)
                        ((AudioDeletingDestructor)((void**)value->vtable)[2])(value, 1);
                    refs = 0;
                } else
                    refs = value->refs;
            } while (refs > 0);
        }
        n->value = 0;
    }
    if (queue) {
        QueueNode* n = queue->head.next;
        if (n != &queue->head) {
            QueueNode* last = queue->head.prev;
            n->prev->next = last->next;
            last->next->prev = n->prev;
        }
        while (n != &queue->head) {
            QueueNode* next = n->next;
            EnsureAudioManagerDefaults();
            ((AudioReleaseCallback)((unsigned int*)g_gtlAudioManager)[kAudioReleaseWord])(n, (void*)g_gtlAudioManager);
            --queue->count;
            n = next;
        }
        __dl__FPv(queue);
    }
    self->streamQueue = 0;
    OSRestoreInterrupts(interruptCleanup0.release());
    // Phase 4: interrupts are restored before fade/update synchronization and backend shutdown.
    gtl_AudioController_BeginShutdownFade(gtl_AudioController_GetInstance());
    gtl_Audio_WaitForPendingUpdates();
    gtl_Audio_ShutdownRuntime();
    void* runtimeMemory = self->runtimeMemory;
    if (runtimeMemory != 0) {
        EnsureAudioManagerDefaults();
        ((AudioReleaseCallback)((unsigned int*)g_gtlAudioManager)[kAudioReleaseWord])(runtimeMemory,
                                                                                      (void*)g_gtlAudioManager);
        self->runtimeMemory = 0;
    }
    self->active = 0;
}

// Ensure default singleton storage exists, then process channel updates.
void gtl_Audio_Update(void) {
    EnsureAudioManagerDefaults();
    gtl_Audio_UpdateChannels((void*)g_gtlAudioManager);
}

// Create a sound-resource wrapper using the manager allocator (16 bytes).
// out is required and always written. Return is always 1, even when allocation yields
// null; callers must inspect *out. arg is borrowed during construction; any retention
// by the external constructor is unresolved here. This factory does not enqueue it.
int gtl_Audio_CreateSoundResource(void* arg, void** out) {
    EnsureAudioManagerDefaults();
    EnsureAudioManagerDefaults();
    // Defaults are now available; allocate and construct the requested wrapper.
    void* p = ((Allocator*)g_gtlAudioManager)->allocate(16);
    if (p != 0)
        ((gtl::ISoundResource*)p)->Init(arg);
    *out = p;
    return 1;
}

#pragma opt_lifetimes off
// Create a buffer source using the manager allocator (0x2c bytes).
// out is required and always written. Return is always 1, even when allocation yields
// null; callers must inspect *out. arg is borrowed during construction; any retention
// by the external constructor is unresolved here. This factory does not enqueue it.
int gtl_Audio_CreateSoundBufferSource(void* arg, void** out) {
    EnsureAudioManagerDefaults();
    EnsureAudioManagerDefaults();
    // Defaults are now available; allocate and construct the requested wrapper.
    void* p = ((Allocator*)g_gtlAudioManager)->allocate(0x2c);
    if (p != 0)
        ((gtl::CSoundBufferSource*)p)
            ->Init((const SoundCopyData*)*(void**)arg, (const SoundRefData*)*((void**)arg + 1));
    *out = p;
    return 1;
}

// Create a stream source using the manager allocator (0xa0 bytes).
// out is required and always written. Return is always 1, even when allocation yields
// null; callers must inspect *out. arg is borrowed during construction; any retention
// by the external constructor is unresolved here. This factory does not enqueue it.
int gtl_Audio_CreateSoundStreamSource(void* arg, void** out) {
    EnsureAudioManagerDefaults();
    EnsureAudioManagerDefaults();
    // Defaults are now available; allocate and construct the requested wrapper.
    void* p = ((Allocator*)g_gtlAudioManager)->allocate(0xa0);
    if (p != 0)
        ((gtl::CSoundStreamSource*)p)
            ->Init((const SoundStreamSourceInit*)*(void**)arg, (const unsigned int*)*((void**)arg + 1));
    *out = p;
    return 1;
}
#pragma opt_lifetimes reset

struct ListNode {
    ListNode* next;
    ListNode* prev;
    RefObject* value;
};

// Create a tracked buffer from the borrowed source-list view.
// Return 0 at queue capacity and leave *out untouched. Return 1 after linking and
// adding the caller reference; callers must eventually release that reference.
// Successful path assumes both object and node allocations succeed (no recovery here).
int gtl_Audio_CreateSoundBuffer(ListNode* list, void** out) {
    if (g_gtlAudioManager == 0) {
        if (s_audioStateInitialized == 0) {
            unsigned int v0 = kAudioPropertyDefaults[0];
            unsigned int v1 = kAudioPropertyDefaults[1];
            unsigned int v2 = kAudioPropertyDefaults[2];
            unsigned int v3 = kAudioPropertyDefaults[3];
            unsigned int v4 = kAudioPropertyDefaults[4];
            unsigned int v5 = kAudioPropertyDefaults[5];
            unsigned int v6 = kAudioPropertyDefaults[6];
            unsigned int v7 = kAudioPropertyDefaults[7];
            unsigned int v8 = kAudioPropertyDefaults[8];
            unsigned int v9 = kAudioPropertyDefaults[9];
            unsigned int v10 = kAudioPropertyDefaults[10];
            unsigned int v11 = kAudioPropertyDefaults[11];
            unsigned int* dst = s_gtlAudioRuntime;
            dst[0] = 0;
            dst[6] = 2;
            dst[7] = 0;
            *(float*)&dst[8] = kFloat_1_GtlAudio;
            dst[9] = v0;
            dst[10] = v1;
            dst[11] = v2;
            dst[12] = v3;
            dst[13] = v4;
            dst[14] = v5;
            dst[15] = v6;
            dst[16] = v7;
            dst[17] = v8;
            dst[18] = v9;
            dst[19] = v10;
            dst[20] = v11;
            dst[21] = 0;
            dst[22] = 0;
            dst[23] = 0;
            memset(dst + 1, 0, 0x14);
            s_audioStateInitialized = 1;
        }
        g_gtlAudioManager = (unsigned int)&s_gtlAudioRuntime;
    }

    // Phase 1: mask interrupts and reserve against this queue's object limit.
    unsigned int* managerWords = (unsigned int*)g_gtlAudioManager;
    BOOL level = OSDisableInterrupts();
    InterruptUnwindCleanup interruptCleanup0(level);
    unsigned int* count = (unsigned int*)managerWords[kAudioBufferQueueWord];
    if (*count >= managerWords[kAudioBufferLimitWord]) {
        OSRestoreInterrupts(interruptCleanup0.release());
        return 0;
    }

    if (g_gtlAudioManager == 0) {
        if (s_audioStateInitialized == 0) {
            unsigned int v0 = kAudioPropertyDefaults[0];
            unsigned int v1 = kAudioPropertyDefaults[1];
            unsigned int v2 = kAudioPropertyDefaults[2];
            unsigned int v3 = kAudioPropertyDefaults[3];
            unsigned int v4 = kAudioPropertyDefaults[4];
            unsigned int v5 = kAudioPropertyDefaults[5];
            unsigned int v6 = kAudioPropertyDefaults[6];
            unsigned int v7 = kAudioPropertyDefaults[7];
            unsigned int v8 = kAudioPropertyDefaults[8];
            unsigned int v9 = kAudioPropertyDefaults[9];
            unsigned int v10 = kAudioPropertyDefaults[10];
            unsigned int v11 = kAudioPropertyDefaults[11];
            unsigned int* dst = s_gtlAudioRuntime;
            dst[0] = 0;
            dst[6] = 2;
            dst[7] = 0;
            *(float*)&dst[8] = kFloat_1_GtlAudio;
            dst[9] = v0;
            dst[10] = v1;
            dst[11] = v2;
            dst[12] = v3;
            dst[13] = v4;
            dst[14] = v5;
            dst[15] = v6;
            dst[16] = v7;
            dst[17] = v8;
            dst[18] = v9;
            dst[19] = v10;
            dst[20] = v11;
            dst[21] = 0;
            dst[22] = 0;
            dst[23] = 0;
            memset(dst + 1, 0, 0x14);
            s_audioStateInitialized = 1;
        }
        g_gtlAudioManager = (unsigned int)&s_gtlAudioRuntime;
    }
    // Phase 2: construct the object before inserting it into the manager-owned queue.
    managerWords = (unsigned int*)g_gtlAudioManager;
    unsigned int* object = (unsigned int*)((Allocator*)managerWords)->allocate(12);
    if (object != 0)
        ((gtl::ISoundBuffer*)object)->Init((unsigned int*)list);

    unsigned int* queue = (unsigned int*)managerWords[kAudioBufferQueueWord];
    QueueNode* head = (QueueNode*)(queue + 1);
    EnsureAudioManagerDefaults();
    QueueNode* node = (QueueNode*)((Allocator*)g_gtlAudioManager)->allocate(12);
    if (node != 0)
        node->value = (RefObject*)object;
    node->prev = head->prev;
    head->prev->next = node;
    head->prev = node;
    node->next = head;
    ++queue[0];

    // Phase 3: publish the object and retain one reference for the caller.
    *out = object;
    ++object[0];
    OSRestoreInterrupts(interruptCleanup0.release());
    return 1;
}

// Create a tracked stream from the borrowed source-list view.
// Return 0 at queue capacity and leave *out untouched. Return 1 after linking and
// adding the caller reference; callers must eventually release that reference.
// Successful path assumes both object and node allocations succeed (no recovery here).
int gtl_Audio_CreateSoundStream(ListNode* list, void** out) {
    if (g_gtlAudioManager == 0) {
        if (s_audioStateInitialized == 0) {
            unsigned int v0 = kAudioPropertyDefaults[0];
            unsigned int v1 = kAudioPropertyDefaults[1];
            unsigned int v2 = kAudioPropertyDefaults[2];
            unsigned int v3 = kAudioPropertyDefaults[3];
            unsigned int v4 = kAudioPropertyDefaults[4];
            unsigned int v5 = kAudioPropertyDefaults[5];
            unsigned int v6 = kAudioPropertyDefaults[6];
            unsigned int v7 = kAudioPropertyDefaults[7];
            unsigned int v8 = kAudioPropertyDefaults[8];
            unsigned int v9 = kAudioPropertyDefaults[9];
            unsigned int v10 = kAudioPropertyDefaults[10];
            unsigned int v11 = kAudioPropertyDefaults[11];
            unsigned int* dst = s_gtlAudioRuntime;
            dst[0] = 0;
            dst[6] = 2;
            dst[7] = 0;
            *(float*)&dst[8] = kFloat_1_GtlAudio;
            dst[9] = v0;
            dst[10] = v1;
            dst[11] = v2;
            dst[12] = v3;
            dst[13] = v4;
            dst[14] = v5;
            dst[15] = v6;
            dst[16] = v7;
            dst[17] = v8;
            dst[18] = v9;
            dst[19] = v10;
            dst[20] = v11;
            dst[21] = 0;
            dst[22] = 0;
            dst[23] = 0;
            memset(dst + 1, 0, 0x14);
            s_audioStateInitialized = 1;
        }
        g_gtlAudioManager = (unsigned int)&s_gtlAudioRuntime;
    }

    // Phase 1: mask interrupts and reserve against this queue's object limit.
    unsigned int* managerWords = (unsigned int*)g_gtlAudioManager;
    BOOL level = OSDisableInterrupts();
    InterruptUnwindCleanup interruptCleanup0(level);
    unsigned int* count = *(unsigned int**)(managerWords + kAudioStreamQueueWord);
    if (*count >= managerWords[kAudioStreamLimitWord]) {
        OSRestoreInterrupts(interruptCleanup0.release());
        return 0;
    }

    if (g_gtlAudioManager == 0) {
        if (s_audioStateInitialized == 0) {
            unsigned int v0 = kAudioPropertyDefaults[0];
            unsigned int v1 = kAudioPropertyDefaults[1];
            unsigned int v2 = kAudioPropertyDefaults[2];
            unsigned int v3 = kAudioPropertyDefaults[3];
            unsigned int v4 = kAudioPropertyDefaults[4];
            unsigned int v5 = kAudioPropertyDefaults[5];
            unsigned int v6 = kAudioPropertyDefaults[6];
            unsigned int v7 = kAudioPropertyDefaults[7];
            unsigned int v8 = kAudioPropertyDefaults[8];
            unsigned int v9 = kAudioPropertyDefaults[9];
            unsigned int v10 = kAudioPropertyDefaults[10];
            unsigned int v11 = kAudioPropertyDefaults[11];
            unsigned int* dst = s_gtlAudioRuntime;
            dst[0] = 0;
            dst[6] = 2;
            dst[7] = 0;
            *(float*)&dst[8] = kFloat_1_GtlAudio;
            dst[9] = v0;
            dst[10] = v1;
            dst[11] = v2;
            dst[12] = v3;
            dst[13] = v4;
            dst[14] = v5;
            dst[15] = v6;
            dst[16] = v7;
            dst[17] = v8;
            dst[18] = v9;
            dst[19] = v10;
            dst[20] = v11;
            dst[21] = dst[22] = dst[23] = 0;
            memset(dst + 1, 0, 0x14);
            s_audioStateInitialized = 1;
        }
        g_gtlAudioManager = (unsigned int)&s_gtlAudioRuntime;
    }
    // Phase 2: construct the object before inserting it into the manager-owned queue.
    managerWords = (unsigned int*)g_gtlAudioManager;
    void* object = ((Allocator*)managerWords)->allocate(12);
    if (object != 0)
        ((gtl::ISoundStream*)object)->Init(list);

    if (g_gtlAudioManager == 0) {
        if (s_audioStateInitialized == 0) {
            unsigned int v0 = kAudioPropertyDefaults[0];
            unsigned int v1 = kAudioPropertyDefaults[1];
            unsigned int v2 = kAudioPropertyDefaults[2];
            unsigned int v3 = kAudioPropertyDefaults[3];
            unsigned int v4 = kAudioPropertyDefaults[4];
            unsigned int v5 = kAudioPropertyDefaults[5];
            unsigned int v6 = kAudioPropertyDefaults[6];
            unsigned int v7 = kAudioPropertyDefaults[7];
            unsigned int v8 = kAudioPropertyDefaults[8];
            unsigned int v9 = kAudioPropertyDefaults[9];
            unsigned int v10 = kAudioPropertyDefaults[10];
            unsigned int v11 = kAudioPropertyDefaults[11];
            unsigned int* dst = s_gtlAudioRuntime;
            dst[0] = 0;
            dst[6] = 2;
            dst[7] = 0;
            *(float*)&dst[8] = kFloat_1_GtlAudio;
            dst[9] = v0;
            dst[10] = v1;
            dst[11] = v2;
            dst[12] = v3;
            dst[13] = v4;
            dst[14] = v5;
            dst[15] = v6;
            dst[16] = v7;
            dst[17] = v8;
            dst[18] = v9;
            dst[19] = v10;
            dst[20] = v11;
            dst[21] = dst[22] = dst[23] = 0;
            memset(dst + 1, 0, 0x14);
            s_audioStateInitialized = 1;
        }
        g_gtlAudioManager = (unsigned int)&s_gtlAudioRuntime;
    }
    managerWords = (unsigned int*)g_gtlAudioManager;
    QueueNode* node = (QueueNode*)((Allocator*)managerWords)->allocate(12);
    QueueNode* head = (QueueNode*)((char*)count + 4);
    if (node != 0)
        node->value = (RefObject*)object;
    node->prev = head->prev;
    head->prev->next = node;
    head->prev = node;
    node->next = head;
    ++*count;
    // Phase 3: publish the object and retain one reference for the caller.
    *out = object;
    ++((RefObject*)object)->refs;
    OSRestoreInterrupts(interruptCleanup0.release());
    return 1;
}

} // extern "C"

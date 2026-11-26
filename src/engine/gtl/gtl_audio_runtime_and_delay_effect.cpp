// Audio runtime state: initialization, the buffer and stream lists and the channel update.
// Also the delay sound effect (CSoundEffectDelay) and the effect registry wrapper.

#include <decomp/static_init.h>
#include "classes/GtlSoundStreamInterface.h"

#include "sdk_protos.h"
#include "classes/GtlAudioRuntimeState.h"
#include "classes/SoundEffectFields803FB370.h"
#include "classes/GtlSoundBuffer.h"
#include "classes/SoundEffectDelayState.h"

// The intrusive sentinel occupies only its two links; the value belongs to real nodes.
struct U803List;
struct AudioRuntimeChannels {
    char reserved_000[0x14];
    void* modulationContext;
    char reserved_018[0x3C];
    U803List* buffers;
    U803List* streams;
    void* runtimeMemory;
};
struct ChannelValueProbe {
    unsigned int reserved;
    void** key;
};
static const int kDelayEffectObjectBytes = 0x9C;
static const int kAudioListHeaderBytes = 12;
static const int kDefaultModulationContextWord = 0x21;
static const int kAudioDefaultOutputMode = 2;
static const int kAudioControllerDefaultMode = 1;
// Runtime option meanings beyond their initialization slots remain unconfirmed.
static const int kRuntimeOption0 = 4;
static const int kRuntimeOption2 = 3;
static const int kRuntimeSharedOption = 0x4000;
static const int kAudioCallbackCount = 5;
// Word-addressed runtime ABI. Property block is opaque: preserve all 12
// words in order; only callbacks/mode/mainSend and the three owned resources
// are identified. mainSend's physical meaning is unresolved.
// Named indices keep raw storage compatible with headers.
enum AudioRuntimeWord {
    kRuntimeInitialized = 0,
    kAllocateCallback = 1,
    kFreeCallback = 2,
    kCallback2 = 3,
    kCallback3 = 4,
    kModulationContext = 5,
    kOutputMode = 6,
    kCachedOutputOption = 7,
    kRuntimeMainSend = 8,
    kPropertyFirst = 9,
    kBufferList = 21,
    kStreamList = 22,
    kRuntimeMemory = 23
};
enum ChannelVirtualSlot {
    kChannelDeletingDestructor = 2
};
struct ChannelReferenceView {
    int references;
    void** vtable;
};

extern "C" {

struct InterruptUnwindCleanup {
    BOOL level;
    bool active;
    // Adopt the already-disabled interrupt state for exceptional cleanup.
    explicit InterruptUnwindCleanup(BOOL saved) : level(saved), active(true) {}
    // Transfer restoration responsibility to the explicit normal-path restore.
    BOOL release() {
        active = false;
        return level;
    }
    // Restore interrupts only while this guard still owns the saved state.
    ~InterruptUnwindCleanup() {
        if (active)
            OSRestoreInterrupts(level);
    }
};

void* memset(void* dst, int value, unsigned int size);

int gtl_Audio_InitializeState(void* object, void* self);
int gtl_EffectRegistry_Initialize(void* object, int arg1);
void GtlAudio_InitializeDefaultConstants();
void AIInit(int);
void AXInit();
void gtl_Audio_InitializeRuntime(void*, void*, void*);
void* gtl_AudioController_GetInstance();
void gtl_AudioController_SetOutputMode(void*, int);
void* __dl__FPv(void* ptr);
void* dtor_803FBAAC(void* ptr, int deleting);
void* gtl_Audio_Alloc(unsigned int size);
void gtl_nw4r_FxDelay_Ctor(void* self);
void gtl_SoundEffect_Initialize(void* self, void* member);
void* __nw__FUl(unsigned int size);

extern float kFloat_6_2831855;
extern float kAudioEffectDefault0;
extern float kAudioEffectDefault1;
extern float kAudioEffectDefault2;
extern float kAudioEffectDefault3;
extern float kAudioEffectDefault4;
extern float kAudioEffectDefault5;
extern float s_gtlAudioTau;
extern float s_audioEffectDefault0;
extern float s_audioEffectDefault1;
extern float s_audioEffectDefault2;
extern float s_audioEffectDefault3;
extern float s_audioEffectDefault4;
extern float s_audioEffectDefault5;
extern unsigned int g_gtlAudioManager;
extern signed char s_audioStateInitialized;
extern unsigned int s_gtlEffectRegistry[2];
extern signed char s_audioEffectRegistryInitialized;
extern unsigned int g_gtlEffectRegistry;
extern unsigned int kAudioPropertyDefaults[];
extern unsigned int s_gtlAudioRuntime[];
extern char s_gtlEffectRegistryDtorRegistration[];
extern char __vt__Q23gtl17CSoundEffectDelay[];
extern float kFloat_1_GtlAudio;
extern unsigned int kAudioRuntimeMemoryAllocationBytes;

} // extern "C"

namespace gtl {

// Return the allocation size of a delay-effect wrapper when an output pointer is supplied.
int CSoundEffectDelay::GetObjectSize(void* arg0, int* size) {
    if (size == 0) {
        return 0;
    }
    *size = kDelayEffectObjectBytes;
    return 1;
}

} // namespace gtl

extern "C" {

// Initialize the audio two-pi cache and copy the six effect defaults into writable storage.
void GtlAudio_InitializeDefaultConstants() {
    s_gtlAudioTau = kFloat_6_2831855;
    s_audioEffectDefault0 = kAudioEffectDefault0;
    s_audioEffectDefault1 = kAudioEffectDefault1;
    s_audioEffectDefault2 = kAudioEffectDefault2;
    s_audioEffectDefault3 = kAudioEffectDefault3;
    s_audioEffectDefault4 = kAudioEffectDefault4;
    s_audioEffectDefault5 = kAudioEffectDefault5;
}

// Free an empty audio wrapper when called as a deleting destructor.
void* dtor_803FBAAC(void* ptr, int deleting) {
    if (ptr != 0 && deleting > 0) {
        __dl__FPv(ptr);
    }
    return ptr;
}

// Allocate a delay-effect wrapper, construct its embedded effect, and return it through the output pointer.
// output is required; the caller receives the initial reference. Return 1 means
// the output argument was accepted, even if allocation failed and *output is null.
// arg0 is unused. The embedded effect is owned by the wrapper, not separately freed.
} // extern "C"

namespace gtl {

int CSoundEffectDelay::Create(void* arg0, CSoundEffectDelay** output) {
    if (output == 0) {
        return 0;
    }
    CSoundEffectDelay* obj = (CSoundEffectDelay*)gtl_Audio_Alloc(kDelayEffectObjectBytes);
    if (obj != 0) {
        obj->referenceCount = 1;
        obj->effectEnabled = 0;
        obj->registryIndex = -1;
        obj->auxiliaryStorage = 0;
        obj->vtable = __vt__Q23gtl17CSoundEffectDelay;
        void* member = obj->effect;
        gtl_nw4r_FxDelay_Ctor(member);
        gtl_SoundEffect_Initialize(obj, member);
    }
    *output = obj;
    return 1;
}

} // namespace gtl

extern "C" {

// Restore the saved interrupt state and optionally free the guard storage.
void* InterruptGuard_dtor(void* ptr, int deleting) {
    if (ptr != 0) {
        OSRestoreInterrupts(*(BOOL*)ptr);
        if (deleting > 0) {
            __dl__FPv(ptr);
        }
    }
    return ptr;
}

struct U803Link {
    U803Link* prev;
    U803Link* next;
    void* value;
};
struct U803List {
    unsigned int count;
    U803Link sentinel;
};
struct U803Pair {
    unsigned int pad;
    void* value;
};

struct U803Param {
    unsigned int a, b, c, d, e;
};
// rawArg contains five callback/context words, copied in order to object+4..+0x14.
// a/b are allocate/free callbacks; the roles of c/d are not established here.
// e is passed to both buffer and stream modulation updates, with 0 replaced by 0x21.
// A U803List allocation is only 12 bytes: count, sentinel.prev, sentinel.next.
// Only real nodes have a value; never read sentinel.value. Lists own one reference
// per channel and their nodes, but not any channel's separate client references.
struct U803Guard {
    BOOL level;
    // Disable interrupts for the entire channel cleanup/update traversal.
    U803Guard() : level(OSDisableInterrupts()) {}
    // Restore the previous interrupt state without deleting this stack guard.
    ~U803Guard() {
        InterruptGuard_dtor(this, 0);
    }
};

// Ensure static runtime defaults exist before reading an allocator callback.
// This only constructs the callback owner, not AI/AX or channel lists.
static inline void EnsureAudioAllocator() {
    if (g_gtlAudioManager == 0) {
        if (s_audioStateInitialized == 0) {
            unsigned int* entry = (unsigned int*)&kAudioPropertyDefaults[0];
            unsigned int* table = s_gtlAudioRuntime;
            table[kRuntimeInitialized] = 0;
            table[kOutputMode] = 2;
            table[kCachedOutputOption] = 0;
            ((float*)table)[kRuntimeMainSend] = kFloat_1_GtlAudio;
            // Declaration order preserves register assignment when this helper inlines.
            unsigned int a10, a9, a8, a7, a6, a5, a4, a3, a2, a1, a0, a11;
            a0 = *entry++;
            a1 = *entry++;
            a2 = *entry++;
            a3 = *entry++;
            a4 = *entry++;
            a5 = *entry++;
            a6 = *entry++;
            a7 = *entry++;
            a8 = *entry++;
            a9 = *entry++;
            a10 = *entry++;
            a11 = *entry++;
            table[kPropertyFirst + 0] = a0;
            table[kPropertyFirst + 1] = a1;
            table[kPropertyFirst + 2] = a2;
            table[kPropertyFirst + 3] = a3;
            table[kPropertyFirst + 4] = a4;
            table[kPropertyFirst + 5] = a5;
            table[kPropertyFirst + 6] = a6;
            table[kPropertyFirst + 7] = a7;
            table[kPropertyFirst + 8] = a8;
            table[kPropertyFirst + 9] = a9;
            table[kPropertyFirst + 10] = a10;
            table[kPropertyFirst + 11] = a11;
            table[kBufferList] = 0;
            table[kStreamList] = 0;
            table[kRuntimeMemory] = 0;
            memset(table + 1, 0, kAudioCallbackCount * sizeof(unsigned int));
            s_audioStateInitialized = 1;
        }
        g_gtlAudioManager = (unsigned int)s_gtlAudioRuntime;
    }
}

// Install callbacks and defaults, create channel lists, initialize AI/AX and configure runtime buffers.
int gtl_Audio_InitializeState(void* object, void* rawArg) {
    // Phase 1: copy caller callbacks/context, then the default property block.
    U803Param* initializationWords = (U803Param*)rawArg;
    unsigned int* runtimeWords = (unsigned int*)object;
    unsigned int modulationContextWord = initializationWords->e;
    unsigned int allocationCallbackWord = initializationWords->a;
    unsigned int freeCallbackWord = initializationWords->b;
    unsigned int callbackWord2 = initializationWords->c;
    unsigned int callbackWord3 = initializationWords->d;
    runtimeWords[kAllocateCallback] = allocationCallbackWord;
    runtimeWords[kFreeCallback] = freeCallbackWord;
    runtimeWords[kCallback2] = callbackWord2;
    runtimeWords[kCallback3] = callbackWord3;
    runtimeWords[kModulationContext] = modulationContextWord;
    if (runtimeWords[kModulationContext] == 0)
        runtimeWords[kModulationContext] = kDefaultModulationContextWord;

    unsigned int* src = kAudioPropertyDefaults;
    unsigned int a0 = src[0];
    unsigned int a1 = src[1];
    unsigned int a2 = src[2];
    unsigned int a3 = src[3];
    unsigned int a4 = src[4];
    unsigned int a5 = src[5];
    unsigned int a6 = src[6];
    unsigned int a7 = src[7];
    unsigned int a8 = src[8];
    unsigned int a9 = src[9];
    unsigned int a10 = src[10];
    unsigned int a11 = src[11];
    ((float*)object)[kRuntimeMainSend] = kFloat_1_GtlAudio;
    runtimeWords[kPropertyFirst + 0] = a0;
    runtimeWords[kPropertyFirst + 1] = a1;
    runtimeWords[kPropertyFirst + 2] = a2;
    runtimeWords[kPropertyFirst + 3] = a3;
    runtimeWords[kPropertyFirst + 4] = a4;
    runtimeWords[kPropertyFirst + 5] = a5;
    runtimeWords[kPropertyFirst + 6] = a6;
    runtimeWords[kPropertyFirst + 7] = a7;
    runtimeWords[kPropertyFirst + 8] = a8;
    runtimeWords[kPropertyFirst + 9] = a9;
    runtimeWords[kPropertyFirst + 10] = a10;
    runtimeWords[kPropertyFirst + 11] = a11;

    // Phase 2: create empty channel lists and initialize the hardware interfaces.
    U803List* bufferList = (U803List*)__nw__FUl(kAudioListHeaderBytes);
    if (bufferList) {
        bufferList->count = 0;
        bufferList->sentinel.next = &bufferList->sentinel;
        bufferList->sentinel.prev = &bufferList->sentinel;
    }
    ((AudioRuntimeChannels*)object)->buffers = bufferList;
    U803List* streamList = (U803List*)__nw__FUl(kAudioListHeaderBytes);
    if (streamList) {
        streamList->count = 0;
        streamList->sentinel.next = &streamList->sentinel;
        streamList->sentinel.prev = &streamList->sentinel;
    }
    ((AudioRuntimeChannels*)object)->streams = streamList;
    AIInit(0);
    AXInit();
    BOOL level = OSDisableInterrupts();
    InterruptUnwindCleanup interruptCleanup0(level);
    // Phase 3: ensure the callback owner exists, then allocate runtime memory
    // through its allocator while interrupts remain disabled.
    if (g_gtlAudioManager == 0) {
        if (s_audioStateInitialized == 0) {
            unsigned int* s = kAudioPropertyDefaults;
            unsigned int b0 = s[0], b1 = s[1], b2 = s[2], b3 = s[3], b4 = s[4], b5 = s[5];
            unsigned int b6 = s[6], b7 = s[7], b8 = s[8], b9 = s[9], b10 = s[10], b11 = s[11];
            unsigned int* t = s_gtlAudioRuntime;
            t[kRuntimeInitialized] = 0;
            t[kOutputMode] = 2;
            t[kCachedOutputOption] = 0;
            ((float*)t)[kRuntimeMainSend] = kFloat_1_GtlAudio;
            t[kPropertyFirst + 0] = b0;
            t[kPropertyFirst + 1] = b1;
            t[kPropertyFirst + 2] = b2;
            t[kPropertyFirst + 3] = b3;
            t[kPropertyFirst + 4] = b4;
            t[kPropertyFirst + 5] = b5;
            t[kPropertyFirst + 6] = b6;
            t[kPropertyFirst + 7] = b7;
            t[kPropertyFirst + 8] = b8;
            t[kPropertyFirst + 9] = b9;
            t[kPropertyFirst + 10] = b10;
            t[kPropertyFirst + 11] = b11;
            t[kBufferList] = 0;
            t[kStreamList] = 0;
            t[kRuntimeMemory] = 0;
            memset(t + 1, 0, kAudioCallbackCount * sizeof(unsigned int));
            s_audioStateInitialized = 1;
        }
        g_gtlAudioManager = (unsigned int)s_gtlAudioRuntime;
    }
    void* runtimeAllocationBytes = (void*)kAudioRuntimeMemoryAllocationBytes;
    void* runtimeMemory =
        ((void* (*)(void*))((void*)AsGtlAudioRuntimeState(g_gtlAudioManager)->allocate))(runtimeAllocationBytes);
    ((AudioRuntimeChannels*)object)->runtimeMemory = runtimeMemory;
    int settings[4];
    settings[0] = kRuntimeOption0;
    settings[1] = kRuntimeSharedOption;
    settings[2] = kRuntimeOption2;
    settings[3] = kRuntimeSharedOption;
    gtl_Audio_InitializeRuntime(settings, runtimeMemory, runtimeAllocationBytes);
    OSRestoreInterrupts(interruptCleanup0.release());
    // Phase 4: synchronize the runtime's 1-based mode with the controller's
    // 0-based mode; output mode 2 corresponds to controller mode 1.
    if (*(int*)&AsGtlAudioRuntimeState(object)->outputMode != kAudioDefaultOutputMode) {
        gtl_AudioController_SetOutputMode(gtl_AudioController_GetInstance(), kAudioControllerDefaultMode);
        *(int*)&AsGtlAudioRuntimeState(object)->outputMode = kAudioDefaultOutputMode;
    }
    if (*(int*)&AsGtlAudioRuntimeState(object)->cachedOutputOption != 0)
        *(int*)&AsGtlAudioRuntimeState(object)->cachedOutputOption = 0;
    return 1;
}

// Remove every node whose value is the key (list::remove): unlink each run of matches, free its nodes.
// Unlink and free matching channel nodes through the audio allocator, preserving the original scan order.
void GtlAudioChannelList_RemoveMatchingValue(U803List* list, void* probe) {
    // probe borrows a pointer-to-key at +4. Only matching nodes are freed here;
    // the caller must release any channel reference before removing its node.
    U803Link* end = &list->sentinel;
    U803Link* node = end->next;
    while (node != end) {
        void* key = *((ChannelValueProbe*)probe)->key;
        if (node->value == key) {
            U803Link* scan = node->next;
            while (scan != end && scan->value == key)
                scan = scan->next;
            node->prev->next = scan->prev->next;
            scan->prev = node->prev;
            while (node != scan) {
                U803Link* victim = node;
                node = node->next;
                if (g_gtlAudioManager == 0) {
                    if (s_audioStateInitialized == 0) {
                        unsigned int* src = kAudioPropertyDefaults;
                        unsigned int a0 = src[0], a1 = src[1], a2 = src[2], a3 = src[3];
                        unsigned int a4 = src[4], a5 = src[5], a6 = src[6], a7 = src[7];
                        unsigned int a8 = src[8], a9 = src[9], a10 = src[10], a11 = src[11];
                        unsigned int* table = s_gtlAudioRuntime;
                        table[kRuntimeInitialized] = 0;
                        table[kOutputMode] = 2;
                        table[kCachedOutputOption] = 0;
                        ((float*)table)[kRuntimeMainSend] = kFloat_1_GtlAudio;
                        table[kPropertyFirst + 0] = a0;
                        table[kPropertyFirst + 1] = a1;
                        table[kPropertyFirst + 2] = a2;
                        table[kPropertyFirst + 3] = a3;
                        table[kPropertyFirst + 4] = a4;
                        table[kPropertyFirst + 5] = a5;
                        table[kPropertyFirst + 6] = a6;
                        table[kPropertyFirst + 7] = a7;
                        table[kPropertyFirst + 8] = a8;
                        table[kPropertyFirst + 9] = a9;
                        table[kPropertyFirst + 10] = a10;
                        table[kPropertyFirst + 11] = a11;
                        table[kBufferList] = 0;
                        table[kStreamList] = 0;
                        table[kRuntimeMemory] = 0;
                        memset(table + 1, 0, kAudioCallbackCount * sizeof(unsigned int));
                        s_audioStateInitialized = 1;
                    }
                    g_gtlAudioManager = (unsigned int)s_gtlAudioRuntime;
                }
                // Only the node is freed here; channel ownership was handled by the caller.
                ((void (**)(void*))g_gtlAudioManager)[kFreeCallback](victim);
                --*(int*)list;
            }
            if (scan == end)
                break;
            node = scan;
        }
        node = node->next;
    }
}

// Release the list-owned reference only when no client references remain.
// Channel header: +0 signed reference count, +4 vptr; vtable+8 deletes.
// The temporary retain/release probes the count; refs==1 means only this
// list owns it. Release that last reference and leave a null list value
// for RemoveMatchingValue to erase. Nodes are not freed by this helper.
static inline void U803_ReleaseUnreferenced(U803List* list) {
    for (U803Link* p = list->sentinel.next; p != &list->sentinel; p = p->next) {
        ChannelReferenceView* item = (ChannelReferenceView*)p->value;
        if (item) {
            ++item->references;
            int refs = --item->references;
            if (refs == 0) {
                ((void (**)(void*, int))item->vtable)[kChannelDeletingDestructor](item, 1);
                refs = 0;
            }
            if (refs == 1) {
                item = (ChannelReferenceView*)p->value;
                if (--item->references == 0 && item)
                    ((void (**)(void*, int))item->vtable)[kChannelDeletingDestructor](item, 1);
                p->value = 0;
            }
        }
    }
}
// Remove unreferenced channels and update parameters and modulation for remaining buffers and streams.
// Requires both list headers from successful initialization, even when empty.
// Callbacks borrow channel/context pointers; the interrupt guard spans all calls.
void gtl_Audio_UpdateChannels(void* self) {
    U803Guard guard;
    void* member = ((AudioRuntimeChannels*)self)->modulationContext;
    // Phase 1: release channels with no external owners, then erase null nodes.
    U803_ReleaseUnreferenced(((AudioRuntimeChannels*)self)->buffers);
    int zero1 = 0;
    struct {
        int pad;
        int* key;
    } probe1 = {0, &zero1};
    GtlAudioChannelList_RemoveMatchingValue(((AudioRuntimeChannels*)self)->buffers, &probe1);
    U803_ReleaseUnreferenced(((AudioRuntimeChannels*)self)->streams);
    int zero2 = 0;
    struct {
        int pad;
        int* key;
    } probe2 = {0, &zero2};
    GtlAudioChannelList_RemoveMatchingValue(((AudioRuntimeChannels*)self)->streams, &probe2);
    // Phase 2: refresh surviving channels; the guard prevents list mutation by
    // interrupt handlers throughout both parameter and modulation callbacks.
    if (((AudioRuntimeChannels*)self)->buffers)
        for (U803Link* p = (((AudioRuntimeChannels*)self)->buffers)->sentinel.next;
             p != &(((AudioRuntimeChannels*)self)->buffers)->sentinel; p = p->next)
            if (p->value) {
                ((gtl::ISoundBuffer*)(p->value))->UpdateParameters();
                ((gtl::ISoundBuffer*)(p->value))->UpdateModulation((int)member);
            }
    if (((AudioRuntimeChannels*)self)->streams)
        for (U803Link* p = (((AudioRuntimeChannels*)self)->streams)->sentinel.next;
             p != &(((AudioRuntimeChannels*)self)->streams)->sentinel; p = p->next)
            if (p->value) {
                ((gtl::ISoundStream*)(p->value))->UpdateParameters();
                ((gtl::ISoundStream*)(p->value))->UpdateModulation(member);
            }
}

// Free this allocator-owned object through the lazily initialized audio free callback.
void dtor_803FC100(void* self) {
    EnsureAudioAllocator();
    void** vtable = (void**)g_gtlAudioManager;
    ((void (*)(void*))vtable[kFreeCallback])(self);
}

// Free this allocator-owned object through the lazily initialized audio free callback.
void dtor_803FC214(void* self) {
    EnsureAudioAllocator();
    void** vtable = (void**)g_gtlAudioManager;
    ((void (*)(void*))vtable[kFreeCallback])(self);
}

// Free this allocator-owned object through the lazily initialized audio free callback.
void dtor_803FC328(void* self) {
    EnsureAudioAllocator();
    void** vtable = (void**)g_gtlAudioManager;
    ((void (*)(void*))vtable[kFreeCallback])(self);
}

// Free this allocator-owned object through the lazily initialized audio free callback.
void dtor_803FC43C(void* self) {
    EnsureAudioAllocator();
    void** vtable = (void**)g_gtlAudioManager;
    ((void (*)(void*))vtable[kFreeCallback])(self);
}

// Free this allocator-owned object through the lazily initialized audio free callback.
void dtor_803FC550(void* self) {
    EnsureAudioAllocator();
    void** vtable = (void**)g_gtlAudioManager;
    ((void (*)(void*))vtable[kFreeCallback])(self);
}

// Free the effect-registry wrapper through the audio callback when deletion is requested.
void* GtlEffectRegistry_DeleteWrapper(void* self, int deletingFlag) {
    if (self != 0 && deletingFlag > 0) {
        EnsureAudioAllocator();
        void** vtable = (void**)g_gtlAudioManager;
        ((void (*)(void*))vtable[kFreeCallback])(self);
    }
    return self;
}

// Initialize the audio runtime and effect registry, returning false if either initialization fails.
int gtl_Audio_Initialize(void* self) {
    // Phase 1: lazily select the runtime singleton and initialize its callbacks/channels.
    EnsureAudioAllocator();
    void* object = (void*)g_gtlAudioManager;
    if (!gtl_Audio_InitializeState(object, self))
        return 0;
    // Phase 2: create/register the effect registry and publish initialized state
    // only if its initialization succeeds. Earlier runtime initialization is not rolled back.
    if (g_gtlEffectRegistry == 0) {
        if (s_audioEffectRegistryInitialized == 0) {
            s_gtlEffectRegistry[0] = 0;
            s_gtlEffectRegistry[1] = 0;
            __register_global_object(s_gtlEffectRegistry, (void*)GtlEffectRegistry_DeleteWrapper,
                                     s_gtlEffectRegistryDtorRegistration);
            s_audioEffectRegistryInitialized = 1;
        }
        g_gtlEffectRegistry = (unsigned int)s_gtlEffectRegistry;
    }
    if (!gtl_EffectRegistry_Initialize((void*)g_gtlEffectRegistry, 0))
        return 0;
    *(unsigned int*)object = 1;
    return 1;
}

} // extern "C"

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU80462684", _ctors_u803FB718_0, GtlAudio_InitializeDefaultConstants);
#pragma pop

// Balloon system: the three shared queues of balloons, their hit and reset messages, and the
// helpers that list and free queue nodes.

#include <decomp/static_init.h>
typedef unsigned long new_size_t;
extern "C" {
extern unsigned char s_spawnedBalloonList[];
}
#include "MSL/list.h"
// The balloon queues: std::list of object pointers with the game allocator.
typedef std::list<void*, app::app_allocator<void*> > BalloonList;
#include "classes/CBalloonSystem.h"
#include "classes/GameInputManager.h"

// Interpret a borrowed complete-object address using the shared balloon ABI view.
inline CBalloonSystem* AsCBalloonSystem(const void* object) {
    return (CBalloonSystem*)object;
}
// Interpret a borrowed input-manager address; no allocation or ownership change.
inline GameInputManager* AsGameInputManager(void* object) {
    return (GameInputManager*)object;
}

// Construct in caller-provided storage without allocating.
inline void* operator new(new_size_t, void* address) {
    return address;
}

extern "C" {

extern unsigned char s_balloonSystemTypeInfo;
extern float g_balloonSystemStaticTwoPi;
extern unsigned char s_balloonQueueStatics[];
extern unsigned int s_unhitBalloonUpdateCallback[];
extern unsigned int s_hitBalloonUpdateCallback[];
extern void __register_global_object(void*, void (*)(void*, int), void*);
extern void appFreeUni__FPv(void*);
extern void Balloon_PrepareForDestroy(void*);
extern void Balloon_SetModelVisibility(void*, int);
extern int Balloon_IsActive(void*);
// Collision arguments occupy separate GPR/FPR lanes: candidate r3, world-position address r4,
// radius f1. party_balloon_system.cpp spells the same ABI as (self, float radius, const void* position).
extern int Balloon_TestSphereCollision(void*, float, int);
extern void Balloon_SetCurrentIndex(void*, short);
extern int Balloon_GetRandomSlot(void*);
extern int Balloon_GetCurrentScaleIndex(void*);
extern float Balloon_GetScale(void*);
extern int Balloon_GetPosition(void*);
extern void Balloon_ProjectAndClampScreenPosition(float*, void*);
extern int GolfBall__Fv();
extern void* appMallocDummy__FUl(unsigned int);
extern void __dl__FPv(void*);
extern void CursorLayoutVector_Init(void*);
extern void CursorTrailVector_Init(void*);
extern void CursorLayoutVector_Resize(void*, unsigned int, float);
extern void CursorTrailVector_Resize(void*, unsigned int);
extern void* CursorSlotArray_Begin(void*);
extern void CursorSlotIterator_Copy(void**, void**);
extern void* PointerArray_GetCurrentSlot(void*);
extern int CursorTrailIterator_NotEqual(int*, int*);
extern void* CursorLayoutIterator_GetPointer(void*);
extern void CursorLayoutIterator_Advance(void*);
extern void* InputDeviceSlotArray_Begin(void*);
extern void InputDeviceSlotIterator_Copy(void**, void**);
extern void* PointerArray_GetCurrentSlot_1A30(void*);
extern int CursorLayoutIterator_NotEqual(int*, int*);
extern void* CursorTrailIterator_GetPointer(void*);
extern void CursorTrailIterator_Advance(void*);
extern void ColorRGBA_Copy(void*, const void*);
extern void KPADSetPosParam(int, float, float);
extern unsigned char cursorResourceTemplates[];
extern float kDefaultRemotePointerPlayRadius;  // 0.15: KPAD pointer play radius.
extern float kDefaultRemotePointerSensitivity; // 0.9: KPAD pointer sensitivity.
extern const float kFloat_0_BalloonInput;
struct ListNode {
    ListNode* prev;
    ListNode* next;
    void* value;
};
struct List {
    unsigned int count;
    ListNode sentinel;
};
// The shared static lists store only the sentinel links, never its unused value word.
struct StaticQueueHeader {
    unsigned int count;
    ListNode* last;
    ListNode* first;
};
enum BalloonQueueOffset {
    kOwnedBalloonQueue = 12,
    kUnhitBalloonQueue = 36,
    kHitBalloonQueue = 60
};

struct UpdateObject {
    unsigned char actorFields[0x18];
    int index;
    float x, y, z;
    int balloonsHitThisFrame;
};

extern float kBalloonBallCollisionRadius; // 5: ball-to-balloon collision radius.
extern unsigned char s_unhitBalloonQueue[];
extern unsigned char s_hitBalloonQueue[];
struct PmfObj {
    int dummy;
};
typedef void (PmfObj::*PmfFn)();

#pragma cpp_exceptions on
#pragma opt_propagation off
#pragma opt_strength_reduction off
#pragma opt_common_subs off
#pragma opt_loop_invariants off
#pragma opt_dead_code off
struct ProcessIterator {
    ListNode* volatile node;
};
// Get the intrusive-list sentinel while preserving the library iterator address calculation.
inline ListNode* process_end(List* list) {
    union {
        List* pointer;
        unsigned int address;
    } value;
    value.pointer = list;
    return (ListNode*)(value.address + (unsigned int)&((List*)0)->sentinel);
}

// Start an iterator at the first list node.
inline void process_begin(List* list, ProcessIterator& it) {
    ProcessIterator result;
    result.node = list->sentinel.next;
    it.node = list->sentinel.next;
}

// Unlink and free the current node, advancing the iterator to its successor.
inline void process_erase(List* list, ProcessIterator& it) {
    ProcessIterator result;
    ListNode* node = it.node;
    ListNode* next = node->next;
    node->prev->next = node->next;
    node->next->prev = node->prev;
    appFreeUni__FPv(node);
    result.node = next;
    it.node = next;
    list->count--;
}

// Unlink and free a list node and update the caller's cursor.
inline void list_erase(List* list, ListNode*& it) {
    ListNode* next = it->next;
    it->prev->next = next;
    next->prev = it->prev;
    appFreeUni__FPv(it);
    list->count--;
    it = next;
}

struct ProcessAllocation {
    ListNode* sentinel;
    ListNode* node;
    // Guard a newly allocated node until it has been linked into its destination queue.
    ProcessAllocation(ListNode* end, ListNode* value) : sentinel(end), node(value) {}
    // Free only an uncommitted node; the balloon payload is always borrowed here.
    ~ProcessAllocation() {
        if (node)
            appFreeUni__FPv(node);
    }
};

// Copy a list payload using the exception-aware allocator construction path.
inline void process_construct(void** destination, void* const& value) {
    if (destination) {
        void* copy = value;
        try {
            *destination = copy;
        } catch (...) {
            throw;
        }
    }
}

// Append a borrowed balloon pointer to a queue without transferring the balloon allocation.
inline ProcessIterator list_append(List* list, const ProcessIterator& sourceIterator) {
    volatile bool allocator = false;
    const ListNode* source = sourceIterator.node;
    ListNode* sentinel = process_end(list);
    ListNode* node = (ListNode*)appMallocDummy__FUl(12);
    ProcessIterator result;
    result.node = node;
    ProcessIterator allocated;
    allocated.node = node;
    ProcessAllocation allocation(sentinel, node);
    process_construct(&node->value, source->value);
    allocation.node = 0;
    sentinel->prev->next = node;
    node->prev = sentinel->prev;
    sentinel->prev = node;
    node->next = sentinel;
    list->count++;
    return result;
}

// Invoke the supplied update callback on every balloon in a queue.
inline void process_each(List* list, PmfFn function) {
    ListNode* end = &list->sentinel;
    for (ListNode* it = end->next; it != end; it = it->next)
        (((PmfObj*)it->value)->*function)();
}

// Move ball/chain collisions to the hit queue, record the last hit, and update both queues.
void CBalloonSystem::OnProcess(UpdateObject* object) {
// These aliases re-evaluate the global address under the original disabled optimizations.
#define hitBalloons (*(List*)s_hitBalloonQueue)
#define unhitBalloons (*(List*)s_unhitBalloonQueue)
    ProcessIterator chainSource;
    ProcessIterator collisionCandidate;
    object->balloonsHitThisFrame = 0;
    // Phase 1: retire inactive hit nodes; the owned queue still owns their balloon objects.
    {
        ListNode* end = &hitBalloons.sentinel;
        ProcessIterator it;
        process_begin(&hitBalloons, it);
        while (it.node != end) {
            if (Balloon_IsActive(it.node->value) == 0)
                process_erase(&hitBalloons, it);
            else
                it.node = it.node->next;
        }
    }

    // Phase 2: seed the chain with golf-ball collisions and retain the last valid screen hit.
    object->index = -1;
    {
        ListNode* end = &unhitBalloons.sentinel;
        ProcessIterator it;
        process_begin(&unhitBalloons, it);
        while (it.node != end) {
            if (Balloon_TestSphereCollision(it.node->value, kBalloonBallCollisionRadius, GolfBall__Fv()) == 1) {
                Balloon_SetCurrentIndex(it.node->value, 0);
                list_append(&hitBalloons, it);
                object->balloonsHitThisFrame++;
                void* balloon = it.node->value;
                int idx = Balloon_GetRandomSlot(balloon);
                if (idx != -1) {
                    float pos[3];
                    object->index = idx;
                    Balloon_ProjectAndClampScreenPosition(pos, balloon);
                    object->x = pos[0];
                    object->y = pos[1];
                    object->z = pos[2];
                }
                process_erase(&unhitBalloons, it);
            } else {
                it.node = it.node->next;
            }
        }
    }

    {
        // Phase 3: newly appended hits also become chain sources in this same traversal.
        // The collision API carries a position address in an int; it is not an ID/key.
        process_begin(&hitBalloons, chainSource);
        while (chainSource.node != &hitBalloons.sentinel) {
            process_begin(&unhitBalloons, collisionCandidate);
            while (collisionCandidate.node != &unhitBalloons.sentinel) {
                float collisionRadius = Balloon_GetScale(chainSource.node->value);
                int sourcePositionAddress = Balloon_GetPosition(chainSource.node->value);
                if (Balloon_TestSphereCollision(collisionCandidate.node->value, collisionRadius,
                                                sourcePositionAddress) == 1) {
                    int nextScaleIndex = Balloon_GetCurrentScaleIndex(chainSource.node->value) + 1;
                    Balloon_SetCurrentIndex(collisionCandidate.node->value, (short)nextScaleIndex);
                    list_append(&hitBalloons, collisionCandidate);
                    object->balloonsHitThisFrame++;
                    void* balloon = collisionCandidate.node->value;
                    int idx = Balloon_GetRandomSlot(balloon);
                    if (idx != -1) {
                        float pos[3];
                        object->index = idx;
                        Balloon_ProjectAndClampScreenPosition(pos, balloon);
                        object->x = pos[0];
                        object->y = pos[1];
                        object->z = pos[2];
                    }
                    process_erase(&unhitBalloons, collisionCandidate);
                } else {
                    collisionCandidate.node = collisionCandidate.node->next;
                }
            }
            chainSource.node = chainSource.node->next;
        }
    }

    // Phase 4: run each queue's own update callback after all node transfers are complete.
    if (unhitBalloons.count != 0 || hitBalloons.count != 0) {
        PmfFn pmfB = *(PmfFn*)s_unhitBalloonUpdateCallback;
        process_each(&unhitBalloons, pmfB);
        PmfFn pmfA = *(PmfFn*)s_hitBalloonUpdateCallback;
        process_each(&hitBalloons, pmfA);
    }
}
#undef hitBalloons
#undef unhitBalloons
#pragma cpp_exceptions reset
#pragma opt_propagation reset
#pragma opt_strength_reduction reset
#pragma opt_common_subs reset
#pragma opt_loop_invariants reset
#pragma opt_dead_code reset
// Free list nodes without deleting their balloon payloads.
void clear_list(List* list) {
    ListNode* node = list->sentinel.next;
    ListNode* sentinel = &list->sentinel;
    if (node != sentinel) {
        sentinel->prev->next = node->next;
        node->next->prev = sentinel->prev;
        while (node != sentinel) {
            ListNode* next = node->next;
            appFreeUni__FPv(node);
            --list->count;
            node = next;
        }
    }
}

// Invoke the object's virtual destructor (vtable slot 2) with the delete flag set.
static inline void CallDeletingDtor(void* object) {
    ((void (**)(void*, int))(*(void***)object))[2](object, 1);
}

// Unload and delete each balloon payload without removing list nodes.
void clear_resources(List* list) {
    ListNode* node = list->sentinel.next;
    ListNode* sentinel = &list->sentinel;
    while (node != sentinel) {
        Balloon_PrepareForDestroy(node->value);
        if (node->value) {
            CallDeletingDtor(node->value);
            node->value = 0;
        }
        node = node->next;
    }
}

// Unload and delete all remaining balloon payloads before queue teardown.
inline void DeleteQueuedBalloons(List* list) {
    ListNode* sentinel = &list->sentinel;
    ListNode* node = list->sentinel.next;
    while (node != sentinel) {
        Balloon_PrepareForDestroy(node->value);
        if (node->value) {
            if (node->value) {
                CallDeletingDtor(node->value);
            }
            node->value = 0;
        }
        node = node->next;
    }
}

// Free every node in the queue while preserving the original splice order.
inline void FreeQueueNodes(List* list) {
    ListNode* node = list->sentinel.next;
    ListNode* sentinel = &list->sentinel;
    if (node != sentinel) {
        ListNode* last = sentinel->prev;
        node->prev->next = last->next;
        last->next->prev = node->prev;
        while (node != sentinel) {
            ListNode* cur = node;
            node = node->next;
            appFreeUni__FPv(cur);
            --list->count;
        }
    }
}

// Release owned balloon resources and clear all three shared queues.
void CBalloonSystem::OnDestroy() {
    unsigned char* globals = s_balloonQueueStatics;
    DeleteQueuedBalloons((List*)(((char*)(globals + kOwnedBalloonQueue))));
    FreeQueueNodes((List*)(((char*)(globals + kOwnedBalloonQueue))));
    FreeQueueNodes((List*)(((char*)(globals + kUnhitBalloonQueue))));
    FreeQueueNodes((List*)(((char*)(globals + kHitBalloonQueue))));
    CBalloonSystem::DestroySharedQueue();
}

enum BalloonMessage {
    kGetLastBalloonHit = 0,
    kGetBalloonsHitThisFrame = 1,
    kGetHitBalloonCount = 2,
    kResetBalloonQueues = 3,
    kSetBalloonVisibility = 4
};
struct Event {
    int actorMessageHeader;
    int type;
    void* output;
};
// Message argument contract (32-bit ABI):
// 0: output -> {int slot, float projectedX, projectedY, projectedZ}; slot -1 means no valid slot.
// 1: output -> unsigned frame-hit count. 2: output -> unsigned HIT queue node count.
// 3: output ignored; destroys owned balloons and all queue nodes.
// 4: output is an integer boolean carried in the pointer word, never dereferenced.
// Query buffers are borrowed writable storage; this handler retains none of them.
struct EventData {
    unsigned int words[4];
};
struct MsgVec {
    float words[3];
};
struct MsgOutput {
    int index;
    MsgVec pos;
};
struct MsgPair {
    unsigned int y;
    unsigned long x;
};
struct MsgObject {
    char pad[0x18];
    int index;
    MsgVec pos;
    unsigned int count;
};
struct MsgResource {
    virtual ~MsgResource();
};

#pragma opt_propagation off
#pragma opt_strength_reduction off
#pragma opt_common_subs on
// Select a shared balloon queue using the original iterator address representation.
inline List* msg_list(unsigned char* globals, unsigned int offset) {
    union {
        unsigned int address;
        List* ptr;
    } value;
    value.ptr = (List*)(globals + offset);
    return (List*)value.address;
}
// Return the first node, or the sentinel when the queue is empty.
inline ListNode* msg_first(List* list) {
    return list->sentinel.next;
}
// Return the sentinel used to terminate message-handler iteration.
inline ListNode* msg_end(List* list) {
    return &list->sentinel;
}
// Unload and delete queue payloads during a reset message.
inline void msg_clear_resources(List* list) {
    ListNode* sentinel;
    int zero = 0;
    ListNode* node;
    node = msg_first(list);
    sentinel = msg_end(list);
    while (node != sentinel) {
        Balloon_PrepareForDestroy(node->value);
        if (node->value) {
            delete (MsgResource*)node->value;
            node->value = (void*)zero;
        }
        node = node->next;
    }
}

// Free queue nodes after a reset message has released their payloads.
inline void msg_clear_list(List* list) {
    ListNode* node = msg_first(list);
    ListNode* sentinel = msg_end(list);
    if (node != sentinel) {
        ListNode* last = sentinel->prev;
        node->prev->next = last->next;
        last->next->prev = node->prev;
        while (node != sentinel) {
            ListNode* cur = node;
            node = node->next;
            appFreeUni__FPv(cur);
            --list->count;
        }
    }
}

// Set every balloon's model visibility from the message value.
inline void msg_set_enabled(List* list, int value) {
    ListNode* node;
    int enabled;
    ListNode* sentinel;
    node = msg_first(list);
    sentinel = msg_end(list);
    enabled = value != 0;
    while (node != sentinel) {
        Balloon_SetModelVisibility(node->value, enabled);
        node = node->next;
    }
}

// Return hit information, reset shared queues, or change balloon visibility.
void CBalloonSystem::HandleMsg(const Event* event) {
    const MsgObject* object = (const MsgObject*)this;
    unsigned char* globals = s_balloonQueueStatics;
    switch (event->type) {
    case kGetLastBalloonHit: {
        MsgPair pair;
        volatile unsigned int* out = (volatile unsigned int*)event->output;
        const volatile unsigned long* src = (const volatile unsigned long*)(&object->index);
        out[0] = src[0];
        pair.x = src[1];
        pair.y = src[2];
        out[1] = pair.x;
        out[2] = pair.y;
        out[3] = src[3];
        break;
    }
    case kGetBalloonsHitThisFrame:
        *(unsigned int*)event->output = AsCBalloonSystem(object)->balloonsHitThisFrame;
        break;
    case kResetBalloonQueues:
        msg_clear_resources((List*)((char*)(msg_list(globals, kOwnedBalloonQueue))));
        msg_clear_list((List*)((char*)(msg_list(globals, kOwnedBalloonQueue))));
        msg_clear_list((List*)((char*)(msg_list(globals, kUnhitBalloonQueue))));
        msg_clear_list((List*)((char*)(msg_list(globals, kHitBalloonQueue))));
        CBalloonSystem::DestroySharedQueue();
        break;
    case kSetBalloonVisibility: {
        msg_set_enabled((List*)((char*)(msg_list(globals, kOwnedBalloonQueue))), (int)event->output);
        break;
    }
    // This is the hit queue, not the remaining/unhit count; do not change the queue to fit a label.
    case kGetHitBalloonCount:
        *(unsigned int*)event->output = ((StaticQueueHeader*)(((char*)(globals + kHitBalloonQueue))))->count;
        break;
    }
}
#pragma opt_common_subs reset
#pragma opt_propagation reset
#pragma opt_strength_reduction reset

// Return the balloon-system type registration record.
void* CBalloonSystem::GetRTTI() {
    return &s_balloonSystemTypeInfo;
}

// Destroy a balloon list (free every node); free the list object too when should_free > 0.
void* List_Dtor(BalloonList* self, int should_free) {
    return std::__list_deleting_dtor(self, should_free);
}

// Initialize three shared balloon queues and register their static destructors.
void BalloonQueues_RegisterStatics() {
    float tau = 6.2831854820251465f;
    unsigned char* globals = s_balloonQueueStatics;
    unsigned int zero = 0;
    StaticQueueHeader* head;
    g_balloonSystemStaticTwoPi = tau;
    head = (StaticQueueHeader*)(((char*)(globals + kOwnedBalloonQueue)));
    head->count = zero;
    head->last = (ListNode*)&head->last;
    head->first = (ListNode*)&head->last;
    __register_global_object(head, (void (*)(void*, int))List_Dtor, globals);
    head = (StaticQueueHeader*)(((char*)(globals + kUnhitBalloonQueue)));
    head->count = zero;
    head->last = (ListNode*)&head->last;
    head->first = (ListNode*)&head->last;
    __register_global_object(head, (void (*)(void*, int))List_Dtor, ((char*)(globals + 24)));
    head = (StaticQueueHeader*)(((char*)(globals + kHitBalloonQueue)));
    head->count = zero;
    head->last = (ListNode*)&head->last;
    head->first = (ListNode*)&head->last;
    __register_global_object(head, (void (*)(void*, int))List_Dtor, ((char*)(globals + 48)));
}

// Balloon objects draw themselves; the system has no additional display work.
void CBalloonSystem::OnDisplay() {}

// Default actor reload hook performs no work.
void IActor_OnReload() {}

struct ActorDisplayFields {
    void* vtable;
    int actorState;
    int displayPriority;
};

// Return the actor's stored display sorting priority.
int IActor_GetDisplayPriority(ActorDisplayFields* actor) {
    return actor->displayPriority;
}

// Copy a complete four-channel RGBA palette.
inline void CopyPalette16(unsigned char* dst, const unsigned char* src) {
    for (int i = 0; i < 16; i++)
        dst[i] = src[i];
}
#pragma opt_propagation off
#pragma opt_strength_reduction off
// Address a borrowed palette byte block while retaining the original address calculation.
inline unsigned char* PaletteTemplateAt(unsigned char* g, int offset) {
    union {
        unsigned int address;
        unsigned char* ptr;
    } value;
    value.ptr = g + offset;
    return (unsigned char*)value.address;
}
void CursorTrailVector_dtor(void*, int);
void CursorLayoutVector_dtor(void*, int);

enum {
    kWiiRemoteChannelCount = 4,
    kInputActionCount = 39,
    kActionWordsPerResetBatch = 7,
    kActionFullResetBatches = 5,
    kNoRumbleRequest = -1,
    kCursorPaletteBytes = 16
};
struct StartupPointerArray {
    void** entries;
    int size;
    int capacity;
};
struct StartupCursorMotion {
    int xBits;
    int yBits;
    int zBits;
};
struct StartupPadFlags {
    int state;
    unsigned char flags[3];
    unsigned char padding;
};
// Construction view for fields not yet exposed in GameInputManager.h.
// Scratch banks are kept separate because their later meanings are not all established.
struct InputStartupState {
    int selectedPlayer;
    StartupPointerArray cursorLayouts;
    StartupPointerArray inputDevices;
    int actionBlocked[kInputActionCount];
    int actionBlockedSecondary[kInputActionCount];
    int selectedPadLookupDisabled;
    int pendingActionPress[kInputActionCount];
    int actionPress[kInputActionCount];
    int rumbleFramesRemaining[kWiiRemoteChannelCount];
    int rumblePaused;
    float remotePointerPlayRadius;
    float remotePointerSensitivity;
    int reserved2AC;
    int cursorDrawingSuspendCount;
    unsigned char cursorBaseColors[kCursorPaletteBytes];
    unsigned char cursorActiveColors[kCursorPaletteBytes];
    unsigned char cursorAlternateColors[kCursorPaletteBytes];
    unsigned char cursorColors[kCursorPaletteBytes];
    StartupCursorMotion cursorMotion[kWiiRemoteChannelCount];
    StartupPadFlags padFlags[kWiiRemoteChannelCount];
    int cursorReservedA[6];
    int cursorReservedB[6];
    int cursorReservedC[kWiiRemoteChannelCount];
    int cursorReservedD[kWiiRemoteChannelCount];
    int cursorReservedE[kWiiRemoteChannelCount];
    int cursorReservedF[kWiiRemoteChannelCount];
    int cursorAppearance[kWiiRemoteChannelCount];
    int cursorMode;
    int cursorModeRequestCount;
    int channelReservedA[kWiiRemoteChannelCount];
    int channelReservedB[kWiiRemoteChannelCount];
    int channelReservedC[kWiiRemoteChannelCount];
    int pointerOverrideEnabled;
    float pointerOverrideX;
    float pointerOverrideY;
};
#define STARTUP_OFFSET(field, offset) \
    typedef char StartupOffset_##field[((int)&((InputStartupState*)0)->field == offset) ? 1 : -1]
STARTUP_OFFSET(remotePointerPlayRadius, 0x2A4);
STARTUP_OFFSET(cursorMotion, 0x2F4);
STARTUP_OFFSET(padFlags, 0x324);
STARTUP_OFFSET(cursorReservedA, 0x344);
STARTUP_OFFSET(channelReservedA, 0x3CC);
STARTUP_OFFSET(pointerOverrideY, 0x404);
#undef STARTUP_OFFSET

// Initialize four cursor/device slots, action latches, rumble state, and pointer palettes.
void* InputManager_Init(void* object) {
    int lateZero;
    unsigned char* defaultPalettes = cursorResourceTemplates;
    unsigned char* self = (unsigned char*)object;
    CursorLayoutVector_Init(&AsGameInputManager(self)->cursorLayouts);
    CursorTrailVector_Init(&AsGameInputManager(self)->cursorTrails);
    float remotePlayRadius = kDefaultRemotePointerPlayRadius;
    float remoteSensitivity = kDefaultRemotePointerSensitivity;
    ((InputStartupState*)self)->remotePointerPlayRadius = remotePlayRadius;
    ((InputStartupState*)self)->remotePointerSensitivity = remoteSensitivity;
    ((GameInputManager*)self)->cursorModeRequestCount = 0;
    try {
        CursorLayoutVector_Resize(&AsGameInputManager(self)->cursorLayouts, kWiiRemoteChannelCount, remotePlayRadius);
        CursorTrailVector_Resize(&AsGameInputManager(self)->cursorTrails, kWiiRemoteChannelCount);
        // Wii Remote integration: configure KPAD pointer filtering for every channel.
        // This call is the SDK boundary to change when replacing Remote pointer input.
        for (int i = 0; i < kWiiRemoteChannelCount; i++) {
            KPADSetPosParam(i, ((InputStartupState*)self)->remotePointerPlayRadius,
                            ((InputStartupState*)self)->remotePointerSensitivity);
        }
    } catch (...) {
        CursorTrailVector_dtor(&AsGameInputManager(self)->cursorTrails, -1);
        CursorLayoutVector_dtor(&AsGameInputManager(self)->cursorLayouts, -1);
        throw;
    }
    // Clear the newly allocated cursor-layout pointer slots.
    int cursorSlotIterator;
    int deviceSlotIterator;
    void* cursorSlotsBegin;
    void* cursorSlotsEnd;
    void* deviceSlotsBegin;
    void* deviceSlotsEnd;
    cursorSlotsBegin = CursorSlotArray_Begin(&AsGameInputManager(self)->cursorLayouts);
    CursorSlotIterator_Copy((void**)&cursorSlotIterator, &cursorSlotsBegin);
    while ((cursorSlotsEnd = PointerArray_GetCurrentSlot(&AsGameInputManager(self)->cursorLayouts),
            CursorTrailIterator_NotEqual(&cursorSlotIterator, (int*)&cursorSlotsEnd))) {
        *(int*)CursorLayoutIterator_GetPointer(&cursorSlotIterator) = 0;
        CursorLayoutIterator_Advance(&cursorSlotIterator);
    }
    // Clear the newly allocated input-device pointer slots.
    deviceSlotsBegin = InputDeviceSlotArray_Begin(&AsGameInputManager(self)->cursorTrails);
    InputDeviceSlotIterator_Copy((void**)&deviceSlotIterator, &deviceSlotsBegin);
    while ((deviceSlotsEnd = PointerArray_GetCurrentSlot_1A30(&AsGameInputManager(self)->cursorTrails),
            CursorLayoutIterator_NotEqual(&deviceSlotIterator, (int*)&deviceSlotsEnd))) {
        *(int*)CursorTrailIterator_GetPointer(&deviceSlotIterator) = 0;
        CursorTrailIterator_Advance(&deviceSlotIterator);
    }

    // Reset all four parallel action banks: 5 batches * 7 words, then 4 tail words = 39.
    // actionBatch is a displaced field-address base, not another GameInputManager object.
    // Each cast accesses originalBank[resetActionCount + localIndex]; keep the four banks aligned.
    unsigned char* actionBatch = self;
    int resetActionCount = 0;
    for (int i = 0; i < kActionFullResetBatches; i++) {
        ((GameInputManager*)actionBatch)->actionBlocked[0] = 0;
        resetActionCount += kActionWordsPerResetBatch;
        ((GameInputManager*)actionBatch)->actionBlockedSecondary[0] = 0;
        ((GameInputManager*)actionBatch)->pendingActionPress[0] = 0;
        ((GameInputManager*)actionBatch)->actionPress[0] = 0;
        ((GameInputManager*)actionBatch)->actionBlocked[1] = 0;
        ((GameInputManager*)actionBatch)->actionBlockedSecondary[1] = 0;
        ((GameInputManager*)actionBatch)->pendingActionPress[1] = 0;
        ((GameInputManager*)actionBatch)->actionPress[1] = 0;
        ((GameInputManager*)actionBatch)->actionBlocked[2] = 0;
        ((GameInputManager*)actionBatch)->actionBlockedSecondary[2] = 0;
        ((GameInputManager*)actionBatch)->pendingActionPress[2] = 0;
        ((GameInputManager*)actionBatch)->actionPress[2] = 0;
        ((GameInputManager*)actionBatch)->actionBlocked[3] = 0;
        ((GameInputManager*)actionBatch)->actionBlockedSecondary[3] = 0;
        ((GameInputManager*)actionBatch)->pendingActionPress[3] = 0;
        ((GameInputManager*)actionBatch)->actionPress[3] = 0;
        ((GameInputManager*)actionBatch)->actionBlocked[4] = 0;
        ((GameInputManager*)actionBatch)->actionBlockedSecondary[4] = 0;
        ((GameInputManager*)actionBatch)->pendingActionPress[4] = 0;
        ((GameInputManager*)actionBatch)->actionPress[4] = 0;
        ((GameInputManager*)actionBatch)->actionBlocked[5] = 0;
        ((GameInputManager*)actionBatch)->actionBlockedSecondary[5] = 0;
        ((GameInputManager*)actionBatch)->pendingActionPress[5] = 0;
        ((GameInputManager*)actionBatch)->actionPress[5] = 0;
        ((GameInputManager*)actionBatch)->actionBlocked[6] = 0;
        ((GameInputManager*)actionBatch)->actionBlockedSecondary[6] = 0;
        ((GameInputManager*)actionBatch)->pendingActionPress[6] = 0;
        ((GameInputManager*)actionBatch)->actionPress[6] = 0;
        actionBatch += kActionWordsPerResetBatch * sizeof(int);
    }
    int resetByteCount = resetActionCount * 4; // 140
    lateZero = 0;
    unsigned char* p = self + resetByteCount;
    ((GameInputManager*)p)->actionBlocked[0] = 0;
    ((GameInputManager*)p)->actionBlockedSecondary[0] = 0;
    ((GameInputManager*)p)->pendingActionPress[0] = 0;
    ((GameInputManager*)p)->actionPress[0] = 0;
    ((GameInputManager*)p)->actionBlocked[1] = 0;
    ((GameInputManager*)p)->actionBlockedSecondary[1] = 0;
    ((GameInputManager*)p)->pendingActionPress[1] = 0;
    ((GameInputManager*)p)->actionPress[1] = 0;
    ((GameInputManager*)p)->actionBlocked[2] = 0;
    ((GameInputManager*)p)->actionBlockedSecondary[2] = 0;
    ((GameInputManager*)p)->pendingActionPress[2] = 0;
    ((GameInputManager*)p)->actionPress[2] = 0;
    ((GameInputManager*)p)->actionBlocked[3] = 0;
    ((GameInputManager*)p)->actionBlockedSecondary[3] = 0;
    ((GameInputManager*)p)->pendingActionPress[3] = 0;
    ((GameInputManager*)p)->actionPress[3] = 0;
    ((GameInputManager*)self)->selectedPadLookupDisabled = 0;
    ((GameInputManager*)self)->rumbleFramesRemaining[0] = kNoRumbleRequest;
    ((GameInputManager*)self)->rumbleFramesRemaining[1] = kNoRumbleRequest;
    ((GameInputManager*)self)->rumbleFramesRemaining[2] = kNoRumbleRequest;
    ((GameInputManager*)self)->rumbleFramesRemaining[3] = kNoRumbleRequest;
    ((GameInputManager*)self)->rumblePaused = 0;
    // Scratch storage: A/B each have SIX words; C..F each have FOUR (one per channel).
    // Their runtime meanings are unresolved; these are distinct banks, not a single channel array.
    ((InputStartupState*)self)->cursorReservedA[0] = 0;
    ((InputStartupState*)self)->cursorReservedB[0] = 0;
    ((InputStartupState*)self)->cursorReservedA[1] = 0;
    ((InputStartupState*)self)->cursorReservedB[1] = 0;
    ((InputStartupState*)self)->cursorReservedA[2] = 0;
    ((InputStartupState*)self)->cursorReservedB[2] = 0;
    ((InputStartupState*)self)->cursorReservedA[3] = 0;
    ((InputStartupState*)self)->cursorReservedB[3] = 0;
    ((InputStartupState*)self)->cursorReservedA[4] = 0;
    ((InputStartupState*)self)->cursorReservedB[4] = 0;
    ((InputStartupState*)self)->cursorReservedA[5] = 0;
    ((InputStartupState*)self)->cursorReservedB[5] = 0;
    ((InputStartupState*)self)->cursorReservedC[0] = 0;
    ((InputStartupState*)self)->cursorReservedD[0] = 0;
    ((InputStartupState*)self)->cursorReservedC[1] = 0;
    ((InputStartupState*)self)->cursorReservedD[1] = 0;
    ((InputStartupState*)self)->cursorReservedC[2] = 0;
    ((InputStartupState*)self)->cursorReservedD[2] = 0;
    ((InputStartupState*)self)->cursorReservedC[3] = 0;
    ((InputStartupState*)self)->cursorReservedD[3] = 0;
    ((InputStartupState*)self)->cursorReservedE[0] = 0;
    ((InputStartupState*)self)->cursorReservedF[0] = 0;
    ((InputStartupState*)self)->cursorReservedE[1] = 0;
    ((InputStartupState*)self)->cursorReservedF[1] = 0;
    ((InputStartupState*)self)->cursorReservedE[2] = 0;
    ((InputStartupState*)self)->cursorReservedF[2] = 0;
    ((InputStartupState*)self)->cursorReservedE[3] = 0;
    ((InputStartupState*)self)->cursorReservedF[3] = 0;

    // These byte tables are copied to stack temporaries before the four
    // per-entry calls below.
    unsigned char* t0 = PaletteTemplateAt(defaultPalettes, 0);
    unsigned char arr1[16] = {t0[0], t0[1], t0[2],  t0[3],  t0[4],  t0[5],  t0[6],  t0[7],
                              t0[8], t0[9], t0[10], t0[11], t0[12], t0[13], t0[14], t0[15]};
    unsigned char* t1 = PaletteTemplateAt(defaultPalettes, 16);
    unsigned char arr2[16] = {t1[0], t1[1], t1[2],  t1[3],  t1[4],  t1[5],  t1[6],  t1[7],
                              t1[8], t1[9], t1[10], t1[11], t1[12], t1[13], t1[14], t1[15]};
    unsigned char* t2 = PaletteTemplateAt(defaultPalettes, 32);
    unsigned char arr3[16];
    CopyPalette16(arr3, t2);
    unsigned char* s0 = (unsigned char*)arr1;
    unsigned char* p0 = AsGameInputManager(self)->cursorBaseColors[0];
    unsigned char* s1 = (unsigned char*)arr2;
    unsigned char* p1 = AsGameInputManager(self)->cursorActiveColors[0];
    unsigned char* s2 = (unsigned char*)arr3;
    unsigned char* p2 = AsGameInputManager(self)->cursorAlternateColors[0];
    // The next three bases advance by their respective element strides (4, 12, 8 bytes).
    // Whole-object casts only recover field offsets; these never denote complete objects.
    unsigned char* channelScratch = self;
    unsigned char* cursorMotion = self;
    unsigned char* padFlags = self;
    int channel = 0;
    do {
        ColorRGBA_Copy(p0, s0);
        ColorRGBA_Copy(p1, s1);
        ColorRGBA_Copy(p2, s2);
        ((InputStartupState*)channelScratch)->channelReservedA[0] = lateZero;
        channel++;
        s0 += 4;
        ((InputStartupState*)channelScratch)->channelReservedB[0] = lateZero;
        p0 += 4;
        s1 += 4;
        p1 += 4;
        ((InputStartupState*)channelScratch)->channelReservedC[0] = lateZero;
        s2 += 4;
        p2 += 4;
        channelScratch += 4;
        ((InputStartupState*)cursorMotion)->cursorMotion[0].xBits = lateZero;
        ((InputStartupState*)cursorMotion)->cursorMotion[0].yBits = lateZero;
        ((InputStartupState*)cursorMotion)->cursorMotion[0].zBits = lateZero;
        cursorMotion += 0xc;
        ((InputStartupState*)padFlags)->padFlags[0].state = lateZero;
        ((InputStartupState*)padFlags)->padFlags[0].flags[0] = lateZero;
        ((InputStartupState*)padFlags)->padFlags[0].flags[1] = lateZero;
        ((InputStartupState*)padFlags)->padFlags[0].flags[2] = lateZero;
        padFlags += 8;
    } while (channel < kWiiRemoteChannelCount);

    float zeroCoordinate = kFloat_0_BalloonInput;
    ((GameInputManager*)self)->pointerOverrideEnabled = 0;
    ((GameInputManager*)self)->pointerOverrideY = zeroCoordinate;
    ((GameInputManager*)self)->pointerOverrideX = zeroCoordinate;
    ((GameInputManager*)self)->cursorDrawingSuspendCount = 0;
    return self;
}
#pragma opt_strength_reduction reset
#pragma opt_propagation reset

} // extern "C"

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU80462000", _ctors_u8006DAE8_0, BalloonQueues_RegisterStatics);
#pragma pop

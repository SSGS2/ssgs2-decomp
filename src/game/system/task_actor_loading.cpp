// Translation unit 0x801FDF50-0x801FE884 (6 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

#include "game/singletons.h"
#include "sdk_protos.h"
#include "classes/TaskListFields.h"
extern "C" {

void* s_actorLoadingContext;
extern char s_homeButtonResourceState[], s_homeButtonStateDtorRegistration_801FDF50[], s_gameInputManager[], s_inputManagerDtorRegistration_ActorLoading[];
signed char s_homeMenuInitialized;
signed char s_inputManagerInitialized;
void HomeButtonMenu_InitializeState(void*);
void HomeButtonResources_dtor(void*);
void HomeMenu_OverrideBlockedState(void*, int);
void InputManager_Init(void*);
void InputManager_Dtor(void*);
void CAnotherHand_StartCallbackThread(void*, void*, int, void*, int, int);
int strcmp(const char*, const char*);
void* TaskRegistry_GetInstance(void*);
void* TaskRegistry_CreateActorByName(void*, const char*);
void IActor_ClearSkipFlags(void*, void*);
void* __nwa__FUl(unsigned int);
void* appMallocUni__FUl(unsigned int);
void* operator_new(unsigned int);
void CAnotherHand_Lock(void*);
void CAnotherHand_Unlock(void*);

struct WInstanceLock {
    void* lock;
    // Acquire one recursive registry-mutex level.
    WInstanceLock(void* p) : lock(p) { OSLockMutex((OSMutex*)p); }
    // Release this guard's recursive mutex level on every scope exit.
    ~WInstanceLock() { OSUnlockMutex((OSMutex*)lock); }
};

struct ActorLifecycleSlots {
    virtual void s0();
    virtual void s1();
    virtual void s2();
    virtual void loadResources();
    virtual void initializeForTask(void*);
    virtual void s5();
    virtual void s6();
    virtual void s7();
    virtual void s8();
    virtual void dispatchMessage(void*);
    virtual void s10();
    virtual void s11();
    virtual void s12();
    virtual void s13();
    virtual void s14();
    virtual void s15();
    virtual void initializeTaskState();
};

struct ActorLoadingSlot { void* table; unsigned char reserved; unsigned char complete; };
// Report completion of the actor-loading callback associated with the shared slot.
unsigned char CTask_IsActorLoadingComplete() { return ((ActorLoadingSlot*)&s_actorLoadingContext)->complete; }

// Registry ownership/views: node.data -> ActorHolder -> RegisteredActor.
// node.value and holder.arg are the SAME borrowed, NUL-terminated lookup key;
// name is instead the factory's actor class name. Do not free the key here.
// next/prev form circular doubly-linked rings; link is a hash collision chain.
// registry.list is the active ring; registry.first the available-node ring;
// registry.tail tracks allocation blocks (their first node anchors that ring).
// A node leaves the free ring before it joins the hash table and active ring.
// All mutations nest locks on the SAME recursive mutex; a one-node ring points
// to itself, and removing that last node sets its ring head to null.
struct ActorRegistryNode { void* data; void* value; unsigned char flag; char pad[3]; ActorRegistryNode* next; ActorRegistryNode* prev; void* link; };
struct ActorRegistryTask { char interfaces[0xC]; unsigned char synchronousLoad; char pad0D[3]; void* list; void* traversalCursor; void* tail; void* first; int count; unsigned int mask; char reserved28[4]; void** buckets; char mutex[0x1c]; void* value; };
struct ActorTaskView { ActorRegistryTask registry; char callbackThread[1]; };
struct ActorRegistryContext { void* activeHead; void* traversalCursor; char state[0x18]; char mutex[1]; };
struct InputActionCounters { char prefix[0x1C]; unsigned int counters[39]; };
struct RegisteredActor { void* vtable; char actorBase[0xc]; void* owner; void* value; };
struct ActorHolder { void* entry; unsigned char flag; char pad[3]; void* arg; };
// Metrowerks narrow string storage: word0's high bit tags heap storage; inline
// length is byte0 & 0x7f with characters at +1; heap length/pointer are +4/+8.
// opt is required even for empty text. Message +0/+4/+8 are zero/type/text;
// dispatch is synchronous and receives a stack message plus borrowed text.
struct StartupTextStorage { unsigned int word0; int word4; void* word8; };
static const int kActorStartupTextMessage = 0x2712;
enum ActorNameHashConstants {
    kActorHashFirstByteMask = 0x2A, kActorHashLastByteMask = 0x15,
    kActorHashLengthShift = 6, kActorInputActionCount = 39
};
// Combine length with alternating bits from the first and final bytes; caller applies bucket mask.
static inline unsigned int ActorNameHashWord(unsigned int length, unsigned int edgeBits) {
    return (length << kActorHashLengthShift) | edgeBits;
}
// End a traversal after its predecessor reaches the active-ring head; caller holds the mutex.
static inline void EndActorCursorAtRingHead(void*& predecessor, void* const& activeHead) {
    if (predecessor == activeHead) predecessor = 0;
}

// Cursor contract for both traversal loops: mutex protects link reads and cursor
// publication, but is released before actor virtual calls. The cursor advances
// BEFORE dispatch. Actors/holders must stay alive across that unlocked call;
// these loops do not retain them or prove arbitrary concurrent deletion safe.
// Mutators must repair traversalCursor when unlinking its target (see pool removal).
enum ActorTaskFlags { kActorTaskStateInitialized = 2 };
struct ActorStartupMessage { int a; int b; const char* str; int c[5]; };

#pragma push
#pragma use_lmw_stmw on
#pragma opt_common_subs off
// Create an actor, register its name under the recursive mutex, and send optional startup text.
void* TaskActorTable_CreateNamedActor(ActorRegistryTask* self, const char* name, void* arg, void* extra, StartupTextStorage* opt) {
    // Phase 1: factory-create actor and holder; publish task owner/context.
    // Returns null on factory failure, otherwise the registry-held actor.
    // Reuses one ABI word: borrowed name key, then numeric bucket index, then ring head.
    void* keyOrBucketWord;
    ActorRegistryNode* node;
    RegisteredActor* entry;
    entry = (RegisteredActor*)TaskRegistry_CreateActorByName(TaskRegistry_GetInstance(self), name);
    if (!entry) return 0;
    entry->value = self->value;
    entry->owner = self;
    unsigned int len = strlen(name);
    unsigned int holderAllocationBytes = len + 0xC;
    ActorHolder* item = (ActorHolder*)__nwa__FUl(holderAllocationBytes * 2 - holderAllocationBytes);
    item->entry = entry;
    item->arg = arg;
    IActor_ClearSkipFlags(entry, extra);
    item->flag = 0;
    {
        keyOrBucketWord = item->arg;
        WInstanceLock registryGuard(((ActorRegistryTask*)self)->mutex);
        {
            WInstanceLock freeRingGuard(((ActorRegistryTask*)self)->mutex);
            if (!self->first) {
                // Phase 2: retain one block anchor, add the remaining count-1
                // nodes to the free ring. Each block needs a header and at least one usable node.
                ActorRegistryNode* nodes = (ActorRegistryNode*)appMallocUni__FUl(self->count * 0x18);
                char* lock = ((ActorRegistryTask*)self)->mutex;
                ActorRegistryNode* tail = (ActorRegistryNode*)self->tail;
                OSLockMutex((OSMutex*)lock);
                if (!tail) {
                    nodes->next = nodes;
                    nodes->prev = nodes;
                    OSUnlockMutex((OSMutex*)lock);
                    tail = nodes;
                } else {
                    nodes->prev = tail;
                    nodes->next = tail->next;
                    tail->next = nodes;
                    nodes->next->prev = nodes;
                    OSUnlockMutex((OSMutex*)lock);
                }
                int i = 1;
                self->tail = tail;
                if (self->count > 1) {
                    ActorRegistryNode* first;
                    char* lock2 = ((ActorRegistryTask*)self)->mutex;
                    ActorRegistryNode* n = nodes + 1;
                    for (; i < self->count; ++n, ++i) {
                        first = (ActorRegistryNode*)self->first;
                        OSLockMutex((OSMutex*)lock2);
                        if (!first) {
                            n->next = n;
                            n->prev = n;
                            OSUnlockMutex((OSMutex*)lock2);
                            first = n;
                        } else {
                            n->prev = first;
                            n->next = first->next;
                            first->next = n;
                            n->next->prev = n;
                            OSUnlockMutex((OSMutex*)lock2);
                        }
                        self->first = first;
                    }
                }
            }
            char* lock3 = ((ActorRegistryTask*)self)->mutex;
            // Detach exactly one node; its next/prev are overwritten on publish.
            node = (ActorRegistryNode*)self->first;
            OSLockMutex((OSMutex*)lock3);
            node->prev->next = node->next;
            node->next->prev = node->prev;
            ActorRegistryNode* newFirst = node->prev;
            if (newFirst == node) newFirst = 0;
            OSUnlockMutex((OSMutex*)lock3);
            self->first = newFirst;
        }
        node->data = item;
        // Phase 3: attach holder/key, prepend to hash bucket when keyed, then
        // splice into the active ring. The hash and iteration views coexist.
        node->flag = 0;
        node->value = keyOrBucketWord;
        if (keyOrBucketWord) {
            unsigned int mask;
            unsigned int bits;
            unsigned int last;
            unsigned int size;
            char* lock4 = ((ActorRegistryTask*)self)->mutex;
            OSLockMutex((OSMutex*)lock4);
            size = strlen((const char*)keyOrBucketWord);
            last = (int)size > 0 ? size - 1 : 0;
            bits = (((unsigned char*)keyOrBucketWord)[0] & kActorHashFirstByteMask) | (((unsigned char*)keyOrBucketWord)[last] & kActorHashLastByteMask);
            mask = self->mask;
            keyOrBucketWord = (void*)(mask & ActorNameHashWord(size, bits));
            OSUnlockMutex((OSMutex*)lock4);
            OSLockMutex((OSMutex*)lock4);
            node->link = self->buckets[(unsigned int)keyOrBucketWord];
            self->buckets[(unsigned int)keyOrBucketWord] = node;
            OSUnlockMutex((OSMutex*)lock4);
        }
        char* lock5 = ((ActorRegistryTask*)self)->mutex;
        keyOrBucketWord = self->list;
        OSLockMutex((OSMutex*)lock5);
        if (!keyOrBucketWord) {
            node->next = node;
            node->prev = node;
            OSUnlockMutex((OSMutex*)lock5);
        } else {
            node->prev = (ActorRegistryNode*)keyOrBucketWord;
            node->next = ((ActorRegistryNode*)keyOrBucketWord)->next;
            ((ActorRegistryNode*)keyOrBucketWord)->next = node;
            node->next->prev = node;
            OSUnlockMutex((OSMutex*)lock5);
            node = (ActorRegistryNode*)keyOrBucketWord;
        }
        self->list = node;
    }
    int small = !((unsigned int)opt->word0 >> 31);
    // Phase 4: optional startup text; the actor must not retain the stack msg.
    int lenVal = small ? (*(unsigned char*)opt & 0x7F) : opt->word4;
    if (lenVal != 0) {
        ActorStartupMessage msg;
        const char* str = small ? (const char*)opt + 1 : (const char*)opt->word8;
        msg.a = 0;
        msg.b = kActorStartupTextMessage;
        msg.str = str;
        msg.c[0] = 0;
        msg.c[1] = 0;
        msg.c[2] = 0;
        msg.c[3] = 0;
        msg.c[4] = 0;
        if (entry) {
            ((ActorLifecycleSlots*)entry)->dispatchMessage(&msg);
        }
    }
    return entry;
}
#pragma opt_common_subs reset
#pragma pop

class Obj801FE508 { public:
    virtual void v0();
    virtual void v1();
    virtual void v2();
    virtual void v3();
    virtual void v4();
    virtual void v5();
    virtual void v6();
    virtual void v7();
    virtual void v8();
    virtual void v9();
    virtual void v10();
    virtual void v11();
    virtual void v12();
    virtual void v13();
    virtual void v14();
    virtual void v15();
    virtual void v16();
    virtual void v17();
    virtual void v18();
    virtual void v19();
    virtual void v20();
    virtual void loadActors(int, void*);
};

// Block the HOME menu, start synchronous or threaded actor loading, and reset input counters.
void CTask_StartActorLoading(void* self) {
    // Phase 1: publish a borrowed pointer to self's active-list subobject and
    // block HOME. Phase 2: invoke loadActors directly or start the callback
    // thread using the secondary interface at self+4. Phase 3: reset input.
    // self and its actor list must remain alive until the callback completes.
    void* __g = (void*)Sing_805DC820();
    s_actorLoadingContext = (char*)&((ActorRegistryTask*)self)->list;
    if (!__g) {
        if (!s_homeMenuInitialized) {
            HomeButtonMenu_InitializeState(&s_homeButtonResourceState);
            __register_global_object(&s_homeButtonResourceState, (void*)HomeButtonResources_dtor, &s_homeButtonStateDtorRegistration_801FDF50);
            s_homeMenuInitialized = 1;
        }
        Sing_805DC820() = &s_homeButtonResourceState;
    }
    HomeMenu_OverrideBlockedState((void*)Sing_805DC820(), 1);
    if (((ActorRegistryTask*)self)->synchronousLoad) {
        ((Obj801FE508*)self)->loadActors(0, &s_actorLoadingContext);
    } else {
        void* arg = self;
        if (self) arg = (char*)&((ActorRegistryTask*)self)->interfaces[4];
        CAnotherHand_StartCallbackThread(((ActorTaskView*)self)->callbackThread, arg, 0, &s_actorLoadingContext, 0, 0);
    }
    if (!Sing_805DC5C8()) {
        if (!s_inputManagerInitialized) {
            InputManager_Init(&s_gameInputManager);
            __register_global_object(&s_gameInputManager, (void*)InputManager_Dtor, &s_inputManagerDtorRegistration_ActorLoading);
            s_inputManagerInitialized = 1;
        }
        Sing_805DC5C8() = &s_gameInputManager;
    }
    unsigned int* p = ((InputActionCounters*)Sing_805DC5C8())->counters;
    for (int i = 0; i < kActorInputActionCount; ++i) p[i] = 0;
}

#pragma opt_common_subs off
// Initialize every registered actor, unblock HOME, and run the one-time task setup hook.
void CTask_InitActors(void* self) {
    // Phase 1: unblock HOME. Phase 2: walk the active ring through prev, unwrap
    // node -> holder -> actor and initialize each actor outside the list lock.
    // Phase 3: mark task initialization before invoking its once-only hook.
    if (!Sing_805DC820()) {
        if (!s_homeMenuInitialized) {
            HomeButtonMenu_InitializeState(&s_homeButtonResourceState);
            __register_global_object(&s_homeButtonResourceState, (void*)HomeButtonResources_dtor, &s_homeButtonStateDtorRegistration_801FDF50);
            s_homeMenuInitialized = 1;
        }
        Sing_805DC820() = &s_homeButtonResourceState;
    }
    HomeMenu_OverrideBlockedState((void*)Sing_805DC820(), 0);
    void* mutex;
    void* holder;
    {
        WInstanceLock lockList(((ActorRegistryTask*)self)->mutex);
        ((ActorRegistryTask*)self)->traversalCursor = ((ActorRegistryTask*)self)->list; // +0x10: active list
        mutex = ((ActorRegistryTask*)self)->mutex;
        OSLockMutex((OSMutex*)mutex);
        void* nxt;
        void* cur = ((ActorRegistryTask*)self)->traversalCursor;
        if (cur) {
            nxt = ((ActorRegistryNode*)cur)->prev; // +0x10: circular traversal link
            holder = *(void**)cur;
            EndActorCursorAtRingHead(nxt, ((ActorRegistryTask*)self)->list);
            ((ActorRegistryTask*)self)->traversalCursor = nxt;
            OSUnlockMutex((OSMutex*)mutex);
        } else {
            OSUnlockMutex((OSMutex*)mutex);
            holder = 0;
        }
    }
    if (holder) {
        mutex = ((ActorRegistryTask*)self)->mutex;
        while (holder) {
            void* object = *(void**)holder;
            ((ActorLifecycleSlots*)object)->initializeForTask(self);
            OSLockMutex((OSMutex*)mutex);
            void* nxt;
            void* cur = ((ActorRegistryTask*)self)->traversalCursor;
            if (cur) {
                nxt = ((ActorRegistryNode*)cur)->prev;
                holder = *(void**)cur;
                EndActorCursorAtRingHead(nxt, ((ActorRegistryTask*)self)->list);
                ((ActorRegistryTask*)self)->traversalCursor = nxt;
                OSUnlockMutex((OSMutex*)mutex);
            } else {
                OSUnlockMutex((OSMutex*)mutex);
                holder = 0;
            }
        }
    }
    // +0x3A0 mixes view-setup bit 0 with task-initialization-done bit 1.
    volatile unsigned int* flags = (volatile unsigned int*)&AsTaskListFields(self)->taskFlags;
    if (!(*flags & kActorTaskStateInitialized)) {
        *flags |= kActorTaskStateInitialized;
        ((ActorLifecycleSlots*)self)->initializeTaskState();
    }
}
#pragma opt_common_subs reset

// Look up the actor holder by its length/edge-byte hash and return the contained actor.
void* TaskActorTable_FindByName(void* self, const char* name) {
    char* lock = ((ActorRegistryTask*)self)->mutex;
    void* found = 0;
    {
        WInstanceLock guard(lock);
        if (name && AsTaskListFields(self)->hashBuckets) { // +0x2C
            unsigned int len;
            unsigned int idx;
            {
                WInstanceLock guard2(lock);
                len = strlen(name);
                unsigned int last = len ? len - 1 : 0;
                unsigned int bits = (name[last] & kActorHashLastByteMask) | (name[0] & kActorHashFirstByteMask);
                idx = ActorNameHashWord(len, bits) & AsTaskListFields(self)->hashMask; // +0x24
            }
            void* node = ((void**)AsTaskListFields(self)->hashBuckets)[idx];
            while (node) {
                // A match unwraps node -> holder here, then holder -> actor at
                // return. Both dereferences are required by the registry ABI.
                if (!strcmp((const char*)((ActorRegistryNode*)node)->value, name)) { found = *(void**)node; break; } // +4: lookup key
                node = ((ActorRegistryNode*)node)->link; // +0x14: hash chain
            }
        }
    }
    if (found) return *(void**)found;
    return 0;
}

#pragma push
#pragma use_lmw_stmw on
#pragma opt_common_subs off
// Load each actor under callback-thread ownership, then mark the shared completion slot.
void CTask_LoadActorsCallback(void* self, void*, void* slot) {
    // slot is a borrowed {active-list context pointer, reserved byte, complete}
    // shared with CTask_IsActorLoadingComplete. Phase 1: own the callback thread and seed the
    // circular iterator. Phase 2: call each actor's resource hook outside the
    // list mutex. Phase 3: publish complete before releasing thread ownership.
    CAnotherHand_Lock(((ActorTaskView*)self)->callbackThread);
    void* lock;
    void* first;
    void* itemNode;
    {
        first = *(void**)slot;
        WInstanceLock outer(((ActorRegistryContext*)first)->mutex);
        ((ActorRegistryContext*)first)->traversalCursor = *(void**)first;
        lock = ((ActorRegistryContext*)first)->mutex;
        OSLockMutex((OSMutex*)lock);
        void* link;
        void* previous = ((ActorRegistryContext*)first)->traversalCursor;
        if (previous) {
            link = ((ActorRegistryNode*)previous)->prev; // +0x10
            itemNode = *(void**)previous;
            EndActorCursorAtRingHead(link, *(void**)first);
            ((ActorRegistryContext*)first)->traversalCursor = link;
            OSUnlockMutex((OSMutex*)lock);
        } else {
            OSUnlockMutex((OSMutex*)lock);
            itemNode = 0;
        }
    }
    while (itemNode) {
        void* object = *(void**)itemNode;
        ((ActorLifecycleSlots*)object)->loadResources();
        first = *(void**)slot;
        lock = ((ActorRegistryContext*)first)->mutex;
        OSLockMutex((OSMutex*)lock);
        void* link;
        void* previous = ((ActorRegistryContext*)first)->traversalCursor;
        if (previous) {
            link = ((ActorRegistryNode*)previous)->prev;
            itemNode = *(void**)previous;
            EndActorCursorAtRingHead(link, *(void**)first);
            ((ActorRegistryContext*)first)->traversalCursor = link;
            OSUnlockMutex((OSMutex*)lock);
        } else {
            OSUnlockMutex((OSMutex*)lock);
            itemNode = 0;
        }
    }
    ((ActorLoadingSlot*)slot)->complete = 1;
    CAnotherHand_Unlock(((ActorTaskView*)self)->callbackThread);
}
#pragma pop

} // extern "C"

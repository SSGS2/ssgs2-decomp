// The task manager: its singleton and pending command storage, the circular actor list and its
// iterators, and the per-frame processing and drawing of actors.
// Also the actor factory name tree, the actor type checks and the active-task replacement by name.

#include "game/singletons.h"
#include "classes/CTask.h"
extern "C" {
enum ActorRendering {
    kLastRenderPriority = 5,
    kCourseScenePriority = 2,
    kFlushPriority1 = 1,
    kFlushPriority4 = 4,
    kSkipViewSetup = 1
};
enum ActorNameHash {
    kFirstCharHashBits = 0x2a,
    kLastCharHashBits = 0x15,
    kLengthHashShift = 6
};
enum FactoryString {
    kInlineLengthMask = 127
};
struct TaskResource;
extern void* g_taskManager;
extern char s_taskManagerSingletonVtable[], __vt__12CTaskManager[];
void BaseObject_dtor(void*, int);
void BaseObject_ctor(void*);
void appFreeUni__FPv(void*);
void __dl__FPv(void*);
// Optional task lifecycle hook with no base implementation.
void CTask::vfunc_18() {}
// Optional task lifecycle hook with no base implementation.
void CTask::vfunc_19() {}
// Manager owns activeTask and the paged command deque at +0x24 (see
// ctask_transition_command_queue.cpp: TaskCommandQueue and TaskCommandDeque_Dtor). Each 0x2c-byte record
// owns a small-string command name; the queue owns pages and the page-pointer ring.
// Startup clears +10 pending, +14 requested parent name, +18 requested child name.
// CTask::RequestTransition writes those same offsets before calling ReplaceActiveTaskByName.
// +20 is deliberately not initialized. Queue word map: +24 front record offset,
// +28 record count, +2c page pointers, +30 first page, +34 page count, +38 capacity.
struct TaskManagerStorage {
    void* vtable;
    char pad04[12];
    unsigned char transitionFlag;
    int requestedParentTaskName; // +14: requested parent factory name, borrowed C string
    int requestedChildTaskName;  // +18: requested child factory name, borrowed C string
    TaskResource* activeTask;
    int preservedWord20;       // Not initialized by the original constructor.
    int commandFrontOffset;    // +24: front record offset within paged command deque
    int queuedCommandCount;    // +28: number of live command records
    int commandPages;          // +2C: owned ring of page pointers
    int firstCommandPageIndex; // +30: first page in the pointer ring
    int commandPageCount;      // +34: live page count
    int commandPageCapacity;   // +38: page-pointer capacity, not record capacity
};

#include "classes/TaskListFields.h"
#include "classes/CSceneManager.h"
#include "classes/CTask.h"
#include "classes/CTaskManager.h"
#include "classes/CTaskManagerSingleton.h"
// Initialize the base and publish the task-manager singleton before clearing derived state.
inline void InitializeTaskManagerSingleton(TaskManagerStorage* self) {
    BaseObject_ctor(self);
    *(void* volatile*)&self->vtable = s_taskManagerSingletonVtable;
    g_taskManager = self;
}
// Set the manager vtable; clear pending names and the owning command deque, preserving +20.
inline void ClearTaskManagerState(TaskManagerStorage* self, int zero) {
    self->vtable = __vt__12CTaskManager;
    self->transitionFlag = zero;
    self->requestedParentTaskName = zero;
    self->requestedChildTaskName = zero;
    self->activeTask = (TaskResource*)zero;
    self->commandFrontOffset = zero;
    self->queuedCommandCount = zero;
    self->commandPages = zero;
    self->firstCommandPageIndex = zero;
    self->commandPageCount = zero;
    self->commandPageCapacity = zero;
}
// Construct and register an empty task manager.
CTaskManager::CTaskManager() {
    TaskManagerStorage* self = (TaskManagerStorage*)this;
    InitializeTaskManagerSingleton(self);
    ClearTaskManagerState(self, 0);
}
// Unpublish the task manager before destroying its base object.
inline void DestroyTaskManagerSingletonBase(void* self, short deleting) {
    BaseObject_dtor(self, (g_taskManager = 0, deleting));
}
// Destroy the singleton base and optionally release its storage.
CTaskManagerSingleton::~CTaskManagerSingleton() {
    DestroyTaskManagerSingletonBase(this, 0);
}

// Owning allocation of pointers: releasing the buffer frees slots, not pointees.
struct TaskPointerBuffer {
    void* data;
    int preserved04;
    int count;
};
// Clear the element count and free the buffer allocation.
inline void ReleaseTaskPointerBuffer(TaskPointerBuffer* self) {
    if (self) {
        if (self) {
            if (self) {
                if (self->data) {
                    self->count -= (unsigned int)self->count;
                    appFreeUni__FPv(self->data);
                }
            }
        }
    }
}
// Destroy a task-pointer buffer, optionally deleting the wrapper.
void* TaskPointerBuffer_Destroy(TaskPointerBuffer* self, int deleting) {
    if (self) {
        ReleaseTaskPointerBuffer(self);
        if (deleting > 0)
            __dl__FPv(self);
    }
    return self;
}
// Destroy the alternate task-pointer buffer instantiation with the same ownership policy.
void* TaskPointerBuffer_dtor(TaskPointerBuffer* self, int deleting) {
    if (self) {
        if (self) {
            if (self)
                ReleaseTaskPointerBuffer(self);
        }
        if (deleting > 0)
            __dl__FPv(self);
    }
    return self;
}
}

extern "C" {
struct TaskResourceDispatch;
struct TaskResourceDispatch {
    virtual void v2();
    virtual void destroy(int);
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
    virtual void close();
};
void TaskCommandDeque_Dtor(void*, int);
// Close and destroy the active task, release manager resources, then unpublish the singleton.
CTaskManager::~CTaskManager() {
    TaskManagerStorage* self = (TaskManagerStorage*)this;
    // Phase 1: close may change activeTask; reload the owner slot before deleting.
    TaskResourceDispatch* r = (TaskResourceDispatch*)self->activeTask;
    bool hasResource = r != 0;
    self->vtable = __vt__12CTaskManager;
    if (hasResource) {
        r->close();
        r = (TaskResourceDispatch*)self->activeTask;
        if (r)
            r->destroy(1);
        self->activeTask = 0;
    }
    // Phase 2: free queued names, record pages and the page-pointer ring.
    void* commandDeque = &self->commandFrontOffset;
    if (commandDeque)
        if (commandDeque)
            if (commandDeque)
                if (commandDeque)
                    if (commandDeque)
                        TaskCommandDeque_Dtor(commandDeque, 0);
    // Phase 3: unpublish singleton and tear down base before optional storage deletion.
    if (self)
        DestroyTaskManagerSingletonBase(self, 0);
}
void OSLockMutex(void*);
void OSUnlockMutex(void*);
struct WInstanceLock {
    void* p;
    // Lock the borrowed mutex for this scope; nested helpers also take this mutex.
    WInstanceLock(void* mutex) : p(mutex) {
        OSLockMutex(p);
    }
    // Release the scope lock even if a callback unwinds.
    ~WInstanceLock() {
        OSUnlockMutex(p);
    }
};
struct ActorDispatch;
// Dispatch-only ABI: each declaration occupies one consecutive 4-byte vtable slot.
// Undecoded entries keep placeholders; changing order would call another operation.
struct ActorDispatch {
    virtual void v2();
    virtual void v3();
    virtual void preLoadInit();
    virtual void v5();
    virtual void v6();
    virtual void reload();
    virtual void process(float);
    virtual void display();
    virtual void notifyDestroy();
    virtual void applyMode(int);
    virtual void v12();
    virtual int renderPriority();
    virtual int isBlocked();
};
// Nodes borrow actor slots. Circular links end at first; cursor==null means exhausted.
struct ActorListNode {
    ActorDispatch** data;
    char pad[12];
    ActorListNode* next;
};
struct ActorIterator {
    ActorListNode* node;
};
struct ActorList {
    char pad[16];
    ActorListNode* first;
    ActorListNode* current; // shared cursor; independent iterators keep a separate node
    char pad18[24];
    char mutex[24]; // recursive OS mutex; callers may nest iterator locks
    ActorDispatch* selected;
};
// Advance the shared circular-list cursor under the mutex, stopping on wraparound.
inline ActorDispatch** NextActor(ActorList* self) {
    WInstanceLock lock(self->mutex);
    if (self->current) {
        ActorListNode* next = self->current->next;
        ActorDispatch** value = self->current->data;
        if (next == self->first)
            next = 0;
        self->current = next;
        return value;
    }
    return 0;
}
// Reload every registered actor using the mutex-protected shared iterator.
void CTask::ReloadActors() {
    ActorList* self = (ActorList*)this;
    ActorDispatch** item;
    {
        WInstanceLock lock(self->mutex);
        self->current = self->first;
        item = NextActor(self);
    }
    if (item)
        while (item) {
            (*item)->reload();
            item = NextActor(self);
        }
}
// Send the destruction notification to every registered actor.
void CTask::NotifyActorsDestroy() {
    ActorList* self = (ActorList*)this;
    ActorDispatch** item;
    {
        WInstanceLock lock(self->mutex);
        self->current = self->first;
        item = NextActor(self);
    }
    if (item)
        while (item) {
            (*item)->notifyDestroy();
            item = NextActor(self);
        }
}
// Reset the shared cursor BEFORE this helper locks, then fetch/advance under the
// mutex. Callers hold an outer lock on the same recursive mutex to protect reset.
inline ActorDispatch** ResetAndNextActor(ActorList* self) {
    ActorListNode* first = self->first;
    void* mutex = self->mutex;
    OSLockMutex((self->current = first, mutex));
    if (self->current) {
        ActorListNode* next = self->current->next;
        ActorDispatch** value = self->current->data;
        if (next == self->first)
            next = 0;
        self->current = next;
        OSUnlockMutex(mutex);
        return value;
    }
    OSUnlockMutex(mutex);
    return 0;
}
// Run each registered actor pre-load initialization callback.
void CTask::PreLoadInitActors() {
    ActorList* self = (ActorList*)this;
    ActorDispatch** item;
    {
        WInstanceLock lock(self->mutex);
        item = ResetAndNextActor(self);
    }
    if (item)
        while (item) {
            (*item)->preLoadInit();
            item = NextActor(self);
        }
}
// Read the first actor node with the original volatile load semantics.
inline ActorListNode* FirstActorNode(ActorList* self) {
    ActorListNode* value = *(ActorListNode* volatile*)&self->first;
    if (value)
        return value;
    return 0;
}
// Start an independent iterator under the list mutex.
inline ActorDispatch** FirstActor(ActorList* self, ActorListNode*& node) {
    void* mutex = self->mutex;
    OSLockMutex(mutex);
    node = FirstActorNode(self);
    ActorDispatch** result;
    if (self->first)
        result = self->first->data;
    else
        result = 0;
    OSUnlockMutex(mutex);
    return result;
}
// Advance an independent actor iterator and stop when the circular list wraps.
inline ActorDispatch** AdvanceActor(ActorList* self, ActorListNode*& node) {
    WInstanceLock lock(self->mutex);
    if (node) {
        node = node->next;
        if (node == self->first) {
            node = 0;
            return 0;
        }
        return node->data;
    }
    return 0;
}
#pragma push
#pragma use_lmw_stmw on
// Apply a mode to the selected actor first, then to every other non-null actor.
void CTask::ApplyModeToActors(int mode) {
    ActorList* self = (ActorList*)this;
    if (self->selected)
        self->selected->applyMode(mode);
    ActorDispatch** item;
    ActorIterator iter;
    item = FirstActor(self, iter.node);
    if (item)
        while (item) {
            ActorDispatch* value = *item;
            if (value != self->selected && value)
                value->applyMode(mode);
            item = AdvanceActor(self, iter.node);
        }
}
extern void* g_sceneManager;
struct SceneDispatch {
    virtual void v2();
    virtual void setup(int, int);
    virtual void flush(int);
};
extern SceneDispatch* g_view;
// Draw unblocked actors in six priority passes, interleaving course rendering and view flushes.
void CTask::DisplayActorsByPriority() {
    ActorList* self = (ActorList*)this;
    ActorDispatch** item;
    ActorIterator iter;
    for (int phase = 0; phase <= kLastRenderPriority; ++phase) {
        item = FirstActor(self, iter.node);
        if (item)
            while (item) {
                ActorDispatch* value = *item;
                if (value && value->renderPriority() == phase && !(*item)->isBlocked())
                    (*item)->display();
                item = AdvanceActor(self, iter.node);
            }
        if (phase == kCourseScenePriority) {
            if (g_sceneManager)
                ((CSceneManager*)g_sceneManager)->RenderCourseScene(g_view);
            if (!(AsTaskListFields(self)->taskFlags & kSkipViewSetup))
                g_view->setup(0, kCourseScenePriority);
        }
        if (phase == kFlushPriority4 || phase == kFlushPriority1)
            g_view->flush(0);
    }
}
#pragma pop
}
extern "C" {
struct HomeMenuState {
    char bytes[64];
};
extern signed char s_homeMenuInitialized;
extern HomeMenuState s_homeButtonResourceState;
extern char s_homeButtonStateDtorRegistration_801FDF50[], taskScoreExceptionStrings[];
void HomeButtonMenu_InitializeState(HomeMenuState*);
void HomeButtonResources_dtor(void*, int);
void __register_global_object(void*, void (*)(void*, int), void*);
int HomeButtonMenu_IsActive(HomeMenuState*);
unsigned int strlen(const char*) throw();
int strcmp(const char*, const char*);
void DrawScene_UpdateRegisteredObjects(void*, float);
struct ActorHashNode {
    ActorDispatch** data;
    const char* key;
    char pad[12];
    ActorHashNode* next;
};
// Hash the name length and selected endpoint bits into the actor bucket mask.
inline unsigned int HashActorName(ActorList* self, const char* key) {
    void* mutex = self->mutex;
    OSLockMutex(mutex);
    unsigned int len = strlen(key);
    unsigned int bits =
        (((unsigned char)key[0] & kFirstCharHashBits) |
         ((unsigned char)key[(int)len > 0 ? len - 1 : 0] & kLastCharHashBits) | (len << kLengthHashShift));
    unsigned int hash = bits & AsTaskListFields(self)->hashMask;
    OSUnlockMutex(mutex);
    return hash;
}
// Find a borrowed actor-pointer slot by name; null key/buckets or no match returns 0.
// The typed bucket cursor borrows nodes under the list mutex. It never changes
// the caller's ActorList pointer. The returned slot remains registry-owned.
inline ActorDispatch** FindNamedActor(ActorList* list, const char* key) {
    union {
        unsigned int hash;
        ActorDispatch** data;
    } result;
    const char* text = key;
    WInstanceLock lock(list->mutex);
    if (!text || !*(ActorHashNode***)&AsTaskListFields(list)->hashBuckets)
        return 0;
    result.hash = HashActorName(list, text);
    // From here, reuse the local pointer word as a typed node cursor. No ActorList
    // access occurs after this assignment; the guard already saved its mutex.
    ActorHashNode*& node = (ActorHashNode*&)list;
    node = (*(ActorHashNode***)&AsTaskListFields(list)->hashBuckets)[result.hash];
    while (node) {
        if (!strcmp(node->key, text)) {
            result.data = node->data;
            return result.data;
        }
        node = node->next;
    }
    return 0;
}
// Lazily initialize the shared HOME-menu state and its shutdown registration.
inline HomeMenuState* GetHomeMenuState() {
    if (!((HomeMenuState* volatile&)Sing_805DC820())) {
        if (!s_homeMenuInitialized) {
            HomeButtonMenu_InitializeState(&s_homeButtonResourceState);
            __register_global_object(&s_homeButtonResourceState, HomeButtonResources_dtor,
                                     s_homeButtonStateDtorRegistration_801FDF50);
            s_homeMenuInitialized = 1;
        }
        ((HomeMenuState* volatile&)Sing_805DC820()) = &s_homeButtonResourceState;
    }
    return ((HomeMenuState* volatile&)Sing_805DC820());
}
// Lock the list and restart its shared actor iteration.
inline ActorDispatch** ResetActorIteration(ActorList* self) {
    WInstanceLock lock(self->mutex);
    return ResetAndNextActor(self);
}
// Pause for the HOME menu; otherwise process the selected actor and permitted background actors.
void CTask::ProcessActors(float time) {
    ActorList* self = (ActorList*)this;
    HomeMenuState* manager = ((HomeMenuState* volatile&)Sing_805DC820());
    if (!manager) {
        if (!s_homeMenuInitialized) {
            HomeButtonMenu_InitializeState(&s_homeButtonResourceState);
            __register_global_object(&s_homeButtonResourceState, HomeButtonResources_dtor,
                                     s_homeButtonStateDtorRegistration_801FDF50);
            s_homeMenuInitialized = 1;
        }
        ((HomeMenuState* volatile&)Sing_805DC820()) = &s_homeButtonResourceState;
    }
    if (!HomeButtonMenu_IsActive(((HomeMenuState* volatile&)Sing_805DC820()))) {
        if (self->selected)
            self->selected->process(time);
        if (!AsTaskListFields(self)->skipOtherTaskUpdates) {
            if (AsTaskListFields(self)->namedTaskOnly) {
                struct Result {
                    ActorDispatch** item;
                } found;
                // This path requires the configured named actor to exist: the result
                // is dereferenced unconditionally after lookup.
                found.item = FindNamedActor(self, taskScoreExceptionStrings);
                ActorDispatch* value = !found.item ? 0 : *found.item;
                value->process(time);
            } else {
                ActorDispatch** item;
                {
                    WInstanceLock lock(self->mutex);
                    self->current = self->first;
                    item = NextActor(self);
                }
                if (item)
                    while (item) {
                        if (*item != self->selected && !(*item)->isBlocked())
                            (*item)->process(time);
                        item = NextActor(self);
                    }
                if (g_sceneManager)
                    DrawScene_UpdateRegisteredObjects(g_sceneManager, time);
            }
        }
    }
}
}
extern "C" {
struct ActorTypeInfo {
    void* name;
    ActorTypeInfo* parent;
};
struct TaskResource {
    virtual ActorTypeInfo* type();
    virtual void destroy(int);
    virtual void v4();
    virtual void v5();
    virtual void v6();
    virtual void v7();
    virtual void v8();
    virtual void v9();
    virtual void init();
    virtual void v11();
    virtual void start();
    virtual void v13();
    virtual void v14();
    virtual void v15();
    virtual void close();
    virtual void v17();
    virtual void v18();
    virtual void v19();
    virtual void v20();
    virtual void v21();
    virtual void finish();
    TaskResource* owner;
    char pad08[0x44];
    TaskResource* child;
};
// Factory keys use a 12-byte owning small string; tree nodes retain their keys
// and factory callbacks. A lookup owns only its temporary copied key.
// Duplicate search helpers below keep different declaration/lifetime order to
// reproduce parent/child call sites; both implement the same lexicographic lookup.
struct FactoryNameString {
    union {
        struct {
            unsigned int bits, length;
            char* pointer;
        } heap;
        struct {
            unsigned char length;
            char text[11];
        } small;
    };
    // Return the heap-storage bit; a clear bit means inline bytes.
    inline unsigned int large() const {
        return heap.bits >> 31;
    }
    // Test the inline string representation.
    inline bool isshort() const {
        return !large();
    }
    // Read key length in bytes from the active string representation.
    inline unsigned int size() const {
        return isshort() ? (small.length & kInlineLengthMask) : heap.length;
    }
    // Borrow key bytes; ownership stays with the containing string.
    inline const char* data() const {
        return isshort() ? small.text : heap.pointer;
    }
};
struct ActorFactoryNode {
    ActorFactoryNode* left;
    ActorFactoryNode* right;
    ActorFactoryNode* parent;
    FactoryNameString key;
    TaskResource* (*create)();
};
struct ActorFactoryTree {
    char pad[16];
    ActorFactoryNode* root;
};
ActorFactoryTree* ActorFactoryRegistry_GetInstance();
void String_ReserveCapacity(FactoryNameString*, unsigned int);
void String_ReplaceRange(FactoryNameString*, unsigned int, unsigned int, const char*, const char*, const char*);
int memcmp(const char*, const char*, unsigned int);
extern TaskResource* g_lastLoadedChildTaskResource;
extern ActorTypeInfo s_iObjectTypeInfo;
extern unsigned char s_actorLoadingContext[8];
extern unsigned char s_taskTransitionsDeferred;
// Return a reference to the shorter of two lengths.
inline const unsigned int& ShorterLength(const unsigned int& a, const unsigned int& b) {
    return a < b ? a : b;
}

struct LoadAlloc {
    char value;
    char padding[3];
};
// Comparison scratch preserves reference lifetimes used by ShorterStoredLength.
// Loop compares stored key against request; final compares request against candidate.
struct LoadLoopTemps {
    unsigned int count;     // Stored key size from size().
    unsigned int length;    // Requested key length.
    unsigned int remaining; // Stored key size from its active representation.
    unsigned int own;       // Minimum of the two stored-key length observations.
};
struct LoadFinalTemps {
    unsigned int own;       // Minimum of the two requested-key length observations.
    unsigned int remaining; // Requested key size from its active representation.
    unsigned int length;    // Candidate key length.
    unsigned int count;     // Requested key size selected by nameUsesHeap.
};
struct LoadWorkspace {
    LoadAlloc allocator2, copy2, allocator1, copy1;
    LoadFinalTemps final2;
    LoadLoopTemps loop2;
    LoadFinalTemps final1;
    LoadLoopTemps loop1;
    FactoryNameString key2, key1;
};
// Construct a short-or-heap string for an actor-factory lookup.
inline void BuildFactoryName(FactoryNameString& s, const char* text, volatile LoadAlloc& allocator, LoadAlloc& copy) {
    s.heap.bits = 0;
    s.heap.length = 0;
    s.heap.pointer = 0;
    unsigned int length = strlen(text);
    String_ReserveCapacity(&s, length);
    copy.value = allocator.value;
    String_ReplaceRange(&s, 0, 0, text, text + length, &copy.value);
}
// Expose the string bytes and length for either storage representation.
inline void GetFactoryNameSpan(const FactoryNameString* key, const char*& data, unsigned int& length) {
    if (!key->large()) {
        data = key->small.text;
        length = key->small.length & 127;
    } else {
        data = key->heap.pointer;
        length = key->heap.length;
    }
}
// Break a byte-comparison tie using string lengths.
inline int CompareLengthsWhenEqual(int result, const unsigned int& own, unsigned int length) {
    if (!result) {
        if (own < length)
            return -1;
        if (own == length)
            return 0;
        return 1;
    }
    return result;
}
// Choose a stored length by its cached value to preserve comparison lifetimes.
inline const unsigned int& ShorterStoredLength(const unsigned int& a, const unsigned int& b, unsigned int av,
                                               unsigned int bv) {
    return av < bv ? a : b;
}
int __cntlzw(unsigned int);
// Compare a stored key to the request by bytes, then by length; return negative/zero/positive.
// loop owns no memory; it retains the two observations of stored length for the original ABI.
inline int CompareStoredFactoryKey(ActorFactoryNode* node, const char* requestedBytes, unsigned int requestedLength,
                                   LoadLoopTemps& loop) {
    loop.length = requestedLength;
    unsigned int storedSize = node->key.size();
    loop.count = storedSize;
    const char* storedBytes;
    unsigned int storedSpanLength;
    GetFactoryNameSpan(&node->key, storedBytes, storedSpanLength);
    loop.remaining = storedSpanLength;
    unsigned int storedCompareLength = ShorterStoredLength(loop.count, loop.remaining, storedSize, storedSpanLength);
    loop.own = storedCompareLength;
    int storedVsRequest = memcmp(storedBytes, requestedBytes,
                                 ShorterStoredLength(loop.length, loop.own, requestedLength, storedCompareLength));
    storedVsRequest = CompareLengthsWhenEqual(storedVsRequest, loop.own, requestedLength);
    return storedVsRequest;
}

// Find the lower-bound factory key, verify equality, release the temporary name and create the task.
// Compare stored key to requested key by memcmp up to the shorter length, then
// length. Track first stored key >= request (root-address sentinel means absent).
// Recheck the inverse comparison to require equality, free the copied key, and
// invoke create only for an exact hit. A missing key or null factory result returns
// null; the caller of ReplaceActiveTaskByName assumes parent creation succeeds.
inline TaskResource* CreateTaskByName(const char* requestedName, FactoryNameString& key, volatile LoadAlloc& allocator,
                                      LoadAlloc& copy, LoadLoopTemps& loop, LoadFinalTemps& final, int& nameUsesHeap,
                                      ActorFactoryNode*& candidateFactory) {
    // Local order and scratch references retain CodeWarrior register lifetimes;
    // only candidateFactory is a lookup result. All other outputs are observations.
    unsigned int inlineNameLength;
    int requestUsesHeap;
    const char* heapNameData;
    unsigned int heapNameLength;
    unsigned int spanLength;
    ActorFactoryNode* node;
    ActorFactoryTree* registry = ActorFactoryRegistry_GetInstance();
    // Phase 1: own a temporary request key and cache its active representation.
    BuildFactoryName(key, requestedName, allocator, copy);
    requestUsesHeap = key.large();
    node = registry->root;
    candidateFactory = (ActorFactoryNode*)&registry->root;
    unsigned int requestIsInline = (unsigned int)__cntlzw(requestUsesHeap) >> 5;
    nameUsesHeap = (unsigned int)__cntlzw(requestIsInline) >> 5;
    inlineNameLength = key.small.length & 127;
    heapNameData = key.heap.pointer;
    heapNameLength = key.heap.length;
    // Phase 2: lower-bound search; remember the first stored key >= request.
    while (node) {
        const char* requestedBytes;

        if (!nameUsesHeap) {
            spanLength = inlineNameLength;
            requestedBytes = key.small.text;
        } else {
            requestedBytes = heapNameData;
            spanLength = heapNameLength;
        }
        int storedVsRequest = CompareStoredFactoryKey(node, requestedBytes, spanLength, loop);
        if (storedVsRequest >= 0) {
            candidateFactory = node;
            node = node->left;
        } else
            node = node->right;
    }
    // Phase 3: lower-bound alone can be greater than the requested key; reject it.
    // 0x801FE884: validate a candidate only when the search found one.
    do {
        if (candidateFactory != (ActorFactoryNode*)&registry->root) {
            const char* candidateBytes;

            GetFactoryNameSpan(&candidateFactory->key, candidateBytes, spanLength);
            final.length = spanLength;
            unsigned int requestedSize = !nameUsesHeap ? inlineNameLength : key.heap.length;
            final.count = requestedSize;
            const char* requestedBytes;
            unsigned int requestedSpanLength;
            if (!nameUsesHeap) {
                requestedBytes = key.small.text;
                requestedSpanLength = inlineNameLength;
            } else {
                requestedBytes = key.heap.pointer;
                requestedSpanLength = key.heap.length;
            }
            final.remaining = requestedSpanLength;
            unsigned int requestedCompareLength =
                ShorterStoredLength(final.count, final.remaining, requestedSize, requestedSpanLength);
            final.own = requestedCompareLength;
            int requestedVsCandidate =
                memcmp(requestedBytes, candidateBytes,
                       ShorterStoredLength(final.length, final.own, spanLength, requestedCompareLength));
            requestedVsCandidate = CompareLengthsWhenEqual(requestedVsCandidate, final.own, spanLength);
            // candidate >= request from lower-bound; request >= candidate means equality.
            if (requestedVsCandidate >= 0)
                break;
        }
        candidateFactory = (ActorFactoryNode*)&registry->root;
    } while (0);
    // Phase 4: release only the copied request key, then invoke the borrowed factory.
    node = candidateFactory;
    if (requestUsesHeap)
        appFreeUni__FPv(key.heap.pointer);
    return node != (ActorFactoryNode*)&registry->root ? node->create() : 0;
}
// Create the child task through the same factory search with its original temporary ordering.
// Same lookup/ownership/null result contract as CreateTaskByName. Keep this
// declaration order until any consolidation also matches both generated call sites.
inline TaskResource* CreateChildTaskByName(const char* requestedName, FactoryNameString& key,
                                           volatile LoadAlloc& allocator, LoadAlloc& copy, LoadLoopTemps& loop,
                                           LoadFinalTemps& final, int& nameUsesHeap,
                                           ActorFactoryNode*& candidateFactory) {
    // Same logical search as the parent path; this declaration order is ABI scaffolding.
    unsigned int spanLength;
    ActorFactoryNode* node;
    const char* heapNameData;
    unsigned int heapNameLength;
    unsigned int inlineNameLength;
    int requestUsesHeap;
    ActorFactoryTree* registry = ActorFactoryRegistry_GetInstance();
    // Phase 1: own/cache the request; declaration order differs from the parent path.
    BuildFactoryName(key, requestedName, allocator, copy);
    requestUsesHeap = key.large();
    node = registry->root;
    candidateFactory = (ActorFactoryNode*)&registry->root;
    unsigned int requestIsInline = (unsigned int)__cntlzw(requestUsesHeap) >> 5;
    nameUsesHeap = (unsigned int)__cntlzw(requestIsInline) >> 5;
    inlineNameLength = key.small.length & 127;
    heapNameData = key.heap.pointer;
    heapNameLength = key.heap.length;
    // Phase 2: search for the first stored key >= request.
    while (node) {
        const char* requestedBytes;

        if (!nameUsesHeap) {
            spanLength = inlineNameLength;
            requestedBytes = key.small.text;
        } else {
            requestedBytes = heapNameData;
            spanLength = heapNameLength;
        }
        int storedVsRequest = CompareStoredFactoryKey(node, requestedBytes, spanLength, loop);
        if (storedVsRequest >= 0) {
            candidateFactory = node;
            node = node->left;
        } else
            node = node->right;
    }
    // Phase 3: inverse comparison rejects a candidate greater than the request.
    do {
        if (candidateFactory != (ActorFactoryNode*)&registry->root) {
            const char* candidateBytes;

            GetFactoryNameSpan(&candidateFactory->key, candidateBytes, spanLength);
            final.length = spanLength;
            unsigned int requestedSize = !nameUsesHeap ? inlineNameLength : key.heap.length;
            final.count = requestedSize;
            const char* requestedBytes;
            unsigned int requestedSpanLength;
            if (!nameUsesHeap) {
                requestedBytes = key.small.text;
                requestedSpanLength = inlineNameLength;
            } else {
                requestedBytes = key.heap.pointer;
                requestedSpanLength = key.heap.length;
            }
            final.remaining = requestedSpanLength;
            unsigned int requestedCompareLength =
                ShorterStoredLength(final.count, final.remaining, requestedSize, requestedSpanLength);
            final.own = requestedCompareLength;
            int requestedVsCandidate =
                memcmp(requestedBytes, candidateBytes,
                       ShorterStoredLength(final.length, final.own, spanLength, requestedCompareLength));
            requestedVsCandidate = CompareLengthsWhenEqual(requestedVsCandidate, final.own, spanLength);
            // candidate >= request from lower-bound; request >= candidate means equality.
            if (requestedVsCandidate >= 0)
                break;
        }
        candidateFactory = (ActorFactoryNode*)&registry->root;
    } while (0);
    // Phase 4: release the request copy before calling the matched factory.
    node = candidateFactory;
    if (requestUsesHeap)
        appFreeUni__FPv(key.heap.pointer);
    return node != (ActorFactoryNode*)&registry->root ? node->create() : 0;
}
// Walk the runtime type-parent chain looking for the IObject type.
inline bool IsActorType(ActorTypeInfo* type) {
    while (type) {
        if (type == &s_iObjectTypeInfo)
            return true;
        type = type->parent;
    }
    return false;
}
// Check a non-null resource for IObject ancestry.
inline TaskResource* CheckedActorCast(TaskResource* resource) {
    ActorTypeInfo* type = resource->type();
    if (IsActorType(type))
        return resource;
    return 0;
}
// Check a nullable resource for IObject ancestry.
inline TaskResource* NullableActorCast(TaskResource* resource) {
    if (resource) {
        ActorTypeInfo* type = resource->type();
        if (IsActorType(type))
            return resource;
        return 0;
    }
    return 0;
}
#pragma push
#pragma use_lmw_stmw on
// Close the old task, create and link the named parent/child tasks, then run their startup hooks.
// name must name a registered factory returning a valid task: lookup failure has
// no recovery here and later startup dereferences activeTask. childname may be null
// or empty; a child factory miss/non-IObject creates no child link. Parent is owned
// by manager; an accepted child gets reciprocal parent/child links. A created child
// failing the IObject test remains in g_lastLoadedChildTaskResource, not deleted here.
void CTask::ReplaceActiveTaskByName(const char* name, const char* childname) {
    TaskManagerStorage* self = (TaskManagerStorage*)this;
    if (self->activeTask) {
        ((TaskResource*)self->activeTask)->close();
        if (self->activeTask)
            ((TaskResource*)self->activeTask)->destroy(1);
        self->activeTask = 0;
    }
    // Phase 2: old task is gone; create the parent and optional child by exact name.
    int nameUsesHeap;
    ActorFactoryNode* candidateFactory;
    LoadWorkspace workspace;
    self->activeTask = CreateTaskByName(name, workspace.key1, workspace.allocator1, workspace.copy1, workspace.loop1,
                                        workspace.final1, nameUsesHeap, candidateFactory);
    if (childname && strlen(childname)) {
        g_lastLoadedChildTaskResource =
            CreateChildTaskByName(childname, workspace.key2, workspace.allocator2, workspace.copy2, workspace.loop2,
                                  workspace.final2, nameUsesHeap, candidateFactory);
        TaskResource* cast;
        if (g_lastLoadedChildTaskResource) {
            cast = g_lastLoadedChildTaskResource;
            if (!IsActorType(cast->type()))
                cast = 0;
        } else
            cast = 0;
        ((TaskResource*)self->activeTask)->child = cast;
        TaskResource* resource = (TaskResource*)self->activeTask;
        if (resource->child)
            resource->child->owner = resource;
    } else
        ((TaskResource*)self->activeTask)->child = 0;
    // Phase 3: link first, then init/start, clear pending transition names, finish, and defer
    // subsequent transitions. Startup callbacks can observe the published parent.
    ((TaskResource*)self->activeTask)->init();
    ((TaskResource*)self->activeTask)->start();
    int zero = 0;
    // Loading-context bytes 4/5 are also cleared by the adjacent transition flow.
    s_actorLoadingContext[5] = zero;
    s_actorLoadingContext[4] = zero;
    self->transitionFlag = 0;
    self->requestedParentTaskName = 0;
    self->requestedChildTaskName = 0;
    ((TaskResource*)self->activeTask)->finish();
    s_taskTransitionsDeferred = 1;
}
#pragma pop
}

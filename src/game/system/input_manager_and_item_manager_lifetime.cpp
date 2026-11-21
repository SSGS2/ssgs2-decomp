#include <decomp/static_init.h>
// Controller input/action queries and item-table lifetime support.
// Lookup index:
// - Button edges: Pointer_GetRisingActionBits; InputPointerState/CInputManager
//   headers describe the pointer and recursive mutex, s_monitoredActionNames
//   selects the nine names tested by CInputManager_HasAnyMonitoredAction.
// - Pointer movement/suppression: Pointer_GetMotionDelta, Pointer_SuppressInput,
//   Pointer_ClearInputSuppression; deltas are copied as float bits, not scaled.
// - Per-update action cleanup: InputManager_ClearNodeFlagBit1 (contract below).
// - IFF item storage: CItemManager_Ctor/dtor and the thirteen descriptors in
//   classes/CItemManager.h; this unit frees tables, it does not load/lookup items.
// - Container deleting destructors release storage, never pointed-to items.
// InputItemTables_InitFullTurn copies the shared 2*pi float; no cadence tuning
// or item-table values are defined here. ABI addresses remain in symbols.txt.
#include "MSL/vector.h"
#include "MSL/list.h"
// Placeholder element types for the containers whose deleting destructors this unit keeps out of line:
// the destructor depends only on whether the element is a scalar (pointer) or a record.
struct MSLRecord { unsigned int words[1]; };
typedef std::vector<void*, app::app_allocator<void*> > MSLPointerVector;
typedef std::vector<MSLRecord, app::app_allocator<MSLRecord> > MSLRecordVector;
typedef std::list<void*, app::app_allocator<void*> > MSLPointerList;
#include "classes/CInputManager.h"
#include "classes/InputPointerState.h"
// Pointer action lookup/traversal locks the input-manager mutex at original +0x30.
#include "classes/CItemManager.h"

#define AsCItemManager(p) ((CItemManager*)(p))
// Translation unit 0x8018747C-0x8018839C (23 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

enum { kMonitoredActionCount = 9, kRequireActiveBinding = 1, kActionPressed = 1, kInputSuppressed = 2, kPointerAdapterDevice = 2 };

extern "C" {

extern float kFloat_6_2831855;
extern float s_inputItemTablesFullTurn;
extern char __vt__12CItemManager[];
extern char* s_monitoredActionNames[];

// Word views preserve the original load/store order of the float motion delta.
struct PointerMotionWords {
    char padding00[0x48];
    unsigned int deltaXBits;
    unsigned int deltaYBits;
    unsigned int deltaZBits;
};

struct MotionDeltaWords {
    unsigned int xBits;
    unsigned int yBits;
    unsigned int zBits;
};

struct BindingDeviceAdapter {
    virtual void slot0();
    virtual void slot1();
    virtual void slot2();
    virtual void slot3();
    virtual void slot4();
    virtual void slot5();
    virtual void slot6();
    virtual void slot7();
    virtual void* call(int, void*);
};

struct EnabledPointerState {
    char padding00[0xE4];
    unsigned char enabled;
};

// Table descriptor shared by the dtor_801879F4..dtor_80187FC4 family: `owned` is a heap buffer freed with
// appFreeUni__FPv, `count` is the entry count. In every destructor of the family the nested repeated
// "object != 0" tests and the `count -= (object+count)-object` expression are compiler-matching forms of
// plain C: `if (object) { if (object->owned) { object->count = 0; free(object->owned); } if (shouldDelete > 0) delete object; }`
// (a flattened rewrite was tried and does not compile identically, so the original shape is kept).
struct VolatileOwnedTable {
    void* owned;
    volatile int count;
};

struct OwnedTable {
    void* owned;
    int count;
};

extern BindingDeviceAdapter* g_bindingDeviceAdapter;
extern void appFreeUni__FPv(void*);
extern void __dl__FPv(void*);
extern void Mutex_Init(void*);
extern void OSLockMutex(void*);
extern void OSUnlockMutex(void*);
extern int strcmp(const char*, const char*);
extern unsigned int strlen(const char*);

struct WInstanceLock {
    void* lock;
    // Acquire one recursive input-manager mutex level.
    WInstanceLock(void* p) { lock = p; OSLockMutex(p); }
    // Release the matching mutex level on normal or exceptional exit.
    ~WInstanceLock() { OSUnlockMutex(lock); }
};


// Word 0 supplies the action payload; +0x10 supplies the circular iteration link.
struct ActionIterationNode {
    ActionIterationNode* payload;
    char padding04[0x0C];
    ActionIterationNode* iterationLink;
    unsigned int flags;
};

struct ActionIterationState {
    char padding00[0x10];
    ActionIterationNode* first;
    ActionIterationNode* current;
    char padding18[0x18];
};

// Two views deliberately share this type: bucket NODES have payload/key/link;
// a successful comparison replaces node with node->payload. Only then are
// flags/value/linkFlags interpreted as action STATE, not hash-chain members.
// For the edge query: payload.flags=current action bits, value=previous bits,
// linkFlags bit0=active binding. current & (current ^ previous) == current &
// ~previous gives a bitmask of rising edges, not a boolean or just pressed bit0.
struct ActionEdgeNode {
    ActionEdgeNode* payload;
    const char* key;
    char padding08[0x0C];
    union {
        ActionEdgeNode* link;
        unsigned int linkFlags;
    };
    unsigned int flags;
    unsigned int value;
};

struct ActionEdgeLookupState {
    char padding00[0x24];
    unsigned int bucketMask;
    char padding28[4];
    ActionEdgeNode** buckets;
    char padding30[0xAC];
    int lookupMode;
    char paddingE0[4];
    unsigned char enabled;
};

struct MonitoredActionLookupState {
    char padding00[0x24];
    unsigned int bucketMask;
    char padding28[4];
    ActionEdgeNode** buckets;
    char padding30[0xAC];
    int lookupMode;
    char paddingE0[4];
    unsigned char enabled;
};

// Same node/payload overlay, but the monitored query's payload view reads
// flags at +0x14 (active bit0) and value at +0x18 (current action bits).
// Do not substitute the edge query's payload offsets: its state starts later.
struct MonitoredActionNode {
    MonitoredActionNode* payload;
    const char* key;
    char padding08[0x0C];
    union {
        MonitoredActionNode* link;
        unsigned int flags;
    };
    unsigned int value;
};


// Initialize this input/item unit's full-turn constant.
void InputItemTables_InitFullTurn() {
    s_inputItemTablesFullTurn = kFloat_6_2831855;
}

#pragma use_lmw_stmw on
#pragma opt_loop_invariants off
#pragma opt_common_subs off
#pragma opt_lifetimes off
// Original CInputManager_HasAnyMonitoredAction: release the lookup lock on both search outcomes.
inline void FindMonitoredActionAndUnlock(MonitoredActionNode*& node, const char* key, void* volatile& lock) {
    while (node) {
        if (strcmp(node->key, key) == 0) {
            node = node->payload;
            OSUnlockMutex(lock);
            return;
        }
        node = node->link;
    }
    OSUnlockMutex(lock);
    node = 0;
}

// Return whether any of the nine monitored action names is currently pressed.
int CInputManager_HasAnyMonitoredAction(void* inputManager) {
    // Read-only query over nine borrowed names. Each iteration locks lookup,
    // recursively locks to hash, then FindMonitoredActionAndUnlock releases the
    // outer lookup lock whether found or absent. The mutex must be recursive.
    // Hash = ((length<<6) | (first&0x2a) | (last&0x15)) & bucketMask.
    // Null names/missing buckets yield no action. Mode 1 additionally requires
    // active binding; result short-circuiting protects node->flags on a miss.
    MonitoredActionLookupState* object = (MonitoredActionLookupState*)inputManager;
    const char* key;
    const char** monitoredActionName = (const char**)s_monitoredActionNames;
    unsigned int i = 0;
    for (; i < kMonitoredActionCount; ++i, ++monitoredActionName) {
        key = *monitoredActionName;
        int found;
        MonitoredActionNode* node;
        if (!object->enabled) {
            found = 0;
        } else {
            void* volatile lock;
            OSLockMutex(lock = ((CInputManager*)object)->mutex);
            if (!key || !object->buckets) {
                OSUnlockMutex(lock);
                node = 0;
            } else {
                unsigned int hash;
                {
                    void* inner = ((CInputManager*)object)->mutex;
                    OSLockMutex(inner);
                    int len = (int)strlen(key);
                    int lastIndex = len > 0 ? len - 1 : 0;
                    unsigned int tail = (unsigned char)key[lastIndex];
                    hash = ((((unsigned int)len << 6) | (((unsigned char)key[0] & 0x2A) | (tail & 0x15))) & object->bucketMask);
                    OSUnlockMutex(inner);
                }
                node = (MonitoredActionNode*)object->buckets[hash];
                FindMonitoredActionAndUnlock(node, key, lock);
            }
            unsigned int result = node ? node->value : 0;
            if (object->lookupMode == kRequireActiveBinding && result && !(node->flags & 1)) result = 0;
            found = result & kActionPressed;
        }
        if (found) return 1;
    }
    return 0;
}
#pragma opt_loop_invariants reset
#pragma opt_common_subs reset
#pragma use_lmw_stmw reset

#pragma opt_propagation off
#pragma use_lmw_stmw on
#pragma opt_lifetimes on
// Field-derived byte addresses retain CodeWarrior register allocation in the table guards.
// Release all owned IFF table buffers and optionally delete the item manager; preserve the original clear order.
void* CItemManager_dtor(CItemManager* object, int shouldDelete) {
    // Phase 1: empty all descriptors and disable manager locking. Expressions
    // size-((object+size)-object) compute zero; the aliases/guards reproduce
    // inlined vector cleanup and are not multiple independent allocations.
    // Phase 2: free thirteen independent owned buffers in reverse declaration
    // order (aux parts through characters, including clubSets). Inline records
    // need no per-entry cleanup. Pointers/capacities are not reset: this object
    // is being destroyed, not prepared for reuse. Phase 3: delete iff requested.
    if (object) {
        int emptyCharacterCount;
        int emptyPartCount;
        int emptyClubCount;
        int emptyBallCount;
        int emptyItemCount;
        int emptyCaddieCount;
        int emptyCaddieItemCount;
        int emptySetItemCount;
        int emptyCourseCount;
        int emptyHairStyleCount;
        int emptyDescriptionCount;
        int emptyAuxPartCount;
        object->vtbl = __vt__12CItemManager;
        emptyCharacterCount = object->characters.size - (((char*)object + object->characters.size) - (char*)object);
        emptyPartCount = object->parts.size - (((char*)object + object->parts.size) - (char*)object);
        emptyClubCount = object->clubs.size - (((char*)object + object->clubs.size) - (char*)object);
        emptyBallCount = object->balls.size - (((char*)object + object->balls.size) - (char*)object);
        emptyItemCount = object->items.size - (((char*)object + object->items.size) - (char*)object);
        emptyCaddieCount = object->caddies.size - (((char*)object + object->caddies.size) - (char*)object);
        emptyCaddieItemCount = object->caddieItems.size - (((char*)object + object->caddieItems.size) - (char*)object);
        emptySetItemCount = object->setItems.size - (((char*)object + object->setItems.size) - (char*)object);
        emptyCourseCount = object->courses.size - (((char*)object + object->courses.size) - (char*)object);
        emptyHairStyleCount = object->hairStyles.size - (((char*)object + object->hairStyles.size) - (char*)object);
        emptyDescriptionCount = object->descriptions.size - (((char*)object + object->descriptions.size) - (char*)object);
        emptyAuxPartCount = object->auxParts.size - (((char*)object + object->auxParts.size) - (char*)object);
        object->vtbl = __vt__12CItemManager;
        object->characters.size = emptyCharacterCount;
        object->parts.size = emptyPartCount;
        object->clubs.size = emptyClubCount;
        object->balls.size = emptyBallCount;
        object->items.size = emptyItemCount;
        object->caddies.size = emptyCaddieCount;
        object->caddieItems.size = emptyCaddieItemCount;
        object->setItems.size = emptySetItemCount;
        object->courses.size = emptyCourseCount;
        object->hairStyles.size = emptyHairStyleCount;
        object->descriptions.size = emptyDescriptionCount;
        object->auxParts.size = emptyAuxPartCount;
        AsCItemManager(object)->lockEnabled = 0;
        if ((char*)object + IFF_OFFSET(CItemManager, auxParts) && (char*)object + IFF_OFFSET(CItemManager, auxParts) && (char*)object + IFF_OFFSET(CItemManager, auxParts) && (char*)object + IFF_OFFSET(CItemManager, auxParts)) {
            if (object->auxParts.data) {
                object->auxParts.size = emptyAuxPartCount - (((char*)object + emptyAuxPartCount) - (char*)object);
                appFreeUni__FPv((void*)object->auxParts.data);
            }
        }
        if ((char*)object + IFF_OFFSET(CItemManager, hairStyles) && (char*)object + IFF_OFFSET(CItemManager, hairStyles) && (char*)object + IFF_OFFSET(CItemManager, hairStyles) && (char*)object + IFF_OFFSET(CItemManager, hairStyles)) {
            if (object->hairStyles.data) {
                object->hairStyles.size -= ((char*)object + object->hairStyles.size) - (char*)object;
                appFreeUni__FPv((void*)object->hairStyles.data);
            }
        }
        if ((char*)object + IFF_OFFSET(CItemManager, descriptions) && (char*)object + IFF_OFFSET(CItemManager, descriptions) && (char*)object + IFF_OFFSET(CItemManager, descriptions) && (char*)object + IFF_OFFSET(CItemManager, descriptions)) {
            if (object->descriptions.data) {
                object->descriptions.size -= ((char*)object + object->descriptions.size) - (char*)object;
                appFreeUni__FPv((void*)object->descriptions.data);
            }
        }
        if ((char*)object + IFF_OFFSET(CItemManager, courses) && (char*)object + IFF_OFFSET(CItemManager, courses) && (char*)object + IFF_OFFSET(CItemManager, courses) && (char*)object + IFF_OFFSET(CItemManager, courses)) {
            if (object->courses.data) {
                object->courses.size -= ((char*)object + object->courses.size) - (char*)object;
                appFreeUni__FPv((void*)object->courses.data);
            }
        }
        if ((char*)object + IFF_OFFSET(CItemManager, setItems) && (char*)object + IFF_OFFSET(CItemManager, setItems) && (char*)object + IFF_OFFSET(CItemManager, setItems) && (char*)object + IFF_OFFSET(CItemManager, setItems)) {
            if (object->setItems.data) {
                object->setItems.size -= ((char*)object + object->setItems.size) - (char*)object;
                appFreeUni__FPv((void*)object->setItems.data);
            }
        }
        if ((char*)object + IFF_OFFSET(CItemManager, caddieItems) && (char*)object + IFF_OFFSET(CItemManager, caddieItems) && (char*)object + IFF_OFFSET(CItemManager, caddieItems) && (char*)object + IFF_OFFSET(CItemManager, caddieItems)) {
            if (object->caddieItems.data) {
                object->caddieItems.size -= ((char*)object + object->caddieItems.size) - (char*)object;
                appFreeUni__FPv((void*)object->caddieItems.data);
            }
        }
        if ((char*)object + IFF_OFFSET(CItemManager, caddies) && (char*)object + IFF_OFFSET(CItemManager, caddies) && (char*)object + IFF_OFFSET(CItemManager, caddies) && (char*)object + IFF_OFFSET(CItemManager, caddies)) {
            if (object->caddies.data) {
                object->caddies.size -= ((char*)object + object->caddies.size) - (char*)object;
                appFreeUni__FPv((void*)object->caddies.data);
            }
        }
        if ((char*)object + IFF_OFFSET(CItemManager, items) && (char*)object + IFF_OFFSET(CItemManager, items) && (char*)object + IFF_OFFSET(CItemManager, items) && (char*)object + IFF_OFFSET(CItemManager, items)) {
            if (object->items.data) {
                object->items.size -= ((char*)object + object->items.size) - (char*)object;
                appFreeUni__FPv((void*)object->items.data);
            }
        }
        if ((char*)object + IFF_OFFSET(CItemManager, balls) && (char*)object + IFF_OFFSET(CItemManager, balls) && (char*)object + IFF_OFFSET(CItemManager, balls) && (char*)object + IFF_OFFSET(CItemManager, balls)) {
            if (object->balls.data) {
                object->balls.size -= ((char*)object + object->balls.size) - (char*)object;
                appFreeUni__FPv((void*)object->balls.data);
            }
        }
        if ((char*)object + IFF_OFFSET(CItemManager, clubSets) && (char*)object + IFF_OFFSET(CItemManager, clubSets) && (char*)object + IFF_OFFSET(CItemManager, clubSets) && (char*)object + IFF_OFFSET(CItemManager, clubSets)) {
            if (object->clubSets.data) {
                object->clubSets.size -= ((char*)object + object->clubSets.size) - (char*)object;
                appFreeUni__FPv((void*)object->clubSets.data);
            }
        }
        if ((char*)object + IFF_OFFSET(CItemManager, clubs) && (char*)object + IFF_OFFSET(CItemManager, clubs) && (char*)object + IFF_OFFSET(CItemManager, clubs) && (char*)object + IFF_OFFSET(CItemManager, clubs)) {
            if (object->clubs.data) {
                object->clubs.size -= ((char*)object + object->clubs.size) - (char*)object;
                appFreeUni__FPv((void*)object->clubs.data);
            }
        }
        if ((char*)object + IFF_OFFSET(CItemManager, parts) && (char*)object + IFF_OFFSET(CItemManager, parts) && (char*)object + IFF_OFFSET(CItemManager, parts) && (char*)object + IFF_OFFSET(CItemManager, parts)) {
            if (object->parts.data) {
                object->parts.size -= ((char*)object + object->parts.size) - (char*)object;
                appFreeUni__FPv((void*)object->parts.data);
            }
        }
        if ((char*)object + IFF_OFFSET(CItemManager, characters) && (char*)object + IFF_OFFSET(CItemManager, characters) && (char*)object + IFF_OFFSET(CItemManager, characters) && (char*)object + IFF_OFFSET(CItemManager, characters)) {
            if (object->characters.data) {
                object->characters.size -= ((char*)object + object->characters.size) - (char*)object;
                appFreeUni__FPv((void*)object->characters.data);
            }
        }
        if (shouldDelete > 0) __dl__FPv(object);
    }
    return object;
}
#pragma use_lmw_stmw reset
#pragma opt_propagation reset
#pragma opt_lifetimes off

// Copy the three motion-delta float bit patterns without converting them.
void Pointer_GetMotionDelta(MotionDeltaWords* result, PointerMotionWords* data) {
    unsigned int first = data->deltaYBits;
    unsigned int second = data->deltaXBits;
    result->xBits = second;
    result->yBits = first;
    result->zBits = data->deltaZBits;
}

// Suppress all three pointer buttons and set the pointer suppression flag.
void Pointer_SuppressInput(InputPointerState* object) {
    object->stateFlags |= kInputSuppressed;
    object->buttons[0].suppressPressed = 1;
    object->buttons[1].suppressPressed = 1;
    object->buttons[2].suppressPressed = 1;
}

// Restore all three pointer buttons and clear the pointer suppression flag.
void Pointer_ClearInputSuppression(InputPointerState* object) {
    object->stateFlags &= ~kInputSuppressed;
    object->buttons[0].suppressPressed = 0;
    object->buttons[1].suppressPressed = 0;
    object->buttons[2].suppressPressed = 0;
}

// Request the pointer binding device only while this pointer is enabled.
void* Pointer_GetEnabledBindingDevice(EnabledPointerState* object, void* arg) {
    if (object->enabled) {
        return g_bindingDeviceAdapter->call(kPointerAdapterDevice, arg);
    }
    return 0;
}

// Zero the thirteen IFF table descriptors, initialize their mutex, and disable locking.
void* CItemManager_Ctor(CItemManager* object) {
    unsigned int* p = (unsigned int*)&object->characters;
    p[0] = 0;
    object->vtbl = __vt__12CItemManager;
    for (int i = 1; i < 39; ++i) p[i] = 0;
    Mutex_Init(object->inventoryMutexStorage);
    object->lockEnabled = 0;
    return object;
}

#pragma opt_common_subs off
// Clear bit 1 on each action payload while taking the input mutex for each iterator step.
// Required: a live manager with a circular first/link ring, or first==0 for empty.
// Each ring node supplies a payload live for the entire call. A null payload
// ends traversal early. current is shared iterator state, reset to first here;
// after the last link it becomes null. Do not nest another traversal on this manager.
// Lock scope: initialization takes an RAII level plus a recursive step level;
// later steps lock only while reading current/link/payload and advancing current.
// The payload flags &= ~2 write happens AFTER unlocking. The step lock alone
// therefore does not protect payload lifetime or a concurrent flags update.
// Callers must serialize ring insert/erase, payload destruction/flag mutation,
// and other uses of current across the WHOLE call (e.g. owning update thread).
// No nodes are allocated/freed here; only payload bit1 and manager.current change.
void InputManager_ClearNodeFlagBit1(ActionIterationState* object) {
    ActionIterationNode* node = 0;
    char* lock;
    {
        WInstanceLock initial(((CInputManager*)object)->mutex);
        object->current = object->first;
        lock = ((CInputManager*)object)->mutex;
        OSLockMutex(lock);
        ActionIterationNode* link;
        ActionIterationNode* cur = object->current;
        if (cur) {
            link = cur->iterationLink;
            node = cur->payload;
            if (link == object->first) link = 0;
            object->current = link;
            OSUnlockMutex(lock);
        } else {
            OSUnlockMutex(lock);
            node = 0;
        }
    }
    if (node) {
        char* loopLock = ((CInputManager*)object)->mutex;
        while (node) {
            node->flags &= ~2u;
            OSLockMutex(loopLock);
            ActionIterationNode* link;
            ActionIterationNode* cur = object->current;
            if (cur) {
                link = cur->iterationLink;
                ActionIterationNode* nextPayload = cur->payload;
                if (link == object->first) link = 0;
                object->current = link;
                OSUnlockMutex(loopLock);
                node = nextPayload;
            } else {
                OSUnlockMutex(loopLock);
                node = 0;
            }
        }
    }
}
#pragma opt_common_subs reset

// Rising-action-bit lookup: release the lock on both search outcomes.
inline void FindActionEdgeAndUnlock(ActionEdgeNode*& node, const char* key, void* volatile& lock) {
    while (node) {
        if (strcmp(node->key, key) == 0) {
            node = node->payload;
            OSUnlockMutex(lock);
            return;
        }
        node = node->link;
    }
    OSUnlockMutex(lock);
    node = 0;
}

// Return rising action bits, optionally requiring the binding to be active in mode 1.
unsigned int Pointer_GetRisingActionBits(ActionEdgeLookupState* object, const char* key, int requireActiveBinding) {
    // Returns the full rising-bit mask; disabled/missing lookup normally gives
    // zero. With requireActiveBinding && mode==1, a SUCCESSFUL lookup is a caller
    // precondition: the final active-bit test dereferences node even on a miss.
    // Preserve that behavior; adding a null guard would be a functional fix.
    // The internal node/payload is borrowed; the RETURN is an unsigned bit mask.
    // Only lookup/traversal is mutex-locked: the owner must keep payload and key
    // alive and serialize action-state writes through the post-unlock bit tests.
    if (!object->enabled) return 0;
    ActionEdgeNode* node;
    {
        void* volatile lock;
        lock = ((CInputManager*)object)->mutex;
        OSLockMutex(((CInputManager*)object)->mutex);
        if (!key || !object->buckets) {
            OSUnlockMutex(lock);
            node = 0;
        } else {
            unsigned int hash;
            {
                void* inner = ((CInputManager*)object)->mutex;
                OSLockMutex(inner);
                int len = (int)strlen(key);
                unsigned int first = (unsigned char)key[0] & 0x2A;
                unsigned int tail = (unsigned char)key[len > 0 ? len - 1 : 0] & 0x15;
                hash = (((unsigned int)len << 6) | (first | tail)) & object->bucketMask;
                OSUnlockMutex(inner);
            }
            node = object->buckets[hash];
            FindActionEdgeAndUnlock(node, key, lock);
        }
    }
    unsigned int result;
    if (node) result = node->flags & (node->flags ^ node->value);
    else result = 0;
    if (requireActiveBinding && object->lookupMode == kRequireActiveBinding && !(node->linkFlags & 1)) result = 0;
    return result;
}

// Clear and free the owned table buffer, then optionally delete its descriptor.
void* dtor_801879F4(MSLRecordVector* self, int shouldDelete) {
    return ::std::__vector_deleting_dtor(self, shouldDelete);
}

// Clear and free the owned table buffer, then optionally delete its descriptor.
void* dtor_80187A70(MSLRecordVector* self, int shouldDelete) {
    return ::std::__vector_deleting_dtor(self, shouldDelete);
}

// Clear and free the owned table buffer, then optionally delete its descriptor.
void* dtor_80187AEC(MSLRecordVector* self, int shouldDelete) {
    return ::std::__vector_deleting_dtor(self, shouldDelete);
}

// Clear and free the owned table buffer, then optionally delete its descriptor.
void* dtor_80187B68(MSLRecordVector* self, int shouldDelete) {
    return ::std::__vector_deleting_dtor(self, shouldDelete);
}

// Clear and free the owned table buffer, then optionally delete its descriptor.
void* dtor_80187BE4(MSLRecordVector* self, int shouldDelete) {
    return ::std::__vector_deleting_dtor(self, shouldDelete);
}

// Clear and free the owned table buffer, then optionally delete its descriptor.
void* dtor_80187C60(MSLRecordVector* self, int shouldDelete) {
    return ::std::__vector_deleting_dtor(self, shouldDelete);
}

// Clear and free the owned table buffer, then optionally delete its descriptor.
void* dtor_80187CDC(MSLRecordVector* self, int shouldDelete) {
    return ::std::__vector_deleting_dtor(self, shouldDelete);
}

// Clear and free the owned table buffer, then optionally delete its descriptor.
void* dtor_80187D58(MSLRecordVector* self, int shouldDelete) {
    return ::std::__vector_deleting_dtor(self, shouldDelete);
}

// Clear and free the owned table buffer, then optionally delete its descriptor.
void* dtor_80187DD4(MSLRecordVector* self, int shouldDelete) {
    return ::std::__vector_deleting_dtor(self, shouldDelete);
}

// Clear and free the owned table buffer, then optionally delete its descriptor.
void* dtor_80187E50(MSLRecordVector* self, int shouldDelete) {
    return ::std::__vector_deleting_dtor(self, shouldDelete);
}

// Clear and free the owned table buffer, then optionally delete its descriptor.
void* dtor_80187ECC(MSLRecordVector* self, int shouldDelete) {
    return ::std::__vector_deleting_dtor(self, shouldDelete);
}

// Clear and free the owned table buffer, then optionally delete its descriptor.
void* dtor_80187F48(MSLRecordVector* self, int shouldDelete) {
    return ::std::__vector_deleting_dtor(self, shouldDelete);
}

// Clear and free the owned table buffer, then optionally delete its descriptor.
void* dtor_80187FC4(MSLRecordVector* self, int shouldDelete) {
    return ::std::__vector_deleting_dtor(self, shouldDelete);
}

} // extern "C"

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU80462150", _ctors_u8018747C_0, InputItemTables_InitFullTurn);
#pragma pop

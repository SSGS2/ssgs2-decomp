// Controller input queries for game actions and pointer state (position, motion and suppression),
// plus the lifetime of the item manager's inventory tables.
// The item tables are freed here; loading and looking up items happens elsewhere.

#include <decomp/static_init.h>
// Controller input/action queries and item-table lifetime support.
// Lookup index:
// - Button edges: Pointer_GetRisingActionBits; InputPointerState/CInputManager
//   headers describe the pointer and recursive mutex, s_monitoredActionNames
//   selects the nine names tested by CInputManager::HasAnyMonitoredAction.
// - Pointer movement/suppression: Pointer_GetMotionDelta, Pointer_SuppressInput,
//   Pointer_ClearInputSuppression; deltas are copied as float bits, not scaled.
// - Per-update action cleanup: InputManager_ClearNodeFlagBit1 (contract below).
// - IFF item storage: CItemManager::CItemManager/dtor and the thirteen descriptors in
//   classes/CItemManager.h; this unit frees tables, it does not load/lookup items.
// - Container deleting destructors release storage, never pointed-to items.
// InputItemTables_InitFullTurn copies the shared 2*pi float; no cadence tuning
// or item-table values are defined here. ABI addresses remain in symbols.txt.
#include "MSL/vector.h"
#include "MSL/list.h"
// Placeholder element types for the containers whose deleting destructors this unit keeps out of line:
// the destructor depends only on whether the element is a scalar (pointer) or a record.
struct MSLRecord {
    unsigned int words[1];
};
typedef std::vector<void*, app::app_allocator<void*> > MSLPointerVector;
typedef std::vector<MSLRecord, app::app_allocator<MSLRecord> > MSLRecordVector;
typedef std::list<void*, app::app_allocator<void*> > MSLPointerList;
#include "classes/CInputManager.h"
#include "classes/InputPointerState.h"
// Pointer action lookup/traversal locks the input-manager mutex at original +0x30.
#include "classes/CItemManager.h"

enum {
    kMonitoredActionCount = 9,
    kRequireActiveBinding = 1,
    kActionPressed = 1,
    kInputSuppressed = 2,
    kPointerAdapterDevice = 2
};

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
    WInstanceLock(void* p) {
        lock = p;
        OSLockMutex(p);
    }
    // Release the matching mutex level on normal or exceptional exit.
    ~WInstanceLock() {
        OSUnlockMutex(lock);
    }
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
// Original CInputManager::HasAnyMonitoredAction: release the lookup lock on both search outcomes.
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
int CInputManager::HasAnyMonitoredAction(void* inputManager) {
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
                    hash = ((((unsigned int)len << 6) | (((unsigned char)key[0] & 0x2A) | (tail & 0x15))) &
                            object->bucketMask);
                    OSUnlockMutex(inner);
                }
                node = (MonitoredActionNode*)object->buckets[hash];
                FindMonitoredActionAndUnlock(node, key, lock);
            }
            unsigned int result = node ? node->value : 0;
            if (object->lookupMode == kRequireActiveBinding && result && !(node->flags & 1))
                result = 0;
            found = result & kActionPressed;
        }
        if (found)
            return 1;
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
CItemManager::~CItemManager() {
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
    vtbl = __vt__12CItemManager;
    emptyCharacterCount = this->characters.size - (((char*)this + this->characters.size) - (char*)this);
    emptyPartCount = this->parts.size - (((char*)this + this->parts.size) - (char*)this);
    emptyClubCount = this->clubs.size - (((char*)this + this->clubs.size) - (char*)this);
    emptyBallCount = this->balls.size - (((char*)this + this->balls.size) - (char*)this);
    emptyItemCount = this->items.size - (((char*)this + this->items.size) - (char*)this);
    emptyCaddieCount = this->caddies.size - (((char*)this + this->caddies.size) - (char*)this);
    emptyCaddieItemCount = this->caddieItems.size - (((char*)this + this->caddieItems.size) - (char*)this);
    emptySetItemCount = this->setItems.size - (((char*)this + this->setItems.size) - (char*)this);
    emptyCourseCount = this->courses.size - (((char*)this + this->courses.size) - (char*)this);
    emptyHairStyleCount = this->hairStyles.size - (((char*)this + this->hairStyles.size) - (char*)this);
    emptyDescriptionCount = this->descriptions.size - (((char*)this + this->descriptions.size) - (char*)this);
    emptyAuxPartCount = this->auxParts.size - (((char*)this + this->auxParts.size) - (char*)this);
    vtbl = __vt__12CItemManager;
    this->characters.size = emptyCharacterCount;
    this->parts.size = emptyPartCount;
    this->clubs.size = emptyClubCount;
    this->balls.size = emptyBallCount;
    this->items.size = emptyItemCount;
    this->caddies.size = emptyCaddieCount;
    this->caddieItems.size = emptyCaddieItemCount;
    this->setItems.size = emptySetItemCount;
    this->courses.size = emptyCourseCount;
    this->hairStyles.size = emptyHairStyleCount;
    this->descriptions.size = emptyDescriptionCount;
    this->auxParts.size = emptyAuxPartCount;
    lockEnabled = 0;
    if ((char*)this + IFF_OFFSET(CItemManager, auxParts) && (char*)this + IFF_OFFSET(CItemManager, auxParts) &&
        (char*)this + IFF_OFFSET(CItemManager, auxParts) && (char*)this + IFF_OFFSET(CItemManager, auxParts)) {
        if (this->auxParts.data) {
            this->auxParts.size = emptyAuxPartCount - (((char*)this + emptyAuxPartCount) - (char*)this);
            appFreeUni__FPv((void*)this->auxParts.data);
        }
    }
    if ((char*)this + IFF_OFFSET(CItemManager, hairStyles) && (char*)this + IFF_OFFSET(CItemManager, hairStyles) &&
        (char*)this + IFF_OFFSET(CItemManager, hairStyles) && (char*)this + IFF_OFFSET(CItemManager, hairStyles)) {
        if (this->hairStyles.data) {
            this->hairStyles.size -= ((char*)this + this->hairStyles.size) - (char*)this;
            appFreeUni__FPv((void*)this->hairStyles.data);
        }
    }
    if ((char*)this + IFF_OFFSET(CItemManager, descriptions) && (char*)this + IFF_OFFSET(CItemManager, descriptions) &&
        (char*)this + IFF_OFFSET(CItemManager, descriptions) && (char*)this + IFF_OFFSET(CItemManager, descriptions)) {
        if (this->descriptions.data) {
            this->descriptions.size -= ((char*)this + this->descriptions.size) - (char*)this;
            appFreeUni__FPv((void*)this->descriptions.data);
        }
    }
    if ((char*)this + IFF_OFFSET(CItemManager, courses) && (char*)this + IFF_OFFSET(CItemManager, courses) &&
        (char*)this + IFF_OFFSET(CItemManager, courses) && (char*)this + IFF_OFFSET(CItemManager, courses)) {
        if (this->courses.data) {
            this->courses.size -= ((char*)this + this->courses.size) - (char*)this;
            appFreeUni__FPv((void*)this->courses.data);
        }
    }
    if ((char*)this + IFF_OFFSET(CItemManager, setItems) && (char*)this + IFF_OFFSET(CItemManager, setItems) &&
        (char*)this + IFF_OFFSET(CItemManager, setItems) && (char*)this + IFF_OFFSET(CItemManager, setItems)) {
        if (this->setItems.data) {
            this->setItems.size -= ((char*)this + this->setItems.size) - (char*)this;
            appFreeUni__FPv((void*)this->setItems.data);
        }
    }
    if ((char*)this + IFF_OFFSET(CItemManager, caddieItems) && (char*)this + IFF_OFFSET(CItemManager, caddieItems) &&
        (char*)this + IFF_OFFSET(CItemManager, caddieItems) && (char*)this + IFF_OFFSET(CItemManager, caddieItems)) {
        if (this->caddieItems.data) {
            this->caddieItems.size -= ((char*)this + this->caddieItems.size) - (char*)this;
            appFreeUni__FPv((void*)this->caddieItems.data);
        }
    }
    if ((char*)this + IFF_OFFSET(CItemManager, caddies) && (char*)this + IFF_OFFSET(CItemManager, caddies) &&
        (char*)this + IFF_OFFSET(CItemManager, caddies) && (char*)this + IFF_OFFSET(CItemManager, caddies)) {
        if (this->caddies.data) {
            this->caddies.size -= ((char*)this + this->caddies.size) - (char*)this;
            appFreeUni__FPv((void*)this->caddies.data);
        }
    }
    if ((char*)this + IFF_OFFSET(CItemManager, items) && (char*)this + IFF_OFFSET(CItemManager, items) &&
        (char*)this + IFF_OFFSET(CItemManager, items) && (char*)this + IFF_OFFSET(CItemManager, items)) {
        if (this->items.data) {
            this->items.size -= ((char*)this + this->items.size) - (char*)this;
            appFreeUni__FPv((void*)this->items.data);
        }
    }
    if ((char*)this + IFF_OFFSET(CItemManager, balls) && (char*)this + IFF_OFFSET(CItemManager, balls) &&
        (char*)this + IFF_OFFSET(CItemManager, balls) && (char*)this + IFF_OFFSET(CItemManager, balls)) {
        if (this->balls.data) {
            this->balls.size -= ((char*)this + this->balls.size) - (char*)this;
            appFreeUni__FPv((void*)this->balls.data);
        }
    }
    if ((char*)this + IFF_OFFSET(CItemManager, clubSets) && (char*)this + IFF_OFFSET(CItemManager, clubSets) &&
        (char*)this + IFF_OFFSET(CItemManager, clubSets) && (char*)this + IFF_OFFSET(CItemManager, clubSets)) {
        if (this->clubSets.data) {
            this->clubSets.size -= ((char*)this + this->clubSets.size) - (char*)this;
            appFreeUni__FPv((void*)this->clubSets.data);
        }
    }
    if ((char*)this + IFF_OFFSET(CItemManager, clubs) && (char*)this + IFF_OFFSET(CItemManager, clubs) &&
        (char*)this + IFF_OFFSET(CItemManager, clubs) && (char*)this + IFF_OFFSET(CItemManager, clubs)) {
        if (this->clubs.data) {
            this->clubs.size -= ((char*)this + this->clubs.size) - (char*)this;
            appFreeUni__FPv((void*)this->clubs.data);
        }
    }
    if ((char*)this + IFF_OFFSET(CItemManager, parts) && (char*)this + IFF_OFFSET(CItemManager, parts) &&
        (char*)this + IFF_OFFSET(CItemManager, parts) && (char*)this + IFF_OFFSET(CItemManager, parts)) {
        if (this->parts.data) {
            this->parts.size -= ((char*)this + this->parts.size) - (char*)this;
            appFreeUni__FPv((void*)this->parts.data);
        }
    }
    if ((char*)this + IFF_OFFSET(CItemManager, characters) && (char*)this + IFF_OFFSET(CItemManager, characters) &&
        (char*)this + IFF_OFFSET(CItemManager, characters) && (char*)this + IFF_OFFSET(CItemManager, characters)) {
        if (this->characters.data) {
            this->characters.size -= ((char*)this + this->characters.size) - (char*)this;
            appFreeUni__FPv((void*)this->characters.data);
        }
    }
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
CItemManager::CItemManager() {
    unsigned int* p = (unsigned int*)&characters;
    p[0] = 0;
    vtbl = __vt__12CItemManager;
    for (int i = 1; i < 39; ++i)
        p[i] = 0;
    Mutex_Init(inventoryMutexStorage);
    lockEnabled = 0;
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
            if (link == object->first)
                link = 0;
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
                if (link == object->first)
                    link = 0;
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
    if (!object->enabled)
        return 0;
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
    if (node)
        result = node->flags & (node->flags ^ node->value);
    else
        result = 0;
    if (requireActiveBinding && object->lookupMode == kRequireActiveBinding && !(node->linkFlags & 1))
        result = 0;
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

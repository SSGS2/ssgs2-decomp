// Translation unit 0x800C1FB4-0x800C2F30 (15 functions; boundaries inferred by decomp-toolkit from extab).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

struct BpControllerFlags {
    char reservedBeforeDrawingFlag[0x10];
    int drawingEnabled;
    int callbackPauseState;
};

struct Node {
    Node* left;
    Node* right;
    Node* parent;
    int key;
    int value;
};

// The iterator view keeps the tree color in the low bit of the parent word.
struct EntryNode {
    EntryNode* left;
    EntryNode* right;
    unsigned parent;
    int entryId; // Integer map key; every registered ID must be in [0, kEntryIdCount).
    void* object;
};

// Request (1) becomes paused (2) at DrawEntries even when drawing is disabled.
// State 1 still permits callbacks/resets; ResumeCallbacks returns either state to 0.
enum CallbackPauseState { kCallbacksRunning = 0, kPauseRequested = 1, kCallbacksPaused = 2 };
enum { kEntryIdCount = 32, kInvalidEntryId = -1 };

// Only the two words reset at the frame boundary are modeled here.
struct BpObjectFrameState {
    char reservedBeforeFrameState[0x3c];
    int frameState;    // Cleared on every reset; other meanings are unresolved.
    int callbackState; // Retained while callbacks are paused.
};

struct Lock {
    char pad[8];
};

// Owns tree nodes. &root is the end sentinel (links only, never a payload).
// firstEntry is the least ID, or that sentinel when empty. DeleteEntryById deletes
// the object as well as its node; MapOwner cleanup itself frees nodes only.
struct MapOwner {
    int count;
    Node* root;
    Node* firstEntry;
    // Leave header initialization to the enclosing controller constructor.
    MapOwner() {}
    ~MapOwner();
};

struct CBpCtrl {
    void* vt;
    MapOwner map;
    int drawingEnabled;
    int callbackPauseState;
    CBpCtrl();
};
typedef CBpCtrl BpController;

extern "C" {
extern char __vt__7CBpCtrl[];
extern float kFloat_0_8333333_BpCtrl;
extern float kFloat_1_BpPlatformInit;
void CBpCtrl_InitializeObjectEntries();
void B2dFile_InitializeSlotsAndIndex();
void Math_InitFixedPointTrigTables();
void B2dInpResetKeyList();
int EventTiming_IsDurationScalingEnabled();
void ValueAnimation_SetDurationScale(float);
void BpCtrl_InstallPlatformCallbacks();
void BpCtrl_SetGlobalHook(void*);
void B2d_LogFourArgumentDiagnostic();
void TableLock_ctor(Lock*, int, int);
int BpObject_PollLoad(void*);
void BpObject_TryInvokeCurrentCallback(void*, int);
int CBpCtrl_PollAllEntriesLoaded(BpController*);
int CBpCtrl_InvokeEntryCallbacks(BpController*, int);
struct TableLock : Lock {
    // Enter the shared table-lock scope using the original mode arguments.
    TableLock(int a, int b) { TableLock_ctor(this, a, b); }
    ~TableLock();  // the game's __dt__9TableLockFv
};
void CBpObject_ResetSpriteVertexCountsLocked(void*);

// Set the controller drawing flag without changing callback pause state.
int CBpCtrl_SetDrawingEnabled(BpControllerFlags* p, int v) {
    p->drawingEnabled = v;
    return 1;
}

// Resume callbacks by clearing the controller pause state.
void CBpCtrl_ResumeCallbacks(BpControllerFlags* p) {
    p->callbackPauseState = kCallbacksRunning;
}

// Request a callback pause, which becomes effective on the next draw pass.
void CBpCtrl_RequestCallbackPause(BpControllerFlags* p) {
    if (p->callbackPauseState == kCallbacksRunning) {
        p->callbackPauseState = kPauseRequested;
    }
}

// Empty variadic callback retained as a no-op hook.
void BpCtrl_NoopVariadicCallback(int a, ...) {
}

} // extern "C"

// Initialize the locked entry map and BP subsystems, then install callbacks and enable drawing.
CBpCtrl::CBpCtrl() {
    vt = __vt__7CBpCtrl;
    map.count = 0;
    map.root = 0;
    map.firstEntry = (Node*)&map.root;
    CBpCtrl_InitializeObjectEntries();
    B2dFile_InitializeSlotsAndIndex();
    Math_InitFixedPointTrigTables();
    B2dInpResetKeyList();
    ValueAnimation_SetDurationScale(EventTiming_IsDurationScalingEnabled() ? kFloat_0_8333333_BpCtrl : kFloat_1_BpPlatformInit);
    BpCtrl_InstallPlatformCallbacks();
    BpCtrl_SetGlobalHook((void*)B2d_LogFourArgumentDiagnostic);
    drawingEnabled = 1;
}

extern "C" {

// Return the borrowed object word under the table lock, or zero if the ID is absent.
int IdMap_Lookup(BpController* owner, int key) {
    TableLock lock(0, 1);
    Node* entry = owner->map.root;
    Node* best = (Node*)&owner->map.root;
    while (entry) {
        if (entry->key >= key) {
            best = entry;
            entry = entry->left;
        } else {
            entry = entry->right;
        }
    }
    if (best == (Node*)&owner->map.root || key < best->key) best = (Node*)&owner->map.root;
    int r = (best != (Node*)&owner->map.root) ? best->value : 0;
    return r;
}

// Advance an in-order iterator; the parent low bit stores the red/black color.
inline void AdvanceEntryNode(EntryNode*& node) {
    EntryNode* right = node->right;
    if (right) {
        while (right->left) right = right->left;
        node = right;
    } else {
        EntryNode* parent;
        while (parent = (EntryNode*)(node->parent & ~1u), node != parent->left) node = parent;
        node = parent;
    }
}

// Poll every registered object under the table lock and report whether all loads are complete.
int CBpCtrl_PollAllEntriesLoaded(BpController* owner) {
    TableLock lock(0, 1);
    EntryNode* entry = (EntryNode*)owner->map.firstEntry;
    EntryNode* endEntry = (EntryNode*)&owner->map.root;
    while (entry != endEntry) {
        if (!BpObject_PollLoad(entry->object)) {
            return 0;
        }
        AdvanceEntryNode(entry);
    }
    return 1;
}

// Invoke object callbacks under the table lock unless the requested pause has become effective.
int CBpCtrl_InvokeEntryCallbacks(BpController* owner, int a) {
    TableLock lock(0, 1);
    if (owner->callbackPauseState == kCallbacksPaused) {
        return 1;
    }
    EntryNode* entry = (EntryNode*)owner->map.firstEntry;
    EntryNode* endEntry = (EntryNode*)&owner->map.root;
    while (entry != endEntry) {
        BpObject_TryInvokeCurrentCallback(entry->object, a);
        AdvanceEntryNode(entry);
    }
    return 1;
}

#pragma optimize_for_size on
// Advance an in-order tree iterator using the parent pointer with its color bit masked off.
inline void nextNode(EntryNode*& entry) {
    EntryNode* r = entry->right;
    if (r) {
        // Original nextNode: descend to the leftmost successor.
        while (r->left) {
            r = r->left;
        }
        entry = r;
    } else {
        EntryNode* p;
        // Climb until the current node is a left child; its parent is the successor.
        while (p = (EntryNode*)(entry->parent & ~1u), entry != p->left) {
            entry = p;
        }
        entry = p;
    }
}

void BpObject_Draw(void*);
// Apply a pending callback pause and draw every entry when controller drawing is enabled.
int CBpCtrl_DrawEntries(BpController* owner) {
    TableLock lock(0, 1);
    if (owner->callbackPauseState == kPauseRequested) owner->callbackPauseState = kCallbacksPaused;
    if (!owner->drawingEnabled) {  return 1; }
    EntryNode* entry = (EntryNode*)owner->map.firstEntry;
    EntryNode* endEntry = (EntryNode*)&owner->map.root;
    while (entry != endEntry) { BpObject_Draw(entry->object); nextNode(entry); }
     return 1;
}

struct Iter { EntryNode* node; };
// Advance the iterator value while preserving its original by-value calling form.
inline Iter nextIter(Iter entry) { nextNode(entry.node); return entry; }
#pragma optimize_for_size reset
#pragma opt_lifetimes off
// Clear each object frame flag and, while callbacks run, reset its callback and sprite-vertex state.
int CBpCtrl_ResetEntryFrameState(BpController* owner) {
    TableLock lock(0, 1);
    EntryNode* endEntry = (EntryNode*)&owner->map.root;
    Iter entry = { (EntryNode*)owner->map.firstEntry };
    while (entry.node != endEntry) {
        BpObjectFrameState* object = (BpObjectFrameState*)entry.node->object;
        if (object) {
            object->frameState = 0;
            if (owner->callbackPauseState != kCallbacksPaused) {
                object->callbackState = 0;
                CBpObject_ResetSpriteVertexCountsLocked(object);
            }
        }
        entry = nextIter(entry);
    }

    return 1;
}
#pragma opt_lifetimes reset

struct RawObj;
#pragma optimize_for_size reset
void CBpObject_Ctor(RawObj*);
// Construct a BP object in the allocator-provided 0x74-byte storage.
struct RawObj { char data[0x74]; RawObj() { CBpObject_Ctor(this); } };
int CBpCtrl_FindFreeEntryId(BpController*, int);
int CBpObject_SetScriptResourcePath(RawObj*, void*, int);
Node* IntegerKeyTree_FindInsertionPosition(void*, int*, Node**, unsigned char*, unsigned char*) __attribute__((nothrow));
Node* BpEntryTree_InsertNode(void*, Node*, unsigned char, unsigned char, void*);
extern int s_bpEntryDefaultObject;
// Find an unused ID, construct and register its object, and assign the requested script resource.
// scriptPath is required; entryIdOut is optional. Zero searches IDs 0..31, nonzero 31..0.
// Returns the path setter's result; registration and the output ID precede that call,
// so a failed path setup does not roll back the newly owned object or its ID.
int CBpCtrl_CreateEntry(BpController* owner, void* scriptPath, int* entryIdOut, int searchFromLastId) {
    TableLock lock(0, 1);
    if (!scriptPath) {  return 0; }
    int key = CBpCtrl_FindFreeEntryId(owner, searchFromLastId);
    if (key == kInvalidEntryId) {  return 0; }
    RawObj* v = new RawObj();
    Node* found;
    unsigned char insertionFlag1, insertionFlag0; // Opaque outputs, passed back unchanged.
    Node* pos = IntegerKeyTree_FindInsertionPosition(&owner->map, &key, &found, &insertionFlag0, &insertionFlag1);
    int* pair;
    if (!found || (pair = &found->key, found->key < key)) {
        int item[2];
        item[0] = key;
        item[1] = s_bpEntryDefaultObject;
        pair = &BpEntryTree_InsertNode(&owner->map, pos, insertionFlag0, insertionFlag1, item)->key;
    }
    pair[1] = (int)v;
    if (entryIdOut) *entryIdOut = key;
    int result = CBpObject_SetScriptResourcePath(v, scriptPath, key);

    return result;
}
#pragma optimize_for_size on
#pragma opt_propagation on
#pragma opt_lifetimes off
// Build a 32-slot occupancy table and return the first or last free ID, or minus one.
// Requires every existing map key to be in [0,31]; no bounds check is performed here.
int CBpCtrl_FindFreeEntryId(BpController* owner, int reverse) {
    TableLock lock(0, 1);
    int used[kEntryIdCount];
    extern void* memset(void*, int, unsigned long) __attribute__((nothrow));
    memset(used, 0, sizeof(used));
    Iter entry = { (EntryNode*)owner->map.firstEntry };
    EntryNode* endEntry = (EntryNode*)&owner->map.root;
    while (entry.node != endEntry) {
        used[entry.node->entryId] = 1;
        entry = nextIter(entry);
    }
    if (reverse) {
        int* p = used + (kEntryIdCount - 1);
        for (int i = kEntryIdCount - 1; i >= 0; --p, --i) {
            if (!*p) {
                return i;
            }
        }
    } else {
        int* p = used;
        for (int i = 0; i < kEntryIdCount; ++p, ++i) {
            if (!*p) {
                return i;
            }
        }
    }
    return kInvalidEntryId;
}
#pragma opt_propagation reset
#pragma opt_lifetimes reset

struct Deletable { 
virtual ~Deletable();
};
void RBTree_RebalanceAfterErase(EntryNode*, Node*);
void appFreeUni__FPv(EntryNode*);
#pragma optimize_for_size reset
#pragma function_align 4
#pragma opt_lifetimes off
// Destroy the matching entry object, unlink and rebalance its tree node, and free the node.
int CBpCtrl_DeleteEntryById(BpController* owner, int key) {
    // Locate an exact key; an absent key is a successful no-op (return 1).
    TableLock lock(0, 1);
    EntryNode* entry = (EntryNode*)owner->map.root;
    Iter best = { (EntryNode*)&owner->map.root };
    while (entry) {
        if (entry->entryId >= key) {
            best.node = entry;
            entry = entry->left;
        } else {
            entry = entry->right;
        }
    }
    if (best.node == (EntryNode*)&owner->map.root || key < best.node->entryId) best.node = (EntryNode*)&owner->map.root;
    if (best.node != (EntryNode*)&owner->map.root) {
        // Destroy the payload before repairing the tree and freeing its node.
        if (best.node->object) {
            delete (Deletable*)best.node->object;
            best.node->object = 0;
        }
        Iter erase = best;
        if (best.node == (EntryNode*)owner->map.firstEntry) {
            nextNode(best.node);
            owner->map.firstEntry = (Node*)best.node;
        }
        RBTree_RebalanceAfterErase(erase.node, owner->map.root);
        appFreeUni__FPv(erase.node);
        owner->map.count -= 1;
    }
    return 1;
}
#pragma opt_lifetimes reset
void RBTree_FreeTrivialSubtree(void*, EntryNode*);
void __dl__FPv(void*);
#pragma use_lmw_stmw on
// Free a nonempty subtree after releasing its child subtrees.
inline void clear1(void* owner, EntryNode* entry) {
    if (entry) {
        if (entry->left) RBTree_FreeTrivialSubtree(owner, entry->left);
        if (entry->right) RBTree_FreeTrivialSubtree(owner, entry->right);
        appFreeUni__FPv(entry);
    }
}
// Free two unrolled levels of a trivial tree subtree.
inline void clear2(void* owner, EntryNode* entry) {
    if (entry) { clear1(owner, entry->left); clear1(owner, entry->right); appFreeUni__FPv(entry); }
}
// Free three unrolled levels of a trivial tree subtree.
inline void clear3(void* owner, EntryNode* entry) {
    if (entry) { clear2(owner, entry->left); EntryNode* r = entry->right; clear2(owner, r); appFreeUni__FPv(entry); }
}
// Free four unrolled levels of a trivial tree subtree.
inline void clear4(void* owner, EntryNode* entry) {
    if (entry) { clear3(owner, entry->left); clear3(owner, entry->right); appFreeUni__FPv(entry); }
}
void CBpObject_ClearResources(void*);
void BpSprite_EmptyModuleHook();
void B2dFile_EmptyModuleHook();
// Clear all entry resources and global BP state, free the map nodes, and optionally free the controller.
BpController* CBpCtrl_dtor(BpController* owner, int flag) {
    // Reusable traversal cursors in the compiler's four-level postorder expansion.
    // branchCursor0..2 alternate between depths; only free the cursor after its children.
    Iter rootEntry;
    EntryNode *branchCursor0, *branchCursor1, *branchCursor2;
    if (owner) {
        owner->vt = __vt__7CBpCtrl;
        // Phase 1: clear each object's resources while the entry table is locked.
        {
            TableLock lock(0, 1);
            Iter entry = { (EntryNode*)owner->map.firstEntry };
            EntryNode* endEntry = (EntryNode*)&owner->map.root;
            while (entry.node != endEntry) {
                CBpObject_ClearResources(entry.node->object);
                EntryNode* successor = entry.node->right;
                if (successor) {
                    while (successor->left) successor = successor->left;
                    entry.node = successor;
                    continue;
                } else {
                    EntryNode* parent;
                    while (parent = (EntryNode*)(entry.node->parent & ~1u), entry.node != parent->left)
                        entry.node = parent;
                    entry.node = parent;
                }
            }
        }
        // Phase 2: reset global BP services after releasing the table lock.
        B2dInpResetKeyList();
        BpSprite_EmptyModuleHook();
        B2dFile_EmptyModuleHook();
        // Phase 3: free tree nodes (payload resources were cleared above).
        // Keep the original nested cleanup guards and unrolling for CodeWarrior exception metadata.
        if (&owner->map) if (&owner->map) if (&owner->map)
        if (&owner->map) if (&owner->map) if (&owner->map)
        {
            rootEntry.node = (EntryNode*)owner->map.root;
            if (rootEntry.node) {
                branchCursor0 = rootEntry.node->left;
                if (branchCursor0) {
                    branchCursor1 = branchCursor0->left;
                    if (branchCursor1) {
                        branchCursor2 = branchCursor1->left;
                        if (branchCursor2) {
                            if (branchCursor2->left) RBTree_FreeTrivialSubtree(&owner->map, branchCursor2->left);
                            if (branchCursor2->right) RBTree_FreeTrivialSubtree(&owner->map, branchCursor2->right);
                            appFreeUni__FPv(branchCursor2);
                        }
                        branchCursor2 = branchCursor1->right;
                        if (branchCursor2) {
                            if (branchCursor2->left) RBTree_FreeTrivialSubtree(&owner->map, branchCursor2->left);
                            if (branchCursor2->right) RBTree_FreeTrivialSubtree(&owner->map, branchCursor2->right);
                            appFreeUni__FPv(branchCursor2);
                        }
                        appFreeUni__FPv(branchCursor1);
                    }
                    branchCursor2 = branchCursor0->right;
                    if (branchCursor2) {
                        branchCursor1 = branchCursor2->left;
                        if (branchCursor1) {
                            if (branchCursor1->left) RBTree_FreeTrivialSubtree(&owner->map, branchCursor1->left);
                            if (branchCursor1->right) RBTree_FreeTrivialSubtree(&owner->map, branchCursor1->right);
                            appFreeUni__FPv(branchCursor1);
                        }
                        branchCursor1 = branchCursor2->right;
                        if (branchCursor1) {
                            if (branchCursor1->left) RBTree_FreeTrivialSubtree(&owner->map, branchCursor1->left);
                            if (branchCursor1->right) RBTree_FreeTrivialSubtree(&owner->map, branchCursor1->right);
                            appFreeUni__FPv(branchCursor1);
                        }
                        appFreeUni__FPv(branchCursor2);
                    }
                    appFreeUni__FPv(branchCursor0);
                }
                branchCursor2 = rootEntry.node->right;
                if (branchCursor2) {
                    branchCursor1 = branchCursor2->left;
                    if (branchCursor1) {
                        branchCursor0 = branchCursor1->left;
                        if (branchCursor0) {
                            if (branchCursor0->left) RBTree_FreeTrivialSubtree(&owner->map, branchCursor0->left);
                            if (branchCursor0->right) RBTree_FreeTrivialSubtree(&owner->map, branchCursor0->right);
                            appFreeUni__FPv(branchCursor0);
                        }
                        branchCursor0 = branchCursor1->right;
                        if (branchCursor0) {
                            if (branchCursor0->left) RBTree_FreeTrivialSubtree(&owner->map, branchCursor0->left);
                            if (branchCursor0->right) RBTree_FreeTrivialSubtree(&owner->map, branchCursor0->right);
                            appFreeUni__FPv(branchCursor0);
                        }
                        appFreeUni__FPv(branchCursor1);
                    }
                    branchCursor1 = branchCursor2->right;
                    if (branchCursor1) {
                        branchCursor0 = branchCursor1->left;
                        if (branchCursor0) {
                            if (branchCursor0->left) RBTree_FreeTrivialSubtree(&owner->map, branchCursor0->left);
                            if (branchCursor0->right) RBTree_FreeTrivialSubtree(&owner->map, branchCursor0->right);
                            appFreeUni__FPv(branchCursor0);
                        }
                        branchCursor0 = branchCursor1->right;
                        if (branchCursor0) {
                            if (branchCursor0->left) RBTree_FreeTrivialSubtree(&owner->map, branchCursor0->left);
                            if (branchCursor0->right) RBTree_FreeTrivialSubtree(&owner->map, branchCursor0->right);
                            appFreeUni__FPv(branchCursor0);
                        }
                        appFreeUni__FPv(branchCursor1);
                    }
                    appFreeUni__FPv(branchCursor2);
                }
                appFreeUni__FPv(rootEntry.node);
            }
        }
        if (flag > 0) __dl__FPv(owner);
    }
    return owner;
}
// Free all trivial map nodes and optionally free the map container.
void* __dt__8MapOwnerFv(EntryNode** owner, int flag) {
    // Phase 1: postorder free, unrolled through four levels before falling back
    // to RBTree_FreeTrivialSubtree. Node payloads are not destroyed by this pass.
    if (owner) {
        if (owner) if (owner) if (owner) if (owner) if (owner)
        {
            Iter rootEntry;
            EntryNode *branchCursor0, *branchCursor1, *branchCursor2;
            rootEntry.node = (EntryNode*)owner[1];
            if (rootEntry.node) {
                branchCursor0 = rootEntry.node->left;
                if (branchCursor0) {
                    branchCursor1 = branchCursor0->left;
                    if (branchCursor1) {
                        branchCursor2 = branchCursor1->left;
                        if (branchCursor2) {
                            if (branchCursor2->left) RBTree_FreeTrivialSubtree(owner, branchCursor2->left);
                            if (branchCursor2->right) RBTree_FreeTrivialSubtree(owner, branchCursor2->right);
                            appFreeUni__FPv(branchCursor2);
                        }
                        branchCursor2 = branchCursor1->right;
                        if (branchCursor2) {
                            if (branchCursor2->left) RBTree_FreeTrivialSubtree(owner, branchCursor2->left);
                            if (branchCursor2->right) RBTree_FreeTrivialSubtree(owner, branchCursor2->right);
                            appFreeUni__FPv(branchCursor2);
                        }
                        appFreeUni__FPv(branchCursor1);
                    }
                    branchCursor2 = branchCursor0->right;
                    if (branchCursor2) {
                        branchCursor1 = branchCursor2->left;
                        if (branchCursor1) {
                            if (branchCursor1->left) RBTree_FreeTrivialSubtree(owner, branchCursor1->left);
                            if (branchCursor1->right) RBTree_FreeTrivialSubtree(owner, branchCursor1->right);
                            appFreeUni__FPv(branchCursor1);
                        }
                        branchCursor1 = branchCursor2->right;
                        if (branchCursor1) {
                            if (branchCursor1->left) RBTree_FreeTrivialSubtree(owner, branchCursor1->left);
                            if (branchCursor1->right) RBTree_FreeTrivialSubtree(owner, branchCursor1->right);
                            appFreeUni__FPv(branchCursor1);
                        }
                        appFreeUni__FPv(branchCursor2);
                    }
                    appFreeUni__FPv(branchCursor0);
                }
                branchCursor2 = rootEntry.node->right;
                if (branchCursor2) {
                    branchCursor1 = branchCursor2->left;
                    if (branchCursor1) {
                        branchCursor0 = branchCursor1->left;
                        if (branchCursor0) {
                            if (branchCursor0->left) RBTree_FreeTrivialSubtree(owner, branchCursor0->left);
                            if (branchCursor0->right) RBTree_FreeTrivialSubtree(owner, branchCursor0->right);
                            appFreeUni__FPv(branchCursor0);
                        }
                        branchCursor0 = branchCursor1->right;
                        if (branchCursor0) {
                            if (branchCursor0->left) RBTree_FreeTrivialSubtree(owner, branchCursor0->left);
                            if (branchCursor0->right) RBTree_FreeTrivialSubtree(owner, branchCursor0->right);
                            appFreeUni__FPv(branchCursor0);
                        }
                        appFreeUni__FPv(branchCursor1);
                    }
                    branchCursor1 = branchCursor2->right;
                    if (branchCursor1) {
                        branchCursor0 = branchCursor1->left;
                        if (branchCursor0) {
                            if (branchCursor0->left) RBTree_FreeTrivialSubtree(owner, branchCursor0->left);
                            if (branchCursor0->right) RBTree_FreeTrivialSubtree(owner, branchCursor0->right);
                            appFreeUni__FPv(branchCursor0);
                        }
                        branchCursor0 = branchCursor1->right;
                        if (branchCursor0) {
                            if (branchCursor0->left) RBTree_FreeTrivialSubtree(owner, branchCursor0->left);
                            if (branchCursor0->right) RBTree_FreeTrivialSubtree(owner, branchCursor0->right);
                            appFreeUni__FPv(branchCursor0);
                        }
                        appFreeUni__FPv(branchCursor1);
                    }
                    appFreeUni__FPv(branchCursor2);
                }
                appFreeUni__FPv(rootEntry.node);
            }
        }
        // Phase 2: the deleting form also releases the container allocation.
        if (flag > 0) __dl__FPv(owner);
    }
    return owner;
}
#pragma use_lmw_stmw reset
} // extern "C"

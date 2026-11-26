// Shop set-item and character product records.
//
// CShopProductSetItem decides whether a costume set is fully owned for a character.
// CShopProduct and CShopProductAvatar hold character product records. The file also frees
// the AI shot-position map tree.

#include <decomp/static_init.h>
#include <decomp/local_class.h>
#include "classes/MWStringStorage.h"
#include "classes/ShopProductRecord.h"
#define AsShopProductRecord(p) ((ShopProductRecord*)(p))
#include "classes/ShopTreeValue802307FC.h"
#include "classes/ShopTree802307FC.h"
#include "classes/ShopPointerArray802307FC.h"
#include "classes/ShopProductCharFields.h"
#include "classes/ShopTreeNode802307FC.h"
// Compare iterators without changing their pointer types.
template <class T> inline bool same_pos(const T& lhs, const T& rhs) {
    return lhs == rhs;
}

// This unit installs s_shopProductCharVtable explicitly. Declaring virtual methods here
// makes CodeWarrior add a second, undefined template table on destruction.
namespace IFF_STRUCT {
struct sChar;
}
template <class T> class CShopProduct;
template <> class CShopProduct<IFF_STRUCT::sChar> {
public:
    CShopProduct(void*);
    ~CShopProduct();
    void* vtable;
    char data[0x10];
};
#include "classes/CShopProductSetItem.h"
#include "classes/CShopProductInterface.h"

extern "C" {
extern "C" float kFloat_6_2831855;
extern "C" float s_shopCharacterProductRotationPeriod;
extern "C" float s_shopSetItemProductRotationPeriod;
extern "C" int s_shopProductCharVtable[16];
extern "C" void appFreeUni__FPv(void*);
extern "C" void __dl__FPv(void*);
extern "C" void Memory_Copy(void*, void*, int);
extern "C" void ShopSetItemPointerArray_dtor(void*, int);
extern "C" void* ShopSetItemPointerArray_Initialize(void*, void*);
extern "C" int SaveData_TestPartFlag(void*, void*, void*);
extern "C" void ComThinkShotPosMap_DestroyStringValue(void*, void*);
extern "C" char __vt__19CShopProductSetItem[];
extern "C" void* appMallocDummy__FUl(unsigned int);
extern "C" void* __nwa__FUl(unsigned int);
extern "C" void* String_ReserveCapacity(void*, void*);
extern "C" void String_ReplaceRange(void*, int, int, void*, void*, void*);
extern "C" void RBTree_RebalanceAfterInsert(void*, void*);
extern "C" void* __nw__FUl(unsigned int);
extern "C" void __dla__FPv(void*);
extern "C" void __throw(void*, void*, void*);
extern "C" unsigned int strlen(const char*);
extern "C" char* strcpy(char*, const char*);
extern "C" char spatialDiagnosticStrings[];
extern "C" char __vt__Q23std11logic_error[];
extern "C" char s_sharedCharDeleterVtable[];
extern "C" char __vt__Q23std12length_error[];
extern "C" void __dt__Q23std12length_errorFv(void*, int);

struct ErrBase {
    void* volatile vptr;
    void* text;
    void* owner;
    // This ABI error-base shell has no local member cleanup.
    ~ErrBase() {}
};

struct ErrTemp : ErrBase {};

struct ErrObj {
    void* vptr;
    int a;
    int b;
    void* text;
    // Initialize shared error-text ownership with both counters at one.
    ErrObj(char* t) {
        a = 1;
        b = 1;
        vptr = s_sharedCharDeleterVtable;
        text = t;
    }
};

struct VB {
    volatile char c;
};

struct BufGuard {
    char* p;
    char* flag;
    // Retain temporary diagnostic storage and its allocator-tag address.
    BufGuard(char* buffer, char* b) : p(buffer), flag(b) {}
    // Delete diagnostic storage if construction did not transfer ownership.
    ~BufGuard() {
        if (p)
            __dla__FPv(p);
    }
};

struct NodeGuard_u802307FC {
    void* tree;
    void* node;
    // Leave fields for insertion to initialize before any throwing operation.
    NodeGuard_u802307FC() {}
    // Free an unlinked node unless successful construction disarmed this owner.
    ~NodeGuard_u802307FC() {
        if (node)
            appFreeUni__FPv(node);
    }
};

// Payload copy view: keep integer loads/stores for XYZ so their float bits copy unchanged.
// This legacy view corresponds to ShotCandidateFields in com_think_shot_pos_candidates.cpp.
struct CopyData {
    int worldXBits, worldYBits, worldZBits; // world X/Y/Z bits, payload +0x00..+0x08
    float radius;                           // radius in world units, payload +0x0C
    int basePriority;                       // base priority, payload +0x10
    int player0PriorityAdjustment, player1PriorityAdjustment, player2PriorityAdjustment,
        player3PriorityAdjustment; // four additive player priority adjustments, +0x14..+0x20
    int contextId;                 // context ID, payload +0x24
};
typedef char CandidatePayload_size[sizeof(CopyData) == 0x28 ? 1 : -1];

enum {
    kSetItemProductCopyWordPairs = 0x23,
    kTreeValueOffset = 0xc,
    kTreePayloadOffset = 0x10,
    kShotCandidateTreeNodeBytes = 0x44,
    kCharacterTypeGroup = 1,
    kItemTypeGroupShift = 21,
    kItemTypeGroupMask = 0x1f,
    kItemCharacterShift = 13,
    kItemCharacterMask = 0xff,
    kCharacterTypeIdMask = 0x03ffffff,
    kStringHeapFlagBit = 31
};
// Despite the retained ShopTree* header names, this is the CComThinkShotPos map.
// Cross-unit evidence: com_think_shot_pos_candidates.cpp inserts into priorityMap; com_think_ctrl_and_shot_pos.cpp destroys the
// cached-position map with the same specialization. Adjacent shop functions do not
// make this tree a product/name index. Shared view names are provisional.
// Layout: node links +0/+4/+8; integer key +0x0c; 12-byte MW string +0x10;
// 40-byte candidate payload +0x1c (world XYZ, radius, priority, scores, context id).
// Node allocation totals 0x44 bytes. Inline string bytes overlay length/data words;
// bit31 marks an owned heap string. The tree owns nodes and heap strings.
struct ShotCandidateMapNodeView {
    void* left;
    void* right;
    void* parent;
    ComShotPositionTreeValue value;
};
struct ShotCandidateMapPayloadView {
    char reserved00[0x10];
    CopyData payload;
};
struct ShopOwnedBufferView {
    unsigned int reserved00;
    void* data;
};
struct ShopStringRecord {
    MWStringStorage text;
    char reserved0C[0x28];
};
struct ShopStringRecordArray {
    ShopStringRecord* data;
    unsigned int count;
    char reserved08[8];
    unsigned int startIndex;
};

typedef ComShotPositionTreeView ShotCandidateTree;
typedef ComShotPositionTreeNode ShotCandidateTreeNode;
typedef ComShotPositionTreeValue ShotCandidateTreeValue;
// View the AI map header without taking ownership or changing its storage.
static inline ShotCandidateTree* CandidateTree(void* p) {
    return (ShotCandidateTree*)p;
}
// View an existing node; callers must exclude the end sentinel before reading payloads.
static inline ShotCandidateTreeNode* CandidateNode(void* p) {
    return (ShotCandidateTreeNode*)p;
}
// View the integer-key/string prefix of a borrowed candidate value.
static inline const ShotCandidateTreeValue* CandidateValue(const void* p) {
    return (const ShotCandidateTreeValue*)p;
}

void ComThinkShotPosMap_DestroySubtree(void*, void*);
// Destroy a non-null candidate's descendants and string, then release its allocation.
static inline void DestroyCandidateNode(void* tree, void* node) {
    if (CandidateNode(node)->left)
        ComThinkShotPosMap_DestroySubtree(tree, CandidateNode(node)->left);
    if (CandidateNode(node)->right)
        ComThinkShotPosMap_DestroySubtree(tree, CandidateNode(node)->right);
    ComThinkShotPosMap_DestroyStringValue(tree, (char*)&((ShotCandidateMapNodeView*)node)->value);
    appFreeUni__FPv(node);
}

// Copy a keyed string and payload into a new red-black-tree node and rebalance it.
void* ComThinkShotPosMap_InsertNode(void* self, void* position, int insertBefore, int updateFirstNode,
                                    const void* value) {
    // Phase 1: count limit and expanded length_error construction. Unsigned wrap makes
    // (nodeCount+0x10000)==0xffff equivalent to nodeCount==0xffffffff, not a 65535-node limit.
    // err*/h*/flag* are runtime exception/allocator temporaries, not tree data.
    ErrObj* volatile errorControlAnchor;
    void* volatile allocationAnchor2;
    void* volatile allocationAnchor1;
    ErrTemp temp;
    VB flag18;
    VB zeroA;
    VB zeroB;
    VB flag;
    VB src8;
    unsigned int nodeCount = CandidateTree(self)->count;
    if ((nodeCount + 0x10000) == 0xffff) {
        char* text = spatialDiagnosticStrings + 0x9e;
        ErrTemp* tp = &temp;
        temp.vptr = __vt__Q23std11logic_error;
        zeroA.c = 0;
        zeroB.c = 0;
        unsigned int n = strlen(text) + 1;
        char* buffer = (char*)__nwa__FUl(n * 2 - n);
        temp.text = buffer;
        {
            BufGuard bg(buffer, (char*)&zeroB.c);
            ErrObj* err = (ErrObj*)__nw__FUl(0x10);
            if (err) {
                err->a = 1;
                err->b = 1;
                err->vptr = s_sharedCharDeleterVtable;
                err->text = buffer;
            }
            errorControlAnchor = err;
            temp.owner = err;
            bg.p = 0;
        }
        strcpy((char*)temp.text, text);
        tp->vptr = __vt__Q23std12length_error;
        __throw(spatialDiagnosticStrings + 0x63, tp, (void*)__dt__Q23std12length_errorFv);
    }
    flag18.c = 0;
    // Phase 2: raw allocation is guarded until payload construction completes.
    void* node = appMallocDummy__FUl(kShotCandidateTreeNodeBytes);
    NodeGuard_u802307FC guard;
    allocationAnchor1 = node;
    allocationAnchor2 = node;
    guard.tree = (char*)&CandidateTree(self)->root;
    guard.node = node;
    // Both borrowed inputs must describe complete records. Key is copied by value;
    // inline string bytes copy directly, heap text gets a separate owned allocation.
    // The 40-byte candidate body contains no separately destroyed resource here.
    // Copy key, small string (deep copy if heap-backed), and the 40-byte candidate body.
    char* nodeValueBytes = (char*)&((ShotCandidateMapNodeView*)node)->value;
    if (nodeValueBytes) {
        unsigned int flags = (*(const unsigned int*)&CandidateValue(value)->stringFlags);
        *(unsigned int*)nodeValueBytes = *(const unsigned int*)value;
        char* destinationString = nodeValueBytes + 4;
        try {
            if ((flags >> 31) == 0) {
                *(unsigned int*)destinationString = flags;
                AsMWStringStorage(destinationString)->length =
                    (*(const unsigned int*)&CandidateValue(value)->stringLength);
                (*(unsigned int*)&AsMWStringStorage(destinationString)->data) =
                    (*(const unsigned int*)&CandidateValue(value)->stringData);
            } else {
                *(unsigned int*)destinationString = 0;
                AsMWStringStorage(destinationString)->length = 0;
                (*(unsigned int*)&AsMWStringStorage(destinationString)->data) = 0;
                String_ReserveCapacity(destinationString, (*(void**)&CandidateValue(value)->stringLength));
                flag.c = src8.c;
                unsigned int sourceStringAddress = (*(const unsigned int*)&CandidateValue(value)->stringData);
                String_ReplaceRange(
                    destinationString, 0, 0, (void*)sourceStringAddress,
                    (void*)(sourceStringAddress + (*(const unsigned int*)&CandidateValue(value)->stringLength)),
                    (void*)&flag.c);
            }
            CopyData* destinationBody = (CopyData*)(destinationString + 0xc);
            CopyData* sourceBody = (CopyData*)((const char*)&((const ShotCandidateMapPayloadView*)value)->payload);
            unsigned int firstWord = sourceBody->worldXBits;
            unsigned int secondWord = sourceBody->worldYBits;
            destinationBody->worldXBits = firstWord;
            firstWord = sourceBody->worldZBits;
            destinationBody->worldYBits = secondWord;
            float floatComponent = sourceBody->radius;
            destinationBody->worldZBits = firstWord;
            secondWord = sourceBody->basePriority;
            destinationBody->radius = floatComponent;
            firstWord = sourceBody->player0PriorityAdjustment;
            destinationBody->basePriority = secondWord;
            secondWord = sourceBody->player1PriorityAdjustment;
            destinationBody->player0PriorityAdjustment = firstWord;
            firstWord = sourceBody->player2PriorityAdjustment;
            destinationBody->player1PriorityAdjustment = secondWord;
            secondWord = sourceBody->player3PriorityAdjustment;
            destinationBody->player2PriorityAdjustment = firstWord;
            firstWord = sourceBody->contextId;
            destinationBody->player3PriorityAdjustment = secondWord;
            destinationBody->contextId = firstWord;
        } catch (...) {
            throw;
        }
    }
    // Phase 3: transfer node ownership to the tree, link, increment count and rebalance.
    // position is a valid parent/sentinel; insertBefore chooses left, updateFirstNode updates cache.
    char* nodeBytes = (char*)guard.node;
    (*(unsigned int*)&CandidateNode(nodeBytes)->right) = 0;
    guard.node = 0;
    (*(unsigned int*)&CandidateNode(nodeBytes)->left) = 0;
    void** slot = &CandidateNode(nodeBytes)->parent;
    if (slot)
        *slot = position;
    if (insertBefore)
        *(void**)position = nodeBytes;
    else
        CandidateNode(position)->right = nodeBytes;
    unsigned int count = CandidateTree(self)->count;
    CandidateTree(self)->count = count + 1;
    RBTree_RebalanceAfterInsert(nodeBytes, CandidateTree(self)->root);
    if (updateFirstNode)
        CandidateTree(self)->firstNode = nodeBytes;
    return nodeBytes;
}

// Copy the 0x11c-byte product payload and initialize it through virtual slot seven.
CShopProductSetItem::CShopProductSetItem(void* recordPayload, void* context, void* initializationArgument) {
    void* payload = recordPayload;
    void* initializationData = initializationArgument;
    ShopSetItemPointerArray_Initialize(this, context);
    vtbl = (void*)__vt__19CShopProductSetItem;
    // Copy the 0x11C-byte payload two words at a time, plus one trailing word (loop unrolled by the compiler's idiom).
    int wordPairs = kSetItemProductCopyWordPairs;
    unsigned int* sourceWords = (unsigned int*)((char*)payload - 4);
    unsigned int* destinationWords = (unsigned int*)&itemRecordCapacity;
    do {
        unsigned int a = sourceWords[1];
        unsigned int b = sourceWords[2];
        sourceWords += 2;
        destinationWords[1] = a;
        destinationWords[2] = b;
        destinationWords += 2;
    } while (--wordPairs);
    destinationWords[1] = sourceWords[1];
    ((void (*)(void*, void*))(*(void***)this)[7])(this, initializationData);
}

#pragma use_lmw_stmw on
#pragma opt_propagation off
// Accept a set only when every part is owned and its character type matches.
int CShopProductSetItem::IsSetOwnedForCharacter(void* entry, void* partFlagContext) {
    void* itemCursor;
    unsigned int characterTypeId;
    unsigned int setTypeId;
    int ownsAllParts;
    int partIndex;
    void* flagContext;
    void* setRecord;
    void* product;
    product = this;
    setRecord = entry;
    flagContext = partFlagContext;
    ownsAllParts = 1;
    partIndex = 0;
    characterTypeId = this->characterTypeId;
    setTypeId = AsShopProductRecord(entry)->typeId;
    itemCursor = entry;
    while (partIndex < ((IFF_STRUCT::sSetItem*)setRecord)->itemCount) {
        if (!SaveData_TestPartFlag(AsShopProductCharFields(product)->context,
                                   (void*)((IFF_STRUCT::sSetItem*)itemCursor)->itemIds[0], flagContext)) {
            ownsAllParts = 0;
            break;
        }
        itemCursor = (void*)((unsigned int*)itemCursor + 1);
        ++partIndex;
    }
    int result = 0;
    if (ownsAllParts) {
        // The set belongs to this character when its type group is the character group and its character id matches.
        if ((((setTypeId >> kItemTypeGroupShift) & kItemTypeGroupMask) == kCharacterTypeGroup) &&
            (((setTypeId >> kItemCharacterShift) & kItemCharacterMask) == (characterTypeId & kCharacterTypeIdMask)))
            result = 1;
    }
    return result;
}
#pragma opt_propagation reset
#pragma use_lmw_stmw reset

// Free the node's heap string if it owns one (string flag bit 31); the node itself is freed by the caller.
// (The address tests are no-ops from the original expansion, kept so the compiled code matches.)
static inline void FreeCandidateHeapString(void* node) {
    if (((char*)&((ShotCandidateMapNodeView*)node)->value) != 0 && ((char*)&CandidateNode(node)->stringFlags) != 0 &&
        (*(unsigned int*)((char*)&CandidateNode(node)->stringFlags) >> 31))
        appFreeUni__FPv(CandidateNode(node)->stringHeap);
}

// Release a branch's string value and allocation after both child subtrees are gone.
static inline void DestroyCandidateValueAndNode(void* tree, void* node) {
    ComThinkShotPosMap_DestroyStringValue(tree, (char*)&((ShotCandidateMapNodeView*)node)->value);
    appFreeUni__FPv(node);
}

// Destroy this branch in postorder, retaining the caller's mutable child cursor.
static inline void DestroyCandidateBranch(void* tree, void* branch, void*& child) {
    child = CandidateNode(branch)->left;
    if (child)
        DestroyCandidateNode(tree, child);
    child = CandidateNode(branch)->right;
    if (child)
        DestroyCandidateNode(tree, child);
    DestroyCandidateValueAndNode(tree, branch);
}

// Free both child subtrees and each node-owned string; the expanded traversal preserves code generation.
void ComThinkShotPosMap_DestroySubtree(void* self, void* input) {
    // Non-null input is required. Walk left subtree, right subtree, then payload/node.
    // The first three levels are expanded; deeper children recurse with the same owner.
    // No device/product object is deleted and no header count/sentinel is reset here.
    void* tree = self;
    void* root = input;
    void* left = CandidateNode(input)->left;
    if (left) {
        void* leftLeft = CandidateNode(left)->left;
        if (leftLeft) {
            void* leftLeftLeft = CandidateNode(leftLeft)->left;
            if (leftLeftLeft) {
                DestroyCandidateNode(tree, leftLeftLeft);
            }
            void* sibling = CandidateNode(leftLeft)->right;
            if (sibling) {
                DestroyCandidateNode(tree, sibling);
            }
            DestroyCandidateValueAndNode(tree, leftLeft);
        }
        void* sibling = CandidateNode(left)->right;
        if (sibling) {
            void* child;
            DestroyCandidateBranch(tree, sibling, child);
        }
        FreeCandidateHeapString(left);
        appFreeUni__FPv(left);
    }
    // The left subtree is gone; destroy both branches of the right subtree next.
    void* rightSubtree = CandidateNode(root)->right;
    if (rightSubtree) {
        void* branch = CandidateNode(rightSubtree)->left;
        if (branch) {
            void* grandchild;
            DestroyCandidateBranch(tree, branch, grandchild);
        }
        branch = CandidateNode(rightSubtree)->right;
        if (branch) {
            void* grandchild;
            DestroyCandidateBranch(tree, branch, grandchild);
        }
        FreeCandidateHeapString(rightSubtree);
        appFreeUni__FPv(rightSubtree);
    }
    FreeCandidateHeapString(root);
    appFreeUni__FPv(root);
}

} // extern "C"

#pragma opt_propagation off
#pragma opt_common_subs off
// Release the character-product data vector and destroy its product-interface base.
CShopProduct<IFF_STRUCT::sChar>::~CShopProduct() {
    void* self = this;
    struct ProductVirtuals {
        virtual void m0() DECOMP_LOCAL_VIRTUAL;
        virtual void m1() DECOMP_LOCAL_VIRTUAL;
    };
    *(void**)self = s_shopProductCharVtable;
    ((ProductVirtuals*)self)->m1();
    int field = (int)&AsShopProductCharFields(self)->data;
    if (field != 0 && field != 0 && field != 0 && field != 0 && field != 0 &&
        AsShopProductCharFields(self)->data != 0) {
        int* p = &AsShopProductCharFields(self)->size;
        int v = *p;
        *p -= v;
        appFreeUni__FPv((*(void**)&AsShopProductCharFields(self)->data));
    }
    ((CShopProductInterface*)self)->Destroy(0);
}
#pragma opt_propagation reset
#pragma opt_common_subs reset

// CodeWarrior's deleting-destructor flag convention: <= 0 destroys in place, a positive flag frees the object.

extern "C" {

// Return the product pointer-array count through the existing word-sized ABI.
void* CShopProductAvatar_GetCount(void* p) {
    return (void*)AsShopPointerArray802307FC(p)->count;
}

// Initialize the character-product rotation-period value.
void ShopCharacterProduct_InitRotationPeriod() {
    s_shopCharacterProductRotationPeriod = kFloat_6_2831855;
}

// Initialize the set-item product rotation-period value.
void ShopSetItemProduct_InitRotationPeriod() {
    s_shopSetItemProductRotationPeriod = kFloat_6_2831855;
}

// Guard the local views used by this unit against accidental layout changes.
#define POLISH_OFFSET(T, F, O) typedef char polish_##T##_##F[((int)&((T*)0)->F == (O)) ? 1 : -1]
POLISH_OFFSET(ShotCandidateMapNodeView, value, 0xc);
POLISH_OFFSET(ShotCandidateMapPayloadView, payload, 0x10);
POLISH_OFFSET(ShopStringRecordArray, startIndex, 0x10);
#undef POLISH_OFFSET
typedef char polish_size_ShopStringRecord[(sizeof(ShopStringRecord) == 0x34) ? 1 : -1];

} // extern "C"

// Initialize an empty character-product vector with its owning context.
CShopProduct<IFF_STRUCT::sChar>::CShopProduct(void* context) {
    void* self = this;
    AsShopProductCharFields(self)->data = 0;
    *(void**)self = (void*)&s_shopProductCharVtable;
    AsShopProductCharFields(self)->size = 0;
    AsShopProductCharFields(self)->capacity = 0;
    AsShopProductCharFields(self)->context = context;
}

extern "C" {

// Free a tree value string only when its small-string header marks heap ownership.
void ComThinkShotPosMap_DestroyStringValue(void* tree, void* p) {
    if (p == 0)
        return;
    int t = (int)p + 4;
    if (t == 0)
        return;
    if (t == 0)
        return;
    unsigned int flags = CandidateValue(p)->stringFlags;
    if (!(flags >> 31))
        return;
    appFreeUni__FPv((*(void* const*)&CandidateValue(p)->stringData));
}

// Destroy the set-item product and optionally free its allocation.
CShopProductSetItem::~CShopProductSetItem() {
    ShopSetItemPointerArray_dtor(this, 0);
}

// Free the owned buffer and optionally delete its owner.
void* ShopOwnedBufferView_dtor(void* self, int deleting) {
    void* savedSelf = self;
    int deleteObject = deleting;
    if (self != 0) {
        void* p = ((ShopOwnedBufferView*)self)->data;
        if (p != 0) {
            appFreeUni__FPv(p);
        }
        if (deleteObject > 0) {
            __dl__FPv(savedSelf);
        }
    }
    return savedSelf;
}

// Destroy heap strings in the active record range, release storage, and optionally delete the array.
void* ShopStringRecordArray_dtor(void* self, int deleting) {
    void* savedSelf = self;
    int deleteObject = deleting;
    if (self != 0) {
        unsigned int startIndex = ((ShopStringRecordArray*)self)->startIndex;
        unsigned int recordCount = ((ShopStringRecordArray*)self)->count;
        char* data = *(char**)self;
        char* activeBegin = data + startIndex * sizeof(ShopStringRecord);
        char* recordCursor = activeBegin + recordCount * sizeof(ShopStringRecord);
        // Phase 1: destroy active names in reverse order; records are 0x34 bytes.
        while (recordCursor > activeBegin) {
            recordCursor -= sizeof(ShopStringRecord);
            if (recordCursor != 0 && recordCursor != 0 &&
                (((ShopStringRecord*)recordCursor)->text.flags >> kStringHeapFlagBit))
                appFreeUni__FPv(((ShopStringRecord*)recordCursor)->text.data);
        }
        ((ShopStringRecordArray*)self)->count = 0;
        // Phase 2: preserve the base cleanup path, then release the record allocation.
        if (*(void**)self != 0) {
            ((ShopStringRecordArray*)self)->count = 0;
            char* baseCleanupCursor = *(char**)self;
            while ((*(int*)&((ShopStringRecordArray*)self)->count) != 0) {
                baseCleanupCursor -= sizeof(ShopStringRecord);
                if (baseCleanupCursor != 0 && baseCleanupCursor != 0 &&
                    (((ShopStringRecord*)baseCleanupCursor)->text.flags >> kStringHeapFlagBit))
                    appFreeUni__FPv(((ShopStringRecord*)baseCleanupCursor)->text.data);
                --(*(int*)&((ShopStringRecordArray*)self)->count);
            }
            appFreeUni__FPv(*(void**)self);
        }
        if (deleteObject > 0)
            __dl__FPv(self);
    }
    return savedSelf;
}

// Copy one non-null avatar IFF record by index, reporting missing records as failure.
int CShopProductAvatar_CopyRecord(void* self, void* outRecord, unsigned int recordIndex) {
    if (outRecord == 0)
        return 0;
    unsigned int recordCount = AsShopPointerArray802307FC(self)->count;
    if (recordCount <= recordIndex)
        return 0;
    void* recordPointers = (*(void**)&AsShopPointerArray802307FC(self)->data);
    void* record = ((void**)recordPointers)[recordIndex];
    if (record == 0)
        return 0;
    Memory_Copy(outRecord, record, sizeof(IFF_STRUCT::sChar));
    return 1;
}

// Delete every owned object pointer and reset the array count.
int PtrArray_DeleteAllAndClear(void* self) {
    void* array = self;
    void** cursor = AsShopPointerArray802307FC(array)->data;
    int nullValue = 0;
    unsigned int count;
    while (!same_pos((count = AsShopPointerArray802307FC(array)->count,
                      (void**)((char*)(*(void**)&AsShopPointerArray802307FC(array)->data) + count * 4)),
                     cursor)) {
        void* object = *cursor;
        if (object != 0) {
            __dl__FPv(object);
            *cursor = (void*)nullValue;
        }
        cursor = cursor + 1;
    }
    AsShopPointerArray802307FC(array)->count -= count;
    return 1;
}

// Copy two adjacent header bytes in the original load/load/store/store order.
static inline void CopyIffHeaderBytePair(unsigned char* destination, const char* source, unsigned int offset) {
    unsigned char first = source[offset];
    unsigned char second = source[offset + 1];
    destination[offset] = first;
    destination[offset + 1] = second;
}

#pragma opt_propagation off
// Fetch the current character record and copy its shared IFF item header.
int CShopProductAvatar_CopyCommonHeader(void* self, void* out) {
    // Borrow caller output: at least sizeof(IffItemCommon)=0x5c bytes. Return 0 for
    // null output or failed virtual record fetch; return 1 after copying the common header.
    // The 0x2c0 scratch buffer accommodates a full product record; only +0x00..+0x5b
    // is exported. It is NOT sizeof(sChar) and no character-only fields are copied.
    if (out == 0)
        return 0;
    char recordBytes[0x2c0];
    // Borrow the product's record-fetch interface; slot two fills caller-owned scratch.
    // The non-null result signals success; no ownership transfers through it.
    struct ProductRecordFetch {
        virtual void* slot0() DECOMP_LOCAL_VIRTUAL;
        virtual void* slot1() DECOMP_LOCAL_VIRTUAL;
        virtual void* fetchRecord(void*) DECOMP_LOCAL_VIRTUAL;
    };
    if (((ProductRecordFetch*)self)->fetchRecord(recordBytes) == 0)
        return 0;
    unsigned char* outputBytes = (unsigned char*)out;
    // Header phase 1: 40-byte name/message id followed by 40-byte icon resource name.
    CopyIffHeaderBytePair(outputBytes, recordBytes, 0);
    CopyIffHeaderBytePair(outputBytes, recordBytes, 2);
    CopyIffHeaderBytePair(outputBytes, recordBytes, 4);
    CopyIffHeaderBytePair(outputBytes, recordBytes, 6);
    CopyIffHeaderBytePair(outputBytes, recordBytes, 8);
    CopyIffHeaderBytePair(outputBytes, recordBytes, 10);
    CopyIffHeaderBytePair(outputBytes, recordBytes, 12);
    CopyIffHeaderBytePair(outputBytes, recordBytes, 14);
    CopyIffHeaderBytePair(outputBytes, recordBytes, 16);
    CopyIffHeaderBytePair(outputBytes, recordBytes, 18);
    CopyIffHeaderBytePair(outputBytes, recordBytes, 20);
    CopyIffHeaderBytePair(outputBytes, recordBytes, 22);
    CopyIffHeaderBytePair(outputBytes, recordBytes, 24);
    CopyIffHeaderBytePair(outputBytes, recordBytes, 26);
    CopyIffHeaderBytePair(outputBytes, recordBytes, 28);
    CopyIffHeaderBytePair(outputBytes, recordBytes, 30);
    CopyIffHeaderBytePair(outputBytes, recordBytes, 32);
    CopyIffHeaderBytePair(outputBytes, recordBytes, 34);
    CopyIffHeaderBytePair(outputBytes, recordBytes, 36);
    CopyIffHeaderBytePair(outputBytes, recordBytes, 38);
    CopyIffHeaderBytePair(outputBytes, recordBytes, 40);
    CopyIffHeaderBytePair(outputBytes, recordBytes, 42);
    CopyIffHeaderBytePair(outputBytes, recordBytes, 44);
    CopyIffHeaderBytePair(outputBytes, recordBytes, 46);
    CopyIffHeaderBytePair(outputBytes, recordBytes, 48);
    CopyIffHeaderBytePair(outputBytes, recordBytes, 50);
    CopyIffHeaderBytePair(outputBytes, recordBytes, 52);
    CopyIffHeaderBytePair(outputBytes, recordBytes, 54);
    CopyIffHeaderBytePair(outputBytes, recordBytes, 56);
    CopyIffHeaderBytePair(outputBytes, recordBytes, 58);
    CopyIffHeaderBytePair(outputBytes, recordBytes, 60);
    CopyIffHeaderBytePair(outputBytes, recordBytes, 62);
    CopyIffHeaderBytePair(outputBytes, recordBytes, 64);
    CopyIffHeaderBytePair(outputBytes, recordBytes, 66);
    CopyIffHeaderBytePair(outputBytes, recordBytes, 68);
    CopyIffHeaderBytePair(outputBytes, recordBytes, 70);
    CopyIffHeaderBytePair(outputBytes, recordBytes, 72);
    CopyIffHeaderBytePair(outputBytes, recordBytes, 74);
    CopyIffHeaderBytePair(outputBytes, recordBytes, 76);
    CopyIffHeaderBytePair(outputBytes, recordBytes, 78);
    // Header phase 2: type id +0x50, price +0x54, level/shop flags +0x58/+0x59,
    // then the shared unknown halfword +0x5a. Keep byte/word copy order intact.
    AsShopProductRecord(out)->typeId = AsShopProductRecord(recordBytes)->typeId;
    *(unsigned int*)&((IffItemCommon*)out)->price = (unsigned int)((IffItemCommon*)recordBytes)->price;
    outputBytes[0x58] = recordBytes[0x58];
    outputBytes[0x59] = recordBytes[0x59];
    // Preserve the common header's trailing halfword; its meaning is not established by this copy.
    ((IffItemCommon*)out)->commonFlagWord = ((IffItemCommon*)recordBytes)->commonFlagWord;
    return 1;
}
#pragma opt_propagation reset
} // extern "C"

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU80462244", _ctors_u802307FC_0, ShopCharacterProduct_InitRotationPeriod);
STATIC_INIT_ENTRY(".CRT$XCU80462244", _ctors_u802307FC_1, ShopSetItemProduct_InitRotationPeriod);
#pragma pop

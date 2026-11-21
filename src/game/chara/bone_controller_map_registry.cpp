#include <decomp/static_init.h>
#include "MSL/error_message.h"
// Translation unit 0x8025F0BC-0x8025F9F0 (8 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

#include "sdk_protos.h"

enum { kFullCountTestBias = 0x10000u, kBiasedFullCount = 0xFFFFu, kLengthErrorTypeOffset = 0x1A,
       kControllerNodeBytes = 0x14, kControllerPayloadBytes = 0x40 };

extern "C" {

extern float kFloat_6_2831855;
extern float s_modelRegistryFullTurn;
void __dl__FPv(void*);
void* __nw__FUl(unsigned long);
extern char __vt__15CRotateRegistry[];
void RotateRegistry_Init(void*, void*);
void RBTree_RebalanceAfterErase(void*, void*);
void appFreeUni__FPv(void*);
void RBTree_RebalanceAfterInsert(void*, void*, unsigned int);
void* appMallocDummy__FUl(unsigned int);
void* __nwa__FUl(unsigned int);
void* BoneControllerVectorTree_FindInsertionPosition(void*, void*, void**, unsigned char*, unsigned char*);
extern unsigned int s_boneControllerNullMappedValue;
extern char __vt__Q23std11logic_error[];
extern char s_sharedCharDeleterVtable[];
extern char __vt__Q23std12length_error[];
extern char modelRegistryTreeErrorStrings[];
void __dt__Q23std12length_errorFv(void*);
struct LayerControllerNode {
    LayerControllerNode* left; // 0
    LayerControllerNode* right; // 4
    unsigned int parent_color; // 8
    unsigned int key; // c
    void* data; // 10
};
struct LayerControllerTree {
    char padding00[0x08];
    int count; // 8
    LayerControllerNode* root; // c
    LayerControllerNode* header; // 10
};
// Shared tree convention in this file: header/end is an embedded pseudo-node
// at &root, not an allocated payload node. header/last cache the first iterator
// (see erase's successor update); never dereference their payload at end().
// Parent links in iterator views pack the red/black bit in bit 0.
// Nodes use appMallocDummy/appFreeUni; mapped objects use scalar new/delete.

struct BoneControllerVectorNode {
    BoneControllerVectorNode* left;
    BoneControllerVectorNode* right;
    unsigned int parent_color;
    unsigned int key;
    unsigned int words[12]; // words[0] is the mapped 64-byte payload pointer.
};
// This oversized historical view is used only for links/key/words[0]. Actual
// inserted nodes occupy 0x14 bytes, NOT sizeof(BoneControllerVectorNode).

struct BoneControllerVectorTree {
    char padding00[4];
    unsigned int count; // 4
    char padding08[4];
    BoneControllerVectorNode* root; // c
    BoneControllerVectorNode* header; // 10
};

struct BoneControllerPayload {
    unsigned int words[16];
};

struct BoneControllerNode {
    BoneControllerNode* left;
    BoneControllerNode* right;
    BoneControllerNode* parent;
    unsigned int key;
    unsigned int data;
};

struct Fn8025F4E4Exc {
    void* vtable;
    char* msg;
    void* rep;
};


// Shared RTTI: std::tr1::detail::shared_ptr_deleter<char, std::detail::default_delete<char[]> >.
// Use the common 16-byte ABI view in MSL/error_message.h; keep this local alias for callers.
typedef ::std::detail::__message_control Fn8025F4E4Rep;

void __dla__FPv(void*);
struct Fn8025F4E4Guard {
    char* p;
    char* allocator;
    // Free the error text if construction of its ownership block fails.
    ~Fn8025F4E4Guard() { if (p) __dla__FPv(p); }
};

// Construction/unwind overlay. Numeric suffixes are historical slot labels;
// infer layout from declarations/compiler output, not from those suffixes.
// ABI-preserving construction/unwind slots; padding is compiler frame storage, not model data.
// Slot roles: b08/b0c are error-string allocator tags; b10 is the node allocator tag.
// node14/node18 retain allocation addresses; node2c is the armed node-free guard.
// self28 borrows &tree->root (the sentinel); self54/self6c anchor the unwind frame.
// These stores describe construction state, not additional owning pointers.
struct Fn8025F4E4Frame {
    unsigned char b08;
    char padding01[3];
    unsigned char b0c;
    char padding05[3];
    unsigned char b10;
    char padding09[3];
    BoneControllerNode* node14;
    BoneControllerNode* node18;
    char padding14[4];
    char* self28;
    BoneControllerNode* node2c;
    Fn8025F4E4Exc exc;
    char padding34[0x18];
    void* self54;
    char padding50[0x14];
    void* self6c;
};

// Source view of a node's key/mapped-object pair; pointer ownership belongs to the map.
struct BoneControllerMappedEntry {
    unsigned int key;
    BoneControllerPayload* ownedPayload;
};

struct BoneControllerPair {
    unsigned int key;
    unsigned int data;
};
struct BoneControllerTree {
    unsigned int count;
    BoneControllerNode* root;
    BoneControllerNode* firstNode; // Cached first iterator.
};
BoneControllerVectorNode* BoneControllerMap_InsertNode(BoneControllerTree*, BoneControllerNode*, int, int, BoneControllerPair*);
int BoneControllerMap_SetIndexedPayload(BoneControllerVectorTree*, void*, const BoneControllerPayload*);

#pragma use_lmw_stmw on
// Copy a key/value pair into a new tree node, link and rebalance it, or throw if the tree is full.
BoneControllerVectorNode* BoneControllerMap_InsertNode(BoneControllerTree* tree, BoneControllerNode* parent, int insertLeft, int becomesFirst, BoneControllerPair* entry) {
    // ABI phase: construct the length_error and its owned message only on the
    // full-tree sentinel. The bias comparison recognizes UINT_MAX, not 65535 nodes.
    // The construction frame's numbered slots and alloca(0) reproduce unwind bookkeeping; they
    // are not controller fields. Parent and entry are borrowed; node copies entry.
    Fn8025F4E4Frame construction;
    if ((unsigned int)(tree->count + kFullCountTestBias) == kBiasedFullCount) {
        Fn8025F4E4Exc* exc = &construction.exc;
        exc->vtable = __vt__Q23std11logic_error;
        construction.b0c = 0;
        construction.b08 = 0;
        const char* messageText = modelRegistryTreeErrorStrings;
        unsigned int messageBytes = strlen(messageText) + 1;
        char* ownedMessageCopy = (char*)__nwa__FUl(messageBytes * 2 - messageBytes);
        exc->msg = ownedMessageCopy;
        {
            Fn8025F4E4Guard guard;
            guard.p = ownedMessageCopy;
            guard.allocator = (char*)&construction.b08;
            exc->rep = new Fn8025F4E4Rep(ownedMessageCopy);
            guard.p = 0;
        }
        strcpy(exc->msg, messageText);
        exc->vtable = __vt__Q23std12length_error;
        __throw(modelRegistryTreeErrorStrings + kLengthErrorTypeOffset, exc, (void*)__dt__Q23std12length_errorFv);
    }
    // Allocate the node under the original construction guard, then link it.
    void* unwindFrameBase = (char*)__alloca(0) - 8;
    construction.b10 = 0;
    BoneControllerNode* node = (BoneControllerNode*)appMallocDummy__FUl(kControllerNodeBytes);
    construction.node14 = node;
    construction.node18 = node;
    construction.self28 = (char*)&tree->root;
    construction.node2c = node;
    BoneControllerPair* nodeValue = (BoneControllerPair*)&node->key;
    if (nodeValue != 0) {
        unsigned int data = entry->data;
        unsigned int key = entry->key;
        nodeValue->key = key;
        construction.self54 = unwindFrameBase;
        nodeValue->data = data;
    }
    node->right = 0;
    construction.node2c = 0;
    node->left = 0;
    unsigned int* parentLinkWord = (unsigned int*)&node->parent;
    if (parentLinkWord != 0) {
        construction.self6c = unwindFrameBase;
        *parentLinkWord = (unsigned int)parent;
    }
    if (insertLeft != 0) {
        parent->left = node;
    } else {
        parent->right = node;
    }
    // Publish count, restore RB invariants and update the cached first iterator.
    unsigned int previousCount = tree->count;
    tree->count = previousCount + 1;
    RBTree_RebalanceAfterInsert(node, tree->root, previousCount);
    if (becomesFirst != 0) {
        tree->firstNode = node;
    }
    if (construction.node2c != 0) {
        appFreeUni__FPv(construction.node2c);
    }
    return (BoneControllerVectorNode*)node;
}
#pragma use_lmw_stmw reset

#pragma opt_common_subs off
#pragma opt_propagation off
// Copy the 64-byte controller payload for an in-range index, creating its map entry when needed.
int BoneControllerMap_SetIndexedPayload(BoneControllerVectorTree* self, void* key, const BoneControllerPayload* value) {
    // key points to an unsigned index word (it is NOT the integer cast to void*).
    // self->count bounds valid indices; the tree subobject begins at padding08.
    // value is borrowed and copied by value. Return 0 for invalid index/allocation
    // failure; 1 means the map owns a 64-byte copy. Existing payloads are reused.
    const unsigned int* indexKey = (const unsigned int*)key;
    BoneControllerPayload* payloadCopy;
    BoneControllerVectorNode* insertionCandidate;
    unsigned char becomesFirst, insertLeft;
    unsigned int count = self->count;
    if (count <= *indexKey) return 0;
    // Lower_bound: retain the first node >= key; reject a non-equal candidate.
    BoneControllerVectorNode* node = self->root;
    BoneControllerVectorNode* parent = (BoneControllerVectorNode*)&self->root;
    while (node) {
        if (node->key >= *indexKey) { parent = node; node = node->left; }
        else node = node->right;
    }
    if (parent == (BoneControllerVectorNode*)&self->root || *indexKey < parent->key) parent = (BoneControllerVectorNode*)&self->root;
    if ((BoneControllerVectorNode*)&self->root == parent) payloadCopy = (BoneControllerPayload*)__nw__FUl(kControllerPayloadBytes);
    else payloadCopy = ((const BoneControllerMappedEntry*)&parent->key)->ownedPayload;
    if (!payloadCopy) return 0;
    *payloadCopy = *value;
    // Find/create the key slot after copying the payload. The pair begins at
    // +0xC; mappedEntry->ownedPayload is the pointer at +0x10.
    void* insertionParent = BoneControllerVectorTree_FindInsertionPosition(&self->padding08, key, (void**)&insertionCandidate, &insertLeft, &becomesFirst);
    BoneControllerVectorNode* candidate = insertionCandidate;
    BoneControllerMappedEntry* mappedEntry;
    // An existing key skips insertion and keeps its slot.
    do {
        if (candidate != 0) {
            mappedEntry = (BoneControllerMappedEntry*)&candidate->key;
            unsigned int wanted = *indexKey;
            unsigned int existing = candidate->key;
            if (existing >= wanted) break;
        }
        {
            unsigned int keyAndDefaultValue[2];
            keyAndDefaultValue[0] = *indexKey;
            keyAndDefaultValue[1] = s_boneControllerNullMappedValue;
            mappedEntry = (BoneControllerMappedEntry*)&BoneControllerMap_InsertNode((BoneControllerTree*)(&self->padding08), (BoneControllerNode*)insertionParent, insertLeft, becomesFirst, (BoneControllerPair*)keyAndDefaultValue)->key;
        }
    } while (0);
    mappedEntry->ownedPayload = payloadCopy;
    return 1;
}
#pragma opt_common_subs reset
#pragma opt_propagation reset

// Initialize this registry unit's full-turn constant.
void ModelRegistry_InitFullTurn() {
    s_modelRegistryFullTurn = kFloat_6_2831855;
}

struct Fn8025F428Object {
    virtual void padding00();
    virtual void padding04();
    virtual void call10();
};

// Dispatch the object's third virtual operation (vtable slot +0x10).
void Registry_DispatchVirtualOperation10(Fn8025F428Object* self) {
    self->call10();
}

struct RegistryTreeNode {
    RegistryTreeNode* left;
    RegistryTreeNode* right;
};

struct RegistrySubtree {
    RegistryTreeNode* root;
    RegistryTreeNode* right;
};

void RBTree_DestroySubtree(RegistrySubtree*, RegistrySubtree*);
// Recurse below a non-null node, then free that node; mapped objects are untouched.
static inline void DestroyRegistryNodeWithDescendants(RegistrySubtree* self, RegistryTreeNode* node) {
    if (node->left) RBTree_DestroySubtree(self, (RegistrySubtree*)node->left);
    if (node->right) RBTree_DestroySubtree(self, (RegistrySubtree*)node->right);
    appFreeUni__FPv(node);
}

#pragma use_lmw_stmw on
// Recursively free both registry subtrees and then their parent; payloads are managed by the caller.
void RBTree_DestroySubtree(RegistrySubtree* self, RegistrySubtree* tree) {
    RegistryTreeNode* third;
    RegistryTreeNode* second;
    RegistryTreeNode* first;
    // Traversal map (L/R are child links, root is the supplied allocation):
    // L: LL{LLL,LLR}, LR{LRL,LRR}, free L; R: RL{RLL,RLR}, RR{RRL,RRR}, free R;
    // finally free root. At each brace, recurse below both children, then free parent.
    // first/second/third are reusable traversal registers, not stable depth/side names.
    // Each allocation is freed once; this routine never destroys mapped objects.
    // Postorder phase 1: free the entire left subtree, with three levels
    // expanded before recursion. Nodes contain borrowed/unmanaged payload here.
    first = (RegistryTreeNode*)tree->root;
    if (first) {
        second = first->left;
        if (second) {
            third = second->left;
            if (third) {
                DestroyRegistryNodeWithDescendants(self, third);
            }
            third = second->right;
            if (third) {
                DestroyRegistryNodeWithDescendants(self, third);
            }
            appFreeUni__FPv(second);
        }
        // L.left is gone; process L.right before freeing L.
        third = first->right;
        if (third) {
            second = third->left;
            if (second) {
                DestroyRegistryNodeWithDescendants(self, second);
            }
            // This branch's left child is gone; visit its right child before freeing it.
        second = third->right;
            if (second) {
                DestroyRegistryNodeWithDescendants(self, second);
            }
            appFreeUni__FPv(third);
        }
        appFreeUni__FPv(first);
    }
    // Postorder phase 2: free the right subtree, then the supplied root node.
    // self is only the recursion context; this never frees a tree header.
    third = (RegistryTreeNode*)tree->right;
    if (third) {
        second = third->left;
        if (second) {
            first = second->left;
            if (first) {
                DestroyRegistryNodeWithDescendants(self, first);
            }
            first = second->right;
            if (first) {
                DestroyRegistryNodeWithDescendants(self, first);
            }
            appFreeUni__FPv(second);
        }
        // This branch's left child is gone; visit its right child before freeing it.
        second = third->right;
        if (second) {
            first = second->left;
            if (first) {
                DestroyRegistryNodeWithDescendants(self, first);
            }
            first = second->right;
            if (first) {
                DestroyRegistryNodeWithDescendants(self, first);
            }
            appFreeUni__FPv(second);
        }
        appFreeUni__FPv(third);
    }
    appFreeUni__FPv(tree);
}
#pragma use_lmw_stmw reset

struct TextureCoordinateNode {
    TextureCoordinateNode* left;
    TextureCoordinateNode* right;
    unsigned int parent_color;
    unsigned int key;
    void* data;
};

struct TextureCoordinateTree {
    char padding00[0x0C];
    TextureCoordinateNode* rootSlot; // Root slot; its ADDRESS serves as the end iterator.
    TextureCoordinateNode* firstNode; // Cached first in-order node, not the tree root.
};

// Delete every mapped texture-coordinate object while preserving the tree and its keys.
int CTextureCoordinateRegistry_DeleteAllValues(TextureCoordinateTree* self) {
    TextureCoordinateNode* node = self->firstNode;
    void* zero = 0;
    TextureCoordinateNode* sentinel = (TextureCoordinateNode*)&self->rootSlot;
    while (sentinel != node) {
        if (node->data != 0) {
            __dl__FPv(node->data);
            node->data = zero;
        }
        if (node->right != 0) {
            TextureCoordinateNode* child = static_cast<TextureCoordinateNode*>(node->right);
            while (child->left != 0)
                child = child->left;
            node = child;
        } else {
            TextureCoordinateNode* parent = reinterpret_cast<TextureCoordinateNode*>(node->parent_color & ~1u);
            while (node != parent->left) {
                node = parent;
                parent = reinterpret_cast<TextureCoordinateNode*>(node->parent_color & ~1u);
            }
            node = parent;
        }
    }
    return 1;
}

struct Fn8025F940Obj {
    void* vtable;      // 0x00
    void* field04;     // 0x04
    int field08;       // 0x08
    int field0C;       // 0x0C
    void* field10;     // 0x10
    // Initialize the base registry, then install the rotation-registry vtable.
    Fn8025F940Obj(void* arg) {
        RotateRegistry_Init(this, arg);
        vtable = (void*)__vt__15CRotateRegistry;
    }
};

struct Fn8025F940VTable {
    void* padding00;
    void* padding04;
    void (*destroy)(Fn8025F940Obj*, int);
    int (*initialize)(Fn8025F940Obj*);
};

class Q0b { public: 
virtual void v08(int); virtual int v0C();
};
// Construct and initialize a rotation registry; destroy it and return null if initialization fails.
void* CRotateRegistry_Create(void* arg) {
    Fn8025F940Obj* p = new Fn8025F940Obj(arg);
    if (p == 0) return 0;
    int r = ((Q0b*)p)->v0C();
    if (r == 0) {
        if (p != 0) {
            ((Q0b*)p)->v08(1);
        }
        return 0;
    }
    return p;
}

// Advance to the next in-order node, masking the color bit from each parent link.
inline void AdvanceLayerControllerIterator(LayerControllerNode*& node) {
    if (node->right != 0) {
        LayerControllerNode* child = node->right;
        while (child->left != 0) child = child->left;
        node = child;
    } else {
        LayerControllerNode* parent;
        while (node != (parent = (LayerControllerNode*)(node->parent_color & ~1u))->left)
            node = parent;
        node = parent;
    }
}

#pragma opt_lifetimes off
// Find the layer controller, delete its mapped value, erase/rebalance the node, and update the first iterator.
int LayerControllerMap_EraseAndDeleteByKey(LayerControllerTree* self, void* arg) {
    // arg borrows an unsigned key word. Return 0 if absent, 1 after deletion.
    // Phase 1: lower_bound + equality test, then delete ONLY the owned value.
    LayerControllerNode* candidate = (LayerControllerNode*)&self->root;
    LayerControllerNode* search = self->root;
    while (search != 0) {
        if (search->key >= *(unsigned int*)arg) {
            candidate = search;
            search = search->left;
        } else {
            search = search->right;
        }
    }
    if (candidate == (LayerControllerNode*)&self->root || *(unsigned int*)arg < candidate->key)
        candidate = (LayerControllerNode*)&self->root;
    if ((LayerControllerNode*)&self->root == candidate) {
        return 0;
    }
    if (candidate->data != 0) {
        __dl__FPv(candidate->data);
        candidate->data = 0;
    }
    // Phase 2 intentionally repeats lookup: this is the inlined map erase path,
    // separate from value destruction. Keep the two searches in their order.
    LayerControllerNode* eraseSearch = self->root;
    LayerControllerNode* eraseCandidate = (LayerControllerNode*)&self->root;
    while (eraseSearch != 0) {
        if (eraseSearch->key >= *(unsigned int*)arg) {
            eraseCandidate = eraseSearch;
            eraseSearch = eraseSearch->left;
        } else {
            eraseSearch = eraseSearch->right;
        }
    }
    if (eraseCandidate == (LayerControllerNode*)&self->root || *(unsigned int*)arg < eraseCandidate->key)
        eraseCandidate = (LayerControllerNode*)&self->root;
    if (eraseCandidate != (LayerControllerNode*)&self->root) {
    // Advance cached begin before unlinking; rebalance, free node, decrement count.
    candidate = eraseCandidate;
    if (eraseCandidate == self->header) {
        AdvanceLayerControllerIterator(eraseCandidate);
        self->header = eraseCandidate;
    }
    RBTree_RebalanceAfterErase(candidate, self->root);
    appFreeUni__FPv(candidate);
    self->count -= 1;
    }
    return 1;
}
#pragma opt_lifetimes reset

} // extern "C"

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU80462308", _ctors_u8025F0BC_0, ModelRegistry_InitFullTurn);
#pragma pop

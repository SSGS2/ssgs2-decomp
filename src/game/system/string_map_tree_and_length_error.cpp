#include <decomp/ct_member.h>
typedef unsigned long new_size_t;
// Packed byte view of the existing string pool; storage and encoding stay in the original symbol.
struct CameraMapErrorStrings {
    char reserved000[0x1B6];
    char lengthErrorTypeName[55]; // 0x1B6: "!std::exception!!std::logic_error!!std::length_error!!"
    char reserved1ED[0x9D];
    char treeInsertLengthError[26]; // 0x28A: "tree::insert length error"
};
namespace exception_runtime {
extern "C" {
void __throw(const char*, void*, void*) __attribute__((noreturn));
void __dt__Q23std12length_errorFv(void*, int);
extern const char cameraWorkStrings[];
}
}

#include "game/exception.h"
// Translation unit 0x800E5FFC-0x800E7340 (4 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

extern "C" {

// This unit combines string-key tree cleanup, byte/word-key insertion and std::length_error
// code; the 'camera' in its string pool name is the only camera link.
// Outer resource-map ABI view (legacy field names kept as-is):
//   both: +00 left, +04 right, +08 parent;
//   outer resource-map node: +0c key, +10 nested tree count, +14 nested root,
//       +18 nested cached node (outside this short view);
//   inner string-value nodes use StringValueNode below, with an owned
//       tagged string at +0x0C instead of the outer key and nested header.
// Thus payloadOrNestedRoot means a child-tree root ONLY in an outer node.
// Cleanup owns both levels of nodes and heap strings; inline strings are part
// of their node. parent is not consulted during postorder destruction.
struct ResourceMapNode {
    ResourceMapNode* leftChild;
    ResourceMapNode* rightChild;
    ResourceMapNode* parent; // 0x08: parent link; cleanup never reads it
    unsigned int flags; // 0x0C: outer key storage; name retained pending shared rename
    void* valueWord10; // 0x10: nested count WORD; address is passed as inner-tree context
    void* payloadOrNestedRoot; // 0x14: owned inner string-tree root
};

// Inner specialization: no nested-map header lives in these six words.
// Use this view after crossing an OUTER node's +0x14 root pointer.
struct StringValueNode {
    StringValueNode* leftChild; // +0: owned child
    StringValueNode* rightChild; // +4: owned child
    StringValueNode* parent; // +8: borrowed tree linkage
    unsigned int stringStorageFlags; // +0x0C: high bit selects heap representation
    unsigned int stringLengthOrInlineBytes; // +0x10: length only in heap representation
    void* heapCharacters; // +0x14: owned only when the heap bit is set
};
typedef char StringValueNodeSizeCheck[sizeof(StringValueNode) == 0x18 ? 1 : -1];

void MapTree_FreeNodesRecursive(void*, ResourceMapNode*);

struct ByteKeyTreeNode {
    ByteKeyTreeNode* leftChild; // 0x00
    ByteKeyTreeNode* rightChild; // 0x04
    ByteKeyTreeNode* parent; // 0x08
    union { char bytes[16]; unsigned int alignment; } payload; // 0x0C: key plus 12-byte tree value
};

typedef char ByteKeyTreeNodeSizeCheck[sizeof(ByteKeyTreeNode)==0x1C ? 1 : -1];

struct TreeHeader {
    int size;
    int rootWord; // 0x04: root-node word; &rootWord is the embedded end sentinel
    ByteKeyTreeNode* headerLink08; // 0x08: unused here; no end-sentinel role established
    ByteKeyTreeNode* cachedNode; // 0x0C: caller-selected node, or &rootWord when cleared
};

extern ByteKeyTreeNode* appMallocDummy__FUl(unsigned int);
extern void CameraStringValueTree_Copy(char*, char*);
extern void RBTree_RebalanceAfterInsert(ByteKeyTreeNode*, int);
extern void appFreeUni__FPv(void*);
extern void CameraResourceMap_DestroyNodePayload(void*, void*);
extern void StringValueTree_DestroySubtree(void*, void*);
extern int __nwa__FUl(unsigned int);
extern void* __nw__FUl(unsigned int);
extern unsigned int strlen(const char*);
extern char* strcpy(char*, const char*);
extern void __throw(void*, void*, void*);
#pragma does_not_return __throw
extern char cameraWorkStrings[];
extern char __vt__Q23std11logic_error[];
extern char s_sharedCharDeleterVtable[];

#pragma push
#pragma use_lmw_stmw on
// Free one inner string node after its children: heap bytes first, then node.
// Valid node addresses make these guards true; their presence/order still affects
// MWCC scheduling. Keep them when editing the owned-heap test that follows.
static inline void DestroyStringValueNode(StringValueNode* node) {
    if ((char*)&node->stringStorageFlags && (char*)&node->stringStorageFlags) {
        if (node->stringStorageFlags >> 31) appFreeUni__FPv(node->heapCharacters);
    }
    appFreeUni__FPv(node);
}

// Finish an outer leaf of the visible unroll, releasing its nested payload before storage.
static inline void DestroyResourceMapLeaf(void* owner, ResourceMapNode* node) {
    if (node->leftChild) MapTree_FreeNodesRecursive(owner, node->leftChild);
    if (node->rightChild) MapTree_FreeNodesRecursive(owner, node->rightChild);
    CameraResourceMap_DestroyNodePayload(owner, (char*)&node->flags);
    appFreeUni__FPv(node);
}

// Finish an inner leaf of the visible unroll, releasing its tagged string before storage.
static inline void DestroyStringValueLeaf(void* owner, StringValueNode* node) {
    if (node->leftChild) StringValueTree_DestroySubtree(owner, node->leftChild);
    if (node->rightChild) StringValueTree_DestroySubtree(owner, node->rightChild);
    DestroyStringValueNode(node);
}

// Remaining link snapshots retain MWCC register allocation.
// Destroy every resource-map node and its owned string-value subtree; traversal is unrolled.
void MapTree_FreeNodes(TreeHeader* tree) {
    // Equivalent traversal, with the first levels unrolled below:
    //   destroyOuter(n): destroyOuter(n.left); destroyOuter(n.right);
    //                    destroyStrings(n.value.root); free(n);
    //   destroyStrings(s): recurse left/right; free heap string if tagged; free(s).
    // *Snapshot aliases are repeated views of one child link, not additional
    // children or allocations. Never free a snapshot separately.
    // Phase 1: destroy root's left and right OUTER branches (helper handles
    // deeper levels). Phase 2: destroy root's INNER value tree. Phase 3: free
    // root and restore {size=0,rootWord=0,cachedNode=&rootWord}. Empty input is a no-op.
    TreeHeader* treeCopy;
    treeCopy = tree;
    ResourceMapNode* subtreeRoot = (ResourceMapNode*)treeCopy->rootWord;
    if (subtreeRoot) {
        ResourceMapNode* subtreeRootLeftSnapshot1 = subtreeRoot->leftChild;
        ResourceMapNode* subtreeRootLeftSnapshot2 = subtreeRootLeftSnapshot1;
        ResourceMapNode* subtreeRootLeftSnapshot3 = subtreeRoot->leftChild;
        ResourceMapNode* subtreeRootLeftSnapshot4 = subtreeRootLeftSnapshot2;
        ResourceMapNode* subtreeRootLeftSnapshot5 = subtreeRootLeftSnapshot4;
        ResourceMapNode* leftBranch = subtreeRootLeftSnapshot5;
        if (leftBranch) {
            ResourceMapNode* childBranch = leftBranch->leftChild;
            if (childBranch) {
                ResourceMapNode* leafBranch = childBranch->leftChild;
                if (leafBranch) {
                    DestroyResourceMapLeaf(tree, leafBranch);
                }
                leafBranch = childBranch->rightChild;
                if (leafBranch) {
                    DestroyResourceMapLeaf(tree, leafBranch);
                }
                CameraResourceMap_DestroyNodePayload(tree, (char*)&childBranch->flags);
                appFreeUni__FPv(childBranch);
            }
            ResourceMapNode* leafBranch = leftBranch->rightChild;
            if (leafBranch) {
                ResourceMapNode* leafBranchLeftSnapshot1 = leafBranch->leftChild;
                ResourceMapNode* leafBranchLeftSnapshot2 = leafBranchLeftSnapshot1;
                ResourceMapNode* rightBranchChild = leafBranchLeftSnapshot2;
                if (rightBranchChild) {
                    DestroyResourceMapLeaf(tree, rightBranchChild);
                }
                ResourceMapNode* leafBranchRightSnapshot1 = leafBranch->rightChild;
                ResourceMapNode* leafBranchRightSnapshot2 = leafBranch->rightChild;
                ResourceMapNode* leafBranchRightSnapshot3 = leafBranchRightSnapshot2;
                rightBranchChild = leafBranchRightSnapshot3;
                if (rightBranchChild) {
                    DestroyResourceMapLeaf(tree, rightBranchChild);
                }
                CameraResourceMap_DestroyNodePayload(tree, (char*)&leafBranch->flags);
                appFreeUni__FPv(leafBranch);
            }
            CameraResourceMap_DestroyNodePayload(tree, (char*)&leftBranch->flags);
            appFreeUni__FPv(leftBranch);
        }
        // Outer-tree phase: left branch is gone; now destroy the right branch.
        ResourceMapNode* rightBranch = subtreeRoot->rightChild;
        if (rightBranch) {
            ResourceMapNode* rightChildBranch = rightBranch->leftChild;
            if (rightChildBranch) {
                ResourceMapNode* rightLeftLeaf = rightChildBranch->leftChild;
                if (rightLeftLeaf) {
                    DestroyResourceMapLeaf(tree, rightLeftLeaf);
                }
                ResourceMapNode* rightChildBranchRightSnapshot1 = rightChildBranch->rightChild;
                rightLeftLeaf = rightChildBranchRightSnapshot1;
                if (rightLeftLeaf) {
                    DestroyResourceMapLeaf(tree, rightLeftLeaf);
                }
                CameraResourceMap_DestroyNodePayload(tree, (char*)&rightChildBranch->flags);
                appFreeUni__FPv(rightChildBranch);
            }
            ResourceMapNode* rightBranchRightSnapshot1 = rightBranch->rightChild;
            rightChildBranch = rightBranchRightSnapshot1;
            if (rightChildBranch) {
                ResourceMapNode* rightRightLeaf = rightChildBranch->leftChild;
                if (rightRightLeaf) {
                    DestroyResourceMapLeaf(tree, rightRightLeaf);
                }
                ResourceMapNode* rightChildBranchRightSnapshot2 = rightChildBranch->rightChild;
                ResourceMapNode* rightChildBranchRightSnapshot3 = rightChildBranch->rightChild;
                rightRightLeaf = rightChildBranchRightSnapshot2;
                if (rightRightLeaf) {
                    DestroyResourceMapLeaf(tree, rightRightLeaf);
                }
                CameraResourceMap_DestroyNodePayload(tree, (char*)&rightChildBranch->flags);
                appFreeUni__FPv(rightChildBranch);
            }
            CameraResourceMap_DestroyNodePayload(tree, (char*)&rightBranch->flags);
            appFreeUni__FPv(rightBranch);
        }
        // Value phase: this outer node owns one nested string tree.
        if ((char*)&subtreeRoot->flags && (char*)&subtreeRoot->valueWord10 && (char*)&subtreeRoot->valueWord10 &&
            (char*)&subtreeRoot->valueWord10 && (char*)&subtreeRoot->valueWord10 &&
            (char*)&subtreeRoot->valueWord10 && (char*)&subtreeRoot->valueWord10) {
            // Value phase: switch overlays from outer key/tree header to
            // INNER tagged-string nodes; high-bit tests below free strings.
            StringValueNode* valueRoot = (StringValueNode*)subtreeRoot->payloadOrNestedRoot;
            if (valueRoot) {
                StringValueNode* valueLeft = valueRoot->leftChild;
                if (valueLeft) {
                    StringValueNode* valueLeftBranch = valueLeft->leftChild;
                    if (valueLeftBranch) {
                        StringValueNode* valueLeftLeaf = valueLeftBranch->leftChild;
                        if (valueLeftLeaf) {
                            DestroyStringValueLeaf((char*)&subtreeRoot->valueWord10, valueLeftLeaf);
                        }
                        StringValueNode* valueLeftBranchRightSnapshot1 = valueLeftBranch->rightChild;
                        valueLeftLeaf = valueLeftBranchRightSnapshot1;
                        if (valueLeftLeaf) {
                            DestroyStringValueLeaf((char*)&subtreeRoot->valueWord10, valueLeftLeaf);
                        }
                        DestroyStringValueNode(valueLeftBranch);
                    }
                    StringValueNode* valueLeftRightSnapshot1 = valueLeft->rightChild;
                    valueLeftBranch = valueLeftRightSnapshot1;
                    if (valueLeftBranch) {
                        StringValueNode* valueRightLeaf = valueLeftBranch->leftChild;
                        if (valueRightLeaf) {
                            DestroyStringValueLeaf((char*)&subtreeRoot->valueWord10, valueRightLeaf);
                        }
                        StringValueNode* valueLeftBranchRightSnapshot2 = valueLeftBranch->rightChild;
                        valueRightLeaf = valueLeftBranchRightSnapshot2;
                        if (valueRightLeaf) {
                            DestroyStringValueLeaf((char*)&subtreeRoot->valueWord10, valueRightLeaf);
                        }
                        DestroyStringValueNode(valueLeftBranch);
                    }
                    DestroyStringValueNode(valueLeft);
                }
                StringValueNode* valueRight = valueRoot->rightChild;
                if (valueRight) {
                    StringValueNode* valueRightLeftSnapshot1 = valueRight->leftChild;
                    StringValueNode* valueRightBranch = valueRightLeftSnapshot1;
                    if (valueRightBranch) {
                        StringValueNode* valueRightLeftLeaf = valueRightBranch->leftChild;
                        if (valueRightLeftLeaf) {
                            DestroyStringValueLeaf((char*)&subtreeRoot->valueWord10, valueRightLeftLeaf);
                        }
                        valueRightLeftLeaf = valueRightBranch->rightChild;
                        if (valueRightLeftLeaf) {
                            DestroyStringValueLeaf((char*)&subtreeRoot->valueWord10, valueRightLeftLeaf);
                        }
                        DestroyStringValueNode(valueRightBranch);
                    }
                    valueRightBranch = valueRight->rightChild;
                    if (valueRightBranch) {
                        StringValueNode* valueRightBranchLeftSnapshot1 = valueRightBranch->leftChild;
                        StringValueNode* valueRightBranchLeftSnapshot2 = valueRightBranch->leftChild;
                        StringValueNode* valueRightBranchLeftSnapshot3 = valueRightBranchLeftSnapshot2;
                        StringValueNode* valueRightBranchLeftSnapshot4 = valueRightBranchLeftSnapshot3;
                        StringValueNode* valueRightBranchLeftSnapshot5 = valueRightBranch->leftChild;
                        StringValueNode* valueRightRightLeaf = valueRightBranchLeftSnapshot5;
                        if (valueRightRightLeaf) {
                            DestroyStringValueLeaf((char*)&subtreeRoot->valueWord10, valueRightRightLeaf);
                        }
                        valueRightRightLeaf = valueRightBranch->rightChild;
                        if (valueRightRightLeaf) {
                            DestroyStringValueLeaf((char*)&subtreeRoot->valueWord10, valueRightRightLeaf);
                        }
                        DestroyStringValueNode(valueRightBranch);
                    }
                    DestroyStringValueNode(valueRight);
                }
                DestroyStringValueNode(valueRoot);
            }
        }
        // Header phase: both outer children and the nested value are gone.
        appFreeUni__FPv(subtreeRoot);
        tree->size = 0;
        tree->rootWord = 0;
        tree->cachedNode = (ByteKeyTreeNode*)&tree->rootWord; // +0x04: root-slot end sentinel
    }
}
#pragma pop

extern char __vt__Q23std12length_error[];
extern void __dt__Q23std12length_errorFv();
extern void __dla__FPv(void*);
extern void dtor_8005E4D8(void*, int);

struct StringGuard {
    char* data;
    char* allocator;
    // Free exception text unless shared ownership was established.
    ~StringGuard() { if (data) __dla__FPv(data); }
};
// std::logic_error layout: vtable, then shared message storage (the vtable is swapped by derived types).
struct LogicErrorBase : std::exception {
    // Install the current exception subtype's original vtable.
    void SetTable(const void* vt) { *(const void**)this = vt; }

    char* data;
    void* holder;
    // Copy error text into shared-reference storage under an allocation guard.
    LogicErrorBase(const char* const& message) {
        SetTable(__vt__Q23std11logic_error);
        char allocator = 0;
        char allocator2 = 0;
        unsigned int length = strlen(message) + 1;
        data = (char*)__nwa__FUl(length * 2 - length);
        char* copy = data;
        {
            StringGuard guard = { copy, &allocator };
            // The two integer words belong to shared-reference control, not a tree.
            struct Holder { void* vtable; int count, controlWord08; char* data; };
            Holder* h = (Holder*)__nw__FUl(16);
            if (h) {
                h->count = 1;
                h->controlWord08 = 1;
                h->vtable = s_sharedCharDeleterVtable;
                h->data = copy;
            }
            holder = h;
            guard.data = 0;
        }
        strcpy(data, message);
    }
};
} // extern "C"
namespace std {
struct length_error : LogicErrorBase {
    // Construct the error message and install the length_error subtype vtable.
    length_error(const char* message) : LogicErrorBase(message) { SetTable(__vt__Q23std12length_error); }
    ~length_error();
    DECOMP_CT_MEMBER(length_error)
};
}
extern "C" {
struct NodeGuard_u800E5FFC {
    char state;
    ByteKeyTreeNode* begin;
    ByteKeyTreeNode* lastNode;
    void* allocator;
    ByteKeyTreeNode* node;
    // Allocate and own one unlinked 0x1c-byte map node during value construction.
    NodeGuard_u800E5FFC(void* count) : state(0), begin(appMallocDummy__FUl(0x1c)), lastNode(begin), allocator(count), node(begin) {}
    // Free only an uncommitted node; the tree takes over after node is cleared.
    ~NodeGuard_u800E5FFC() { if (node) appFreeUni__FPv(node); }
};
struct ValueBase {
    int count, root;
    void* cachedNode;
    // The nested header also uses the address of its root slot as the end sentinel.
    ValueBase() : count(0), root(0), cachedNode(&root) {}
    // Destroy the owned nested tree without deleting this embedded header.
    ~ValueBase() { dtor_8005E4D8(this, 0); }
};
struct ByteKeyTreePairSource { unsigned char key; char alignment[3]; ValueBase value; };
struct ByteKeyTreePair {
    unsigned char key;
    ValueBase value;
    // Copy a byte key and deep-copy the source pair's nested string tree.
    ByteKeyTreePair(const char* const& source) : key(*source), value() {
        CameraStringValueTree_Copy((char*)&value, (char*)&((const ByteKeyTreePairSource*)source)->value);
    }
};
struct WordKeyTreePairSource { unsigned int key; ValueBase value; };
struct WordKeyTreePair {
    unsigned int key;
    ValueBase value;
    // Copy a word key and deep-copy the source pair's nested string tree.
    WordKeyTreePair(const unsigned int* const& source) : key(*source), value() {
        CameraStringValueTree_Copy((char*)&value, (char*)&((const WordKeyTreePairSource*)source)->value);
    }
};
struct TreeLink {
    // p addresses parent's leftChild at offset 0, or the embedded root sentinel.
    // Reinterpret that address as a parent-node pointer for the rebalance ABI.
    ByteKeyTreeNode* parentNode;
    // Store the parent-node/sentinel identity encoded by its child-slot address.
    TreeLink(ByteKeyTreeNode** p) : parentNode((ByteKeyTreeNode*)p) {}
};
} // extern "C"
// Borrow the node's existing payload storage for placement construction.
inline void* operator new(new_size_t, void* p) { return p; }
extern "C" {

// Allocate and link a byte-key entry, copy its nested value tree, then rebalance the map.
ByteKeyTreeNode* CameraByteKeyTree_InsertValueAtLink(TreeHeader* tree, ByteKeyTreeNode** parentLink, int insertAsLeftChild, int updateCachedNode, const char* sourcePair) {
    // parentLink is a pointer to the parent's TWO child slots: [0]=left, [1]=right.
    // At the root it addresses the root-slot end sentinel (&tree->rootWord). It is also stored
    // as entry.parent. Source is borrowed {u8 key,pad[3],nested header}; copy is
    // deep. Return is tree-owned node. updateCachedNode is supplied by the caller;
    // this routine does no key comparisons to establish which extreme is cached.
    // Phase 1: count-overflow check and guarded allocation/value construction.
    if ((unsigned int)tree->size == 0xffffffffu) {
        {
            union { double alignment; unsigned char bytes[sizeof(std::length_error)]; } storage;
            std::length_error* error = (std::length_error*)storage.bytes;
            error->__ct(((const CameraMapErrorStrings*)cameraWorkStrings)->treeInsertLengthError);
            exception_runtime::__throw(((const CameraMapErrorStrings*)exception_runtime::cameraWorkStrings)->lengthErrorTypeName, error,
                (void*)exception_runtime::__dt__Q23std12length_errorFv);
        }
    }
    NodeGuard_u800E5FFC guard(&tree->rootWord);
    try {
        new (&guard.node->payload) ByteKeyTreePair(sourcePair);
    } catch (...) { __throw(0, 0, 0); }
    ByteKeyTreeNode* entry = guard.node;
    // Phase 2: transfer allocation to tree, initialize links, splice and count;
    // phase 3: rebalance through the root-word ABI and optionally update cachedNode.
    entry->rightChild = 0;
    guard.node = 0;
    entry->leftChild = 0;
    new (&entry->parent) TreeLink(parentLink);
    if (insertAsLeftChild)
        *parentLink = entry;
    else
        parentLink[1] = entry;
    ++tree->size;
    RBTree_RebalanceAfterInsert(entry, tree->rootWord);
    if (updateCachedNode)
        tree->cachedNode = entry;
    return entry;
}

void MapTree_FreeNodesRecursive(void*, ResourceMapNode*);

#pragma push
#pragma use_lmw_stmw on
// Allocate and link a word-key entry, copy its nested value tree, then rebalance the map.
ByteKeyTreeNode* CameraWordKeyTree_InsertValueAtLink(TreeHeader* tree, ByteKeyTreeNode** parentLink, int insertAsLeftChild, int updateCachedNode, const unsigned int* sourcePair) {
    // Same child-slot/sentinel/ownership contract as byte-key insertion above;
    // source key is a full 32-bit word followed by a nested-tree header.
    // Phase 1: validate count, allocate, deep-copy value under NodeGuard.
    if ((unsigned int)tree->size == 0xffffffffu) {
        {
            union { double alignment; unsigned char bytes[sizeof(std::length_error)]; } storage;
            std::length_error* error = (std::length_error*)storage.bytes;
            error->__ct(((const CameraMapErrorStrings*)cameraWorkStrings)->treeInsertLengthError);
            exception_runtime::__throw(((const CameraMapErrorStrings*)exception_runtime::cameraWorkStrings)->lengthErrorTypeName, error,
                (void*)exception_runtime::__dt__Q23std12length_errorFv);
        }
    }
    NodeGuard_u800E5FFC guard(&tree->rootWord);
    try {
        new (&guard.node->payload) WordKeyTreePair(sourcePair);
    } catch (...) { __throw(0, 0, 0); }
    ByteKeyTreeNode* entry = guard.node;
    entry->rightChild = 0;
    guard.node = 0;
    entry->leftChild = 0;
    new (&entry->parent) TreeLink(parentLink);
    if (insertAsLeftChild)
        *parentLink = entry;
    else
        parentLink[1] = entry;
    ++tree->size;
    RBTree_RebalanceAfterInsert(entry, tree->rootWord);
    if (updateCachedNode)
        tree->cachedNode = entry;
    return entry;
}
#pragma pop

#pragma push
#pragma use_lmw_stmw on
// Recursively destroy one resource subtree, including heap-owned string values.
void MapTree_FreeNodesRecursive(void* owner, ResourceMapNode* tree) {
    // Requires a non-null OUTER node. owner is borrowed allocator/container
    // context, never freed. Postorder is left branch -> right branch -> nested
    // string tree -> this node; the owner header is not reset by this helper.
    // Path-named *Snapshot aliases below preserve MWCC lifetimes only.
    void* ownerCopy;
    ResourceMapNode* treeCopy;
    ownerCopy = owner;
    treeCopy = tree;
    ResourceMapNode* leftBranch = treeCopy->leftChild;
    if (leftBranch) {
        ResourceMapNode* leftChildBranch = leftBranch->leftChild;
        if (leftChildBranch) {
            ResourceMapNode* leftLeaf = leftChildBranch->leftChild;
            if (leftLeaf) {
                DestroyResourceMapLeaf(owner, leftLeaf);
            }
            leftLeaf = leftChildBranch->rightChild;
            if (leftLeaf) {
                DestroyResourceMapLeaf(owner, leftLeaf);
            }
            CameraResourceMap_DestroyNodePayload(owner, (char*)&leftChildBranch->flags);
            appFreeUni__FPv(leftChildBranch);
        }
        leftChildBranch = leftBranch->rightChild;
        if (leftChildBranch) {
            ResourceMapNode* leftLeaf = leftChildBranch->leftChild;
            if (leftLeaf) {
                DestroyResourceMapLeaf(owner, leftLeaf);
            }
            ResourceMapNode* leftChildBranchRightSnapshot1 = leftChildBranch->rightChild;
            leftLeaf = leftChildBranchRightSnapshot1;
            if (leftLeaf) {
                DestroyResourceMapLeaf(owner, leftLeaf);
            }
            CameraResourceMap_DestroyNodePayload(owner, (char*)&leftChildBranch->flags);
            appFreeUni__FPv(leftChildBranch);
        }
        CameraResourceMap_DestroyNodePayload(owner, (char*)&leftBranch->flags);
        appFreeUni__FPv(leftBranch);
    }
    // Outer-tree phase: finish the other child before touching this node's value.
    ResourceMapNode* rightBranch = tree->rightChild;
    if (rightBranch) {
        ResourceMapNode* rightChildBranch = rightBranch->leftChild;
        if (rightChildBranch) {
            ResourceMapNode* rightLeaf = rightChildBranch->leftChild;
            if (rightLeaf) {
                DestroyResourceMapLeaf(owner, rightLeaf);
            }
            ResourceMapNode* rightChildBranchRightSnapshot1 = rightChildBranch->rightChild;
            rightLeaf = rightChildBranchRightSnapshot1;
            if (rightLeaf) {
                DestroyResourceMapLeaf(owner, rightLeaf);
            }
            CameraResourceMap_DestroyNodePayload(owner, (char*)&rightChildBranch->flags);
            appFreeUni__FPv(rightChildBranch);
        }
        ResourceMapNode* rightBranchRightSnapshot1 = rightBranch->rightChild;
        rightChildBranch = rightBranchRightSnapshot1;
        if (rightChildBranch) {
            ResourceMapNode* rightLeaf = rightChildBranch->leftChild;
            if (rightLeaf) {
                DestroyResourceMapLeaf(owner, rightLeaf);
            }
            ResourceMapNode* rightChildBranchRightSnapshot2 = rightChildBranch->rightChild;
            rightLeaf = rightChildBranchRightSnapshot2;
            if (rightLeaf) {
                DestroyResourceMapLeaf(owner, rightLeaf);
            }
            CameraResourceMap_DestroyNodePayload(owner, (char*)&rightChildBranch->flags);
            appFreeUni__FPv(rightChildBranch);
        }
        CameraResourceMap_DestroyNodePayload(owner, (char*)&rightBranch->flags);
        appFreeUni__FPv(rightBranch);
    }
    if ((char*)&tree->flags && (char*)&tree->valueWord10 && (char*)&tree->valueWord10
    && (char*)&tree->valueWord10 && (char*)&tree->valueWord10 && (char*)&tree->valueWord10
    && (char*)&tree->valueWord10) {
        // Value phase: this pointer now enters the INNER string-node overlay.
        StringValueNode* subtreeRoot = (StringValueNode*)tree->payloadOrNestedRoot;
        if (subtreeRoot) {
            StringValueNode* subtreeRootLeftSnapshot1 = subtreeRoot->leftChild;
            StringValueNode* childBranch = subtreeRootLeftSnapshot1;
            if (childBranch) {
                StringValueNode* nestedLeaf = childBranch->leftChild;
                if (nestedLeaf) {
                    StringValueNode* leafBranch = nestedLeaf->leftChild;
                    if (leafBranch) {
                        DestroyStringValueLeaf(&tree->valueWord10, leafBranch);
                    }
                    StringValueNode* nestedLeafRightSnapshot1 = nestedLeaf->rightChild;
                    leafBranch = nestedLeafRightSnapshot1;
                    if (leafBranch) {
                        DestroyStringValueLeaf(&tree->valueWord10, leafBranch);
                    }
                    DestroyStringValueNode(nestedLeaf);
                }
                StringValueNode* childBranchRightSnapshot1 = childBranch->rightChild;
                StringValueNode* leafBranch = childBranchRightSnapshot1;
                if (leafBranch) {
                    StringValueNode* leafBranchLeftSnapshot1 = leafBranch->leftChild;
                    StringValueNode* leafBranchLeftSnapshot2 = leafBranch->leftChild;
                    StringValueNode* leafBranchLeftSnapshot3 = leafBranch->leftChild;
                    StringValueNode* leafBranchLeftSnapshot4 = leafBranchLeftSnapshot3;
                    StringValueNode* leafBranchLeftSnapshot5 = leafBranchLeftSnapshot4;
                    StringValueNode* leafBranchLeftSnapshot6 = leafBranchLeftSnapshot5;
                    StringValueNode* leafBranchLeftSnapshot7 = leafBranchLeftSnapshot6;
                    StringValueNode* nestedLeaf = leafBranchLeftSnapshot7;
                    if (nestedLeaf) {
                        DestroyStringValueLeaf(&tree->valueWord10, nestedLeaf);
                    }
                    StringValueNode* leafBranchRightSnapshot1 = leafBranch->rightChild;
                    nestedLeaf = leafBranchRightSnapshot1;
                    if (nestedLeaf) {
                        DestroyStringValueLeaf(&tree->valueWord10, nestedLeaf);
                    }
                    DestroyStringValueNode(leafBranch);
                }
                DestroyStringValueNode(childBranch);
            }
            childBranch = subtreeRoot->rightChild;
            if (childBranch) {
                StringValueNode* leafBranch = childBranch->leftChild;
                if (leafBranch) {
                    StringValueNode* leafBranchLeftSnapshot8 = leafBranch->leftChild;
                    StringValueNode* nestedLeaf = leafBranchLeftSnapshot8;
                    if (nestedLeaf) {
                        DestroyStringValueLeaf(&tree->valueWord10, nestedLeaf);
                    }
                    StringValueNode* leafBranchRightSnapshot2 = leafBranch->rightChild;
                    nestedLeaf = leafBranchRightSnapshot2;
                    if (nestedLeaf) {
                        DestroyStringValueLeaf(&tree->valueWord10, nestedLeaf);
                    }
                    DestroyStringValueNode(leafBranch);
                }
                StringValueNode* childBranchRightSnapshot2 = childBranch->rightChild;
                leafBranch = childBranchRightSnapshot2;
                if (leafBranch) {
                    StringValueNode* leafBranchLeftSnapshot9 = leafBranch->leftChild;
                    StringValueNode* leafBranchLeftSnapshot10 = leafBranchLeftSnapshot9;
                    StringValueNode* nestedLeaf = leafBranchLeftSnapshot10;
                    if (nestedLeaf) {
                        DestroyStringValueLeaf(&tree->valueWord10, nestedLeaf);
                    }
                    StringValueNode* leafBranchRightSnapshot3 = leafBranch->rightChild;
                    StringValueNode* leafBranchRightSnapshot4 = leafBranchRightSnapshot3;
                    StringValueNode* leafBranchRightSnapshot5 = leafBranch->rightChild;
                    StringValueNode* leafBranchRightSnapshot6 = leafBranchRightSnapshot5;
                    StringValueNode* leafBranchRightSnapshot7 = leafBranchRightSnapshot6;
                    StringValueNode* leafBranchRightSnapshot8 = leafBranchRightSnapshot7;
                    StringValueNode* leafBranchRightSnapshot9 = leafBranchRightSnapshot4;
                    StringValueNode* leafBranchRightSnapshot10 = leafBranchRightSnapshot8;
                    nestedLeaf = leafBranchRightSnapshot10;
                    if (nestedLeaf) {
                        DestroyStringValueLeaf(&tree->valueWord10, nestedLeaf);
                    }
                    DestroyStringValueNode(leafBranch);
                }
                DestroyStringValueNode(childBranch);
            }
            DestroyStringValueNode(subtreeRoot);
        }
    }
    appFreeUni__FPv(tree);
}
#pragma pop

} // extern "C"

// Translation unit 0x800E7340-0x800E8708 (5 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

#include "sdk_protos.h"

extern "C" {

// String words use the high bit of the first word to select heap versus inline storage.
struct ByteStringEntry {
    unsigned char kind;
    unsigned char pad[3];
    unsigned int stringStorageFlags;
    unsigned int stringLength;
    unsigned int stringDataAddress;
};

// Link-only prefix used by the insertion ABI; it makes no payload-layout claim.
// All real nodes have a parent word at +8 and payload at +0x0C (see NodeHead).
struct Node {
    Node* left;
    Node* right;
};

struct CameraMapHeader {
    unsigned int count;
    unsigned int sentinel;
    Node* last;
    Node* first; // leftmost iterator updated after insertion
};

struct ByteStringSource {
    unsigned char kind;
    unsigned char pad[3];
    unsigned int stringStorageFlags;
    unsigned int stringLength;
    unsigned int stringDataAddress;
};
struct StringPairSource {
    unsigned int stringStorageFlags;
    unsigned int stringLength;
    unsigned int stringDataAddress;
    unsigned int mappedStringStorageFlags;
    unsigned int mappedStringLength;
    unsigned int mappedStringDataAddress;
};

extern void* appMallocDummy__FUl(unsigned int);
extern void String_ReserveCapacity(void*, unsigned int);
extern void String_ReplaceRange(void*, int, int, unsigned int, unsigned int, void*);
extern void RBTree_RebalanceAfterInsert(void*, unsigned int);
extern void appFreeUni__FPv(void*);
extern unsigned int __nwa__FUl(unsigned int);
extern void* __nw__FUl(unsigned int);
extern void StringMap_DestroyStringPair(void*, void*);
extern void CameraByteStringMap_DestroyEntry(void*, void*);
extern void CameraNestedStringMap_DestroyEntry(void*, void*);
extern char __vt__Q23std11logic_error[];
extern char s_sharedCharDeleterVtable[];
extern const char cameraWorkStrings[];

enum { kCameraMapLengthErrorOffset = 0x28A, kLengthErrorTypeOffset = 0x1B6, kTreeCountWrapBias = 0x10000, kTreeCountWrapCompareValue = 0xFFFF };
// The limit test is 32-bit UNSIGNED arithmetic: count+0x10000 == 0xffff
// iff count==0xffffffff (wraparound). kTreeCountWrapCompareValue is the biased
// comparison result. Replacing the test with count>=65535 changes behavior.
extern const char __vt__Q23std12length_error[];
extern void __dt__Q23std12length_errorFv(void*);
extern void __dla__FPv(void*);

struct SharedErrorText { void* vtable; unsigned int strongRefs, weakRefs; void* text; };

struct NodeGuard_u800E7340 {
    void* owner;
    void* p;
    // Release unlinked node storage if payload construction fails.
    ~NodeGuard_u800E7340() { if (p) appFreeUni__FPv(p); }
};

struct AllocatorByte {
    volatile char v;
    char pad[3];
};

// Full byte/string allocation: child links +0/+4, parent +8, byte key +0x0C,
// tagged mapped string +0x10..+0x1B. Node is only the shared two-link prefix.
struct NodeHead {
    NodeHead* left;
    NodeHead* right;
    void* link;
    ByteStringEntry data;
};

// String/string uses the same 12-byte link prefix and two adjacent tagged strings.
struct StringPairNodeHead {
    Node* left;
    Node* right;
    void* parent;
    StringPairSource data;
};
typedef char ByteStringPayloadOffsetCheck[((int)&((NodeHead*)0)->data == 12) ? 1 : -1];
typedef char StringPairPayloadOffsetCheck[((int)&((StringPairNodeHead*)0)->data == 12) ? 1 : -1];

struct StrHold {
    char* p;
    char* ref;
    // Roll back exception-message allocation before its shared ref block takes ownership.
    ~StrHold() { if (p) __dla__FPv(p); }
};

#pragma push
#pragma use_lmw_stmw on
// Allocate, copy the byte/string payload, link the tree node, and rebalance; throw on maximum size.
// link identifies the parent link: before selects its left/right slot; update
// requests updating the cached first iterator. This function does not search.
// source is borrowed; successful return transfers the new node/string to self.
Node* CameraByteStringMap_InsertNode(CameraMapHeader* self, void* link, int before, int update, const ByteStringSource* source) {
    void __throw(const char*, void*, void*) __attribute__((noreturn));
    struct Err {
        void* vtable; char* text; SharedErrorText* value;
        // Build shared exception text without yet changing the logic_error vtable.
        void init(const char* what, AllocatorByte& flagA, AllocatorByte& flagB,
                  char*& dstText, SharedErrorText*& dstValue) {
            vtable = (void*)__vt__Q23std11logic_error;
            flagA.v = 0;
            flagB.v = 0;
            int n = (int)strlen(what) + 1;
            {
                StrHold hold;
                char* copy = (char*)__nwa__FUl(n * 2 - n);
                dstText = copy;
                hold.p = copy;
                hold.ref = (char*)&flagB;
                SharedErrorText* v = (SharedErrorText*)__nw__FUl(0x10);
                if (v) {
                    v->strongRefs = 1;
                    v->weakRefs = 1;
                    v->vtable = s_sharedCharDeleterVtable;
                    v->text = copy;
                }
                dstValue = v;
                hold.p = 0;
            }
            strcpy(dstText, what);
        }
    };
    Err local;
    NodeGuard_u800E7340 guard;
    void* volatile constructedNode;
    void* volatile allocatedNode;
    AllocatorByte zero;
    AllocatorByte flagA;
    AllocatorByte flagB;
    AllocatorByte byteB;
    AllocatorByte byteA;
    // Phase 1: enforce the biased unsigned limit before allocating a node.
    if (self->count + kTreeCountWrapBias == kTreeCountWrapCompareValue) {
        Err* lp = &local;
        lp->init((const char*)cameraWorkStrings + kCameraMapLengthErrorOffset, flagA, flagB, local.text, local.value);
        lp->vtable = (void*)__vt__Q23std12length_error;
        __throw((const char*)cameraWorkStrings + kLengthErrorTypeOffset, lp, (void*)__dt__Q23std12length_errorFv);
    }
    // Phase 2: allocate guarded storage; copy inline strings or deep-copy heap strings.
    zero.v = 0;
    void* allocation = appMallocDummy__FUl(0x1C);
    ByteStringEntry* data = &((NodeHead*)allocation)->data;
    allocatedNode = allocation;
    constructedNode = allocation;
    guard.owner = &self->sentinel;
    guard.p = allocation;
    if (data) {
        unsigned int a = source->stringStorageFlags;
        unsigned int kind = source->kind;
        try {
            data->kind = kind;
            if ((a >> 31) == 0) {
                data->stringStorageFlags = a;
                data->stringLength = source->stringLength;
                data->stringDataAddress = source->stringDataAddress;
            } else {
                data->stringStorageFlags = 0;
                unsigned int b = source->stringLength;
                data->stringLength = 0;
                data->stringDataAddress = 0;
                String_ReserveCapacity(&data->stringStorageFlags, b);
                byteA.v = byteB.v;
                String_ReplaceRange(&data->stringStorageFlags, 0, 0, source->stringDataAddress, source->stringDataAddress + source->stringLength, &byteA);
            }
        } catch (...) {
            __throw(0, 0, 0);
        }
    }
    // Phase 3: commit ownership, initialize links, attach, then rebalance and update first.
    NodeHead* node = (NodeHead*)guard.p;
    node->right = 0;
    guard.p = 0;
    node->left = 0;
    if (&((NodeHead*)node)->link) {
        try {
            ((NodeHead*)node)->link = link;
        } catch (...) {
            __throw(0, 0, 0);
        }
    }
    if (before) *(NodeHead**)link = node;
    else *((NodeHead**)link + 1) = node;
    self->count++;
    RBTree_RebalanceAfterInsert(node, self->sentinel);
    if (update) self->first = (Node*)node;
    return (Node*)node;
}
#pragma pop

struct PairValue { unsigned int a, b, c, d, e, f; };

struct ErrorTextBufferGuard {
    char* p;
    char* ref;
    // Track an exception text allocation until ownership transfer.
    ErrorTextBufferGuard(char* q, char* r) : p(q), ref(r) {}
    // Release text still owned by this guard.
    ~ErrorTextBufferGuard() { if (p) appFreeUni__FPv(p); }
};

struct CameraMapLengthError {
    void* vtable;
    char* text;
    SharedErrorText* value;
    // Construct the length_error object and its shared message reference.
    CameraMapLengthError(const char* what) {
        char flagA;
        char flagB;
        vtable = (void*)__vt__Q23std11logic_error;
        flagB = 0;
        flagA = 0;
        int n = (int)strlen(what) + 1;
        char* copy = (char*)__nwa__FUl(n * 2 - n);
        text = copy;
        ErrorTextBufferGuard hold(copy, &flagA);
        SharedErrorText* v = (SharedErrorText*)__nw__FUl(0x10);
        if (v) {
            v->strongRefs = 1;
            v->weakRefs = 1;
            v->vtable = s_sharedCharDeleterVtable;
            v->text = copy;
        }
        value = v;
        hold.p = 0;
        strcpy(text, what);
        vtable = (void*)__vt__Q23std12length_error;
    }
};

#pragma push
#pragma use_lmw_stmw on
// Insert a copied string/string pair, cleaning up the first string if construction of the second throws.
// Same parent-link/before/update contract as byte/string insertion; no tree search.
Node* CameraStringMap_InsertNode(CameraMapHeader* self, Node* link, int before, int update, const StringPairSource* source) {
    struct Err { void* vtable; char* text; SharedErrorText* value; } local;
    void* volatile constructedPairNode;
    void* volatile allocatedPairNode;
    AllocatorByte flag;
    AllocatorByte flagB, flagA;
    AllocatorByte u14, u10, u0c, u08;
    // Phase 1: limit failure constructs/throws its own shared-message exception.
    if (self->count + kTreeCountWrapBias == kTreeCountWrapCompareValue) {
        const char* what = cameraWorkStrings + kCameraMapLengthErrorOffset;
        Err* lp = &local;
        lp->vtable = __vt__Q23std11logic_error;
        flagB.v = 0;
        flagA.v = 0;
        int n = (int)strlen(what) + 1;
        {
            StrHold hold;
            char* copy = (char*)__nwa__FUl(n * 2 - n);
            lp->text = copy;
            hold.p = copy;
            hold.ref = (char*)&flagA;
            SharedErrorText* value = (SharedErrorText*)__nw__FUl(0x10);
            if (value) {
                value->strongRefs = 1;
                value->weakRefs = 1;
                value->vtable = s_sharedCharDeleterVtable;
                value->text = copy;
            }
            lp->value = value;
            hold.p = 0;
        }
        strcpy(lp->text, what);
        lp->vtable = (void*)__vt__Q23std12length_error;
        __throw(cameraWorkStrings + kLengthErrorTypeOffset, lp, (void*)__dt__Q23std12length_errorFv);
    }
    // Phase 2: construct key then mapped string under the allocation guard.
    flag.v = 0;
    void* base = appMallocDummy__FUl(0x24);
    NodeGuard_u800E7340 guard;
    allocatedPairNode = base;
    constructedPairNode = base;
    guard.owner = &self->sentinel;
    guard.p = base;
    StringPairSource* value = &((StringPairNodeHead*)base)->data;
    if (value) {
        bool firstConstructed = false;
        try {
            if ((source->stringStorageFlags >> 31) == 0) {
                value->stringStorageFlags = source->stringStorageFlags;
                value->stringLength = source->stringLength;
                value->stringDataAddress = source->stringDataAddress;
            } else {
                value->stringStorageFlags = 0;
                value->stringLength = 0;
                value->stringDataAddress = 0;
                String_ReserveCapacity(value, source->stringLength);
                u10.v = u14.v;
                String_ReplaceRange(value, 0, 0, source->stringDataAddress, source->stringDataAddress + source->stringLength, &u10);
            }
            firstConstructed = true;
            if ((source->mappedStringStorageFlags >> 31) == 0) {
                value->mappedStringStorageFlags = source->mappedStringStorageFlags;
                value->mappedStringLength = source->mappedStringLength;
                value->mappedStringDataAddress = source->mappedStringDataAddress;
            } else {
                value->mappedStringStorageFlags = 0;
                value->mappedStringLength = 0;
                value->mappedStringDataAddress = 0;
                String_ReserveCapacity(&value->mappedStringStorageFlags, source->mappedStringLength);
                u08.v = u0c.v;
                String_ReplaceRange(&value->mappedStringStorageFlags, 0, 0, source->mappedStringDataAddress, source->mappedStringDataAddress + source->mappedStringLength, &u08);
            }
        } catch (...) {
            if (firstConstructed && (value->stringStorageFlags >> 31))
                appFreeUni__FPv((void*)value->stringDataAddress);
            throw;
        }
    }
    // Phase 3: commit and attach the fully constructed pair before rebalancing.
    Node* node = (Node*)guard.p;
    node->right = 0;
    guard.p = 0;
    node->left = 0;
    if (&((NodeHead*)node)->link) {
        ((NodeHead*)node)->link = link;
    }
    if (before) *(Node**)link = node;
    else *((Node**)link + 1) = node;
    self->count++;
    RBTree_RebalanceAfterInsert(node, self->sentinel);
    if (update) self->first = node;
    return node;
}
#pragma pop

// Shared word view for three STL instantiations: payload starts at valueA (+12).
// Different destructors interpret the words as a string pair, byte/string pair, or nested map.
struct TreeNode {
    TreeNode* first;
    TreeNode* second;
    unsigned int parentAndColor;
    unsigned int valueA;
    unsigned int payloadWord10; // nested-map header or mapped-string storage flags
    unsigned int valueB;
    unsigned int payloadWord18; // mapped-string heap address in byte/string nodes
    unsigned int payloadWord1C;
    unsigned int payloadWord20; // mapped-string heap address in string/string nodes
};

// Payload ownership by specialization (offsets from a node, not from its payload):
// string/string: key at +0xc, mapped string at +0x18; release both heap forms.
// byte/string: byte key at +0xc, mapped string at +0x10; release mapped heap form.
// nested: numeric key at +0xc, inner-map header +0x10 and root +0x14; release
// inner string-key nodes and their key storage. Pointer values are not deleted here.
// Each function below requires a non-null real node; context is a borrowed tree header.
// Every unrolled block obeys left -> right -> payload -> node. Member-address tests
// are retained inlined destructor guards, not tests for optional payload ownership.
// Valid-node member addresses are non-null; keep every check in its original
// order for MWCC code generation. Conjunctions below flatten those guard scopes.
// Walk guide: outerLeft/Right* are numeric-key nodes; innerRoot/Left/Right* are
// string-key nodes. Every inner leaf releases its heap key (high bit) then node.
// The borrowed context never becomes an owned object and is never freed.

extern void StringValueTree_DestroySubtree(void*, TreeNode*);

#pragma push
#pragma use_lmw_stmw on
// Release one inner string-key payload and its node, after both children are gone.
// The parent word is borrowed linkage; only high-bit-tagged character storage is owned.
static inline void DestroyInnerStringNode(TreeNode* node) {
    if (&node->valueA && &node->valueA) {
        if (node->valueA >> 31) appFreeUni__FPv((void*)node->valueB);
    }
    appFreeUni__FPv(node);
}

void CameraNestedStringMap_DestroySubtree(void*, TreeNode*);
// Finish an outer leaf of the visible unroll; deeper descendants use the recursive ABI.
static inline void DestroyNestedMapLeaf(void* context, TreeNode* node) {
    if (node->first) CameraNestedStringMap_DestroySubtree(context, node->first);
    if (node->second) CameraNestedStringMap_DestroySubtree(context, node->second);
    CameraNestedStringMap_DestroyEntry(context, &node->valueA);
    appFreeUni__FPv(node);
}

// Finish an inner leaf of the visible unroll, including its owned string key.
static inline void DestroyInnerStringLeaf(void* context, TreeNode* node) {
    if (node->first) StringValueTree_DestroySubtree(context, node->first);
    if (node->second) StringValueTree_DestroySubtree(context, node->second);
    DestroyInnerStringNode(node);
}

// Destroy both tree branches, each nested string map payload, and finally the containing node.
void CameraNestedStringMap_DestroySubtree(void* context, TreeNode* source) {
    // Phase 1: destroy the outer left branch, recursively beyond the visible levels.
    TreeNode* outerLeft = source->first;
    if (outerLeft) {
        TreeNode* outerLeftLeft = outerLeft->first;
        if (outerLeftLeft) {
            TreeNode* outerLeftLeftLeft = outerLeftLeft->first;
            if (outerLeftLeftLeft) {
                DestroyNestedMapLeaf(context, outerLeftLeftLeft);
            }
            TreeNode* outerLeftLeftRight = outerLeftLeft->second;
            if (outerLeftLeftRight) {
                DestroyNestedMapLeaf(context, outerLeftLeftRight);
            }
            CameraNestedStringMap_DestroyEntry(context, &outerLeftLeft->valueA);
            appFreeUni__FPv(outerLeftLeft);
        }
        TreeNode* outerLeftRight = outerLeft->second;
        if (outerLeftRight) {
            TreeNode* outerLeftRightLeft = outerLeftRight->first;
            if (outerLeftRightLeft) {
                DestroyNestedMapLeaf(context, outerLeftRightLeft);
            }
            TreeNode* outerLeftRightRight = outerLeftRight->second;
            if (outerLeftRightRight) {
                DestroyNestedMapLeaf(context, outerLeftRightRight);
            }
            CameraNestedStringMap_DestroyEntry(context, &outerLeftRight->valueA);
            appFreeUni__FPv(outerLeftRight);
        }
        CameraNestedStringMap_DestroyEntry(context, &outerLeft->valueA);
        appFreeUni__FPv(outerLeft);
    }
    // Phase 2: destroy the outer right branch before touching this node's own inner map.
    TreeNode* outerRight = source->second;
    if (outerRight) {
        TreeNode* outerRightLeft = outerRight->first;
        if (outerRightLeft) {
            TreeNode* outerRightLeftLeft = outerRightLeft->first;
            if (outerRightLeftLeft) {
                DestroyNestedMapLeaf(context, outerRightLeftLeft);
            }
            TreeNode* outerRightLeftRight = outerRightLeft->second;
            if (outerRightLeftRight) {
                DestroyNestedMapLeaf(context, outerRightLeftRight);
            }
            CameraNestedStringMap_DestroyEntry(context, &outerRightLeft->valueA);
            appFreeUni__FPv(outerRightLeft);
        }
        TreeNode* outerRightRight = outerRight->second;
        if (outerRightRight) {
            TreeNode* outerRightRightLeft = outerRightRight->first;
            if (outerRightRightLeft) {
                DestroyNestedMapLeaf(context, outerRightRightLeft);
            }
            TreeNode* outerRightRightRight = outerRightRight->second;
            if (outerRightRightRight) {
                DestroyNestedMapLeaf(context, outerRightRightRight);
            }
            CameraNestedStringMap_DestroyEntry(context, &outerRightRight->valueA);
            appFreeUni__FPv(outerRightRight);
        }
        CameraNestedStringMap_DestroyEntry(context, &outerRight->valueA);
        appFreeUni__FPv(outerRight);
    }
    // Phase 3: walk the owned inner string tree, using the inner specialization at depth.
    if (&source->valueA && &source->payloadWord10 && &source->payloadWord10 &&
        &source->payloadWord10 && &source->payloadWord10 &&
        &source->payloadWord10 && &source->payloadWord10) {
        TreeNode* innerRoot = (TreeNode*)source->valueB;
        if (innerRoot) {
            TreeNode* innerLeft = innerRoot->first;
            if (innerLeft) {
                TreeNode* innerLeftLeft = innerLeft->first;
                if (innerLeftLeft) {
                    TreeNode* innerLeftLeftLeft = innerLeftLeft->first;
                    if (innerLeftLeftLeft) {
                        DestroyInnerStringLeaf(&source->payloadWord10, innerLeftLeftLeft);
                    }
                    TreeNode* innerLeftLeftRight = innerLeftLeft->second;
                    if (innerLeftLeftRight) {
                        DestroyInnerStringLeaf(&source->payloadWord10, innerLeftLeftRight);
                    }
                    DestroyInnerStringNode(innerLeftLeft);
                }
                TreeNode* innerLeftRight = innerLeft->second;
                if (innerLeftRight) {
                    TreeNode* innerLeftRightLeft = innerLeftRight->first;
                    if (innerLeftRightLeft) {
                        DestroyInnerStringLeaf(&source->payloadWord10, innerLeftRightLeft);
                    }
                    TreeNode* innerLeftRightRight = innerLeftRight->second;
                    if (innerLeftRightRight) {
                        DestroyInnerStringLeaf(&source->payloadWord10, innerLeftRightRight);
                    }
                    DestroyInnerStringNode(innerLeftRight);
                }
                DestroyInnerStringNode(innerLeft);
            }
            TreeNode* innerRight = innerRoot->second;
            if (innerRight) {
                TreeNode* innerRightLeft = innerRight->first;
                if (innerRightLeft) {
                    TreeNode* innerRightLeftLeft = innerRightLeft->first;
                    if (innerRightLeftLeft) {
                        DestroyInnerStringLeaf(&source->payloadWord10, innerRightLeftLeft);
                    }
                    TreeNode* innerRightLeftRight = innerRightLeft->second;
                    if (innerRightLeftRight) {
                        DestroyInnerStringLeaf(&source->payloadWord10, innerRightLeftRight);
                    }
                    DestroyInnerStringNode(innerRightLeft);
                }
                TreeNode* innerRightRight = innerRight->second;
                if (innerRightRight) {
                    TreeNode* innerRightRightLeft = innerRightRight->first;
                    if (innerRightRightLeft) {
                        DestroyInnerStringLeaf(&source->payloadWord10, innerRightRightLeft);
                    }
                    TreeNode* innerRightRightRight = innerRightRight->second;
                    if (innerRightRightRight) {
                        DestroyInnerStringLeaf(&source->payloadWord10, innerRightRightRight);
                    }
                    DestroyInnerStringNode(innerRightRight);
                }
                DestroyInnerStringNode(innerRight);
            }
            DestroyInnerStringNode(innerRoot);
        }
    }
    // Phase 4: no descendant or owned key storage remains; release the outer node.
    appFreeUni__FPv(source);
}
#pragma pop

#pragma use_lmw_stmw on
// Destroy this byte/string map subtree, releasing heap-backed mapped strings before each node.
void MapTree_FreeSubtreeWithRelease(void* context, TreeNode* source) {
    // Phase 1: release the left subtree's descendants, mapped strings, and nodes.
    TreeNode* node = source->first;
    if (node) {
        TreeNode* child = node->first;
        if (child) {
            TreeNode* firstValue = child->first;
            TreeNode* grandchild = firstValue;
            if (grandchild) {
                if (grandchild->first) MapTree_FreeSubtreeWithRelease(context, grandchild->first);
                if (grandchild->second) MapTree_FreeSubtreeWithRelease(context, grandchild->second);
                CameraByteStringMap_DestroyEntry(context, &grandchild->valueA);
                appFreeUni__FPv(grandchild);
            }
            grandchild = child->second;
            if (grandchild) {
                if (grandchild->first) MapTree_FreeSubtreeWithRelease(context, grandchild->first);
                if (grandchild->second) MapTree_FreeSubtreeWithRelease(context, grandchild->second);
                CameraByteStringMap_DestroyEntry(context, &grandchild->valueA);
                appFreeUni__FPv(grandchild);
            }
            if (&child->valueA && &child->payloadWord10) {
                if (child->payloadWord10 >> 31) appFreeUni__FPv((void*)child->payloadWord18);
            }
            appFreeUni__FPv(child);
        }
        TreeNode* child2 = node->second;
        if (child2) {
            TreeNode* grandchild = child2->first;
            if (grandchild) {
                if (grandchild->first) MapTree_FreeSubtreeWithRelease(context, grandchild->first);
                if (grandchild->second) MapTree_FreeSubtreeWithRelease(context, grandchild->second);
                CameraByteStringMap_DestroyEntry(context, &grandchild->valueA);
                appFreeUni__FPv(grandchild);
            }
            TreeNode* secondValue = child2->second;
            grandchild = secondValue;
            if (grandchild) {
                if (grandchild->first) MapTree_FreeSubtreeWithRelease(context, grandchild->first);
                if (grandchild->second) MapTree_FreeSubtreeWithRelease(context, grandchild->second);
                CameraByteStringMap_DestroyEntry(context, &grandchild->valueA);
                appFreeUni__FPv(grandchild);
            }
            if (&child2->valueA && &child2->payloadWord10) {
                if (child2->payloadWord10 >> 31) appFreeUni__FPv((void*)child2->payloadWord18);
            }
            appFreeUni__FPv(child2);
        }
        if (&node->valueA && &node->payloadWord10) {
            if (node->payloadWord10 >> 31) appFreeUni__FPv((void*)node->payloadWord18);
        }
        appFreeUni__FPv(node);
    }
    // Phase 2: apply the same postorder policy to the right subtree.
    TreeNode* sibling = source->second;
    if (sibling) {
        TreeNode* firstValue2 = sibling->first;
        TreeNode* child = firstValue2;
        if (child) {
            TreeNode* grandchild = child->first;
            if (grandchild) {
                if (grandchild->first) MapTree_FreeSubtreeWithRelease(context, grandchild->first);
                if (grandchild->second) MapTree_FreeSubtreeWithRelease(context, grandchild->second);
                CameraByteStringMap_DestroyEntry(context, &grandchild->valueA);
                appFreeUni__FPv(grandchild);
            }
            grandchild = child->second;
            if (grandchild) {
                if (grandchild->first) MapTree_FreeSubtreeWithRelease(context, grandchild->first);
                if (grandchild->second) MapTree_FreeSubtreeWithRelease(context, grandchild->second);
                CameraByteStringMap_DestroyEntry(context, &grandchild->valueA);
                appFreeUni__FPv(grandchild);
            }
            if (&child->valueA && &child->payloadWord10) {
                if (child->payloadWord10 >> 31) appFreeUni__FPv((void*)child->payloadWord18);
            }
            appFreeUni__FPv(child);
        }
        child = sibling->second;
        if (child) {
            TreeNode* firstValue3 = child->first;
            TreeNode* grandchild = firstValue3;
            if (grandchild) {
                if (grandchild->first) MapTree_FreeSubtreeWithRelease(context, grandchild->first);
                if (grandchild->second) MapTree_FreeSubtreeWithRelease(context, grandchild->second);
                CameraByteStringMap_DestroyEntry(context, &grandchild->valueA);
                appFreeUni__FPv(grandchild);
            }
            grandchild = child->second;
            if (grandchild) {
                if (grandchild->first) MapTree_FreeSubtreeWithRelease(context, grandchild->first);
                if (grandchild->second) MapTree_FreeSubtreeWithRelease(context, grandchild->second);
                CameraByteStringMap_DestroyEntry(context, &grandchild->valueA);
                appFreeUni__FPv(grandchild);
            }
            if (&child->valueA && &child->payloadWord10) {
                if (child->payloadWord10 >> 31) appFreeUni__FPv((void*)child->payloadWord18);
            }
            appFreeUni__FPv(child);
        }
        if (&sibling->valueA && &sibling->payloadWord10) {
            if (sibling->payloadWord10 >> 31) appFreeUni__FPv((void*)sibling->payloadWord18);
        }
        appFreeUni__FPv(sibling);
    }
    // Phase 3: release this node's mapped heap string, then the node itself.
    if (&source->valueA && &source->payloadWord10) {
        if (source->payloadWord10 >> 31) appFreeUni__FPv((void*)source->payloadWord18);
    }
    appFreeUni__FPv(source);
}

#pragma use_lmw_stmw on
// Destroy this string/string map subtree, releasing both heap-backed strings before each node.
void StringMap_FreeSubtree(void* context, TreeNode* node) {
    // Phase 1: destroy the left branch, releasing both strings at every visited node.
    TreeNode* grandchild;
    TreeNode* child;
    TreeNode* branch;
    branch = node->first;
    if (branch) {
        child = branch->first;
        if (child) {
            grandchild = child->first;
            if (grandchild) {
                if (grandchild->first) StringMap_FreeSubtree(context, grandchild->first);
                if (grandchild->second) StringMap_FreeSubtree(context, grandchild->second);
                StringMap_DestroyStringPair(context, &grandchild->valueA);
                appFreeUni__FPv(grandchild);
            }
            grandchild = child->second;
            if (grandchild) {
                if (grandchild->first) StringMap_FreeSubtree(context, grandchild->first);
                if (grandchild->second) StringMap_FreeSubtree(context, grandchild->second);
                StringMap_DestroyStringPair(context, &grandchild->valueA);
                appFreeUni__FPv(grandchild);
            }
            StringMap_DestroyStringPair(context, &child->valueA);
            appFreeUni__FPv(child);
        }
        grandchild = branch->second;
        if (grandchild) {
            child = grandchild->first;
            if (child) {
                if (child->first) StringMap_FreeSubtree(context, child->first);
                if (child->second) StringMap_FreeSubtree(context, child->second);
                StringMap_DestroyStringPair(context, &child->valueA);
                appFreeUni__FPv(child);
            }
            child = grandchild->second;
            if (child) {
                if (child->first) StringMap_FreeSubtree(context, child->first);
                if (child->second) StringMap_FreeSubtree(context, child->second);
                StringMap_DestroyStringPair(context, &child->valueA);
                appFreeUni__FPv(child);
            }
            StringMap_DestroyStringPair(context, &grandchild->valueA);
            appFreeUni__FPv(grandchild);
        }
        StringMap_DestroyStringPair(context, &branch->valueA);
        appFreeUni__FPv(branch);
    }

    // Phase 2: destroy the right branch before releasing this node's own pair.
    grandchild = node->second;
    if (grandchild) {
        child = grandchild->first;
        if (child) {
            branch = child->first;
            if (branch) {
                if (branch->first) StringMap_FreeSubtree(context, branch->first);
                if (branch->second) StringMap_FreeSubtree(context, branch->second);
                StringMap_DestroyStringPair(context, &branch->valueA);
                appFreeUni__FPv(branch);
            }
            branch = child->second;
            if (branch) {
                if (branch->first) StringMap_FreeSubtree(context, branch->first);
                if (branch->second) StringMap_FreeSubtree(context, branch->second);
                StringMap_DestroyStringPair(context, &branch->valueA);
                appFreeUni__FPv(branch);
            }
            StringMap_DestroyStringPair(context, &child->valueA);
            appFreeUni__FPv(child);
        }
        child = grandchild->second;
        if (child) {
            branch = child->first;
            if (branch) {
                if (branch->first) StringMap_FreeSubtree(context, branch->first);
                if (branch->second) StringMap_FreeSubtree(context, branch->second);
                StringMap_DestroyStringPair(context, &branch->valueA);
                appFreeUni__FPv(branch);
            }
            branch = child->second;
            if (branch) {
                if (branch->first) StringMap_FreeSubtree(context, branch->first);
                if (branch->second) StringMap_FreeSubtree(context, branch->second);
                StringMap_DestroyStringPair(context, &branch->valueA);
                appFreeUni__FPv(branch);
            }
            StringMap_DestroyStringPair(context, &child->valueA);
            appFreeUni__FPv(child);
        }
        StringMap_DestroyStringPair(context, &grandchild->valueA);
        appFreeUni__FPv(grandchild);
    }

    // Phase 3: mapped string first, key second, containing node last.
    if (&node->valueA) {
        if (&node->payloadWord18) {
            if (node->payloadWord18 >> 31) appFreeUni__FPv((void*)node->payloadWord20);
        }
        if (&node->valueA) {
            if (node->valueA >> 31) appFreeUni__FPv((void*)node->valueB);
        }
    }
    appFreeUni__FPv(node);
}

} // extern "C"

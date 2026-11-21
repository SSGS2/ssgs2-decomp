#include <decomp/static_init.h>
extern "C" {
void __throw(const char*, void*, void*) __attribute__((noreturn));
}
#include "sdk_protos.h"
#include "classes/CClock.h"
#include "classes/CSharedDoc.h"
#include "classes/CGolfDoc.h"

#define AsCClock(p) ((CClock*)(p))
// Translation unit 0x800E8708-0x800E9BCC (12 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

extern "C" {

extern const float kFloat_6_2831855;
extern float s_twoPi_800E8708;
extern void appFreeUni__FPv(void*);
extern void __dl__FPv(void*);
extern void __ct__18NativeBase80201D18Fv(void*);
extern void IActor_AddSkipFlags(void*, unsigned int);
extern void* dtor_8029EA18(void*, int);
extern char __vt__6CClock[];
extern void IActor_DispatchMessage(void*, void*, const char*, int, unsigned int, void*, int, int);
extern void Sound_StopByAlias(void*, const char*);
extern void* g_sharedDoc;
extern void* g_soundManager;
struct ClockMessageStringPool {
    char screen[7];
    char timeSoundAlias[7]; // Shift-JIS localized time alias.
    char golfRule[9];
    char clockClass[7];
    char padding[2];
};
extern const ClockMessageStringPool clockMessageStrings;
extern const float kFloat_0_20943952_Clock;
extern void* appMallocDummy__FUl(unsigned int);
extern void StringValueTree_DestroySubtree(void*, void*);

// Tree links precede an owned-string payload; the high flag bit owns its data.
// first/second are left/right child links. The unrolled destruction below is
// postorder: grandchildren, branch root, then top. Repeated address tests reflect
// nested destructor layers; they do not validate an arbitrary address or allocation.
struct RefNode { RefNode* first; RefNode* second; char pad8[4]; unsigned int flags; char pad10[4]; void* owned; };
struct RefTree { unsigned int count; RefNode* root; RefNode* begin; };
struct CameraMapEntry { unsigned int prefix; RefTree strings; };
struct OwnedCameraArray { void* vtable; void* data; };
enum CameraTreeConstants {
    kNestedTreeOffset = (int)&((CameraMapEntry*)0)->strings,
    kOwnedStringFlagsOffset = (int)&((RefNode*)0)->flags,
    kOwnedStringBit = 31
};
enum ClockConstants { kClockActorSkipFlags = 0x84400, kClockDisplayMessage = 8 };
static const unsigned int kClockDisplayColor = 0xff147eff;

// Release both child branches, then the node and any owned string data.
// Repeated integer-address guards preserve the original compiler output; typed pointer guards do not.
static void releaseNode(RefNode* n, void* owner) {
    if (n->first) StringValueTree_DestroySubtree(owner, n->first);
    if (n->second) StringValueTree_DestroySubtree(owner, n->second);
    if (((int)n + kOwnedStringFlagsOffset) && ((int)n + kOwnedStringFlagsOffset) && ((n->flags >> kOwnedStringBit) != 0)) appFreeUni__FPv(n->owned);
    appFreeUni__FPv(n);
}

#pragma use_lmw_stmw on
// Destroy the nested string tree stored in a camera-map entry, retaining unrolled cleanup order.
void CameraNestedStringMap_DestroyEntry(void* owner, RefNode* entryStorage) {
    // Phase 1: find the embedded nested tree and release its root's left branch.
    if (entryStorage && (owner = (void*)((int)entryStorage + kNestedTreeOffset)) && ((int)entryStorage + kNestedTreeOffset) && ((int)entryStorage + kNestedTreeOffset) && ((int)entryStorage + kNestedTreeOffset) && ((int)entryStorage + kNestedTreeOffset) && ((int)entryStorage + kNestedTreeOffset)) {
        RefNode* top = ((CameraMapEntry*)entryStorage)->strings.root;
        if (top == 0) return;
        RefNode* first = top->first;
        if (first) {
            RefNode* leftLeft = first->first;
            if (leftLeft) {
                RefNode* leftLeftLeft = leftLeft->first;
                if (leftLeftLeft) {
                    if (leftLeftLeft->first) StringValueTree_DestroySubtree(owner, leftLeftLeft->first);
                    if (leftLeftLeft->second) StringValueTree_DestroySubtree(&((CameraMapEntry*)entryStorage)->strings, leftLeftLeft->second);
                    if (((int)leftLeftLeft + kOwnedStringFlagsOffset) && ((int)leftLeftLeft + kOwnedStringFlagsOffset) && ((leftLeftLeft->flags >> kOwnedStringBit) != 0)) appFreeUni__FPv(leftLeftLeft->owned);
                    appFreeUni__FPv(leftLeftLeft);
                }
                RefNode* leftLeftRight = leftLeft->second;
                if (leftLeftRight) releaseNode(leftLeftRight, &((CameraMapEntry*)entryStorage)->strings);
                if (((int)leftLeft + kOwnedStringFlagsOffset) && ((int)leftLeft + kOwnedStringFlagsOffset) && ((leftLeft->flags >> kOwnedStringBit) != 0)) appFreeUni__FPv(leftLeft->owned);
                appFreeUni__FPv(leftLeft);
            }
            RefNode* leftRight = first->second;
            if (leftRight) {
                RefNode* leftRightLeft = leftRight->first;
                if (leftRightLeft) {
                    if (leftRightLeft->first) StringValueTree_DestroySubtree(&((CameraMapEntry*)entryStorage)->strings, leftRightLeft->first);
                    if (leftRightLeft->second) StringValueTree_DestroySubtree(&((CameraMapEntry*)entryStorage)->strings, leftRightLeft->second);
                    if (((int)leftRightLeft + kOwnedStringFlagsOffset) && ((int)leftRightLeft + kOwnedStringFlagsOffset) && ((leftRightLeft->flags >> kOwnedStringBit) != 0)) appFreeUni__FPv(leftRightLeft->owned);
                    appFreeUni__FPv(leftRightLeft);
                }
                RefNode* leftRightRight = leftRight->second;
                if (leftRightRight) releaseNode(leftRightRight, &((CameraMapEntry*)entryStorage)->strings);
                if (((int)leftRight + kOwnedStringFlagsOffset) && ((int)leftRight + kOwnedStringFlagsOffset) && ((leftRight->flags >> kOwnedStringBit) != 0)) appFreeUni__FPv(leftRight->owned);
                appFreeUni__FPv(leftRight);
            }
            if (((int)first + kOwnedStringFlagsOffset) && ((int)first + kOwnedStringFlagsOffset) && ((first->flags >> kOwnedStringBit) != 0)) appFreeUni__FPv(first->owned);
            appFreeUni__FPv(first);
        }
        // Phase 2: release the right branch, then the nested root node. The containing
        // map entry/header is borrowed and is not freed or reset by this function.
        RefNode* last = top->second;
        if (last) {
            RefNode* rightLeft = last->first;
            if (rightLeft) {
                RefNode* rightLeftLeft = rightLeft->first; if (rightLeftLeft) releaseNode(rightLeftLeft, &((CameraMapEntry*)entryStorage)->strings);
                RefNode* rightLeftRight = rightLeft->second; if (rightLeftRight) releaseNode(rightLeftRight, &((CameraMapEntry*)entryStorage)->strings);
                if (((int)rightLeft + kOwnedStringFlagsOffset) && ((int)rightLeft + kOwnedStringFlagsOffset) && ((rightLeft->flags >> kOwnedStringBit) != 0)) appFreeUni__FPv(rightLeft->owned);
                appFreeUni__FPv(rightLeft);
            }
            RefNode* rightRight = last->second;
            if (rightRight) {
                RefNode* rightRightLeft = rightRight->first; if (rightRightLeft) releaseNode(rightRightLeft, &((CameraMapEntry*)entryStorage)->strings);
                RefNode* rightRightRight = rightRight->second; if (rightRightRight) releaseNode(rightRightRight, &((CameraMapEntry*)entryStorage)->strings);
                if (((int)rightRight + kOwnedStringFlagsOffset) && ((int)rightRight + kOwnedStringFlagsOffset) && ((rightRight->flags >> kOwnedStringBit) != 0)) appFreeUni__FPv(rightRight->owned);
                appFreeUni__FPv(rightRight);
            }
            if (((int)last + kOwnedStringFlagsOffset) && ((int)last + kOwnedStringFlagsOffset) && ((last->flags >> kOwnedStringBit) != 0)) appFreeUni__FPv(last->owned);
            appFreeUni__FPv(last);
        }
        if (((int)top + kOwnedStringFlagsOffset) && ((int)top + kOwnedStringFlagsOffset) && ((top->flags >> kOwnedStringBit) != 0)) appFreeUni__FPv(top->owned);
        appFreeUni__FPv(top);
    }
}
#pragma use_lmw_stmw off

#pragma use_lmw_stmw on
// Destroy a camera-resource entry's nested string tree and its owned payloads.
void CameraResourceMap_DestroyNodePayload(void* owner, RefNode* entryStorage) {
    // Phase 1: find the embedded nested tree and release its root's left branch.
    if (entryStorage && (owner = (void*)((int)entryStorage + kNestedTreeOffset)) && ((int)entryStorage + kNestedTreeOffset) && ((int)entryStorage + kNestedTreeOffset) && ((int)entryStorage + kNestedTreeOffset) && ((int)entryStorage + kNestedTreeOffset) && ((int)entryStorage + kNestedTreeOffset)) {
        RefNode* top = ((CameraMapEntry*)entryStorage)->strings.root;
        if (!top) return;
        RefNode* first = top->first;
        if (first) {
            RefNode* leftLeft = first->first;
            if (leftLeft) {
                RefNode* leftLeftLeft = leftLeft->first;
                if (leftLeftLeft) {
                    if (leftLeftLeft->first) StringValueTree_DestroySubtree(owner, leftLeftLeft->first);
                    if (leftLeftLeft->second) StringValueTree_DestroySubtree(&((CameraMapEntry*)entryStorage)->strings, leftLeftLeft->second);
                    if (((int)leftLeftLeft + kOwnedStringFlagsOffset) && ((int)leftLeftLeft + kOwnedStringFlagsOffset) && ((leftLeftLeft->flags >> kOwnedStringBit) != 0)) appFreeUni__FPv(leftLeftLeft->owned);
                    appFreeUni__FPv(leftLeftLeft);
                }
                RefNode* leftLeftRight = leftLeft->second;
                if (leftLeftRight) releaseNode(leftLeftRight, &((CameraMapEntry*)entryStorage)->strings);
                if (((int)leftLeft + kOwnedStringFlagsOffset) && ((int)leftLeft + kOwnedStringFlagsOffset) && ((leftLeft->flags >> kOwnedStringBit) != 0)) appFreeUni__FPv(leftLeft->owned);
                appFreeUni__FPv(leftLeft);
            }
            RefNode* leftRight = first->second;
            if (leftRight) {
                RefNode* leftRightLeft = leftRight->first;
                if (leftRightLeft) releaseNode(leftRightLeft, &((CameraMapEntry*)entryStorage)->strings);
                RefNode* leftRightRight = leftRight->second;
                if (leftRightRight) releaseNode(leftRightRight, &((CameraMapEntry*)entryStorage)->strings);
                if (((int)leftRight + kOwnedStringFlagsOffset) && ((int)leftRight + kOwnedStringFlagsOffset) && ((leftRight->flags >> kOwnedStringBit) != 0)) appFreeUni__FPv(leftRight->owned);
                appFreeUni__FPv(leftRight);
            }
            if (((int)first + kOwnedStringFlagsOffset) && ((int)first + kOwnedStringFlagsOffset) && ((first->flags >> kOwnedStringBit) != 0)) appFreeUni__FPv(first->owned);
            appFreeUni__FPv(first);
        }
        // Phase 2: release the right branch, then the nested root node. The containing
        // map entry/header is borrowed and is not freed or reset by this function.
        RefNode* last = top->second;
        if (last) {
            RefNode* rightLeft = last->first;
            if (rightLeft) {
                RefNode* rightLeftLeft = rightLeft->first; if (rightLeftLeft) releaseNode(rightLeftLeft, &((CameraMapEntry*)entryStorage)->strings);
                RefNode* rightLeftRight = rightLeft->second; if (rightLeftRight) releaseNode(rightLeftRight, &((CameraMapEntry*)entryStorage)->strings);
                if (((int)rightLeft + kOwnedStringFlagsOffset) && ((int)rightLeft + kOwnedStringFlagsOffset) && ((rightLeft->flags >> kOwnedStringBit) != 0)) appFreeUni__FPv(rightLeft->owned);
                appFreeUni__FPv(rightLeft);
            }
            RefNode* rightRight = last->second;
            if (rightRight) {
                RefNode* rightRightLeft = rightRight->first;
                if (rightRightLeft) releaseNode(rightRightLeft, &((CameraMapEntry*)entryStorage)->strings);
                RefNode* rightRightRight = rightRight->second; if (rightRightRight) releaseNode(rightRightRight, &((CameraMapEntry*)entryStorage)->strings);
                if (((int)rightRight + kOwnedStringFlagsOffset) && ((int)rightRight + kOwnedStringFlagsOffset) && ((rightRight->flags >> kOwnedStringBit) != 0)) appFreeUni__FPv(rightRight->owned);
                appFreeUni__FPv(rightRight);
            }
            if (((int)last + kOwnedStringFlagsOffset) && ((int)last + kOwnedStringFlagsOffset) && ((last->flags >> kOwnedStringBit) != 0)) appFreeUni__FPv(last->owned);
            appFreeUni__FPv(last);
        }
        if (((int)top + kOwnedStringFlagsOffset) && ((int)top + kOwnedStringFlagsOffset) && ((top->flags >> kOwnedStringBit) != 0)) appFreeUni__FPv(top->owned);
        appFreeUni__FPv(top);
    }
}
#pragma use_lmw_stmw off

// Release owned array storage and optionally delete its wrapper.
void* dtor_800E9008(void* self, int shouldFree) {
    if (self != 0) {
        void* value = ((OwnedCameraArray*)self)->data;
        if (value != 0) {
            appFreeUni__FPv(value);
        }
        if (shouldFree > 0) {
            __dl__FPv(self);
        }
    }
    return self;
}

// Release owned array storage and optionally delete its wrapper.
void* dtor_800E9068(void* self, int shouldFree) {
    if (self != 0) {
        void* value = ((OwnedCameraArray*)self)->data;
        if (value != 0) {
            appFreeUni__FPv(value);
        }
        if (shouldFree > 0) {
            __dl__FPv(self);
        }
    }
    return self;
}

// Release owned array storage and optionally delete its wrapper.
void* OwnedCameraArray_dtor(void* self, int shouldFree) {
    if (self != 0) {
        void* value = ((OwnedCameraArray*)self)->data;
        if (value != 0) {
            appFreeUni__FPv(value);
        }
        if (shouldFree > 0) {
            __dl__FPv(self);
        }
    }
    return self;
}

// Release owned array storage and optionally delete its wrapper.
void* dtor_800E9128(void* self, int shouldFree) {
    if (self != 0) {
        void* value = ((OwnedCameraArray*)self)->data;
        if (value != 0) appFreeUni__FPv(value);
        if (shouldFree > 0) __dl__FPv(self);
    }
    return self;
}

// Initialize the clock module's full-turn angle constant.
void Clock_InitializeAngleConstant() {
    s_twoPi_800E8708 = kFloat_6_2831855;
}

// Reset document timers and the clock display, then stop the countdown sound.
void CClock_OnInit(void* self) {
    float value = kFloat_0_20943952_Clock;
    unsigned int color = kClockDisplayColor;
    CSharedDoc* sharedDoc = (CSharedDoc*)g_sharedDoc;
    unsigned int timeLimitMs = sharedDoc->turnTimeLimitMs;
    CGolfDoc* golfDoc = (CGolfDoc*)sharedDoc->golfDoc;
    golfDoc->remainingTimeMs = timeLimitMs; // 0x800E9B40: clock initialization, +0x3B8
    CGolfDoc* currentGolfDoc = (CGolfDoc*)((CSharedDoc*)g_sharedDoc)->golfDoc;
    currentGolfDoc->elapsedTimeMs = 0; // +0x3BC
    AsCClock(self)->countdownDisplayValue = value;
    AsCClock(self)->displayColor = color;
    AsCClock(self)->running = 0;
    IActor_DispatchMessage(self, self, clockMessageStrings.screen, kClockDisplayMessage, color, &AsCClock(self)->countdownDisplayValue, 0, 0);
    Sound_StopByAlias(g_soundManager, clockMessageStrings.timeSoundAlias);
}

// flags packs the parent pointer and red/black bit; stringTag carries string ownership.
// Bit 0 of flags is color; all remaining bits hold the parent address.
// A root parent is the address of its owner root slot, never a real payload node.
struct Node { Node* left; Node* right; unsigned int flags; unsigned int stringTag; unsigned int length; unsigned int stringStorage; unsigned int mappedValue; };
struct CameraStringTree { unsigned int count; Node* root; Node* begin; };
// &root doubles as the end sentinel; begin caches the leftmost node or &root
// when empty. Parent links occupy flags with color in bit 0. Character ownership
// is independent: stringTag bit 31 owns stringStorage, otherwise its three words
// hold inline text. Node mappedValue is copied as a word, not deep-owned here.
extern void String_ReserveCapacity(void*, unsigned int);
extern void String_ReplaceRange(void*, int, int, unsigned int, unsigned int, unsigned char*);

struct CopyGuard_u800E8708 {
    void* owner;
    Node* ptr;
    // Start without a node; cloning installs one before payload construction.
    CopyGuard_u800E8708() : ptr(0) {}
    // Own a newly allocated node until its parent link is installed.
    CopyGuard_u800E8708(void* o, Node* p) : owner(o), ptr(p) {}
    // Release an unlinked allocation on exceptional exit.
    ~CopyGuard_u800E8708() { if (ptr) appFreeUni__FPv(ptr); }
};
struct CopyFrame {
    unsigned char flag;
    Node* first;
    Node* second;
    unsigned int pad;
};
// Empty allocator state retained by the matched STL expansion; this byte has no
// string-content meaning. The volatile indeterminate read is original scaffolding,
// not a random seed, initialization value, or setting that callers should supply.
struct CopyByte { unsigned char value; char pad[3]; };
// Copy inline string words or deep-copy an owned string, then copy its mapped value.
static inline void copyKey(unsigned int* key, Node* item) {
    unsigned int zero = 0;
    unsigned int tag = item->stringTag;
    try {
        if ((tag >> kOwnedStringBit) == 0) {
            unsigned int length = item->length;
            key[0] = tag;
            key[1] = length;
            key[2] = item->stringStorage;
        } else {
            key[0] = zero; key[1] = zero; key[2] = zero;
            String_ReserveCapacity(key, item->length);
            CopyByte allocatorCopy;
            volatile CopyByte allocatorState;
            allocatorCopy.value = allocatorState.value;
            String_ReplaceRange(key, 0, 0, item->stringStorage, item->stringStorage + item->length, &allocatorCopy.value);
        }
        key[3] = item->mappedValue;
    } catch (...) { __throw(0, 0, 0); }
}

#pragma use_lmw_stmw on
// Clone one child branch recursively, preserving parent/color bits and ownership during exceptions.
// side!=0 copies source->left into parent->left; side==0 copies the right branch.
// The chosen source child must exist. tree owns each copied node after linking.
void CameraStringValueTree_CloneBranch(void* tree, Node* parent, Node* source, int side) { unsigned int* key; Node* item; int zero = 0; Node* frameSecond; Node* frameFirst; unsigned int frameFlag; CopyGuard_u800E8708 guard;

    *(volatile unsigned char*)&frameFlag = zero;
    Node* node = (Node*)appMallocDummy__FUl(0x1C);
    node->right = (Node*)zero;
    node->left = (Node*)zero;
    *(volatile Node**)&frameFirst = node;
    *(volatile Node**)&frameSecond = node;
    guard.owner = &((CameraStringTree*)tree)->root;
    guard.ptr = node;
    node->flags = (unsigned int)parent | ((node->flags & 1));
    if (side) {
        item = source->left;
        key = &node->stringTag;
        if (key) {
            copyKey(key, item);
        }
        ((CameraStringTree*)tree)->begin = guard.ptr;
        Node* p = guard.ptr;
        parent->left = p;
        parent = p;
        guard.ptr = 0;
        source = source->left;
    } else {
        item = source->right;
        key = &node->stringTag;
        if (key) {
            copyKey(key, item);
        }
        Node* p = guard.ptr;

        parent->right = p;
        parent = p;
        guard.ptr = 0;
        source = source->right;
    }
    if ((1 & source->flags) != 0) parent->flags |= 1; else parent->flags &= ~1u;
    ((CameraStringTree*)tree)->count++;
    if (source->right) CameraStringValueTree_CloneBranch(tree, parent, source, 0);
    if (source->left) CameraStringValueTree_CloneBranch(tree, parent, source, 1);
}
#pragma use_lmw_stmw off

// Release an owned payload, then its containing node.
static void relFree(RefNode* n) {
    if (((int)n + kOwnedStringFlagsOffset) && ((int)n + kOwnedStringFlagsOffset) && ((n->flags >> kOwnedStringBit) != 0)) appFreeUni__FPv(n->owned);
    appFreeUni__FPv(n);
}
// Destroy child subtrees through the generic routine before releasing this node.
static void relLeaf(RefNode* n, void* tree) {
    if (n->first) StringValueTree_DestroySubtree(tree, n->first);
    if (n->second) StringValueTree_DestroySubtree(tree, n->second);
    relFree(n);
}
// Release two child levels in the original unrolled cleanup order.
static void relMid(RefNode* n, void* tree) {
    RefNode* l = n->first;
    if (l) relLeaf(l, tree);
    RefNode* r = n->second;
    if (r) relLeaf(r, tree);
    relFree(n);
}
// Release three child levels in the original unrolled cleanup order.
static void relUp(RefNode* n, void* tree) {
    RefNode* l = n->first;
    if (l) relMid(l, tree);
    RefNode* r = n->second;
    if (r) relMid(r, tree);
    relFree(n);
}
// Release four child levels in the original unrolled cleanup order.
static void relTop(RefNode* n, void* tree) {
    RefNode* l = n->first;
    if (l) relUp(l, tree);
    RefNode* r = n->second;
    if (r) relUp(r, tree);
    relFree(n);
}
// Destroy a complete temporary string tree after its storage has been exchanged.
static void fn_800E91B8(void* stringTree, void* rootNode) {
    RefNode* root = (RefNode*)rootNode;
    if (root) relTop(root, stringTree);
}

#pragma use_lmw_stmw on
// Deep-copy a nonempty string tree, swap it into the destination, then destroy the old tree.
// An empty source leaves self unchanged (it does not clear it). Both headers are
// borrowed, and copied character buffers become destination-owned. Self-copy is
// supported by building the replacement before exchanging the headers.
void CameraStringValueTree_Copy(void* self, const void* other) {
    typedef CameraStringTree Tree;
    if (((const CameraStringTree*)other)->root == 0) return;
    // Phase 1: copy the root payload while the allocation guard owns its node.
    Tree tmp;
    Node* frameSecond;
    Node* frameFirst;
    unsigned int frameFlag;
    CopyGuard_u800E8708 guard;

     *(volatile unsigned char*)&frameFlag = 0;
    Node* n = (Node*)appMallocDummy__FUl(0x1C);
    *(volatile Node**)&frameFirst = n;
    Node* src = ((const CameraStringTree*)other)->root;
    unsigned int* key = &n->stringTag;
    *(volatile Node**)&frameSecond = n;
    guard.owner = &((CameraStringTree*)self)->root;
    guard.ptr = n;

    if (key) {
        copyKey(key, src);
    }

    // Phase 2: establish the temporary sentinel and recursively clone children.
    tmp.count = 0;
    tmp.root = 0;
    Node* flagSrc = ((const CameraStringTree*)other)->root;
    tmp.begin = (Node*)&tmp.root;
    guard.ptr->right = 0;
    guard.ptr->left = 0;
    guard.ptr->flags = (unsigned int)&tmp.root | ((1 & guard.ptr->flags));
    if (flagSrc->flags & 1) guard.ptr->flags |= 1; else guard.ptr->flags &= ~1u;
    __typeof__(guard.ptr) newRoot = guard.ptr; tmp.root = guard.ptr;
    tmp.count = 1;
    __typeof__(guard.ptr) newParent = guard.ptr;
    tmp.begin = newRoot;
    guard.ptr = 0;
    Node* parent = newParent;
    if ((((const CameraStringTree*)other)->root)->right) CameraStringValueTree_CloneBranch(&tmp, parent, ((const CameraStringTree*)other)->root, 0);
    if ((((const CameraStringTree*)other)->root)->left) CameraStringValueTree_CloneBranch(&tmp, tmp.root, ((const CameraStringTree*)other)->root, 1);

    // Phase 3: exchange ownership and repair both roots' sentinel parent links.
    Tree* dst = (Tree*)self;
    if (dst != &tmp) {
        unsigned int c = dst->count;
        dst->count = tmp.count;
        tmp.count = c;
        Node* r = dst->root;
        dst->root = tmp.root;
        tmp.root = r;
        Node* b = dst->begin;
        dst->begin = tmp.begin;
        tmp.begin = b;
        if (dst->root) dst->root->flags = (unsigned int)&dst->root | (dst->root->flags & 1);
        else dst->begin = (Node*)&dst->root;
        if (tmp.root) tmp.root->flags = (unsigned int)&tmp.root | (1 & tmp.root->flags);
        else tmp.begin = (Node*)&tmp.root;
    }
    // Phase 4: the temporary now owns the old destination tree; release it.
    fn_800E91B8(&tmp, tmp.root);
}
#pragma use_lmw_stmw off

struct RefThing {
    unsigned int mapKey;
    unsigned int flags;
    unsigned int stringLength;
    void* ptr;
};

// Free a byte-string entry's data only when its ownership flag is set.
void CameraByteStringMap_DestroyEntry(void* unused, RefThing* entry) {
    if (entry != 0 && &entry->flags != 0 && (entry->flags >> kOwnedStringBit) != 0) {
        appFreeUni__FPv(entry->ptr);
    }
}

struct NativeBase80201D18 {
    void* vtable;
    // Construct the clock's native actor base.
    NativeBase80201D18() { __ct__18NativeBase80201D18Fv(this); }
    // Destroy the native actor base without deleting this storage.
    ~NativeBase80201D18() { dtor_8029EA18(this, 0); }
};

struct ClockActorImplementation : NativeBase80201D18 {
    char data[0x24];
    // Install the clock vtable and its initial actor skip flags.
    ClockActorImplementation() {
        *(void**)this = &__vt__6CClock;
        IActor_AddSkipFlags(this, kClockActorSkipFlags);
    }
};

// Allocate a clock actor and initialize its native base and message skip flags.
ClockActorImplementation* CClock_Create() {
    return new ClockActorImplementation;
}

} // extern "C"

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU804620E8", _ctors_u800E8708_0, Clock_InitializeAngleConstant);
#pragma pop


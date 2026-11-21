#include <decomp/cw_string_guard.h>
#include <decomp/static_init.h>
typedef unsigned long new_size_t;
#include "classes/CGolfCaddie.h"

// CodeWarrior strings share a 12-byte inline/heap representation.
// The high flags bit selects heap storage; otherwise characters begin inside the header.
// Narrow characters begin at +1; wide characters at +2. Length excludes the terminator.
struct CaddieStringStorage {
    unsigned int flags, length;
    void* buffer;
};
struct DialogueStringPair {
    CaddieStringStorage key;
    unsigned char valueTag;
    char reserved[3];
    CaddieStringStorage value;
};
struct IntStringPayload {
    int key;
    CaddieStringStorage text;
};
struct CaddieTreeLinks {
    void* left;
    void* right;
    unsigned int parentAndColor;
};
// Each node owns its payload strings at +0xC. The low parent bit stores tree color.
// DestroyValue destroys only the payload; DestroySubtree also frees every node.
// Subtree entry points require a real node, never the container's end sentinel.
typedef char CaddieTreeLinks_size_check[(sizeof(CaddieTreeLinks) == 0xC) ? 1 : -1];
struct CaddieStringTree {
    unsigned int count;
    void* root;
    void* sentinel;
    void* firstNode; // Legacy name: cached leftmost node; updated when insertion precedes all keys.
};
struct ExceptionCharReference {
    void* vtable;
    unsigned int strongRefs, weakRefs;
    void* buffer;
};

// Verified packed golfCaddieStrings entries (globals round 1).
enum CaddieStringOffset {
    kShadowGroupName = 0x20, kSunLightName = 0x27, kLengthErrorType = 0x3F,
    kStoryPetFormat = 0x1BF, kItemPetFormat = 0x1DB, kDefaultPetFormat = 0x1ED,
    kPetCenterName = 0x203, kTreeInsertLengthError = 0x20A,
    kStringReserveLengthError = 0x224, kCaddieClassName = 0x2B4
};
enum CaddieTreeLayout {
    kTreePayloadOffset = sizeof(CaddieTreeLinks),
    kSecondaryCaddieBaseOffset = 0x18,
};
enum CaddieStringCapacity {
    kWideInlineCapacity = 5,
    kMaxWideStringLength = 0x7FFFFFFE,
};

#include "iff/IffStruct.h"
#include "classes/CSharedDoc.h"
#define AsCSharedDoc(p) ((CSharedDoc*)(p))
#include "classes/ErrorBuffer80132240.h"
#define AsErrorBuffer80132240(p) ((ErrorBuffer80132240*)(p))
#include "classes/IntStringTreeNode8013.h"
#define AsIntStringTreeNode8013(p) ((IntStringTreeNode8013*)(p))
// Translation unit 0x80130F58-0x8013246C (16 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

#include "game/singletons.h"
#include <decomp/cw_bitfields.h>
// Use caller-provided storage for a node payload without allocating another block.
inline void* operator new(new_size_t, void* p) { return p; }
extern "C" {

extern unsigned char s_golfCaddieTypeInfo;
// Return the caddie actor type descriptor.
void* CGolfCaddie_GetRTTI() { return &s_golfCaddieTypeInfo; }

void CGolfCaddie_Draw(void*);
// Adjust the secondary-base pointer by 0x18 before drawing the caddie.
void CGolfCaddie_vfunc_1Thunk18(void* p) { CGolfCaddie_Draw((char*)p - kSecondaryCaddieBaseOffset); }

void appFreeUni__FPv(void*);
void IntStringTree_DestroyValue(void*, void*);
void __dl__FPv(void*);
// Free an owned error buffer and optionally delete its wrapper.
void* dtor_80132240(void* p, int deleting) {
    if (p != 0) {
        void* object = AsErrorBuffer80132240(p)->buffer;
        if (object != 0) appFreeUni__FPv(object);
        if (deleting > 0) __dl__FPv(p);
    }
    return p;
}
// Free an owned error buffer and optionally delete its wrapper.
void* dtor_801322A0(void* p, int deleting) {
    if (p != 0) {
        void* object = AsErrorBuffer80132240(p)->buffer;
        if (object != 0) appFreeUni__FPv(object);
        if (deleting > 0) __dl__FPv(p);
    }
    return p;
}

// Release heap-backed key and value strings in a dialogue map payload.
void CaddieDialogueStringPair_Destroy(void*, void* p) {
    if (p == 0) return;
    if ((int)p + 0xC != 0) {
        if ((int)p + 0x10 != 0) {
            if ((((DialogueStringPair*)p)->value.flags >> 31) != 0) {
                appFreeUni__FPv(((DialogueStringPair*)p)->value.buffer);
            }
        }
    }
    if (p != 0) {
        if ((((DialogueStringPair*)p)->key.flags >> 31) != 0) {
            appFreeUni__FPv(((DialogueStringPair*)p)->key.buffer);
        }
    }
}

#pragma push
#pragma use_lmw_stmw on
// Destroy string payloads and free a subtree in postorder; the top levels are unrolled.
void IntStringTree_DestroySubtree(void* self, void* p) {
    // Descend left, then right, then destroy the root; the repeated blocks inline three levels.
    void* root = p;
    void* owner = self;
    void* walkSlotA;
    void* walkSlotB;
    void* walkSlotC;

    // Visit root.left; free descendants before this node's payload/allocation.
    walkSlotA = AsIntStringTreeNode8013(root)->left;
    if (walkSlotA != 0) {
        // Visit root.left.left; free descendants before this node's payload/allocation.
        walkSlotB = AsIntStringTreeNode8013(walkSlotA)->left;
        if (walkSlotB != 0) {
            // Visit root.left.left.left; free descendants before this node's payload/allocation.
            walkSlotC = AsIntStringTreeNode8013(walkSlotB)->left;
            if (walkSlotC != 0) {
                if (AsIntStringTreeNode8013(walkSlotC)->left != 0) IntStringTree_DestroySubtree(owner, AsIntStringTreeNode8013(walkSlotC)->left);
                if (AsIntStringTreeNode8013(walkSlotC)->right != 0) IntStringTree_DestroySubtree(owner, AsIntStringTreeNode8013(walkSlotC)->right);
                IntStringTree_DestroyValue(owner, &AsIntStringTreeNode8013(walkSlotC)->key);
                appFreeUni__FPv(walkSlotC);
            }
            // Visit root.left.left.right; free descendants before this node's payload/allocation.
            walkSlotC = AsIntStringTreeNode8013(walkSlotB)->right;
            if (walkSlotC != 0) {
                if (AsIntStringTreeNode8013(walkSlotC)->left != 0) IntStringTree_DestroySubtree(owner, AsIntStringTreeNode8013(walkSlotC)->left);
                if (AsIntStringTreeNode8013(walkSlotC)->right != 0) IntStringTree_DestroySubtree(owner, AsIntStringTreeNode8013(walkSlotC)->right);
                IntStringTree_DestroyValue(owner, &AsIntStringTreeNode8013(walkSlotC)->key);
                appFreeUni__FPv(walkSlotC);
            }
            if ((int)walkSlotB + 0xC != 0 && (int)walkSlotB + 0x10 != 0 && (AsIntStringTreeNode8013(walkSlotB)->stringFlags >> 31)) appFreeUni__FPv(AsIntStringTreeNode8013(walkSlotB)->stringBuffer);
            appFreeUni__FPv(walkSlotB);
        }
        // Visit root.left.right; free descendants before this node's payload/allocation.
        walkSlotC = AsIntStringTreeNode8013(walkSlotA)->right;
        if (walkSlotC != 0) {
            // Visit root.left.right.left; free descendants before this node's payload/allocation.
            walkSlotB = AsIntStringTreeNode8013(walkSlotC)->left;
            if (walkSlotB != 0) {
                if (AsIntStringTreeNode8013(walkSlotB)->left != 0) IntStringTree_DestroySubtree(owner, AsIntStringTreeNode8013(walkSlotB)->left);
                if (AsIntStringTreeNode8013(walkSlotB)->right != 0) IntStringTree_DestroySubtree(owner, AsIntStringTreeNode8013(walkSlotB)->right);
                IntStringTree_DestroyValue(owner, &AsIntStringTreeNode8013(walkSlotB)->key);
                appFreeUni__FPv(walkSlotB);
            }
            // Visit root.left.right.right; free descendants before this node's payload/allocation.
            walkSlotB = AsIntStringTreeNode8013(walkSlotC)->right;
            if (walkSlotB != 0) {
                if (AsIntStringTreeNode8013(walkSlotB)->left != 0) IntStringTree_DestroySubtree(owner, AsIntStringTreeNode8013(walkSlotB)->left);
                if (AsIntStringTreeNode8013(walkSlotB)->right != 0) IntStringTree_DestroySubtree(owner, AsIntStringTreeNode8013(walkSlotB)->right);
                IntStringTree_DestroyValue(owner, &AsIntStringTreeNode8013(walkSlotB)->key);
                appFreeUni__FPv(walkSlotB);
            }
            if ((int)walkSlotC + 0xC != 0 && (int)walkSlotC + 0x10 != 0 && (AsIntStringTreeNode8013(walkSlotC)->stringFlags >> 31)) appFreeUni__FPv(AsIntStringTreeNode8013(walkSlotC)->stringBuffer);
            appFreeUni__FPv(walkSlotC);
        }
        if ((int)walkSlotA + 0xC != 0 && (int)walkSlotA + 0x10 != 0 && (AsIntStringTreeNode8013(walkSlotA)->stringFlags >> 31)) appFreeUni__FPv(AsIntStringTreeNode8013(walkSlotA)->stringBuffer);
        appFreeUni__FPv(walkSlotA);
    }

    // Visit root.right; free descendants before this node's payload/allocation.
    walkSlotC = AsIntStringTreeNode8013(root)->right;
    if (walkSlotC != 0) {
        // Visit root.right.left; free descendants before this node's payload/allocation.
        walkSlotB = AsIntStringTreeNode8013(walkSlotC)->left;
        if (walkSlotB != 0) {
            // Visit root.right.left.left; free descendants before this node's payload/allocation.
            walkSlotA = AsIntStringTreeNode8013(walkSlotB)->left;
            if (walkSlotA != 0) {
                if (AsIntStringTreeNode8013(walkSlotA)->left != 0) IntStringTree_DestroySubtree(owner, AsIntStringTreeNode8013(walkSlotA)->left);
                if (AsIntStringTreeNode8013(walkSlotA)->right != 0) IntStringTree_DestroySubtree(owner, AsIntStringTreeNode8013(walkSlotA)->right);
                IntStringTree_DestroyValue(owner, &AsIntStringTreeNode8013(walkSlotA)->key);
                appFreeUni__FPv(walkSlotA);
            }
            // Visit root.right.left.right; free descendants before this node's payload/allocation.
            walkSlotA = AsIntStringTreeNode8013(walkSlotB)->right;
            if (walkSlotA != 0) {
                if (AsIntStringTreeNode8013(walkSlotA)->left != 0) IntStringTree_DestroySubtree(owner, AsIntStringTreeNode8013(walkSlotA)->left);
                if (AsIntStringTreeNode8013(walkSlotA)->right != 0) IntStringTree_DestroySubtree(owner, AsIntStringTreeNode8013(walkSlotA)->right);
                IntStringTree_DestroyValue(owner, &AsIntStringTreeNode8013(walkSlotA)->key);
                appFreeUni__FPv(walkSlotA);
            }
            if ((int)walkSlotB + 0xC != 0 && (int)walkSlotB + 0x10 != 0 && (AsIntStringTreeNode8013(walkSlotB)->stringFlags >> 31)) appFreeUni__FPv(AsIntStringTreeNode8013(walkSlotB)->stringBuffer);
            appFreeUni__FPv(walkSlotB);
        }
        // Visit root.right.right; free descendants before this node's payload/allocation.
        walkSlotB = AsIntStringTreeNode8013(walkSlotC)->right;
        if (walkSlotB != 0) {
            // Visit root.right.right.left; free descendants before this node's payload/allocation.
            walkSlotA = AsIntStringTreeNode8013(walkSlotB)->left;
            if (walkSlotA != 0) {
                if (AsIntStringTreeNode8013(walkSlotA)->left != 0) IntStringTree_DestroySubtree(owner, AsIntStringTreeNode8013(walkSlotA)->left);
                if (AsIntStringTreeNode8013(walkSlotA)->right != 0) IntStringTree_DestroySubtree(owner, AsIntStringTreeNode8013(walkSlotA)->right);
                IntStringTree_DestroyValue(owner, &AsIntStringTreeNode8013(walkSlotA)->key);
                appFreeUni__FPv(walkSlotA);
            }
            // Visit root.right.right.right; free descendants before this node's payload/allocation.
            walkSlotA = AsIntStringTreeNode8013(walkSlotB)->right;
            if (walkSlotA != 0) {
                if (AsIntStringTreeNode8013(walkSlotA)->left != 0) IntStringTree_DestroySubtree(owner, AsIntStringTreeNode8013(walkSlotA)->left);
                if (AsIntStringTreeNode8013(walkSlotA)->right != 0) IntStringTree_DestroySubtree(owner, AsIntStringTreeNode8013(walkSlotA)->right);
                IntStringTree_DestroyValue(owner, &AsIntStringTreeNode8013(walkSlotA)->key);
                appFreeUni__FPv(walkSlotA);
            }
            if ((int)walkSlotB + 0xC != 0 && (int)walkSlotB + 0x10 != 0 && (AsIntStringTreeNode8013(walkSlotB)->stringFlags >> 31)) appFreeUni__FPv(AsIntStringTreeNode8013(walkSlotB)->stringBuffer);
            appFreeUni__FPv(walkSlotB);
        }
        if ((int)walkSlotC + 0xC != 0 && (int)walkSlotC + 0x10 != 0 && (AsIntStringTreeNode8013(walkSlotC)->stringFlags >> 31)) appFreeUni__FPv(AsIntStringTreeNode8013(walkSlotC)->stringBuffer);
        appFreeUni__FPv(walkSlotC);
    }

    if ((int)root + 0xC != 0 && (int)root + 0x10 != 0 && (AsIntStringTreeNode8013(root)->stringFlags >> 31)) appFreeUni__FPv(AsIntStringTreeNode8013(root)->stringBuffer);
    appFreeUni__FPv(root);
}
#pragma pop

struct TreeNode {
    TreeNode* left;
    TreeNode* right;
    unsigned int parentAndColor;
    DialogueStringPair payload;
};
#pragma push
#pragma use_lmw_stmw on
#pragma inline_depth(0)
// Destroy child subtrees, then this node's owned pair and allocation (not just a leaf).
// Keep expansion under inline_depth(0): an ordinary helper would add an out-of-line call.
#define DESTROY_DIALOGUE_SUBTREE_ROOT(n) \
    if ((n)->left) CaddieDialogueStringMap_DestroySubtree(self, (n)->left); \
    if ((n)->right) CaddieDialogueStringMap_DestroySubtree(self, (n)->right); \
    CaddieDialogueStringPair_Destroy(self, &(n)->payload); \
    appFreeUni__FPv(n)
// Destroy dialogue pairs and free a subtree in postorder; retain the unrolled cleanup shape.
void CaddieDialogueStringMap_DestroySubtree(void* self, TreeNode* link) {
    // The same postorder walk owns both strings in each dialogue pair, then its node.
    // Visit root.left; free descendants before this node's payload/allocation.
    TreeNode* walkSlotA = link->left;
    TreeNode* walkSlotB;
    TreeNode* walkSlotC;
    if (walkSlotA) {
        // Visit root.left.left; free descendants before this node's payload/allocation.
        walkSlotB = walkSlotA->left;
        if (walkSlotB) {
            // Visit root.left.left.left; free descendants before this node's payload/allocation.
            walkSlotC = walkSlotB->left;
            if (walkSlotC) { DESTROY_DIALOGUE_SUBTREE_ROOT(walkSlotC); }
            // Visit root.left.left.right; free descendants before this node's payload/allocation.
            walkSlotC = walkSlotB->right;
            if (walkSlotC) { DESTROY_DIALOGUE_SUBTREE_ROOT(walkSlotC); }
            CaddieDialogueStringPair_Destroy(self, &walkSlotB->payload);
            appFreeUni__FPv(walkSlotB);
        }
        // Visit root.left.right; free descendants before this node's payload/allocation.
        walkSlotC = walkSlotA->right;
        if (walkSlotC) {
            // Visit root.left.right.left; free descendants before this node's payload/allocation.
            walkSlotB = walkSlotC->left;
            if (walkSlotB) { DESTROY_DIALOGUE_SUBTREE_ROOT(walkSlotB); }
            // Visit root.left.right.right; free descendants before this node's payload/allocation.
            walkSlotB = walkSlotC->right;
            if (walkSlotB) { DESTROY_DIALOGUE_SUBTREE_ROOT(walkSlotB); }
            CaddieDialogueStringPair_Destroy(self, &walkSlotC->payload);
            appFreeUni__FPv(walkSlotC);
        }
        CaddieDialogueStringPair_Destroy(self, &walkSlotA->payload);
        appFreeUni__FPv(walkSlotA);
    }
    // Visit root.right; free descendants before this node's payload/allocation.
    walkSlotC = link->right;
    if (walkSlotC) {
        // Visit root.right.left; free descendants before this node's payload/allocation.
        walkSlotB = walkSlotC->left;
        if (walkSlotB) {
            // Visit root.right.left.left; free descendants before this node's payload/allocation.
            walkSlotA = walkSlotB->left;
            if (walkSlotA) { DESTROY_DIALOGUE_SUBTREE_ROOT(walkSlotA); }
            // Visit root.right.left.right; free descendants before this node's payload/allocation.
            walkSlotA = walkSlotB->right;
            if (walkSlotA) { DESTROY_DIALOGUE_SUBTREE_ROOT(walkSlotA); }
            CaddieDialogueStringPair_Destroy(self, &walkSlotB->payload);
            appFreeUni__FPv(walkSlotB);
        }
        // Visit root.right.right; free descendants before this node's payload/allocation.
        walkSlotB = walkSlotC->right;
        if (walkSlotB) {
            // Visit root.right.right.left; free descendants before this node's payload/allocation.
            walkSlotA = walkSlotB->left;
            if (walkSlotA) { DESTROY_DIALOGUE_SUBTREE_ROOT(walkSlotA); }
            // Visit root.right.right.right; free descendants before this node's payload/allocation.
            walkSlotA = walkSlotB->right;
            if (walkSlotA) { DESTROY_DIALOGUE_SUBTREE_ROOT(walkSlotA); }
            CaddieDialogueStringPair_Destroy(self, &walkSlotB->payload);
            appFreeUni__FPv(walkSlotB);
        }
        CaddieDialogueStringPair_Destroy(self, &walkSlotC->payload);
        appFreeUni__FPv(walkSlotC);
    }
    if ((int)link + 0xC != 0) {
        if ((int)link + 0x18 != 0) {
            if ((int)link + 0x1C != 0) {
                if (link->payload.value.flags >> 31)
                    appFreeUni__FPv(link->payload.value.buffer);
            }
        }
        if ((unsigned int)link + 0xCu != 0) {
            if (link->payload.key.flags >> 31)
                appFreeUni__FPv(link->payload.key.buffer);
        }
    }
    appFreeUni__FPv(link);
}
#undef DESTROY_DIALOGUE_SUBTREE_ROOT
#pragma pop

// Release the heap-backed string in an integer/string map payload.
void IntStringTree_DestroyValue(void*, void* p) {
    if (p == 0) return;
    if ((int)p + 4 == 0) return;
    if (!(((IntStringPayload*)p)->text.flags >> 31)) return;
    appFreeUni__FPv(((IntStringPayload*)p)->text.buffer);
}

struct RefObject {
    virtual void unused00(); virtual void unused01(); virtual void unused02(); virtual void unused03();
    virtual void unused04(); virtual void unused05(); virtual void unused06(); virtual void unused07();
    virtual void unused08(); virtual void unused09(); virtual void unused10(); virtual void unused11();
    virtual void unused12(); virtual void unused13(); virtual void unused14(); virtual void unused15();
    virtual void unused16(); virtual void unused17(); virtual void unused18(); virtual void unused19();
    virtual void unused20(); virtual void unused21(); virtual void unused22(); virtual void unused23();
    virtual void unused24(); virtual void unused25();
    virtual void destroy(unsigned int, unsigned int);
};
struct RefObject8 {
    virtual void destroy(unsigned int);
};

struct RefHolder {
    unsigned int referenceFlags;
    unsigned int ownsObject;
    RefObject** object;
};

// Destroy the referenced actor when this holder owns it.
void CConteCaddie_DestroyOwnedActor(RefHolder* p) {
    if (p->ownsObject == 0) return;
    RefObject** q = p->object;
    if (q == 0) return;
    {
        RefObject* obj = *q;
        obj->destroy(1, 1);
    }
}

// Destroy and clear an owned polymorphic object pointer.
void CaddieOwnedObject_DestroyAndClear(void*, void* p) {
    if (p == 0) return;
    RefObject8* obj = *(RefObject8**)p;
    if (obj == 0) return;
    if (obj != 0) obj->destroy(1);
    *(void**)p = 0;
}

void CGolfDoc_Ctor(void*);
struct AllocatedObject {
    char padding[0x6DC];
    // Construct the allocated golf-document storage.
    AllocatedObject() { CGolfDoc_Ctor(this); }
};
// Allocate and construct the golf document.
void* CGolfDoc_Create() {
    return new AllocatedObject();
}

void IntPair_Set(void*, void*, void*);
void* ActorFactoryRegistry_GetInstance();
void StringMap_SetByName(void*, void*, const char*);
void CGolfCaddie_Create();
void* TaskRegistry_GetInstance();
void TaskRegistry_AppendFactory(void*, const char*);
extern unsigned char s_bpSharedStartupGuard;
extern unsigned char s_bpSecondaryStartupGuard;
extern unsigned char s_golfCaddieTypeInfo;
extern unsigned char s_actorTypeInfo;
extern volatile float s_caddieTwoPiStorage;
extern const float kFloat_6_2831855;
extern const char s_golfCaddieFactoryName[];
extern const char s_golfCaddieTaskName[];
extern const char golfCaddieStrings[];
// Register the caddie actor factory, task name, and runtime type information.
void CGolfCaddie_RegisterType() {
    s_caddieTwoPiStorage = kFloat_6_2831855;
    IntPair_Set(&s_golfCaddieTypeInfo, (char*)golfCaddieStrings + kCaddieClassName, &s_actorTypeInfo);
    void* state = ActorFactoryRegistry_GetInstance();
    StringMap_SetByName(state, (void*)CGolfCaddie_Create, s_golfCaddieFactoryName);
    state = TaskRegistry_GetInstance();
    TaskRegistry_AppendFactory(state, s_golfCaddieTaskName);
    if ((signed char)s_bpSharedStartupGuard == 0) s_bpSharedStartupGuard = 1;
    if ((signed char)s_bpSecondaryStartupGuard == 0) s_bpSecondaryStartupGuard = 1;
}

void* __nwa__FUl(unsigned int);
void* __nw__FUl(unsigned int);
void* appMallocDummy__FUl(unsigned int);
void String_ReserveCapacity(void*, unsigned int);
void String_ReplaceRange(...);
void RBTree_RebalanceAfterInsert(void*, void*);
unsigned int strlen(const char*);
char* strcpy(char*, const char*);
void* memmove(void*, const void*, unsigned int);
void __dla__FPv(void*);
void __throw(const char*, void*, void (*)(void*));
void __dt__Q23std12length_errorFv(void*);
void* __dt__Q33std6detail16char_array_guardFv(void*, int);

extern const char __vt__Q23std11logic_error[];
extern const char s_sharedCharDeleterVtable[];
extern const char __vt__Q23std12length_error[];
#pragma push
#pragma use_lmw_stmw on
// Copy an integer/string pair into a new tree node and rebalance the insertion.
// sourcePair is borrowed; heap text is copied. Return the tree-owned node, not its payload.
// parentNode is the insertion parent (or root slot); updateFirstNode updates the cached minimum.
void* IntStringTree_InsertAtPosition(void* self, void* parentNode, int insertLeft, int updateFirstNode, const void* sourcePair) {
    // Phase 1: reject count overflow before allocating any node storage.
    volatile unsigned char zero __attribute__((aligned(4)));
    volatile char extra __attribute__((aligned(4)));
    char done __attribute__((aligned(4)));
    volatile unsigned char noUnit __attribute__((aligned(4)));
    unsigned char unit __attribute__((aligned(4)));
    if (*(unsigned int*)self == 0xFFFFFFFFu) {
        const char* msg = (const char*)golfCaddieStrings + kTreeInsertLengthError;
        struct __Holder {
            char* p; char* flag;
            // Guard the exception message while its reference-counted owner is allocated.
            __Holder(char* b, char* f) { p = b; flag = f; }
            // Free the message unless ownership has been transferred to the control block.
            ~__Holder() { if (p) __dla__FPv(p); }
        };
        struct __ErrBase {
            void* vt; char* buf; void* ref;
            // Install the logic-error base table for exception construction.
            __ErrBase() { vt = (void*)__vt__Q23std11logic_error; }
            // Empty base cleanup hook; message ownership is handled separately.
            ~__ErrBase() {}
        };
        struct __Err2 : __ErrBase {
            // Allocate the exception message, attach its owner, and throw length_error.
            __Err2(volatile char* extra, char* done, const char* msg) {
                *extra = 0;
                *done = 0;
                unsigned n = strlen(msg) + 1;
                char* b = (char*)__nwa__FUl(n * 2 - n);
                buf = b;
                {
                    __Holder h(b, done);
                    void* r = __nw__FUl(0x10);
                    if (r) { ((ExceptionCharReference*)r)->strongRefs = 1; ((ExceptionCharReference*)r)->weakRefs = 1; *(void**)r = (void*)s_sharedCharDeleterVtable; ((ExceptionCharReference*)r)->buffer = b; }
                    ref = r;
                    h.p = 0;
                }
                strcpy(buf, msg);
                vt = (void*)__vt__Q23std12length_error;
                __throw((char*)golfCaddieStrings + kLengthErrorType, this, (void (*)(void*))__dt__Q23std12length_errorFv);
            }
            // Empty cleanup hook retained for the thrown exception construction path.
            ~__Err2() {}
        };
        __Err2 e(&extra, &done, msg);
    }
    struct NodeHolder {
        void* owner; void* ptr;
        // Guard an unlinked tree node until insertion takes ownership.
        NodeHolder(void* o, void* p) { owner = o; ptr = p; }
        // Free an unlinked node if payload construction fails.
        ~NodeHolder() { if (ptr) appFreeUni__FPv(ptr); }
    };
    // Phase 2: guard unlinked storage while copying inline bytes or duplicating heap text.
    zero = 0;
    void* p = appMallocDummy__FUl(0x1C);
    void* volatile t2;
    void* volatile t1;
    t1 = p;
    t2 = p;
    NodeHolder holder((char*)self + 4, p);
    struct Payload {
        unsigned int first, header, length, data;
        // Copy the integer key and narrow text, duplicating sourceIsHeap storage when needed.
        static void construct(Payload* node, unsigned int sourceFlags, unsigned int sourceKey, unsigned int sourceIsHeap, const void* sourcePair, volatile unsigned char* noUnit, unsigned char* unit) {
        try {
        node->first = sourceKey;
        if (!sourceIsHeap) {
            node->header = sourceFlags;
            node->length = *(const unsigned int*)((const char*)sourcePair + 8);
            node->data = *(const unsigned int*)((const char*)sourcePair + 0xC);
        } else {
            node->header = 0;
            node->length = 0;
            node->data = 0;
            String_ReserveCapacity(&node->header, *(const unsigned int*)((const char*)sourcePair + 8));
                *unit = *noUnit;
                ((void (*)(void*, int, int, unsigned int, unsigned int, unsigned char*))String_ReplaceRange)(&node->header, 0, 0, *(const unsigned int*)((const char*)sourcePair + 0xC),
                            *(const unsigned int*)((const char*)sourcePair + 0xC) + *(const unsigned int*)((const char*)sourcePair + 8), unit);
        }
        } catch (...) {
            // Runtime rethrow does not return; this catch has no normal exit.
            for (;;) { throw; }
        }
        }
    };
    Payload* node = (Payload*)((char*)p + kTreePayloadOffset);
    if (node) {
        unsigned int sourceFlags = *(volatile const unsigned int*)((const char*)sourcePair + 4);
        unsigned int sourceKey = *(volatile const unsigned int*)sourcePair;
        Payload::construct(node, sourceFlags, sourceKey, sourceFlags >> 31, sourcePair, &noUnit, &unit);
    }
    // Phase 3: node words [0,1,2] are left, right, parent/color; payload
    // begins at +0xC. parentNode may be the container root-link sentinel.
    // Link only after payload construction: the holder owns unlinked storage.
    // Rebalancing receives the previous count (zero identifies first insertion).
    // Publish: clear child links, store the insertion parent, then attach
    // left/right and rebalance. The cached minimum changes only when requested.
    unsigned int* alloc = (unsigned int*)holder.ptr;
    alloc[1] = 0;
    holder.ptr = 0;
    alloc[0] = 0;
    unsigned int* slot = alloc + 2;
    if (slot != 0) { try { *slot = (unsigned int)parentNode; } catch (...) { __throw(0, 0, 0); } }
    if (insertLeft != 0) *(unsigned int*)parentNode = (unsigned int)alloc; else (*(unsigned int*)&AsIntStringTreeNode8013(parentNode)->right) = (unsigned int)alloc;
    unsigned int oldSize = *(unsigned int*)self;
    *(unsigned int*)self = oldSize + 1;
    ((void (*)(void*, void*, unsigned int))RBTree_RebalanceAfterInsert)(alloc, ((CaddieStringTree*)self)->root, oldSize);
    if (updateFirstNode != 0) (*(unsigned int*)&((CaddieStringTree*)self)->firstNode) = (unsigned int)alloc;
    return alloc;
}
#pragma pop

struct StringTreeContainerWords {
    unsigned int count, rootBits;
    void* firstNode;
    void* firstNodeCache; // Legacy name: cached leftmost node, not the maximum key.
};
struct StringPairSource {
    unsigned int narrowFlags, narrowLength, narrowBufferBits;
    unsigned char valueTag;
    char reserved[3];
    unsigned int wideFlags, wideLength, wideBufferBits;
};
typedef char StringPairSource_wide_check[((int)&((StringPairSource*)0)->wideFlags == 0x10) ? 1 : -1];
void WString_ReplaceRange(void*, int, int, void*, unsigned int, unsigned char*);
void WString_ReserveCapacity(void*, unsigned int);

char* String_FormatTemporary(const char*, ...);
unsigned int ItemInventory_FindCaddieItemData(void*, void*);
void* g_sharedDoc;
void* CPetBody_Ctor(void*, const char*, int);
void CPetBody_SetPosition(void*, float*);
void CPetBody_SetNameString(void*, const char*);
void* CPetBody_SetupLighting(void*);
void* CPetBody_GetPuppet(void*);
void WPuppet_RebuildBoneController(void*);
void WPuppet_InitializeTextureCoordinateAnimation(void*);
void CGolfCaddie_ParseAnimationMetadata(void*, void*);
void WPuppet_SetRendererOption(void*, int);
void* GfxObjList_FindByName(void*, const char*);
void RenderPacket_Init(void*);
void __register_global_object(void*, void*, void*);
void CShadowManager_GetOrCreateEntry(void*, const char*, float, int, const char*, const char*);
void* g_shadowManager;
signed char s_renderPacketInitialized;
void* RenderPacket_Destroy();
extern const char s_renderPacketDtorRegistration_804F68E8[];
extern const char s_renderPacket[];
extern const float kFloat_0_GolfCaddie;
extern const float kFloat_0_09_CaddieShadow;

// Entry passed in/out: bodyAddress at +0 must be zero on entry; value at +0x10 is a borrowed
// IFF caddie record. Creation overwrites the first word with the owned CPetBody pointer.
struct CaddieBodyEntryView {
    int bodyAddress;
    char pad[0xC];
    const char* value;
};
struct FormatArg {
    int type;
    char pad[0xC];
    const char* value;
};
struct LocalString {
    unsigned int header, length, heapDataOrInlineBytes;
    // Initialize an empty inline model-command string.
    LocalString() { header = 0; length = 0; heapDataOrInlineBytes = 0; }
    // Release a heap-backed model-command string.
    ~LocalString() { if (header >> 31) appFreeUni__FPv((void*)heapDataOrInlineBytes); }
    // Borrow the current inline or heap string contents.
    const char* value() const { if ((header >> 31) == 0) return (const char*)this + 1; else return (const char*)heapDataOrInlineBytes; }
};
struct PetBodyAllocation {
    char bytes[0x1B4];
    // Construct a pet body from borrowed model-command text.
    PetBodyAllocation(const LocalString& text, int flag) {
        const char* value;
        if ((text.header >> 31) == 0) value = (const char*)&text + 1;
        else value = (const char*)text.heapDataOrInlineBytes;
        CPetBody_Ctor(this, value, flag);
    }
};

// Keep these expansions at the call site: allocator-byte lifetimes also control EH code.
// Each expansion replaces the entire model command; result is borrowed only during the copy.
#define ASSIGN_TEXT_BODY \
    if ((text.header >> 31) == 0) textLength = *(unsigned char*)&text.header & 0x7f; else textLength = text.length; \
    volatile unsigned char unused __attribute__((aligned(4))); \
    unsigned char unit __attribute__((aligned(4))) = unused; \
    ((void (*)(void*, int, unsigned int, const char*, const char*, unsigned char*))String_ReplaceRange)(&text, 0, textLength, result, result + strlen(result), &unit)
#define ASSIGN_TEXT(resultExpr) \
    unsigned int textLength; \
    const char* result = (resultExpr); \
    ASSIGN_TEXT_BODY

// Build the entry model command, create its pet body, and initialize animation and shadows.
void CGolfCaddie_CreatePetBodyForEntry(void* self, CaddieBodyEntryView* arg, FormatArg* lookup) {
    if (arg->bodyAddress != 0) return;
    LocalString text;
    // A story-pet context selects the story model format and skips ordinary animation metadata.
    if (((CGolfCaddie*)self)->storyConteCaddieOverride != 0) {
        const char* result = String_FormatTemporary(golfCaddieStrings + kStoryPetFormat, ((IFF_STRUCT::sCaddie*)arg->value)->model);
        unsigned int textLength;
        ASSIGN_TEXT_BODY;
    } else {
        void* found = 0;
        if (lookup != 0) {
            found = (void*)ItemInventory_FindCaddieItemData(AsCSharedDoc(g_sharedDoc)->itemInventory, lookup);
            if (found != 0) {
                ASSIGN_TEXT(String_FormatTemporary(golfCaddieStrings + kItemPetFormat, ((IFF_STRUCT::sCadItem*)found)->model));
            } else {
                ASSIGN_TEXT(String_FormatTemporary(golfCaddieStrings + kDefaultPetFormat, ((IFF_STRUCT::sCaddie*)arg->value)->model));
            }
        } else {
            ASSIGN_TEXT(String_FormatTemporary(golfCaddieStrings + kDefaultPetFormat, ((IFF_STRUCT::sCaddie*)arg->value)->model));
        }
    }
    // Phase 2: construct the chosen model and transfer the new body into the entry.
    void* object = new PetBodyAllocation(text, 0);
    *(void**)arg = object;
    if (object != 0) {
        float pos[3] = { kFloat_0_GolfCaddie, kFloat_0_GolfCaddie, kFloat_0_GolfCaddie };
        CPetBody_SetPosition(*(void**)arg, pos);
        CPetBody_SetNameString(*(void**)arg, golfCaddieStrings + kPetCenterName);
        CPetBody_SetupLighting(*(void**)arg);
    }
    // Phase 3: prepare puppet animation/controllers; story overrides skip metadata parsing.
    void* child = CPetBody_GetPuppet(*(void**)arg);
    if (child != 0) { WPuppet_RebuildBoneController(CPetBody_GetPuppet(*(void**)arg)); WPuppet_InitializeTextureCoordinateAnimation(CPetBody_GetPuppet(*(void**)arg)); }
    if (((CGolfCaddie*)self)->storyConteCaddieOverride == 0) CGolfCaddie_ParseAnimationMetadata(self, arg);
    WPuppet_SetRendererOption(CPetBody_GetPuppet(*(void**)arg), 2);
    // Phase 4: lazily install the shared shadow group; it is not owned by this entry.
    if (arg != 0 && *(void**)arg != 0 && GfxObjList_FindByName(g_shadowManager, golfCaddieStrings + kShadowGroupName) == 0) {
        if (Sing_805DC7B0() == 0) {
            if (s_renderPacketInitialized == 0) {
                RenderPacket_Init((void*)s_renderPacket);
                __register_global_object((void*)s_renderPacket, (void*)RenderPacket_Destroy, (void*)s_renderPacketDtorRegistration_804F68E8);
                s_renderPacketInitialized = 1;
            }
            Sing_805DC7B0() = (char*)s_renderPacket;
        }
        CShadowManager_GetOrCreateEntry(g_shadowManager, golfCaddieStrings + kShadowGroupName, kFloat_0_09_CaddieShadow, 0x10,
                    golfCaddieStrings + kSunLightName, (char*)Sing_805DC7B0() + 0x4C);
    }
}
#undef ASSIGN_TEXT
#undef ASSIGN_TEXT_BODY

#pragma push
#pragma use_lmw_stmw on
struct Pair {
    unsigned int a, b;
};
// Copy narrow/wide strings into an owned tree node and rebalance its insertion.
// first is borrowed; both strings are copied. link is an insertion parent/root slot.
void* StringWStringMap_InsertNode(StringTreeContainerWords* self, void** link, int insertLeft, int updateFirstNode,
                  const StringPairSource* first) {
    volatile unsigned char zero __attribute__((aligned(4)));
    volatile char extra __attribute__((aligned(4)));
    char done __attribute__((aligned(4)));
    volatile unsigned char noUnit0 __attribute__((aligned(4)));
    unsigned char unit0 __attribute__((aligned(4)));
    unsigned char unit1 __attribute__((aligned(4)));
    volatile unsigned char noUnit1 __attribute__((aligned(4)));
    if (self->count + 0x10000 == 0xFFFF) {
        struct __Holder {
            char* p; char* flag;
            // Guard the exception message while its reference-counted owner is allocated.
            __Holder(char* b, char* f) { p = b; flag = f; }
            // Free the message unless ownership has been transferred to the control block.
            ~__Holder() { if (p) __dla__FPv(p); }
        };
        struct __ErrBase {
            void* vt; char* buf; void* ref;
            // Install the logic-error base table for exception construction.
            __ErrBase() { vt = (void*)__vt__Q23std11logic_error; }
            // Empty base cleanup hook; message ownership is handled separately.
            ~__ErrBase() {}
        };
        struct __Err2 : __ErrBase {
            // Allocate the exception message, attach its owner, and throw length_error.
            __Err2(volatile char* extra, char* done, const char* msg0) {
                *extra = 0;
                *done = 0;
                const char* msg = msg0;
                unsigned n = strlen(msg) + 1;
                char* b = (char*)__nwa__FUl(n * 2 - n);
                buf = b;
                {
                    __Holder h(b, done);
                    void* r = __nw__FUl(0x10);
                    if (r) { ((ExceptionCharReference*)r)->strongRefs = 1; ((ExceptionCharReference*)r)->weakRefs = 1; *(void**)r = (void*)s_sharedCharDeleterVtable; ((ExceptionCharReference*)r)->buffer = b; }
                    ref = r;
                    h.p = 0;
                }
                strcpy(buf, msg);
                vt = (void*)__vt__Q23std12length_error;
                __throw((char*)golfCaddieStrings + kLengthErrorType, this, (void (*)(void*))__dt__Q23std12length_errorFv);
            }
            // Empty cleanup hook retained for the thrown exception construction path.
            ~__Err2() {}
        };
        const char* msg;
        msg = (const char*)golfCaddieStrings + kTreeInsertLengthError;
        __Err2 e(&extra, &done, msg);
    }
    struct NodeHolder {
        void* owner; void* ptr;
        // Guard an unlinked tree node until insertion takes ownership.
        NodeHolder(void* o, void* p) { owner = o; ptr = p; }
        // Free an unlinked node if payload construction fails.
        ~NodeHolder() { if (ptr) appFreeUni__FPv(ptr); }
    };
    zero = 0;
    void* volatile t2;
    void* volatile t1;
    // Guard the unlinked node while constructing the narrow key, tag, then wide value.
    void* p = appMallocDummy__FUl(0x28);
    t1 = p;
    t2 = p;
    NodeHolder holder((char*)self + 4, p);
    char* node = (char*)p + kTreePayloadOffset;
    {
            struct Narrow {
                unsigned int header, length, data;
                // Copy the narrow map key, duplicating its heap buffer if present.
                Narrow(const StringPairSource* first, volatile unsigned char* unused, unsigned char* unit) {
                    unsigned int a = first->narrowFlags;
                    if (!(a >> 31)) {
                        header = a;
                        length = first->narrowLength;
                        data = first->narrowBufferBits;
                    } else {
                        header = 0;
                        unsigned int e = first->narrowLength;
                        length = 0; data = 0;
                        String_ReserveCapacity(this, e);
                        *unit = *unused;
                        ((void (*)(void*, int, int, unsigned int, unsigned int, unsigned char*))String_ReplaceRange)(this, 0, 0, first->narrowBufferBits, first->narrowBufferBits + first->narrowLength, unit);
                    }
                }
                // Release the copied narrow key when its payload is destroyed.
                ~Narrow() { if (header >> 31) appFreeUni__FPv((void*)data); }
            };
            struct Payload {
                Narrow narrow;
                unsigned char flag; char pad[3];
                unsigned int header, length, data;
                // Construct the narrow key, then copy the value tag and wide text.
                Payload(const StringPairSource* first, volatile unsigned char* unused0, unsigned char* unit0,
                        volatile unsigned char* unused1, unsigned char* unit1) try : narrow(first, unused0, unit0) {
                    unsigned int copiedFlag;
                    unsigned int b;
                    b = first->wideFlags;
                    copiedFlag = first->valueTag;
                    assign(b, copiedFlag, this, first, unused1, unit1);
                } catch (...) { for (;;) { throw; } }
                // Copy the value tag and wide string into the payload with independent ownership.
                static void assign(unsigned int b, unsigned int copiedFlag, Payload* dest, const StringPairSource* first,
                                   volatile unsigned char* unused1, unsigned char* unit1) {
                    dest->flag = copiedFlag;
                    if (!(b >> 31)) {
                        dest->header = b;
                        dest->length = first->wideLength;
                        dest->data = first->wideBufferBits;
                    } else {
                        unsigned int req = first->wideLength;
                        dest->header = 0; dest->length = 0; dest->data = 0;
                        WString_ReserveCapacity(&dest->header, req);
                        *unit1 = *unused1;
                        WString_ReplaceRange(&dest->header, 0, 0, ((void*)first->wideBufferBits),
                            first->wideBufferBits + 2 * first->wideLength, unit1);
                    }
                }
            };
            new(node) Payload(first, &noUnit0, &unit0, &noUnit1, &unit1);
    }
    // Publish: clear child links, store the insertion parent, then attach
    // left/right and rebalance. The cached minimum changes only when requested.
    unsigned int* alloc = (unsigned int*)holder.ptr;
    alloc[1] = 0;
    holder.ptr = 0;
    alloc[0] = 0;
    unsigned int* slot = alloc + 2;
    if (slot != 0) { try { *slot = (unsigned int)link; } catch (...) { __throw(0, 0, 0); } }
    // Ownership now belongs to the container; rebalance only after both strings exist.
    if (insertLeft) *link = alloc; else link[1] = alloc;
    unsigned int oldSize = self->count;
    self->count = oldSize + 1;
    ((void (*)(void*, void*, unsigned int))RBTree_RebalanceAfterInsert)(alloc, (void*)self->rootBits, oldSize);
    if (updateFirstNode) ((StringTreeContainerWords*)self)->firstNodeCache = alloc;
    return alloc;
}
#pragma pop

#pragma push
#pragma use_lmw_stmw on
// Resize wide-string storage, preserving its contents and selecting inline or heap storage.
// requested is a character count, not bytes. Existing text is never truncated; a failed
// shrink allocation leaves storage unchanged, while a failed growth allocation propagates.
void WString_ReserveCapacity(void* object, unsigned int requested) {
    if (requested > kMaxWideStringLength) {
        struct __Holder {
            char* p; char* flag;
            // Guard the exception message while its reference-counted owner is allocated.
            __Holder(char* b, char* f) { p = b; flag = f; }
            // Free the message unless ownership has been transferred to the control block.
            ~__Holder() { if (p) __dla__FPv(p); }
        };
        struct __ErrBase {
            void* vt; char* buf; void* ref;
            // Allocate and share an exception message before throwing its derived error.
            __ErrBase(const char* msg0) {
                char done __attribute__((aligned(4))) = 0;
                vt = (void*)__vt__Q23std11logic_error;
                const char* msg = msg0;
                unsigned n = strlen(msg) + 1;
                char* b = (char*)__nwa__FUl(n * 2 - n);
                buf = b;
                {
                    __Holder h(b, &done);
                    void* r = __nw__FUl(0x10);
                    if (r) { ((ExceptionCharReference*)r)->strongRefs = 1; ((ExceptionCharReference*)r)->weakRefs = 1; *(void**)r = (void*)s_sharedCharDeleterVtable; ((ExceptionCharReference*)r)->buffer = b; }
                    ref = r;
                    h.p = 0;
                }
                strcpy(buf, msg);
            }
            // Empty base cleanup hook; message ownership is handled separately.
            ~__ErrBase() {}
        };
        struct __Err2 : __ErrBase {
            // Install the length-error table and transfer the exception to the runtime.
            __Err2(const char* msg) : __ErrBase(msg) { vt = (void*)__vt__Q23std12length_error; __throw((char*)golfCaddieStrings + kLengthErrorType, this, (void (*)(void*))__dt__Q23std12length_errorFv); }
            // Empty cleanup hook retained for the thrown exception construction path.
            ~__Err2() {}
        };
        __Err2 e(golfCaddieStrings + kStringReserveLengthError);
    }
    struct WordHdr { CW_BITS2( unsigned int heap : 1, unsigned int cap : 31) };
    struct ByteHdr { CW_BITS2( unsigned char heap : 1, unsigned char len : 7) };
    WordHdr* w = (WordHdr*)object;
    ByteHdr* b = (ByteHdr*)object;
    unsigned int isHeap = w->heap;
    unsigned int length;
    unsigned int capacity;
    if (!isHeap) {
        length = b->len;
        capacity = kWideInlineCapacity;
    } else {
        length = ((CaddieStringStorage*)object)->length;
        capacity = w->cap;
    }
    if (requested < length) requested = length;
    // Phase 2: choose inline storage or an eight-character-aligned allocation including NUL.
    unsigned int newCapacity = kWideInlineCapacity;
    if (requested + 1 > kWideInlineCapacity) newCapacity = (requested + 8) & ~7u;
    if (newCapacity == capacity) return;

    char* dest;
    char* source;
    unsigned int isHeapNew;
    if (newCapacity == kWideInlineCapacity) {
        source = (*(char**)&((CaddieStringStorage*)object)->buffer);
        dest = (char*)object + 2;
        isHeapNew = 0;
    } else {
        if (newCapacity > capacity) {
            dest = (char*)appMallocDummy__FUl(newCapacity * 2);
        } else {
            try {
                dest = (char*)appMallocDummy__FUl(newCapacity * 2);
            } catch (...) {
                return;
            }
        }
        isHeapNew = 1;
        if (isHeap) source = (*(char**)&((CaddieStringStorage*)object)->buffer);
        else source = (char*)object + 2;
    }
    // Phase 3: copy text and terminator before releasing the old heap allocation.
    unsigned int bytes = (((length >> 30) & 1) + (length * 2)) & ~1u;
    memmove(dest, source, bytes);
    *(unsigned short*)(dest + bytes) = 0;
    if (isHeap) appFreeUni__FPv(source);
    w->heap = isHeapNew;
    if (!isHeapNew) {
        b->len = length;
    } else {
        (*(char**)&((CaddieStringStorage*)object)->buffer) = dest;
        ((CaddieStringStorage*)object)->length = length;
        w->cap = newCapacity;
    }
}
#pragma pop

} // extern "C"

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU80462124", _ctors_u80130F58_0, CGolfCaddie_RegisterType);
#pragma pop

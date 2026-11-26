// Effect box interpreter: the interpreter's list of live effects, the effect box update, draw
// and height-crossing checks, and the resource-name cache with its tree maps.

#include <decomp/static_init.h>
#include <decomp/ct_member.h>
#include "MSL/error_message.h"
// Effect-box resource plumbing: the per-frame update that advances live sprays
// and sequences, resource filename parsing, the red-black trees the live effects
// are stored in, and the interpreter's shutdown. No golf-shot rules are defined
// here.
namespace Port2ExceptionRuntime {
extern "C" {
void __throw(const char*, void*, void*) __attribute__((noreturn, nothrow));
void __dt__Q23std12length_errorFv();
void __dt__Q23std12out_of_rangeFv();
}
}
#include "classes/FxInterpreter.h"
#include "classes/CFxSpray.h"
#include "classes/CFxBox.h"

#include "sdk_protos.h"
#include "classes/CFxSequence.h"
#include "classes/CFx.h"

extern "C" {

// Default effect-box lifecycle hook; subclasses may override it.
void CFxBox::vfunc_1() {}

// Links occupy +0/+4/+8, then key at +12 and effect at +16. Parent bit zero
// is tree color: mask it before traversing. End is the embedded root/header
// address, not null. Different trees below own nodes, strings or effects separately.
struct RBNode {
    RBNode* left;
    RBNode* right;
    RBNode* parent;
    unsigned int key;
    void* effect;
};
// Pair trees put first at +8; sprite-cache trees have an extra header word.
struct ListAnchor {
    unsigned int count;
    unsigned int root;
    void* firstPair;
    void* firstSprite;
};
enum {
    kMaximumTreeEntries = 0xFFFFFFFFu,
    kFilenameCapacity = 64,
    kParseSpaceDelimited = 0,
    kParseQuoted = 1,
    kResourceSequence = 1,
    kResourceSpray = 2,
    kResourceUnknown = 0,
    kInterpreterLinksOffset = (int)&((FxInterpreter*)0)->sentinel
};
struct StringLike {
    void* vtable;
    int refs;
    int capacity;
    char* data;
};
struct FixedString {
    char* vtable;
    char storage[1];
};
struct Cursor {
    void* first;
    void* second;
};
extern void* __vt__Q23std11logic_error[4];
extern void* s_sharedCharDeleterVtable[5];
extern void* __vt__Q23std12length_error[4];
extern char hudBitmapVariantStrings[0xF0];
extern void* __nwa__FUl(unsigned int);
extern void* appMallocDummy__FUl(unsigned int);
extern void* __nw__FUl(unsigned int);
extern void __dla__FPv(void*);
extern void RBTree_RebalanceAfterInsert(void*, void*);
extern void __dt__Q23std12length_errorFv(void*);
extern void String_ReserveCapacity(void*, void*);
struct __CopyTag_8010B8EC {};
extern void String_ReplaceRange(void*, int, int, const char*, const char*, __CopyTag_8010B8EC);
extern void appFreeUni__FPv(void*);
extern void __dt__Q33std6detail16char_array_guardFv(void*, int);

#pragma use_lmw_stmw on
#pragma scheduling on
#pragma opt_common_subs off
extern "C" void __dt__Q23std9exceptionFv(void*, int);
namespace std {
struct exception {
    void* vtable;
    // Initialize the standard logic-error exception identity.
    exception() : vtable((void*)__vt__Q23std11logic_error) {}
    ~exception();
};
}
// Shared RTTI: std::tr1::detail::shared_ptr_deleter<char, std::detail::default_delete<char[]> >.
// Use the common 16-byte ABI view in MSL/error_message.h, including its member names.
typedef ::std::detail::__message_control __ErrorCount_8010B8EC;
struct __Holder_8010B8EC {
    void* p;
    void* flag;
    // Guard the allocated text until it is owned by its reference block.
    __Holder_8010B8EC(void* b, void* f) : p(b), flag(f) {}
    // Delete text if ownership has not transferred from the guard.
    ~__Holder_8010B8EC() {
        if (p)
            __dla__FPv(p);
    }
};
struct __AllocatorByte_8010B8EC {
    volatile char byte;
    char padding[3];
    // Initialize the allocator tag used by string and error construction.
    __AllocatorByte_8010B8EC() : byte(0) {}
};
namespace std {
struct logic_error_u8010B244 : exception {
    char* buf;
    __ErrorCount_8010B8EC* ref;
    // Allocate and share a NUL-terminated copy of diagnostic text.
    logic_error_u8010B244(const char* msg) {
        __AllocatorByte_8010B8EC allocator;
        volatile char done = 0;
        const char* m = msg;
        unsigned n = strlen(m) + 1;
        char* buffer = (char*)__nwa__FUl(n * 2 - n);
        buf = buffer;
        {
            __Holder_8010B8EC h(buffer, (void*)&done);
            __ErrorCount_8010B8EC* r = (__ErrorCount_8010B8EC*)__nw__FUl(16);
            if (r) {
                r->strong = 1;
                r->weak = 1;
                r->table = (void*)s_sharedCharDeleterVtable;
                r->pointer = buffer;
            }
            ref = r;
            h.p = 0;
        }
        strcpy(buf, m);
    }
    // Release the copied diagnostic through the runtime helper.
    ~logic_error_u8010B244() {
        __dt__Q33std6detail16char_array_guardFv(this, -1);
    }
};
}
struct HudDiagnosticStrings {
    char precedingStrings[0x23];
    char lengthErrorType[55];
};
struct __LengthError_8010B8EC : std::logic_error_u8010B244 {
    // Construct and throw the sprite-cache length-error identity.
    __LengthError_8010B8EC(const char* msg) : std::logic_error_u8010B244(msg) {
        this->vtable = (void*)__vt__Q23std12length_error;
        __throw(((HudDiagnosticStrings*)hudBitmapVariantStrings)->lengthErrorType, this,
                (void*)__dt__Q23std12length_errorFv);
    }
};
namespace std {
struct length_error : logic_error_u8010B244 {
    // Construct the standard length-error identity from copied text.
    length_error(const char* msg) : logic_error_u8010B244(msg) {
        vtable = __vt__Q23std12length_error;
    }
    ~length_error();
    DECOMP_CT_MEMBER(length_error)
};
}
#pragma use_lmw_stmw on
#pragma scheduling on
// Construct in caller-supplied storage without allocating.
inline void* operator new(size_t, void* p) {
    return p;
}
struct NodeFreeGuard {
    void* c;
    void* node;
    // Free an uncommitted sprite-cache node during unwinding.
    ~NodeFreeGuard() {
        if (node)
            appFreeUni__FPv(node);
    }
};
struct RBParentLink {
    RBNode* value;
    // Copy the tree parent link while preserving the construction boundary.
    RBParentLink(RBNode* v) {
        try {
            value = v;
        } catch (...) {
            throw;
        }
    }
};
struct __ValueCopy_8010B8EC {
    unsigned flags;
    unsigned length;
    unsigned data;
    unsigned a;
    unsigned b;
    // Copy a cache string and its two associated words with heap text duplication.
    __ValueCopy_8010B8EC(const __ValueCopy_8010B8EC& src) {
        try {
            if (!(src.flags >> 31)) {
                flags = src.flags;
                unsigned ptr;
                unsigned len;
                len = src.length;
                ptr = src.data;
                length = len;
                data = ptr;
            } else {
                flags = 0;
                length = 0;
                data = 0;
                String_ReserveCapacity(this, (void*)src.length);
                __CopyTag_8010B8EC tag;
                String_ReplaceRange(this, 0, 0, (const char*)src.data, (const char*)src.data + src.length, tag);
            }
            unsigned second;
            unsigned first;
            first = src.a;
            second = src.b;
            a = first;
            b = second;
        } catch (...) {
            throw;
        }
    }
};
// Copy a sprite-cache value into a new red-black node and update the first iterator.
void* AnimatedSpriteCache_InsertNode(ListAnchor* owner, RBNode* position, int insertBefore, int updateFirst,
                                     const Cursor* value) {
    if (owner->count == kMaximumTreeEntries) {
        do {
            union {
                double align;
                unsigned char bytes[sizeof(std::length_error)];
            } port2Storage;
            std::length_error* port2Error = (std::length_error*)port2Storage.bytes;
            port2Error->__ct((char*)hudBitmapVariantStrings + 9);
            Port2ExceptionRuntime::__throw("!std::exception!!std::logic_error!!std::length_error!!", port2Error,
                                           (void*)Port2ExceptionRuntime::__dt__Q23std12length_errorFv);
        } while (0);
    }
    void* volatile constructed;
    void* volatile allocated;
    __AllocatorByte_8010B8EC nodeAllocator;
    void* allocation = appMallocDummy__FUl(32);
    allocated = allocation;
    constructed = allocation;
    NodeFreeGuard guard;
    guard.c = &owner->root;
    guard.node = allocation;
    new ((char*)allocation + 12) __ValueCopy_8010B8EC(*(const __ValueCopy_8010B8EC*)value);
    RBNode* result = (RBNode*)guard.node;
    result->right = 0;
    guard.node = 0;
    result->left = 0;
    new (&result->parent) RBParentLink(position);
    if (insertBefore)
        position->left = result;
    else
        position->right = result;
    ++owner->count;
    RBTree_RebalanceAfterInsert(result, (void*)owner->root);
    if (updateFirst)
        owner->firstSprite = result;
    return result;
}
#pragma use_lmw_stmw reset
#pragma scheduling reset

#pragma use_lmw_stmw on
#pragma scheduling on
struct PairNodeFreeGuard {
    void* c;
    void* node;
    // Free an uncommitted pair-tree node during unwinding.
    ~PairNodeFreeGuard() {
        if (node)
            appFreeUni__FPv(node);
    }
};
struct __PairCopy_8010BEF8 {
    void* a;
    void* b;
    // Copy the two pointer words of a non-owning tree payload.
    __PairCopy_8010BEF8(const __PairCopy_8010BEF8& src) {
        try {
            a = src.a;
            b = src.b;
        } catch (...) {
            throw;
        }
    }
};
// Insert an ID/object pair and rebalance the effect tree.
void* FxBoxTree_InsertNode(ListAnchor* owner, RBNode* position, int insertBefore, int updateFirst,
                           const Cursor* value) {
    if (owner->count == kMaximumTreeEntries) {
        do {
            union {
                double align;
                unsigned char bytes[sizeof(std::length_error)];
            } port2Storage;
            std::length_error* port2Error = (std::length_error*)port2Storage.bytes;
            port2Error->__ct((char*)hudBitmapVariantStrings + 9);
            Port2ExceptionRuntime::__throw("!std::exception!!std::logic_error!!std::length_error!!", port2Error,
                                           (void*)Port2ExceptionRuntime::__dt__Q23std12length_errorFv);
        } while (0);
    }
    void* volatile constructed;
    void* volatile allocated;
    __AllocatorByte_8010B8EC nodeAllocator;
    void* allocation = appMallocDummy__FUl(20);
    allocated = allocation;
    constructed = allocation;
    PairNodeFreeGuard guard;
    guard.c = &owner->root;
    guard.node = allocation;
    new ((char*)allocation + 12) __PairCopy_8010BEF8(*(const __PairCopy_8010BEF8*)value);
    RBNode* result = (RBNode*)allocation;
    result->right = 0;
    guard.node = 0;
    result->left = 0;
    new (&result->parent) RBParentLink(position);
    if (insertBefore)
        position->left = result;
    else
        position->right = result;
    ++owner->count;
    RBTree_RebalanceAfterInsert(result, (void*)owner->root);
    if (updateFirst)
        owner->firstPair = result;
    return result;
}
#pragma use_lmw_stmw reset
#pragma scheduling reset

#pragma opt_common_subs reset
extern float kFloat_6_2831855;
extern float s_twoPi_FxBox;

// Initialize this effect-box unit's mutable two-pi constant.
void FxBox_InitializeUnitConstants() {
    s_twoPi_FxBox = kFloat_6_2831855;
}

void appFreeUni__FPv(void*);
void __dl__FPv(void*);

struct OwnedEffectBuffer {
    void* vtable;
    void* allocation;
};

// Free the owned buffer and optionally delete its wrapper.
void* dtor_8010C34C(void* self, int deleting) {
    if (self != 0) {
        void* value = ((OwnedEffectBuffer*)self)->allocation;
        if (value != 0)
            appFreeUni__FPv(value);
        if (deleting > 0)
            __dl__FPv(self);
    }
    return self;
}

struct InitObj {
    unsigned int vtable;
    unsigned int entryCount;
    unsigned int next;
    unsigned int prev;
    unsigned int state;
};
extern InitObj s_fxInterpreter;
extern unsigned char s_fxInterpreterInitialized;
extern const char __vt__13FxInterpreter[];
void* FxInterpreter_dtor(void*, int);
extern char s_fxInterpreterDtorRegistration[];

// Lazily initialize the interpreter list and register its shutdown destructor.
InitObj* FxInterpreter_GetInstance() {
    if ((signed char)s_fxInterpreterInitialized == 0) {
        s_fxInterpreter.vtable = (unsigned int)__vt__13FxInterpreter;
        s_fxInterpreter.entryCount = 0;
        s_fxInterpreter.prev = (unsigned int)((char*)&s_fxInterpreter + kInterpreterLinksOffset);
        s_fxInterpreter.next = (unsigned int)((char*)&s_fxInterpreter + kInterpreterLinksOffset);
        s_fxInterpreter.state = 0;
        __register_global_object(&s_fxInterpreter, (void*)FxInterpreter_dtor, s_fxInterpreterDtorRegistration);
        s_fxInterpreterInitialized = 1;
    }
    return &s_fxInterpreter;
}

// Reset draw-pass counters and draw particles/bounds for every registered spray.
void CFxBox::DrawSprays(void* configuration) {
    RBNode* sprayNode = (RBNode*)sprayTreeFirst;
    RBNode* endNode = (RBNode*)&sprayTreeRoot;
    // Reset both draw-pass counters before visiting sprays; their individual units are unresolved.
    (*(unsigned int*)&unk_11C) = 0;
    (*(unsigned int*)&unk_120) = 0;
    // Draw in key order with borrowed configuration. Successor is leftmost in
    // the right subtree, otherwise the first ancestor reached from its left.
    while (sprayNode != endNode) {
        ((CFxSpray*)(sprayNode->effect))->DrawParticlesAndBounds(configuration);
        void* child = sprayNode->right;
        if (child != 0) {
            RBNode* leaf = (RBNode*)child;
            while (leaf->left != 0)
                leaf = leaf->left;
            sprayNode = leaf;
        } else {
            while (sprayNode != ((RBNode*)((unsigned int)sprayNode->parent & ~1u))->left) {
                RBNode* parent = (RBNode*)((unsigned int)sprayNode->parent & ~1u);
                sprayNode = parent;
            }
            sprayNode = (RBNode*)((unsigned int)sprayNode->parent & ~1u);
        }
    }
}

struct CachedResourceName {
    int flags;
    unsigned int length;
    void* value;
};

struct FxBoxTreeNode {
    FxBoxTreeNode* left;
    FxBoxTreeNode* right;
    FxBoxTreeNode* parent;
    char keyStorage[4];
    void* owner;
    void* payload;
};
// The box's virtual interface, of which the update loop calls only the
// effect-finished notification. The notification name is inferred from that call.
struct FxBoxNotifier {
    virtual void Slot0();
    virtual void Slot1();
    virtual void Slot2();
    virtual void Slot3();
    virtual void Slot4();
    virtual void Slot5();
    virtual void NotifyEffectFinished(void* effect);
};
struct FxBoxEffectOwner {
    virtual void dispose(int);
};
extern float kFloat_1_5_FxBox;
void RBTree_RebalanceAfterErase(void*, void*);

// The next node in key order. The parent link's low bit is the red-black
// colour, so it is masked off before the parent is used.
static inline void FxBox_NextNode(FxBoxTreeNode*& node) {
    FxBoxTreeNode* next = node->right;
    if (next != 0) {
        while (next->left != 0)
            next = next->left;
        node = next;
    } else {
        while (node != (next = (FxBoxTreeNode*)((unsigned int)node->parent & ~1u))->left) {
            node = next;
        }
        node = next;
    }
}

// A cursor into one of the effect trees.
struct FxBoxIterator {
    FxBoxTreeNode* node;
    FxBoxIterator(FxBoxTreeNode* p) : node(p) {}
    FxBoxIterator(const FxBoxIterator& p) : node(p.node) {}

    // Step to the successor and return this cursor.
    FxBoxIterator& operator++() {
        FxBox_NextNode(node);
        return *this;
    }
    // Step to the successor and return the cursor before the step.
    FxBoxIterator operator++(int) {
        FxBoxIterator old = *this;
        ++*this;
        return old;
    }
};

// Advance every live spray and sequence, drop the finished ones and report a
// sequence that has expired.
//
// A paused box does nothing. When clamping is on, the step is capped before it
// is scaled so a long frame cannot advance an effect too far; the excess is
// dropped rather than made up. A finished spray is unlinked, its owner is
// notified, it is detached from its sequence and deleted. A sequence is erased
// when its update returns 0; an expired sequence returns 0 from the whole pass
// and leaves the rest for the next one. Otherwise the pass returns 1.
#pragma inline_depth(3)
int CFxBox::UpdateEffects(float delta) {
    if (pause)
        return 1;
    if (clampSimulationStep && delta > kFloat_1_5_FxBox * nominalStepMilliseconds)
        delta = kFloat_1_5_FxBox * nominalStepMilliseconds;

    FxBoxIterator end((FxBoxTreeNode*)&sprayTreeRoot);
    FxBoxIterator it((FxBoxTreeNode*)sprayTreeFirst);
    while (it.node != end.node) {
        if (!it.node->owner) {
            FxBoxIterator erased = it++;
            if (erased.node == (FxBoxTreeNode*)sprayTreeFirst) {
                ++erased;
                sprayTreeFirst = erased.node;
            }
            RBTree_RebalanceAfterErase(erased.node, (void*)sprayTreeRoot);
            appFreeUni__FPv(erased.node);
            --sprayCount;
        } else if (!((CFxSpray*)(it.node->owner))->Update(delta * timeScale)) {
            void* owner = it.node->owner;
            FxBoxIterator erased = it++;
            if (erased.node == (FxBoxTreeNode*)sprayTreeFirst) {
                ++erased;
                sprayTreeFirst = erased.node;
            }
            RBTree_RebalanceAfterErase(erased.node, (void*)sprayTreeRoot);
            appFreeUni__FPv(erased.node);
            --sprayCount;
            ((FxBoxNotifier*)this)->NotifyEffectFinished(owner);
            // The finished spray belongs to a sequence until it is detached.
            if (*(void**)&((CFxSpray*)owner)->parentSequence)
                ((CFxSequence*)(*(void**)&((CFxSpray*)owner)->parentSequence))->UnregisterSpray((unsigned int)owner);
            if (owner)
                ((FxBoxEffectOwner*)owner)->dispose(1);
        } else {
            ++it;
        }
    }

    FxBoxIterator sequence((FxBoxTreeNode*)sequenceTreeFirst);
    FxBoxIterator sequenceEnd((FxBoxTreeNode*)&sequenceTreeRoot);
    while (sequence.node != sequenceEnd.node) {
        if (!sequence.node->owner) {
            FxBoxIterator erased = sequence++;
            if (erased.node == (FxBoxTreeNode*)sequenceTreeFirst) {
                ++erased;
                sequenceTreeFirst = erased.node;
            }
            RBTree_RebalanceAfterErase(erased.node, (void*)sequenceTreeRoot);
            appFreeUni__FPv(erased.node);
            --sequenceCount;
        } else {
            unsigned char expired;
            if (!((CFxSequence*)(sequence.node->owner))->UpdateTimeline(delta * timeScale, &expired)) {
                EraseSequenceAtIterator((FxRegistryNode**)&sequence, 0);
            } else {
                ++sequence;
            }
            if (expired)
                return 0;
        }
    }
    return 1;
}
#pragma inline_depth reset

// Free a resource-name buffer only when its narrow string uses heap storage.
void FxBoxResourceMap_DestroyStringStorage(void*, CachedResourceName* cachedName) {
    // The null test appears twice only because two inlined copies were emitted by the original compiler.
    if (cachedName == 0)
        return;
    if (cachedName == 0)
        return;
    if (((unsigned int)cachedName->flags >> 31) != 0)
        appFreeUni__FPv(cachedName->value);
}

struct InterpreterListNode {
    InterpreterListNode* prev;
    InterpreterListNode* next;
};
extern unsigned int g_fx;
#pragma opt_common_subs off
#pragma scheduling on
// Unregister interpreter-owned effects from the live manager and free all list nodes.
void* FxInterpreter_dtor(void* self, int deleting) {
    if (self) {
        unsigned int* list = (unsigned int*)self;
        (*(unsigned int*)&((FxInterpreter*)list)->vtbl) = (unsigned int)__vt__13FxInterpreter;
        // Phase 1: unregister payloads through g_fx while preserving links. Manager
        // removal handles effects; this loop does not delete their pointers directly.
        // If g_fx is absent, only the list nodes are subsequently freed.
        if (((FxInterpreter*)list)->entryCount != 0) {
            InterpreterListNode* sentinel = (InterpreterListNode*)&((FxInterpreter*)list)->sentinel;
            InterpreterListNode* node = sentinel->next;
            while (node != sentinel) {
                if (g_fx != 0) {
                    int type = ((FxInterpreterEntry*)node)->kind;
                    if (type == kResourceSpray)
                        ((CFx*)g_fx)->RemoveSpray((unsigned int)(((FxInterpreterEntry*)node)->value));
                    else if (type == kResourceSequence)
                        ((CFx*)g_fx)->RemoveSequence((unsigned int)(((FxInterpreterEntry*)node)->value), 1);
                }
                node = node->next;
            }
            // Phase 2: detach the range, then free nodes and decrement entryCount.
            InterpreterListNode* begin;
            InterpreterListNode* current;
            current = ((InterpreterListNode*)&((FxInterpreter*)list)->sentinel)->next;
            begin = (InterpreterListNode*)&((FxInterpreter*)list)->sentinel;
            if (current != begin) {
                InterpreterListNode* last = begin->prev;
                current->prev->next = last->next;
                last->next->prev = current->prev;
                while (current != begin) {
                    InterpreterListNode* nxt = current->next;
                    InterpreterListNode* dead = current;
                    current = nxt;
                    appFreeUni__FPv(dead);
                    --((FxInterpreter*)list)->entryCount;
                }
            }
        }
        // The four identical address tests below are an always-true guard (the member address is never null),
        // duplicated by the original inlining; ignore them.
        // Phase 3: container destruction repeats node-only cleanup if any remain.
        // The embedded sentinel is never freed separately.
        if (&((FxInterpreter*)list)->entryCount && &((FxInterpreter*)list)->entryCount &&
            &((FxInterpreter*)list)->entryCount && &((FxInterpreter*)list)->entryCount) {
            InterpreterListNode* sentinel = (InterpreterListNode*)&((FxInterpreter*)list)->sentinel;
            InterpreterListNode* begin = sentinel;
            InterpreterListNode* current = sentinel->next;
            if (current != begin) {
                InterpreterListNode* last = begin->prev;
                current->prev->next = last->next;
                last->next->prev = current->prev;
                while (current != begin) {
                    InterpreterListNode* nxt = current->next;
                    InterpreterListNode* dead = current;
                    current = nxt;
                    appFreeUni__FPv(dead);
                    --list[1];
                }
            }
        }
        if (deleting > 0)
            __dl__FPv(self);
    }
    return self;
}
#pragma scheduling reset
#pragma opt_common_subs reset

#pragma use_lmw_stmw on
#pragma scheduling on
// Recursively free sequence-tree nodes without destroying their referenced effects.
// Postorder cleanup has its first levels expanded inline: recurse left/right,
// free node, retain referenced effect. No payload destructor is called.
void FxSequenceTree_FreeSubtree(void* self, void** treeNode) {
    // In plain C this is just: if (n->left) free(n->left); if (n->right) free(n->right); free(n);
    // applied recursively (node[0] = left, node[1] = right). The four nested levels below are the
    // compiler-unrolled form of that recursion; skip to the end of the function.
    // Phases: free left subtree, free right subtree, release this node.
    // The expanded first levels preserve that postorder without deleting payloads.
    void** first = (void**)treeNode[0];
    if (first != 0) {
        void** node = (void**)first[0];
        if (node != 0) {
            void** child = (void**)node[0];
            if (child != 0) {
                if (child[0] != 0)
                    FxSequenceTree_FreeSubtree(self, (void**)child[0]);
                if (child[1] != 0)
                    FxSequenceTree_FreeSubtree(self, (void**)child[1]);
                appFreeUni__FPv(child);
            }
            child = (void**)node[1];
            if (child != 0) {
                if (child[0] != 0)
                    FxSequenceTree_FreeSubtree(self, (void**)child[0]);
                if (child[1] != 0)
                    FxSequenceTree_FreeSubtree(self, (void**)child[1]);
                appFreeUni__FPv(child);
            }
            appFreeUni__FPv(node);
        }
        node = (void**)first[1];
        if (node != 0) {
            void** child = (void**)node[0];
            if (child != 0) {
                if (child[0] != 0)
                    FxSequenceTree_FreeSubtree(self, (void**)child[0]);
                if (child[1] != 0)
                    FxSequenceTree_FreeSubtree(self, (void**)child[1]);
                appFreeUni__FPv(child);
            }
            child = (void**)node[1];
            if (child != 0) {
                if (child[0] != 0)
                    FxSequenceTree_FreeSubtree(self, (void**)child[0]);
                if (child[1] != 0)
                    FxSequenceTree_FreeSubtree(self, (void**)child[1]);
                appFreeUni__FPv(child);
            }
            appFreeUni__FPv(node);
        }
        appFreeUni__FPv(first);
    }
    void** second = (void**)treeNode[1];
    if (second != 0) {
        void** node = (void**)second[0];
        if (node != 0) {
            void** child = (void**)node[0];
            if (child != 0) {
                if (child[0] != 0)
                    FxSequenceTree_FreeSubtree(self, (void**)child[0]);
                if (child[1] != 0)
                    FxSequenceTree_FreeSubtree(self, (void**)child[1]);
                appFreeUni__FPv(child);
            }
            child = (void**)node[1];
            if (child != 0) {
                if (child[0] != 0)
                    FxSequenceTree_FreeSubtree(self, (void**)child[0]);
                if (child[1] != 0)
                    FxSequenceTree_FreeSubtree(self, (void**)child[1]);
                appFreeUni__FPv(child);
            }
            appFreeUni__FPv(node);
        }
        node = (void**)second[1];
        if (node != 0) {
            void** child = (void**)node[0];
            if (child != 0) {
                if (child[0] != 0)
                    FxSequenceTree_FreeSubtree(self, (void**)child[0]);
                if (child[1] != 0)
                    FxSequenceTree_FreeSubtree(self, (void**)child[1]);
                appFreeUni__FPv(child);
            }
            child = (void**)node[1];
            if (child != 0) {
                if (child[0] != 0)
                    FxSequenceTree_FreeSubtree(self, (void**)child[0]);
                if (child[1] != 0)
                    FxSequenceTree_FreeSubtree(self, (void**)child[1]);
                appFreeUni__FPv(child);
            }
            appFreeUni__FPv(node);
        }
        appFreeUni__FPv(second);
    }
    appFreeUni__FPv(treeNode);
}
#pragma use_lmw_stmw reset
#pragma scheduling reset

struct BBNode {
    BBNode* left;
    BBNode* right;
    char parentStorage[4];
    int flags;
    char nameLength[4];
    void* value;
};
#pragma inline_depth(3)
#pragma use_lmw_stmw on
#pragma scheduling on
// Free each resource-cache name before releasing its tree node.
void FxBoxResourceMap_DestroySubtree(void* self, BBNode* node) {
    if (node->left)
        FxBoxResourceMap_DestroySubtree(self, node->left);
    if (node->right)
        FxBoxResourceMap_DestroySubtree(self, node->right);
    FxBoxResourceMap_DestroyStringStorage(self, (CachedResourceName*)&node->flags);
    appFreeUni__FPv(node);
}
#pragma inline_depth reset
#pragma use_lmw_stmw reset
#pragma scheduling reset

extern const char fxConditionCommandTokens[];
struct EffectFilenameExtensions {
    char sequence[4];
    char spray[4];
};
extern const char kEffectFilenameExtensions[];
void* String_ParseSpaceDelimitedToken(void*, const char*);
void* String_ParseQuotedToken(void*, const char*);
char* strrchr(const char*, int);
int String_CompareCString(const char*, const char*);
int EffectCommand_ParseResourceFilename(void*, void*, const char*, int, int*);

// Parse a filename token and accept only supported sequence/spray extensions.
// parseCursor points to a mutable script cursor. outputFilename is WRITABLE
// caller storage despite the legacy const signature: both token parsers write it.
// Writes are unbounded; reserve room for the complete token plus NUL. The 64-byte
// check occurs after parsing. Cursor advances even when extension checks fail.
// resourceKind is assigned only after extension checks; return 1 supported, 0 rejected.
// Ownership stays with caller.
int EffectCommand_ParseResourceFilename(void* unused, void* parseCursor, const char* outputFilename, int tokenMode,
                                        int* resourceKind) {
    (void)unused;
    switch (tokenMode) {
    case kParseSpaceDelimited: {
        char** cursorSlot = (char**)parseCursor;
        *cursorSlot =
            (char*)String_ParseSpaceDelimitedToken(*cursorSlot + strlen(fxConditionCommandTokens), outputFilename);
        break;
    }
    case kParseQuoted: {
        char** cursorSlot = (char**)parseCursor;
        *cursorSlot = (char*)String_ParseQuotedToken(*cursorSlot + strlen(fxConditionCommandTokens), outputFilename);
        break;
    }
    default:
        return 0;
    }
    int filenameLength = (int)strlen(outputFilename);
    if (filenameLength == 0 || filenameLength >= kFilenameCapacity)
        return 0;
    char* extension = strrchr(outputFilename, '.');
    if (extension == 0 || strlen(extension) < 4)
        return 0;
    if (String_CompareCString(extension + 1, ((const EffectFilenameExtensions*)kEffectFilenameExtensions)->sequence) ==
        0) {
        *resourceKind = kResourceSequence;
    } else if (String_CompareCString(extension + 1,
                                     ((const EffectFilenameExtensions*)kEffectFilenameExtensions)->spray) == 0) {
        *resourceKind = kResourceSpray;
    } else {
        *resourceKind = kResourceUnknown;
        return 0;
    }
    return 1;
}

extern float kFxBoxPositiveBoundaryTolerance;
extern float kFxBoxNegativeBoundaryTolerance;
// Test strict positive/negative floating-point tolerances used by box boundary crossing.
static inline int box_near(float lhs, float rhs) {
    float difference = lhs - rhs;
    int withinTolerance = 0;
    if (difference < kFxBoxPositiveBoundaryTolerance && difference > kFxBoxNegativeBoundaryTolerance)
        withinTolerance = 1;
    return withinTolerance;
}
// Whether a height crossing lies between previous and current at target.x, or
// the next step is close enough to it. Only previous.y, current.y and target.x
// are read; target is not advanced.
int CFxBox::TestHeightCrossing(float maximumStep, Vec2* previous, Vec2* current, Vec2* target) {
    if (!box_near(current->y, target->x)) {
        if (current->y < target->x) {
            if (box_near(previous->y, target->x) || previous->y < target->x) {
                int result = 1;
                if (!box_near(current->y + maximumStep, target->x)) {
                    int crossed = current->y + maximumStep > target->x;
                    if (!crossed)
                        result = 0;
                }
                return result;
            }
        } else {
            if (box_near(previous->y, target->x) || previous->y > target->x) {
                int result = 1;
                if (!box_near(current->y - maximumStep, target->x)) {
                    int crossed = current->y - maximumStep < target->x;
                    if (!crossed)
                        result = 0;
                }
                return result;
            }
        }
    }
    return 1;
}

} // extern "C"

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU80462108", _ctors_u8010B244_0, FxBox_InitializeUnitConstants);
#pragma pop

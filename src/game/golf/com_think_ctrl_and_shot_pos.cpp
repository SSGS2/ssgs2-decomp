// CPU thinking controller and shot positions: CComThinkCtrl holds the command results and
// options for the rival's decisions, and CComThinkShotPos caches the shot positions it picks.
// Also the name maps and entry lookup the controller uses.

#include <decomp/static_init.h>
#include <decomp/ct_member.h>
// Controller results: specialLandingResult (+1AC), firstLandingWithinTargetResult
// (+1B0), and finalPositionWithinTargetResult (+1B4).
// CComShotOnePoint::EvaluateShotResult supplies the three simulation results; the boost,
// carry and area-stop condition evaluators treat zero as false and nonzero
// as true. These are results, not difficulty or probability controls.
// conditionOptions (+EC..+1A8) holds 48 condition options: callers distinguish
// 1/3 (enabled) from 2/other (disabled). Slot-specific policy is unresolved;
// editing a result word is different from changing an indexed condition option.
struct ComThinkSettingsView {
    char prefix[0x2c];
    char commandState[0xc0];
    int conditionOptions[48];
    int specialLandingResult, firstLandingWithinTargetResult, finalPositionWithinTargetResult;
};
struct OwnedBufferView {
    void* vtable;
    void* data;
};
struct ShotPositionArray {
    void* data;
    int count;
    int capacity;
};
struct EcbTableCountView {
    char header[8];
    int count;
};
enum {
    kShotPositionStride = 0x34,
    kMapNodeAllocationSize = 0x20
};
struct ComThinkExceptionStrings {
    char commandsAndDiagnostics[0x2ee];
    char treeInsertLengthError[26];
    char lengthErrorType[55];
};
#define COM_MEMBER_OFFSET(type, member) ((int)&((type*)0)->member)
namespace exception_runtime {
extern "C" {
void __throw(const char*, void*, void*) __attribute__((noreturn));
void __dt__Q23std12length_errorFv(void*, int);
extern const char comThinkCommandStrings[];
}
}

#include "sdk_protos.h"
#include "classes/CComThinkShotPos.h"
#include "classes/CComThinkEcb.h"
#include "classes/CComThinkCtrl.h"

#define AsCComThinkEcb(p) ((CComThinkEcb*)(p))

#define AsCComThinkShotPos(p) ((CComThinkShotPos*)(p))

extern "C" {

extern "C" void* appMallocDummy__FUl(unsigned int);
extern "C" void* __dla__FPv(void*);
extern "C" void String_ReserveCapacity(void*, const void*);
extern "C" void String_ReplaceRange(void*, int, int, const void*, const void*);
extern "C" void RBTree_RebalanceAfterInsert(void*, int);
extern "C" void appFreeUni__FPv(void*);
extern "C" void Tree_FreeSubtree(void*, void*);
extern "C" void ComActionMap_DestroyStringPayload(void*, char*);
// __throw declared in sdk_protos.h
// strlen declared in sdk_protos.h
extern "C" void* __nwa__FUl(unsigned int);
extern "C" void* __nw__FUl(unsigned int);
extern "C" char comThinkCommandStrings[];
extern "C" char __vt__Q23std11logic_error[];
extern "C" char s_sharedCharDeleterVtable[];
extern "C" char __vt__Q23std12length_error[];
extern "C" char __vt__16CComThinkShotPos[];
extern "C" void __dt__Q23std12length_errorFv(void*);
// strcpy declared in sdk_protos.h

struct InsertList {
    int size;
    int root;
    void* unused;
    void* first;
};
struct InsertNode {
    void* leftChild;
    void* rightChild;
    void* parentAndColor;
    unsigned char value[20];
};
struct InsertValue {
    int key;
    unsigned int stringFlags;
    unsigned char data[12];
    int extra;
};

// Tree ABI used by both insertions: root at +4 is the root link (stored as an
// integer), unused at +8 is part of the sentinel, first at +0xC caches
// the leftmost node. Payload starts at node+12: integer key, 12-byte
// engine string, callback word. InsertValue is a raw copy view, not a vector;
// stringFlags is the first string word, not a size or a node count.
// Nodes and owned string buffers use appMallocDummy/appFreeUni; inline strings
// copy their three storage words. Unwind guards own a node until it is linked.

// ABI support for the two map insertions follows. Their near-duplicate string/error
// constructors retain different unwind lifetimes; the AI-facing operations begin
// at CComThinkCtrl::GetCommandState. Reuse ownership helpers, not these ABI types.
// Allocation guards remain live until string construction transfers ownership.
struct ErrHolder {
    void* p;
    char* flag;
    // Own the error-text allocation until its shared control block adopts it.
    ErrHolder(void* b, char* f) {
        p = b;
        flag = f;
    }
    // Free unadopted error-text storage on exceptional exit.
    ~ErrHolder() {
        if (p)
            __dla__FPv(p);
    }
};

struct ErrRef {
    int vt;
    int c1;
    int c2;
    char* s;
};

struct ErrBase2 {
    void* vt;
    char* buf;
    ErrRef* ref;
    // Build owned logic-error text with shared character deletion metadata.
    ErrBase2(const char* msg) {
        char done __attribute__((aligned(4))) = 0;
        vt = __vt__Q23std11logic_error;
        unsigned n = strlen(msg) + 1;
        char* b = (char*)__nwa__FUl(n);
        buf = b;
        {
            ErrHolder h(b, &done);
            ErrRef* r = (ErrRef*)__nw__FUl(16);
            if (r) {
                r->c1 = 1;
                r->c2 = 1;
                r->vt = (int)s_sharedCharDeleterVtable;
                r->s = b;
            }
            ref = r;
            h.p = 0;
        }
        strcpy(buf, msg);
    }
    // Perform no additional local cleanup in this reconstructed error base.
    ~ErrBase2() {}
};

struct Err2 : ErrBase2 {
    // Construct and immediately throw the reconstructed length-error object.
    Err2(const char* msg) : ErrBase2(msg) {
        vt = __vt__Q23std12length_error;
        __throw(comThinkCommandStrings + COM_MEMBER_OFFSET(ComThinkExceptionStrings, lengthErrorType), this,
                (void (*)())__dt__Q23std12length_errorFv);
    }
    // Perform no additional local cleanup for this error specialization.
    ~Err2() {}
};

extern "C++" {
// Return caller-provided placement storage without allocating.
inline void* operator new(size_t, void* p) {
    return p;
}
}

struct TargetErrBase {
    void* vt;
    char* buf;
    ErrRef* ref;
    // Build owned logic-error text with this instantiation's unwind lifetimes.
    TargetErrBase(const char* msg) {
        vt = __vt__Q23std11logic_error;
        char done __attribute__((aligned(4)));
        volatile char allocator __attribute__((aligned(4))) = 0;
        done = 0;
        int n = strlen(msg) + 1;
        char* buffer = (char*)__nwa__FUl(n * 2 - n);
        buf = buffer;
        {
            ErrHolder h(buffer, &done);
            ErrRef* r = (ErrRef*)__nw__FUl(16);
            if (r) {
                r->c1 = 1;
                r->c2 = 1;
                r->vt = (int)s_sharedCharDeleterVtable;
                r->s = buffer;
            }
            ref = r;
            h.p = 0;
        }
        strcpy(buf, msg);
    }
    // Release exception text through its shared control block (runtime definition).
    ~TargetErrBase();
};
extern "C++" {
namespace std {
struct length_error : TargetErrBase {
    // Construct the vector length-error text and its shared character owner.
    length_error(const char* msg) : TargetErrBase(msg) {
        vt = __vt__Q23std12length_error;
    }
    ~length_error();
    DECOMP_CT_MEMBER(length_error)
};
}
}

// Engine string copy: inline words are copied; a high flag bit requires a new owned buffer.
struct InsertString {
    unsigned int flags;
    unsigned int length;
    char* data;
    // Copy inline text words or allocate an independent heap string.
    InsertString(const InsertString& s) {
        if (!(s.flags >> 31)) {
            flags = s.flags;
            unsigned int n = s.length;
            char* p = s.data;
            length = n;
            data = p;
        } else {
            flags = 0;
            length = 0;
            data = 0;
            String_ReserveCapacity(this, (const void*)s.length);
            volatile char allocator __attribute__((aligned(4)));
            char copy __attribute__((aligned(4))) = allocator;
            ((void (*)(void*, int, int, const void*, const void*, const void*))String_ReplaceRange)(
                this, 0, 0, s.data, s.data + s.length, &copy);
        }
    }
};
struct InsertData {
    InsertString str;
    int extra;
    // Copy the string-bearing value and its callback word.
    InsertData(const InsertData& v) : str(v.str), extra(v.extra) {}
};
struct InsertPayload {
    int key;
    InsertData data;
    // Copy the integer key and its string/callback value.
    InsertPayload(const InsertPayload& v) : key(v.key), data(v.data) {}
};
struct InsertPosition {
    void* p;
    // Store the borrowed parent/sentinel link with the original unwind boundary.
    InsertPosition(void* pos) {
        try {
            p = pos;
        } catch (...) {
            throw;
        }
    }
};
struct InsertGuard {
    void* allocator;
    InsertNode* p;
    // Own a newly allocated node until tree insertion links it.
    InsertGuard(void* a, InsertNode* const& n) : allocator(a), p(n) {}
    // Free a node whose insertion did not transfer ownership to the tree.
    ~InsertGuard() {
        if (p)
            appFreeUni__FPv(p);
    }
};

struct TargetGuard_u8022DBE4 {
    void* allocator;
    InsertNode* p;
    // Own the volatile-slot node until its payload has been copied and linked.
    TargetGuard_u8022DBE4(void* a, InsertNode* volatile const& n) : allocator(a), p(n) {}
    // Free a node whose insertion did not transfer ownership to the tree.
    ~TargetGuard_u8022DBE4() {
        if (p)
            appFreeUni__FPv(p);
    }
};

// Separate copy view retains this instantiation's exception/unwind and allocator lifetimes.
struct TargetString {
    unsigned int flags;
    unsigned int length;
    char* data;
    int extra;
    // Copy inline or owned text, then preserve the callback word.
    TargetString(const TargetString& s) {
        try {
            if (!(s.flags >> 31)) {
                flags = s.flags;
                unsigned int n = s.length;
                char* p = s.data;
                length = n;
                data = p;
            } else {
                flags = 0;
                length = 0;
                data = 0;
                String_ReserveCapacity(this, (const void*)s.length);
                volatile char allocator __attribute__((aligned(4)));
                char copy __attribute__((aligned(4))) = allocator;
                ((void (*)(void*, int, int, const void*, const void*, const void*))String_ReplaceRange)(
                    this, 0, 0, s.data, s.data + s.length, &copy);
            }
            extra = s.extra;
        } catch (...) {
            throw;
        }
    }
};
struct TargetData {
    TargetString str;
    // Copy the complete string/callback value.
    TargetData(const TargetData& v) : str(v.str) {}
};
struct TargetPayload {
    int key;
    TargetData data;
    // Copy the integer key and its complete value.
    TargetPayload(const TargetPayload& v) : key(v.key), data(v.data) {}
};

#pragma push
#pragma use_lmw_stmw on
// Copy an action payload into a new node, link/rebalance it, and update the cached first node.
// position points at a parent node (or the header root slot); insertLeft selects its
// left link. updateFirstNode comes from FindInsertionPosition's second output flag.
// value is borrowed and deep-copied; the returned node belongs to list.
void* ComActionMap_InsertNode(InsertList* list, void** position, int insertLeft, int updateFirstNode,
                              const InsertValue* value) {
    // Phase 1: enforce the node-count limit before acquiring any storage.
    if ((unsigned int)list->size == 0xffffffffU) {
        const char* msg = comThinkCommandStrings + COM_MEMBER_OFFSET(ComThinkExceptionStrings, treeInsertLengthError);
        {
            union {
                double alignment;
                unsigned char bytes[sizeof(std::length_error)];
            } storage;
            std::length_error* error = (std::length_error*)storage.bytes;
            error->__ct(msg);
            exception_runtime::__throw(exception_runtime::comThinkCommandStrings +
                                           COM_MEMBER_OFFSET(ComThinkExceptionStrings, lengthErrorType),
                                       error, (void*)exception_runtime::__dt__Q23std12length_errorFv);
        }
    }
    // Phase 2: allocate and copy the payload while the guard owns the node.
    volatile char allocator __attribute__((aligned(4))) = 0;
    InsertNode* allocated = (InsertNode*)appMallocDummy__FUl(kMapNodeAllocationSize);
    InsertNode* allocatedCopy = allocated;
    InsertNode* volatile allocatedSlot;
    InsertNode* volatile allocatedCopySlot;
    allocatedCopySlot = allocatedCopy;
    allocatedSlot = allocated;
    InsertGuard guard(&list->root, allocated);
    new ((char*)allocatedCopy + 12) TargetPayload(*(const TargetPayload*)value);
    InsertNode* node = guard.p;
    // Phase 3: transfer ownership, link into the tree, then restore its balance.
    node->rightChild = 0;
    guard.p = 0;
    node->leftChild = 0;
    new (&node->parentAndColor) InsertPosition(position);
    if (insertLeft)
        position[0] = node;
    else
        position[1] = node;
    list->size++;
    RBTree_RebalanceAfterInsert(node, list->root);
    if (updateFirstNode)
        list->first = node;
    return node;
}
#pragma pop

#pragma push
#pragma use_lmw_stmw on
// Copy a condition payload into a new tree node with unwind cleanup before linking.
void* ComConditionMap_InsertNode(InsertList* list, void** position, int insertLeft, int updateFirstNode,
                                 const InsertValue* value) {
    // Same insertion contract as the action map; this string copy has its own
    // exception-lifetime shape. Keep its guards separate from the action copy.
    if ((unsigned int)list->size == 0xffffffffU) {
        const char* msg = comThinkCommandStrings + COM_MEMBER_OFFSET(ComThinkExceptionStrings, treeInsertLengthError);
        {
            union {
                double alignment;
                unsigned char bytes[sizeof(std::length_error)];
            } storage;
            std::length_error* error = (std::length_error*)storage.bytes;
            error->__ct(msg);
            exception_runtime::__throw(exception_runtime::comThinkCommandStrings +
                                           COM_MEMBER_OFFSET(ComThinkExceptionStrings, lengthErrorType),
                                       error, (void*)exception_runtime::__dt__Q23std12length_errorFv);
        }
    }
    volatile char allocator __attribute__((aligned(4))) = 0;
    InsertNode* allocated = (InsertNode*)appMallocDummy__FUl(kMapNodeAllocationSize);
    InsertNode* volatile allocatedSlot;
    InsertNode* volatile allocatedCopySlot;
    allocatedCopySlot = allocated;
    allocatedSlot = allocated;
    TargetGuard_u8022DBE4 guard(&list->root, allocated);
    // Payload construction completes before the node becomes reachable.
    try {
        new ((char*)allocated + 12) InsertPayload(*(const InsertPayload*)value);
    } catch (...) {
        throw;
    }
    InsertNode* node = guard.p;
    // Ownership now passes from the allocation guard to the tree.
    node->rightChild = 0;
    guard.p = 0;
    node->leftChild = 0;
    new (&node->parentAndColor) InsertPosition(position);
    if (insertLeft)
        position[0] = node;
    else
        position[1] = node;
    list->size++;
    RBTree_RebalanceAfterInsert(node, list->root);
    if (updateFirstNode)
        list->first = node;
    return node;
}
#pragma pop

extern "C" void __dl__FPv(void*);
extern "C" void* __dla__FPv(void*);

// Destroy a stateless base object, optionally releasing its allocation.
void* dtor_8022DD24(void* self, int flag) {
    if (self != 0) {
        if (flag > 0)
            __dl__FPv(self);
    }
    return self;
}

// Return the embedded command-state block beginning after the controller header.
void* CComThinkCtrl::GetCommandState(void* controller) {
    return ((ComThinkSettingsView*)controller)->commandState;
}

// Store the simulated special-landing result (zero=false, nonzero=true); no normalization is performed.
void CComThinkCtrl::SetSpecialLandingResult(int value) {
    specialLandingResult = value;
}

// Read the simulated special-landing result (zero=false, nonzero=true).
int CComThinkCtrl::GetSpecialLandingResult(void* controller) {
    return ((ComThinkSettingsView*)controller)->specialLandingResult;
}

// Store the simulated first-landing-within-target result; no normalization is performed.
void CComThinkCtrl::SetFirstLandingWithinTargetResult(int value) {
    firstLandingWithinTargetResult = value;
}

// Read the simulated first-landing-within-target result.
int CComThinkCtrl::GetFirstLandingWithinTargetResult(void* controller) {
    return ((ComThinkSettingsView*)controller)->firstLandingWithinTargetResult;
}

// Store the simulated final-position-within-target result; no normalization is performed.
void CComThinkCtrl::SetFinalWithinTargetResult(int value) {
    finalPositionWithinTargetResult = value;
}

// Read the simulated final-position-within-target result.
int CComThinkCtrl::GetFinalWithinTargetResult(void* controller) {
    return ((ComThinkSettingsView*)controller)->finalPositionWithinTargetResult;
}

// Read one condition option result. The caller supplies a valid index; there is
// no bounds check here.
int CComThinkCtrl::GetOption(int optionIndex) {
    return conditionOptions[optionIndex];
}

extern "C" float kFloat_6_2831855;
extern "C" float s_comThinkTwoPi;

// Initialize the AI unit's shared scalar at startup.
void InitializeComThinkScalars(void) {
    s_comThinkTwoPi = kFloat_6_2831855;
}

extern "C" void appFreeUni__FPv(void*);
extern "C" void __dl__FPv(void*);
extern "C" void* __dla__FPv(void*);
extern "C" char __vt__16CComThinkShotPos[];
extern "C" void* ComThinkShotPosMap_dtor(void*, int);
extern "C" void* ShotPositionArray_dtor(void*, int);
extern "C" void ComThinkShotPosMap_DestroySubtree(void*, void*);

// Clear cached shot positions and their lookup map, then optionally free the owner.
CComThinkShotPos::~CComThinkShotPos() {
    *(void**)this = __vt__16CComThinkShotPos;
    ClearCachedData();
    ComThinkShotPosMap_dtor(((char*)&priorityCount), -1);
    ShotPositionArray_dtor(((char*)&candidates), -1);
}

// Destroy cached 52-byte string-bearing records and free their backing allocation.
// self is a pointer/count/capacity array header. Destruction walks records in
// reverse and frees only their heap strings, then the array, then (flag>0) self.
// (The repeated `data != 0` tests below are no-ops kept so the compiled code matches.)
void* ShotPositionArray_dtor(void* self, int flag) {
    if (self != 0) {
        void** data = (void**)self;
        if (data != 0 && data != 0 && data != 0 && data[0] != 0) {
            char* p;
            int n = ((ShotPositionArray*)self)->count;
            p = (char*)data[0] + n * kShotPositionStride;
            ((ShotPositionArray*)self)->count = ((ShotPositionArray*)self)->count - n;
            while (n) {
                p -= kShotPositionStride;
                if (p != 0 && p != 0 && ((*(unsigned int*)p >> 31) != 0))
                    appFreeUni__FPv(*(void**)(p + 8));
                n--;
            }
            appFreeUni__FPv(data[0]);
        }
        if (flag > 0)
            __dl__FPv(self);
    }
    return self;
}

struct TreeNode {
    TreeNode* first;
    TreeNode* second;
    int parentAndColor;
    int key;
    unsigned int flag;
    unsigned int stringLength;
    void* stringData;
};
// first/second are left/right children; parentAndColor holds the packed parent pointer and node color.
// flag bit 31 owns stringData; otherwise the three string words contain inline
// bytes. TreeNode is a node/payload view, never a map header or sentinel.
// Free a node's heap string when it owns one (flag bit 31 set); the node itself is freed by the caller.
static inline void FreeNodeString(TreeNode* node) {
    // (the repeated address tests are the compiler's own checks from the original expansion)
    char* n = (char*)node;
    if (n + COM_MEMBER_OFFSET(TreeNode, key) != 0 && n + COM_MEMBER_OFFSET(TreeNode, flag) != 0 &&
        n + COM_MEMBER_OFFSET(TreeNode, flag) != 0) {
        unsigned int stringFlags = node->flag;
        if (stringFlags >> 31)
            appFreeUni__FPv(node->stringData);
    }
}
// Destruction schedule for each non-null node:
//   left descendants -> right descendants -> owned string -> node allocation.
// arg is a real node, never the header/sentinel; self is the borrowed map.
// The next two levels are expanded before recursion (three in the map dtor).
// Repeated member-address guards retain the inlined string destructor ABI;
// they do not imply additional allocations. String cleanup never frees nodes.
// Subtree cleanup does not reset header count/root/first; the owner is dying.
void ComConditionMap_DestroyStringPayload(void*, char*);

// Release this real node's heap-backed string; leave its allocation to the caller.
static inline void ReleaseTreeNodeString(TreeNode* node) {
    if ((char*)node + COM_MEMBER_OFFSET(TreeNode, key) != 0) {
        if ((char*)node + COM_MEMBER_OFFSET(TreeNode, flag) != 0) {
            if ((char*)node + COM_MEMBER_OFFSET(TreeNode, flag) != 0) {
                if (node->flag >> 31)
                    appFreeUni__FPv(node->stringData);
            }
        }
    }
}

#pragma push
#pragma use_lmw_stmw on
void Tree_FreeSubtree(void*, void*);

// Release both descendants, the owned string, then this non-null map node.
static inline void FreeActionMapLeaf(void* owner, TreeNode* node) {
    if (node->first)
        Tree_FreeSubtree(owner, node->first);
    if (node->second)
        Tree_FreeSubtree(owner, node->second);
    ComActionMap_DestroyStringPayload(owner, (char*)node + COM_MEMBER_OFFSET(TreeNode, key));
    appFreeUni__FPv(node);
}

// Free action-map descendants and each heap-backed payload string in postorder.
void Tree_FreeSubtree(void* self, void* arg) {
    // Phase 1: descend the left branch, freeing each string before its node.
    TreeNode* leftBranch = *(TreeNode**)arg;
    if (0 != leftBranch) {
        TreeNode* child = leftBranch->first;
        if (child != 0) {
            TreeNode* grandchild = child->first;
            if (grandchild != 0) {
                FreeActionMapLeaf(self, grandchild);
            }
            __typeof__(child->second) rightGrandchild = child->second;
            grandchild = rightGrandchild;
            if (grandchild != 0) {
                FreeActionMapLeaf(self, grandchild);
            }
            ComActionMap_DestroyStringPayload(self, (char*)child + COM_MEMBER_OFFSET(TreeNode, key));
            appFreeUni__FPv(child);
        }
        __typeof__(leftBranch->second) rightChild = leftBranch->second;
        child = rightChild;
        if (0 != child) {
            __typeof__(child->first) leftGrandchild = child->first;
            TreeNode* grandchild = leftGrandchild;
            if (grandchild != 0) {
                FreeActionMapLeaf(self, grandchild);
            }
            grandchild = child->second;
            if (grandchild != 0) {
                FreeActionMapLeaf(self, grandchild);
            }
            ComActionMap_DestroyStringPayload(self, (char*)child + COM_MEMBER_OFFSET(TreeNode, key));
            appFreeUni__FPv(child);
        }
        ReleaseTreeNodeString((TreeNode*)leftBranch);
        appFreeUni__FPv(leftBranch);
    }
    // Phase 2: release the right subtree after all left descendants.
    TreeNode* rightBranch = ((TreeNode*)arg)->second;
    if (0 != rightBranch) {
        TreeNode* child = rightBranch->first;
        if (0 != child) {
            TreeNode* grandchild = child->first;
            if (grandchild != 0) {
                FreeActionMapLeaf(self, grandchild);
            }
            grandchild = child->second;
            if (grandchild) {
                FreeActionMapLeaf(self, grandchild);
            }
            ComActionMap_DestroyStringPayload(self, (char*)child + COM_MEMBER_OFFSET(TreeNode, key));
            appFreeUni__FPv(child);
        }
        __typeof__(rightBranch->second) savedSecond = rightBranch->second;
        child = savedSecond;
        if (child) {
            TreeNode* grandchild = child->first;
            if (grandchild) {
                FreeActionMapLeaf(self, grandchild);
            }
            grandchild = child->second;
            if (grandchild) {
                FreeActionMapLeaf(self, grandchild);
            }
            ComActionMap_DestroyStringPayload(self, (char*)child + COM_MEMBER_OFFSET(TreeNode, key));
            appFreeUni__FPv(child);
        }
        {
            ReleaseTreeNodeString((TreeNode*)rightBranch);
        }
        appFreeUni__FPv(rightBranch);
    }
    {
        ReleaseTreeNodeString((TreeNode*)arg);
    }
    // Phase 3: release this node after its string and both branches.
    appFreeUni__FPv(arg);
}
#pragma pop

#pragma push
#pragma use_lmw_stmw on
void ComConditionMap_FreeSubtree(void*, void*);

// Release both descendants, the owned string, then this non-null map node.
static inline void FreeConditionMapLeaf(void* owner, TreeNode* node) {
    if (node->first)
        ComConditionMap_FreeSubtree(owner, node->first);
    if (node->second)
        ComConditionMap_FreeSubtree(owner, node->second);
    ComConditionMap_DestroyStringPayload(owner, (char*)node + COM_MEMBER_OFFSET(TreeNode, key));
    appFreeUni__FPv(node);
}

// Free condition-map descendants and each heap-backed payload string in postorder.
void ComConditionMap_FreeSubtree(void* self, void* arg) {
    // Phase 1: release the left subtree in postorder.
    TreeNode* leftBranch = *(TreeNode**)arg;
    TreeNode* child;
    TreeNode* grandchild;
    if (leftBranch != 0) {
        child = leftBranch->first;
        if (child != 0) {
            grandchild = child->first;
            if (grandchild != 0) {
                FreeConditionMapLeaf(self, grandchild);
            }
            grandchild = child->second;
            if (grandchild != 0) {
                FreeConditionMapLeaf(self, grandchild);
            }
            ComConditionMap_DestroyStringPayload(self, (char*)child + COM_MEMBER_OFFSET(TreeNode, key));
            appFreeUni__FPv(child);
        }
        grandchild = leftBranch->second;
        if (grandchild != 0) {
            child = grandchild->first;
            if (child != 0) {
                FreeConditionMapLeaf(self, child);
            }
            child = grandchild->second;
            if (child != 0) {
                FreeConditionMapLeaf(self, child);
            }
            ComConditionMap_DestroyStringPayload(self, (char*)grandchild + COM_MEMBER_OFFSET(TreeNode, key));
            appFreeUni__FPv(grandchild);
        }
        {
            ReleaseTreeNodeString((TreeNode*)leftBranch);
        }
        appFreeUni__FPv(leftBranch);
    }
    // Phase 2: release the right subtree with the same ownership rule.
    grandchild = ((TreeNode*)arg)->second;
    if (grandchild != 0) {
        child = grandchild->first;
        if (child != 0) {
            leftBranch = child->first;
            if (leftBranch != 0) {
                FreeConditionMapLeaf(self, leftBranch);
            }
            leftBranch = child->second;
            if (leftBranch != 0) {
                FreeConditionMapLeaf(self, leftBranch);
            }
            ComConditionMap_DestroyStringPayload(self, (char*)child + COM_MEMBER_OFFSET(TreeNode, key));
            appFreeUni__FPv(child);
        }
        child = grandchild->second;
        if (child != 0) {
            leftBranch = child->first;
            if (leftBranch != 0) {
                FreeConditionMapLeaf(self, leftBranch);
            }
            leftBranch = child->second;
            if (leftBranch != 0) {
                FreeConditionMapLeaf(self, leftBranch);
            }
            ComConditionMap_DestroyStringPayload(self, (char*)child + COM_MEMBER_OFFSET(TreeNode, key));
            appFreeUni__FPv(child);
        }
        {
            ReleaseTreeNodeString((TreeNode*)grandchild);
        }
        appFreeUni__FPv(grandchild);
    }
    {
        ReleaseTreeNodeString((TreeNode*)arg);
    }
    appFreeUni__FPv(arg);
}
#pragma pop

extern "C" int Hash_XorStringBytes(void*, const char*);
extern "C" void* Table_ResolveDataOffset(void*, int);
extern "C" int OffsetStringNode_GetText(void*);
// strcmp declared in sdk_protos.h
extern "C" void* Table_GetEntry(void*, int);
extern "C" char __vt__12CComThinkEcb[];

void* dtor_8022DD24(void*, int);
struct LocalException {
    void* volatile p;
    const char* volatile s;
};
void* dtor_8022DD24(void*, int);
#pragma push
#pragma use_lmw_stmw on
// Advance the cursor using the compiler's nonzero-mask form of index+1.
// For a zero sum the mask becomes zero; every other sum is retained unchanged.
// Keep the arithmetic shape here because simplifying it changes the emitted code.
static inline int AdvanceEcbCursor(int index) {
    return (index + 1) & ~((~((index + 1) | (-1 - index))) >> 31);
}

// Fetch one candidate after index; return zero at the count bound or a null entry.
// Only a successful name match publishes index/entry through the caller's cursor.
// Reference parameters reuse its scratch slots so this ABI helper emits the same loop.
static inline int FetchNextEcbCandidate(void* volatile& tableSlot, int& index, void*& table, int& limit, int& candidate,
                                        int& more) {
    return (table = tableSlot,
            // Phase 1: sample the current table and its count.
            limit = table != 0 ? ((EcbTableCountView*)table)->count : 0,
            // Phase 2: stop at the bound, otherwise advance before looking up.
            index >= limit - 1 ? (candidate = 0, more = 0)
                               : (index = AdvanceEcbCursor(index), candidate = (int)Table_GetEntry(table, index),
                                  more = candidate != 0),
            more != 0);
}

// Advance the ECB cursor until both name hash and full name match; return the index and entry.
int CComThinkCtrl::FindNextEntryByName(void* entryName, int* cursor) {
    LocalException local;
    local.p = 0;
    void* current = AsCComThinkEcb(this)->entryTable;
    if (current != 0)
        local.p = current;
    int index = *cursor;
    local.s = __vt__12CComThinkEcb;
    int nameHash = Hash_XorStringBytes(entryName, __vt__12CComThinkEcb);
    int candidate;
    int dataOffset;
    void* entry;
    void* namedEntry;
    int storedName;
    int matched;
    int more;
    // Original 0x8022DBE4: advance before testing each candidate.
    void* table;
    int limit;
    while (FetchNextEcbCandidate(local.p, index, table, limit, candidate, more)) {
        dataOffset = *(int*)candidate;
        entry = Table_ResolveDataOffset(local.p, dataOffset);
        if (*(int*)entry != nameHash) {
            matched = 0;
        } else {
            namedEntry = Table_ResolveDataOffset(local.p, dataOffset);
            storedName = namedEntry != 0 ? OffsetStringNode_GetText(namedEntry) : 0;
            matched = strcmp((const char*)entryName, (const char*)storedName) == 0;
        }
        if (matched) {
            cursor[1] = candidate;
            cursor[0] = index;
            return 1;
        }
    }
    return 0;
}
#pragma pop

// Release this owner's allocated buffer, then optionally free the owner.
void* dtor_8022E95C(void* self, int flag) {
    if (self != 0) {
        void* p = ((OwnedBufferView*)self)->data;
        if (p != 0)
            appFreeUni__FPv(p);
        if (flag > 0)
            __dl__FPv(self);
    }
    return self;
}

// Release this owner's allocated buffer, then optionally free the owner.
void* dtor_8022E9BC(void* self, int flag) {
    if (self != 0) {
        void* p = ((OwnedBufferView*)self)->data;
        if (p != 0)
            appFreeUni__FPv(p);
        if (flag > 0)
            __dl__FPv(self);
    }
    return self;
}

// Release the heap string inside a condition-map key/value payload.
void ComConditionMap_DestroyStringPayload(void* /*a*/, char* p) {
    if (p == 0)
        return;
    if (p + 4 == 0)
        return;
    if (p + 4 == 0)
        return;
    unsigned int v = *(unsigned int*)(p + 4);
    if (!(v >> 31))
        return;
    appFreeUni__FPv(*(void**)(p + 0xc));
}

// Release the heap string inside an action-map key/value payload.
void ComActionMap_DestroyStringPayload(void* /*a*/, char* p) {
    if (p == 0)
        return;
    if (p + 4 == 0)
        return;
    if (p + 4 == 0)
        return;
    unsigned int v = *(unsigned int*)(p + 4);
    if (!(v >> 31))
        return;
    appFreeUni__FPv(*(void**)(p + 0xc));
}

// Initialize an empty cache array and a map whose tail points at its root slot.
CComThinkShotPos::CComThinkShotPos() {
    char* base = (char*)this;
    void* vt = __vt__16CComThinkShotPos;
    int zero = 0;
    char* rootLink = base + COM_MEMBER_OFFSET(CComThinkShotPos, priorityRoot);
    *(void**)base = vt;
    AsCComThinkShotPos(base)->candidates = 0;
    AsCComThinkShotPos(base)->candidateCount = zero;
    AsCComThinkShotPos(base)->candidateCapacity = zero;
    AsCComThinkShotPos(base)->priorityCount = zero;
    AsCComThinkShotPos(base)->priorityRoot = 0;
    AsCComThinkShotPos(base)->priorityFirst = rootLink;
}

#pragma push
#pragma use_lmw_stmw on
// Finish one expanded shot-position leaf; scratch references retain the caller's traversal lifetimes.
static inline void FreeShotPositionLeaf(void* owner, char* leafNode, void*& leftDescendant, void*& rightDescendant) {
    leftDescendant = ((TreeNode*)leafNode)->first;
    if (leftDescendant != 0)
        ComThinkShotPosMap_DestroySubtree(owner, leftDescendant);
    rightDescendant = ((TreeNode*)leafNode)->second;
    if (rightDescendant != 0)
        ComThinkShotPosMap_DestroySubtree(owner, rightDescendant);
    FreeNodeString((TreeNode*)leafNode);
    appFreeUni__FPv(leafNode);
}

// Free cached-position tree nodes and their owned strings, then optionally free the map.
// Plain C: postorder walk of the map's tree (left subtree, right subtree, then the node): free each node's
// heap string if it owns one, then the node. The nested blocks below are that walk unrolled by hand for
// the first levels (the original compiler inlining); deeper levels call ComThinkShotPosMap_DestroySubtree.
void* ComThinkShotPosMap_dtor(void* self, int deleting) {
    // Phase 1: walk left descendants; only flag-bit-31 strings own character storage.
    char* rootNode;
    char* leftBranch;
    char* rightBranch;
    char* leftLeftBranch;
    char* leftRightBranch;
    char* rightLeftBranch;
    char* rightRightBranch;
    char* leafNode;
    void* leftDescendant;
    void* rightDescendant;
    unsigned int stringFlags;
    if (self != 0) {
        rootNode = (char*)((OwnedBufferView*)self)->data;
        if (rootNode != 0) {
            leftBranch = (char*)((TreeNode*)rootNode)->first;
            if (leftBranch != 0) {
                leftLeftBranch = (char*)((TreeNode*)leftBranch)->first;
                if (leftLeftBranch != 0) {
                    leafNode = (char*)((TreeNode*)leftLeftBranch)->first;
                    if (leafNode != 0) {
                        FreeShotPositionLeaf(self, leafNode, leftDescendant, rightDescendant);
                    }
                    leafNode = (char*)((TreeNode*)leftLeftBranch)->second;
                    if (leafNode != 0) {
                        FreeShotPositionLeaf(self, leafNode, leftDescendant, rightDescendant);
                    }
                    FreeNodeString((TreeNode*)leftLeftBranch);
                    appFreeUni__FPv(leftLeftBranch);
                }
                leftRightBranch = (char*)((TreeNode*)leftBranch)->second;
                if (leftRightBranch != 0) {
                    leafNode = (char*)((TreeNode*)leftRightBranch)->first;
                    if (leafNode != 0) {
                        FreeShotPositionLeaf(self, leafNode, leftDescendant, rightDescendant);
                    }
                    leafNode = (char*)((TreeNode*)leftRightBranch)->second;
                    if (leafNode != 0) {
                        FreeShotPositionLeaf(self, leafNode, leftDescendant, rightDescendant);
                    }
                    FreeNodeString((TreeNode*)leftRightBranch);
                    appFreeUni__FPv(leftRightBranch);
                }
                FreeNodeString((TreeNode*)leftBranch);
                appFreeUni__FPv(leftBranch);
            }
            // Phase 2: release right descendants using the same postorder rule.
            rightBranch = (char*)((TreeNode*)rootNode)->second;
            if (rightBranch != 0) {
                rightLeftBranch = (char*)((TreeNode*)rightBranch)->first;
                if (rightLeftBranch != 0) {
                    leafNode = (char*)((TreeNode*)rightLeftBranch)->first;
                    if (leafNode != 0) {
                        FreeShotPositionLeaf(self, leafNode, leftDescendant, rightDescendant);
                    }
                    leafNode = (char*)((TreeNode*)rightLeftBranch)->second;
                    if (leafNode != 0) {
                        FreeShotPositionLeaf(self, leafNode, leftDescendant, rightDescendant);
                    }
                    FreeNodeString((TreeNode*)rightLeftBranch);
                    appFreeUni__FPv(rightLeftBranch);
                }
                rightRightBranch = (char*)((TreeNode*)rightBranch)->second;
                if (rightRightBranch != 0) {
                    leafNode = (char*)((TreeNode*)rightRightBranch)->first;
                    if (leafNode != 0) {
                        FreeShotPositionLeaf(self, leafNode, leftDescendant, rightDescendant);
                    }
                    leafNode = (char*)((TreeNode*)rightRightBranch)->second;
                    if (leafNode != 0) {
                        FreeShotPositionLeaf(self, leafNode, leftDescendant, rightDescendant);
                    }
                    FreeNodeString((TreeNode*)rightRightBranch);
                    appFreeUni__FPv(rightRightBranch);
                }
                FreeNodeString((TreeNode*)rightBranch);
                appFreeUni__FPv(rightBranch);
            }
            FreeNodeString((TreeNode*)rootNode);
            appFreeUni__FPv(rootNode);
        }
        // Phase 3: optionally release the map header itself.
        if (deleting > 0)
            __dl__FPv(self);
    }
    return self;
}
#pragma pop

} // extern "C"

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU80462240", _ctors_u8022DBE4_0, InitializeComThinkScalars);
#pragma pop

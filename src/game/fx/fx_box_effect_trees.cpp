// Effect box trees: the string-keyed tree maps that hold the spray and sequence templates and
// the resource names of an effect box, and the node code that builds and frees them.

#include <decomp/static_init.h>
#include "sdk_protos.h"
#include "classes/CFx.h"
#include "classes/CFxBox.h"
#include "classes/CFxEffectTree.h"

extern "C" {

extern const float kFloat_6_2831855;
extern float s_twoPi_80107180;

// Initialize this effects unit's writable two-pi value.
void FxBox_InitTwoPiCache() {
    s_twoPi_80107180 = kFloat_6_2831855;
}

void* __dt__3CFxFv(void*, int);
extern const float kFxBoxDefaultFrameLimitMilliseconds;
extern float kFloat_1_FxBox;
extern float kFloat_0_FxBox;
extern unsigned char s_fxBoxVtable[];

// Node of the effect-control tree: child links, the parent pointer with its
// colour bit packed into the low bit, the signed serial key, and the control.
struct FxControlTreeNode {
    FxControlTreeNode* left;     // 0x00
    FxControlTreeNode* right;    // 0x04
    unsigned int parentAndColor; // 0x08
    int key;                     // 0x0C
    void* control;               // 0x10  owned control, destroyed before the node
};

// Find the node to insert a signed control key under and which of its sides to
// use. The address of the root slot is the end sentinel, so an empty tree
// returns that address.
FxControlTreeNode* CFxEffectTree::FindInsertionPosition(int* key, FxControlTreeNode** parent, unsigned char* insertLeft,
                                                        unsigned char* beforeFirst) {
    FxControlTreeNode* result = (FxControlTreeNode*)&root;
    FxControlTreeNode* node;
    *parent = 0;
    node = root;
    *insertLeft = 1;
    *beforeFirst = 1;
    while (node) {
        result = node;
        if (*key < node->key) {
            node = node->left;
            *insertLeft = 1;
        } else {
            *parent = result;
            node = node->right;
            *insertLeft = 0;
            *beforeFirst = 0;
        }
    }
    return result;
}

extern const char shotEffectResourceStrings[];
extern char __vt__Q23std11logic_error[];
extern char s_sharedCharDeleterVtable[];
extern char __vt__Q23std12length_error[];
extern void* appMallocDummy__FUl(unsigned int);
extern void RBTree_RebalanceAfterInsert(void*, void*, unsigned int);
extern void* __nwa__FUl(unsigned int);
extern void __dla__FPv(void*);
extern void __dt__Q23std12length_errorFv(void*);
extern void* __nw__FUl(unsigned int);
extern void __dt__Q33std6detail16char_array_guardFv(void*);
extern "C" void __dt__Q23std9exceptionFv(void*);
extern void appFreeUni__FPv(void*);

#pragma use_lmw_stmw on
// Copy the spray key/value pair into a new node, attach it, and rebalance the tree.
void* FxSprayTree_InsertNode(void* list, void* parent, int insertLeft, int updateCachedFirst, const void* source) {
    // Shared exception-message owner: vtable, strong/weak counts, owned character array.
    struct Ref {
        int vt;
        int c1;
        int c2;
        char* s;
    };
    // Compiler-expanded insertion frame: a/b retain the new allocation; c is the
    // root-slot sentinel; d owns the node until attachment commits. flag is initialized only.
    struct Tmp {
        char flag;
        void* a;
        void* b;
        void* pad[3];
        void* c;
        void* d;
    };
    struct Holder {
        char* p;
        char* flag;
        // Guard the copied message allocation during control-block construction.
        Holder(char* b, char* f) {
            p = b;
            flag = f;
        }
        // Delete message bytes if ownership has not been transferred.
        ~Holder() {
            if (p)
                __dla__FPv(p);
        }
    };
    struct ErrBase {
        void* vt;
        char* buf;
        Ref* ref;
        // Copy the exception message and establish shared ownership of its bytes.
        ErrBase(const char* msg0) {
            vt = (void*)__vt__Q23std11logic_error;
            char done __attribute__((aligned(4))) = 0;
            struct Guard {
                char flag;
                // Initialize the compiler-expanded allocator state byte.
                Guard() {
                    flag = 0;
                }
            } guard;
            const char* msg = msg0;
            unsigned n = strlen(msg) + 1;
            char* buffer = (char*)__nwa__FUl(n * 2 - n);
            buf = buffer;
            {
                Holder h(buffer, &done);
                Ref* r = (Ref*)__nw__FUl(16);
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
        // Destroy the standard exception base and its message state.
        ~ErrBase() {
            __dt__Q23std9exceptionFv(this);
        }
    };
    struct ErrThrow : ErrBase {
        // Install the length-error vtable and throw the constructed exception.
        ErrThrow(const char* msg) : ErrBase(msg) {
            vt = (void*)__vt__Q23std12length_error;
            __throw(shotEffectResourceStrings + 0x27c, this, (void (*)())__dt__Q23std12length_errorFv);
        }
        // Preserve the empty derived exception cleanup hook.
        ~ErrThrow() {}
    };
    struct Failure {
        // Throw the spray-tree length error before its count can overflow.
        static void run() {
            ErrThrow err(shotEffectResourceStrings + 0x262);
        }
    };
    // Phase 1: reject count overflow before allocating a node.
    if (*reinterpret_cast<unsigned int*>(list) == 0xFFFFFFFFu)
        Failure::run();
    struct Node {
        void* left;
        void* right;
        void* parent;
        unsigned int data0;
        unsigned int data1;
    };
    Tmp insertionCleanup;
    insertionCleanup.flag = 0;
    // Phase 2: copy the borrowed key/value payload under the insertion cleanup frame.
    Node* node = (Node*)appMallocDummy__FUl(0x14);
    void* payloadStorage = &node->data0; // +0xC: node payload
    insertionCleanup.a = node;
    insertionCleanup.b = node;
    insertionCleanup.c = (char*)list + 4;
    insertionCleanup.d = node;
    if (payloadStorage != 0) {
        try {
            ((unsigned int*)payloadStorage)[0] = ((const unsigned int*)source)[0];
            ((unsigned int*)payloadStorage)[1] = ((const unsigned int*)source)[1];
        } catch (...) {
            throw;
        }
    }
    node->right = 0;
    insertionCleanup.d = 0;
    node->left = 0;
    if (&node->parent != 0) { // +8
        try {
            node->parent = parent;
        } catch (...) {
            throw;
        }
    }
    // Phase 3: attach to the selected parent side, update counts, and rebalance.
    if (insertLeft)
        *(void**)parent = node;
    else
        *((void**)parent + 1) = node;
    unsigned int count = *(unsigned int*)list;
    *(unsigned int*)list = count + 1;
    RBTree_RebalanceAfterInsert(node, *((void**)list + 1), count);
    if (updateCachedFirst)
        *((void**)list + 2) = node;
    if (insertionCleanup.d)
        appFreeUni__FPv(insertionCleanup.d);
    return node;
}
#pragma use_lmw_stmw reset

// Four same-layout parameter records; only initialization is recovered here.
// Names identify storage roles, not an assumed color/vector/channel meaning.
// The state word and float pair have no proven units or downstream reader yet.
struct BoxParameterRecord {
    char prefix[0x10];
    int unresolvedStateWord;
    char reserved14[4];
    float defaultPair[2];
    char tail[0x18];
};
struct BoxParameterStorage {
    char boxPrefix[0x170];
    BoxParameterRecord* recordPointers[4]; // Physical record order 0,1,3,2.
    int unresolvedState;
    BoxParameterRecord physicalRecords[4];
};
typedef char BoxParameterRecord_size[(sizeof(BoxParameterRecord) == 0x38) ? 1 : -1];
typedef char BoxParameterStorage_offset[((int)&((BoxParameterStorage*)0)->physicalRecords == 0x184) ? 1 : -1];
// Borrow the embedded records; this view owns no allocations.
static inline BoxParameterStorage* BoxParameters(void* box) {
    return (BoxParameterStorage*)box;
}

// Borrow effect-box fields without changing pointer or ownership.
static inline CFxBox* AsCFxBox(void* p) {
    return (CFxBox*)p;
}

struct Init80107930 {
    char data[0x24C];
};
// Initialize empty effect caches, playback controls, and embedded effect-state records.
// Timing contract: callers convert seconds to milliseconds before UpdateEffects
// (cfx_shot_effects.cpp/screen_ball_preview.cpp). pause=0 allows updates; timeScale=1 preserves speed;
// Shared clampSimulationStep enables a 1.5*nominalStepMilliseconds cap, initially about 50 ms.
// nominalStepMilliseconds is a nominal step duration (1000/30 milliseconds), not a
// multiplier. The shared name is kept as-is; this local
// defaultStepMilliseconds makes the unit explicit. These are simulation steps.
// Each full CFxBox tree stores count/root/cached-first; an empty cached-first points to the root slot
// inside this object. Resource manager and camera transform start unbound.
CFxBox::CFxBox() {
    Init80107930* self = (Init80107930*)this;
    // Phase 1: form embedded sentinels; no heap allocations are owned yet.
    char* boxBytes = (char*)self;
    register float defaultValue = 1.0f;
    register float defaultStepMilliseconds = kFxBoxDefaultFrameLimitMilliseconds;
    // Byte-addressed sentinel formation preserves the original constructor's register choices.
    void* sequenceTemplateSentinel = boxBytes + (int)&((CFxBox*)0)->sequenceTemplateCacheRoot;
    void* spriteCacheSentinel = boxBytes + (int)&((CFxBox*)0)->spriteCacheRoot;
    void* sprayTemplateSentinel = boxBytes + (int)&((CFxBox*)0)->sprayTemplateCacheRoot;
    void* spraySentinel = boxBytes + (int)&((CFxBox*)0)->sprayTreeRoot;
    int zero = 0;
    void* sequenceSentinel = boxBytes + (int)&((CFxBox*)0)->sequenceTreeRoot;
    BoxParameterRecord* firstState = &BoxParameters(boxBytes)->physicalRecords[0];
    void* vtable = (void*)s_fxBoxVtable;
    BoxParameterRecord* secondState = &BoxParameters(boxBytes)->physicalRecords[1];
    BoxParameterRecord* fourthState = &BoxParameters(boxBytes)->physicalRecords[3];
    BoxParameterRecord* thirdState = &BoxParameters(boxBytes)->physicalRecords[2];
    int enabled = 1;
    int invalidIndex = -1;

    AsCFxBox(boxBytes)->vtbl = vtable; // +0x0
    AsCFxBox(boxBytes)->sprayTemplateCacheCount = zero;
    AsCFxBox(boxBytes)->sprayTemplateCacheRoot = zero;
    AsCFxBox(boxBytes)->sprayTemplateCacheFirst = sprayTemplateSentinel;
    AsCFxBox(boxBytes)->sequenceTemplateCacheCount = zero;
    AsCFxBox(boxBytes)->sequenceTemplateCacheRoot = zero;
    AsCFxBox(boxBytes)->sequenceTemplateCacheFirst = sequenceTemplateSentinel;
    AsCFxBox(boxBytes)->spriteCacheCount = zero;
    AsCFxBox(boxBytes)->spriteCacheRoot = zero;
    AsCFxBox(boxBytes)->spriteCacheFirst = spriteCacheSentinel;
    AsCFxBox(boxBytes)->sprayCount = zero;
    AsCFxBox(boxBytes)->sprayTreeRoot = zero;
    AsCFxBox(boxBytes)->sprayTreeFirst = spraySentinel;
    AsCFxBox(boxBytes)->sequenceCount = zero;
    AsCFxBox(boxBytes)->sequenceTreeRoot = zero;
    AsCFxBox(boxBytes)->sequenceTreeFirst = sequenceSentinel;
    // Phase 2: set playback gates and timing defaults shared by every effect.
    AsCFxBox(boxBytes)->resourceManager = 0;
    AsCFxBox(boxBytes)->pause = (char)zero;
    AsCFxBox(boxBytes)->renderFlags = zero;
    AsCFxBox(boxBytes)->nominalStepMilliseconds = defaultStepMilliseconds;
    AsCFxBox(boxBytes)->clampSimulationStep = (char)enabled;
    AsCFxBox(boxBytes)->timeScale = defaultValue;
    // +0x0C has no behavioral reader in the recovered box methods; its gate
    // semantics are unknown. Do not treat it as a second pause flag.
    AsCFxBox(boxBytes)->unk_C = (char)zero;
    // +0x11C/+0x120 are reset before each DrawSprays pass (fx_box_interpreter_resources.cpp).
    // No reader/increment establishes what these two draw-pass words measure.
    AsCFxBox(boxBytes)->unk_11C = zero;
    AsCFxBox(boxBytes)->unk_120 = zero;
    AsCFxBox(boxBytes)->debugBoundsEnabled = (char)zero;
    AsCFxBox(boxBytes)->cameraTransformAddress = 0;
    // Phase 3: four 0x38-byte records at 184/1BC/1F4/22C. Pointer slots
    // 170/174/178/17C refer to records in order 0/1/3/2, not address order.
    // Relative +10 is initialized to -1; +18/+1C float defaults are respectively
    // (0,1), (0,0), (1,1), (1,0). Their channel identities and units are unresolved;
    // do not assume these per-record floats use the millisecond update unit.
    BoxParameters(boxBytes)->recordPointers[0] = firstState;
    BoxParameters(boxBytes)->recordPointers[1] = secondState;
    BoxParameters(boxBytes)->recordPointers[2] = fourthState;
    BoxParameters(boxBytes)->recordPointers[3] = thirdState;
    BoxParameters(boxBytes)->unresolvedState = zero;
    // 0x80107180 unit: unconditional initial values.
    defaultValue = kFloat_0_FxBox;
    BoxParameters(boxBytes)->physicalRecords[0].defaultPair[0] = defaultValue;
    defaultValue = kFloat_1_FxBox;
    BoxParameters(boxBytes)->physicalRecords[0].defaultPair[1] = defaultValue;
    BoxParameters(boxBytes)->physicalRecords[0].unresolvedStateWord = invalidIndex;
    defaultValue = kFloat_0_FxBox;
    BoxParameters(boxBytes)->physicalRecords[1].defaultPair[0] = defaultValue;
    BoxParameters(boxBytes)->physicalRecords[1].defaultPair[1] = kFloat_0_FxBox;
    {
        float unitValue = *(float*)&kFloat_1_FxBox;
        BoxParameters(boxBytes)->physicalRecords[1].unresolvedStateWord = invalidIndex;
        BoxParameters(boxBytes)->physicalRecords[2].defaultPair[0] = unitValue;
    }
    defaultValue = kFloat_1_FxBox;
    BoxParameters(boxBytes)->physicalRecords[2].defaultPair[1] = defaultValue;
    {
        float unitValue = *(float*)&kFloat_1_FxBox;
        BoxParameters(boxBytes)->physicalRecords[2].unresolvedStateWord = invalidIndex;
        BoxParameters(boxBytes)->physicalRecords[3].defaultPair[0] = unitValue;
    }
    BoxParameters(boxBytes)->physicalRecords[3].defaultPair[1] = kFloat_0_FxBox;
    BoxParameters(boxBytes)->physicalRecords[3].unresolvedStateWord = invalidIndex;
}

// Adjust an interface pointer back to the owning effect before destruction.
// This is a vtable adjustor, not a destructor body, so it stays a named member.
void* CFx::DtorThunk4(int forwarded1) {
    return __dt__3CFxFv((char*)this - 4, forwarded1);
}

// Node of the unsigned-key resource tree (same layout family as EffectKeyNode).
struct ResourceKeyNode {
    ResourceKeyNode* left;
    ResourceKeyNode* right;
    char parentAndColor[4];
    unsigned int value;
};

// This prefix stops at +4 root storage; the separate cached-first field is outside the view.
struct ResourceKeyTreeHeader {
    char nodeCount[4];
    ResourceKeyNode* root;
};

// Find an unsigned resource key's insertion parent and insertion-side flags.
void* FxBoxTree_FindInsertHint(ResourceKeyTreeHeader* root, unsigned int* key, ResourceKeyNode** parent,
                               unsigned char* insertLeft, unsigned char* beforeFirst) {
    ResourceKeyNode* result = (ResourceKeyNode*)&root->root; // +4: tree sentinel
    ResourceKeyNode* node;
    *parent = 0;
    ResourceKeyNode* savedRoot = root->root;
    *insertLeft = 1;
    *beforeFirst = 1;
    node = savedRoot;
    while (node) {
        result = node;
        if (*key < node->value) {
            node = node->left;
            *insertLeft = 1;
        } else {
            *parent = result;
            node = node->right;
            *insertLeft = 0;
            *beforeFirst = 0;
        }
    }
    return result;
}

// Bare binary-tree node view (left/right only) used by the postorder free routines; payload is not touched.
struct TwoChildNode {
    TwoChildNode* left;
    TwoChildNode* right;
};
// Both node-only traversals below implement the same ownership rule:
//   destroy(left), destroy(right), free(node), with three levels expanded.
// root must be a real non-null heap node, never the embedded tree sentinel.
// context is forwarded to recursive calls; no payload destructor runs here.
// Expanded levels/local copies retain the matched compiler's call/spill shape;
// the second specialization casts child prefixes to its distinct root type.
void appFreeUni__FPv(void*);
void FxBoxResourceTree_FreeSubtreeNodes(void*, TwoChildNode*);
#pragma use_lmw_stmw on
// Free resource-tree nodes in postorder; the first levels are expanded in place.
void FxBoxResourceTree_FreeSubtreeNodes(void* context, TwoChildNode* root) {
    TwoChildNode* child;
    TwoChildNode* node = root->left;
    TwoChildNode* grandchild;
    // Phase 1: finish the entire left subtree before releasing any right branch.
    if (node) {
        child = node->left;
        if (child) {
            grandchild = child->left;
            if (grandchild) {
                if (grandchild->left)
                    FxBoxResourceTree_FreeSubtreeNodes(context, grandchild->left);
                if (grandchild->right)
                    FxBoxResourceTree_FreeSubtreeNodes(context, grandchild->right);
                appFreeUni__FPv(grandchild);
            }
            grandchild = child->right;
            if (grandchild) {
                if (grandchild->left)
                    FxBoxResourceTree_FreeSubtreeNodes(context, grandchild->left);
                if (grandchild->right)
                    FxBoxResourceTree_FreeSubtreeNodes(context, grandchild->right);
                appFreeUni__FPv(grandchild);
            }
            appFreeUni__FPv(child);
        }
        child = node->right;
        if (child) {
            grandchild = child->left;
            if (grandchild) {
                if (grandchild->left)
                    FxBoxResourceTree_FreeSubtreeNodes(context, grandchild->left);
                if (grandchild->right)
                    FxBoxResourceTree_FreeSubtreeNodes(context, grandchild->right);
                appFreeUni__FPv(grandchild);
            }
            grandchild = child->right;
            if (grandchild) {
                if (grandchild->left)
                    FxBoxResourceTree_FreeSubtreeNodes(context, grandchild->left);
                if (grandchild->right)
                    FxBoxResourceTree_FreeSubtreeNodes(context, grandchild->right);
                appFreeUni__FPv(grandchild);
            }
            appFreeUni__FPv(child);
        }
        appFreeUni__FPv(node);
    }
    // Phase 2: finish the right subtree, then release this root exactly once.
    child = root->right;
    if (child) {
        node = child->left;
        if (node) {
            // Keep these repeated reads/copies: collapsing them changes the matched traversal.
            TwoChildNode* leftReadForResult = node->left;
            TwoChildNode* leftReadForCopy = node->left;
            TwoChildNode* leftReadForType = node->left;
            TwoChildNode* copiedLeft = leftReadForCopy;
            TwoChildNode* retainedLeft = copiedLeft;
            TwoChildNode* finalLeft = leftReadForResult;
            grandchild = finalLeft;
            if (grandchild) {
                if (grandchild->left)
                    FxBoxResourceTree_FreeSubtreeNodes(context, grandchild->left);
                if (grandchild->right)
                    FxBoxResourceTree_FreeSubtreeNodes(context, grandchild->right);
                appFreeUni__FPv(grandchild);
            }
            grandchild = node->right;
            if (grandchild) {
                if (grandchild->left)
                    FxBoxResourceTree_FreeSubtreeNodes(context, grandchild->left);
                if (grandchild->right)
                    FxBoxResourceTree_FreeSubtreeNodes(context, grandchild->right);
                appFreeUni__FPv(grandchild);
            }
            appFreeUni__FPv(node);
        }
        node = child->right;
        if (node) {
            // Keep these repeated reads/copies: collapsing them changes the matched traversal.
            TwoChildNode* leftReadForResult = node->left;
            TwoChildNode* leftReadForCopy = node->left;
            TwoChildNode* leftReadForType = node->left;
            TwoChildNode* copiedLeft = leftReadForCopy;
            TwoChildNode* retainedLeft = copiedLeft;
            TwoChildNode* finalLeft = leftReadForResult;
            grandchild = finalLeft;
            if (grandchild) {
                if (grandchild->left)
                    FxBoxResourceTree_FreeSubtreeNodes(context, grandchild->left);
                if (grandchild->right)
                    FxBoxResourceTree_FreeSubtreeNodes(context, grandchild->right);
                appFreeUni__FPv(grandchild);
            }
            TwoChildNode* savedRight = node->right;
            grandchild = savedRight;
            if (grandchild) {
                if (grandchild->left)
                    FxBoxResourceTree_FreeSubtreeNodes(context, grandchild->left);
                if (grandchild->right)
                    FxBoxResourceTree_FreeSubtreeNodes(context, grandchild->right);
                appFreeUni__FPv(grandchild);
            }
            appFreeUni__FPv(node);
        }
        appFreeUni__FPv(child);
    }
    appFreeUni__FPv(root);
}
#pragma use_lmw_stmw reset
struct TwoChildRoot {
    TwoChildNode* left;
    TwoChildNode* right;
};
void FxTree_DestroySubtreeNodes(void*, TwoChildRoot*);
// Complete the deepest expanded effect branch while retaining its distinct recursion entry.
static inline void FinishEffectTreeBranch(void* context, TwoChildNode* node) {
    if (node->left)
        FxTree_DestroySubtreeNodes(context, (TwoChildRoot*)node->left);
    if (node->right)
        FxTree_DestroySubtreeNodes(context, (TwoChildRoot*)node->right);
    appFreeUni__FPv(node);
}
#pragma use_lmw_stmw on
// Free effect-tree nodes in postorder using the same expanded traversal.
void FxTree_DestroySubtreeNodes(void* context, TwoChildRoot* root) {
    TwoChildNode* savedLeft = root->left;
    TwoChildNode* child;
    TwoChildNode* node = savedLeft;
    TwoChildNode* grandchild;
    // Phase 1: free the left subtree in the same postorder as the resource tree.
    if (node) {
        TwoChildNode* savedLeft2 = node->left;
        child = savedLeft2;
        if (child) {
            TwoChildNode* savedLeft3 = child->left;
            grandchild = savedLeft3;
            if (grandchild) {
                FinishEffectTreeBranch(context, grandchild);
            }
            grandchild = child->right;
            if (grandchild) {
                FinishEffectTreeBranch(context, grandchild);
            }
            appFreeUni__FPv(child);
        }
        TwoChildNode* savedRight2 = node->right;
        child = savedRight2;
        if (child) {
            TwoChildNode* savedLeft4 = child->left;
            grandchild = savedLeft4;
            if (grandchild) {
                FinishEffectTreeBranch(context, grandchild);
            }
            grandchild = child->right;
            if (grandchild) {
                FinishEffectTreeBranch(context, grandchild);
            }
            appFreeUni__FPv(child);
        }
        appFreeUni__FPv(node);
    }
    // Phase 2: free the right subtree before the supplied root.
    child = root->right;
    if (child) {
        node = child->left;
        if (node) {
            TwoChildNode* savedLeft5 = node->left;
            grandchild = savedLeft5;
            if (grandchild) {
                FinishEffectTreeBranch(context, grandchild);
            }
            TwoChildNode* savedRight3 = node->right;
            grandchild = savedRight3;
            if (grandchild) {
                FinishEffectTreeBranch(context, grandchild);
            }
            appFreeUni__FPv(node);
        }
        node = child->right;
        if (node) {
            // Keep these repeated reads/copies: collapsing them changes the matched traversal.
            TwoChildNode* leftReadForResult = node->left;
            TwoChildNode* leftReadForCopy = node->left;
            TwoChildNode* leftReadForType = node->left;
            TwoChildNode* copiedLeft = leftReadForCopy;
            TwoChildNode* retainedLeft = copiedLeft;
            TwoChildNode* finalLeft = leftReadForResult;
            grandchild = finalLeft;
            if (grandchild) {
                FinishEffectTreeBranch(context, grandchild);
            }
            TwoChildNode* savedRight4 = node->right;
            grandchild = savedRight4;
            if (grandchild) {
                FinishEffectTreeBranch(context, grandchild);
            }
            appFreeUni__FPv(node);
        }
        appFreeUni__FPv(child);
    }
    appFreeUni__FPv(root);
}
#pragma use_lmw_stmw reset

// String-map node view: +00/+04 child links, +08 packed parent/color,
// +0C..+17 embedded 12-byte narrow string. The legacy owned field is the
// string's flags/inline prefix; its top bit selects heap storage at +14.
// Inline bytes belong to the node and must not be passed to appFreeUni.
// The repeated +0xC/null tests below are compiler-expanded subobject guards,
// not separate allocations. Each node releases its heap string (if flagged)
// before freeing the node; the wrapper/sentinel is freed only for deleteFlag>0.
// Node of a map whose values are narrow strings (12-byte tagged representation; byte zero is metadata, heap pointer at +8 within it).
struct StringMapNode {
    StringMapNode* left;
    StringMapNode* right;
    char parentAndColor[4];
    unsigned int stringFlagsAndInlinePrefix;
    char valuePrefix[4];
    void* heapStringData;
};
// The +4 root starts subtree destruction; cached-first storage is outside this prefix.
struct StringMapHeader {
    char nodeCount[4];
    StringMapNode* root;
};
void StringValueTree_DestroySubtree(void*, void*);
void __dl__FPv(void*);

// Release a node's flagged heap string, then free the node itself.
inline void FreeOwnedStringNode(StringMapNode* node) {
    if (&node->stringFlagsAndInlinePrefix) {
        if (&node->stringFlagsAndInlinePrefix) {
            if (node->stringFlagsAndInlinePrefix >> 31)
                appFreeUni__FPv(node->heapStringData);
        }
    }
    appFreeUni__FPv(node);
}

// Finish a non-null expanded leaf: recurse into deeper nodes, then release key/node.
static inline void DestroySprayStringLeaf(StringMapNode* node, void* context) {
    if (node->left)
        StringValueTree_DestroySubtree(context, node->left);
    if (node->right)
        StringValueTree_DestroySubtree(context, node->right);
    FreeOwnedStringNode(node);
}

#pragma use_lmw_stmw on
// Destroy every owned string value and tree node, then optionally free the tree wrapper.
void* FxSprayTemplateMap_dtor(void* context, int deleteFlag) {
    if (context) {
        // Repeated wrapper guards preserve the original destructor expansion.
        if (context) {
            if (context) {
                if (context) {
                    if (context) {
                        if (context) {
                            // Phase 1: destroy the left branch in expanded postorder; descendants
                            // release flagged heap strings before their own node storage.
                            StringMapNode* root = ((StringMapHeader*)context)->root; // +4
                            if (root) {
                                StringMapNode* child = root->left;
                                if (child) {
                                    StringMapNode* savedLeft6 = child->left;
                                    StringMapNode* grandchild = savedLeft6;
                                    if (grandchild) {
                                        StringMapNode* savedLeft7 = grandchild->left;
                                        StringMapNode* leaf = savedLeft7;
                                        if (leaf) {
                                            DestroySprayStringLeaf(leaf, context);
                                        }
                                        leaf = grandchild->right;
                                        if (leaf) {
                                            DestroySprayStringLeaf(leaf, context);
                                        }
                                        FreeOwnedStringNode(grandchild);
                                    }
                                    grandchild = child->right;
                                    if (grandchild) {
                                        StringMapNode* leaf = grandchild->left;
                                        if (leaf) {
                                            DestroySprayStringLeaf(leaf, context);
                                        }
                                        leaf = grandchild->right;
                                        if (leaf) {
                                            DestroySprayStringLeaf(leaf, context);
                                        }
                                        FreeOwnedStringNode(grandchild);
                                    }
                                    FreeOwnedStringNode(child);
                                }
                                // Phase 2: destroy the right branch, then this root and the optional wrapper.
                                StringMapNode* savedRight5 = root->right;
                                child = savedRight5;
                                if (child) {
                                    StringMapNode* grandchild = child->left;
                                    if (grandchild) {
                                        StringMapNode* leaf = grandchild->left;
                                        if (leaf) {
                                            DestroySprayStringLeaf(leaf, context);
                                        }
                                        leaf = grandchild->right;
                                        if (leaf) {
                                            DestroySprayStringLeaf(leaf, context);
                                        }
                                        FreeOwnedStringNode(grandchild);
                                    }
                                    StringMapNode* savedRight6 = child->right;
                                    grandchild = savedRight6;
                                    if (grandchild) {
                                        StringMapNode* leaf = grandchild->left;
                                        if (leaf) {
                                            DestroySprayStringLeaf(leaf, context);
                                        }
                                        leaf = grandchild->right;
                                        if (leaf) {
                                            DestroySprayStringLeaf(leaf, context);
                                        }
                                        FreeOwnedStringNode(grandchild);
                                    }
                                    FreeOwnedStringNode(child);
                                }
                                FreeOwnedStringNode(root);
                            }
                        }
                    }
                }
            }
        }
        if (deleteFlag > 0)
            __dl__FPv(context);
    }
    return context;
}
#pragma use_lmw_stmw reset

void FxBoxResourceMap_DestroySubtree(void*, void*);

// Destroy a resource-map node's descendants and owned string before freeing the node.
inline void DestroyResourceStringNode(StringMapNode* node, void* context) {
    if (node->left)
        FxBoxResourceMap_DestroySubtree(context, node->left);
    if (node->right)
        FxBoxResourceMap_DestroySubtree(context, node->right);
    FreeOwnedStringNode(node);
}

#pragma use_lmw_stmw on
// Destroy the resource-map string values and nodes, then optionally free its wrapper.
void* FxBoxResourceMap_dtor(void* context, int deleteFlag) {
    if (context) {
        // Repeated wrapper guards preserve the original destructor expansion.
        if (context) {
            if (context) {
                if (context) {
                    if (context) {
                        if (context) {
                            StringMapNode* root = ((StringMapHeader*)context)->root; // +4
                            if (root) {
                                StringMapNode* child = root->left;
                                if (child) {
                                    StringMapNode* savedLeft8 = child->left;
                                    StringMapNode* grandchild = savedLeft8;
                                    if (grandchild) {
                                        StringMapNode* savedLeft9 = grandchild->left;
                                        StringMapNode* leaf = savedLeft9;
                                        if (leaf)
                                            DestroyResourceStringNode(leaf, context);
                                        leaf = grandchild->right;
                                        if (leaf)
                                            DestroyResourceStringNode(leaf, context);
                                        FreeOwnedStringNode(grandchild);
                                    }
                                    grandchild = child->right;
                                    if (grandchild) {
                                        StringMapNode* leaf = grandchild->left;
                                        if (leaf)
                                            DestroyResourceStringNode(leaf, context);
                                        leaf = grandchild->right;
                                        if (leaf)
                                            DestroyResourceStringNode(leaf, context);
                                        FreeOwnedStringNode(grandchild);
                                    }
                                    FreeOwnedStringNode(child);
                                }
                                StringMapNode* savedRight7 = root->right;
                                child = savedRight7;
                                if (child) {
                                    StringMapNode* grandchild = child->left;
                                    if (grandchild) {
                                        StringMapNode* leaf = grandchild->left;
                                        if (leaf)
                                            DestroyResourceStringNode(leaf, context);
                                        leaf = grandchild->right;
                                        if (leaf)
                                            DestroyResourceStringNode(leaf, context);
                                        FreeOwnedStringNode(grandchild);
                                    }
                                    StringMapNode* savedRight8 = child->right;
                                    grandchild = savedRight8;
                                    if (grandchild) {
                                        StringMapNode* leaf = grandchild->left;
                                        if (leaf)
                                            DestroyResourceStringNode(leaf, context);
                                        leaf = grandchild->right;
                                        if (leaf)
                                            DestroyResourceStringNode(leaf, context);
                                        FreeOwnedStringNode(grandchild);
                                    }
                                    FreeOwnedStringNode(child);
                                }
                                FreeOwnedStringNode(root);
                            }
                        }
                    }
                }
            }
        }
        if (deleteFlag > 0)
            __dl__FPv(context);
    }
    return context;
}
#pragma use_lmw_stmw reset

#pragma use_lmw_stmw on
// Destroy a value-tree node's descendants and owned string before freeing the node.
inline void DestroyValueStringNode(StringMapNode* node, void* context) {
    if (node->left)
        StringValueTree_DestroySubtree(context, node->left);
    if (node->right)
        StringValueTree_DestroySubtree(context, node->right);
    if (&node->stringFlagsAndInlinePrefix) {
        if (&node->stringFlagsAndInlinePrefix && (node->stringFlagsAndInlinePrefix >> 31))
            appFreeUni__FPv(node->heapStringData);
    }
    appFreeUni__FPv(node);
}

// Destroy string-tree values and nodes, then optionally free the containing tree.
void* FxSequenceTemplateMap_dtor(void* context, int deleteFlag) {
    if (context) {
        // Repeated wrapper guards preserve the original destructor expansion.
        if (context) {
            if (context) {
                if (context) {
                    if (context) {
                        if (context) {
                            StringMapNode* root = ((StringMapHeader*)context)->root; // +4
                            if (root) {
                                StringMapNode* savedLeft10 = root->left;
                                StringMapNode* child = savedLeft10;
                                if (child) {
                                    StringMapNode* savedLeft11 = child->left;
                                    StringMapNode* grandchild = savedLeft11;
                                    if (grandchild) {
                                        StringMapNode* savedLeft12 = grandchild->left;
                                        StringMapNode* leaf = savedLeft12;
                                        if (leaf)
                                            DestroyValueStringNode(leaf, context);
                                        StringMapNode* savedRight9 = grandchild->right;
                                        leaf = savedRight9;
                                        if (leaf)
                                            DestroyValueStringNode(leaf, context);
                                        if ((char*)(grandchild) + 0xC) {
                                            if (((char*)(grandchild) + 0xC) &&
                                                (((grandchild)->stringFlagsAndInlinePrefix) >> 31))
                                                appFreeUni__FPv((grandchild)->heapStringData);
                                        }
                                        appFreeUni__FPv(grandchild);
                                    }
                                    StringMapNode* savedRight10 = child->right;
                                    grandchild = savedRight10;
                                    if (grandchild) {
                                        StringMapNode* leaf = grandchild->left;
                                        if (leaf)
                                            DestroyValueStringNode(leaf, context);
                                        leaf = grandchild->right;
                                        if (leaf)
                                            DestroyValueStringNode(leaf, context);
                                        if ((char*)(grandchild) + 0xC) {
                                            if (((char*)(grandchild) + 0xC) &&
                                                (((grandchild)->stringFlagsAndInlinePrefix) >> 31))
                                                appFreeUni__FPv((grandchild)->heapStringData);
                                        }
                                        appFreeUni__FPv(grandchild);
                                    }
                                    if ((char*)(child) + 0xC) {
                                        if (((char*)(child) + 0xC) && (((child)->stringFlagsAndInlinePrefix) >> 31))
                                            appFreeUni__FPv((child)->heapStringData);
                                    }
                                    appFreeUni__FPv(child);
                                }
                                child = root->right;
                                if (child) {
                                    StringMapNode* grandchild = child->left;
                                    if (grandchild) {
                                        StringMapNode* leaf = grandchild->left;
                                        if (leaf)
                                            DestroyValueStringNode(leaf, context);
                                        leaf = grandchild->right;
                                        if (leaf)
                                            DestroyValueStringNode(leaf, context);
                                        if ((char*)(grandchild) + 0xC) {
                                            if (((char*)(grandchild) + 0xC) &&
                                                (((grandchild)->stringFlagsAndInlinePrefix) >> 31))
                                                appFreeUni__FPv((grandchild)->heapStringData);
                                        }
                                        appFreeUni__FPv(grandchild);
                                    }
                                    StringMapNode* savedRight11 = child->right;
                                    grandchild = savedRight11;
                                    if (grandchild) {
                                        StringMapNode* savedLeft13 = grandchild->left;
                                        StringMapNode* leaf = savedLeft13;
                                        if (leaf)
                                            DestroyValueStringNode(leaf, context);
                                        leaf = grandchild->right;
                                        if (leaf)
                                            DestroyValueStringNode(leaf, context);
                                        if ((char*)(grandchild) + 0xC) {
                                            if (((char*)(grandchild) + 0xC) &&
                                                (((grandchild)->stringFlagsAndInlinePrefix) >> 31))
                                                appFreeUni__FPv((grandchild)->heapStringData);
                                        }
                                        appFreeUni__FPv(grandchild);
                                    }
                                    if ((char*)(child) + 0xC) {
                                        if (((char*)(child) + 0xC) && (((child)->stringFlagsAndInlinePrefix) >> 31))
                                            appFreeUni__FPv((child)->heapStringData);
                                    }
                                    appFreeUni__FPv(child);
                                }
                                if ((char*)(root) + 0xC) {
                                    if (((char*)(root) + 0xC) && (((root)->stringFlagsAndInlinePrefix) >> 31))
                                        appFreeUni__FPv((root)->heapStringData);
                                }
                                appFreeUni__FPv(root);
                            }
                        }
                    }
                }
            }
        }
        if (0 < deleteFlag)
            __dl__FPv(context);
    }
    return context;
}
#pragma use_lmw_stmw reset

} // extern "C"

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU80462104", _ctors_u80107180_0, FxBox_InitTwoPiCache);
#pragma pop

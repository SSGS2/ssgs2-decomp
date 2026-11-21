#include <decomp/local_class.h>
// Shot effects: attachment creation, control lifetime, and camera shake/flash.
// Modding index: CFx_CreateAttachedEffect controls script/cache and attachment
// lifetime; CFx_ClearEffects forwards the +4-byte removal policy to
// CFxBox_ClearEffects (fx_box_spray_sequence_registry.cpp); FxShot_AccumulateDistanceWeightedOffsets changes
// the camera transform and flash opacity used by CProjectG_Draw (project_g_app_and_devices.cpp).
// Shared layouts: CFx/CFxBox/CFxSequence headers; scalar externs below identify
// the shot-effect pools by symbol.
#include "game/wvector.h"
#include "game/fxcontrol.h"
#include "classes/CFx.h"
#include "classes/CFxSequence.h"
#include "classes/CFxBox.h"

#define AsCFx(p) ((CFx*)(p))
// Translation unit 0x80102E18-0x80104570 (14 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

extern "C" {

// Ordered-tree node: left/right links, tagged parent, signed/unsigned key, payload.
// Parent bit 0 is balancing metadata; mask it before EVERY parent dereference.
// The address of a tree's root slot doubles as its end sentinel. A root search
// starts at root; an in-order walk starts at first. They are different pointers.
// Control trees own node->value; node-only cleanup never destroys that payload.
struct TreeNode8010 {
    TreeNode8010* left;
    TreeNode8010* right;
    TreeNode8010* parent;
    int key;
    void* value;
};

// Lookup overlay: searches start at roots; shot-offset iteration starts at first.
// +0x16C shotOffsetCount, +0x170 sentinel/root slot, +0x174 firstShotOffset.
// +0x178 sequenceCountStorage, +0x17C sequenceRoot
//   (CFxBox starts at +0x18; its sequenceTreeRoot is +0x164).
// +0x3B0 controlRoot. Its address is the lookup's not-found sentinel.
// Do not start lookup at the shot-offset first node or iterate from a search root.
struct FxLookupView {
    char baseStorage[0x16C];
    unsigned int shotOffsetCount;
    TreeNode8010* shotOffsetRoot;
    TreeNode8010* firstShotOffset;
    char sequenceCountStorage[4];
    TreeNode8010* sequenceRoot;
    char interveningStorage[0x230];
    TreeNode8010* controlRoot;
};

struct FxTimeStepView {
    char baseStorage[0x14];
    float pendingDeltaTime;
};

// Set the pending frame delta consumed by the next effect update.
void CFx_vfunc_2(FxTimeStepView* self, float deltaSeconds) {
    self->pendingDeltaTime = deltaSeconds;
}

void CFxBox_RemoveSequence(void*, unsigned int, int);
void CFxBox_RemoveSpray(void*, unsigned int);
struct Manager8010;
void CFx_UpdateEffects(Manager8010*);
void CFxBox_DrawSprays(void*, void*);
void* CFxSequence_Create(void*, const char*, int, int, int);
void CFxBox_RegisterSequence(void*, void*);
void* CFxSpray_CreateFromScript(void*, const char*, int, int);
void* CFxBox_RegisterSpray(void*, void*);
void CFxBox_UpdateEffects(void*, float);
int strlen(const char*);
int CString_Compare(const char*, const char*);
void RBTree_RebalanceAfterErase(void*, void*);
void appFreeUni__FPv(void*);
void __dl__FPv(void*);
extern void* g_view;
extern unsigned int g_fx;
extern const char __vt__3CFx[];
extern const char shotEffectResourceStrings[];
extern float kFloat_0_shotEffects;

struct Vec2_8010 { float x, y; };
static const float kMillisecondsPerSecond = 1000.0f;
enum { kFullOpacity = 255, kMinimumEffectFilenameLength = 5 };

// The common attachment header is followed by spray- or sequence-specific data.
struct EffectAttachmentHeader { void* vtable; unsigned char attachmentFlags; };
struct AttachedSprayView {
    EffectAttachmentHeader header;
    Vec2_8010* sourcePosition;
    Vec2_8010 lastPosition;
};
typedef char AttachedSpray_source_offset[((int)&((AttachedSprayView*)0)->sourcePosition == 8) ? 1 : -1];
// All local tree descriptors have this ONE contract. The root slot's address
// is end(); firstInOrder points at the minimum key or that end address when empty.
// Tagged parent pointers are an independent representation in TreeNode8010.
struct FxTreeStorage {
    unsigned int count;
    TreeNode8010* searchRoot;
    TreeNode8010* firstInOrder;
};

// Extend the shared CFx view only for embedded storage not yet described there.
struct Manager8010 {
    void* vtable;
    void* secondaryVtable;
    char baseObjectTail[0xC];
    float pendingDeltaTime;
    union {
        char effectBox[0x394];
        struct {
            char boxPrefix[0x37C];
            FxTreeStorage resourceTreeA; // manager +0x394: node-only resource index A.
            FxTreeStorage resourceTreeB; // manager +0x3A0: node-only resource index B.
        } boxTrees;
    };
    unsigned int controlCount;
    TreeNode8010* controlRoot;
    TreeNode8010* firstControl;
};
typedef char Manager_box_offset[((int)&((Manager8010*)0)->effectBox == 0x18) ? 1 : -1];
typedef char Manager_controls_offset[((int)&((Manager8010*)0)->controlCount == 0x3AC) ? 1 : -1];

// Byte-address form preserves CodeWarrior's original box-address lifetimes.
enum { kEmbeddedEffectBoxOffset = (int)&((Manager8010*)0)->effectBox,
       kControlRootOffset = (int)&((Manager8010*)0)->controlRoot };
// Borrow the embedded effect box at manager +0x18.
static inline CFxBox* EffectBoxOf(void* owner) {
    return (CFxBox*)((char*)owner + kEmbeddedEffectBoxOffset);
}


// Create a spray (.spr) or sequence, bind its 2D position, and register it with the box.
// attachmentFlags is stored verbatim in effect byte +4. ClearEffects(mode!=0)
// preserves effects whose WHOLE byte is zero; nonzero values are removed.
// mode==0 removes both groups. This policy applies to sprays and sequences.
// No individual bit meanings are established, so this is not a known bit mask.
// resourceMode is ignored by sequence creation; spray creation passes
// (resourceMode == 0) to its loader. Do not assume the two loaders share a mode enum.
// cacheMode: sequence loader consumes the low byte as bypassResourceCache;
// spray loader takes the whole integer as replace/bypass. Zero permits cache
// reuse/publication; nonzero opens/parses the resource and avoids cache publication.
// For sprays, resourceMode==0 copies a cache hit; nonzero parses it directly.
// sourcePosition is borrowed and stored for future attachment updates; it must
// outlive the effect. The returned effect is registered/owned by the box.
// Null result means disabled attachment, name shorter than 5 bytes, or failed
// creation. Names ending in the exact compared three-byte suffix select sprays;
// all others select sequences. No sourcePosition null check is performed.
void* CFx_CreateAttachedEffect(unsigned char* self, Vec2_8010* sourcePosition, const char* scriptName,
                  unsigned char attachmentFlags, int resourceMode, int cacheMode) {
    if (!*(unsigned char*)&AsCFx(self)->attachedEffectsEnabled)
        return 0;
    int filenameLength = strlen(scriptName);
    if (kMinimumEffectFilenameLength > filenameLength)
        return 0;
    if (CString_Compare(filenameLength + scriptName - 3, shotEffectResourceStrings) == 0) {
        EffectAttachmentHeader* entry = (EffectAttachmentHeader*)CFxSpray_CreateFromScript(self + kEmbeddedEffectBoxOffset, scriptName, resourceMode, cacheMode);
        if (entry == 0)
            return 0;
        entry->attachmentFlags = attachmentFlags;
        float y;
        float x = sourcePosition->x;
        ((AttachedSprayView*)entry)->sourcePosition = sourcePosition;
        y = sourcePosition->y;
        ((AttachedSprayView*)entry)->lastPosition.x = x;
        ((AttachedSprayView*)entry)->lastPosition.y = y;
        CFxBox_RegisterSpray(self + kEmbeddedEffectBoxOffset, entry);
        return entry;
    }
    EffectAttachmentHeader* entry = (EffectAttachmentHeader*)CFxSequence_Create(self + kEmbeddedEffectBoxOffset, scriptName, 1, resourceMode, cacheMode);
    if (entry == 0)
        return 0;
    entry->attachmentFlags = attachmentFlags;
    float y;
    float x = sourcePosition->x;
    (*(Vec2_8010**)&((CFxSequence*)entry)->attachment) = sourcePosition;
    y = sourcePosition->y;
    ((CFxSequence*)entry)->attachmentPositionX = x;
    ((CFxSequence*)entry)->attachmentPositionY = y;
    CFxBox_RegisterSequence(self + kEmbeddedEffectBoxOffset, entry);
    return entry;
}

// Virtual slots +8 and +12 are deleting destruction and per-frame update.
struct Obj8010 { 
virtual void destroy(int); virtual int update();
};

// Update controls, delete completed controls, then consume pending seconds as box milliseconds.
// A control update returns nonzero to remain live, zero to remove and destroy.
// Early exits preserve pendingDeltaTime; a completed pass consumes it to zero.
void CFx_UpdateEffects(Manager8010* self) {
    TreeNode8010* node;
    TreeNode8010* end;
    void* item;
    TreeNode8010* original;
    if (!*(unsigned char*)&AsCFx(self)->updateEnabled || *(unsigned char*)&AsCFx(self)->paused ||
        kFloat_0_shotEffects == self->pendingDeltaTime)
        return;
    ((Obj8010*)(EffectBoxOf(self)))->update();
    node = self->firstControl;
    end = (TreeNode8010*)((char*)self + (int)&((Manager8010*)0)->controlRoot);
    while (end != node) {
        item = node->value;
        int keep = ((Obj8010*)item)->update();
        if (!keep) {
            // Cache the successor BEFORE rebalancing/freeing the current node.
            // Successor = leftmost of right subtree, else first ancestor reached
            // from its left child (mask parent tag on each climb).
            original = node;
            if (original->right) {
                TreeNode8010* next = original->right;
                while (next->left) next = next->left;
                node = next;
            } else {
                while (node != ((TreeNode8010*)((unsigned int)node->parent & ~1u))->left) {
                    node = (TreeNode8010*)((unsigned int)node->parent & ~1u);
                }
                node = (TreeNode8010*)((unsigned int)node->parent & ~1u);
            }
            if (original == self->firstControl) {
                if (original->right) {
                    TreeNode8010* t = original->right;
                    while (t->left) t = t->left;
                    self->firstControl = t;
                } else {
                    TreeNode8010* c = original;
                    while (c != ((TreeNode8010*)((unsigned int)c->parent & ~1u))->left) {
                        c = (TreeNode8010*)((unsigned int)c->parent & ~1u);
                    }
                    self->firstControl = (TreeNode8010*)((unsigned int)c->parent & ~1u);
                }
            }
            // Repair cached first, unlink/rebalance, free node, then destroy value.
            // item remains valid after its tree node has been released.
            RBTree_RebalanceAfterErase(original, self->controlRoot);
            appFreeUni__FPv(original);
            self->controlCount--;
            if (item != 0) {
                ((Obj8010*)item)->destroy(1);
            }
        } else {
            if (node->right) {
                TreeNode8010* next = node->right;
                while (next->left) next = next->left;
                node = next;
            } else {
                while (node != ((TreeNode8010*)((unsigned int)node->parent & ~1u))->left) {
                    node = (TreeNode8010*)((unsigned int)node->parent & ~1u);
                }
                node = (TreeNode8010*)((unsigned int)node->parent & ~1u);
            }
        }
    }
    // Controls have finished; the embedded box takes milliseconds, not seconds.
    CFxBox_UpdateEffects(EffectBoxOf(self), kMillisecondsPerSecond * self->pendingDeltaTime);
    self->pendingDeltaTime = kFloat_0_shotEffects;
}

// Forward the manager render/update hook to its embedded effect box.
void CFx_UpdateAndDraw(char* self) {
    CFx_UpdateEffects((Manager8010*)self);
    CFxBox_DrawSprays(EffectBoxOf(self), g_view);
}

void FxTree_DestroySubtreeNodes(void*, void*);
void FxBoxResourceTree_FreeSubtreeNodes(void*, void*);
void CFxBox_ResetResources(void*);
void CFxBox_ClearEffects(void*, void*);
void CFxBox_dtor(void*, int);
void BaseObject_dtor(void*, int);

// Postorder cleanup vocabulary used by the destructor below. These expansions
// preserve declaration scopes required by the baseline compiler (inline functions
// were tested and changed register allocation). tree is a descriptor address;
// node must be a plain pointer variable, never an expression with side effects.
// freeSubtree releases deeper links only; payload destruction is always separate.
// Release descendants through the existing helper, then release this node.
#define FX_FREE_NODE_AND_SUBTREES(tree, node, freeSubtree) \
    if (node) { \
        if (node->left) freeSubtree(tree, node->left); \
        if (node->right) freeSubtree(tree, node->right); \
        appFreeUni__FPv(node); \
    }
// Expand two levels in left/right/node order; helper owns all deeper links.
#define FX_FREE_TWO_LEVELS(tree, node, freeSubtree) \
    if (node) { \
        TreeNode8010* node##Left; \
        node##Left = node->left; \
        FX_FREE_NODE_AND_SUBTREES(tree, node##Left, freeSubtree) \
        TreeNode8010* node##Right; \
        node##Right = node->right; \
        FX_FREE_NODE_AND_SUBTREES(tree, node##Right, freeSubtree) \
        appFreeUni__FPv(node); \
    }
// Expand three levels while retaining each left/right temporary's lexical scope.
#define FX_FREE_THREE_LEVELS(tree, node, freeSubtree) \
    if (node) { \
        TreeNode8010* node##Left; \
        node##Left = node->left; \
        FX_FREE_TWO_LEVELS(tree, node##Left, freeSubtree) \
        TreeNode8010* node##Right; \
        node##Right = node->right; \
        FX_FREE_TWO_LEVELS(tree, node##Right, freeSubtree) \
        appFreeUni__FPv(node); \
    }
// Free the whole tree: four expanded levels, then the existing subtree helper.
#define FX_FREE_FOUR_LEVELS(tree, node, freeSubtree) \
    if (node) { \
        TreeNode8010* node##Left; \
        node##Left = node->left; \
        FX_FREE_THREE_LEVELS(tree, node##Left, freeSubtree) \
        TreeNode8010* node##Right; \
        node##Right = node->right; \
        FX_FREE_THREE_LEVELS(tree, node##Right, freeSubtree) \
        appFreeUni__FPv(node); \
    }

#pragma push
#pragma opt_common_subs off
#pragma use_lmw_stmw on
struct Owned8010 { 
virtual ~Owned8010();
};

// Delete live controls and free their trees before destroying the box and singleton base.
// Destruction audit ledger (manager-relative addresses; arrows indicate order):
//   +3B4 firstControl -> each owned control value (virtual deleting destructor)
//   +3AC control descriptor -> nodes only -> {count=0, root=0, first=&root}
//   +3AC embedded descriptor -> redundant node-only destructor layers
//   +3A0 resource index B -> nodes only (payload lifetimes remain external)
//   +394 resource index A -> nodes only (payload lifetimes remain external)
//   +018 effect box -> +004 singleton base -> manager allocation if requested
// Each expanded subtree below follows L then R then node. Variables encode
// paths (RootLeftRight = root->left->right); subtree helpers take over at depth 4.
// The repeated address tests are ABI destructor layers, not extra resources.
// Inline extraction was checked against the baseline object and changed register
// allocation, so these statement scopes retain their original expansion.
// Ownership order: (1) control payloads, (2) control nodes, (3) embedded control
// destructor expansion, (4) resource-index B nodes, (5) resource-index A nodes,
// (6) effect box, (7) singleton base, (8) self only if shouldDelete > 0.
// The expanded frees are postorder: left descendants, right descendants, node.
// Remaining deeper descendants delegate to subtree helpers. Address guards are
// compiler-matched destructor layers, not independent nullable allocations.
void* CFx_Dtor(Manager8010* self, int shouldDelete) {
    TreeNode8010* controlRootOrCursor;
    TreeNode8010* controlLeft;
    TreeNode8010* controlLeftLeft;
    TreeNode8010* controlLeftLeftLeft;
    TreeNode8010* endNode;
    if (self) {
        controlRootOrCursor = self->firstControl;
        self->vtable = (void*)__vt__3CFx;
        self->secondaryVtable = (void*)(__vt__3CFx + 0xC);
        endNode = (TreeNode8010*)((char*)self + (int)&((Manager8010*)0)->controlRoot);
        // Phase 1: destroy payloads while retaining links for successor traversal.
        while (controlRootOrCursor != endNode) {
            delete (Owned8010*)controlRootOrCursor->value;
            TreeNode8010* next = controlRootOrCursor->right;
            if (next) {
                TreeNode8010* child;
                while ((child = next->left) != 0) next = child;
                controlRootOrCursor = next;
            } else {
                while (controlRootOrCursor != (next = (TreeNode8010*)((unsigned int)controlRootOrCursor->parent & ~1u))->left)
                controlRootOrCursor = next;
                controlRootOrCursor = next;
            }
        }
        // Phase 2: payloads are gone; free only the control tree nodes and reset
        // its root/count/first sentinel. Names below encode paths from this root.
        controlRootOrCursor = self->controlRoot;
        if (controlRootOrCursor) {
            controlLeft = controlRootOrCursor->left;
            if (controlLeft) {
                controlLeftLeft = controlLeft->left;
                if (controlLeftLeft) {
                    controlLeftLeftLeft = controlLeftLeft->left;
                    FX_FREE_NODE_AND_SUBTREES(&self->controlCount, controlLeftLeftLeft, FxTree_DestroySubtreeNodes);
                    TreeNode8010* controlLeftLeftRight;
                    controlLeftLeftRight = controlLeftLeft->right;
                    FX_FREE_NODE_AND_SUBTREES(&self->controlCount, controlLeftLeftRight, FxTree_DestroySubtreeNodes);
                    appFreeUni__FPv(controlLeftLeft);
                }
                TreeNode8010* controlLeftRight;
                controlLeftRight = controlLeft->right;
                if (controlLeftRight) {
                    TreeNode8010* controlLeftRightLeft;
                    controlLeftRightLeft = controlLeftRight->left;
                    FX_FREE_NODE_AND_SUBTREES(&self->controlCount, controlLeftRightLeft, FxTree_DestroySubtreeNodes);
                    TreeNode8010* controlLeftRightRight;
                    controlLeftRightRight = controlLeftRight->right;
                    FX_FREE_NODE_AND_SUBTREES(&self->controlCount, controlLeftRightRight, FxTree_DestroySubtreeNodes);
                    appFreeUni__FPv(controlLeftRight);
                }
                appFreeUni__FPv(controlLeft);
            }
            // Left half is now released; release the right half before its root.
            TreeNode8010* controlRight;
            controlRight = controlRootOrCursor->right;
            FX_FREE_THREE_LEVELS(&self->controlCount, controlRight, FxTree_DestroySubtreeNodes);

            appFreeUni__FPv(controlRootOrCursor);
            self->controlCount = 0;
            self->controlRoot = 0;
            self->firstControl = (TreeNode8010*)((char*)self + (int)&((Manager8010*)0)->controlRoot);
        }
        // Phase 3: embedded control-tree destructor. Phase 2 left root==0, so
        // this node-only expansion normally has nothing further to release.
        if (&self->controlCount)
        if (&self->controlCount)
        if (&self->controlCount)
        if (&self->controlCount)
        if (&self->controlCount)
        if (&self->controlCount) {
            TreeNode8010* embeddedControlsRoot;
            embeddedControlsRoot = (*(TreeNode8010**)&AsCFx(self)->controlRoot);
            FX_FREE_FOUR_LEVELS(&self->controlCount, embeddedControlsRoot, FxTree_DestroySubtreeNodes);
        }
        // Phases 4/5: release B then A indexing nodes, without deleting values.
        // Payload/resource lifetime belongs to the effect box's own cleanup.
        if (EffectBoxOf(self)) {
            if (&self->boxTrees.resourceTreeB)
            if (&self->boxTrees.resourceTreeB)
            if (&self->boxTrees.resourceTreeB)
            if (&self->boxTrees.resourceTreeB)
            if (&self->boxTrees.resourceTreeB)
            if (&self->boxTrees.resourceTreeB) {
                TreeNode8010* resourceBRoot;
                resourceBRoot = self->boxTrees.resourceTreeB.searchRoot;
                FX_FREE_FOUR_LEVELS(&self->boxTrees.resourceTreeB, resourceBRoot, FxBoxResourceTree_FreeSubtreeNodes);
            }
            // Phase 5: B nodes are gone; the same postorder algorithm frees A nodes.
            if (&self->boxTrees.resourceTreeA)
            if (&self->boxTrees.resourceTreeA)
            if (&self->boxTrees.resourceTreeA)
            if (&self->boxTrees.resourceTreeA)
            if (&self->boxTrees.resourceTreeA)
            if (&self->boxTrees.resourceTreeA) {
                TreeNode8010* resourceARoot;
                resourceARoot = self->boxTrees.resourceTreeA.searchRoot;
                FX_FREE_FOUR_LEVELS(&self->boxTrees.resourceTreeA, resourceARoot, FxBoxResourceTree_FreeSubtreeNodes);
            }
            // Phase 6: finish the embedded box after its resource indices.
            CFxBox_dtor(EffectBoxOf(self), 0);
        }
        // Phase 7: detach the singleton before the base object is torn down.
        if (&self->secondaryVtable) {
            g_fx = 0;
            BaseObject_dtor(&self->secondaryVtable, 0);
        }
        if (0 < shouldDelete) __dl__FPv(self);
    }
    return self;
}
#undef FX_FREE_NODE_AND_SUBTREES
#undef FX_FREE_TWO_LEVELS
#undef FX_FREE_THREE_LEVELS
#undef FX_FREE_FOUR_LEVELS
#pragma pop

#pragma push
#pragma use_lmw_stmw on
struct Item8010 { 
virtual void destroy(int);
};

// Destroy registered controls and clear box effects using the supplied clear mode.
// As in CFx_Dtor, destroy all payloads first, then free nodes and restore the
// embedded sentinel. clearMode is forwarded to CFxBox_ClearEffects: nonzero
// preserves effects with attachment byte +4 == 0 and removes nonzero-byte effects;
// zero removes all box effects. Controls are destroyed regardless of clearMode.
void CFx_ClearEffects(Manager8010* self, void* clearMode) {
    TreeNode8010* root;
    void* item;
    // Phase 1: retain links while destroying values in ascending key order.
    TreeNode8010* node = self->firstControl;
    TreeNode8010* controlEnd = (TreeNode8010*)((char*)self + (int)&((Manager8010*)0)->controlRoot);
    while (node != controlEnd) {
        item = node->value;
        if (item) {
            ((Item8010*)item)->destroy(1);
        }
        TreeNode8010* next = node->right;
        if (next) {
            while (next->left) next = next->left;
            node = next;
        } else {
            TreeNode8010* parent;
            while (node != (parent = (TreeNode8010*)((unsigned int)node->parent & ~1u))->left) {
                node = parent;
            }
            node = parent;
        }
    }

// Keep these cleanup expansions: inline helpers change register assignment.
#define CLEAN_NODE(c) \
    if (c) { \
        if (c->left) FxTree_DestroySubtreeNodes(&self->controlCount, c->left); \
        if (c->right) FxTree_DestroySubtreeNodes(&self->controlCount, c->right); \
        appFreeUni__FPv(c); \
    }
#define CLEAN_PAIR(p) \
    if (p) { \
        { TreeNode8010* c = p->left; CLEAN_NODE(c) } \
        { TreeNode8010* c = p->right; CLEAN_NODE(c) } \
        appFreeUni__FPv(p); \
    }
#define CLEAN_BRANCH(p) \
    if (p) { \
        { TreeNode8010* b = p->left; CLEAN_PAIR(b) } \
        { TreeNode8010* b = p->right; CLEAN_PAIR(b) } \
        appFreeUni__FPv(p); \
    }
    // Phase 2: values are gone; free nodes and restore the empty sentinel.
    root = self->controlRoot;
    if (root) {
        { TreeNode8010* a = root->left; CLEAN_BRANCH(a) }
        { TreeNode8010* a = root->right; CLEAN_BRANCH(a) }
        appFreeUni__FPv(root);
        self->controlCount = 0;
        self->controlRoot = 0;
        self->firstControl = (TreeNode8010*)&self->controlRoot;
    }
#undef CLEAN_NODE
#undef CLEAN_PAIR
#undef CLEAN_BRANCH
    CFxBox_ClearEffects(EffectBoxOf(self), clearMode);
}
#pragma pop

// Delete all controls and reset the box resources for reuse.
void CFx_ClearControlsAndResetBox(Manager8010* self) {
    struct Item {
        virtual ~Item() DECOMP_LOCAL_VIRTUAL;
    };
// Keep the same three-level expansion used by the matched control cleanup.
#define CLEAN_TARGET_NODE(c) \
    if (c) { \
        if (c->left) FxTree_DestroySubtreeNodes(&self->controlCount, c->left); \
        if (c->right) FxTree_DestroySubtreeNodes(&self->controlCount, c->right); \
        appFreeUni__FPv(c); \
    }
#define CLEAN_TARGET_PAIR(p) \
    if (p) { \
        { TreeNode8010* c = p->left; CLEAN_TARGET_NODE(c) } \
        { TreeNode8010* c = p->right; CLEAN_TARGET_NODE(c) } \
        appFreeUni__FPv(p); \
    }
#define CLEAN_TARGET_BRANCH(p) \
    if (p) { \
        { TreeNode8010* b = p->left; CLEAN_TARGET_PAIR(b) } \
        { TreeNode8010* b = p->right; CLEAN_TARGET_PAIR(b) } \
        appFreeUni__FPv(p); \
    }
    TreeNode8010* controlEnd = (TreeNode8010*)((char*)self + (int)&((Manager8010*)0)->controlRoot);
    // Phase 1: retain links while destroying values in ascending key order.
    TreeNode8010* node = self->firstControl;
    while (controlEnd != node) {
        Item* item = (Item*)node->value;
        delete item;
        TreeNode8010* next = node->right;
        if (next) {
            while (next->left) next = next->left;
            node = next;
        } else {
            TreeNode8010* parent;
            while (node != (parent = (TreeNode8010*)((unsigned int)node->parent & ~1u))->left) {
                node = parent;
            }
            node = parent;
        }
    }

    // Phase 2: release links only, then reset the embedded box for reuse.
    node = *(TreeNode8010**)((char*)self + kControlRootOffset);
    if (node) {
        { TreeNode8010* a = node->left; CLEAN_TARGET_BRANCH(a) }
        { TreeNode8010* a = node->right; CLEAN_TARGET_BRANCH(a) }
        appFreeUni__FPv(node);
        self->controlCount = 0;
        self->controlRoot = 0;
        self->firstControl = (TreeNode8010*)((unsigned int)self + (int)&((Manager8010*)0)->controlRoot);
    }
    CFxBox_ResetResources(EffectBoxOf(self));
#undef CLEAN_TARGET_NODE
#undef CLEAN_TARGET_PAIR
#undef CLEAN_TARGET_BRANCH
}

}
// Empty control base destructor; derived controls own their specific resources.
CFxControl::~CFxControl() {}
extern "C" {

// Create and register a sequence; return null if script creation fails.
// The box owns a successful return; the caller borrows it until removal/clear.
// playbackMode is forwarded unchanged to the sequence constructor.
void* CFx_CreateSequence(char* self, const char* scriptName, int playbackMode) {
    void* sequence = CFxSequence_Create(self + kEmbeddedEffectBoxOffset, scriptName, playbackMode, 0, 0);
    if (sequence != 0) {
        CFxBox_RegisterSequence(self + kEmbeddedEffectBoxOffset, sequence);
    }
    return sequence;
}

// Remove the sequence identified by key with the requested removal mode.
void CFx_RemoveSequence(char* self, unsigned int key, int mode) {
    CFxBox_RemoveSequence(EffectBoxOf(self), key, mode);
}

// Remove a spray from the embedded box by its registered ID.
void CFx_RemoveSpray(char* self, unsigned int sprayId) {
    CFxBox_RemoveSpray(EffectBoxOf(self), sprayId);
}

// Test whether the sequence tree contains an exact unsigned ID match.
int CFx_HasSequence(FxLookupView* self, unsigned int key) {
    TreeNode8010* node = self->sequenceRoot;
    TreeNode8010* result = (TreeNode8010*)&self->sequenceRoot;
    while (node != 0) {
        if ((unsigned int)node->key >= key) {
            result = node;
            node = node->left;
        } else {
            node = node->right;
        }
    }
    if (result == (TreeNode8010*)&self->sequenceRoot || key < (unsigned int)result->key)
        result = (TreeNode8010*)&self->sequenceRoot;
    return result != (TreeNode8010*)&self->sequenceRoot;
}

// Return the control stored under a signed ID, or null when absent.
void* Map_FindByKey(FxLookupView* self, int key) {
    TreeNode8010* node = self->controlRoot;
    TreeNode8010* result = (TreeNode8010*)&self->controlRoot;
    while (node != 0) {
        if (node->key >= key) {
            result = node;
            node = node->left;
        } else {
            node = node->right;
        }
    }
    if (result == (TreeNode8010*)&self->controlRoot || key < result->key)
        result = (TreeNode8010*)&self->controlRoot;
    if (result != (TreeNode8010*)&self->controlRoot)
        return result->value;
    return 0;
}

void __mi__FRC7WVectorRC7WVector(void*, void*, void*);
float Magnitude__7WVectorCFv(void*);
void Vec3_DivideScalar(void*, float, void*);
void __apl__7WVectorFRC7WVector(void*, void*);
extern float kFloat_1_FxShot;

struct ShotOffsetView {
    char effectHeader[0x94];
    WVector displacement;
    WVector origin;
    unsigned char opacity;
};

// Add distance-attenuated displacements and saturating opacity from active shot effects.
// CProjectG_Draw passes the fourth three-float vector of s_cameraTransform (+0x24),
// then submits that modified transform to WView_SetViewMatrix. Thus this edits
// the camera transform's translation vector, not a projected screen position.
// Matrix_BuildCameraTransform (wview_camera_projection.cpp) stores world-space eye position in
// this vector; WView_SetViewMatrix queues that frame before the view's later
// inversion. Origins/displacements therefore use world coordinates, with X/Y/Z
// in the camera frame's game-distance scale (Y is the up hint in its builder).
// No projection, camera rotation, yard conversion or units conversion occurs.
// position and opacity are required writable borrowed outputs, unchanged on an
// empty tree; neither may alias effect storage. For each effect:
// position += displacement/(1+distance(position,origin)); opacity saturates at 255.
// Later effects see the already modified position, so accumulation order matters.
// Return whether any shot-offset node existed, even if it contributed zero.
int FxShot_AccumulateDistanceWeightedOffsets(FxLookupView* self, void* position, unsigned char* opacity) {
    TreeNode8010* node = self->firstShotOffset;
    char* offsetEffect;
    TreeNode8010* shotOffsetEnd = (TreeNode8010*)&self->shotOffsetRoot;
    float unitDistance = kFloat_1_FxShot;
    while (node != shotOffsetEnd) { unsigned char weightedDisplacement[12];
        offsetEffect = (char*)node->value;

        WVector offsetFromOrigin = *(WVector*)position - ((ShotOffsetView*)offsetEffect)->origin;
        Vec3_DivideScalar(weightedDisplacement, unitDistance + offsetFromOrigin.Magnitude(), &((ShotOffsetView*)offsetEffect)->displacement);
        *(WVector*)position += *(WVector*)weightedDisplacement;
        int sum = *opacity + ((ShotOffsetView*)offsetEffect)->opacity;
        *opacity = sum > kFullOpacity ? kFullOpacity : sum;
        TreeNode8010* child = node->right;
        if (child != 0) {
            while (child->left != 0)
                child = child->left;
            node = child;
        } else {
            TreeNode8010* parent;
            while (node != (parent = (TreeNode8010*)((unsigned int)node->parent & ~1u))->left) {
                node = parent;
            }
            node = parent;
        }
    }
    return 0 != self->shotOffsetCount;
}

} // extern "C"

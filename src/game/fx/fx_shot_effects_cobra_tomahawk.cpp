#include <decomp/local_class.h>
#include "classes/CFxTomahawk.h"
#include "classes/CFxSpike.h"
#include "classes/CFxPowerShot2.h"
#include "classes/CFxBoosterWarp.h"
#include "classes/CFxSequence.h"
#include "classes/CFx.h"
#include "game/types.h"
#include "game/wvector.h"
#include "game/fxcontrol.h"
// Translation unit 0x80104570-0x80105A48 (19 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

extern "C" {

extern void FxTree_DestroySubtreeNodes(void*, int);
extern void appFreeUni__FPv(void*);
extern void CFxBox_ClearEffects(void*, int);
extern void* __nw__FUl(unsigned int);
extern char __vt__14CFxBoosterWarp[];
extern char __vt__11CFxTomahawk[];
// Original string-pool layout; arrays include terminators and alignment bytes.
struct ShotEffectScriptNames {
    char sprayExtension[4];
    char groundRun[14];
    char waterRun[13];
    char spikeGround[12];
    char spikeBall[15];
    char boosterTwist[10];
    char boosterBurst[20];
    char cobraGround[13];
    char cobraWater[13];
    char cobraRing[13];
    char cobraBall[12];
    char cobraLaunch[12];
};
typedef char ShotNames_cobra_launch[((int)&((ShotEffectScriptNames*)0)->cobraLaunch == 0x8B) ? 1 : -1];
enum { kCobraSequenceCount = 4, kCobraFlightFlag = 0x20, kEffectBoxOffset = 0x18, kControlTreeOffset = 0x3AC };
// Control IDs are serial numbers, not pointers. Creation stores an owned control
// in the manager's tree; callers retain only the ID passed to CFx_RemoveControl.
// Ball position/velocity arguments are borrowed and must outlive the control.
// Tree nodes: left/right at +0/+4, parent with colour in bit 0 at +8,
// integer ID at +0xC, owned control at +0x10. The root-pointer slot is the end
// sentinel; the header's final pointer is the first node used for traversal.
extern ShotEffectScriptNames shotEffectResourceStrings;
extern void* CFxSequence_Create(void*, void*, int, int, int);
extern void CFxBox_RegisterSequence(void*, void*);
extern void* CFxEffectTree_FindInsertionPosition(void*, int*, void**, unsigned char*, unsigned char*);
extern void FxSprayTree_InsertNode(void*, void*, unsigned char, unsigned char, int*);

struct TreeNode_80104CEC { TreeNode_80104CEC* left; TreeNode_80104CEC* right; TreeNode_80104CEC* parent; int key; void* control; };
// Construction view of CFxBoosterWarp; canonical members: classes/CFxBoosterWarp.h.
// Retains CFxControl base construction and its exception cleanup.
struct CFxBoosterWarpConstructorView : CFxControl {
    void* parent; int result; void* a; void* b; unsigned char flag;
    // Create/register twist and burst; leave burst playback disabled initially.
    CFxBoosterWarpConstructorView(void* self, int value) {
        vtable = &__vt__14CFxBoosterWarp;
        parent = (char*)self + kEffectBoxOffset;
        result = value;
        flag = 0;
        a = CFxSequence_Create(parent, shotEffectResourceStrings.boosterTwist, 0, 0, 0);
        b = CFxSequence_Create(parent, shotEffectResourceStrings.boosterBurst, 0, 0, 0);
        if (a && b) {
            CFxBox_RegisterSequence(parent, a);
            CFxBox_RegisterSequence(parent, b);
            if ((*(unsigned char*)&((CFxSequence*)b)->playbackEnabled))
                (*(unsigned char*)&((CFxSequence*)b)->playbackEnabled) = 0;
        }
    }
};
struct Tree_80104CEC { void* head; void* root; void* firstNode; };
struct Owner_80104CEC { char baseStorage[0x18]; char effectBox[0x394]; Tree_80104CEC tree; };
struct Virtual_80104CEC { 
virtual void destroy(int);
};

// Free descendant tree nodes and then this node; controls were destroyed earlier.
inline void FreeControlTreeBranch(Owner_80104CEC* owner, TreeNode_80104CEC* node) {
    if (node->left) FxTree_DestroySubtreeNodes(&owner->tree, (int)node->left);
    if (node->right) FxTree_DestroySubtreeNodes(&owner->tree, (int)node->right);
    appFreeUni__FPv(node);
}

#pragma push
#pragma use_lmw_stmw on
#pragma opt_lifetimes off
// Clear current effects and install a booster-warp control with twist and burst sequences.
int CFx_ResetAndCreateBoosterWarp(void* self, int value) {
    TreeNode_80104CEC *sentinel;
    TreeNode_80104CEC *controlNode;
    TreeNode_80104CEC *c;
    unsigned char updateFirst;
    TreeNode_80104CEC *b;
    TreeNode_80104CEC *root;
    Owner_80104CEC* owner = (Owner_80104CEC*)self;
    
    sentinel = (TreeNode_80104CEC*)(&((Owner_80104CEC*)self)->tree.root);
    controlNode = (TreeNode_80104CEC*)owner->tree.firstNode;
    
    // First destroy control payloads, keeping nodes valid for in-order traversal.
    while (controlNode != sentinel) {
        if ((*(Virtual_80104CEC**)&controlNode->control))
            ((*(Virtual_80104CEC**)&controlNode->control))->destroy(1);
        TreeNode_80104CEC* successor = controlNode->right;
        if (!(!(successor))) {
            while (successor->left) successor = successor->left;
            controlNode = successor;
        } else {
            TreeNode_80104CEC* parent;
            while (controlNode != (parent = (TreeNode_80104CEC*)((unsigned int)controlNode->parent & ~1u))->left)
                controlNode = parent;
            controlNode = parent;
        }
    }
    // Then free the node hierarchy and restore the empty-tree sentinel.
    void* savedRoot = owner->tree.root;
    root = (TreeNode_80104CEC*)savedRoot;
    if (root) {
        TreeNode_80104CEC* savedLeft = root->left;
        TreeNode_80104CEC *a;
        a=savedLeft;
        if(a) {
            TreeNode_80104CEC* savedLeft2 = a->left;
            b=savedLeft2;
            if(b) {
                TreeNode_80104CEC* savedLeft3 = b->left;
                c=savedLeft3;
                if (c) FreeControlTreeBranch((Owner_80104CEC*)self, c);
                TreeNode_80104CEC* savedRight = b->right;
                c=savedRight;
                if (c) FreeControlTreeBranch((Owner_80104CEC*)self, c);
                TreeNode_80104CEC * savedNode = b;
                appFreeUni__FPv(savedNode);
            }
            TreeNode_80104CEC* savedRight2 = a->right;
            c=savedRight2;
            if(c) {
                TreeNode_80104CEC* savedLeft4 = c->left;
                b=savedLeft4;
                if (b) FreeControlTreeBranch((Owner_80104CEC*)self, b);
                TreeNode_80104CEC* savedRight3 = c->right;
                b=savedRight3;
                if (b) FreeControlTreeBranch((Owner_80104CEC*)self, b);
                TreeNode_80104CEC * savedNode2 = c;
                appFreeUni__FPv(savedNode2);
            }
            TreeNode_80104CEC * savedNode3 = a;
            appFreeUni__FPv(savedNode3);
        }
        TreeNode_80104CEC* savedRight4 = root->right;
        c=savedRight4;
        if(c) {
            a=c->left;
            if(a) {
                TreeNode_80104CEC* savedLeft5 = a->left;
                b=savedLeft5;
                if (b) FreeControlTreeBranch((Owner_80104CEC*)self, b);
                TreeNode_80104CEC* savedRight5 = a->right;
                b=savedRight5;
                if (b) FreeControlTreeBranch((Owner_80104CEC*)self, b);
                TreeNode_80104CEC * savedNode4 = a;
                appFreeUni__FPv(savedNode4);
            }
            TreeNode_80104CEC* savedRight6 = c->right;
            a=savedRight6;
            if(a) {
                TreeNode_80104CEC* savedLeft6 = a->left;
                b=savedLeft6;
                if (b) FreeControlTreeBranch((Owner_80104CEC*)self, b);
                TreeNode_80104CEC* savedRight7 = a->right;
                b=savedRight7;
                if (b) FreeControlTreeBranch((Owner_80104CEC*)self, b);
                TreeNode_80104CEC * savedNode5 = a;
                appFreeUni__FPv(savedNode5);
            }
            TreeNode_80104CEC * savedNode6 = c;
            appFreeUni__FPv(savedNode6);
        }
        appFreeUni__FPv(root);
        owner->tree.head = 0;
        owner->tree.root = 0;
        owner->tree.firstNode = &owner->tree.root;
    }
    // Clear the effect box only after control destruction, then install the new ID.
    CFxBox_ClearEffects(&owner->effectBox, 1);
    CFxBoosterWarpConstructorView* control = new CFxBoosterWarpConstructorView(self, value);
    struct { int key; void* object; } search;
    void* node;
    search.key = control->CFxControl::result;
    search.object = control;
    unsigned char insertLeft;
    void* insertionParent = CFxEffectTree_FindInsertionPosition((char*)self + kControlTreeOffset, &search.key, &node, &insertLeft, &updateFirst);
    if (node == 0 || ((TreeNode_80104CEC*)node)->key < search.key)
        FxSprayTree_InsertNode(&owner->tree, insertionParent, insertLeft, updateFirst, &search.key);
    return *(int*)((char*)control + 4);
}
#pragma pop

typedef CFx Struct_80105130;

// Enable or suppress effect-manager updates.
void CFx_SetUpdateEnabled(Struct_80105130* self, unsigned char value) { self->updateEnabled = value; }
// Pause effect simulation without discarding live effects.
void CFx_SetPaused(Struct_80105130* self, unsigned char value) { self->paused = value; }
// Allow or reject creation of new attached effects.
void CFx_SetAttachedEffectCreationEnabled(Struct_80105130* self, unsigned char value) { self->attachedEffectsEnabled = value; }

struct Child_801055E4 {
    virtual void vf(int);
};

struct Struct_801055E4 {
    void* vtable;
    char basePadding[4];
    void* owner;
    char keyStorage[4];
    Child_801055E4* children[4];
};

extern void CFxBox_RemoveSequence(void*, Child_801055E4*, int);
extern char __vt__8CFxCobra[];

#pragma use_lmw_stmw on
// Unregister and destroy the four Cobra sequences, then optionally free the control.
Struct_801055E4* CFxCobra_dtor(Struct_801055E4* self, int shouldDelete) {
    if (self != 0) {
        self->vtable = __vt__8CFxCobra;
        for (unsigned int i = 0; i < 4; ++i) {
            CFxBox_RemoveSequence(self->owner, self->children[i], 0);
            if (self->children[i] != 0) {
                self->children[i]->vf(1);
            }
            self->children[i] = 0;
        }
        if (shouldDelete > 0) delete self;
    }
    return self;
}
#pragma use_lmw_stmw reset

struct Struct_801059AC {
    void* vtable;
    char basePadding[4];
    void* owner;
    char keyStorage[4];
    Child_801055E4* child;
};

// Unregister and destroy the owned Tomahawk sequence.
Struct_801059AC* CFxTomahawk_dtor(Struct_801059AC* self, int shouldDelete) {
    if (self != 0) {
        self->vtable = __vt__11CFxTomahawk;
        if (self->child != 0) {
            CFxBox_RemoveSequence(self->owner, self->child, 0);
            if (self->child != 0) {
                ((Child_801055E4*)self->child)->vf(1);
            }
            self->child = 0;
        }
        if (shouldDelete > 0) delete self;
    }
    return self;
}

struct CFxCobra : CFxControl {
    void* owner;
    int value;
    void* children[4];
    unsigned char flag;
    CFxCobra(void* o, int v);
};

extern void* CFxEffectTree_FindInsertionPosition(void*, int*, void**, unsigned char*, unsigned char*);
extern void FxSprayTree_InsertNode(void*, void*, unsigned char, unsigned char, int*);
extern char __vt__13CFxPowerShot2[];
extern int s_fxInstanceSerial;

// Construction view of CFxPowerShot2; canonical members: classes/CFxPowerShot2.h.
// Retains the serial word at +4, represented as padding in the shared header.
struct CFxPowerShot2ConstructorView {
    void* vtable;
    int result;
    unsigned char initialized;
    char flagPadding[3];
    void* parent;
    int value;
    // Bind PowerShot2 to its effect box/argument and allocate a serial ID.
    CFxPowerShot2ConstructorView(void* p, int v) {
        vtable = __vt__13CFxPowerShot2;
        parent = (char*)p + kEffectBoxOffset;
        value = v;
        result = s_fxInstanceSerial++;
        initialized = 0;
    }
};

// Allocate a PowerShot2 control, insert its serial ID, and return that ID.
int CFx_CreatePowerShot2(void* self, int value) {
    CFxPowerShot2ConstructorView* object = new CFxPowerShot2ConstructorView(self, value);
    struct { int key; void* object; } search;
    void* inserted;
    void* node;
    unsigned char flagB, flagA;
    search.key = object->result;
    search.object = object;
    inserted = CFxEffectTree_FindInsertionPosition((char*)self + kControlTreeOffset, &search.key, &node, &flagA, &flagB);
    if (node == 0 || ((TreeNode_80104CEC*)node)->key < search.key)
        FxSprayTree_InsertNode(&((Owner_80104CEC*)self)->tree, inserted, flagA, flagB, &search.key);
    return object->result;
}

extern char __vt__11CFxTomahawk[];
// Construction view of CFxTomahawk; canonical members: classes/CFxTomahawk.h.
// Retains the serial word at +4 and integer argument stores.
struct CFxTomahawkConstructorView {
    void* vtable;
    int result;
    void* parent;
    void* first;
    int zero2;
    unsigned char flag;
    char flagPadding[3];
    int zero;
    // Store the borrowed ball and creation flag; sequence creation is deferred.
    CFxTomahawkConstructorView(void* p, int value, unsigned char f) {
        vtable = &__vt__11CFxTomahawk;
        parent = (char*)p + kEffectBoxOffset;
        first = (void*)value;
        zero = 0;
        zero2 = 0;
        result = s_fxInstanceSerial++;
        flag = f;
    }
};

// Bind a Tomahawk control to its ball and register its serial ID.
int CFx_CreateTomahawk(void* self, int value, unsigned char flag) {
    CFxTomahawkConstructorView* object = new CFxTomahawkConstructorView(self, value, flag);
    struct { int key; void* object; } search;
    void* node;
    unsigned char flagB, flagA;
    search.key = object->result;
    search.object = object;
    void* inserted = CFxEffectTree_FindInsertionPosition((char*)self + kControlTreeOffset, &search.key, &node, &flagA, &flagB);
    if (node == 0 || ((TreeNode_80104CEC*)node)->key < search.key)
        FxSprayTree_InsertNode(&((Owner_80104CEC*)self)->tree, inserted, flagA, flagB, &search.key);
    return object->result;
}

extern char __vt__8CFxSpike[];
// Construction view of CFxSpike; canonical members: classes/CFxSpike.h.
// Retains CFxControl base construction and its exception cleanup.
struct CFxSpikeConstructorView : CFxControl {
    void* parent;
    void* value;
    void* child1;
    void* child2;
    unsigned char flag;
    char flagPadding[3];
    int zero;
    // Create the ground/ball Spike pair and assign the control's serial ID.
    CFxSpikeConstructorView(void* p, void* value, unsigned char f) {
        vtable = &__vt__8CFxSpike;
        parent = (char*)p + kEffectBoxOffset;
        this->value = value;
        child1 = CFxSequence_Create(parent, shotEffectResourceStrings.spikeGround, 0, 0, 0);
        child2 = CFxSequence_Create(parent, shotEffectResourceStrings.spikeBall, 0, 0, 0);
        zero = 0;
        result = s_fxInstanceSerial++;
        flag = f;
    }
};

#pragma push
#pragma use_lmw_stmw on
// Create ground and ball Spike sequences and register their control ID.
int CFx_CreateSpike(void* self, void* value, unsigned char flag) {
    CFxSpikeConstructorView* object = new CFxSpikeConstructorView(self, value, flag);
    struct { int key; void* object; } search;
    void* node;
    unsigned char flagB, flagA;
    search.key = object->result;
    search.object = object;
    void* inserted = CFxEffectTree_FindInsertionPosition((char*)self + kControlTreeOffset, &search.key, &node, &flagA, &flagB);
    if (node == 0 || ((TreeNode_80104CEC*)node)->key < search.key)
        FxSprayTree_InsertNode(&((Owner_80104CEC*)self)->tree, inserted, flagA, flagB, &search.key);
    return object->result;
}
#pragma pop

// Register a Cobra control; the legacy pointer return carries its integer serial ID.
CFxCobra* CFx_CreateCobra(void* self, int value) {
    CFxCobra* object = new CFxCobra((char*)self + kEffectBoxOffset, value);
    struct { int key; void* object; } search;
    void* node;
    unsigned char flagB, flagA;
    search.key = object->result;
    search.object = object;
    void* inserted = CFxEffectTree_FindInsertionPosition((char*)self + kControlTreeOffset, &search.key, &node, &flagA, &flagB);
    if (node == 0 || ((TreeNode_80104CEC*)node)->key < search.key) {
        FxSprayTree_InsertNode(&((Owner_80104CEC*)self)->tree, inserted, flagA, flagB, &search.key);
    }
    return (CFxCobra*)object->result;
}

extern void CFxPowerSpin_Ctor(void*, void*, int);
extern void CFxShotImpact_Ctor(void*, void*, int);
struct Struct_8010498C {
    void* vtable;
    int result;
    char controllerStorage[0x10];
    // Forward PowerSpin construction with the embedded effect-box address.
    Struct_8010498C(void* parent, int value) { CFxPowerSpin_Ctor(this, (char*)parent + kEffectBoxOffset, value); }
};

struct Struct_80104A44 {
    void* vtable;
    int result;
    char controllerStorage[0x10];
    // Forward shot-impact construction with the embedded effect-box address.
    Struct_80104A44(void* parent, int value) { CFxShotImpact_Ctor(this, (char*)parent + kEffectBoxOffset, value); }
};

extern void* CFxEffectTree_FindInsertionPosition(void*, int*, void**, unsigned char*, unsigned char*);
extern void FxSprayTree_InsertNode(void*, void*, unsigned char, unsigned char, int*);

// Allocate and register a PowerSpin control; return its serial ID.
int CFx_CreatePowerSpin(void* self, int value) {
    Struct_8010498C* object = new Struct_8010498C(self, value);
    struct { int key; void* object; } search;
    void* node;
    unsigned char flagB, flagA;
    search.key = object->result;
    search.object = object;
    void* inserted = CFxEffectTree_FindInsertionPosition((char*)self + kControlTreeOffset, &search.key, &node, &flagA, &flagB);
    if (node == 0 || ((TreeNode_80104CEC*)node)->key < search.key)
        FxSprayTree_InsertNode(&((Owner_80104CEC*)self)->tree, inserted, flagA, flagB, &search.key);
    return object->result;
}

// Allocate and register a shot-impact control; return its serial ID.
int CFx_MakeShotImpact(void* self, int value) {
    Struct_80104A44* object = new Struct_80104A44(self, value);
    struct { int key; void* object; } search;
    void* inserted;
    void* node;
    unsigned char flagB, flagA;
    search.key = object->result;
    search.object = object;
    inserted = CFxEffectTree_FindInsertionPosition((char*)self + kControlTreeOffset, &search.key, &node, &flagA, &flagB);
    if (node == 0 || ((TreeNode_80104CEC*)node)->key < search.key)
        FxSprayTree_InsertNode(&((Owner_80104CEC*)self)->tree, inserted, flagA, flagB, &search.key);
    (void)value;
    return object->result;
}

struct Struct_80105260 {
    void* vtable;
    int result;
    void* owner;
    int argA;
    int argB;
    void* childA;
    void* childB;
};

struct TreeNode_80104570 {
    TreeNode_80104570* left;
    TreeNode_80104570* right;
    TreeNode_80104570* parent;
    char keyStorage[4];
    void* value;
};

struct TreeNode_801046A8 {
    TreeNode_801046A8* left;
    TreeNode_801046A8* right;
    TreeNode_801046A8* parent;
    int key;
    void* value;
};
struct Struct_801046A8 {
    char baseStorage[0x3AC];
    int count;
    TreeNode_801046A8* treeRoot;
    TreeNode_801046A8* firstNode;
};
extern void RBTree_RebalanceAfterErase(void*, void*);
extern void appFreeUni__FPv(void*);
#pragma push
#pragma use_lmw_stmw on
#pragma opt_common_subs off
#pragma opt_propagation off
#pragma opt_lifetimes off
#pragma opt_strength_reduction off
// Erase a control ID, repair the tree cursor, and invoke deleting destruction.
// Return 1 when removed, 0 when absent. The ID is invalid after removal.
int CFx_RemoveControl(Struct_801046A8* self, int key) {
    TreeNode_801046A8* node;
    TreeNode_801046A8* result;
    TreeNode_801046A8* erased;
    void* value;
    TreeNode_801046A8* next;
    TreeNode_801046A8* parent;
    // Find an exact ID via lower_bound: treeRoot is the search root;
    // firstNode caches the first traversal node.
    node = ((Struct_801046A8*)self)->treeRoot;
    result = (TreeNode_801046A8*)(&((Owner_80104CEC*)self)->tree.root);
    while (node != 0) {
        if (node->key >= key) { result = node; node = node->left; }
        else node = node->right;
    }
    if (result == (TreeNode_801046A8*)(&((Owner_80104CEC*)self)->tree.root) || key < result->key) result = (TreeNode_801046A8*)(&((Owner_80104CEC*)self)->tree.root);
    if (result != (TreeNode_801046A8*)(&((Owner_80104CEC*)self)->tree.root)) {
        erased = result;
        value = erased->value;
        if (erased == self->firstNode) {
            next = erased->right;
            if (next != 0) {
                TreeNode_801046A8* left;
                while ((left = next->left) != 0) next = left;
                result = next;
            } else {
                result = erased;
                // 0x801046A8: ascend until arriving from a left child.
                while ((parent = (TreeNode_801046A8*)((unsigned int)result->parent & ~1u)),
                    result != parent->left) {
                    result = parent;
                }
                result = parent;
            }
            ((Struct_801046A8*)self)->firstNode = result;
        }
        // Repair tree links before freeing the node and destroying its payload.
        RBTree_RebalanceAfterErase(erased, (*(void**)&((Struct_801046A8*)self)->treeRoot));
        appFreeUni__FPv(erased);
        --self->count;
        if (value != 0) { struct VD_801046A8 { 
        virtual void destroy(int) DECOMP_LOCAL_VIRTUAL;
        }; ((VD_801046A8*)value)->destroy(1); }
        return 1;
    }
    return 0;
}
#pragma use_lmw_stmw reset
#pragma opt_propagation reset
#pragma opt_lifetimes reset
#pragma opt_strength_reduction reset
#pragma opt_common_subs reset
#pragma pop
struct Vec3_8010531C { void* xBits; float y, z; };
struct Struct_8010531C {
    void* baseStorage;
    char basePadding[8];
    Vec3_8010531C* vector;
    void* velocity;
    struct Flag { char sequenceHeader[0x80]; unsigned char value; }* groundSequence;
    struct Flag* waterSequence;
};
extern void* CQuadTree_GetInstance(void*);
extern void GetGroundPoint__9CQuadTreeF7WVectorbPib(void*, void*, Vec3_8010531C*, int, int, int);
extern float Magnitude__7WVectorCFv(void*);
extern int QuadTree_IsWaterAtPoint(void*, Vec3_8010531C*);
extern void __as__7WVectorFRC7WVector(void*, void*);
extern float kFloat_0_5_FxShot;
extern float kRunDustMaximumHeightAboveGround;
// Enable ground or water dust only while the ball moves close to the ground.
int CFxRunDust_UpdateGroundWaterSequences(Struct_8010531C* self) {
    Vec3_8010531C groundPosition = *self->vector;
    Vec3_8010531C groundQueryResult;
    Vec3_8010531C ballPosition = *self->vector;
    void* groundTree = CQuadTree_GetInstance(ballPosition.xBits);
    GetGroundPoint__9CQuadTreeF7WVectorbPib(&groundQueryResult, groundTree, (Vec3_8010531C*)&ballPosition, 1, 0, 1);
    groundPosition.y = groundQueryResult.y;
    if ((*(WVector*)self->velocity).Magnitude() < kFloat_0_5_FxShot ||
        self->vector->y > kRunDustMaximumHeightAboveGround + groundPosition.y) {
        if (self->groundSequence->value) self->groundSequence->value = 0;
        if (self->waterSequence->value) self->waterSequence->value = 0;
    } else {
        Vec3_8010531C surfacePosition = groundPosition;
        groundTree = ((void* (*)())CQuadTree_GetInstance)();
        Struct_8010531C::Flag* selectedSequence;
        if (QuadTree_IsWaterAtPoint(groundTree, &surfacePosition)) {
            if (self->groundSequence->value) self->groundSequence->value = 0;
            if (self->waterSequence->value != 1) self->waterSequence->value = 1;
            selectedSequence = self->waterSequence;
        } else {
            if (self->groundSequence->value != 1) self->groundSequence->value = 1;
            if (self->waterSequence->value) self->waterSequence->value = 0;
            selectedSequence = self->groundSequence;
        }
        ((CFxSequence*)selectedSequence)->worldPosition = *(WVector*)&groundPosition;
    }
    return 1;
}

extern void CFxSpray_SetWindVector(void*, void*);
extern void __as__7WVectorFRC7WVector(void*, void*);
struct WindEffectTrees {
    char managerHeader[0x16C];
    unsigned int sprayCount;
    TreeNode_80104570* sprayRoot;
    TreeNode_80104570* firstSpray;
    unsigned int sequenceCount;
    TreeNode_80104570* sequenceRoot;
    TreeNode_80104570* firstSequence;
};
struct SequenceWindView { char sequenceHeader[8]; WVector wind; };

// Propagate wind to all active sprays and sequences.
void CFx_SetWindVector(void* self, void* windVector) {
    // Update spray payloads in key order; no tree ownership changes occur.
    TreeNode_80104570* end = (TreeNode_80104570*)&((WindEffectTrees*)self)->sprayRoot;
    TreeNode_80104570* node = ((WindEffectTrees*)self)->firstSpray;
    while (node != end) {
        CFxSpray_SetWindVector(((TreeNode_80104570*)node)->value, windVector);
        TreeNode_80104570* child = node->right;
        if (child != 0) {
            TreeNode_80104570* p = child;
            TreeNode_80104570* left;
            while ((left = p->left) != 0) p = left;
            node = p;
        } else {
            // 0x80104570: ascend until arriving from a left child.
            while ((child = (TreeNode_80104570*)((unsigned int)node->parent & ~1u)),
                node != child->left) {
                node = child;
            }
            node = child;
        }
    }
    // Repeat for sequences, whose wind vector is stored directly at +8.
    node = ((WindEffectTrees*)self)->firstSequence;
    end = (TreeNode_80104570*)&((WindEffectTrees*)self)->sequenceRoot;
    while (end != node) {
        ((SequenceWindView*)node->value)->wind = *(WVector*)windVector;
        TreeNode_80104570* child = node->right;
        if (child != 0) {
            TreeNode_80104570* p = child;
            TreeNode_80104570* left;
            while ((left = p->left) != 0) p = left;
            node = p;
        } else {
            while ((child = (TreeNode_80104570*)((unsigned int)node->parent & ~1u)),
                node != child->left) {
                node = child;
            }
            node = child;
        }
    }
}

extern char __vt__8CFxCobra[];
// Create the ground/water/ring/ball sequence slots; register only a complete set.
CFxCobra::CFxCobra(void* o, int v) {
    value = v;
    owner = o;
    vtable = &__vt__8CFxCobra;
    flag = 0;
    children[0] = CFxSequence_Create(o, shotEffectResourceStrings.cobraGround, 0, 0, 0);
    children[1] = CFxSequence_Create(owner, shotEffectResourceStrings.cobraWater, 0, 0, 0);
    children[2] = CFxSequence_Create(owner, shotEffectResourceStrings.cobraRing, 0, 0, 0);
    children[3] = CFxSequence_Create(owner, shotEffectResourceStrings.cobraBall, 0, 0, 0);
    if (children[0] && children[1] && children[2] && children[3]) {
        CFxBox_RegisterSequence(owner, children[0]);
        CFxBox_RegisterSequence(owner, children[1]);
        CFxBox_RegisterSequence(owner, children[2]);
        CFxBox_RegisterSequence(owner, children[3]);
    }
    result = s_fxInstanceSerial++;
}

extern char __vt__10CFxRunDust[];
extern void* CFxEffectTree_FindInsertionPosition(void*, int*, void**, unsigned char*, unsigned char*);
extern void FxSprayTree_InsertNode(void*, void*, unsigned char, unsigned char, int*);
struct Object_80104570 : CFxControl {
    void* owner;
    int argA, argB;
    void* childA;
    void* childB;
    int unused;
    // Bind borrowed position/velocity sources and register the ground/water pair.
    Object_80104570(void* self, int a, int b) {
        vtable = (void*)__vt__10CFxRunDust;
        owner = (char*)self + kEffectBoxOffset;
        argA = a;
        argB = b;
        childA = CFxSequence_Create(owner, shotEffectResourceStrings.groundRun, 0, 0, 0);
        childB = CFxSequence_Create(owner, shotEffectResourceStrings.waterRun, 0, 0, 0);
        if (childA && childB) {
            CFxBox_RegisterSequence(owner, childA);
            CFxBox_RegisterSequence(owner, childB);
        }
        result = s_fxInstanceSerial++;
    }
};
#pragma push
#pragma use_lmw_stmw on
// Create the ground/water running pair bound to position and velocity sources.
int CFx_MakeRunDust(void* self, int ballPosition, int ballVelocity) {
    Object_80104570* object = new Object_80104570(self, ballPosition, ballVelocity);
    struct { int key; void* object; } search;
    void* node; unsigned char insertBefore, updateFirst;
    search.key = object->result;
    search.object = object;
    void* inserted = CFxEffectTree_FindInsertionPosition((char*)self + kControlTreeOffset, &search.key, &node, &insertBefore, &updateFirst);
    if (node == 0 || ((TreeNode_80104CEC*)node)->key < search.key)
        FxSprayTree_InsertNode(&((Owner_80104CEC*)self)->tree, inserted, insertBefore, updateFirst, &search.key);
    return object->result;
}
#pragma pop
extern char __vt__10CFxRunDust[];
// Remove and destroy both running-dust sequences.
Struct_80105260* CFxRunDust_dtor(Struct_80105260* self, int shouldDelete) {
    if (self != 0) {
        self->vtable = (void*)__vt__10CFxRunDust;
        CFxBox_RemoveSequence(self->owner, (Child_801055E4*)self->childA, 0);
        CFxBox_RemoveSequence(self->owner, (Child_801055E4*)self->childB, 0);
        if (self->childA != 0) ((Child_801055E4*)self->childA)->vf(1);
        if (self->childB != 0) ((Child_801055E4*)self->childB)->vf(1);
        if (shouldDelete > 0) delete self;
    }
    return self;
}

typedef Vec3f Vec3_80105688;
struct VectorOwner_80105688 {
    void* positionXBits;
    float y;
    float z;
    float velocityX;
    float velocityY;
    float velocityZ;
    char physicsStateBeforeFlags[0x30];
    unsigned int flightFlags;
};
typedef char Cobra_ball_flags_offset[((int)&((VectorOwner_80105688*)0)->flightFlags == 0x48) ? 1 : -1];
struct Struct_80105688 {
    char baseStorage[8];
    void* owner;
    VectorOwner_80105688* vector;
    void* child1;
    void* child2;
    void* child3;
    void* child4;
    unsigned char active;
};
extern void CFxBox_RegisterSequence(void*, void*);
extern float kFloat_0_shotEffects, kCobraGroundEffectHeightThreshold, kFloat_0_5_FxShot, kFloat_1_FxShot;
extern ShotEffectScriptNames shotEffectResourceStrings;

// Latch Cobra activation on motion and update its ball/near-ground sequence selection.
// child1/2/3/4 = ground/water/ring/ball. Return 0 if unavailable or active with
// negative vertical velocity; return 1 on the remaining update paths.
int CFxCobra_Update(Struct_80105688* self) {
    if (((unsigned int)self->vector->flightFlags & kCobraFlightFlag) == 0 ||
        self->child1 == 0 || self->child2 == 0 || self->child3 == 0 || self->child4 == 0)
        return 0;
    Vec3_80105688 groundPosition = *(Vec3_80105688*)self->vector;
    void* groundQuerySource = self->vector->positionXBits;
    Vec3_80105688 groundQueryResult;
    Vec3_80105688 ballPosition = *(Vec3_80105688*)self->vector;
    void* groundTree = CQuadTree_GetInstance(groundQuerySource);
    GetGroundPoint__9CQuadTreeF7WVectorbPib(&groundQueryResult, groundTree, (Vec3_8010531C*)&ballPosition, 1, 0, 1);
    groundPosition.y = groundQueryResult.y;
    // After activation, track the ball and switch ground effects by height/speed.
    if (self->active) {
        if (self->vector->velocityY < kFloat_0_shotEffects) return 0;
        ((CFxSequence*)self->child4)->worldPosition = *(WVector*)self->vector;
        if (self->vector->y < kCobraGroundEffectHeightThreshold + groundPosition.y) {
            if ((*(WVector*)&self->vector->velocityX).Magnitude() < kFloat_0_5_FxShot) {
                if ((*(unsigned char*)&((CFxSequence*)self->child1)->playbackEnabled)) (*(unsigned char*)&((CFxSequence*)self->child1)->playbackEnabled) = 0;
                if ((*(unsigned char*)&((CFxSequence*)self->child2)->playbackEnabled)) (*(unsigned char*)&((CFxSequence*)self->child2)->playbackEnabled) = 0;
                if ((*(unsigned char*)&((CFxSequence*)self->child3)->playbackEnabled)) (*(unsigned char*)&((CFxSequence*)self->child3)->playbackEnabled) = 0;
            } else {
                Vec3_80105688 surfacePosition = groundPosition;
                groundTree = ((void* (*)())CQuadTree_GetInstance)();
                if (QuadTree_IsWaterAtPoint(groundTree, (Vec3_8010531C*)&surfacePosition)) {
                    if ((*(unsigned char*)&((CFxSequence*)self->child1)->playbackEnabled)) (*(unsigned char*)&((CFxSequence*)self->child1)->playbackEnabled) = 0;
                    if ((*(unsigned char*)&((CFxSequence*)self->child2)->playbackEnabled) != 1) (*(unsigned char*)&((CFxSequence*)self->child2)->playbackEnabled) = 1;
                    if ((*(unsigned char*)&((CFxSequence*)self->child3)->playbackEnabled) != 1) (*(unsigned char*)&((CFxSequence*)self->child3)->playbackEnabled) = 1;
                    ((CFxSequence*)self->child2)->worldPosition = *(WVector*)&groundPosition;
                    ((CFxSequence*)self->child3)->worldPosition = *(WVector*)&groundPosition;
                } else {
                    if ((*(unsigned char*)&((CFxSequence*)self->child1)->playbackEnabled) != 1) (*(unsigned char*)&((CFxSequence*)self->child1)->playbackEnabled) = 1;
                    if ((*(unsigned char*)&((CFxSequence*)self->child2)->playbackEnabled)) (*(unsigned char*)&((CFxSequence*)self->child2)->playbackEnabled) = 0;
                    if ((*(unsigned char*)&((CFxSequence*)self->child3)->playbackEnabled)) (*(unsigned char*)&((CFxSequence*)self->child3)->playbackEnabled) = 0;
                    ((CFxSequence*)self->child1)->worldPosition = *(WVector*)&groundPosition;
                }
            }
        } else {
            if ((*(unsigned char*)&((CFxSequence*)self->child1)->playbackEnabled)) (*(unsigned char*)&((CFxSequence*)self->child1)->playbackEnabled) = 0;
            if ((*(unsigned char*)&((CFxSequence*)self->child2)->playbackEnabled)) (*(unsigned char*)&((CFxSequence*)self->child2)->playbackEnabled) = 0;
            if ((*(unsigned char*)&((CFxSequence*)self->child3)->playbackEnabled)) (*(unsigned char*)&((CFxSequence*)self->child3)->playbackEnabled) = 0;
        }
    // Before activation, sufficient speed starts a separate one-shot launch effect.
    } else if ((*(WVector*)&self->vector->velocityX).Magnitude() > kFloat_1_FxShot) {
        self->active = 1;

        void* launchSequence = ((void*(*)(void*, void*, int, int, int))CFxSequence_Create)(self->owner, shotEffectResourceStrings.cobraLaunch, 1, 0, 0);
        CFxBox_RegisterSequence(self->owner, launchSequence);
        ((CFxSequence*)launchSequence)->worldPosition = *(WVector*)self->vector;
    }
    return 1;
}

} // extern "C"

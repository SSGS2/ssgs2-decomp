// Camera controller construction and teardown: the controller's own
// initialisation, the camera-work base object, and the destructors for the
// string-alias, indexed-group and nested-map containers it owns.
#include "iff/IffStruct.h"
// Address-only layout of the existing string pool; no new string storage.
struct CameraWorkStringPool {
    char m_def[6];                // 0x0
    char ken[4];                  // 0x6
    char f_def[6];                // 0xA
    char eri[4];                  // 0x10
    char a_def[6];                // 0x14
    char dai[4];                  // 0x1A
    char c_def[6];                // 0x1E
    char ces[4];                  // 0x24
    char d_def[6];                // 0x28
    char max[4];                  // 0x2E
    char e_def[6];                // 0x32
    char koo[4];                  // 0x38
    char g_def[6];                // 0x3C
    char ari[4];                  // 0x42
    char h_def[6];                // 0x46
    char kaz[4];                  // 0x4C
    char albatrossPose[23];       // 0x50
    char Albatross[10];           // 0x67
    char holeInOnePose[25];       // 0x71
    char HoleInOne[10];           // 0x8A
    char birdiePose[25];          // 0x94
    char Birdie[7];               // 0xAD
    char parPose[21];             // 0xB4
    char Par[4];                  // 0xC9
    char bogeyPose[19];           // 0xCD
    char Bogey[6];                // 0xE0
    char doublePower04Pose[25];   // 0xE6
    char DoublePower04[14];       // 0xFF
    char doublePower03Pose[25];   // 0x10D
    char DoublePower03[14];       // 0x126
    char doublePower02Pose[25];   // 0x134
    char DoublePower02[14];       // 0x14D
    char doublePower01Pose[25];   // 0x15B
    char DoublePower01[14];       // 0x174
    char stroke[7];               // 0x182
    char match[6];                // 0x189
    char singleAwardCeremony[12]; // 0x18F
    char Single[7];               // 0x19B
};

#include "classes/MWStringStorage.h"
#define GAME_NSTRING_TEXT_CONSTRUCTOR
#include "game/string.h"
#include "classes/CCameraWork.h"

extern "C" {
void* CameraWorkTree_dtor(void*, int);
void* CameraNestedMap_dtor(void*, int);
void* StringMap_dtor(void*, int);
void* CameraIndexedGroupMap_dtor(void*, int);
void* CameraActionVector_dtor(void*, int);
void* CameraWork_DestroyBase(void*, int);

void BaseObject_ctor(void*);
void* CameraWorkBase_Init(void*);
void BaseObject_dtor(void*, int);
void appFreeUni__FPv(void*);
void StringMap_DestroyStringPair(void*, void*);
void StringMap_FreeSubtree(void*, void*);
void MapTree_FreeSubtreeWithRelease(void*, void*);
void CameraInputOffset_DefaultConstruct(void*); // CameraInputOffset_DefaultConstruct.
void WVector_DefaultConstructNoOp(void*);       // Leaves the vector storage unchanged.
void WMatrix_DefaultConstructNoOp(void*);       // Leaves the matrix storage unchanged.
void CameraQuaternion_DefaultConstruct(void*);  // CameraQuaternion_DefaultConstruct.
void CameraActionHistory_InitEmpty(int*);       // CameraActionHistory_InitEmpty.
void CameraStringAliasTree_InitEmpty(int*);     // CameraStringAliasTree_InitEmpty.
void CameraIndexStringTree_InitEmpty(int*);     // CameraIndexStringTree_InitEmpty.
void CameraNestedMapTree_InitEmpty(int*);       // CameraNestedMapTree_InitEmpty.
void CameraWorkTree_Initialize(void*);
// Return the larger signed value.
int Math_MaxInt(int, int);
int GolfBall__Fv(void);
void MWString_ConstructCString(void*, const char*);
void* CameraStringMap_GetOrInsertString(void*, void*);
void MWString_AssignCString(void*, const char*);

void* CameraByteStringMap_GetOrInsert(void*, const char*);
extern char __vt__11CCameraWork[];
extern char zeroInitializationWords[];
extern int s_cameraModeWeightsStorage[];
extern int s_extendedCameraModeWeights[];
extern char cameraWorkStrings[];
extern char s_cameraStrokeGroupKey; // s_cameraStrokeGroupKey.
extern char s_cameraMatchGroupKey;  // s_cameraMatchGroupKey.
extern float kFloat_0_CameraWork;
void __dl__FPv(void*);
extern char s_cameraWorkBaseVtable[]; // s_cameraWorkBaseVtable.
extern void* g_cameraWork;

typedef NString LocalString;
enum {
    kStandardCameraModeCount = 28,
    kExtendedCameraModeCount = 13
};

// Named views of the alias maps stored in the controller.
static inline char* CharacterMotionAliases(CCameraWork* work) {
    return work->characterModelMappings;
}
static inline char* PowerShotMotionAliases(CCameraWork* work) {
    return work->powerShotMotionAliases;
}
static inline char* HoleResultMotionAliases(CCameraWork* work) {
    return work->resourceGroupNames;
}
static inline char* AwardMotionAliases(CCameraWork* work) {
    return work->awardMotionAliases;
}

// Controller state after construction:
// - position/target/curves/transforms are constructed but their no-op constructors do NOT zero them;
// - standard/extended totals sum max(0, weight[i]) over 28/13 modes for weighted selection;
// - character aliases map model tokens to motion groups; result/power/award aliases map pose names;
// - indexed groups bind the stroke/match byte keys, while loaded/nested maps start empty.
// Map header: size +0, root +4, opaque word +8 (left untouched), first/end +12.
// Empty first/end points to &root. Nodes own heap strings, never the static pool literals.
// Action history owns only its element allocation (data,size,capacity).
struct CameraActionVector {
    void* data;
    unsigned int size;
    unsigned int capacity;
};
struct CameraMapHeader {
    unsigned int size;
    void* root;
    void* reserved;
    void* end;
};
struct CameraStringPairNode {
    void* left;
    void* right;
    void* parent;
    MWStringStorage key;
    MWStringStorage value;
};
struct CameraIndexStringNode {
    void* left;
    void* right;
    void* parent;
    int key;
    MWStringStorage value;
};
struct CameraNestedMapNode {
    void* left;
    void* right;
    void* parent;
    int key;
    CameraMapHeader value;
};

} // extern "C"

#pragma push
#pragma opt_common_subs off
// Build the controller in storage owned by the caller. The owning object runs
// this from its constructor, so it is a named member and returns the storage.
CCameraWork* CCameraWork::Init() {
    // Phase 1: publish the base, construct state and empty owning containers.
    CameraWorkBase_Init(this);
    *(void**)this = __vt__11CCameraWork;
    CameraInputOffset_DefaultConstruct((char*)&inputFovOffset);
    int def1;
    int def0;
    int* defaults = (int*)zeroInitializationWords;
    def0 = *defaults++;
    def1 = *defaults++;
    motionCallbackThisAdjustment = def0;
    motionCallbackVtableOffset = def1;
    motionCallbackFunction = *defaults;
    WVector_DefaultConstructNoOp((char*)position);
    WVector_DefaultConstructNoOp((char*)target);
    modeWeightTotal = 0;
    extendedModeWeightTotal = 0;
    WMatrix_DefaultConstructNoOp((char*)transform);
    WMatrix_DefaultConstructNoOp((char*)previousTransform);
    char* itemEnd;
    char* item = (char*)curveControlPoints;
    itemEnd = (char*)motionStartPosition;
    do {
        WVector_DefaultConstructNoOp(item);
        item += 12;
    } while (item < itemEnd);
    WVector_DefaultConstructNoOp(itemEnd);
    WMatrix_DefaultConstructNoOp((char*)rotation);
    groupAnimationActive = 0;
    WVector_DefaultConstructNoOp((char*)pivotPosition);
    WVector_DefaultConstructNoOp((char*)initialBallPosition);
    CameraQuaternion_DefaultConstruct((char*)startQuaternion);
    CameraQuaternion_DefaultConstruct((char*)endQuaternion);
    WVector_DefaultConstructNoOp((char*)startPosition);
    WVector_DefaultConstructNoOp((char*)endPosition);
    CameraActionHistory_InitEmpty((int*)((char*)&actionHistory));
    useSavedCameraTransform = 0;
    WMatrix_DefaultConstructNoOp((char*)savedCameraRight);
    CameraStringAliasTree_InitEmpty((int*)(CharacterMotionAliases(this)));
    CameraIndexStringTree_InitEmpty((int*)((char*)indexedCameraGroups));
    CameraStringAliasTree_InitEmpty((int*)(PowerShotMotionAliases(this)));
    CameraStringAliasTree_InitEmpty((int*)(HoleResultMotionAliases(this)));
    CameraStringAliasTree_InitEmpty((int*)(AwardMotionAliases(this)));
    CameraNestedMapTree_InitEmpty((int*)((char*)loadedCharacterResourceGroups));
    CameraNestedMapTree_InitEmpty((int*)((char*)nestedCameraMap));
    CameraWorkTree_Initialize((char*)cameraWorkTree);
    // Phase 2: establish weighted-selection ranges; negative entries contribute zero.
    modeCount = kStandardCameraModeCount;
    extendedModeCount = kExtendedCameraModeCount;
    {
        int* arrPtr = (int*)s_cameraModeWeightsStorage;
        int i = 0;
        while (i < modeCount) {
            modeWeightTotal += Math_MaxInt(0, *arrPtr);
            ++arrPtr;
            ++i;
        }
    }
    {
        int* arrPtr = (int*)s_extendedCameraModeWeights;
        int i = 0;
        while (i < extendedModeCount) {
            extendedModeWeightTotal += Math_MaxInt(0, *arrPtr);
            ++arrPtr;
            ++i;
        }
    }
    roll = kFloat_0_CameraWork;
    pivot = GolfBall__Fv();
    // Defaults for the action state: +0x240 is a byte, +0x23C an integer that starts at 1.
    unk_240 = 0;
    randomModeRequested = 0;
    shotCameraSelectionEnabled = 1;
    unk_23C = 1;
    installed = 0;
    animationGroup = 0;
    playingAnimationGroup = 0;
    animationSegmentIndex = 0;
    groupValueCount = 0;
    // Phase 3: copy alias keys/values from borrowed pool strings into owning maps.
    {
        LocalString key(cameraWorkStrings + IFF_OFFSET(CameraWorkStringPool, m_def));
        MWString_AssignCString(CameraStringMap_GetOrInsertString(CharacterMotionAliases(this), &key),
                               cameraWorkStrings + IFF_OFFSET(CameraWorkStringPool, ken));
    }
    {
        LocalString key(cameraWorkStrings + IFF_OFFSET(CameraWorkStringPool, f_def));
        MWString_AssignCString(CameraStringMap_GetOrInsertString(CharacterMotionAliases(this), &key),
                               cameraWorkStrings + IFF_OFFSET(CameraWorkStringPool, eri));
    }
    {
        LocalString key(cameraWorkStrings + IFF_OFFSET(CameraWorkStringPool, a_def));
        MWString_AssignCString(CameraStringMap_GetOrInsertString(CharacterMotionAliases(this), &key),
                               cameraWorkStrings + IFF_OFFSET(CameraWorkStringPool, dai));
    }
    {
        LocalString key(cameraWorkStrings + IFF_OFFSET(CameraWorkStringPool, c_def));
        MWString_AssignCString(CameraStringMap_GetOrInsertString(CharacterMotionAliases(this), &key),
                               cameraWorkStrings + IFF_OFFSET(CameraWorkStringPool, ces));
    }
    {
        LocalString key(cameraWorkStrings + IFF_OFFSET(CameraWorkStringPool, d_def));
        MWString_AssignCString(CameraStringMap_GetOrInsertString(CharacterMotionAliases(this), &key),
                               cameraWorkStrings + IFF_OFFSET(CameraWorkStringPool, max));
    }
    {
        LocalString key(cameraWorkStrings + IFF_OFFSET(CameraWorkStringPool, e_def));
        MWString_AssignCString(CameraStringMap_GetOrInsertString(CharacterMotionAliases(this), &key),
                               cameraWorkStrings + IFF_OFFSET(CameraWorkStringPool, koo));
    }
    {
        LocalString key(cameraWorkStrings + IFF_OFFSET(CameraWorkStringPool, g_def));
        MWString_AssignCString(CameraStringMap_GetOrInsertString(CharacterMotionAliases(this), &key),
                               cameraWorkStrings + IFF_OFFSET(CameraWorkStringPool, ari));
    }
    {
        LocalString key(cameraWorkStrings + IFF_OFFSET(CameraWorkStringPool, h_def));
        MWString_AssignCString(CameraStringMap_GetOrInsertString(CharacterMotionAliases(this), &key),
                               cameraWorkStrings + IFF_OFFSET(CameraWorkStringPool, kaz));
    }
    {
        LocalString key(cameraWorkStrings + IFF_OFFSET(CameraWorkStringPool, albatrossPose));
        MWString_AssignCString(CameraStringMap_GetOrInsertString(HoleResultMotionAliases(this), &key),
                               cameraWorkStrings + IFF_OFFSET(CameraWorkStringPool, Albatross));
    }
    {
        LocalString key(cameraWorkStrings + IFF_OFFSET(CameraWorkStringPool, holeInOnePose));
        MWString_AssignCString(CameraStringMap_GetOrInsertString(HoleResultMotionAliases(this), &key),
                               cameraWorkStrings + IFF_OFFSET(CameraWorkStringPool, HoleInOne));
    }
    {
        LocalString key(cameraWorkStrings + IFF_OFFSET(CameraWorkStringPool, birdiePose));
        MWString_AssignCString(CameraStringMap_GetOrInsertString(HoleResultMotionAliases(this), &key),
                               cameraWorkStrings + IFF_OFFSET(CameraWorkStringPool, Birdie));
    }
    {
        LocalString key(cameraWorkStrings + IFF_OFFSET(CameraWorkStringPool, parPose));
        MWString_AssignCString(CameraStringMap_GetOrInsertString(HoleResultMotionAliases(this), &key),
                               cameraWorkStrings + IFF_OFFSET(CameraWorkStringPool, Par));
    }
    {
        LocalString key(cameraWorkStrings + IFF_OFFSET(CameraWorkStringPool, bogeyPose));
        MWString_AssignCString(CameraStringMap_GetOrInsertString(HoleResultMotionAliases(this), &key),
                               cameraWorkStrings + IFF_OFFSET(CameraWorkStringPool, Bogey));
    }
    {
        LocalString key(cameraWorkStrings + IFF_OFFSET(CameraWorkStringPool, doublePower04Pose));
        MWString_AssignCString(CameraStringMap_GetOrInsertString(PowerShotMotionAliases(this), &key),
                               cameraWorkStrings + IFF_OFFSET(CameraWorkStringPool, DoublePower04));
    }
    {
        LocalString key(cameraWorkStrings + IFF_OFFSET(CameraWorkStringPool, doublePower03Pose));
        MWString_AssignCString(CameraStringMap_GetOrInsertString(PowerShotMotionAliases(this), &key),
                               cameraWorkStrings + IFF_OFFSET(CameraWorkStringPool, DoublePower03));
    }
    {
        LocalString key(cameraWorkStrings + IFF_OFFSET(CameraWorkStringPool, doublePower02Pose));
        MWString_AssignCString(CameraStringMap_GetOrInsertString(PowerShotMotionAliases(this), &key),
                               cameraWorkStrings + IFF_OFFSET(CameraWorkStringPool, DoublePower02));
    }
    {
        LocalString key(cameraWorkStrings + IFF_OFFSET(CameraWorkStringPool, doublePower01Pose));
        MWString_AssignCString(CameraStringMap_GetOrInsertString(PowerShotMotionAliases(this), &key),
                               cameraWorkStrings + IFF_OFFSET(CameraWorkStringPool, DoublePower01));
    }
    MWString_AssignCString(CameraByteStringMap_GetOrInsert((char*)indexedCameraGroups, &s_cameraStrokeGroupKey),
                           cameraWorkStrings + IFF_OFFSET(CameraWorkStringPool, stroke));
    MWString_AssignCString(CameraByteStringMap_GetOrInsert((char*)indexedCameraGroups, &s_cameraMatchGroupKey),
                           cameraWorkStrings + IFF_OFFSET(CameraWorkStringPool, match));
    {
        LocalString key(cameraWorkStrings + IFF_OFFSET(CameraWorkStringPool, singleAwardCeremony));
        MWString_AssignCString(CameraStringMap_GetOrInsertString(AwardMotionAliases(this), &key),
                               cameraWorkStrings + IFF_OFFSET(CameraWorkStringPool, Single));
    }
    return this;
}
#pragma pop

extern "C" {

// Construct the base object and publish it as the active camera-work controller.
void* CameraWorkBase_Init(void* self) {
    BaseObject_ctor(self);
    void* vtable = s_cameraWorkBaseVtable;
    *(void**)self = vtable;
    g_cameraWork = self;
    return self;
}

// Unpublish and destroy the camera-work base, optionally freeing its allocation.
void* CameraWork_DestroyBase(void* self, int shouldDelete) {
    if (self != 0) {
        g_cameraWork = 0;
        BaseObject_dtor(self, 0);
        if (shouldDelete > 0) {
            __dl__FPv(self);
        }
    }
    return self;
}

#pragma push
#pragma opt_propagation off
#pragma opt_strength_reduction off
// Release the camera action-history vector storage and optionally free the vector wrapper.
// In plain C: if (data) { size = 0; free(data); }  if (shouldDelete > 0) delete self;
void* CameraActionVector_dtor(void* self, int shouldDelete) {
    if (self != 0) {
        if (self != 0 && self != 0 && self != 0 && self != 0) {
            void* p = *(void**)self;
            if (p != 0) {
                int* count = (int*)&((CameraActionVector*)self)->size;
                *count -= ((CameraActionVector*)self)->size;
                appFreeUni__FPv(p);
            }
        }
        if (shouldDelete > 0) {
            __dl__FPv(self);
        }
    }
    return self;
}
#pragma pop

// All expanded destructors below are postorder: descendants -> payload -> node.
// Repeated address/null guards mirror the original nested destructor expansion.
// Alias nodes release TWO strings; index nodes release ONE; nested nodes release an inner map.
// shouldDelete > 0 additionally frees the header; no cleanup here reinitializes it for reuse.
// Generic tree node as seen by the destructors: left/right child pointers, flags (parent|color), and the
// node payload (key/value) starting at +12.
struct DtorNode {
    void* first;
    void* second;
    int flags;
    union {
        void* payloadStart;     // first payload word; never a child link
        void* keyOwnershipBits; // inner node: pointer-sized string flags word
    };
};

#pragma push
#pragma always_inline on
// Destroy deeper alias descendants, release both owned strings, then free this frontier node.
static inline void DestroyAliasFrontier(void* owner, DtorNode* node) {
    if (node->first != 0)
        StringMap_FreeSubtree(owner, node->first);
    if (node->second != 0)
        StringMap_FreeSubtree(owner, node->second);
    StringMap_DestroyStringPair(owner, &node->payloadStart);
    appFreeUni__FPv(node);
}
#pragma pop

#pragma push
#pragma use_lmw_stmw on
#pragma opt_common_subs off
// Destroy the string-to-string alias tree, including owned key/value strings, then optionally free it.
// In plain C:  freeNode(n) { if (n->left) freeNode(n->left); if (n->right) freeNode(n->right);
//   free heap storage of n's two strings (bit 31 of a string's flags marks heap storage); free(n); }
//   freeNode(root);  if (shouldDelete > 0) delete self;
// The code below is the compiler's inlined expansion of that recursion: three nested levels (root, its
// children, their children) written out by hand, with DestroyAliasFrontier doing the remaining recursive calls.
// A reader can skip to the final `if (shouldDelete > 0)`.
void* StringMap_dtor(void* self, int shouldDelete) {
    if (self != 0) {
        if (self != 0 && self != 0 && self != 0 && self != 0 && self != 0) {
            DtorNode* root = (DtorNode*)((CameraMapHeader*)self)->root;
            if (root != 0) {
                DtorNode* child = (DtorNode*)root->first;
                if (child != 0) {
                    DtorNode* grandchild = (DtorNode*)child->first;
                    if (grandchild != 0) {
                        DtorNode* leaf0 = (DtorNode*)grandchild->first;
                        if (leaf0 != 0)
                            DestroyAliasFrontier(self, leaf0);
                        DtorNode* leaf1 = (DtorNode*)grandchild->second;
                        if (leaf1 != 0)
                            DestroyAliasFrontier(self, leaf1);
                        StringMap_DestroyStringPair(self, &grandchild->payloadStart);
                        appFreeUni__FPv(grandchild);
                    }
                    DtorNode* grandchild1 = (DtorNode*)child->second;
                    if (grandchild1 != 0) {
                        DtorNode* leaf2 = (DtorNode*)grandchild1->first;
                        if (leaf2 != 0)
                            DestroyAliasFrontier(self, leaf2);
                        DtorNode* leaf3 = (DtorNode*)grandchild1->second;
                        if (leaf3 != 0)
                            DestroyAliasFrontier(self, leaf3);
                        StringMap_DestroyStringPair(self, &grandchild1->payloadStart);
                        appFreeUni__FPv(grandchild1);
                    }
                    StringMap_DestroyStringPair(self, &child->payloadStart);
                    appFreeUni__FPv(child);
                }
                DtorNode* child1 = (DtorNode*)root->second;
                if (child1 != 0) {
                    DtorNode* grandchild = (DtorNode*)child1->first;
                    if (grandchild != 0) {
                        DtorNode* leaf = (DtorNode*)grandchild->first;
                        if (leaf != 0)
                            DestroyAliasFrontier(self, leaf);
                        DtorNode* leaf1 = (DtorNode*)grandchild->second;
                        if (leaf1 != 0)
                            DestroyAliasFrontier(self, leaf1);
                        StringMap_DestroyStringPair(self, &grandchild->payloadStart);
                        appFreeUni__FPv(grandchild);
                    }
                    DtorNode* grandchild1 = (DtorNode*)child1->second;
                    if (grandchild1 != 0) {
                        DtorNode* leaf = (DtorNode*)grandchild1->first;
                        if (leaf != 0)
                            DestroyAliasFrontier(self, leaf);
                        DtorNode* leaf1 = (DtorNode*)grandchild1->second;
                        if (leaf1 != 0)
                            DestroyAliasFrontier(self, leaf1);
                        StringMap_DestroyStringPair(self, &grandchild1->payloadStart);
                        appFreeUni__FPv(grandchild1);
                    }
                    StringMap_DestroyStringPair(self, &child1->payloadStart);
                    appFreeUni__FPv(child1);
                }
                if (&(((CameraStringPairNode*)root)->key) != 0) {
                    if (&(((CameraStringPairNode*)root)->value) != 0 &&
                        (((CameraStringPairNode*)root)->value.flags >> 31) != 0)
                        appFreeUni__FPv(((CameraStringPairNode*)root)->value.data);
                    if (&(((CameraStringPairNode*)root)->key) != 0 &&
                        (((CameraStringPairNode*)root)->key.flags >> 31) != 0)
                        appFreeUni__FPv(((CameraStringPairNode*)root)->key.data);
                }
                appFreeUni__FPv(root);
            }
        }
        if (shouldDelete > 0)
            __dl__FPv(self);
    }
    return self;
}
#pragma pop

#pragma push
#pragma always_inline on
#pragma opt_common_subs off
#pragma opt_lifetimes off
// Release only an indexed camera group's owned string and node; its integer key has no allocation.
static inline void ReleaseCameraIndexedStringNode(DtorNode* node) {
    if (&((CameraIndexStringNode*)node)->key != 0) {
        if (&((CameraIndexStringNode*)node)->value != 0) {
            if (((CameraIndexStringNode*)node)->value.flags >> 31)
                appFreeUni__FPv(((CameraIndexStringNode*)node)->value.data);
        }
    }
    appFreeUni__FPv(node);
}
#pragma pop

#pragma push
#pragma always_inline on
#pragma opt_common_subs off
#pragma opt_lifetimes off
// Destroy deeper indexed-string descendants using their owner, then release the owned string and node.
static inline void DestroyIndexedStringFrontier(void* owner, DtorNode* node) {
    if (node->first != 0)
        MapTree_FreeSubtreeWithRelease(owner, node->first);
    if (node->second != 0)
        MapTree_FreeSubtreeWithRelease(owner, node->second);
    ReleaseCameraIndexedStringNode(node);
}
#pragma pop
#pragma push
#pragma use_lmw_stmw on
#pragma opt_common_subs off
#pragma opt_lifetimes off
// Destroy the indexed camera-group tree and its owned strings, then optionally free it.
// In plain C: free every node of the tree (children first, then the node's index-keyed string value if it
// has heap storage, then the node), then `if (shouldDelete > 0) delete self`. The nesting below is the
// hand-unrolled form of that recursion (DestroyIndexedStringFrontier does the last level and recurses beyond it).
void* CameraIndexedGroupMap_dtor(void* self, int shouldDelete) {
    if (self != 0) {
        if (self != 0 && self != 0 && self != 0 && self != 0 && self != 0) {
            DtorNode* root = (DtorNode*)((CameraMapHeader*)self)->root;
            if (root != 0) {
                // Indexed-string descendants: release each owned string before its node.
                /* node0 = root->first (pair of: first=pair-of-leaves, second=leaf-pair) */
                DtorNode* node0 = (DtorNode*)root->first;
                if (node0 != 0) {
                    DtorNode* a = (DtorNode*)node0->first; /* pair of leaf-pairs */
                    if (a != 0) {
                        DtorNode* a1 = (DtorNode*)a->first;
                        if (a1 != 0)
                            DestroyIndexedStringFrontier(self, a1);
                        DtorNode* a2 = (DtorNode*)a->second;
                        if (a2 != 0)
                            DestroyIndexedStringFrontier(self, a2);
                        ReleaseCameraIndexedStringNode(a);
                    }
                    DtorNode* b = (DtorNode*)node0->second; /* pair of leaf-pairs */
                    if (b != 0) {
                        DtorNode* b1 = (DtorNode*)b->first;
                        if (b1 != 0)
                            DestroyIndexedStringFrontier(self, b1);
                        DtorNode* b2 = (DtorNode*)b->second;
                        if (b2 != 0)
                            DestroyIndexedStringFrontier(self, b2);
                        ReleaseCameraIndexedStringNode(b);
                    }
                    ReleaseCameraIndexedStringNode(node0);
                }
                /* node0 = root->second (pair of leaf-pairs) */
                node0 = (DtorNode*)root->second;
                if (node0 != 0) {
                    DtorNode* a = (DtorNode*)node0->first; /* pair of leaf-pairs */
                    if (a != 0) {
                        DtorNode* a1 = (DtorNode*)a->first;
                        if (a1 != 0)
                            DestroyIndexedStringFrontier(self, a1);
                        DtorNode* a2 = (DtorNode*)a->second;
                        if (a2 != 0)
                            DestroyIndexedStringFrontier(self, a2);
                        ReleaseCameraIndexedStringNode(a);
                    }
                    DtorNode* b = (DtorNode*)node0->second; /* pair of leaf-pairs */
                    if (b != 0) {
                        DtorNode* b1 = (DtorNode*)b->first;
                        if (b1 != 0)
                            DestroyIndexedStringFrontier(self, b1);
                        DtorNode* b2 = (DtorNode*)b->second;
                        if (b2 != 0)
                            DestroyIndexedStringFrontier(self, b2);
                        ReleaseCameraIndexedStringNode(b);
                    }
                    ReleaseCameraIndexedStringNode(node0);
                }
                ReleaseCameraIndexedStringNode(root);
            }
        }
        if (shouldDelete > 0)
            __dl__FPv(self);
    }
    return self;
}
#pragma pop

void CameraNestedStringMap_DestroySubtree(void*, void*);
void CameraNestedStringMap_DestroyEntry(void*, void*);
void StringValueTree_DestroySubtree(void*, void*);
enum {
    kNestedInnerMapOffset = (int)&((CameraNestedMapNode*)0)->value
};
// Ownership: each outer index node owns its inner string map. Each inner
// node owns only its heap key (flags bit 31); node allocations are freed last.
// L/R suffixes spell the path from the root, making each expanded visit unique.
// Only the frontier helpers call the existing recursive routines. No node is visited
// by both a helper and a recursive routine.
#pragma push
#pragma always_inline on
// Release an outer node's owned key/map pair through its outer map, then free the node.
static inline void ReleaseOuterNode(void* outerMap, DtorNode* node) {
    CameraNestedStringMap_DestroyEntry(outerMap, &node->payloadStart);
    appFreeUni__FPv(node);
}
// Destroy a frontier outer node, including deeper descendants and its inner map.
static inline void DestroyOuterFrontier(void* outerMap, DtorNode* node) {
    if (node->first != 0)
        CameraNestedStringMap_DestroySubtree(outerMap, node->first);
    if (node->second != 0)
        CameraNestedStringMap_DestroySubtree(outerMap, node->second);
    ReleaseOuterNode(outerMap, node);
}
#pragma push
#pragma opt_common_subs off
// Free a heap key only; the enclosing node's allocation has separate ownership.
static inline void ReleaseInnerHeapKey(DtorNode* node) {
    if (((unsigned int)node->keyOwnershipBits >> 31) != 0)
        appFreeUni__FPv(((CameraStringPairNode*)node)->key.data);
}
// Preserve the original subobject guards, then release key storage and the node.
static inline void ReleaseInnerNode(DtorNode* node) {
    if (&((CameraStringPairNode*)node)->key != 0) {
        if (&((CameraStringPairNode*)node)->key != 0) {
            ReleaseInnerHeapKey(node);
        }
    }
    appFreeUni__FPv(node);
}
#pragma pop
// Destroy a frontier inner node using its own inner-map header as recursion context.
static inline void DestroyInnerFrontier(CameraMapHeader* innerMap, DtorNode* node) {
    if (node->first != 0)
        StringValueTree_DestroySubtree(innerMap, node->first);
    if (node->second != 0)
        StringValueTree_DestroySubtree(innerMap, node->second);
    ReleaseInnerNode(node);
}

#pragma pop
#pragma push
#pragma use_lmw_stmw on
// Destroy the nested camera maps and their string payloads, then optionally free the outer map.
// In plain C: for every node of the outer (index-keyed) tree, destroy its inner string-keyed map (all inner
// nodes, freeing each heap key) and then the node; finally `if (shouldDelete > 0) delete self`.
// L/R suffixes spell the path from the root; every branch is postorder.
// The remaining expansion preserves local lifetimes; frontier helpers below release outer / inner leaves with explicit owner contexts.
void* CameraNestedMap_dtor(void* self, int shouldDelete) {
    if (self != 0) {
        if (self != 0 && self != 0 && self != 0 && self != 0 && self != 0) {
            DtorNode* root = (DtorNode*)((CameraMapHeader*)self)->root;
            if (root != 0) {
                // Phase 1: outer descendants, left subtree then right subtree.
                DtorNode* outerL = (DtorNode*)root->first;
                if (outerL != 0) {
                    DtorNode* outerLL = (DtorNode*)outerL->first;
                    if (outerLL != 0) {
                        DtorNode* outerLLL = (DtorNode*)outerLL->first;
                        if (outerLLL != 0) {
                            DestroyOuterFrontier(self, outerLLL);
                        }
                        DtorNode* outerLLR = (DtorNode*)outerLL->second;
                        if (outerLLR != 0) {
                            DestroyOuterFrontier(self, outerLLR);
                        }
                        ReleaseOuterNode(self, outerLL);
                    }
                    DtorNode* outerLR = (DtorNode*)outerL->second;
                    if (outerLR != 0) {
                        DtorNode* outerLRL = (DtorNode*)outerLR->first;
                        if (outerLRL != 0) {
                            DestroyOuterFrontier(self, outerLRL);
                        }
                        DtorNode* outerLRR = (DtorNode*)outerLR->second;
                        if (outerLRR != 0) {
                            DestroyOuterFrontier(self, outerLRR);
                        }
                        ReleaseOuterNode(self, outerLR);
                    }
                    ReleaseOuterNode(self, outerL);
                }
                DtorNode* outerR = (DtorNode*)root->second;
                if (outerR != 0) {
                    DtorNode* outerRL = (DtorNode*)outerR->first;
                    if (outerRL != 0) {
                        DtorNode* outerRLL = (DtorNode*)outerRL->first;
                        if (outerRLL != 0) {
                            DestroyOuterFrontier(self, outerRLL);
                        }
                        DtorNode* outerRLR = (DtorNode*)outerRL->second;
                        if (outerRLR != 0) {
                            DestroyOuterFrontier(self, outerRLR);
                        }
                        ReleaseOuterNode(self, outerRL);
                    }
                    DtorNode* outerRR = (DtorNode*)outerR->second;
                    if (outerRR != 0) {
                        DtorNode* outerRRL = (DtorNode*)outerRR->first;
                        if (outerRRL != 0) {
                            DestroyOuterFrontier(self, outerRRL);
                        }
                        DtorNode* outerRRR = (DtorNode*)outerRR->second;
                        if (outerRRR != 0) {
                            DestroyOuterFrontier(self, outerRRR);
                        }
                        ReleaseOuterNode(self, outerRR);
                    }
                    ReleaseOuterNode(self, outerR);
                }
                // Phase 2: release the final inner string tree before its outer node.
                // Repeated field-address checks are destructor expansion artifacts.
                // Heap ownership is decided only by the inner key flag, not these checks.
                if (&((CameraNestedMapNode*)root)->key != 0) {
                    if ((unsigned int)root + kNestedInnerMapOffset != 0 &&
                        (unsigned int)root + kNestedInnerMapOffset != 0 &&
                        (unsigned int)root + kNestedInnerMapOffset != 0 &&
                        (unsigned int)root + kNestedInnerMapOffset != 0 &&
                        (unsigned int)root + kNestedInnerMapOffset != 0 &&
                        (unsigned int)root + kNestedInnerMapOffset != 0) {
                        DtorNode* top = (DtorNode*)((CameraNestedMapNode*)root)->value.root;
                        if (top != 0) {
                            DtorNode* innerL = (DtorNode*)top->first;
                            if (innerL != 0) {
                                DtorNode* innerLL = (DtorNode*)innerL->first;
                                if (innerLL != 0) {
                                    DtorNode* innerLLL = (DtorNode*)innerLL->first;
                                    if (innerLLL != 0) {
                                        DestroyInnerFrontier(&((CameraNestedMapNode*)root)->value, innerLLL);
                                    }
                                    DtorNode* innerLLR = (DtorNode*)innerLL->second;
                                    if (innerLLR != 0) {
                                        DestroyInnerFrontier(&((CameraNestedMapNode*)root)->value, innerLLR);
                                    }
                                    ReleaseInnerNode(innerLL);
                                }
                                DtorNode* innerLR = (DtorNode*)innerL->second;
                                if (innerLR != 0) {
                                    DtorNode* innerLRL = (DtorNode*)innerLR->first;
                                    if (innerLRL != 0) {
                                        DestroyInnerFrontier(&((CameraNestedMapNode*)root)->value, innerLRL);
                                    }
                                    DtorNode* innerLRR = (DtorNode*)innerLR->second;
                                    if (innerLRR != 0) {
                                        DestroyInnerFrontier(&((CameraNestedMapNode*)root)->value, innerLRR);
                                    }
                                    ReleaseInnerNode(innerLR);
                                }
                                ReleaseInnerNode(innerL);
                            }
                            DtorNode* innerR = (DtorNode*)top->second;
                            if (innerR != 0) {
                                DtorNode* innerRL = (DtorNode*)innerR->first;
                                if (innerRL != 0) {
                                    DtorNode* innerRLL = (DtorNode*)innerRL->first;
                                    if (innerRLL != 0) {
                                        DestroyInnerFrontier(&((CameraNestedMapNode*)root)->value, innerRLL);
                                    }
                                    DtorNode* innerRLR = (DtorNode*)innerRL->second;
                                    if (innerRLR != 0) {
                                        DestroyInnerFrontier(&((CameraNestedMapNode*)root)->value, innerRLR);
                                    }
                                    ReleaseInnerNode(innerRL);
                                }
                                DtorNode* innerRR = (DtorNode*)innerR->second;
                                if (innerRR != 0) {
                                    DtorNode* innerRRL = (DtorNode*)innerRR->first;
                                    if (innerRRL != 0) {
                                        DestroyInnerFrontier(&((CameraNestedMapNode*)root)->value, innerRRL);
                                    }
                                    DtorNode* innerRRR = (DtorNode*)innerRR->second;
                                    if (innerRRR != 0) {
                                        DestroyInnerFrontier(&((CameraNestedMapNode*)root)->value, innerRRR);
                                    }
                                    ReleaseInnerNode(innerRR);
                                }
                                ReleaseInnerNode(innerR);
                            }
                            ReleaseInnerNode(top);
                        }
                    }
                }
                appFreeUni__FPv(root);
            }
        }
        // Phase 3: only the deleting-destructor variant releases the map object.
        if (shouldDelete > 0)
            __dl__FPv(self);
    }
    return self;
}
#pragma pop

// Default vector construction leaves its three components uninitialized.
void WVector_DefaultConstructNoOp(void*) {}
// Default matrix construction leaves its components uninitialized.
void WMatrix_DefaultConstructNoOp(void*) {}
// Default quaternion construction leaves its components uninitialized.
void CameraQuaternion_DefaultConstruct(void*) {}

// Return the larger signed value.
int Math_MaxInt(int a, int b) {
    if (a < b) {
        return b;
    }
    return a;
}

// Default input-offset construction leaves its fields uninitialized.
void CameraInputOffset_DefaultConstruct(void*) {}

// Initialize an empty action-history vector without allocating storage.
void CameraActionHistory_InitEmpty(int* p) {
    // CameraActionVector slots: [0] data, [1] size, [2] capacity.
    p[0] = 0;
    p[1] = 0;
    p[2] = 0;
}

// Initialize an empty string alias tree whose end iterator points at its root slot.
void CameraStringAliasTree_InitEmpty(int* p) {
    // CameraMapHeader slots: [0] size, [1] root, [2] opaque/untouched, [3] first/end.
    // The sentinel is the ADDRESS of the root slot, not the root pointer value.
    p[0] = 0;
    p[1] = 0;
    p[3] = (int)(p + 1);
}

// Initialize an empty indexed-string tree whose end iterator points at its root slot.
void CameraIndexStringTree_InitEmpty(int* p) {
    // CameraMapHeader slots: [0] size, [1] root, [2] opaque/untouched, [3] first/end.
    // The sentinel is the ADDRESS of the root slot, not the root pointer value.
    p[0] = 0;
    p[1] = 0;
    p[3] = (int)(p + 1);
}

// Initialize an empty nested-map tree whose end iterator points at its root slot.
void CameraNestedMapTree_InitEmpty(int* p) {
    // CameraMapHeader slots: [0] size, [1] root, [2] opaque/untouched, [3] first/end.
    // The sentinel is the ADDRESS of the root slot, not the root pointer value.
    p[0] = 0;
    p[1] = 0;
    p[3] = (int)(p + 1);
}

} // extern "C"

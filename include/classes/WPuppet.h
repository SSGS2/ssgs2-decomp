// WPuppet: layout reviewed in wpuppet_model_bone_animation.cpp (0x8004FC28-0x8005DA74).
// Plain layout only: no inheritance, virtual declarations, constructors, or RTTI.
// Casts on named fields preserve the source access types; the model resource also has word views.
#ifndef CLASSES_WPUPPET_H
#define CLASSES_WPUPPET_H

#include "game/wvector.h"

// Intrusive instance bookkeeping: fn_800516C4_next and WPuppetOwnedInstances_dtor/B030.
struct WPuppetInstanceList {
    void* first; // 0x00
    void* cursor; // 0x04
    void* ownedHead; // 0x08
    char pad_0C[0x10];
    void* storage; // 0x1C
    char mutex[0x18]; // 0x20: passed to OS mutex lock/unlock
};

// Circular-list links used by child and cached-bone lists in this unit.
struct WPuppetListNode {
    void* prev; // 0x00
    void* next; // 0x04
    void* value; // 0x08
};

struct Params50BD4 {
    unsigned int a0, a1, a2, a3, a4, a5, a6, a7, a8, a9;
    unsigned int a10, a11, a12, a13, a14, a15, a16, a17, a18;
    float f0, f1, f2, f3, f4, f5, f6;
};

class WPuppet {
public:
    void* vtbl; // 0x00: WPuppet_dtor restores __vt__7WPuppet
    char pad_004[0xC];
    void* resourceManager; // 0x10: WPuppet_LoadPET calls Resource_OpenResolvedPath
    char pad_014[4];
    WPuppetInstanceList ownedInstances; // 0x18: WPuppet_dtor frees this list
    float worldMatrix[12]; // 0x50: Model_SetMatrixRecursive copies an affine matrix
    WPuppetInstanceList animationInstances; // 0x80: WPuppet_GetInverseRootMotionOffset iterates this list
    // 0xB8: WPuppet_SetHierarchyContextRecursive propagates this pointer to children; purpose unknown
    void* hierarchyContext;
    // 0xBC: f54_has / WPuppet_dtor gate resource-name lookups
    unsigned char hasResourceName;
    char pad_0BD[0x3];
    // 0xC0: WPuppet_LoadKeyRecordsFromChunk allocates 12-byte key records; WPuppet_FindKeyRange searches their keys
    void* keyRecords;
    // 0xC4: WPuppet_LoadKeyRecordsFromChunk stores the key-record count
    int keyRecordCount;
    char resourceName[0x40]; // 0xC8: WPuppet_LoadPET copies the model basename here
    // 0x108: WPuppet_LoadModelFromArchive counts index triples with three distinct vertices
    unsigned int triangleCount;
    WVector boundsMin; // 0x10C: WPuppet_AccumulateAnimationBounds accumulates bounds; WPuppet_UpdateBound computes their extent
    WVector boundsMax; // 0x118: WPuppet_AccumulateAnimationBounds accumulates the other bounds endpoint
    WVector boundsCenter; // 0x124: WPuppet_UpdateBoundsFromBone averages the transformed endpoints
    // 0x130: WPuppet_UpdateBoundsFromBone computes distance from the center to an endpoint
    float boundsRadius;
    // 0x134: WPuppet_UpdateBoundsFromBone transforms bounds using this bone at +0x78
    void* boundsBone;
    // 0x138: WPuppet_UpdateBoundsFromBone reads the two bounding endpoints here
    void* boundsData;
    // 0x13C: WPuppet_Share builds name/bone pairs; WPuppet_FindNamedBoneEntry searches by name
    void* namedBones;
    // 0x140: WPuppet_Share allocates namedBoneCount * 8 bytes
    unsigned int namedBoneCount;
    // 0x144: WPuppet_SetAnimationTimeFromRanges searches start/end times and selects animation index
    void* animationRanges;
    // 0x148: WPuppet_FindOrCacheBone increments after inserting a cached bone
    int cachedBoneCount;
    // 0x14C: WPuppet_FindOrCacheBone uses +0x14C as the cached-bone sentinel
    void* cachedBoneTail;
    // 0x150: WPuppet_FindOrCacheBone starts cached-bone traversal here
    void* cachedBoneHead;
    // 0x154: WPuppet_LoadFromChunks stores the animation-loader return value
    float animationDuration;
    // 0x158: WPuppet_LoadFromChunks stores flags controlling resource loaders
    int loadFlags;
    // 0x15C: WPuppet_MakeClone copies boneCount objects of size 0xD8
    int boneCount;
    // 0x160: WPuppet_MakeClone allocates/copies the bone array
    void* bones;
    // 0x164: WPuppet_LoadBonesFromChunks builds the bone hierarchy; WPuppet_FindBoneByName searches it
    void* boneRoot;
    // 0x168: WPuppet_LoadMaterialTextures loads the model resource; resource APIs query it
    union {
        void* modelResource;
        unsigned int modelResourceWord;
        unsigned int* modelResourceWords;
    };
    // 0x16C: WPuppet_AllocateSkinMatrices allocates matrices for the snrt resource
    void* skinMatrices;
    // 0x170: WPuppet_RefreshMatrixBuffers allocates ft-count * 0x30 and fills matrices
    void* boneMatrices;
    // 0x174: WPuppet_UpdateSkinFromAnimationSource returns early when this byte is set
    unsigned char animationDisabled;
    char pad_175[0x3];
    // 0x178: WPuppet_ApplyNamedResourcePairRecursive uses List_GetByteByKey before material/resource updates
    void* nameIndexMap;
    // 0x17C: WPuppet_LoadModelAndMaterialsFromChunks builds the 0xF0-byte material vector
    void* materials;
    // 0x180: WPuppet_MakeClone / WPuppet_LoadModelAndMaterialsFromChunks maintain material-vector size
    unsigned int materialCount;
    // 0x184: WPuppet_MakeClone / WPuppet_LoadModelAndMaterialsFromChunks maintain material-vector capacity
    unsigned int materialCapacity;
    // 0x188: copied 0x1C-byte mtrl records; texture IDs at +0xC used by WPuppet_LoadMaterialTextures
    void* materialMetadata;
    // 0x18C: owned model-layer words; 1 = visible, 0 = hidden; copied as ldm-count * 4 bytes
    void* modelLayerVisibilityWords;
    // 0x190: WPuppet_InsertChildBySortKey increments after inserting a child
    int childCount;
    // 0x194: WPuppet_InsertChildBySortKey / recursive methods use this address as list sentinel
    void* childTail;
    // 0x198: recursive methods start child traversal here
    void* childHead;
    // 0x19C: WPuppet_LoadFromChunks constructs CPetAnimation here
    void* animation;
    // 0x1A0: WPuppet_LoadKeyRecordsFromChunk allocates a pair of 16-bit key-range values
    void* keyRange;
    // 0x1A4: WPuppet_LinkBonesFromModelResource caches the root bone; WPuppet_FindOrCacheBone inherits it
    void* defaultBone;
    // 0x1A8: WPuppet_SetRenderParams copies parameters passed to render state 7
    Params50BD4 renderParams;
    // 0x210: WPuppet_RenderWithCamera/5 gate application of renderParams
    void* renderParamsEnabled;
    // 0x214: WPuppet_InsertChildBySortKey orders children by this value
    int boneSortKey;
    // 0x218: WPuppet_ReplaceChildAndLayerAnimation allocates pt-count * 0x40 bytes
    void* layerMatrices;
    // 0x21C: WPuppet_ReplaceChildAndLayerAnimation allocates a 0x40-byte output matrix
    void* layerScratch;
    // 0x220: WPuppet_ReplaceChildAndLayerAnimation creates controller; WPuppet_ApplyNamedMaterialLayerAnimation applies it
    void* layerController;
    // 0x224: f2368_ensure locates the resource containing dpt / pt layers
    void* layerResource;
    // 0x228: WPuppet_RenderWithCamera/5 copy this value into a render-state list
    unsigned int renderStateValue;
    // 0x22C: WPuppet_LoadCollisionChunks allocates bound objects; WPuppet_ReleaseNamedBoneData destroys them
    void* boundObjects;
    // 0x230: WPuppet_LoadNamedResourcesFromChunk / WPuppet_PreloadNamedResource maintain named-resource vector
    void* namedResources;
    // 0x234: WPuppet_RebuildBoneController creates it from ft; WPuppet_SetBoneControllerVector updates it
    void* boneController;
    // 0x238: WPuppet_RenderWithCamera passes it to gtl_GenericRenderer_SetCullingMode before and after rendering
    void* rendererOption;
};

#define WPUPPET_OFF(f, o) typedef char WPuppet_off_##f[((int)&((WPuppet*)0)->f == (o)) ? 1 : -1]
WPUPPET_OFF(vtbl, 0x0);
WPUPPET_OFF(resourceManager, 0x10);
WPUPPET_OFF(ownedInstances, 0x18);
WPUPPET_OFF(worldMatrix, 0x50);
WPUPPET_OFF(animationInstances, 0x80);
WPUPPET_OFF(hierarchyContext, 0xB8);
WPUPPET_OFF(hasResourceName, 0xBC);
WPUPPET_OFF(keyRecords, 0xC0);
WPUPPET_OFF(keyRecordCount, 0xC4);
WPUPPET_OFF(resourceName, 0xC8);
WPUPPET_OFF(triangleCount, 0x108);
WPUPPET_OFF(boundsMin, 0x10C);
WPUPPET_OFF(boundsMax, 0x118);
WPUPPET_OFF(boundsCenter, 0x124);
WPUPPET_OFF(boundsRadius, 0x130);
WPUPPET_OFF(boundsBone, 0x134);
WPUPPET_OFF(boundsData, 0x138);
WPUPPET_OFF(namedBones, 0x13C);
WPUPPET_OFF(namedBoneCount, 0x140);
WPUPPET_OFF(animationRanges, 0x144);
WPUPPET_OFF(cachedBoneCount, 0x148);
WPUPPET_OFF(cachedBoneTail, 0x14C);
WPUPPET_OFF(cachedBoneHead, 0x150);
WPUPPET_OFF(animationDuration, 0x154);
WPUPPET_OFF(loadFlags, 0x158);
WPUPPET_OFF(boneCount, 0x15C);
WPUPPET_OFF(bones, 0x160);
WPUPPET_OFF(boneRoot, 0x164);
WPUPPET_OFF(modelResource, 0x168);
WPUPPET_OFF(modelResourceWord, 0x168);
WPUPPET_OFF(modelResourceWords, 0x168);
WPUPPET_OFF(skinMatrices, 0x16C);
WPUPPET_OFF(boneMatrices, 0x170);
WPUPPET_OFF(animationDisabled, 0x174);
WPUPPET_OFF(nameIndexMap, 0x178);
WPUPPET_OFF(materials, 0x17C);
WPUPPET_OFF(materialCount, 0x180);
WPUPPET_OFF(materialCapacity, 0x184);
WPUPPET_OFF(materialMetadata, 0x188);
WPUPPET_OFF(modelLayerVisibilityWords, 0x18C);
WPUPPET_OFF(childCount, 0x190);
WPUPPET_OFF(childTail, 0x194);
WPUPPET_OFF(childHead, 0x198);
WPUPPET_OFF(animation, 0x19C);
WPUPPET_OFF(keyRange, 0x1A0);
WPUPPET_OFF(defaultBone, 0x1A4);
WPUPPET_OFF(renderParams, 0x1A8);
WPUPPET_OFF(renderParamsEnabled, 0x210);
WPUPPET_OFF(boneSortKey, 0x214);
WPUPPET_OFF(layerMatrices, 0x218);
WPUPPET_OFF(layerScratch, 0x21C);
WPUPPET_OFF(layerController, 0x220);
WPUPPET_OFF(layerResource, 0x224);
WPUPPET_OFF(renderStateValue, 0x228);
WPUPPET_OFF(boundObjects, 0x22C);
WPUPPET_OFF(namedResources, 0x230);
WPUPPET_OFF(boneController, 0x234);
WPUPPET_OFF(rendererOption, 0x238);
#undef WPUPPET_OFF
typedef char WPuppet_size[(sizeof(WPuppet) == 0x23C) ? 1 : -1];
#define WPUPPET_LIST_OFF(T, f, o) typedef char T##_off_##f[((int)&((T*)0)->f == (o)) ? 1 : -1]
WPUPPET_LIST_OFF(WPuppetInstanceList, first, 0);
WPUPPET_LIST_OFF(WPuppetInstanceList, cursor, 4);
WPUPPET_LIST_OFF(WPuppetInstanceList, ownedHead, 8);
WPUPPET_LIST_OFF(WPuppetInstanceList, storage, 0x1C);
WPUPPET_LIST_OFF(WPuppetInstanceList, mutex, 0x20);
WPUPPET_LIST_OFF(WPuppetListNode, prev, 0);
WPUPPET_LIST_OFF(WPuppetListNode, next, 4);
WPUPPET_LIST_OFF(WPuppetListNode, value, 8);
WPUPPET_LIST_OFF(Params50BD4, a0, 0x00);
WPUPPET_LIST_OFF(Params50BD4, a1, 0x04);
WPUPPET_LIST_OFF(Params50BD4, a2, 0x08);
WPUPPET_LIST_OFF(Params50BD4, a3, 0x0C);
WPUPPET_LIST_OFF(Params50BD4, a4, 0x10);
WPUPPET_LIST_OFF(Params50BD4, a5, 0x14);
WPUPPET_LIST_OFF(Params50BD4, a6, 0x18);
WPUPPET_LIST_OFF(Params50BD4, a7, 0x1C);
WPUPPET_LIST_OFF(Params50BD4, a8, 0x20);
WPUPPET_LIST_OFF(Params50BD4, a9, 0x24);
WPUPPET_LIST_OFF(Params50BD4, a10, 0x28);
WPUPPET_LIST_OFF(Params50BD4, a11, 0x2C);
WPUPPET_LIST_OFF(Params50BD4, a12, 0x30);
WPUPPET_LIST_OFF(Params50BD4, a13, 0x34);
WPUPPET_LIST_OFF(Params50BD4, a14, 0x38);
WPUPPET_LIST_OFF(Params50BD4, a15, 0x3C);
WPUPPET_LIST_OFF(Params50BD4, a16, 0x40);
WPUPPET_LIST_OFF(Params50BD4, a17, 0x44);
WPUPPET_LIST_OFF(Params50BD4, a18, 0x48);
WPUPPET_LIST_OFF(Params50BD4, f0, 0x4C);
WPUPPET_LIST_OFF(Params50BD4, f1, 0x50);
WPUPPET_LIST_OFF(Params50BD4, f2, 0x54);
WPUPPET_LIST_OFF(Params50BD4, f3, 0x58);
WPUPPET_LIST_OFF(Params50BD4, f4, 0x5C);
WPUPPET_LIST_OFF(Params50BD4, f5, 0x60);
WPUPPET_LIST_OFF(Params50BD4, f6, 0x64);
#undef WPUPPET_LIST_OFF
typedef char Params50BD4_size[(sizeof(Params50BD4) == 0x68) ? 1 : -1];
typedef char WPuppetInstanceList_size[(sizeof(WPuppetInstanceList) == 0x38) ? 1 : -1];
typedef char WPuppetListNode_size[(sizeof(WPuppetListNode) == 12) ? 1 : -1];

#define AsWPuppet(p) ((WPuppet*)(p))

#endif // CLASSES_WPUPPET_H

// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CEXHIBITION_H
#define CLASSES_CEXHIBITION_H

#include "game/types.h"

class CExhibition {
public:
    void* vtbl;                         // 0x0: original vtable pointer
    int ownedNameStrings;                          // 0x4: 3 uses in CExhibition_Ctor, CExhibition_dtor, ModelOperationBase_IsResourceLoaded
    unsigned int ownedNameStringCount;                 // 0x8: 3 uses in CExhibition_dtor, CExhibition_Ctor
    int ownedNameStringCapacity;                          // 0xc: 1 use in CExhibition_Ctor
    unsigned char unk_10;               // 0x10: 1 use in CExhibition_Ctor
    char pad_011[0x3];                  // 0x11
    float rotationX;                       // 0x14: 7 uses in CExhibition_Ctor
    float rotationY;                       // 0x18: 7 uses in CExhibition_Ctor
    float rotationZ;                       // 0x1c: 4 uses in CExhibition_Ctor
    float animationDelay;                       // 0x20: 1 use in CExhibition_Ctor
    float animationCycleDuration;                       // 0x24: 1 use in CExhibition_Ctor
    float opacity;                       // 0x28: 1 use in CExhibition_Ctor
    float cameraOffsetZ;                       // 0x2c: 4 uses in CExhibition_Ctor
    float screenCenterX;                       // 0x30: 3 uses in CExhibition_Ctor, CExhibition_CreateAttachedEffectAtScreenCenter
    float screenCenterY;                       // 0x34: 3 uses in CExhibition_Ctor, CExhibition_CreateAttachedEffectAtScreenCenter
    char cameraMatrixPrefix[0x2c];                 // 0x38
    float cameraDistance;                       // 0x64: 1 use in CExhibition_Ctor
    float animationFrame;                       // 0x68: 1 use in CExhibition_Ctor
    void* animationRange;                       // 0x6c: 3 uses in CExhibition_Ctor, CExhibition_ParseAnimationGatedCommand, CExhibition_ParseBoneReferences
    void* modelController;                       // 0x70: 5 uses in CExhibition_dtor, CExhibition_Ctor, CExhibition_PlayAnimationIfPartPresent...
    void* puppet;                       // 0x74: 6 uses in CExhibition_dtor, CExhibition_Ctor, CExhibition_ParseBoneReferences
    char modelTransform[0x30];                 // 0x78
    float unk_A8;                       // 0xa8: 1 use in CExhibition_Ctor
    float unk_AC;                       // 0xac: 1 use in CExhibition_Ctor
    char pad_0B0[0x4];                  // 0xb0
    float unk_B4;                       // 0xb4: 1 use in CExhibition_Ctor
    char pad_0B8[0x154];
    void* sprayTreeFirst;                     // 0x20c: CExhibition_UpdateEffectScreenCenter iterates a child tree from this node.
    char pad_210[8];
    void* sequenceTreeFirst;                     // 0x218: CExhibition_UpdateEffectScreenCenter iterates a second child tree.
    char pad_21C[0x228];
    int attachedSprays;                        // 0x444: 1 use in CExhibition_Ctor
    int attachedSprayCount;                        // 0x448: 1 use in CExhibition_Ctor
    int attachedSprayCapacity;                        // 0x44c: 1 use in CExhibition_Ctor
    int attachedSequences;                        // 0x450: 1 use in CExhibition_Ctor
    int attachedSequenceCount;                        // 0x454: 1 use in CExhibition_Ctor
    int attachedSequenceCapacity;                        // 0x458: 1 use in CExhibition_Ctor
    int ownedPreviewListCount;                        // 0x45c: 1 use in CExhibition_Ctor
    void* ownedPreviewListSentinelNext;                      // 0x460: 1 use in CExhibition_Ctor
    void* ownedPreviewListSentinelPrev;                      // 0x464: 2 uses in CExhibition_Ctor, CExhibition_dtor
    int modelKind;                        // 0x468: 5 uses in CExhibition_Ctor, CExhibition_dtor
    int itemTypeId;                        // 0x46c: 1 use in CExhibition_Ctor
    int avatarResourceOptions;                        // 0x470: 1 use in CExhibition_Ctor
    unsigned int avatarCharacterId;            // 0x474: CExhibition_ConfigureAvatarPreview stores the header preceding avatarSelectionData.
    char avatarSelectionData[0x64];               // 0x478: CExhibition_ConfigureAvatarPreview copies the supplied description here.
    unsigned char partLoadPending;              // 0x4dc: 1 use in CExhibition_Ctor
    char pad_4DD[0x3];                  // 0x4dd
    int viewportLeft;                        // 0x4e0: 2 uses in CExhibition_Ctor
    int viewportTop;                        // 0x4e4: 2 uses in CExhibition_Ctor
    int viewportRight;                        // 0x4e8: 2 uses in CExhibition_Ctor
    int viewportBottom;                        // 0x4ec: 4 uses in CExhibition_Ctor
    float translationX;                      // 0x4f0: 4 uses in CExhibition_Ctor
    float translationY;                      // 0x4f4: 6 uses in CExhibition_Ctor
    int loadState;                        // 0x4f8: 1 use in CExhibition_Ctor
    unsigned int animationNameFlags;               // 0x4fc: 2 uses in CExhibition_Ctor, CExhibition_dtor
    int animationNameLength;                        // 0x500: 1 use in CExhibition_Ctor
    int animationNameData;                        // 0x504: 2 uses in CExhibition_Ctor, CExhibition_dtor
    float maxAnimationDelay;                      // 0x508: 1 use in CExhibition_Ctor
    int textureVariantIndex;                        // 0x50c: 1 use in CExhibition_Ctor
    int modelPreloader;                        // 0x510: 1 use in CExhibition_Ctor
    unsigned char animationMode;              // 0x514: 1 use in CExhibition_Ctor
    char pad_515[0x3];                  // 0x515
    unsigned int modelPathFlags;               // 0x518: 2 uses in CExhibition_Ctor, CExhibition_dtor
    int modelPathLength;                        // 0x51c: 1 use in CExhibition_Ctor
    int modelPathData;                        // 0x520: 2 uses in CExhibition_Ctor, CExhibition_dtor
    int partReloadMask;                        // 0x524: 1 use in CExhibition_Ctor
    int lightingResource;                        // 0x528: 3 uses in CExhibition_dtor, CExhibition_Ctor
    int preloadFile;                        // 0x52c: 3 uses in CExhibition_dtor, CExhibition_Ctor
    int lights;                        // 0x530: 2 uses in CExhibition_Ctor, CExhibition_dtor
    unsigned int lightCount;               // 0x534: 3 uses in CExhibition_dtor, CExhibition_Ctor
    int lightCapacity;                        // 0x538: 1 use in CExhibition_Ctor
    unsigned char displaySuppressed;              // 0x53c: 1 use in CExhibition_Ctor
    char pad_53D[0xf];                  // 0x53d
    int loadWaitFrames;                        // 0x54c: 1 use in CExhibition_Ctor
    unsigned int resolvedCharacterId;           // 0x550: CExhibition_UpdateLoadState copies the selected description header.
    char resolvedPartData[0xc4];              // 0x554: CExhibition_UpdateLoadState copies the remaining description here.
    float minRotationX;                      // 0x618: 1 use in CExhibition_Ctor
    float maxRotationX;                      // 0x61c: 1 use in CExhibition_Ctor
    float minRotationY;                      // 0x620: 1 use in CExhibition_Ctor
    float maxRotationY;                      // 0x624: 1 use in CExhibition_Ctor
    float minCameraDistance;                      // 0x628: 2 uses in CExhibition_Ctor
    float maxCameraDistance;                      // 0x62c: 2 uses in CExhibition_Ctor
    int screenCenterMinX;                        // 0x630: 1 use in CExhibition_Ctor
    int screenCenterMaxX;                        // 0x634: 1 use in CExhibition_Ctor
    int screenCenterMinY;                        // 0x638: 1 use in CExhibition_Ctor
    int screenCenterMaxY;                        // 0x63c: 1 use in CExhibition_Ctor

    // offsets whose address is passed on (sub-objects or arrays, not placed): 0x4, 0x8, 0x38, 0x78, 0xb0, 0x42c, 0x438, 0x444, 0x450, 0x45c, 0x460, 0x474, 0x4fc, 0x518, 0x530, 0x534, 0x540, 0x550, 0x640
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define CEXHIBITION_OFF(f, o) typedef char CExhibition_off_##f[((int)&((CExhibition*)0)->f == (o)) ? 1 : -1]
CEXHIBITION_OFF(vtbl, 0x0);
CEXHIBITION_OFF(ownedNameStrings, 0x4);
CEXHIBITION_OFF(ownedNameStringCount, 0x8);
CEXHIBITION_OFF(ownedNameStringCapacity, 0xc);
CEXHIBITION_OFF(unk_10, 0x10);
CEXHIBITION_OFF(rotationX, 0x14);
CEXHIBITION_OFF(rotationY, 0x18);
CEXHIBITION_OFF(rotationZ, 0x1c);
CEXHIBITION_OFF(animationDelay, 0x20);
CEXHIBITION_OFF(animationCycleDuration, 0x24);
CEXHIBITION_OFF(opacity, 0x28);
CEXHIBITION_OFF(cameraOffsetZ, 0x2c);
CEXHIBITION_OFF(screenCenterX, 0x30);
CEXHIBITION_OFF(screenCenterY, 0x34);
CEXHIBITION_OFF(cameraDistance, 0x64);
CEXHIBITION_OFF(animationFrame, 0x68);
CEXHIBITION_OFF(animationRange, 0x6c);
CEXHIBITION_OFF(modelController, 0x70);
CEXHIBITION_OFF(puppet, 0x74);
CEXHIBITION_OFF(unk_A8, 0xa8);
CEXHIBITION_OFF(unk_AC, 0xac);
CEXHIBITION_OFF(unk_B4, 0xb4);
CEXHIBITION_OFF(attachedSprays, 0x444);
CEXHIBITION_OFF(attachedSprayCount, 0x448);
CEXHIBITION_OFF(attachedSprayCapacity, 0x44c);
CEXHIBITION_OFF(attachedSequences, 0x450);
CEXHIBITION_OFF(attachedSequenceCount, 0x454);
CEXHIBITION_OFF(attachedSequenceCapacity, 0x458);
CEXHIBITION_OFF(ownedPreviewListCount, 0x45c);
CEXHIBITION_OFF(ownedPreviewListSentinelNext, 0x460);
CEXHIBITION_OFF(ownedPreviewListSentinelPrev, 0x464);
CEXHIBITION_OFF(modelKind, 0x468);
CEXHIBITION_OFF(itemTypeId, 0x46c);
CEXHIBITION_OFF(avatarResourceOptions, 0x470);
CEXHIBITION_OFF(partLoadPending, 0x4dc);
CEXHIBITION_OFF(viewportLeft, 0x4e0);
CEXHIBITION_OFF(viewportTop, 0x4e4);
CEXHIBITION_OFF(viewportRight, 0x4e8);
CEXHIBITION_OFF(viewportBottom, 0x4ec);
CEXHIBITION_OFF(translationX, 0x4f0);
CEXHIBITION_OFF(translationY, 0x4f4);
CEXHIBITION_OFF(loadState, 0x4f8);
CEXHIBITION_OFF(animationNameFlags, 0x4fc);
CEXHIBITION_OFF(animationNameLength, 0x500);
CEXHIBITION_OFF(animationNameData, 0x504);
CEXHIBITION_OFF(maxAnimationDelay, 0x508);
CEXHIBITION_OFF(textureVariantIndex, 0x50c);
CEXHIBITION_OFF(modelPreloader, 0x510);
CEXHIBITION_OFF(animationMode, 0x514);
CEXHIBITION_OFF(modelPathFlags, 0x518);
CEXHIBITION_OFF(modelPathLength, 0x51c);
CEXHIBITION_OFF(modelPathData, 0x520);
CEXHIBITION_OFF(partReloadMask, 0x524);
CEXHIBITION_OFF(lightingResource, 0x528);
CEXHIBITION_OFF(preloadFile, 0x52c);
CEXHIBITION_OFF(lights, 0x530);
CEXHIBITION_OFF(lightCount, 0x534);
CEXHIBITION_OFF(lightCapacity, 0x538);
CEXHIBITION_OFF(displaySuppressed, 0x53c);
CEXHIBITION_OFF(loadWaitFrames, 0x54c);
CEXHIBITION_OFF(minRotationX, 0x618);
CEXHIBITION_OFF(maxRotationX, 0x61c);
CEXHIBITION_OFF(minRotationY, 0x620);
CEXHIBITION_OFF(maxRotationY, 0x624);
CEXHIBITION_OFF(minCameraDistance, 0x628);
CEXHIBITION_OFF(maxCameraDistance, 0x62c);
CEXHIBITION_OFF(screenCenterMinX, 0x630);
CEXHIBITION_OFF(screenCenterMaxX, 0x634);
CEXHIBITION_OFF(screenCenterMinY, 0x638);
CEXHIBITION_OFF(screenCenterMaxY, 0x63c);
CEXHIBITION_OFF(sprayTreeFirst, 0x20C);
CEXHIBITION_OFF(sequenceTreeFirst, 0x218);
CEXHIBITION_OFF(avatarCharacterId, 0x474);
CEXHIBITION_OFF(avatarSelectionData, 0x478);
CEXHIBITION_OFF(resolvedCharacterId, 0x550);
CEXHIBITION_OFF(resolvedPartData, 0x554);
#undef CEXHIBITION_OFF

#endif  // CLASSES_CEXHIBITION_H

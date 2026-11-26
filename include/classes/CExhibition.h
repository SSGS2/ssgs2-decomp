#ifndef CLASSES_CEXHIBITION_H
#define CLASSES_CEXHIBITION_H

#include "game/types.h"

class CExhibition {
public:
    void* vtbl;                        // 0x0: original vtable pointer
    int ownedNameStrings;              // 0x4
    unsigned int ownedNameStringCount; // 0x8
    int ownedNameStringCapacity;       // 0xc
    unsigned char unk_10;              // 0x10
    char pad_011[0x3];                 // 0x11
    float rotationX;                   // 0x14
    float rotationY;                   // 0x18
    float rotationZ;                   // 0x1c
    float animationDelay;              // 0x20
    float animationCycleDuration;      // 0x24
    float opacity;                     // 0x28
    float cameraOffsetZ;               // 0x2c
    float screenCenterX;               // 0x30
    float screenCenterY;               // 0x34
    char cameraMatrixPrefix[0x2c];     // 0x38
    float cameraDistance;              // 0x64
    float animationFrame;              // 0x68
    void* animationRange;              // 0x6c
    void* modelController;             // 0x70
    void* puppet;                      // 0x74
    char modelTransform[0x30];         // 0x78
    float unk_A8;                      // 0xa8
    float unk_AC;                      // 0xac
    char fxBox[0x4];                   // 0xb0
    float unk_B4;                      // 0xb4
    char pad_0B8[0x154];
    void* sprayTreeFirst; // 0x20c: CExhibition_UpdateEffectScreenCenter iterates a child tree from this node.
    char pad_210[8];
    void* sequenceTreeFirst; // 0x218: CExhibition_UpdateEffectScreenCenter iterates a second child tree.
    char pad_21C[0x228];
    int attachedSprays;                 // 0x444
    int attachedSprayCount;             // 0x448
    int attachedSprayCapacity;          // 0x44c
    int attachedSequences;              // 0x450
    int attachedSequenceCount;          // 0x454
    int attachedSequenceCapacity;       // 0x458
    int ownedPreviewListCount;          // 0x45c
    void* ownedPreviewListSentinelNext; // 0x460
    void* ownedPreviewListSentinelPrev; // 0x464
    int modelKind;                      // 0x468
    int itemTypeId;                     // 0x46c
    int avatarResourceOptions;          // 0x470
    unsigned int
        avatarCharacterId; // 0x474: CExhibition::ConfigureAvatarPreview stores the header preceding avatarSelectionData.
    char avatarSelectionData[0x64];  // 0x478: CExhibition::ConfigureAvatarPreview copies the supplied description here.
    unsigned char partLoadPending;   // 0x4dc
    char pad_4DD[0x3];               // 0x4dd
    int viewportLeft;                // 0x4e0
    int viewportTop;                 // 0x4e4
    int viewportRight;               // 0x4e8
    int viewportBottom;              // 0x4ec
    float translationX;              // 0x4f0
    float translationY;              // 0x4f4
    int loadState;                   // 0x4f8
    unsigned int animationNameFlags; // 0x4fc
    int animationNameLength;         // 0x500
    int animationNameData;           // 0x504
    float maxAnimationDelay;         // 0x508
    int textureVariantIndex;         // 0x50c
    int modelPreloader;              // 0x510
    unsigned char animationMode;     // 0x514
    char pad_515[0x3];               // 0x515
    unsigned int modelPathFlags;     // 0x518
    int modelPathLength;             // 0x51c
    int modelPathData;               // 0x520
    int partReloadMask;              // 0x524
    int lightingResource;            // 0x528
    int preloadFile;                 // 0x52c
    int lights;                      // 0x530
    unsigned int lightCount;         // 0x534
    int lightCapacity;               // 0x538
    unsigned char displaySuppressed; // 0x53c
    char pad_53D[0xf];               // 0x53d
    int loadWaitFrames;              // 0x54c
    unsigned int resolvedCharacterId; // 0x550: CExhibition::UpdateLoadState copies the selected description header.
    char resolvedPartData[0xc4];      // 0x554: CExhibition::UpdateLoadState copies the remaining description here.
    float minRotationX;               // 0x618
    float maxRotationX;               // 0x61c
    float minRotationY;               // 0x620
    float maxRotationY;               // 0x624
    float minCameraDistance;          // 0x628
    float maxCameraDistance;          // 0x62c
    int screenCenterMinX;             // 0x630
    int screenCenterMaxX;             // 0x634
    int screenCenterMinY;             // 0x638
    int screenCenterMaxY;             // 0x63c

    static int model_pos_vfunc_1();
    static int model_pos_vfunc_2();
    static int model_pos_vfunc_3();
    static int model_pos_vfunc_4();
    static int model_pos_vfunc_5();
    static int model_pos_vfunc_6();
    static int model_pos_vfunc_7();
    static int model_pos_vfunc_8();
    static int model_pos_vfunc_9();
    static int model_pos_vfunc_10();
    static int model_pos_vfunc_11();
    static int model_pos_vfunc_12();
    static int model_pos_vfunc_13();
    static float model_pos_GetCameraDistance(void* __restrict p, int index);
    static float model_pos_GetCameraDistanceLimit1(void* __restrict p, int index);
    static float model_pos_GetCameraDistanceLimit2(void* __restrict p, int index);
    static float model_pos_GetViewportCenterX(void* __restrict p, int index);
    static float model_pos_GetViewportBottom(void* __restrict p, int index);
    static float model_pos_GetRotationX(void* __restrict p, int index);
    static float model_pos_GetRotationLimit1(const void* p, int index);
    static float model_pos_GetRotationLimit2(const void* p, int index);
    static float model_pos_GetRotationY(const void* p, int index);

    int OffsetViewport(int x, int y);
    void SetItemType(unsigned int itemTypeId);
    int SetCameraDistance(float cameraDistance);
    float GetCameraDistance();
    int SetRotationLimits(float minimumAngle, float maximumAngle);
    int SetViewportYLimits(unsigned int minimumY, unsigned int maximumY);
    int SetViewport(int* rect, float cameraDistance);
    void ConfigureAvatarPreview(void* selectedDescription, void* resourceOptions, const char* animationName,
                                void* textureVariant, float maximumAnimationDelay);
    int SetRotation(const float* rotationAngles);
    void* GetRotation();
    int SetAttachment(unsigned int index, void* value);
    int PlayAnimationByName(void* nameOrAnimationRange, int flag, float delta);
    void UpdateLoadState(float dt, int); // callers pass a second int that the body does not read
    void DrawScene();
    int ClearControlFlag(int); // callers pass a second int that the body does not read
    int SetCameraDistanceLimits(float minimumDistance, float maximumDistance);
    int ApplyTextureVariant(void* textureVariant, int variantIndex);
    int CopyResolvedAvatarSelection(void* out);
    int SetVisible(int value);
    int GetCallbackResult(int callbackAddress, void* callbackContext);
    int ClearModelPreloader();
    int SetModelPreloader(int value);
    int QueueModelUpdate(void* selectedDescription, void* resolvedDescription, int reloadMask);
    int IsReady();
    void CreateBallEffects();
    int UpdateEffectScreenCenter();
    void UpdateModelPose();
    void RebuildAttachedEffects();
    int InitializeLoadedModel();
    int UpdateAnimationAndEffects(float dt);
    int LoadEnvironmentLights();
    int ParseMaterialLayerAnimationCommand(int arg2);
    void HideNamedPuppetBones(void* scriptCursor);
    void ParseAnimationGatedCommand(void* scriptCursor);
    void PlayAnimationIfPartPresent(void* scriptCursor);
    void vfunc_21();
    int ParseConditionedQuotedTokenImpl(int arg2, int arg3, int arg4);
    int CreateAttachedEffectAtScreenCenter(float* output, int arg3);
    int ParseBoneReferences(int arg2, int arg3);
    void PlaySoundCommand(void* scriptCursor);

    CExhibition(int* viewportRect, float cameraDistance);
    ~CExhibition();
};

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

#endif // CLASSES_CEXHIBITION_H

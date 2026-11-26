// CPvsFxBox: the course effect box.
//
// It owns the sky, the weather particles and the course/scene effect managers,
// and it also provides the storage the effect-box wrapper is built on. The
// wrapper stores its vtable in the first word; the sky storage keeps the frame
// time waiting for the next update there instead, so the first two words are
// unions.
//
// The class is declared without virtual functions on purpose: its vtable is
// still taken from the original game data, so `vtbl` is spelled out as a member.
#ifndef CLASSES_CPVSFXBOX_H
#define CLASSES_CPVSFXBOX_H

#include "game/types.h"
#include "game/wvector.h"

class CPvsFxBox {
public:
    // Storage lifecycle. The real constructor and destructor are the effect-box
    // wrapper's; Init and Destroy set up and release this sky/particle storage.
    CPvsFxBox();
    ~CPvsFxBox();
    CPvsFxBox* Init();
    CPvsFxBox* Destroy(int deallocate);
    void ReleaseResources();

    // Course environment.
    void ReloadCourseEffects();
    void RebuildSceneEffects();
    void RebuildAuxiliaryEffect();
    void SetPendingDelta(float deltaSeconds);

    // Sky.
    void LoadSkyResources(const char* text, float scale);
    void DrawEnabledEffects(void* camera);
    void DrawSkyCaps(void* camera);
    void DrawNearSkyStrip(void* camera);
    void DrawFarSkyStrip(void* camera);

    // Weather particles.
    void LoadVariantBitmap(const char* text);
    void ReleaseParticleResource();
    void InitWindParticles(int count, float spawnSeconds);
    void UpdateWindParticles();
    void DrawParticles(void* camera);

    // Attachment caches.
    void EraseSequenceAttachmentCache(unsigned int key);
    void EraseSprayAttachmentCache(unsigned int key);
    void CacheSprayAttachmentInput(void* entry);
    void CacheSequenceAttachmentInput(void* entry);
    void SetParticleColor(unsigned int value);
    static int ComputeMeanAttachmentPosition(void* items, void* context);
    void UpdateAttachedEffectPositions();
    int TestGroundCrossing(float verticalStep, float* previousPosition, float* currentPosition,
                           float* groundHeightInOut);

    // The generated-vertex buffer at the end of the storage. Its vtable pointer
    // follows the vertex count, so its members are spelled out.
    struct GeneratedGeometry {
        unsigned char vertexCount; // 0x9a0
        char pad_9A1[3];
        void* vtbl;     // 0x9a4
        void* vertices; // 0x9a8
    };

    // Retained sky graphics buffers and textures.
    struct SkyBuffers {
        void* farVertices;  // 0x9ac
        void* nearVertices; // 0x9b0
        void* farIndices;   // 0x9b4
        void* nearIndices;  // 0x9b8
        void* farTexture;   // 0x9bc
        void* nearTexture;  // 0x9c0
        void* upperTexture; // 0x9c4
        void* lowerTexture; // 0x9c8
    };

    // The first word is the wrapper's vtable and, in the sky storage, the frame
    // time waiting for the next update.
    union {
        void* vtbl;           // 0x00
        float pendingSeconds; // 0x00
    };
    // The second word is the wrapper's second vtable word and, in the sky
    // storage, whether the sky resources are loaded.
    union {
        int unk_4;              // 0x04
        unsigned int skyLoaded; // 0x04
    };
    void* farSkySourceVertices;                // 0x08
    unsigned int farSkyResourceFlags;          // 0x0c
    int unk_10;                                // 0x10
    float unk_14;                              // 0x14
    float unk_18;                              // 0x18
    float unk_1C;                              // 0x1c
    float unk_20;                              // 0x20
    float unk_24;                              // 0x24
    float unk_28;                              // 0x28
    float unk_2C;                              // 0x2c
    float unk_30;                              // 0x30
    float unk_34;                              // 0x34
    char pad_038[0x2c];                        // 0x38
    float unk_64;                              // 0x64
    void* nearSkySourceVertices;               // 0x68
    unsigned int nearSkyResourceFlags;         // 0x6c
    float unk_70;                              // 0x70
    int unk_74;                                // 0x74
    char pad_078[0x10];                        // 0x78
    float unk_88;                              // 0x88
    float unk_8C;                              // 0x8c
    float unk_90;                              // 0x90
    char pad_094[0x10];                        // 0x94
    float unk_A4;                              // 0xa4
    float unk_A8;                              // 0xa8
    float unk_AC;                              // 0xac
    float unk_B0;                              // 0xb0
    float unk_B4;                              // 0xb4
    char pad_0B8[0x10];                        // 0xb8
    void* upperSkySourceVertices;              // 0xc8
    unsigned int upperSkyResourceFlags;        // 0xcc
    char pad_0D0[0x28];                        // 0xd0
    float unk_F8;                              // 0xf8
    int unk_FC;                                // 0xfc
    int unk_100;                               // 0x100
    int unk_104;                               // 0x104
    int unk_108;                               // 0x108
    float unk_10C;                             // 0x10c
    float unk_110;                             // 0x110
    char pad_114[0x14];                        // 0x114
    void* lowerSkySourceVertices;              // 0x128
    unsigned int lowerSkyResourceFlags;        // 0x12c
    char pad_130[0x10];                        // 0x130
    float unk_140;                             // 0x140
    int unk_144;                               // 0x144
    int unk_148;                               // 0x148
    int unk_14C;                               // 0x14c
    int unk_150;                               // 0x150
    float unk_154;                             // 0x154
    float unk_158;                             // 0x158
    char pad_15C[0x24];                        // 0x15c
    float unk_180;                             // 0x180
    char pad_184[0x4];                         // 0x184
    float particleSpawnRadius;                 // 0x188
    float particleCameraFadeDistance;          // 0x18c
    float particleSpawnHeight;                 // 0x190
    void* particleBitmap;                      // 0x194
    int particleCapacity;                      // 0x198
    float unk_19C;                             // 0x19c: first wind-particle slot
    float unk_1A0;                             // 0x1a0
    float unk_1A4;                             // 0x1a4
    void* unk_1A8;                             // 0x1a8
    void* unk_1AC;                             // 0x1ac
    char pad_1B0[0x50];                        // 0x1b0
    float unk_200;                             // 0x200
    float unk_204;                             // 0x204
    char pad_208[0x4];                         // 0x208
    float unk_20C;                             // 0x20c
    char pad_210[0x44];                        // 0x210
    int unk_254;                               // 0x254
    char pad_258[0xc];                         // 0x258
    int unk_264;                               // 0x264
    float unk_268;                             // 0x268
    float unk_26C;                             // 0x26c
    int unk_270;                               // 0x270
    char pad_274[0x4];                         // 0x274
    int unk_278;                               // 0x278
    char pad_27C[0x4];                         // 0x27c
    float unk_280;                             // 0x280
    char pad_284[0xfc];                        // 0x284
    void* sprayAttachmentTreeRoot;             // 0x380
    char pad_384[0x8];                         // 0x384
    void* sequenceAttachmentTreeRoot;          // 0x38c
    char pad_390[0x1c];                        // 0x390
    unsigned int auxiliaryAttachmentTreeCount; // 0x3ac
    unsigned int auxiliaryAttachmentTreeRoot;  // 0x3b0
    void* auxiliaryAttachmentTreeRightmost;    // 0x3b4
    char unk_3B8;                              // 0x3b8
    char unk_3B9;                              // 0x3b9
    char unk_3BA;                              // 0x3ba
    char pad_3BB[0x89];                        // 0x3bb
    int unk_444;                               // 0x444
    int unk_448;                               // 0x448
    int unk_44C;                               // 0x44c
    int unk_450;                               // 0x450
    int unk_454;                               // 0x454
    int unk_458;                               // 0x458
    int unk_45C;                               // 0x45c
    void* unk_460;                             // 0x460
    void* unk_464;                             // 0x464
    int unk_468;                               // 0x468
    int unk_46C;                               // 0x46c
    int unk_470;                               // 0x470
    char pad_474[0x68];                        // 0x474
    unsigned char unk_4DC;                     // 0x4dc
    char pad_4DD[0x3];                         // 0x4dd
    int unk_4E0;                               // 0x4e0
    int unk_4E4;                               // 0x4e4
    int unk_4E8;                               // 0x4e8
    int unk_4EC;                               // 0x4ec
    float unk_4F0;                             // 0x4f0
    float unk_4F4;                             // 0x4f4
    int unk_4F8;                               // 0x4f8
    int unk_4FC;                               // 0x4fc
    int unk_500;                               // 0x500
    int unk_504;                               // 0x504
    float unk_508;                             // 0x508
    int unk_50C;                               // 0x50c
    int unk_510;                               // 0x510
    unsigned char unk_514;                     // 0x514
    char pad_515[0x3];                         // 0x515
    int unk_518;                               // 0x518
    int unk_51C;                               // 0x51c
    int unk_520;                               // 0x520
    int unk_524;                               // 0x524
    int unk_528;                               // 0x528
    int unk_52C;                               // 0x52c
    int unk_530;                               // 0x530
    int unk_534;                               // 0x534
    int unk_538;                               // 0x538
    unsigned char unk_53C;                     // 0x53c
    char pad_53D[0xf];                         // 0x53d
    int unk_54C;                               // 0x54c
    char pad_550[0x4c];                        // 0x550
    void* unk_59C;                             // 0x59c
    void* unk_5A0;                             // 0x5a0
    void* unk_5A4;                             // 0x5a4
    void* unk_5A8;                             // 0x5a8
    int unk_5AC;                               // 0x5ac
    char pad_5B0[0x60];                        // 0x5b0
    int unk_610;                               // 0x610
    char pad_614[0x4];                         // 0x614
    float unk_618;                             // 0x618
    float unk_61C;                             // 0x61c
    float unk_620;                             // 0x620
    float unk_624;                             // 0x624
    float unk_628;                             // 0x628
    float unk_62C;                             // 0x62c
    int unk_630;                               // 0x630
    int unk_634;                               // 0x634
    int unk_638;                               // 0x638
    int unk_63C;                               // 0x63c
    char pad_640[0x264];                       // 0x640
    int activeParticleCount;                   // 0x8a4
    float spawnInterval;                       // 0x8a8
    float spawnTimer;                          // 0x8ac
    float unk_8B0;                             // 0x8b0
    float skyVerticalOffset;                   // 0x8b4
    unsigned int particleColor;                // 0x8b8
    float skyAngularPhaseScale;                // 0x8bc
    int skyTextureRepeatCount;                 // 0x8c0
    int nearSkyTextureRepeatCount;             // 0x8c4
    char pad_8C8[0xc0];                        // 0x8c8
    WVector skyTranslation;                    // 0x988
    void* auxiliaryEffect;                     // 0x994
    void* courseEffects;                       // 0x998
    void* sceneEffects;                        // 0x99c
    GeneratedGeometry geometry;                // 0x9a0
    SkyBuffers skyBuffers;                     // 0x9ac
};

#define CPVSFXBOX_OFF(f, o) typedef char CPvsFxBox_off_##f[((int)&((CPvsFxBox*)0)->f == (o)) ? 1 : -1]
CPVSFXBOX_OFF(pendingSeconds, 0x0);
CPVSFXBOX_OFF(unk_4, 0x4);
CPVSFXBOX_OFF(farSkySourceVertices, 0x8);
CPVSFXBOX_OFF(farSkyResourceFlags, 0xc);
CPVSFXBOX_OFF(unk_14, 0x14);
CPVSFXBOX_OFF(nearSkySourceVertices, 0x68);
CPVSFXBOX_OFF(nearSkyResourceFlags, 0x6c);
CPVSFXBOX_OFF(upperSkySourceVertices, 0xc8);
CPVSFXBOX_OFF(upperSkyResourceFlags, 0xcc);
CPVSFXBOX_OFF(lowerSkySourceVertices, 0x128);
CPVSFXBOX_OFF(lowerSkyResourceFlags, 0x12c);
CPVSFXBOX_OFF(particleSpawnRadius, 0x188);
CPVSFXBOX_OFF(particleCameraFadeDistance, 0x18c);
CPVSFXBOX_OFF(particleSpawnHeight, 0x190);
CPVSFXBOX_OFF(particleBitmap, 0x194);
CPVSFXBOX_OFF(particleCapacity, 0x198);
CPVSFXBOX_OFF(activeParticleCount, 0x8a4);
CPVSFXBOX_OFF(spawnInterval, 0x8a8);
CPVSFXBOX_OFF(spawnTimer, 0x8ac);
CPVSFXBOX_OFF(skyVerticalOffset, 0x8b4);
CPVSFXBOX_OFF(particleColor, 0x8b8);
CPVSFXBOX_OFF(skyAngularPhaseScale, 0x8bc);
CPVSFXBOX_OFF(skyTextureRepeatCount, 0x8c0);
CPVSFXBOX_OFF(nearSkyTextureRepeatCount, 0x8c4);
CPVSFXBOX_OFF(skyTranslation, 0x988);
CPVSFXBOX_OFF(auxiliaryEffect, 0x994);
CPVSFXBOX_OFF(geometry, 0x9a0);
CPVSFXBOX_OFF(skyBuffers, 0x9ac);
CPVSFXBOX_OFF(sprayAttachmentTreeRoot, 0x380);
CPVSFXBOX_OFF(sequenceAttachmentTreeRoot, 0x38c);
#undef CPVSFXBOX_OFF

typedef char CPvsFxBox_size[(sizeof(CPvsFxBox) == 0x9cc) ? 1 : -1];
typedef char CPvsFxBox_geometry_size[(sizeof(CPvsFxBox::GeneratedGeometry) == 0xc) ? 1 : -1];
typedef char CPvsFxBox_buffers_size[(sizeof(CPvsFxBox::SkyBuffers) == 0x20) ? 1 : -1];

#endif // CLASSES_CPVSFXBOX_H

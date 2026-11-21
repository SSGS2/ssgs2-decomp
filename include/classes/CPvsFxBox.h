// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CPVSFXBOX_H
#define CLASSES_CPVSFXBOX_H

#include "game/types.h"

class CPvsFxBox {
public:
    void* vtbl;                         // 0x0: original vtable pointer
    int unk_4;                          // 0x4: loaded flag in sky view; vtable in CFx base view
    int farSkySourceVertices;                          // 0x8: 2 uses in CExhibition_Ctor, CRoundDol3D_Ctor
    int farSkyResourceFlags;                          // 0xc: 3 uses in ScreenBallPreview_Ctor, CExhibition_Ctor, CRoundDol3D_Ctor
    void* unk_10;                       // 0x10: 3 uses in ScreenBallPreview_Ctor, CExhibition_Ctor, CRoundDol3D_Ctor (CONFLICT sizes [1, 4])
    float unk_14;                       // 0x14: 10 uses in CExhibition_Ctor, CPvsFxBox_ctor, ScreenBallPreview_Ctor...
    float unk_18;                       // 0x18: 9 uses in CExhibition_Ctor, ScreenBallPreview_Ctor, CRoundDol3D_Ctor
    float unk_1C;                       // 0x1c: 7 uses in CExhibition_Ctor, CPvsFxBox_ctor, ScreenBallPreview_Ctor...
    float unk_20;                       // 0x20: 3 uses in ScreenBallPreview_Ctor, CExhibition_Ctor, CRoundDol3D_Ctor
    float unk_24;                       // 0x24: 2 uses in CExhibition_Ctor, CRoundDol3D_Ctor
    float unk_28;                       // 0x28: 2 uses in CExhibition_Ctor, CRoundDol3D_Ctor
    float unk_2C;                       // 0x2c: 5 uses in CExhibition_Ctor, CRoundDol3D_Ctor
    float unk_30;                       // 0x30: 3 uses in CExhibition_Ctor, CRoundDol3D_Ctor
    float unk_34;                       // 0x34: 2 uses in CExhibition_Ctor
    char pad_038[0x2c];                 // 0x38
    float unk_64;                       // 0x64: 2 uses in ScreenBallPreview_Ctor, CExhibition_Ctor
    float nearSkySourceVertices;                       // 0x68: 2 uses in ScreenBallPreview_Ctor, CExhibition_Ctor
    float nearSkyResourceFlags;                       // 0x6c: 2 uses in ScreenBallPreview_Ctor, CExhibition_Ctor
    float unk_70;                       // 0x70: 2 uses in ScreenBallPreview_Ctor, CExhibition_Ctor
    int unk_74;                         // 0x74: 1 use in CExhibition_Ctor
    char pad_078[0x10];                 // 0x78
    float unk_88;                       // 0x88: 1 use in ScreenBallPreview_Ctor
    float unk_8C;                       // 0x8c: 1 use in ScreenBallPreview_Ctor
    float unk_90;                       // 0x90: 1 use in ScreenBallPreview_Ctor
    char pad_094[0x10];                 // 0x94
    float unk_A4;                       // 0xa4: 1 use in ScreenBallPreview_Ctor
    float unk_A8;                       // 0xa8: 3 uses in ScreenBallPreview_Ctor, CExhibition_Ctor, CRoundDol3D_Ctor
    float unk_AC;                       // 0xac: 3 uses in ScreenBallPreview_Ctor, CExhibition_Ctor, CRoundDol3D_Ctor
    float unk_B0;                       // 0xb0: 1 use in CRoundDol3D_Ctor
    float unk_B4;                       // 0xb4: 2 uses in CExhibition_Ctor, CRoundDol3D_Ctor
    char pad_0B8[0x14];                 // 0xb8
    int upperSkyResourceFlags;                         // 0xcc: 1 use in CRoundDol3D_Ctor
    int unk_D0;                         // 0xd0: 1 use in CRoundDol3D_Ctor
    char pad_0D4[0x24];                 // 0xd4
    float unk_F8;                       // 0xf8: 1 use in ScreenBallPreview_Ctor
    int unk_FC;                         // 0xfc: 1 use in ScreenBallPreview_Ctor
    int unk_100;                        // 0x100: 1 use in ScreenBallPreview_Ctor
    int unk_104;                        // 0x104: 1 use in ScreenBallPreview_Ctor
    int unk_108;                        // 0x108: 1 use in ScreenBallPreview_Ctor
    float unk_10C;                      // 0x10c: 1 use in ScreenBallPreview_Ctor
    float unk_110;                      // 0x110: 1 use in ScreenBallPreview_Ctor
    char pad_114[0x2c];                 // 0x114
    float unk_140;                      // 0x140: 1 use in ScreenBallPreview_Ctor
    int unk_144;                        // 0x144: 1 use in ScreenBallPreview_Ctor
    int unk_148;                        // 0x148: 1 use in ScreenBallPreview_Ctor
    int unk_14C;                        // 0x14c: 1 use in ScreenBallPreview_Ctor
    int unk_150;                        // 0x150: 1 use in ScreenBallPreview_Ctor
    float unk_154;                      // 0x154: 1 use in ScreenBallPreview_Ctor
    float unk_158;                      // 0x158: 1 use in ScreenBallPreview_Ctor
    char pad_15C[0x24];                 // 0x15c
    float unk_180;                      // 0x180: 1 use in ScreenBallPreview_Ctor
    char pad_184[0x4];                  // 0x184
    float particleSpawnRadius;                      // 0x188: 1 use in ScreenBallPreview_Ctor
    int particleCameraFadeDistance;                        // 0x18c: 1 use in ScreenBallPreview_Ctor
    int particleSpawnHeight;                        // 0x190: 1 use in ScreenBallPreview_Ctor
    int particleBitmap;                        // 0x194: 1 use in ScreenBallPreview_Ctor
    int particleCapacity;                        // 0x198: 1 use in ScreenBallPreview_Ctor
    float unk_19C;                      // 0x19c: 1 use in ScreenBallPreview_Ctor
    float unk_1A0;                      // 0x1a0: 1 use in ScreenBallPreview_Ctor
    float unk_1A4;                      // 0x1a4: 1 use in ScreenBallPreview_Ctor
    void* unk_1A8;                      // 0x1a8: 3 uses in ScreenBallPreview_Ctor
    void* unk_1AC;                      // 0x1ac: 3 uses in ScreenBallPreview_Ctor
    char pad_1B0[0x50];                 // 0x1b0
    float unk_200;                      // 0x200: 1 use in ScreenBallPreview_Ctor
    float unk_204;                      // 0x204: 1 use in ScreenBallPreview_Ctor
    char pad_208[0x4];                  // 0x208
    float unk_20C;                      // 0x20c: 1 use in ScreenBallPreview_Ctor
    char pad_210[0x44];                 // 0x210
    int unk_254;                        // 0x254: 1 use in CRoundDol3D_Ctor
    char pad_258[0xc];                  // 0x258
    int unk_264;                        // 0x264: 1 use in CRoundDol3D_Ctor
    float unk_268;                      // 0x268: 1 use in CRoundDol3D_Ctor
    float unk_26C;                      // 0x26c: 1 use in CRoundDol3D_Ctor
    int unk_270;                        // 0x270: 1 use in CRoundDol3D_Ctor
    char pad_274[0x4];                  // 0x274
    int unk_278;                        // 0x278: 1 use in CRoundDol3D_Ctor
    char pad_27C[0x4];                  // 0x27c
    float unk_280;                      // 0x280: 1 use in CRoundDol3D_Ctor
    char pad_284[0xfc];                 // 0x284
    void* sprayAttachmentTreeRoot;                      // 0x380: 1 use in CPvsFxBox_dtor
    char pad_384[0x8];                  // 0x384
    void* sequenceAttachmentTreeRoot;                      // 0x38c: 1 use in CPvsFxBox_dtor
    char pad_390[0x1c];                 // 0x390
    unsigned int auxiliaryAttachmentTreeCount;               // 0x3ac: 1 use in CPvsFxBox_ctor
    unsigned int auxiliaryAttachmentTreeRoot;               // 0x3b0: 1 use in CPvsFxBox_ctor
    void* auxiliaryAttachmentTreeRightmost;                      // 0x3b4: 1 use in CPvsFxBox_ctor
    char unk_3B8;                       // 0x3b8: 1 use in CPvsFxBox_ctor
    char unk_3B9;                       // 0x3b9: 1 use in CPvsFxBox_ctor
    char unk_3BA;                       // 0x3ba: 1 use in CPvsFxBox_ctor
    char pad_3BB[0x89];                 // 0x3bb
    int unk_444;                        // 0x444: 1 use in CExhibition_Ctor
    int unk_448;                        // 0x448: 1 use in CExhibition_Ctor
    int unk_44C;                        // 0x44c: 1 use in CExhibition_Ctor
    int unk_450;                        // 0x450: 1 use in CExhibition_Ctor
    int unk_454;                        // 0x454: 1 use in CExhibition_Ctor
    int unk_458;                        // 0x458: 1 use in CExhibition_Ctor
    int unk_45C;                        // 0x45c: 1 use in CExhibition_Ctor
    void* unk_460;                      // 0x460: 1 use in CExhibition_Ctor
    void* unk_464;                      // 0x464: 1 use in CExhibition_Ctor
    int unk_468;                        // 0x468: 4 uses in CExhibition_Ctor
    int unk_46C;                        // 0x46c: 1 use in CExhibition_Ctor
    int unk_470;                        // 0x470: 1 use in CExhibition_Ctor
    char pad_474[0x68];                 // 0x474
    unsigned char unk_4DC;              // 0x4dc: 1 use in CExhibition_Ctor
    char pad_4DD[0x3];                  // 0x4dd
    int unk_4E0;                        // 0x4e0: 2 uses in CExhibition_Ctor
    int unk_4E4;                        // 0x4e4: 2 uses in CExhibition_Ctor
    int unk_4E8;                        // 0x4e8: 2 uses in CExhibition_Ctor
    int unk_4EC;                        // 0x4ec: 4 uses in CExhibition_Ctor
    float unk_4F0;                      // 0x4f0: 4 uses in CExhibition_Ctor
    float unk_4F4;                      // 0x4f4: 6 uses in CExhibition_Ctor
    int unk_4F8;                        // 0x4f8: 1 use in CExhibition_Ctor
    int unk_4FC;                        // 0x4fc: 1 use in CExhibition_Ctor
    int unk_500;                        // 0x500: 1 use in CExhibition_Ctor
    int unk_504;                        // 0x504: 1 use in CExhibition_Ctor
    float unk_508;                      // 0x508: 1 use in CExhibition_Ctor
    int unk_50C;                        // 0x50c: 1 use in CExhibition_Ctor
    int unk_510;                        // 0x510: 1 use in CExhibition_Ctor
    unsigned char unk_514;              // 0x514: 1 use in CExhibition_Ctor
    char pad_515[0x3];                  // 0x515
    int unk_518;                        // 0x518: 1 use in CExhibition_Ctor
    int unk_51C;                        // 0x51c: 1 use in CExhibition_Ctor
    int unk_520;                        // 0x520: 1 use in CExhibition_Ctor
    int unk_524;                        // 0x524: 1 use in CExhibition_Ctor
    int unk_528;                        // 0x528: 1 use in CExhibition_Ctor
    int unk_52C;                        // 0x52c: 1 use in CExhibition_Ctor
    int unk_530;                        // 0x530: 1 use in CExhibition_Ctor
    int unk_534;                        // 0x534: 1 use in CExhibition_Ctor
    int unk_538;                        // 0x538: 1 use in CExhibition_Ctor
    unsigned char unk_53C;              // 0x53c: 1 use in CExhibition_Ctor
    char pad_53D[0xf];                  // 0x53d
    int unk_54C;                        // 0x54c: 1 use in CExhibition_Ctor
    char pad_550[0x4c];                 // 0x550
    void* unk_59C;                      // 0x59c: 1 use in ScreenBallPreview_Ctor
    void* unk_5A0;                      // 0x5a0: 1 use in ScreenBallPreview_Ctor
    void* unk_5A4;                      // 0x5a4: 1 use in ScreenBallPreview_Ctor
    void* unk_5A8;                      // 0x5a8: 1 use in ScreenBallPreview_Ctor
    int unk_5AC;                        // 0x5ac: 1 use in ScreenBallPreview_Ctor
    char pad_5B0[0x60];                 // 0x5b0
    int unk_610;                        // 0x610: 1 use in CRoundDol3D_Ctor
    char pad_614[0x4];                  // 0x614
    float unk_618;                      // 0x618: 1 use in CExhibition_Ctor
    float unk_61C;                      // 0x61c: 1 use in CExhibition_Ctor
    float unk_620;                      // 0x620: 1 use in CExhibition_Ctor
    float unk_624;                      // 0x624: 1 use in CExhibition_Ctor
    float unk_628;                      // 0x628: 2 uses in CExhibition_Ctor
    float unk_62C;                      // 0x62c: 2 uses in CExhibition_Ctor
    int unk_630;                        // 0x630: 1 use in CExhibition_Ctor
    int unk_634;                        // 0x634: 1 use in CExhibition_Ctor
    int unk_638;                        // 0x638: 1 use in CExhibition_Ctor
    int unk_63C;                        // 0x63c: 1 use in CExhibition_Ctor
    char pad_640[0x264];                // 0x640
    int activeParticleCount;            // 0x8a4: CPvsFxBox_UpdateWindParticles increments on spawn, decrements on expiry
    float spawnInterval;               // 0x8a8: CPvsFxBox_UpdateWindParticles compares and subtracts from spawnTimer
    float spawnTimer;                  // 0x8ac: CPvsFxBox_UpdateWindParticles accumulates its frame delta
    float unk_8B0;                     // 0x8b0: CPvsFxBox_Init initializes to 65.0f
    float skyVerticalOffset;                     // 0x8b4: CPvsFxBox_Init initializes to zero
    unsigned int particleColor;        // 0x8b8: CPvsFxBox_DrawParticles stores color read by the particle draw path
    float skyAngularPhaseScale;                     // 0x8bc: CPvsFxBox_Init initializes to zero
    int skyTextureRepeatCount;                       // 0x8c0: CPvsFxBox_Init initializes to two
    int nearSkyTextureRepeatCount;                       // 0x8c4: CPvsFxBox_Init initializes to two
    char pad_8C8[0xCC];                // 0x8c8
    void* auxiliaryEffect;             // 0x994: CPvsFxBox_RebuildAuxiliaryEffect creates, CPvsFxBox_ReleaseResources destroys via PvsFxAuxiliaryResources_Dtor
    void* courseEffects;                      // 0x998: 13 uses in CPvsFxBox_ReloadCourseEffects
    void* sceneEffects;                      // 0x99c: 14 uses in CPvsFxBox_RebuildSceneEffects

    // offsets whose address is passed on (sub-objects or arrays, not placed): 0x4, 0x18, 0x28, 0x34, 0x38, 0x58, 0x64, 0x78, 0xa4, 0xb0, 0xcc, 0x114, 0x15c, 0x1b0, 0x1b8, 0x1f8, 0x208, 0x258, 0x27c, 0x37c, 0x388, 0x3b0, 0x460, 0x474
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define CPVSFXBOX_OFF(f, o) typedef char CPvsFxBox_off_##f[((int)&((CPvsFxBox*)0)->f == (o)) ? 1 : -1]
CPVSFXBOX_OFF(unk_4, 0x4);
CPVSFXBOX_OFF(farSkySourceVertices, 0x8);
CPVSFXBOX_OFF(farSkyResourceFlags, 0xc);
CPVSFXBOX_OFF(unk_10, 0x10);
CPVSFXBOX_OFF(unk_14, 0x14);
CPVSFXBOX_OFF(unk_18, 0x18);
CPVSFXBOX_OFF(unk_1C, 0x1c);
CPVSFXBOX_OFF(unk_20, 0x20);
CPVSFXBOX_OFF(unk_24, 0x24);
CPVSFXBOX_OFF(unk_28, 0x28);
CPVSFXBOX_OFF(unk_2C, 0x2c);
CPVSFXBOX_OFF(unk_30, 0x30);
CPVSFXBOX_OFF(unk_34, 0x34);
CPVSFXBOX_OFF(unk_64, 0x64);
CPVSFXBOX_OFF(nearSkySourceVertices, 0x68);
CPVSFXBOX_OFF(nearSkyResourceFlags, 0x6c);
CPVSFXBOX_OFF(unk_70, 0x70);
CPVSFXBOX_OFF(unk_74, 0x74);
CPVSFXBOX_OFF(unk_88, 0x88);
CPVSFXBOX_OFF(unk_8C, 0x8c);
CPVSFXBOX_OFF(unk_90, 0x90);
CPVSFXBOX_OFF(unk_A4, 0xa4);
CPVSFXBOX_OFF(unk_A8, 0xa8);
CPVSFXBOX_OFF(unk_AC, 0xac);
CPVSFXBOX_OFF(unk_B0, 0xb0);
CPVSFXBOX_OFF(unk_B4, 0xb4);
CPVSFXBOX_OFF(upperSkyResourceFlags, 0xcc);
CPVSFXBOX_OFF(unk_D0, 0xd0);
CPVSFXBOX_OFF(unk_F8, 0xf8);
CPVSFXBOX_OFF(unk_FC, 0xfc);
CPVSFXBOX_OFF(unk_100, 0x100);
CPVSFXBOX_OFF(unk_104, 0x104);
CPVSFXBOX_OFF(unk_108, 0x108);
CPVSFXBOX_OFF(unk_10C, 0x10c);
CPVSFXBOX_OFF(unk_110, 0x110);
CPVSFXBOX_OFF(unk_140, 0x140);
CPVSFXBOX_OFF(unk_144, 0x144);
CPVSFXBOX_OFF(unk_148, 0x148);
CPVSFXBOX_OFF(unk_14C, 0x14c);
CPVSFXBOX_OFF(unk_150, 0x150);
CPVSFXBOX_OFF(unk_154, 0x154);
CPVSFXBOX_OFF(unk_158, 0x158);
CPVSFXBOX_OFF(unk_180, 0x180);
CPVSFXBOX_OFF(particleSpawnRadius, 0x188);
CPVSFXBOX_OFF(particleCameraFadeDistance, 0x18c);
CPVSFXBOX_OFF(particleSpawnHeight, 0x190);
CPVSFXBOX_OFF(particleBitmap, 0x194);
CPVSFXBOX_OFF(particleCapacity, 0x198);
CPVSFXBOX_OFF(unk_19C, 0x19c);
CPVSFXBOX_OFF(unk_1A0, 0x1a0);
CPVSFXBOX_OFF(unk_1A4, 0x1a4);
CPVSFXBOX_OFF(unk_1A8, 0x1a8);
CPVSFXBOX_OFF(unk_1AC, 0x1ac);
CPVSFXBOX_OFF(unk_200, 0x200);
CPVSFXBOX_OFF(unk_204, 0x204);
CPVSFXBOX_OFF(unk_20C, 0x20c);
CPVSFXBOX_OFF(unk_254, 0x254);
CPVSFXBOX_OFF(unk_264, 0x264);
CPVSFXBOX_OFF(unk_268, 0x268);
CPVSFXBOX_OFF(unk_26C, 0x26c);
CPVSFXBOX_OFF(unk_270, 0x270);
CPVSFXBOX_OFF(unk_278, 0x278);
CPVSFXBOX_OFF(unk_280, 0x280);
CPVSFXBOX_OFF(sprayAttachmentTreeRoot, 0x380);
CPVSFXBOX_OFF(sequenceAttachmentTreeRoot, 0x38c);
CPVSFXBOX_OFF(auxiliaryAttachmentTreeCount, 0x3ac);
CPVSFXBOX_OFF(auxiliaryAttachmentTreeRoot, 0x3b0);
CPVSFXBOX_OFF(auxiliaryAttachmentTreeRightmost, 0x3b4);
CPVSFXBOX_OFF(unk_3B8, 0x3b8);
CPVSFXBOX_OFF(unk_3B9, 0x3b9);
CPVSFXBOX_OFF(unk_3BA, 0x3ba);
CPVSFXBOX_OFF(unk_444, 0x444);
CPVSFXBOX_OFF(unk_448, 0x448);
CPVSFXBOX_OFF(unk_44C, 0x44c);
CPVSFXBOX_OFF(unk_450, 0x450);
CPVSFXBOX_OFF(unk_454, 0x454);
CPVSFXBOX_OFF(unk_458, 0x458);
CPVSFXBOX_OFF(unk_45C, 0x45c);
CPVSFXBOX_OFF(unk_460, 0x460);
CPVSFXBOX_OFF(unk_464, 0x464);
CPVSFXBOX_OFF(unk_468, 0x468);
CPVSFXBOX_OFF(unk_46C, 0x46c);
CPVSFXBOX_OFF(unk_470, 0x470);
CPVSFXBOX_OFF(unk_4DC, 0x4dc);
CPVSFXBOX_OFF(unk_4E0, 0x4e0);
CPVSFXBOX_OFF(unk_4E4, 0x4e4);
CPVSFXBOX_OFF(unk_4E8, 0x4e8);
CPVSFXBOX_OFF(unk_4EC, 0x4ec);
CPVSFXBOX_OFF(unk_4F0, 0x4f0);
CPVSFXBOX_OFF(unk_4F4, 0x4f4);
CPVSFXBOX_OFF(unk_4F8, 0x4f8);
CPVSFXBOX_OFF(unk_4FC, 0x4fc);
CPVSFXBOX_OFF(unk_500, 0x500);
CPVSFXBOX_OFF(unk_504, 0x504);
CPVSFXBOX_OFF(unk_508, 0x508);
CPVSFXBOX_OFF(unk_50C, 0x50c);
CPVSFXBOX_OFF(unk_510, 0x510);
CPVSFXBOX_OFF(unk_514, 0x514);
CPVSFXBOX_OFF(unk_518, 0x518);
CPVSFXBOX_OFF(unk_51C, 0x51c);
CPVSFXBOX_OFF(unk_520, 0x520);
CPVSFXBOX_OFF(unk_524, 0x524);
CPVSFXBOX_OFF(unk_528, 0x528);
CPVSFXBOX_OFF(unk_52C, 0x52c);
CPVSFXBOX_OFF(unk_530, 0x530);
CPVSFXBOX_OFF(unk_534, 0x534);
CPVSFXBOX_OFF(unk_538, 0x538);
CPVSFXBOX_OFF(unk_53C, 0x53c);
CPVSFXBOX_OFF(unk_54C, 0x54c);
CPVSFXBOX_OFF(unk_59C, 0x59c);
CPVSFXBOX_OFF(unk_5A0, 0x5a0);
CPVSFXBOX_OFF(unk_5A4, 0x5a4);
CPVSFXBOX_OFF(unk_5A8, 0x5a8);
CPVSFXBOX_OFF(unk_5AC, 0x5ac);
CPVSFXBOX_OFF(unk_610, 0x610);
CPVSFXBOX_OFF(unk_618, 0x618);
CPVSFXBOX_OFF(unk_61C, 0x61c);
CPVSFXBOX_OFF(unk_620, 0x620);
CPVSFXBOX_OFF(unk_624, 0x624);
CPVSFXBOX_OFF(unk_628, 0x628);
CPVSFXBOX_OFF(unk_62C, 0x62c);
CPVSFXBOX_OFF(unk_630, 0x630);
CPVSFXBOX_OFF(unk_634, 0x634);
CPVSFXBOX_OFF(unk_638, 0x638);
CPVSFXBOX_OFF(unk_63C, 0x63c);
CPVSFXBOX_OFF(courseEffects, 0x998);
CPVSFXBOX_OFF(sceneEffects, 0x99c);
CPVSFXBOX_OFF(activeParticleCount, 0x8A4);
CPVSFXBOX_OFF(spawnInterval, 0x8A8);
CPVSFXBOX_OFF(spawnTimer, 0x8AC);
CPVSFXBOX_OFF(unk_8B0, 0x8B0);
CPVSFXBOX_OFF(skyVerticalOffset, 0x8B4);
CPVSFXBOX_OFF(particleColor, 0x8B8);
CPVSFXBOX_OFF(skyAngularPhaseScale, 0x8BC);
CPVSFXBOX_OFF(skyTextureRepeatCount, 0x8C0);
CPVSFXBOX_OFF(nearSkyTextureRepeatCount, 0x8C4);
CPVSFXBOX_OFF(auxiliaryEffect, 0x994);
#undef CPVSFXBOX_OFF

#endif  // CLASSES_CPVSFXBOX_H

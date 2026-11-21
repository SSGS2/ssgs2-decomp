// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CGOLFDOC_H
#define CLASSES_CGOLFDOC_H

#include "game/types.h"
#include "game/golf_shot_types.h"

class CGolfDoc {
public:
    void* vtbl;                         // 0x0: original vtable pointer
    char pad_004[0x4];                  // 0x4
    void* courseData;                        // 0x8: 4 uses in CGolfDoc_Ctor, CGolfDoc_dtor
    void* courseEffectBox;                        // 0xc: 3 uses in CGolfDoc_dtor, CGolfDoc_Ctor
    void* aimPath;                       // 0x10: 3 uses in CGolfDoc_dtor, CGolfDoc_Ctor
    int terrainDeclarations;                         // 0x14: 1 use in CGolfDoc_Ctor
    int terrainDeclarationCount;                         // 0x18: 1 use in CGolfDoc_Ctor
    int terrainDeclarationCapacity;                         // 0x1c: 1 use in CGolfDoc_Ctor
    int terrainAliasCount;                         // 0x20: 1 use in CGolfDoc_Ctor
    int terrainAliasTreeRoot;                         // 0x24: 1 use in CGolfDoc_Ctor
    void* terrainAliasTreeSentinel;                       // 0x28: 1 use in CGolfDoc_Ctor
    int defaultTerrainDeclarationIndex;                         // 0x2c: 1 use in CGolfDoc_Ctor
    int equipmentStringMapCount;                         // 0x30: 1 use in CGolfDoc_Ctor
    int equipmentStringMapRoot;                         // 0x34: 1 use in CGolfDoc_Ctor
    char pad_038[0x4];                  // 0x38
    void* equipmentStringMapSentinel;                       // 0x3c: 1 use in CGolfDoc_Ctor
    int beachParameterTreeCount;                         // 0x40: 1 use in CGolfDoc_Ctor
    int beachParameterTreeRoot;                         // 0x44: 1 use in CGolfDoc_Ctor
    char pad_048[0x4];                  // 0x48
    void* beachParameterTreeSentinel;                       // 0x4c: 1 use in CGolfDoc_Ctor
    int unk_50;                         // 0x50: 1 use in CGolfDoc_Ctor
    int unk_54;                         // 0x54: 1 use in CGolfDoc_Ctor
    char pad_058[0x4];                  // 0x58
    void* unk_5C;                       // 0x5c: 1 use in CGolfDoc_Ctor
    int instanceListHead;                         // 0x60: 1 use in CGolfDoc_Ctor
    int instanceIterator;                         // 0x64: 1 use in CGolfDoc_Ctor
    int unk_68;                         // 0x68: 1 use in CGolfDoc_Ctor
    int recycledInstanceListHead;                         // 0x6c: 1 use in CGolfDoc_Ctor
    int instanceInitialBucketCount;                         // 0x70: 2 uses in CGolfDoc_Ctor
    int instanceBucketMask;                         // 0x74: 1 use in CGolfDoc_Ctor
    int instanceSlotCount;                         // 0x78: 3 uses in CGolfDoc_Ctor
    void* instanceSlots;                       // 0x7c: 4 uses in CGolfDoc_Ctor
    char pad_080[0x18];                 // 0x80
    int tutorialStage;                         // 0x98: 1 use in CGolfDoc_Ctor
    int shotCommandCount;                         // 0x9c: 1 use in CGolfDoc_Ctor
    void* shotCommandHead;                       // 0xa0: 1 use in CGolfDoc_Ctor
    void* shotCommandTail;                       // 0xa4: 1 use in CGolfDoc_Ctor
    int tutorialMarkerModel;                         // 0xa8: 1 use in CGolfDoc_Ctor
    char pad_0AC[0xc];                  // 0xac
    unsigned char tutorialMarkerVisible;               // 0xb8: 1 use in CGolfDoc_Ctor
    char pad_0B9[0x3];                  // 0xb9
    int parameterXmlPathFlags;                         // 0xbc: 1 use in CGolfDoc_Ctor
    int parameterXmlPathLength;                         // 0xc0: 1 use in CGolfDoc_Ctor
    int parameterXmlPathData;                         // 0xc4: 1 use in CGolfDoc_Ctor
    void* playerSnapshots;                       // 0xc8: 3 uses in CGolfDoc_dtor, CGolfDoc_Ctor
    char pad_0CC[0xb8];                 // 0xcc
    int unk_184;                        // 0x184: 1 use in CGolfDoc_Ctor
    char pad_188[0x130];                // 0x188
    unsigned char activePlayerCount;              // 0x2b8: 1 use in CGolfDoc_Ctor
    unsigned char playerCount;          // 0x2b9: 1 use in CGolfDoc_Ctor
    char pad_2BA[0x2];                  // 0x2ba
    union {
        void* holeRecords; // +2BC: legacy allocation/access spelling
        ShotHoleRecord* shotHoleRecords; // tee/pin/par records, stride 0x20
    };
    unsigned char unk_2C0;              // 0x2c0: 1 use in CGolfDoc_Ctor
    char pad_2C1[0x3]; // 0x2c1
    float shotPowerGaugeX; // 0x2c4: CGolfRule_CommitShotGauge, CGolfRule_ResetShotGauge, CGolfRule_RecordDirectionalShotInput, CGolfRule_ProcessButtonSwingInput
    float shotImpactGaugeX; // 0x2c8: CGolfRule_CommitShotGauge, CGolfRule_ResetShotGauge, CGolfRule_ApplyImpactAssistance
    float shotGaugeCursorX; // 0x2cc: CGolfRule_ResetShotGauge, CGolfRule_RecordDirectionalShotInput, CGolfRule_ProcessButtonSwingInput, CClub_Process
    float roundedShotGaugeCursorX; // 0x2d0: CGolfRule_ResetShotGauge, CGolfRule_ProcessButtonSwingInput, CClub_Process
    int impactGrade; // 0x2d4: CGolfRule_CommitShotGauge, CGolfRule_ClassifyShotPresentation, CGolfRule_ProcessButtonShotAnimation, CGolfRule_ProcessMotionShotAnimation, CGolfRule_ProcessBallFlight
    char pad_2D8[0x4]; // 0x2d8
    int shotGaugeDirection; // 0x2dc: CGolfRule_ResetShotGauge, CGolfRule_ProcessButtonSwingInput
    float pangyaTargetX; // 0x2e0: CGolfRule_CommitShotGauge, CGolfRule_ResetShotGauge, CGolfRule_ApplyImpactAssistance, CGolfRule_ProcessButtonSwingInput
    float calipersGaugeX; // 0x2e4: CGolfRule_ResetShotGauge
    float pangyaZoneScale; // 0x2e8: CGolfRule_ResetShotGauge, fn_8016E124_step, fn_8016E124_step2
    int impactTimingSpriteIndex; // 0x2ec: CGolfRule_ResetShotGauge
    float impactFeedbackGaugeX; // 0x2f0: CGolfRule_ResetShotGauge
    int powerShotBonusPang; // 0x2f4: CGolfRule_ResetShotGauge
    int pangyaAssistState; // 0x2f8: CGolfRule_ResetShotGauge, CGolfRule_ApplyImpactAssistance
    float unassistedImpactGaugeX; // 0x2fc: CGolfRule_ResetShotGauge, CGolfRule_ApplyImpactAssistance
    int unassistedImpactGrade; // 0x300: CGolfRule_ResetShotGauge, CGolfRule_ApplyImpactAssistance
    char pad_304[0x8]; // 0x304
    float cameraGroundClearance;                      // 0x30c: 1 use in CGolfDoc_Ctor
    char pad_310[0xc];                  // 0x310
    float headingRadians;               // 0x31c: 1 use in CGolfDoc_Ctor
    float unk_320;                      // 0x320: 1 use in CGolfDoc_Ctor
    char pad_324[0x30];                 // 0x324: camera matrix storage
    float cameraPitchRadians; // 0x354: CGolfDoc_BuildCameraMatrix+0x44 / CGolfDoc_UpdateCamera+0x19C load this for tan()
    int gameStateFlags;                        // 0x358: 1 use in CGolfDoc_Ctor
    unsigned char unk_35C;              // 0x35c: 1 use in CGolfDoc_Ctor
    unsigned char unk_35D;              // 0x35d: 1 use in CGolfDoc_Ctor
    unsigned char unk_35E;              // 0x35e: 1 use in CGolfDoc_Ctor
    unsigned char unk_35F;              // 0x35f: 1 use in CGolfDoc_Ctor
    unsigned char isDaytime;              // 0x360: 1 use in CGolfDoc_Ctor
    unsigned char unk_361;              // 0x361: 1 use in CGolfDoc_Ctor
    char pad_362[0x2];                  // 0x362
    int unk_364;                        // 0x364: 1 use in CGolfDoc_Ctor
    int cameraMode;                        // 0x368: 1 use in CGolfDoc_Ctor
    char pad_36C[0x48];                 // 0x36c
    unsigned char shotMeterFadeResetRequested;              // 0x3b4: 1 use in CGolfDoc_Ctor
    unsigned char powerShotMeterOverride;              // 0x3b5: 1 use in CGolfDoc_Ctor
    unsigned char currentHoleIndex; // 0x3b6: CGolfRule_ResetHoleBallAndWind, CGolfRule_ProcessHoleResults
    char pad_3B7[0x1]; // 0x3b7
    unsigned int remainingTimeMs; // 0x3b8: CClock_OnProcess, CClock_HandleMsg
    unsigned int elapsedTimeMs; // 0x3bc: CGolfRule_CommitShotGauge, CClock_OnProcess, CClock_HandleMsg
    unsigned char shotPhase;              // 0x3c0: 1 use in CGolfDoc_Ctor
    unsigned char currentPlayer;              // 0x3c1: 1 use in CGolfDoc_Ctor
    char pad_3C2[0x2];                  // 0x3c2
    float bright;                      // 0x3c4: 1 use in CGolfDoc_Ctor
    unsigned char dropRequest;              // 0x3c8: 1 use in CGolfDoc_Ctor
    unsigned char unk_3C9;              // 0x3c9: 1 use in CGolfDoc_Ctor
    unsigned char maxGrade;              // 0x3ca: 1 use in CGolfDoc_Ctor
    unsigned char unk_3CB;              // 0x3cb: 1 use in CGolfDoc_Ctor
    unsigned char unk_3CC;              // 0x3cc: 1 use in CGolfDoc_Ctor
    char pad_3CD[0x3];                  // 0x3cd
    float unk_3D0;                      // 0x3d0: 1 use in CGolfDoc_Ctor
    float unk_3D4;                      // 0x3d4: 1 use in CGolfDoc_Ctor
    int unk_3D8;                        // 0x3d8: 1 use in CGolfDoc_Ctor
    int unk_3DC;                        // 0x3dc: 1 use in CGolfDoc_Ctor
    short unk_3E0;                      // 0x3e0: 1 use in CGolfDoc_Ctor
    char pad_3E2[0x2];                  // 0x3e2
    int unk_3E4;                        // 0x3e4: 1 use in CGolfDoc_Ctor
    int unk_3E8;                        // 0x3e8: 1 use in CGolfDoc_Ctor
    int unk_3EC;                        // 0x3ec: 1 use in CGolfDoc_Ctor
    unsigned char unk_3F0;              // 0x3f0: 1 use in CGolfDoc_Ctor
    char pad_3F1[0x3];                  // 0x3f1
    int unk_3F4;                        // 0x3f4: 1 use in CGolfDoc_Ctor
    int unk_3F8;                        // 0x3f8: 1 use in CGolfDoc_Ctor
    int alternateMotionPowerDecided;                        // 0x3fc: 1 use in CGolfDoc_Ctor
    int shotReactionActive; // 0x400: CGolfRule_ProcessAimInput, CGolfRule_StartBallFlightFromSample
    int puttingReactionPending; // 0x404: CGolfRule_ProcessAimInput
    int afterPuttPoseShown; // 0x408: CGolfRule_ProcessAimInput
    int motionSwingState;                        // 0x40c: 1 use in CGolfDoc_Ctor

    // offsets whose address is passed on (sub-objects or arrays, not placed): 0x14, 0x24, 0x30, 0x34, 0x40, 0x44, 0x50, 0x54, 0x60, 0x80, 0x9c, 0xa0, 0xbc, 0xcc, 0x294, 0x36c, 0x410, 0x470, 0x500, 0x534, 0x53c, 0x6dc
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define CGOLFDOC_OFF(f, o) typedef char CGolfDoc_off_##f[((int)&((CGolfDoc*)0)->f == (o)) ? 1 : -1]
CGOLFDOC_OFF(cameraPitchRadians, 0x354);
CGOLFDOC_OFF(courseData, 0x8);
CGOLFDOC_OFF(courseEffectBox, 0xc);
CGOLFDOC_OFF(aimPath, 0x10);
CGOLFDOC_OFF(terrainDeclarations, 0x14);
CGOLFDOC_OFF(terrainDeclarationCount, 0x18);
CGOLFDOC_OFF(terrainDeclarationCapacity, 0x1c);
CGOLFDOC_OFF(terrainAliasCount, 0x20);
CGOLFDOC_OFF(terrainAliasTreeRoot, 0x24);
CGOLFDOC_OFF(terrainAliasTreeSentinel, 0x28);
CGOLFDOC_OFF(defaultTerrainDeclarationIndex, 0x2c);
CGOLFDOC_OFF(equipmentStringMapCount, 0x30);
CGOLFDOC_OFF(equipmentStringMapRoot, 0x34);
CGOLFDOC_OFF(equipmentStringMapSentinel, 0x3c);
CGOLFDOC_OFF(beachParameterTreeCount, 0x40);
CGOLFDOC_OFF(beachParameterTreeRoot, 0x44);
CGOLFDOC_OFF(beachParameterTreeSentinel, 0x4c);
CGOLFDOC_OFF(unk_50, 0x50);
CGOLFDOC_OFF(unk_54, 0x54);
CGOLFDOC_OFF(unk_5C, 0x5c);
CGOLFDOC_OFF(instanceListHead, 0x60);
CGOLFDOC_OFF(instanceIterator, 0x64);
CGOLFDOC_OFF(unk_68, 0x68);
CGOLFDOC_OFF(recycledInstanceListHead, 0x6c);
CGOLFDOC_OFF(instanceInitialBucketCount, 0x70);
CGOLFDOC_OFF(instanceBucketMask, 0x74);
CGOLFDOC_OFF(instanceSlotCount, 0x78);
CGOLFDOC_OFF(instanceSlots, 0x7c);
CGOLFDOC_OFF(tutorialStage, 0x98);
CGOLFDOC_OFF(shotCommandCount, 0x9c);
CGOLFDOC_OFF(shotCommandHead, 0xa0);
CGOLFDOC_OFF(shotCommandTail, 0xa4);
CGOLFDOC_OFF(tutorialMarkerModel, 0xa8);
CGOLFDOC_OFF(tutorialMarkerVisible, 0xb8);
CGOLFDOC_OFF(parameterXmlPathFlags, 0xbc);
CGOLFDOC_OFF(parameterXmlPathLength, 0xc0);
CGOLFDOC_OFF(parameterXmlPathData, 0xc4);
CGOLFDOC_OFF(playerSnapshots, 0xc8);
CGOLFDOC_OFF(unk_184, 0x184);
CGOLFDOC_OFF(activePlayerCount, 0x2b8);
CGOLFDOC_OFF(playerCount, 0x2b9);
CGOLFDOC_OFF(holeRecords, 0x2bc);
CGOLFDOC_OFF(unk_2C0, 0x2c0);
CGOLFDOC_OFF(cameraGroundClearance, 0x30c);
CGOLFDOC_OFF(headingRadians, 0x31c);
CGOLFDOC_OFF(unk_320, 0x320);
CGOLFDOC_OFF(gameStateFlags, 0x358);
CGOLFDOC_OFF(unk_35C, 0x35c);
CGOLFDOC_OFF(unk_35D, 0x35d);
CGOLFDOC_OFF(unk_35E, 0x35e);
CGOLFDOC_OFF(unk_35F, 0x35f);
CGOLFDOC_OFF(isDaytime, 0x360);
CGOLFDOC_OFF(unk_361, 0x361);
CGOLFDOC_OFF(unk_364, 0x364);
CGOLFDOC_OFF(cameraMode, 0x368);
CGOLFDOC_OFF(shotMeterFadeResetRequested, 0x3b4);
CGOLFDOC_OFF(powerShotMeterOverride, 0x3b5);
CGOLFDOC_OFF(shotPhase, 0x3c0);
CGOLFDOC_OFF(currentPlayer, 0x3c1);
CGOLFDOC_OFF(bright, 0x3c4);
CGOLFDOC_OFF(dropRequest, 0x3c8);
CGOLFDOC_OFF(unk_3C9, 0x3c9);
CGOLFDOC_OFF(maxGrade, 0x3ca);
CGOLFDOC_OFF(unk_3CB, 0x3cb);
CGOLFDOC_OFF(unk_3CC, 0x3cc);
CGOLFDOC_OFF(unk_3D0, 0x3d0);
CGOLFDOC_OFF(unk_3D4, 0x3d4);
CGOLFDOC_OFF(unk_3D8, 0x3d8);
CGOLFDOC_OFF(unk_3DC, 0x3dc);
CGOLFDOC_OFF(unk_3E0, 0x3e0);
CGOLFDOC_OFF(unk_3E4, 0x3e4);
CGOLFDOC_OFF(unk_3E8, 0x3e8);
CGOLFDOC_OFF(unk_3EC, 0x3ec);
CGOLFDOC_OFF(unk_3F0, 0x3f0);
CGOLFDOC_OFF(unk_3F4, 0x3f4);
CGOLFDOC_OFF(unk_3F8, 0x3f8);
CGOLFDOC_OFF(alternateMotionPowerDecided, 0x3fc);
CGOLFDOC_OFF(motionSwingState, 0x40c);
CGOLFDOC_OFF(shotPowerGaugeX, 0x2c4);
CGOLFDOC_OFF(shotImpactGaugeX, 0x2c8);
CGOLFDOC_OFF(shotGaugeCursorX, 0x2cc);
CGOLFDOC_OFF(roundedShotGaugeCursorX, 0x2d0);
CGOLFDOC_OFF(impactGrade, 0x2d4);
CGOLFDOC_OFF(shotGaugeDirection, 0x2dc);
CGOLFDOC_OFF(pangyaTargetX, 0x2e0);
CGOLFDOC_OFF(calipersGaugeX, 0x2e4);
CGOLFDOC_OFF(pangyaZoneScale, 0x2e8);
CGOLFDOC_OFF(impactTimingSpriteIndex, 0x2ec);
CGOLFDOC_OFF(impactFeedbackGaugeX, 0x2f0);
CGOLFDOC_OFF(powerShotBonusPang, 0x2f4);
CGOLFDOC_OFF(pangyaAssistState, 0x2f8);
CGOLFDOC_OFF(unassistedImpactGaugeX, 0x2fc);
CGOLFDOC_OFF(unassistedImpactGrade, 0x300);
CGOLFDOC_OFF(currentHoleIndex, 0x3b6);
CGOLFDOC_OFF(remainingTimeMs, 0x3b8);
CGOLFDOC_OFF(elapsedTimeMs, 0x3bc);
CGOLFDOC_OFF(shotReactionActive, 0x400);
CGOLFDOC_OFF(puttingReactionPending, 0x404);
CGOLFDOC_OFF(afterPuttPoseShown, 0x408);
#undef CGOLFDOC_OFF

#endif  // CLASSES_CGOLFDOC_H

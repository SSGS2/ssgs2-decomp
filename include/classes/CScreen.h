// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CSCREEN_H
#define CLASSES_CSCREEN_H

#include "game/types.h"

class CScreen {
public:
    void* vtbl;                         // 0x0: original vtable pointer
    char pad_004[0x4];                  // 0x4
    int unk_8;                          // 0x8: 1 use in CScreen_Ctor
    char pad_00C[0xc];                  // 0xc
    void* taskMainVtable;                       // 0x18: 1 use in CScreen_Ctor
    void* holeInfoSet;                       // 0x1c: 7 uses in CScreen_OnDestroy, CScreen_OnLoad, CScreen_Ctor
    void* flyInfo;                       // 0x20: 4 uses in CScreen_OnDestroy, CScreen_Ctor, CScreen_OnLoad
    void* ballPreview;                       // 0x24: 5 uses in CScreen_OnDestroy, CScreen_Ctor, CScreen_OnLoad
    void* windInfo;                       // 0x28: 4 uses in CScreen_OnDestroy, CScreen_Ctor, CScreen_OnLoad
    void* remainingTimeInfo;                       // 0x2c: 4 uses in CScreen_OnDestroy, CScreen_Ctor, CScreen_OnLoad
    void* powerInfo;                       // 0x30: 4 uses in CScreen_OnDestroy, CScreen_Ctor, CScreen_OnLoad
    void* accuracyInfo;                       // 0x34: 4 uses in CScreen_OnDestroy, CScreen_Ctor, CScreen_OnLoad
    void* windIndicatorResource;                       // 0x38: 6 uses in CScreen_OnDestroy, CScreen_OnLoad, CScreen_Ctor
    void* ingameStatus;                       // 0x3c: 5 uses in CScreen_OnDestroy, CScreen_Ctor, CScreen_OnLoad
    void* auxiliaryMenu;                       // 0x40: 4 uses in CScreen_OnDestroy, CScreen_Ctor, CScreen_OnLoad
    void* pangInfo;                       // 0x44: 4 uses in CScreen_OnDestroy, CScreen_Ctor, CScreen_OnLoad
    void* skinsInfo;                       // 0x48: 3 uses in CScreen_OnDestroy, CScreen_Ctor
    void* nearestPinInfo;                       // 0x4c: 3 uses in CScreen_OnDestroy, CScreen_Ctor
    void* modeInfo;                       // 0x50: 5 uses in CScreen_OnDestroy, CScreen_OnLoad, CScreen_Ctor
    void* modeLabelRenderer;                       // 0x54: 6 uses in CScreen_OnDestroy, CScreen_OnLoad, CScreen_Ctor
    void* statusTextRenderer;                       // 0x58: 14 uses in CScreen_OnLoad, CScreen_OnDestroy, CScreen_Ctor
    char pad_05C[0x4];                  // 0x5c
    float displayTimeRemaining;         // 0x60: CScreen_OnProcess subtracts dt and clamps to zero; HUD draw path tests it
    void* nameHashHead;                       // 0x64: 10 uses in CScreen_OnDestroy, CScreen_Ctor
    void* nameHashIterator;                       // 0x68: 6 uses in CScreen_OnDestroy, CScreen_Ctor
    int unk_6C;                         // 0x6c: 1 use in CScreen_Ctor
    void* nameHashFreeList;                       // 0x70: 5 uses in CScreen_OnDestroy, CScreen_Ctor
    int nameHashInitialBucketCount;                         // 0x74: 2 uses in CScreen_Ctor
    int nameHashMask;                   // 0x78: name hash masked by bucket count minus one
    int nameHashBucketCount;            // 0x7c: bounds bucket initialization and clearing
    void* nameHashBuckets;              // 0x80: allocated pointer array for name lookup
    char pad_084[0x18];                 // 0x84
    int chatMessageFontResource;                         // 0x9c: 2 uses in CScreen_Ctor, CScreen_OnLoad
    int meterPresentation;                         // 0xa0: 1 use in CScreen_Ctor
    int clubChangeDisplayState;                         // 0xa4: 1 use in CScreen_Ctor
    int displayedClubIndex;                         // 0xa8: 1 use in CScreen_Ctor
    char pad_0AC[0x28]; // 0xac
    unsigned int terrainIndicatorColor; // 0xd4: CScreen_OnProcess
    char pad_0D8[0xb0]; // 0xd8
    int hudFlags;                        // 0x188: 1 use in CScreen_Ctor
    char pad_18C[0x10];                 // 0x18c
    unsigned char tutorialFlashCount;              // 0x19c: 1 use in CScreen_OnLoad
    unsigned char tutorialFlashOn;              // 0x19d: 1 use in CScreen_OnLoad
    char pad_19E[0x2];                  // 0x19e
    float tutorialFlashInterval;                      // 0x1a0: 1 use in CScreen_OnLoad
    float tutorialFlashTimer;                      // 0x1a4: 1 use in CScreen_OnLoad
    unsigned char tutorialCompletionPhase;              // 0x1a8: 1 use in CScreen_OnLoad
    char pad_1A9[0x3];                  // 0x1a9
    float tutorialCompletionInterval;                      // 0x1ac: 1 use in CScreen_OnLoad
    char pad_1B0[0xc];                  // 0x1b0
    int tutorialTitlePhase;                        // 0x1bc: 1 use in CScreen_OnLoad
    unsigned char modeLabelOpacity;              // 0x1c0: 1 use in CScreen_OnLoad
    char pad_1C1[0xb];                  // 0x1c1
    float tutorialTitleTimer;                      // 0x1cc: 1 use in CScreen_OnLoad
    char pad_1D0[0x5];                  // 0x1d0
    unsigned char hudDisplayEnabled;              // 0x1d5: 1 use in CScreen_OnPreLoadInit
    char pad_1D6[0x2]; // 0x1d6
    unsigned int previousGameStateFlags; // 0x1d8: CScreen_OnProcess
    float pangyaFeedbackRed; // 0x1dc: CScreen_OnProcess
    char pad_1E0[0x14]; // 0x1e0
    void* calipers;                      // 0x1f4: 4 uses in CScreen_OnDestroy, CScreen_Ctor, CScreen_OnLoad
    int fxInterpreterKey;                        // 0x1f8: 2 uses in CScreen_Ctor, CScreen_OnDestroy
    char pad_1FC[0x60];                 // 0x1fc
    int hudInitialized;                        // 0x25c: 1 use in CScreen_Ctor
    char pad_260[0xc];                  // 0x260
    int shotOptionEnabled;                        // 0x26c: 1 use in CScreen_Ctor
    char pad_270[0x14];                 // 0x270
    int tutorialGaugeEnabled;                       // 0x284: CScreen_OnInit clears; draw path tests visibility gate
    char pad_288[0x4];                  // 0x288
    int tutorialClubChangeEnabled;                       // 0x28c: CScreen_OnInit sets; CScreen_UpdateClubChangeDisplay tests
    char pad_290[0x20];                 // 0x290
    int swingFeedbackPending;                       // 0x2b0: CScreen_OnInit clears; CScreen_StartDirectionalSwingFeedback sets
    int ballWindScriptId;                        // 0x2b4: 5 uses in CScreen_OnDestroy, CScreen_Ctor, CScreen_OnLoad
    int partyGolfBaseScriptId;                        // 0x2b8: 5 uses in CScreen_OnDestroy, CScreen_Ctor, CScreen_OnLoad
    int timebombScriptId;                        // 0x2bc: 5 uses in CScreen_OnDestroy, CScreen_Ctor, CScreen_OnLoad
    int pululuJarashiScriptId;                        // 0x2c0: 5 uses in CScreen_OnDestroy, CScreen_Ctor, CScreen_OnLoad
    int featherBadgeScriptId;                        // 0x2c4: 5 uses in CScreen_OnDestroy, CScreen_Ctor, CScreen_OnLoad
    void* ballWindScript;                      // 0x2c8: 1 use in CScreen_OnLoad
    void* partyGolfBaseScript;                      // 0x2cc: 1 use in CScreen_OnLoad
    void* timebombScript;                      // 0x2d0: 1 use in CScreen_OnLoad
    void* pululuJarashiScript;                      // 0x2d4: 1 use in CScreen_OnLoad
    void* featherBadgeScript;                      // 0x2d8: 1 use in CScreen_OnLoad
    char pad_2DC[0x28];                 // 0x2dc
    int hudDisplayMask;                       // 0x304: CScreen_UpdateShotMeterPresentation selects display modes 1/2/4/8/16
    char pad_308[0xc];                  // 0x308
    int hudPhase;                      // 0x314: CScreen_OnProcess selects HUD modes; display routines branch on it
    char pad_318[0x14];                 // 0x318
    void* layout;                      // 0x32c: 7 uses in CScreen_OnDestroy, CScreen_Ctor, CScreen_OnLoad
    void* strokePointDisplay;                      // 0x330: 8 uses in CScreen_OnDestroy, CScreen_Ctor, CScreen_OnLoad
    void* mapCameraDisplay;                      // 0x334: 8 uses in CScreen_OnDestroy, CScreen_Ctor, CScreen_OnLoad
    void* ballWindDisplay;                      // 0x338: 8 uses in CScreen_OnDestroy, CScreen_Ctor, CScreen_OnLoad
    void* golfBaseDisplay;                      // 0x33c: 8 uses in CScreen_OnDestroy, CScreen_Ctor, CScreen_OnLoad
    void* swingResultDisplay;                      // 0x340: 7 uses in CScreen_OnDestroy, CScreen_OnLoad, CScreen_Ctor
    void* tutorialIllustration;                      // 0x344: 11 uses in CScreen_OnLoad, CScreen_OnDestroy, CScreen_Ctor
    void* textTable;                      // 0x348: 5 uses in CScreen_OnDestroy, CScreen_Ctor
    char pad_34C[0x8];                  // 0x34c
    int hudEnabled;                        // 0x354: 1 use in CScreen_Ctor
    int lightType;                        // 0x358: 1 use in CScreen_Ctor
    float lightDiffuseRed;                      // 0x35c: 1 use in CScreen_Ctor
    float lightDiffuseGreen;                      // 0x360: 1 use in CScreen_Ctor
    float lightDiffuseBlue;                      // 0x364: 1 use in CScreen_Ctor
    char pad_368[0x4];                  // 0x368
    float lightSpecularRed;                      // 0x36c: 1 use in CScreen_Ctor
    float lightSpecularGreen;                      // 0x370: 1 use in CScreen_Ctor
    float lightSpecularBlue;                      // 0x374: 1 use in CScreen_Ctor
    char pad_378[0x4];                  // 0x378
    float lightAmbientRed;                      // 0x37c: 1 use in CScreen_Ctor
    float lightAmbientGreen;                      // 0x380: 1 use in CScreen_Ctor
    float lightAmbientBlue;                      // 0x384: 1 use in CScreen_Ctor
    char pad_388[0x10];                 // 0x388
    float lightDirectionX;                      // 0x398: 1 use in CScreen_Ctor
    float lightDirectionY;                      // 0x39c: 1 use in CScreen_Ctor
    float lightDirectionZ;                      // 0x3a0: 1 use in CScreen_Ctor

    // offsets whose address is passed on (sub-objects or arrays, not placed): 0x64, 0x84, 0x194, 0x20c, 0x2b4, 0x2b8, 0x2bc, 0x2c0, 0x2c4, 0x358, 0x398
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define CSCREEN_OFF(f, o) typedef char CScreen_off_##f[((int)&((CScreen*)0)->f == (o)) ? 1 : -1]
CSCREEN_OFF(unk_8, 0x8);
CSCREEN_OFF(taskMainVtable, 0x18);
CSCREEN_OFF(holeInfoSet, 0x1c);
CSCREEN_OFF(flyInfo, 0x20);
CSCREEN_OFF(ballPreview, 0x24);
CSCREEN_OFF(windInfo, 0x28);
CSCREEN_OFF(remainingTimeInfo, 0x2c);
CSCREEN_OFF(powerInfo, 0x30);
CSCREEN_OFF(accuracyInfo, 0x34);
CSCREEN_OFF(windIndicatorResource, 0x38);
CSCREEN_OFF(ingameStatus, 0x3c);
CSCREEN_OFF(auxiliaryMenu, 0x40);
CSCREEN_OFF(pangInfo, 0x44);
CSCREEN_OFF(skinsInfo, 0x48);
CSCREEN_OFF(nearestPinInfo, 0x4c);
CSCREEN_OFF(modeInfo, 0x50);
CSCREEN_OFF(modeLabelRenderer, 0x54);
CSCREEN_OFF(statusTextRenderer, 0x58);
CSCREEN_OFF(nameHashHead, 0x64);
CSCREEN_OFF(nameHashIterator, 0x68);
CSCREEN_OFF(unk_6C, 0x6c);
CSCREEN_OFF(nameHashFreeList, 0x70);
CSCREEN_OFF(nameHashInitialBucketCount, 0x74);
CSCREEN_OFF(nameHashMask, 0x78);
CSCREEN_OFF(nameHashBucketCount, 0x7c);
CSCREEN_OFF(nameHashBuckets, 0x80);
CSCREEN_OFF(chatMessageFontResource, 0x9c);
CSCREEN_OFF(meterPresentation, 0xa0);
CSCREEN_OFF(clubChangeDisplayState, 0xa4);
CSCREEN_OFF(displayedClubIndex, 0xa8);
CSCREEN_OFF(hudFlags, 0x188);
CSCREEN_OFF(tutorialFlashCount, 0x19c);
CSCREEN_OFF(tutorialFlashOn, 0x19d);
CSCREEN_OFF(tutorialFlashInterval, 0x1a0);
CSCREEN_OFF(tutorialFlashTimer, 0x1a4);
CSCREEN_OFF(tutorialCompletionPhase, 0x1a8);
CSCREEN_OFF(tutorialCompletionInterval, 0x1ac);
CSCREEN_OFF(tutorialTitlePhase, 0x1bc);
CSCREEN_OFF(modeLabelOpacity, 0x1c0);
CSCREEN_OFF(tutorialTitleTimer, 0x1cc);
CSCREEN_OFF(hudDisplayEnabled, 0x1d5);
CSCREEN_OFF(calipers, 0x1f4);
CSCREEN_OFF(fxInterpreterKey, 0x1f8);
CSCREEN_OFF(hudInitialized, 0x25c);
CSCREEN_OFF(shotOptionEnabled, 0x26c);
CSCREEN_OFF(ballWindScriptId, 0x2b4);
CSCREEN_OFF(partyGolfBaseScriptId, 0x2b8);
CSCREEN_OFF(timebombScriptId, 0x2bc);
CSCREEN_OFF(pululuJarashiScriptId, 0x2c0);
CSCREEN_OFF(featherBadgeScriptId, 0x2c4);
CSCREEN_OFF(ballWindScript, 0x2c8);
CSCREEN_OFF(partyGolfBaseScript, 0x2cc);
CSCREEN_OFF(timebombScript, 0x2d0);
CSCREEN_OFF(pululuJarashiScript, 0x2d4);
CSCREEN_OFF(featherBadgeScript, 0x2d8);
CSCREEN_OFF(layout, 0x32c);
CSCREEN_OFF(strokePointDisplay, 0x330);
CSCREEN_OFF(mapCameraDisplay, 0x334);
CSCREEN_OFF(ballWindDisplay, 0x338);
CSCREEN_OFF(golfBaseDisplay, 0x33c);
CSCREEN_OFF(swingResultDisplay, 0x340);
CSCREEN_OFF(tutorialIllustration, 0x344);
CSCREEN_OFF(textTable, 0x348);
CSCREEN_OFF(hudEnabled, 0x354);
CSCREEN_OFF(lightType, 0x358);
CSCREEN_OFF(lightDiffuseRed, 0x35c);
CSCREEN_OFF(lightDiffuseGreen, 0x360);
CSCREEN_OFF(lightDiffuseBlue, 0x364);
CSCREEN_OFF(lightSpecularRed, 0x36c);
CSCREEN_OFF(lightSpecularGreen, 0x370);
CSCREEN_OFF(lightSpecularBlue, 0x374);
CSCREEN_OFF(lightAmbientRed, 0x37c);
CSCREEN_OFF(lightAmbientGreen, 0x380);
CSCREEN_OFF(lightAmbientBlue, 0x384);
CSCREEN_OFF(lightDirectionX, 0x398);
CSCREEN_OFF(lightDirectionY, 0x39c);
CSCREEN_OFF(lightDirectionZ, 0x3a0);
CSCREEN_OFF(terrainIndicatorColor, 0xd4);
CSCREEN_OFF(previousGameStateFlags, 0x1d8);
CSCREEN_OFF(pangyaFeedbackRed, 0x1dc);
CSCREEN_OFF(displayTimeRemaining, 0x60);
CSCREEN_OFF(tutorialGaugeEnabled, 0x284);
CSCREEN_OFF(tutorialClubChangeEnabled, 0x28c);
CSCREEN_OFF(swingFeedbackPending, 0x2b0);
CSCREEN_OFF(hudDisplayMask, 0x304);
CSCREEN_OFF(hudPhase, 0x314);
#undef CSCREEN_OFF

#endif  // CLASSES_CSCREEN_H

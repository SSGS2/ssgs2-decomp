#ifndef CLASSES_CSCREEN_H
#define CLASSES_CSCREEN_H

#include "game/types.h"

// One terrain-meter sample block (0x38 bytes): float/int pairs copied as a unit by the shot meter.
struct TerrainPoint {
    float v;
    unsigned int col;
};

struct TerrainProfileSlot {
    float f0;
    int i0;
    float f1;
    int i1;
    float f2;
    int i2;
    float f3;
    int i3;
    float f4;
    int i4;
    float f5;
    int i5;
    float f6;
    int i6;
};

struct ScreenMessage;

class CScreen {
public:
    CScreen();

    // Lifecycle: OnPreLoadInit, then OnLoad, OnInit and OnDestroy.
    void OnPreLoadInit();
    void OnDestroy();
    void OnLoad();
    void OnProcess(float dt);
    void OnDisplay();
    void* OnInit();

    void DrawClubModelOverlay();

    // Both return 1 when the HUD's scripts and layouts are ready.
    int PollLoadComplete();
    int AreHudTransitionsComplete();
    // Draws the impact timing sprite during the unassisted impact phase.
    void DrawImpactTimingSprite();
    // Sets up the loaded HUD layouts and their initial visibility.
    void InitializeLoadedLayouts();
    // Animates club changes and shows the overlay groups for the selected club.
    void UpdateClubChangeDisplay();
    // Picks the shot-phase and motion captions from the current shot state.
    void UpdateSwingCaption();
    // Camera button and shot-hint state; sets the highlighted camera indicator.
    void UpdateCameraAndShotHints();
    // Advances the panel open/close state machine each frame.
    void UpdatePanelTransitions();
    // Classifies the impact direction against the target and starts the swing result display.
    void StartDirectionalSwingFeedback();

    // Selects the shot-meter widgets for the current camera, putting and tutorial state, and runs the tutorial blink.
    void UpdateShotMeterPresentation();
    // Routes one screen command (ScreenMessageId) to the HUD state it changes.
    void HandleMsg(const ScreenMessage* message);

    static void BuildOrientedBasisMatrix(char* out, void* source);

    static char* GetRTTI();

    void* vtbl;                     // 0x0: original vtable pointer
    char pad_004[0x4];              // 0x4
    int unk_8;                      // 0x8
    char pad_00C[0xc];              // 0xc
    void* taskMainVtable;           // 0x18
    void* holeInfoSet;              // 0x1c
    void* flyInfo;                  // 0x20
    void* ballPreview;              // 0x24
    void* windInfo;                 // 0x28
    void* remainingTimeInfo;        // 0x2c
    void* powerInfo;                // 0x30
    void* accuracyInfo;             // 0x34
    void* windIndicatorResource;    // 0x38
    void* ingameStatus;             // 0x3c
    void* auxiliaryMenu;            // 0x40
    void* pangInfo;                 // 0x44
    void* skinsInfo;                // 0x48
    void* nearestPinInfo;           // 0x4c
    void* modeInfo;                 // 0x50
    void* modeLabelRenderer;        // 0x54
    void* statusTextRenderer;       // 0x58
    float ballRingHeight;           // 0x5c
    float displayTimeRemaining;     // 0x60: CScreen_OnProcess subtracts dt and clamps to zero; HUD draw path tests it
    void* nameHashHead;             // 0x64
    void* nameHashIterator;         // 0x68
    int unk_6C;                     // 0x6c
    void* nameHashFreeList;         // 0x70
    int nameHashInitialBucketCount; // 0x74
    int nameHashMask;               // 0x78: name hash masked by bucket count minus one
    int nameHashBucketCount;        // 0x7c: bounds bucket initialization and clearing
    void* nameHashBuckets;          // 0x80: allocated pointer array for name lookup
    char nameMutex[0x18];           // 0x84: lock for the named-overlay registry
    int chatMessageFontResource;    // 0x9c
    int meterPresentation;          // 0xa0
    int clubChangeDisplayState;     // 0xa4
    int displayedClubIndex;         // 0xa8
    char pad_0AC[0x4];              // 0xac
    float windHeading;              // 0xb0
    int windStrength;               // 0xb4
    unsigned char initialDisplayEnabled; // 0xb8
    char pad_0B9[0x3];
    unsigned int powerFeedbackColor;    // 0xbc
    unsigned int feedbackColor0;        // 0xc0
    unsigned int feedbackColor1;        // 0xc4
    char pad_0C8[0x4];                  // 0xc8
    int terrainMarkerIndex;             // 0xcc
    int terrainSampleCount;             // 0xd0
    unsigned int terrainIndicatorColor; // 0xd4: CScreen_OnProcess
    float shotIndicatorPulse;           // 0xd8
    union {
        TerrainProfileSlot terrainProfile[3]; // 0xdc: sample blocks copied whole by UpdateShotMeterPresentation
        TerrainPoint terrainPoints[21];       // 0xdc: the same bytes as value/colour pairs used by the HUD handler
    };
    float terrainProfileScale;             // 0x184
    int hudFlags;                          // 0x188
    float shotFeedbackBlend;               // 0x18c
    float groundLieBlend;                  // 0x190
    unsigned char tutorialStageVisible[8]; // 0x194
    unsigned char tutorialFlashCount;      // 0x19c
    unsigned char tutorialFlashOn;         // 0x19d
    char pad_19E[0x2];                     // 0x19e
    float tutorialFlashInterval;           // 0x1a0
    float tutorialFlashTimer;              // 0x1a4
    unsigned char tutorialCompletionPhase; // 0x1a8
    char pad_1A9[0x3];                     // 0x1a9
    float tutorialCompletionInterval;      // 0x1ac
    float tutorialCompletionTimer;         // 0x1b0
    int previousTutorialStage;             // 0x1b4
    float cameraCaptionOpacity;            // 0x1b8
    int tutorialTitlePhase;                // 0x1bc
    unsigned char modeLabelOpacity;        // 0x1c0
    char pad_1C1[0x3];                     // 0x1c1
    float tutorialTitleX;                  // 0x1c4
    float tutorialTitleY;                  // 0x1c8
    float tutorialTitleTimer;              // 0x1cc
    float tutorialPointerDelay;            // 0x1d0
    unsigned char tutorialPointerDelayed;  // 0x1d4
    unsigned char hudDisplayEnabled;       // 0x1d5
    char pad_1D6[0x2];                     // 0x1d6
    unsigned int previousGameStateFlags;   // 0x1d8: CScreen_OnProcess
    float pangyaFeedbackRed;               // 0x1dc: CScreen_OnProcess
    float pangyaFeedbackGreen;             // 0x1e0
    float pangyaFeedbackBlue;              // 0x1e4
    float clubChangeAngle;                 // 0x1e8
    int clubChangeAnimationDirection;      // 0x1ec
    float clubChangeAnimationTime;         // 0x1f0
    void* calipers;                        // 0x1f4
    int fxInterpreterKey;                  // 0x1f8
    char pad_1FC[0x10];                    // 0x1fc
    char tutorialTitleKeys[5][16];         // 0x20c
    int hudInitialized;                    // 0x25c
    int panelOpenRequested;                // 0x260
    int itemPanelOpenRequested;            // 0x264
    int itemPanelReady;                    // 0x268
    int shotOptionEnabled;                 // 0x26c
    int shotOptionChanged;                 // 0x270
    int resumeAimRequested;                // 0x274
    int hudInputBlocked;                   // 0x278
    int panelsReady;                       // 0x27c
    int tutorialMeterEnabled;              // 0x280: meter tutorial on; UpdateShotMeterPresentation tests it
    int tutorialGaugeEnabled;              // 0x284: CScreen_OnInit clears; draw path tests visibility gate
    int tutorialAimEnabled;                // 0x288: UpdatePanelTransitions gates the aim blink on it
    int tutorialClubChangeEnabled;         // 0x28c: CScreen_OnInit sets; UpdateClubChangeDisplay tests
    int tutorialAimControlEnabled;         // 0x290
    int tutorialSpinControlEnabled;        // 0x294
    char pad_298[0x4];                     // 0x298
    int tutorialIllustrationActive;        // 0x29c
    int cameraControlsEnabled;             // 0x2a0
    int aimControlsEnabled;                // 0x2a4
    int dropModeActive;                    // 0x2a8
    int shotTransitionRequested;           // 0x2ac
    int swingFeedbackPending;              // 0x2b0: CScreen_OnInit clears; CScreen_StartDirectionalSwingFeedback sets
    int ballWindScriptId;                  // 0x2b4
    int partyGolfBaseScriptId;             // 0x2b8
    int timebombScriptId;                  // 0x2bc
    int pululuJarashiScriptId;             // 0x2c0
    int featherBadgeScriptId;              // 0x2c4
    void* ballWindScript;                  // 0x2c8
    void* partyGolfBaseScript;             // 0x2cc
    void* timebombScript;                  // 0x2d0
    void* pululuJarashiScript;             // 0x2d4
    void* featherBadgeScript;              // 0x2d8
    int pululuAnimationPhase;              // 0x2dc
    int meterFadeFrame;                    // 0x2e0: frames of the meter fade-out
    int impactFeedbackStarted;             // 0x2e4: counts the impact feedback frames
    int tutorialAimBlinkFrame;             // 0x2e8
    int tutorialAimBlinkCount;             // 0x2ec
    int tutorialMeterBlinkFrame;           // 0x2f0
    int tutorialMeterBlinkCount;           // 0x2f4
    int tutorialGaugeBlinkFrame;           // 0x2f8
    int tutorialGaugeBlinkCount;           // 0x2fc
    char pad_300[0x4];                     // 0x300
    int hudDisplayMask;      // 0x304: CScreen_UpdateShotMeterPresentation selects display modes 1/2/4/8/16
    int requestedPanel;      // 0x308
    int savedPanel;          // 0x30c
    int previousPanel;       // 0x310
    int hudPhase;            // 0x314: CScreen_OnProcess selects HUD modes; display routines branch on it
    int previousHudPhase;    // 0x318
    int shotOptionResult;    // 0x31c
    int shotDirectionResult; // 0x320
    int partyMeterIndex;     // 0x324
    int meterOpacity; // 0x328: set to 0xff with the loaded layouts; the impact timing sprite uses it as its alpha
    void* layout;     // 0x32c
    void* strokePointDisplay;   // 0x330
    void* mapCameraDisplay;     // 0x334
    void* ballWindDisplay;      // 0x338
    void* golfBaseDisplay;      // 0x33c
    void* swingResultDisplay;   // 0x340
    void* tutorialIllustration; // 0x344
    void* textTable;            // 0x348
    char pad_34C[0x8];          // 0x34c
    int hudEnabled;             // 0x354
    int lightType;              // 0x358
    float lightDiffuseRed;      // 0x35c
    float lightDiffuseGreen;    // 0x360
    float lightDiffuseBlue;     // 0x364
    char pad_368[0x4];          // 0x368
    float lightSpecularRed;     // 0x36c
    float lightSpecularGreen;   // 0x370
    float lightSpecularBlue;    // 0x374
    char pad_378[0x4];          // 0x378
    float lightAmbientRed;      // 0x37c
    float lightAmbientGreen;    // 0x380
    float lightAmbientBlue;     // 0x384
    char pad_388[0x10];         // 0x388
    float lightDirectionX;      // 0x398
    float lightDirectionY;      // 0x39c
    float lightDirectionZ;      // 0x3a0

    ~CScreen();
};

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
CSCREEN_OFF(panelOpenRequested, 0x260);
CSCREEN_OFF(itemPanelOpenRequested, 0x264);
CSCREEN_OFF(itemPanelReady, 0x268);
CSCREEN_OFF(panelsReady, 0x27c);
CSCREEN_OFF(tutorialAimEnabled, 0x288);
CSCREEN_OFF(tutorialAimBlinkFrame, 0x2e8);
CSCREEN_OFF(tutorialAimBlinkCount, 0x2ec);
CSCREEN_OFF(requestedPanel, 0x308);
CSCREEN_OFF(previousPanel, 0x310);
CSCREEN_OFF(partyMeterIndex, 0x324);
CSCREEN_OFF(meterOpacity, 0x328);
CSCREEN_OFF(terrainMarkerIndex, 0xcc);
CSCREEN_OFF(terrainSampleCount, 0xd0);
CSCREEN_OFF(terrainProfile, 0xdc);
CSCREEN_OFF(tutorialMeterEnabled, 0x280);
CSCREEN_OFF(meterFadeFrame, 0x2e0);
CSCREEN_OFF(impactFeedbackStarted, 0x2e4);
CSCREEN_OFF(tutorialMeterBlinkFrame, 0x2f0);
CSCREEN_OFF(tutorialGaugeBlinkCount, 0x2fc);
CSCREEN_OFF(ballRingHeight, 0x5c);
CSCREEN_OFF(windHeading, 0xb0);
CSCREEN_OFF(windStrength, 0xb4);
CSCREEN_OFF(powerFeedbackColor, 0xbc);
CSCREEN_OFF(feedbackColor0, 0xc0);
CSCREEN_OFF(feedbackColor1, 0xc4);
CSCREEN_OFF(shotIndicatorPulse, 0xd8);
CSCREEN_OFF(shotFeedbackBlend, 0x18c);
CSCREEN_OFF(groundLieBlend, 0x190);
CSCREEN_OFF(tutorialStageVisible, 0x194);
CSCREEN_OFF(tutorialCompletionTimer, 0x1b0);
CSCREEN_OFF(previousTutorialStage, 0x1b4);
CSCREEN_OFF(tutorialPointerDelay, 0x1d0);
CSCREEN_OFF(tutorialPointerDelayed, 0x1d4);
CSCREEN_OFF(shotOptionChanged, 0x270);
CSCREEN_OFF(resumeAimRequested, 0x274);
CSCREEN_OFF(hudInputBlocked, 0x278);
CSCREEN_OFF(tutorialAimControlEnabled, 0x290);
CSCREEN_OFF(tutorialSpinControlEnabled, 0x294);
CSCREEN_OFF(tutorialIllustrationActive, 0x29c);
CSCREEN_OFF(cameraControlsEnabled, 0x2a0);
CSCREEN_OFF(aimControlsEnabled, 0x2a4);
CSCREEN_OFF(dropModeActive, 0x2a8);
CSCREEN_OFF(shotTransitionRequested, 0x2ac);
CSCREEN_OFF(shotOptionResult, 0x31c);
CSCREEN_OFF(shotDirectionResult, 0x320);
CSCREEN_OFF(nameMutex, 0x84);
CSCREEN_OFF(initialDisplayEnabled, 0xb8);
CSCREEN_OFF(terrainProfileScale, 0x184);
CSCREEN_OFF(cameraCaptionOpacity, 0x1b8);
CSCREEN_OFF(tutorialTitleX, 0x1c4);
CSCREEN_OFF(pangyaFeedbackGreen, 0x1e0);
CSCREEN_OFF(clubChangeAngle, 0x1e8);
CSCREEN_OFF(tutorialTitleKeys, 0x20c);
CSCREEN_OFF(pululuAnimationPhase, 0x2dc);
CSCREEN_OFF(savedPanel, 0x30c);
CSCREEN_OFF(previousHudPhase, 0x318);
#undef CSCREEN_OFF

#endif // CLASSES_CSCREEN_H

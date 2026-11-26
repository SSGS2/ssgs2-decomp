#ifndef CLASSES_CGOLFRULE_H
#define CLASSES_CGOLFRULE_H

#include "game/types.h"
#include "game/golf_shot_types.h"

struct CE00Self;

class CGolfRule {
public:
    // Construction. Init constructs the rule in place.
    void* Init();
    ~CGolfRule();
    void* DtorThunk18(int flags);
    void ApplyPostRoundChoice(int state);
    void UpdatePlayerRanksAndHoleOrder();
    int IsCurrentHoleComplete();
    void ResetPlayerPositionsAndShotHistory(const char* position);
    void ProcessHoleResults(float dt);
    void ProcessDelayedStateTransition(float dt);
    void ProcessPlayerPreview(float dt);
    void ProcessButtonShotAnimation(float delta);
    void ProcessMotionShotAnimation(float delta);
    void ProcessPostRoundMenus(float dt);
    void HandleMsg(const unsigned char* msg);

    // Actor type descriptor and the static registration hook.
    static void* GetRTTI();
    static void RegisterType();

    // Load, init, reload and display hooks of the rule.
    void OnPreLoadInit();
    void OnLoad();
    void OnInit();
    void OnReload();
    void OnDestroy();
    void OnDisplay();

    // Player snapshots and round flow.
    void SetupPlayerSnapshots(int loadSelectedEquipment);
    void InitializePlayerSnapshot(int playerIndex, void* playerName, void* displayName, void* playerSlot,
                                  void* caddieId, void* controllerIndex);
    void ResetHoleBallAndWind();
    void BeginPlayerTurn(float dt);
    void ProcessFightResult(float dt);

    // State processors and helpers called from the rule's state machine.
    int PollAdvanceOrDismiss();
    int PollCameraToggle();
    void UpdateCameraModeTransition(float dt); // dt is not read
    void PrepareNextShot(float dt);
    void ProcessCourseReload();
    void ProcessStorySequence(float value);
    void WaitForHoleActors(float dt); // dt is not read
    void ProcessRoundResultPresentation(float dt);
    int PreparePlayerTurn();
    void InitializeAimHeading();
    void StartBallFlightFromSample(void* arg);
    void FinalizeShotAndAdvanceState();
    void ResetToShotStart(void* arg);
    void RestorePreviousShot(int index, int copy);
    void UpdateBezierPathCamera(float delta);

    // Per-frame update; dt is the elapsed time in seconds.
    void OnProcess(float dt);

    // Shot input, gauges, playback and shot flow.
    void RecordDirectionalShotInput();
    void UpdateSingleStageGauge(int swingPressed, float elapsedSeconds);
    void UpdatePowerAndImpactGauge(int pressed, float frameStep);
    void ProcessMotionCameraInput(float delta);
    void ProcessMotionSwingInput(float deltaSeconds);
    void ProcessButtonSwingInput(float deltaSeconds);
    void ProcessAimInput(float dt);
    void UpdateBallPlayback(float delta);
    void ProcessShotOutcome(float delta);
    void ProcessBallFlight(float delta);
    // Takes the rule object as its first argument (see the source).
    static void StartRoundResultPresentation(char* self, int arg1, int arg2, char* arg3);
    // Takes the rule object as its first argument (see the source).
    static int StartHoleResultPresentation(CE00Self* self);
    int BuildShotSimulation(int previewOnly);
    void EnterState(int requestedState);

    // Round flow, shot gauges, playback and the dialog result.
    int SelectNextPlayer(int arg); // returns playerCount when no player is left in a party game
    void AdvanceTurnOrRound(int flag);
    void CommitShotGauge();
    void ResetShotGauge();
    void ClassifyShotPresentation();
    void StartShotPlaybackCamera();
    void SetupLast30FramesPlayback();
    static void RequestModeExitTransition(int unused = 0);
    static void ApplyImpactAssistance(void* unused,
                                      int adjust); // unused object argument, kept for the original registers
    int OnStateDialogResult(int state);

    // Preloads, result menus and the background-music playlist.
    int PreloadShotAndPartySequences();
    int PreloadBallEffects();
    void* CreateResultMenu(int arg);
    void* CreateFightResultOrRetryMenu();
    void BuildBgmPlaylist(const char* directory); // directory is passed by callers but not read

    void* vtbl;            // 0x0: original vtable pointer
    char pad_004[0x4];     // 0x4
    int unk_8;             // 0x8
    char pad_00C[0xc];     // 0xc
    void* secondaryVtable; // 0x18
    char pad_01C[0x18];
    float elapsedTime; // +34: OnProcess timer
    char pad_038[0x3C];
    unsigned char animationStarted; // +74
    unsigned char swingAllowed;     // 0x75
    char pad_076[2];
    WVector aimTarget;               // +78
    float unk_084[3];                // +84: vector storage; wider role unresolved
    unsigned char stateEntryPending; // +90
    unsigned char unk_091, unk_092;
    unsigned char shotUndoEnabled; // 0x93
    unsigned char unk_094;
    char pad_095[3];
    int roundRestarted;       // 0x98
    float stateTimer;         // +9C: camera duration/playback fraction, by state
    void* courseCameraBounds; // +A0: borrowed until course reload
    unsigned char doublePowerCameraReset;
    char pad_0A5[3];
    float launchHeadingOffset; // 0xa8
    union {
        ShotHistoryStorage shotHistories[4]; // +AC: four six-word segmented deques
        struct {
            char pad_0AC[4];
            int shotHistoryCount; // legacy signed count of slot zero
            char pad_0B4[0x58];
        };
    };
    ShotCheckpoint currentShotCheckpoints[4]; // +10C, stride 0x54
    char pad_25C[0x84];
    unsigned char nextPlayerIndex; // 0x2e0
    char pad_2E1[0x6f];            // 0x2e1
    int unk_350;                   // 0x350
    char pad_354[8];
    char clubName[0x18];               // +35C
    char ballFlightSoundName[0x20];    // +374
    int cameraPathPoints;              // 0x394
    int cameraPathPointCount;          // 0x398
    int cameraPathPointCapacity;       // 0x39c
    unsigned char turn;                // 0x3a0
    char pad_3A1[0x3];                 // 0x3a1
    int impactGaugeDirection;          // 0x3a4
    int lastGaugeTick, firstGaugeTick; // +3A8, +3AC: signed OS millisecond samples
    char unk_3B0;                      // 0x3b0
    unsigned char processingPaused;    // 0x3b1
    unsigned char networkSyncPending;  // 0x3b2
    char pad_3B3[0x1];                 // 0x3b3
    int netMsgWaitLine;                // 0x3b4
    int netMsgSend;                    // 0x3b8
    int netMsgRecv;                    // 0x3bc
    int netMsgSendTime;                // 0x3c0
    int netMsgRecvTime;                // 0x3c4
    float networkWaitTimeout;          // +3C8
    char pad_3CC[1];
    unsigned char autoShot;             // 0x3cd
    char pad_3CE[0xa];                  // 0x3ce
    unsigned char puttAnimationEnabled; // 0x3d8
    char pad_3D9[0x3];                  // 0x3d9
    int deferredInputButtons;           // 0x3dc
    unsigned char selectedBgmIndex;     // 0x3e0
    char pad_3E1[3];
    int heldDirectionTick; // +3E4
    char pad_3E8[1];
    unsigned char shotBlock;   // 0x3e9
    char pad_3EA[0x2];         // 0x3ea
    float shotBlockTime;       // 0x3ec
    char unk_3F0;              // 0x3f0
    char pad_3F1[0x3];         // 0x3f1
    int unk_3F4;               // 0x3f4
    char unk_3F8;              // 0x3f8
    char pad_3F9[0x7];         // 0x3f9
    int stateTransitionDialog; // 0x400
    int unk_404;               // 0x404
    union {
        char trajectoryEventDispatched;
        unsigned char trajectoryEventDispatchedByte;
    }; // +408: preserve both access widths
    char pad_409[3];
    float minimumPowerFrameSeconds; // +40C: minimum nonzero gauge delta
    char forgetDlg;                 // 0x410
    char pad_411[0x3];              // 0x411
    float forgetDlgTime;            // 0x414
    int forgetItemSlot;             // 0x418
    char unk_41C;                   // 0x41c
    char pad_41D[0x3];              // 0x41d
    float coinTime;                 // 0x420
    int coinCount;                  // 0x424
    int coinNum;                    // 0x428
    int bgmNames;                   // 0x42c
    int bgmNameCount;               // 0x430
    int bgmNameCapacity;            // 0x434
    int forgetItemDialog;           // 0x438
    int unk_43C;                    // 0x43c
    int unk_440;                    // 0x440
    int unk_444;                    // 0x444
    int unk_448;                    // 0x448
    int unk_44C;                    // 0x44c
    int unk_450;                    // 0x450
    int unk_454;                    // 0x454
    char pad_458[0xc];              // 0x458
    int unk_464;                    // 0x464
    int unk_468;                    // 0x468
    int unk_46C;                    // 0x46c
    char unk_470;                   // 0x470
    char pad_471[3];
    int simulationStartFrame, playbackFrame, shotContactFrame; // +474/+478/+47C
    int unk_480;                                               // 0x480
    int displayUpdateCounter;                                  // 0x484
    int forceDisplayUpdate;                                    // 0x488
    unsigned char hudVisible;                                  // 0x48c
    char unk_48D;                                              // 0x48d
    char unk_48E;                                              // 0x48e
    char pad_48F[0x1];                                         // 0x48f
    int shotPresentationFlags;                                 // 0x490
    int presentationStep;                                      // +494
    char unk_498;                                              // 0x498
    char pad_499[0x3];                                         // 0x499
    int playerResultSequenceActive;                            // 0x49c
    int playerResultSequenceIndex;                             // 0x4a0
    int playerResultSequencePhase;                             // 0x4a4
    char pad_4A8[0x4];                                         // 0x4a8
    int resultMenu;                                            // 0x4ac
    int retryMenu;                                             // 0x4b0
    int postRoundMenuState;                                    // 0x4b4
    int nextRoundAction;                                       // 0x4b8
    int tutorialFeedbackEnabled;                               // 0x4bc
    int tutorialAdvancePending;                                // 0x4c0
    void* padSwingController;                                  // 0x4c4
    char pad_4C8[8];
    int courseReloadStage;   // +4D0
    void* childObjects;      // 0x4d4
    int childObjectCount;    // 0x4d8
    int childObjectCapacity; // 0x4dc
    char pad_4E0[0x4];       // 0x4e0
    union {
        char fastPlayback;
        unsigned char fastPlaybackByte;
    }; // +4E4: playback uses unsigned byte
    char pad_4E5[3];
    WVector holeSpawnPosition; // +4E8
    int miniGameSystemResult;  // 0x4f4
};

#define CGOLFRULE_OFF(f, o) typedef char CGolfRule_off_##f[((int)&((CGolfRule*)0)->f == (o)) ? 1 : -1]
typedef char CGolfRule_size_check[(sizeof(CGolfRule) == 0x4f8) ? 1 : -1];
CGOLFRULE_OFF(unk_8, 0x8);
CGOLFRULE_OFF(secondaryVtable, 0x18);
CGOLFRULE_OFF(elapsedTime, 0x34);
CGOLFRULE_OFF(animationStarted, 0x74);
CGOLFRULE_OFF(aimTarget, 0x78);
CGOLFRULE_OFF(unk_084, 0x84);
CGOLFRULE_OFF(stateEntryPending, 0x90);
CGOLFRULE_OFF(unk_091, 0x91);
CGOLFRULE_OFF(unk_092, 0x92);
CGOLFRULE_OFF(unk_094, 0x94);
CGOLFRULE_OFF(stateTimer, 0x9C);
CGOLFRULE_OFF(courseCameraBounds, 0xA0);
CGOLFRULE_OFF(doublePowerCameraReset, 0xA4);
CGOLFRULE_OFF(shotHistories, 0xAC);
CGOLFRULE_OFF(currentShotCheckpoints, 0x10C);
CGOLFRULE_OFF(clubName, 0x35C);
CGOLFRULE_OFF(ballFlightSoundName, 0x374);
CGOLFRULE_OFF(lastGaugeTick, 0x3A8);
CGOLFRULE_OFF(firstGaugeTick, 0x3AC);
CGOLFRULE_OFF(networkWaitTimeout, 0x3C8);
CGOLFRULE_OFF(heldDirectionTick, 0x3E4);
CGOLFRULE_OFF(trajectoryEventDispatchedByte, 0x408);
CGOLFRULE_OFF(minimumPowerFrameSeconds, 0x40C);
CGOLFRULE_OFF(simulationStartFrame, 0x474);
CGOLFRULE_OFF(playbackFrame, 0x478);
CGOLFRULE_OFF(shotContactFrame, 0x47C);
CGOLFRULE_OFF(presentationStep, 0x494);
CGOLFRULE_OFF(courseReloadStage, 0x4D0);
CGOLFRULE_OFF(fastPlaybackByte, 0x4E4);
CGOLFRULE_OFF(holeSpawnPosition, 0x4E8);
CGOLFRULE_OFF(swingAllowed, 0x75);
CGOLFRULE_OFF(shotUndoEnabled, 0x93);
CGOLFRULE_OFF(roundRestarted, 0x98);
CGOLFRULE_OFF(launchHeadingOffset, 0xa8);
CGOLFRULE_OFF(shotHistoryCount, 0xb0);
CGOLFRULE_OFF(nextPlayerIndex, 0x2e0);
CGOLFRULE_OFF(unk_350, 0x350);
CGOLFRULE_OFF(cameraPathPoints, 0x394);
CGOLFRULE_OFF(cameraPathPointCount, 0x398);
CGOLFRULE_OFF(cameraPathPointCapacity, 0x39c);
CGOLFRULE_OFF(turn, 0x3a0);
CGOLFRULE_OFF(impactGaugeDirection, 0x3a4);
CGOLFRULE_OFF(unk_3B0, 0x3b0);
CGOLFRULE_OFF(processingPaused, 0x3b1);
CGOLFRULE_OFF(networkSyncPending, 0x3b2);
CGOLFRULE_OFF(netMsgWaitLine, 0x3b4);
CGOLFRULE_OFF(netMsgSend, 0x3b8);
CGOLFRULE_OFF(netMsgRecv, 0x3bc);
CGOLFRULE_OFF(netMsgSendTime, 0x3c0);
CGOLFRULE_OFF(netMsgRecvTime, 0x3c4);
CGOLFRULE_OFF(autoShot, 0x3cd);
CGOLFRULE_OFF(puttAnimationEnabled, 0x3d8);
CGOLFRULE_OFF(deferredInputButtons, 0x3dc);
CGOLFRULE_OFF(selectedBgmIndex, 0x3e0);
CGOLFRULE_OFF(shotBlock, 0x3e9);
CGOLFRULE_OFF(shotBlockTime, 0x3ec);
CGOLFRULE_OFF(unk_3F0, 0x3f0);
CGOLFRULE_OFF(unk_3F4, 0x3f4);
CGOLFRULE_OFF(unk_3F8, 0x3f8);
CGOLFRULE_OFF(stateTransitionDialog, 0x400);
CGOLFRULE_OFF(unk_404, 0x404);
CGOLFRULE_OFF(trajectoryEventDispatched, 0x408);
CGOLFRULE_OFF(forgetDlg, 0x410);
CGOLFRULE_OFF(forgetDlgTime, 0x414);
CGOLFRULE_OFF(forgetItemSlot, 0x418);
CGOLFRULE_OFF(unk_41C, 0x41c);
CGOLFRULE_OFF(coinTime, 0x420);
CGOLFRULE_OFF(coinCount, 0x424);
CGOLFRULE_OFF(coinNum, 0x428);
CGOLFRULE_OFF(bgmNames, 0x42c);
CGOLFRULE_OFF(bgmNameCount, 0x430);
CGOLFRULE_OFF(bgmNameCapacity, 0x434);
CGOLFRULE_OFF(forgetItemDialog, 0x438);
CGOLFRULE_OFF(unk_43C, 0x43c);
CGOLFRULE_OFF(unk_440, 0x440);
CGOLFRULE_OFF(unk_444, 0x444);
CGOLFRULE_OFF(unk_448, 0x448);
CGOLFRULE_OFF(unk_44C, 0x44c);
CGOLFRULE_OFF(unk_450, 0x450);
CGOLFRULE_OFF(unk_454, 0x454);
CGOLFRULE_OFF(unk_464, 0x464);
CGOLFRULE_OFF(unk_468, 0x468);
CGOLFRULE_OFF(unk_46C, 0x46c);
CGOLFRULE_OFF(unk_470, 0x470);
CGOLFRULE_OFF(unk_480, 0x480);
CGOLFRULE_OFF(displayUpdateCounter, 0x484);
CGOLFRULE_OFF(forceDisplayUpdate, 0x488);
CGOLFRULE_OFF(hudVisible, 0x48c);
CGOLFRULE_OFF(unk_48D, 0x48d);
CGOLFRULE_OFF(unk_48E, 0x48e);
CGOLFRULE_OFF(shotPresentationFlags, 0x490);
CGOLFRULE_OFF(unk_498, 0x498);
CGOLFRULE_OFF(playerResultSequenceActive, 0x49c);
CGOLFRULE_OFF(playerResultSequenceIndex, 0x4a0);
CGOLFRULE_OFF(playerResultSequencePhase, 0x4a4);
CGOLFRULE_OFF(resultMenu, 0x4ac);
CGOLFRULE_OFF(retryMenu, 0x4b0);
CGOLFRULE_OFF(postRoundMenuState, 0x4b4);
CGOLFRULE_OFF(nextRoundAction, 0x4b8);
CGOLFRULE_OFF(tutorialFeedbackEnabled, 0x4bc);
CGOLFRULE_OFF(tutorialAdvancePending, 0x4c0);
CGOLFRULE_OFF(padSwingController, 0x4c4);
CGOLFRULE_OFF(childObjects, 0x4d4);
CGOLFRULE_OFF(childObjectCount, 0x4d8);
CGOLFRULE_OFF(childObjectCapacity, 0x4dc);
CGOLFRULE_OFF(fastPlayback, 0x4e4);
CGOLFRULE_OFF(miniGameSystemResult, 0x4f4);
#undef CGOLFRULE_OFF

#endif // CLASSES_CGOLFRULE_H

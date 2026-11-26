// Golf task (CGolfTask): loading the course and actors behind the loading screen, tearing them down,
// and the pause menu with its scorecard.
// The per-hole reads (strokes, results, Pang and party scores) come from the current golf document.

#include "classes/MPBase1Layout.h"
#include "classes/CWiiPauseScore.h"
#include "classes/GolfDocSnapshot8013.h"
#include "game/golfball.h"
#include "classes/StateObj80170224.h"
#include "classes/CGolfDoc.h"
#include "classes/CSharedDoc.h"
// Keep cast expressions at their use sites: helper calls change register scheduling.
#define AsCGolfDoc(p) ((CGolfDoc*)(p))
#define AsCSharedDoc(p) ((CSharedDoc*)(p))

#include "game/singletons.h"
#include "classes/CFx.h"
#include "classes/CGolfTask.h"
#include "classes/CPackTextureResource.h"
#include "classes/CPackedFile.h"
#include "classes/CMouseCursor.h"
#include "classes/CTask.h"
#include "classes/CBalloonSystem.h"
#include "classes/CGolfRule.h"
#include "classes/CLoadingDraw.h"
extern "C" {

void MPBase1_ResetExecutionState(void*);
int MPBase1_Process(void*, float);
void Layout_SetHitTestingEnabled(int);
int ResourceTable_FindEntryByName(void*, const char*, void**);
void __ct__14CWiiPauseScoreFiUcUcPv(void*, int, unsigned char, unsigned char, int);
void LoadingScreen_ReleaseSharedResources();
void GolfTips_CreateDialogResources();
void __register_global_object(void*, void*, void*);
void Allocation_Dtor();
extern void* g_sharedDoc;
extern void* g_fx;
extern void* g_actorSkipMask;
extern signed char s_resourceTableInitialized;
extern void* s_resourceTableStorage;
extern const char* s_golfTaskPreloadArchiveNames[];
extern const char s_resourceTableDtorRegistration_80170224[];
extern const char golfTaskActorStrings[];
extern char s_roundPlayerList[];
extern void* s_partyModePreloadArchiveNames[];
extern float kFloat_0_PauseMenu;

struct SharedRoundHoleNumbers {
    char padBeforeHoleNumbers[0x82a];
    unsigned char courseHoleNumbers[18];
};

struct ScorecardHoleRecord {
    char padBeforePar[0x18];
    unsigned char par;
    char padEnd[7];
};
struct PartyScoreRecord {
    char padBeforeDistance[0x178];
    float driveDistance;
    int holeScores[18];
};
struct PauseRoundTotals {
    char padBeforeTotals[0x94];
    int primaryTotals[4];
    int secondaryTotals[4];
};

static const unsigned int kPreloadFilesOffset = (unsigned int)&((StateObj*)0)->preloadFiles;
static const unsigned int kPreloadFileCount = 6;

struct GolfTaskStringPool {
    char pad0[11];
    char ruleActor[9];
    char pad14[345];
    char dartsActor[12];
    char pad179[92];
    char screenActor[7];
    char pad1dc[72];
    char preloadArchive[12];
    char inGameTexturePack1[16];
    char inGameTexturePack2[16];
    char tutorialTexturePack[16];
    char dialogOpenSound[15];
    char dialogCloseSound[16];
    char enterAction[6];
    char alternateEnterAction[9]; // Second action string tested alongside enterAction.
    char leftControlAction[9];
    char rightControlAction[9];
    char leftShiftAction[7];
    char rightShiftAction[7];
};
#define TASK_STRING_OFFSET(field) ((unsigned int)&((GolfTaskStringPool*)0)->field)

enum GolfTaskRuleState {
    kStateHoleCameraPath = 0x1,
    kStateButtonShotAnimation = 0x40,
    kStateBallFlight = 0x80,
    kStateDelayedTransition = 0x1000,
    kStateHoleResults = 0x2000,
    kStateBeginPlayerTurn = 0x4000,
    kStatePostRoundMenus = 0x20000,
    kStateCourseReload = 0x800000,
    kStateStorySequence = 0x1000000,
    kStateTourExit = 0x2000000,
    kStateRoundResults = 0x4000000,
    kStateMotionSwing = 0x8000000,
    kStateWaitPlayerAndCaddie = 0x10000000,
    kStateWaitHoleResources = 0x40000000,
};
static const int kPauseActionPrimary = 0x11;
static const int kPauseActionSecondary = 0x12;
// Negative player selects GameInputManager::selectedPlayer in Input_IsActionPressed.
static const int kSelectedInputChannel = -1;
static const int kTutorialRule = 2;
// Overall modes and rule types are separate inputs to score-menu selection.
// Mode titles are unresolved; these names specify the observed numeric protocol.
enum GolfTaskGameMode {
    kGameMode0 = 0,
    kGameMode1 = 1,
    kGameMode2 = 2,
    kGameMode3 = 3,
    kGameMode4 = 4,
    kGameMode5 = 5,
    kGameMode6 = 6,
    kGameMode7 = 7
};
enum GolfTaskScoreRule {
    kStrokeRule = 0,
    kMatchRule = 1
};
enum PauseContextCode {
    kPauseCameraIdle = 0,
    kPauseBlockedMotionShotPhase = 2,
    kPauseActionScreenMode = 2,
    kPauseGroundType0 = 0,
    kPauseGroundType1 = 1,
    kPauseGroundType2 = 2,
    kPauseActorActionButton = 3,
    kPauseRetryButton = 4
};

enum ScoreDisplayMode {
    kScoreStroke = 0,
    kScoreStrokeAlternate = 1,
    kScoreMatch = 2,
    kScoreParty = 3,
    kScoreTraining = 4,
    kScoreMatchAlternate = 5,
    kScoreConfirmationOnly = 6
};
enum PartyScoreRule {
    kPartyNone = 0,
    kPartyLowerAdjustedValueWins = 1,
    kPartyHigherCompetitionValueWins = 2,
    kPartySumStrokeValues = 3
};
enum GolfActorMessage {
    kRuleRequestPlayerAction = 0x19,
    kRuleQueryPauseAllowed = 0x2b,
    kRuleQueryRetryAllowed = 0x2c,
    kScreenRequestPauseAction = 0x2e,
    kScreenQueryMode = 0x30,
    kScreenQueryBusy = 0x56
};
enum PauseMenuAction {
    kPauseResume = 0,
    kPauseExitMode = 1,
    kPauseActorAction = 3,
    kPauseRetry = 4,
    kPauseModeTransition = 5
};

// These are separate protocols: gameMode 0/1 choose stroke-alt or match by rule;
// 2/3/4 use confirmation-only, 5 training, 6/7 use party (when partyGameMode!=0)
// or stroke-alt/match-alt, and other values use the basic stroke display.
// Pause actions 0/1/3/4/5 are resume/exit/actor-action/retry/mode-transition.
// SEND_GOLF_MESSAGE sends a borrowed stack payload synchronously; query outputs
// require the named rule/screen actor to exist and write them. This task owns
// preload wrappers, loadingDraw and pauseScoreMenu; actors own resources they retain.

struct Res20 {
    char storage[0x20];
    // Construct a texture-pack resource wrapper around the supplied table entry.
    Res20(const char* name, void* data, int count) {
        ((CPackTextureResource*)this)->Init((const char*)name, (void**)data, (unsigned int)count);
    }
};

struct CLoadingDrawStorage {
    char storage[0x40];
    // Construct loading-screen menu state in its fixed-size allocation.
    CLoadingDrawStorage() {
        ((CLoadingDraw*)this)->__ct();
    }
};

struct CWiiPauseScoreStorage {
    char storage[0x150];
    // Construct the score menu using display mode, player/hole counts, and post-hole flag.
    CWiiPauseScoreStorage(int displayMode, unsigned char players, unsigned char holes, int postHoleMode) {
        __ct__14CWiiPauseScoreFiUcUcPv(this, displayMode, players, holes, postHoleMode);
    }
};

// Start shared/party resource loads and build the loading screen and pause scorecard.
void CGolfTask::BeginLoadingAndBuildPauseMenu() {
    StateObj* self = (StateObj*)this;
    // Phase 1: disable UI hit tests, start common archives, and create texture owners.
    g_actorSkipMask = &AsCGolfDoc((void*)AsCSharedDoc(g_sharedDoc)->golfDoc)->gameStateFlags;
    ((CFx*)g_fx)->SetPaused(0);
    Layout_SetHitTestingEnabled(0);
    unsigned int i = 0;
    const char** preloadName = s_golfTaskPreloadArchiveNames;
    // Keep the store relative to the object base; a direct array cursor changes code.
    char* preloadSlotCursor = (char*)self;
    for (; i < kPreloadFileCount; ++i) {
        *(void**)(preloadSlotCursor + kPreloadFilesOffset) = CPackedFile::Create((void*)*preloadName);
        ++preloadName;
        preloadSlotCursor += sizeof(void*);
    }

    void* textureData1 = 0;
    if (Sing_805DC620() == 0) {
        if (!s_resourceTableInitialized) {
            s_resourceTableStorage = 0;
            __register_global_object(&s_resourceTableStorage, (void*)Allocation_Dtor,
                                     (void*)s_resourceTableDtorRegistration_80170224);
            s_resourceTableInitialized = 1;
        }
        Sing_805DC620() = &s_resourceTableStorage;
    }
    int count = ResourceTable_FindEntryByName(
        Sing_805DC620(), golfTaskActorStrings + TASK_STRING_OFFSET(inGameTexturePack1), &textureData1);
    self->inGameTexturePack1 =
        new Res20(golfTaskActorStrings + TASK_STRING_OFFSET(inGameTexturePack1), textureData1, count);

    void* textureData2 = 0;
    if (0 == Sing_805DC620()) {
        if (!s_resourceTableInitialized) {
            s_resourceTableStorage = 0;
            __register_global_object(&s_resourceTableStorage, (void*)Allocation_Dtor,
                                     (void*)s_resourceTableDtorRegistration_80170224);
            s_resourceTableInitialized = 1;
        }
        Sing_805DC620() = &s_resourceTableStorage;
    }
    count = ResourceTable_FindEntryByName(Sing_805DC620(),
                                          golfTaskActorStrings + TASK_STRING_OFFSET(inGameTexturePack2), &textureData2);
    self->inGameTexturePack2 =
        new Res20(golfTaskActorStrings + TASK_STRING_OFFSET(inGameTexturePack2), textureData2, count);

    if (AsCSharedDoc(g_sharedDoc)->golfRuleType == kTutorialRule) {
        void* tutorialTextureData = 0;
        if (Sing_805DC620() == 0) {
            if (!s_resourceTableInitialized) {
                s_resourceTableStorage = 0;
                __register_global_object(&s_resourceTableStorage, (void*)Allocation_Dtor,
                                         (void*)s_resourceTableDtorRegistration_80170224);
                s_resourceTableInitialized = 1;
            }
            Sing_805DC620() = &s_resourceTableStorage;
        }
        count = ResourceTable_FindEntryByName(
            Sing_805DC620(), golfTaskActorStrings + TASK_STRING_OFFSET(tutorialTexturePack), &tutorialTextureData);
        self->tutorialTexturePack =
            new Res20(golfTaskActorStrings + TASK_STRING_OFFSET(tutorialTexturePack), tutorialTextureData, count);
    }

    // Phase 2: select any party archive, then synchronously initialize the loader.
    int variant = ((int)AsCSharedDoc(g_sharedDoc)->partyGameMode);
    if (variant != 0) {
        void* variantArchives[4];
        void* base = s_partyModePreloadArchiveNames;
        variantArchives[0] = ((void**)base)[0];
        variantArchives[1] = ((void**)base)[1];
        variantArchives[2] = ((void**)base)[2];
        variantArchives[3] = ((void**)base)[3];
        self->variantPreloadFile = CPackedFile::Create((void*)variantArchives[variant]);
    }

    self->loadingDraw = new CLoadingDrawStorage();
    while (AsMPBase1(self->loadingDraw)->initialized == 0)
        MPBase1_Process(self->loadingDraw, kFloat_0_PauseMenu);
    self->loadingDrawFinished = 0;

    // Phase 3: choose the score display from overall game mode and rule type. Every branch constructs
    // CWiiPauseScoreStorage(displayMode, first round player, round hole count, 0); only the displayMode (kScore*) differs.
    int gameMode = ((int)AsCSharedDoc(g_sharedDoc)->gameMode);
    switch (gameMode) {
    case kGameMode0: {
        int ruleType = AsCSharedDoc(g_sharedDoc)->golfRuleType;
        switch (ruleType) {
        case kStrokeRule:
            self->pauseScoreMenu =
                new CWiiPauseScoreStorage(kScoreStrokeAlternate, (unsigned char)*(unsigned int*)s_roundPlayerList,
                                          AsCSharedDoc(g_sharedDoc)->roundHoleCount, 0);
            break;
        case kMatchRule:
            self->pauseScoreMenu =
                new CWiiPauseScoreStorage(kScoreMatch, (unsigned char)*(unsigned int*)s_roundPlayerList,
                                          AsCSharedDoc(g_sharedDoc)->roundHoleCount, 0);
            break;
        }
    } break;
    case kGameMode1: {
        // Identical to mode 0 (kept as a separate case because the compiled switch has two blocks).
        int ruleType = AsCSharedDoc(g_sharedDoc)->golfRuleType;
        switch (ruleType) {
        case kStrokeRule:
            self->pauseScoreMenu =
                new CWiiPauseScoreStorage(kScoreStrokeAlternate, (unsigned char)*(unsigned int*)s_roundPlayerList,
                                          AsCSharedDoc(g_sharedDoc)->roundHoleCount, 0);
            break;
        case kMatchRule:
            self->pauseScoreMenu =
                new CWiiPauseScoreStorage(kScoreMatch, (unsigned char)*(unsigned int*)s_roundPlayerList,
                                          AsCSharedDoc(g_sharedDoc)->roundHoleCount, 0);
            break;
        }
    } break;
    case kGameMode2:
    case kGameMode3:
    case kGameMode4:
        self->pauseScoreMenu =
            new CWiiPauseScoreStorage(kScoreConfirmationOnly, (unsigned char)*(unsigned int*)s_roundPlayerList,
                                      AsCSharedDoc(g_sharedDoc)->roundHoleCount, 0);
        break;
    case kGameMode5:
        self->pauseScoreMenu =
            new CWiiPauseScoreStorage(kScoreTraining, (unsigned char)*(unsigned int*)s_roundPlayerList,
                                      AsCSharedDoc(g_sharedDoc)->roundHoleCount, 0);
        break;
    case kGameMode6:
    case kGameMode7:
        if (((int)AsCSharedDoc(g_sharedDoc)->partyGameMode) != 0) {
            self->pauseScoreMenu =
                new CWiiPauseScoreStorage(kScoreParty, (unsigned char)*(unsigned int*)s_roundPlayerList,
                                          AsCSharedDoc(g_sharedDoc)->roundHoleCount, 0);
        } else if (AsCSharedDoc(g_sharedDoc)->golfRuleType == kStrokeRule) {
            self->pauseScoreMenu =
                new CWiiPauseScoreStorage(kScoreStrokeAlternate, (unsigned char)*(unsigned int*)s_roundPlayerList,
                                          AsCSharedDoc(g_sharedDoc)->roundHoleCount, 0);
        } else {
            self->pauseScoreMenu =
                new CWiiPauseScoreStorage(kScoreMatchAlternate, (unsigned char)*(unsigned int*)s_roundPlayerList,
                                          AsCSharedDoc(g_sharedDoc)->roundHoleCount, 0);
        }
        break;
    default:
        self->pauseScoreMenu = new CWiiPauseScoreStorage(kScoreStroke, (unsigned char)*(unsigned int*)s_roundPlayerList,
                                                         AsCSharedDoc(g_sharedDoc)->roundHoleCount, 0);
        break;
    }

    // Phase 4: seed scorecard metadata before starting actor loading.
    int k = 0;
    while (k < AsCSharedDoc(g_sharedDoc)->roundHoleCount) {
        ((CWiiPauseScore*)(self->pauseScoreMenu))
            ->SetHoleInfo((unsigned char)k, ((SharedRoundHoleNumbers*)(k + (int)g_sharedDoc))->courseHoleNumbers[0],
                          ((ScorecardHoleRecord*)AsCGolfDoc(((char*)AsCSharedDoc(g_sharedDoc)->golfDoc))
                               ->holeRecords)[(unsigned char)k]
                              .par);
        ++k;
    }
    ((CWiiPauseScore*)self->pauseScoreMenu)->displayedHoleCount =
        AsCGolfDoc(((void*)AsCSharedDoc(g_sharedDoc)->golfDoc))->currentHoleIndex;
    LoadingScreen_ReleaseSharedResources();
    GolfTips_CreateDialogResources();
    ((CTask*)self)->StartActorLoading();
}

struct VObj {
    virtual void del(int);
};

struct VObj11 {
    virtual void m0();
    virtual void m1();
    virtual void unresolvedSlot2(); // Unresolved slot 2; unrelated to the input-manager local.
    virtual void m3();
    virtual void m4();
    virtual void m5();
    virtual void m6();
    virtual void m7();
    virtual void m8();
    virtual void m9();
};

struct Msg;
struct VObj10 {
    virtual void m0();
    virtual void m1();
    virtual void unresolvedSlot2(); // Unresolved slot 2; unrelated to the input-manager local.
    virtual void m3();
    virtual void m4();
    virtual void m5();
    virtual void m6();
    virtual void m7();
    virtual void m8();
    virtual void send(Msg*);
};

// Load the common preload archive, initialize actor preloads, then release the archive.
void CGolfTask::LoadPreloadArchive() {
    void* ptr = CPackedFile::Create((void*)(golfTaskActorStrings + TASK_STRING_OFFSET(preloadArchive)));
    while (((CPackedFile*)ptr)->IsLoadFinished() == 0)
        ((CPackedFile*)ptr)->PollLoad();
    ((CTask*)this)->PreLoadInitActors();
    if (ptr != 0) {
        if (ptr != 0)
            ((VObj*)ptr)->del(1);
    }
}

// Dismiss the loading screen, initialize actors, and refresh pause-score hole metadata.
void CGolfTask::InitActors() {
    StateObj* self = (StateObj*)this;
    VObj* v = (VObj*)self->loadingDraw;
    if (v != 0) {
        if (v != 0)
            v->del(1);
        self->loadingDraw = 0;
    }
    Layout_SetHitTestingEnabled(1);
    ((CTask*)this)->InitActors();
    MPBase1_ResetExecutionState(self->pauseScoreMenu);
    int i = 0;
    while (i < AsCSharedDoc(g_sharedDoc)->roundHoleCount) {
        ((CWiiPauseScore*)(self->pauseScoreMenu))
            ->SetHoleInfo((unsigned char)i, ((SharedRoundHoleNumbers*)(i + (int)g_sharedDoc))->courseHoleNumbers[0],
                          ((ScorecardHoleRecord*)AsCGolfDoc(((char*)AsCSharedDoc(g_sharedDoc)->golfDoc))
                               ->holeRecords)[(unsigned char)i]
                              .par);
        ++i;
    }
}

// Wait for all preloads, finish actor loading, release preload owners, and notify the loading screen.
void CGolfTask::LoadActorsCallback(int loadArgument1, int loadArgument2) {
    // Phase 1: blocking polls finish every task-owned preload before actor callbacks.
    for (unsigned int i = 0; i < kPreloadFileCount; ++i) {
        if (((StateObj*)this)->preloadFiles[i] != 0) {
            while (((CPackedFile*)(((StateObj*)this)->preloadFiles[i]))->IsLoadFinished() == 0)
                ((CPackedFile*)(((StateObj*)this)->preloadFiles[i]))->PollLoad();
        }
    }
    if (((StateObj*)this)->inGameTexturePack1 != 0) {
        while (((CPackTextureResource*)(((StateObj*)this)->inGameTexturePack1))->IsLoadFinished() == 0)
            ((CPackTextureResource*)(((StateObj*)this)->inGameTexturePack1))->PollLoad();
    }
    if (((StateObj*)this)->inGameTexturePack2 != 0) {
        while (((CPackTextureResource*)(((StateObj*)this)->inGameTexturePack2))->IsLoadFinished() == 0)
            ((CPackTextureResource*)(((StateObj*)this)->inGameTexturePack2))->PollLoad();
    }
    if (((StateObj*)this)->tutorialTexturePack != 0) {
        while (((CPackTextureResource*)(((StateObj*)this)->tutorialTexturePack))->IsLoadFinished() == 0)
            ((CPackTextureResource*)(((StateObj*)this)->tutorialTexturePack))->PollLoad();
    }
    if (((StateObj*)this)->variantPreloadFile != 0) {
        while (((CPackedFile*)(((StateObj*)this)->variantPreloadFile))->IsLoadFinished() == 0)
            ((CPackedFile*)(((StateObj*)this)->variantPreloadFile))->PollLoad();
    }
    // Phase 2: actors consume ready resources; initialize the score menu fully.
    ((CTask*)((StateObj*)this))->LoadActorsCallback((void*)loadArgument1, (void*)loadArgument2);
    while (AsMPBase1(((StateObj*)this)->pauseScoreMenu)->initialized == 0)
        MPBase1_Process(((StateObj*)this)->pauseScoreMenu, kFloat_0_PauseMenu);
    // Phase 3: release preload owners after consumers are ready, then notify loader.
    for (unsigned int i = 0; i < kPreloadFileCount; ++i) {
        void* q = ((StateObj*)this)->preloadFiles[i];
        if (q != 0) {
            if (q != 0) {
                ((VObj*)q)->del(1);
            }
            ((StateObj*)this)->preloadFiles[i] = 0;
        }
    }
    if (((StateObj*)this)->inGameTexturePack1 != 0) {
        if (((StateObj*)this)->inGameTexturePack1 != 0) {
            ((VObj*)((StateObj*)this)->inGameTexturePack1)->del(1);
        }
        ((StateObj*)this)->inGameTexturePack1 = 0;
    }
    if (((StateObj*)this)->inGameTexturePack2 != 0) {
        if (((StateObj*)this)->inGameTexturePack2 != 0) {
            ((VObj*)((StateObj*)this)->inGameTexturePack2)->del(1);
        }
        ((StateObj*)this)->inGameTexturePack2 = 0;
    }
    if (((StateObj*)this)->tutorialTexturePack != 0) {
        if (((StateObj*)this)->tutorialTexturePack != 0) {
            ((VObj*)((StateObj*)this)->tutorialTexturePack)->del(1);
        }
        ((StateObj*)this)->tutorialTexturePack = 0;
    }
    if (((StateObj*)this)->variantPreloadFile != 0) {
        if (((StateObj*)this)->variantPreloadFile != 0) {
            ((VObj*)((StateObj*)this)->variantPreloadFile)->del(1);
        }
        ((StateObj*)this)->variantPreloadFile = 0;
    }
    ((VObj11*)((StateObj*)this)->loadingDraw)->m9();
}

void GolfTips_ReleaseDialogAndMessageResource();
void PartyItemSlots_DestroyInstance();
extern void* g_mouseCursor;
void SaveData_PersistLocalPlayerSnapshot();
extern void* g_fx;

// Tear down golf UI and actors, restore pointer input, and persist the local player snapshot.
void CGolfTask::NotifyActorsDestroy() {
    StateObj* self = (StateObj*)this;
    self->unk3A5 = 0;
    GolfTips_ReleaseDialogAndMessageResource();
    ((CTask*)self)->NotifyActorsDestroy();
    if (self->pauseScoreMenu != 0) {
        if (self->pauseScoreMenu != 0)
            ((VObj*)self->pauseScoreMenu)->del(1);
        self->pauseScoreMenu = 0;
    }
    PartyItemSlots_DestroyInstance();
    CBalloonSystem::DestroyAllQueuedBalloons();
    CMouseCursor::SetPointerInputEnabled(g_mouseCursor, 1);
    SaveData_PersistLocalPlayerSnapshot();
    ((CFx*)g_fx)->ClearControlsAndResetBox();
}

// Update the base task and record whether the loading-screen animation has finished.
void CGolfTask::ProcessLoadingDraw(float delta) {
    StateObj* self = (StateObj*)this;
    ((CTask*)self)->vfunc_18();
    void* ptr = self->loadingDraw;
    if (ptr != 0)
        self->loadingDrawFinished = MPBase1_Process(ptr, delta);
}

// Report readiness only when both the base task and loading screen are complete.
int CGolfTask::IsLoadingComplete() {
    StateObj* self = (StateObj*)this;
    int result = 0;
    if (CTask::IsActorLoadingComplete(self) != 0 && self->loadingDrawFinished != 0)
        result = 1;
    return result;
}

struct Msg {
    int a;
    int type;
    void* ptr;
    int d, e, f, g, h;
};

struct Recv {
    virtual void m0() = 0;
    virtual void m1() = 0;
    virtual void unresolvedSlot2() = 0; // Unresolved slot 2.
    virtual void m3() = 0;
    virtual void m4() = 0;
    virtual void m5() = 0;
    virtual void m6() = 0;
    virtual void m7() = 0;
    virtual void m8() = 0;
    virtual void handle(struct Msg*) = 0;
};

struct Vec3 {
    int x, y, z;
};

Recv* TaskActorTable_FindByName(StateObj*, const char*);
void InputManager_Init(void*);
void InputManager_Dtor();
void HomeButtonMenu_InitializeState(void*);
void HomeButtonResources_dtor();
int HomeButtonMenu_IsActive(void*);
int Input_IsActionPressed(void*, int, int);
void* InputManager_GetSelectedPlayer(void*);
void InputManager_SetCursorDrawOptions(void*, int, int, int, void*);
void InputManager_SetCursorColorOverride(void*, int, int, int, int, void*);
void MPBase1_Start(void*);
Vec3* GolfBall__Fv();
void* CQuadTree_GetInstance();
int CQuadTree_OverlappedWithObject(void*, Vec3*, float);
int SharedDoc_IsPlayerFlag0Set(int);
void* GolfPlayer_GetEntryByIndex(void*, int);
void SoundManager_PausePlayback(void*);
void SoundManager_ResumePlayback(void*);
void Sound_PlayByName(void*, const char*, int, int, int, int, float, float, float);
void NoticeQueue_Clear(void*);
void Object_GetField14C(void*, int*);
int Pointer_GetRisingActionBits(void*, const char*, int);
int Pointer_IsActionActive(void*, const char*, int);
void GolfTips_UpdateRegisteredDialogs(float);
extern signed char s_inputManagerInitialized;
extern char s_gameInputManager[];
extern char s_inputManagerDtorRegistration_GolfMenu[];
extern signed char s_homeMenuInitialized;
extern char s_homeButtonResourceState[];
extern char s_homeButtonResourcesDtorRegistration[];
extern void* g_helpMsg;
extern void* g_soundManager;
extern int s_golfTaskPauseInputSuppressed; // Nonzero suspends actor/tip processing after the pause-menu update.
extern void* g_inputPointer;

// Resolve the lazily registered input singleton without changing registration order.
static inline void EnsureGolfInputManager() {
    if (GetInputManagerSlot() == 0) {
        if (!s_inputManagerInitialized) {
            InputManager_Init(s_gameInputManager);
            __register_global_object(s_gameInputManager, (void*)InputManager_Dtor,
                                     s_inputManagerDtorRegistration_GolfMenu);
            s_inputManagerInitialized = 1;
        }
        GetInputManagerSlot() = s_gameInputManager;
    }
}

// Keep message construction at the call site: evaluating the hole argument before
// the header stores changes this compiler's instruction/stack-slot scheduling.
#define SEND_GOLF_MESSAGE(typeCode, payload, argument, actorName)            \
    {                                                                        \
        Msg message;                                                         \
        message.a = 0;                                                       \
        message.type = (typeCode);                                           \
        message.ptr = (payload);                                             \
        message.d = (argument);                                              \
        message.e = 0;                                                       \
        message.f = 0;                                                       \
        message.g = 0;                                                       \
        message.h = 0;                                                       \
        void* actor = TaskActorTable_FindByName((StateObj*)this, actorName); \
        if (actor)                                                           \
            ((VObj10*)actor)->send(&message);                                \
    }

// Borrow the current shared golf document as bytes for the reconstructed ABI.
static inline char* GolfDocumentBytes() {
    return (char*)AsCSharedDoc(g_sharedDoc)->golfDoc;
}
// Return the document's zero-based current-hole index.
static inline unsigned char CurrentScorecardHole() {
    return AsCGolfDoc(GolfDocumentBytes())->currentHoleIndex;
}
// Read one player's stroke count at a valid zero-based hole index.
static inline int ReadHoleStrokeCount(void* player, unsigned char hole) {
    return ((GolfDocSnapshot8013*)player)->holeStrokeCounts[hole];
}
// Read the mode-specific result value at a valid zero-based hole index.
static inline int ReadHoleResult(void* player, unsigned char hole) {
    return ((GolfDocSnapshot8013*)player)->holeResultValues[hole];
}
// Read the player's per-hole Pang total; the stored word keeps its signed ABI.
static inline int ReadHolePang(void* player, unsigned char hole) {
    return ((int*)((GolfDocSnapshot8013*)player)->holePangTotals)[hole];
}
// Read the party-specific score array, which is separate from stroke counts.
static inline int ReadPartyHoleScore(void* player, unsigned char hole) {
    return ((PartyScoreRecord*)player)->holeScores[hole];
}

// Handle pause-menu input/results, update actors and tips, then refresh player scorecard values.
void CGolfTask::UpdatePauseMenuActorsAndScorecard(float delta) {
    // Phase 1: query rule permission and let the HOME menu suspend this entire tick.
    unsigned int pauseAllowed;
    int screenBusy;
    int screenMode;
    int retryAllowed;
    int selectedAction;
    SEND_GOLF_MESSAGE(kRuleQueryPauseAllowed, &pauseAllowed, 0, golfTaskActorStrings + TASK_STRING_OFFSET(ruleActor))
    EnsureGolfInputManager();
    void* inputManager = GetInputManagerSlot();
    if (0 == Sing_805DC820()) {
        if (!s_homeMenuInitialized) {
            HomeButtonMenu_InitializeState(s_homeButtonResourceState);
            __register_global_object(s_homeButtonResourceState, (void*)HomeButtonResources_dtor,
                                     s_homeButtonResourcesDtorRegistration);
            s_homeMenuInitialized = 1;
        }
        Sing_805DC820() = s_homeButtonResourceState;
    }
    if (HomeButtonMenu_IsActive(Sing_805DC820()) != 0)
        return;

    // Phase 2: gate pause entry on rule/camera/shot states and configure available
    // actions before freezing effects and sound. Query payloads are borrowed.
    CGolfDoc* golfDoc = AsCGolfDoc((void*)AsCSharedDoc(g_sharedDoc)->golfDoc);
    unsigned int ruleState = ((unsigned int)golfDoc->gameStateFlags);
    // Pause opens only when ALL hold: none of the 11 blocking rule-state bits is set (hole camera path, delayed
    // transition, hole results, player turn start, post-round menus, course reload, story sequence, round results,
    // tour exit, waiting for player/caddie, waiting for hole resources); the camera is idle; not (motion swing in the
    // blocked shot phase); the rule actor allows pausing; no message is already showing; and the pause button
    // (primary or secondary action) was pressed this frame.
    if (!(ruleState & kStateHoleCameraPath) && !(ruleState & kStateDelayedTransition) &&
        !(ruleState & kStateHoleResults) && !(ruleState & kStateBeginPlayerTurn) &&
        !(ruleState & kStatePostRoundMenus) && !(ruleState & kStateCourseReload) &&
        !(ruleState & kStateStorySequence) && !(ruleState & kStateRoundResults) && !(ruleState & kStateTourExit) &&
        !(ruleState & kStateWaitPlayerAndCaddie) && !(ruleState & kStateWaitHoleResources) &&
        golfDoc->cameraMode == kPauseCameraIdle &&
        !((ruleState & kStateMotionSwing) && golfDoc->shotPhase == kPauseBlockedMotionShotPhase) && pauseAllowed != 0 &&
        ((StateObj*)this)->messageActive == 0 &&
        (Input_IsActionPressed(inputManager, kPauseActionPrimary, kSelectedInputChannel) != 0 ||
         Input_IsActionPressed(inputManager, kPauseActionSecondary, kSelectedInputChannel) != 0)) {
        InputManager_SetCursorDrawOptions(inputManager, 1, 1, 0, InputManager_GetSelectedPlayer(inputManager));
        InputManager_SetCursorColorOverride(inputManager, 1, 0xd6, 0x48, 0x5c,
                                            InputManager_GetSelectedPlayer(inputManager));
        MPBase1_Start(((StateObj*)this)->pauseScoreMenu);
        Vec3 vec = *GolfBall__Fv();
        void* ballBoundsHit = (void*)CQuadTree_OverlappedWithObject(CQuadTree_GetInstance(), &vec, kFloat_0_PauseMenu);
        screenBusy = 0;
        SEND_GOLF_MESSAGE(kScreenQueryMode, &screenMode, 0, golfTaskActorStrings + TASK_STRING_OFFSET(screenActor))
        SEND_GOLF_MESSAGE(kScreenQueryBusy, &screenBusy, 0, golfTaskActorStrings + TASK_STRING_OFFSET(screenActor))
        // Button 3 is disabled when ANY of these holds: no bounds hit on ground
        // types 0/1/2; screen mode != 2; player flag 0 set; screen busy; party
        // mode 1 or 2. The double negation below preserves the original branches.
        // Bounds-test result is boolean stored through a pointer-shaped ABI.
        if (!(!((ballBoundsHit == 0 && (((CGolfBall*)GolfBall__Fv())->m_groundType == kPauseGroundType0 ||
                                        ((CGolfBall*)GolfBall__Fv())->m_groundType == kPauseGroundType1 ||
                                        ((CGolfBall*)GolfBall__Fv())->m_groundType == kPauseGroundType2)) ||
                screenMode != kPauseActionScreenMode ||
                SharedDoc_IsPlayerFlag0Set(AsCGolfDoc(((char*)AsCSharedDoc(g_sharedDoc)->golfDoc))->currentPlayer) !=
                    0 ||
                screenBusy != 0 || (unsigned int)(((int)AsCSharedDoc(g_sharedDoc)->partyGameMode) - 1) <= 1))) {
            ((CWiiPauseScore*)(((StateObj*)this)->pauseScoreMenu))->SetMenuButton3Available(0);
        } else {
            ((CWiiPauseScore*)(((StateObj*)this)->pauseScoreMenu))->SetMenuButton3Available(1);
        }
        SEND_GOLF_MESSAGE(kRuleQueryRetryAllowed, &retryAllowed, 0,
                          golfTaskActorStrings + TASK_STRING_OFFSET(ruleActor))
        // Retry additionally needs >1 strokes on the current hole. The byte at
        // player+0x28+hole is GolfDocSnapshot8013::holeStrokeCounts[hole].
        // Keep the byte expression and repeated singleton reads in their order.
        if (retryAllowed != 0 && screenMode == kPauseActionScreenMode &&
            *((unsigned char*)GolfPlayer_GetEntryByIndex(
                  ((void*)AsCSharedDoc(g_sharedDoc)->golfDoc),
                  AsCGolfDoc(((char*)AsCSharedDoc(g_sharedDoc)->golfDoc))->currentPlayer) +
              AsCGolfDoc(((char*)AsCSharedDoc(g_sharedDoc)->golfDoc))->currentHoleIndex + 0x28) > 1 &&
            screenBusy == 0) {
            ((CWiiPauseScore*)(((StateObj*)this)->pauseScoreMenu))->SetMenuButtonAvailable(kPauseRetryButton, 1);
        } else {
            ((CWiiPauseScore*)(((StateObj*)this)->pauseScoreMenu))->SetMenuButtonAvailable(kPauseRetryButton, 0);
        }
        ((CFx*)g_fx)->SetPaused(1);
        if (((StateObj*)this)->messageActive == 0)
            SoundManager_PausePlayback(g_soundManager);
        // Game modes 2, 3 and 4 open the pause menu without the open/close sound.
        unsigned int gameMode = AsCSharedDoc(g_sharedDoc)->gameMode;
        if (gameMode != kGameMode3 && gameMode != kGameMode2 && gameMode != kGameMode4)
            Sound_PlayByName(g_soundManager, golfTaskActorStrings + TASK_STRING_OFFSET(dialogOpenSound), 0, 0, 0, 0,
                             0.5f, 200.0f, 1.0f);
        ((StateObj*)this)->messageActive = 1;
        if (g_helpMsg)
            NoticeQueue_Clear(g_helpMsg);
    }

    // Phase 3: while paused, only poll the score menu. On completion dispatch its
    // action, restore all four cursor lanes, and resume effects/sound.
    if (((StateObj*)this)->messageActive != 0) {
        if (MPBase1_Process(((StateObj*)this)->pauseScoreMenu, delta) == 0)
            return;
        ((CWiiPauseScore*)(((StateObj*)this)->pauseScoreMenu))->HideBpObjects();
        Object_GetField14C(((StateObj*)this)->pauseScoreMenu, &selectedAction);
        switch (selectedAction) {
        case kPauseResume: {
            unsigned int gameMode = AsCSharedDoc(g_sharedDoc)->gameMode;
            if (gameMode != kGameMode3 && gameMode != kGameMode2 && gameMode != kGameMode4)
                Sound_PlayByName(g_soundManager, golfTaskActorStrings + TASK_STRING_OFFSET(dialogCloseSound), 0, 0, 0,
                                 0, 0.5f, 200.0f, 1.0f);
            break;
        }
        case kPauseExitMode:
            CGolfRule::RequestModeExitTransition(0);
            break;
        case kPauseActorAction:
            if (((int)AsCSharedDoc(g_sharedDoc)->partyGameMode) != 3) {
                SEND_GOLF_MESSAGE(kScreenRequestPauseAction, 0, 0,
                                  golfTaskActorStrings + TASK_STRING_OFFSET(screenActor))
            } else {
                SEND_GOLF_MESSAGE(kRuleRequestPlayerAction, 0, 0, golfTaskActorStrings + TASK_STRING_OFFSET(ruleActor))
            }
            break;
        case kPauseRetry:
            SEND_GOLF_MESSAGE(kRuleRequestPlayerAction, (void*)1, 0,
                              golfTaskActorStrings + TASK_STRING_OFFSET(ruleActor))
            break;
        case kPauseModeTransition:
            CGolfTask::RequestModeTransition();
            break;
        }
        EnsureGolfInputManager();
        void* unresolvedSlot2 = GetInputManagerSlot();
        for (int j = 0; 4 > j; ++j) {
            InputManager_SetCursorDrawOptions(unresolvedSlot2, 0, 0, 0, (void*)j);
            InputManager_SetCursorColorOverride(unresolvedSlot2, 0, 0xd6, 0x48, 0x5c, (void*)j);
        }
        ((CFx*)g_fx)->SetPaused(0);
        if (((StateObj*)this)->messageActive != 0)
            SoundManager_ResumePlayback(g_soundManager);
        ((StateObj*)this)->messageActive = 0;
    }

    // Phase 4: resume actor/tip updates and the delay timer. delta uses the same
    // time base as MPBase1_Process; its physical unit is not established here.
    if (s_golfTaskPauseInputSuppressed != 0)
        return;
    if (AsCSharedDoc(g_sharedDoc)->golfRuleType != kTutorialRule) {
        if (Pointer_GetRisingActionBits(g_inputPointer, golfTaskActorStrings + TASK_STRING_OFFSET(enterAction), 1) !=
                0 ||
            Pointer_GetRisingActionBits(g_inputPointer, golfTaskActorStrings + TASK_STRING_OFFSET(alternateEnterAction),
                                        1) != 0) {
            if (!(Pointer_IsActionActive(g_inputPointer, golfTaskActorStrings + TASK_STRING_OFFSET(leftControlAction),
                                         1) != 0 ||
                  Pointer_IsActionActive(g_inputPointer, golfTaskActorStrings + TASK_STRING_OFFSET(rightControlAction),
                                         1) != 0) ||
                !(Pointer_IsActionActive(g_inputPointer, golfTaskActorStrings + TASK_STRING_OFFSET(leftShiftAction),
                                         1) != 0 ||
                  Pointer_IsActionActive(g_inputPointer, golfTaskActorStrings + TASK_STRING_OFFSET(rightShiftAction),
                                         1) != 0)) {
                if (!(Pointer_IsActionActive(g_inputPointer, golfTaskActorStrings + TASK_STRING_OFFSET(leftShiftAction),
                                             1) != 0 ||
                      Pointer_IsActionActive(g_inputPointer,
                                             golfTaskActorStrings + TASK_STRING_OFFSET(rightShiftAction), 1) != 0 ||
                      Pointer_IsActionActive(g_inputPointer,
                                             golfTaskActorStrings + TASK_STRING_OFFSET(leftControlAction), 1) != 0))
                    Pointer_IsActionActive(g_inputPointer,
                                           golfTaskActorStrings + TASK_STRING_OFFSET(rightControlAction), 1);
            }
        }
    }
    ((CTask*)this)->ProcessActors(delta);
    GolfTips_UpdateRegisteredDialogs(delta);
    float t = ((StateObj*)this)->delayTimer;
    if (t > kFloat_0_PauseMenu) {
        t -= delta;
        ((StateObj*)this)->delayTimer = t;
        if (t < 0.1f)
            ((StateObj*)this)->delayTimer = -10.0f;
    }

    // Phase 5: outside shot-animation/flight states, copy current player/hole
    // data to the scorecard. Drive distance is truncated for holeScore and scaled
    // by 100 before truncation for holeResult; source distance units are unknown.
    golfDoc = AsCGolfDoc((void*)AsCSharedDoc(g_sharedDoc)->golfDoc);
    ruleState = ((unsigned int)golfDoc->gameStateFlags);
    if (!(ruleState & kStateButtonShotAnimation) && !(ruleState & kStateBallFlight)) {
        float scale = 100.0f;
        ((CWiiPauseScore*)((StateObj*)this)->pauseScoreMenu)->displayedHoleCount = golfDoc->currentHoleIndex;
        int i = 0;
        while (i < AsCGolfDoc(GolfDocumentBytes())->playerCount) {
            unsigned int holePang;
            int holeScore;
            holeScore = ReadHoleStrokeCount(GolfPlayer_GetEntryByIndex(GolfDocumentBytes(), (unsigned char)i),
                                            CurrentScorecardHole());
            int holeResult;
            holeResult = ReadHoleResult(GolfPlayer_GetEntryByIndex(GolfDocumentBytes(), (unsigned char)i),
                                        CurrentScorecardHole());
            holePang =
                ReadHolePang(GolfPlayer_GetEntryByIndex(GolfDocumentBytes(), (unsigned char)i), CurrentScorecardHole());
            switch (((int)AsCSharedDoc(g_sharedDoc)->partyGameMode)) {
            case kPartyLowerAdjustedValueWins:
                holeScore = ReadPartyHoleScore(GolfPlayer_GetEntryByIndex(GolfDocumentBytes(), (unsigned char)i),
                                               CurrentScorecardHole());
                SEND_GOLF_MESSAGE(0, &holeResult, CurrentScorecardHole(),
                                  golfTaskActorStrings + TASK_STRING_OFFSET(dartsActor))
                break;
            case kPartyHigherCompetitionValueWins:
                holeScore = (int)((PartyScoreRecord*)GolfPlayer_GetEntryByIndex(GolfDocumentBytes(), (unsigned char)i))
                                ->driveDistance;
                holeResult =
                    (int)(scale * ((PartyScoreRecord*)GolfPlayer_GetEntryByIndex(GolfDocumentBytes(), (unsigned char)i))
                                      ->driveDistance);
                break;
            case kPartySumStrokeValues:
                holeScore = ReadPartyHoleScore(GolfPlayer_GetEntryByIndex(GolfDocumentBytes(), (unsigned char)i),
                                               CurrentScorecardHole());
                holeResult = ReadHoleResult(GolfPlayer_GetEntryByIndex(GolfDocumentBytes(), (unsigned char)i),
                                            CurrentScorecardHole());
                break;
            }
            ((CWiiPauseScore*)(((StateObj*)this)->pauseScoreMenu))
                ->SetPlayerHoleScore((unsigned char)i, AsCGolfDoc(GolfDocumentBytes())->currentHoleIndex, holeScore,
                                     holeResult, holePang);
            int primaryRoundTotal =
                (int)((GolfDocSnapshot8013*)GolfPlayer_GetEntryByIndex(GolfDocumentBytes(), (unsigned char)i))
                    ->roundResultWords[0];
            int secondaryRoundTotal =
                (int)((GolfDocSnapshot8013*)GolfPlayer_GetEntryByIndex(GolfDocumentBytes(), (unsigned char)i))
                    ->roundResultWords[1];
            // This shifted view selects totals[i], not a separate menu object.
            char* playerTotalsCursor = (char*)((StateObj*)this)->pauseScoreMenu + (unsigned char)i * sizeof(int);
            ((PauseRoundTotals*)playerTotalsCursor)->secondaryTotals[0] = secondaryRoundTotal;
            ((PauseRoundTotals*)playerTotalsCursor)->primaryTotals[0] = primaryRoundTotal;
            i = (i + 1);
        }
    }
}

} // extern "C"

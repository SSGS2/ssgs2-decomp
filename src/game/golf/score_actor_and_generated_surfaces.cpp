// The score actor (CScore) that shows the scorecard, pause and rank menus and bonus
// rows, and the generated surfaces it builds from polygons: texture layers, UV arrays,
// vertices and indices, with their init and destroy paths.

#include <decomp/static_init.h>
// Score actor readiness, pause/rank menus and generated ground/sea surfaces.
// Frame rate lookup: CScore::OnProcess calls MPBase1_Process until all present
// child forms report initialized; it deliberately waits until the NEXT call to use
// them. This is readiness polling, not a fixed frame-count delay or save I/O.
// transitionProgress advances in deltaSeconds toward shared 1.0/0.0 endpoints;
// opening clamps to 1, closing requires a STRICT overshoot below zero to close.
// CScore::HandleMsg selects/owns the forms and configures shared input cursors;
// OnDisplay chooses the initialized form. Preserve readiness, selection and
// transition order when changing cadence. The owner must supply valid round
// data: 1..18 holes, 1..4 players, currentHoleIndex<roundHoleCount, player IDs 0..3.
// Texture/course lookup: GeneratedSurface_BuildFromPolygon appends TWO layers
// to a THREE-slot owner, flattens vertices to first-point Y, registers a slab
// below it, and maps X/Z to UV using 200/100 world units per repeat. Update
// polygon bounds, UV scaling and layer ownership together; this is not a course
// file loader. See the data section for scalar storage and seaSurfaceTexturePathFormat
// for bitmap names; ScoreHoleCategoryBonuses is shared table data, not a timer.
// Contracts: required self throughout; returned self is the same address (possibly
// freed with positive deleting flags). Borrowed documents/messages/polygons are
// never released. Owned form/resource/array cleanup is described at each entry.
// These lifecycle calls assume serialized scene access; input/sound/ground-region
// changes are global side effects. No viewport/scissor/resolution is set here.
// GolfPlayer_GetEntryByIndex returns this round result prefix.
struct ScorePlayerFields {
    int playerId;
    char reserved4[0x24];
    unsigned char strokes[18];
    unsigned char holeResults[18];
    int holePang[18];
    char reserved94[0x34];
    int scoreToPar;
    int earnedPang;
    char reservedD0[0xA8];
    union {
        int dartsPoints;
        float distance;
    } partyValue;
    int partyHolePoints[18];
};
struct ScoreHoleFields {
    char reserved[0x18];
    unsigned char par;
    unsigned char resultCategory;
    char reserved1A[6];
};
struct ScoreMessage {
    void* sender;
    int command;
    union {
        int value;
        void* output;
    } argument;
};
struct RoundHoleNumbers {
    char reserved[0x82A];
    unsigned char holeNumbers[18];
};
struct ScoreFormTotals {
    char reserved[0x94];
    int scoreToPar[4];
    int earnedPang[4];
};

enum ScoreTransition {
    kScoreClosed = 0,
    kScoreOpening = 2,
    kScoreClosing = 3
};
enum ScoreCommand {
    kScoreSetBonusOverlay = 0,
    kScoreSetMenuActive = 1,
    kScoreClose = 2,
    kScoreAcknowledge = 12,
    kScoreQueryVisible = 13,
    kScoreQueryHoleBonus = 14,
    kScoreQueryResult = 17
};
enum ScoreMenuAction {
    kScoreActionResult2 = 6,
    kScoreActionResult4 = 7,
    kScoreActionResult3 = 8,
    kScoreActionResult1 = 9,
    kScoreActionShowRanks = 10
};
enum ScoreLimits {
    kScoreTutorialAdmissionStage = 5,
    kScorePlayerSlotCount = 4,
    kScoreLastPlayerId = 3,
    kScoreBonusFirstMode = 2,
    kScoreBonusModeSpan = 2,
    kHoleBonusRowBytes = 0x2C,
    kCategory2BonusOffset = 8,
    kCategory3BonusOffset = 0x10,
    kCategory4BonusOffset = 0x1C,
    kSurfaceLayerCapacity = 3,
    kSurfaceLayersPerPolygon = 2,
    kSurfaceArrayCookieBytes = 0x10,
    kSurfaceLayerBytes = 0x14
};
enum ScoreCursorColor {
    kScoreCursorRed = 0xD6,
    kScoreCursorGreen = 0x48,
    kScoreCursorBlue = 0x5C
};
// State model: transitionState 0 = closed, 2 = opening, 3 = closing; progress advances
// by deltaSeconds toward 1 or kScoreInitialTime. scoreMenuActive gates input, rankViewActive
// chooses the form, and menusInitialized requires every present form to finish initialization.
// Menu action -> actor result: 6->2, 7->4, 8->3, 9->1, default->2; 10 opens ranks.
// Result-query consumer: CGolfRule's continuation in golf_rule_round_flow_and_shot_sim.cpp (0x801532E0)
// reads command 17 into an int initialized to 0 (pending/no selection).
// In training (gameMode 5): 1 saves the first-player snapshot for the
// continuation; 2 advances the current hole when course-record updates are off;
// 3 clears input/audio/effects and requests cTrainingTask; 4 requests mode exit.
// With course-record updates enabled, 1 also disables updates and selects the
// golf-background continuation; 3/4 dispatch their transitions immediately.
// In ordinary round continuations ALL nonzero results request the fade/next-hole
// path; the mode-6 special branch can select round results instead. Thus 1..4
// are context-dependent requests, not four universally named pause buttons.
// Preserve menu-action mapping and caller mode/record-update gates together.
// Form mode map (CWiiPauseScore::CWiiPauseScore's first argument): game modes 0/1 use 1 for rule 0,
// 2 for rule 1; game modes 2..4 use 6; mode 5 uses 4; modes 6/7 use 3 for party play,
// otherwise 1 for rule 0 or 5; the default uses 0. These are menu formats, not player IDs.
// Complete selection protocol (producer gameMode; consumer wii_pause_score_ctor.cpp/wii_pause_score_screen.cpp):
// gameMode       rule/party            form format / behavior
// 0,1            stroke(0)            1 stroke, alternate score groups
// 0,1            match(1)             2 match
// 0,1            other                no allocation (must not enter this path)
// 2,3,4          any                  6 confirmation-only, skips score layouts
// 5 (training)   any                  4 fixed training score tables
// 6,7            party!=0             3 party score (partyGameMode selects units)
// 6,7            party==0,stroke      1 stroke
// 6,7            party==0,other       5 alternate match
// other          any                  0 suppresses score card
// Only gameMode 0/rule 0 also creates ViewRank. Modes 6/7 are story/party entry
// paths; user-facing names for gameMode 1..4 are not proved here. Keep their
// protocol codes rather than guessing titles. Form constructors receive player
// option low byte, hole count 1..18, final option=1; no ownership is transferred
// from the document. Unk64/Unk150 below are storage wrappers for ViewRank and
// CWiiPauseScore; deleting virtual slot 0 releases them on close/replacement.

#include "classes/ViewRank.h"
#include "classes/CWiiPauseScore.h"
struct GeneSurfaceFields {
    float minimumX, minimumY, minimumZ; // 0x00: world-space bounds
    float maximumX, maximumY, maximumZ; // 0x0C
    void* points;                       // 0x18: array-new vertices
    int pointCount;                     // 0x1C
    void* layers;                       // 0x20: three GeneratedSurfaceLayerView records
    unsigned char indexCount;           // 0x24: byte-vector prefix
    char reserved25[3];
    void* indexVtable; // 0x28
    int indexBuffer;   // 0x2C: allocated byte indices
    int layerCount;    // 0x30
    unsigned int reserved34;
    float scale; // 0x38
};
#define AsGeneSurface(p) ((GeneSurfaceFields*)(p))

// Byte offsets within the existing pooled strings; storage and encodings stay unchanged.
enum ScoreTextOffset {
    kScoreTextDartsActor = 0xf, // DartsSystem
    kScoreTextActorType = 0x1b, // CScore
};

#include "classes/GeneratedSurfaceLayerView.h"
#define AsGeneratedSurfaceLayerView(p) ((GeneratedSurfaceLayerView*)(p))
#include "classes/CGolfDoc.h"
#include "classes/CSharedDoc.h"
#define AsCGolfDoc(p) ((CGolfDoc*)(p))
#define AsCSharedDoc(p) ((CSharedDoc*)(p))

#include "game/singletons.h"
#define __register_global_object sdk_register_global_object
#include "sdk_protos.h"
#include "classes/CGeneTex3D.h"
#include "classes/CScore.h"
#include "classes/MPBase1Layout.h"
#include "classes/GolfDocSnapshot8013.h"
#include "classes/CBonusInfo.h"

#define AsCGeneTex3D(p) ((CGeneTex3D*)(p))
#undef __register_global_object

extern "C" {

extern char s_scoreRttiDescriptor;
extern float s_scoreActorFullTurnAngle; // Cached full-turn angle for the score actor module.
extern const float kFloat_6_2831855;
extern char s_actorTypeInfo;
extern char s_scoreFactoryLookupKey;        // Score factory lookup key.
extern char s_scoreTaskFactoryRegistration; // Score task-registration descriptor.
extern const char scoreActorStrings[];
extern "C" void __dt__18NativeBase80201D18Fv(void*, int);
extern "C" void __dl__FPv(void*);
extern "C" void* IntPair_Set(void*, const char*, void*);
extern "C" void* ActorFactoryRegistry_GetInstance();
extern "C" void CScore_Create();
extern "C" void StringMap_SetByName(void*, void (*)(void), const char*);
extern "C" void* TaskRegistry_GetInstance();
extern "C" void TaskRegistry_AppendFactory(void*, const char*);
extern "C" void MPBase1_DrawIfInitialized(void*);
extern "C" void __dla__FPv(void*);
extern "C" void Vec3_Zero(void*);
extern "C" void* __nwa__FUl(unsigned int, const char*);
extern "C" void ResourceManager_ReleaseId(void*, void*);
extern "C" void __destroy_new_array(void*, void (*)(void*, int));
extern "C" char __vt__10CGeneTex3D[];
extern "C" void* g_resourceManager;
extern "C" const float kFloat_1_Score;
extern "C" const float kFloat_neg10000_Sea;
extern "C" const float kGeneSurfaceBoundsPositiveSeed;
extern "C" const float kFloat_0_01_GeneSurface;
extern "C" const double kSeaTextureScrollAngleRadians;
extern "C" const float kFloat_200_SeaSurface;
extern "C" const float kFloat_100_GeneSurface;
extern "C" const char seaSurfaceTexturePathFormat[];
extern "C" void __ml__FfRC7WVector(void*, void*, float);
extern "C" void __as__7WVectorFRC7WVector(void*, void*);
extern "C" void AABB_Copy(void*, void*);
extern "C" void* CQuadTree_GetInstance();
extern "C" void GroundRegionList_AppendBounds(void*, void*, int);
extern "C" char* String_FormatTemporary(const char*, ...);
extern "C" void* ResourceManager_LoadBitmap(void*, const char*, int, int, int);
extern "C" void* g_sharedDoc;
extern "C" void* g_soundManager;
extern "C" const float kFloat_0_5_Score;
extern "C" const float kFloat_200_ScoreSound;
extern "C" const float kFloat_1_0_Score;
extern "C" float kScoreInitialTime;
extern "C" int MPBase1_Process(void*, float);
extern "C" void ViewRank_SetEnabled(void*, int);
extern "C" void MPBase1_Start(void*);
extern "C" void Sound_PlayByName(void*, const char*, int, int, int, int, float, float, float);
extern "C" void Object_GetField14C(void*, int*);
extern "C" void* __nw__FUl(unsigned int);
extern "C" void InputManager_Init(void*);
extern "C" void InputManager_SetCursorDrawOptions(void*, int, int, int, int);
extern "C" void InputManager_SetCursorColorOverride(void*, int, int, int, int, int);
extern "C" void InputManager_SetSelectedPlayer(void*, int);
extern "C" void InputManager_Dtor();
extern "C" void __register_global_object(void*, void (*)(void), void*);
extern "C" void SoundManager_StopAndClearChannels(void*);
extern "C" void* GolfPlayer_GetEntryByIndex(void*, unsigned char);
extern "C" int SharedDoc_IsPlayerFlag0Set(int);
extern "C" void* IActor_FindDispatchTargetByName(void*, const char*);
extern "C" void __ct__14CWiiPauseScoreFiUcUcPv(void*, int, unsigned char, unsigned char, int);
// Create the ranking layout, score groups, message resource handle, and number renderer.
extern "C" void ViewRank_Ctor(void*);
struct ScoreOption {
    unsigned int value;
    char pad[8];
    // Read the round-player option's stored word for the menu's byte-sized argument.
    operator unsigned int() const {
        return value;
    }
};
extern "C" ScoreOption s_roundPlayerList;
extern "C" char s_scoreInputManagerDestructorRegistration[];
extern "C" char s_gameInputManager[];
extern "C" const char s_scoreHoleCategoryBonuses[];
extern "C" signed char s_inputManagerInitialized;
extern "C" const float kFloat_100_PauseScore;

struct Overlay801EC334 {
    virtual void v0() = 0;
    virtual void v1() = 0;
    virtual void v2() = 0;
    virtual void v3(float) = 0;
};
struct CBonusInfoStorage {
    // Construct the owned 0x40-byte bonus overlay in this allocation.
    CBonusInfoStorage() {
        ((CBonusInfo*)this)->__ct();
    }
    char pad[0x40];
};
struct BonusInfoView {
    virtual void v0();
    virtual void v1(float);
};
// Poll score/ranking menus, translate selected actions, animate transitions, and update the bonus overlay.

// Allocation wrappers, not unknown menu payloads: rank view (0x64) and pause-score form
// (0x150). Their existing type names are retained because exception helpers can name them.
struct Unk64 {
    // Construct a ViewRank in caller/new-provided 0x64-byte storage.
    Unk64() {
        ViewRank_Ctor(this);
    }
    char pad[0x64];
};
struct Unk150 {
    // Construct the score form; forward format, player option, hole count and final option.
    Unk150(int a, unsigned char b, unsigned char holeCount, int d) {
        __ct__14CWiiPauseScoreFiUcUcPv(this, a, b, holeCount, d);
    }
    char pad[0x150];
};
// DartsSystem request 0 -> CDartsSystem_HandleMsg (drive_contest_and_darts.cpp): first four
// words match DartsInfo {unused,request,output,index}. Output is the FIRST
// nonzero target value for the indexed hole, or entry.items[0] on exhaustion;
// it is not a scored-shot category. Receiver clamps an invalid hole to zero,
// but this score-table caller still requires a valid round hole index.
// The four extension words are zeroed ABI tail, unused by that consumer.
// Dispatch synchronously borrows packet/output, retaining neither pointer.
struct DartsHoleTargetQuery {
    int unusedHeader;
    int request;
    int* targetOut;
    int holeIndex;
    int unused10;
    int unused14;
    int unused18;
    int unused1C;
};
struct V801EC6F4 {
    virtual void v0(int);
};
struct ScoreCommandTarget {
    virtual void v0();
    virtual void v1();
    virtual void v2();
    virtual void v3();
    virtual void v4();
    virtual void v5();
    virtual void v6();
    virtual void v7();
    virtual void v8();
    virtual void dispatchMessage(DartsHoleTargetQuery*);
    // actor slot +0x24
};

#pragma opt_lifetimes off
// Advance score/rank menus after tutorial play and publish their completion or return choice.
// Required actor and round document; deltaSeconds is elapsed seconds passed
// unchanged to forms/bonus animation. Returns no readiness status; publishes
// menusInitialized/result/transition state. A form becoming ready during
// this call still leaves allMenusReady=0 until the following poll. No menu
// objects are freed here; lazy bonus creation transfers ownership to self.
void CScore::OnProcess(float deltaSeconds) {
    // Phase 1: tutorial stages below 5 suppress the entire workflow, including transitions.
    void* sharedDoc = g_sharedDoc;
    if (AsCGolfDoc((*(void**)&AsCSharedDoc(sharedDoc)->golfDoc))->tutorialStage < kScoreTutorialAdmissionStage)
        return;
    if (this->transitionState == kScoreClosed) {
        result = 0;
        scoreMenuActive = 0;
    }
    // Phase 2: initialize every present form before processing selections.
    // Null forms pass this readiness scan. An active selection path still
    // requires its chosen form to exist before the following call.
    if (menusInitialized == 0) {
        int allMenusReady = 1;
        void* object = scoreForm;
        if (object != 0 && AsMPBase1(object)->initialized == 0) {
            MPBase1_Process(object, deltaSeconds);
            allMenusReady = 0;
        }
        object = rankView;
        if (0 != object && AsMPBase1(object)->initialized == 0) {
            MPBase1_Process(object, deltaSeconds);
            allMenusReady = 0;
        }
        menusInitialized = allMenusReady;
    } else {
        void* object = (*(void**)&menusInitialized);
        if (scoreMenuActive != 0) {
            if (rankViewActive != 0) {
                if (MPBase1_Process(rankView, deltaSeconds)) {
                    object = rankView;
                    if (((ViewRank*)object)->selectionResult == 1) {
                        result = 2;
                    } else {
                        ViewRank_SetEnabled(object, 0);
                        MPBase1_Start(scoreForm);
                        rankViewActive = 0;
                        Sound_PlayByName(g_soundManager, scoreActorStrings, 0, 0, 0, 0, kFloat_0_5_Score,
                                         kFloat_200_ScoreSound, kFloat_1_0_Score);
                    }
                }
            } else if (MPBase1_Process(scoreForm, deltaSeconds)) {
                int selectedAction;
                Object_GetField14C(scoreForm, &selectedAction);
                switch (selectedAction) {
                case kScoreActionResult2:
                    result = 2;
                    break;
                case kScoreActionResult4:
                    result = 4;
                    break;
                case kScoreActionResult3:
                    result = 3;
                    break;
                case kScoreActionResult1:
                    result = 1;
                    break;
                case kScoreActionShowRanks:
                    ((CWiiPauseScore*)scoreForm)->HideInactiveScore();
                    MPBase1_Start(rankView);
                    rankViewActive = 1;
                    ViewRank_SetEnabled(rankView, 1);
                    Sound_PlayByName(g_soundManager, scoreActorStrings, 0, 0, 0, 0, kFloat_0_5_Score,
                                     kFloat_200_ScoreSound, kFloat_1_0_Score);
                    break;
                default:
                    result = 2;
                    break;
                }
            }
        }
    }
    // Phase 3: animate opening/closing independently of menu selection completion.
    // Preserve the existing strict comparison at the closing boundary:
    // landing on exactly zero leaves state=closing, and later calls do not
    // subtract again because value>0 is false. This code does not fix it.
    int transitionState = this->transitionState;
    if (transitionState == kScoreOpening) {
        float value = transitionProgress;
        if (value < kFloat_1_0_Score) {
            value += deltaSeconds;
            transitionProgress = value;
            value = (kFloat_1_0_Score > value) ? value : kFloat_1_0_Score;
            transitionProgress = value;
        }
    } else if (transitionState == kScoreClosing) {
        float zero;
        float value;
        value = transitionProgress;
        zero = kScoreInitialTime;
        if (value > zero) {
            (zero > (transitionProgress -= deltaSeconds)) ? zero : transitionProgress;
            if (*(volatile float*)&transitionProgress < kScoreInitialTime) {
                transitionProgress = kScoreInitialTime;
                this->transitionState = kScoreClosed;
            }
        }
    }
    // Phase 4: only game modes 2..4 advance/create the optional bonus overlay.
    if (bonusOverlayEnabled == 1) {
        unsigned int bonusModeIndex = AsCSharedDoc(g_sharedDoc)->gameMode - kScoreBonusFirstMode;
        if (bonusModeIndex <= kScoreBonusModeSpan) {
            elapsedTime += deltaSeconds;
            if ((*(int*)&bonusOverlay) == 0) {
                void* overlay = new CBonusInfoStorage();
                bonusOverlay = overlay;
            }
            void* overlay = bonusOverlay;
            if (overlay != 0)
                ((BonusInfoView*)overlay)->v1(deltaSeconds);
        }
    }
}
#pragma opt_lifetimes reset

// Handle score commands, populate hole/player results, route input ownership, and answer score queries.
// message is borrowed; commands 0/1 take integer values, 2 takes no payload.
// Queries 12/13 write one byte; 14/17 write one int through argument.output.
// Command 1 owns the creation/deletion of rankView and scoreForm and configures shared cursors.
// Require nonnull query outputs of the specified width and initialized input
// manager backing storage. Unhandled commands leave actor/output unchanged.
// Opening expects the supported game/rule modes in the map above; it does
// not delete an old scoreForm first. Callers must close/reset before reopen.
// Cursor RGB below is a UI color, not a format/alpha mask. Command 14 indexes
// 0x2C-byte bonus rows by category counts; preserve the category/column pairing.
void CScore::HandleMsg(void* message) {
    switch (((ScoreMessage*)message)->command) {
    case kScoreSetBonusOverlay: {
        int value = ((ScoreMessage*)message)->argument.value;
        bonusOverlayEnabled = value;
        if (1 != value)
            break;
        void* old = bonusOverlay;
        float one;
        float zero;
        zero = kScoreInitialTime;
        one = kFloat_1_0_Score;
        elapsedTime = zero;
        transitionState = kScoreClosing;
        transitionProgress = one;
        if (old) {
            if (old) {
                ((V801EC6F4*)old)->v0(1);
            }
            bonusOverlay = 0;
        }
        break;
    }
    case kScoreSetMenuActive: {
        if (((ScoreMessage*)message)->argument.value == 0) {
            // Reset only the score workflow; menu objects are released below.
            transitionState = kScoreClosed;
            result = 0;
            scoreMenuActive = 0;
            rankViewActive = 0;
            menusInitialized = 0;
            if (!GetInputManagerSlot()) {
                if (s_inputManagerInitialized == 0) {
                    InputManager_Init(s_gameInputManager);
                    __register_global_object(s_gameInputManager, InputManager_Dtor,
                                             s_scoreInputManagerDestructorRegistration);
                    s_inputManagerInitialized = 1;
                }
                GetInputManagerSlot() = s_gameInputManager;
            }
            int i;
            void* inputManager = GetInputManagerSlot();
            for (i = 0; kScorePlayerSlotCount > i; i++) {
                InputManager_SetCursorDrawOptions(inputManager, 0, 0, 0, i);
                InputManager_SetCursorColorOverride(inputManager, 0, kScoreCursorRed, kScoreCursorGreen,
                                                    kScoreCursorBlue, i);
            }
            void* p = rankView;
            if (p) {
                if (p) {
                    ((V801EC6F4*)p)->v0(1);
                }
                rankView = 0;
            }
            p = this->scoreForm;
            if (p) {
                if (p) {
                    ((V801EC6F4*)p)->v0(1);
                }
                this->scoreForm = 0;
            }
        } else {
            // Open phase 1: select the score-form format from game/rule/party mode.
            transitionState = kScoreOpening;
            scoreMenuActive = 1;
            menusInitialized = 0;
            if (!rankView && !AsCSharedDoc(g_sharedDoc)->golfRuleType &&
                !(*(int*)&AsCSharedDoc(g_sharedDoc)->gameMode)) {
                void* q = new Unk64();
                rankView = q;
                ViewRank_SetEnabled(q, 1);
                rankViewActive = 1;
            } else
                rankViewActive = 0;
            switch ((*(int*)&AsCSharedDoc(g_sharedDoc)->gameMode)) {
            case 0:
                switch (AsCSharedDoc(g_sharedDoc)->golfRuleType) {
                case 0:
                    this->scoreForm =
                        new Unk150(1, (unsigned char)s_roundPlayerList, AsCSharedDoc(g_sharedDoc)->roundHoleCount, 1);
                    break;
                case 1:
                    this->scoreForm =
                        new Unk150(2, (unsigned char)s_roundPlayerList, AsCSharedDoc(g_sharedDoc)->roundHoleCount, 1);
                    break;
                }
                break;
            case 1:
                switch (AsCSharedDoc(g_sharedDoc)->golfRuleType) {
                case 0:
                    this->scoreForm =
                        new Unk150(1, (unsigned char)s_roundPlayerList, AsCSharedDoc(g_sharedDoc)->roundHoleCount, 1);
                    break;
                case 1:
                    this->scoreForm =
                        new Unk150(2, (unsigned char)s_roundPlayerList, AsCSharedDoc(g_sharedDoc)->roundHoleCount, 1);
                    break;
                }
                break;
            case 2:
            case 3:
            case 4:
                this->scoreForm =
                    new Unk150(6, (unsigned char)s_roundPlayerList, AsCSharedDoc(g_sharedDoc)->roundHoleCount, 1);
                break;
            case 5:
                this->scoreForm =
                    new Unk150(4, (unsigned char)s_roundPlayerList, AsCSharedDoc(g_sharedDoc)->roundHoleCount, 1);
                break;
            case 6:
            case 7:
                if ((*(int*)&AsCSharedDoc(g_sharedDoc)->partyGameMode))
                    this->scoreForm =
                        new Unk150(3, (unsigned char)s_roundPlayerList, AsCSharedDoc(g_sharedDoc)->roundHoleCount, 1);
                else if (AsCSharedDoc(g_sharedDoc)->golfRuleType == 0)
                    this->scoreForm =
                        new Unk150(1, (unsigned char)s_roundPlayerList, AsCSharedDoc(g_sharedDoc)->roundHoleCount, 1);
                else
                    this->scoreForm =
                        new Unk150(5, (unsigned char)s_roundPlayerList, AsCSharedDoc(g_sharedDoc)->roundHoleCount, 1);
                break;
            default:
                this->scoreForm =
                    new Unk150(0, (unsigned char)s_roundPlayerList, AsCSharedDoc(g_sharedDoc)->roundHoleCount, 1);
                break;
            }
            if (!GetInputManagerSlot()) {
                if (s_inputManagerInitialized == 0) {
                    InputManager_Init(s_gameInputManager);
                    __register_global_object(s_gameInputManager, InputManager_Dtor,
                                             s_scoreInputManagerDestructorRegistration);
                    s_inputManagerInitialized = 1;
                }
                GetInputManagerSlot() = s_gameInputManager;
            }
            // Open phase 2: choose cursor ownership. Player-list index and controller/player
            // ID are distinct: prefer the lowest ID among entries with player flag 0 clear.
            // If none qualify, use the lowest ID in the full list and the fallback cursor.
            int i;
            int lowestPlayerId;
            unsigned int useFallbackCursor;
            int cursorPlayer;
            void* inputManager;
            inputManager = GetInputManagerSlot();
            cursorPlayer = 0;
            SoundManager_StopAndClearChannels(g_soundManager);
            lowestPlayerId = kScoreLastPlayerId;
            useFallbackCursor = 1;
            for (i = 0; i < AsCGolfDoc((*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc))->playerCount; i++) {
                void* q = GolfPlayer_GetEntryByIndex((*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc), (unsigned char)i);
                if (lowestPlayerId > ((ScorePlayerFields*)q)->playerId) {
                    if (!SharedDoc_IsPlayerFlag0Set(i)) {
                        q = GolfPlayer_GetEntryByIndex((*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc),
                                                       (unsigned char)i);
                        lowestPlayerId = ((ScorePlayerFields*)q)->playerId;
                        useFallbackCursor = 0;
                    }
                }
            }
            if (useFallbackCursor) {
                void* q = GolfPlayer_GetEntryByIndex(
                    (*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc),
                    AsCGolfDoc((*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc))->currentPlayer);
                currentPlayerId = ((ScorePlayerFields*)q)->playerId;
                for (i = 0; i < AsCGolfDoc((*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc))->playerCount; i++) {
                    void* x =
                        GolfPlayer_GetEntryByIndex((*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc), (unsigned char)i);
                    if (lowestPlayerId > ((ScorePlayerFields*)x)->playerId) {
                        x = GolfPlayer_GetEntryByIndex((*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc),
                                                       (unsigned char)i);
                        lowestPlayerId = ((ScorePlayerFields*)x)->playerId;
                    }
                }
                InputManager_SetCursorDrawOptions(inputManager, 1, 1, 0, lowestPlayerId);
                InputManager_SetCursorColorOverride(inputManager, 1, kScoreCursorRed, kScoreCursorGreen,
                                                    kScoreCursorBlue, lowestPlayerId);
                InputManager_SetSelectedPlayer(inputManager, lowestPlayerId);
            } else {
                for (i = 0; i < AsCGolfDoc((*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc))->playerCount; i++) {
                    void* x =
                        GolfPlayer_GetEntryByIndex((*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc), (unsigned char)i);
                    if (lowestPlayerId == ((ScorePlayerFields*)x)->playerId) {
                        if (!SharedDoc_IsPlayerFlag0Set(i)) {
                            cursorPlayer = i;
                            break;
                        }
                    }
                }
                currentPlayerId = ((ScorePlayerFields*)GolfPlayer_GetEntryByIndex(
                                       (*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc),
                                       AsCGolfDoc((*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc))->currentPlayer))
                                      ->playerId;
                InputManager_SetCursorDrawOptions(
                    inputManager, 1, 1, cursorPlayer,
                    ((ScorePlayerFields*)GolfPlayer_GetEntryByIndex((*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc),
                                                                    (unsigned char)cursorPlayer))
                        ->playerId);
                InputManager_SetCursorColorOverride(
                    inputManager, 1, kScoreCursorRed, kScoreCursorGreen, kScoreCursorBlue,
                    ((ScorePlayerFields*)GolfPlayer_GetEntryByIndex((*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc),
                                                                    (unsigned char)cursorPlayer))
                        ->playerId);
                InputManager_SetSelectedPlayer(
                    inputManager, ((ScorePlayerFields*)GolfPlayer_GetEntryByIndex(
                                       (*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc), (unsigned char)cursorPlayer))
                                      ->playerId);
            }
            // Open phase 3: populate hole headers, completed-hole results, then player totals.
            // Repeated document lookups preserve original load/call ordering; do not cache
            // them casually across callbacks. Hole indices here are zero-based.
            for (i = 0; i < AsCSharedDoc(g_sharedDoc)->roundHoleCount; i++) {
                ((CWiiPauseScore*)(this->scoreForm))
                    ->SetHoleInfo((unsigned char)i,
                                  *(unsigned char*)((char*)(i + (unsigned int)g_sharedDoc) +
                                                    (int)&((RoundHoleNumbers*)0)->holeNumbers),
                                  ((ScoreHoleFields*)AsCGolfDoc((*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc))
                                       ->holeRecords)[(unsigned char)i]
                                      .par);
            }
            void* scoreForm = this->scoreForm;
            unsigned char currentHole = AsCGolfDoc((*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc))->currentHoleIndex;
            float percentScale = kFloat_100_PauseScore;
            ((CWiiPauseScore*)scoreForm)->displayedHoleCount = 1 + currentHole;
            // Score-format boundary -> CWiiPauseScore::SetPlayerHoleScore
            // (wii_pause_score_screen.cpp) stores {strokes,competitionValue,pang} as three
            // signed words per hole/player. Normal: stroke count, result
            // category byte, hole Pang. Party 1 (Darts): hole points,
            // queried target, unchanged hole Pang; ranking uses target
            // minus points (lower wins). Party 2 (Drive): trunc(distance),
            // trunc(100*distance), unchanged Pang; ranking uses the latter
            // (higher wins). Distance is the existing partyValue.distance
            // quantity, with no new physical-unit conversion here. Party
            // 3: hole points, result byte, unchanged Pang; totals sum points.
            // Keep the union's int/float interpretation and truncation
            // order. If DartsSystem is absent, competitionValue retains
            // the initial holeResults byte. Packet/output are stack-only.
            for (int holeIndex = 0;
                 holeIndex <= AsCGolfDoc((*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc))->currentHoleIndex;
                 holeIndex++) {
                for (int i = 0; i < AsCGolfDoc((*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc))->playerCount; i++) {
                    int displayedScore;
                    int holePang;
                    void* x =
                        GolfPlayer_GetEntryByIndex((*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc), (unsigned char)i);
                    displayedScore = ((ScorePlayerFields*)x)->strokes[holeIndex];
                    x = GolfPlayer_GetEntryByIndex((*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc), (unsigned char)i);
                    int competitionValue = ((ScorePlayerFields*)x)->holeResults[holeIndex];
                    holePang = ((ScorePlayerFields*)GolfPlayer_GetEntryByIndex(
                                    (*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc), (unsigned char)i))
                                   ->holePang[holeIndex];
                    switch ((*(int*)&AsCSharedDoc(g_sharedDoc)->partyGameMode)) {
                    case 1:
                        displayedScore = ((ScorePlayerFields*)GolfPlayer_GetEntryByIndex(
                                              (*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc), (unsigned char)i))
                                             ->partyHolePoints[holeIndex];
                        DartsHoleTargetQuery targetQuery;
                        targetQuery.unusedHeader = 0;
                        targetQuery.request = 0;
                        targetQuery.targetOut = &competitionValue;
                        targetQuery.holeIndex = holeIndex;
                        targetQuery.unused10 = 0;
                        targetQuery.unused14 = 0;
                        targetQuery.unused18 = 0;
                        targetQuery.unused1C = 0;
                        void* dartsActor =
                            IActor_FindDispatchTargetByName(this, scoreActorStrings + kScoreTextDartsActor);
                        if (dartsActor)
                            ((ScoreCommandTarget*)dartsActor)->dispatchMessage(&targetQuery);
                        break;
                    case 2:
                        displayedScore = (int)((ScorePlayerFields*)GolfPlayer_GetEntryByIndex(
                                                   (*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc), (unsigned char)i))
                                             ->partyValue.distance;
                        competitionValue =
                            (int)(percentScale * ((ScorePlayerFields*)GolfPlayer_GetEntryByIndex(
                                                      (*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc), (unsigned char)i))
                                                     ->partyValue.distance);
                        break;
                    case 3:
                        displayedScore = ((ScorePlayerFields*)GolfPlayer_GetEntryByIndex(
                                              (*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc), (unsigned char)i))
                                             ->partyHolePoints[holeIndex];
                        competitionValue = ((ScorePlayerFields*)GolfPlayer_GetEntryByIndex(
                                                (*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc), (unsigned char)i))
                                               ->holeResults[holeIndex];
                        break;
                    }
                    ((CWiiPauseScore*)(this->scoreForm))
                        ->SetPlayerHoleScore((unsigned char)i, (unsigned char)holeIndex, displayedScore,
                                             competitionValue, holePang);
                }
            }
            int scoreToPar;
            for (int i = 0; i < AsCGolfDoc((*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc))->playerCount; i++) {
                scoreToPar = ((ScorePlayerFields*)GolfPlayer_GetEntryByIndex(
                                  (*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc), (unsigned char)i))
                                 ->scoreToPar;
                void* x = GolfPlayer_GetEntryByIndex((*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc), (unsigned char)i);
                void* form = this->scoreForm;
                unsigned int offset = (unsigned char)i * 4;
                int earnedPang = ((ScorePlayerFields*)x)->earnedPang;
                char* dst = (char*)form + offset;
                ((ScoreFormTotals*)dst)->earnedPang[0] = earnedPang;
                ((ScoreFormTotals*)dst)->scoreToPar[0] = scoreToPar;
            }
        }
        break;
    }
    case kScoreClose: {
        float one = kFloat_1_0_Score;
        transitionState = kScoreClosing;
        transitionProgress = one;
        break;
    }
    case kScoreAcknowledge:
        *(unsigned char*)((ScoreMessage*)message)->argument.output = 1;
        break;
    case kScoreQueryVisible: {
        unsigned int x = transitionState;
        *(unsigned char*)((ScoreMessage*)message)->argument.output = (0 != x);
        break;
    }
    case kScoreQueryHoleBonus: {

        unsigned char category2Count = 0;
        unsigned char category3Count = 0;
        unsigned char category4Count = 0;
        unsigned int last = (unsigned char)(AsCSharedDoc(g_sharedDoc)->roundHoleCount - 1);
        void* players = (*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc);
        for (unsigned int i = 0; i <= last; i++) {
            switch (((ScoreHoleFields*)AsCGolfDoc(players)->holeRecords)[(unsigned char)i].resultCategory) {
            case 2:
                category2Count++;
                break;
            case 3:
                category3Count++;
                break;
            case 4:
                category4Count++;
                break;
            }
        }
        int category2Bonus =
            *(int*)(s_scoreHoleCategoryBonuses + category2Count * kHoleBonusRowBytes + kCategory2BonusOffset);
        int category4Bonus =
            *(int*)(s_scoreHoleCategoryBonuses + category4Count * kHoleBonusRowBytes + kCategory4BonusOffset);
        int category3Bonus =
            *(int*)(s_scoreHoleCategoryBonuses + category3Count * kHoleBonusRowBytes + kCategory3BonusOffset);
        int partial = category2Bonus + category4Bonus;
        int sum = partial + category3Bonus;
        *(int*)((ScoreMessage*)message)->argument.output = sum;
        break;
    }
    case 6:
    case 7:
    case 8:
    case 9:
    case 10:
    case 11:
    case 3:
    case 4:
    case 5:
    case 15:
    case 16:
        break;
    case kScoreQueryResult:
        *(int*)((ScoreMessage*)message)->argument.output = result;
        break;
    default:
        break;
    }
}

// Destroy the score actor base and optionally deallocate the actor.
void* __dt__6CScoreFv(void*, int);

// Destroy the actor base; positive deallocate frees self; return the original object address.
CScore::~CScore() {
    __dt__18NativeBase80201D18Fv(this, 0);
}

// Register the score actor type and factory with the task system.
void CScore_RegisterFactory() {
    s_scoreActorFullTurnAngle = kFloat_6_2831855;
    IntPair_Set(&s_scoreRttiDescriptor, scoreActorStrings + kScoreTextActorType, &s_actorTypeInfo);
    void* object = ActorFactoryRegistry_GetInstance();
    StringMap_SetByName(object, CScore_Create, &s_scoreFactoryLookupKey);
    void* score = TaskRegistry_GetInstance();
    TaskRegistry_AppendFactory(score, &s_scoreTaskFactoryRegistration);
}

struct VObj801EC660 {
    virtual void v0() = 0;
    virtual void v1() = 0;
    virtual void v2() = 0;
    virtual void v3() = 0;
    virtual void v4() = 0;
};
// Draw the enabled bonus overlay and active score or ranking form.
// Borrows self/forms; no ownership changes. The bonus can draw before forms
// are ready; rank/score drawing requires menusInitialized. Keep that order.
void CScore::OnDisplay() {
    if (bonusOverlayEnabled == 1) {
        if (bonusOverlay != 0) {
            VObj801EC660* item = (VObj801EC660*)bonusOverlay;
            item->v2();
        }
    }
    if ((*(void**)&scoreMenuActive) != 0) {
        if ((*(void**)&rankViewActive) != 0) {
            if ((*(void**)&menusInitialized) != 0) {
                MPBase1_DrawIfInitialized(rankView);
            }
        } else if ((*(void**)&menusInitialized) != 0) {
            MPBase1_DrawIfInitialized(scoreForm);
        }
    }
}

// Return the score actor type descriptor.
void* CScore::GetRTTI() {
    return &s_scoreRttiDescriptor;
}

// Adjust the secondary-interface pointer back to the score object and forward
// destruction. The secondary interface sits 0x18 bytes into the object.
void* CScore::DestroyThunkMinus24(int deleting) {
    return __dt__6CScoreFv((char*)this - 0x18, deleting);
}

// Initialize one generated surface texture layer with no UV array or bitmap.
void GeneratedSurfaceLayer_Init(void* self) {
    AsGeneratedSurfaceLayerView(self)->textureCoordinates = 0;
    AsGeneratedSurfaceLayerView(self)->bitmapResource = 0;
}

} // extern "C"

// Allocate generated-surface layer arrays through the game's typed array-allocation entry.
inline void* operator new[](size_t size) {
    return __nwa__FUl(size, __vt__10CGeneTex3D);
}
#define GENETEX3D_NEW_ARRAY new
struct PointED510 {
    // Leave XYZ uninitialized until the polygon copy fills it.
    PointED510() {}
    // Allocate vertex arrays through the raw game allocator; compiler supplies the cookie.
    static void* operator new[](size_t size) {
        return ((void* (*)(unsigned int))__nwa__FUl)(size);
    }
    float x, y, z;
};
struct TexED510 {
    // Leave UV uninitialized until projection from world X/Z.
    TexED510() {}
    // Allocate UV arrays through the raw game allocator; compiler supplies the cookie.
    static void* operator new[](size_t size) {
        return ((void* (*)(unsigned int))__nwa__FUl)(size);
    }
    float u, v;
};
struct ByteED510 {
    unsigned char value;
    virtual void v0();
    virtual void v1();
    virtual unsigned char v2(unsigned char);
};
struct SeaED510 {
    float minimum[3], maximum[3];
    PointED510* points;
    int count;
    char* entries;
    ByteED510 state;
};
extern "C" char s_sceneLighting[];
extern "C" {
// Copy a polygon, flatten its height bounds, register ground bounds, and generate two textured surface layers.
// polygon borrows {XYZ* points, int count}; all output points use the FIRST point's world Y.
// self owns new points/UV arrays and retained bitmap handles. Requires nonempty polygon,
// a point count representable by the byte index vector, and room for two more layers in
// the three-entry allocation. This routine appends layers; it does not reset layerCount.
// Point count must be 1..255 (indexCount is u8), layerCount at entry 0 or 1.
// GeneSurfaceFields and SeaED510 describe the SAME base: +0x24 ByteED510 is
// the embedded byte-index container (value,+4 vtable,+8 buffer); it is not a
// separately allocated surface. +0x20 owns three 0x14-byte TextureLayer records.
// Each layer's count is POSTincremented before filling its UV/bitmap fields;
// the new count also selects texture suffix min(count,2). Layer owns UVs;
// the resource manager owns the bitmap backing, with a retained ID here.
// Required successful point/UV/index allocations; no allocation rollback.
// A null bitmap result is stored without testing: points, registered ground
// slab, appended layer count and UVs remain published, and the second layer
// is still attempted. Destroy/GeneratedSurfaceLayer_Destroy releases each
// nonnull bitmap ID and UV array; no retry or layer-count decrement occurs.
void GeneratedSurface_BuildFromPolygon(void* self, void* polygon) {
    struct Bounds {
        unsigned int a[2], b[2], c[2];
    };
    struct FloatBounds {
        float a[3], b[3];
    };
    FloatBounds bounds;
    Bounds packed;
    int pointIndex;
    int sourceByteOffset;
    unsigned int uvByteOffset;
    int index;
    char* layer;
    int pointCount;
    void* uvArray;
    float positiveBoundsSeed[3];
    float positiveSeedBasis[3];
    float negativeBoundsSeed[3];
    float negativeSeedBasis[3];
    // Phase 1: replace owned points and resize the byte index vector.
    // The -0x10 frees below recover CodeWarrior's new[] cookie, not an earlier vertex.
    void* old = AsGeneSurface(self)->points;
    if (old != 0) {
        if (old != 0)
            __dla__FPv((char*)old - 0x10);
        AsGeneSurface(self)->points = 0;
    }
    int count = *(int*)((char*)polygon + 4);
    AsGeneSurface(self)->pointCount = count;
    void* data = new PointED510[count];
    AsGeneSurface(self)->points = data;
    unsigned char bytePointCount = (unsigned char)AsGeneSurface(self)->pointCount;
    if (*(unsigned char*)((char*)self + (int)&((GeneSurfaceFields*)0)->indexCount) != bytePointCount) {
        unsigned char result = ((ByteED510*)((unsigned int)self + 0x24))->v2(bytePointCount);
        ((SeaED510*)self)->state.value = result;
    }
    // Phase 2: seed an inverted AABB, copy/flatten points and accumulate world-space bounds.
    negativeSeedBasis[0] = kFloat_1_Score;
    negativeSeedBasis[1] = kFloat_1_Score;
    negativeSeedBasis[2] = kFloat_1_Score;
    __ml__FfRC7WVector(negativeBoundsSeed, negativeSeedBasis, kFloat_neg10000_Sea);
    positiveSeedBasis[0] = kFloat_1_Score;
    positiveSeedBasis[1] = kFloat_1_Score;
    positiveSeedBasis[2] = kFloat_1_Score;
    __ml__FfRC7WVector(positiveBoundsSeed, positiveSeedBasis, kGeneSurfaceBoundsPositiveSeed);
    __as__7WVectorFRC7WVector(bounds.a, positiveBoundsSeed);
    __as__7WVectorFRC7WVector(bounds.b, negativeBoundsSeed);
    AABB_Copy(self, bounds.a);
    pointIndex = 0;
    uvByteOffset = 0;
    sourceByteOffset = 0;
    while (pointIndex < AsGeneSurface(self)->pointCount) {
        __as__7WVectorFRC7WVector((float*)AsGeneSurface(self)->points + pointIndex * 3,
                                  (char*)*(void**)polygon + sourceByteOffset);
        float y = *(float*)((char*)*(void**)polygon + 4);
        *(float*)((float*)AsGeneSurface(self)->points + pointIndex * 3 + 1) = y;
        AsGeneSurface(self)->minimumX =
            AsGeneSurface(self)->minimumX > *(float*)((float*)AsGeneSurface(self)->points + pointIndex * 3)
                ? *(float*)((float*)AsGeneSurface(self)->points + pointIndex * 3)
                : AsGeneSurface(self)->minimumX;
        AsGeneSurface(self)->maximumX =
            AsGeneSurface(self)->maximumX < *(float*)((float*)AsGeneSurface(self)->points + pointIndex * 3)
                ? *(float*)((float*)AsGeneSurface(self)->points + pointIndex * 3)
                : AsGeneSurface(self)->maximumX;
        unsigned int off4 = (unsigned int)(pointIndex * 3) * 4 + 4;
        AsGeneSurface(self)->minimumY =
            AsGeneSurface(self)->minimumY > *(float*)((char*)AsGeneSurface(self)->points + off4)
                ? *(float*)((char*)AsGeneSurface(self)->points + off4)
                : AsGeneSurface(self)->minimumY;
        AsGeneSurface(self)->maximumY =
            AsGeneSurface(self)->maximumY < *(float*)((char*)AsGeneSurface(self)->points + off4)
                ? *(float*)((char*)AsGeneSurface(self)->points + off4)
                : AsGeneSurface(self)->maximumY;
        unsigned int off8 = (unsigned int)(pointIndex * 3) * 4 + 8;
        AsGeneSurface(self)->minimumZ =
            AsGeneSurface(self)->minimumZ > *(float*)((char*)AsGeneSurface(self)->points + off8)
                ? *(float*)((char*)AsGeneSurface(self)->points + off8)
                : AsGeneSurface(self)->minimumZ;
        AsGeneSurface(self)->maximumZ =
            AsGeneSurface(self)->maximumZ < *(float*)((char*)AsGeneSurface(self)->points + off8)
                ? *(float*)((char*)AsGeneSurface(self)->points + off8)
                : AsGeneSurface(self)->maximumZ;
        ++pointIndex;
        sourceByteOffset += 0xC;
    }
    // Ground slab lies 1..2 world units below the flattened polygon; tag 1 is passed unchanged.
    float height = AsGeneSurface(self)->maximumY;
    float one = kFloat_1_Score;
    height -= one;
    AsGeneSurface(self)->maximumY = height;
    AsGeneSurface(self)->minimumY = height - one;
    packed = *(Bounds*)self;
    GroundRegionList_AppendBounds(CQuadTree_GetInstance(), &packed, 1);
    // Phase 3: append two scrolling texture layers. Directions use sin/cos(pi/6) at 0.01 scale.
    // Layer 1 projects world X/Z relative to min X/Z with 200 world units per UV unit;
    // layer 2 uses 100 and reverses the Z scroll direction. Alpha is forced opaque.
    float scale = kFloat_0_01_GeneSurface;
    float angle = (float)sin(kSeaTextureScrollAngleRadians);
    float directionX = -scale * angle;
    angle = (float)cos(kSeaTextureScrollAngleRadians);
    float directionZ = -scale * angle;
    layer = (char*)AsGeneSurface(self)->layers + (AsGeneSurface(self)->layerCount)++ * 0x14;
    AsGeneratedSurfaceLayerView(layer)->directionZ = directionZ;
    AsGeneratedSurfaceLayerView(layer)->directionX = directionX;
    AsGeneratedSurfaceLayerView(layer)->packedColor = *(unsigned int*)(s_sceneLighting + 0x18) | 0xFF000000;
    pointCount = AsGeneSurface(self)->pointCount;
    uvArray = new TexED510[pointCount];
    *(void**)&AsGeneratedSurfaceLayerView(layer)->textureCoordinates = uvArray;
    *(void**)&AsGeneratedSurfaceLayerView(layer)->bitmapResource = ResourceManager_LoadBitmap(
        g_resourceManager,
        String_FormatTemporary(seaSurfaceTexturePathFormat,
                               AsGeneSurface(self)->layerCount > 2 ? 2 : AsGeneSurface(self)->layerCount),
        0, 0, 0);
    pointIndex = 0;
    float uvScale = kFloat_200_SeaSurface;
    for (int pointOff = 0, uvOff = 0; pointIndex < AsGeneSurface(self)->pointCount;
         ++pointIndex, pointOff += 0xC, uvOff += 8) {
        float uCoordinate =
            (*(float*)((char*)AsGeneSurface(self)->points + pointOff) - AsGeneSurface(self)->minimumX) / uvScale;
        *(float*)((char*)*(void**)&AsGeneratedSurfaceLayerView(layer)->textureCoordinates + uvOff) = uCoordinate;
        float vCoordinate =
            (*(float*)((char*)AsGeneSurface(self)->points + pointOff + 8) - AsGeneSurface(self)->minimumZ) / uvScale;
        *(float*)((char*)*(void**)&AsGeneratedSurfaceLayerView(layer)->textureCoordinates + uvOff + 4) = vCoordinate;
    }
    angle = (float)sin(kSeaTextureScrollAngleRadians);
    directionX = -scale * angle;
    angle = (float)cos(kSeaTextureScrollAngleRadians);
    layer = (char*)AsGeneSurface(self)->layers + (AsGeneSurface(self)->layerCount)++ * 0x14;
    AsGeneratedSurfaceLayerView(layer)->directionZ = scale * angle;
    AsGeneratedSurfaceLayerView(layer)->directionX = directionX;
    AsGeneratedSurfaceLayerView(layer)->packedColor = *(unsigned int*)(s_sceneLighting + 0x18) | 0xFF000000;
    pointCount = AsGeneSurface(self)->pointCount;
    uvArray = new TexED510[pointCount];
    *(void**)&AsGeneratedSurfaceLayerView(layer)->textureCoordinates = uvArray;
    *(void**)&AsGeneratedSurfaceLayerView(layer)->bitmapResource = ResourceManager_LoadBitmap(
        g_resourceManager,
        String_FormatTemporary(seaSurfaceTexturePathFormat,
                               AsGeneSurface(self)->layerCount > 2 ? 2 : AsGeneSurface(self)->layerCount),
        0, 0, 0);
    pointIndex = 0;
    uvScale = kFloat_100_GeneSurface;
    for (sourceByteOffset = 0, uvByteOffset = 0; pointIndex < AsGeneSurface(self)->pointCount;
         ++pointIndex, uvByteOffset += 8, sourceByteOffset += 0xC) {
        float uCoordinate =
            (*(float*)((char*)AsGeneSurface(self)->points + sourceByteOffset) - AsGeneSurface(self)->minimumX) /
            uvScale;
        *(float*)((char*)*(void**)&AsGeneratedSurfaceLayerView(layer)->textureCoordinates + uvByteOffset) = uCoordinate;
        float vCoordinate =
            (*(float*)((char*)AsGeneSurface(self)->points + sourceByteOffset + 8) - AsGeneSurface(self)->minimumZ) /
            uvScale;
        *(float*)((char*)*(void**)&AsGeneratedSurfaceLayerView(layer)->textureCoordinates + uvByteOffset + 4) =
            vCoordinate;
    }
}

// Release a surface layer UV array and bitmap handle, then optionally deallocate the layer.
void* GeneratedSurfaceLayer_Destroy(void* self, int deallocate) {
    if (self != 0) {
        void* uvArray = (*(void**)&AsGeneratedSurfaceLayerView(self)->textureCoordinates);
        if (uvArray != 0) {
            if (uvArray != 0) {
                __dla__FPv((char*)uvArray - 0x10);
            }
            (*(void**)&AsGeneratedSurfaceLayerView(self)->textureCoordinates) = 0;
        }
        if (g_resourceManager != 0) {
            void* resource = (*(void**)&AsGeneratedSurfaceLayerView(self)->bitmapResource);
            if (resource != 0) {
                ResourceManager_ReleaseId(g_resourceManager, resource);
                (*(void**)&AsGeneratedSurfaceLayerView(self)->bitmapResource) = 0;
            }
        }
        if (deallocate > 0)
            __dl__FPv(self);
    }
    return self;
}

} // extern "C"

struct Entry801ED2C0 {
    // Initialize one owned generated-surface layer's pointers.
    Entry801ED2C0() {
        GeneratedSurfaceLayer_Init(this);
    }
    // Release layer resources without freeing this array element individually.
    ~Entry801ED2C0() {
        GeneratedSurfaceLayer_Destroy(this, 0);
    }
    char data[0x14];
};

extern "C" {
// Initialize generated surface bounds, vertex/index storage, and the three-layer array.
void* GeneratedSurface_Init(void* self) {
    Vec3_Zero(self);
    Vec3_Zero((char*)&AsGeneSurface(self)->maximumX);
    float value = kFloat_1_Score;
    AsGeneSurface(self)->points = 0;
    AsGeneSurface(self)->indexCount = 0;
    AsGeneSurface(self)->indexVtable = __vt__10CGeneTex3D;
    AsGeneSurface(self)->indexBuffer = 0;
    AsGeneSurface(self)->pointCount = 0;
    AsGeneSurface(self)->layerCount = 0;
    AsGeneSurface(self)->scale = value;
    void* result = GENETEX3D_NEW_ARRAY Entry801ED2C0[3];
    AsGeneSurface(self)->layers = result;
    return self;
}

#pragma push
#pragma use_lmw_stmw on
// Release generated surface layer resources, vertices, indices, and optionally the surface object.
void* GeneratedSurface_Destroy(void* self, int deallocate) {
    if (self != 0) {
        int offset;
        int i;
        i = 0;
        offset = 0;
        while (i < AsGeneSurface(self)->layerCount) {
            if (g_resourceManager != 0) {
                void* resource =
                    *(void**)&AsGeneratedSurfaceLayerView((char*)AsGeneSurface(self)->layers + offset)->bitmapResource;
                if (resource != 0) {
                    ResourceManager_ReleaseId(g_resourceManager, resource);
                    *(void**)&AsGeneratedSurfaceLayerView((char*)AsGeneSurface(self)->layers + offset)->bitmapResource =
                        0;
                }
            }
            offset += 0x14;
            ++i;
        }
        void* array = AsGeneSurface(self)->layers;
        if (array != 0) {
            __destroy_new_array(array, (void (*)(void*, int))GeneratedSurfaceLayer_Destroy);
            AsGeneSurface(self)->layers = 0;
        }
        void* points = AsGeneSurface(self)->points;
        if (points != 0) {
            if (points != 0) {
                __dla__FPv((char*)points - 0x10);
            }
            AsGeneSurface(self)->points = 0;
        }
        if (((char*)&AsGeneSurface(self)->indexCount) != 0) {
            void* indexBuffer = (*(void**)&AsGeneSurface(self)->indexBuffer);
            AsGeneSurface(self)->indexVtable = __vt__10CGeneTex3D;
            if (indexBuffer != 0) {
                __dla__FPv(indexBuffer);
                (*(void**)&AsGeneSurface(self)->indexBuffer) = 0;
            }
        }
        if (deallocate > 0)
            __dl__FPv(self);
    }
    return self;
}
#pragma pop

} // extern "C"

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU804621B0", _ctors_u801EC334_0, CScore_RegisterFactory);
#pragma pop

// Verify the partial views against the offsets used by this unit.
typedef char ScorePlayerFields_strokes_offset[((int)&((ScorePlayerFields*)0)->strokes == 0x28) ? 1 : -1];
typedef char ScorePlayerFields_holeResults_offset[((int)&((ScorePlayerFields*)0)->holeResults == 0x3a) ? 1 : -1];
typedef char ScorePlayerFields_holePang_offset[((int)&((ScorePlayerFields*)0)->holePang == 0x4c) ? 1 : -1];
typedef char ScorePlayerFields_scoreToPar_offset[((int)&((ScorePlayerFields*)0)->scoreToPar == 0xc8) ? 1 : -1];
typedef char ScorePlayerFields_earnedPang_offset[((int)&((ScorePlayerFields*)0)->earnedPang == 0xcc) ? 1 : -1];
typedef char ScorePlayerFields_partyValue_offset[((int)&((ScorePlayerFields*)0)->partyValue == 0x178) ? 1 : -1];
typedef char
    ScorePlayerFields_partyHolePoints_offset[((int)&((ScorePlayerFields*)0)->partyHolePoints == 0x17c) ? 1 : -1];
typedef char ScoreFormTotals_scoreToPar_offset[((int)&((ScoreFormTotals*)0)->scoreToPar == 0x94) ? 1 : -1];
typedef char ScoreFormTotals_earnedPang_offset[((int)&((ScoreFormTotals*)0)->earnedPang == 0xa4) ? 1 : -1];
typedef char ScoreMessage_argument_offset[((int)&((ScoreMessage*)0)->argument == 0x8) ? 1 : -1];
typedef char GeneSurfaceFields_points_offset[((int)&((GeneSurfaceFields*)0)->points == 0x18) ? 1 : -1];
typedef char GeneSurfaceFields_pointCount_offset[((int)&((GeneSurfaceFields*)0)->pointCount == 0x1c) ? 1 : -1];
typedef char GeneSurfaceFields_indexCount_offset[((int)&((GeneSurfaceFields*)0)->indexCount == 0x24) ? 1 : -1];
typedef char GeneSurfaceFields_layerCount_offset[((int)&((GeneSurfaceFields*)0)->layerCount == 0x30) ? 1 : -1];

// Golf rules (CGolfRule): round flow, player selection and their snapshots, hole reset,
// and the simulation of each shot with its shot statistics.

#include <decomp/static_init.h>
#include <decomp/local_class.h>
#include <decomp/ct_member.h>
// Golf round rule: input, launch simulation, playback, scoring and round flow.
// Search the following section banners; address/function order is intentional.
// S1 - Type anchor and player thunks
// S2 - Rule lifecycle and player snapshots
// S3 - State dispatcher and round continuation
// S4 - Shot-history container internals
// S5 - Round flow, ranking and hole results
// S6 - Player preview and shot animations
// S7 - Rule message receiver
// S8 - State entries and button launch copy
// S9 - Rewards and result presentation
// S10 - Ball playback and shot outcomes
// S11 - Aim, swing and motion camera input
// S12 - Shot simulation and launch model
// S13 - Directional commands, gauges and camera input
// S14 - Shot statistics and turn preparation
// S15 - Aim reset, shot start and undo
// S16 - Camera paths, player order and replay
// S17 - Gauge commit, reset and mode exit
// S18 - BGM playlist and sequence preloads
// S19 - Turn advancement, assistance and result menus
// Gauge knobs: UpdatePowerAndImpactGauge and its relationship table.
// Ranking/tie policy: S5 UpdatePlayerRanksAndHoleOrder; R20_SCORE in P2 explains
// the two rank nibbles and the independent per-hole order bytes.
// Result progression: S5 ProcessHoleResults / PlayerResultPhase; score 0x11
// replies have different admission/effects with course-record updates enabled.
// Reward edits: P2 BD74Tables maps indices to destinations/caps; currency labels
// for account +40/+44 remain unverified, so consult their consumers before a mod.
// History allocation/alias rules: S4 section invariant and InsertIteratorRange.
// Launch edits require BOTH BuildShotSimulation and EnterState's button entry.
// Integration lives in course_collision_and_ball_physics.cpp.
// Float/string storage: the data section; pool entries can have several consumers.
// Shared borrowed model: game/golf_shot_model.h and the class headers.
// Lazy singleton macros and pragma blocks retain CodeWarrior's emitted code shape.
// Prototypes retained in P2 include duplicates that affect local jump-table names.

extern "C" char s_actorRenderScaleTween[];
extern "C" char s_effectFadeDtorRegistration_GolfRule[];
extern "C" char s_fadeOverlayRegistration_804F72D8[];
extern "C" char s_nowReloadingDtorRegistration_8014B210[];
struct Fn8016C2B8List;
extern "C" Fn8016C2B8List s_shotEffectPreloadTasks;
#include "classes/CRival.h"
#include "classes/FadeState.h"
#include "fp_fused.h"
#define AsCRival(p) ((CRival*)(p))
#include "classes/CPadSwingCtrl.h"
#define AsCPadSwingCtrl(p) ((CPadSwingCtrl*)(p))

// Circular word storage used by the shot-history container helpers.
struct RingWordStorage {
    unsigned int* data;      // 0x00
    unsigned int head;       // 0x04
    unsigned int count;      // 0x08
    unsigned int capacity;   // 0x0C, includes one reserved empty slot
    unsigned int allocation; // 0x10, outer map/block allocation word
};
typedef char RingWordStorageSize[(sizeof(RingWordStorage) == 0x14) ? 1 : -1];

// Shot setup runs on fixed 30 Hz ticks; gauge values are screen units.
// Golf-rule states are bit values; combinations also appear in input/camera masks.
enum GolfRuleState {
    kStateHoleCameraPath = 0x1,
    kStatePlayerPreview = 0x2,
    kStateTurnStartDelay = 0x4,
    kStateAim = 0x8,
    kStateBeginButtonSwing = 0x10,
    kStateButtonSwing = 0x20,
    kStateButtonShotAnimation = 0x40,
    kStateBallFlight = 0x80,
    kStateCameraView = 0x100,
    kStateCameraTransition = 0x200,
    kStateUnresolved400 = 0x400, // no additional action in EnterState; wider role unresolved
    kStatePrepareNextShot = 0x800,
    kStateDelayedTransition = 0x1000,
    kStateHoleResults = 0x2000,
    kStateBeginPlayerTurn = 0x4000,
    kStateCameraAwaitInput = 0x8000,
    kStateRestoreSavedShot = 0x10000,  // restores saved position/wind, then enters aim
    kStateStartTurnWithBgm = 0x100000, // loads turn audio before shot setup
    kStatePostRoundMenus = 0x20000,
    kStateCameraReturn = 0x40000,
    kStateFightResult = 0x80000,
    kStateHoleOverview = 0x200000,
    kStateCourseReload = 0x800000,
    kStateStorySequence = 0x1000000,
    kStateTourExit = 0x2000000,
    kStateRoundResults = 0x4000000,
    kStateMotionSwing = 0x8000000,
    kStateWaitPlayerAndCaddie = 0x10000000,
    kStateMotionShotAnimation = 0x20000000,
    kStateWaitHoleResources = 0x40000000,
    kStateMotionCameraInput = (int)0x80000000,
};

// OnProcess policies, expressed as state sets so adding a state is a local edit.
enum GolfProcessStateSet {
    kClubSelectionUpdateStates = kStateAim | kStateCameraView | kStateCameraTransition | kStateCameraAwaitInput |
        kStateCameraReturn | kStateMotionSwing,                             // 0x08048308
    kDebugFastForwardStates = kStateButtonShotAnimation | kStateBallFlight, // 0xC0
    kKeepCameraScaleStates = kStateButtonShotAnimation | kStateBallFlight |
        kStatePrepareNextShot // 0x8C0; other states reset scale under the task-status gate
};
enum GolfFastPlaybackTiming {
    kFastPlaybackMinimumRemainingSamples = 90, // strict >3 seconds at 30 Hz
    kFastPlaybackFadeStartSample = 15,         // half a second of sampled ball history
    kFastPlaybackFadeOutTicks = 60             // FadeState_StartFadeOut's tick argument
};
enum GolfClubKind {
    kClubKindWood = 0,
    kClubKindIron = 1,
    kClubKindWedge = 2,
    kClubKindPutter = 3
};

// IDs are receiver-specific. A float sent with 0x2717 is copied synchronously;
// the screen handler interprets it as ring height, even when the sender computed
// a heading. Do not assume every recipient assigns the same meaning to that ID.
enum GolfAimMessage {
    kAimFloatBroadcastMessage = 0x2717,
    kScreenQueryHudPhaseMessage = 0x30,
    kScreenQueryPanelsReadyMessage = 0x40,
    kScreenSetPowerColorMessage = 9,
    kPlayerQueryHeadingMessage = 3,
    kPlayerQueryShotKeySecondsMessage = 0x20,
    kTutorialQueryGateMessage = 7
};
// Playback event indices count samples at the 30 Hz step.
// These are indices into m_eventPreDataIndex, not flags or elapsed seconds.
// Except playback cursor 0, events are latched once at a sample index; -1 is unset.
// Special-shot names: EnterState creates Tomahawk/Cobra/Spike effects for
// flags 10/20/40; CFx::CreateCobra (fx_shot_effects_cobra_tomahawk.cpp:378) constructs CFxCobra, whose
// update tests flight flag 0x20. Slot 10 is the corresponding distance latch.
enum BallSimulationEventSlot {
    kBallPlaybackCursor = 0,
    kBallFirstDescendingSample = 2,
    kBallCobraDistanceSample = 10,
    kBallTomahawkEndSample = 11,
    kBallSpikeMotionSample = 12,
    kBallSpikeDescentSample = 13,
    kBallSpikeEndSample = 14,
    kSpikeDescentDelaySamples = 60 // At 30 samples/second, delay is two seconds.
};

// Shot flags: effect-creator evidence is recorded above.
enum ShotCommandFlags {
    kSpecialShotMask = 0x70,
    kShotCurveNegative = 4,
    kShotCurvePositive = 8,
    kShotTomahawk = 0x10,
    kShotCobra = 0x20,
    kShotSpike = 0x40
};
// Fixed 30 Hz sample counts: 1800 = 60 seconds, 900 = 30 seconds.
enum ShotSimulationStopLimit {
    kShotMaximumSamples = 1800,
    kShotStallCheckSamples = 900,
    kShotMaximumCollisionEvents = 10
};

#include "classes/CCameraWork.h"
#include "classes/CPangyaCommonDialog.h"
#include "classes/CGolfPlayer.h"
#include "classes/CFx.h"
#include "classes/CComThinkCtrl.h"
#include "classes/CAllCourse.h"
#include "classes/CPreloadSeqAsync.h"
#include "classes/CPreloadSeqSync.h"
#include "classes/CNowReloading.h"
#include "classes/CResultFightMenu.h"
#include "classes/CComRandomUse.h"
#include "classes/CComStrokeUse.h"
#include "classes/CB2dPlayerHandle.h"
#include "classes/CClub.h"
#include "classes/CEffectFade.h"
#include "classes/CBpItemVector.h"
#include "classes/CTask.h"
#include "classes/CGolfTask.h"
#include "classes/CShadowManager.h"
#include "classes/CPreloadSprSync.h"
#include "classes/CRetryMenu.h"
#define AsCCameraWork(p) ((CCameraWork*)(p))
// These are logical GameInputManager action IDs, not raw WPAD button bits.
enum GolfInputAction {
    kActionSkipFlight = 4,
    kActionSwing = 12,
    kActionAdvance = 0x20,
    kActionCancel = 0x23,
    kActionRequestPresentationBit4 = 0x21, // OnProcess ORs bit 4; wider action meaning unresolved.
    kSelectedInputChannel = -1,            // Negative player selects GameInputManager::selectedPlayer.
    kSwingHitRumbleFrames = 5
};

// CPadSwingCtrl update results consumed by the Wii Remote swing path.
enum MotionSwingResult {
    kMotionIdle = 0,
    kMotionStarted = 1,
    kMotionPowerDecided = 2,
    kMotionAlternatePowerDecided = 3,
    kMotionHit = 4,
    kMotionCancelled = 5,
    kMotionPreviewHit = 6,
    kMotionFinished = 7,
    kMotionAutoShot = 8
};

enum ShotPhase {
    kShotIdle = 0,
    kShotSelectPower = 1,
    kShotSelectImpact = 2,
    kShotImpactDecided = 3
};

enum ShotGaugeDirection {
    kGaugeReturning = -1,
    kGaugeStopped = 0,
    kGaugeAdvancing = 1
};

enum ShotImpactGrade {
    kImpactUnclassified = 0,
    kImpactMiss = 1,
    kImpactPoor = 2,
    kImpactGood = 3,
    kImpactPangya = 4
};

// Names and byte offsets follow the documented GolfRuleStringPool layout.
// Keep byte-pointer expressions: a struct view changes CodeWarrior code generation.
// Byte offsets for message helpers that accept a string-pool offset.
enum GolfRuleTextOffset {
    kText_golfRule = 0x0,
    kText_bgmDirectoryFormat = 0x9,
    kText_menuSoundBank = 0x16,
    kText_golfSoundBank = 0x1E,
    kText_computerThinkPrefix = 0x26,
    kText_ecbExtension = 0x31,
    kText_com_random_tbl_shot_ecb = 0x36,
    kText_com_random_tbl_pat_ecb = 0x4E,
    kText_com_random_tbl_approach_ecb = 0x65,
    kText_com_random_tbl_special_ecb = 0x81,
    kText_computerStrokePrefix = 0x9C,
    kText_screen = 0xA8,
    kText_letter = 0xAF,
    kText_spot = 0xB6,
    kText_storyConte = 0xBB,
    kText_cTourTask = 0xC6,
    kText_tourMain = 0xD0,
    kText_cSaveProcessTask = 0xD9,
    kText_ambientSound = 0xEB,
    kText_leftControlKey = 0xF8,
    kText_replayKey = 0x101,
    kText_item = 0x108,
    kText_playerInfoKey = 0x10D,
    kText_rightControlKey = 0x119,
    kText_hideKey = 0x122,
    kText_enterKey = 0x127,
    kText_spaceBarKey = 0x12D,
    kText_leftAltKey = 0x13C,
    kText_rightAltKey = 0x141,
    kText_skipKey = 0x146,
    kText_fastForwardKey = 0x14B,
    kText_clock = 0x157,
    kText_play_my_turn = 0x15D,
    kText_camera = 0x16A,
    kText_pangyaNotice = 0x171,
    kText_number9Key = 0x179,
    kText_pad9Key = 0x17F,
    kText_playerActorFormat = 0x185,
    kText_clubActor = 0x18F,
    kText_caddieActor = 0x196,
    kText_score = 0x1A1,
    kText_player = 0x1A7,
    kText_caddie = 0x1AE,
    kText_effect = 0x1B5,
    kText_balloonSystem = 0x1BC,
    kText_tutorial = 0x1CA,
    kText_grid = 0x1D3,
    kText_basicSequence = 0x1D8,
    kText_ballSequence = 0x1DD,
    kText_shot_dist_count = 0x1E4,
    kText_cPartyTask = 0x1F4,
    kText_partyMain = 0x1FF,
    kText_cTrainingTask = 0x209,
    kText_cFightTask = 0x217,
    kText_fightMain = 0x222,
    kText_cSingleTaskRev = 0x22C,
    kText_singleMainRev = 0x23B,
    kText_saveProcessMain = 0x249,
    kText_golfBg = 0x259,
    kText_dartsSystem = 0x260,
    kText_driveSystem = 0x26C,
    kText_ball = 0x278,
    kText_voice = 0x27D,
    kText_doPlayerPreview = 0x283,
    kText_woodPowerShotSequenceFormat = 0x293,
    kText_cupin_underpar_wav = 0x2AE,
    kText_cupin_overpar_wav = 0x2C1,
    kText_scoreboard_wav = 0x2D3,
    kText_trajectory = 0x2E2,
    kText_play_unplayable = 0x2ED,
    kText_pM_NET_GAMESYNC_START = 0x2FD,
    kText_rainSequence = 0x313,
    kText_lightningPrepareFormat = 0x316,
    kText_npcActor = 0x321,
    kText_entranceAnimation = 0x325,
    kText_chanceSequence = 0x334,
    kText_spin = 0x33D,
    kText_timeSoundAlias = 0x342,
    kText_powershot_ball_effect = 0x349,
    kText_computerCameraDebugFormat = 0x35F,
    kText_wb_fly = 0x37F,
    kText_ballEffectStage1 = 0x386,
    kText_ballEffectStage2 = 0x396,
    kText_ballEffectStage3 = 0x3A6,
    kText_cometSequence = 0x3B6,
    kText_ballCometSequence = 0x3BB,
    kText_stardustSequence = 0x3C9,
    kText_ballStardustSequence = 0x3CE,
    kText_gallery = 0x3DC,
    kText_windSequence = 0x3E4,
    kText_ballWindSequence = 0x3E7,
    kText_powerShotEffectSequence = 0x3F3,
    kText_timeOverSequence = 0x40D,
    kText_guideLine = 0x41C,
    kText_timeOverBunkerObDisappointmentPose = 0x426,
    kText_bunkerSequence = 0x449,
    kText_awardCamera = 0x452,
    kText_individualAwardSequence = 0x45F,
    kText_story_lose_wav = 0x46B,
    kText_albatrossVictoryPose = 0x47A,
    kText_holeInOneSequence = 0x491,
    kText_albatrossSequence = 0x4A0,
    kText_holeInOneVictoryPose = 0x4AD,
    kText_hole_in = 0x4C6,
    kText_eagleSequence = 0x4CE,
    kText_eagleBirdieVictoryPose = 0x4D3,
    kText_birdieSequence = 0x4EC,
    kText_saveParVictoryPose = 0x4F7,
    kText_parSequence = 0x50C,
    kText_bogeyDisqualifiedDisappointmentPose = 0x511,
    kText_disqualifiedSequence = 0x524,
    kText_bogeySequence = 0x529,
    kText_doubleBogeySequence = 0x52E,
    kText_tripleBogeySequence = 0x53B,
    kText_overParScoreFormat = 0x548,
    kText_scoreUnit = 0x54C,
    kText_thirdPlaceAnimation = 0x54F,
    kText_lose = 0x55D,
    kText_secondPlaceAnimation = 0x562,
    kText_firstPlaceAnimation = 0x570,
    kText_draw = 0x57E,
    kText_win = 0x583,
    kText_hole_win_wav = 0x587,
    kText_hole_lose_wav = 0x594,
    kText_cameraResourceTag = 0x5A2,
    kText_stringFormat = 0x5A7,
    kText_party_darts_hit = 0x5AA,
    kText_spin_super = 0x5BA,
    kText_hell_ob_seq = 0x5C5,
    kText_puttSuccessSequence = 0x5D1,
    kText_birdieExhibitionFormat = 0x5E2,
    kText_parExhibitionFormat = 0x5F7,
    kText_overParExhibitionFormat = 0x606,
    kText_birdie = 0x61D,
    kText_par = 0x624,
    kText_bogey = 0x628,
    kText_puttFailureSequence = 0x62E,
    kText_afterPuttPose = 0x63F,
    kText_waterHazardSequence = 0x658,
    kText_ball_fall_into_water = 0x66B,
    kText_outOfBoundsSequence = 0x680,
    kText_ball_OB_area = 0x683,
    kText_swing_power_decide = 0x690,
    kText_hitShot = 0x6A3,
    kText_cTitleTask = 0x6AB,
    kText_titleMain2 = 0x6B6,
    kText_skip_skip_skip_wav = 0x6C1,
    kText_so_nice_wav = 0x6D4,
    kText_fun_time_wav = 0x6E0,
    kText_daydream_wav = 0x6ED,
    kText_breeze_wav = 0x6FA,
    kText_frog_wav = 0x705,
    kText_spring_wav = 0x70E,
    kText_twilight_mood_wav = 0x719,
    kText_autumn_colors_wav = 0x72B,
    kText_bunny_picnic_wav = 0x73D,
    kText_magic_wand_wav = 0x74E,
    kText_a_shiny_day_wav = 0x75D,
    kText_ruffles_wav = 0x76D,
    kText_southern_cross_wav = 0x779,
    kText_navy_blue_memory_wav = 0x78C,
    kText_rising_sun_wav = 0x7A1,
    kText_stratosphere_wav = 0x7B0,
    kText_distant_bells_wav = 0x7C1,
    kText_snowscape_wav = 0x7D3,
    kText_winter_ride_wav = 0x7E1,
    kText_somewhere_wav = 0x7F1,
    kText_nowhere_wav = 0x7FF,
    kText_journey_for_treasure_wav = 0x80B,
    kText_crystal_waver_wav = 0x824,
    kText_happy_flight_wav = 0x836,
    kText_polaris_wav = 0x847,
    kText_sakura_front_wav = 0x853,
    kText_sweet_blossom_wav = 0x864,
    kText_dive_into_volcano_wav = 0x876,
    kText_vermilion_sunset_wav = 0x88C,
    kText_crystal_lake_wav = 0x8A1,
    kText_fade_into_white_wav = 0x8B2,
    kText_tutorial_wav = 0x8C6,
    kText_vector_length_error = 0x8D3,
    kText_lengthErrorTypeName = 0x8E7,
    kText_seq = 0x91E,
    kText_spr = 0x923,
    kText_story_win_wav = 0x928,
    kText_deque_length_error = 0x936,
    kText_cdeque_length_error = 0x94B,
    kText_basic_string_reserve_length_error = 0x95F,
    kText_basic_string_out_of_range = 0x982,
    kText_outOfRangeTypeName = 0x99D,
    kText_basic_string_length_error = 0x9D4,
    kText_cGolfRule = 0x9EF,
    kText_emptyTaskArgument = 0xEA
};

#include "iff/IffStruct.h"
#include "classes/CMouseCursor.h"
#define AsCMouseCursor(p) ((CMouseCursor*)(p))
// g_mouseCursor is the cursor passed to CMouseCursor::Reset below.
#include "game/golfball.h"
#include "classes/CWind.h"
#define AsCGolfBall(p) ((CGolfBall*)(p))
// Ball fields retain the offsets documented in game/golfball.h.
#include "classes/CQuadTree.h"
#define AsCQuadTree(p) ((CQuadTree*)(p))
#include "classes/CSmallCommonDialog.h"
#define AsCSmallCommonDialog(p) ((CSmallCommonDialog*)(p))
// Field offsets are checked in the class headers; original function addresses stay below.
#include "classes/CGolfDoc.h"
#define AsCGolfDoc(p) ((CGolfDoc*)(p))
#include "classes/CGolfRule.h"
#include "game/golf_shot_model.h"
#define AsGolfRuleFrame(p) ((GolfRuleFrameFields*)(p))
#define AsCGolfRule(p) ((CGolfRule*)(p))
#include "classes/CSharedDoc.h"
#define AsCSharedDoc(p) ((CSharedDoc*)(p))

extern "C" void CharacterPreviewTable_dtor(void*, int);
#include "game/vector.h"
#include "game/wvector.h"
extern "C" void __construct_array(void*, void*, void*, unsigned int, unsigned int);
#define GAME_NSTRING_TEXT_CONSTRUCTOR
#include "game/string.h"

#include "game/singletons.h"
#include "sdk_protos.h"
#include "game/types.h"
#include "classes/GolfDocSnapshot8013.h"
#define AsGolfDocSnapshot(p) ((GolfDocSnapshot8013*)(p))
#include "classes/CSceneManager.h"
#define AsCSceneManager(p) ((CSceneManager*)(p))

enum PlayerRoundResultWord {
    kRoundScoreWord = 0,
    kRoundPangWord = 1,
    kRoundBonusPangWord = 2
};

enum HoleInCryptByte {
    kHoleInEncodedWord = 0,
    kHoleInKeyWord = 8
};

// Receiver-local message ABI: HandleMsg reads +4 and up to three 32-bit words.
// Pointer payloads are Wii addresses borrowed for this synchronous dispatch.
union GolfRuleMessageArgument {
    int word;
    unsigned int unsignedWord;
    unsigned char* bytes;
};
struct GolfRuleMessage {
    int unk_000;
    int id;
    GolfRuleMessageArgument arg0, arg1, arg2;
};
typedef char GolfRuleMessageIdOffset[(int)&((GolfRuleMessage*)0)->id == 4 ? 1 : -1];
typedef char GolfRuleMessageArg0Offset[(int)&((GolfRuleMessage*)0)->arg0 == 8 ? 1 : -1];
typedef char GolfRuleMessageArg1Offset[(int)&((GolfRuleMessage*)0)->arg1 == 0xC ? 1 : -1];
typedef char GolfRuleMessageArg2Offset[(int)&((GolfRuleMessage*)0)->arg2 == 0x10 ? 1 : -1];
typedef char GolfRuleMessageSize[sizeof(GolfRuleMessage) == 0x14 ? 1 : -1];

// Selected-club +A domain, distinct from powerShotMode at selection table +44.
// golf_player_animation_and_model_loads.cpp's reaction branches identify wood/iron/putter; IFF sClub (IffStruct.h)
// lists wood/iron/wedge/putter. These are categories, not individual club numbers.
// Club and ground categories/impact grades are used below.
enum ClubKind {
    kClubWood = 0,
    kClubIron = 1,
    kClubWedge = 2,
    kClubPutter = 3
};
enum BallMotionState {
    kBallStopped = 0,
    kBallAirborne = 1,
    kBallRolling = 3
};
// Bit 2 starts the special-shot camera/notice sequence; presentationStep (+494)
// is its phase. Bit 4 is also classified from hole-in distance and set by action
// 0x21; no single broader intent is established, so retain a neutral bit name.
enum ShotPresentationPolicy {
    kSpecialShotPresentation = 2,
    kPresentationBit4 = 4
};
enum FastPlaybackPolicy {
    kFastPlaybackFirstSample = 15,
    kFastPlaybackTailSamples = 90
};
struct GolfTaskActivityFlags {
    char pad_000[0x3A4];
    unsigned char sceneHiddenForReload;
    signed char holeResultActive;
};
#define LOCAL_FIELD_OFF(t, f, o) typedef char t##_##f[((int)&((t*)0)->f == (o)) ? 1 : -1]
LOCAL_FIELD_OFF(RuleShotHistories, players, 0xAC);
LOCAL_FIELD_OFF(SharedComputerRoundFields, profileName, 0x9E4);
LOCAL_FIELD_OFF(SharedComputerRoundFields, strokeScores, 0xB70);
LOCAL_FIELD_OFF(BallGroundContact, triangle, 0x28);
LOCAL_FIELD_OFF(QuadTreeRecoveryPoint, position, 0x2084);
LOCAL_FIELD_OFF(AimRecoveryPoint, position, 0xC);
LOCAL_FIELD_OFF(CourseCameraAnchor, position, 0x44);
LOCAL_FIELD_OFF(PlayerSwingOptions, buttonSwingSelected, 0x1DC);
LOCAL_FIELD_OFF(CameraCaddieOption, caddieCameraEnabled, 0x1A2);
LOCAL_FIELD_OFF(GolfTaskActivityFlags, sceneHiddenForReload, 0x3A4);
LOCAL_FIELD_OFF(GolfTaskActivityFlags, holeResultActive, 0x3A5);

// Circular iterators use addresses as integer words to preserve signed deltas.
struct RingIteratorAddressWords {
    unsigned int current;
    unsigned int begin;
    unsigned int unused;
    unsigned int end;
};

// These entries share a contiguous static-data region with the first destructor
// record. Retain the base symbol and addends used by the original relocations.
enum GolfRuleStaticOffset {
    kEffectFadeDestructorRecord = 0x18,
    kReloadingDestructorRecord = 0x24,
    kFadeOverlayDestructorRecord = 0x30,
    kTurnTween = 0x3C,
    kPreloadListDestructorRecord = 0x58,
    kPreloadList = 0x64
};

// Placement delete has no owned allocation to release.
inline void operator delete(void*, void*) {}

// Shared document accessors retain the pointer-slot form used by the matched code.
// These views are borrowed and must be observed at every original call site.
#define SharedDocBytes() ((unsigned char*)g_sharedDoc)
#define GolfDocBytes() ((*(unsigned char**)&AsCSharedDoc(g_sharedDoc)->golfDoc))

// ===== P2 - Shared ABI declarations and retained legacy helpers =====
// Shared one-off views remain here when relocation or emission order requires it.
// Entry points for infrastructure edits:
//   directional command queue -> INSERT63 / Fn63Guard (node ownership and splice);
//   history block-map growth -> ShotHistory_InsertIteratorRange in S4;
//   round awards -> RoundRewardAccountView / BD74Tables and UpdateRoundRewards;
//   sampled swing factor / animation settling -> ShotGauge54 ABI table below.
// These are borrowed overlays, not additional objects. The outer history stores
// 0x54-byte checkpoints; the inner circular map stores 4-byte block pointers.
// Do not interchange their counts, capacities or iterator element sizes.
// ---- merged from golf_rule_round_flow_and_shot_sim.cpp ----
namespace ns0 {

#include "MSL/vector.h"
struct E8014B5E0;
/* size reset form 3 of the vector destructor */
namespace std {
template <> struct __vec_destroys_elements<E8014B5E0*> {
    enum {
        value = 3
    };
    static void destroy(E8014B5E0**) {}
};
}
typedef std::vector<E8014B5E0*, app::app_allocator<E8014B5E0*> > V8014B5E0;
extern "C" {

void __ct__18NativeBase80201D18Fv(void*);
void* ItemInventory_GetItemIdAt(void*, int);
void Camera_SetFov(void*, float);
void Camera_SetClip(void*, float, float, int);
void Matrix_SetIdentityAffine(void*);
void __ct__12CRandomHouseFv(void*);
void __ct__14CDrawDriveLineFv(void*);
void FileIO_GetDefaultDevicePath(unsigned int, void*);
void SoundManager_ReleaseSoundBank(void*, const char*);
void SoundManager_AcquireSoundBank(void*, const char*);
void* memset(void*, int, unsigned int);
void RingBuffer32_dtor(void*, int);
void appFreeUni__FPv(void*);
void __dl__FPv(void*);
void* __nw__FUl(unsigned int);
unsigned char SharedDoc_GetCourseId(void*);
void DriveLineMarkers_Generate(void*, unsigned char);
void* CQuadTree_GetInstance(void);
void* GolfBall__Fv(void);
void __pl__FRC7WVectorRC7WVector(void*, void*, void*);
void* GolfPlayer_GetEntryByIndex(void*, int);
int SharedDoc_IsPlayerFlag0Set(int);
void IActor_DispatchMessage(void*, void*, const char*, int, void*, void*, void*, void*);
void DrawScene_InvokePrimaryHook(void*);
void __destroy_arr(void*, void*, int, int);
void __dt__18NativeBase80201D18Fv(void*, int);
void* RuleShotHistory_dtor(void*, int);
void SharedDoc_InvalidatePlayerState(void*);
void String_ReserveCapacity(void*, unsigned int);
struct BAAlloc;
void String_ReplaceRange(void*, int, int, const char*, const char*, const char*);
void Generate_ComputerStrokeScores(void*, void*);
void MT19937_Seed(void*, void*);
void ShadowManager_SetResourceLookupEnabled(void*, int);

int SharedDoc_IsMode6PartyMode0(void);
void SharedDoc_CopyPlayerSnapshot(void*, int);
void SaveData_Ctor(const char*);
void* SaveData_GetCurrentRecord(void*);
void SaveData_InitPlayerRecord(void*);

void PartyItemRecord_InitDefaults(void*);
void PlayerRenderRecord_dtor(void*);
void RenderContext_ResetPlayerRecords(void*);
void __dt__13RenderContextFv(void*);
void* PartyItemSlots_GetInstance(void);
void PartyItemSlots_RefreshDisplay(void*);
extern signed char s_saveDataInitialized;
extern signed char s_renderContextInitialized;
extern char s_renderContext[];
extern char s_saveData[];
extern char s_roundPlayerList[];
extern char s_renderContextDtorRegistration_804F72B4[];
extern void* g_sharedDoc;
extern void* g_sceneManager;
extern const float kFloat_0_GolfRule;
extern const float kGolfCameraFovRadians;
extern const float kFloat_3_0;
extern const float kFloat_100000_GolfRule;
extern const float kFloat_1_805DEA38;
extern void* g_mouseCursor;
extern void* g_rival;
extern void* g_randomHouse;
extern void* g_currentTimeMilliseconds;
extern void* g_cameraWork;
extern void* g_shadowManager;
extern char g_courseLoadFailed;
extern void* g_view;
extern void* g_soundManager;
extern char s_cameraTransform[0x10];
extern const char golfRuleStrings[0x40];
extern char __vt__9CGolfRule[0x4C];

struct AllocatedObject {
    char data[0x4F8];
    AllocatedObject();
};

// Construct the allocated object through the existing game constructor.
AllocatedObject::AllocatedObject() {
    ((CGolfRule*)this)->Init();
}

// Allocate and construct the golf-rule actor.
void* CGolfRule_Create(void);

// Initialize the six-word shot-history header to an empty state.
void Struct24_Clear(void* self);

// Initialize rule-owned resources, per-player history and shot/network state.

struct Obj300 {
    char x[0x300];
    Obj300();
};
struct ObjB0 {
    char x[0xb0];
    ObjB0();
};
struct Obj1C {
    char x[0x1c];
    Obj1C();
};
struct Obj2C {
    char x[0x2c];
    Obj2C();
};
// Construct the camera controller in the allocated object's storage.
Obj300::Obj300() {
    AsCCameraWork(this)->Init();
}
// Construct the allocated rival through its initialiser.
ObjB0::ObjB0() {
    ((CRival*)this)->Init();
}
// Construct the allocated object through the existing game constructor.
Obj1C::Obj1C() {
    __ct__12CRandomHouseFv(this);
}
// Construct the allocated object through the existing game constructor.
Obj2C::Obj2C() {
    __ct__14CDrawDriveLineFv(this);
}
// Reset round state and create camera, rival and swing-related services before loading.

// Destroy the owned container storage and optionally free the object.
void* RuleShotHistory_dtor(void* self, int deleting);

// Destroy the owned container storage and optionally free the object.
void* dtor_8014B478(void* self, int deleting);

// Destroy the owned container storage and optionally free the object.
void* dtor_8014B5E0(void* self, int deleting);

// Destroy the owned container storage and optionally free the object.
void* dtor_8014B4F4(void* self, int deleting);

#include "MSL/vector.h"
#include <decomp/cw_bitfields.h>
// CGolfRule +0x4D4 childObjects: vector of child task pointers.
typedef std::vector<void*, app::app_allocator<void*> > GolfRuleChildVector;
// CGolfRule +0x394 cameraPathPoints: a vector of non-scalar records; the element type is not known yet
// (GolfRulePathPoint is a placeholder: the destructor does not depend on its layout).
struct GolfRulePathPoint {
    int words[3];
};
typedef std::vector<GolfRulePathPoint, app::app_allocator<GolfRulePathPoint> > GolfRulePathVector;
// CGolfRule +0x42C bgmNames: a vector of strings; dropping an element runs the string destructor.
namespace std {
template <> struct __vec_destroys_elements<NString> {
    enum {
        value = 1
    };
    // NString's destructor, inline here (elsewhere this unit calls it out of line).
    static void destroy(NString* s) {
        if (s != 0)
            if (s->flags >> 31)
                appFreeUni__FPv(s->data);
    }
};
}
typedef std::vector<NString, app::app_allocator<NString> > GolfRuleNameVector;

struct VBase {
    char pad[0x58];
    virtual void destroy(int);
};
struct ChildObj {
    virtual void m0();
    virtual void m1();
    virtual void destroy(int);
};
struct SelfObj {
    virtual void v0();
    virtual void v1();
    virtual void v2();
    virtual void v3();
    virtual void v4();
    virtual void v5();
    virtual void v6();
    virtual void destroy2();
};
// Start course-related loads and construct child preload tasks.

struct NewObj {
    char data[0x5c];
    NewObj();
};
// Construct the allocated object through the existing game constructor.
NewObj::NewObj() {
    ((CPadSwingCtrl*)this)->__ct();
}
// Attach the loaded course and create the pad swing controller.

#define MGR_OF_LBL() ((*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc))
// Reset the ball, ground lookup and wind before selecting the next hole state.

struct BAAlloc {
    char c __attribute__((aligned(4)));
};

struct BAStr {
    unsigned int hdr;
    unsigned int len;
    char* ptr;
    // Initialize an empty inline narrow string.
    BAStr() : hdr(0), len(0), ptr(0) {}
    // Free heap string storage only when the top ownership bit is set.
    ~BAStr() {
        if (hdr >> 31)
            appFreeUni__FPv(ptr);
    }
    // Return string length (legacy method name index does not denote a position).
    unsigned int index() const {
        return (hdr >> 31) == 0 ? (*(const unsigned char*)this & 0x7f) : len;
    }
    // Borrow inline characters or the flagged heap string.
    const char* cstr() const {
        return (hdr >> 31) == 0 ? (const char*)this + 1 : ptr;
    }
};

// Insert a counted byte range into the temporary string.
inline void BAInsN(BAStr& s, unsigned int idx, const char* k, unsigned int n) {
    BAAlloc extra;
    BAAlloc copy = extra;
    String_ReplaceRange(&s, idx, 0, k, k + n, (const char*)&copy);
}

// Insert a null-terminated string into the temporary string.
inline void BAIns(BAStr& s, unsigned int idx, const char* k) {
    BAAlloc extra;
    BAAlloc copy = extra;
    String_ReplaceRange(&s, idx, 0, k, k + strlen(k), (const char*)&copy);
}

#pragma push
#pragma use_lmw_stmw on
// Load computer-shot tables and restore per-hole/player state for a course reload.
#pragma pop

struct RoundPlayerNode {
    int previousLink;
    RoundPlayerNode* next;
    unsigned int playerIndex;
    char paddingBeforeName[0x16];
    char name[0x2E];
    int controllerIndex;
    int partyItemSlot;
    unsigned int isComputer;
};

struct RoundPlayerList {
    int count;
    int sentinelLink;
    RoundPlayerNode* first;
};

struct RoundPlayerSelection {
    char paddingBeforeFields[0x44];
    int pangBalance;
    char paddingBeforeEquipment[0x44];
    int equipmentWords[8];
    char paddingBeforeCharacterId[0x74];
    int characterId;
    char paddingBeforeParts[4];
    unsigned int outfitParts[24];
    unsigned char appearance;
    char paddingBeforeCaddie[0x2F];
    int caddieId;
    char paddingAfterCaddie[0x18];
};

struct RoundSelectionState {
    RoundPlayerSelection players[4];
    char paddingBeforeMode[0xB8];
    unsigned int gameMode;
    char paddingBeforeRuleType[0xB4];
    unsigned char golfRuleType;
    char paddingBeforePartyMode[0x217];
    int partyGameMode;
    char paddingBeforeGolfDoc[0xB4];
    struct TutorialProgress* golfDoc;
};

struct TutorialProgress {
    char paddingBeforeFields[0x98];
    int tutorialStage;
};

struct PlayerSnapshotSetup {
    int controllerIndex;
    int isComputer;
    int playerState;
    char rankTieBreakOrder;
    char paddingBeforePower[0x9B];
    float powerShotMeter;
};

struct PlayerEquipmentWords {
    int equipmentWords[8];
};

struct RulePlayerSetup {
    char paddingBeforeFields[0x260];
    PlayerEquipmentWords slot[4];
    char paddingBeforeTurn[0xC0];
    unsigned char turn;
};

struct RenderPlayerSave {
    char paddingBeforeFields[0x24];
    int pangBalance;
    char paddingAfterPang[0x800];
};

typedef char RoundPlayerSelectionSize[(sizeof(RoundPlayerSelection) == 0x1D4) ? 1 : -1];
typedef char RoundPlayerCharacterOffset[((int)&((RoundPlayerSelection*)0)->characterId == 0x120) ? 1 : -1];
typedef char RoundPlayerPartsOffset[((int)&((RoundPlayerSelection*)0)->outfitParts == 0x128) ? 1 : -1];
typedef char RoundPlayerAppearanceOffset[((int)&((RoundPlayerSelection*)0)->appearance == 0x188) ? 1 : -1];
typedef char RenderPlayerSaveSize[(sizeof(RenderPlayerSave) == 0x828) ? 1 : -1];

// Keep the byte-add form: a member address merges the two sentinel calculations.
enum {
    kRoundPlayerSentinelLinkOffset = 4
};

// View the current shared selection without retaining a singleton across calls.

// Allocate player snapshots and populate them from selected save records.

} // extern "C"
} // namespace ns0

// ---- merged from golf_rule_round_flow_and_shot_sim.cpp ----
namespace ns1 {

extern "C" {

extern void* g_sharedDoc;
extern void* g_cameraWork;
extern void* g_rival;
extern void* g_randomHouse;
extern void* g_drawDriveLine;
extern void* g_view;
extern void* g_soundManager;
extern unsigned char s_cameraTransform[];
extern unsigned char golfRuleStrings[];
extern const float kFloat_3_0;
extern const float kFloat_100000_GolfRule;
extern const float kFloat_0_GolfRule;
extern const double kDouble_4503601774854144_GolfRule;
extern const float kFloat_33_GolfRule;
extern unsigned int lbl_804AE420[];
extern unsigned int s_shotActionRestrictionState[];

struct ObjVtable {
    void* unk0;
    void* unk4;
    void (*destroy)(void*, int);
};

void* CQuadTree_GetInstance();
void GetGroundPoint__9CQuadTreeF7WVectorbPib(void*, void*, void*, int, void*, void*);
void* GolfBall__Fv();
void __as__7WVectorFRC7WVector(void*, void*);
int WView_DefaultCapabilityTrueA(void*, void*);
void __mi__FRC7WVectorRC7WVector(void*, void*, void*);
float WVectorLen__FRC7WVector(void*);
void CourseData_FindSurfaceByName(void*, int);
void Camera_SetClip(void*, float, float, int);
void Camera_BuildFrustumPlanes(void*);
void appFreeUni__FPv(void*);
void SoundManager_ReleaseSoundBank(void*, void*);
void SoundManager_AcquireSoundBank(void*, void*);
void SoundManager_StopAndClearChannels(void*);
void SoundManager_RequestBgmStop(void*, int);
void* GolfPlayer_GetEntryByIndex(void*, int);
void* ItemInventory_FindCaddieItemData(void*, void*);
int PlayerRank_FindTierForValue(void*);
void ItemSelection_BuildCharacterOutfit(void*, int, int, int, void*, int);
int SharedDoc_CompactPlayerItemSlots(void*, int);
void Memory_Clear(void*, int);
void* memset(void*, int, unsigned int);

// Clear one hole's stroke/result bytes and its word-sized Pang total.

// Initialize one player snapshot, equipment selection and derived stats.

// Update display gating and refresh camera clipping/frustum state.

struct DestroyObject {
    virtual ~DestroyObject();
};
struct DestroyBase58 {
    char pad[0x58];
};
struct DestroyObject58 : DestroyBase58 {
    virtual ~DestroyObject58();
};
#pragma use_lmw_stmw on
#pragma opt_propagation off
// Release child tasks, swing controller and shared round services.

#pragma opt_propagation reset
#pragma use_lmw_stmw reset

} // extern "C"

} // namespace ns1

// ---- merged from golf_rule_round_flow_and_shot_sim.cpp ----
namespace ns2 {

extern "C" {

struct Obj438 {
    virtual ~Obj438();
    virtual void vx();
    virtual int vf(float);
    int pad4;
    int field8;
};
// Dispatcher ABI views: the same objects as CGolfRule/GolfRuleFrameFields,
// CGolfDoc, CSharedDoc, ClubSelectionRuntime and CGolfBall above, NOT new state.
// Keep the original signed int/unsigned byte loads here. In particular +410 and
// +4E4 are unsigned bytes here, while CGolfRule's legacy header uses plain char;
// shared partyGameMode is unsigned but this path observes it as signed int.
// The document/club/ball typedefs below reuse the common model directly. Only
// rule/shared ABI views remain for the byte signedness and typed pointer slots.
// Field names match the common views; the checks below bind every used member.
// Rule-only overviewTarget (+28) feeds the HoleOverview camera subtraction;
// unk_094 and shared unk_8A4 have no established wider meaning.
// pendingDialog aliases CGolfRule::forgetItemDialog (+438). field8==1 sends
// item message 9 with forgetItemSlot; completion deletes/clears that owned dialog.
// hudVisible is forwarded to screen/letter/spot and mouse cursor.
struct ObjCFE4 {
    char pad0[0x28];
    float overviewTarget[3];
    float elapsedTime;
    char pad38[0x58];
    unsigned char stateEntryPending;
    char pad91[3];
    unsigned char unk_094;
    char pad95[7];
    float stateTimer;
    char pada0[0x240];
    unsigned char nextPlayerIndex;
    char pad2e1[0x108];
    unsigned char shotBlock;
    char pad3ea[2];
    float shotBlockTime;
    char pad3f0[0x20];
    unsigned char forgetDlg;
    char pad411[3];
    float forgetDlgTime;
    int forgetItemSlot;
    char pad41c[4];
    float coinTime;
    int coinCount;
    int coinNum;
    char pad42c[0xc];
    Obj438* pendingDialog;
    char pad43c[0x4c];
    int forceDisplayUpdate;
    unsigned char hudVisible;
    char pad48d[3];
    int shotPresentationFlags;
    int presentationStep;
    unsigned char unk_498;
    char pad499[0x23];
    int tutorialFeedbackEnabled;
    int tutorialAdvancePending;
    char pad4c4[0x20];
    unsigned char fastPlayback;
};
typedef CGolfDoc GameCFE4;
struct GlobalE2F8 {
    char pad0[0x8a4];
    unsigned char unk_8A4;
    char pad8a5[0x1b];
    unsigned char golfRuleType;
    char pad8c1[0x104];
    unsigned char roundHoleCount;
    char pad9c6[0x112];
    int partyGameMode;
    char padADC[0xb4];
    GameCFE4* golfDoc;
};
struct MouseCursorView {
    char pad0[0x105];
    unsigned char enabled;
};
struct Cam784 {
    char pad0[0x28];
    unsigned char b28;
    char pad29[0x3b];
    float f64;
};
typedef SelectedClubRuntime E4;
typedef ClubSelectionRuntime EA168;
typedef CGolfBall S12;
struct S5958 {
    char pad0[0xc];
    int i0c;
    char pad10[0x58];
    void* i68;
};
struct S3A5 {
    char pad0[0x3a5];
    unsigned char b3a5;
};
struct FadeStateView {
    char pad0[0x10];
    unsigned char captureRequested;
};
struct F3C {
    int a;
    float f;
};
typedef Vec3f Vec3;
struct Vec3i {
    int x, y, z;
};
struct Mtx {
    float m[12];
};

// Explicit correspondence to the shared model, including its extension fields.
#define PROCESS_SAME(T, f, Shared, member) \
    typedef char Process_##T##_##f[((int)&((T*)0)->f == (int)&((Shared*)0)->member) ? 1 : -1]
PROCESS_SAME(ObjCFE4, elapsedTime, GolfRuleFrameFields, elapsedTime);
PROCESS_SAME(ObjCFE4, stateEntryPending, GolfRuleFrameFields, stateEntryPending);
PROCESS_SAME(ObjCFE4, unk_094, RuleUnresolvedShotBytes, unk_094);
PROCESS_SAME(ObjCFE4, stateTimer, GolfRuleFrameFields, stateTimer);
PROCESS_SAME(ObjCFE4, nextPlayerIndex, CGolfRule, nextPlayerIndex);
PROCESS_SAME(ObjCFE4, shotBlock, CGolfRule, shotBlock);
PROCESS_SAME(ObjCFE4, shotBlockTime, CGolfRule, shotBlockTime);
PROCESS_SAME(ObjCFE4, forgetDlg, CGolfRule, forgetDlg);
PROCESS_SAME(ObjCFE4, forgetDlgTime, CGolfRule, forgetDlgTime);
PROCESS_SAME(ObjCFE4, forgetItemSlot, CGolfRule, forgetItemSlot);
PROCESS_SAME(ObjCFE4, coinTime, CGolfRule, coinTime);
PROCESS_SAME(ObjCFE4, coinCount, CGolfRule, coinCount);
PROCESS_SAME(ObjCFE4, coinNum, CGolfRule, coinNum);
PROCESS_SAME(ObjCFE4, pendingDialog, CGolfRule, forgetItemDialog);
PROCESS_SAME(ObjCFE4, forceDisplayUpdate, CGolfRule, forceDisplayUpdate);
PROCESS_SAME(ObjCFE4, hudVisible, CGolfRule, hudVisible);
PROCESS_SAME(ObjCFE4, shotPresentationFlags, CGolfRule, shotPresentationFlags);
PROCESS_SAME(ObjCFE4, presentationStep, GolfRuleFrameFields, presentationStep);
PROCESS_SAME(ObjCFE4, unk_498, CGolfRule, unk_498);
PROCESS_SAME(ObjCFE4, tutorialFeedbackEnabled, CGolfRule, tutorialFeedbackEnabled);
PROCESS_SAME(ObjCFE4, tutorialAdvancePending, CGolfRule, tutorialAdvancePending);
PROCESS_SAME(ObjCFE4, fastPlayback, GolfRuleFrameFields, fastPlayback);
PROCESS_SAME(GameCFE4, tutorialStage, CGolfDoc, tutorialStage);
PROCESS_SAME(GameCFE4, holeRecords, CGolfDoc, holeRecords);
PROCESS_SAME(GameCFE4, shotPowerGaugeX, CGolfDoc, shotPowerGaugeX);
PROCESS_SAME(GameCFE4, impactGrade, CGolfDoc, impactGrade);
PROCESS_SAME(GameCFE4, gameStateFlags, CGolfDoc, gameStateFlags);
PROCESS_SAME(GameCFE4, cameraMode, CGolfDoc, cameraMode);
PROCESS_SAME(GameCFE4, currentHoleIndex, CGolfDoc, currentHoleIndex);
PROCESS_SAME(GameCFE4, currentPlayer, CGolfDoc, currentPlayer);
PROCESS_SAME(GlobalE2F8, golfRuleType, CSharedDoc, golfRuleType);
PROCESS_SAME(GlobalE2F8, roundHoleCount, CSharedDoc, roundHoleCount);
PROCESS_SAME(GlobalE2F8, partyGameMode, CSharedDoc, partyGameMode);
PROCESS_SAME(GlobalE2F8, golfDoc, CSharedDoc, golfDoc);
PROCESS_SAME(E4, clubKind, SelectedClubRuntime, clubKind);
PROCESS_SAME(EA168, selectedClub, ClubSelectionRuntime, selectedClub);
PROCESS_SAME(EA168, powerShotMode, ClubSelectionRuntime, powerShotMode);
PROCESS_SAME(S12, m_state, CGolfBall, m_state);
PROCESS_SAME(S12, m_powerFlags, CGolfBall, m_powerFlags);
PROCESS_SAME(S12, m_preDataCount, CGolfBall, m_preDataCount);
PROCESS_SAME(FadeStateView, captureRequested, FadeState, captureRequested);
#undef PROCESS_SAME
typedef char ProcessOverviewTargetOffset[(int)&((ObjCFE4*)0)->overviewTarget == 0x28 ? 1 : -1];
typedef char ProcessUnknownSharedByteOffset[(int)&((GlobalE2F8*)0)->unk_8A4 == 0x8A4 ? 1 : -1];
// screen message 0x30 writes this HUD phase; CClub_ClubProcess reads +78.
typedef char ProcessHudPhaseOffset[(int)&((EA168*)0)->hudPhase == 0x78 ? 1 : -1];

extern GlobalE2F8* g_sharedDoc;
extern const char golfRuleStrings[];
extern char s_golfRuleInputDestructorRecord[];
extern char s_gameInputManager[];
extern char s_fadeOverlay[];
extern char s_cameraTransform[];
extern MouseCursorView* g_mouseCursor;
extern void* g_taskManager;
extern Cam784* g_project;
extern void* g_inputPointer;
extern signed char s_inputManagerInitialized;
extern signed char s_fadeOverlayInitialized;
extern void* g_cameraWork;
extern void* g_sceneManager;
extern void* g_shadowManager;
extern void* g_rival;
extern int g_soundManager;
extern float s_golfRuleCameraScaleReference;
extern const float kFloat_0_GolfRule;
extern float kFloat_3_0;
extern float kFloat_1_805DEA38;
extern const float kFloat_5_GolfRule;
extern float kGolfRuleComparisonEpsilon;
extern float kFloat_0_5_GolfRule;
extern const float kFloat_200_GolfRule;
extern float kFloat_149_805DEA58;
extern float kFloat_360_GolfRule;
extern const float kFloat_0_8_GolfRule;
extern float kFloat_neg0_34906584_GolfRule;
extern float kFloat_0_17453292_GolfRule;
extern const float kFloat_640_GolfRule;
extern float kFloat_0_1_GolfRule;
extern void GolfTransition_ClearInputAudioAndEffects(void*);
extern void SaveProcess_SetTransitionTargetName(const char*);
extern void SaveProcess_SetFollowUpTaskName(const char*);
extern int FloatTween_Update(const char*, float);
extern void SoundManager_SetNamedVolumeScale(int, const char*, float);
extern int Pointer_IsActionActive(void*, const char*, int);
extern int Pointer_GetRisingActionBits(void*, const char*, int);
extern void IActor_DispatchMessage(void*, void*, const char*, int, int, int, int, int);
extern S12* GolfBall__Fv(void);
extern int SharedDoc_IsPlayerFlag0Set(int);
extern S5958* PartyItemSlots_GetInstance(void);
extern void PartyItems_ApplySelection(void*, int);
extern int GolfTask_NoOpStatus(void);
extern void* CQuadTree_GetInstance(void);
extern EA168* InstanceTable_Get(void);
extern int Input_IsActionPressed(void*, int, int);
extern void InputManager_Init(void*);
extern void InputManager_Dtor(void);
extern void* __register_global_object(void*, void*, void*);
extern S3A5* Object_GetChild(void*);
extern void Sound_PlayByName(int, const char*, int, int, int, int, float, float, float);
extern void FadeOverlay_Init(void*);
extern void ResourceState_Dtor(void);
extern void FadeState_RequestFramebufferCapture(void*);
extern void FadeState_StartFadeIn(void*, int);
extern void FadeState_StartFadeOut(void*, int);
extern void FadeState_RequestReleaseOnFadeCompletion(void*);
extern void DrawScene_NoOpHook(void*, int);
extern void DrawScene_SetNamedObjectEnabled(void*, const char*, int);
extern char* String_FormatTemporary(const char*, ...);
extern void Matrix_SetAxisRotation(Mtx*, float, int);
extern void Matrix_ConcatAffine(Mtx*, Mtx*, Mtx*);
extern void Matrix_CopyAffine(void*, Mtx*);
extern void __mi__FRC7WVectorRC7WVector(Vec3*, void*, void*);
extern float WVectorLen__FRC7WVector(Vec3*);
extern void __ml__FfRC7WVector(Vec3*, void*, float);
extern void __as__7WVectorFRC7WVector(void*, Vec3*);
extern void ComUnplayable_UpdateCurrentDropSequence(void*, void*);
extern void* GolfPlayer_GetEntryByIndex(void*, unsigned char);
extern void PartyGoodsEffectList_AddAppearance(void*, int, int, void*);

// Read the current value of the sound-volume tween.
static float getF3C(F3C* t) {
    return t->f;
}

typedef ObjCFE4 GolfRuleProcessView;

// Dispatch the active golf state and process shared UI, timing, camera and skip input.
// dt is elapsed seconds. The screen actor must exist: query 0x30 writes the
// stack result synchronously through an int encoded in the 32-bit payload word.

struct ObjE2F8 {
    char pad0[0x34];
    float elapsedTime;
    char pad38[0x64];
    float stateTimer;
    char padA0[0x311];
    unsigned char processingPaused, networkSyncPending;
};

// Wait for the result camera, then advance the fight result or leave the mode.

} // extern "C"

} // namespace ns2

// ---- merged from golf_rule_round_flow_and_shot_sim.cpp ----
namespace ns3 {

extern "C" {

extern char s_golfRuleRttiDescriptor;
extern void* g_sharedDoc;
extern void* g_mouseCursor;
extern void* g_sceneManager;
extern void* g_soundManager;
extern void* g_cameraWork;
extern const char golfRuleStrings[];
extern const float kFloat_3_0;
extern const float kFloat_1_805DEA38;
extern const float kFloat_0_5_GolfRule;
extern const float kFloat_200_GolfRule;

void* GolfPlayer_GetEntryByIndex(void*, unsigned char);
void IActor_DispatchMessage(void*, void*, const char*, ...);
void* NamedEffectList_FindByName(void*, const char*);
void WPuppet_SetOpacityRecursive(void*, int);
void Sound_StopByAlias(void*, const char*);
void Sound_PlayByName(void*, const char*, int, int, int, int, float, float, float);
extern void* g_cameraWork;
extern void* g_taskManager;
extern signed char s_effectFadeInitialized, s_fadeOverlayInitialized, s_inputManagerInitialized, s_saveDataInitialized;
extern float kFloat_0_GolfRule, kFloat_8_GolfRule, kFloat_15_GolfRule;
extern const double kDouble_4503601774854144_GolfRule;
extern char s_golfRuleInputDestructorRecord[], s_effectFade[], s_fadeOverlay[], s_gameInputManager[], s_saveData[];
int Layout_SetHitTestingEnabled(int);
int MPBase1_Process(void*, float);
int GolfRule_UpdateCourseClearStatus();
void* __ct__10CRetryMenuFi(void*, int);
void __ct__11CEffectFadeFv(void*);
void __dt__11CEffectFadeFv();
void FadeQuad_SetRgb(void*, int);
void InputManager_Init(void*);
void InputManager_Dtor();
int SharedDoc_IsPlayerFlag0Set(int);
void InputManager_SetCursorDrawOptions(void*, int, int, int, int);
void InputManager_SetCursorColorOverride(void*, int, int, int, int, int);
void InputManager_SetSelectedPlayer(void*, int);
void ItemRequestState_ClearGlobalEntries();
void FadeOverlay_Init(void*);
void ResourceState_Dtor();
void FadeState_Reset(void*);
void FadeState_RequestFramebufferCapture(void*);
void FadeState_StartFadeIn(void*, int);
void SaveData_Ctor(void*);
void* SaveData_GetCurrentRecord(void*);
void SaveRecord_CopyCtor(void*, void*);
void SaveData_SetCurrentRecord(void*, void*);
int SharedStringState_GetFirstString(int);
void __register_global_object(void*, void*, const char*);

// Return this actor type descriptor.

struct GolfRuleTurnStartView {
    char pad0[0x34];
    float value34;
    char pad38[0x59];
    unsigned char flag91;
    char pad92[0x24E];
    unsigned char nextPlayerIndex;
    char pad2E1[0xD0];
    unsigned char flag3B1;
    unsigned char flag3B2;
};

struct SharedDocTurnView {
    char pad0[0xAD8];
    int partyGameMode;
    char padADC[0xB90 - 0xADC];
    void* golfDoc;
};

struct Object8014 {
    char pad0[0x28];
    unsigned char count;
};

struct Character8014 {
    char pad0[0x28];
    unsigned char count;
};

struct GolfRulePostRoundView {
    char pad[0x4A8];
    unsigned char tick4a8;
    char pad2[3];
    void* resultMenu;
    void* retryMenu;
    int postRoundMenuState;
    char pad3[0x10];
    int value4c8;
};
struct ManagerBig8014 {
    char pad[0x808];
    int mode;
    char pad2[0x384];
    void* objb90;
    char pad3[0x128];
    unsigned char flag8c0;
};
struct Dialog8014 {
    char data[0x28];
    // Construct the retry dialog using the requested mode.
    Dialog8014(int mode) {
        ((CRetryMenu*)this)->__ct(mode);
    }
};
struct Virtual8014 {
    virtual ~Virtual8014();
    char data[0x1c];
    int result;
};

#define SINGLE8014(ptr, flag, obj, ctor, dtor, registration)          \
    if (!(ptr)) {                                                     \
        if (!(flag)) {                                                \
            ctor(obj);                                                \
            __register_global_object(obj, (void*)dtor, registration); \
            flag = 1;                                                 \
        }                                                             \
        ptr = obj;                                                    \
    }
#define INPUT8014()                                                                                     \
    SINGLE8014(GetInputManagerSlot(), s_inputManagerInitialized, s_gameInputManager, InputManager_Init, \
               InputManager_Dtor, globalBase + 0)
#define FADE8014(registration)                                                                                       \
    SINGLE8014(Sing_805DC660(), s_effectFadeInitialized, s_effectFade, __ct__11CEffectFadeFv, __dt__11CEffectFadeFv, \
               registration)
#define SOUND8014(registration)                                                                                \
    SINGLE8014(Sing_805DC608(), s_fadeOverlayInitialized, s_fadeOverlay, FadeOverlay_Init, ResourceState_Dtor, \
               registration)
#define SAVE8014()                      \
    if (!GetSaveDataSlot()) {           \
        if (!s_saveDataInitialized) {   \
            SaveData_Ctor(s_saveData);  \
            s_saveDataInitialized = 1;  \
        }                               \
        GetSaveDataSlot() = s_saveData; \
    }
#define DELETE8014(member)                 \
    if (self->member) {                    \
        delete (Virtual8014*)self->member; \
        self->member = 0;                  \
    }
#define UI8014(a, b, c, d, e)                                                               \
    ((void (*)(void*, void*, const char*, int, int, int, int, int))IActor_DispatchMessage)( \
        self, self, golfRuleStrings + kText_player, a, b, c, d, e)

// Poll result/retry menus and apply the selected post-round action.

// Prepare the next player turn after the opening delay and pending messages.

} // extern "C"

} // namespace ns3

// ---- merged from golf_rule_round_flow_and_shot_sim.cpp ----
namespace ns4 {

struct VText {
    virtual void v00();
    virtual void v01();
    virtual void v02();
    virtual void v03();
    virtual void v04();
    virtual void v05();
    virtual void v06();
    virtual void v07();
    virtual void v08();
    virtual void v09(void* values);
    virtual void v10();
    virtual void v11(void* values);
};

extern "C" {

extern signed char s_fadeOverlayInitialized;
extern void* __RTTI__6IActor;
extern void* __RTTI__11CGolfCaddie;
extern char s_fadeOverlay[];
extern void InputManager_Init(void*);
extern void Vec2_Zero(void*);
extern void* CourseData_FindBoundsContainingPoint(void*, void*);
extern void* CourseData_FindNamedRecord(void*, char*);
extern void* TaskActorTable_FindByName(void*, const char*);
extern void* Object_GetChild(void*);
extern void SoundManager_SetEnabled(void*, int);
extern void FadeOverlay_Init(void*);
extern void ResourceState_Dtor(void*);
extern void* Runtime_DynamicCast(void*, int, void*, void*, int);
extern int String_ScanTokensAfterSubstring(char*, const char*, const char*, ...);
extern void* __nwa__FUl(unsigned int);
extern void* InstanceTable_Get();

void fn_80346F58();
void __as__7WVectorFRC7WVector(void*, const void*);
float* GolfBall__Fv();
void* InstanceTable_Get();
int Sound_PlayByName(void*, const char*, int, int, int, int, float, float, float);
void InputManager_Dtor(void*);
extern unsigned int* g_sharedDoc;
extern void* g_rival;
extern signed char s_inputManagerInitialized;
extern char s_gameInputManager[];
extern unsigned int s_golfRuleInputDestructorRecord[];
extern unsigned int* g_soundManager;
extern char golfRuleStrings[];
int SharedDoc_IsPlayerFlag0Set(int);
int PlayerRank_FindTierForValue(int);
int CSharedDoc_ComputeAuxPartModifier(int, int, int);
void* ItemInventory_GetAuxPartIdAt(void*, int);
extern float s_shotOutcomeClubSetModifiers[];
extern double kDouble_4503601774854144_GolfRule;
extern const float kFloat_0_GolfRule;
extern float kFloat_1_805DEA38;
extern const float kFloat_99_GolfRule;
extern char s_shotActionRestrictionState[];
void* memcpy(void*, const void*, unsigned int);
float Vector_SignedAngleXZ(void*, void*);
void __mi__FRC7WVectorRC7WVector(void*, void*, void*);
void IActor_DispatchMessage(void*, void*, const char*, int, int, int, int, int);
void InputManager_Init(void*);
int Input_IsActionPressed(void*, int, int);
int MPBase1_Process(void*);
extern signed char s_effectFadeInitialized;
extern char s_effectFade[];
void* __ct__11CEffectFadeFv(void*);
void __dt__11CEffectFadeFv(void*);
void* FadeQuad_SetRgb(void*, int);

#pragma push
#pragma opt_propagation off
#pragma opt_common_subs off
#pragma opt_lifetimes off
// Query the screen's HUD phase into caller storage; the receiver must be present.
// CScreen_HandleMsg writes *outPhase for ID 0x30 and does not retain its address.
static inline void QueryScreenHudPhase(void* rule, int* outPhase) {
    IActor_DispatchMessage(rule, rule, golfRuleStrings + kText_screen, kScreenQueryHudPhaseMessage, (int)outPhase, 0, 0,
                           0);
}
// Query whether screen panels finished their transitions; receiver writes *outReady.
static inline void QueryScreenPanelsReady(void* rule, int* outReady) {
    IActor_DispatchMessage(rule, rule, golfRuleStrings + kText_screen, kScreenQueryPanelsReadyMessage, (int)outReady, 0,
                           0, 0);
}

// Handle aiming, tutorial gates and swing start; computer players use their prepared shot.
// Actor-message p0 is a 32-bit word: pointer payloads borrow stack storage only for
// the dispatch. Query outputs rely on a live receiver; no fallback is supplied.
// HUD phase 1 admits aiming, phase 2 permits a camera-mode transition. Panel-ready
// query 0x40 overwrites the same gate before swing input is examined.

#pragma pop

extern signed char s_nowReloadingInitialized;
extern char s_nowReloading;

int LoadingIndicator_CreatePlayer(void*);
int LoadingIndicator_IsAbsentOrState3(void*);
void FadeState_StartFadeOut(void*, int);
void __dt__13CNowReloadingFv(void*);
void* IActor_GetDispatchObject(void*);
void* GolfPlayer_GetEntryByIndex(int, int = 0);
void ResourceState_Dtor(void*);

void InputManager_Init(void*);
int Input_IsActionPressed(void*, int, int);
int InputManager_StartRumble(void*, int);
void IActor_DispatchMessage(void*, void*, const char*, int, int, int, int, int);
void __mi__FRC7WVectorRC7WVector(void*, void*, void*);
float Vector_SignedAngleXZ(void*, void*);
void GolfAim_FindTargetAlongPath(void*, void*, int);
int SharedDoc_IsPlayerFlag0Set(int);
extern void* g_rival;
extern float kFloat_1_805DEA38;
extern signed char s_uintVectorMaxSizeInitialized;
extern unsigned int s_fourByteVectorMaxElements;
extern signed char s_inputManagerInitialized;
extern void* g_helpMsg;
extern char s_gameInputManager[];
void InputManager_Dtor(void*);
void NoticeQueue_Clear(void*);
unsigned int RingBuffer32_CalculateGrowth(void*, unsigned int);
void* Object_GetChild(void*);
char* String_FormatTemporary(const char*, ...);
void* TaskActorTable_FindByName(void*, const char*);
int Input_IsActionButtonDown(void*, int, int);
void Vec2_Zero(void*);
int PartyGame_EvaluatePlayerThresholdTier(int);
extern unsigned char s_cameraDirectedShotEnabled;

void RingBuffer32_AppendWord(unsigned int*, unsigned int*);
void __dl__FPv(void*);
char* strcpy(char*, const char*);
void* __nwa__FUl(unsigned int);
void* __nw__FUl(unsigned int);
void __dla__FPv(void*);
void __dt__Q23std12length_errorFv(void*);
void* appMallocDummy__FUl(int offset);
void* memset(void*, int, unsigned int);
// The destructor under its linker name: the thunk calls it this way so the call stays out of line.
void* __dt__9CGolfRuleFv(void* self, int flags);

void ShotHistory_InsertIteratorRange(void* self, unsigned int* arg1, void* arg2, void* arg3, void* arg4);
void FloatTween_Init(void*, float);
void* IntPair_Set(void*, const char*, void*);
void* ActorFactoryRegistry_GetInstance();
void StringMap_SetByName(void*, void*, const char*);
void* TaskRegistry_GetInstance();
void TaskRegistry_AppendFactory(void*, const char*);
void SoundManager_RequestBgmChange(void*, const char*, int, int);
extern float kFloat_114_GolfRule;
extern unsigned int* g_soundManager;
extern int puttingBgmFileNames[2];
void ShotParameters_UpdateControlWindow(void*);
void FloatTween_Start(const char*, int, float, float);
void InputManager_SetCursorDrawOptions(void*, int, int, int, int);
void InputManager_SetCursorColorOverride(void*, int, int, int, int, int);
void InputManager_SetSelectedPlayer(void*, void*);
void CGolfRule_Create();
void dtor_8014B5E0(void*, int);
struct Fn8016C2B8List {
    unsigned int* data;
    unsigned int size, capacity;
    operator unsigned int*() const {
        return data;
    }
};
extern Fn8016C2B8List s_shotEffectPreloadTasks;
extern unsigned int s_golfRuleInputDestructorRecord[];
extern char golfRuleStrings[];
extern char __vt__Q23std11logic_error[];
extern char s_sharedCharDeleterVtable[];
extern char __vt__Q23std12length_error[];
extern const unsigned int s_ballEffectPreloadSequenceNames[];
extern signed char s_ringBufferWordLimitInitialized;
extern unsigned int s_ringBuffer32MaxWords;
extern char s_golfRuleActorFactoryName[];
extern char s_golfRuleTaskRegistryName[];
extern unsigned int s_golfRuleRttiDescriptor;
extern unsigned int s_taskMainTypeInfo;
extern signed char s_saveDataInitialized;
extern char s_saveData[];
void SaveData_Ctor(const char*);
void* SaveData_GetCurrentRecord(void*);
void SaveRecord_CopyCtor(void*, void*);
extern float s_twoPi_8014B210_805E2A00;
extern const float kFloat_6_2831855;
extern const float kFloat_0_GolfRule;
extern int s_shotHistoryFrontGrowthBias;
extern signed char s_shotHistoryFrontGrowthBiasInitialized;

void FadeOverlay_Init(void*);
void* CQuadTree_GetInstance();
void ResourceState_Dtor(void*);
void FadeState_Reset(void*);
void FadeState_RequestFramebufferCapture(void*);
void FadeState_StartFadeIn(void*, int);
void CQuadTree_SetObjectSoundEnabled(void*, int);
void DrawScene_SetNamedObjectEnabled(void*, void*, int);
extern signed char s_fadeOverlayInitialized;
extern char s_fadeOverlay[];
extern void* golfRuleAmbientCreatureNames[];
extern unsigned int* g_sceneManager;

// Advance the story sequence and fade before returning to normal hole play.

struct Fn8016C700Task {
    virtual void unused();
    virtual void update();
    virtual int done();
};
#pragma opt_strength_reduction off
// Update each queued preload task and report whether every task is complete.
int GolfRule_PollSequencePreloads();

#pragma opt_strength_reduction reset

extern unsigned int* g_sharedDoc;
extern unsigned int* g_sceneManager;
extern unsigned int* g_cameraWork;
extern unsigned int* g_fx;
void DrawScene_NoOpHook(void*, int);
extern unsigned int* g_taskManager;
extern unsigned int* g_project;
void GolfTransition_ClearInputAudioAndEffects(void*);
void SaveProcess_SetTransitionTargetName(const char*);
void SaveProcess_SetFollowUpTaskName(const char*);
int SaveData_IsSharedRecordMode(void*);
void* SaveData_AcquireInitializedInstance();
void TaskCommandQueue_AppendNamedRequest(void*, int, const char*, int, int, int, int, int);
void IActor_DispatchMessage(void*, void*, const char*, int, int, int, int, int);
int GolfTask_NoOpStatus();
float* GolfBall__Fv();
void* CQuadTree_GetInstance();
void* InstanceTable_Get();
float ShotParameters_GetGoodImpactHalfWidth(void*);
int GolfTask_NoOpStatus();
int OS_GetTimeMilliseconds();
void CGolfRule_UpdateSingleStageGauge(void*, int);
void CGolfRule_UpdatePowerAndImpactGauge(unsigned char*, int);
extern float kFloat_10_GolfRule;
extern const float kGolfRuleMillisecondsToSeconds;
extern char golfRuleStrings[];
int OS_GetTimeLow();
void* GolfPlayer_GetEntryByIndex(int, int);
void* ItemInventory_GetItemIdAt(void*, int);
int ItemInventory_IsSlot14ClubSetId(void*);
void* PartyItemSlots_GetInstance();
float ShotParameters_GetGoodImpactHalfWidth(void*);
float ShotParameters_GetPoorImpactHalfWidth(void*);
int SharedDoc_GetCourseId();
void StringArray_ClearElements(void*);
void* SharedDoc_GetCurrent();
void MWString_ConstructCString(void*, const char*);
void StringVector_AppendCopy(void*, void*);

void* PreData__9CGolfBallFi(void*, int);
void __mi__FRC7WVectorRC7WVector(void*, void*, void*);
float Magnitude__7WVectorCFv(void*);
void __ct__10CRetryMenuFi(void*, int);
void DriveLineMarkers_Generate(void*, unsigned char);
void __ct__16CResultFightMenuFPv(void*, int);
void __as__7WVectorFRC7WVector(void*, const void*);
void* Normalize__7WVectorFv(void*);
extern const float kFloat_20_GolfRule;
extern const float kFloat_0_GolfRule;
extern float kFloat_0_5_GolfRule, kFloat_149_805DEA58, kFloat_0_01_GolfRule, kFloat_3_2_GolfRule;
extern float kFloat_64_GolfRule, kFloat_320_GolfRule, kFloat_121_6_GolfRule, kFloat_9_GolfRule;
extern const float kFloat_0_7_GolfRule;
extern double kDouble_4503601774854144_GolfRule;
extern int s_pangyaZoneBaseWidthUnits;
void TourMap_CommitNodeCompletionAndRewards();
void ItemRequestState_ClearGlobalEntries();
void SharedStringState_SetContext(void*);
extern const int s_strokePlayRankPangRewards[];
extern const int s_matchPlayHoleCountPangRewards[][2];

void* Matrix_SetAxisRotation(void*, float, int);
void* PartyItemSlots_GetInstance();
void __pl__FRC7WVectorRC7WVector(void*, const void*, const void*);
void __ml__FfRC7WVector(void*, const void*, float);
void* Matrix_TransformPoint(void*, const void*, const void*);
void Matrix_BuildCameraTransform(void*, const void*, const void*);
void Matrix_CopyAffine(void*, const void*);
float Magnitude__7WVectorCFv(void*);
void Camera_SetFov(void*, float);
int Sound_StopByAlias(void*, const char*);
void NoticeQueue_ResetAndActivate(void*);
extern void* g_view;
extern const float kFloat_2_0943952_GolfRule;
extern float kFloat_32_GolfRule;
extern const float kFloat_neg1_9198622_GolfRuleCamera;
extern float kFloat_0_5_GolfRule;
extern float kFloat_2_805DEA84;
extern char golfRuleMessageTags[0x500];
extern char s_cameraTransform[];

extern unsigned int s_bezierCameraPointIndex;
extern const double kDouble_5_GolfRule;
extern const float kFloat_5_GolfRule;
extern float kFloat_0_8_GolfRule;
extern const float kFloat_neg0_1_GolfRule;
extern const float kFloat_6_25_GolfRule;
extern const float kFloat_neg0_9_BezierPathCamera;

// Evaluate the current course camera-path segment and build its view transform.

#pragma push
#pragma use_lmw_stmw on
#pragma opt_lifetimes off
// Restore the first simulated ball sample and set up the selected playback camera.

#pragma opt_lifetimes reset
#pragma pop

// Wait for screen replies before completing or cancelling a camera-mode transition.

// A value object keeps the message address separate from the wait-flag temporaries.
struct GolfRuleWaitMessage {
    const char* value;
    // Borrow a network-wait message string without taking ownership.
    GolfRuleWaitMessage(const char* p) : value(p) {}
};
// Publish network-wait flags, timeout and borrowed message, then stamp send time.
static inline void GolfRule_StartNetworkWait(void* self, int active, int line, GolfRuleWaitMessage message,
                                             float timeout) {
    ((unsigned char*)self)[0x3B2] = active;
    AsCGolfRule(self)->netMsgWaitLine = line;
    AsGolfRuleFrame(self)->networkWaitTimeout = timeout;
    (*(const char**)&AsCGolfRule(self)->netMsgSend) = message.value;
    AsCGolfRule(self)->netMsgSendTime = OS_GetTimeLow();
}
// Commit the gauge result through network synchronization or start the shot animation.

// Set presentation flags from shot distance, special-shot bits, impact and landing state.

// Wait for player and caddie resources, then reset hole positions and start its camera.

// Wait for the camera delay, then enter the player-specific continuation state.

struct Rf4B {
    char pad[4];
};
class Rf4 : public Rf4B {
public:
    virtual void v08(int);
};
// Release the state-transition dialog and resume the selected turn state on acceptance.

// Choose the task transition appropriate to the current game mode.

// Apply a post-round menu choice, including replay, next hole and leaving the mode.

struct Fn80151B80List {
    unsigned int* data;
    volatile unsigned int size;
    unsigned int capacity;
    // Initialize an empty list/vector header without allocating.
    Fn80151B80List() : data(0), size(0), capacity(0) {}
    DECOMP_CT_MEMBER(Fn80151B80List)
};
// Register the golf-rule actor factory and its type descriptor.

void ShotHistory_InsertRangeDispatch(unsigned int* self, const void* a, const void* b, const void* c, int flag);
void* RingBuffer32_GetCountAddressForResize(void*);
void* RingBuffer32_GetCountAddress(void*);
void RingBuffer32_PopFront(void*);
void RingBuffer32_AppendWordForward(unsigned int*, unsigned int*);
void* RingBuffer32_GetFrontPointer(void*);
void* RingBuffer32_GetBackPointer(void*);
void* UnsignedWord_LoadPointerValue(void*);
int ShotHistory_GetFrontGrowthBias(void*);
unsigned int ShotHistory_GetBlockCapacity(void*);
void RingBuffer_ThrowLengthError(const char*);
unsigned int RingBuffer32_GetUsableCapacity(void*);
void* RingBuffer32_GetCountWord(void*);
void* ShotHistory_InitEmptyHeader(void*);
void* UnsignedWord_SelectMaxPointer(void*, void*);
void RingBuffer32_ReallocateStorage(unsigned int*, unsigned int);
void* ShotHistory_AllocateEntries(void*, int);
int RingBuffer_IsEmpty(void*);
void RingBuffer32_MakeEndIterator(unsigned int*, const unsigned int*);
void RingIterator32_CopyRetreated(unsigned int*, const unsigned int*, int);
void RingBuffer32_MakeBeginIteratorVolatile(unsigned int*, unsigned int*);
void RingBuffer32_SwapStorage(unsigned int*, unsigned int*);
void RingBuffer32_ResetCount(void*);
void* RingBuffer32_dtor(void*, int);

struct Fn8014F458Local {
    unsigned int words[6];
    // Release the temporary block-map allocation if resize exits or throws.
    ~Fn8014F458Local() {
        RingBuffer32_dtor(this, -1);
    }
};

// Resize the shot-history container while preserving its circular block ordering.
// This grows storage, not the logical shot count. count is additional requested
// entry capacity, rounded up to 16-entry blocks; each entry is 0x54 bytes.
// The six-word outer header begins [frontBias, entryCount, mapData, mapHead,
// mapCount, mapCapacity]. Passing self+8 exposes the four-word circular map;
// these helpers' pointer-shaped accessors sometimes return scalar counts.
// The map stores pointers to owned 16-entry blocks plus a reserved zero slot.
// Logical entry n: block=(frontBias+n)>>4, withinBlock=(frontBias+n)&15;
// advance the map's circular iterator to that block before indexing its payload.
// No entry destructors run here. Existing blocks are rotated/transferred, and
// newly allocated blocks stay owned by this history after the storage swap.
void ShotHistory_Resize(void* self, unsigned int count);

#pragma use_lmw_stmw on
struct Fn80150508Range {
    unsigned int a, b, c, d;
    char allocator;
    // Leave iterator words uninitialized until the caller supplies a range.
    Fn80150508Range() {}
    // Copy all four iterator words without changing their positions.
    Fn80150508Range(const Fn80150508Range& x) : a(x.a), b(x.b), c(x.c), d(x.d) {}
};
#pragma opt_lifetimes off
#pragma opt_propagation off
// Snapshot position/source descriptors before insertion; flag is an unused ABI arg.
void ShotHistory_InsertRangeDispatch(unsigned int* self, const void* position, const void* sourceFirst,
                                     const void* sourceLast, int flag);

#pragma opt_propagation reset
#pragma opt_lifetimes reset
#pragma use_lmw_stmw reset

// Forward one word to the circular buffer append implementation.
void RingBuffer32_AppendWordForward(unsigned int* self, unsigned int* value);

struct Fn8014F6E0Buffer {
    void* vtable;
    int a, b;
    char* data;
};
struct Fn8014F6E0Error {
    char* name;
    char* data;
};
struct Fn8014F6E0Guard {
    char* data;
    char* allocator;
    // Guard a newly allocated character array until its pointer is released.
    Fn8014F6E0Guard(char* p, char* a) : data(p), allocator(a) {}
    // Free the guarded character array unless ownership was transferred.
    ~Fn8014F6E0Guard() {
        if (data)
            __dla__FPv(data);
    }
};
struct Fn8014F6E0Base {
    char* vtable;
    char* data;
    Fn8014F6E0Buffer* buffer;
    // Copy the exception message and install its shared character deleter.
    Fn8014F6E0Base(const char* name) {
        *(char* volatile*)&vtable = __vt__Q23std11logic_error;
        volatile char allocator = 0;
        unsigned int n = strlen(name) + 1;
        char* copy = (char*)__nwa__FUl(n * 2 - n);
        data = copy;
        {
            Fn8014F6E0Guard guard(copy, (char*)&allocator);
            Fn8014F6E0Buffer* p = (Fn8014F6E0Buffer*)__nw__FUl(0x10);
            if (p) {
                p->a = 1;
                p->b = 1;
                p->vtable = s_sharedCharDeleterVtable;
                p->data = copy;
            }
            buffer = p;
            guard.data = 0;
        }
        strcpy(data, name);
    }
    // Preserve the original empty exception-cleanup hook.
    ~Fn8014F6E0Base() {}
};
struct Fn8014F6E0Derived : Fn8014F6E0Base {
    // Install the length-error vtable and throw the constructed exception.
    Fn8014F6E0Derived(const char* name) : Fn8014F6E0Base(name) {
        vtable = __vt__Q23std12length_error;
        __throw(golfRuleStrings + kText_lengthErrorTypeName, this, (void*)__dt__Q23std12length_errorFv);
    }
};
struct Fn8014F6E0Local {
    Fn8014F6E0Error error;
    // Preserve the original empty exception-cleanup hook.
    ~Fn8014F6E0Local() {}
};

// Construct and throw the container length error with the supplied message.
void RingBuffer_ThrowLengthError(const char* name);

void fn_801801510E8(void* self, unsigned int n);

struct Fn801510E8Buffer {
    void* vtable;
    int a, b;
    char* data;
};
struct Fn801510E8Error {
    void* vtable;
    char* data;
    Fn801510E8Buffer* buffer;
};
struct Fn801510E8Guard {
    char* data;
    char* allocator;
    // Guard a newly allocated character array until its pointer is released.
    Fn801510E8Guard(char* p, char* a) : data(p), allocator(a) {}
    // Free the guarded character array unless ownership was transferred.
    ~Fn801510E8Guard() {
        if (data)
            __dla__FPv(data);
    }
};

#pragma push
#pragma use_lmw_stmw on
// Check the circular-buffer allocation limit and allocate the requested word storage.
void RingBuffer32_AllocateWords(void* self, void* unused, unsigned int n);

#pragma pop

struct Iter512 {
    unsigned int* current;
    unsigned int* begin;
    unsigned int* unused;
    unsigned int* end;
};
struct Data512 {
    unsigned int* data;
    unsigned int count;
    unsigned int free;
};
// Copy an iterator range into the tail, splitting the copy at a wrap boundary.
void RingBuffer32_CopyWrappedRangeToTail(Data512* self, const Iter512* first, const Iter512* last);

struct Ring8014F340 {
    unsigned int unk0;
    unsigned int unk4;
    unsigned int* data;
    unsigned int head;
    unsigned int count;
    unsigned int cap;
    // Return the circular buffer occupied-word count.
    unsigned int size() const {
        return count;
    }
    // Return the allocated circular-buffer word capacity.
    unsigned int size_cap() const {
        return cap;
    }
    // Return the wrapped end index, or zero for an unallocated buffer.
    unsigned int tail() const {
        return cap ? (head + count) % cap : 0;
    }
};

#pragma use_lmw_stmw on
// Destroy the owned container storage and optionally free the object.
void* RingBuffer32_dtor(void* self, int flags);

#pragma use_lmw_stmw reset

#pragma push
#pragma opt_common_subs off
// Build the circular word buffer begin iterator from its head index.
void RingBuffer32_MakeBeginIterator(void* destination, const unsigned int* source);

#pragma pop

// Swap the circular-buffer storage words without changing their evaluation order.
void RingBuffer32_SwapStorage(unsigned int* first, unsigned int* second);

// Move the current iterator backward with wraparound.
void RingIterator32_RetreatInPlace(void* self, int offset);

// Copy the iterator and advance it by a wrapped word offset.
void RingIterator32_AddOffset(unsigned int* destination, const unsigned int* source, int offset);

// Copy an iterator and move its current pointer backward with wraparound.
void RingIterator32_CopyRetreated(unsigned int* destination, const unsigned int* source, int offset);

// Copy the iterator and retreat it by a wrapped word offset.
void RingIterator32_SubtractOffset(unsigned int* destination, const unsigned int* source, int offset);

// Return the iterator current pointer.
void* RingIterator32_GetCurrentPointer(void* self);

// Read the allocation word from the circular-buffer header.
void* RingBuffer32_GetAllocationWord(void* self);

// Return the address of the circular-buffer capacity word.
void* RingBuffer32_GetCapacityAddress(void* self);

// Return the capacity-word address used by the allocation path.
void* RingBuffer32_GetCapacityAddressForAllocation(void* self);

// Read a word through the pointer-shaped interface used by the container helpers.
void* UnsignedWord_LoadPointerValue(void* self);

// Allocate storage for the requested number of 0x54-byte shot-history entries.
void* ShotHistory_AllocateEntries(void* self, int entryCount);

// Return the address of the circular-buffer element count.
void* RingBuffer32_GetCountAddress(void* self);

// Read the circular-buffer count through the existing pointer-shaped return type.
void* RingBuffer32_GetCountWord(void* self);

// Clear the circular-buffer element count while retaining its allocation.
void RingBuffer32_ClearCount(void* self);

// Reset the circular-buffer count using the original pointer-difference expression.
void RingBuffer32_ResetCount(void* self);

// Return the first occupied word using the current circular-buffer head index.
void* RingBuffer32_GetFrontPointer(void* self);

// Swap two integer values.
void Swap_Int32(int* first, int* second);

// Swap two integer storage words.
void Swap_Int32ForRingStorage(int* first, int* second);

// Return the address of the larger unsigned word.
void* UnsignedWord_SelectMaxPointer(void* first, void* second);

// Return the capacity excluding the reserved empty slot, or zero for no allocation.
unsigned int RingBuffer32_GetUsableCapacity(void* self);

struct Fn8014F9C0Ring {
    unsigned int* data;
    unsigned int start, len, cap;
    unsigned int* alloc;
    // Initialize empty temporary ring storage with a borrowed allocator context.
    Fn8014F9C0Ring(unsigned int* a) : data(0), start(0), len(0), cap(0), alloc(a) {}
    // Clear the temporary count and free any remaining owned word allocation.
    ~Fn8014F9C0Ring() {
        if (data) {
            len -= ((char*)this + len) - (char*)this;
            appFreeUni__FPv(data);
        }
    }
};

struct Fn8014F9C0Err {
    void* vtable;
    char* data;
    Fn801510E8Buffer* buffer;
};
#pragma use_lmw_stmw on
#pragma opt_common_subs off
// Reallocate circular-buffer storage and preserve the occupied wrapped ranges.
void RingBuffer32_ReallocateStorage(unsigned int* raw, unsigned int index);

#pragma opt_common_subs reset
#pragma use_lmw_stmw reset

struct Fn8014FD54Base {
    char* vtable;
    char* data;
    Fn8014F6E0Buffer* buffer;
    // Copy the exception message and install its shared character deleter.
    Fn8014FD54Base(const char* name) {
        vtable = __vt__Q23std11logic_error;
        volatile char allocator2 = 0;
        volatile char allocator = 0;
        unsigned int n = strlen(name) + 1;
        char* copy = (char*)__nwa__FUl(n * 2 - n);
        data = copy;
        {
            Fn8014F6E0Guard guard(copy, (char*)&allocator);
            Fn8014F6E0Buffer* p = (Fn8014F6E0Buffer*)__nw__FUl(0x10);
            if (p) {
                p->a = 1;
                p->b = 1;
                p->vtable = s_sharedCharDeleterVtable;
                p->data = copy;
            }
            buffer = p;
            guard.data = 0;
        }
        strcpy(data, name);
    }
    // Preserve the original empty exception-cleanup hook.
    ~Fn8014FD54Base() {}
};
struct Fn8014FD54Error : Fn8014FD54Base {
    // Install the length-error vtable and throw the constructed exception.
    Fn8014FD54Error(const char* name) : Fn8014FD54Base(name) {
        vtable = __vt__Q23std12length_error;
        __throw(golfRuleStrings + kText_lengthErrorTypeName, this, (void*)__dt__Q23std12length_errorFv);
    }
};

extern "C++" {}

struct Fn8014FD54Alloc {
    volatile char c;
};
struct Fn8014FD54Err {
    void* vtable;
    char* data;
    void* buffer;
};
struct Fn8014FD54Ring {
    unsigned int* data;
    unsigned int start, len, cap;
    unsigned int* alloc;
    // Initialize empty temporary ring storage with a borrowed allocator context.
    Fn8014FD54Ring(unsigned int* a) : data(0), start(0), len(0), cap(0), alloc(a) {}
    // Clear the temporary count and free any remaining owned word allocation.
    ~Fn8014FD54Ring() {
        if (data) {
            Fn8014FD54Alloc a;
            a.c = 0;
            len -= ((char*)this + len) - (char*)this;
            appFreeUni__FPv(data);
        }
    }
};

// Count words in one contiguous [first,last) range; callers add separate wrap segments.

// Copy the contiguous word range in the direction required by the caller.

#pragma use_lmw_stmw on
// Append one word, growing and copying wrapped storage when necessary.
void RingBuffer32_AppendWord(unsigned int* self, unsigned int* value);

#pragma use_lmw_stmw reset

// Dereference the first pointer stored in the supplied buffer view.
void* RingBuffer32_GetFirstPointer(void* self);

struct Fn801514E0Guard {
    char* value;
    unsigned char* flag;
    // Guard a newly allocated character array until its pointer is released.
    Fn801514E0Guard(char* p, unsigned char* f) : value(p), flag(f) {}
    // Free the guarded character array unless ownership was transferred.
    ~Fn801514E0Guard() {
        if (value)
            __dla__FPv(value);
    }
};
struct Fn801514E0Ring {
    unsigned int* data;
    unsigned int index, size, capacity;
};
struct Fn801514E0Iter {
    unsigned int* current;
    unsigned int* begin;
    unsigned int* rangeBegin;
    unsigned int* end;
    // Leave iterator words uninitialized for the caller to fill.
    Fn801514E0Iter() {}
    // Construct an iterator from current, base, origin and limit pointers.
    Fn801514E0Iter(unsigned int* c, unsigned int* b, unsigned int* u, unsigned int* e) {
        volatile Fn801514E0Iter* out = this;
        out->current = c;
        out->begin = b;
        out->rangeBegin = u;
        out->end = e;
    }
    // Copy the four wrapped iterator pointers.
    Fn801514E0Iter(const Fn801514E0Iter& x) {
        unsigned int* c = x.current;
        unsigned int* b = x.begin;
        unsigned int* u = x.rangeBegin;
        unsigned int* e = x.end;
        volatile Fn801514E0Iter* out = this;
        out->current = c;
        out->begin = b;
        out->rangeBegin = u;
        out->end = e;
    }
};
// Copy the circular iterator pointer and boundary words.

struct Fn801514E0Error {
    void* vt;
    char* str;
    void* ref;
};

// Count words in one contiguous [first,last) range; callers add separate wrap segments.

// Calculate the wrapped end index of the occupied circular range.

// Build or advance a circular iterator, wrapping at the storage boundary.

// Copy the contiguous word range in the direction required by the caller.

#pragma use_lmw_stmw on
#pragma opt_propagation off
#pragma opt_common_subs off
// Initialize an empty shot-history header and its circular block map.
void* ShotHistory_InitEmptyHeader(void* a);

#pragma opt_propagation reset
#pragma opt_common_subs reset
#pragma use_lmw_stmw reset

// Copy the four-word circular iterator descriptor.
void* RingIterator32_Copy(unsigned int* destination, const unsigned int* source);

// Copy the four iterator words without advancing either iterator.
void RingIterator32_CopyWords(unsigned int* destination, const unsigned int* source);

// Advance the current word pointer, wrapping from the end to the beginning.
void RingIterator32_AdvanceInPlace(void* self);

// Report whether the circular-buffer element count is zero.
int RingBuffer_IsEmpty(void* self);

// Adjust the secondary-interface this pointer and forward to the owning actor method.

// Return the count-word address used by shot-history resizing.
void* RingBuffer32_GetCountAddressForResize(void* self);

// Return the capacity-word address used by shot-history insertion.
void* RingBuffer32_GetCapacityAddressForInsert(void* self);

// Initialize an empty circular-buffer header with the supplied allocation word.
void RingBuffer32_InitializeHeader(void* self, unsigned int value);

#pragma use_lmw_stmw on
// Append a possibly wrapped iterator range to the circular buffer.
void RingBuffer32_AppendIteratorRange(unsigned int* self, const unsigned int* first, const unsigned int* last);

#pragma use_lmw_stmw reset

// Convert occupied map slots (including the spare slot) to checkpoint capacity:
// (mapCount-1)*16, or zero when no slots exist. Not map allocation capacity +14.
unsigned int ShotHistory_GetBlockCapacity(void* self);

// Copy the dialog-owned word range and append the requested value.

extern const char s_roundRewardTables[];

// Round-reward account schema (account stride 0x1D4 in CSharedDoc).
// UpdateRoundRewards writes +44/+1C4 together from the rank reward, +40 from
// the separate bonus table, +48 from player Pang (+CC) and bonus Pang (+D0).
// +3C is the minimum reward CATEGORY seen, not the player's strokes or rank.
// The public labels for +40 and +44 remain unverified: do not call them Pang,
// PP, experience or tickets on this evidence. Their distinct widths, caps and
// producers below are sufficient to follow settlement without guessing a label.
// Safe currency/UI mods still require tracing the save/UI consumers of +40/+44.
// Proven +44 consumer: SetupPlayerSnapshots feeds it to PlayerRank_FindTierForValue
// (pvsfxbox_sky_and_wind_particles.cpp, 56 thresholds); ApplyShotOutcomeModifier uses that tier to derive
// a five-tier grade bucket. It is rank progress, independently of its UI label.
struct RoundRewardAccountView {
    char padToBestRank[0x3C];
    int bestRankRewardIndex;
    short rankTableBonusBalance;
    char pad42[2];
    unsigned int rankRewardBalance, pangBalance;
    char padToSubtotal[0x1C4 - 0x4C];
    unsigned int rankRewardSubtotal;
};
struct RoundRewardPlayerView {
    char padToComputerFlag[4];
    int isComputer;
    char padToRank[0xC4 - 8];
    unsigned char rankBits;
    char padC5[3];
    int matchScore, pang, bonusPang;
    char padToRoundReward[0x164 - 0xD4];
    int roundRankReward, rankRewardIndex;
    short rankTableBonus;
    unsigned char repeatPangAward;
};
enum RoundRewardLimits {
    kMaximumRewardBalance = 99999999,
    kMaximumRewardSubtotal = 999999,
    kMaximumRankTableBonus = 999,
    kFirstRankPangBonus = 500,
    kFullRoundHoles = 18
};
typedef char RoundRewardPlayerBonusOffset[((int)&((RoundRewardPlayerView*)0)->rankTableBonus == 0x16C) ? 1 : -1];
struct BD74Bits {
    CW_BITS2(unsigned char high : 4, unsigned char low : 4)
};
// Select the bounded value while retaining the original operand order.
static inline const int& BD74Max(const int& a, const int& b) {
    return a < b ? b : a;
}
// s_roundRewardTables is a relocation anchor, not an array of this type at its
// +80 scores[rank-1], ranks 1..8: base award * (enemyLevelIndex/2+1), then
//     multiply by roundHoleCount / 18 with integer truncation; writes +164/+44/+1C4.
// +A0 scores2[roundHoleCount-1][rank-1]: 18 rows, two Match ranks (1..2);
//     same difficulty multiplier, no further hole scaling; same destinations.
// +130 values[rank-1], ranks 1..8: signed-short bonus at player +16C/account +40.
// +150 values2[category], categories 0..3: 0 for non-first place, otherwise
//     2 + (opponent matchScore <= roundHoleCount/3); same short destinations.
// Match categories map to reward indices 8/1/0 for category 0/2/3 respectively.
// No bounds checks: callers must supply valid rank, hole count and rule domains.
// Account caps are separate: +44/+48 99,999,999; +1C4 999,999; +40 0..999.
struct BD74Tables {
    char pad[0x80];
    const int scores[8];
    const int scores2[18][2];
    const int values[8];
    const int values2[4];
};
// Field-derived offsets retain the original signed access widths and aliasing.
// Direct struct lvalues change CodeWarrior's register allocation in this routine.
#define ROUND_ACCOUNT_I32(p, f) (*(int*)((unsigned char*)(p) + (int)&((RoundRewardAccountView*)0)->f))
#define ROUND_ACCOUNT_S16(p, f) (*(short*)((unsigned char*)(p) + (int)&((RoundRewardAccountView*)0)->f))
#define BD74_I(p, o) (*(int*)((unsigned char*)(p) + (o)))
#define GolfPlayerEntryWordDoc(n) ((unsigned char*)GolfPlayer_GetEntryByIndex(BD74_I(SharedDocBytes(), 0xB90), (n)))
#define BD74_R(p) (((BD74Bits*)((p) + 0xC4))->high)
#pragma use_lmw_stmw on
// Update the shared round rewards from player scores and the active rules.
void GolfRule_UpdateRoundRewards();

#pragma use_lmw_stmw reset

// Lazily initialize and return the negative front-growth bias.
int ShotHistory_GetFrontGrowthBias(void* self);

// Copy the iterator-range descriptor, including its trailing flag byte.
void* ShotHistory_CopyIteratorRange(void* dst, const void* src);

// Construct a begin iterator from the storage header in the original load order.
void RingBuffer32_MakeBeginIteratorVolatile(unsigned int* dst, unsigned int* src);

// Return the last occupied word, accounting for circular-buffer wraparound.
void* RingBuffer32_GetBackPointer(void* self);

// Empty cleanup guard used by the placement-construction helper.
struct Fn801510B8Local {
    ~Fn801510B8Local() {}
};

// Placement-construct one unsigned word in the supplied output slot.
void RingBuffer32_ConstructWord(void* self, void* out, void* src);

// Advance the circular head and decrement the occupied count.
void RingBuffer32_PopFront(void* self);

// Compute the wrapped distance between two circular word iterators.
int RingIterator32_Distance(const unsigned int* first, const unsigned int* second);

void __dl__FPv(void*);

// Destroy the owned container storage and optionally free the object.
void* RingContainer_dtor(char* self, int flags);

// Destroy the owned container storage and optionally free the object.
void* RingBuffer32_Release(char* self, int flags);

#pragma opt_common_subs off
#pragma opt_propagation off
#pragma opt_lifetimes off
struct Fn8014F874Iterator {
    unsigned int* volatile current, * volatile base;
    unsigned int *begin, *end;
    // Install supplied end/origin/base/current pointers, preserving volatile members.
    Fn8014F874Iterator(unsigned int* d, unsigned int* c, unsigned int* b, unsigned int* a)
        : current(a), base(b), begin(c), end(d) {}
    DECOMP_CT_MEMBER(Fn8014F874Iterator)
};
// Construct an end iterator from the circular head, count and capacity.
void RingBuffer32_MakeEndIterator(unsigned int* destination, const unsigned int* source);

#pragma opt_lifetimes reset
#pragma opt_propagation reset
#pragma opt_common_subs reset

#pragma push
#pragma opt_common_subs off
// Construct the circular end iterator while preserving the original volatile loads.
void RingBuffer32_MakeEndIteratorVolatile(unsigned int* destination, const unsigned int* source);

#pragma pop

typedef NString BgmTrackName;

struct Fn8016BCB4Object {
    char pad[0x42C];
    char list[12];
};

// Build the background-music playlist for the active course and party rules.

// Reset power, impact, Pangya assistance and gauge direction for a new shot.

// Request the exit transition appropriate to the active game mode.

struct CopyIterator {
    int* current;
    int* begin;
    int* middle;
    int* end;
    // Install the supplied end, middle, begin and current iterator pointers.
    CopyIterator(int* d, int* c, int* b, int* a) : current(a), begin(b), middle(c), end(d) {}
};
// Select the bounded value while retaining the original operand order.
inline const int& copy_min(const int& a, const int& b) {
    return a < b ? a : b;
}
// Compare the current pointers of two circular iterators.

// Copy the contiguous word range in the direction required by the caller.

// Copy the contiguous word range in the direction required by the caller.

#pragma use_lmw_stmw on
// Copy a wrapped iterator range forward in contiguous chunks.
CopyIterator RingIterator32_CopyForward(CopyIterator* a, CopyIterator* b, CopyIterator* c);

// Copy a wrapped iterator range backward in contiguous chunks.
CopyIterator RingIterator32_CopyBackward(const CopyIterator* a, CopyIterator* b, CopyIterator* c);

#pragma use_lmw_stmw reset

// Check the current hole and player progress and update ranks when required.

// 0x8016531C: return the input result without materializing the full advance predicate.
// Poll the logical advance action through the lazily initialized input manager.
static inline int GolfRule_PollAdvanceAction() {
    if (!GetInputManagerSlot()) {
        if (!s_inputManagerInitialized) {
            InputManager_Init(s_gameInputManager);
            __register_global_object(s_gameInputManager, (void*)InputManager_Dtor,
                                     (void*)s_golfRuleInputDestructorRecord);
            s_inputManagerInitialized = 1;
        }
        GetInputManagerSlot() = s_gameInputManager;
    }
    return Input_IsActionPressed(GetInputManagerSlot(), kActionAdvance, kSelectedInputChannel);
}

// Poll screen replies and logical input to advance a view or dismiss its notice.

// Poll the screen and logical input for a camera-view transition.

#pragma use_lmw_stmw on
// Calculate a larger circular-buffer allocation without exceeding the word limit.
unsigned int RingBuffer32_CalculateGrowth(void* self, unsigned int requested);

#pragma use_lmw_stmw reset

#pragma opt_propagation off
#pragma opt_common_subs off
#pragma opt_lifetimes off
struct RingValue {
    unsigned int data;
};
struct RingBase {
    RingValue* first;
    RingValue* begin;
    RingValue* end;
    RingValue* capacity;
};
struct RingIt {
    RingBase b;
    unsigned char flag;
    // Leave iterator state uninitialized until supplied by the caller.
    RingIt() {}
    // Copy the iterator pointer words and allocator-state byte.
    RingIt(const RingIt& o) : b(o.b), flag(o.flag) {}
    // Replace iterator state without moving or owning any elements.
    RingIt& operator=(const RingIt& o) {
        b = o.b;
        flag = o.flag;
        return *this;
    }
};
// Count words in one contiguous [first,last) range; callers add separate wrap segments.

// Build or advance a circular iterator, wrapping at the storage boundary.

// Copy shot-history blocks between wrapped source and destination iterators.
void ShotHistory_CopyWrappedBlocks(RingBase* result, RingIt* source, RingIt* last, RingIt* dest);

#pragma opt_common_subs reset
#pragma opt_propagation reset
#pragma opt_lifetimes reset

// Update the swing-controller camera interface and leave it when input requests exit.

// Create the fight result or retry menu appropriate to the round outcome.
#define CTL ((int)(*(unsigned char**)&AsCSharedDoc(SharedDocBytes())->golfDoc))

#pragma use_lmw_stmw on
struct Size530 {
    int used;
    // Exclude the reserved map slot from the stored slot count.
    unsigned int capacity() const {
        if (used == 0)
            return 0;
        return (used - 1);
    }
};
// Copy a borrowed world position to eligible players and clear their shot histories.
// Legacy symbol name is misleading: position points to three floats, never text.
// Player status 3 is skipped; +0x10 and +0x1C both receive the same WVector.
#define P530 ((unsigned char*)GolfPlayer_GetEntryByIndex((int)GolfDocBytes(), (unsigned char)i))
#pragma use_lmw_stmw reset

extern float kFloat_0_5_GolfRule, kFloat_200_GolfRule, kFloat_1_805DEA38;
extern unsigned int* g_mouseCursor;
extern unsigned int* g_soundManager;
int NamedEffectList_FindByName(void*, const char*);
int WPuppet_SetOpacityRecursive(int, int);
int Sound_StopByAlias(void*, const char*);
int Sound_PlayByName(void*, const char*, int, int, int, int, float, float, float);

extern float kFloat_149_805DEA58, kFloat_360_GolfRule, kFloat_509_GolfRule, kGolfRuleComparisonEpsilon;
extern float kFloat_30_GolfRule, kFloat_509_ShotGauge, kGolfSimulationStepSeconds, kFloat_0_03_GolfRule,
    kFloat_114_GolfRule;
extern const char* shotTimingSoundNames[];
int Sound_PlayByName(void*, const char*, int, int, int, int, float, float, float);
int GolfTask_NoOpStatus();
void* InstanceTable_Get();

// Compute the absolute magnitude while preserving the original floating-point behavior.
static inline float abs164(float x) {
    union {
        float f;
        unsigned int u;
    } c;
    c.f = x;
    return (c.u & 0x80000000) ? -x : x;
}
// Compare floating-point values using the original ordered comparison and tolerance.
static inline bool near164(float a, float b) {
    return abs164(a - b) < kGolfRuleComparisonEpsilon;
}

// Update the single-stage gauge mode and commit power or reset an expired swing.
void CGolfRule_UpdateSingleStageGauge(void* self, int swingPressed);

// Finish the player preview, initialize aim and gauge state, then enable turn input.

extern float kFloat_11_GolfRule;
// Play the committed button-shot animation and advance to ball flight after its delay.

extern float kFloat_11_GolfRule;
// Play the committed motion-shot animation and advance to ball flight after its delay.

#pragma opt_common_subs off
#pragma opt_propagation off
#pragma opt_lifetimes off
// Update course-clear status after applying the current course score to a copied save record.
int GolfRule_UpdateCourseClearStatus();

#pragma opt_common_subs reset
#pragma opt_propagation reset
#pragma opt_lifetimes reset

// Clear item-request state and refresh the first-player request context.
void GolfRule_ResetItemRequestState(void* unused, void* arg);
#define FirstGolfPlayerEntryFromShared(g) \
    ((unsigned char*)((void* (*)(int, int))GolfPlayer_GetEntryByIndex)(*(int*)((g) + 0xB90), 0))

// Point the shot heading from the ball toward the current hole target.

#define GolfPlayerEntryAbi(s) \
    ((unsigned char*)((void* (*)(unsigned char*, int))GolfPlayer_GetEntryByIndex)(GolfDocBytes(), (s)))
#pragma use_lmw_stmw on
#pragma opt_propagation off
#pragma opt_common_subs off
#pragma opt_lifetimes off
#pragma opt_strength_reduction off
// Select the next player or round-result state according to player completion and rules.

#pragma opt_propagation reset
#pragma opt_common_subs reset
#pragma opt_lifetimes reset
#pragma opt_strength_reduction reset
#pragma use_lmw_stmw reset

void __ct__11CResultMenuFPv(void*, int);
void* memset(void*, int, unsigned int);
int SharedStringState_GetShortValue();
void SoundManager_RequestBgmChange(void*, const char*, int, int);
struct GolfResultMenuPayload;
void SetResultData__11CResultMenuFP14ResultMenuData(void*, GolfResultMenuPayload*);

// Create and initialize the result menu for the active game mode.
#define GolfPlayerEntryAbiByte(s) \
    ((unsigned char*)((void* (*)(unsigned char*, int))GolfPlayer_GetEntryByIndex)(GolfDocBytes(), (unsigned char)(s)))

extern const float kFloat_354_2_GolfRule;
extern const float kFloat_437_GolfRule;
extern void* s_heldDirectionShotCommandMask;
extern void* s_conflictingShotDirectionsCommand;
int OS_GetTimeMilliseconds();

struct Fn63Guard {
    void* head;
    void* node;
    // Guard a newly allocated command node until it is linked.
    Fn63Guard(void* h, void* n) : head(h), node(n) {}
    // Free an unlinked command node if construction exits early.
    ~Fn63Guard() {
        if (node)
            appFreeUni__FPv(node);
    }
};
// Queue one borrowed command word in GolfDoc's circular doubly linked list.
// Header: +9C count, +A0 sentinel.next, +A4 sentinel.previous. Each allocated
// 12-byte node is [next, previous, command word]. head is the sentinel address;
// *head is the old first node. The four stores below splice BEFORE that first
// node (the consumer determines processing order); they also work when empty.
// Sequence: allocate -> guard -> construct payload -> release guard -> splice
// -> increment count. No callbacks occur after guard release, and the payload
// is a scalar pointer-shaped word with no destructor. If construction unwinds,
// Fn63Guard frees the unlinked node and the queue/count have not been written.
// Allocation failure has no local null check; this does not promise recovery.
// The queue owns node storage until T_CLEAR/command removal calls appFreeUni;
// it does not own a pointee encoded by value. value is evaluated once, after
// allocation. Caller supplies its scope (including any if/else braces).
// allocator and iter are retained volatile ABI scaffolding, not queue state.
enum ShotCommandNodeSize {
    kShotCommandNodeBytes = 12
};
// These lvalues preserve the byte-pointer representation of [next,previous,word].
// The sentinel has links only; never request its payload address.
#define SHOT_COMMAND_NEXT(p) (*(unsigned char**)(p))
#define SHOT_COMMAND_PREVIOUS(p) (*(unsigned char**)((p) + 4))
#define SHOT_COMMAND_PAYLOAD_ADDRESS(p) ((p) + 8)
#define INSERT63(value)                                                               \
    volatile char allocator[4];                                                       \
    allocator[0] = 0;                                                                 \
    unsigned char* owner = (*(unsigned char**)&AsCSharedDoc(g_sharedDoc)->golfDoc);   \
    unsigned char* head = owner + 0xA0;                                               \
    unsigned char* node = (unsigned char*)appMallocDummy__FUl(kShotCommandNodeBytes); \
    volatile struct {                                                                 \
        unsigned char* first;                                                         \
        unsigned char* second;                                                        \
    } iter;                                                                           \
    iter.first = node;                                                                \
    iter.second = node;                                                               \
    Fn63Guard guard(head, node);                                                      \
    new SHOT_COMMAND_PAYLOAD_ADDRESS(node) void*((void*)(value));                     \
    guard.node = 0;                                                                   \
    SHOT_COMMAND_PREVIOUS(SHOT_COMMAND_NEXT(head)) = node;                            \
    SHOT_COMMAND_NEXT(node) = SHOT_COMMAND_NEXT(head);                                \
    SHOT_COMMAND_NEXT(head) = node;                                                   \
    SHOT_COMMAND_PREVIOUS(node) = head;                                               \
    ++*(int*)(owner + 0x9C)

// Record directional shot commands, including held-direction timing and conflicting inputs.

struct Fn801505E8Ring {
    unsigned int* data;
    unsigned int head;
    unsigned int count;
    unsigned int cap;
    unsigned int alloc;
};

#pragma use_lmw_stmw on
#pragma dont_inline on
// Insert an iterator range into shot history, growing storage and moving wrapped ranges.
void ShotHistory_InsertIteratorRange(void* self, unsigned int* insertionPosition, void* sourceFirst, void* sourceLast,
                                     void* allocatorContext);

#pragma dont_inline reset
#pragma use_lmw_stmw reset

// Test whether the current player's selected item matches inventory slot id.
static inline int fn_8016E124_is(int id) {
    unsigned char* layout = (unsigned char*)&AsCSharedDoc(g_sharedDoc)->itemInventory;
    unsigned char* control = (*(unsigned char**)&AsCSharedDoc(g_sharedDoc)->golfDoc);
    unsigned char* player = (unsigned char*)GolfPlayer_GetEntryByIndex((int)control, control[0x3C1]);
    return *(unsigned int*)(player + 0xA0) == (unsigned int)ItemInventory_GetItemIdAt(layout, id);
}

// Calculate the equipped-item adjustment used by impact assistance.
static inline float fn_8016E124_step(int adjust) {
    unsigned char* layout;
    int repeat = 0;
    float step = 2.0f * (float)s_pangyaZoneBaseWidthUnits;
    layout = (unsigned char*)&AsCSharedDoc(g_sharedDoc)->itemInventory;
    unsigned char* control = (*(unsigned char**)&AsCSharedDoc(g_sharedDoc)->golfDoc);
    unsigned char* player = (unsigned char*)GolfPlayer_GetEntryByIndex((int)control, control[0x3C1]);
    // Item tests at hit share the following adjustment.
    if (*(unsigned int*)(player + 0xA0) == (unsigned int)ItemInventory_GetItemIdAt(layout, 9) ||
        (layout = (unsigned char*)&AsCSharedDoc(g_sharedDoc)->itemInventory,
         control = (*(unsigned char**)&AsCSharedDoc(g_sharedDoc)->golfDoc),
         player = (unsigned char*)GolfPlayer_GetEntryByIndex((int)control, control[0x3C1]),
         *(unsigned int*)(player + 0xA0) == (unsigned int)ItemInventory_GetItemIdAt(layout, 1))) {
        repeat = 1;
        step *= 2.0f;
    }
    if (adjust) {
        step *= AsCGolfDoc((*(unsigned char**)&AsCSharedDoc(g_sharedDoc)->golfDoc))->pangyaZoneScale;
        if ((*(int*)&AsCSharedDoc(g_sharedDoc)->partyGameMode) == 3) {
            unsigned char* anim = (unsigned char*)PartyItemSlots_GetInstance();
            if (!repeat)
                step *= 1.0f + *(float*)(anim + 0x18);
            step /= *(float*)(anim + 0x3C);
        }
    }
    return step;
}

// Calculate the equipped-item adjustment used by impact assistance.
static inline float fn_8016E124_step2(int adjust) {
    int repeat = 0;
    float step = 2.0f * (float)s_pangyaZoneBaseWidthUnits;
    unsigned char* layout = (unsigned char*)&AsCSharedDoc(g_sharedDoc)->itemInventory;
    unsigned char* control = (*(unsigned char**)&AsCSharedDoc(g_sharedDoc)->golfDoc);
    unsigned char* player = (unsigned char*)GolfPlayer_GetEntryByIndex((int)control, control[0x3C1]);
    // Item tests at hit share the following adjustment.
    if (*(unsigned int*)(player + 0xA0) == (unsigned int)ItemInventory_GetItemIdAt(layout, 9) ||
        (layout = (unsigned char*)&AsCSharedDoc(g_sharedDoc)->itemInventory,
         control = (*(unsigned char**)&AsCSharedDoc(g_sharedDoc)->golfDoc),
         player = (unsigned char*)GolfPlayer_GetEntryByIndex((int)control, control[0x3C1]),
         *(unsigned int*)(player + 0xA0) == (unsigned int)ItemInventory_GetItemIdAt(layout, 1))) {
        repeat = 1;
        step *= 2.0f;
    }
    if (adjust) {
        step *= AsCGolfDoc((*(unsigned char**)&AsCSharedDoc(g_sharedDoc)->golfDoc))->pangyaZoneScale;
        if ((*(int*)&AsCSharedDoc(g_sharedDoc)->partyGameMode) == 3) {
            unsigned char* anim = (unsigned char*)PartyItemSlots_GetInstance();
            if (!repeat)
                step *= 1.0f + *(float*)(anim + 0x18);
            step /= *(float*)(anim + 0x3C);
        }
    }
    return step;
}

// Apply equipped-item impact assistance while retaining the unassisted gauge result.

// Restore the shot-start ball position and rebuild aiming, gauge and presentation state.

int GetGroundType__9CQuadTreeF7WVector(void*, void*);

// 0x80165980 / 0x801659B8: refresh the player and document in their original order.
// Read the current player hole strokes through the selected rule-mode layout.
static inline unsigned char GolfRule_ReadPlayerHoleStrokes(unsigned char*& manager, unsigned char*& control) {
    control = (*(unsigned char**)&AsCSharedDoc(manager)->golfDoc);
    void* o = GolfPlayer_GetEntryByIndex((int)control, (control[0x3C1]));
    manager = (unsigned char*)g_sharedDoc;
    control = (*(unsigned char**)&AsCSharedDoc(manager)->golfDoc);
    unsigned char* oo = (unsigned char*)o;
    return oo[(control[0x3B6]) + 0x28];
}
// Read the current player hole strokes through the selected rule-mode layout.
static inline unsigned char GolfRule_ReadPlayerHoleStrokesMode1(unsigned char*& manager, unsigned char*& control) {
    control = (*(unsigned char**)&AsCSharedDoc(manager)->golfDoc);
    unsigned char cidx = control[0x3C1];
    void* o = GolfPlayer_GetEntryByIndex((int)control, cidx);
    control = (*(unsigned char**)&AsCSharedDoc(manager)->golfDoc);
    manager = (unsigned char*)g_sharedDoc;
    unsigned char idx2 = control[0x3B6];
    unsigned char* oo = (unsigned char*)o;
    unsigned char val = oo[idx2 + 0x28];
    return val;
}

// Accumulate the completed shot into the active player statistics.
void GolfRule_AccumulateCurrentShotStatistics();

typedef Vec3f Fn8016AA98Vec;
struct Fn8016AA98Words {
    int v[3];
};
extern float kFloat_0_7853982_GolfRuleCamera, kFloat_16_GolfRule, kFloat_9_6_GolfPlayerHelpers,
    kFloat_neg0_7853982_GolfRule;
void GetGroundPoint__9CQuadTreeF7WVectorbPib(void*, void*, const void*, int, int, int);
// Copy the shot-playback offset vector.
inline void CopyShotSampleVelocity(void* dest, void* source) {
    ((ShotSample*)dest)->velocity = ((ShotSample*)source)->velocity;
}
struct ShotWorldClockFields {
    char pad[0x21A8];
    float fractionalSeconds;
    int frame;
};
// Set the playback controller time while clearing its fractional frame.
inline void SetShotWorldClockFrame(void* controller, int time) {
    ((ShotWorldClockFields*)controller)->frame = time;
    ((ShotWorldClockFields*)controller)->fractionalSeconds = kFloat_0_GolfRule;
}

// Set up playback of the final thirty simulated frames with the appropriate camera.

typedef Vec3f Fn8016A4C0Vec;

#define GolfPlayerEntryByte(i) ((unsigned char*)GolfPlayer_GetEntryByIndex((int)GolfDocBytes(), (unsigned char)(i)))
extern "C" {
// Select the next eligible player according to turn order, distance and game rules.
}

// Restore the first simulation sample and set up ball-flight camera and timing.

struct Fn8016C2B8Storage {
    unsigned int* data;
    unsigned int size, capacity;
    unsigned int* allocator;
    unsigned int offset;
    // Initialize temporary vector storage with the supplied allocator context.
    Fn8016C2B8Storage(unsigned int* a) : data(0), size(0), capacity(0), allocator(a), offset(0) {}
    // Clear the temporary size and free its remaining owned backing allocation.
    ~Fn8016C2B8Storage() {
        if (data) {
            size = 0;
            appFreeUni__FPv(data);
        }
    }
};

struct Fn8016C2B8Allocator {
    volatile char c;
    char pad[3];
};
struct Fn8016C2B8Error {
    char* vtable;
    char* data;
    Fn8014F6E0Buffer* buffer;
    // Copy the exception message and install its shared character deleter.
    Fn8016C2B8Error(const char* name) {
        vtable = __vt__Q23std11logic_error;
        Fn8016C2B8Allocator allocator;
        allocator.c = 0;
        unsigned int n = strlen(name) + 1;
        char* copy = (char*)__nwa__FUl(n * 2 - n);
        data = copy;
        {
            Fn8014F6E0Guard guard(copy, (char*)&allocator.c);
            Fn8014F6E0Buffer* p = (Fn8014F6E0Buffer*)__nw__FUl(0x10);
            if (p) {
                p->a = 1;
                p->b = 1;
                p->vtable = s_sharedCharDeleterVtable;
                p->data = copy;
            }
            buffer = p;
            guard.data = 0;
        }
        strcpy(data, name);
    }
    // Preserve the original empty exception-cleanup hook.
    ~Fn8016C2B8Error() {}
};
struct Fn8016C2B8LengthError : Fn8016C2B8Error {
    // Install the length-error vtable and throw the constructed exception.
    Fn8016C2B8LengthError(const char* name) : Fn8016C2B8Error(name) {
        vtable = __vt__Q23std12length_error;
        __throw(golfRuleStrings + kText_lengthErrorTypeName, this, (void*)__dt__Q23std12length_errorFv);
    }
};

#define FN8016C2B8_ERROR()                                            \
    const char* source = golfRuleStrings + kText_vector_length_error; \
    Fn8016C2B8Error error(source);                                    \
    error.vtable = __vt__Q23std12length_error;                        \
    __throw(golfRuleStrings + kText_lengthErrorTypeName, &error, (void*)__dt__Q23std12length_errorFv)

#pragma use_lmw_stmw on
// Queue the shared shot-effect sequence resources.
int GolfRule_PreloadEffectSequences();

#pragma use_lmw_stmw reset

// Return the magnitude using the original sign-bit test, including signed zero.
static inline float ButtonSwingAbs(const float& value) {
    return (*(const unsigned int*)&value & 0x80000000U) ? -value : value;
}
// Test button-swing magnitude against the shared comparison tolerance.
static inline int ButtonSwingNearZero(const float& value) {
    int result = ButtonSwingAbs(value) < kGolfRuleComparisonEpsilon;
    return result;
}
// Select the smaller gauge value with the original ordered comparison.
static inline float ButtonSwingMinimum(float incoming, float previous) {
    return previous > incoming ? incoming : previous;
}
// Poll button swing input, update the selected gauge mode and round the displayed cursor.

// Virtual ABI view: CPadSwingCtrl::interfaceVtable is at +0x58; slots stay in game order.
struct SwingInterfacePrefix {
    char pad[0x58];
};
struct PlayerSwingInterface {
    virtual void v08();
    virtual void v0c();
    virtual void v10();
    virtual void v14();
    virtual void v18();
    virtual void v1c();
    virtual void v20();
    virtual void v24();
    virtual void v28();
    virtual void v2c();
    virtual void v30();
    virtual void v34();
    virtual void v38();
    virtual void v3c();
    virtual void v40();
    virtual void v44();
    virtual void v48();
    virtual void v4c();
    virtual void v50();
    virtual void v54();
    virtual void v58();
    virtual void v5c();
    virtual void v60();
    virtual void v64();
    virtual void v68();
    virtual void v6c();
    virtual void v70();
    virtual void v74();
    virtual void v78();
    virtual void v7c();
    virtual void v80();
    virtual void v84();
    virtual void v88();
    virtual void v8c();
    virtual void v90();
    virtual void v94();
    virtual void v98();
    virtual void v9c();
    virtual void va0();
    virtual void va4();
    virtual void va8();
    virtual void SetSwingAnimationState(int);
    virtual void vb0();
    virtual void vb4();
    virtual void vb8();
    virtual void vbc();
    virtual void vc0();
    virtual void vc4();
    virtual void vc8();
    virtual void SetSwingMotionHandle(void*);
};
struct PadSwingInterface : SwingInterfacePrefix {
    virtual void v08();
    virtual void v0c();
    virtual int UpdateSwing(float, void*, void*, int);
    virtual void v14();
    virtual void v18();
    virtual void v1c();
    virtual void v20();
    virtual void v24();
    virtual void* GetPlayerSwingHandle();
    virtual void v2c();
    virtual int QueryShotState();
    virtual void v34();
    virtual void v38();
    virtual void v3c();
    virtual void v40();
    virtual void v44();
    virtual int QueryAutoShotReady();
};
// Consume Wii Remote swing-controller results, commit hits and manage pointer input and rumble.

// Advance the course-reload task and restore camera, player and loading-screen state.

void* ItemInventory_FindBallData(void*, void*);
void* memcpy(void*, const void*, unsigned int);

extern unsigned int s_roundPlayerList[];

struct Fn8016D08CList {
    unsigned int* data;
    unsigned int size, capacity;
};

#define FN8016D08C_ERROR()                                            \
    const char* source = golfRuleStrings + kText_vector_length_error; \
    Fn8016C2B8LengthError error(source)

#pragma use_lmw_stmw on
#pragma inline_depth(8)
#pragma inline_max_size(10000)
#pragma inline_max_total_size(100000)
// Grow the preload-task pointer vector and retain its existing entries.
inline void Fn8016D08CGrow(Fn8016D08CList* list, const unsigned int& value) {
    Fn8016C2B8Storage tmp(&list->capacity);
    unsigned int needed = list->size + 1 - list->capacity;
    unsigned int oldcap = list->capacity;
    unsigned int capacity;
    unsigned int growthA, growthB;
    if (needed > 0x3FFFFFFFU - oldcap) {
        FN8016D08C_ERROR();
    }
    if (oldcap < 0x15555555U) {
        growthA = ((oldcap + 1) * 3) / 5;
        capacity = oldcap + *(growthA < needed ? &needed : &growthA);
    } else if (oldcap < 0x2AAAAAAAU) {
        growthB = (oldcap + 1) / 2;
        capacity = oldcap + *(growthB < needed ? &needed : &growthB);
    } else
        capacity = 0x3FFFFFFF;
    if (capacity > 0x3FFFFFFFU) {
        FN8016D08C_ERROR();
    }
    unsigned int* data = (unsigned int*)appMallocDummy__FUl(capacity * 4);
    tmp.offset = list->size;
    unsigned int tmpSize = tmp.size + 1;
    tmp.capacity = capacity;
    (data + tmp.offset)[tmp.size] = value;
    unsigned int* old = list->data;
    unsigned int* end = old + list->size;
    tmp.data = data;
    tmp.size = tmpSize;
    unsigned int count = ((int)end - (int)old) / 4;
    tmp.offset -= count;
    memcpy(data + tmp.offset, old, count * 4);
    memset(old, 0, count * 4);
    unsigned int newSize = tmp.size + count;
    tmp.size = newSize;
    unsigned int* oldData = list->data;
    unsigned int* newData = tmp.data;
    unsigned int oldCapacity = list->capacity;
    list->size = 0;
    tmp.data = oldData;
    list->capacity = tmp.capacity;
    tmp.capacity = oldCapacity;
    list->data = newData;
    list->size = newSize;
    tmp.size = 0;
}

// Append one preload-task pointer, growing the vector when it is full.
inline void Fn8016D08CPush(Fn8016D08CList* list, const unsigned int& value) {
    unsigned int size = list->size;
    unsigned int cap = list->capacity;
    if (size < cap) {
        unsigned int next = size + 1;
        unsigned int* data = list->data;
        list->size = next;
        (data + next)[-1] = value;
    } else {
        if (0x3FFFFFFFU - cap < 1U) {
            const char* source = golfRuleStrings + kText_vector_length_error;
            Fn8016C2B8LengthError error(source);
        }
        Fn8016D08CGrow(list, value);
    }
}

// Queue effect resources for each equipped ball.

#pragma use_lmw_stmw reset
#pragma inline_depth reset
#pragma inline_max_size reset
#pragma inline_max_total_size reset

struct ShotUpdateActor {
    virtual void v0();
    virtual void v1();
    virtual void v2();
    virtual void v3();
    virtual void v4();
    virtual void v5();
    virtual void update(float);
    virtual void v7();
    virtual void v8();
    virtual void v9();
    virtual void v10();
    virtual void v11();
    virtual void v12();
    virtual void v13();
    virtual void v14();
    virtual void v15();
    virtual void caddie();
};

// 0x8015D800: lazily poll the current player's shot-camera action.
// Poll the logical swing action used by the ball-flight camera.
static inline int GolfRule_PollShotCameraAction12() {
    if (GetInputManagerSlot() == 0) {
        if (!s_inputManagerInitialized) {
            InputManager_Init(s_gameInputManager);
            __register_global_object(s_gameInputManager, InputManager_Dtor, s_golfRuleInputDestructorRecord);
            s_inputManagerInitialized = 1;
        }
        GetInputManagerSlot() = s_gameInputManager;
    }
    return Input_IsActionButtonDown(GetInputManagerSlot(), kActionSwing, kSelectedInputChannel);
}

// Advance ball-flight playback and handle camera input and completed-shot transitions.

void Vec3_Zero(void*);
int SoundManager_IsAliasPrefixOrStreamStemActive(void*, const char*);
void __ml__FRC7WVectorf(void*, const void*, float);
unsigned long long CursorVec2_Scale(void*, float);
unsigned long long Vec2_Add(void*, void*);
void Vec2_Copy(void*, const void*);
int Map_FindByKey(void*, int);
int Pointer_IsActionActive(void*, const char*, int);
void Matrix_ExtractUniformScaleNormalizedRotation(void*, void*);
extern int s_powerSpinEffectId;
extern unsigned char lbl_805DC7F1;
extern unsigned int lbl_805DC7F4;
extern void* g_inputPointer;
struct EPlaybackPosition {
    float v[3];
};

// Advance the ball sample index, update its displayed transform and handle playback limits.
// delta is seconds; samples are spaced 1/30 second. The local ballBytes is the
// shared CGolfBall, while E_PL() accesses the actual current player snapshot.
// Ball offsets: +64 highest sample, +68 playback cursor; sample offsets: +00
// position, +0C velocity, +18 rotation, +60 event kind, +61 effect subtype,
// +70 physics state. Event sample indices in the ball schedule one-shot notices.
// payload0 for effect IDs 8/5 is the sample INDEX, not a pointer to the sample.
// Event handling clears special-shot bits 0x70; the rest of m_flags survives.
// Effect-message protocol by sample subtype: 1 -> ID 7 (no payload),
// 2 -> ID 8 (sample index, only if speed>30), 3 -> ID 5 (sample index),
// 4 -> ID 10 (no payload). Kind 2 also sends gallery ID 6; kind 5 creates
// Kind 5 creates a booster-warp effect; message IDs belong to their recipients.
#define E_I(p, o) (*(int*)((char*)(p) + (o)))
#define E_F(p, o) (*(float*)((char*)(p) + (o)))
#define CurrentGolfPlayerEntry() \
    ((unsigned char*)GolfPlayer_GetEntryByIndex((int)GolfDocBytes(), AsCGolfDoc(GolfDocBytes())->currentPlayer))
#define GolfPlayerEntry(i) ((unsigned char*)GolfPlayer_GetEntryByIndex((int)GolfDocBytes(), (i)))
// Advance one player slot while preserving the current-hole snapshot order.
static inline void F8168_Advance(unsigned char index) {
    if (index < AsCGolfDoc(GolfDocBytes())->playerCount && *(int*)(GolfPlayerEntry(index) + 8) != 3) {
        unsigned char* first = GolfPlayerEntry(index);
        *(int*)(first + 0xA0) = 0;
        unsigned char slot = AsCGolfDoc(GolfDocBytes())->currentHoleIndex;
        if (*(int*)(GolfPlayerEntry(index) + 0xD4) == slot) {
            if (!GolfDocBytes()[0x35C]) {
                unsigned char s2 = AsCGolfDoc(GolfDocBytes())->currentHoleIndex;
                unsigned char* a = GolfPlayerEntry(index);
                unsigned char* b = GolfPlayerEntry(index);
                b[0xC5] += *(a + s2 + 0x28);
                if (AsCSharedDoc(SharedDocBytes())->golfRuleType != 1) {
                    unsigned char s3 = AsCGolfDoc(GolfDocBytes())->currentHoleIndex;
                    unsigned char* c = GolfPlayerEntry(index);
                    unsigned char* ctl = GolfDocBytes();
                    unsigned char* rec = *(unsigned char**)(ctl + 0x2BC) + ctl[0x3B6] * 32;
                    *(int*)((unsigned char*)GolfPlayer_GetEntryByIndex((int)ctl, index) + 0xC8) +=
                        (int)*(c + s3 + 0x28) - (int)rec[0x18];
                }
            }
            ++*(int*)(GolfPlayerEntry(index) + 0xD4);
        }
        unsigned char* last = GolfPlayerEntry(index);
        *(int*)(last + 8) = 1;
        unsigned char* mgr = SharedDocBytes();
        if (*(unsigned int*)(mgr + 0x808) == 6)
            *(int*)(mgr + 0xAF0) = PartyGame_EvaluatePlayerThresholdTier(index);
    }
}
// Finalize shot statistics and presentation, then choose the next player/result state.

#if 0
// Retain the disabled earlier shot-finalization implementation for reference.
void fn_80168898_old(unsigned char* self) {
    unsigned char* object;
    unsigned char* control;
    unsigned char index;
    unsigned char slot;
    unsigned char target;
    int value;
    *(unsigned char*)(self + 0x90) = 0;
    *(unsigned char*)(self + 0x92) = 0;
    *(unsigned char*)(self + 0x94) = 0;
    // Phase 1: finish rewards/score/NPC presentation and stop flight sounds.
    IActor_DispatchMessage(self, self, golfRuleStrings + kText_screen, 0x17, 0, 0, 0, 0);
    IActor_DispatchMessage(self, self, golfRuleStrings + kText_score, 5, 0, 0, 0, 0);
    IActor_DispatchMessage(self, self, golfRuleStrings + kText_npcActor, 0, 0, 0, 0, 0);
    object = (unsigned char*)GolfBall__Fv();
    control = (*(unsigned char**)&AsCSharedDoc(g_sharedDoc)->golfDoc);
    if (*(int*)(control + 0x358) == 0x40) {
        if (*(int*)(object + 0x88) > 0) IActor_DispatchMessage(self, self, golfRuleStrings + kText_ball, 3, 0, 0, 0, 0);
        IActor_DispatchMessage(self, self, golfRuleStrings + kText_screen, 7, 0, 0, 0, 0);
        IActor_DispatchMessage(self, self, golfRuleStrings + kText_spin, 4, 0, 0, 0, 0);
    }
    WPuppet_SetOpacityRecursive(NamedEffectList_FindByName(g_sceneManager, golfRuleStrings + kText_ballSequence), 0xFF);
    Sound_StopByAlias(g_soundManager, golfRuleStrings + kText_shot_dist_count);
    ((CCameraWork*)g_cameraWork)->StopBallSound();
    // Phase 2: commit statistics, then resolve hole-in and the next turn state.
    GolfRule_AccumulateCurrentShotStatistics();
    if (((*(unsigned int*)&AsCGolfBall(object)->m_holeInCrypt[kHoleInEncodedWord]) ^ (*(unsigned int*)&AsCGolfBall(object)->m_holeInCrypt[kHoleInKeyWord])) == 0xFFFFFFFF) {
        control = (*(unsigned char**)&AsCSharedDoc(g_sharedDoc)->golfDoc);
        index = control[0x3C1];
        if (index < control[0x2B9] && *(int*)((char*)GolfPlayer_GetEntryByIndex((int)control, index) + 8) != 3) {
            *(int*)((char*)GolfPlayer_GetEntryByIndex((int)control, index) + 0xA0) = 0;
            slot = control[0x3B6];
            object = (unsigned char*)GolfPlayer_GetEntryByIndex((int)control, index);
            if (*(int*)(object + 0xD4) == slot) {
                if (!control[0x35C]) {
                    *(unsigned char*)((char*)GolfPlayer_GetEntryByIndex((int)control, index) + 0xC5) += *((unsigned char*)GolfPlayer_GetEntryByIndex((int)control, index) + slot + 0x28);
                    if (AsCSharedDoc(g_sharedDoc)->golfRuleType != 1) {
                        object = (unsigned char*)GolfPlayer_GetEntryByIndex((int)control, index);
                        *(int*)(object + 0xC8) += (int)object[0x28 + slot] - (int)*(unsigned char*)(*(unsigned int*)(control + 0x2BC) + slot * 32 + 0x18);
                    }
                }
                ++*(int*)((char*)GolfPlayer_GetEntryByIndex((int)control, index) + 0xD4);
            }
            *(int*)((char*)GolfPlayer_GetEntryByIndex((int)control, index) + 8) = 1;
            if ((*(int*)&AsCSharedDoc(g_sharedDoc)->gameMode) == 6) (*(int*)&AsCSharedDoc(g_sharedDoc)->partyResultTier) = PartyGame_EvaluatePlayerThresholdTier(index);
        }
        ((CGolfRule*)self)->EnterState(kStateDelayedTransition);
        GolfPlayer_GetEntryByIndex(control[0x3C1]);
    } else {
        if (object[0x58]) {
            control = (*(unsigned char**)&AsCSharedDoc(g_sharedDoc)->golfDoc);
            object = (unsigned char*)GolfPlayer_GetEntryByIndex(control[0x3C1]); ++object[0xBC];
            slot = control[0x3B6];
            object = (unsigned char*)GolfPlayer_GetEntryByIndex(control[0x3C1]); ++object[slot + 0x28];
            if (SharedDoc_IsPlayerFlag0Set(control[0x3C1])) ((CRival*)g_rival)->SetShotPositionCandidateContext();
        } else {
            if (((*(unsigned int*)&AsCGolfBall(object)->m_holeInCrypt[kHoleInEncodedWord]) ^ (*(unsigned int*)&AsCGolfBall(object)->m_holeInCrypt[kHoleInKeyWord])) == 0xFFFFFFFF && !object[0xAC]) {
                control = (*(unsigned char**)&AsCSharedDoc(g_sharedDoc)->golfDoc);
                object = (unsigned char*)GolfPlayer_GetEntryByIndex(control[0x3C1]); ++object[control[0x3B6] + 0x28];
            }
            control = (*(unsigned char**)&AsCSharedDoc(g_sharedDoc)->golfDoc);
            object = (unsigned char*)GolfPlayer_GetEntryByIndex(control[0x3C1]); object[0xBC] = 0;
        }
        control = (*(unsigned char**)&AsCSharedDoc(g_sharedDoc)->golfDoc);
        index = control[0x3C1];
        if (*(int*)((char*)GolfPlayer_GetEntryByIndex((int)control, index) + 8) == 3) value = 0;
        else {
            slot = control[0x3B6];
            target = (unsigned char)(*(unsigned char*)(*(unsigned int*)(control + 0x2BC) + slot * 32 + 0x18) + 4);
            object = (unsigned char*)GolfPlayer_GetEntryByIndex((int)control, index);
            value = *(int*)(object + 8) == 1 ? object[slot + 0x28] < target : object[slot + 0x28] > target;
        }
        if (value) {
            IActor_DispatchMessage(self, self, golfRuleStrings + kText_guideLine, 1, 0, 0, 0, 0);
            control = (*(unsigned char**)&AsCSharedDoc(g_sharedDoc)->golfDoc);
            index = control[0x3C1]; slot = control[0x3B6];
            if (index < control[0x2B9]) {
                target = (unsigned char)(*(unsigned char*)(*(unsigned int*)(control + 0x2BC) + slot * 32 + 0x18) + 4);
                object = (unsigned char*)GolfPlayer_GetEntryByIndex((int)control, index); object[slot + 0x28] = target + 1;
                ++((unsigned char*)GolfPlayer_GetEntryByIndex((int)control, index))[0xB0];
            }
            control = (*(unsigned char**)&AsCSharedDoc(g_sharedDoc)->golfDoc);
            index = control[0x3C1];
            if (index < control[0x2B9] && *(int*)((char*)GolfPlayer_GetEntryByIndex((int)control, index) + 8) != 3) {
                *(int*)((char*)GolfPlayer_GetEntryByIndex((int)control, index) + 0xA0) = 0;
                slot = control[0x3B6];
                object = (unsigned char*)GolfPlayer_GetEntryByIndex((int)control, index);
                if (*(int*)(object + 0xD4) == slot) {
                    if (!control[0x35C]) {
                        *(unsigned char*)((char*)GolfPlayer_GetEntryByIndex((int)control, index) + 0xC5) += *((unsigned char*)GolfPlayer_GetEntryByIndex((int)control, index) + slot + 0x28);
                        if (AsCSharedDoc(g_sharedDoc)->golfRuleType != 1) {
                            object = (unsigned char*)GolfPlayer_GetEntryByIndex((int)control, index);
                            *(int*)(object + 0xC8) += (int)object[0x28 + slot] - (int)*(unsigned char*)(*(unsigned int*)(control + 0x2BC) + slot * 32 + 0x18);
                        }
                    }
                    ++*(int*)((char*)GolfPlayer_GetEntryByIndex((int)control, index) + 0xD4);
                }
                *(int*)((char*)GolfPlayer_GetEntryByIndex((int)control, index) + 8) = 1;
                if ((*(int*)&AsCSharedDoc(g_sharedDoc)->gameMode) == 6) (*(int*)&AsCSharedDoc(g_sharedDoc)->partyResultTier) = PartyGame_EvaluatePlayerThresholdTier(index);
            }
            ((CGolfRule*)self)->EnterState(AsCSharedDoc(g_sharedDoc)->golfRuleType == 1 ? 0x1000 : 0x8000);
        } else ((CGolfRule*)self)->EnterState(AsCSharedDoc(g_sharedDoc)->golfRuleType == 1 ? 0x1000 : 0x100000);
    }
}
#endif

void GolfTipsDialog_RequestClose(int, float);
void SoundManager_RequestBgmChange(void*, const char*, int, int);
char* strcpy(char*, const char*);
void* GolfPlayer_GetEntryByIndex(int, int);
void SoundManager_RequestBgmChange(void*, const char*, int, int);
void SoundManager_RequestBgmChange(void*, const char*, int, int);
extern float kFloat_6_4_GolfRule, kFloat_neg32_GolfRule, kFloat_3_5_GolfRule;
struct CE00Score {
    char pad[0x28];
    unsigned char score;
};
typedef Vec3f CE00Vec;
}
}
// Hole-result object: the rule view used by StartHoleResultPresentation.
struct CE00Self {
    char pad[0x84];
    Vec3f point;
    char pad90[0xC];
    float duration;
};
namespace ns4 {
extern "C" {
struct CE00Manager {
    char pad[0x808];
    unsigned int mode;
    char pad80C[0x2E4];
    int result;
};
// Choose the hole-result animation, camera and music from player scores and rules.
extern float kFloat_2_805DEA84;
extern void GolfTips_ShowIndexedMessage(int, const char*, int, float, float);
// Prepare per-player turn data, instructions and computer-shot thinking state.
#define T7A(o) (golfRuleMessageTags + (o))

extern const CE00Vec s_characterResultCameraPresets[][4];
extern const float kFloat_3_1415927_GolfRule;
// Decode the result category from packed player-state bits.
static inline int C6Decode(const unsigned char* state) {
    return ~(*(int*)(state + 0x1989C) ^ *(int*)(state + 0x198A4));
}
struct C6WordVec {
    int data[3];
};
// Choose the result presentation category from the game mode and score difference.
static inline int C6Category(int mode, int delta) {
    int category = 0;
    if (mode != 1) {
        switch (delta) {
        case -3:
        case -2:
            category = 0;
            break;
        case -1:
            category = 1;
            break;
        case 0:
            category = 2;
            break;
        case 1:
        case 2:
        case 3:
        default:
            category = 3;
            break;
        }
    }
    return category;
}
// Choose result poses, score text, camera and music for a hole or forced result.

#define SINGLETON_A()                                                                                               \
    if (!Sing_805DC608()) {                                                                                         \
        if (!s_fadeOverlayInitialized) {                                                                            \
            FadeOverlay_Init(s_fadeOverlay);                                                                        \
            __register_global_object(s_fadeOverlay, (void*)ResourceState_Dtor, s_fadeOverlayRegistration_804F72D8); \
            s_fadeOverlayInitialized = 1;                                                                           \
        }                                                                                                           \
        Sing_805DC608() = s_fadeOverlay;                                                                            \
    }
#define SINGLETON_B()                                                            \
    if (!Sing_805DC660()) {                                                      \
        if (!s_effectFadeInitialized) {                                          \
            ((CEffectFade*)s_effectFade)->__ct();                                \
            __register_global_object(s_effectFade, (void*)__dt__11CEffectFadeFv, \
                                     s_effectFadeDtorRegistration_GolfRule);     \
            s_effectFadeInitialized = 1;                                         \
        }                                                                        \
        Sing_805DC660() = s_effectFade;                                          \
    }

// Lazily initialize and return the result fade resource.
static inline void* GetSingletonB() {
    SINGLETON_B();
    return Sing_805DC660();
}

struct SelfFields {
    char pad[0x4A8];
    unsigned char count;
};

struct VObj {
    virtual ~VObj();
};

// Advance round-result animations, result menus, fades and the eventual task transition.

struct Fn8016C79CNames6 {
    const char* name[6];
};
struct Fn8016C79CNames7 {
    const char* name[7];
};
extern const Fn8016C79CNames6 s_courseShotSequenceFormats;
extern const Fn8016C79CNames7 s_partyMode3PreloadSequenceNames;
struct Fn8016C79CObject {
    char pad[0x4D4];
    unsigned int* data;
    unsigned int size, capacity;
};
struct Fn8016C79CStorage {
    unsigned int* data;
    unsigned int size, capacity;
    unsigned int* allocator;
    unsigned int offset;
    // Initialize temporary vector storage with the supplied allocator context.
    Fn8016C79CStorage(unsigned int* a) : data(0), size(0), capacity(0), allocator(a), offset(0) {}
    // Clear the temporary size and free its remaining owned backing allocation.
    ~Fn8016C79CStorage() {
        if (data) {
            size = 0;
            appFreeUni__FPv(data);
        }
    }
};

struct Fn8016C79CAllocator {
    volatile char c;
    char pad[3];
};
struct Fn8016C79CError {
    char* vtable;
    char* data;
    Fn8014F6E0Buffer* buffer;
    // Copy the exception message and install its shared character deleter.
    Fn8016C79CError(const char* name) {
        vtable = __vt__Q23std11logic_error;
        Fn8016C79CAllocator allocator;
        allocator.c = 0;
        unsigned int n = strlen(name) + 1;
        char* copy = (char*)__nwa__FUl(n * 2 - n);
        data = copy;
        {
            Fn8014F6E0Guard guard(copy, (char*)&allocator.c);
            Fn8014F6E0Buffer* p = (Fn8014F6E0Buffer*)__nw__FUl(0x10);
            if (p) {
                p->a = 1;
                p->b = 1;
                p->vtable = s_sharedCharDeleterVtable;
                p->data = copy;
            }
            buffer = p;
            guard.data = 0;
        }
        strcpy(data, name);
    }
    // Preserve the original empty exception-cleanup hook.
    ~Fn8016C79CError() {}
};
struct Fn8016C79CLengthError : Fn8016C79CError {
    // Install the length-error vtable and throw the constructed exception.
    Fn8016C79CLengthError(const char* name) : Fn8016C79CError(name) {
        vtable = __vt__Q23std12length_error;
        __throw(golfRuleStrings + kText_lengthErrorTypeName, this, (void*)__dt__Q23std12length_errorFv);
    }
};
#define FN8016C79C_ERROR()                                            \
    const char* source = golfRuleStrings + kText_vector_length_error; \
    Fn8016C79CLengthError error(source)
#define FN8016C79C_STORAGE_ERROR()                                    \
    const char* source = golfRuleStrings + kText_vector_length_error; \
    Fn8016C79CError error(source);                                    \
    error.vtable = __vt__Q23std12length_error;                        \
    __throw(golfRuleStrings + kText_lengthErrorTypeName, &error, (void*)__dt__Q23std12length_errorFv)

// Both name lists use the same vector insertion, including its overflow checks
// and the temporary storage that owns the allocation until the final swap.
#define FN8016C79C_APPEND(value, second)                                        \
    unsigned int cap = obj->capacity;                                           \
    unsigned int size = obj->size;                                              \
    if (size < cap) {                                                           \
        unsigned int next = size + 1;                                           \
        unsigned int* data = obj->data;                                         \
        obj->size = next;                                                       \
        (data + next)[-1] = (unsigned int)(value);                              \
    } else {                                                                    \
        if (0x3FFFFFFFU - obj->capacity < 1U) {                                 \
            FN8016C79C_ERROR();                                                 \
        }                                                                       \
        Fn8016C79CStorage tmp(&obj->capacity);                                  \
        unsigned int growthA, growthB;                                          \
        unsigned int needed = obj->size + 1 - obj->capacity;                    \
        unsigned int oldCapacity = obj->capacity;                               \
        if (needed > 0x3FFFFFFFU - oldCapacity) {                               \
            FN8016C79C_STORAGE_ERROR();                                         \
        }                                                                       \
        unsigned int capacity;                                                  \
        if (oldCapacity < 0x15555555U) {                                        \
            growthA = ((oldCapacity + 1) * 3) / 5;                              \
            capacity = oldCapacity + *(growthA < needed ? &needed : &growthA);  \
        } else if (oldCapacity < 0x2AAAAAAAU) {                                 \
            growthB = (oldCapacity + 1) / 2;                                    \
            capacity = oldCapacity + *(growthB < needed ? &needed : &growthB);  \
        } else {                                                                \
            capacity = 0x3FFFFFFF;                                              \
        }                                                                       \
        if (capacity > 0x3FFFFFFFU) {                                           \
            FN8016C79C_STORAGE_ERROR();                                         \
        }                                                                       \
        unsigned int* fresh = (unsigned int*)appMallocDummy__FUl(capacity * 4); \
        tmp.offset = obj->size;                                                 \
        if (second)                                                             \
            tmp.data = fresh;                                                   \
        (fresh + obj->size)[tmp.size] = (unsigned int)(value);                  \
        tmp.capacity = capacity;                                                \
        if (!second)                                                            \
            tmp.data = fresh;                                                   \
        tmp.size++;                                                             \
        unsigned int* old = obj->data;                                          \
        unsigned int count = ((int)(old + obj->size) - (int)old) / 4;           \
        tmp.offset -= count;                                                    \
        memcpy(fresh + tmp.offset, old, count * 4);                             \
        memset(old, 0, count * 4);                                              \
        unsigned int newSize = tmp.size + count;                                \
        tmp.size = newSize;                                                     \
        unsigned int* oldData = obj->data;                                      \
        unsigned int* newData = tmp.data;                                       \
        unsigned int oldCap = obj->capacity;                                    \
        obj->size = 0;                                                          \
        tmp.data = oldData;                                                     \
        obj->capacity = tmp.capacity;                                           \
        tmp.capacity = oldCap;                                                  \
        obj->data = newData;                                                    \
        obj->size = newSize;                                                    \
        tmp.size = 0;                                                           \
    }

#pragma use_lmw_stmw on
// Queue player-shot sequences and the resources required by the selected party rules.

#pragma use_lmw_stmw reset

struct T16 {
    float v[16];
};

#define D834_IDX() \
    ItemInventory_IsSlot14ClubSetId((void*)*(int*)((char*)g_sharedDoc + (int)GolfDocBytes()[0x3C1] * 0x1D4 + 0x1CC))
#define CurrentGolfPlayerEntryAbi() \
    ((unsigned char*)((void* (*)(void*, int))GolfPlayer_GetEntryByIndex)(GolfDocBytes(), GolfDocBytes()[0x3C1]))
// Clamp a reward modifier to its lower and upper bounds.
static inline float D834_clamp(float v, float lo, float hi) {
    if (v < lo)
        return lo;
    if (v > hi)
        return hi;
    return v;
}
// Clamp a reward modifier to the allowed range.
static inline float D834_clamp2(float v) {
    if (v < kFloat_0_GolfRule)
        return kFloat_0_GolfRule;
    if (v > kFloat_99_GolfRule)
        return kFloat_99_GolfRule;
    return v;
}
// Test whether the current player has the requested inventory item equipped.
static inline bool D834_is(int k) {
    void* layout = ((char*)&AsCSharedDoc(g_sharedDoc)->itemInventory);
    unsigned char* pl = CurrentGolfPlayerEntryAbi();
    return *(unsigned int*)(pl + 0xA0) == (unsigned int)ItemInventory_GetItemIdAt(layout, k);
}
// Apply impact, equipment and effect modifiers to the current player shot-outcome value.
void GolfRule_ApplyShotOutcomeModifier();

void Camera_SetFov(void*, float);
void PartyItemSlots_SetSelectedPlayer(void*, int);
void PartyItemSlots_RebuildEffectParameters(void*);
void CourseData_UpdateWindActors(void*);
extern float kGolfCameraFovRadians, kFloat_6_4_GolfRule, kFloat_19_2_GolfRule, kFloat_44_8_GolfRule;
struct B5Base {
    char pad[0x58];
};
struct B5Object : B5Base {
    virtual void v08();
    virtual void v0c();
    virtual void v10();
    virtual void v14();
    virtual void v18();
    virtual void v1c();
    virtual void v20();
    virtual void v24();
    virtual void v28();
    virtual void v2c();
    virtual void v30();
    virtual void v34();
    virtual void v38();
    virtual void v3c();
    virtual void v40();
    virtual void v44();
    virtual void v48();
    virtual void v4c();
    virtual void v50();
    virtual void v54();
    virtual int check();
};
struct B5PlayerFields {
    char pad[0xAE];
    unsigned char flag;
    char padAF[0xC1];
    int field170;
    int field174;
};
struct B5ContextFields {
    char pad[0x34];
    float f34, f38;
    char pad3C[0x28];
    int i64, i68;
};
struct B5Manager {
    char pad[0xB90];
    unsigned char* game;
};
#define B5_GAME (((B5Manager*)g_sharedDoc)->game)
#define B5_F(p, n) (*(float*)((char*)(p) + (n)))
#define CurrentGolfPlayerEntryFromDoc(g) \
    ((unsigned char*)((void* (*)(void*, int))GolfPlayer_GetEntryByIndex)((g), (g)[0x3C1]))
#define B5_MSG(n, c, v) IActor_DispatchMessage(self, self, golfRuleStrings + (n), (c), (int)(v), 0, 0, 0)
#define B5_MSG_THIS(n, c, v) IActor_DispatchMessage(this, this, golfRuleStrings + (n), (c), (int)(v), 0, 0, 0)
// Prepare the next shot or complete the current player/hole after presentation ends.

struct Elem939C {
    unsigned char b[0x54];
};
struct ShotCheckpointDeque {
    unsigned int start;
    unsigned int size;
    Elem939C** map;
    unsigned int mapStart;
    unsigned int blocks;
    unsigned int mapSize;
    // Return the last logical 0x54-byte entry; requires a nonempty block deque.
    Elem939C& back() {
        unsigned int i = start + size - 1;
        Elem939C* e = &map[(mapStart + (i >> 4)) % mapSize][i & 15];
        return *e;
    }
};
#define Back939C(d) ((d)->back().b)
// Look up the player snapshot for the supplied slot.
static inline unsigned char* Player939C(unsigned char slot) {
    return (unsigned char*)GolfPlayer_GetEntryByIndex((int)(*(unsigned char**)&AsCSharedDoc(g_sharedDoc)->golfDoc),
                                                      slot);
}
#define P939C Player939C(slot)
// Return the requested word in a player snapshot.
static inline int* Cur939C(unsigned char* g, unsigned char slot, int off) {
    unsigned char w = g[0x3B6];
    return (int*)((unsigned char*)GolfPlayer_GetEntryByIndex((int)g, slot) + w * 4 + off);
}
// Return the requested byte in a player snapshot.
static inline unsigned char* CurB939C(unsigned char* g, unsigned char slot, int off) {
    unsigned char w = g[0x3B6];
    return (unsigned char*)GolfPlayer_GetEntryByIndex((int)g, slot) + w + off;
}
void ClubSelection_SelectByName(void*, void*);
// Restore a selected shot-history entry, including ball, gauge and player state.

int RivalStrokeUse_GetStrokesThroughHole(void*, int, int);
int RivalStrokeUse_GetPangThroughHole(void*, int, int);
signed char ItemInventory_FindCharacterIndex(void*, int);
int Math_Abs32(int);
struct Rank20Bits {
    CW_BITS2(unsigned char high : 4, unsigned char low : 4)
};
// Player +C4 packs two ONE-BASED ranks: high is the display/order rank after
// tie breaking; low is the competition rank before that last tie break. Both
// start at 1 and increment for each strictly better opponent. Only high changes
// for an otherwise tied opponent with an earlier tie-order key. Equal keys can
// therefore still share high rank: this routine does not guarantee uniqueness.
// Match ranks compare wins (+C8), then Pang (+CC), both descending; Stroke uses
// round score +C8 ascending, then +CC descending. Both use rankTieBreakOrder (computer +4)
// as the final high-only key. Balloon Pop uses +1C4 descending, Pang descending,
// then the unadjusted rankTieBreakOrder. Bonus Pang (+D0) is NOT a rank tie break.
// Raw signed lvalues preserve the comparison domain despite unsigned storage.
#define R20_SCORE(p) (*(int*)&((GolfDocSnapshot8013*)(p))->roundResultWords[kRoundScoreWord])
#define R20_PANG(p) (*(int*)&((GolfDocSnapshot8013*)(p))->roundResultWords[kRoundPangWord])
// Hole record schema: 0x20-byte rows, +19 eligible/reset player count, +1A..+1D
// one byte per player. In normal Stroke these bytes rank CURRENT-HOLE strokes
// ascending with turnOrder as tie break; excluded state 3 receives playerCount.
// In Vs CPU and party rules these bytes copy the player's high round-rank nibble.
// Next turnOrder = byte-1, except Balloon Pop uses abs(byte-playerCount), reversing
// the order. Match does not write this row here: it switches rule->turn when the
// opponent used fewer strokes. Keep row bytes distinct from round-rank nibbles.
enum HoleOrderRecordOffset {
    kHoleEligiblePlayerCount = 0x19,
    kHolePlayerOrder = 0x1A
};
enum RankingPlayerOffset {
    kPlayerHoleStrokeCounts = (int)&((GolfDocSnapshot8013*)0)->holeStrokeCounts,
    kMatchHoleAwardFlags = (int)&((GolfDocSnapshot8013*)0)->holeAwardFlags,
    kBalloonRankingResult = 0x1C4
};
#define R20_I(p, o) (*(int*)((unsigned char*)(p) + (o)))
#define R20_BALLOON_RESULT(p) R20_I(p, kBalloonRankingResult)
#define R20_B(p) (*(Rank20Bits*)((p) + 0xC4))
#define R20_REC(i) (*(unsigned char**)(GolfDocBytes() + 0x2BC) + (i) + (GolfDocBytes()[0x3B6] << 5))
#define R20_HOLE_ORDER(i) R20_REC(i)[kHolePlayerOrder]
// Fetch the opponent first, as each comparison in the original does.
#pragma use_lmw_stmw on
// Recompute player ranks and the order used to start the next hole.

#pragma use_lmw_stmw reset

// Gauge X values are UI coordinates, not world distances. Origin 149 maps to
// zero power; 509 = 149+360 maps to full power; 114 is the return/reset limit.
static const float kButtonGaugeOriginX = 149.0f;
static const float kButtonGaugeFullPowerX = 509.0f;
static const float kButtonGaugeReturnLimitX = 114.0f;
static const float kButtonGaugeImpactLockMaximumX = 249.0f;
static const float kButtonGaugeUpdatesPerSecond = 30.0f;
static const float kButtonGaugeStepSeconds = 0.033333335f;
static const float kButtonGaugeReturnPauseSeconds = 0.03f;
enum GaugeItemEffectSlot {
    kGaugeSlowSlot7 = 7,
    kGaugeSlowSlot8 = 8,
    kGaugeSlowAndWideSlot9 = 9,
    kGaugeSlowSlot2 = 2,
    kGaugeWideSlot1 = 1
};
struct ButtonGaugeTuningView {
    char padToAutoPower[0x3D0];
    float autoPowerStopX, autoImpactStopX;
    char padToSpeed[0x3FC - 0x3D8];
    float partyMode3GaugeSpeedMultiplier;
};
// Sparse owner-specific views for the gauge update; the speed-reduction statistic
// has not been identified as a particular player skill.
struct ButtonGaugeClubSpeed {
    char prefix[0x20];
    float baseGaugeSpeed;
};

struct ButtonGaugePowerLatch {
    char prefix[0x2D8];
    unsigned char powerLockedFromStoppedGauge;
};
#define GAUGE_BALL_BYTES ((unsigned char*)GolfBall__Fv())
#define GAUGE_PARAMETERS_BYTES ((unsigned char*)InstanceTable_Get())
#define GAUGE_CLUB_MODE ((ClubSelectionRuntime*)GAUGE_PARAMETERS_BYTES)->selectedClub->clubKind
// 0x80163D94: refresh the active player before each short-circuited item test.
#define GaugePlayerHasItem(id)                                                                            \
    (itemInventoryBytes = SharedDocBytes() + 0xA04, golfDocBytes = GolfDocBytes(),                        \
     playerSnapshot = (unsigned char*)GolfPlayer_GetEntryByIndex((int)golfDocBytes, golfDocBytes[0x3C1]), \
     *(unsigned int*)(playerSnapshot + 0xA0) == (unsigned int)ItemInventory_GetItemIdAt(itemInventoryBytes, id))
// Compute the absolute magnitude while preserving the original floating-point behavior.
static inline float abs3D(float f) {
    union {
        float f;
        unsigned int u;
    } v;
    v.f = f;
    return (v.u & 0x80000000) ? -f : f;
}
// Compare floating-point values using the original ordered comparison and tolerance.
static inline bool near3D(float f) {
    return abs3D(f) < 0.00001f;
}
// Compute the absolute magnitude while preserving the original floating-point behavior.
static inline float magnitude3D(float f) {
    return f > 0.0f ? f : -f;
}
// Select the bounded value while retaining the original operand order.
static inline float minimum3D(float f) {
    return f > kButtonGaugeImpactLockMaximumX ? kButtonGaugeImpactLockMaximumX : f;
}
// Compute the absolute magnitude while preserving the original floating-point behavior.
static inline float absref3D(float& f) {
    return (*(unsigned int*)&f & 0x80000000) ? -(volatile float&)f : (volatile float&)f;
}
// Compare floating-point values using the original ordered comparison and tolerance.
static inline bool nearref3D(float& f) {
    return absref3D(f) < 0.00001f;
}

struct Node3D {
    Node3D* next;
    Node3D* prev;
    unsigned int value;
};
// Pack the recorded directional shot commands into their comparison code.
static inline int combo3D(unsigned char* game) {
    int code = 0;
    if (AsCGolfDoc(game)->shotCommandCount && !(((Node3D*)AsCGolfDoc(game)->shotCommandHead)->value & 0x40)) {
        Node3D* sentinel = (Node3D*)(game + 0xA0);
        for (Node3D* n = *(Node3D**)(game + 0xA4); n != sentinel; n = n->prev) {
            if (code && n->value != 0x10)
                code <<= 8;
            code |= n->value;
        }
    }
    return code;
}
extern float s_golfRulePowerFlag4DirectionX, s_golfRuleFloat_1;
extern float kFloat_0_5_GolfRule, kFloat_149_805DEA58, kFloat_360_GolfRule, kFloat_0_875_GolfRule, kFloat_1_75_GolfRule;
// Advance the button gauge, lock power and impact, and validate directional special shots.
void CGolfRule_UpdatePowerAndImpactGauge(unsigned char* self, int pressed);

unsigned int ItemInventory_GetBallIdAt(void*, int);
extern float kFloat_3_0, kFloat_4_GolfPlayerHelpers, kFloat_32_GolfRule, kFloat_22_4_GolfRule, kFloat_3_5_GolfRule;

#define EA_I(p, o) (*(int*)((unsigned char*)(p) + (o)))
#define EA_F(p, o) (*(float*)((unsigned char*)(p) + (o)))
#define EA_B(p, o) (*((unsigned char*)(p) + (o)))
#define EA_PTR(p, o) (*(unsigned char**)((unsigned char*)(p) + (o)))
#define EA_SEND(o, c, p) IActor_DispatchMessage(self, self, golfRuleStrings + (o), (c), (int)(p), 0, 0, 0)
#define EA_SOUND(o)                                                                                                  \
    ((int (*)(void*, const char*, float, float, float, int, int, int, int))Sound_PlayByName)(                        \
        g_soundManager, golfRuleStrings + (o), kFloat_0_5_GolfRule, kFloat_200_GolfRule, kFloat_1_805DEA38, 0, 0, 0, \
        0)
// These sequences occur independently in both result paths in the original.

// Resolve the simulated shot outcome, penalties, scoring and next presentation state.

void* CourseData_FindNamedPoint(void*, int);
void MPBase1_DrawIfInitialized(void*);
void Vec2_Copy(void*, const void*);
extern void* g_currentTaskChild_GolfRule;
extern void* g_option;
extern char s_golfTaskTypeInfo[8];
extern float kFloat_0_01_GolfRule, kFloat_22_GolfRule, kFloat_7_GolfRule, kFloat_100_GolfRule;

struct Event545Vector {
    int x, y, z;
};
struct Event545Type {
    virtual Event545Type* type();
    Event545Type* next;
};
struct Event545PlayerBits {
    unsigned char pad[0xC4];
    CW_BITS2(unsigned char high : 4, unsigned char low : 4)
};
struct Event545UIData {
    char pad[0x58];
};
struct Event545UI : Event545UIData {
    virtual void v08();
    virtual void v0C();
    virtual void v10();
    virtual void v14();
    virtual void v18();
    virtual void v1C();
    virtual void v20();
    virtual void v24();
    virtual void v28();
    virtual int v2C();
    virtual void v30();
    virtual void v34();
    virtual void v38();
    virtual void v3C();
    virtual void v40(float, int);
    virtual void v44();
    virtual void v48();
    virtual void v4C(int);
    virtual void v50(int);
    virtual void v54();
};

#define E545_I(p, o) (*(int*)((unsigned char*)(p) + (o)))
#define E545_U(p, o) (*(unsigned int*)((unsigned char*)(p) + (o)))
#define E545_F(p, o) (*(float*)((unsigned char*)(p) + (o)))
#define E545_P(p, o) (*(unsigned char**)((unsigned char*)(p) + (o)))
#define E545_SEND(s, c, a, b) IActor_DispatchMessage(this, this, golfRuleStrings + (s), (c), (int)(a), (int)(b), 0, 0)

// Test the active shot type used by the message handler.

// Dispatch golf-rule messages to shot, camera, player, menu and input state handlers.
// msg is borrowed for this synchronous call: +4 message ID, +8/+C/+10 payload
// words. Pointer payloads below are borrowed input/output storage, not retained.
// IDs 17/1A reset hole state; 18 enables putt animation; 2719 invokes the actor
// reset hook. Query cases write through +8. Recipient-specific IDs sent to other
// actors are separate protocols even when their numeric values coincide.

extern int s_pangyaZoneBaseWidthUnits;
extern float kFloat_2_805DEA84;
int SharedDoc_IsMode6PartyMode0();
int ShotGauge_AddClampedTotal(float);
void Vec3_Zero(void*);
void __ml__FRC7WVectorf(void*, const void*, float);
float ClubSelection_GetLaunchAngle(void*, int);
float ShotParameters_GetBaseLaunchSpeed(void*);
float Math_SquareRootApprox(float);
extern "C" void GetWindVector__5CWindFv(void* result, const void* self);
float GolfParameter_ResolveFloat(void*, int);
void Vec2_Copy(void*, const void*);
unsigned char CourseData_IsPointOutsidePlayableRegion(void*, void*);
void __amu__7WVectorFf(void*, float);
void gtl_Stopwatch_Initialize(void*);
void gtl_Stopwatch_Start(void*);
void gtl_Stopwatch_Reset(void*);
void gtl_Stopwatch_GetElapsedSeconds(void*);
int QuadTree_IsWaterAtPoint(void*, void*);
void GolfAim_FindPlayableRecoveryPoint(void*, void*);
extern float s_golfRuleShotSpeedCap, s_golfRuleMode1SpeedCap306, s_golfRuleSimulationMode2SpeedCap273;

// Borrow a scalar at an unresolved ABI offset; callers retain object ownership.
// These return lvalues so writes keep the same width and alias behavior as reads.
static inline float& RuleFloatAt(void* base, int offset) {
    return *(float*)((unsigned char*)base + offset);
}
// Borrow a signed 32-bit word at the stated object offset.
static inline int& RuleIntAt(void* base, int offset) {
    return *(int*)((unsigned char*)base + offset);
}
// Borrow one byte at the stated object offset.
static inline unsigned char& RuleByteAt(void* base, int offset) {
    return *((unsigned char*)base + offset);
}
// Expression-form access is retained in simulation and state-entry helpers.
// Keep the existing lvalue access shape when changing names or comments.
// As with Rule*At, offsets are bytes and accesses borrow storage without ownership.
#define SIM_FLOAT_AT(base, offset) (*(float*)((unsigned char*)(base) + (offset)))
#define SIM_INT_AT(base, offset) (*(int*)((unsigned char*)(base) + (offset)))
#define SIM_BYTE_AT(base, offset) (*(unsigned char*)((unsigned char*)(base) + (offset)))
// Borrowed accessory multipliers used by impact classification and launch setup.
struct ShotAccessoryModifiers {
    char prefix[0x18];
    float impactWidthBonus;
    char pad1C[8];
    float verticalVelocityBonus;
    char pad28[0x14];
    float gaugeRateScale;
};
// Replay input slice owned by the golf rule, captured before the simulation runs.
// Doc byte 3B7 and player byte AE retain neutral names: their value domains are unknown.
struct GolfRuleShotReplayInputs {
    char prefix[0x2E4];
    unsigned char sourceDocByte3B7;
    char pad2E5[3];
    WVector wind;
    unsigned char weatherVariant, holeStrokeCount;
    char pad2F6[2];
    unsigned char shotConfiguration[0x40];
    WVector ballStartPosition;
    float headingRadians, curveSetting, spinSetting;
    int worldTickAtCapture, powerFlags, powerMode;
    char pad35C[4];
    unsigned char playerSnapshotByteAE;
    char pad361[3];
    int itemId;
};
typedef char ShotAccessoryGaugeOffset[(int)&((ShotAccessoryModifiers*)0)->gaugeRateScale == 0x3C ? 1 : -1];
typedef char ShotReplayHeadingOffset[(int)&((GolfRuleShotReplayInputs*)0)->headingRadians == 0x344 ? 1 : -1];
typedef char ShotReplayItemOffset[(int)&((GolfRuleShotReplayInputs*)0)->itemId == 0x364 ? 1 : -1];

enum BallStorageByteOffset {
    kBallVelocityByteOffset = (int)&((CGolfBall*)0)->m_velocity,
    kBallSimulationSecondsOffset = 0xA4 // Float storage overlapping legacy event-array slot 15.
};
// Ball/sample layout map for both simulation pipelines below:
// ball +0x00 position, +0x0c velocity, +0x2c rotation pair, +0x64 highest sample;
// ball +0x68 playback cursor, +0x6c..+0xa0 event sample slots, +0xa4 elapsed seconds.
// sample +0/+0xc/+0x18 receive position/velocity/rotation; +0x70 receives state.
// m_preDataCount is the highest valid sample index (sample zero exists), not a
// byte count. Velocity is world-space, Y-up, integrated at 1/30-second steps.
// The header's 16-int event view includes the float at +0xa4: do not treat that
// last word as an index or reset it with the other event slots.
#define GOLF_BALL_BYTES ((unsigned char*)GolfBall__Fv())
#define GOLF_SHOT_PARAMETERS_BYTES ((unsigned char*)InstanceTable_Get())
#define GOLF_CLUB_CATEGORY RuleByteAt(*(unsigned char**)(GOLF_SHOT_PARAMETERS_BYTES + 4), 0xA)

struct D3 {
    int v[3];
};
struct F16 {
    int v[16];
};

// Superseded layout: ShotCheckpoint; retain this assignment shape for container copies.
struct S54 {
    D3 position;
    float d;
    unsigned char e, f;
    float g;
    int h, i, j;
    int k[8];
    unsigned char l[7];
    int m, n;
};

extern float kFloat_1_3_GolfPowerFlags, kFloat_neg1_GolfRule, kFloat_0_69813168_GolfRule;

// Compute the absolute magnitude while preserving the original floating-point behavior.
static inline double d_fabs(float x) {
    union {
        float f;
        unsigned u;
    } c;
    c.f = x;
    return (c.u & 0x80000000) ? -(double)x : (double)x;
}

// Compute the absolute magnitude while preserving the original floating-point behavior.
static inline float WabsHint(const float& value) {
    return (*(const unsigned long*)&value & 0x80000000) ? -value : value;
}

#pragma opt_common_subs off
#pragma opt_lifetimes off
struct RingIt54 {
    unsigned char** cur;
    unsigned char** begin;
    unsigned char** start;
    unsigned char** end;
    // Advance this block-map iterator, wrapping once within its allocated map.
    void advance(int n) {
        int count = end - begin;
        int index = n + (cur - begin);
        if (index < 0)
            index += count;
        else if (index >= count)
            index -= count;
        cur = begin + index;
    }
};
// Build or advance a circular iterator, wrapping at the storage boundary.
static inline RingIt54 RingBegin54(unsigned char* ring) {
    RingIt54 it;
    unsigned char** map = *(unsigned char***)ring;
    it.cur = map + RuleIntAt(ring, 4);
    it.begin = map;
    it.start = map + RuleIntAt(ring, 4);
    it.end = map + RuleIntAt(ring, 0xC);
    return it;
}
// Build or advance a circular iterator, wrapping at the storage boundary.
static inline RingIt54 RingAdvance54(RingIt54 it, int n) {
    int count = it.end - it.begin;
    int index = n + (it.cur - it.begin);
    if (index < 0)
        index += count;
    else if (index >= count)
        index -= count;
    it.cur = it.begin + index;
    return it;
}
// Find the current player item slot used by the forget-item dialog.
static inline int FindSlot418(unsigned char* self) {
    if (RuleIntAt(SharedDocBytes(), 0xAD8) != 0)
        return 0;
    for (int i = 0; i < 8; i++) {
        unsigned char* m = SharedDocBytes();
        unsigned char* g = GolfDocBytes();
        unsigned h = (unsigned)ItemInventory_GetItemIdAt(m + 0xA04, 6);
        if (h == (unsigned)RuleIntAt(m + i * 4 + g[0x3C1] * 0x1D4, 0x8C)) {
            RuleIntAt(self, 0x418) = i;
            return 1;
        }
    }
    return 0;
}
// Compare floating-point values using the original ordered comparison and tolerance.
static inline int OrderedLess54(float x, float y) {
    int result = x < y;
    return result;
}
// Select the bounded value while retaining the original operand order.
static inline const float& Minimum54(const float& x, const float& y) {
    return y < x ? y : x;
}
// Decode the ball hole-in value from its obfuscated storage words.
static inline int KeyCheck54(unsigned char* p) {
    int v = ~(RuleIntAt(p, 0x1989C) ^ RuleIntAt(p, 0x198A4));
    return v;
}
struct ShotGaugePrefix54 {
    char data[0x58];
};
// Borrowed CPadSwingCtrl view: the vptr is at object +58, virtual entries start
// at table +08 after RTTI/adjustment. Do not add/remove a slot or a destructor.
// Only slot1c and value are invoked through this partial view in this unit:
// +1C = CPadSwingCtrl::FastForwardMotion(animationTarget), pad_swing_ctrl_motion.cpp: run at most 150
//   fixed 1/30-second motion steps with skipPlayback=1. Returns zero-based
//   completion index 0..149, or 150 exhausted; resets motion only on completion.
//   BuildShotSimulation subtracts this count from world frame (clamping at 0),
//   with fallback 30 when the player actor is absent. It is NOT a success bool.
// +38 = CPadSwingCtrl_GetSampledMotionFactor(), pad_swing_ctrl_and_com_think.cpp: dimensionless
//   sampled-motion factor, accumulated in manual swing statistics (cap 1e6).
//   The implementation clamps to kFloat_0_1..kFloat_3_PadSwing and replaces
//   nonfinite results with kFloat_0_1 (0.1..3.0). It is not time.
// Both require a live controller; slot1c also requires a live animation target.
// Other numbered declarations only preserve vtable positions; no calls here
// justify operation/argument contracts for them. See phase1_class_tables.cpp
// __vt__13CPadSwingCtrl for the complete slot-to-function map.
struct ShotGauge54 : ShotGaugePrefix54 {
    virtual void slot08();
    virtual void slot0c();
    virtual void slot10();
    virtual void slot14();
    virtual void slot18();
    virtual unsigned slot1c(void*);
    virtual void slot20();
    virtual void slot24();
    virtual void slot28();
    virtual void slot2c();
    virtual void slot30();
    virtual void slot34();
    virtual float value();
};
// Clear per-shot collision contacts and all recorded sample/event cursors.
static inline void ResetShotCollisionAndSamples(unsigned char* ballBytes) {
    ((CQuadTree*)CQuadTree_GetInstance())->ResetPerShotSimulationState();
    ((CGolfBall*)ballBytes)->ClearBuffer();
    {
        int m1 = -1;
        AsCGolfBall(ballBytes)->m_preDataCount = 0;
        AsCGolfBall(ballBytes)->m_eventPreDataIndex[1] = m1;
        AsCGolfBall(ballBytes)->m_eventPreDataIndex[kBallFirstDescendingSample] = m1;
        AsCGolfBall(ballBytes)->m_eventPreDataIndex[3] = m1;
        AsCGolfBall(ballBytes)->m_eventPreDataIndex[4] = m1;
        AsCGolfBall(ballBytes)->m_eventPreDataIndex[5] = m1;
        AsCGolfBall(ballBytes)->m_eventPreDataIndex[6] = m1;
        AsCGolfBall(ballBytes)->m_eventPreDataIndex[9] = m1;
        AsCGolfBall(ballBytes)->m_eventPreDataIndex[7] = m1;
        AsCGolfBall(ballBytes)->m_eventPreDataIndex[8] = -(AsCGolfBall(ballBytes)->m_state != 3);
        AsCGolfBall(ballBytes)->m_shotStage = 0;
        AsCGolfBall(ballBytes)->m_bounceCount = 0;
        AsCGolfBall(ballBytes)->m_firstContactIndex = m1;
        AsCGolfBall(ballBytes)->m_firstContactId = m1;
        AsCGolfBall(ballBytes)->m_lastContactIndex = m1;
        AsCGolfBall(ballBytes)->m_lastContactId = m1;
        SIM_INT_AT(ballBytes, 0x28) = m1;
        AsCGolfBall(ballBytes)->m_eventPreDataIndex[kBallCobraDistanceSample] = m1;
        AsCGolfBall(ballBytes)->m_eventPreDataIndex[kBallTomahawkEndSample] = m1;
        Vec3_Zero(ballBytes + 0x4C);
        AsCGolfBall(ballBytes)->m_eventPreDataIndex[kBallSpikeMotionSample] = m1;
        AsCGolfBall(ballBytes)->m_eventPreDataIndex[kBallSpikeDescentSample] = m1;
        AsCGolfBall(ballBytes)->m_eventPreDataIndex[kBallSpikeEndSample] = m1;
    }
}

// Capture replay inputs before simulation mutates the live ball and player state.
// self owns the captured slice; ballBytes and shotWind scratch remain borrowed.
static inline void CaptureShotReplayInputs(unsigned char* self, unsigned char* ballBytes, const float& headingRadians,
                                           WVector& shotWind) {
    ((GolfRuleShotReplayInputs*)self)->sourceDocByte3B7 = SIM_BYTE_AT(GolfDocBytes(), 0x3B7);
    GetWindVector__5CWindFv(&shotWind, CWind::GetInstance());
    *(WVector*)(self + 0x2E8) = shotWind;
    ((GolfRuleShotReplayInputs*)self)->weatherVariant = SIM_BYTE_AT(SharedDocBytes(), 0x9C6);
    ((GolfRuleShotReplayInputs*)self)->holeStrokeCount =
        *((unsigned char*)GolfPlayer_GetEntryByIndex((int)GolfDocBytes(), AsCGolfDoc(GolfDocBytes())->currentPlayer) +
          AsCGolfDoc(GolfDocBytes())->currentHoleIndex + 0x28);
    ((GolfRuleShotReplayInputs*)self)->headingRadians = headingRadians;
    ((GolfRuleShotReplayInputs*)self)->worldTickAtCapture = SIM_INT_AT(CQuadTree_GetInstance(), 0x21AC);
    *(WVector*)(self + 0x338) = *(WVector*)ballBytes;
    memcpy(self + 0x2F8, GolfDocBytes() + 0x2C4, 0x40);
    ((GolfRuleShotReplayInputs*)self)->curveSetting = AsCGolfBall(ballBytes)->m_initCurve;
    ((GolfRuleShotReplayInputs*)self)->spinSetting = AsCGolfBall(ballBytes)->m_initSpin;
    ((GolfRuleShotReplayInputs*)self)->powerFlags = AsCGolfBall(ballBytes)->m_powerFlags;
    ((GolfRuleShotReplayInputs*)self)->powerMode = SIM_INT_AT(GOLF_SHOT_PARAMETERS_BYTES, 0x44);
    ((GolfRuleShotReplayInputs*)self)->playerSnapshotByteAE =
        AsGolfDocSnapshot(GolfPlayer_GetEntryByIndex((int)GolfDocBytes(), AsCGolfDoc(GolfDocBytes())->currentPlayer))
            ->unk_AE;
    strcpy(AsGolfRuleFrame(self)->clubName, (char*)(*(unsigned char**)(GOLF_SHOT_PARAMETERS_BYTES + 4) + 0x18));
    ((GolfRuleShotReplayInputs*)self)->itemId =
        AsGolfDocSnapshot(GolfPlayer_GetEntryByIndex((int)GolfDocBytes(), AsCGolfDoc(GolfDocBytes())->currentPlayer))
            ->itemId;
}

// Resolve impact and shot physics, optionally preview the hit, and build simulated ball samples.
// previewOnly still changes impact/shot flags and feedback, then returns 1 before
// running physics. Full mode also returns 1, mutates the shared ball/player/history
// and leaves the sample buffer ready for playback; this is not a pure query.
// Geometry uses world XYZ with +Y up, yaw/elevation in radians, ticks at 30 Hz.
// Impact widths/offsets use
// gauge units; lie scales and impact ratios are dimensionless. Later ball radius
// and travel limits use world units. Launch speed uses world units/second;
// the later endpoint distance includes roll. Named vectors keep XYZ spaces.

#pragma opt_common_subs reset
#pragma opt_lifetimes reset

void* SmallCommonDialog_CreateAndRegisterMode5(int, const char*, void*, float);
void* Runtime_AdjustMemberPointer(int, const char*, void*);
void CourseRenderStates_ApplyPair(void*, int);
void* CQuadTree_GetInstance();
extern unsigned int* g_cameraWork;
extern unsigned int lbl_805DC7F4;
extern char s_golfRuleSecondaryInterfaceCallback[];

extern float kFloat_0_15_GolfRule, kFloat_0_1_GolfRule, kGolfSimulationStepSeconds, kFloat_neg10_299999_GolfRule,
    kFloat_neg10_799999_GolfRule, kFloat_0_017453292_GolfRule, kGolfCameraFovRadians, kGolfCameraTransitionFovRadians,
    kFloat_0_34906584_GolfRule, kFloat_0_17453292_GolfRule, kFloat_0_69813168_GolfRule, kFloat_1_7453293_GolfRule,
    kFloat_0_0033333336_GolfRule, kFloat_3_1_GolfRule, kFloat_20_2_GolfRule, kFloat_3_2_GolfRule,
    kFloat_100000_GolfRule, kFloat_200_GolfRule, kFloat_149_805DEA58, kFloat_360_GolfRule, kFloat_neg15_GolfRule,
    kFloat_neg20_GolfRule, kFloat_230_GolfRule, kFloat_170_GolfRule, kFloat_100_GolfRule, kFloat_344_GolfRule,
    kFloat_112_GolfRule, kFloat_306_GolfRule, kFloat_105_GolfRule, kFloat_273_GolfRule, kFloat_416_GolfRule,
    kFloat_320_GolfRule, kFloat_640_GolfRule, kFloat_480_GolfRule, kFloat_36_GolfRule, kFloat_neg1_GolfRule,
    kFloat_50_GolfRule, kFloat_neg5_GolfRule, kFloat_30_GolfRule, kFloat_74_GolfRule, kFloat_76_GolfRule,
    kFloat_80_GolfRule, kFloat_72_5_GolfRule, kFloat_21_5_GolfRule, kFloat_neg8_GolfRule, kFloat_20_5_GolfRule,
    kFloat_0_5_GolfRule, kFloat_3_0, kFloat_1_805DEA38, kFloat_1_3_GolfPowerFlags, kFloat_0_2_GolfRule,
    kFloat_1_5_GolfRule, kFloat_2_805DEA84, kFloat_2_5_GolfRule, kFloat_7_GolfRule, kFloat_0_8_GolfRule,
    kFloat_4_GolfPlayerHelpers;
extern float s_golfRuleMode0SpeedCap344, s_golfRuleStateEntryMode1SpeedCap306, s_golfRuleMode2LaunchSpeedCap273;
extern void* g_shadowManager;
extern int s_shotImpactEffectId;
void OSReport(const char*, ...);
void Matrix_SetIdentityAffine(void*);
void* CourseData_FindNameRecord(void*, const char*);
extern const int s_computerPuttCameraChoices[4], s_computerShotCameraChoices[4];
float Random_RangeFloat(int, float, float);
void Camera_SetClip(void*, float, float, int);
void DrawScene_InvokePrimaryHook(void*);
void CourseWeather_ConfigureEffects(void*, int, int);
void PartyItemSlots_RunLifecycleHook(void*);
void PartyItemSlots_RunFinalLifecycleHook(void*);
void FileIO_GetDefaultDevicePath(int, void*);
int FadeState_GetState(void*);
void* FadeState_GetInstance();
int FadeState_RequestReleaseOnFadeCompletion(void*);
int PartyGame_ApplyAchievedTierRewardToPlayer(int);
#define T_CUR GolfPlayerEntryByte(GolfDocBytes()[0x3C1])
#define T_MSG(n, c, a, b) IActor_DispatchMessage(self, self, golfRuleStrings + (n), (c), (int)(a), (int)(b), 0, 0)
#define T_SOUND(s, loop, key)                                                                         \
    ((int (*)(void*, const char*, float, float, float, int, int, const char*, int))Sound_PlayByName)( \
        g_soundManager, (s), kFloat_0_5_GolfRule, kFloat_200_GolfRule, kFloat_1_805DEA38, 0, (loop), (key), 0)
#define T_SINGLE(registration)                                                                    \
    do {                                                                                          \
        if (!Sing_805DC608()) {                                                                   \
            if (!s_fadeOverlayInitialized) {                                                      \
                FadeOverlay_Init(s_fadeOverlay);                                                  \
                __register_global_object(s_fadeOverlay, (void*)ResourceState_Dtor, registration); \
                s_fadeOverlayInitialized = 1;                                                     \
            }                                                                                     \
            Sing_805DC608() = s_fadeOverlay;                                                      \
        }                                                                                         \
    } while (0)
#define T_CLEAR()                                                     \
    do {                                                              \
        unsigned char* g = GolfDocBytes();                            \
        unsigned char* end = g + 0xA0;                                \
        unsigned char* node = *(unsigned char**)(g + 0xA4);           \
        if (node != end) {                                            \
            unsigned char* first = *(unsigned char**)end;             \
            *(int*)(*(unsigned char**)node + 4) = *(int*)(first + 4); \
            **(int**)(first + 4) = *(int*)node;                       \
            while (node != end) {                                     \
                unsigned char* old = node;                            \
                node = *(unsigned char**)(node + 4);                  \
                appFreeUni__FPv(old);                                 \
                --RuleIntAt(g, 0x9C);                                 \
            }                                                         \
        }                                                             \
    } while (0)
// Clear the camera's pending input byte, timer and two-component rotation offset.

#define T_END_UI()                        \
    do {                                  \
        T_MSG(kText_ball, 1, 0, 0);       \
        T_MSG(kText_spot, 4, 0, 0);       \
        T_MSG(kText_grid, 3, 0, 0);       \
        T_MSG(kText_screen, 7, 0, 0);     \
        T_MSG(kText_screen, 3, 0, 0);     \
        T_MSG(kText_effect, 0xB, 0, 0);   \
        T_MSG(kText_trajectory, 6, 0, 0); \
        ((CFx*)g_fx)->ClearEffects(1);    \
    } while (0)
#define T_SINGLE_G(registration)                                                                  \
    do {                                                                                          \
        if (!Sing_805DC608()) {                                                                   \
            if (!s_fadeOverlayInitialized) {                                                      \
                FadeOverlay_Init(s_fadeOverlay);                                                  \
                __register_global_object(s_fadeOverlay, (void*)ResourceState_Dtor, registration); \
                s_fadeOverlayInitialized = 1;                                                     \
            }                                                                                     \
            Sing_805DC608() = s_fadeOverlay;                                                      \
        }                                                                                         \
    } while (0)
// Test the current player equipment identifier against an inventory entry.

// Test the current player equipment identifier against an inventory entry.

// Check the current player against the hole penalty/stroke limit.

// Initialize the requested golf state, including UI, camera, swing controller and shot simulation.
// flag selects one GolfRuleState value; it is stored as gameStateFlags after
// the drop-confirmation gate. Most cases return after initialization; the
// ongoing work is dispatched by OnProcess. Recursive calls deliberately enter
// the next state immediately. A pending drop dialog may defer entry entirely.
// The long ButtonShotAnimation branch repeats the launch/sample pipeline above
// with state-entry-specific gates. It is not interchangeable with a call to
// BuildShotSimulation: heading adjustment and feedback timing differ.
// Enter the advancing button gauge, disable world effects, and seed its mode speed.

// Seed the tee/pin midpoint, configure the overview camera, and hide player models.

// Prepare the fight result presentation.

// Publish the story sequence state and its presentation.

// Prepare the round-results display.

// Bind motion-camera input and initialize its pending state.

// Reset camera input and schedule the turn-start delay.

// Initialize aiming UI and controller state for the current ball.

// Set the camera transition pose and publish camera mode two.

// Apply turn bookkeeping, clear shot UI, and choose the next player phase.

// Publish the requested golf state after the drop-confirmation gate, then run its entry actions.

extern float kFloat_3_0;
extern float kGolfCameraFovRadians;
void FadeState_StartFadeIn(void*, int);
void* FadeState_GetInstance(void);
int FadeState_RequestReleaseOnFadeCompletion(void*);
int SharedDoc_IsMode6PartyMode0();
signed char ItemInventory_FindCharacterIndex(void*, int);

// Process per-player hole results, course-record updates and next-round choices.

} // extern "C"
} // namespace ns4

extern "C" {}
/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU80462130", _ctors_u8014B210_0, CGolfRule::RegisterType);
#pragma pop

// Out-of-line definitions follow the original address order.
#pragma push

// ===== S1 - Type anchor and player thunks =====
// Secondary-interface forwarding; no gameplay tuning.
extern "C" void __dt__11CGolfPlayerFv(void* player, int deleting);

// Adjust the secondary-interface this pointer and forward to the owning actor method.
void CGolfPlayer::vfunc_46Thunk18(void* command, int playerIndex) {
    ParseMaterialLayerAnimationCommand((char*)this - 0x18, command, playerIndex);
}

// Adjust the secondary-interface this pointer and forward to the owning actor method.
void CGolfPlayer::ParseInterpolationCommandThunk18(int* commandCursor, int playerIndex) {
    ParseInterpolationCommand((char*)this - 0x18, commandCursor, playerIndex);
}

// Adjust the secondary-interface this pointer and forward to the owning actor method.
void CGolfPlayer::vfunc_44Thunk18(int* commandCursor, int playerIndex) {
    PlayAnimationIfPartPresent((char*)this - 0x18, commandCursor, playerIndex);
}

// Adjust the secondary-interface this pointer and forward to the owning actor method.
void CGolfPlayer::vfunc_43Thunk18(int* commandCursor, int playerIndex) {
    ((CGolfPlayer*)((char*)this - 0x18))->HideNamedPartLayers(commandCursor, playerIndex);
}

// Adjust the secondary-interface this pointer and forward to the owning actor method.
void CGolfPlayer::vfunc_42Thunk18(int* commandCursor, int playerIndex) {
    ((CGolfPlayer*)((char*)this - 0x18))->ShowNamedPartLayers(commandCursor, playerIndex);
}

// Adjust the secondary-interface this pointer and forward to the owning actor method.
void CGolfPlayer::vfunc_48Thunk18(void* command, int playerIndex) {
    ((CGolfPlayer*)((char*)this - 0x18))->ParseBoneReferenceCommand(command, playerIndex);
}

// Adjust the secondary-interface this pointer and forward to the owning actor method.
void CGolfPlayer::vfunc_41Thunk18(void* command, int playerIndex) {
    ((CGolfPlayer*)((char*)this - 0x18))->LockClubAttachment(command, playerIndex);
}

// Adjust the secondary-interface this pointer and forward to the owning actor method.
void CGolfPlayer::vfunc_40Thunk18(int command, int playerIndex) {
    ((CGolfPlayer*)((char*)this - 0x18))->HideClub(command, playerIndex);
}

// Adjust the secondary-interface this pointer and forward to the owning actor method.
void CGolfPlayer::vfunc_39Thunk18(int command, int playerIndex) {
    ((CGolfPlayer*)((char*)this - 0x18))->ShowClub(command, playerIndex);
}

// Adjust the secondary-interface this pointer and forward to the owning actor method.
void CGolfPlayer::StartTailEffectThunk18() {
    ((CGolfPlayer*)((char*)this - 0x18))->StartTailEffect();
}

// Adjust the secondary-interface this pointer and forward to the owning actor method.
void CGolfPlayer::vfunc_37Thunk18(int command, int playerIndex) {
    ((CGolfPlayer*)((char*)this - 0x18))->PlayShotSoundsAndStartBallFlight(command, playerIndex);
}

// Adjust the secondary-interface this pointer and forward to the owning actor method.
void CGolfPlayer::PlaySoundCommandThunk18(int* commandCursor, int playerIndex) {
    ((CGolfPlayer*)((char*)this - 0x18))->PlaySoundCommand(commandCursor, playerIndex);
}

// Adjust the secondary-interface this pointer and forward to the owning actor method.
void CGolfPlayer::vfunc_36Thunk18() {
    ((CGolfPlayer*)((char*)this - 0x18))->vfunc_36();
}

// The secondary interface's destructor entry. It forwards the deleting flag to
// the real destructor of the complete object.
void CGolfPlayer::Destroy(int deleting) {
    __dt__11CGolfPlayerFv((char*)this - 0x18, deleting);
}

// Adjust the secondary-interface this pointer and forward to the owning actor method.
int CGolfPlayer::vfunc_47Thunk18(void* command, int playerIndex, void* condition) {
    return ((CGolfPlayer*)((char*)this - 0x18))->ParseConditionedCommandToken(command, playerIndex, condition);
}

namespace ns0 {
extern "C" {
// Allocate and construct the golf-rule actor.
// No arguments; allocates an AllocatedObject, whose constructor calls CGolfRule::Init.
// Returns the allocation pointer; the caller receives ownership.
// There is no local allocation-failure branch or recovery in this wrapper.
void* CGolfRule_Create(void) {
    return new AllocatedObject;
}
}
}

// Initialize rule-owned resources, per-player history and shot/network state.
// Requires writable rule storage and initialized shared document. Constructs the base,
// four history headers and empty owned containers; clears per-shot/network/menu state.
// Returns self. Party mode is cleared outside game modes 6/7; no local failure return.
void* CGolfRule::Init() {
    using namespace ns0;
    char* p = (char*)this;
    __ct__18NativeBase80201D18Fv(this);
    void* vtbl = (void*)__vt__9CGolfRule;
    *(void**)p = vtbl;
    void* vtbl2 = (void*)(__vt__9CGolfRule + 0x3c);
    AsCGolfRule(p)->secondaryVtable = vtbl2;
    int z = 0;
    AsCGolfRule(p)->roundRestarted = z;
    __construct_array(p + 0xac, (void*)Struct24_Clear, (void*)RuleShotHistory_dtor, 0x18, 4);
    AsCGolfRule(p)->cameraPathPoints = z;
    AsCGolfRule(p)->cameraPathPointCount = z;
    AsCGolfRule(p)->cameraPathPointCapacity = z;
    AsCGolfRule(p)->bgmNames = z;
    AsCGolfRule(p)->bgmNameCount = z;
    AsCGolfRule(p)->bgmNameCapacity = z;
    AsCGolfRule(p)->playerResultSequenceActive = z;
    AsCGolfRule(p)->playerResultSequenceIndex = z;
    AsCGolfRule(p)->playerResultSequencePhase = z;
    AsCGolfRule(p)->resultMenu = z;
    AsCGolfRule(p)->retryMenu = z;
    AsCGolfRule(p)->postRoundMenuState = z;
    AsCGolfRule(p)->nextRoundAction = z;
    (*(int*)&AsCGolfRule(p)->padSwingController) = z;
    (*(int*)&AsCGolfRule(p)->childObjects) = z;
    AsCGolfRule(p)->childObjectCount = z;
    AsCGolfRule(p)->childObjectCapacity = z;
    AsCGolfRule(p)->fastPlayback = z;
    AsCGolfRule(p)->unk_8 = z;
    void* g = g_sharedDoc;
    unsigned int mode = AsCSharedDoc(g)->gameMode;
    if (mode != 7 && mode != 6)
        (*(int*)&AsCSharedDoc(g)->partyGameMode) = z;
    float f0 = kFloat_0_GolfRule;
    AsCGolfRule(p)->unk_43C = z;
    AsCGolfRule(p)->unk_440 = z;
    AsCGolfRule(p)->unk_444 = z;
    AsCGolfRule(p)->unk_448 = z;
    AsCGolfRule(p)->unk_44C = z;
    AsCGolfRule(p)->unk_450 = z;
    AsCGolfRule(p)->forgetItemDialog = z;
    AsCGolfRule(p)->forgetDlg = z;
    AsCGolfRule(p)->forgetDlgTime = f0;
    AsCGolfRule(p)->forgetItemSlot =
        (int)ItemInventory_GetItemIdAt(((char*)&AsCSharedDoc(g_sharedDoc)->itemInventory), 0xe);
    AsCGolfRule(p)->unk_404 = z;
    AsCGolfRule(p)->miniGameSystemResult = z;
    AsCGolfRule(p)->unk_454 = z;
    memset(p + 0x458, 0, 0xc);
    AsCGolfRule(p)->unk_468 = z;
    char* playerEquipmentWords = p + 0x260;
    int playerByteIndex = 0;
    AsCGolfRule(p)->unk_46C = z;
    AsCGolfRule(p)->unk_464 = z;
    AsCGolfRule(p)->unk_48E = z;
    AsCGolfRule(p)->unk_48D = z;
    AsCGolfRule(p)->unk_470 = z;
    for (int i = 0; i < 4; i++) {
        char* playerByteBase = p + playerByteIndex;
        *(playerByteBase + 0x25c) = z;
        memset(playerEquipmentWords, 0, 0x20);
        playerByteIndex += 1;
        playerEquipmentWords += 0x20;
    }
    return this;
}

namespace ns0 {
extern "C" {
// ===== S2 - Rule lifecycle and player snapshots =====
// Rule/document/player views; constructs and tears down owned services.
}
}

namespace ns0 {
extern "C" {
// Initialize the six-word shot-history header to an empty state.
// Requires six writable 32-bit words (the 24-byte shot-history header).
// Sets each word to zero; does not allocate, free or send messages.
// No return value or failure branch.
void Struct24_Clear(void* self) {
    int* words = (int*)self;
    words[0] = 0;
    words[1] = 0;
    words[2] = 0;
    words[3] = 0;
    words[4] = 0;
    words[5] = 0;
}
}
}

namespace ns0 {
extern "C" {
// Destroy the owned container storage and optionally free the object.
// Accepts null self; otherwise releases storage referenced by header word 0.
// Clears the live size word before freeing that storage; deleting > 0 frees self.
// Returns self, including after deletion; no error result is produced.
void* dtor_8014B478(void* self, int deleting) {
    if (self != 0) {
        if (self != 0) {
            if (self != 0) {
                if (self != 0) {
                    int* words = (int*)self;
                    if (words[0] != 0) {
                        words[2] -= *(volatile int*)&words[2];
                        appFreeUni__FPv((void*)words[0]);
                    }
                }
            }
        }
        if (deleting > 0)
            __dl__FPv(self);
    }
    return self;
}
}
}

namespace ns0 {
extern "C" {
// Destroy the owned container storage and optionally free the object.
// Accepts null self; otherwise releases storage referenced by header word 0.
// Zeros header word 2 before that release; deleting > 0 additionally frees self.
// Returns self, including after deletion; no error result is produced.
void* dtor_8014B4F4(void* self, int deleting) {
    if (self != 0) {
        if (self != 0) {
            if (self != 0) {
                if (self != 0) {
                    if (self != 0) {
                        if (self != 0) {
                            int* words = (int*)self;
                            if (words[0] != 0) {
                                words[2] = words[2] - words[2];
                                appFreeUni__FPv((void*)words[0]);
                            }
                        }
                    }
                }
            }
        }
        if (deleting > 0)
            __dl__FPv(self);
    }
    return self;
}
}
}

namespace ns0 {
extern "C" {
// Destroy the owned container storage and optionally free the object.
// Accepts null self; otherwise forwards header destruction to RingBuffer32_dtor.
// deleting > 0 additionally frees the outer object after member cleanup.
// Returns self; the nested null tests retain the original compiler shape.
void* RuleShotHistory_dtor(void* self, int deleting) {
    if (self != 0) {
        if (self != 0) {
            if (self != 0) {
                if (self != 0) {
                    if (self != 0) {
                        RingBuffer32_dtor(self, 0);
                    }
                }
            }
        }
        if (deleting > 0)
            __dl__FPv(self);
    }
    return self;
}
}
}

namespace ns0 {
extern "C" {
// Destroy the owned container storage and optionally free the object.
// Forwards self and deleting to the selected MSL vector-deleting destructor.
// The V8014B5E0 element-destruction specialization and allocator define cleanup.
// Returns the delegated pointer; this wrapper adds no failure handling.
void* dtor_8014B5E0(void* self, int deleting) {
    return std::__vector_deleting_dtor((V8014B5E0*)self, deleting);
}
}
}

// Destroy the child, BGM-name and camera-path containers, then the four shot
// histories and the native base, in that order.
CGolfRule::~CGolfRule() {
    using namespace ns0;
    ((GolfRuleChildVector*)&childObjects)->~GolfRuleChildVector();
    ((GolfRuleNameVector*)&bgmNames)->~GolfRuleNameVector();
    ((GolfRulePathVector*)&cameraPathPoints)->~GolfRulePathVector();
    __destroy_arr(((RuleShotHistories*)this)->players, (void*)RuleShotHistory_dtor, 0x18, 4);
    if (this != 0)
        __dt__18NativeBase80201D18Fv(this, 0);
}

// Reset round state and create camera, rival and swing-related services before loading.
// Requires rule storage, shared golf document and audio/camera services.
// Resets hole/rule state, creates camera/rival/random/drive-line objects and builds
// the playlist. Path buffer is 0x200 bytes and sprintf formats it into itself.
// Switches sound banks and sets Pang multiplier to 1; no failure return.
void CGolfRule::OnPreLoadInit() {
    using namespace ns0;
    char* p = (char*)this;
    (*(int*)&AsCSharedDoc(g_sharedDoc)->unk_AC4) = 0;
    AsCGolfRule(p)->unk_350 = 0;
    ((CGolfDoc*)((*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc)))->AllocateHoleRecords();
    ((CGolfDoc*)((*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc)))->SetCurrentHole(0);
    float fov = kGolfCameraFovRadians;
    AsCGolfDoc((*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc))->tutorialStage = 6;
    Camera_SetFov(g_view, fov);
    Camera_SetClip(g_view, kFloat_3_0, kFloat_100000_GolfRule, 1);
    Matrix_SetIdentityAffine(s_cameraTransform);
    Matrix_SetIdentityAffine((char*)(*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc) + 0x324);
    float z = kFloat_0_GolfRule;
    AsCGolfRule(p)->shotPresentationFlags = 0;
    AsCGolfRule(p)->unk_3B0 = 0;
    (*(char*)&AsCGolfRule(p)->processingPaused) = 0;
    (*(char*)&AsCGolfRule(p)->networkSyncPending) = 0;
    AsCGolfRule(p)->netMsgSend = 0;
    AsCGolfRule(p)->netMsgRecv = 0;
    AsCGolfRule(p)->netMsgSendTime = 0;
    AsCGolfRule(p)->netMsgRecvTime = 0;
    AsCGolfRule(p)->netMsgWaitLine = -1;
    (*(char*)&AsCGolfRule(p)->hudVisible) = 1;
    (*(char*)&AsCGolfRule(p)->selectedBgmIndex) = 0;
    AsCGolfRule(p)->deferredInputButtons = 0;
    AsCGolfRule(p)->unk_3F4 = 0;
    AsCGolfRule(p)->launchHeadingOffset = z;
    AsCGolfRule(p)->displayUpdateCounter = 0;
    AsCGolfRule(p)->unk_480 = 1;
    AsCGolfRule(p)->forceDisplayUpdate = 0;
    (*(char*)&AsCGolfRule(p)->autoShot) = 0;
    AsCGolfRule(p)->unk_3F0 = 0;
    (*(char*)&AsCGolfRule(p)->nextPlayerIndex) = 0;
    AsCGolfRule(p)->stateTransitionDialog = 0;
    (*(char*)&AsCGolfRule(p)->shotBlock) = 0;
    AsCGolfRule(p)->shotBlockTime = z;
    AsCGolfRule(p)->tutorialFeedbackEnabled = 0;
    AsCGolfRule(p)->tutorialAdvancePending = 0;
    AsCGolfRule(p)->unk_3F8 = 0;
    AsCGolfRule(p)->unk_498 = 0;
    AsCGolfRule(p)->impactGaugeDirection = -1;
    AsCGolfRule(p)->fastPlayback = 0;
    AsCMouseCursor(g_mouseCursor)->autoHideEnabled = 1; // original +0xBC
    new Obj300;
    new ObjB0;
    new Obj1C;
    new Obj2C;
    char buffer[0x200];
    FileIO_GetDefaultDevicePath(0x200, buffer);
    sprintf(buffer, (char*)golfRuleStrings + kText_bgmDirectoryFormat, buffer);
    BuildBgmPlaylist(buffer);
    AsCSharedDoc(g_sharedDoc)->pangRewardMultiplier = kFloat_1_805DEA38;
    // This shared round byte is reset here; its purpose is still unclassified.
    *((char*)g_sharedDoc + 0x8a4) = 0;
    SoundManager_ReleaseSoundBank(g_soundManager, (char*)golfRuleStrings + kText_menuSoundBank);
    SoundManager_AcquireSoundBank(g_soundManager, (char*)golfRuleStrings + kText_golfSoundBank);
    float w = kFloat_0_GolfRule;
    AsCGolfRule(p)->unk_41C = 0;
    AsCGolfRule(p)->coinTime = w;
    AsCGolfRule(p)->coinCount = 0;
    AsCGolfRule(p)->coinNum = 0;
}

// Start course-related loads and construct child preload tasks.
// Requires rule-owned child container and documents. Generates drive-line markers
// unless rule type 2, destroys/clears previous children, then queues shot/party and
// ball-effect preloads before the base load hook. No success/failure return.
void CGolfRule::OnLoad() {
    using namespace ns0;
    if (AsCSharedDoc(g_sharedDoc)->golfRuleType != 2) {
        unsigned char val = SharedDoc_GetCourseId(this);
        DriveLineMarkers_Generate((char*)(*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc) + 0x36c, val);
    }
    void** p = (*(void***)&AsCGolfRule(this)->childObjects);
    int zero = 0;
    while ((*(void***)&AsCGolfRule(this)->childObjects) + AsCGolfRule(this)->childObjectCount != p) {
        if (*p) {
            if (*p) {
                ((void (*)(void*, int))((*(void***)*p)[2]))(*p, 1);
            }
        }
        *p = (void*)zero;
        ++p;
    }
    AsCGolfRule(this)->childObjectCount = AsCGolfRule(this)->childObjectCount - AsCGolfRule(this)->childObjectCount;
    ((CGolfRule*)this)->PreloadShotAndPartySequences();
    ((CGolfRule*)this)->PreloadBallEffects();
    ((void (*)(void*))((*(void***)this)[7]))(this);
}

#pragma use_lmw_stmw on
// Load computer-shot tables and restore per-hole/player state for a course reload.
// Requires loaded shared/course services; invalidates player reset state first.
// Course-hole load failure sets g_courseLoadFailed and returns. Successful loading
// busy-polls CPU tables, refreshes player snapshots/HUD, seeds MT19937 from the
// millisecond clock and rebuilds shadow capture storage. No timeout on those polls.
void CGolfRule::OnReload() {
    using namespace ns0;
    unsigned char i;
    void* resource;
    void* layout;
    // Invalidate prior round/player reset records before loading the replacement shot resources.
    SharedDoc_InvalidatePlayerState(g_sharedDoc);
    ((CGolfDoc*)((*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc)))->ClearPlayerBallResetRecords();
    i = 0;
    while (i < AsCSharedDoc(g_sharedDoc)->roundHoleCount) {
        void* mgr = (*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc);
        *(unsigned char*)((char*)AsCGolfDoc(mgr)->holeRecords + i * 0x20 + 0x19) = 0;
        if ((((unsigned char (*)())SharedDoc_GetCourseId)() == 0xfd ||
             ((unsigned char (*)())SharedDoc_GetCourseId)() == 0xfe) &&
            i == 0)
            break;
        ++i;
    }
    void* global = g_sharedDoc;
    void* manager = (*(void**)&AsCSharedDoc(global)->golfDoc);
    if (!((CAllCourse*)(*(void**)((char*)manager + 8)))->LoadRoundHoleRecords(AsCSharedDoc(global)->roundHoleCount)) {
        *(unsigned char*)&g_courseLoadFailed = 1;
        return;
    }
    resource = *(void**)&g_rival;
    layout = ((CRival*)resource)->GetThinkController();
    {
        int tailIndex;
        unsigned int n;
        const char* str;
        BAStr s;
        str = ((char*)golfRuleStrings) + kText_computerThinkPrefix;
        n = strlen(str);
        String_ReserveCapacity(&s, n);
        BAAlloc a0e;
        BAAlloc a0c = a0e;
        String_ReplaceRange(&s, 0, 0, str, str + n, (const char*)&a0c);
        const char* player = ((SharedComputerRoundFields*)g_sharedDoc)->profileName;
        unsigned int i1 = s.index();
        BAAlloc b1e;
        BAAlloc b1c = b1e;
        String_ReplaceRange(&s, i1, 0, player, player + strlen(player), (const char*)&b1c);
        str = ((char*)golfRuleStrings) + kText_ecbExtension;
        tailIndex = s.index();
        BAAlloc c1e;
        BAAlloc c1c = c1e;
        String_ReplaceRange(&s, tailIndex, 0, str, str + strlen(str), (const char*)&c1c);
        ((CComThinkCtrl*)layout)->RequestFileLoad(s.cstr());
        while (!((CComThinkCtrl*)layout)->PollFileLoad()) {
        }
        void* pane = AsCRival(resource)->randomUse;
        ((CComRandomUse*)pane)->RequestShotTableLoad(((char*)golfRuleStrings) + kText_com_random_tbl_shot_ecb);
        ((CComRandomUse*)pane)->RequestPuttTableLoad(((char*)golfRuleStrings) + kText_com_random_tbl_pat_ecb);
        ((CComRandomUse*)pane)->RequestNoTargetTableLoad(((char*)golfRuleStrings) + kText_com_random_tbl_approach_ecb);
        ((CComRandomUse*)pane)->RequestSpecialTableLoad(((char*)golfRuleStrings) + kText_com_random_tbl_special_ecb);
        while (!((CComRandomUse*)pane)->AreTablesLoaded()) {
        }
        global = g_sharedDoc;
        if ((*(int*)&AsCSharedDoc(global)->gameMode) == 0 && AsCSharedDoc(global)->golfRuleType == 0) {
            int tailIndex;
            unsigned int m;
            const char* str;
            resource = AsCRival(resource)->strokeUse;
            BAStr t;
            str = ((char*)golfRuleStrings) + kText_computerStrokePrefix;
            m = strlen(str);
            String_ReserveCapacity(&t, m);
            BAAlloc d0e;
            BAAlloc d0c = d0e;
            String_ReplaceRange(&t, 0, 0, str, str + m, (const char*)&d0c);
            const char* player = ((SharedComputerRoundFields*)g_sharedDoc)->profileName;
            unsigned int u1 = t.index();
            BAAlloc e1e;
            BAAlloc e1c = e1e;
            String_ReplaceRange(&t, u1, 0, player, player + strlen(player), (const char*)&e1c);
            str = (const char*)((unsigned int)((char*)golfRuleStrings) + kText_ecbExtension);
            tailIndex = t.index();
            BAAlloc f1e;
            BAAlloc f1c = f1e;
            String_ReplaceRange(&t, tailIndex, 0, str, str + strlen(str), (const char*)&f1c);
            ((CComStrokeUse*)resource)->RequestFileLoad((void*)t.cstr());
            while (!((CComStrokeUse*)resource)->IsFileLoaded()) {
            }
            Generate_ComputerStrokeScores(resource, ((SharedComputerRoundFields*)g_sharedDoc)->strokeScores);
        }
    }
    ((CGolfRule*)this)->SetupPlayerSnapshots(1);
    unsigned char slot = AsCGolfRule(this)->hudVisible;
    IActor_DispatchMessage(this, this, ((char*)golfRuleStrings) + kText_screen, 5, (void*)(unsigned int)slot, 0, 0, 0);
    slot = AsCGolfRule(this)->hudVisible;
    IActor_DispatchMessage(this, this, ((char*)golfRuleStrings) + kText_letter, 4, (void*)(unsigned int)slot, 0, 0, 0);
    slot = AsCGolfRule(this)->hudVisible;
    IActor_DispatchMessage(this, this, ((char*)golfRuleStrings) + kText_spot, 9, (void*)(unsigned int)slot, 0, 0, 0);
    AsCMouseCursor(g_mouseCursor)->enabled = AsCGolfRule(this)->hudVisible; // original +0x105
    MT19937_Seed(g_randomHouse, g_currentTimeMilliseconds);
    ((CCameraWork*)g_cameraWork)->PreloadPlayerAndIndexedGroups();
    ((CMouseCursor*)g_mouseCursor)->Reset(1);
    int captureQueueDimension = 0x100;
    AsCMouseCursor(g_mouseCursor)->hitTestMode = 1; // original +0x38
    unsigned char mode = ((unsigned char (*)())SharedDoc_GetCourseId)();
    switch (mode) {
    case 4:
    case 5:
    case 6:
    case 7:
    case 8:
    case 9:
    case 13:
        captureQueueDimension = 0x80;
    }
    ((CShadowManager*)g_shadowManager)->ClearEntriesAndResizeCaptureQueue(captureQueueDimension, captureQueueDimension);
    unsigned char count = *((unsigned char*)(*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc) + 0x2b9);
    ShadowManager_SetResourceLookupEnabled(g_shadowManager, count <= 2);
}

#pragma use_lmw_stmw reset
namespace ns0 {
extern "C" {
static inline RoundSelectionState* GetRoundSelection() {
    return (RoundSelectionState*)g_sharedDoc;
}
}
}

// Allocate player snapshots and populate them from selected save records.
// Requires round-player list and selected documents. Allocates player snapshots
// for supported rule types and initializes equipment, CPU/rank bytes and actors.
// loadSelectedEquipment chooses save/selection import versus stored equipment reuse.
// May lazily construct save/render services; recounts active players before return.
void CGolfRule::SetupPlayerSnapshots(int loadSelectedEquipment) {
    using namespace ns0;
    RulePlayerSetup* self = (RulePlayerSetup*)this;
    RoundPlayerList* list = (RoundPlayerList*)s_roundPlayerList;
    int mode = GetRoundSelection()->golfRuleType;
    switch (mode) {
    case 0:
    case 1:
    case 2: {
        ((CGolfDoc*)(GetRoundSelection()->golfDoc))->AllocatePlayerSnapshots((unsigned char)list->count);
        char* p;
        char* pool;
        RoundPlayerNode* sentinel;
        RoundPlayerNode* node;
        unsigned int selectedPlayerIndex;
        RoundPlayerNode* it;
        int bestControllerIndex;
        node = list->first;
        selectedPlayerIndex = 0;
        bestControllerIndex = 4;
        for (it = node; (char*)it != (char*)list + kRoundPlayerSentinelLinkOffset; it = it->next) {
            if (it->isComputer != 1 &&
                (bestControllerIndex > it->controllerIndex ||
                 (bestControllerIndex == it->controllerIndex && selectedPlayerIndex > it->playerIndex))) {
                selectedPlayerIndex = it->playerIndex;
                bestControllerIndex = *(volatile int*)&it->controllerIndex;
            }
        }
        sentinel = (RoundPlayerNode*)&list->sentinelLink;
        pool = s_renderContext;
        for (; node != sentinel; node = node->next) {
            if (loadSelectedEquipment) {
                if (selectedPlayerIndex == node->playerIndex) {
                    if (!GetSaveDataSlot()) {
                        if (!s_saveDataInitialized) {
                            SaveData_Ctor(s_saveData);
                            s_saveDataInitialized = 1;
                        }
                        GetSaveDataSlot() = (void*)s_saveData;
                    }
                    SharedDoc_CopyPlayerSnapshot(SaveData_GetCurrentRecord(GetSaveDataSlot()), node->playerIndex);
                } else if (GetRoundSelection()->gameMode == 1 || GetRoundSelection()->gameMode == 7) {
                    if (!Sing_805DC648()) {
                        if (!s_renderContextInitialized) {
                            p = pool;
                            do {
                                SaveData_InitPlayerRecord(p);
                                p += 0x828;
                            } while (p < pool + 0x20a0);
                            ((IntVectorArray*)(pool + 0x20a0))->__ct();
                            p = pool + 0x20d0;
                            __construct_array(p, (void*)PartyItemRecord_InitDefaults, (void*)PlayerRenderRecord_dtor,
                                              0xa4, 4);
                            *(int*)(p + 0x290) = 0;
                            try {
                                RenderContext_ResetPlayerRecords(pool);
                            } catch (...) {
                                ((void (*)(void*, int))CharacterPreviewTable_dtor)(pool + 0x20D0, -1);
                                ((IntVectorArray*)(pool + 0x20A0))->~IntVectorArray();
                                throw;
                            }
                            __register_global_object(s_renderContext, (void*)__dt__13RenderContextFv,
                                                     s_renderContextDtorRegistration_804F72B4);
                            s_renderContextInitialized = 1;
                        }
                        Sing_805DC648() = pool;
                    }
                    GetRoundSelection()->players[node->playerIndex].pangBalance =
                        ((RenderPlayerSave*)Sing_805DC648())[node->playerIndex].pangBalance;
                } else {
                    GetRoundSelection()->players[node->playerIndex].pangBalance = 0;
                }
                self->slot[node->playerIndex].equipmentWords[0] =
                    GetRoundSelection()->players[node->playerIndex].equipmentWords[0];
                self->slot[node->playerIndex].equipmentWords[1] =
                    GetRoundSelection()->players[node->playerIndex].equipmentWords[1];
                self->slot[node->playerIndex].equipmentWords[2] =
                    GetRoundSelection()->players[node->playerIndex].equipmentWords[2];
                self->slot[node->playerIndex].equipmentWords[3] =
                    GetRoundSelection()->players[node->playerIndex].equipmentWords[3];
                self->slot[node->playerIndex].equipmentWords[4] =
                    GetRoundSelection()->players[node->playerIndex].equipmentWords[4];
                self->slot[node->playerIndex].equipmentWords[5] =
                    GetRoundSelection()->players[node->playerIndex].equipmentWords[5];
                self->slot[node->playerIndex].equipmentWords[6] =
                    GetRoundSelection()->players[node->playerIndex].equipmentWords[6];
                self->slot[node->playerIndex].equipmentWords[7] =
                    GetRoundSelection()->players[node->playerIndex].equipmentWords[7];
            } else if (GetRoundSelection()->gameMode != 0 && !SharedDoc_IsMode6PartyMode0()) {
                GetRoundSelection()->players[node->playerIndex].equipmentWords[0] =
                    self->slot[node->playerIndex].equipmentWords[0];
                GetRoundSelection()->players[node->playerIndex].equipmentWords[1] =
                    self->slot[node->playerIndex].equipmentWords[1];
                GetRoundSelection()->players[node->playerIndex].equipmentWords[2] =
                    self->slot[node->playerIndex].equipmentWords[2];
                GetRoundSelection()->players[node->playerIndex].equipmentWords[3] =
                    self->slot[node->playerIndex].equipmentWords[3];
                GetRoundSelection()->players[node->playerIndex].equipmentWords[4] =
                    self->slot[node->playerIndex].equipmentWords[4];
                GetRoundSelection()->players[node->playerIndex].equipmentWords[5] =
                    self->slot[node->playerIndex].equipmentWords[5];
                GetRoundSelection()->players[node->playerIndex].equipmentWords[6] =
                    self->slot[node->playerIndex].equipmentWords[6];
                GetRoundSelection()->players[node->playerIndex].equipmentWords[7] =
                    self->slot[node->playerIndex].equipmentWords[7];
            }
            ((PlayerSnapshotSetup*)GolfPlayer_GetEntryByIndex(GetRoundSelection()->golfDoc,
                                                              (unsigned char)node->playerIndex))
                ->isComputer = node->isComputer == 1;
            PlayerSnapshotSetup* nodeEntry = (PlayerSnapshotSetup*)GolfPlayer_GetEntryByIndex(
                GetRoundSelection()->golfDoc, (unsigned char)node->playerIndex);
            nodeEntry->rankTieBreakOrder = node->playerIndex;
            if (GetRoundSelection()->partyGameMode == 3) {
                void* partyItemSlots = PartyItemSlots_GetInstance();
                ((CBpItemVector*)partyItemSlots)
                    ->SetItemAtSlot(node->partyItemSlot, node->playerIndex, node->controllerIndex);
                PartyItemSlots_RefreshDisplay(PartyItemSlots_GetInstance());
            }
            ((CGolfRule*)this)
                ->InitializePlayerSnapshot(
                    (unsigned char)node->playerIndex, node->name, node->name, (void*)node->playerIndex,
                    (void*)GetRoundSelection()->players[node->playerIndex].caddieId, (void*)node->controllerIndex);
        }
        if (GetRoundSelection()->golfRuleType == 2) {
            if (GetRoundSelection()->gameMode == 4)
                GetRoundSelection()->golfDoc->tutorialStage = 2;
            else
                GetRoundSelection()->golfDoc->tutorialStage = 1;
            PlayerSnapshotSetup* entry =
                (PlayerSnapshotSetup*)GolfPlayer_GetEntryByIndex(GetRoundSelection()->golfDoc, 0);
            entry->powerShotMeter = kFloat_0_GolfRule;
        }
        self->turn = 0;
    }
    }
    ((CGolfDoc*)(GetRoundSelection()->golfDoc))->RecountActivePlayers();
}

// Attach the loaded course and create the pad swing controller.
// Requires rule/services. If selected course data exists, publishes its Pang rate
// to the quadtree's slopeGravityScale; invokes the scene hook and clears trajectoryEventDispatched.
// Destroys any old pad controller, creates its replacement and stores it; no result.
void CGolfRule::OnInit() {
    using namespace ns0;
    void* g = g_sharedDoc;
    void* stage = AsCSharedDoc(g)->selectedCourseData;
    if (stage) {
        float value = ((IFF_STRUCT::sCourse*)stage)->pangRate;
        void* obj = CQuadTree_GetInstance();
        AsCQuadTree(obj)->slopeGravityScale = value;
    }
    DrawScene_InvokePrimaryHook(g_sceneManager);
    int* p = (int*)this;
    AsCGolfRule(this)->trajectoryEventDispatched = 0;
    if ((*(int*)&AsCGolfRule(p)->padSwingController)) {
        if ((*(int*)&AsCGolfRule(p)->padSwingController))
            ((VBase*)(void*)(*(int*)&AsCGolfRule(p)->padSwingController))->destroy(1);
    }
    (*(int*)&AsCGolfRule(p)->padSwingController) = 0;
    NewObj* obj = new NewObj;
    (*(int*)&AsCGolfRule(p)->padSwingController) = (int)obj;
}

// Reset the ball, ground lookup and wind before selecting the next hole state.
// Requires rule, current golf document, ball and terrain services. Tutorial setup
// resets to tee plus ball radius, stores the ground handle unchanged and resets
// player/history records before selecting a player. Ordinary flow initializes wind
// in game mode 6, polls story admission, then enters story or hole-camera state.
void CGolfRule::ResetHoleBallAndWind() {
    using namespace ns0;
    char* p = (char*)this;
    void* global = g_sharedDoc;
    if (AsCGolfDoc((*(void**)&AsCSharedDoc(global)->golfDoc))->tutorialStage < 6) {
        int out2[3];
        int out[3];
        int ints[3];
        ((CQuadTree*)CQuadTree_GetInstance())->InitGroundOverrideBounds();
        void* ballForRadius = GolfBall__Fv();
        float height = AsCGolfBall(ballForRadius)->m_radius;
        float floats[3] = {kFloat_0_GolfRule, height, kFloat_0_GolfRule};
        int slot = AsCGolfDoc(MGR_OF_LBL())->currentHoleIndex;
        char* positions = (*(char**)&AsCGolfDoc(MGR_OF_LBL())->holeRecords);
        int off = slot << 5;
        char* e = positions + off;
        ints[1] = *(int*)(e + 4);
        ints[2] = *(int*)(e + 8);
        ints[0] = *(int*)(positions + off);
        __pl__FRC7WVectorRC7WVector(out, ints, floats);
        out2[0] = out[0];
        out2[1] = out[1];
        out2[2] = out[2];
        void* ballToReset = GolfBall__Fv();
        ((CGolfBall*)ballToReset)->Reset(out2);
        void* ballForGroundQuery = GolfBall__Fv();
        void* groundTriangle = (void*)((CQuadTree*)CQuadTree_GetInstance())->GetMeshIndex(ballForGroundQuery, 0);
        void* c = GolfBall__Fv();
        ((BallGroundContact*)c)->triangle = groundTriangle;
        void* c2 = GolfBall__Fv();
        ((CGolfRule*)this)->ResetPlayerPositionsAndShotHistory((const char*)c2);
        unsigned char index;
        if ((*(int*)&AsCSharedDoc(g_sharedDoc)->gameMode) == 0 && AsCSharedDoc(g_sharedDoc)->golfRuleType == 0) {
            index = 0;
        } else {
            unsigned char active = AsCSharedDoc(g_sharedDoc)->golfRuleType;
            if (active == 1) {
                index = ((CGolfRule*)(p))->turn;
            } else {
                index = 0;
                for (; index < 4;) {
                    void* item = GolfPlayer_GetEntryByIndex(MGR_OF_LBL(), index);
                    if (AsGolfDocSnapshot(item)->turnOrder == 0)
                        break;
                    ++index;
                }
            }
        }
        AsCGolfDoc(MGR_OF_LBL())->currentPlayer = index;
        void* item = GolfPlayer_GetEntryByIndex(MGR_OF_LBL(), AsCGolfDoc(MGR_OF_LBL())->currentPlayer);
        *((unsigned char*)item + AsCGolfDoc(MGR_OF_LBL())->currentHoleIndex + 0x28) = 1;
        void* entry = GolfPlayer_GetEntryByIndex(MGR_OF_LBL(), AsCGolfDoc(MGR_OF_LBL())->currentPlayer);
        *(int*)((char*)entry + 8) = 0;
    } else {
        if (AsCSharedDoc(global)->gameMode == 6) {
            CWind::GetInstance()->LoadCourseRegions();
            CWind::GetInstance()->InitOverrideFromCourse();
            *(int*)(p + 0x4c8) = 0;
            void* manager = (*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc);
            if (AsCGolfDoc(manager)->currentHoleIndex == 0) {
                int zero = 0;
                IActor_DispatchMessage(this, this, (char*)golfRuleStrings + kText_storyConte, 4, &zero, p + 0x4c8, 0,
                                       0);
                if (zero) {
                    ((CGolfRule*)this)->EnterState(kStateStorySequence);
                    return;
                }
            }
        }
        int i = 0;
        while (i < AsCGolfDoc((*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc))->playerCount) {
            void* item = GolfPlayer_GetEntryByIndex((*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc), (unsigned char)i);
            int one = *(int*)((char*)item + 4) == 1;
            int present = SharedDoc_IsPlayerFlag0Set(i) != 0;
            if (one != present) {
                IActor_DispatchMessage(this, this, (char*)golfRuleStrings, 0x13, (void*)i, 0, 0, 0);
            }
            ++i;
        }
        ((CGolfRule*)this)->EnterState(kStateHoleCameraPath);
    }
}

namespace ns1 {
extern "C" {
static inline void ClearPlayerHoleResult(GolfDocSnapshot8013* player, int holeIndex) {
    player->holeStrokeCounts[holeIndex] = 0;
    player->holeResultValues[holeIndex] = 0;
    *(int*)&player->holePangTotals[holeIndex * sizeof(int)] = 0;
}
}
}

// Initialize one player snapshot, equipment selection and derived stats.
// Requires rule and documents; returns without changes for an out-of-range player.
// Initializes player state/equipment/score and derived stats. Caddie item data may
// set the starting power meter, with restriction/tutorial/party overrides afterward.
// Builds outfit, compacts inventory and clears per-shot result storage; no error result.
void CGolfRule::InitializePlayerSnapshot(int playerIndex, void* playerName, void* displayName, void* playerSlot,
                                         void* caddieId, void* controllerIndex) {
    using namespace ns1;
    int i = 0;
    void* sharedDoc = g_sharedDoc;
    void* golfDoc = (*(void**)&AsCSharedDoc(sharedDoc)->golfDoc);
    if ((unsigned)playerIndex >= (AsCGolfDoc(golfDoc)->playerCount))
        return;

    float initialPowerMeter = kFloat_0_GolfRule;
    void* snapshot = GolfPlayer_GetEntryByIndex(golfDoc, playerIndex);

    unsigned int initialRank = 1;
    AsGolfDocSnapshot(snapshot)->turnOrder = 0;
    unsigned char* snapshotBytes = (unsigned char*)snapshot;
    AsGolfDocSnapshot(snapshotBytes)->rankFlags =
        (AsGolfDocSnapshot(snapshotBytes)->rankFlags & 0x0F) | (initialRank << 4);

    for (i = 0; i < playerIndex; ++i) {
        golfDoc = (*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc);
        void* o = GolfPlayer_GetEntryByIndex(golfDoc, (unsigned char)i);
        if (*(int*)((char*)o + 8) != 3) {
            AsGolfDocSnapshot(snapshotBytes)->turnOrder = (1 + AsGolfDocSnapshot(snapshotBytes)->turnOrder);
            int v = (AsGolfDocSnapshot(snapshotBytes)->rankFlags >> 4) & 0x0F;
            v = ((v + 1));
            AsGolfDocSnapshot(snapshotBytes)->rankFlags =
                (AsGolfDocSnapshot(snapshotBytes)->rankFlags & 0x0F) | (v << 4);
        }
    }
    int initialPlayerState = 2;
    ((GolfDocSnapshot8013*)(snapshotBytes))->unk_94 = lbl_804AE420[(unsigned char)playerIndex];
    *(void**)&((GolfDocSnapshot8013*)(snapshotBytes))->playerSlot = playerSlot;
    AsGolfDocSnapshot(snapshotBytes)->playerState = initialPlayerState;
    AsGolfDocSnapshot(snapshotBytes)->equippedItemCount = 0;
    AsGolfDocSnapshot(snapshotBytes)->itemId = 0;
    AsGolfDocSnapshot(snapshotBytes)->totalStrokeCount = 0;
    ((GolfDocSnapshot8013*)(snapshotBytes))->unk_C0 = 0;
    AsGolfDocSnapshot(snapshotBytes)->rankFlags = (AsGolfDocSnapshot(snapshotBytes)->rankFlags & 0xF0) | 1;
    unsigned int selectionOffset = playerIndex * 0x1D4;
    sharedDoc = g_sharedDoc;
    if (*(void**)((char*)sharedDoc + selectionOffset + 0x1BC)) {
        if (caddieId) {
            IFF_STRUCT::sCadItem* caddieItem = (IFF_STRUCT::sCadItem*)ItemInventory_FindCaddieItemData(
                (char*)&AsCSharedDoc(sharedDoc)->itemInventory,
                *(void**)((char*)sharedDoc + selectionOffset +
                          0x1BC)); // original r4 = lwz 0x1bc(r3) at 0x8014C97C, kept to the bl
            if (caddieItem)
                initialPowerMeter = (float)((short)caddieItem->initialPowerShotMeter);
        }
    }
    AsGolfDocSnapshot(snapshotBytes)->powerShotMeter = initialPowerMeter;
    if (s_shotActionRestrictionState[0x48 / 4] & 0x08)
        AsGolfDocSnapshot(snapshotBytes)->powerShotMeter = kFloat_0_GolfRule;
    golfDoc = (*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc);
    if (AsCGolfDoc(golfDoc)->tutorialStage < 6)
        AsGolfDocSnapshot(snapshotBytes)->powerShotMeter = kFloat_0_GolfRule;
    if ((*(int*)&AsCSharedDoc(g_sharedDoc)->partyGameMode) == 2)
        AsGolfDocSnapshot(snapshotBytes)->powerShotMeter = kFloat_33_GolfRule;
    // Reuse the caddie argument register for clearing, as required by this build.
    caddieId = 0;
    *(int*)&((GolfDocSnapshot8013*)(snapshotBytes))->roundResultWords[0] = (int)caddieId;
    void* memptr = snapshotBytes + 0xEC;
    *(int*)&((GolfDocSnapshot8013*)(snapshotBytes))->roundResultWords[1] = (int)caddieId;
    *(int*)&((GolfDocSnapshot8013*)(snapshotBytes))->roundResultWords[2] = (int)caddieId;
    *(int*)&((GolfDocSnapshot8013*)(snapshotBytes))->roundResultWords[3] = (int)caddieId;
    snapshotBytes[0x104] = (unsigned char)(int)caddieId;

    memset(memptr, 0, 0x18);
    ((GolfDocSnapshot8013*)(snapshotBytes))->rankTier =
        (unsigned char)PlayerRank_FindTierForValue(*(void**)((char*)g_sharedDoc + selectionOffset + 0x44));
    // unrolled zeroing
    ClearPlayerHoleResult(AsGolfDocSnapshot(snapshotBytes), 0);
    ClearPlayerHoleResult(AsGolfDocSnapshot(snapshotBytes), 1);
    ClearPlayerHoleResult(AsGolfDocSnapshot(snapshotBytes), 2);
    ClearPlayerHoleResult(AsGolfDocSnapshot(snapshotBytes), 3);
    ClearPlayerHoleResult(AsGolfDocSnapshot(snapshotBytes), 4);
    ClearPlayerHoleResult(AsGolfDocSnapshot(snapshotBytes), 5);
    ClearPlayerHoleResult(AsGolfDocSnapshot(snapshotBytes), 6);
    ClearPlayerHoleResult(AsGolfDocSnapshot(snapshotBytes), 7);
    ClearPlayerHoleResult(AsGolfDocSnapshot(snapshotBytes), 8);
    ClearPlayerHoleResult(AsGolfDocSnapshot(snapshotBytes), 9);
    ClearPlayerHoleResult(AsGolfDocSnapshot(snapshotBytes), 10);
    ClearPlayerHoleResult(AsGolfDocSnapshot(snapshotBytes), 11);
    ClearPlayerHoleResult(AsGolfDocSnapshot(snapshotBytes), 12);
    ClearPlayerHoleResult(AsGolfDocSnapshot(snapshotBytes), 13);
    ClearPlayerHoleResult(AsGolfDocSnapshot(snapshotBytes), 14);
    ClearPlayerHoleResult(AsGolfDocSnapshot(snapshotBytes), 15);
    ClearPlayerHoleResult(AsGolfDocSnapshot(snapshotBytes), 16);
    ClearPlayerHoleResult(AsGolfDocSnapshot(snapshotBytes), 17);
    void* sharedDocForOutfit = g_sharedDoc;
    playerSlot = (void*)((unsigned char*)sharedDocForOutfit + selectionOffset);
    void* itemSelection =
        ((CGolfDoc*)((*(void**)&AsCSharedDoc(sharedDocForOutfit)->golfDoc)))->GetItemSelection(playerIndex);

    unsigned char appearance = ((ns0::RoundPlayerSelection*)playerSlot)->appearance;
    ItemSelection_BuildCharacterOutfit(itemSelection, ((ns0::RoundPlayerSelection*)playerSlot)->characterId,
                                       (appearance >> 3) & 0x1F, ((appearance & 7)),
                                       ((ns0::RoundPlayerSelection*)playerSlot)->outfitParts, 0xFF);
    AsGolfDocSnapshot(snapshotBytes)->equippedItemCount =
        (unsigned char)SharedDoc_CompactPlayerItemSlots(g_sharedDoc, playerIndex);
    ((CGolfDoc*)((*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc)))->RefreshPlayerSnapshotStats(playerIndex);
    *(void**)snapshotBytes = controllerIndex;
    *(int*)(snapshotBytes + 0x170) = (int)caddieId;
    *(int*)(snapshotBytes + 0x174) = (int)caddieId;
    *(int*)(snapshotBytes + 0x178) = (int)caddieId;
    Memory_Clear(snapshotBytes + 0x17C, 0x48);
    *(int*)(snapshotBytes + 0x1C4) = (int)caddieId;
}

#pragma opt_propagation off
#pragma use_lmw_stmw on
// Release child tasks, swing controller and shared round services.
// Requires a constructed rule. Deletes camera/rival/random/drive-line services,
// consumes history records and frees spare blocks before deleting the pad controller.
// Destroys child tasks, switches sound banks and stops audio. No failure result;
// this teardown is separate from the container/base destructor.
void CGolfRule::OnDestroy() {
    using namespace ns1;
    unsigned char* base;
    unsigned char* state;
    int i;
    void** array;
    int remain;
    unsigned long stride;
    unsigned long pos;
    unsigned long cap;
    unsigned int slot;
    unsigned long blocks;
    unsigned long offset;
    unsigned char* p = (unsigned char*)this;
    void* x = g_cameraWork;
    if (x) {
        delete (DestroyObject*)x;
    }
    x = g_rival;
    if (x) {
        delete (DestroyObject*)x;
    }
    x = g_randomHouse;
    if (x) {
        delete (DestroyObject*)x;
    }
    x = g_drawDriveLine;
    if (x) {
        delete (DestroyObject*)x;
    }
    base = p;
    state = p + 0xAC;
    for (i = 0; i < 4; ++i, base += 0x18, state += 0x18) {
        if (AsCGolfRule(base)->shotHistoryCount)
            while (AsCGolfRule(base)->shotHistoryCount) {
                unsigned long n = --AsCGolfRule(base)->shotHistoryCount;
                blocks = *(int*)(state + 0x10);
                offset = *(int*)(base + 0xAC) + n;
                if (blocks == 0)
                    remain = -offset;
                else
                    remain = ((blocks - 1) << 4) - offset;
                while ((unsigned)remain >= 0x20) {
                    cap = --*(int*)(state + 0x10);
                    pos = cap + *(int*)(state + 0x0C) - 1;
                    stride = *(int*)(state + 0x14);
                    slot = (unsigned int)pos % (unsigned int)stride;
                    array = *(void***)(state + 8);
                    ::appFreeUni__FPv(array[slot]);
                    array[slot] = 0;
                    remain -= 16;
                }
            }
    }
    x = AsCGolfRule(p)->padSwingController;
    if (x) {
        delete (DestroyObject58*)x;
        AsCGolfRule(p)->padSwingController = 0;
    }
    SoundManager_ReleaseSoundBank(g_soundManager, golfRuleStrings + kText_golfSoundBank);
    SoundManager_AcquireSoundBank(g_soundManager, golfRuleStrings + kText_menuSoundBank);
    void** items = (*(void***)&AsCGolfRule(p)->childObjects);
    void** end;
    int count;
    while (count = AsCGolfRule(p)->childObjectCount, end = (*(void***)&AsCGolfRule(p)->childObjects) + count,
           !(end == items)) {
        if (*items) {
            delete (DestroyObject*)*items;
            *items = 0;
        }
        ++items;
    }
    AsCGolfRule(p)->childObjectCount -= count;
    SoundManager_StopAndClearChannels(g_soundManager);
    SoundManager_RequestBgmStop(g_soundManager, 1);
}

#pragma opt_propagation reset
#pragma use_lmw_stmw reset
// Maintain camera clipping/frustum state; when forced, round an odd display counter up.
// Requires rule/documents/view/terrain. Several transition states return immediately;
// display flags gate the other updates. Computes camera/ground/ball distance,
// then refreshes clipping/frustum state; no success/failure result.
void CGolfRule::OnDisplay() {
    using namespace ns1;
    unsigned char* p = (unsigned char*)this;
    void* gm = (*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc);
    int mode = AsCGolfDoc(gm)->gameStateFlags;
    switch (mode) {
    case 0x01000000:
    case 0x00800000:
    case 0x00400000:
    case 0x04000000:
    case 0x02000000:
        return;
    default: {
        if (AsCGolfRule(p)->forceDisplayUpdate) {
            int v = AsCGolfRule(p)->displayUpdateCounter;
            if (v % 2) {
                ++v;
                AsCGolfRule(p)->displayUpdateCounter = v;
            }
        }
        gm = (*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc);
        mode = AsCGolfDoc(gm)->gameStateFlags;
        if ((mode & ~0x00222220) == 0 && !AsCGolfRule(p)->forceDisplayUpdate)
            return;
        float b[3];
        unsigned int a[3] = {*(unsigned int*)(s_cameraTransform + 0x24), *(unsigned int*)(s_cameraTransform + 0x28),
                             *(unsigned int*)(s_cameraTransform + 0x2C)};
        float sphere[4];
        float v[3];
        int index;
        float z;
        void* object = CQuadTree_GetInstance();
        GetGroundPoint__9CQuadTreeF7WVectorbPib(b, object, a, 1, &index, 0);
        float groundHeight = b[1];
        float d = *(float*)(s_cameraTransform + 0x28) - groundHeight;
        if (index == -1)
            d = *(float*)(s_cameraTransform + 0x28) - *(float*)((char*)CQuadTree_GetInstance() + 0x2180);
        gm = (*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc);
        // 0x8014CDE8: Skip the distance comparison when the ball is outside the view.
        do {
            if ((*(unsigned int*)&AsCGolfDoc(gm)->gameStateFlags) != 0x80) {
                void* x = GolfBall__Fv();
                z = AsCGolfBall(x)->m_radius;
                x = GolfBall__Fv();
                __as__7WVectorFRC7WVector(sphere, x);
                sphere[3] = z;
                if (!WView_DefaultCapabilityTrueA(g_view, sphere))
                    break;
            }
            {
                void* x = GolfBall__Fv();
                __mi__FRC7WVectorRC7WVector(v, s_cameraTransform + 0x24, x);
                float distance = WVectorLen__FRC7WVector(v);
                if (d < distance)
                    d = distance;
            }
        } while (0);

        gm = (*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc);
        CourseData_FindSurfaceByName(AsCGolfDoc(gm)->courseData, 0);
        Camera_SetClip(g_view, kFloat_3_0, kFloat_100000_GolfRule, 1);
        Camera_BuildFrustumPlanes(g_view);
    }
    }
}

namespace ns2 {
extern "C" {
// Lazily initialize and return the fade overlay.
static inline FadeStateView* getRes(char* gb) {
    if (((FadeStateView*&)Sing_805DC608()) == 0) {
        if (s_fadeOverlayInitialized == 0) {
            FadeOverlay_Init(s_fadeOverlay);
            __register_global_object(s_fadeOverlay, (void*)ResourceState_Dtor, (gb + 0x30));
            s_fadeOverlayInitialized = 1;
        }
        ((FadeStateView*&)Sing_805DC608()) = (FadeStateView*)s_fadeOverlay;
    }
    return ((FadeStateView*&)Sing_805DC608());
}
// Lazily initialize and return the logical input manager.
static inline void* getInput(char* gb) {
    if (GetInputManagerSlot() == 0) {
        if (s_inputManagerInitialized == 0) {
            InputManager_Init(s_gameInputManager);
            __register_global_object(s_gameInputManager, (void*)InputManager_Dtor, &((F3C*)gb)->a);
            s_inputManagerInitialized = 1;
        }
        GetInputManagerSlot() = s_gameInputManager;
    }
    return GetInputManagerSlot();
}
struct Tail {
    int i;
    Vec3 v;
};
// ===== S3 - State dispatcher and round continuation =====
// GolfRuleProcessView and actor messages; dispatch and menu flow.
}
}

// stack result synchronously through an int encoded in the 32-bit payload word.
// Requires constructed rule/services; dt is seconds. Reload, story, Tour exit and
// round results return through dedicated processors before common work.
// Other states perform audio/dialog/input work, state dispatch, then CPU/timer/cursor
// updates. Actor replies use stack storage synchronously. Keeps RNG and call order.
void CGolfRule::OnProcess(float dt) {
    using namespace ns2;
    char* staticStorageBase = s_golfRuleInputDestructorRecord;
    ns2::Mtx overviewYawMatrix;
    ns2::Mtx overviewPitchMatrix;
    ns2::Mtx overviewCameraMatrix;
    Tail balloonEffectReply;
    Vec3i teePositionWords;
    Vec3i pinPositionWords;
    int screenHudPhase;
    int balloonAwardReply;
    union {
        float f;
        unsigned int i;
    } u;
    int caddieReadyReply;
    int playerReadyReply;

    // Phase 1: process state-dependent timing before shared sound/input work.
    switch ((g_sharedDoc->golfDoc->gameStateFlags)) {
    case kStateCourseReload:
        ProcessCourseReload();
        return;
    case kStateStorySequence:
        ProcessStorySequence(dt);
        return;
    case kStateTourExit:
        GolfTransition_ClearInputAudioAndEffects(g_project);
        SaveProcess_SetTransitionTargetName(golfRuleStrings + kText_cTourTask);
        SaveProcess_SetFollowUpTaskName(golfRuleStrings + kText_tourMain);
        ((CTask*)g_taskManager)
            ->RequestTransitionWithFade(golfRuleStrings + kText_cSaveProcessTask,
                                        golfRuleStrings + kText_emptyTaskArgument, 0);
        return;
    case kStateRoundResults:
        ProcessRoundResultPresentation(dt);
        return;
    default:
        break;
    }
    if (0 != g_soundManager) {
        if (FloatTween_Update((staticStorageBase + 0x3c), dt)) {
            SoundManager_SetNamedVolumeScale(g_soundManager, golfRuleStrings + kText_ambientSound,
                                             getF3C((F3C*)((staticStorageBase + 0x3c))));
        }
    }
    if (Pointer_IsActionActive(g_inputPointer, golfRuleStrings + kText_leftControlKey, 1))
        Pointer_GetRisingActionBits(g_inputPointer, golfRuleStrings + kText_replayKey, 1);
    Obj438* pendingDialog = ((GolfRuleProcessView*)this)->pendingDialog;
    if (pendingDialog != 0 && pendingDialog->vf(kFloat_0_GolfRule)) {
        if (((GolfRuleProcessView*)this)->pendingDialog->field8 == 1)
            IActor_DispatchMessage(((GolfRuleProcessView*)this), ((GolfRuleProcessView*)this),
                                   golfRuleStrings + kText_item, 9, ((GolfRuleProcessView*)this)->forgetItemSlot, 0, 0,
                                   0);
        Obj438* completedDialog = ((GolfRuleProcessView*)this)->pendingDialog;
        float dialogTimerZero = kFloat_0_GolfRule;
        ((GolfRuleProcessView*)this)->forgetDlg = 0;
        ((GolfRuleProcessView*)this)->forgetDlgTime = dialogTimerZero;
        delete completedDialog;
        ((GolfRuleProcessView*)this)->pendingDialog = 0;
    }
    Pointer_GetRisingActionBits(g_inputPointer, golfRuleStrings + kText_playerInfoKey, 1);
    if (0 != ((GolfRuleProcessView*)this)->shotBlock) {
        ((GolfRuleProcessView*)this)->shotBlockTime += dt;
        if (((GolfRuleProcessView*)this)->shotBlockTime > 5.0f)
            ((GolfRuleProcessView*)this)->shotBlock = 0;
    }
    if (((Pointer_IsActionActive(g_inputPointer, golfRuleStrings + kText_leftControlKey, 1) ||
          Pointer_IsActionActive(g_inputPointer, golfRuleStrings + kText_rightControlKey, 1)) &&
         Pointer_GetRisingActionBits(g_inputPointer, golfRuleStrings + kText_hideKey, 1)) ||
        (!((GolfRuleProcessView*)this)->hudVisible &&
         (Pointer_GetRisingActionBits(g_inputPointer, golfRuleStrings + kText_enterKey, 1) ||
          Pointer_GetRisingActionBits(g_inputPointer, golfRuleStrings + kText_spaceBarKey, 1)))) {
        int visibilityToggleValue = (((GolfRuleProcessView*)this)->hudVisible == 0);
        ((GolfRuleProcessView*)this)->hudVisible = visibilityToggleValue;
        IActor_DispatchMessage(((GolfRuleProcessView*)this), ((GolfRuleProcessView*)this),
                               golfRuleStrings + kText_screen, 5, visibilityToggleValue, 0, 0, 0);
        IActor_DispatchMessage(((GolfRuleProcessView*)this), ((GolfRuleProcessView*)this),
                               golfRuleStrings + kText_letter, 4, ((GolfRuleProcessView*)this)->hudVisible, 0, 0, 0);
        IActor_DispatchMessage(((GolfRuleProcessView*)this), ((GolfRuleProcessView*)this), golfRuleStrings + kText_spot,
                               9, ((GolfRuleProcessView*)this)->hudVisible, 0, 0, 0);
        g_mouseCursor->enabled = ((GolfRuleProcessView*)this)->hudVisible;
    }
    if (0 != ((GolfRuleProcessView*)this)->unk_498)
        ((GolfRuleProcessView*)this)->unk_498 = 0;
    if ((unsigned)g_sharedDoc->golfDoc->gameStateFlags != (unsigned)kStateButtonSwing &&
        ((GolfRuleProcessView*)this)->forceDisplayUpdate == 0 && Object_GetChild(g_taskManager)->b3a5 == 0) {
        ((CQuadTree*)CQuadTree_GetInstance())->UpdateAnimatedObjects(dt, 0);
    }

    IActor_DispatchMessage(((GolfRuleProcessView*)this), ((GolfRuleProcessView*)this), golfRuleStrings + kText_screen,
                           kScreenQueryHudPhaseMessage, (int)&screenHudPhase, 0, 0, 0);
    InstanceTable_Get()->hudPhase = screenHudPhase;
    GameCFE4* golfDocument = g_sharedDoc->golfDoc;
    if ((golfDocument->gameStateFlags & kClubSelectionUpdateStates) && !((GolfRuleProcessView*)this)->shotBlock &&
        SharedDoc_IsPlayerFlag0Set(golfDocument->currentPlayer) == 0) {
        ((CClub*)InstanceTable_Get())->Process(dt);
    }
    if ((g_sharedDoc->golfDoc->gameStateFlags & kStateAim) && InstanceTable_Get()->powerShotMode == 0) {
        if (Pointer_IsActionActive(g_inputPointer, golfRuleStrings + kText_leftAltKey, 1) ||
            Pointer_IsActionActive(g_inputPointer, golfRuleStrings + kText_rightAltKey, 1))
            return;
    }
    if (Pointer_GetRisingActionBits(g_inputPointer, golfRuleStrings + kText_skipKey, 1) &&
        0 != ((GolfRuleProcessView*)this)->stateEntryPending &&
        (unsigned)g_sharedDoc->golfDoc->gameStateFlags != (unsigned)kStateUnresolved400 && GolfTask_NoOpStatus() == 0)
        FinalizeShotAndAdvanceState();
    if (Pointer_GetRisingActionBits(g_inputPointer, golfRuleStrings + kText_fastForwardKey, 1) &&
        0 != ((GolfRuleProcessView*)this)->unk_094 &&
        (g_sharedDoc->golfDoc->gameStateFlags & kDebugFastForwardStates) && GolfTask_NoOpStatus() &&
        (g_project->b28 != 0 || g_sharedDoc->unk_8A4 != 0))
        ((GolfRuleProcessView*)this)->unk_094 = 0;
    if (GolfTask_NoOpStatus()) {
        Cam784* projectCamera = g_project;
        float d = projectCamera->f64 - s_golfRuleCameraScaleReference;

        u.f = d;
        if (u.i & 0x80000000)
            d = -d;
        if (((d < kGolfRuleComparisonEpsilon) == 0) && (g_sharedDoc->golfDoc->gameStateFlags & ~kKeepCameraScaleStates))
            projectCamera->f64 = kFloat_1_805DEA38;
    }
    if (0 == ((GolfRuleProcessView*)this)->forgetDlg) {
        if (Input_IsActionPressed(getInput(staticStorageBase), kActionRequestPresentationBit4, kSelectedInputChannel) &&
            (g_sharedDoc->golfDoc->gameStateFlags & kStateBallFlight) && GolfBall__Fv()->m_state == 0 &&
            SharedDoc_IsPlayerFlag0Set(g_sharedDoc->golfDoc->currentPlayer) == 0 && !g_sharedDoc->partyGameMode &&
            g_sharedDoc->golfDoc->tutorialStage == 6)
            ((GolfRuleProcessView*)this)->shotPresentationFlags |= kPresentationBit4;
    }
    ((GolfRuleProcessView*)this)->elapsedTime = (((GolfRuleProcessView*)this)->elapsedTime + dt);
    // Phase 2: apply party-item state, then dispatch the active golf-state handler.
    PartyItems_ApplySelection(PartyItemSlots_GetInstance(), g_sharedDoc->golfDoc->gameStateFlags);
    switch (g_sharedDoc->golfDoc->gameStateFlags) {
    case kStatePlayerPreview:
        ((CGolfRule*)(((GolfRuleProcessView*)this)))->ProcessPlayerPreview(dt);
        break;
    case kStateTurnStartDelay:
        if (((GolfRuleProcessView*)this)->elapsedTime < kFloat_3_0) {
            ((CCameraWork*)g_cameraWork)->UpdateMotionAndTarget(dt, kFloat_0_GolfRule);
        } else {
            ((CMouseCursor*)g_mouseCursor)->Reset(0);
            IActor_DispatchMessage(((GolfRuleProcessView*)this), ((GolfRuleProcessView*)this),
                                   golfRuleStrings + kText_clock, 0, 0, 0, 0, 0);
            Sound_PlayByName(g_soundManager, golfRuleStrings + kText_play_my_turn, 0, 0, 0, 0, kFloat_0_5_GolfRule,
                             kFloat_200_GolfRule, kFloat_1_805DEA38);
            ((CGolfRule*)(((GolfRuleProcessView*)this)))->EnterState(kStateAim);
        }
        break;
    case kStateAim:
        ((CGolfRule*)(((GolfRuleProcessView*)this)))->ProcessAimInput(dt);
        break;
    case kStateBeginButtonSwing:
        ((CGolfRule*)(((GolfRuleProcessView*)this)))->EnterState(kStateButtonSwing);
        break;
    case kStateButtonSwing:
        ((CGolfRule*)(((GolfRuleProcessView*)this)))->ProcessButtonSwingInput(dt);
        break;
    case kStateCameraView:
        if (PollAdvanceOrDismiss()) {
            ((CMouseCursor*)g_mouseCursor)->SavePosition();
            g_sharedDoc->golfDoc->cameraMode = 0;
            ((CGolfRule*)(((GolfRuleProcessView*)this)))->EnterState(kStateAim);
            IActor_DispatchMessage(((GolfRuleProcessView*)this), ((GolfRuleProcessView*)this),
                                   golfRuleStrings + kText_camera, 3, 0, 0, 0, 0);
        }
        if (PollCameraToggle()) {
            g_sharedDoc->golfDoc->cameraMode = 2;
            ((CGolfRule*)(((GolfRuleProcessView*)this)))->EnterState(kStateCameraTransition);
        }
        break;
    case kStateCameraTransition:
        UpdateCameraModeTransition(dt);
        break;
    case kStatePrepareNextShot:
        PrepareNextShot(dt);
        break;
    case kStateButtonShotAnimation:
        ((CGolfRule*)(((GolfRuleProcessView*)this)))->ProcessButtonShotAnimation(dt);
        break;
    case kStateBallFlight:
        // Skip admission (30 Hz samples): ordinary round, both tutorial gates off,
        // non-putter, moving ball and strictly more than 90 samples remaining.
        // The request latches fastPlayback; the fade starts only at cursor>=15
        // and when the overlay's captureRequested byte clears. RequestPendingTransition
        // requests a framebuffer capture (fade_overlay_and_shared_file.cpp); retain that historical symbol.
        // Fade-out duration is 60 overlay update calls, not 60 simulation samples.
        // Keep repeated observations; sampling, capture and fade have separate clocks.
        if (g_sharedDoc->partyGameMode == 0) {
            if (!((GolfRuleProcessView*)this)->tutorialFeedbackEnabled) {
                if (!((GolfRuleProcessView*)this)->tutorialAdvancePending) {
                    if (0 == ((GolfRuleProcessView*)this)->fastPlayback) {
                        if (InstanceTable_Get()->selectedClub->clubKind != kClubKindPutter) {
                            if (GolfBall__Fv()->m_eventPreDataIndex[kBallPlaybackCursor] +
                                    kFastPlaybackMinimumRemainingSamples <
                                (GolfBall__Fv()->m_preDataCount)) {
                                if (GolfBall__Fv()->m_state != 0) {
                                    if (Input_IsActionPressed(getInput(staticStorageBase), kActionSkipFlight,
                                                              kSelectedInputChannel)) {
                                        ((GolfRuleProcessView*)this)->fastPlayback = 1;
                                        if (GolfBall__Fv()->m_eventPreDataIndex[kBallPlaybackCursor] >=
                                            kFastPlaybackFadeStartSample)
                                            FadeState_RequestFramebufferCapture(getRes(staticStorageBase));
                                    }
                                }
                            }
                        }
                    } else {
                        if (GolfBall__Fv()->m_eventPreDataIndex[kBallPlaybackCursor] == kFastPlaybackFadeStartSample)
                            FadeState_RequestFramebufferCapture(getRes(staticStorageBase));
                    }
                    if (((GolfRuleProcessView*)this)->fastPlayback != 0 &&
                        GolfBall__Fv()->m_eventPreDataIndex[kBallPlaybackCursor] >= kFastPlaybackFadeStartSample &&
                        getRes(staticStorageBase)->captureRequested == 0) {
                        FadeState_StartFadeIn(getRes(staticStorageBase), 0);
                        FadeState_StartFadeOut(getRes(staticStorageBase), kFastPlaybackFadeOutTicks);
                        FadeState_RequestReleaseOnFadeCompletion(getRes(staticStorageBase));
                        // Ordinary club kinds 0/1/2, committed power fraction >.8,
                        // no special bits in ball +44 (m_powerFlags), Pangya impact.
                        // Special presentation only admits its step-2 notice here.
                        if (InstanceTable_Get()->selectedClub->clubKind == kClubKindWood ||
                            InstanceTable_Get()->selectedClub->clubKind == kClubKindIron ||
                            InstanceTable_Get()->selectedClub->clubKind == kClubKindWedge) {
                            if ((g_sharedDoc->golfDoc->shotPowerGaugeX - kFloat_149_805DEA58) / kFloat_360_GolfRule >
                                kFloat_0_8_GolfRule) {
                                if ((GolfBall__Fv()->m_powerFlags & kSpecialShotMask) == 0) {
                                    if (g_sharedDoc->golfDoc->impactGrade == kImpactPangya) {
                                        if ((((GolfRuleProcessView*)this)->shotPresentationFlags &
                                             kSpecialShotPresentation) == 0 ||
                                            ((GolfRuleProcessView*)this)->presentationStep == 2)
                                            IActor_DispatchMessage(
                                                ((GolfRuleProcessView*)this), ((GolfRuleProcessView*)this),
                                                golfRuleStrings + kText_letter, 3,
                                                (int)(golfRuleStrings + kText_pangyaNotice), 0, 0, 0);
                                    }
                                }
                            }
                        }
                        if (((GolfRuleProcessView*)this)->shotPresentationFlags & kSpecialShotPresentation) {
                            ((GolfRuleProcessView*)this)->presentationStep = 0;
                            ((GolfRuleProcessView*)this)->shotPresentationFlags &= ~kSpecialShotPresentation;
                        }
                    }
                }
            }
        }
        ((CGolfRule*)(((GolfRuleProcessView*)this)))->ProcessBallFlight(dt);
        break;
    case kStateDelayedTransition:
        ((CGolfRule*)(((GolfRuleProcessView*)this)))->ProcessDelayedStateTransition(dt);
        break;
    case kStateHoleResults:
        ((CGolfRule*)(((GolfRuleProcessView*)this)))->ProcessHoleResults(dt);
        break;
    case kStateBeginPlayerTurn:
        ((CGolfRule*)((GolfRuleProcessView*)this))->BeginPlayerTurn(dt);
        break;
    case kStateCameraAwaitInput:
        if (PollAdvanceOrDismiss()) {
            ((CMouseCursor*)g_mouseCursor)->SavePosition();
            ((CGolfRule*)(((GolfRuleProcessView*)this)))->EnterState(kStateAim);
        } else if (PollCameraToggle()) {
            ((CGolfRule*)(((GolfRuleProcessView*)this)))->EnterState(kStateCameraTransition);
        }
        break;
    case kStatePostRoundMenus:
        ((CGolfRule*)(((GolfRuleProcessView*)this)))->ProcessPostRoundMenus(dt);
        break;
    case kStateHoleCameraPath:
        UpdateBezierPathCamera(dt);
        if (Pointer_GetRisingActionBits(g_inputPointer, golfRuleStrings + kText_spaceBarKey, 1) ||
            g_sharedDoc->golfDoc->currentHoleIndex != 0)
            ((CGolfRule*)(((GolfRuleProcessView*)this)))->EnterState(kStatePlayerPreview);
        break;
    case kStateCameraReturn:
        if (PollAdvanceOrDismiss()) {
            ((CMouseCursor*)g_mouseCursor)->SavePosition();
            g_sharedDoc->golfDoc->cameraMode = 0;
            ((CGolfRule*)(((GolfRuleProcessView*)this)))->EnterState(kStateAim);
        } else if (PollCameraToggle()) {
            ((CGolfRule*)(((GolfRuleProcessView*)this)))->EnterState(kStateAim);
            DrawScene_NoOpHook(g_sceneManager, 0);
            g_sharedDoc->golfDoc->cameraMode = 1;
        }
        break;
    case kStateFightResult:
        ((CGolfRule*)((GolfRuleProcessView*)this))->ProcessFightResult(dt);
        break;
    case kStateHoleOverview:
        if (!(!(Pointer_GetRisingActionBits(g_inputPointer, golfRuleStrings + kText_number9Key, 1) ||
                Pointer_GetRisingActionBits(g_inputPointer, golfRuleStrings + kText_pad9Key, 1)))) {
            void* sceneManager = g_sceneManager;
            DrawScene_SetNamedObjectEnabled(
                sceneManager,
                String_FormatTemporary(golfRuleStrings + kText_playerActorFormat, g_sharedDoc->golfDoc->currentPlayer),
                1);
            DrawScene_SetNamedObjectEnabled(g_sceneManager, golfRuleStrings + kText_clubActor, 1);
            DrawScene_SetNamedObjectEnabled(g_sceneManager, golfRuleStrings + kText_caddieActor, 1);
            ((CShadowManager*)g_shadowManager)->SetSelectionFlags(1);
            IActor_DispatchMessage(((GolfRuleProcessView*)this), ((GolfRuleProcessView*)this),
                                   golfRuleStrings + kText_score, 0xf, 0, 0, 0, 0);
            ((CGolfRule*)(((GolfRuleProcessView*)this)))->EnterState(kStateAim);
        } else {
            float overviewTime = ((GolfRuleProcessView*)this)->elapsedTime + ((GolfRuleProcessView*)this)->stateTimer;
            Matrix_SetAxisRotation(&overviewYawMatrix, kFloat_0_17453292_GolfRule * (overviewTime), 1);
            Matrix_SetAxisRotation(&overviewPitchMatrix, kFloat_neg0_34906584_GolfRule, 0);
            Matrix_ConcatAffine(&overviewCameraMatrix, &overviewPitchMatrix, &overviewYawMatrix);
            Matrix_CopyAffine(s_cameraTransform, &overviewCameraMatrix);
            GlobalE2F8* g = g_sharedDoc;
            int holeRecordByteOffset = g->golfDoc->currentHoleIndex * 32;
            void* holeRecords = g->golfDoc->holeRecords;
            teePositionWords = *(Vec3i*)((char*)g->golfDoc->holeRecords + holeRecordByteOffset);
            pinPositionWords = *(Vec3i*)((char*)holeRecords + 32 * g->golfDoc->currentHoleIndex + 0xc);
            WVector teeToPin = *(WVector*)&pinPositionWords - *(WVector*)&teePositionWords;
            WVector cameraOffset = (kFloat_0_5_GolfRule * WVectorLen(teeToPin) + kFloat_640_GolfRule) *
                                   *(WVector*)(s_cameraTransform + 0x18);
            WVector cameraPosition = *(WVector*)((GolfRuleProcessView*)this)->overviewTarget - cameraOffset;
            *(WVector*)(s_cameraTransform + 0x24) = cameraPosition;
        }
        break;
    // Wii Remote swing results enter the shared shot simulation through this path.
    case kStateMotionSwing:
        ((CGolfRule*)(((GolfRuleProcessView*)this)))->ProcessMotionSwingInput(dt);
        break;
    case kStateWaitPlayerAndCaddie: {
        IActor_DispatchMessage(((GolfRuleProcessView*)this), ((GolfRuleProcessView*)this),
                               golfRuleStrings + kText_player, 0x27, (int)&playerReadyReply,
                               ((GolfRuleProcessView*)this)->nextPlayerIndex, 0, 0);
        IActor_DispatchMessage(((GolfRuleProcessView*)this), ((GolfRuleProcessView*)this),
                               golfRuleStrings + kText_caddie, 0x19, (int)&caddieReadyReply,
                               ((GolfRuleProcessView*)this)->nextPlayerIndex, 0, 0);
        if (playerReadyReply != 0 && 0 != caddieReadyReply)
            ((CGolfRule*)(((GolfRuleProcessView*)this)))->EnterState(kStatePrepareNextShot);
        break;
    }
    case kStateMotionShotAnimation:
        ((CGolfRule*)(((GolfRuleProcessView*)this)))->ProcessMotionShotAnimation(dt);
        break;
    case kStateWaitHoleResources:
        WaitForHoleActors(dt);
        break;
    case kStateUnresolved400:
        break;
    case kStateMotionCameraInput:
        ((CGolfRule*)(((GolfRuleProcessView*)this)))->ProcessMotionCameraInput(dt);
        break;
    default:
        break;
    }
    // Phase 3: finish shared player/timer/cursor updates after the state handler.
    if (SharedDoc_IsPlayerFlag0Set(g_sharedDoc->golfDoc->currentPlayer)) {
        ComUnplayable_UpdateCurrentDropSequence(g_rival, ((GolfRuleProcessView*)this));
        ((CRival*)g_rival)->UpdateShotProcessor(((GolfRuleProcessView*)this));
    }
    float zero = kFloat_0_GolfRule;
    if (((GolfRuleProcessView*)this)->coinTime != zero) {
        ((GolfRuleProcessView*)this)->coinTime = ((GolfRuleProcessView*)this)->coinTime - dt;
        if (((GolfRuleProcessView*)this)->coinTime < zero) {
            ((GolfRuleProcessView*)this)->coinTime = zero;
            ((GolfRuleProcessView*)this)->coinCount -= 1;
            if (((GolfRuleProcessView*)this)->coinCount > 0)
                ((GolfRuleProcessView*)this)->coinTime = kFloat_0_1_GolfRule;
            IActor_DispatchMessage(((GolfRuleProcessView*)this), ((GolfRuleProcessView*)this),
                                   golfRuleStrings + kText_effect, 0x11, ((GolfRuleProcessView*)this)->coinNum, 0, 0,
                                   0);
        }
    }
    ((CMouseCursor*)g_mouseCursor)->SetState(0);
    if (g_sharedDoc->partyGameMode == 3) {
        balloonAwardReply = 0;
        IActor_DispatchMessage(((GolfRuleProcessView*)this), ((GolfRuleProcessView*)this),
                               golfRuleStrings + kText_balloonSystem, 1, (int)&balloonAwardReply, 0, 0, 0);
        if (0 < balloonAwardReply) {
            unsigned char savedPlayerId = g_sharedDoc->golfDoc->currentPlayer;
            char* p = (char*)GolfPlayer_GetEntryByIndex(g_sharedDoc->golfDoc, savedPlayerId);
            int balloonAward = balloonAwardReply;
            *(int*)(p + 0x174) += balloonAward;

            IActor_DispatchMessage(((GolfRuleProcessView*)this), ((GolfRuleProcessView*)this),
                                   golfRuleStrings + kText_balloonSystem, 0, (int)&balloonEffectReply.i, 0, 0, 0);
            int balloonEffectKind = balloonEffectReply.i;
            S5958* q = PartyItemSlots_GetInstance();
            int v = balloonEffectKind;
            if (0 != q->i68 && v != -1)
                PartyGoodsEffectList_AddAppearance(q->i68, v, q->i0c, &balloonEffectReply.v);
        }
    }
}

// Wait for the result camera, then advance the fight result or leave the mode.
// Requires rule/player state; dt updates camera before the pause/sync gate.
// Mode-specific timeout/stroke predicates can defer completion. Admitted completion
// in Darts updates player state/completed-hole count; selects hole results or actor wait.
// No failure result; tutorial exit and task status can return before transition.
void CGolfRule::ProcessFightResult(float dt) {
    using namespace ns2;
    ObjE2F8* self = (ObjE2F8*)this;
    (void)dt;
    if (self->elapsedTime < self->stateTimer) {
        ((CCameraWork*)g_cameraWork)->UpdateMotionAndTarget(dt, kFloat_0_GolfRule);
        return;
    }
    GlobalE2F8* g = g_sharedDoc;
    GameCFE4* player = g->golfDoc;
    int id = AsCGolfDoc(player)->currentPlayer;
    if (g->golfRuleType == 1)
        id = ((CGolfDoc*)player)->ResolveMatchTurnFromHoleStrokes();
    IActor_DispatchMessage(self, self, golfRuleStrings + kText_player, 0x32, 1, id, 0, 0);
    if (self->processingPaused || self->networkSyncPending)
        return;
    GlobalE2F8* mgr = g_sharedDoc;
    player = mgr->golfDoc;

    unsigned char* state = (unsigned char*)GolfPlayer_GetEntryByIndex(player, (player->currentPlayer));
    int mode = ((GolfDocSnapshot8013*)(state))->playerSlot;
    if (!mode) {
        GameCFE4* savedPB = g_sharedDoc->golfDoc;
        player = savedPB;
        if (AsCGolfDoc(player)->tutorialStage == 5) {
            IActor_DispatchMessage(self, self, golfRuleStrings, 0xd, 1, 0, 0, 0);
            IActor_DispatchMessage(self, self, golfRuleStrings + kText_tutorial, 0, 0, 0, 0, 0);
            return;
        }
        if (AsCGolfDoc(player)->tutorialStage == 6) {
            if ((AsCSharedDoc(g_sharedDoc)->gameMode - 2u) <= 2) {
                state = (unsigned char*)GolfPlayer_GetEntryByIndex(player, player->currentPlayer);
                if (*(int*)(state + 8) == 3)
                    return;
                if (g_sharedDoc->golfRuleType == 1) {
                    state = (unsigned char*)GolfPlayer_GetEntryByIndex(player, player->currentPlayer);
                    if (((GolfDocSnapshot8013*)(state))->timeoutsUsed >= 3)
                        return;
                    int threshold = 3;
                    int calc = (g_sharedDoc->roundHoleCount + 1) / 2;
                    if (3 < calc)
                        threshold = calc;
                    state = (unsigned char*)GolfPlayer_GetEntryByIndex(player, player->currentPlayer);
                    if (((GolfDocSnapshot8013*)(state))->strokeLimitHitCount >= threshold)
                        return;
                } else {
                    state = (unsigned char*)GolfPlayer_GetEntryByIndex(player, player->currentPlayer);
                    if (((GolfDocSnapshot8013*)(state))->timeoutsUsed >= 3)
                        return;
                    int threshold = 3;
                    int calc = (g_sharedDoc->roundHoleCount + 1) / 2;
                    if (calc > 3)
                        threshold = calc;
                    state = (unsigned char*)GolfPlayer_GetEntryByIndex(player, player->currentPlayer);
                    if (((GolfDocSnapshot8013*)(state))->strokeLimitHitCount >= threshold)
                        return;
                }
            }
        }
    }
    mgr = g_sharedDoc;
    if (mgr->partyGameMode == 1) {
        player = mgr->golfDoc;
        state = (unsigned char*)GolfPlayer_GetEntryByIndex(player, player->currentPlayer);
        *(int*)(8 + state) = 3;
        player = g_sharedDoc->golfDoc;
        state = (unsigned char*)GolfPlayer_GetEntryByIndex(player, player->currentPlayer);
        ++*(int*)&((GolfDocSnapshot8013*)(state))->roundResultWords[3];
    }
    if (((CGolfRule*)self)->IsCurrentHoleComplete()) {
        player = g_sharedDoc->golfDoc;
        if (AsCGolfDoc(player)->tutorialStage == 5) {
            CGolfRule::RequestModeExitTransition(0);
            return;
        }
        if (GolfTask_NoOpStatus())
            return;
        ((CGolfRule*)self)->EnterState(kStateHoleResults);
        return;
    }
    IActor_DispatchMessage(self, self, golfRuleStrings + kText_score, 1, 0, 0, 0, 0);
    ((CGolfRule*)self)->EnterState(kStateWaitPlayerAndCaddie);
}

// Prepare the next player turn after the opening delay and pending messages.
// Requires rule/player and ball services; pause/sync admission can return early.
// Updates current-hole stroke state, rebuilds turn UI/gauge/camera and enters Aim.
// Keeps the separate new-turn/continuation branches and original message order;
// no success/failure result.
void CGolfRule::BeginPlayerTurn(float dt) {
    using namespace ns3;
    GolfRuleTurnStartView* self = (GolfRuleTurnStartView*)this;
    if (!(self->value34 > kFloat_3_0) || self->flag3B1 || self->flag3B2)
        return;

    self->flag91 = 0;
    if (((SharedDocTurnView*)g_sharedDoc)->partyGameMode != 0) {
        ((CGolfRule*)self)->AdvanceTurnOrRound(0);
        return;
    }

    Object8014* object = (Object8014*)((SharedDocTurnView*)g_sharedDoc)->golfDoc;
    if (self->nextPlayerIndex == AsCGolfDoc(object)->currentPlayer) {
        void* base = GolfPlayer_GetEntryByIndex(object, AsCGolfDoc(object)->currentPlayer);
        SharedDocTurnView* manager2 = (SharedDocTurnView*)g_sharedDoc;
        object = (Object8014*)manager2->golfDoc;
        Character8014* character = (Character8014*)((char*)base + AsCGolfDoc(object)->currentHoleIndex);
        character->count++;
        ((void (*)(void*, void*, const char*, int, int, int, int, int))IActor_DispatchMessage)(
            self, self, golfRuleStrings + kText_spot, 3, 0, 0, 0, 0);
        ((void (*)(void*, void*, const char*, int, int, int, int, int))IActor_DispatchMessage)(
            self, self, golfRuleStrings + kText_grid, 2, 0, 0, 0, 0);
        ((void (*)(void*, void*, const char*, int, int, int, int, int))IActor_DispatchMessage)(
            self, self, golfRuleStrings + kText_clock, 0, 0, 0, 0, 0);
        ((void (*)(void*, void*, const char*, int, int, int, int, int))IActor_DispatchMessage)(
            self, self, golfRuleStrings + kText_player, 1, 0, 0, 0, 0);
        ((void (*)(void*, void*, const char*, int, int, int, int, int))IActor_DispatchMessage)(
            self, self, golfRuleStrings + kText_caddie, 0x2715, (int)(golfRuleStrings + kText_basicSequence), 0, 0, 0);
        ((void (*)(void*, void*, const char*, int, int, int, int, int))IActor_DispatchMessage)(
            self, self, golfRuleStrings + kText_caddie, 10, 0, 0, 0, 0);
        ((CGolfRule*)self)->EnterState(kStateAim);
    } else {
        AsCGolfDoc(object)->currentPlayer = self->nextPlayerIndex;
        object = (Object8014*)((SharedDocTurnView*)g_sharedDoc)->golfDoc;
        void* base2 = GolfPlayer_GetEntryByIndex(object, AsCGolfDoc(object)->currentPlayer);
        object = (Object8014*)((SharedDocTurnView*)g_sharedDoc)->golfDoc;
        ((Character8014*)((char*)base2 + AsCGolfDoc(object)->currentHoleIndex))->count++;
        ((CGolfRule*)self)->InitializeAimHeading();
        ((CGolfRule*)self)->PreparePlayerTurn();
        ((CMouseCursor*)g_mouseCursor)->Reset(0);
        ((void (*)(void*, void*, const char*, int, int, int, int, int))IActor_DispatchMessage)(
            self, self, golfRuleStrings + kText_clock, 0, 0, 0, 0, 0);
        ((CGolfRule*)self)->ResetShotGauge();
        ((CGolfDoc*)(((SharedDocTurnView*)g_sharedDoc)->golfDoc))->UpdateCamera(1);
        void* found = NamedEffectList_FindByName(g_sceneManager, golfRuleStrings + kText_ballSequence);
        if (found)
            WPuppet_SetOpacityRecursive(found, 0xFF);
        Sound_StopByAlias(g_soundManager, golfRuleStrings + kText_shot_dist_count);
        Sound_PlayByName(g_soundManager, golfRuleStrings + kText_play_my_turn, 0, 0, 0, 0, kFloat_0_5_GolfRule,
                         kFloat_200_GolfRule, kFloat_1_805DEA38);
        ((CCameraWork*)g_cameraWork)->StopBallSound();
        ((CGolfRule*)self)->EnterState(kStateAim);
    }
}

namespace ns3 {
extern "C" {
struct Save8014 {
    char pad[0x1c4];
    unsigned int flags;
    char pad2[0x30c];
    unsigned char flags2;
    char rest[0x353];
};
}
}

// Poll result/retry menus and apply the selected post-round action.
// Requires rule-owned menus and documents; dt is seconds. Waits for camera duration
// and the eight-second admission gate before polling menu handles and fade state.
// May create/destroy dialogs, update save records or request task/reload/exit changes.
// Pending menu/fade replies defer progress; no failure return is provided.
void CGolfRule::ProcessPostRoundMenus(float dt) {
    using namespace ns3;
    float value = AsGolfRuleFrame(this)->elapsedTime;
    const char* globalBase = s_golfRuleInputDestructorRecord;
    if (value < (AsGolfRuleFrame(this)->stateTimer)) {
        ((CCameraWork*)g_cameraWork)->UpdateMotionAndTarget(dt, kFloat_0_GolfRule);
        return;
    }
    if (value < kFloat_8_GolfRule || AsCGolfRule(this)->processingPaused || AsCGolfRule(this)->networkSyncPending)
        return;
    if (AsCSharedDoc(g_sharedDoc)->gameMode == 5)
        AsCGolfRule(this)->processingPaused = 1;
    // After camera/pause/network gates, postRoundMenuState drives dialog creation,
    // polling and task/fade transitions; dialog pointers remain rule-owned.
    switch (((GolfRulePostRoundView*)this)->postRoundMenuState) {
    case 0:
    case 1: {
        Layout_SetHitTestingEnabled(1);
        if (0 == ((GolfRulePostRoundView*)this)->resultMenu) {
            switch ((*(int*)&AsCSharedDoc(g_sharedDoc)->gameMode)) {
            case 0:
            case 6:
                ((GolfRulePostRoundView*)this)->resultMenu = ((CGolfRule*)this)->CreateResultMenu(1);
                break;
            case 1:
            case 7:
                ((GolfRulePostRoundView*)this)->resultMenu = ((CGolfRule*)this)->CreateFightResultOrRetryMenu();
                break;
            }
        }
        if (((GolfRulePostRoundView*)this)->resultMenu == 0)
            return;
        if (!MPBase1_Process(((GolfRulePostRoundView*)this)->resultMenu, dt))
            return;
        if (AsCSharedDoc(g_sharedDoc)->gameMode == 0) {
            if (((GolfRulePostRoundView*)this)->resultMenu) {
                delete (Virtual8014*)((GolfRulePostRoundView*)this)->resultMenu;
                ((GolfRulePostRoundView*)this)->resultMenu = 0;
            }
            if (GolfRule_UpdateCourseClearStatus()) {
                ((GolfRulePostRoundView*)this)->postRoundMenuState = 5;
                return;
            }
            ((GolfRulePostRoundView*)this)->retryMenu = new Dialog8014(0);
            ((GolfRulePostRoundView*)this)->postRoundMenuState = 2;
            return;
        }
        if (AsCSharedDoc(g_sharedDoc)->gameMode == 6) {
            ((GolfRulePostRoundView*)this)->tick4a8 = 0;
            FADE8014((globalBase + 0x18));
            FadeQuad_SetRgb(Sing_805DC660(), 0);
            FADE8014((globalBase + 0x18));
            ((CEffectFade*)Sing_805DC660())->SetAlpha(0);
            ((GolfRulePostRoundView*)this)->postRoundMenuState = 4;
            return;
        }
        if (((GolfRulePostRoundView*)this)->resultMenu) {
            delete (Virtual8014*)((GolfRulePostRoundView*)this)->resultMenu;
            ((GolfRulePostRoundView*)this)->resultMenu = 0;
        }
        ((GolfRulePostRoundView*)this)->retryMenu = new Dialog8014(1);
        ((GolfRulePostRoundView*)this)->postRoundMenuState = 2;
        return;
    }
    case 2: {
        if (0 == ((GolfRulePostRoundView*)this)->retryMenu) {
            unsigned int mode = AsCSharedDoc(g_sharedDoc)->gameMode;
            if (1 == mode || mode == 7)
                ((GolfRulePostRoundView*)this)->retryMenu = new Dialog8014(1);
            else
                ((GolfRulePostRoundView*)this)->retryMenu = new Dialog8014(0);
        }
        INPUT8014();
        void* input = GetInputManagerSlot();
        int selected = 0;
        int all = 1;
        int minimum = 3;
        int i;
        for (i = 0; i < *((unsigned char*)((ManagerBig8014*)g_sharedDoc)->objb90 + 0x2b9); i++) {
            if (minimum > *(int*)GolfPlayer_GetEntryByIndex(((ManagerBig8014*)g_sharedDoc)->objb90, (unsigned char)i)) {
                if (!SharedDoc_IsPlayerFlag0Set(i)) {
                    minimum =
                        *(int*)GolfPlayer_GetEntryByIndex(((ManagerBig8014*)g_sharedDoc)->objb90, (unsigned char)i);
                    all = 0;
                }
            }
        }
        if (!(!(!(!(all))))) {
            for (i = 0; i < *((unsigned char*)((ManagerBig8014*)g_sharedDoc)->objb90 + 0x2b9); i++) {
                if (minimum >
                    *(int*)GolfPlayer_GetEntryByIndex(((ManagerBig8014*)g_sharedDoc)->objb90, (unsigned char)i))
                    minimum =
                        *(int*)GolfPlayer_GetEntryByIndex(((ManagerBig8014*)g_sharedDoc)->objb90, (unsigned char)i);
            }
            InputManager_SetCursorDrawOptions(input, 1, 1, 0, minimum);
            InputManager_SetCursorColorOverride(input, 1, 0xd6, 0x48, 0x5c, minimum);
            InputManager_SetSelectedPlayer(input, minimum);
        } else {
            for (i = 0; i < *((unsigned char*)((ManagerBig8014*)g_sharedDoc)->objb90 + 0x2b9); i++) {
                if (minimum ==
                    *(int*)GolfPlayer_GetEntryByIndex(((ManagerBig8014*)g_sharedDoc)->objb90, (unsigned char)i)) {
                    if (!SharedDoc_IsPlayerFlag0Set(i)) {
                        selected = i;
                        break;
                    }
                }
            }
            InputManager_SetCursorDrawOptions(
                input, 1, 1, selected,
                *(int*)GolfPlayer_GetEntryByIndex(((ManagerBig8014*)g_sharedDoc)->objb90, (unsigned char)selected));
            InputManager_SetCursorColorOverride(
                input, 1, 0xd6, 0x48, 0x5c,
                *(int*)GolfPlayer_GetEntryByIndex(((ManagerBig8014*)g_sharedDoc)->objb90, (unsigned char)selected));
            InputManager_SetSelectedPlayer(input, *(int*)GolfPlayer_GetEntryByIndex(
                                                      ((ManagerBig8014*)g_sharedDoc)->objb90, (unsigned char)selected));
        }
        if ((*(int*)&AsCSharedDoc(g_sharedDoc)->unk_80C) == 3) {
            ItemRequestState_ClearGlobalEntries();
            ((CTask*)g_taskManager)
                ->RequestTransitionWithFade(golfRuleStrings + kText_cTourTask,
                                            golfRuleStrings + kText_emptyTaskArgument, 0);
        }
        if (!MPBase1_Process(((GolfRulePostRoundView*)this)->retryMenu, dt))
            return;
        SOUND8014((globalBase + 0x30));
        FadeState_RequestFramebufferCapture(Sing_805DC608());
        ((GolfRulePostRoundView*)this)->postRoundMenuState = 3;
        return;
    }
    case 4: {
        if (((GolfRulePostRoundView*)this)->tick4a8 < 15) {
            int alpha = (int)((float)(++((GolfRulePostRoundView*)this)->tick4a8 * 255) / kFloat_15_GolfRule);
            FADE8014((globalBase + 0x18));
            ((CEffectFade*)Sing_805DC660())->SetAlpha((unsigned char)alpha);
            return;
        }
        SOUND8014((globalBase + 0x30));
        FadeState_Reset(Sing_805DC608());
        SOUND8014((globalBase + 0x30));
        FadeState_RequestFramebufferCapture(Sing_805DC608());
        SOUND8014((globalBase + 0x30));
        FadeState_StartFadeIn(Sing_805DC608(), 0);
        ((GolfRulePostRoundView*)this)->postRoundMenuState = 3;
        return;
    }
    case 3: {
        for (int i = 0; i < 4; i++)
            ((void (*)(void*, void*, const char*, int, int, int, int, int))IActor_DispatchMessage)(
                this, this, golfRuleStrings + kText_player, 0x32, 1, i, 0, 0);
        if (AsCSharedDoc(g_sharedDoc)->gameMode == 6) {
            ((GolfRulePostRoundView*)this)->postRoundMenuState = 0;
            FADE8014((globalBase + 0x18));
            ((CEffectFade*)Sing_805DC660())->SetAlpha(0);
            if (((GolfRulePostRoundView*)this)->resultMenu) {
                delete (Virtual8014*)((GolfRulePostRoundView*)this)->resultMenu;
                ((GolfRulePostRoundView*)this)->resultMenu = 0;
            }
            ((void (*)(void*, void*, const char*, int, int, int, int, int))IActor_DispatchMessage)(
                this, this, golfRuleStrings + kText_player, 0x2c, 0, 0, 0, 0);
            ((GolfRulePostRoundView*)this)->value4c8 = 1;
            if (SharedStringState_GetFirstString(1))
                ((CGolfRule*)this)->EnterState(kStateCourseReload);
            else
                ((CGolfRule*)this)->EnterState(kStateTourExit);
        } else {
            int result = 5;
            if (((GolfRulePostRoundView*)this)->retryMenu)
                result = ((Virtual8014*)((GolfRulePostRoundView*)this)->retryMenu)->result;
            if (((GolfRulePostRoundView*)this)->retryMenu) {
                delete (Virtual8014*)((GolfRulePostRoundView*)this)->retryMenu;
                ((GolfRulePostRoundView*)this)->retryMenu = 0;
            }
            ((GolfRulePostRoundView*)this)->postRoundMenuState = 0;
            ((CGolfRule*)this)->ApplyPostRoundChoice(result);
        }
        return;
    }
    case 5: {
        for (int i = 0; 4 > i; i++)
            ((void (*)(void*, void*, const char*, int, int, int, int, int))IActor_DispatchMessage)(
                this, this, golfRuleStrings + kText_player, 0x32, 1, i, 0, 0);
        if (((GolfRulePostRoundView*)this)->retryMenu) {
            delete (Virtual8014*)((GolfRulePostRoundView*)this)->retryMenu;
            ((GolfRulePostRoundView*)this)->retryMenu = 0;
        }
        ((GolfRulePostRoundView*)this)->postRoundMenuState = 0;
        Save8014 save;

        SAVE8014();
        SaveRecord_CopyCtor(&save, SaveData_GetCurrentRecord(GetSaveDataSlot()));
        if (AsCSharedDoc(g_sharedDoc)->golfRuleType == 1) {
            save.flags |= 0x400;
            save.flags2 |= 0xc0;
        }
        // Non-match course-clear completion also sets this unclassified save flag.
        else
            *((unsigned char*)g_sharedDoc + 0x36) = 1;
        SAVE8014();
        SaveData_SetCurrentRecord(GetSaveDataSlot(), &save);
        ((CGolfRule*)this)->ApplyPostRoundChoice(2);
        return;
    }
    }
}

// Return this actor type descriptor.
// Returns the address of the rule's static RTTI descriptor.
// Does not mutate rule state or allocate storage.
// No failure branch; ownership of the returned descriptor stays static.
void* CGolfRule::GetRTTI() {
    using namespace ns3;
    return &s_golfRuleRttiDescriptor;
}

#pragma use_lmw_stmw on
namespace ns4 {
extern "C" {
// ===== S4 - Shot-history container internals =====
// 0x54-byte checkpoint elements; container mechanics, nothing to tune here.
// Two nested containers share these helpers: the OUTER header is six words
// [frontBias, entryCount, mapData, mapHead, mapCount, mapCapacity]; its INNER map
// at +8 is [data, head, count, capacity] of 4-byte block-pointer words. One map
// slot is always spare, distinguishing begin == end (empty) from a full ring.
// Inner iterators are four address words [current, begin, logicalBegin, end],
// optionally followed by an allocator byte. begin/end bound one allocation;
// logicalBegin partitions its high and wrapped-low portions for distance tests.
// An address is not an entry index. All distance/offset helpers here count words.
// Offsets must need at most ONE wrap: after adding an offset to current's index,
// the result must lie in [-capacity, 2*capacity). Zero-capacity rings are used
// only for equal/empty ranges, never for advancing or dereferencing an iterator.
// Pointer/descriptor arguments are required, borrowed, 4-byte aligned, and from
// live storage. Copy helpers mutate descriptors but never allocate/free blocks.
// Payload ownership changes only when Resize swaps maps; InitEmptyHeader owns
// nothing. See InsertIteratorRange for in-place versus reallocation alias rules.

// Destroy the owned container storage and optionally free the object.
void* RingBuffer32_dtor(void* self, int flags) {
    if (self) {
        unsigned char* p = (unsigned char*)self;
        Ring8014F340* r = (Ring8014F340*)self;
        r->unk4 = 0;
        if (r->count != 0) {
            unsigned int* stop = r->data + r->tail();
            unsigned int* cur = r->data + r->head;
            unsigned int* base = r->data;
            unsigned int* end = (unsigned int*)((char*)r->data + r->cap * 4);
            while (cur != stop) {
                if (*cur)
                    appFreeUni__FPv((void*)*cur);
                ++cur;
                if (cur == end)
                    cur = base;
            }
        }
        if (p + 8 && p + 8 && p + 8 && p + 8 && p + 8 && p + 8) {
            void* data = *(void**)(p + 8);
            if (data) {
                r->count -= (p + r->count) - p;
                appFreeUni__FPv(data);
            }
        }
        if (flags > 0)
            __dl__FPv(self);
    }
    return self;
}
}
}

#pragma use_lmw_stmw reset
namespace ns4 {
extern "C" {
#pragma dont_inline on
// Return the address of the circular-buffer element count.
void* RingBuffer32_GetCountAddress(void* self) {
    return &((RingWordStorage*)self)->count;
}
#pragma dont_inline reset
}
}

namespace ns4 {
extern "C" {
// newly allocated blocks stay owned by this history after the storage swap.
void ShotHistory_Resize(void* self, unsigned int count) {
    void* blockMap = RingBuffer32_GetCountAddressForResize(self);
    void* allocationContext = RingBuffer32_GetCountAddress(self);
    unsigned int zero = 0;
    int blocks = (int)(count >> 4) + ((count & 0xF) != 0 ? 1 : 0);

    // Phase 1: reclaim whole blocks hidden by the front bias before allocating.
    while (blocks != 0 && (unsigned int)UnsignedWord_LoadPointerValue(self) >= 0x10) {
        unsigned int value = *(unsigned int*)RingBuffer32_GetFrontPointer(blockMap);
        RingBuffer32_PopFront(blockMap);
        *(unsigned int*)RingBuffer32_GetBackPointer(blockMap) = value;
        unsigned int pushed = zero;
        RingBuffer32_AppendWordForward((unsigned int*)blockMap, &pushed);
        *(unsigned int*)self = *(unsigned int*)self - 0x10;
        blocks--;
    }

    if (blocks == 0)
        return;

    // Phase 2: enforce the existing length bound, then use spare map capacity.
    // entryGrowth counts entries (blocks*16), not allocation bytes.
    unsigned int entryGrowth = (unsigned int)blocks << 4;
    if (entryGrowth > (unsigned int)ShotHistory_GetFrontGrowthBias(self) ||
        (unsigned int)ShotHistory_GetBlockCapacity(self) >
            (unsigned int)((unsigned int)ShotHistory_GetFrontGrowthBias(self) - entryGrowth)) {
        RingBuffer_ThrowLengthError(golfRuleStrings + kText_deque_length_error);
    }

    unsigned int limit = RingBuffer32_GetUsableCapacity(blockMap);
    if ((unsigned int)blocks + (unsigned int)RingBuffer32_GetCountWord(blockMap) <= limit) {
        while (blocks != 0) {
            void* item = ShotHistory_AllocateEntries(allocationContext, 0x10);
            *(unsigned int*)RingBuffer32_GetBackPointer(blockMap) = (unsigned int)item;
            unsigned int pushed = zero;
            RingBuffer32_AppendWordForward((unsigned int*)blockMap, &pushed);
            blocks--;
        }
    } else {
        // Phase 3: allocate a larger map, append new block pointers and transfer
        // the old occupied range excluding its reserved trailing slot.
        Fn8014F458Local local;
        ShotHistory_InitEmptyHeader(&local);
        void* ring = RingBuffer32_GetCountAddressForResize(&local);
        unsigned int twice;
        unsigned int bound = (unsigned int)blocks + (unsigned int)RingBuffer32_GetCountWord(blockMap) + 1;
        twice = (unsigned int)RingBuffer32_GetUsableCapacity(blockMap) << 1;
        void* chosen = UnsignedWord_SelectMaxPointer(&twice, &bound);
        RingBuffer32_ReallocateStorage((unsigned int*)ring, *(unsigned int*)chosen);
        unsigned int pushed = zero;
        RingBuffer32_AppendWordForward((unsigned int*)ring, &pushed);
        while (blocks != 0) {
            void* item = ShotHistory_AllocateEntries(allocationContext, 0x10);
            *(unsigned int*)RingBuffer32_GetBackPointer(ring) = (unsigned int)item;
            unsigned int pushedInner = zero;
            RingBuffer32_AppendWordForward((unsigned int*)ring, &pushedInner);
            blocks--;
        }
        if (!RingBuffer_IsEmpty(blockMap)) {
            unsigned int secondRange[4];
            unsigned int firstRange[4];
            unsigned int before[4];
            unsigned int after[4];
            RingBuffer32_MakeEndIterator(after, (unsigned int*)blockMap);
            RingIterator32_CopyRetreated(before, after, 1);
            RingBuffer32_MakeBeginIteratorVolatile(firstRange, (unsigned int*)blockMap);
            RingBuffer32_MakeBeginIteratorVolatile(secondRange, (unsigned int*)ring);
            ShotHistory_InsertRangeDispatch((unsigned int*)ring, secondRange, firstRange, before, 0);
        }
        // Phase 4: publish the replacement map; the temporary cleans up only
        // the old map allocation after its count is reset, not transferred blocks.
        RingBuffer32_SwapStorage((unsigned int*)blockMap, (unsigned int*)ring);
        RingBuffer32_ResetCount(ring);
    }
}
}
}

namespace ns4 {
extern "C" {
// Construct and throw the container length error with the supplied message.
void RingBuffer_ThrowLengthError(const char* name) {
    Fn8014F6E0Derived local(name);
}
}
}

namespace ns4 {
extern "C" {
// Forward one word to the circular buffer append implementation.
void RingBuffer32_AppendWordForward(unsigned int* self, unsigned int* value) {
    RingBuffer32_AppendWord(self, value);
}
}
}

namespace ns4 {
extern "C" {
// Advance the circular head and decrement the occupied count.
void RingBuffer32_PopFront(void* self) {
    unsigned int v = ((RingWordStorage*)self)->head + 1;
    ((RingWordStorage*)self)->head = v;
    if (v == ((RingWordStorage*)self)->capacity)
        ((RingWordStorage*)self)->head = 0;
    unsigned int v8 = ((RingWordStorage*)self)->count;
    ((RingWordStorage*)self)->count = v8 - 1;
}
}
}

namespace ns4 {
extern "C" {
// Swap the circular-buffer storage words without changing their evaluation order.
void RingBuffer32_SwapStorage(unsigned int* first, unsigned int* second) {
    volatile unsigned int* a = first;
    volatile unsigned int* b = second;
    unsigned int value = a[0];
    a[0] = b[0];
    b[0] = value;
    value = b[1];
    unsigned int other = a[1];
    a[1] = value;
    b[1] = other;
    value = b[2];
    other = a[2];
    a[2] = value;
    b[2] = other;
    value = b[3];
    other = a[3];
    a[3] = value;
    b[3] = other;
}
}
}

namespace ns4 {
extern "C" {
// Construct a begin iterator from the storage header in the original load order.
void RingBuffer32_MakeBeginIteratorVolatile(unsigned int* dst, unsigned int* src) {
    int c = src[0];
    int a = src[3];
    a <<= 2;

    int b = src[1];
    b <<= 2;
    int savedNode = c;
    int savedNode2 = savedNode + b;
    b = savedNode2;
    int savedNode3 = c;
    int savedNode4 = savedNode3 + a;
    dst[1] = c;
    int savedNode5 = b;
    a = savedNode4;
    int savedNode6 = b;
    dst[0] = savedNode5;
    dst[2] = savedNode6;
    int savedNode7 = a;
    dst[3] = savedNode7;
}
}
}

#pragma opt_common_subs off
#pragma opt_lifetimes off
#pragma opt_propagation off
namespace ns4 {
extern "C" {
// Construct an end iterator from the circular head, count and capacity.
void RingBuffer32_MakeEndIterator(unsigned int* destination, const unsigned int* source) {
    unsigned int index;
    index = source[3] ? (source[1] + source[2]) % source[3] : 0;
    unsigned int* base = (unsigned int*)source[0];
    ((Fn8014F874Iterator*)destination)->__ct(base + source[3], base + source[1], base, base + index);
}
}
}

#pragma opt_common_subs reset
#pragma opt_lifetimes reset
#pragma opt_propagation reset
namespace ns4 {
extern "C" {
// Return the first occupied word using the current circular-buffer head index.
void* RingBuffer32_GetFrontPointer(void* self) {
    return ((RingWordStorage*)self)->data + ((RingWordStorage*)self)->head;
}
}
}

namespace ns4 {
extern "C" {
// Return the last occupied word, accounting for circular-buffer wraparound.
void* RingBuffer32_GetBackPointer(void* self) {
    unsigned int a = ((RingWordStorage*)self)->head;
    unsigned int b = ((RingWordStorage*)self)->count;
    unsigned int c = ((RingWordStorage*)self)->capacity;
    void* base = *(void**)self;
    unsigned int sum = a + b;
    unsigned int rem = (sum - 1) % c;
    return (char*)base + rem * 4;
}
}
}

namespace ns4 {
extern "C" {
// Destroy the owned container storage and optionally free the object.
void* RingContainer_dtor(char* self, int flags) {
    if (self != 0) {
        void* ptr = *(void**)self;
        if (ptr != 0) {
            register int value = *(volatile int*)(self + 8);
            *(int*)(self + 8) = value - value;
            appFreeUni__FPv(ptr);
        }
        if (flags > 0)
            __dl__FPv(self);
    }
    return self;
}
}
}

namespace ns4 {
extern "C" {
// Read the circular-buffer count through the existing pointer-shaped return type.
void* RingBuffer32_GetCountWord(void* self) {
    return (void*)((RingWordStorage*)self)->count;
}
}
}

namespace ns4 {
extern "C" {
// Report whether the circular-buffer element count is zero.
int RingBuffer_IsEmpty(void* self) {
    return ((RingWordStorage*)self)->count == 0;
}
}
}

namespace ns4 {
extern "C" {
// Return capacity excluding the circular buffer's reserved empty slot.
unsigned int RingBuffer32_GetUsableCapacity(void* self) {
    unsigned int value = ((RingWordStorage*)self)->capacity;
    return value == 0 ? 0 : value - 1;
}
}
}

#pragma opt_common_subs off
#pragma use_lmw_stmw on
namespace ns4 {
extern "C" {
// Reallocate circular-buffer storage and preserve the occupied wrapped ranges.
void RingBuffer32_ReallocateStorage(unsigned int* raw, unsigned int index) {
    char* source;
    Fn8014F9C0Err* localPtr;
    char* copy;
    unsigned int* oldBase;
    unsigned int* middle;
    unsigned int* oldEnd;
    unsigned int* capEnd;
    Fn8014F9C0Ring* self;
    unsigned int* insert;
    self = (Fn8014F9C0Ring*)raw;
    unsigned int size = self->cap;
    unsigned int maxSize = size == 0 ? 0 : size - 1;
    if (index > maxSize) {
        Fn8014F9C0Ring tmp(&self->cap);
        if (!s_ringBufferWordLimitInitialized) {
            s_ringBuffer32MaxWords = 0x3FFFFFFE;
            s_ringBufferWordLimitInitialized = 1;
        }
        if (index > s_ringBuffer32MaxWords) {
            source = golfRuleStrings + kText_cdeque_length_error;
            Fn8014F9C0Err local;
            localPtr = &local;
            localPtr->vtable = __vt__Q23std11logic_error;
            volatile char allocator = 0;
            unsigned int len = strlen(source) + 1;
            copy = (char*)__nwa__FUl(len * 2 - len);
            localPtr->data = copy;
            {
                Fn801510E8Guard guard(copy, (char*)&allocator);
                Fn801510E8Buffer* buffer = (Fn801510E8Buffer*)__nw__FUl(0x10);
                if (buffer) {
                    buffer->a = 1;
                    buffer->b = 1;
                    buffer->vtable = s_sharedCharDeleterVtable;
                    buffer->data = copy;
                }
                localPtr->buffer = buffer;
                guard.data = 0;
            }
            strcpy(localPtr->data, source);
            localPtr->vtable = __vt__Q23std12length_error;
            __throw(golfRuleStrings + kText_lengthErrorTypeName, localPtr, (void*)__dt__Q23std12length_errorFv);
        }
        tmp.data = (unsigned int*)appMallocDummy__FUl((index + 1) * 4);
        tmp.cap = index + 1;
        unsigned int offset = self->cap ? (self->start + self->len) % self->cap : 0;
        insert = tmp.data + ((tmp.start + tmp.len) % tmp.cap);
        oldBase = self->data;
        middle = oldBase + offset;
        capEnd = oldBase + self->cap;
        oldEnd = oldBase + self->start;
        if (oldEnd <= middle) {
            memmove(insert, oldEnd, (middle - oldEnd) * 4);
            tmp.len += middle - oldEnd;
        } else {
            unsigned int n = (capEnd - oldEnd) * 4;
            memmove(insert, oldEnd, n);
            memmove((char*)insert + n, oldBase, (middle - oldBase) * 4);
            tmp.len += (capEnd - oldEnd) + (middle - oldBase);
        }
        self->len = 0;
        unsigned int t0, t1, t2, t3;
        t0 = (unsigned int)self->data;
        self->data = tmp.data;
        tmp.data = (unsigned int*)t0;
        t1 = self->start;
        self->start = tmp.start;
        tmp.start = t1;
        t2 = self->len;
        self->len = tmp.len;
        tmp.len = t2;
        t3 = self->cap;
        self->cap = tmp.cap;
        tmp.cap = t3;
    }
}
}
}

#pragma use_lmw_stmw reset
namespace ns4 {
extern "C" {
// Build the circular word buffer begin iterator from its head index.
void RingBuffer32_MakeBeginIterator(void* destination, const unsigned int* source) {
    volatile unsigned int* output = (volatile unsigned int*)destination;
    const volatile unsigned int* input = source;
    unsigned int head = input[1];
    unsigned int base = input[0];
    output[0] = base + head * 4;
    base = input[0];
    output[1] = base;
    output[2] = base + input[1] * 4;
    output[3] = base + input[3] * 4;
}
}
}

namespace ns4 {
extern "C" {
// Construct the circular end iterator while preserving the original volatile loads.
void RingBuffer32_MakeEndIteratorVolatile(unsigned int* destination, const unsigned int* source) {
    volatile unsigned int* output = destination;
    const volatile unsigned int* input = source;
    unsigned int base;
    unsigned int count;
    count = input[3];
    base = input[0];
    unsigned int index = count ? (input[1] + input[2]) % count : 0;
    output[0] = base + index * 4;
    base = input[0];
    output[1] = base;
    output[2] = base + input[1] * 4;
    output[3] = base + input[3] * 4;
}
}
}

#pragma opt_common_subs reset
#pragma use_lmw_stmw on
namespace ns4 {
extern "C" {
inline unsigned int Fn8014FD54Dist(unsigned int* first, unsigned int* last) {
    return ((int)last - (int)first) / 4;
}
inline unsigned int* Fn8014FD54Copy(unsigned int* first, unsigned int* last, unsigned int* dest) {
    unsigned int* d = dest - (last - first);
    memmove(d, first, (last - first) * 4);
    return d;
}
// Append one word, growing and copying wrapped storage when necessary.
void RingBuffer32_AppendWord(unsigned int* self, unsigned int* value) {
    // Phase 1: construct in the wrapped tail if the reserved slot remains free.
    if (self[2] + 1 < self[3]) {
        unsigned int* slot = (unsigned int*)self[0] + ((self[1] + self[2]) % self[3]);
        new (slot) unsigned int(*value);
        self[2] += 1;
        return;
    }
    Fn8014FD54Ring tmp(self + 3);
    unsigned int one = 1;
    unsigned int growthB;
    unsigned int growthA;
    // Phase 2: check length/growth bounds before allocating replacement storage.
    if (s_uintVectorMaxSizeInitialized == 0) {
        s_uintVectorMaxSizeInitialized = 1;
        s_fourByteVectorMaxElements = 0x3FFFFFFE;
    }
    unsigned int cap = self[3];
    unsigned int n = s_fourByteVectorMaxElements;
    unsigned int size = !cap ? 0 : cap - 1;
    if (n - size < 1) {
        Fn8014FD54Err local;
        char* source = golfRuleStrings + kText_cdeque_length_error;

        Fn8014FD54Err* localPtr = &local;
        Fn8014FD54Alloc allocator2;
        local.vtable = __vt__Q23std11logic_error;
        allocator2.c = 0;
        Fn8014FD54Alloc allocator;
        allocator.c = 0;
        int len = strlen(source) + 1;
        char* copy = (char*)__nwa__FUl(2 * len - len);
        local.data = copy;
        {
            Fn801510E8Guard guard(copy, (char*)&allocator.c);
            Fn8014F6E0Buffer* buffer = (Fn8014F6E0Buffer*)__nw__FUl(0x10);
            if (buffer) {
                buffer->a = 1;
                buffer->b = 1;
                buffer->vtable = s_sharedCharDeleterVtable;
                buffer->data = copy;
            }
            local.buffer = buffer;
            guard.data = 0;
        }
        strcpy(local.data, source);
        localPtr->vtable = __vt__Q23std12length_error;
        __throw(golfRuleStrings + kText_lengthErrorTypeName, localPtr, (void*)__dt__Q23std12length_errorFv);
    }

    if (size < n / 3) {
        growthA = ((size + 1) * 3) / 5;
        n = size + *(((growthA < one)) ? &one : &growthA);
    } else if (size < (n / 3) * 2) {
        growthB = (size + 1) / 2;
        n = size + *((growthB < one) ? &one : &growthB);
    }
    {
        Fn8014FD54Alloc alloc3;
        alloc3.c = 0;
        if (s_ringBufferWordLimitInitialized == 0) {
            s_ringBuffer32MaxWords = 0x3FFFFFFE;
            s_ringBufferWordLimitInitialized = 1;
        }
        if (n > s_ringBuffer32MaxWords) {
            Fn8014FD54Err local;
            char* source = golfRuleStrings + kText_cdeque_length_error;

            local.vtable = __vt__Q23std11logic_error;
            Fn8014FD54Err* localPtr = &local;
            Fn8014FD54Alloc allocator2;
            allocator2.c = 0;
            Fn8014FD54Alloc allocator;
            allocator.c = 0;
            unsigned int len = strlen(source) + 1;
            char* copy = (char*)__nwa__FUl((2 * len) - len);
            local.data = copy;
            {
                Fn801510E8Guard guard(copy, (char*)&allocator.c);
                Fn8014F6E0Buffer* buffer = (Fn8014F6E0Buffer*)__nw__FUl(0x10);
                if (buffer) {
                    buffer->a = 1;
                    buffer->b = 1;
                    buffer->vtable = s_sharedCharDeleterVtable;
                    buffer->data = copy;
                }
                guard.data = 0;
                local.buffer = buffer;
            }
            strcpy(local.data, source);
            localPtr->vtable = __vt__Q23std12length_error;
            __throw(golfRuleStrings + kText_lengthErrorTypeName, localPtr, (void*)__dt__Q23std12length_errorFv);
        }
    }
    // Phase 3: construct the word, copy old wrapped ranges and swap storage;
    // the temporary owns any allocation left after the swap.
    tmp.data = (unsigned int*)appMallocDummy__FUl(((1 + n)) * 4);
    tmp.cap = 1 + n;
    tmp.start = self[2];
    unsigned int* slot = ((tmp.data + tmp.start));
    new (slot) unsigned int(*value);
    tmp.len++;
    unsigned int offset = self[3] ? (self[1] + self[2]) % self[3] : 0;
    unsigned int* oldBase = (unsigned int*)self[0];
    unsigned int* middle = oldBase + offset;
    unsigned int* first = self[1] + oldBase;
    unsigned int* capEnd = (((oldBase + self[3])));
    Fn8014FD54Alloc alloc4;
    alloc4.c = 0;
    unsigned int* insert = tmp.data + tmp.start;
    if (!(!(first <= middle))) {
        Fn8014FD54Copy(first, middle, insert);
        unsigned int n = Fn8014FD54Dist(first, middle);
        tmp.len += n;
        tmp.start -= n;
    } else {
        unsigned int* pos = Fn8014FD54Copy(oldBase, middle, insert);
        Fn8014FD54Copy(first, capEnd, pos);
        unsigned int count = Fn8014FD54Dist(oldBase, middle) + Fn8014FD54Dist(first, capEnd);
        tmp.start = tmp.start - count;
        tmp.len = count + tmp.len;
    }
    self[2] = 0;
    Fn8014FD54Alloc alloc5;
    unsigned int t;
    alloc5.c = 0;
    t = self[0];
    self[0] = (unsigned int)tmp.data;
    tmp.data = (unsigned int*)t;
    t = self[1];
    self[1] = tmp.start;
    tmp.start = t;
    t = self[2];
    self[2] = tmp.len;
    tmp.len = t;
    t = self[3];
    self[3] = tmp.cap;
    tmp.cap = t;
}
}
}

#pragma use_lmw_stmw reset
namespace ns4 {
extern "C" {
// Reset the circular-buffer count using the original pointer-difference expression.
void RingBuffer32_ResetCount(void* self) {
    (*(int*)&((RingWordStorage*)self)->count) -=
        ((char*)self + (*(int*)&((RingWordStorage*)self)->count)) - (char*)self;
}
}
}

#pragma use_lmw_stmw on
namespace ns4 {
extern "C" {
// Choose bounded growth for requested additional words; throw on maximum-size overflow.
unsigned int RingBuffer32_CalculateGrowth(void* self, unsigned int requested) {
    unsigned int growthA;
    unsigned int growthB;
    if (!s_uintVectorMaxSizeInitialized) {
        s_fourByteVectorMaxElements = 0x3ffffffe;
        s_uintVectorMaxSizeInitialized = 1;
    }
    unsigned int limit = s_fourByteVectorMaxElements;
    unsigned int cap = ((RingWordStorage*)self)->capacity;
    unsigned int size = cap == 0 ? 0 : cap - 1;
    if (requested > limit - size) {
        Fn8014FD54Err error;
        Fn8014FD54Err* local = &error;
        char* source = golfRuleStrings + kText_cdeque_length_error;
        volatile char allocator = 0;
        local->vtable = __vt__Q23std11logic_error;
        unsigned int n = strlen(source) + 1;
        char* copy = (char*)__nwa__FUl(n * 2 - n);
        local->data = copy;
        {
            Fn8014F6E0Guard guard(copy, (char*)&allocator);
            Fn8014F6E0Buffer* p = (Fn8014F6E0Buffer*)__nw__FUl(0x10);
            if (p) {
                p->a = 1;
                p->b = 1;
                p->vtable = s_sharedCharDeleterVtable;
                p->data = copy;
            }
            local->buffer = p;
            guard.data = 0;
        }
        strcpy(local->data, source);
        local->vtable = __vt__Q23std12length_error;
        __throw(golfRuleStrings + kText_lengthErrorTypeName, local, (void*)__dt__Q23std12length_errorFv);
    }
    unsigned int third = limit / 3;
    if (size < third) {
        growthA = ((size + 1) * 3) / 5;
        return size + *(growthA < requested ? &requested : &growthA);
    }
    if (size < third * 2) {
        growthB = (size + 1) / 2;
        return size + *(growthB < requested ? &requested : &growthB);
    }
    return limit;
}
}
}

#pragma use_lmw_stmw reset
namespace ns4 {
extern "C" {
// Clear the circular-buffer element count while retaining its allocation.
void RingBuffer32_ClearCount(void* self) {
    (*(int*)&((RingWordStorage*)self)->count) = 0;
}
}
}

namespace ns4 {
extern "C" {
// Copy an iterator and move its current pointer backward with wraparound.
void RingIterator32_CopyRetreated(unsigned int* destination, const unsigned int* source, int offset) {
    struct Iterator {
        unsigned int current, begin, unknown, end;
    };
    int negOffset = 0 - offset;
    Iterator local = *(const Iterator*)source;
    int count = ((int)local.end - (int)local.begin) / 4;
    int index = negOffset + ((int)local.current - (int)local.begin) / 4;
    if (index < 0)
        index += count;
    else if (index >= count)
        index -= count;
    local.current = ((volatile Iterator*)&local)->begin + index * 4;
    *(Iterator*)destination = local;
}
}
}

namespace ns4 {
extern "C" {
// Copy the four-word circular iterator descriptor.
void* RingIterator32_Copy(unsigned int* destination, const unsigned int* source) {
    struct Words {
        unsigned int values[4];
    };
    *(Words*)destination = *(const Words*)source;
    return destination;
}
}
}

namespace ns4 {
extern "C" {
// Return the iterator current pointer.
void* RingIterator32_GetCurrentPointer(void* self) {
    return *(void**)self;
}
}
}

#pragma opt_lifetimes off
#pragma opt_propagation off
#pragma use_lmw_stmw on
namespace ns4 {
extern "C" {
struct V4U {
    unsigned int v[4];
};
// Snapshot four address words from each required descriptor, then insert into self.
// Source/position storage is read before self changes and is not written back;
// the insertion routine receives independent local descriptor copies. flag is
// unused; the explicit allocator context passed instead is a zero byte. The two
// range copies' own trailing allocator bytes remain unspecified, as before;
// wrapped-copy helpers propagate that metadata but never branch on it here.
// Source payload still follows InsertIteratorRange's nonaliasing precondition.
void ShotHistory_InsertRangeDispatch(unsigned int* self, const void* position, const void* sourceFirst,
                                     const void* sourceLast, int flag) {
    V4U positionCopy;
    Fn80150508Range firstCopy;
    Fn80150508Range lastCopy;
    unsigned int positionEnd, positionPivot, positionBegin, positionCurrent, zero, lastCurrent, lastBegin, lastPivot,
        lastEnd, firstCurrent, firstBegin, firstPivot, firstEnd;
    zero = 0;
    lastCurrent = ((const unsigned int*)sourceLast)[0];
    lastBegin = ((const unsigned int*)sourceLast)[1];
    lastPivot = ((const unsigned int*)sourceLast)[2];
    lastEnd = ((const unsigned int*)sourceLast)[3];
    firstCurrent = ((const unsigned int*)sourceFirst)[0];
    firstBegin = ((const unsigned int*)sourceFirst)[1];
    firstPivot = ((const unsigned int*)sourceFirst)[2];
    firstEnd = ((const unsigned int*)sourceFirst)[3];
    positionCurrent = ((const unsigned int*)position)[0];
    positionBegin = ((const unsigned int*)position)[1];
    positionPivot = ((const unsigned int*)position)[2];
    positionEnd = ((const unsigned int*)position)[3];
    lastCopy.a = lastCurrent;
    lastCopy.b = lastBegin;
    lastCopy.c = lastPivot;
    lastCopy.d = lastEnd;
    firstCopy.a = firstCurrent;
    firstCopy.b = firstBegin;
    firstCopy.c = firstPivot;
    firstCopy.d = firstEnd;
    positionCopy.v[0] = positionCurrent;
    positionCopy.v[1] = positionBegin;
    positionCopy.v[2] = positionPivot;
    positionCopy.v[3] = positionEnd;
    char allocator = zero;
    ShotHistory_InsertIteratorRange(self, positionCopy.v, &firstCopy, &lastCopy, &allocator);
}
}
}

#pragma opt_lifetimes reset
#pragma opt_propagation reset
#pragma use_lmw_stmw reset
namespace ns4 {
extern "C" {
// Return the address of the larger unsigned word.
void* UnsignedWord_SelectMaxPointer(void* first, void* second) {
    unsigned int firstValue = *(unsigned int*)first;
    unsigned int secondValue = *(unsigned int*)second;
    return firstValue < secondValue ? second : first;
}
}
}

namespace ns4 {
extern "C" {
// Swap two integer values.
void Swap_Int32(int* first, int* second) {
    int value = *first;
    *first = *second;
    *second = value;
}
}
}

namespace ns4 {
extern "C" {
// Swap two integer storage words.
void Swap_Int32ForRingStorage(int* first, int* second) {
    int value = *first;
    *first = *second;
    *second = value;
}
}
}

#pragma dont_inline on
#pragma use_lmw_stmw on
namespace ns4 {
extern "C" {
// Insert [sourceFirst,sourceLast) before insertionPosition in the INNER word map.
// All descriptors are borrowed snapshots; position belongs to self, and source
// endpoints describe one valid nonnegative range with at most one wrap. Source
// payload must be independent of the map being shifted/reallocated: self-range
// insertion is not supported by the observed Resize caller (it uses a temporary).
// No return/output iterator: sourceFirst is advanced ONLY in the growth branch;
// position and sourceLast remain unchanged. allocatorContext is unused here.
// Empty range returns without writes. With room: publish count/head, move the
// shorter side (forward for prefix, backward for suffix), then fill the gap.
// Without room: construct a temporary map, copy inserted words, append old suffix,
// prepend old prefix, clear old count, swap storage, free old map (not its blocks).
// Allocation/growth may throw; there is no local rollback guard for the temporary
// or already published in-place writes. Do not infer a strong exception guarantee.
// The separately named descriptor copies retain lifetime and load ordering;
// their stack placement is not part of the container's public representation.
void ShotHistory_InsertIteratorRange(void* self, unsigned int* insertionPosition, void* sourceFirst, void* sourceLast,
                                     void* allocatorContext) {
    Fn801505E8Ring* ring = (Fn801505E8Ring*)self;
    Fn801505E8Ring replacementMap;
    unsigned int distanceFirst[5], distanceLast[5], copyFirst[5], copyLast[5];
    unsigned int gapCursor[4], oldBegin[4];
    unsigned int prefixResult[4];
    unsigned int prefixFirst[4], prefixLast[4], prefixDestination[4], newEnd[4];
    unsigned int suffixResult[4];
    unsigned int suffixFirst[4], suffixLast[4], suffixDestination[4], insertedResult[4], insertedDestination[4],
        oldEnd[4], originalBegin[4];
    unsigned int* lastForDistance;
    unsigned int* logicalBegin;
    unsigned int* constructionSlot;
    int insertedWords;
    int prefixWords;
    unsigned int usableCapacity;
    void* lastForCopy;

    lastForDistance = (unsigned int*)ShotHistory_CopyIteratorRange(distanceLast, sourceLast);
    insertedWords = RingIterator32_Distance((unsigned int*)ShotHistory_CopyIteratorRange(distanceFirst, sourceFirst),
                                            lastForDistance);
    if (insertedWords == 0)
        return;

    logicalBegin = ring->data + ring->head;
    prefixWords = ((int)(*insertionPosition - (unsigned int)logicalBegin)) / 4;
    if (*insertionPosition < (unsigned int)logicalBegin)
        prefixWords += *(unsigned int*)RingBuffer32_GetCapacityAddressForInsert(self);

    usableCapacity = RingBuffer32_GetUsableCapacity(self);
    if ((unsigned int)insertedWords <= usableCapacity && ring->count <= usableCapacity - insertedWords) {
        RingBuffer32_MakeBeginIterator(gapCursor, (unsigned int*)self);
        unsigned int suffixWords = ring->count - prefixWords;
        if ((unsigned int)prefixWords < suffixWords) {
            RingIterator32_Copy(oldBegin, gapCursor);
            RingIterator32_RetreatInPlace(gapCursor, insertedWords);
            unsigned int* first = (unsigned int*)RingIterator32_GetCurrentPointer(gapCursor);
            int head = first - ring->data;
            ring->count = ring->count + insertedWords;
            ring->head = head;
            if (prefixWords != 0) {
                void* prefixOutput = RingIterator32_Copy(prefixDestination, gapCursor);
                RingIterator32_AddOffset(prefixLast, oldBegin, prefixWords);
                ((void (*)(void*, void*, void*, void*))RingIterator32_CopyForward)(
                    prefixResult, RingIterator32_Copy(prefixFirst, oldBegin), prefixLast, prefixOutput);
                RingIterator32_CopyWords(gapCursor, prefixResult);
            }
        } else {
            ring->count = ring->count + insertedWords;
            RingBuffer32_MakeEndIteratorVolatile(newEnd, (unsigned int*)self);
            RingIterator32_CopyWords(gapCursor, newEnd);
            if (suffixWords != 0) {
                void* suffixOutput = RingIterator32_Copy(suffixDestination, gapCursor);
                RingIterator32_SubtractOffset(suffixLast, gapCursor, insertedWords);
                RingIterator32_SubtractOffset(suffixFirst, gapCursor, insertedWords + suffixWords);
                ((void (*)(void*, void*, void*, void*))RingIterator32_CopyBackward)(suffixResult, suffixFirst,
                                                                                    suffixLast, suffixOutput);
                RingIterator32_CopyWords(gapCursor, suffixResult);
            }
            RingIterator32_RetreatInPlace(gapCursor, insertedWords);
        }
        void* gapForCopy = RingIterator32_Copy(insertedDestination, gapCursor);
        lastForCopy = ShotHistory_CopyIteratorRange(copyLast, sourceLast);
        void* firstForCopy = ShotHistory_CopyIteratorRange(copyFirst, sourceFirst);
        ShotHistory_CopyWrappedBlocks((RingBase*)insertedResult, (RingIt*)firstForCopy, (RingIt*)lastForCopy,
                                      (RingIt*)gapForCopy);
        return;
    }

    {
        RingBuffer32_InitializeHeader(&replacementMap,
                                      (unsigned int)RingBuffer32_GetCapacityAddressForAllocation(self));
        RingBuffer32_AllocateWords(&replacementMap, (void*)(ring->count + insertedWords),
                                   RingBuffer32_CalculateGrowth(self, insertedWords));
        replacementMap.head = prefixWords;
        constructionSlot = replacementMap.data + prefixWords;
        while (insertedWords != 0) {
            void* blockPointer = RingBuffer32_GetFirstPointer(sourceFirst);
            RingBuffer32_ConstructWord(RingBuffer32_GetAllocationWord(&replacementMap), constructionSlot,
                                       &blockPointer);
            unsigned int constructedWords = replacementMap.count;
            insertedWords -= 1;
            constructionSlot += 1;
            replacementMap.count = constructedWords + 1;
            RingIterator32_AdvanceInPlace(sourceFirst);
        }
        RingBuffer32_MakeEndIteratorVolatile(oldEnd, (unsigned int*)self);
        RingBuffer32_AppendIteratorRange((unsigned int*)&replacementMap, insertionPosition, oldEnd);
        RingBuffer32_MakeBeginIterator(originalBegin, (unsigned int*)self);
        RingBuffer32_CopyWrappedRangeToTail((Data512*)&replacementMap, (Iter512*)originalBegin,
                                            (Iter512*)insertionPosition);
        RingBuffer32_ClearCount(self);
        Swap_Int32ForRingStorage((int*)self, (int*)&replacementMap);
        Swap_Int32((int*)((char*)self + 4), (int*)&replacementMap.head);
        Swap_Int32((int*)((char*)self + 8), (int*)&replacementMap.count);
        unsigned int* replacementCapacity = (unsigned int*)RingBuffer32_GetCapacityAddress(&replacementMap);
        Swap_Int32((int*)RingBuffer32_GetCapacityAddressForInsert(self), (int*)replacementCapacity);
        RingBuffer32_Release((char*)&replacementMap, -1);
    }
}
}
}

#pragma dont_inline reset
#pragma opt_common_subs off
#pragma opt_lifetimes off
#pragma opt_propagation off
#pragma use_lmw_stmw reset
namespace ns4 {
extern "C" {
static inline RingIt ring_advance(RingIt it, const int& selected, int& count) {
    int size = (int)(it.b.capacity - it.b.begin);
    int index = (count = selected) + (int)(it.b.first - it.b.begin);
    if (index < 0)
        index += size;
    else if (index >= size)
        index -= size;
    it.b.first = it.b.begin + index;
    return it;
}
// Signed logical distance across logicalBegin, in 4-byte map words. Both pointers
// belong to last's allocation and the range is shorter than one full revolution.
static inline int ring_distance(RingIt* last, RingValue* target, RingValue* pos) {
    int distance;
    int size;
    RingValue* boundary;
    RingValue* begin;
    RingValue* capacity;
    distance = (int)(target - pos);
    boundary = last->b.end;
    if (target >= boundary) {
        if (pos < boundary) {
            begin = last->b.begin;
            capacity = last->b.capacity;
            size = (int)(capacity - begin);
            distance -= size;
        }
    } else if (pos >= boundary) {
        begin = last->b.begin;
        capacity = last->b.capacity;
        size = (int)(capacity - begin);
        distance += size;
    }
    return distance;
}
// Copy borrowed block POINTER words, not the 0x54-byte checkpoint payloads.
// [source,last) must be a valid forward range shorter than a full revolution;
// dest has room for that many words. Separate allocations or forward-safe overlap
// are required (a later overlapping destination would overwrite unread words).
// Each chunk consumes min(destination contiguous room, remaining); that value
// must be positive. Source wraps per word, destination wraps after each chunk.
// remaining strictly decreases to zero, so each range is traversed exactly once.
// dest advances on EVERY chunk; source is saved only between chunks, so it stays
// at the start of the FINAL chunk on return. last is unchanged. result receives
// four destination words, without allocator flag. Empty range only copies dest.
// Caller uses separate scratch descriptors/result; no allocations or callbacks.
void ShotHistory_CopyWrappedBlocks(RingBase* result, RingIt* source, RingIt* last, RingIt* dest) {
    RingValue* target;
    RingValue* pos;
    RingValue* to;
    RingValue* from;
    RingValue* beg;
    RingValue* cap;
    RingValue* moved;
    RingValue* limit;
    int left;
    int count;
    unsigned int flag;
    int remain;
    int room;
    pos = source->b.first;
    target = last->b.first;
    if (pos != target) {
        remain = ring_distance(last, target, pos);
        for (;;) {
            room = (int)(dest->b.capacity - dest->b.first);
            RingIt next = ring_advance(*source, copy_min(room, remain), count);
            pos = next.b.first;
            flag = next.flag;
            to = dest->b.first;
            from = source->b.first;
            beg = source->b.begin;
            cap = source->b.capacity;
            while (from != pos) {
                RingValue value = *from++;
                *to = value;
                if (from == cap)
                    from = beg;
                to++;
            }
            moved = dest->b.first + (count - 1) + 1;
            limit = dest->b.capacity;
            left = remain - count;
            dest->b.first = moved;
            if (moved == limit)
                dest->b.first = dest->b.begin;
            remain = left;
            if (left == 0)
                break;
            source->b = next.b;
            source->flag = flag;
        }
    }
    RingValue* out_cap;
    RingValue* out_end;
    RingValue* out_begin;
    RingValue* out_first;
    out_first = dest->b.first;
    out_begin = dest->b.begin;
    out_end = dest->b.end;
    out_cap = dest->b.capacity;
    result->first = out_first;
    result->begin = out_begin;
    result->end = out_end;
    result->capacity = out_cap;
}
}
}

#pragma opt_common_subs reset
#pragma opt_lifetimes reset
#pragma opt_propagation reset
#pragma use_lmw_stmw on
namespace ns4 {
extern "C" {
// Copy one contiguous word span ending at result; memmove permits local overlap.
inline int* copy_backward(int* first, int* last, int* result) {
    int count = last - first;
    result -= count;
    memmove(result, first, count * 4);
    return result;
}
// Move [a,b) into the range ending at c. a is unchanged; b/c retreat together
// until b.current == a.current; returned descriptor is the new destination start.
// Descriptors are separate borrowed snapshots, with the common S4 ring invariant.
// Storage must be disjoint or overlap in the backward-safe direction (destination
// later than source logically). Chunk memmove alone does not make arbitrary
// overlap safe across wrap. Each iteration includes the initial one-word retreat
// plus n more words, guaranteeing progress even when n == 0 at a boundary.
CopyIterator RingIterator32_CopyBackward(const CopyIterator* a, CopyIterator* b, CopyIterator* c) {
    while (a->current != b->current) {
        if (b->current == b->begin)
            b->current = b->end;
        int* previous = b->current - 1;
        int* current = c->current;
        int* begin = c->begin;
        b->current = previous;
        if (current == begin)
            c->current = c->end;
        --c->current;
        int distance = (char*)c->current - (char*)(c->current >= c->middle ? c->middle : c->begin);
        int* boundary = b->middle;
        int* last = b->current;
        int* first = a->current;
        int* firstBoundary = a->middle;
        int size = distance / 4;
        int n;
        if ((first >= firstBoundary) == (last >= boundary))
            n = copy_min(size, last - first);
        else
            n = copy_min(size, last - (last >= boundary ? boundary : b->begin));
        b->current -= n;
        copy_backward(b->current, (int*)((char*)b->current + (n + 1) * 4), c->current + 1);
        c->current -= n;
    }
    return CopyIterator(c->end, c->middle, c->begin, c->current);
}
}
}

namespace ns4 {
extern "C" {
// Equality by current address: the reserved map slot excludes a full-ring range.
inline bool copy_equal(const CopyIterator* a, const CopyIterator* b) {
    return a->current == b->current;
}
// Copy a contiguous span without changing descriptors; local overlap uses memmove.
inline void copy_forward(int* first, int* last, int* result) {
    memmove(result, first, (last - first) * 4);
}
// Move [a,b) to c, advancing a/c to their respective ends; b is unchanged.
// Returns the final c descriptor. Separate descriptors and enough destination
// room are required. Storage is disjoint or forward-safe (destination earlier
// logically); arbitrary overlap across wrapped chunks is not supported.
// Chunk length is bounded by destination end and source stop/end. The common S4
// valid-range invariant makes it positive; both cursors wrap once per chunk and
// the finite source range shrinks to a.current == b.current. Empty range is a no-op.
CopyIterator RingIterator32_CopyForward(CopyIterator* a, CopyIterator* b, CopyIterator* c) {
    while (!copy_equal(a, b)) {
        int* stop = b->current;
        int* boundary = b->middle;
        bool right = stop >= boundary;
        int* first = a->current;
        bool left = first >= a->middle;
        int size = c->end - c->current;
        int n;
        if (left == right)
            n = copy_min(size, stop - first);
        else
            n = copy_min(size, a->end - first);
        copy_forward(a->current, (int*)((char*)a->current + n * 4), c->current);
        a->current += n - 1;
        if (++a->current == a->end)
            a->current = a->begin;
        c->current += n - 1;
        if (++c->current == c->end)
            c->current = c->begin;
    }
    return CopyIterator(c->end, c->middle, c->begin, c->current);
}
}
}

#pragma use_lmw_stmw reset
namespace ns4 {
extern "C" {
// Count [first,second) in words using second's logical-begin partition. Both
// descriptors refer to the same live allocation/range; equal current means zero,
// not a full revolution. Does not mutate descriptors or dereference payloads.
int RingIterator32_Distance(const unsigned int* first, const unsigned int* second) {
    unsigned int end;
    unsigned int current;
    unsigned int begin;
    unsigned int base;
    int distance;
    current = second[0];
    base = first[0];
    end = second[2];
    distance = (int)(current - base) / 4;
    begin = second[1];
    unsigned int limit = second[3];
    if (current >= end) {
        if (base >= end)
            return distance;
        return distance - (int)(limit - begin) / 4;
    }
    if (base < end)
        return distance;
    return distance + (int)(limit - begin) / 4;
}
}
}

namespace ns4 {
extern "C" {
// Copy the iterator-range descriptor, including its trailing flag byte.
void* ShotHistory_CopyIteratorRange(void* dst, const void* src) {
    struct Block {
        unsigned int a, b, c, d;
        unsigned char e;
    };
    *(Block*)dst = *(const Block*)src;
    return dst;
}
}
}

namespace ns4 {
extern "C" {
// Move the current iterator backward with wraparound.
void RingIterator32_RetreatInPlace(void* self, int offset) {
    int index = -offset;
    int base = (*(int*)&((RingIteratorAddressWords*)self)->begin);
    int end = (*(int*)&((RingIteratorAddressWords*)self)->end);
    int cur = *(int*)self;
    int count = (end - base) / 4;
    int delta = (cur - base) / 4;
    index = index + delta;
    if (index < 0)
        index = index + count;
    else if (index >= count)
        index = index - count;
    *(int*)self = base + index * 4;
}
}
}

namespace ns4 {
extern "C" {
// Copy the iterator and advance it by a wrapped word offset.
void RingIterator32_AddOffset(unsigned int* destination, const unsigned int* source, int offset) {
    struct Iterator {
        unsigned int current, begin, unknown, end;
    };
    Iterator local = *(const Iterator*)source;
    int count = ((int)local.end - (int)local.begin) / 4;
    int index = offset + ((int)local.current - (int)local.begin) / 4;
    if (index < 0)
        index += count;
    else if (index >= count)
        index -= count;
    local.current = ((volatile Iterator*)&local)->begin + index * 4;
    *(Iterator*)destination = local;
}
}
}

namespace ns4 {
extern "C" {
// Copy the four iterator words without advancing either iterator.
void RingIterator32_CopyWords(unsigned int* destination, const unsigned int* source) {
    struct Words {
        unsigned int values[4];
    };
    *(Words*)destination = *(const Words*)source;
}
}
}

namespace ns4 {
extern "C" {
// Copy the iterator and retreat it by a wrapped word offset.
void RingIterator32_SubtractOffset(unsigned int* destination, const unsigned int* source, int offset) {
    struct Iterator {
        unsigned int current, begin, unknown, end;
    };
    int negOffset = -offset;
    Iterator local = *(const Iterator*)source;
    int count = ((int)local.end - (int)local.begin) / 4;
    int index = negOffset;
    index += ((int)local.current - (int)local.begin) / 4;
    if (index < 0)
        index += count;
    else if (index >= count)
        index -= count;
    local.current = ((volatile Iterator*)&local)->begin + index * 4;
    *(Iterator*)destination = local;
}
}
}

namespace ns4 {
extern "C" {
// Dereference the first pointer stored in the supplied buffer view.
void* RingBuffer32_GetFirstPointer(void* self) {
    return **(void***)self;
}
}
}

namespace ns4 {
extern "C" {
// Advance the current word pointer, wrapping from the end to the beginning.
void RingIterator32_AdvanceInPlace(void* self) {
    unsigned int end = ((RingWordStorage*)self)->capacity;
    unsigned int current = *(unsigned int*)self + 4;
    *(unsigned int*)self = current;
    if (current == end)
        *(unsigned int*)self = *(unsigned int*)((char*)self + 4);
}
}
}

namespace ns4 {
extern "C" {
// Placement-construct one unsigned word in the supplied output slot.
void RingBuffer32_ConstructWord(void* self, void* out, void* src) {
    new (out) unsigned int(*(unsigned int*)src);
}
}
}

#pragma use_lmw_stmw on
namespace ns4 {
extern "C" {
// Check the circular-buffer allocation limit and allocate the requested word storage.
void RingBuffer32_AllocateWords(void* self, void* unused, unsigned int n) {
    (void)unused;
    if (!s_ringBufferWordLimitInitialized) {
        s_ringBufferWordLimitInitialized = 1;
        s_ringBuffer32MaxWords = 0x3ffffffe;
    }
    if (n > s_ringBuffer32MaxWords) {
        char* source;
        source = golfRuleStrings + kText_cdeque_length_error;
        Fn801510E8Error local;
        Fn801510E8Error* localPtr = &local;
        localPtr->vtable = __vt__Q23std11logic_error;
        volatile char allocator = 0;
        unsigned int len = strlen(source) + 1;
        char* copy;
        copy = (char*)__nwa__FUl(len * 2 - len);
        localPtr->data = copy;
        {
            Fn801510E8Guard guard(copy, (char*)&allocator);
            Fn801510E8Buffer* buffer = (Fn801510E8Buffer*)__nw__FUl(0x10);
            if (buffer) {
                buffer->a = 1;
                buffer->b = 1;
                buffer->vtable = s_sharedCharDeleterVtable;
                buffer->data = copy;
            }
            localPtr->buffer = buffer;
            guard.data = 0;
        }
        strcpy(copy, source);
        localPtr->vtable = __vt__Q23std12length_error;
        __throw(golfRuleStrings + kText_lengthErrorTypeName, localPtr, (void*)__dt__Q23std12length_errorFv);
    }
    unsigned int* data = (unsigned int*)appMallocDummy__FUl((n + 1) * 4);
    *(unsigned int**)self = data;
    ((RingWordStorage*)self)->capacity = n + 1;
}
}
}

#pragma use_lmw_stmw reset
namespace ns4 {
extern "C" {
// Read the allocation word from the circular-buffer header.
void* RingBuffer32_GetAllocationWord(void* self) {
    return (void*)((RingWordStorage*)self)->allocation;
}
}
}

namespace ns4 {
extern "C" {
// Copy an iterator range into the tail, splitting the copy at a wrap boundary.
void RingBuffer32_CopyWrappedRangeToTail(Data512* self, const Iter512* first, const Iter512* last) {
    unsigned int* a;
    unsigned int* b = last->current;
    a = first->current;
    bool ordered = a <= b;
    unsigned int* destination = self->data + self->count;
    if (ordered) {
        unsigned int n = b - a;
        memmove(destination - n, a, n * 4);
        unsigned int offset = last->current - first->current;
        self->free += offset;
        self->count -= offset;
    } else {
        unsigned int* finish = last->begin;
        unsigned int n = b - finish;
        unsigned int* pos = destination - n;
        memmove(pos, finish, n * 4);
        unsigned int m = first->end - first->current;
        memmove(pos - m, first->current, m * 4);
        unsigned int count = (last->current - last->begin) + (first->end - first->current);
        self->count -= count;
        self->free += count;
    }
}
}
}

namespace ns4 {
extern "C" {
// Initialize an empty circular-buffer header with the supplied allocation word.
void RingBuffer32_InitializeHeader(void* self, unsigned int value) {
    ((RingWordStorage*)self)->allocation = value;
    ((RingWordStorage*)self)->data = 0;
    ((RingWordStorage*)self)->head = 0;
    ((RingWordStorage*)self)->count = 0;
    ((RingWordStorage*)self)->capacity = 0;
}
}
}

#pragma use_lmw_stmw on
namespace ns4 {
extern "C" {
// Append a possibly wrapped iterator range to the circular buffer.
void RingBuffer32_AppendIteratorRange(unsigned int* self, const unsigned int* first, const unsigned int* last) {
    unsigned int* l = (unsigned int*)last[0];
    unsigned int* f = (unsigned int*)first[0];
    unsigned int* pos = (unsigned int*)self[0] + ((self[1] + self[2]) % self[3]);
    if (f <= l) {
        memmove(pos, f, ((int*)l - (int*)f) * 4);
        self[2] += (int*)last[0] - (int*)first[0];
    } else {
        unsigned int n = ((int*)first[3] - (int*)first[0]) * 4;
        memmove(pos, (unsigned int*)first[0], n);
        memmove((char*)pos + n, (unsigned int*)last[1], ((int*)last[0] - (int*)last[1]) * 4);
        self[2] += ((int*)first[3] - (int*)first[0]) + ((int*)last[0] - (int*)last[1]);
    }
}
}
}

#pragma use_lmw_stmw reset
namespace ns4 {
extern "C" {
// Return the address of the circular-buffer capacity word.
void* RingBuffer32_GetCapacityAddress(void* self) {
    return &((RingWordStorage*)self)->capacity;
}
}
}

namespace ns4 {
extern "C" {
// Return the capacity-word address used by the allocation path.
void* RingBuffer32_GetCapacityAddressForAllocation(void* self) {
    return &((RingWordStorage*)self)->capacity;
}
}
}

namespace ns4 {
extern "C" {
// Read a word through the pointer-shaped interface used by the container helpers.
void* UnsignedWord_LoadPointerValue(void* self) {
    return *(void**)self;
}
}
}

namespace ns4 {
extern "C" {
// Lazily initialize and return the negative front-growth bias.
int ShotHistory_GetFrontGrowthBias(void* self) {
    if (!s_shotHistoryFrontGrowthBiasInitialized) {
        s_shotHistoryFrontGrowthBias = -0x20;
        s_shotHistoryFrontGrowthBiasInitialized = 1;
    }
    return s_shotHistoryFrontGrowthBias;
}
}
}

namespace ns4 {
extern "C" {
// Allocate storage for the requested number of 0x54-byte shot-history entries.
void* ShotHistory_AllocateEntries(void* self, int entryCount) {
    return appMallocDummy__FUl(entryCount * 0x54);
}
}
}

#pragma opt_common_subs off
#pragma opt_propagation off
#pragma use_lmw_stmw on
namespace ns4 {
extern "C" {
// Count words between the two temporary-map iterators; equal null cursors give 0
// in InitEmptyHeader's retained empty-source path.
inline int Fn801514E0Dist(const Fn801514E0Iter& a, const Fn801514E0Iter& b) {
    int n = b.current - a.current;
    if (b.current >= b.rangeBegin) {
        if (a.current < b.rangeBegin)
            n -= b.end - b.begin;
    } else {
        if (a.current >= b.rangeBegin)
            n += b.end - b.begin;
    }
    return n;
}
// Return the map's tail slot, treating zero capacity as empty without modulo 0.
inline unsigned int Fn801514E0Tail(const Fn801514E0Ring& a, unsigned int cap) {
    if (cap)
        return (a.index + a.size) % cap;
    else
        return 0;
}
// Advance a temporary iterator by at most one wrap, returning its allocation base.
inline unsigned int* Fn801514E0Advance(Fn801514E0Iter& a, int n) {
    int count = a.end - a.begin;
    n += a.current - a.begin;
    if (n < 0)
        n += count;
    else if (n >= count)
        n -= count;
    unsigned int* base = ((volatile Fn801514E0Iter*)&a)->begin;
    ((volatile Fn801514E0Iter*)&a)->current = base + n;
    return base;
}
// Copy a valid short wrapped source into contiguous nonoverlapping destination;
// advance f to last, retaining the volatile descriptor observations in each chunk.
inline void Fn801514E0Copy(const Fn801514E0Iter& last, Fn801514E0Iter& f, unsigned int* dst) {
    unsigned int* stop = last.current;
    unsigned int side = last.current >= last.rangeBegin;
    while (f.current != stop) {
        int n;
        unsigned int* current = ((volatile Fn801514E0Iter*)&f)->current;
        if (side == (current >= f.rangeBegin))
            n = stop - current;
        else
            n = f.end - current;
        current = ((volatile Fn801514E0Iter*)&f)->current;
        unsigned int* end = current + n;
        unsigned int bytes = ((int)((unsigned int)end - (unsigned int)current) / 4) * 4;
        memmove(dst, current, bytes);
        dst = (unsigned int*)((char*)dst + bytes);
        f.current += n - 1;
        f.current++;
        if (f.current == f.end)
            f.current = f.begin;
    }
}
// Copy a borrowed temporary iterator; no payload or ownership transfer.
inline void Fn801514E0Set(Fn801514E0Iter& out, const Fn801514E0Iter& x) {
    out = x;
}
struct Fn801514E0Frame {
    Fn801514E0Iter source, first, last, next, mid;
    Fn801514E0Iter lastSecond, nextSecond, fSecond;
    Fn801514E0Iter midFirst, midCopy, fFirst;
    Fn801514E0Error error;
    unsigned int padding;
    Fn801514E0Iter firstCopy, lastCopy;
    Fn801514E0Ring local;
};
// Initialize an empty shot-history header and its circular block map.
// Requires uninitialized storage for SIX words, not a populated container: this
// overwrites pointers without freeing previous storage. Returns the same header.
// Observable header result is [0,0,0,0,0,0]: frontBias/entryCount and empty map.
// This has NO input range and cannot linearize a populated history. local is a
// temporary empty map, initialized here; every constructed source/end cursor is
// null and the distance n is zero. Thus the n!=0 allocation/copy/length-error body
// and local.data cleanup do not run for this entry point. Descriptor stores and
// reloads are volatile, so source cleanup must preserve them even on that path.
// The generic nonempty branch remains visible below but is not a second API.
void* ShotHistory_InitEmptyHeader(void* historyHeader) {
    unsigned int* sourceEnd;
    unsigned int* sourceRangeBegin;
    unsigned int* sourceBegin;
    // Phase 1: seed the temporary empty source, then clear the actual six-word header.
    Fn801514E0Frame frame;
    Fn801514E0Ring& local = frame.local;
    volatile Fn801514E0Ring* init = &local;
    unsigned int initialCapacity = 0;
    init->capacity = initialCapacity;
    init->data = 0;
    init->index = 0;
    init->size = 0;
    unsigned int* p = (unsigned int*)historyHeader;
    Fn801514E0Ring* ring = (Fn801514E0Ring*)(p + 2);
    p[0] = 0;
    p[1] = 0;
    ring->data = 0;
    ring->index = 0;
    ring->size = 0;
    ring->capacity = 0;
    // Phase 2: materialize the empty source range; all iterator addresses are null.
    unsigned int index = Fn801514E0Tail(local, initialCapacity);
    unsigned int startIndex = local.index;
    unsigned int lastIndex = index << 2;
    unsigned int* start = local.data;
    unsigned int capacity = ((volatile Fn801514E0Ring*)&local)->capacity;
    Fn801514E0Iter& last = frame.last;
    Fn801514E0Iter& first = frame.first;
    Fn801514E0Iter& source = frame.source;
    volatile Fn801514E0Iter* out = &last;
    out->current = (unsigned int*)((char*)start + lastIndex);
    out->begin = start;
    out->rangeBegin = start + startIndex;
    out->end = start + capacity;
    out = &first;
    out->current = last.rangeBegin;
    out->begin = last.begin;
    out->rangeBegin = last.rangeBegin;
    out->end = last.end;
    out = &source;
    out->current = first.current;
    out->begin = first.begin;
    out->rangeBegin = first.rangeBegin;
    out->end = first.end;
    out = &frame.lastCopy;
    out->current = last.current;
    out->begin = last.begin;
    out->rangeBegin = last.rangeBegin;
    out->end = last.end;
    out = &frame.firstCopy;
    out->current = first.current;
    out->begin = first.begin;
    out->rangeBegin = first.rangeBegin;
    out->end = first.end;
    int n = Fn801514E0Dist(frame.firstCopy, frame.lastCopy);
    // Phase 3 (not taken): generic length check, allocation, then up to two copies.
    // The initialized empty source gives n == 0; these are not history init steps.
    if (n) {
        if (!s_uintVectorMaxSizeInitialized) {
            s_fourByteVectorMaxElements = 0x3ffffffe;
            s_uintVectorMaxSizeInitialized = 1;
        }
        if ((unsigned int)n > s_fourByteVectorMaxElements) {
            Fn801514E0Error& error = frame.error;
            unsigned char flag = 0;
            const char* message = golfRuleStrings + kText_cdeque_length_error;
            error.vt = __vt__Q23std11logic_error;
            unsigned int len = strlen(message) + 1;
            error.str = (char*)__nwa__FUl((len << 1) - len);
            {
                char* str = error.str;
                Fn801514E0Guard guard(str, &flag);
                struct Ref {
                    void* vt;
                    int a, b;
                    char* str;
                };
                Ref* ref = (Ref*)__nw__FUl(16);
                if (ref) {
                    ref->a = 1;
                    ref->b = 1;
                    ref->vt = s_sharedCharDeleterVtable;
                    ref->str = str;
                }
                error.ref = ref;
                guard.value = 0;
            }
            strcpy(error.str, message);
            error.vt = __vt__Q23std12length_error;
            __throw(golfRuleStrings + kText_lengthErrorTypeName, &error, (void*)__dt__Q23std12length_errorFv);
        }
        ring->data = (unsigned int*)appMallocDummy__FUl((n + 1) * 4);
        ring->capacity = n + 1;
        if (ring->data) {
            unsigned int* dst = ring->data + (ring->index + ring->size) % ring->capacity;
            int remaining = Fn801514E0Dist(source, last);
            int available = ring->data + ring->capacity - dst;
            const int* chosen = remaining < available ? &remaining : &available;
            available = *chosen;
            sourceBegin = source.begin;
            sourceEnd = source.end;
            unsigned int* sourceCurrent = source.current;
            sourceRangeBegin = source.rangeBegin;
            ring->size += remaining;
            int* amount = &available;
            Fn801514E0Iter& mid = frame.mid;
            volatile Fn801514E0Iter* m = &mid;
            m->current = sourceCurrent;
            m->begin = sourceBegin;
            m->rangeBegin = sourceRangeBegin;
            m->end = sourceEnd;
            unsigned int* midBase = Fn801514E0Advance(mid, *amount);
            Fn801514E0Set(frame.midFirst, mid);
            frame.midFirst.begin = midBase;
            Fn801514E0Set(frame.midCopy, frame.midFirst);
            Fn801514E0Set(frame.fFirst, first);
            Fn801514E0Copy(frame.midCopy, frame.fFirst, dst);
            remaining -= *amount;
            if (remaining) {
                Fn801514E0Iter& next = frame.next;
                volatile Fn801514E0Iter* q = &next;
                q->current = source.current;
                q->begin = sourceBegin;
                q->rangeBegin = sourceRangeBegin;
                q->end = sourceEnd;
                unsigned int* nextBase = Fn801514E0Advance(next, *amount);
                Fn801514E0Set(frame.nextSecond, next);
                frame.nextSecond.begin = nextBase;
                Fn801514E0Set(frame.lastSecond, last);
                Fn801514E0Set(frame.fSecond, frame.nextSecond);
                Fn801514E0Copy(frame.lastSecond, frame.fSecond, ring->data);
            }
        }
    }
    // Phase 4: release temporary map if owned; this entry initialized it to null.
    if (local.data) {
        unsigned int size = *(volatile unsigned int*)&local.size;
        *(volatile unsigned int*)&local.size = size - size;
        appFreeUni__FPv(local.data);
    }
    return historyHeader;
}
}
}

#pragma opt_common_subs reset
#pragma opt_propagation reset
#pragma use_lmw_stmw reset
namespace ns4 {
extern "C" {
// Read the OUTER header's +10 mapCount, not its +14 allocation capacity. Subtract
// the reserved slot and convert the remaining block slots to 16-entry capacity.
unsigned int ShotHistory_GetBlockCapacity(void* self) {
    unsigned int value = ((RingWordStorage*)self)->allocation;
    return value == 0 ? 0 : (value - 1) << 4;
}
}
}

namespace ns4 {
extern "C" {
// Destroy the owned container storage and optionally free the object.
void* RingBuffer32_Release(char* self, int flags) {
    if (self != 0) {
        void* ptr = *(void**)self;
        if (ptr != 0) {
            register int value = *(volatile int*)(self + 8);
            *(int*)(self + 8) = value - value;
            appFreeUni__FPv(ptr);
        }
        if (flags > 0)
            __dl__FPv(self);
    }
    return self;
}
}
}

// Register the golf-rule actor factory and its type descriptor.
void CGolfRule::RegisterType() {
    using namespace ns4;
    s_twoPi_8014B210_805E2A00 = kFloat_6_2831855;
    FloatTween_Init(((char*)s_golfRuleInputDestructorRecord + kTurnTween), kFloat_0_GolfRule);
    IntPair_Set(&s_golfRuleRttiDescriptor, golfRuleStrings + kText_cGolfRule, &s_taskMainTypeInfo);
    void* rule = ActorFactoryRegistry_GetInstance();
    StringMap_SetByName(rule, (void*)CGolfRule_Create, s_golfRuleActorFactoryName);
    TaskRegistry_AppendFactory(TaskRegistry_GetInstance(), s_golfRuleTaskRegistryName);
    Fn80151B80List* list = (Fn80151B80List*)(((char*)s_golfRuleInputDestructorRecord + kPreloadList));
    list->__ct();
    __register_global_object(list, (void*)dtor_8014B5E0,
                             ((char*)s_golfRuleInputDestructorRecord + kPreloadListDestructorRecord));
}

// Adjust the secondary-interface this pointer and forward to the owning actor method.
void* CGolfRule::DtorThunk18(int flags) {
    using namespace ns4;
    return __dt__9CGolfRuleFv((char*)this - 0x18, flags);
}

// ===== S5 - Round flow, ranking and hole results =====
// Player snapshots, hole records and rank bits.

// Choose the task transition appropriate to the current game mode.
// Uses shared gameMode and task/audio/input services; no arguments or return value.
// Clears transition-sensitive input/audio/effects before requesting Party, Training
// or Fight. Party/Fight also append their named follow-up command.
void CGolfTask::RequestModeTransition() {
    using namespace ns4;
    GolfTransition_ClearInputAudioAndEffects(g_project);
    unsigned int mode = AsCSharedDoc(g_sharedDoc)->gameMode;
    if (mode == 7) {
        ((CTask*)g_taskManager)
            ->RequestTransitionWithFade(golfRuleStrings + kText_cPartyTask, golfRuleStrings + kText_emptyTaskArgument,
                                        1);
        TaskCommandQueue_AppendNamedRequest(g_taskManager, 0, golfRuleStrings + kText_partyMain, 1, 0, 0, 0, 0);
    } else if (mode == 5) {
        ((CTask*)g_taskManager)
            ->RequestTransitionWithFade(golfRuleStrings + kText_cTrainingTask,
                                        golfRuleStrings + kText_emptyTaskArgument, 1);
    } else {
        ((CTask*)g_taskManager)
            ->RequestTransitionWithFade(golfRuleStrings + kText_cFightTask, golfRuleStrings + kText_emptyTaskArgument,
                                        1);
        TaskCommandQueue_AppendNamedRequest(g_taskManager, 0, golfRuleStrings + kText_fightMain, 1, 0, 0, 0, 0);
    }
}

// Apply a post-round menu choice, including replay, next hole and leaving the mode.
// Requires rule/task/shared services; state selects save/return/restart behavior.
// Notifies the player actor first. Restart rebuilds player snapshots and resets the
// hole to 0; exit cases queue the indicated task/save command. No error return.
void CGolfRule::ApplyPostRoundChoice(int state) {
    using namespace ns4;
    IActor_DispatchMessage(this, this, golfRuleStrings + kText_player, 0x2C, 0, 0, 0, 0);
    switch (state) {
    case 2:
        GolfTransition_ClearInputAudioAndEffects(g_project);
        SaveProcess_SetTransitionTargetName(golfRuleStrings + kText_cSingleTaskRev);
        SaveProcess_SetFollowUpTaskName(golfRuleStrings + kText_singleMainRev);
        ((CTask*)g_taskManager)
            ->RequestTransitionWithFade(golfRuleStrings + kText_cSaveProcessTask,
                                        golfRuleStrings + kText_emptyTaskArgument, 1);
        unsigned char flag = AsCSharedDoc(g_sharedDoc)->golfRuleType;
        TaskCommandQueue_AppendNamedRequest(g_taskManager, 0, golfRuleStrings + kText_saveProcessMain, flag == 1, 0, 0,
                                            0, 0);
        break;
    case 1:
        GolfTransition_ClearInputAudioAndEffects(g_project);
        if (AsCSharedDoc(g_sharedDoc)->gameMode == 7) {
            ((CTask*)g_taskManager)
                ->RequestTransitionWithFade(golfRuleStrings + kText_cPartyTask,
                                            golfRuleStrings + kText_emptyTaskArgument, 1);
            TaskCommandQueue_AppendNamedRequest(g_taskManager, 0, golfRuleStrings + kText_partyMain, 0, 0, 0, 0, 0);
        } else {
            ((CTask*)g_taskManager)
                ->RequestTransitionWithFade(golfRuleStrings + kText_cFightTask,
                                            golfRuleStrings + kText_emptyTaskArgument, 1);
            TaskCommandQueue_AppendNamedRequest(g_taskManager, 0, golfRuleStrings + kText_fightMain, 0, 0, 0, 0, 0);
        }
        break;
    case 3:
        GolfTransition_ClearInputAudioAndEffects(g_project);
        {
            unsigned int mode = AsCSharedDoc(g_sharedDoc)->gameMode;
            if (mode == 7) {
                ((CTask*)g_taskManager)
                    ->RequestTransitionWithFade(golfRuleStrings + kText_cPartyTask,
                                                golfRuleStrings + kText_emptyTaskArgument, 1);
                TaskCommandQueue_AppendNamedRequest(g_taskManager, 0, golfRuleStrings + kText_partyMain, 1, 0, 0, 0, 0);
            } else if (mode == 5) {
                ((CTask*)g_taskManager)
                    ->RequestTransitionWithFade(golfRuleStrings + kText_cTrainingTask,
                                                golfRuleStrings + kText_emptyTaskArgument, 1);
            } else {
                ((CTask*)g_taskManager)
                    ->RequestTransitionWithFade(golfRuleStrings + kText_cFightTask,
                                                golfRuleStrings + kText_emptyTaskArgument, 1);
                TaskCommandQueue_AppendNamedRequest(g_taskManager, 0, golfRuleStrings + kText_fightMain, 1, 0, 0, 0, 0);
            }
        }
        break;
    case 4: {
        void* registered = Sing_805DC608();
        roundRestarted = 1;
        if (!registered) {
            if (!s_fadeOverlayInitialized) {
                FadeOverlay_Init(s_fadeOverlay);
                __register_global_object(s_fadeOverlay, (void*)ResourceState_Dtor, s_fadeOverlayRegistration_804F72D8);
                s_fadeOverlayInitialized = 1;
            }
            Sing_805DC608() = s_fadeOverlay;
        }
        FadeState_StartFadeIn(Sing_805DC608(), 0);
        AsCSharedDoc(g_sharedDoc)->courseRecordUpdatesEnabled = 0;
        ((CGolfDoc*)((*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc)))->ClearHoleResetFlags();
        DriveLineMarkers_Generate((char*)(*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc) + 0x36C,
                                  (unsigned char)SharedDoc_GetCourseId());
        SetupPlayerSnapshots(0);
        for (int i = 0;
             i < (unsigned char)((CGolfDoc*)((*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc)))->GetActivePlayerCount();
             ++i) {
            IActor_DispatchMessage(this, this, golfRuleStrings + kText_player, 0x25, i, 0, 0, 0);
            IActor_DispatchMessage(this, this, golfRuleStrings + kText_caddie, 0x17, i, 0, 0, 0);
        }
        ((CGolfDoc*)((*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc)))->SetCurrentHole(0);
        IActor_DispatchMessage(this, this, golfRuleStrings + kText_player, 0x2D, 0, 0, 0, 0);
        IActor_DispatchMessage(this, this, golfRuleStrings + kText_golfBg, 3, 0, 0, 0, 0);
    } break;
    case 5:
    default:
        CGolfRule::RequestModeExitTransition();
    }
}

#pragma use_lmw_stmw on
// Recompute per-player ranks and the next hole order under the active golf rule.
// Uses golfRuleType and partyGameMode to rank players: Match compares hole strokes,
// Stroke compares score then Pang, Balloon Pop compares its result then Pang;
// Darts/Driving Contest delegate by message. Rank nibbles and tie order are retained.
// Refreshes score actor before publishing next-hole order; a nonzero task status
// returns before that second phase. Match updates the rule's turn from hole strokes.
// playerCount must fit order[4]. Match requires players 0/1; Vs CPU compares slot
// 0 against eight rival-table rows, skipping the selected character when found.
// Match awards a hole once, guarded by player 0's byte at +14B+hole (holeAwardFlags);
// tied holes do not set that guard. Ordinary Stroke skips state-3 players in
// both loops; Match and Balloon loops have no corresponding exclusion test.
// R20_SCORE/R20_PANG retain signed word loads. Each comma assignment refreshes
// 'other' BEFORE that comparison; short-circuiting retains the original sequence
// of GolfPlayer_GetEntryByIndex calls. Do not cache across these observations.
// The nibble and hole-row schema/policy live beside R20_SCORE in P2.
void CGolfRule::UpdatePlayerRanksAndHoleOrder() {
    using namespace ns4;
    // Phase 1: update hole results/ranks under the active golf rule.
    unsigned char* other;
    switch (AsCSharedDoc(SharedDocBytes())->golfRuleType) {
    case 1:
        if (AsCGolfDoc(GolfDocBytes())->currentHoleIndex < 18U) {
            unsigned int hole1 = AsCGolfDoc(GolfDocBytes())->currentHoleIndex;
            other = GolfPlayerEntryByte(1);

            unsigned char order[4];
            unsigned int hole0 = AsCGolfDoc(GolfDocBytes())->currentHoleIndex;
            if ((GolfPlayerEntryByte(0) + hole0)[kPlayerHoleStrokeCounts] < (other + hole1)[kPlayerHoleStrokeCounts]) {
                int hole = AsCGolfDoc(GolfDocBytes())->currentHoleIndex;
                if ((GolfPlayerEntryByte(0) + hole)[kMatchHoleAwardFlags] == 0) {
                    unsigned char* p = GolfPlayerEntryByte(0);
                    ++R20_SCORE(p);
                }
                hole = AsCGolfDoc(GolfDocBytes())->currentHoleIndex;
                unsigned char* marked = GolfPlayerEntryByte(0) + hole;
                marked[kMatchHoleAwardFlags] = 1;
            } else {
                unsigned int hole1 = AsCGolfDoc(GolfDocBytes())->currentHoleIndex;
                other = GolfPlayerEntryByte(1);
                unsigned int hole0 = AsCGolfDoc(GolfDocBytes())->currentHoleIndex;
                if ((GolfPlayerEntryByte(0) + hole0)[kPlayerHoleStrokeCounts] >
                    (other + hole1)[kPlayerHoleStrokeCounts]) {
                    unsigned int hole = AsCGolfDoc(GolfDocBytes())->currentHoleIndex;
                    if ((GolfPlayerEntryByte(0) + hole)[kMatchHoleAwardFlags] == 0) {
                        unsigned char* p = GolfPlayerEntryByte(1);
                        ++R20_SCORE(p);
                    }
                    hole = AsCGolfDoc(GolfDocBytes())->currentHoleIndex;
                    unsigned char* marked = GolfPlayerEntryByte(0) + hole;
                    marked[kMatchHoleAwardFlags] = 1;
                }
            }

            unsigned char* key = order;
            int j;
            for (j = 0; j < AsCGolfDoc(GolfDocBytes())->playerCount; ++key, ++j) {
                *key = ((GolfDocSnapshot8013*)(GolfPlayerEntryByte(j)))->rankTieBreakOrder;
                if (((GolfDocSnapshot8013*)(GolfPlayerEntryByte(j)))->unk_4 != 0)
                    *key += 4;
            }
            unsigned char* current = order;
            for (int i = 0; i < AsCGolfDoc(GolfDocBytes())->playerCount; ++current, ++i) {
                R20_B(GolfPlayerEntryByte(i)).high = 1;
                R20_B(GolfPlayerEntryByte(i)).low = 1;
                key = order;
                for (j = 0; j < AsCGolfDoc(GolfDocBytes())->playerCount; ++key, ++j) {
                    if (j != i) {
                        // Match: more wins, then more Pang, beats this player.
                        if ((other = GolfPlayerEntryByte(j), R20_SCORE(GolfPlayerEntryByte(i)) < R20_SCORE(other)) ||
                            ((other = GolfPlayerEntryByte(j), R20_SCORE(other) == R20_SCORE(GolfPlayerEntryByte(i))) &&
                             (other = GolfPlayerEntryByte(j), R20_PANG(GolfPlayerEntryByte(i)) < R20_PANG(other)))) {
                            ++R20_B(GolfPlayerEntryByte(i)).high;
                            ++R20_B(GolfPlayerEntryByte(i)).low;
                        }
                        // Exact score/Pang tie: earlier adjusted key wins high only.
                        if ((other = GolfPlayerEntryByte(j), R20_SCORE(other) == R20_SCORE(GolfPlayerEntryByte(i))) &&
                            (other = GolfPlayerEntryByte(j), R20_PANG(other) == R20_PANG(GolfPlayerEntryByte(i))) &&
                            *current > *key)
                            ++R20_B(GolfPlayerEntryByte(i)).high;
                    }
                }
            }
        }
        break;
    case 0:
        if (*(int*)&((CSharedDoc*)(SharedDocBytes()))->gameMode == 0) {
            R20_B(GolfPlayerEntryByte(0)).high = 1;
            R20_B(GolfPlayerEntryByte(0)).low = 1;
            void* table = AsCRival(g_rival)->strokeUse;
            int own = ((int (*)(void*, int))ItemInventory_FindCharacterIndex)(SharedDocBytes() + 0xA04,
                                                                              R20_I(SharedDocBytes(), 0x120));
            for (int i = 0; i < 8; ++i) {
                if (i != own) {
                    int score = RivalStrokeUse_GetStrokesThroughHole(
                        table, i, (((1 + AsCGolfDoc(GolfDocBytes())->currentHoleIndex))));
                    int tie =
                        RivalStrokeUse_GetPangThroughHole(table, i, 1 + AsCGolfDoc(GolfDocBytes())->currentHoleIndex);
                    if (((GolfDocSnapshot8013*)(GolfPlayerEntryByte(0)))->totalStrokeCount > score ||
                        (score == ((GolfDocSnapshot8013*)(GolfPlayerEntryByte(0)))->totalStrokeCount &&
                         R20_PANG(GolfPlayerEntryByte(0)) < tie)) {
                        ++R20_B(GolfPlayerEntryByte(0)).high;
                        ++R20_B(GolfPlayerEntryByte(0)).low;
                    }
                }
            }
        } else if (*(int*)&((CSharedDoc*)(SharedDocBytes()))->partyGameMode == 3) {
            for (int i = 0; i < AsCGolfDoc(GolfDocBytes())->playerCount; ++i) {
                R20_B(GolfPlayerEntryByte(i)).high = 1;
                R20_B(GolfPlayerEntryByte(i)).low = 1;
                for (int j = 0; AsCGolfDoc(GolfDocBytes())->playerCount > j; ++j) {
                    if (j != i) {
                        // Balloon result descends; Pang and tie order decide equality.
                        if ((other = GolfPlayerEntryByte(j),
                             R20_BALLOON_RESULT(GolfPlayerEntryByte(i)) < R20_BALLOON_RESULT(other))) {
                            ++R20_B(GolfPlayerEntryByte(i)).high;
                            ++R20_B(GolfPlayerEntryByte(i)).low;
                        } else if ((other = GolfPlayerEntryByte(j),
                                    R20_BALLOON_RESULT(other) == R20_BALLOON_RESULT(GolfPlayerEntryByte(i)))) {
                            if ((other = GolfPlayerEntryByte(j), R20_PANG(GolfPlayerEntryByte(i)) < R20_PANG(other))) {
                                ++R20_B(GolfPlayerEntryByte(i)).high;
                                ++R20_B(GolfPlayerEntryByte(i)).low;
                            } else if ((other = GolfPlayerEntryByte(j),
                                        R20_PANG(other) == R20_PANG(GolfPlayerEntryByte(i)))) {
                                if ((other = GolfPlayerEntryByte(j),
                                     ((GolfDocSnapshot8013*)(GolfPlayerEntryByte(i)))->rankTieBreakOrder >
                                         ((GolfDocSnapshot8013*)(other))->rankTieBreakOrder)) {
                                    ++R20_B(GolfPlayerEntryByte(i)).high;
                                }
                            }
                        }
                    }
                }
            }
        } else if (*(int*)&((CSharedDoc*)(SharedDocBytes()))->partyGameMode == 1) {
            IActor_DispatchMessage(this, this, golfRuleStrings + kText_dartsSystem, 2, 0, 0, 0, 0);
        } else if (*(int*)&((CSharedDoc*)(SharedDocBytes()))->partyGameMode == 2) {
            IActor_DispatchMessage(this, this, golfRuleStrings + kText_driveSystem, 1, 0, 0, 0, 0);
        } else {
            unsigned char order[4];
            unsigned char* key = order;
            int j;
            for (j = 0; j < AsCGolfDoc(GolfDocBytes())->playerCount; ++key, ++j) {
                *key = ((GolfDocSnapshot8013*)(GolfPlayerEntryByte(j)))->rankTieBreakOrder;
                if (((GolfDocSnapshot8013*)(GolfPlayerEntryByte(j)))->unk_4 != 0)
                    *key += 4;
            }
            unsigned char* current = order;
            for (int i = 0; i < AsCGolfDoc(GolfDocBytes())->playerCount; ++current, ++i) {
                if (((GolfDocSnapshot8013*)(GolfPlayerEntryByte(i)))->playerState != 3) {
                    R20_B(GolfPlayerEntryByte(i)).high = 1;
                    R20_B(GolfPlayerEntryByte(i)).low = 1;
                    key = order;
                    for (j = 0; j < AsCGolfDoc(GolfDocBytes())->playerCount; ++key, ++j) {
                        if (i != j) {
                            if (((GolfDocSnapshot8013*)(GolfPlayerEntryByte(j)))->playerState != 3) {
                                // Stroke: lower round score, then more Pang, beats i.
                                if ((other = GolfPlayerEntryByte(j),
                                     R20_SCORE(GolfPlayerEntryByte(i)) > R20_SCORE(other)) ||
                                    ((other = GolfPlayerEntryByte(j),
                                      R20_SCORE(other) == R20_SCORE(GolfPlayerEntryByte(i))) &&
                                     (other = GolfPlayerEntryByte(j),
                                      R20_PANG(GolfPlayerEntryByte(i)) < R20_PANG(other)))) {
                                    ++R20_B(GolfPlayerEntryByte(i)).high;
                                    ++R20_B(GolfPlayerEntryByte(i)).low;
                                }
                                if ((other = GolfPlayerEntryByte(j),
                                     R20_SCORE(other) == R20_SCORE(GolfPlayerEntryByte(i))) &&
                                    (other = GolfPlayerEntryByte(j),
                                     R20_PANG(other) == R20_PANG(GolfPlayerEntryByte(i))) &&
                                    *current > *key)
                                    ++R20_B(GolfPlayerEntryByte(i)).high;
                            }
                        }
                    }
                }
            }
        }
        break;
    }
    // Phase 2: refresh score presentation before choosing the next hole/player order.
    IActor_DispatchMessage(this, this, golfRuleStrings + kText_score, 3, 0, 0, 0, 0);
    if (GolfTask_NoOpStatus() != 0)
        return;
    switch (AsCSharedDoc(SharedDocBytes())->golfRuleType) {
    case 0:
        if (*(int*)&((CSharedDoc*)(SharedDocBytes()))->gameMode == 0) {
            unsigned int rank = R20_B(GolfPlayerEntryByte(0)).high;
            R20_HOLE_ORDER(0) = rank;
        } else if (*(int*)&((CSharedDoc*)(SharedDocBytes()))->partyGameMode != 0) {
            for (int i = 0; i < AsCGolfDoc(GolfDocBytes())->playerCount; ++i) {
                unsigned int rank = R20_B(GolfPlayerEntryByte(i)).high;
                R20_HOLE_ORDER(i) = rank;
            }
        } else {
            for (int i = 0; i < AsCGolfDoc(GolfDocBytes())->playerCount; ++i) {
                if (((GolfDocSnapshot8013*)(GolfPlayerEntryByte(i)))->playerState == 3) {
                    R20_HOLE_ORDER(i) = AsCGolfDoc(GolfDocBytes())->playerCount;
                } else {
                    R20_HOLE_ORDER(i) = 1;
                    for (int j = 0; j < AsCGolfDoc(GolfDocBytes())->playerCount; ++j) {
                        if (i != j) {
                            if (((GolfDocSnapshot8013*)(GolfPlayerEntryByte(j)))->playerState != 3) {
                                int hole1 = AsCGolfDoc(GolfDocBytes())->currentHoleIndex;
                                other = GolfPlayerEntryByte(j);
                                unsigned int hole0 = AsCGolfDoc(GolfDocBytes())->currentHoleIndex;
                                // 0x801520C4: compare strokes, then the tie-breaking turn order.
                                if ((GolfPlayerEntryByte(i) + hole0)[kPlayerHoleStrokeCounts] >
                                        (other + hole1)[kPlayerHoleStrokeCounts] ||
                                    ((hole1 = AsCGolfDoc(GolfDocBytes())->currentHoleIndex,
                                      other = GolfPlayerEntryByte(j),
                                      hole0 = AsCGolfDoc(GolfDocBytes())->currentHoleIndex,
                                      (GolfPlayerEntryByte(i) + hole0)[kPlayerHoleStrokeCounts] ==
                                          (hole1 + other)[kPlayerHoleStrokeCounts]) &&
                                     (other = GolfPlayerEntryByte(j),
                                      ((GolfDocSnapshot8013*)(GolfPlayerEntryByte(i)))->turnOrder >
                                          ((GolfDocSnapshot8013*)(other))->turnOrder))) {
                                    ++R20_HOLE_ORDER(i);
                                }
                            }
                        }
                    }
                }
            }
        }
        if (*(int*)&((CSharedDoc*)(SharedDocBytes()))->partyGameMode == 3) {
            for (int i = 0; i < AsCGolfDoc(GolfDocBytes())->playerCount; ++i) {
                unsigned char* record = *(unsigned char**)&((CGolfDoc*)(GolfDocBytes()))->holeRecords +
                                        (AsCGolfDoc(GolfDocBytes())->currentHoleIndex << 5);
                int count = AsCGolfDoc(GolfDocBytes())->playerCount;
                unsigned char* p = GolfPlayerEntryByte(i);
                ((GolfDocSnapshot8013*)(p))->turnOrder = Math_Abs32((record + i)[kHolePlayerOrder] - count);
            }
        } else {
            for (int i = 0; i < AsCGolfDoc(GolfDocBytes())->playerCount; ++i) {
                unsigned char* record = *(unsigned char**)&((CGolfDoc*)(GolfDocBytes()))->holeRecords +
                                        (AsCGolfDoc(GolfDocBytes())->currentHoleIndex << 5);
                unsigned char* p = GolfPlayerEntryByte(i);
                ((GolfDocSnapshot8013*)(p))->turnOrder = (i + record)[kHolePlayerOrder] - 1;
            }
        }
        break;
    case 1: {
        unsigned int hole1 = AsCGolfDoc(GolfDocBytes())->currentHoleIndex;
        other = GolfPlayerEntryByte(1 - turn);
        unsigned int hole0 = AsCGolfDoc(GolfDocBytes())->currentHoleIndex;
        if ((GolfPlayerEntryByte(turn) + hole0)[kPlayerHoleStrokeCounts] > (other + hole1)[kPlayerHoleStrokeCounts])
            turn = 1 - turn;
        break;
    }
    }
}

#pragma use_lmw_stmw reset
// Check the current hole and player progress and update ranks when required.
// Queries current-hole player states without advancing state itself. Stroke Play
// waits for active/unstarted players, updates ranks when none remain, then returns 1.
// Match uses its separate finished/stroke comparisons; unsupported rules return 0.
// Requires initialized player records; receiver-call ABI and byte indices are retained.
int CGolfRule::IsCurrentHoleComplete() {
    using namespace ns4;
    typedef void* (*GetFn)(void*, unsigned char);
    switch (((unsigned char*)g_sharedDoc)[0x8C0]) {
    case 0: {
        int i;
        int found = 0;
        for (i = 0; i < AsCGolfDoc(GolfDocBytes())->playerCount; ++i) {
            // 0x80152EF4: an active or unstarted player keeps this state pending.
            unsigned char index;
            if (*(int*)((char*)((GetFn)GolfPlayer_GetEntryByIndex)(GolfDocBytes(), i) + 8) == 0 ||
                (*(int*)((char*)((GetFn)GolfPlayer_GetEntryByIndex)(GolfDocBytes(), i) + 8) == 2 &&
                 (index = AsCGolfDoc(GolfDocBytes())->currentHoleIndex,
                  *((unsigned char*)((GetFn)GolfPlayer_GetEntryByIndex)(GolfDocBytes(), i) + index + 0x28) == 0))) {
                found = 1;
                break;
            }
        }
        if (found)
            break;
        UpdatePlayerRanksAndHoleOrder();
        return 1;
    }
    case 1: {
        int first = *(int*)((char*)((GetFn)GolfPlayer_GetEntryByIndex)(GolfDocBytes(), 0) + 8) == 1;
        int second = *(int*)((char*)((GetFn)GolfPlayer_GetEntryByIndex)(GolfDocBytes(), 1) + 8) == 1;
        if (first)
            return 1;
        unsigned char index = AsCGolfDoc(GolfDocBytes())->currentHoleIndex;
        unsigned char left = *((unsigned char*)((GetFn)GolfPlayer_GetEntryByIndex)(GolfDocBytes(), 0) + index + 0x28);
        index = AsCGolfDoc(GolfDocBytes())->currentHoleIndex;
        unsigned char right = *((unsigned char*)((GetFn)GolfPlayer_GetEntryByIndex)(GolfDocBytes(), 1) + index + 0x28);
        if (first) {
            if (left <= right)
                return 1;
        } else if (second) {
            if (left >= right)
                return 1;
        }
        return 0;
    }
    }
    return 0;
}

#pragma use_lmw_stmw on
// Player status 3 is skipped; +0x10 and +0x1C both receive the same WVector.
// Requires position to address 12 world-position bytes and a constructed rule.
// Copies position into both stored positions of every player except state 3;
// clears per-shot bytes and consumes all four histories, freeing spare tail blocks.
// Publishes the changed-player count in hole byte 19, clears completion bytes,
// then sends ball message 2. No failure result; the player's +0x10 field stays unnamed.
void CGolfRule::ResetPlayerPositionsAndShotHistory(const char* position) {
    using namespace ns4;
    unsigned char* object = (unsigned char*)this;
    unsigned int changed = 0;
    int zero;
    unsigned int i;
    for (i = 0; (unsigned char)i < AsCGolfDoc(GolfDocBytes())->playerCount; ++i) {
        if (*(int*)(P530 + 8) == 3)
            continue;
        ((GolfDocSnapshot8013*)(P530))->afterPuttPoseFlags = 0;
        ((GolfDocSnapshot8013*)(P530))->unk_BC = 0;
        *(WVector*)(P530 + 0x10) = *(WVector*)position;
        *(WVector*)(P530 + 0x1C) = *(WVector*)position;
        ((GolfDocSnapshot8013*)(P530))->timeoutsUsed = 0;
        ((GolfDocSnapshot8013*)(P530))->unk_148 = 0;
        ((GolfDocSnapshot8013*)(P530))->unk_149 = 0;
        ((GolfDocSnapshot8013*)(P530))->unk_AE = 0;
        ++changed;
    }
    struct Control530 {
        char pad[0x2BC];
        unsigned char* data;
        char pad2[0xF6];
        unsigned char index;
    };
    Control530* control = (Control530*)GolfDocBytes();
    (control->index * 32 + control->data)[0x19] = changed;
    zero = 0;
    ((GolfRuleFrameFields*)(object))->stateEntryPending = zero;
    ((RuleUnresolvedShotBytes*)(object))->unk_091 = zero;
    ((RuleUnresolvedShotBytes*)(object))->unk_092 = zero;
    ((CGolfRule*)(object))->shotUndoEnabled = zero;
    ((RuleUnresolvedShotBytes*)(object))->unk_094 = zero;
    for (unsigned int j = 0; j < 4; ++j) {
        unsigned char* entry = object + (unsigned char)j * 0x18;
        if (*(int*)(entry + 0xB0) == 0)
            continue;
        while (*(int*)(entry + 0xB0)) {
            int remain = --*(int*)(entry + 0xB0);
            unsigned int pos = *(unsigned int*)(entry + 0xAC) + remain;
            int bc = *(int*)(entry + 0xBC);
            unsigned int capacity;
            switch (bc) {
            case 0:
                capacity = 0;
                break;
            default:
                capacity = (bc - 1) << 4;
                break;
            }
            i = capacity - pos;
            while (i >= 32) {
                unsigned int remaining = *(unsigned int*)(entry + 0xBC) - 1;
                *(unsigned int*)(entry + 0xBC) = remaining;
                unsigned int q = (remaining + *(unsigned int*)(entry + 0xB8) - 1) % *(unsigned int*)(entry + 0xC0);
                void** list = *(void***)(entry + 0xB4);
                appFreeUni__FPv(list[q]);
                list[q] = (void*)zero;
                i -= 16;
            }
        }
    }
    IActor_DispatchMessage(this, this, golfRuleStrings + kText_ball, 2, 0, 0, 0, 0);
}

#pragma use_lmw_stmw reset
namespace ns4 {
extern "C" {
struct Fn801532E0Flags {
    CW_BITS2(unsigned char high : 4, unsigned char low : 4)
};
// Hole-results continuation values, not GolfRuleState bit flags.
enum PlayerResultPhase {
    kPlayerResultRequest = 0,
    kPlayerResultWait = 1,
    kPlayerResultAdvance = 2
};
enum HoleResultNextAction {
    kHoleResultNoAction = 0,
    kHoleResultReload = 1,
    kHoleResultRoundResults = 2
};
// Score actor message 0x11 writes a synchronous int reply to argument 1.
// Its interpretation depends on the caller branch: ordinary play accepts ANY
// nonzero reply. Training with record updates enabled recognizes 1/3/4; with
// updates disabled, only 1/2 have special work, but ANY nonzero still queues reload.
enum TrainingScoreReply {
    kTrainingReplayHole = 1,
    kTrainingNextHole = 2,
    kTrainingReturnToSelection = 3,
    kTrainingLeaveMode = 4
};
// Named raw lvalues keep the existing char-pointer/signed-word ABI access shape.
#define RESULT_SEQUENCE_INDEX(p) (*(int*)((p) + 0x4A0))
#define RESULT_SEQUENCE_PHASE(p) (*(int*)((p) + 0x4A4))
#define RESULT_SEQUENCE_ACTIVE(p) (*(int*)((p) + 0x49C))
#define RESULT_NEXT_ACTION(p) (*(int*)((p) + 0x4B8))
}
}

// Process per-player hole results, course-record updates and next-round choices.
// Requires rule and loaded documents; returns while elapsedTime is below 3 seconds.
// Polls score/player readiness, updates permitted course records and drives fades,
// per-player result loading, next-hole reload or result/menu/task transitions.
// Readiness replies are synchronous stack words; unmet gates leave state pending.
// Priority/transition map (one branch per call):
//   sequence active -> Request(0): on player 0 start fade/loading indicator;
//     player 25(index),26(index,1), then Wait(1).
//   Wait -> player 27(&ready,index); zero stays Wait, nonzero sends 28 -> Advance(2).
//   Advance -> ++index, Request; after last active player clear active/index,
//     enter PostRoundMenus, release fade state, fade out, voice 9, request loading
//     script. The rule borrows services; this routine does not destroy them.
//   nextAction pending -> fade in, reset FOV, Reload(1) sends golfBg 3,
//     RoundResults(2) enters that state. This branch does NOT clear nextAction.
//   neither -> optional course-record writes, THEN poll score 11. A zero score
//     reply therefore does not roll back record writes or task-active flags.
//     With record updates: mode6/party0 nonwinner queues RoundResults on nonzero;
//       Training 1 disables records/saves snapshot/queues Reload, 3 goes to training
//       selection, 4 requests mode exit; otherwise nonzero starts player sequence.
//     Without record updates: nonzero advances hole when permitted, queues Reload;
//       Training 1 saves snapshot, 2 increments hole; others still queue Reload.
//   A queued nextAction requests the pending fade at the end of this call; its
//   action executes on a LATER call via the higher-priority branch above.
// Unrecognized sequence phases/pending actions perform their branch's common
// effects but no switch action. No failure status or transaction/rollback exists.
void CGolfRule::ProcessHoleResults(float dt) {
    using namespace ns4;
    char* obj = (char*)this;
    // Phase 1: wait for the result delay and publish task activity.
    if (((GolfRuleFrameFields*)(obj))->elapsedTime < kFloat_3_0)
        return;
    void* message = Object_GetChild(g_taskManager);
    ((GolfTaskActivityFlags*)message)->holeResultActive = 1;
    // Phase 2: handle the result/transition latch before normal hole-result routing.
    if (((CGolfRule*)(obj))->playerResultSequenceActive != 0) {
        message = Object_GetChild(g_taskManager);
        ((GolfTaskActivityFlags*)message)->holeResultActive = 0;
        IActor_DispatchMessage(this, this, golfRuleStrings + kText_player, 0x2b, 1, 0, 0, 0);
        switch (((CGolfRule*)(obj))->playerResultSequencePhase) {
        case kPlayerResultRequest:
            if (((CGolfRule*)(obj))->playerResultSequenceIndex == 0) {
                if (!Sing_805DC608()) {
                    if (!s_fadeOverlayInitialized) {
                        FadeOverlay_Init(s_fadeOverlay);
                        __register_global_object(s_fadeOverlay, (void*)ResourceState_Dtor,
                                                 s_fadeOverlayRegistration_804F72D8);
                        s_fadeOverlayInitialized = 1;
                    }
                    Sing_805DC608() = s_fadeOverlay;
                }
                FadeState_StartFadeIn(Sing_805DC608(), 0);
                if (!Sing_805DC610()) {
                    if (!s_nowReloadingInitialized) {
                        ((CNowReloading*)(&s_nowReloading))->Init();
                        __register_global_object(&s_nowReloading, (void*)__dt__13CNowReloadingFv,
                                                 s_nowReloadingDtorRegistration_8014B210);
                        s_nowReloadingInitialized = 1;
                    }
                    Sing_805DC610() = &s_nowReloading;
                }
                void* singleton = Sing_805DC610();
                if (singleton != 0 && LoadingIndicator_IsAbsentOrState3(singleton) != 0)
                    LoadingIndicator_CreatePlayer(singleton);
            }
            IActor_DispatchMessage(this, this, golfRuleStrings + kText_player, 0x25, RESULT_SEQUENCE_INDEX(obj), 0, 0,
                                   0);
            IActor_DispatchMessage(this, this, golfRuleStrings + kText_player, 0x26, RESULT_SEQUENCE_INDEX(obj), 1, 0,
                                   0);
            RESULT_SEQUENCE_PHASE(obj) = kPlayerResultWait;
            break;
        case kPlayerResultWait: {
            int playerReady = 0;
            IActor_DispatchMessage(this, this, golfRuleStrings + kText_player, 0x27, (int)&playerReady,
                                   ((CGolfRule*)(obj))->playerResultSequenceIndex, 0, 0);
            if (playerReady == 0)
                break;
            IActor_DispatchMessage(this, this, golfRuleStrings + kText_player, 0x28, 0, 0, 0, 0);
            RESULT_SEQUENCE_PHASE(obj) = kPlayerResultAdvance;
            break;
        }
        case kPlayerResultAdvance: {
            int completedPlayer = RESULT_SEQUENCE_INDEX(obj);
            RESULT_SEQUENCE_PHASE(obj) = kPlayerResultRequest;
            RESULT_SEQUENCE_INDEX(obj) = completedPlayer + 1;
            unsigned char limit =
                (unsigned char)((CGolfDoc*)((*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc)))->GetActivePlayerCount();
            if ((int)limit > RESULT_SEQUENCE_INDEX(obj))
                break;
            RESULT_SEQUENCE_ACTIVE(obj) = 0;
            RESULT_SEQUENCE_INDEX(obj) = 0;
            AsCGolfRule(obj)->postRoundMenuState = 0;
            ((CGolfRule*)this)->EnterState(kStatePostRoundMenus);
            IActor_DispatchMessage(this, this, golfRuleStrings + kText_player, 0x2b, 0, 0, 0, 0);
            FadeState_RequestReleaseOnFadeCompletion(FadeState_GetInstance());
            if (!Sing_805DC608()) {
                if (!s_fadeOverlayInitialized) {
                    FadeOverlay_Init(s_fadeOverlay);
                    __register_global_object(s_fadeOverlay, (void*)ResourceState_Dtor,
                                             s_fadeOverlayRegistration_804F72D8);
                    s_fadeOverlayInitialized = 1;
                }
                Sing_805DC608() = s_fadeOverlay;
            }
            FadeState_StartFadeOut(Sing_805DC608(), 0x1e);
            IActor_DispatchMessage(this, this, golfRuleStrings + kText_voice, 9, 0, 0, 0, 0);
            if (!Sing_805DC610()) {
                if (!s_nowReloadingInitialized) {
                    ((CNowReloading*)(&s_nowReloading))->Init();
                    __register_global_object(&s_nowReloading, (void*)__dt__13CNowReloadingFv,
                                             s_nowReloadingDtorRegistration_8014B210);
                    s_nowReloadingInitialized = 1;
                }
                Sing_805DC610() = &s_nowReloading;
            }
            void* singleton = Sing_805DC610();
            if (singleton != 0 && LoadingIndicator_IsAbsentOrState3(singleton) != 0)
                CB2dPlayerHandle::RequestPendingScript(singleton);
            break;
        }
        }
    } else if (((CGolfRule*)(obj))->nextRoundAction != 0) {
        message = Object_GetChild(g_taskManager);
        ((GolfTaskActivityFlags*)message)->holeResultActive = 0;
        void* registered = Sing_805DC608();
        if (!registered) {
            if (!s_fadeOverlayInitialized) {
                FadeOverlay_Init(s_fadeOverlay);
                __register_global_object(s_fadeOverlay, (void*)ResourceState_Dtor, s_fadeOverlayRegistration_804F72D8);
                s_fadeOverlayInitialized = 1;
            }
            Sing_805DC608() = s_fadeOverlay;
        }
        FadeState_StartFadeIn(Sing_805DC608(), 0);
        Camera_SetFov(g_view, kGolfCameraFovRadians);
        switch (RESULT_NEXT_ACTION(obj)) {
        case kHoleResultReload:
            IActor_DispatchMessage(this, this, golfRuleStrings + kText_golfBg, 3, 0, 0, 0, 0);
            break;
        case kHoleResultRoundResults:
            ((CGolfRule*)this)->EnterState(kStateRoundResults);
            break;
        }
    } else {
        if (AsCSharedDoc(g_sharedDoc)->courseRecordUpdatesEnabled != 0) {
            int index = ((CGolfDoc*)((*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc)))->SelectPlayerBySnapshotWords();
            int slot;
            if (index < 0)
                slot = 0;
            else if (index > 3)
                slot = 3;
            else
                slot = index;
            unsigned int recBase = (unsigned int)g_sharedDoc + slot * 0x1d4;
            unsigned char* rec = (unsigned char*)(recBase + 0x10);
            if ((*(int*)&AsCSharedDoc(g_sharedDoc)->gameMode) == 0 && AsCSharedDoc(g_sharedDoc)->golfRuleType == 0) {
                int i1 = SharedDoc_GetCourseId();
                short old = *(short*)(rec + ((unsigned char)i1 << 3) + 0xa4);
                int i2 = SharedDoc_GetCourseId();
                int value = *(int*)(rec + ((unsigned char)i2 << 3) + 0xa0);
                void* first = GolfPlayer_GetEntryByIndex((int)(*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc),
                                                         (unsigned char)slot);
                void* second = GolfPlayer_GetEntryByIndex((int)(*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc),
                                                          (unsigned char)slot);
                int sum = (*(int*)&AsGolfDocSnapshot(second)->roundResultWords[kRoundPangWord]) +
                          (*(int*)&AsGolfDocSnapshot(first)->roundResultWords[kRoundBonusPangWord]);
                void* third = GolfPlayer_GetEntryByIndex((int)(*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc),
                                                         (unsigned char)slot);
                if (old > (*(int*)&AsGolfDocSnapshot(third)->roundResultWords[kRoundScoreWord])) {
                    void* fourth = GolfPlayer_GetEntryByIndex((int)(*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc),
                                                              (unsigned char)slot);
                    slot = (short)(*(int*)&AsGolfDocSnapshot(fourth)->roundResultWords[kRoundScoreWord]);
                    int i4 = SharedDoc_GetCourseId();
                    *(short*)(rec + ((unsigned char)i4 << 3) + 0xa4) = slot;
                    slot = ItemInventory_FindCharacterIndex(((char*)&AsCSharedDoc(g_sharedDoc)->itemInventory),
                                                            *(int*)(rec + 0x110));
                    int i5 = SharedDoc_GetCourseId();
                    *(signed char*)(rec + ((unsigned char)i5 << 3) + 0xa6) = (signed char)slot;
                }
                if (value < sum) {
                    int i6 = SharedDoc_GetCourseId();
                    *(int*)(rec + ((unsigned char)i6 << 3) + 0xa0) = sum;
                }
            }
            // 0x801532E0: Finish the selected player-update path before the shared continuation.
            do {
                if (SharedDoc_IsMode6PartyMode0() != 0) {
                    void* flags = GolfPlayer_GetEntryByIndex((int)(*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc), 0);
                    if (((Fn801532E0Flags*)(&AsGolfDocSnapshot(flags)->rankFlags))->high != 1) {
                        int scoreReply = 0;
                        IActor_DispatchMessage(this, this, golfRuleStrings + kText_score, 0x11, (int)&scoreReply, 0, 0,
                                               0);
                        if (scoreReply != 0)
                            RESULT_NEXT_ACTION(obj) = kHoleResultRoundResults;
                        break;
                    }
                }
                if (AsCSharedDoc(g_sharedDoc)->gameMode == 5) {
                    int scoreReply = 0;
                    IActor_DispatchMessage(this, this, golfRuleStrings + kText_score, 0x11, (int)&scoreReply, 0, 0, 0);
                    switch (scoreReply) {
                    case kTrainingReplayHole:
                        AsCSharedDoc(g_sharedDoc)->courseRecordUpdatesEnabled = 0;
                        ((CGolfDoc*)((*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc)))->SaveFirstPlayerSnapshot();
                        RESULT_NEXT_ACTION(obj) = kHoleResultReload;
                        break;
                    case kTrainingReturnToSelection:
                        GolfTransition_ClearInputAudioAndEffects(g_project);
                        ((CTask*)g_taskManager)
                            ->RequestTransitionWithFade(golfRuleStrings + kText_cTrainingTask,
                                                        golfRuleStrings + kText_emptyTaskArgument, 1);
                        break;
                    case kTrainingLeaveMode:
                        CGolfRule::RequestModeExitTransition(0);
                        break;
                    }
                } else {
                    int scoreReply = 0;
                    IActor_DispatchMessage(this, this, golfRuleStrings + kText_score, 0x11, (int)&scoreReply, 0, 0, 0);
                    if (scoreReply != 0) {
                        void* registered = Sing_805DC608();
                        RESULT_SEQUENCE_ACTIVE(obj) = 1;
                        RESULT_SEQUENCE_INDEX(obj) = 0;
                        RESULT_SEQUENCE_PHASE(obj) = kPlayerResultRequest;
                        if (!registered) {
                            if (!s_fadeOverlayInitialized) {
                                FadeOverlay_Init(s_fadeOverlay);
                                __register_global_object(s_fadeOverlay, (void*)ResourceState_Dtor,
                                                         s_fadeOverlayRegistration_804F72D8);
                                s_fadeOverlayInitialized = 1;
                            }
                            Sing_805DC608() = s_fadeOverlay;
                        }
                        FadeState_RequestFramebufferCapture(Sing_805DC608());
                    }
                }
            } while (0);
            ;
        } else {
            int scoreReply = 0;
            IActor_DispatchMessage(this, this, golfRuleStrings + kText_score, 0x11, (int)&scoreReply, 0, 0, 0);
            if (scoreReply != 0) {
                if (AsCSharedDoc(g_sharedDoc)->gameMode == 5) {
                    switch (scoreReply) {
                    case kTrainingReplayHole:
                        ((CGolfDoc*)((*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc)))->SaveFirstPlayerSnapshot();
                        break;
                    case kTrainingNextHole: {
                        void* player = (*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc);
                        ((CGolfDoc*)player)->SetCurrentHole((unsigned char)(AsCGolfDoc(player)->currentHoleIndex + 1));
                        break;
                    }
                    }
                } else {
                    void* player = (*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc);
                    unsigned char last = AsCSharedDoc(g_sharedDoc)->roundHoleCount;
                    unsigned char cur = AsCGolfDoc(player)->currentHoleIndex;
                    if (cur != (unsigned char)(last - 1))
                        ((CGolfDoc*)player)->SetCurrentHole((unsigned char)(cur + 1));
                }
                RESULT_NEXT_ACTION(obj) = kHoleResultReload;
            }
        }
        if (RESULT_NEXT_ACTION(obj) != kHoleResultNoAction) {
            if (!Sing_805DC608()) {
                if (!s_fadeOverlayInitialized) {
                    FadeOverlay_Init(s_fadeOverlay);
                    __register_global_object(s_fadeOverlay, (void*)ResourceState_Dtor,
                                             s_fadeOverlayRegistration_804F72D8);
                    s_fadeOverlayInitialized = 1;
                }
                Sing_805DC608() = s_fadeOverlay;
            }
            FadeState_RequestFramebufferCapture(Sing_805DC608());
        }
    }
}

// Wait for the camera delay, then enter the player-specific continuation state.
// Requires rule/documents; dt is seconds. Updates the camera until elapsedTime
// reaches stateTimer, then sends player presentation status. Pause/sync gates defer.
// Tutorial returns through its messages; ordinary flow chooses hole results or
// actor wait from IsCurrentHoleComplete. No failure result.
void CGolfRule::ProcessDelayedStateTransition(float dt) {
    using namespace ns4;
    if (AsGolfRuleFrame(this)->elapsedTime < AsGolfRuleFrame(this)->stateTimer) {
        ((CCameraWork*)g_cameraWork)->UpdateMotionAndTarget(dt, kFloat_0_GolfRule);
        return;
    }
    unsigned char* control = (*(unsigned char**)&AsCSharedDoc(g_sharedDoc)->golfDoc);
    int kind = ((CGolfDoc*)(control))->currentPlayer;
    if (AsCSharedDoc(g_sharedDoc)->golfRuleType == 1)
        kind = ((CGolfDoc*)control)->ResolveMatchTurnFromHoleStrokes();
    IActor_DispatchMessage(this, this, golfRuleStrings + kText_player, 0x32, 1, kind, 0, 0);
    if (((unsigned char*)this)[0x3B1] || ((unsigned char*)this)[0x3B2])
        return;
    control = (*(unsigned char**)&AsCSharedDoc(g_sharedDoc)->golfDoc);
    if (AsCGolfDoc(control)->tutorialStage < 6) {
        IActor_DispatchMessage(this, this, golfRuleStrings, 0xD, 1, 0, 0, 0);
        IActor_DispatchMessage(this, this, golfRuleStrings + kText_tutorial, 0, 0, 0, 0, 0);
        return;
    }
    if (IsCurrentHoleComplete()) {
        if (!GolfTask_NoOpStatus())
            ((CGolfRule*)this)->EnterState(kStateHoleResults);
        return;
    }
    IActor_DispatchMessage(this, this, golfRuleStrings + kText_score, 1, 0, 0, 0, 0);
    ((CGolfRule*)this)->EnterState(kStateWaitPlayerAndCaddie);
}

// ===== S6 - Player preview and shot animations =====
// Rule timers, club runtime and document state.

// Finish the player preview, initialize aim and gauge state, then enable turn input.
// Requires rule and player/camera services; dt drives the preview update.
// Waits for preview/party readiness and pause/sync gates, then rebuilds turn setup,
// stops shot audio, enters Aim and updates tutorial messaging. No error return.
void CGolfRule::ProcessPlayerPreview(float dt) {
    using namespace ns4;
    unsigned char* obj = (unsigned char*)this;
    ((CCameraWork*)g_cameraWork)->UpdateMotionAndTarget(dt, kFloat_0_GolfRule);
    unsigned int active = 0;
    if (AsCCameraWork(g_cameraWork)->duration > 0.0f &&
        AsCCameraWork(g_cameraWork)->animationTime > AsCCameraWork(g_cameraWork)->duration)
        active = 1;
    if (active == 0)
        return;
    if (GolfTask_NoOpStatus()) {
        unsigned char* player = (*(unsigned char**)&AsCSharedDoc(g_sharedDoc)->golfDoc);
        if (!player[0x35E]) {
            player[0x35E] = 1;
            obj[0x3B2] = 1;
            *(const char**)(obj + 0x3B8) = golfRuleStrings + kText_doPlayerPreview;
            *(float*)(obj + 0x3C8) = 20.0f;
            AsCGolfRule(obj)->netMsgWaitLine = 0xEB8;
            AsCGolfRule(obj)->netMsgSendTime = OS_GetTimeLow();
        }
    }
    if (((CGolfRule*)(obj))->processingPaused || ((CGolfRule*)(obj))->networkSyncPending)
        return;
    unsigned char* state = (unsigned char*)g_sharedDoc;
    if ((*(int*)&AsCSharedDoc(state)->partyGameMode) == 1) {
        IActor_DispatchMessage(this, this, golfRuleStrings + kText_dartsSystem, 5, (int)(obj + 0x4F4), 0, 0, 0);
        if (((CGolfRule*)(obj))->miniGameSystemResult)
            return;
    }
    if ((*(int*)&AsCSharedDoc(state)->partyGameMode) == 2) {
        if (AsCSharedDoc(state)->gameMode == 6) {
            IActor_DispatchMessage(this, this, golfRuleStrings + kText_driveSystem, 2, (int)(obj + 0x4F4), 0, 0, 0);
            if (((CGolfRule*)(obj))->miniGameSystemResult)
                return;
        }
    }
    PreparePlayerTurn();
    ((CMouseCursor*)g_mouseCursor)->Reset(0);
    IActor_DispatchMessage(this, this, golfRuleStrings + kText_clock, 0, 0, 0, 0, 0);
    ((CGolfRule*)this)->ResetShotGauge();
    ((CGolfDoc*)((*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc)))->UpdateCamera(1);
    {
        int h = NamedEffectList_FindByName(g_sceneManager, golfRuleStrings + kText_ballSequence);
        if (h)
            WPuppet_SetOpacityRecursive(h, 0xFF);
    }
    Sound_StopByAlias((void*)g_soundManager, golfRuleStrings + kText_shot_dist_count);
    Sound_PlayByName((void*)g_soundManager, golfRuleStrings + kText_play_my_turn, 0, 0, 0, 0, kFloat_0_5_GolfRule,
                     kFloat_200_GolfRule, kFloat_1_805DEA38);
    ((CCameraWork*)g_cameraWork)->StopBallSound();
    ((CGolfRule*)this)->EnterState(kStateAim);
    state = (unsigned char*)g_sharedDoc;
    unsigned char* player = (*(unsigned char**)&AsCSharedDoc(state)->golfDoc);
    if (AsCGolfDoc(player)->currentHoleIndex == (unsigned char)(AsCSharedDoc(state)->roundHoleCount - 1))
        IActor_DispatchMessage(this, this, golfRuleStrings + kText_score, 9, 0, 0, 0, 0);
}

// Play the committed button-shot animation and advance to ball flight after its delay.
// Requires rule and club/camera services; delta is seconds. Pause/sync can defer.
// Initializes animation/camera once; eligible wood double-power presentation chooses
// a sequence using rand()%4. The elapsed-time fallback enters BallFlight after 11.
// Preserves the separate animation-ready/double-power admission paths.
void CGolfRule::ProcessButtonShotAnimation(float delta) {
    using namespace ns4;
    unsigned char* obj = (unsigned char*)this;
    if (AsCCameraWork(g_cameraWork)->action >= 0xC8 ||
        (*(int*)((char*)InstanceTable_Get() + 0x44) == 2 && ((GolfRuleFrameFields*)(obj))->animationStarted)) {
        ((CCameraWork*)g_cameraWork)->UpdateMotionAndTarget(delta, kFloat_0_GolfRule);
    }
    if (((CGolfRule*)(obj))->processingPaused || ((CGolfRule*)(obj))->networkSyncPending)
        return;
    if (!((GolfRuleFrameFields*)(obj))->animationStarted) {
        if (!(*(unsigned int*)&((CGolfRule*)(obj))->shotPresentationFlags & 2)) {
            if (!GetInputManagerSlot()) {
                if (!s_inputManagerInitialized) {
                    InputManager_Init(s_gameInputManager);
                    __register_global_object(s_gameInputManager, (void*)InputManager_Dtor,
                                             s_golfRuleInputDestructorRecord);
                    s_inputManagerInitialized = 1;
                }
                GetInputManagerSlot() = s_gameInputManager;
            }
            if (!Input_IsActionButtonDown(GetInputManagerSlot(), kActionSwing, kSelectedInputChannel)) {
                unsigned char* state = (unsigned char*)g_cameraWork;
                if (state[0x1A0]) {
                    state[0x1A0] = 0;
                    AsCCameraWork(state)->inputMoved = 0;
                    AsCCameraWork(state)->inputFovOffset = kFloat_0_GolfRule;
                    Vec2_Zero(state + 0x14);
                }
                ((unsigned char*)g_cameraWork)[0x241] = 0;
            }
        }
        if (*(float*)(obj + 0x34) > kFloat_1_805DEA38) {
            obj[0x74] = 1;
            IActor_DispatchMessage(this, this, golfRuleStrings + kText_player, 0, 0, 0, 0, 0);
            void* mode = InstanceTable_Get();
            if (((ClubSelectionRuntime*)mode)->powerShotMode == kDoublePowerShotMode) {
                mode = InstanceTable_Get();
                unsigned char* info = *(unsigned char**)((char*)mode + 4);
                if (!((SelectedClubRuntime*)(info))->clubKind) {
                    unsigned char* scene = (*(unsigned char**)&AsCSharedDoc(g_sharedDoc)->golfDoc);
                    int phase = AsCGolfDoc(scene)->impactGrade;
                    if (phase != 1 && phase != 0) {
                        void* message = g_taskManager ? Object_GetChild(g_taskManager) : 0;
                        int slot[4];
                        int values[8];
                        values[0] = 0;
                        values[1] = 0x23;
                        values[2] = (int)slot;
                        values[3] = 0;
                        values[4] = 0;
                        values[5] = 0;
                        values[6] = 0;
                        values[7] = 0;
                        void* text = TaskActorTable_FindByName(message, golfRuleStrings + kText_player);
                        if (text) {
                            ((VText*)text)->v09(values);
                        }
                        char* name =
                            String_FormatTemporary(golfRuleStrings + kText_woodPowerShotSequenceFormat, rand() % 4 + 1);
                        ((GolfRuleFrameFields*)(obj))->stateTimer =
                            ((CCameraWork*)g_cameraWork)->StartGroupAnimationAtPivot(2, name, slot);
                        ((GolfRuleFrameFields*)(obj))->doublePowerCameraReset = 0;
                    }
                }
            }
            IActor_DispatchMessage(this, this, golfRuleStrings + kText_voice, 2, 0, 0, 0, 0);
        }
    } else if (((GolfRuleFrameFields*)(obj))->elapsedTime > kFloat_11_GolfRule) {
        ((CGolfRule*)this)->EnterState(kStateBallFlight);
    }
}

// Play the committed motion-shot animation and advance to ball flight after its delay.
// Requires rule and club/camera services; delta is seconds. Pause/sync can defer.
// Initializes animation/camera once; eligible wood double-power presentation chooses
// a sequence using rand()%4. The elapsed-time fallback enters BallFlight after 11.
// This is the motion-swing animation copy; its original call forms remain separate.
void CGolfRule::ProcessMotionShotAnimation(float delta) {
    using namespace ns4;
    unsigned char* obj = (unsigned char*)this;
    // 0x8015426C: Join after the optional camera update.
    do {
        if (AsCCameraWork(g_cameraWork)->action < 0xC8) {
            void* mode = InstanceTable_Get();
            if (((ClubSelectionRuntime*)mode)->powerShotMode != kDoublePowerShotMode ||
                !((GolfRuleFrameFields*)(obj))->animationStarted)
                break;
        }
        ((CCameraWork*)g_cameraWork)->UpdateMotionAndTarget(delta, kFloat_0_GolfRule);
    } while (0);

    if (((CGolfRule*)(obj))->processingPaused || ((CGolfRule*)(obj))->networkSyncPending)
        return;
    if (0 == ((GolfRuleFrameFields*)(obj))->animationStarted) {
        if (!(*(unsigned int*)&((CGolfRule*)(obj))->shotPresentationFlags & 0x2)) {
            if (!GetInputManagerSlot()) {
                if (!s_inputManagerInitialized) {
                    InputManager_Init(s_gameInputManager);
                    __register_global_object(s_gameInputManager, (void*)InputManager_Dtor,
                                             s_golfRuleInputDestructorRecord);
                    s_inputManagerInitialized = 1;
                }
                GetInputManagerSlot() = s_gameInputManager;
            }
            if (!Input_IsActionButtonDown(GetInputManagerSlot(), 11, -1)) {
                unsigned char* state = (unsigned char*)g_cameraWork;
                if (state[0x1A0]) {
                    state[0x1A0] = 0;
                    float zero = kFloat_0_GolfRule;
                    AsCCameraWork(state)->inputMoved = 0;
                    AsCCameraWork(state)->inputFovOffset = zero;
                    Vec2_Zero(state + 0x14);
                }
                ((unsigned char*)g_cameraWork)[0x241] = 0;
            }
        }
        if (*(float*)(obj + 0x34) > kFloat_1_805DEA38) {
            obj[0x74] = 1;
            IActor_DispatchMessage(this, this, golfRuleStrings + kText_player, 0, 0, 0, 0, 0);
            void* mode = InstanceTable_Get();
            if (((ClubSelectionRuntime*)mode)->powerShotMode == kDoublePowerShotMode) {
                mode = InstanceTable_Get();
                unsigned char* info = *(unsigned char**)((char*)mode + 4);
                if (!((SelectedClubRuntime*)(info))->clubKind) {
                    unsigned char* scene = (*(unsigned char**)&AsCSharedDoc(g_sharedDoc)->golfDoc);
                    int phase = AsCGolfDoc(scene)->impactGrade;
                    if (phase != 1) {
                        if (phase != 0) {
                            void* message = g_taskManager ? Object_GetChild(g_taskManager) : 0;
                            int out[3];
                            int values[8];
                            values[0] = 0;
                            values[1] = 0x23;
                            values[2] = (int)out;
                            values[3] = 0;
                            values[4] = 0;
                            values[5] = 0;
                            values[6] = 0;
                            values[7] = 0;
                            void* text = TaskActorTable_FindByName(message, golfRuleStrings + kText_player);
                            struct GolfMessageReceiver {
                                virtual void v0() DECOMP_LOCAL_VIRTUAL;
                                virtual void v1() DECOMP_LOCAL_VIRTUAL;
                                virtual void v2() DECOMP_LOCAL_VIRTUAL;
                                virtual void v3() DECOMP_LOCAL_VIRTUAL;
                                virtual void v4() DECOMP_LOCAL_VIRTUAL;
                                virtual void v5() DECOMP_LOCAL_VIRTUAL;
                                virtual void v6() DECOMP_LOCAL_VIRTUAL;
                                virtual void v7() DECOMP_LOCAL_VIRTUAL;
                                virtual void v8() DECOMP_LOCAL_VIRTUAL;
                                virtual void Handle(int*) DECOMP_LOCAL_VIRTUAL;
                            };
                            if (text)
                                ((GolfMessageReceiver*)text)->Handle(values);

                            char* name = String_FormatTemporary(golfRuleStrings + kText_woodPowerShotSequenceFormat,
                                                                (rand()) % 4 + 1);
                            ((GolfRuleFrameFields*)(obj))->stateTimer =
                                ((CCameraWork*)g_cameraWork)->StartGroupAnimationAtPivot(2, name, out);
                            ((GolfRuleFrameFields*)(obj))->doublePowerCameraReset = 0;
                        }
                    }
                }
            }
            IActor_DispatchMessage(this, this, golfRuleStrings + kText_voice, 2, 0, 0, 0, 0);
        }
    } else if (((GolfRuleFrameFields*)(obj))->elapsedTime > kFloat_11_GolfRule) {
        ((CGolfRule*)this)->EnterState(kStateBallFlight);
    }
}

namespace ns4 {
extern "C" {
static inline int event545HasType() {
    void* object = g_taskManager ? Object_GetChild(g_taskManager) : 0;
    g_currentTaskChild_GolfRule = object;
    if (object) {
        Event545Type* type = ((Event545Type*)object)->type();
        while (type) {
            if (type == (Event545Type*)s_golfTaskTypeInfo)
                return 1;
            type = type->next;
        }
        return 0;
    }
    return 0;
}
// ===== S7 - Rule message receiver =====
// Receiver-local payload words and synchronous replies.
}
}

// actors are separate protocols even when their numeric values coincide.
// Requires self and a readable message record; unknown IDs do nothing. Replies
// write directly through arg0.bytes; pointers/records must have the sizes below.
// Table: id | input words (a0/a1/a2) | reply bytes written through a0 (0 = none).
// 02/03 | 12-byte position pointer, set/get rule result point | 0/12.
// 05 | begin swing or return from camera | 0. 06 | swing-enabled word | 0.
// 07 | state bit | 0. 08 | recompute heading and begin turn | 0.
// 09 | player index; finish network sync/start hole resources | 0.
// 0A | exclude selected player (state 3), recount/rerank | 0.
// 0B | zero clears doc byte35D | 0. 0C | auto-shot enable, integer X/Y | 0.
// 0D | pause word | 0. 0E | named position index, optional target index | 0.
// 0F | output Vec2* for gauge UI position | 8. 10 | output power float* | 4.
// 11 | required world-position pointer, reset shot | 0.
// 12 | exit request word (tutorial stage also gates) | 0.
// 13 | player index, toggle CPU flag | 0. 14 | deferred-button bits | 0.
// 15 | capture current player's eight inventory words | 0.
// 16 | resume state-dependent BGM (putting chooses rand()%2) | 0.
// 17 | reset ball/history to tee plus ball radius | 0. 18 | enable putt animation | 0.
// 19 | copy-inventory word for undo | 0. 1A | reset hole ball/wind | 0.
// 1B | shot-block enable (only word 1 enables) | 0.
// 1D | output presentation-flags int* | 4. 1E | motion-controller v3C | 0.
// 1F | amount as integer percent, controller argument | 0.
// 20/21/22 | motion-controller v4C(1)/v50(1)/v54() | 0.
// 23 | tutorial feedback word | 0. 24 | restore sample 0/tutorial aim | 0.
// 25 | set tutorial advance pending | 0. 26 | output controller-result int* | 4.
// 27/2D | output presentation-phase predicate int* | 4.
// 28 | draw initialized result/retry menus | 0. 29 | pending dialog v03 | 0.
// 2A/2B | output dialog-present/no-dialog predicate int* | 4.
// 2C | output undo-admission predicate int* | 4.
// 2E | input float* converted to checkpoint power-meter cost int | 0.
// 2F | output item-slot-found byte* (also records slot) | 1.
// 2719 | rule v02 call | 0. No case 1C is handled here.
// Effects and state transitions are synchronous call sites below; no error return.
void CGolfRule::HandleMsg(const unsigned char* msg) {
    using namespace ns4;
    register unsigned char* temporary;
    register Event545Vector* input = (Event545Vector*)GolfBall__Fv();
    register int index, offset;

    int ready;
    float zero, angle, resetAngle;
    switch ((((const GolfRuleMessage*)msg)->id)) {
    case 0x1A:
        ((CGolfRule*)this)->ResetHoleBallAndWind();
        break;
    case 0x18:
        AsCGolfRule(this)->puttAnimationEnabled = 1;
        break;
    case 0x2719:
        ((VText*)this)->v02();
        break;
    case 0x17: {
        ((GolfRuleFrameFields*)(this))->stateTimer = kFloat_0_GolfRule;
        float target[3], sum[3], position[3], offset[3];
        ((CGolfDoc*)(GolfDocBytes()))->unk_35E = 0;
        offset[1] = AsCGolfBall(GolfBall__Fv())->m_radius;
        offset[0] = kFloat_0_GolfRule;
        offset[2] = kFloat_0_GolfRule;
        unsigned char* g = GolfDocBytes();
        unsigned char* records = *(unsigned char**)&((CGolfDoc*)(g))->holeRecords;
        int slot = (32 * ((CGolfDoc*)(g))->currentHoleIndex);
        unsigned char* p = records + slot;
        int first = *(int*)((((slot + records))));
        struct Arr3 {
            int v[3];
        };
        *(Arr3*)position = *(Arr3*)(records + slot);
        __pl__FRC7WVectorRC7WVector(sum, position, offset);
        E545_I(target, 0) = E545_I(sum, 0);
        E545_I(target, 4) = E545_I(sum, 4);
        E545_I(target, 8) = E545_I(sum, 8);
        ((CGolfBall*)GolfBall__Fv())->Reset(target);
        temporary = (unsigned char*)GolfBall__Fv();
        temporary = (unsigned char*)((CQuadTree*)CQuadTree_GetInstance())->GetMeshIndex(temporary, 0);
        E545_P(GolfBall__Fv(), 0x28) = temporary;
        ((CGolfRule*)this)->ResetPlayerPositionsAndShotHistory((const char*)GolfBall__Fv());
        break;
    }
    case 0x16: {
        unsigned char* g = GolfDocBytes();
        unsigned int flags = *(unsigned int*)&((CGolfDoc*)(g))->gameStateFlags;
        if (flags == 0x1000) {
            int difference;
            if (AsCSharedDoc(SharedDocBytes())->golfRuleType != 1) {
                unsigned char* p = GolfPlayerEntry(((CGolfDoc*)(g))->currentPlayer);
                g = GolfDocBytes();
                int hole = ((CGolfDoc*)(g))->currentHoleIndex;
                difference = (p + hole)[0x28] - (*(unsigned char**)&((CGolfDoc*)(g))->holeRecords + (hole * 32))[0x18];
            } else {
                unsigned char* p = GolfPlayerEntry(((CGolfDoc*)(g))->currentPlayer);
                g = GolfDocBytes();
                int hole = ((CGolfDoc*)(g))->currentHoleIndex;
                difference = ((hole + p))[0x28] - (*(unsigned char**)&((CGolfDoc*)(g))->holeRecords + 32 * hole)[0x18];
            }
            if (difference <= 0)
                SoundManager_RequestBgmChange(g_soundManager, golfRuleStrings + kText_cupin_underpar_wav, 1, 1);
            else
                SoundManager_RequestBgmChange(g_soundManager, golfRuleStrings + kText_cupin_overpar_wav, 1, 1);
        } else if (flags == 0x2000) {
            SoundManager_RequestBgmChange(g_soundManager, golfRuleStrings + kText_scoreboard_wav, 0, 1);
        } else if (flags == 0x20000) {
            SoundManager_RequestBgmChange(g_soundManager, golfRuleStrings + kText_scoreboard_wav, 1, 1);
        } else if (((unsigned char*)GolfBall__Fv())[0x59] == 2) {
            SoundManager_RequestBgmChange(g_soundManager, (const char*)puttingBgmFileNames[rand() % 2], 0, 1);
            FloatTween_Start(s_actorRenderScaleTween, -1, kFloat_0_7_GolfRule, kFloat_1_805DEA38);
        } else {
            unsigned int n = AsCGolfRule(this)->selectedBgmIndex;
            if (*(unsigned int*)&((CGolfRule*)(this))->bgmNameCount > n) {
                unsigned char* entry = *(unsigned char**)&((CGolfRule*)(this))->bgmNames + (n * 12);
                const char* text;
                text = !(E545_U(entry, 0) >> 31) ? (const char*)entry + 1 : (const char*)E545_P(entry, 8);
                SoundManager_RequestBgmChange(g_soundManager, text, 0, 1);
            }
            FloatTween_Start(s_actorRenderScaleTween, -1, kFloat_1_805DEA38, kFloat_1_805DEA38);
        }
        E545_SEND(kText_voice, 6, 0, 0);
        break;
    }
    case 0x10: {
        unsigned char* g = GolfDocBytes();
        if (((CGolfDoc*)(g))->impactGrade > 2)
            *(float*)(((const GolfRuleMessage*)msg)->arg0.bytes) =
                (((CGolfDoc*)(g))->shotPowerGaugeX - kFloat_149_805DEA58) / kFloat_360_GolfRule;
        else
            *(float*)(((const GolfRuleMessage*)msg)->arg0.bytes) = kFloat_0_01_GolfRule;
        break;
    }
    case 0xF: {
        float point[2];
        float x, margin, height, y;
        unsigned char* g = GolfDocBytes();
        height = E545_F(g_option, 0x10);
        margin = kFloat_22_GolfRule;
        x = ((CGolfDoc*)(g))->roundedShotGaugeCursorX;
        y = height - margin;
        void* out = ((const GolfRuleMessage*)msg)->arg0.bytes;
        point[1] = y;
        point[0] = x - kFloat_7_GolfRule;
        Vec2_Copy(out, point);
        break;
    }
    case 0xE: {
        unsigned char* initial = GolfPlayerEntry(0);
        int itemIndex = ((const GolfRuleMessage*)msg)->arg0.word;
        ((GolfDocSnapshot8013*)(initial))->playerState = 0;
        unsigned char* item = (unsigned char*)CourseData_FindNamedPoint(
            *(unsigned char**)&((CGolfDoc*)(GolfDocBytes()))->courseData, itemIndex);
        float sum[3], transformed[3], position[3], offset[3], eye[3];
        offset[1] = AsCGolfBall(GolfBall__Fv())->m_radius;
        offset[0] = kFloat_0_GolfRule;
        offset[2] = kFloat_0_GolfRule;

        struct Arr3 {
            int v[3];
        };
        *(Arr3*)position = *(Arr3*)(item + 0x24);
        GetGroundPoint__9CQuadTreeF7WVectorbPib(transformed, CQuadTree_GetInstance(), position, 0, 0, 0);
        __pl__FRC7WVectorRC7WVector(sum, transformed, offset);
        __as__7WVectorFRC7WVector(GolfBall__Fv(), sum);
        temporary = (unsigned char*)GolfBall__Fv();
        temporary = (unsigned char*)((CQuadTree*)CQuadTree_GetInstance())->GetMeshIndex(temporary, 0);
        E545_P(GolfBall__Fv(), 0x28) = temporary;
        unsigned char* player = GolfPlayerEntry(AsCGolfDoc(GolfDocBytes())->currentPlayer);
        __as__7WVectorFRC7WVector(player + 0x1C, GolfBall__Fv());
        if (((const GolfRuleMessage*)msg)->arg1.word) {
            item = (unsigned char*)CourseData_FindNamedPoint(
                *(unsigned char**)&((CGolfDoc*)(GolfDocBytes()))->courseData, ((const GolfRuleMessage*)msg)->arg1.word);
            __as__7WVectorFRC7WVector((unsigned char*)this + 0x78, item + 0x24);
        } else {
            GolfAim_FindTargetAlongPath(eye, *(unsigned char**)&((CGolfDoc*)(GolfDocBytes()))->aimPath, 0);
            __as__7WVectorFRC7WVector((unsigned char*)this + 0x78, eye);
        }
        ((CGolfRule*)this)->ResetShotGauge();
        E545_SEND(kText_screen, 1, 0, 0);
        E545_SEND(kText_effect, 0xB, 0, 0);
        E545_SEND(kText_trajectory, 6, 0, 0);
        ((CFx*)g_fx)->ClearEffects(1);
        E545_SEND(kText_golfRule, 0xD, 0, 0);
        InitializeAimHeading();
        ((CGolfDoc*)GolfDocBytes())->UpdateCamera(1);
        ((CGolfRule*)this)->EnterState(kStateAim);
        AsCGolfDoc(GolfDocBytes())->cameraMode = 0;
        break;
    }
    case 0xD:
        AsCGolfRule(this)->processingPaused = ((const GolfRuleMessage*)msg)->arg0.word != 0;
        break;
    case 2:
        __as__7WVectorFRC7WVector((unsigned char*)this + 0x84, ((const GolfRuleMessage*)msg)->arg0.bytes);
        break;
    case 3:
        __as__7WVectorFRC7WVector(((const GolfRuleMessage*)msg)->arg0.bytes, (unsigned char*)this + 0x84);
        break;
    case 0x1B: {
        bool enabled = ((const GolfRuleMessage*)msg)->arg0.word == 1;
        (*(bool*)&AsCGolfRule(this)->shotBlock) = enabled;
        if (enabled)
            AsCGolfRule(this)->shotBlockTime = kFloat_0_GolfRule;
        break;
    }
    case 5: {
        unsigned char* g = GolfDocBytes();
        unsigned int flags = *(unsigned int*)&((CGolfDoc*)(g))->gameStateFlags;
        if (8 == flags) {
            if (!AsCGolfRule(this)->shotBlock) {
                if (!AsCGolfRule(this)->swingAllowed)
                    ((int (*)(void*, const char*, int, int, int, int, float, float, float))Sound_PlayByName)(
                        g_soundManager, golfRuleStrings + kText_play_unplayable, 0, 0, 0, 0, kFloat_0_5_GolfRule,
                        kFloat_200_GolfRule, kFloat_1_805DEA38);
                else
                    ((CGolfRule*)this)->EnterState(kStateBeginButtonSwing);
            }
        } else if (flags & 0x300) {
            float matrix[12];
            CGolfDoc::BuildCameraMatrix(matrix, g);
            Matrix_CopyAffine(s_cameraTransform, matrix);
            ((CMouseCursor*)g_mouseCursor)->SavePosition();
            ((CGolfRule*)this)->EnterState(kStateAim);
        }
        break;
    }
    case 6:
        AsCGolfRule(this)->swingAllowed = ((const GolfRuleMessage*)msg)->arg0.word != 0;
        E545_SEND(kText_screen, 0x41, AsCGolfRule(this)->swingAllowed, 0);
        break;
    case 7:
        ((CGolfRule*)this)->EnterState(((const GolfRuleMessage*)msg)->arg0.word);
        break;
    case 9: {
        AsCGolfRule(this)->networkSyncPending = 0;
        *(unsigned char**)&((CGolfRule*)(this))->netMsgRecv =
            (unsigned char*)golfRuleStrings + kText_pM_NET_GAMESYNC_START;
        AsCGolfRule(this)->netMsgRecvTime = OS_GetTimeLow();
        int slotArg = ((const GolfRuleMessage*)msg)->arg0.word;
        AsCGolfDoc(GolfDocBytes())->currentPlayer = slotArg;
        unsigned char* sel;
        if (!*(int*)&((CSharedDoc*)(SharedDocBytes()))->gameMode && AsCSharedDoc(SharedDocBytes())->golfRuleType == 0)
            temporary = 0;
        else if (AsCSharedDoc(SharedDocBytes())->golfRuleType == 1)
            temporary = (unsigned char*)(unsigned int)AsCGolfRule(this)->turn;
        else {
            temporary = 0;
            do {
                if (!GolfPlayerEntry((unsigned char)(unsigned int)temporary)[0xA4])
                    break;
                temporary = (unsigned char*)((unsigned int)temporary + 1);
            } while ((unsigned int)temporary < 4);
        }
        sel = (unsigned char*)(unsigned int)(unsigned char)(unsigned int)temporary;
        if (AsCGolfDoc(GolfDocBytes())->playerCount > 2) {
            for (index = 0; AsCGolfDoc(GolfDocBytes())->playerCount > index; ++index) {
                if (index != (int)sel) {
                    E545_SEND(kText_player, 0x24, index, 0);
                    E545_SEND(kText_caddie, 0x17, index, 0);
                }
            }
        }
        E545_SEND(kText_player, 0x29, &ready, sel);
        if (ready) {
            E545_SEND(kText_score, 2, 0, 0);
            ((CGolfDoc*)GolfDocBytes())->ResetPlayerBallPositionsForHole();
            if (!AsCGolfRule(this)->roundRestarted && AsCGolfDoc(GolfDocBytes())->currentHoleIndex == 0)
                ((CGolfRule*)this)->EnterState(kStatePlayerPreview);
            else
                ((CGolfRule*)this)->EnterState(kStateHoleCameraPath);
        } else {
            E545_SEND(kText_player, 0x26, sel, 0);
            E545_SEND(kText_caddie, 0x18, sel, 0);
            ((CGolfRule*)this)->EnterState(kStateWaitHoleResources);
        }
        break;
    }
    case 8: {
        float delta[3], axis[3];
        axis[0] = kFloat_0_GolfRule;
        axis[1] = kFloat_0_GolfRule;
        axis[2] = kFloat_1_805DEA38;
        temporary = GolfDocBytes();
        __mi__FRC7WVectorRC7WVector(delta, (unsigned char*)this + 0x78, GolfBall__Fv());
        E545_F(temporary, 0x31C) = Vector_SignedAngleXZ(delta, axis);
        ((CGolfDoc*)GolfDocBytes())->UpdateCamera(1);
        angle = AsCGolfDoc(GolfDocBytes())->headingRadians;
        E545_SEND(kText_camera, 0x2717, &angle, 0);
        E545_SEND(kText_spot, 0x2717, &angle, 0);
        E545_SEND(kText_screen, 0x2717, &angle, 0);
        E545_SEND(kText_grid, 0x2717, &angle, 0);
        E545_SEND(kText_caddie, 0x2717, &angle, 0);
        E545_SEND(kText_player, 0x2717, GolfDocBytes() + 0x31C, 0);
        AsCGolfRule(this)->nextPlayerIndex = event545HasType() ? AsCGolfDoc(GolfDocBytes())->currentPlayer : 0xFF;
        ((CGolfRule*)this)->EnterState(kStateBeginPlayerTurn);
        break;
    }
    case 0xC:
        AsCGolfRule(this)->autoShot = ((const GolfRuleMessage*)msg)->arg0.word != 0;
        if (AsCGolfRule(this)->autoShot) {
            if (!GetInputManagerSlot()) {
                if (!s_inputManagerInitialized) {
                    InputManager_Init(s_gameInputManager);
                    __register_global_object(s_gameInputManager, (void*)InputManager_Dtor,
                                             s_golfRuleInputDestructorRecord);
                    s_inputManagerInitialized = 1;
                }
                GetInputManagerSlot() = s_gameInputManager;
            }
            float y, x;
            x = (float)((const GolfRuleMessage*)msg)->arg1.word;
            y = (float)((const GolfRuleMessage*)msg)->arg2.word;
            E545_I(GetInputManagerSlot(), 0x188) = 1;
            E545_F(this, 0x3D0) = x;
            E545_F(this, 0x3D4) = y;
        }
        break;
    case 0xA: {
        if (((CGolfDoc*)(GolfDocBytes()))->unk_35C)
            break;
        int slot;
        if (event545HasType())
            slot = AsCGolfDoc(GolfDocBytes())->currentPlayer;
        else
            slot = -1;
        unsigned char* player = GolfPlayerEntry((unsigned char)slot);
        ((GolfDocSnapshot8013*)(player))->playerState = 3;
        ((CGolfDoc*)GolfDocBytes())->RecountActivePlayers();
        int other = event545HasType() ? AsCGolfDoc(GolfDocBytes())->currentPlayer : 0xFF;
        unsigned char current = event545HasType() ? AsCGolfDoc(GolfDocBytes())->currentPlayer : 0xFF;
        unsigned char* g = GolfDocBytes();
        unsigned char* a = (unsigned char*)GolfPlayer_GetEntryByIndex((int)g, current);
        unsigned char* b = GolfPlayerEntry(other);

        ((Event545PlayerBits*)a)->high = (((CGolfDoc*)g)->GetActivePlayerCount() + 1);
        ((Event545PlayerBits*)b)->low = ((Event545PlayerBits*)a)->high;
        ((CGolfRule*)this)->UpdatePlayerRanksAndHoleOrder();
        break;
    }
    case 0x11:
        ResetToShotStart(((const GolfRuleMessage*)msg)->arg0.bytes);
        break;
    case 0x12:
        if (AsCGolfDoc(GolfDocBytes())->tutorialStage < 6)
            CGolfRule::RequestModeExitTransition(0);
        else if (((const GolfRuleMessage*)msg)->arg0.word == 1)
            CGolfRule::RequestModeExitTransition(0);
        break;
    case 0x13: {
        unsigned char* pidx = (unsigned char*)(unsigned int)(unsigned char)((const GolfRuleMessage*)msg)->arg0.word;
        if (SharedDoc_IsPlayerFlag0Set((int)pidx))
            E545_U(SharedDocBytes() + (int)pidx * 0x1D4, 0x38) &= ~1U;
        else {
            E545_U(SharedDocBytes() + (int)pidx * 0x1D4, 0x38) |= 1;
            ((CGolfRule*)this)->EnterState(kStateAim);
            InitializeAimHeading();
        }
        break;
    }
    case 0x14:
        AsCGolfRule(this)->deferredInputButtons |= ((const GolfRuleMessage*)msg)->arg0.word;
        break;
    case 0x15: {
        int slot = AsCGolfDoc(GolfDocBytes())->currentPlayer;
        memcpy((unsigned char*)this + slot * 0x54 + 0x130, SharedDocBytes() + slot * 0x1D4 + 0x8C, 0x20);
        break;
    }
    case 0xB:
        if (!((const GolfRuleMessage*)msg)->arg0.word)
            ((CGolfDoc*)(GolfDocBytes()))->unk_35D = 0;
        break;
    case 0x19:
        RestorePreviousShot(AsCGolfDoc(GolfDocBytes())->currentPlayer, ((const GolfRuleMessage*)msg)->arg0.word != 0);
        break;
    case 0x1D:
        E545_I(E545_P(msg, 8), 0) = AsCGolfRule(this)->shotPresentationFlags;
        break;
    case 0x1E: {
        Event545UI* object = (Event545UI*)*(unsigned char**)&((CGolfRule*)(this))->padSwingController;
        object->v3C();
        break;
    }
    case 0x1F: {
        float amount = (float)((const GolfRuleMessage*)msg)->arg0.word;
        AsCGolfRule(this)->autoShot = 1;
        Event545UI* object = (Event545UI*)*(unsigned char**)&((CGolfRule*)(this))->padSwingController;
        object->v40(amount / kFloat_100_GolfRule, ((const GolfRuleMessage*)msg)->arg1.word);
        break;
    }
    case 0x20: {
        Event545UI* object = (Event545UI*)*(unsigned char**)&((CGolfRule*)(this))->padSwingController;
        object->v4C(1);
        break;
    }
    case 0x21: {
        Event545UI* object = (Event545UI*)*(unsigned char**)&((CGolfRule*)(this))->padSwingController;
        object->v50(1);
        break;
    }
    case 0x22: {
        Event545UI* object = (Event545UI*)*(unsigned char**)&((CGolfRule*)(this))->padSwingController;
        object->v54();
        break;
    }
    case 0x23:
        if (((const GolfRuleMessage*)msg)->arg0.word)
            AsCGolfRule(this)->tutorialFeedbackEnabled = 1;
        else
            AsCGolfRule(this)->tutorialFeedbackEnabled = 0;
        break;
    case 0x24: {
        E545_SEND(kText_golfRule, 0xD, 0, 0);
        unsigned char* player = GolfPlayerEntry(AsCGolfDoc(GolfDocBytes())->currentPlayer);
        ((GolfDocSnapshot8013*)(player))->unk_AE = 0;
        unsigned char* p = GolfPlayerEntry(AsCGolfDoc(GolfDocBytes())->currentPlayer);
        __as__7WVectorFRC7WVector(p + 0x1C, PreData__9CGolfBallFi(input, 0));
        __as__7WVectorFRC7WVector(input, PreData__9CGolfBallFi(input, 0));
        E545_P(input, 0x28) = (unsigned char*)((CQuadTree*)CQuadTree_GetInstance())->GetMeshIndex(input, 0);
        E545_SEND(kText_screen, 1, 0, 0);
        AsCGolfDoc(GolfDocBytes())->headingRadians = E545_F(this, 0x344);
        unsigned char* scene = (unsigned char*)InstanceTable_Get();
        E545_I(scene, 0x44) = 0;
        resetAngle = AsCGolfDoc(GolfDocBytes())->headingRadians;
        E545_SEND(kText_camera, 0x2717, &resetAngle, 0);
        E545_SEND(kText_spot, 0x2717, &resetAngle, 0);
        E545_SEND(kText_screen, 0x2717, &resetAngle, 0);
        E545_SEND(kText_grid, 0x2717, &resetAngle, 0);
        E545_SEND(kText_caddie, 0x2717, &resetAngle, 0);
        E545_SEND(kText_player, 6, 0, 0);
        if (AsCGolfDoc(GolfDocBytes())->tutorialStage == 2)
            E545_SEND(kText_caddie, 3, 0, 0);
        zero = kFloat_0_GolfRule;
        E545_SEND(kText_screen, 9, &zero, 0);
        ((CMouseCursor*)g_mouseCursor)->Reset(1);
        ((CGolfRule*)this)->ResetShotGauge();
        ((CGolfRule*)this)->EnterState(kStateAim);
        E545_SEND(kText_screen, 3, 0, 0);
        break;
    }
    case 0x25:
        AsCGolfRule(this)->tutorialAdvancePending = 1;
        break;
    case 0x26: {
        Event545UI* object = (Event545UI*)*(unsigned char**)&((CGolfRule*)(this))->padSwingController;
        int result = object->v2C();
        E545_I(E545_P(msg, 8), 0) = result;
        break;
    }
    case 0x27:
        if (!(*(unsigned int*)&((CGolfRule*)(this))->shotPresentationFlags & 2) ||
            ((GolfRuleFrameFields*)(this))->presentationStep == 2)
            E545_I(E545_P(msg, 8), 0) = 0;
        else
            E545_I(E545_P(msg, 8), 0) = 1;
        break;
    case 0x28:
        if (*(unsigned char**)&((CGolfRule*)(this))->resultMenu)
            MPBase1_DrawIfInitialized(*(unsigned char**)&((CGolfRule*)(this))->resultMenu);
        if (*(unsigned char**)&((CGolfRule*)(this))->retryMenu)
            MPBase1_DrawIfInitialized(*(unsigned char**)&((CGolfRule*)(this))->retryMenu);
        break;
    case 0x29: {
        void* object = *(unsigned char**)&((CGolfRule*)(this))->forgetItemDialog;
        if (object)
            ((VText*)object)->v03();
        break;
    }
    case 0x2A:
        if (AsCGolfRule(this)->stateTransitionDialog)
            E545_I(E545_P(msg, 8), 0) = 1;
        else
            E545_I(E545_P(msg, 8), 0) = 0;
        break;
    case 0x2B: {
        int result;
        if (AsCGolfRule(this)->stateTransitionDialog || *(unsigned char**)&((CGolfRule*)(this))->forgetItemDialog ||
            *(unsigned char**)&((CGolfRule*)(this))->miniGameSystemResult)
            result = 0;
        else
            result = 1;
        E545_I(E545_P(msg, 8), 0) = result;
        break;
    }
    case 0x2C: {
        int result, last, slot;
        unsigned char* g;
        unsigned char* mgr;
        mgr = SharedDocBytes();
        g = *(unsigned char**)&((CSharedDoc*)(mgr))->golfDoc;
        slot = ((CGolfDoc*)(g))->currentPlayer;
        last = ((CGolfDoc*)(g))->playerCount - 1;
        if (0 > slot)
            slot = 0;
        else if (slot > last)
            slot = last;
        result = 1;
        if (AsCGolfRule(this)->shotUndoEnabled && ((CGolfDoc*)(g))->shotMeterFadeResetRequested == 0 &&
            E545_I((unsigned char*)this + slot * 0x18, 0xB0) &&
            (*(int*)&((CSharedDoc*)(mgr))->partyGameMode || !((unsigned char*)this + slot)[0x25C])) {
        } else
            result = 0;
        E545_I(E545_P(msg, 8), 0) = result;
        break;
    }
    case 0x2D:
        if (!(*(unsigned int*)&((CGolfRule*)(this))->shotPresentationFlags & 2) ||
            !((GolfRuleFrameFields*)(this))->presentationStep)
            E545_I(E545_P(msg, 8), 0) = 0;
        else
            E545_I(E545_P(msg, 8), 0) = 1;
        break;
    case 0x2E: {
        int value = (int)*(float*)(((const GolfRuleMessage*)msg)->arg0.bytes);
        E545_I((unsigned char*)this + AsCGolfDoc(GolfDocBytes())->currentPlayer * 0x54, 0x15C) = value;
        break;
    }
    case 0x2F: {
        unsigned char* g;
        unsigned int found;
        unsigned char* mgr;
        if (*(int*)&((CSharedDoc*)(SharedDocBytes()))->partyGameMode)
            found = 0;
        else {
            temporary = 0;
            offset = 0;
            // Event 0x545 (CGolfRule::HandleMsg): record the first matching item slot.
            do {
                mgr = SharedDocBytes();
                g = *(unsigned char**)&((CSharedDoc*)(mgr))->golfDoc;
                void* value = ItemInventory_GetItemIdAt(mgr + 0xA04, 6);
                if (value == (void*)E545_P(
                                 ((CGolfDoc*)(g))->currentPlayer * 0x1D4 + (unsigned char*)(offset + (int)mgr), 0x8C)) {
                    AsCGolfRule(this)->forgetItemSlot = (int)temporary;
                    found = 1;
                    break;
                }
                temporary = (unsigned char*)((int)temporary + 1);
                offset = offset + 4;
            } while ((int)temporary < 8 || (found = 0, false));
        }

        *((const GolfRuleMessage*)msg)->arg0.bytes = found;
        break;
    }
    default:
        break;
    }
}

namespace ns4 {
extern "C" {
// Publish the requested golf state after the drop-confirmation gate, then run its entry actions.
// Entry contract: rule, golf document, current-player record and course are live.
// Each request first handles CameraTransition+dropRequest: BeginPlayerTurn clears
// the pending request; all other requests open/retain its dialog and return BEFORE
// changing gameStateFlags. Otherwise publish requestedState and zero elapsedTime.
// Course render states reset unless the request contains the CameraTransition bit.
// Entry-action map (names below are GolfRuleState values, not sequential indexes):
//   HoleOverview -> tee/pin overview; HoleCameraPath -> reset ball -> PlayerPreview.
//   RestoreSavedShot -> saved weather/ball/gauge -> Aim.
//   StartTurnWithBgm -> audio/reset -> Aim, then install course wind regions.
//   PlayerPreview -> choose player, initialize preview/weather/wind; wait for update.
//   TurnStartDelay -> timer/caddie camera; Aim -> aim UI and controller setup.
//   BeginButtonSwing -> capture spin/curve and clear commands; ButtonSwing -> gauge.
//   PrepareNextShot -> reset timer/camera only when document +0x35C is clear.
//   CameraTransition -> transition pose/mode 2.
//   ButtonShotAnimation -> requires committed gauges; phases A..F generate samples
//     and presentation. BallFlight -> requires those samples; rewind playback.
//   HoleResults -> rankings/scoreboard; DelayedTransition -> result presentation,
//     or WaitPlayerAndCaddie when match hole-result setup returns false.
//   BeginPlayerTurn -> timeout bookkeeping; terminal branches choose
//     DelayedTransition for match rules, FightResult otherwise; else timeout pose.
//   PostRoundMenus -> rewards/award camera; FightResult -> fight presentation.
//   CourseReload -> clear reload progress; StorySequence/RoundResults -> displays.
//   MotionSwing -> controller activation/input reset; WaitPlayerAndCaddie -> hide
//     party UI; MotionCameraInput -> bind player actor; WaitHoleResources -> loader.
//   CameraView/Unresolved400/CameraReturn/MotionShotAnimation -> common entry only.
//   Other values (including TourExit) -> common entry only, no switch action.
// Entry does not validate state-specific resources; callers must satisfy the
// contracts above. Deferred transitions are owned by OnProcess and its handlers.
#pragma push
#pragma always_inline on
// Recompute hole rankings and configure result messages before the result update phase.
static inline void EnterGolfHoleResults(unsigned char* self) {
    if (SIM_INT_AT(SharedDocBytes(), 0xAD8)) {
        PartyItemSlots_RunFinalLifecycleHook(PartyItemSlots_GetInstance());
        T_MSG(kText_letter, 0, 0, 0);
        T_END_UI();
        T_MSG(kText_balloonSystem, 4, 0, 0);
        for (unsigned char i = 0; i < AsCGolfDoc(GolfDocBytes())->playerCount; i++)
            SIM_INT_AT(GolfPlayerEntryByte(i), 8) = 1;
    }
    ((CGolfRule*)self)->UpdatePlayerRanksAndHoleOrder();
    self[0x93] = 0;
    self[0x41C] = 0;
    SoundManager_RequestBgmChange(g_soundManager, golfRuleStrings + kText_scoreboard_wav, 0, 1);
    Sound_StopByAlias(g_soundManager, 0);
    T_MSG(kText_score, 1, 1, 0);
    T_MSG(kText_spot, 4, 0, 0);
    T_MSG(kText_grid, 3, 0, 0);
    SIM_INT_AT(self, 0x4B8) = 0;
    *(bool*)(SharedDocBytes() + 0x804) = AsCGolfDoc(GolfDocBytes())->currentHoleIndex ==
                                         (unsigned char)(AsCSharedDoc(SharedDocBytes())->roundHoleCount - 1);
    if (AsCSharedDoc(SharedDocBytes())->golfRuleType == 1) {
        unsigned char finished = SharedDocBytes()[0x804];
        for (int i = 0; i < AsCGolfDoc(GolfDocBytes())->playerCount; i++) {
            unsigned char hole = AsCGolfDoc(GolfDocBytes())->currentHoleIndex;
            unsigned char* other = GolfPlayerEntryByte(i ^ 1);
            unsigned char hole2 = AsCGolfDoc(GolfDocBytes())->currentHoleIndex;
            unsigned char* pl = GolfPlayerEntryByte(i);
            int diff = pl[hole2 + 0x28] - other[hole + 0x28];
            if (SIM_INT_AT(GolfPlayerEntryByte(i), 0x160) == 2) {
                if (diff <= 0)
                    finished = 1;
            } else if (SIM_INT_AT(GolfPlayerEntryByte(i), 0x160) == 1 && diff < 0)
                finished = 1;
        }
        SharedDocBytes()[0x804] = finished;
    }
    return;
}
#pragma pop

static inline int IdMatches(int k) {
    unsigned char* mgr = SharedDocBytes();
    unsigned char* game = GolfDocBytes();
    unsigned id = ItemInventory_GetBallIdAt(mgr + 0xA04, k);
    return id == (unsigned)RuleIntAt(mgr + game[0x3C1] * 0x1D4, 0x88);
}
static inline int CurrentPlayerHasItemAtSlot(int k) {
    unsigned char* lay = SharedDocBytes() + 0xA04;
    unsigned char* pl =
        (unsigned char*)GolfPlayer_GetEntryByIndex((int)GolfDocBytes(), AsCGolfDoc(GolfDocBytes())->currentPlayer);
    return (unsigned)RuleIntAt(pl, 0xA0) == (unsigned)ItemInventory_GetItemIdAt(lay, k);
}
static inline int PenaltyLimit54(unsigned char* self) {
    int count = T_CUR[0xAF];
    if (count >= 3) {
        if (!GolfTask_NoOpStatus())
            T_MSG(kText_golfRule, 0xA, RuleIntAt(T_CUR, 0x98), 0);
        return 1;
    }
    return 0;
}
static inline void EnterAimState(unsigned char* self, unsigned char* ballBytes, float* aimCameraMatrix) {
    // Restore aiming camera and presentation; recovery clears only after this entry.
    self[0x93] = 1;
    if (AsCGolfBall(ballBytes)->m_recoveryRequired) {
        AsCGolfBall(ballBytes)->m_recoveryRequired = 0;
        ((CGolfDoc*)GolfDocBytes())->UpdateCamera(1);
        AsCGolfBall(ballBytes)->m_lie = 0;
    } else {
        CGolfDoc::BuildCameraMatrix(aimCameraMatrix, GolfDocBytes());
        Matrix_CopyAffine(s_cameraTransform, aimCameraMatrix);
    }
    T_MSG(kText_spin, 3, 0, 0);
    T_MSG(kText_screen, 6, 0, 0);
    if (!SharedDoc_IsPlayerFlag0Set(AsCGolfDoc(GolfDocBytes())->currentPlayer))
        T_MSG(kText_camera, 4, 0, 0);
    T_MSG(kText_camera, 1, 0, 0);
    T_MSG(kText_screen, 2, 1, 0);
    T_MSG(kText_spot, 3, 0, 0);
    Camera_SetFov(g_view, kGolfCameraFovRadians);
    T_MSG(kText_player, 0x32, 2, AsCGolfDoc(GolfDocBytes())->currentPlayer);
    ((CCameraWork*)g_cameraWork)->UninstallCamera();
    return;
}
static inline void EnterCameraTransitionState(unsigned char* self, float* transitionMatrix) {
    // Install camera transition transform and course rendering before camera input.
    CourseRenderStates_ApplyPair(AsCGolfDoc(GolfDocBytes())->courseData, 1);
    Camera_SetFov(g_view, kGolfCameraTransitionFovRadians);
    Camera_SetClip(g_view, kFloat_3_0, kFloat_100000_GolfRule, 1);

    CGolfDoc::BuildModeCameraTransform(transitionMatrix, GolfDocBytes(), 0);
    Matrix_CopyAffine(s_cameraTransform, transitionMatrix);
    T_MSG(kText_camera, 6, 0, 0);
    DrawScene_NoOpHook(g_sceneManager, 0);
    AsCGolfDoc(GolfDocBytes())->cameraMode = 2;
    return;
}
static inline void EnterFightResultState(unsigned char* self) {
    Camera_SetFov(g_view, kGolfCameraFovRadians);
    if (AsCSharedDoc(SharedDocBytes())->golfRuleType != 1 &&
        (unsigned)(AsCGolfDoc(GolfDocBytes())->tutorialStage - 5) <= 1) {
        T_MSG(kText_screen, 0x15, 1, 0);
        T_MSG(kText_score, 5, 0, 0);
    }
    CGolfRule::StartRoundResultPresentation((char*)self, 0, SIM_INT_AT(SharedDocBytes(), 0xAD8) == 1,
                                            golfRuleStrings + kText_story_lose_wav);
    T_END_UI();
    self[0x2E0] = ((CGolfRule*)self)->SelectNextPlayer(0);
    int resultPlayerIndex = AsCGolfDoc(GolfDocBytes())->currentPlayer;
    if (AsCSharedDoc(SharedDocBytes())->golfRuleType == 1)
        resultPlayerIndex = ((CGolfDoc*)GolfDocBytes())->ResolveMatchTurnFromHoleStrokes();
    T_MSG(kText_player, 0x32, 2, resultPlayerIndex);
    return;
}
static inline void EnterHoleOverviewState(unsigned char* self, int* teePositionWords, int* pinPositionWords,
                                          float* teePlusPin, float* overviewTarget) {
    // Set the overview target to the tee/pin midpoint and hide the player models.
    // OnProcess offsets the camera eye from this target.
    AsCGolfDoc(GolfDocBytes())->shotPhase = kShotIdle;
    AsGolfRuleFrame(self)->stateTimer = Random_RangeFloat(-1, kFloat_0_GolfRule, kFloat_36_GolfRule);
    unsigned char* holeRecord =
        *(unsigned char**)(GolfDocBytes() + 0x2BC) + (AsCGolfDoc(GolfDocBytes())->currentHoleIndex << 5);
    *(D3*)teePositionWords = *(D3*)holeRecord;
    holeRecord = *(unsigned char**)(GolfDocBytes() + 0x2BC) + (AsCGolfDoc(GolfDocBytes())->currentHoleIndex << 5);
    *(D3*)pinPositionWords = *(D3*)(holeRecord + 0xC);
    __pl__FRC7WVectorRC7WVector(teePlusPin, pinPositionWords, teePositionWords);
    __ml__FfRC7WVector(overviewTarget, teePlusPin, kFloat_0_5_GolfRule);
    *(WVector*)(self + 0x28) = *(WVector*)overviewTarget;
    Camera_SetClip(g_view, kFloat_3_0, kFloat_100000_GolfRule, 1);
    void* sceneManager = g_sceneManager;
    DrawScene_SetNamedObjectEnabled(
        sceneManager,
        String_FormatTemporary(golfRuleStrings + kText_playerActorFormat, AsCGolfDoc(GolfDocBytes())->currentPlayer),
        0);
    DrawScene_SetNamedObjectEnabled(g_sceneManager, golfRuleStrings + kText_clubActor, 0);
    DrawScene_SetNamedObjectEnabled(g_sceneManager, golfRuleStrings + kText_caddieActor, 0);
    ((CShadowManager*)g_shadowManager)->SetSelectionFlags(0);
    T_MSG(kText_score, 0xF, 1, 0);
    return;
}
static inline void EnterMotionCameraInputState(unsigned char* self) {
    void* actorRoot = g_taskManager ? Object_GetChild(g_taskManager) : 0;
    void* playerActor = TaskActorTable_FindByName(actorRoot, golfRuleStrings + kText_player);
    unsigned char* swingController = *(unsigned char**)(self + 0x4C4);
    ((void (*)(void*, void*))(((void**)AsCPadSwingCtrl(swingController)->interfaceVtable))[5])(swingController,
                                                                                               playerActor);
    return;
}
static inline void EnterRoundResultsState(unsigned char* self) {
    ((CGolfRule*)self)->UpdatePlayerRanksAndHoleOrder();
    if (AsCSharedDoc(SharedDocBytes())->golfRuleType == 0 ||
        (AsCSharedDoc(SharedDocBytes())->golfRuleType == 1 && ((GolfPlayerEntryByte(0)[0xC4] >> 4) & 15) == 1)) {
        if (*(short*)(SharedDocBytes() + 0x42) > 0) {
            --*(short*)(SharedDocBytes() + 0x42);
            GolfPlayerEntryByte(0)[0x16E] = 1;
        }
    }
    GolfRule_ResetItemRequestState(self, 0);
    T_SINGLE(((char*)s_golfRuleInputDestructorRecord + kFadeOverlayDestructorRecord));
    if (FadeState_GetState(Sing_805DC608())) {
        T_SINGLE(((char*)s_golfRuleInputDestructorRecord + kFadeOverlayDestructorRecord));
        FadeState_StartFadeOut(Sing_805DC608(), 30);
    }
    SIM_INT_AT(self, 0x4E0) = 0;
    return;
}
static inline void EnterStorySequenceState(unsigned char* self) {
    T_MSG(kText_storyConte, 0, 0, 0);
    T_MSG(kText_camera, 5, 0, 0);
    T_MSG(kText_screen, 0x59, 0, 0);
    T_MSG(kText_player, 0x2E, 0, 0);
    T_MSG(kText_caddie, 0x1C, 0, 0);
    CQuadTree_SetObjectSoundEnabled(CQuadTree_GetInstance(), 0);
    unsigned char creatureIndex;
    void* sceneManager = g_sceneManager;
    for (creatureIndex = 0; creatureIndex < 3; creatureIndex++)
        DrawScene_SetNamedObjectEnabled(sceneManager, golfRuleAmbientCreatureNames[creatureIndex], 0);
    T_MSG(kText_storyConte, 2, SIM_INT_AT(self, 0x4C8), 0);
    T_SINGLE(((char*)s_golfRuleInputDestructorRecord + kFadeOverlayDestructorRecord));
    FadeState_StartFadeOut(Sing_805DC608(), 0);
    SIM_INT_AT(self, 0x4CC) = 0;
    return;
}
static inline void EnterTurnStartDelayState(unsigned char* self, int* caddiePositionWords) {
    // Prepare caddie transition and turn-start camera; OnProcess advances the delay.
    Camera_SetFov(g_view, kGolfCameraFovRadians);
    T_MSG(kText_screen, 2, 1, 0);
    T_MSG(kText_caddie, 0x2715, golfRuleStrings + kText_chanceSequence, 0);
    if (AsCSharedDoc(SharedDocBytes())->golfRuleType != 1)
        T_MSG(kText_caddie, 0xB, 0x22, 0);

    T_MSG(kText_caddie, 6, caddiePositionWords, 0);
    ((CCameraWork*)g_cameraWork)->SetupMode202Transition(s_cameraTransform, caddiePositionWords);
    ((CCameraWork*)g_cameraWork)->UpdateMotionAndTarget(kFloat_0_GolfRule, kFloat_0_GolfRule);
    return;
}
static inline void InitializeButtonGaugeState(unsigned char* self) {
    AsCGolfDoc(GolfDocBytes())->shotGaugeDirection = kGaugeAdvancing;
    self[0x93] = 0;
    SIM_INT_AT(self, 0x3A8) = 0;
    SIM_INT_AT(self, 0x3AC) = 0;
    ((CFx*)g_fx)->SetUpdateEnabled(0);
    SIM_FLOAT_AT(self, 0x3FC) = kFloat_1_805DEA38;
    if (SIM_INT_AT(SharedDocBytes(), 0xAD8) == 3)
        SIM_FLOAT_AT(self, 0x3FC) = ((ShotAccessoryModifiers*)PartyItemSlots_GetInstance())->gaugeRateScale;
}
static inline void ResetCameraInputForGolfState() {
    RuleByteAt(g_cameraWork, 0x1C) = 0;
    RuleFloatAt(g_cameraWork, 0x10) = kFloat_0_GolfRule;
    Vec2_Zero(AsCCameraWork(g_cameraWork)->inputRotationOffset);
}
// ===== S8 - State entries and button launch copy =====
// Rule, ShotSample, ShotCheckpoint and ball views; keep both launch paths.
}
}

// Publish a state and run its entry actions; recursive transitions retain their original order.
// Requires constructed rule and loaded services appropriate to the requested state.
// Pending drop/dialog handling may return before publishing the requested state.
// Otherwise publishes gameStateFlags, resets elapsed time and runs the entry case.
// Per-case comments list continuation; launch duplicates BuildShotSimulation on purpose.
// RNG, actor messages, floats and accessor-call order follow each original branch.
// Shot flag, ground, impact and time domains are used below.
void CGolfRule::EnterState(int requestedState) {
    using namespace ns4;
    // Phase 1: resolve pending drop confirmation before publishing a new state.
    unsigned char* effectStateBytes;
    unsigned char* firstSampleBytes;
    unsigned char* ballBytes;
    char* inputDestructorRecordBytes = (char*)s_golfRuleInputDestructorRecord;
    unsigned char* self = (unsigned char*)this;
    float yawCorrectionRadians, impactAdjustment, powerFraction, launchSpeed, speedScale, absoluteAdjustment,
        normalizedImpactRatio;
    int impactDirection;
    // screenVectorReply receives a WVector: CScreen_HandleMsg event 0x23 (0x80182AB8-0x80182AD4) assigns 12 bytes to the
    // out pointer; the original passes a vector slot (r31+0x2F4, 0x80157E9C). As a single float its y/z overwrote headingRadians
    // (the shot heading from Player event 3), so Lucky/special shots launched toward heading ~0
    float headingRadians, shotKeySeconds, screenVectorReply[3];
    int screenPowerFlagsReply;
    float woodMissRollingVelocity[3], ironMissRollingVelocity[3], wedgeMissRollingVelocity[3],
        ironMissAirborneVelocity[3], cameraDirectedVelocity[3], worldLaunchVelocity[3], pitchAdjustedVelocity[3],
        wedgeMissAirborneVelocity[3];
    float ironMissPitch[12], wedgeMissPitch[12], launchYaw[12], launchPitch[12];
    float shotWind[3], waterTestPosition[3], outsideTestPosition[3], groundQueryPosition[3], groundRecoveryPosition[3],
        cobraVelocity[3], spikeVelocity[3], cobraTravelXZ[3], spikeTravelXZ[3];
    float normalModeBaseSpeed, modeOneBaseSpeed, modeTwoBaseSpeed, totalTravelXZ[3], finalHorizontalDisplacement[3];
    char simulationStopwatch[0x40];
    if ((unsigned)AsCGolfDoc(GolfDocBytes())->gameStateFlags == kStateCameraTransition &&
        AsCGolfDoc(GolfDocBytes())->dropRequest) {
        if ((unsigned)requestedState == kStateBeginPlayerTurn) {
            AsCGolfDoc(GolfDocBytes())->dropRequest = 0;
            void* o = *(void**)&AsCGolfRule(self)->stateTransitionDialog;
            if (o)
                ((void (*)(void*, int))(*(void***)((char*)o + 4))[4])(o, 0);
        } else {
            if (!*(void**)(self + 0x400)) {
                SmallCommonDialog_CreateAndRegisterMode5(0, golfRuleMessageTags, self + 0x400, kFloat_0_GolfRule);
                void* base = self;
                if (self)
                    base = self + 0x18;
                int tmp[3], text[3];
                int* p = (int*)Runtime_AdjustMemberPointer(-0x18, s_golfRuleSecondaryInterfaceCallback, tmp);
                text[0] = p[0];
                text[1] = p[1];
                text[2] = p[2];
                void* o = *(void**)&AsCGolfRule(self)->stateTransitionDialog;
                ((void (*)(void*, void*, void*))(*(void***)((char*)o + 4))[19])(o, text, base);
            }
            AsCGolfDoc(GolfDocBytes())->cameraMode = 2;
            return;
        }
    }
    // Phase 2: publish the state and reset its elapsed timer, then initialize
    // only the resources/UI/camera required by the selected case below.
    AsCGolfDoc(GolfDocBytes())->gameStateFlags = requestedState;
    AsGolfRuleFrame(self)->elapsedTime = kFloat_0_GolfRule;
    ballBytes = GOLF_BALL_BYTES;
    if (!(requestedState & kStateCameraTransition))
        CourseRenderStates_ApplyPair(AsCGolfDoc(GolfDocBytes())->courseData, 0);
    switch (requestedState) {
    // Publishes the overview camera and hidden player models from loaded tee/pin records.
    // The existing overview processor owns continuation.
    case kStateHoleOverview: {
        // Seed the tee/pin midpoint, configure the overview camera, and hide player models.
        int teePositionWords[3], pinPositionWords[3];
        float teePlusPin[3], overviewTarget[3];
        EnterHoleOverviewState(self, teePositionWords, pinPositionWords, teePlusPin, overviewTarget);
        return;
    }
    // Reset the ball at the hole start, then enter player preview.
    // Publishes a tee reset, ground handle and cleared overview UI.
    // Immediately enters PlayerPreview after the reset messages.
    case kStateHoleCameraPath: {
        // Reset the ball at the hole start, then enter player preview immediately.
        ((CQuadTree*)CQuadTree_GetInstance())->InitGroundOverrideBounds();
        T_MSG(kText_letter, 0, 0, 0);
        T_MSG(kText_score, 0, 0, 0);
        T_MSG(kText_score, 1, 0, 0);
        T_MSG(kText_caddie, 0xE, 0, 0);
        AsGolfRuleFrame(self)->stateTimer = kFloat_0_GolfRule;
        AsCGolfDoc(GolfDocBytes())->unk_35E = 0;
        float a[3] = {kFloat_0_GolfRule, AsCGolfBall(ballBytes)->m_radius, kFloat_0_GolfRule}, c[3];
        int b[3], d[3];
        unsigned char* rec = (unsigned char*)AsCGolfDoc(GolfDocBytes())->holeRecords +
                             (AsCGolfDoc(GolfDocBytes())->currentHoleIndex << 5);
        *(D3*)b = *(D3*)rec;
        __pl__FRC7WVectorRC7WVector(c, b, a);
        *(D3*)d = *(D3*)c;
        ((CGolfBall*)ballBytes)->Reset(d);
        ((BallGroundTriangleView*)ballBytes)->groundTriangleHandle =
            (int)((CQuadTree*)CQuadTree_GetInstance())->GetMeshIndex(ballBytes, 0);
        ((CGolfRule*)self)->ResetPlayerPositionsAndShotHistory((const char*)ballBytes);
        ((CGolfRule*)self)->EnterState(kStatePlayerPreview);
        return;
    }
    // Restore saved shot inputs and re-enter aim.
    // Publishes saved ball/player position, wind and cleared shot presentation.
    // Resets gauge/camera and immediately enters Aim.
    case kStateRestoreSavedShot: {
        // Restore saved weather/wind/ball position, reset the gauge, then enter aim.
        unsigned char* r = (unsigned char*)AsCSharedDoc(SharedDocBytes())->selectedCourseData;
        if (r && *(signed char*)((IFF_STRUCT::sCourse*)r)->ambientSound)
            T_SOUND(((IFF_STRUCT::sCourse*)r)->ambientSound, 1, 0);
        CourseWeather_ConfigureEffects(g_sceneManager, ((GolfRuleShotReplayInputs*)self)->weatherVariant, 0);
        CWind::GetInstance()->SetWindParams(255, 255, 0, 1);
        CWind::GetInstance()->SetWindVector(((GolfRuleShotReplayInputs*)self)->wind);
        CourseData_UpdateWindActors(AsCGolfDoc(GolfDocBytes())->courseData);
        DrawScene_InvokePrimaryHook(g_sceneManager);
        ((CQuadTree*)CQuadTree_GetInstance())->InitGroundOverrideBounds();
        int pos[3], ground[3];
        *(D3*)pos = *(D3*)&((GolfRuleShotReplayInputs*)self)->ballStartPosition;
        GetGroundPoint__9CQuadTreeF7WVectorbPib(ground, CQuadTree_GetInstance(), pos, 0, 0, 0);
        *(WVector*)(self + 0x338) = *(WVector*)ground;
        SIM_FLOAT_AT(self, 0x33C) = SIM_FLOAT_AT(self, 0x33C) + AsCGolfBall(ballBytes)->m_radius;
        int cameraPos[3];
        *(D3*)cameraPos = *(D3*)(self + 0x338);
        ((CGolfBall*)ballBytes)->Reset(cameraPos);
        ((BallGroundTriangleView*)ballBytes)->groundTriangleHandle =
            (int)((CQuadTree*)CQuadTree_GetInstance())->GetMeshIndex(ballBytes, 0);
        ((CGolfRule*)self)->ResetPlayerPositionsAndShotHistory((const char*)ballBytes);
        AsGolfDocSnapshot(GolfPlayerEntryByte(0))->unk_AE = ((GolfRuleShotReplayInputs*)self)->playerSnapshotByteAE;
        void* ctl = CQuadTree_GetInstance();
        ((ShotWorldClockView*)ctl)->frame = 0;
        ((ShotWorldClockView*)ctl)->fractionalSeconds = kFloat_0_GolfRule;
        ((CGolfRule*)self)->InitializeAimHeading();
        float enabled = ((GolfRuleShotReplayInputs*)self)->powerMode ? kFloat_1_805DEA38 : kFloat_0_GolfRule;
        T_MSG(kText_screen, kScreenSetPowerColorMessage, &enabled, 0);
        T_MSG(kText_screen, 0x14, 0, 0);
        T_MSG(kText_screen, 0x27, 0, 0);
        T_MSG(kText_item, 7, 0, 0);
        ((CGolfRule*)self)->ResetShotGauge();
        ((CGolfDoc*)GolfDocBytes())->UpdateCamera(0);
        ((CGolfRule*)self)->EnterState(kStateAim);
        return;
    }
    // Select turn audio, reset the ball/UI, then enter aim.
    // Publishes random BGM, reset ball, gauge, camera and wind state.
    // Immediately enters Aim; playlist must be nonempty before rand()%n.
    case kStateStartTurnWithBgm: {
        // Choose turn audio, reset the ball and shot UI, then select turn presentation.
        // Requires bgmNameCount != 0 after playlist construction (rand()%n).
        // The 0x200-byte path is formatted into itself by the existing sprintf.
        char text[0x200];
        FileIO_GetDefaultDevicePath(0x200, text);
        sprintf(text, golfRuleStrings + kText_bgmDirectoryFormat, text);
        ((CGolfRule*)self)->BuildBgmPlaylist(text);
        // Playlist must be nonempty: selection evaluates rand()%n.
        unsigned n = AsCGolfRule(self)->bgmNameCount;
        AsCGolfRule(self)->selectedBgmIndex = (unsigned)rand() % n;
        Sound_StopByAlias(g_soundManager, 0);
        switch (((SharedEntryProtocol*)SharedDocBytes())->weatherType) {
        case 2:
            if ((unsigned char)SharedDoc_GetCourseId() != 9 && (unsigned char)SharedDoc_GetCourseId() != 8)
                T_SOUND(golfRuleStrings + kText_rainSequence, 1, golfRuleStrings + kText_ambientSound);
            break;
        default: {
            unsigned char* r = (unsigned char*)AsCSharedDoc(SharedDocBytes())->selectedCourseData;
            if (r && *(signed char*)((IFF_STRUCT::sCourse*)r)->ambientSound)
                T_SOUND(((IFF_STRUCT::sCourse*)r)->ambientSound, 1, golfRuleStrings + kText_ambientSound);
            else
                Sound_StopByAlias(g_soundManager, golfRuleStrings + kText_ambientSound);
        }
        }
        FloatTween_Start((inputDestructorRecordBytes + 0x3C), -1, kFloat_1_805DEA38, kFloat_0_GolfRule);
        CQuadTree_SetObjectSoundEnabled(CQuadTree_GetInstance(), 1);
        AsCGolfDoc(GolfDocBytes())->shotPhase = kShotIdle;
        ((CQuadTree*)CQuadTree_GetInstance())->InitGroundOverrideBounds();
        unsigned char* pl = T_CUR;
        int pos[3];
        *(D3*)pos = *(D3*)AsGolfDocSnapshot(pl)->ballPosition;
        ((CGolfBall*)ballBytes)->Reset(pos);
        ((BallGroundTriangleView*)ballBytes)->groundTriangleHandle =
            (int)((CQuadTree*)CQuadTree_GetInstance())->GetMeshIndex(ballBytes, 0);
        AsGolfRuleFrame(self)->stateEntryPending = 0;
        ((RuleUnresolvedShotBytes*)self)->unk_091 = 0;
        ((RuleUnresolvedShotBytes*)self)->unk_092 = 0;
        AsCGolfRule(self)->shotUndoEnabled = 0;
        ((RuleUnresolvedShotBytes*)self)->unk_094 = 0;
        T_MSG(kText_ball, 2, 0, 0);
        T_MSG(kText_screen, 1, 0, 0);
        T_MSG(kText_clock, 0, 0, 0);
        T_MSG(kText_golfBg, 6, 0, 0);
        float mat[12], v[3];
        Matrix_SetAxisRotation(mat, AsCGolfDoc(GolfDocBytes())->headingRadians, 1);
        __pl__FRC7WVectorRC7WVector(v, ballBytes, mat + 6);
        AsGolfRuleFrame(self)->aimTarget = *(WVector*)v;
        float angle = AsCGolfDoc(GolfDocBytes())->headingRadians;
        T_MSG(kText_camera, 0x2717, &angle, 0);
        T_MSG(kText_spot, 0x2717, &angle, 0);
        T_MSG(kText_screen, 0x2717, &angle, 0);
        T_MSG(kText_grid, 0x2717, &angle, 0);
        T_MSG(kText_caddie, 0x2717, &angle, 0);
        T_MSG(kText_player, 6, 0, 0);
        T_MSG(kText_caddie, 3, 0, 0);
        T_MSG(kText_spot, 5, 0, 0);
        float enabled =
            ((ClubSelectionRuntime*)GOLF_SHOT_PARAMETERS_BYTES)->powerShotMode ? kFloat_1_805DEA38 : kFloat_0_GolfRule;
        T_MSG(kText_screen, kScreenSetPowerColorMessage, &enabled, 0);
        T_MSG(kText_screen, 0x14, 0, 0);
        T_MSG(kText_screen, 0x27, 0, 0);
        T_MSG(kText_trajectory, 6, 0, 0);
        T_MSG(kText_letter, 0, 0, 0);
        T_MSG(kText_effect, 0xB, 0, 0);
        T_MSG(kText_effect, 0x13, 0, 0);
        T_MSG(kText_effect, 0xC, 0, 0);
        T_MSG(kText_score, 1, 0, 0);
        ((CGolfRule*)self)->ResetShotGauge();
        ((CGolfDoc*)GolfDocBytes())->UpdateCamera(0);
        T_CLEAR();
        if (NamedEffectList_FindByName(g_sceneManager, golfRuleStrings + kText_ballSequence)) {
            int h = NamedEffectList_FindByName(g_sceneManager, golfRuleStrings + kText_ballSequence);
            ((void (*)(int, int))WPuppet_SetOpacityRecursive)(h, 255);
        }
        ((CFx*)g_fx)->ClearEffects(1);
        ((CGolfRule*)self)->EnterState(kStateAim);
        CWind::GetInstance()->LoadCourseRegions();
        CWind::GetInstance()->InitOverrideFromCourse();
        CourseData_UpdateWindActors(AsCGolfDoc(GolfDocBytes())->courseData);
        return;
    }
    // Initialize player/caddie preview and weather presentation.
    // Publishes player spawn snapshots, preview actors, wind and camera mode 0xCB.
    // ProcessPlayerPreview owns continuation.
    case kStatePlayerPreview: {
        // Entry contract: loaded course/hole records, playerCount in [1,4],
        // currentHoleIndex in [0,18), nonempty BGM list (rand()%count below).
        // Phase ledger (all pointers borrowed; no new per-player allocation):
        // 1. Clear installed camera; request fade-overlay release, duration 30
        //    fade updates (not seconds; completion follows the fade state's test).
        // 2. SharedDoc+9C6 weather protocol: 2 selects rain except courses 8/9;
        //    1 selects random lightning prep except course 9, then FALLS THROUGH
        //    to course ambient sound. Other values select ambient sound only.
        //    +8BC is IFF sCourse*, ambientSound is its optional NUL-terminated key.
        // 3. Start actor-scale tween, enable ground object sounds, choose BGM;
        //    RNG order is lightning choice first (when admitted), BGM second.
        // 4. Choose currentPlayer: normally 0; +808/rule override enables turn
        //    (match rule 1), otherwise find player whose order byte +A4 is 0.
        //    That search must succeed among four entries; there is no fallback.
        // 5. rule+4E8 = holeRecords[hole].XYZ + (0,ball radius,0), world Y-up.
        //    Party mode 3 updates item-slot effects; mode 1 (darts) projects to
        //    ground, reapplies ball radius and resets ball at that position.
        // 6. For every player with state word +8 !=3: state=0, position(+1C)
        //    = spawn, word +178=0; excluded players retain those three values.
        // 7. Synchronous receiver protocols: player 0x28/caddie 0x1A reset;
        //    player 0x26/caddie 0x18 select next player (two separate queries);
        //    caddie 0xD, screen 0x13, npc 8 reset their presentation. Ground
        //    controller +21AC/+21A8 clear before aim initialization/gauge reset.
        // 8. Camera mode 0xCB; darts sends player 6, otherwise entrance animation
        //    via 0x2715. Current score-record strokes[hole](+28)=1; match/rival
        //    saves selected-club byte at +BD. Load wind regions/override and
        //    update available course actors; rival +98=1 on its flagged path.
        //    Finally rule+4F4=0 and voice message 0. This entry returns with
        //    PlayerPreview published; ProcessPlayerPreview owns continuation.
        // Message IDs are receiver-local, not shared enums. Preserve their order
        // and existing partial effects if a callee fails; there is no rollback.
        ((CCameraWork*)g_cameraWork)->UninstallCamera();
        T_SINGLE_G((inputDestructorRecordBytes + 0x30));
        if (FadeState_GetState(Sing_805DC608())) {
            FadeState_RequestReleaseOnFadeCompletion(FadeState_GetInstance());
            T_SINGLE_G((inputDestructorRecordBytes + 0x30));
            FadeState_StartFadeOut(Sing_805DC608(), 30);
        }
        switch (((SharedEntryProtocol*)SharedDocBytes())->weatherType) {
        case 2:
            if ((unsigned char)SharedDoc_GetCourseId() != 9 && (unsigned char)SharedDoc_GetCourseId() != 8)
                T_SOUND(golfRuleStrings + kText_rainSequence, 1, golfRuleStrings + kText_ambientSound);
            break;
        case 1:
            if ((unsigned char)SharedDoc_GetCourseId() != 9)
                T_SOUND(String_FormatTemporary(golfRuleStrings + kText_lightningPrepareFormat, rand() % 2 + 1), 0, 0);
        default: {
            unsigned char* r = (unsigned char*)AsCSharedDoc(SharedDocBytes())->selectedCourseData;
            if (r && *(signed char*)((IFF_STRUCT::sCourse*)r)->ambientSound)
                T_SOUND(((IFF_STRUCT::sCourse*)r)->ambientSound, 1, golfRuleStrings + kText_ambientSound);
            else
                Sound_StopByAlias(g_soundManager, golfRuleStrings + kText_ambientSound);
        }
        }
        FloatTween_Start((inputDestructorRecordBytes + 0x3C), -1, kFloat_1_805DEA38, kFloat_0_GolfRule);
        CQuadTree_SetObjectSoundEnabled(CQuadTree_GetInstance(), 1);
        // Playlist must be nonempty: selection evaluates rand()%n.
        unsigned n = AsCGolfRule(self)->bgmNameCount;
        AsCGolfRule(self)->selectedBgmIndex = (unsigned)rand() % n;
        T_MSG(kText_golfBg, 6, 0, 0);
        T_MSG(kText_effect, 0xE, 0, 0);
        T_MSG(kText_effect, 0xC, 0, 0);
        T_MSG(kText_score, 1, 0, 0);
        unsigned char slot = 0;
        if (*(int*)&AsCSharedDoc(SharedDocBytes())->gameMode != 0 ||
            AsCSharedDoc(SharedDocBytes())->golfRuleType != 0) {
            if (AsCSharedDoc(SharedDocBytes())->golfRuleType == 1)
                slot = AsCGolfRule(self)->turn;
            else
                for (slot = 0; slot < 4; slot++)
                    if (!AsGolfDocSnapshot(GolfPlayerEntryByte(slot))->turnOrder)
                        break;
        }
        AsCGolfDoc(GolfDocBytes())->currentPlayer = slot;
        float a[3] = {kFloat_0_GolfRule, AsCGolfBall(GOLF_BALL_BYTES)->m_radius, kFloat_0_GolfRule}, v[3];
        int b[3];
        unsigned char* rec = (unsigned char*)AsCGolfDoc(GolfDocBytes())->holeRecords +
                             (AsCGolfDoc(GolfDocBytes())->currentHoleIndex << 5);
        *(D3*)b = *(D3*)rec;
        __pl__FRC7WVectorRC7WVector(v, b, a);
        AsGolfRuleFrame(self)->holeSpawnPosition = *(WVector*)v;
        if (*(int*)&AsCSharedDoc(SharedDocBytes())->partyGameMode == 3) {
            void* q = PartyItemSlots_GetInstance();
            PartyItemSlots_RunLifecycleHook(q);
            PartyItemSlots_SetSelectedPlayer(q, AsCGolfDoc(GolfDocBytes())->currentPlayer);
            PartyItemSlots_RebuildEffectParameters(q);
        }
        if (SIM_INT_AT(SharedDocBytes(), 0xAD8) == 1) {
            T_MSG(kText_dartsSystem, 3, self + 0x4E8, 0);
            int pos[3], ground[3];
            *(D3*)pos = *(D3*)(self + 0x4E8);
            unsigned char* q = GOLF_BALL_BYTES;
            GetGroundPoint__9CQuadTreeF7WVectorbPib(ground, CQuadTree_GetInstance(), pos, 0, (int)(q + 0x28), 0);
            *(WVector*)(self + 0x4E8) = *(WVector*)ground;
            float off[3] = {kFloat_0_GolfRule, AsCGolfBall(GOLF_BALL_BYTES)->m_radius, kFloat_0_GolfRule}, sum[3];
            __pl__FRC7WVectorRC7WVector(sum, self + 0x4E8, off);
            *(WVector*)(self + 0x4E8) = *(WVector*)sum;
            int cameraPos[3];
            *(D3*)cameraPos = *(D3*)(self + 0x4E8);
            ((CGolfBall*)GOLF_BALL_BYTES)->Reset(cameraPos);
        }
        for (int i = 0; i < AsCGolfDoc(GolfDocBytes())->playerCount; i++) {
            if (SIM_INT_AT(GolfPlayerEntryByte(i), 8) != 3) {
                SIM_INT_AT(GolfPlayerEntryByte(i), 8) = 0;
                *(WVector*)(GolfPlayerEntryByte(i) + 0x1C) = *(WVector*)(self + 0x4E8);
                SIM_INT_AT(GolfPlayerEntryByte(i), 0x178) = 0;
            }
        }
        T_MSG(kText_player, 0x28, 0, 0);
        T_MSG(kText_caddie, 0x1A, 0, 0);
        void* ctl = CQuadTree_GetInstance();
        ((ShotWorldClockView*)ctl)->frame = 0;
        ((ShotWorldClockView*)ctl)->fractionalSeconds = kFloat_0_GolfRule;
        ((CGolfRule*)self)->InitializeAimHeading();
        T_MSG(kText_player, 0x26, (unsigned char)((CGolfRule*)self)->SelectNextPlayer(0), 0);
        T_MSG(kText_caddie, 0x18, (unsigned char)((CGolfRule*)self)->SelectNextPlayer(0), 0);
        T_MSG(kText_caddie, 0xD, 0, 0);
        T_MSG(kText_screen, 0x13, 0, 0);
        T_MSG(kText_npcActor, 8, 0, 0);
        ((CGolfRule*)self)->ResetShotGauge();
        ((CGolfDoc*)GolfDocBytes())->UpdateCamera(0);
        if (*(int*)&AsCSharedDoc(SharedDocBytes())->partyGameMode == 1) {
            T_MSG(kText_player, 6, 0, 0);
            ((CCameraWork*)g_cameraWork)->ResetMode(0xCB);
        } else {
            T_MSG(kText_player, 0x2715, golfRuleStrings + kText_entranceAnimation, 0);
            ((CCameraWork*)g_cameraWork)->ResetMode(0xCB);
        }
        unsigned char* pl = T_CUR;
        pl[AsCGolfDoc(GolfDocBytes())->currentHoleIndex + 0x28] = 1;
        if (AsCSharedDoc(SharedDocBytes())->golfRuleType == 1) {
            unsigned char mode = GOLF_CLUB_CATEGORY;
            T_CUR[0xBD] = mode;
        }
        CWind::GetInstance()->LoadCourseRegions();
        CWind::GetInstance()->InitOverrideFromCourse();
        if (AsCGolfDoc(GolfDocBytes())->courseData)
            CourseData_UpdateWindActors(AsCGolfDoc(GolfDocBytes())->courseData);
        if (SharedDoc_IsPlayerFlag0Set(AsCGolfDoc(GolfDocBytes())->currentPlayer)) {
            SIM_INT_AT(g_rival, 0x98) = 1;
            unsigned char mode = GOLF_CLUB_CATEGORY;
            T_CUR[0xBD] = mode;
        }
        AsCGolfRule(self)->miniGameSystemResult = 0;
        T_MSG(kText_voice, 0, 0, 0);
        return;
    }
    // Publishes turn-delay camera/input setup through EnterTurnStartDelayState.
    // The existing turn-delay processor owns continuation.
    case kStateTurnStartDelay: {
        // Reset camera input and schedule the turn-start delay.
        int caddiePositionWords[3];
        EnterTurnStartDelayState(self, caddiePositionWords);
        return;
    }
    // Publishes aiming UI, ball camera and controller setup through EnterAimState.
    // Aim processing admits input or a prepared computer shot.
    case kStateAim: {
        // Initialize aiming UI and controller state for the current ball.
        float aimCameraMatrix[12];
        EnterAimState(self, ballBytes, aimCameraMatrix);
        return;
    }
    // Reset button swing UI, controller timing and shot flags.
    // Publishes cleared commands/flags, captured spin/curve and button-gauge setup.
    // Button swing processing owns subsequent power/impact input.
    case kStateBeginButtonSwing: {
        // Capture spin/curve and clear directional commands before gauge motion.
        Sound_StopByAlias(g_soundManager, golfRuleStrings + kText_timeSoundAlias);
        CMouseCursor::SetPointerInputEnabled(g_mouseCursor, 0);
        ((CMouseCursor*)g_mouseCursor)->SetRadialMenuEnabled(0);
        T_MSG(kText_score, 1, 0, 0);
        T_MSG(kText_camera, 5, 0, 0);
        T_MSG(kText_letter, 0, 0, 0);
        ++AsCGolfDoc(GolfDocBytes())->shotPhase;
        unsigned char* a = GOLF_BALL_BYTES;
        unsigned char* b = GOLF_BALL_BYTES;
        T_MSG(kText_spin, 1, b + 0x34, a + 0x38);
        T_MSG(kText_spot, 0xB, self + 0xA8, 0);
        AsCGolfBall(GOLF_BALL_BYTES)->m_powerFlags = 0;
        T_CLEAR();
        ((RuleGaugeInputTiming*)self)->heldDirectionTick = 0;
        ((RuleGaugeInputTiming*)self)->minimumPowerFrameSeconds = kFloat_1_805DEA38;
        if (GolfTask_NoOpStatus()) {
            volatile unsigned char enabled = 1;
            volatile float value = AsCGolfDoc(GolfDocBytes())->pangyaTargetX;
        }
        return;
    }
    // Start the advancing button gauge.
    // Publishes advancing gauge state through InitializeButtonGaugeState.
    // Button swing processing owns subsequent gauge input.
    case kStateButtonSwing:
        InitializeButtonGaugeState(self);
        return;
    // Reset shot-gauge presentation for the next attempt.
    // Publishes a fresh timer or retains installed camera by doc byte35C.
    // Next-shot processing owns continuation; playerState 3 sets unknown rule byte91.
    case kStatePrepareNextShot:
        // Reset the next-shot timer or retain installed camera according to document state.
        if (!AsCGolfDoc(GolfDocBytes())->unk_35C) {
            AsGolfRuleFrame(self)->stateTimer = kFloat_0_GolfRule;
            if (AsGolfDocSnapshot(T_CUR)->playerState == 3)
                ((RuleUnresolvedShotBytes*)self)->unk_091 = 1;
            else
                ((CCameraWork*)g_cameraWork)->UninstallCamera();
        }
        return;
    // Publishes transition camera pose and camera mode two.
    // Screen replies in UpdateCameraModeTransition control continuation.
    case kStateCameraTransition: {
        // Set the camera transition pose and publish camera mode two.
        float transitionMatrix[12];
        EnterCameraTransitionState(self, transitionMatrix);
        return;
    }
    // Build button-shot samples and synchronize playback/camera timing.
    case kStateButtonShotAnimation: {
        // Phases 1-3 of BuildShotSimulation classify input/assistance, record manual
        // statistics and handle preview. This entry starts at phase 4 with
        // committed input; it has no preview-only return.
        // Phase 4: writes sample position/velocity/state, curve/spin and impact.
        // This copy retains kFloat_* loads and the original Normalize call ABI.
        // hide aim helpers, seed sample 0 and derive power,
        // yaw and impact corrections. ballBytes=shared ball; firstSampleBytes=sample-0 record;
        // headingRadians=yaw radians, powerFraction=power fraction, launchSpeed=launch speed, yawCorrectionRadians=yaw delta.
        __alloca(0);
        ((CQuadTree*)CQuadTree_GetInstance())->SetSimulationMode(0);
        if (!((ClubSelectionRuntime*)GOLF_SHOT_PARAMETERS_BYTES)->powerShotMode)
            T_MSG(kText_player, 1, 0, 0);
        {
            unsigned char* g = GolfDocBytes();
            unsigned char* pl = (unsigned char*)GolfPlayer_GetEntryByIndex((int)g, AsCGolfDoc(g)->currentPlayer);
            ((ShotPlayerTotalsView*)pl)->accumulatedShotMilliseconds =
                ((ShotPlayerTotalsView*)pl)->accumulatedShotMilliseconds + *(int*)&AsCGolfDoc(g)->elapsedTimeMs;
        }
        IActor_DispatchMessage(self, self, golfRuleStrings + kText_clock, 1, 0, 0, 0, 0);
        IActor_DispatchMessage(self, self, golfRuleStrings + kText_spot, 4, 0, 0, 0, 0);
        IActor_DispatchMessage(self, self, golfRuleStrings + kText_grid, 3, 0, 0, 0, 0);
        yawCorrectionRadians = kFloat_0_GolfRule;
        impactAdjustment = AsCGolfDoc(GolfDocBytes())->shotImpactGaugeX - AsCGolfDoc(GolfDocBytes())->pangyaTargetX;
        powerFraction = (AsCGolfDoc(GolfDocBytes())->shotPowerGaugeX - kFloat_149_805DEA58) / kFloat_360_GolfRule;
        speedScale = ShotParameters_GetBaseLaunchSpeed(InstanceTable_Get());
        launchSpeed = Math_SquareRootApprox(powerFraction) * speedScale;
        if (AsCGolfBall(GOLF_BALL_BYTES)->m_powerFlags & kShotTomahawk)
            launchSpeed = launchSpeed * kFloat_1_3_GolfPowerFlags;
        if (AsCGolfBall(GOLF_BALL_BYTES)->m_powerFlags & kShotSpike)
            launchSpeed = launchSpeed * kFloat_1_3_GolfPowerFlags;
        IActor_DispatchMessage(self, self, golfRuleStrings + kText_player, kPlayerQueryHeadingMessage,
                               (int)&headingRadians, 0, 0, 0);
        if (!SharedDoc_IsPlayerFlag0Set(AsCGolfDoc(GolfDocBytes())->currentPlayer))
            headingRadians = headingRadians + AsGolfRuleFrame(self)->launchHeadingOffset;
        firstSampleBytes = (unsigned char*)((CGolfBall*)ballBytes)->GetOrAllocatePreDataAt(0);
        ((ShotSample*)firstSampleBytes)->eventKind = 0;
        ((ShotSample*)firstSampleBytes)->m_pos = AsCGolfBall(ballBytes)->m_pos;
        // 0x80155BC8: Mode 3 skips ordinary shot setup before the common state initialization.
        do {
            if (GOLF_CLUB_CATEGORY == 3) {
                ((ShotSample*)firstSampleBytes)->motionState = 3;
                break;
            }
            ((ShotSample*)firstSampleBytes)->motionState = 1;
            if (((ClubSelectionRuntime*)GOLF_SHOT_PARAMETERS_BYTES)->powerShotMode != 0) {
                Sound_PlayByName(g_soundManager, golfRuleStrings + kText_powershot_ball_effect, 0, 0, 0, 0,
                                 kFloat_0_5_GolfRule, kFloat_200_GolfRule, kFloat_1_805DEA38);
                IActor_DispatchMessage(self, self, golfRuleStrings + kText_effect, 0x12, 1, 0, 0, 0);
            }
            {
                unsigned char* g = GolfDocBytes();
                switch (AsCGolfDoc(g)->impactGrade) {
                case 4:
                    if (AsCGolfDoc(g)->pangyaAssistState == 2) {
                        ((ShotAccountStatisticsView*)(SharedDocBytes() + AsCGolfDoc(g)->currentPlayer * 0x1D4))
                            ->assistedPangyas =
                            ((ShotAccountStatisticsView*)(SharedDocBytes() + AsCGolfDoc(g)->currentPlayer * 0x1D4))
                                ->assistedPangyas +
                            1;
                    }
                    launchSpeed = launchSpeed *
                                  Math_SquareRootApprox(((CQuadTree*)CQuadTree_GetInstance())->GetMaxPowerOnGround());
                    yawCorrectionRadians = FP_FNMSUBS(kFloat_0_34906584_GolfRule, AsCGolfBall(ballBytes)->m_initCurve,
                                                      yawCorrectionRadians);
                    impactDirection = 0;
                    break;
                case 3:
                    if (impactAdjustment > kFloat_0_GolfRule) {
                        normalizedImpactRatio =
                            impactAdjustment /
                            ((kFloat_149_805DEA58 + ShotParameters_GetGoodImpactHalfWidth(InstanceTable_Get())) -
                             AsCGolfDoc(g)->pangyaTargetX);
                    } else {
                        normalizedImpactRatio =
                            impactAdjustment /
                            (AsCGolfDoc(g)->pangyaTargetX -
                             (kFloat_149_805DEA58 - ShotParameters_GetGoodImpactHalfWidth(InstanceTable_Get())));
                    }
                    speedScale =
                        (normalizedImpactRatio > kFloat_0_GolfRule) ? normalizedImpactRatio : -normalizedImpactRatio;
                    absoluteAdjustment =
                        (normalizedImpactRatio > kFloat_0_GolfRule) ? normalizedImpactRatio : -normalizedImpactRatio;
                    speedScale = speedScale * ((CQuadTree*)CQuadTree_GetInstance())->GetMinPowerOnGround();
                    speedScale = FP_FMADDS((kFloat_1_805DEA38 - absoluteAdjustment),
                                           ((CQuadTree*)CQuadTree_GetInstance())->GetMaxPowerOnGround(), speedScale);
                    launchSpeed = launchSpeed * Math_SquareRootApprox(speedScale);
                    {
                        float old = AsCGolfBall(ballBytes)->m_initCurve;
                        AsCGolfBall(ballBytes)->m_initCurve =
                            FP_FNMSUBS(kFloat_0_2_GolfRule, normalizedImpactRatio, old);
                        yawCorrectionRadians = FP_FNMSUBS(kFloat_0_34906584_GolfRule, old, yawCorrectionRadians);
                    }
                    if (impactAdjustment > kFloat_0_GolfRule)
                        impactDirection = 1;
                    else
                        impactDirection = -1;
                    break;
                case 2:
                    absoluteAdjustment = AsCGolfDoc(g)->shotImpactGaugeX - kFloat_149_805DEA58;
                    absoluteAdjustment =
                        (absoluteAdjustment > kFloat_0_GolfRule) ? absoluteAdjustment : -absoluteAdjustment;
                    impactAdjustment = ShotParameters_GetGoodImpactHalfWidth(InstanceTable_Get());
                    impactAdjustment = ShotParameters_GetPoorImpactHalfWidth(InstanceTable_Get()) - impactAdjustment;
                    normalizedImpactRatio =
                        (absoluteAdjustment - ShotParameters_GetGoodImpactHalfWidth(InstanceTable_Get())) /
                        impactAdjustment;
                    impactAdjustment =
                        normalizedImpactRatio * ((CQuadTree*)CQuadTree_GetInstance())->GetMinPowerOnGround();
                    launchSpeed = launchSpeed *
                                  Math_SquareRootApprox(FP_FMADDS(
                                      (kFloat_1_805DEA38 - normalizedImpactRatio),
                                      ((CQuadTree*)CQuadTree_GetInstance())->GetMinPowerOnGround(), impactAdjustment));
                    AsCGolfBall(ballBytes)->m_initCurve = kFloat_0_2_GolfRule;
                    AsCGolfBall(ballBytes)->m_initSpin = kFloat_0_GolfRule;
                    if (AsCGolfDoc(GolfDocBytes())->shotImpactGaugeX > kFloat_149_805DEA58) {
                        impactDirection = 2;
                        AsCGolfBall(ballBytes)->m_initCurve =
                            AsCGolfBall(ballBytes)->m_initCurve * kFloat_neg1_GolfRule;
                        yawCorrectionRadians =
                            FP_FNMSUBS(kFloat_0_17453292_GolfRule, normalizedImpactRatio, yawCorrectionRadians);
                    } else {
                        impactDirection = -2;
                        yawCorrectionRadians =
                            FP_FMADDS(kFloat_0_17453292_GolfRule, normalizedImpactRatio, yawCorrectionRadians);
                    }
                    break;
                case 0:
                case 1:
                    AsCGolfBall(ballBytes)->m_initCurve = kFloat_0_GolfRule;
                    AsCGolfBall(ballBytes)->m_initSpin = kFloat_0_GolfRule;
                    switch (GOLF_CLUB_CATEGORY) {
                    case 0:
                        if (AsCGolfDoc(GolfDocBytes())->shotImpactGaugeX > AsCGolfDoc(GolfDocBytes())->pangyaTargetX) {
                            Vec3_Zero(&((ShotSample*)firstSampleBytes)->velocity);
                            ((ShotSample*)firstSampleBytes)->motionState = 0;
                        } else {
                            unsigned t =
                                ((ShotAccountStatisticsView*)(SharedDocBytes() +
                                                              AsCGolfDoc(GolfDocBytes())->currentPlayer * 0x1D4))
                                    ->characterId -
                                0x4000000;
                            if (t == 1 || t == 6) {
                                Vec3_Zero(&((ShotSample*)firstSampleBytes)->velocity);
                                ((ShotSample*)firstSampleBytes)->motionState = 0;
                            } else {
                                woodMissRollingVelocity[0] = kFloat_0_GolfRule;
                                woodMissRollingVelocity[1] = kFloat_0_GolfRule;
                                woodMissRollingVelocity[2] = kFloat_neg15_GolfRule;
                                ((ShotSample*)firstSampleBytes)->velocity = *(WVector*)woodMissRollingVelocity;
                                ((ShotSample*)firstSampleBytes)->motionState = 3;
                                yawCorrectionRadians = yawCorrectionRadians + kFloat_0_69813168_GolfRule;
                            }
                        }
                        break;
                    case 1:
                        if (AsCGolfDoc(GolfDocBytes())->shotImpactGaugeX > AsCGolfDoc(GolfDocBytes())->pangyaTargetX) {
                            ironMissRollingVelocity[0] = kFloat_0_GolfRule;
                            ironMissRollingVelocity[1] = kFloat_0_GolfRule;
                            ironMissRollingVelocity[2] = kFloat_neg20_GolfRule;
                            ((ShotSample*)firstSampleBytes)->velocity = *(WVector*)ironMissRollingVelocity;
                            ((ShotSample*)firstSampleBytes)->motionState = 3;
                        } else {
                            Matrix_SetAxisRotation(ironMissPitch, kFloat_1_7453293_GolfRule, 0);
                            __ml__FfRC7WVector(ironMissAirborneVelocity, ironMissPitch + 6, kFloat_50_GolfRule);
                            ((ShotSample*)firstSampleBytes)->velocity = *(WVector*)ironMissAirborneVelocity;
                            ((ShotSample*)firstSampleBytes)->motionState = 1;
                        }
                        {
                            unsigned t =
                                ((ShotAccountStatisticsView*)(SharedDocBytes() +
                                                              AsCGolfDoc(GolfDocBytes())->currentPlayer * 0x1D4))
                                    ->characterId -
                                0x4000000;
                            if (t == 6) {
                                Vec3_Zero(firstSampleBytes + 0xC);
                                ((ShotSample*)firstSampleBytes)->motionState = 0;
                            }
                        }
                        break;
                    case 2:
                        if (AsCGolfDoc(GolfDocBytes())->shotImpactGaugeX > AsCGolfDoc(GolfDocBytes())->pangyaTargetX) {
                            wedgeMissRollingVelocity[0] = kFloat_0_GolfRule;
                            wedgeMissRollingVelocity[1] = kFloat_0_GolfRule;
                            wedgeMissRollingVelocity[2] = kFloat_neg5_GolfRule;
                            ((ShotSample*)firstSampleBytes)->velocity = *(WVector*)wedgeMissRollingVelocity;
                            ((ShotSample*)firstSampleBytes)->motionState = 3;
                        } else {
                            Matrix_SetAxisRotation(wedgeMissPitch, kFloat_1_7453293_GolfRule, 0);
                            __ml__FfRC7WVector(wedgeMissAirborneVelocity, wedgeMissPitch + 6, kFloat_20_GolfRule);
                            ((ShotSample*)firstSampleBytes)->velocity = *(WVector*)wedgeMissAirborneVelocity;
                            ((ShotSample*)firstSampleBytes)->motionState = 1;
                        }
                        break;
                    default:
                        break;
                    }
                    if (GOLF_CLUB_CATEGORY != 2) {
                        unsigned t =
                            SIM_INT_AT(SharedDocBytes() + AsCGolfDoc(GolfDocBytes())->currentPlayer * 0x1D4, 0x120) -
                            0x4000000;
                        if (t == 5) {
                            Vec3_Zero(&((ShotSample*)firstSampleBytes)->velocity);
                            ((ShotSample*)firstSampleBytes)->motionState = 0;
                        }
                    }
                    if (GOLF_CLUB_CATEGORY != 2) {
                        unsigned t =
                            SIM_INT_AT(SharedDocBytes() + AsCGolfDoc(GolfDocBytes())->currentPlayer * 0x1D4, 0x120) -
                            0x4000000;
                        if (t == 7) {
                            Vec3_Zero(firstSampleBytes + 0xC);
                            ((ShotSample*)firstSampleBytes)->motionState = 0;
                        }
                    }
                    if (impactAdjustment > kFloat_0_GolfRule)
                        impactDirection = 3;
                    else
                        impactDirection = -3;
                    break;
                default:
                    break;
                }
            }
            {
                unsigned char* g = GolfDocBytes();
                int st = AsCGolfDoc(g)->impactGrade;
                if (st > 1 && AsCGolfDoc(g)->powerShotMeterOverride == 0) {
                    if (!(st == 4 && ((GolfDocShotBytes*)g)->unk_2D8 != 0 &&
                          ((ClubSelectionRuntime*)GOLF_SHOT_PARAMETERS_BYTES)->powerShotMode != 0 &&
                          (AsCGolfBall(ballBytes)->m_powerFlags & kSpecialShotMask) == 0) &&
                        !SharedDoc_IsPlayerFlag0Set(AsCGolfDoc(GolfDocBytes())->currentPlayer)) {
                        IActor_DispatchMessage(self, self, golfRuleStrings + kText_screen, 0x23, (int)screenVectorReply,
                                               0, 0, 0);
                        if (AsCGolfBall(ballBytes)->m_powerFlags & kShotSpike)
                            AsCGolfBall(ballBytes)->m_initCurve = FP_FMADDS(kFloat_0_15_GolfRule, screenVectorReply[0],
                                                                            AsCGolfBall(ballBytes)->m_initCurve);
                        else
                            AsCGolfBall(ballBytes)->m_initCurve = FP_FMADDS(kFloat_0_5_GolfRule, screenVectorReply[0],
                                                                            AsCGolfBall(ballBytes)->m_initCurve);
                    }
                }
            }
            IActor_DispatchMessage(self, self, golfRuleStrings + kText_screen, 0xB, 0, 0, 0, 0);
            IActor_DispatchMessage(self, self, golfRuleStrings + kText_caddie, 0x10, impactDirection, 0, 0, 0);
        } while (0);

        AsCGolfBall(ballBytes)->m_initCurve = AsCGolfBall(ballBytes)->m_initCurve * kFloat_neg1_GolfRule;
        // Phase 5: writes launch matrices and sample velocity (as in BuildShotSimulation).
        yawCorrectionRadians = yawCorrectionRadians * kFloat_neg1_GolfRule;
        if (AsCGolfDoc(GolfDocBytes())->impactGrade > 1) {
            absoluteAdjustment = AsCGolfBall(ballBytes)->m_initCurve;
            absoluteAdjustment = (absoluteAdjustment > kFloat_0_GolfRule) ? absoluteAdjustment : -absoluteAdjustment;
            Matrix_SetAxisRotation(launchYaw, headingRadians + yawCorrectionRadians, 1);
            Matrix_SetAxisRotation(launchPitch, ClubSelection_GetLaunchAngle(InstanceTable_Get(), 0), 0);
            __ml__FfRC7WVector(pitchAdjustedVelocity, launchPitch + 6,
                               launchSpeed * FP_FMADDS(kFloat_0_1_GolfRule, absoluteAdjustment, kFloat_1_805DEA38));
            Matrix_TransformPoint(worldLaunchVelocity, pitchAdjustedVelocity, launchYaw);
            ((ShotSample*)firstSampleBytes)->velocity = *(WVector*)worldLaunchVelocity;
        }
        // Phase 6: writes replay/checkpoint inputs, clock, flags and live velocity.
        CaptureShotReplayInputs(self, ballBytes, headingRadians, *(WVector*)shotWind);
        if (s_cameraDirectedShotEnabled != 0) {
            AsCGolfBall(ballBytes)->m_pos = *(WVector*)(s_cameraTransform + 0x24);
            __ml__FRC7WVectorf(cameraDirectedVelocity, s_cameraTransform + 0x18, kFloat_230_GolfRule);
            ((ShotSample*)firstSampleBytes)->velocity = *(WVector*)cameraDirectedVelocity;
            ((ShotSample*)firstSampleBytes)->motionState = 1;
            ((BallGroundTriangleView*)ballBytes)->groundTriangleHandle = -1;
        }
        IActor_DispatchMessage(self, self, golfRuleStrings + kText_player, kPlayerQueryShotKeySecondsMessage,
                               (int)&shotKeySeconds, 0, 0, 0);
        if (GolfTask_NoOpStatus() == 0) {
            IActor_DispatchMessage(self, self, golfRuleStrings + kText_player, 0xC, 0, 0, 0, 0);
            AsGolfRuleFrame(self)->stateEntryPending = 1;
            ((RuleUnresolvedShotBytes*)self)->unk_094 = AsCGolfBall(ballBytes)->m_groundType != 2;
        }
        SIM_INT_AT(self, 0x474) = SIM_INT_AT(CQuadTree_GetInstance(), 0x21AC);
        AsGolfRuleFrame(self)->playbackFrame = SIM_INT_AT(self, 0x474) + 0x1E;
        SIM_INT_AT(self, 0x47C) =
            AsGolfRuleFrame(self)->playbackFrame + Runtime_ConvertDoubleToUnsigned(kFloat_30_GolfRule * shotKeySeconds);
        {
            int saved = SIM_INT_AT(self, 0x47C);
            unsigned char* ctl = (unsigned char*)CQuadTree_GetInstance();
            ((ShotWorldClockView*)ctl)->frame = saved;
            ((ShotWorldClockView*)ctl)->fractionalSeconds = kFloat_0_GolfRule;
        }
        (*(WVector*)(ballBytes + kBallVelocityByteOffset)) = ((ShotSample*)firstSampleBytes)->velocity;
        {
            effectStateBytes = (unsigned char*)PartyItemSlots_GetInstance();
            unsigned char* key = AsCGolfBall(ballBytes)->m_holeInCrypt;
            struct {
                unsigned char* key;
                int sp14;
            } kk;
            unsigned char* kb = (unsigned char*)&kk;
            kk.key = key;
            kk.sp14 = 0;
            AsCGolfBall(ballBytes)->m_velocity.y =
                AsCGolfBall(ballBytes)->m_velocity.y *
                (kFloat_1_805DEA38 + ((ShotAccessoryModifiers*)effectStateBytes)->verticalVelocityBonus);
            AsCGolfBall(ballBytes)->m_state = ((ShotSample*)firstSampleBytes)->motionState;
            AsCGolfBall(ballBytes)->m_flags = AsCGolfBall(ballBytes)->m_powerFlags;
            AsGolfRuleFrame(self)->doublePowerCameraReset = 1;
            AsGolfRuleFrame(self)->animationStarted = 0;
            AsGolfRuleFrame(self)->presentationStep = 0;
            AsCGolfBall(ballBytes)->m_lie = 0;
            SIM_INT_AT(ballBytes, 0x1989C) = ~SIM_INT_AT(ballBytes, 0x198A4);
            for (int i = 0; i < sizeof(kk.key); i++) {
                ballBytes[0x198A0 + i] = key[i] ^ kb[i];
            }
        }
        {
            unsigned char* pl = (unsigned char*)GolfPlayer_GetEntryByIndex((int)GolfDocBytes(),
                                                                           AsCGolfDoc(GolfDocBytes())->currentPlayer);
            AsGolfDocSnapshot(pl)->unk_AC = -1;
            pl = (unsigned char*)GolfPlayer_GetEntryByIndex((int)GolfDocBytes(),
                                                            AsCGolfDoc(GolfDocBytes())->currentPlayer);
            ((ShotPlayerTotalsView*)pl)->clearedShotWordB4 = 0;
            if (AsCGolfBall(GOLF_BALL_BYTES)->m_state != 0) {
                pl = (unsigned char*)GolfPlayer_GetEntryByIndex((int)GolfDocBytes(),
                                                                AsCGolfDoc(GolfDocBytes())->currentPlayer);
                AsGolfDocSnapshot(pl)->unk_AE = 0;
            }
        }
        // Phase 7: writes collision/sample/contact fields and spin/curve (as in BuildShotSimulation).
        ResetShotCollisionAndSamples(ballBytes);
        unsigned char saved59 = AsCGolfBall(ballBytes)->m_groundType;
        if (AsCGolfDoc(GolfDocBytes())->impactGrade == kImpactMiss) {
            AsCGolfBall(ballBytes)->m_spin = kFloat_0_GolfRule;
            AsCGolfBall(ballBytes)->m_curve = kFloat_0_GolfRule;
        } else {
            if (((ClubSelectionRuntime*)GOLF_SHOT_PARAMETERS_BYTES)->shotCommandKind == 4) {
                normalizedImpactRatio =
                    kFloat_0_5_GolfRule * ((float (*)(int, int, int))CSharedDoc_ComputeAuxPartModifier)(
                                              (int)SharedDocBytes(), AsCGolfDoc(GolfDocBytes())->currentPlayer, 0);
                float v = (float)((ShotPlayerTotalsView*)GolfPlayer_GetEntryByIndex(
                                      (int)GolfDocBytes(), AsCGolfDoc(GolfDocBytes())->currentPlayer))
                              ->spinStat;
                float t = (normalizedImpactRatio + v) / kFloat_170_GolfRule;
                t = (kFloat_0_GolfRule < t) ? t : kFloat_0_GolfRule;
                AsCGolfBall(ballBytes)->m_spin = powerFraction * (powerFraction * (kFloat_1_5_GolfRule + t));
            } else {
                AsCGolfBall(ballBytes)->m_spin = kFloat_0_GolfRule;
            }
            double curveSetting = AsCGolfBall(ballBytes)->m_initCurve;
            AsCGolfBall(ballBytes)->m_curve = (float)curveSetting * powerFraction;
        }
        gtl_Stopwatch_Initialize(simulationStopwatch);
        gtl_Stopwatch_Start(simulationStopwatch);
        gtl_Stopwatch_Reset(simulationStopwatch);
        // Shot phase D: record fixed 30-Hz physics samples; preserve the
        // 900/1800-sample stall/timeout policy and special-shot milestones.
        // Phase 8: writes fixed-step samples, integrated ball and event indices.
        while (AsCGolfBall(ballBytes)->m_state != 0) {
            unsigned char* obj2 = (unsigned char*)((CGolfBall*)ballBytes)->NewPreData();
            ((ShotSample*)obj2)->eventSoundName[0] = 0;
            ((ShotSample*)obj2)->eventKind = 0;
            ((ShotSample*)obj2)->eventSubkind = 0;
            ((CQuadTree*)CQuadTree_GetInstance())->UpdateAnimatedObjects(kGolfSimulationStepSeconds, 1);
            ((CQuadTree*)CQuadTree_GetInstance())->StepBallSimulation(kGolfSimulationStepSeconds);
            if (((ClubSelectionRuntime*)GOLF_SHOT_PARAMETERS_BYTES)->shotCommandKind == 4) {
                int m = AsCGolfBall(ballBytes)->m_flags;
                if (m & kShotCobra) {
                    if (AsCGolfBall(ballBytes)->m_eventPreDataIndex[kBallCobraDistanceSample] < 0) {
                        absoluteAdjustment = AsCGolfBall(ballBytes)->m_radius;
                        *(D3*)waterTestPosition = *(D3*)ballBytes;
                        float r = ((CQuadTree*)CQuadTree_GetInstance())
                                      ->ResolveGroundHeight((Vec3f*)waterTestPosition,
                                                            kFloat_2_805DEA84 * absoluteAdjustment);
                        float c = AsCGolfBall(ballBytes)->m_pos.y;
                        r = (c < r) ? r : c;
                        AsCGolfBall(ballBytes)->m_pos.y = r;
                    } else {
                        AsCGolfBall(ballBytes)->m_spin = AsCGolfBall(ballBytes)->m_spin - kGolfSimulationStepSeconds;
                    }
                } else if (m & kShotSpike) {
                    if (AsCGolfBall(ballBytes)->m_eventPreDataIndex[kBallSpikeMotionSample] < 0) {
                        absoluteAdjustment = AsCGolfBall(ballBytes)->m_radius;
                        *(D3*)outsideTestPosition = *(D3*)ballBytes;
                        float r = ((CQuadTree*)CQuadTree_GetInstance())
                                      ->ResolveGroundHeight((Vec3f*)outsideTestPosition,
                                                            kFloat_2_805DEA84 * absoluteAdjustment);
                        float c = AsCGolfBall(ballBytes)->m_pos.y;
                        r = (c < r) ? r : c;
                        AsCGolfBall(ballBytes)->m_pos.y = r;
                    } else {
                        AsCGolfBall(ballBytes)->m_spin = AsCGolfBall(ballBytes)->m_spin - kGolfSimulationStepSeconds;
                    }
                } else {
                    if (AsCGolfBall(ballBytes)->m_eventPreDataIndex[1] < 0 &&
                        ((CQuadTree*)CQuadTree_GetInstance())->GetObjColNum() == 0) {
                        AsCGolfBall(ballBytes)->m_spin = FP_FNMSUBS(
                            kGolfSimulationStepSeconds,
                            FP_FNMSUBS(kFloat_0_1_GolfRule, AsCGolfBall(ballBytes)->m_initSpin, kFloat_0_5_GolfRule),
                            AsCGolfBall(ballBytes)->m_spin);
                    } else {
                        AsCGolfBall(ballBytes)->m_spin = AsCGolfBall(ballBytes)->m_spin - kFloat_0_0033333336_GolfRule;
                    }
                }
            }
            {
                int t64 = AsCGolfBall(ballBytes)->m_preDataCount;
                if (t64 >= kShotMaximumSamples ||
                    (t64 >= kShotStallCheckSamples &&
                     (((*(WVector*)(ballBytes + kBallVelocityByteOffset))).Magnitude() < kFloat_0_1_GolfRule ||
                      ((CQuadTree*)CQuadTree_GetInstance())->GetObjColNum() >= kShotMaximumCollisionEvents ||
                      (int)((CQuadTree*)CQuadTree_GetInstance())->GetMeshIndex(ballBytes, 0) == -1))) {
                    Vec3_Zero(ballBytes + kBallVelocityByteOffset);
                    *(D3*)groundQueryPosition = *(D3*)ballBytes;
                    GetGroundPoint__9CQuadTreeF7WVectorbPib(groundRecoveryPosition, CQuadTree_GetInstance(),
                                                            groundQueryPosition, 0, 0, 0);
                    AsCGolfBall(ballBytes)->m_pos = *(WVector*)groundRecoveryPosition;
                    AsCGolfBall(ballBytes)->m_pos.y =
                        AsCGolfBall(ballBytes)->m_pos.y + AsCGolfBall(ballBytes)->m_radius;
                    AsCGolfBall(ballBytes)->m_state = 0;
                }
            }
            ((ShotSample*)obj2)->m_pos = AsCGolfBall(ballBytes)->m_pos;
            ((ShotSample*)obj2)->velocity = (*(WVector*)(ballBytes + kBallVelocityByteOffset));
            Vec2_Copy(((ShotSample*)obj2)->rotation, ballBytes + 0x2C);
            ((ShotSample*)obj2)->motionState = AsCGolfBall(ballBytes)->m_state;
            if (AsCGolfBall(ballBytes)->m_state == 0)
                AsCGolfBall(ballBytes)->m_flags = AsCGolfBall(ballBytes)->m_flags & ~0x70;
            if (AsCGolfBall(ballBytes)->m_state == 1 && AsCGolfBall(ballBytes)->m_velocity.y < kFloat_0_GolfRule &&
                AsCGolfBall(ballBytes)->m_eventPreDataIndex[kBallFirstDescendingSample] < 0)
                AsCGolfBall(ballBytes)->m_eventPreDataIndex[kBallFirstDescendingSample] =
                    AsCGolfBall(ballBytes)->m_preDataCount;
            if (AsCGolfBall(ballBytes)->m_eventPreDataIndex[kBallTomahawkEndSample] < 0 &&
                (AsCGolfBall(ballBytes)->m_powerFlags & kShotTomahawk) &&
                !(AsCGolfBall(ballBytes)->m_flags & kShotTomahawk))
                AsCGolfBall(ballBytes)->m_eventPreDataIndex[kBallTomahawkEndSample] =
                    AsCGolfBall(ballBytes)->m_preDataCount;
            if (AsCGolfBall(ballBytes)->m_flags & kShotCobra) {
                if (AsCGolfBall(ballBytes)->m_preDataCount == 1) {
                    AsCGolfBall(ballBytes)->m_velocity.y = kFloat_0_GolfRule;
                    __ml__FfRC7WVector(cobraVelocity, Normalize__7WVectorFv(ballBytes + kBallVelocityByteOffset),
                                       launchSpeed);
                    (*(WVector*)(ballBytes + kBallVelocityByteOffset)) = *(WVector*)cobraVelocity;
                }
                __mi__FRC7WVectorRC7WVector(cobraTravelXZ, ballBytes, PreData__9CGolfBallFi(ballBytes, 0));
                cobraTravelXZ[1] = kFloat_0_GolfRule;
                if (AsCGolfBall(ballBytes)->m_eventPreDataIndex[kBallCobraDistanceSample] < 0) {
                    impactAdjustment = GolfParameter_ResolveFloat(InstanceTable_Get(), 0);
                    impactAdjustment = kFloat_3_2_GolfRule * FP_FMSUBS(Math_SquareRootApprox(powerFraction),
                                                                       impactAdjustment, kFloat_100_GolfRule);
                    if ((*(WVector*)cobraTravelXZ).Magnitude() > impactAdjustment) {
                        AsCGolfBall(ballBytes)->m_eventPreDataIndex[kBallCobraDistanceSample] =
                            AsCGolfBall(ballBytes)->m_preDataCount;
                        ((*(WVector*)(ballBytes + kBallVelocityByteOffset))).Normalize();
                        switch (((ClubSelectionRuntime*)GOLF_SHOT_PARAMETERS_BYTES)->selectedClub->specialShotVariant) {
                        case 0:
                            (*(WVector*)(ballBytes + kBallVelocityByteOffset)) *= kFloat_74_GolfRule;
                            break;
                        case 1:
                            (*(WVector*)(ballBytes + kBallVelocityByteOffset)) *= kFloat_76_GolfRule;
                            break;
                        case 2:
                            (*(WVector*)(ballBytes + kBallVelocityByteOffset)) *= kFloat_80_GolfRule;
                            break;
                        default:
                            break;
                        }
                        (*(WVector*)(ballBytes + kBallVelocityByteOffset)) *= Math_SquareRootApprox(powerFraction);
                        AsCGolfBall(ballBytes)->m_spin = kFloat_2_5_GolfRule;
                    }
                }
            }
            if (AsCGolfBall(ballBytes)->m_eventPreDataIndex[kBallSpikeEndSample] < 0 &&
                (AsCGolfBall(ballBytes)->m_powerFlags & kShotSpike) &&
                !(AsCGolfBall(ballBytes)->m_flags & kShotSpike) &&
                AsCGolfBall(ballBytes)->m_eventPreDataIndex[kBallSpikeDescentSample] > 0) {
                AsCGolfBall(ballBytes)->m_eventPreDataIndex[kBallSpikeEndSample] =
                    AsCGolfBall(ballBytes)->m_preDataCount;
            }
            if (AsCGolfBall(ballBytes)->m_flags & kShotSpike) {
                if (AsCGolfBall(ballBytes)->m_preDataCount == 1) {
                    AsCGolfBall(ballBytes)->m_velocity.y = kFloat_0_GolfRule;
                    __ml__FfRC7WVector(spikeVelocity, Normalize__7WVectorFv(ballBytes + kBallVelocityByteOffset),
                                       launchSpeed);
                    (*(WVector*)(ballBytes + kBallVelocityByteOffset)) = *(WVector*)spikeVelocity;
                }
                __mi__FRC7WVectorRC7WVector(spikeTravelXZ, ballBytes, PreData__9CGolfBallFi(ballBytes, 0));
                spikeTravelXZ[1] = kFloat_0_GolfRule;
                if (AsCGolfBall(ballBytes)->m_eventPreDataIndex[kBallSpikeMotionSample] < 0 &&
                    (*(WVector*)spikeTravelXZ).Magnitude() > kFloat_0_GolfRule) {
                    AsCGolfBall(ballBytes)->m_eventPreDataIndex[kBallSpikeMotionSample] =
                        AsCGolfBall(ballBytes)->m_preDataCount;
                    ((*(WVector*)(ballBytes + kBallVelocityByteOffset))).Normalize();
                    (*(WVector*)(ballBytes + kBallVelocityByteOffset)) *= kFloat_72_5_GolfRule;
                    (*(WVector*)(ballBytes + kBallVelocityByteOffset)) *=
                        kFloat_2_805DEA84 * Math_SquareRootApprox(powerFraction);
                    AsCGolfBall(ballBytes)->m_spin = kFloat_3_1_GolfRule;
                }
                if (AsCGolfBall(ballBytes)->m_eventPreDataIndex[kBallFirstDescendingSample] >= 0 &&
                    AsCGolfBall(ballBytes)->m_preDataCount >
                        AsCGolfBall(ballBytes)->m_eventPreDataIndex[kBallFirstDescendingSample] +
                            kSpikeDescentDelaySamples &&
                    AsCGolfBall(ballBytes)->m_eventPreDataIndex[kBallSpikeDescentSample] < 0) {
                    AsCGolfBall(ballBytes)->m_eventPreDataIndex[kBallSpikeDescentSample] =
                        AsCGolfBall(ballBytes)->m_preDataCount;
                    switch (((ClubSelectionRuntime*)GOLF_SHOT_PARAMETERS_BYTES)->selectedClub->specialShotVariant) {
                    case 0: {
                        float speed = ShotParameters_GetBaseLaunchSpeed(InstanceTable_Get());
                        float x = Math_SquareRootApprox(powerFraction) * speed;
                        normalModeBaseSpeed = x;
                        const float* p = &Minimum54(normalModeBaseSpeed, s_golfRuleMode0SpeedCap344);
                        AsCGolfBall(ballBytes)->m_velocity.y =
                            FP_FNMSUBS(kFloat_21_5_GolfRule, ((kFloat_344_GolfRule - *p) / kFloat_112_GolfRule),
                                       kFloat_neg8_GolfRule);
                        break;
                    }
                    case 1: {
                        float speed = ShotParameters_GetBaseLaunchSpeed(InstanceTable_Get());
                        float x = Math_SquareRootApprox(powerFraction) * speed;
                        modeOneBaseSpeed = x;
                        const float* p = &Minimum54(modeOneBaseSpeed, s_golfRuleStateEntryMode1SpeedCap306);
                        AsCGolfBall(ballBytes)->m_velocity.y =
                            FP_FNMSUBS(kFloat_20_5_GolfRule, ((kFloat_306_GolfRule - *p) / kFloat_105_GolfRule),
                                       kFloat_neg10_299999_GolfRule);
                        break;
                    }
                    case 2: {
                        float speed = ShotParameters_GetBaseLaunchSpeed(InstanceTable_Get());
                        float x = Math_SquareRootApprox(powerFraction) * speed;
                        modeTwoBaseSpeed = x;
                        const float* p = &Minimum54(modeTwoBaseSpeed, s_golfRuleMode2LaunchSpeedCap273);
                        AsCGolfBall(ballBytes)->m_velocity.y =
                            FP_FNMSUBS(kFloat_20_2_GolfRule, ((kFloat_273_GolfRule - *p) / kFloat_100_GolfRule),
                                       kFloat_neg10_799999_GolfRule);
                        break;
                    }
                    default:
                        break;
                    }
                    (*(WVector*)(ballBytes + kBallVelocityByteOffset)) *=
                        kFloat_7_GolfRule * Math_SquareRootApprox(powerFraction);
                    AsCGolfBall(ballBytes)->m_spin = AsCGolfBall(ballBytes)->m_initSpin;
                }
            }
        }
        // Shot phase E: retain results, rewind animated objects, append
        // history and select the next playable position or recovery point.
        // Phase 9: writes clock, player recovery, checkpoint and history.
        gtl_Stopwatch_GetElapsedSeconds(simulationStopwatch);
        AsCGolfBall(ballBytes)->m_groundType = saved59;
        AsCGolfBall(ballBytes)->m_flags = AsCGolfBall(ballBytes)->m_powerFlags;
        IActor_DispatchMessage(self, self, golfRuleStrings + kText_npcActor, 4, 0, 0, 0, 0);
        if (((CQuadTree*)CQuadTree_GetInstance())->GetObjColNum() > 0)
            AsCGolfBall(ballBytes)->m_eventPreDataIndex[3] = -1;
        {
            int saved = AsGolfRuleFrame(self)->simulationStartFrame - 1;
            unsigned char* ctl = (unsigned char*)CQuadTree_GetInstance();
            ((ShotWorldClockView*)ctl)->frame = saved;
            ((ShotWorldClockView*)ctl)->fractionalSeconds = kFloat_0_GolfRule;
        }
        ((CQuadTree*)CQuadTree_GetInstance())->UpdateAnimatedObjects(kGolfSimulationStepSeconds, 1);
        AsCGolfBall(ballBytes)->m_curve = AsCGolfBall(ballBytes)->m_initCurve;
        AsCGolfBall(ballBytes)->m_spin = AsCGolfBall(ballBytes)->m_initSpin;
        ((BallShotTimingView*)ballBytes)->simulationSeconds =
            kGolfSimulationStepSeconds * (float)AsCGolfBall(ballBytes)->m_preDataCount;
        if (AsCGolfBall(ballBytes)->m_recoveryRequired == 0)
            AsCGolfBall(ballBytes)->m_recoveryRequired =
                CourseData_IsPointOutsidePlayableRegion(AsCGolfDoc(GolfDocBytes())->courseData, ballBytes);
        ((CGolfRule*)self)->ClassifyShotPresentation();
        IActor_DispatchMessage(self, self, golfRuleStrings + kText_player, 0xB,
                               (int)PreData__9CGolfBallFi(ballBytes, AsCGolfBall(ballBytes)->m_preDataCount), 0, 0, 0);
        ((RuleShotCheckpointView*)(self + AsCGolfDoc(GolfDocBytes())->currentPlayer * 0x54))->playerPosition =
            *(WVector*)AsGolfDocSnapshot(
                 GolfPlayer_GetEntryByIndex((int)GolfDocBytes(), AsCGolfDoc(GolfDocBytes())->currentPlayer))
                 ->ballPosition;
        ((RuleShotCheckpointView*)(self + AsCGolfDoc(GolfDocBytes())->currentPlayer * 0x54))->headingRadians =
            headingRadians;
        ((RuleShotCheckpointView*)(self + AsCGolfDoc(GolfDocBytes())->currentPlayer * 0x54))->recoveryRequired =
            AsCGolfBall(ballBytes)->m_recoveryRequired;
        ((RuleShotCheckpointView*)(self + AsCGolfDoc(GolfDocBytes())->currentPlayer * 0x54))->playerByteAE =
            AsGolfDocSnapshot(
                GolfPlayer_GetEntryByIndex((int)GolfDocBytes(), AsCGolfDoc(GolfDocBytes())->currentPlayer))
                ->unk_AE;
        ((RuleShotCheckpointView*)(self + AsCGolfDoc(GolfDocBytes())->currentPlayer * 0x54))->powerShotMeter =
            AsGolfDocSnapshot(
                GolfPlayer_GetEntryByIndex((int)GolfDocBytes(), AsCGolfDoc(GolfDocBytes())->currentPlayer))
                ->powerShotMeter;
        ((RuleShotCheckpointView*)(self + AsCGolfDoc(GolfDocBytes())->currentPlayer * 0x54))->pang =
            ((ShotPlayerTotalsView*)GolfPlayer_GetEntryByIndex((int)GolfDocBytes(),
                                                               AsCGolfDoc(GolfDocBytes())->currentPlayer))
                ->pang;
        ((RuleShotCheckpointView*)(self + AsCGolfDoc(GolfDocBytes())->currentPlayer * 0x54))->bonusPang =
            ((ShotPlayerTotalsView*)GolfPlayer_GetEntryByIndex((int)GolfDocBytes(),
                                                               AsCGolfDoc(GolfDocBytes())->currentPlayer))
                ->bonusPang;
        {
            unsigned char b6 = AsCGolfDoc(GolfDocBytes())->currentHoleIndex;
            unsigned char* pl = (unsigned char*)GolfPlayer_GetEntryByIndex((int)GolfDocBytes(),
                                                                           AsCGolfDoc(GolfDocBytes())->currentPlayer);
            ((RuleShotCheckpointView*)(self + AsCGolfDoc(GolfDocBytes())->currentPlayer * 0x54))->holePang =
                *(int*)(AsGolfDocSnapshot(pl)->holePangTotals + b6 * 4);
        }
        {
            unsigned char* pl = (unsigned char*)GolfPlayer_GetEntryByIndex((int)GolfDocBytes(),
                                                                           AsCGolfDoc(GolfDocBytes())->currentPlayer);
            ((RuleShotCheckpointView*)(self + AsCGolfDoc(GolfDocBytes())->currentPlayer * 0x54))->holeStrokes =
                AsGolfDocSnapshot(pl)->holeStrokeCounts[AsCGolfDoc(GolfDocBytes())->currentHoleIndex];
        }
        {
            unsigned char* pl = (unsigned char*)GolfPlayer_GetEntryByIndex((int)GolfDocBytes(),
                                                                           AsCGolfDoc(GolfDocBytes())->currentPlayer);
            ((RuleShotCheckpointView*)(self + AsCGolfDoc(GolfDocBytes())->currentPlayer * 0x54))->holeResult =
                AsGolfDocSnapshot(pl)->holeResultValues[AsCGolfDoc(GolfDocBytes())->currentHoleIndex];
        }
        ((RuleShotCheckpointView*)(self + AsCGolfDoc(GolfDocBytes())->currentPlayer * 0x54))->playerByteBC =
            AsGolfDocSnapshot(
                GolfPlayer_GetEntryByIndex((int)GolfDocBytes(), AsCGolfDoc(GolfDocBytes())->currentPlayer))
                ->unk_BC;
        ::strcpy(((RuleShotCheckpointView*)(self + AsCGolfDoc(GolfDocBytes())->currentPlayer * 0x54))->clubName,
                 ((ClubSelectionRuntime*)GOLF_SHOT_PARAMETERS_BYTES)->selectedClub->clubName);
        ((RuleShotCheckpointView*)(self + AsCGolfDoc(GolfDocBytes())->currentPlayer * 0x54))->partyResult =
            ((ShotPlayerTotalsView*)GolfPlayer_GetEntryByIndex((int)GolfDocBytes(),
                                                               AsCGolfDoc(GolfDocBytes())->currentPlayer))
                ->partyResult;
        {
            if (CurrentPlayerHasItemAtSlot(7) || CurrentPlayerHasItemAtSlot(0xA)) {
                unsigned char* rec = self + AsCGolfDoc(GolfDocBytes())->currentPlayer * 0x54;
                ((RuleShotCheckpointView*)(rec))->powerShotMeter =
                    ((RuleShotCheckpointView*)(rec))->powerShotMeter -
                    (float)((RuleShotCheckpointView*)(rec))->powerMeterCost;
            }
        }
        {
            unsigned char* hdr = self + AsCGolfDoc(GolfDocBytes())->currentPlayer * 0x18 + 0xAC;
            unsigned char* rec = self + AsCGolfDoc(GolfDocBytes())->currentPlayer * 0x54;
            int cap = SIM_INT_AT(hdr, 0x10);
            if (((cap == 0) ? 0 : (cap - 1) * 16) - (SIM_INT_AT(hdr, 0) + SIM_INT_AT(hdr, 4)) == 0)
                ShotHistory_Resize(hdr, 1);
            {
                RingIt54 it = RingBegin54(hdr + 8);
                unsigned total = SIM_INT_AT(hdr, 4) + SIM_INT_AT(hdr, 0);
                it.advance(total >> 4);
                unsigned char* blk = (SIM_INT_AT(hdr, 0x10) == 0) ? 0 : *it.cur;
                unsigned char* elem = blk + (total & 0xF) * 0x54;
                if (elem) {
                    try {
                        *(S54*)elem = *(S54*)(rec + 0x10C);
                    } catch (...) {
                        throw;
                    }
                }
                SIM_INT_AT(hdr, 4) = SIM_INT_AT(hdr, 4) + 1;
            }
        }
        if (AsCGolfBall(ballBytes)->m_recoveryRequired != 0) {
            float* src;
            float spc[3];
            if (AsCGolfBall(ballBytes)->m_lie != 0) {
                unsigned char* ctl = (unsigned char*)CQuadTree_GetInstance();
                *(D3*)spc = *(D3*)&((QuadTreeRecoveryPoint*)ctl)->position;
                src = spc;
            } else {
                src = (float*)PreData__9CGolfBallFi(ballBytes, 0);
            }
            *(WVector*)AsGolfDocSnapshot(
                 GolfPlayer_GetEntryByIndex((int)GolfDocBytes(), AsCGolfDoc(GolfDocBytes())->currentPlayer))
                 ->ballPosition = *(WVector*)src;
            AsGolfDocSnapshot(
                GolfPlayer_GetEntryByIndex((int)GolfDocBytes(), AsCGolfDoc(GolfDocBytes())->currentPlayer))
                ->unk_AE = 0;
        } else {
            unsigned char* encoded = GOLF_BALL_BYTES;
            int sp0c = ~(((BallHoleInWordsView*)encoded)->encodedValue ^ ((BallHoleInWordsView*)encoded)->key);
            if (sp0c == 1 || AsCGolfBall(GOLF_BALL_BYTES)->m_bounceCount != 0) {
                unsigned char* pl = (unsigned char*)GolfPlayer_GetEntryByIndex(
                    (int)GolfDocBytes(), AsCGolfDoc(GolfDocBytes())->currentPlayer);
                *(WVector*)AsGolfDocSnapshot(pl)->ballPosition =
                    *(WVector*)(PreData__9CGolfBallFi(ballBytes, AsCGolfBall(ballBytes)->m_preDataCount));
                pl = (unsigned char*)GolfPlayer_GetEntryByIndex((int)GolfDocBytes(),
                                                                AsCGolfDoc(GolfDocBytes())->currentPlayer);
                float v70[3];
                *(D3*)v70 = *(D3*)AsGolfDocSnapshot(pl)->ballPosition;
                if ((unsigned char)GetGroundType__9CQuadTreeF7WVector(CQuadTree_GetInstance(), v70) < 3) {
                    unsigned char* a = (unsigned char*)GolfPlayer_GetEntryByIndex(
                        (int)GolfDocBytes(), AsCGolfDoc(GolfDocBytes())->currentPlayer);
                    unsigned char* b = (unsigned char*)GolfPlayer_GetEntryByIndex(
                        (int)GolfDocBytes(), AsCGolfDoc(GolfDocBytes())->currentPlayer);
                    *(WVector*)AsGolfDocSnapshot(b)->previousBallPosition =
                        *(WVector*)AsGolfDocSnapshot(a)->ballPosition;
                }
            } else {
                unsigned char* g = GolfDocBytes();
                GolfAim_FindPlayableRecoveryPoint(
                    AsCGolfDoc(g)->aimPath, PreData__9CGolfBallFi(ballBytes, AsCGolfBall(ballBytes)->m_preDataCount));
                {
                    unsigned char* q = (unsigned char*)AsCGolfDoc(GolfDocBytes())->aimPath;
                    WVector v64;
                    *(D3*)&v64 = *(D3*)&((AimRecoveryPoint*)q)->position;
                    *(WVector*)AsGolfDocSnapshot(
                         GolfPlayer_GetEntryByIndex((int)GolfDocBytes(), AsCGolfDoc(GolfDocBytes())->currentPlayer))
                         ->ballPosition = v64;
                }
                AsGolfDocSnapshot(
                    GolfPlayer_GetEntryByIndex((int)GolfDocBytes(), AsCGolfDoc(GolfDocBytes())->currentPlayer))
                    ->unk_AE = 0;
            }
        }
        {
            int found = 0;
            if (AsCGolfBall(ballBytes)->m_recoveryRequired == 0) {
                unsigned char* pl = (unsigned char*)GolfPlayer_GetEntryByIndex(
                    (int)GolfDocBytes(), AsCGolfDoc(GolfDocBytes())->currentPlayer);
                float v58[3];
                *(D3*)v58 = *(D3*)AsGolfDocSnapshot(pl)->ballPosition;
                if (QuadTree_IsWaterAtPoint(CQuadTree_GetInstance(), v58) != 0)
                    found = 1;
            }
            if (found) {
                int z = 0;
                AsCGolfBall(ballBytes)->m_recoveryRequired = 1;
                AsCGolfBall(ballBytes)->m_lie = z;
                unsigned char* pl = (unsigned char*)GolfPlayer_GetEntryByIndex(
                    (int)GolfDocBytes(), AsCGolfDoc(GolfDocBytes())->currentPlayer);
                *(WVector*)AsGolfDocSnapshot(pl)->ballPosition = *(WVector*)(PreData__9CGolfBallFi(ballBytes, 0));
                AsGolfDocSnapshot(
                    GolfPlayer_GetEntryByIndex((int)GolfDocBytes(), AsCGolfDoc(GolfDocBytes())->currentPlayer))
                    ->unk_AE = z;
            }
        }
        if ((unsigned)(AsCGolfDoc(GolfDocBytes())->tutorialStage - 5) <= 1) {
            IActor_DispatchMessage(self, self, golfRuleStrings + kText_screen, 0x15, 0, 0, 0, 0);
        }
        *(unsigned char*)&AsCGolfRule(self)->forgetDlg = 0;
        AsCGolfRule(self)->forgetDlgTime = kFloat_0_GolfRule;
        if (AsCGolfBall(ballBytes)->m_recoveryRequired != 0) {
            if (FindSlot418(self))
                self[0x410] = 1;
        }
        // Shot phase F: rewind live ball to sample 0; retain the samples,
        // then prepare playback/camera and player-transition bookkeeping.
        *(WVector*)ballBytes = *(WVector*)firstSampleBytes;
        *(WVector*)(ballBytes + kBallVelocityByteOffset) = *(WVector*)(firstSampleBytes + 0xC);
        AsCGolfBall(ballBytes)->m_velocity.y =
            AsCGolfBall(ballBytes)->m_velocity.y *
            (kFloat_1_805DEA38 + ((ShotAccessoryModifiers*)effectStateBytes)->verticalVelocityBonus);
        SIM_INT_AT(ballBytes, 0x28) = (int)((CQuadTree*)CQuadTree_GetInstance())->GetMeshIndex(ballBytes, 0);
        __mi__FRC7WVectorRC7WVector(
            totalTravelXZ, PreData__9CGolfBallFi(ballBytes, AsCGolfBall(ballBytes)->m_preDataCount), firstSampleBytes);
        totalTravelXZ[1] = kFloat_0_GolfRule;
        if (GOLF_CLUB_CATEGORY == 3) {
            if (AsCGolfBall(ballBytes)->m_groundType == 2) {
                unsigned char* pl = (unsigned char*)GolfPlayer_GetEntryByIndex(
                    (int)GolfDocBytes(), AsCGolfDoc(GolfDocBytes())->currentPlayer);
                unsigned char* e = pl + AsCGolfDoc(GolfDocBytes())->currentHoleIndex;
                AsGolfDocSnapshot(e)->holeResultValues[0] = AsGolfDocSnapshot(e)->holeResultValues[0] + 1;
            }
            ((ShotAccountStatisticsView*)(SharedDocBytes() + AsCGolfDoc(GolfDocBytes())->currentPlayer * 0x1D4))
                ->putterShots =
                ((ShotAccountStatisticsView*)(SharedDocBytes() + AsCGolfDoc(GolfDocBytes())->currentPlayer * 0x1D4))
                    ->putterShots +
                1;
        } else {
            ((ShotAccountStatisticsView*)(SharedDocBytes() + AsCGolfDoc(GolfDocBytes())->currentPlayer * 0x1D4))
                ->nonPutterShots =
                ((ShotAccountStatisticsView*)(SharedDocBytes() + AsCGolfDoc(GolfDocBytes())->currentPlayer * 0x1D4))
                    ->nonPutterShots +
                1;
        }
        if (GOLF_CLUB_CATEGORY != 3)
            ((void (*)(void*))GolfRule_ApplyShotOutcomeModifier)(self);
        self[0x2E0] = ((CGolfRule*)self)->SelectNextPlayer(0);
        self[0x91] = 0;
        self[0x93] = 1;
        self[0x3D8] = 0;
        if (s_cameraDirectedShotEnabled == 0) {
            if (!(AsCGolfRule(self)->shotPresentationFlags & 2)) {
                IActor_DispatchMessage(self, self, golfRuleStrings + kText_caddie, 1, 0, 0, 0, 0);
                void* a = PreData__9CGolfBallFi(GolfBall__Fv(), 0);
                unsigned char* b = GOLF_BALL_BYTES;
                // Last sample minus sample 0, including roll; horizontal endpoint
                // distance in world units, not airborne carry or summed path length.
                __mi__FRC7WVectorRC7WVector(finalHorizontalDisplacement,
                                            PreData__9CGolfBallFi(GolfBall__Fv(), AsCGolfBall(b)->m_preDataCount), a);
                finalHorizontalDisplacement[1] = kFloat_0_GolfRule;
                // The launchSpeed scratch is reused here for that distance.
                launchSpeed = (*(WVector*)finalHorizontalDisplacement).Magnitude();
                if (AsCGolfBall(GOLF_BALL_BYTES)->m_recoveryRequired == 0 && launchSpeed > kFloat_416_GolfRule) {
                    IActor_DispatchMessage(self, self, golfRuleStrings + kText_caddie, 2, 0, 0, 0, 0);
                } else {
                    ((CameraCaddieOption*)g_cameraWork)->caddieCameraEnabled = 0;
                }
            }
            if (!(SIM_INT_AT(GOLF_SHOT_PARAMETERS_BYTES, 0x44) == 2 && GOLF_CLUB_CATEGORY == 0 &&
                  AsCGolfDoc(GolfDocBytes())->impactGrade != kImpactMiss &&
                  AsCGolfDoc(GolfDocBytes())->impactGrade != 0)) {
                if (SharedDoc_IsPlayerFlag0Set(AsCGolfDoc(GolfDocBytes())->currentPlayer)) {
                    struct ChoiceWords54 {
                        int values[4];
                    };
                    int choice;
                    if (GOLF_CLUB_CATEGORY == 3) {
                        ChoiceWords54 choices = *(const ChoiceWords54*)s_computerPuttCameraChoices;
                        choice = choices.values[(unsigned)rand() % 4];
                    } else {
                        ChoiceWords54 choices = *(const ChoiceWords54*)s_computerShotCameraChoices;
                        choice = choices.values[(unsigned)rand() % 4];
                    }
                    OSReport(golfRuleStrings + kText_computerCameraDebugFormat, choice);
                    ((CCameraWork*)g_cameraWork)->ResetMode(choice);
                    ((CCameraWork*)g_cameraWork)->UpdateMotionAndTarget(kFloat_0_GolfRule, kFloat_0_GolfRule);
                } else
                    ((CCameraWork*)g_cameraWork)->ResetMode(-1);
                SIM_INT_AT(g_rival, 0x98) = 0;
            } else {
                ((CCameraWork*)g_cameraWork)->ResetMode(-1);
            }
        }
        return;
    }
    // Rewind samples and choose flight effects and audio.
    // Requires completed samples; publishes cursor, world clock, effects and audio.
    // Playback advances samples; an initially non-airborne sample returns early.
    case kStateBallFlight: {
        // Requires precomputed sample history; rewind playback, choose effects and sound.
        lbl_805DC7F4 = 0;
        self[AsCGolfDoc(GolfDocBytes())->currentPlayer + 0x25C] = 0;
        *(int*)&AsGolfRuleFrame(self)->courseCameraBounds = 0;
        T_MSG(kText_player, 0x32, 1, AsCGolfDoc(GolfDocBytes())->currentPlayer);
        T_MSG(kText_effect, 0x13, 0, 0);
        if (!AsGolfRuleFrame(self)->presentationStep) {
            T_MSG(kText_screen, 7, 0, 0);
            T_MSG(kText_screen, 2, 0, 0);
            T_MSG(kText_spin, 4, 0, 0);
        }
        T_CLEAR();
        firstSampleBytes = (unsigned char*)PreData__9CGolfBallFi(ballBytes, 0);
        AsCGolfBall(ballBytes)->m_eventPreDataIndex[kBallPlaybackCursor] = 0;
        AsCGolfBall(ballBytes)->m_state = ((ShotSample*)firstSampleBytes)->motionState;
        AsCGolfBall(ballBytes)->m_rotation[0] = kFloat_0_GolfRule;
        float axis[3] = {kFloat_0_GolfRule, kFloat_0_GolfRule, kFloat_1_805DEA38};
        AsCGolfBall(ballBytes)->m_rotation[1] = Vector_SignedAngleXZ(axis, ballBytes + 0xC);
        AsGolfRuleFrame(self)->stateTimer = kFloat_0_GolfRule;
        int saved = AsGolfRuleFrame(self)->shotContactFrame;
        void* ctl = CQuadTree_GetInstance();
        ((ShotWorldClockView*)ctl)->frame = saved;
        ((ShotWorldClockView*)ctl)->fractionalSeconds = kFloat_0_GolfRule;
        if (AsCGolfBall(ballBytes)->m_eventPreDataIndex[8] > 0)
            T_MSG(kText_ball, 3, 0, 0);
        if (!(SIM_INT_AT(self, 0x490) & 2) || AsGolfRuleFrame(self)->presentationStep == 2) {
            T_MSG(kText_screen, 3, 1, 0);
            if (!SoundManager_IsAliasPrefixOrStreamStemActive(g_soundManager, golfRuleStrings + kText_shot_dist_count))
                T_SOUND(golfRuleStrings + kText_shot_dist_count, 1, golfRuleStrings + kText_shot_dist_count);
        }
        if (AsCGolfBall(ballBytes)->m_state != 1) {
            if (AsCGolfBall(ballBytes)->m_state == 3)
                T_MSG(kText_trajectory, 5, 0, 0);
            return;
        }
        normalizedImpactRatio = (AsCGolfDoc(GolfDocBytes())->shotPowerGaugeX - 149.0f) / kFloat_360_GolfRule;
        if (GOLF_CLUB_CATEGORY != 3 && normalizedImpactRatio > kFloat_0_8_GolfRule &&
            AsCGolfBall(ballBytes)->m_groundType != 1 && AsCGolfBall(ballBytes)->m_groundType != 6 &&
            !Map_FindByKey(g_fx, s_shotImpactEffectId))
            s_shotImpactEffectId = ((CFx*)g_fx)->MakeShotImpact(ballBytes);
        float delta[3];
        __mi__FRC7WVectorRC7WVector(delta, PreData__9CGolfBallFi(ballBytes, AsCGolfBall(ballBytes)->m_preDataCount),
                                    firstSampleBytes);
        delta[1] = kFloat_0_GolfRule;
        self[0x374] = 0;
        if (IdMatches(0xC) || IdMatches(0x80)) {
            if ((*(WVector*)delta).Magnitude() > kFloat_320_GolfRule)
                ::strcpy(AsGolfRuleFrame(self)->ballFlightSoundName, golfRuleStrings + kText_wb_fly);
        } else if ((*(WVector*)delta).Magnitude() > kFloat_640_GolfRule)
            ::strcpy(AsGolfRuleFrame(self)->ballFlightSoundName, golfRuleStrings + kText_ballEffectStage1);
        else if ((*(WVector*)delta).Magnitude() > kFloat_480_GolfRule)
            ::strcpy(AsGolfRuleFrame(self)->ballFlightSoundName, golfRuleStrings + kText_ballEffectStage2);
        else if ((*(WVector*)delta).Magnitude() > kFloat_320_GolfRule)
            ::strcpy(AsGolfRuleFrame(self)->ballFlightSoundName, golfRuleStrings + kText_ballEffectStage3);
        if ((signed char)*(unsigned char*)&AsGolfRuleFrame(self)->ballFlightSoundName[0]) {
            Sound_StopByAlias(g_soundManager, AsGolfRuleFrame(self)->ballFlightSoundName);
            T_SOUND(AsGolfRuleFrame(self)->ballFlightSoundName, 0, AsGolfRuleFrame(self)->ballFlightSoundName);
        }
        switch (GOLF_CLUB_CATEGORY) {
        case 0:
        case 1:
            if (normalizedImpactRatio > kFloat_0_8_GolfRule) {
                if (AsCGolfBall(ballBytes)->m_powerFlags & kSpecialShotMask) {
                    if (AsCGolfBall(ballBytes)->m_powerFlags & kShotTomahawk)
                        ((CFx*)g_fx)
                            ->CreateTomahawk(ballBytes,
                                             ((ClubSelectionRuntime*)GOLF_SHOT_PARAMETERS_BYTES)->powerShotMode == 2);
                    else if (AsCGolfBall(ballBytes)->m_powerFlags & kShotCobra) {
                        ((CFx*)g_fx)->CreateCobra(ballBytes);
                        if (((ClubSelectionRuntime*)GOLF_SHOT_PARAMETERS_BYTES)->powerShotMode == 2) {
                            void* root = g_taskManager ? Object_GetChild(g_taskManager) : 0;
                            int message[8];
                            message[0] = 0;
                            message[1] = 8;
                            message[2] = 0;
                            message[3] = 0;
                            message[4] = 0;
                            message[5] = 0;
                            message[6] = 0;
                            message[7] = 0;
                            void* ui = TaskActorTable_FindByName(root, golfRuleStrings + kText_trajectory);
                            if (ui)
                                ((void (*)(void*, void*))(*(void***)ui)[11])(ui, message);
                        }
                    } else if (AsCGolfBall(ballBytes)->m_powerFlags & kShotSpike)
                        ((CFx*)g_fx)
                            ->CreateSpike(ballBytes,
                                          ((ClubSelectionRuntime*)GOLF_SHOT_PARAMETERS_BYTES)->powerShotMode == 2);
                } else {
                    if (AsCGolfDoc(GolfDocBytes())->impactGrade == kImpactPangya) {
                        if (((ClubSelectionRuntime*)GOLF_SHOT_PARAMETERS_BYTES)->powerShotMode) {
                            T_MSG(kText_trajectory, 0, 0, 0);
                            T_MSG(kText_effect, 0x15, 0, 0);
                            Sound_StopByAlias(g_soundManager, golfRuleStrings + kText_cometSequence);
                            T_SOUND(golfRuleStrings + kText_ballCometSequence, 0,
                                    golfRuleStrings + kText_cometSequence);
                        } else {
                            T_MSG(kText_effect, 0x14, 0, 0);
                            Sound_StopByAlias(g_soundManager, golfRuleStrings + kText_stardustSequence);
                            T_SOUND(golfRuleStrings + kText_ballStardustSequence, 0,
                                    golfRuleStrings + kText_stardustSequence);
                        }
                        if (!(AsCGolfRule(self)->shotPresentationFlags & 2) ||
                            AsGolfRuleFrame(self)->presentationStep == 2) {
                            float two = kFloat_2_805DEA84;
                            T_MSG(kText_letter, 1, golfRuleStrings + kText_pangyaNotice, &two);
                            T_MSG(kText_gallery, 3, 0, 0);
                            T_SOUND(shotTimingSoundNames[6], 0, 0);
                            T_MSG(kText_voice, 3, 0, 0);
                        }
                    } else {
                        WVector wind;
                        GetWindVector__5CWindFv(&wind, CWind::GetInstance());
                        if (wind.Magnitude() > kFloat_4_GolfPlayerHelpers) {
                            T_MSG(kText_trajectory, 3, 0, 0);
                            Sound_StopByAlias(g_soundManager, golfRuleStrings + kText_windSequence);
                            T_SOUND(golfRuleStrings + kText_ballWindSequence, 0, golfRuleStrings + kText_windSequence);
                        } else
                            T_MSG(kText_trajectory, 1, 0, 0);
                    }
                    if (((ClubSelectionRuntime*)GOLF_SHOT_PARAMETERS_BYTES)->powerShotMode) {
                        if (((ClubSelectionRuntime*)GOLF_SHOT_PARAMETERS_BYTES)->powerShotMode == 1)
                            T_MSG(kText_effect, 0, 0, 0);
                        else if (((ClubSelectionRuntime*)GOLF_SHOT_PARAMETERS_BYTES)->powerShotMode == 2)
                            ((CFx*)g_fx)->CreatePowerShot2(ballBytes);
                        ((CCameraWork*)g_cameraWork)->Quake();
                        T_SOUND(golfRuleStrings + kText_powerShotEffectSequence, 0, 0);
                    }
                }
                if (AsCGolfDoc(GolfDocBytes())->impactGrade > 2)
                    T_MSG(kText_effect, 2, 0, 0);
            } else if (normalizedImpactRatio > kFloat_0_2_GolfRule)
                T_MSG(kText_trajectory, 1, 0, 0);
            T_MSG(kText_ball, 8, 0, 0);
            break;
        case 2:
            if (normalizedImpactRatio > kFloat_0_8_GolfRule) {
                if (AsCGolfBall(ballBytes)->m_powerFlags & kShotTomahawk)
                    ((CFx*)g_fx)
                        ->CreateTomahawk(ballBytes,
                                         ((ClubSelectionRuntime*)GOLF_SHOT_PARAMETERS_BYTES)->powerShotMode == 2);
                else {
                    if (AsCGolfDoc(GolfDocBytes())->impactGrade == kImpactPangya) {
                        if (!(SIM_INT_AT(self, 0x490) & 2) || AsGolfRuleFrame(self)->presentationStep == 2) {
                            float two = kFloat_2_805DEA84;
                            T_MSG(kText_letter, 1, golfRuleStrings + kText_pangyaNotice, &two);
                            T_MSG(kText_gallery, 3, 0, 0);
                            T_SOUND(shotTimingSoundNames[6], 0, 0);
                            T_MSG(kText_voice, 3, 0, 0);
                        }
                    } else
                        T_MSG(kText_trajectory, 1, 0, 0);
                    if (AsCGolfDoc(GolfDocBytes())->impactGrade > 2)
                        T_MSG(kText_effect, 2, 0, 0);
                }
            } else if (normalizedImpactRatio > kFloat_0_2_GolfRule)
                T_MSG(kText_trajectory, 1, 0, 0);
            break;
        case 3:
            T_MSG(kText_trajectory, 5, 0, 0);
            break;
        }
        if ((AsCGolfBall(ballBytes)->m_powerFlags & 3) && AsCGolfBall(ballBytes)->m_eventPreDataIndex[7] > 0)
            T_MSG(kText_trajectory, 2, 0, 0);
        return;
    }
    // Finalize hole rankings and initialize the scoreboard.
    // Publishes hole ranking and scoreboard setup through EnterGolfHoleResults.
    // The existing hole-results processor owns continuation.
    case kStateHoleResults:
        EnterGolfHoleResults(self);
        return;
    // Reset camera input and select the next result/player phase.
    // Publishes result presentation or selects the next waiting player.
    // Result presentation and player/caddie readiness control the subsequent turn.
    case kStateDelayedTransition: {
        ResetCameraInputForGolfState();
        AsGolfRuleFrame(self)->stateEntryPending = 0;
        ((RuleUnresolvedShotBytes*)self)->unk_092 = 0;
        ((RuleUnresolvedShotBytes*)self)->unk_094 = 0;
        T_MSG(kText_letter, 0, 0, 0);
        if (AsCSharedDoc(SharedDocBytes())->golfRuleType == 1) {
            if (!CGolfRule::StartHoleResultPresentation((CE00Self*)self)) {
                AsCGolfRule(self)->nextPlayerIndex = ((CGolfRule*)self)->SelectNextPlayer(0);
                ((CGolfRule*)self)->EnterState(kStateWaitPlayerAndCaddie);
                return;
            }
        } else
            CGolfRule::StartRoundResultPresentation((char*)self, 0, 0, 0);
        Camera_SetFov(g_view, kGolfCameraFovRadians);
        T_END_UI();
        int slot = AsCGolfDoc(GolfDocBytes())->currentPlayer;
        if (AsCSharedDoc(SharedDocBytes())->golfRuleType == 1)
            slot = ((CGolfDoc*)GolfDocBytes())->ResolveMatchTurnFromHoleStrokes();
        T_MSG(kText_player, 0x32, 2, slot);
        T_MSG(kText_voice, 6, 0, 0);
        return;
    }
    // Publishes timeout bookkeeping, cleared input UI and camera/actor feedback.
    // Stroke-limit admission may immediately enter DelayedTransition or FightResult.
    case kStateBeginPlayerTurn: {
        // Apply turn bookkeeping, clear shot UI, and choose the next player phase.
        float clubSetModifiers[16];
        float powerColorOff;
        float timeoutCameraMatrix[12];

        unsigned char* golfDocBytes = GolfDocBytes();
        unsigned char* playerSnapshot = T_CUR;
        ((ShotPlayerTotalsView*)playerSnapshot)->accumulatedShotMilliseconds +=
            *(int*)&AsCGolfDoc(golfDocBytes)->elapsedTimeMs;
        AsGolfDocSnapshot(T_CUR)->itemId = 0;
        if (!*(int*)&AsCSharedDoc(SharedDocBytes())->partyGameMode) {

            *(F16*)clubSetModifiers = *(F16*)s_shotOutcomeClubSetModifiers;
            int clubSetIndex = ItemInventory_IsSlot14ClubSetId(
                ((AccountClubSelectionFields*)(SharedDocBytes() + AsCGolfDoc(GolfDocBytes())->currentPlayer * 0x1D4))
                    ->clubSetValue);
            speedScale = (clubSetModifiers + 12)[clubSetIndex];
            absoluteAdjustment = AsGolfDocSnapshot(T_CUR)->powerShotMeter + speedScale;
            absoluteAdjustment = (kFloat_0_GolfRule < absoluteAdjustment) ? absoluteAdjustment : kFloat_0_GolfRule;
            AsGolfDocSnapshot(T_CUR)->powerShotMeter = absoluteAdjustment;
        }
        Sound_StopByAlias(g_soundManager, golfRuleStrings + kText_timeSoundAlias);
        T_SOUND(golfRuleStrings + kText_timeOverSequence, 0, golfRuleStrings + kText_timeOverSequence);
        if (!*(int*)&AsCSharedDoc(SharedDocBytes())->partyGameMode)
            ++AsGolfDocSnapshot(T_CUR)->timeoutsUsed;
        powerColorOff = kFloat_0_GolfRule;
        void* shotParameters = InstanceTable_Get();
        ((ClubSelectionRuntime*)shotParameters)->powerShotMode = 0;
        T_MSG(kText_spot, 4, 0, 0);
        T_MSG(kText_grid, 3, 0, 0);
        T_MSG(kText_screen, kScreenSetPowerColorMessage, &powerColorOff, 0);
        T_MSG(kText_clock, 0x2719, 0, 0);
        T_MSG(kText_guideLine, 1, 0, 0);
        T_MSG(kText_clock, 1, 0, 0);
        T_MSG(kText_screen, 0x26, 0, 0);
        T_MSG(kText_player, 0xA, 0, 0);
        AsCGolfRule(self)->nextPlayerIndex = ((CGolfRule*)self)->SelectNextPlayer(0);
        if (!*(int*)&AsCSharedDoc(SharedDocBytes())->partyGameMode) {
            // Keep this sequence in EnterState: another inline level changes the
            // implicit modifier copy and the two penalty-check expansions.
            int strokeLimitReached = PenaltyLimit54(self);
            if (!strokeLimitReached) {
                playerSnapshot = T_CUR;
                if (AsGolfDocSnapshot(playerSnapshot)->playerState == 3) {
                    strokeLimitReached = 0;
                } else {
                    unsigned char* holeRecord = (unsigned char*)AsCGolfDoc(GolfDocBytes())->holeRecords +
                                                (AsCGolfDoc(GolfDocBytes())->currentHoleIndex << 5);
                    int playerState;
                    unsigned char strokeLimit = holeRecord[0x18] + 4;
                    playerState = SIM_INT_AT(T_CUR, 8);
                    unsigned char holeStrokes = T_CUR[AsCGolfDoc(GolfDocBytes())->currentHoleIndex + 0x28];
                    strokeLimitReached = playerState == 1 ? holeStrokes > strokeLimit : holeStrokes >= strokeLimit;
                }
            }
            if (strokeLimitReached) {
                unsigned char holeIndex;
                unsigned char playerIndex = AsCGolfDoc(GolfDocBytes())->currentPlayer;
                if (playerIndex < AsCGolfDoc(GolfDocBytes())->playerCount) {
                    holeIndex = AsCGolfDoc(GolfDocBytes())->currentHoleIndex;
                    unsigned char* holeRecord = *(unsigned char**)(GolfDocBytes() + 0x2BC) + (holeIndex << 5);
                    unsigned char strokeLimit = holeRecord[0x18] + 4;
                    GolfPlayerEntryByte(playerIndex)[holeIndex + 0x28] = strokeLimit + 1;
                    ++GolfPlayerEntryByte(playerIndex)[0xB0];
                }
                strokeLimitReached = PenaltyLimit54(self);
                if (!strokeLimitReached) {
                    playerIndex = AsCGolfDoc(GolfDocBytes())->currentPlayer;
                    if (playerIndex < AsCGolfDoc(GolfDocBytes())->playerCount &&
                        AsGolfDocSnapshot(GolfPlayerEntryByte(playerIndex))->playerState != 3) {
                        AsGolfDocSnapshot(GolfPlayerEntryByte(playerIndex))->itemId = 0;
                        unsigned char holeIndex = AsCGolfDoc(GolfDocBytes())->currentHoleIndex;
                        if (*(int*)&AsGolfDocSnapshot(GolfPlayerEntryByte(playerIndex))->roundResultWords[3] ==
                            holeIndex) {
                            if (!AsCGolfDoc(GolfDocBytes())->unk_35C) {
                                holeIndex = AsCGolfDoc(GolfDocBytes())->currentHoleIndex;
                                unsigned char* otherPlayerSnapshot = GolfPlayerEntryByte(playerIndex);
                                playerSnapshot = GolfPlayerEntryByte(playerIndex);
                                playerSnapshot[0xC5] += otherPlayerSnapshot[holeIndex + 0x28];
                                if (AsCSharedDoc(SharedDocBytes())->golfRuleType != 1) {
                                    unsigned char* holeRecord;
                                    unsigned char scoreHoleIndex = AsCGolfDoc(GolfDocBytes())->currentHoleIndex;
                                    otherPlayerSnapshot = GolfPlayerEntryByte(playerIndex);
                                    holeRecord = (unsigned char*)AsCGolfDoc(GolfDocBytes())->holeRecords +
                                                 (AsCGolfDoc(GolfDocBytes())->currentHoleIndex << 5);
                                    playerSnapshot = GolfPlayerEntryByte(playerIndex);
                                    SIM_INT_AT(playerSnapshot, 0xC8) +=
                                        otherPlayerSnapshot[scoreHoleIndex + 0x28] - holeRecord[0x18];
                                }
                            }
                            ++*(int*)&AsGolfDocSnapshot(GolfPlayerEntryByte(playerIndex))->roundResultWords[3];
                        }
                        AsGolfDocSnapshot(GolfPlayerEntryByte(playerIndex))->playerState = 1;
                        if ((unsigned)*(int*)&AsCSharedDoc(SharedDocBytes())->gameMode == 6) {
                            unsigned char* sharedDocBytes = SharedDocBytes();
                            *(int*)&AsCSharedDoc(sharedDocBytes)->partyResultTier =
                                PartyGame_EvaluatePlayerThresholdTier(playerIndex);
                        }
                    }
                }
                if (AsCSharedDoc(SharedDocBytes())->golfRuleType == 1)
                    ((CGolfRule*)self)->EnterState(kStateDelayedTransition);
                else
                    ((CGolfRule*)self)->EnterState(kStateFightResult);
                return;
            }
        }
        if (!AsCGolfDoc(GolfDocBytes())->unk_3CC) {
            GolfTips_ShowIndexedMessage(7, golfRuleMessageTags + 0x44, 0, kFloat_1_5_GolfRule, kFloat_0_5_GolfRule);
            T_MSG(kText_screen, 0x1B, 0, 0);
        }
        CGolfDoc::BuildCameraMatrix(timeoutCameraMatrix, GolfDocBytes());
        Matrix_CopyAffine(s_cameraTransform, timeoutCameraMatrix);
        T_MSG(kText_player, 0x2715, golfRuleStrings + kText_timeOverBunkerObDisappointmentPose, 0);
        T_MSG(kText_caddie, 0x2715, golfRuleStrings + kText_bunkerSequence, 0);
        return;

        return;
    }
    // Grant allowed rewards and initialize award-camera presentation.
    // Publishes admitted rewards, award camera, scoreboard and BGM.
    // The existing post-round processor owns menu continuation.
    case kStatePostRoundMenus: {
        ResetCameraInputForGolfState();
        ((CGolfRule*)self)->UpdatePlayerRanksAndHoleOrder();
        // State 0x20000: consume a ticket only when the current mode allows it.
        if (!*(int*)&AsCSharedDoc(SharedDocBytes())->gameMode || SharedDoc_IsMode6PartyMode0()) {
            if (AsCSharedDoc(SharedDocBytes())->golfRuleType == 0 ||
                (AsCSharedDoc(SharedDocBytes())->golfRuleType == 1 &&
                 ((AsGolfDocSnapshot(GolfPlayerEntryByte(0))->rankFlags >> 4) & 15) == 1)) {
                if (*(short*)(SharedDocBytes() + 0x42) > 0) {
                    --*(short*)(SharedDocBytes() + 0x42);
                    AsGolfDocSnapshot(GolfPlayerEntryByte(0))->unk_16E = 1;
                }
            }
        }
        if ((unsigned)*(int*)&AsCSharedDoc(SharedDocBytes())->gameMode == 6) {
            int value = 1;
            if (*(int*)&AsCSharedDoc(SharedDocBytes())->partyGameMode)
                value = PartyGame_ApplyAchievedTierRewardToPlayer(0);
            GolfRule_ResetItemRequestState(self, (void*)value);
        } else
            ((void (*)(void*))GolfRule_UpdateRoundRewards)(self);
        if (g_sceneManager)
            CourseWeather_ConfigureEffects(g_sceneManager, 0, 1);
        for (int i = 0; i < 4; i++)
            T_MSG(kText_player, 0x32, 2, i);
        T_MSG(kText_score, 0, 1, 0);
        T_MSG(kText_golfBg, 8, 0, 0);
        CWind::GetInstance()->LoadCourseRegions();
        CWind::GetInstance()->InitOverrideFromCourse();
        unsigned char* camera = (unsigned char*)CourseData_FindNameRecord(AsCGolfDoc(GolfDocBytes())->courseData,
                                                                          golfRuleStrings + kText_awardCamera);
        if (camera) {
            Matrix_CopyAffine(s_cameraTransform, camera + 0x20);
            Camera_SetFov(g_view, kFloat_0_017453292_GolfRule * SIM_FLOAT_AT(camera, 0x50));
        } else {
            Matrix_SetIdentityAffine(s_cameraTransform);
            Camera_SetFov(g_view, kGolfCameraFovRadians);
        }
        T_MSG(kText_score, 1, 0, 0);
        T_MSG(kText_score, 2, 0, 0);
        T_MSG(kText_player, 0x12, 0, 0);
        SoundManager_RequestBgmChange(g_soundManager, golfRuleStrings + kText_scoreboard_wav, 1, 1);
        char text[0x40];
        ::strcpy(text, golfRuleStrings + kText_individualAwardSequence);
        int v[3] = {0, 0, 0};
        AsGolfRuleFrame(self)->stateTimer = ((CCameraWork*)g_cameraWork)->StartGroupAnimationAtPivot(1, text, v);
        return;
    }
    // Publishes fight-result setup through EnterFightResultState.
    // The existing fight-result processor owns continuation.
    case kStateFightResult: {
        // Prepare the fight result presentation.
        EnterFightResultState(self);
        return;
    }
    // Clear reload progress; subsequent processing owns the reload operation.
    // Publishes reload stage zero.
    // ProcessCourseReload owns subsequent resource progress.
    case kStateCourseReload:
        AsGolfRuleFrame(self)->courseReloadStage = 0;
        return;
    // Publishes story presentation through EnterStorySequenceState.
    // The existing story processor owns continuation.
    case kStateStorySequence: {
        // Publish the story sequence state and its presentation.
        EnterStorySequenceState(self);
        return;
    }
    // Publishes round-results setup through EnterRoundResultsState.
    // The existing round-result processor owns continuation.
    case kStateRoundResults: {
        // Prepare the round-results display.
        EnterRoundResultsState(self);
        return;
    }
    // Activate motion control and clear previous shot input.
    // Requires constructed controller; publishes motion setup and cleared commands.
    // Wii Remote swing processing owns subsequent power/impact input.
    case kStateMotionSwing: {
        unsigned char* o = *(unsigned char**)(self + 0x4C4);
        ((void (*)(void*, int))(((void**)AsCPadSwingCtrl(o)->interfaceVtable))[9])(
            o, SIM_INT_AT(SharedDocBytes() + AsCGolfDoc(GolfDocBytes())->currentPlayer * 0x1D4, 0x1E0) != 0);
        o = *(unsigned char**)(self + 0x4C4);
        ((void (*)(void*))(((void**)AsCPadSwingCtrl(o)->interfaceVtable))[3])(o);
        AsCGolfDoc(GolfDocBytes())->alternateMotionPowerDecided = 0;
        AsCGolfDoc(GolfDocBytes())->motionSwingState = 0;
        Sound_StopByAlias(g_soundManager, golfRuleStrings + kText_timeSoundAlias);
        CMouseCursor::SetPointerInputEnabled(g_mouseCursor, 0);
        ((CMouseCursor*)g_mouseCursor)->SetRadialMenuEnabled(0);
        T_MSG(kText_score, 1, 0, 0);
        T_MSG(kText_camera, 5, 0, 0);
        T_MSG(kText_letter, 0, 0, 0);
        if (*(int*)&AsCSharedDoc(SharedDocBytes())->partyGameMode == 2)
            T_MSG(kText_screen, 0x27, 0, 0);
        unsigned char* a = GOLF_BALL_BYTES;
        unsigned char* b = GOLF_BALL_BYTES;
        T_MSG(kText_spin, 1, b + 0x34, a + 0x38);
        T_MSG(kText_spot, 0xB, self + 0xA8, 0);
        AsCGolfBall(GOLF_BALL_BYTES)->m_powerFlags = 0;
        T_CLEAR();
        ((RuleGaugeInputTiming*)self)->heldDirectionTick = 0;
        ((RuleGaugeInputTiming*)self)->minimumPowerFrameSeconds = kFloat_1_805DEA38;
        if (GolfTask_NoOpStatus()) {
            volatile unsigned char enabled = 1;
            volatile float value = AsCGolfDoc(GolfDocBytes())->pangyaTargetX;
        }
        return;
    }
    // Clear presentation UI in party modes before waiting for player/caddie readiness.
    // Publishes cleared presentation UI in party modes.
    // The existing wait-state processor owns player/caddie readiness.
    case kStateWaitPlayerAndCaddie:
        if (*(int*)&AsCSharedDoc(SharedDocBytes())->partyGameMode) {
            T_MSG(kText_letter, 0, 0, 0);
            T_END_UI();
        }
        return;
    // Publishes current player binding to the motion controller.
    // Motion-camera processing owns input and exit.
    case kStateMotionCameraInput: {
        // Bind the current player actor to the motion-swing controller.
        EnterMotionCameraInputState(self);
        return;
    }
    // Ensure the loading indicator has a player while hole resources load.
    // Ensures a loading-indicator player exists for pending resources.
    // The existing wait-state processor owns hole-resource readiness.
    case kStateWaitHoleResources:
        if (!Sing_805DC610()) {
            if (!s_nowReloadingInitialized) {
                ((CNowReloading*)(&s_nowReloading))->Init();
                __register_global_object(&s_nowReloading, (void*)__dt__13CNowReloadingFv,
                                         ((char*)s_golfRuleInputDestructorRecord + kReloadingDestructorRecord));
                s_nowReloadingInitialized = 1;
            }
            Sing_805DC610() = &s_nowReloading;
        }
        {
            void* q = Sing_805DC610();
            if (q && LoadingIndicator_IsAbsentOrState3(q))
                LoadingIndicator_CreatePlayer(q);
        }
        return;
    // These states use only the common state publication and elapsed-time reset.
    // These four entries publish state/timer only; existing processors own continuation.
    case kStateCameraView:
    case kStateUnresolved400:
    case kStateCameraReturn:
    case kStateMotionShotAnimation:
        return;
    }
}

namespace ns4 {
extern "C" {
// ===== S9 - Rewards and result presentation =====
// Reward account/player views, player scores and result cameras.
}
}

// Copy the dialog-owned word range and append the requested value.
// Requires a writable dialog and three readable unsigned source words.
// Copies this-adjustment, vtable offset and callback function, then stores value
// as callback target. No allocation, callback invocation or failure return here.
void CSmallCommonDialog::SetCallback(unsigned int* source, unsigned int value) {
    unsigned int first = source[0];
    unsigned int second = source[1];
    unsigned int third = source[2];
    callbackThisAdjustment = first;
    callbackVtableOffset = second;
    callbackFunction = third;
    callbackTarget = value;
}

#pragma use_lmw_stmw on
namespace ns4 {
extern "C" {
// Apply allowed end-of-round rewards to the current shared save/round state.
// Uses shared save/round state and reward tables; no arguments or return value.
// Normal mode settles rank rewards, Pang and table bonus, then caps each balance.
// Alternate modes settle independent maximum Pang and bonus-Pang across eligible
// players into the selected account (selection clamped 0..3). No RNG calls here.
void GolfRule_UpdateRoundRewards() {
    BD74Tables* tables = (BD74Tables*)s_roundRewardTables;
    unsigned char* manager = (unsigned char*)g_sharedDoc;
    // Phase 1: rank/rule table reward, best-rank category and rank-table bonus.
    if (AsCSharedDoc(manager)->gameMode == 0) {
        int previous = ROUND_ACCOUNT_I32(manager, bestRankRewardIndex);
        register int score;
        if (!manager[0x8C0]) {
            unsigned char* item = (unsigned char*)GolfPlayer_GetEntryByIndex(BD74_I(manager, 0xB90), 0);
            score = *(int*)((unsigned int)((int*)((char*)(tables->scores))) + (BD74_R(item) - 1) * 4) *
                    (BD74_I(SharedDocBytes(), 0xB84) / 2 + 1);
            score = score * AsCSharedDoc(SharedDocBytes())->roundHoleCount / 18;
            *(int*)&((RoundRewardPlayerView*)(GolfPlayerEntryWordDoc(0)))->roundRankReward = score;
            *(unsigned int*)&((RoundRewardAccountView*)(manager))->rankRewardBalance += score;
            *(unsigned int*)&((RoundRewardAccountView*)(manager))->rankRewardSubtotal += score;
            switch (BD74_R(GolfPlayerEntryWordDoc(0))) {
            case 1:
                item = GolfPlayerEntryWordDoc(0);
                *(int*)&((RoundRewardPlayerView*)(item))->rankRewardIndex = 0;
                break;
            case 2:
                item = GolfPlayerEntryWordDoc(0);
                *(int*)&((RoundRewardPlayerView*)(item))->rankRewardIndex = 1;
                break;
            case 3:
                item = GolfPlayerEntryWordDoc(0);
                *(int*)&((RoundRewardPlayerView*)(item))->rankRewardIndex = 2;
                break;
            case 4:
                item = GolfPlayerEntryWordDoc(0);
                *(int*)&((RoundRewardPlayerView*)(item))->rankRewardIndex = 3;
                break;
            case 5:
                item = GolfPlayerEntryWordDoc(0);
                *(int*)&((RoundRewardPlayerView*)(item))->rankRewardIndex = 4;
                break;
            case 6:
                item = GolfPlayerEntryWordDoc(0);
                *(int*)&((RoundRewardPlayerView*)(item))->rankRewardIndex = 5;
                break;
            case 7:
                item = GolfPlayerEntryWordDoc(0);
                *(int*)&((RoundRewardPlayerView*)(item))->rankRewardIndex = 6;
                break;
            case 8:
                item = GolfPlayerEntryWordDoc(0);
                *(int*)&((RoundRewardPlayerView*)(item))->rankRewardIndex = 7;
                break;
            }
            item = GolfPlayerEntryWordDoc(0);
            int* minimum = (int*)(item + (int)&((RoundRewardPlayerView*)0)->rankRewardIndex);
            minimum = *minimum < ROUND_ACCOUNT_I32(manager, bestRankRewardIndex)
                          ? minimum
                          : (int*)(manager + (int)&((RoundRewardAccountView*)0)->bestRankRewardIndex);
            ROUND_ACCOUNT_I32(manager, bestRankRewardIndex) = *minimum;
            item = GolfPlayerEntryWordDoc(0);
            unsigned char* dest = GolfPlayerEntryWordDoc(0);
            unsigned int rank = BD74_R(item);
            item = (unsigned char*)((int*)((char*)(tables->values)));
            *(short*)&((RoundRewardPlayerView*)(dest))->rankTableBonus = ((int*)item)[rank - 1];
            dest = GolfPlayerEntryWordDoc(0);
            rank = BD74_R(dest);
            ROUND_ACCOUNT_S16(manager, rankTableBonusBalance) += ((int*)item)[rank - 1];
        } else if (manager[0x8C0] == 1) {
            unsigned char* item = (unsigned char*)GolfPlayer_GetEntryByIndex(BD74_I(manager, 0xB90), 0);
            score = *(int*)((unsigned int)((int*)((char*)(tables->scores2))) +
                            (AsCSharedDoc(SharedDocBytes())->roundHoleCount - 1) * 8 + (BD74_R(item) - 1) * 4);
            score *= (*(int*)&((CSharedDoc*)(SharedDocBytes()))->enemyLevelIndex / 2 + 1);
            *(int*)&((RoundRewardPlayerView*)(GolfPlayerEntryWordDoc(0)))->roundRankReward = score;
            int rank = 0;
            *(unsigned int*)&((RoundRewardAccountView*)(manager))->rankRewardBalance += score;
            *(unsigned int*)&((RoundRewardAccountView*)(manager))->rankRewardSubtotal += score;
            if (BD74_R(GolfPlayerEntryWordDoc(0)) == 1) {
                rank = (*(int*)&((RoundRewardPlayerView*)(GolfPlayerEntryWordDoc(1)))->matchScore <=
                        (int)AsCSharedDoc(SharedDocBytes())->roundHoleCount / 3) +
                       2;
            }
            switch (rank) {
            case 3:
                item = GolfPlayerEntryWordDoc(0);
                *(int*)&((RoundRewardPlayerView*)(item))->rankRewardIndex = 0;
                break;
            case 2:
                item = GolfPlayerEntryWordDoc(0);
                *(int*)&((RoundRewardPlayerView*)(item))->rankRewardIndex = 1;
                break;
            default:
                item = GolfPlayerEntryWordDoc(0);
                *(int*)&((RoundRewardPlayerView*)(item))->rankRewardIndex = 8;
                break;
            }
            item = GolfPlayerEntryWordDoc(0);
            int* minimum = (int*)(item + (int)&((RoundRewardPlayerView*)0)->rankRewardIndex);
            minimum = *minimum < ROUND_ACCOUNT_I32(manager, bestRankRewardIndex)
                          ? minimum
                          : (int*)(manager + (int)&((RoundRewardAccountView*)0)->bestRankRewardIndex);
            ROUND_ACCOUNT_I32(manager, bestRankRewardIndex) = *minimum;
            item = GolfPlayerEntryWordDoc(0);
            int value = *(int*)((unsigned int)((int*)((char*)(tables->values2))) + rank * 4);
            *(short*)&((RoundRewardPlayerView*)(item))->rankTableBonus = value;
            *(short*)&((RoundRewardAccountView*)(manager))->rankTableBonusBalance += value;
        }
        // Phase 2: first-rank Pang, optional repeated Pang award, then independent caps.
        if (previous != 0 && *(int*)&((RoundRewardAccountView*)(manager))->bestRankRewardIndex == 0)
            *(int*)&((RoundRewardPlayerView*)(GolfPlayerEntryWordDoc(0)))->bonusPang += kFirstRankPangBonus;
        unsigned char* item = GolfPlayerEntryWordDoc(0);
        *(unsigned int*)&((RoundRewardAccountView*)(manager))->pangBalance =
            *(int*)&((RoundRewardPlayerView*)(item))->bonusPang +
            (*(int*)&((RoundRewardPlayerView*)(GolfPlayerEntryWordDoc(0)))->pang +
             *(unsigned int*)&((RoundRewardAccountView*)(manager))->pangBalance);
        if (*(unsigned char*)&((RoundRewardPlayerView*)(GolfPlayerEntryWordDoc(0)))->repeatPangAward) {
            item = GolfPlayerEntryWordDoc(0);
            *(unsigned int*)&((RoundRewardAccountView*)(manager))->pangBalance =
                *(int*)&((RoundRewardPlayerView*)(item))->bonusPang +
                (*(int*)&((RoundRewardPlayerView*)(GolfPlayerEntryWordDoc(0)))->pang +
                 *(unsigned int*)&((RoundRewardAccountView*)(manager))->pangBalance);
        }
        *(unsigned int*)&((RoundRewardAccountView*)(manager))->rankRewardBalance =
            *(unsigned int*)&((RoundRewardAccountView*)(manager))->rankRewardBalance >
                    (unsigned int)kMaximumRewardBalance
                ? (unsigned int)kMaximumRewardBalance
                : *(unsigned int*)&((RoundRewardAccountView*)(manager))->rankRewardBalance;
        *(unsigned int*)&((RoundRewardAccountView*)(manager))->rankRewardSubtotal =
            *(unsigned int*)&((RoundRewardAccountView*)(manager))->rankRewardSubtotal >
                    (unsigned int)kMaximumRewardSubtotal
                ? (unsigned int)kMaximumRewardSubtotal
                : *(unsigned int*)&((RoundRewardAccountView*)(manager))->rankRewardSubtotal;
        short s = *(short*)&((RoundRewardAccountView*)(manager))->rankTableBonusBalance;
        *(short*)&((RoundRewardAccountView*)(manager))->rankTableBonusBalance =
            s < 0 ? 0 : (s > kMaximumRankTableBonus ? kMaximumRankTableBonus : s);
        *(unsigned int*)&((RoundRewardAccountView*)(manager))->pangBalance =
            *(unsigned int*)&((RoundRewardAccountView*)(manager))->pangBalance > (unsigned int)kMaximumRewardBalance
                ? (unsigned int)kMaximumRewardBalance
                : *(unsigned int*)&((RoundRewardAccountView*)(manager))->pangBalance;
    } else if (AsCSharedDoc(manager)->gameMode == 1 || *(int*)&((CSharedDoc*)(manager))->partyGameMode) {
        // Alternate modes settle the largest active-player Pang pair into the selected account.
        int course = ((CGolfDoc*)((*(void**)&AsCSharedDoc(manager)->golfDoc)))->SelectPlayerBySnapshotWords();
        course = course < 0 ? 0 : (course > 3 ? 3 : course);
        int high = 0, second = 0;
        for (int i = 0;
             i <
             (unsigned char)((CGolfDoc*)(*(void**)&((CSharedDoc*)(SharedDocBytes()))->golfDoc))->GetActivePlayerCount();
             ++i) {
            if (!*(int*)&((RoundRewardPlayerView*)(GolfPlayerEntryWordDoc((unsigned char)i)))->isComputer) {
                int a = *(int*)&((RoundRewardPlayerView*)(GolfPlayerEntryWordDoc((unsigned char)i)))->pang;
                high = BD74Max(high, a);
                int b = *(int*)&((RoundRewardPlayerView*)(GolfPlayerEntryWordDoc((unsigned char)i)))->bonusPang;
                second = BD74Max(second, b);
            }
        }
        unsigned char* slot = SharedDocBytes() + course * 0x1D4;
        *(unsigned int*)&((RoundRewardAccountView*)(slot))->pangBalance =
            (*(unsigned int*)&((RoundRewardAccountView*)(slot))->pangBalance + high) + second;
        *(unsigned int*)&((RoundRewardAccountView*)(slot))->pangBalance =
            *(unsigned int*)&((RoundRewardAccountView*)(slot))->pangBalance > (unsigned int)kMaximumRewardBalance
                ? (unsigned int)kMaximumRewardBalance
                : *(unsigned int*)&((RoundRewardAccountView*)(slot))->pangBalance;
    }
}
}
}

#pragma use_lmw_stmw reset
namespace ns4 {
extern "C" {
// Clear item-request state and refresh the first-player request context.
// Uses first-player/shared reward state; unused is ignored. Nonzero arg commits
// Tour completion/rewards, zero clears global requests, then sets string context.
// Initializes rank request data, settles rule-specific rewards/Pang and caps balances.
// Requires valid rank/table indices; no bounds-error or failure return is provided.
void GolfRule_ResetItemRequestState(void* unused, void* arg) {
    if (arg)
        TourMap_CommitNodeCompletionAndRewards();
    else
        ItemRequestState_ClearGlobalEntries();
    SharedStringState_SetContext(arg);

    unsigned char* first = FirstGolfPlayerEntryFromShared(SharedDocBytes());
    ((RoundRewardPlayerView*)(first))->rankRewardIndex = 8;
    unsigned char* second = FirstGolfPlayerEntryFromShared(SharedDocBytes());
    ((GolfDocSnapshot8013*)(second))->unk_16C = 0;
    unsigned char* state = SharedDocBytes();
    unsigned char mode = ((CSharedDoc*)(state))->golfRuleType;
    if (mode == 0) {
        unsigned char* player = FirstGolfPlayerEntryFromShared(state);
        unsigned char* g = SharedDocBytes();
        int tableIndex = ((((GolfDocSnapshot8013*)(player))->rankFlags >> 4) & 0xF) - 1;
        int value = s_strokePlayRankPangRewards[tableIndex];
        value *= *(int*)&((CSharedDoc*)(g))->enemyLevelIndex / 2 + 1;
        value *= ((CSharedDoc*)(g))->roundHoleCount;
        value /= 18;
        ((RoundRewardPlayerView*)(FirstGolfPlayerEntryFromShared(g)))->roundRankReward = value;
        *(int*)&((RoundRewardAccountView*)(state))->rankRewardBalance += value;
        *(int*)&((RoundRewardAccountView*)(state))->rankRewardSubtotal += value;
    } else if (mode == 1) {
        unsigned char* player = FirstGolfPlayerEntryFromShared(state);
        unsigned char* g = SharedDocBytes();
        int value = s_matchPlayHoleCountPangRewards[((CSharedDoc*)(g))->roundHoleCount - 1]
                                                   [((((GolfDocSnapshot8013*)(player))->rankFlags >> 4) & 0xF) - 1];
        value *= *(int*)&((CSharedDoc*)(g))->enemyLevelIndex / 2 + 1;
        ((RoundRewardPlayerView*)(FirstGolfPlayerEntryFromShared(g)))->roundRankReward = value;
        *(int*)&((RoundRewardAccountView*)(state))->rankRewardBalance += value;
        *(int*)&((RoundRewardAccountView*)(state))->rankRewardSubtotal += value;
    }

    unsigned char* player = FirstGolfPlayerEntryFromShared(SharedDocBytes());
    unsigned char* next = FirstGolfPlayerEntryFromShared(SharedDocBytes());
    *(int*)&((RoundRewardAccountView*)(state))->pangBalance =
        (*(int*)&((GolfDocSnapshot8013*)(next))->roundResultWords[1] +
         *(int*)&((GolfDocSnapshot8013*)(player))->roundResultWords[2]) +
        *(int*)&((RoundRewardAccountView*)(state))->pangBalance;
    if (((GolfDocSnapshot8013*)(FirstGolfPlayerEntryFromShared(SharedDocBytes())))->unk_16E) {
        player = FirstGolfPlayerEntryFromShared(SharedDocBytes());
        next = FirstGolfPlayerEntryFromShared(SharedDocBytes());
        *(int*)(state + 0x48) = (*(int*)(next + 0xCC) + *(int*)(player + 0xD0)) + *(int*)(state + 0x48);
    }
    unsigned int v = *(unsigned int*)(state + 0x44);
    *(unsigned int*)(state + 0x44) = v > 0x05F5E0FF ? 0x05F5E0FF : v;
    v = *(unsigned int*)(state + 0x1C4);
    *(unsigned int*)(state + 0x1C4) = v > 0x000F423F ? 0x000F423F : v;
    v = *(unsigned int*)(state + 0x48);
    *(unsigned int*)(state + 0x48) = v > 0x05F5E0FF ? 0x05F5E0FF : v;
}
}
}

// Choose result poses, score text, camera and music for a hole or forced result.
// Requires rule/result point and current player. arg2 selects the forced result
// path; otherwise hole strokes and par choose pose, sequence and score text.
// arg1 skips the initial actor placement when nonzero. Writes the presentation
// duration/camera and sends actor/UI/audio requests; string buffers are stack-owned.
// Static, taking the rule object explicitly: a member form that uses this scores lower.
void CGolfRule::StartRoundResultPresentation(char* self, int arg1, int arg2, char* arg3) {
    using namespace ns4;
    struct {
        float matrix[12];
        char a[32], b[32], c[32];
    } storage;
    C6WordVec position;
    CE00Vec table, end1, end0, sum, offset, camera;
    C6WordVec copy;
    CE00Vec sum1, off1, sum0, off0;
    float angle;
    unsigned char valid;
    int delta;
    if (arg2)
        delta = 1;
    else if (AsCSharedDoc(SharedDocBytes())->golfRuleType != 1) {
        unsigned char* player =
            (unsigned char*)GolfPlayer_GetEntryByIndex((int)GolfDocBytes(), AsCGolfDoc(GolfDocBytes())->currentPlayer);
        unsigned char* control = GolfDocBytes();
        int slot = ((CGolfDoc*)(control))->currentHoleIndex;
        unsigned char* record = *(unsigned char**)&((CGolfDoc*)(control))->holeRecords;
        delta = (player + slot)[0x28] - (record + (slot << 5))[0x18];
    } else {
        unsigned char* player =
            (unsigned char*)GolfPlayer_GetEntryByIndex((int)GolfDocBytes(), AsCGolfDoc(GolfDocBytes())->currentPlayer);
        unsigned char* control = GolfDocBytes();
        int slot = ((CGolfDoc*)(control))->currentHoleIndex;
        unsigned char* record = *(unsigned char**)&((CGolfDoc*)(control))->holeRecords;
        delta = (player + slot)[0x28] - (record + (slot << 5))[0x18];
    }
    int mode;
    if (arg2)
        mode = 0;
    else if (*(int*)&((CSharedDoc*)(SharedDocBytes()))->partyGameMode == 1)
        mode = 1;
    else {
        unsigned char* player =
            (unsigned char*)GolfPlayer_GetEntryByIndex((int)GolfDocBytes(), AsCGolfDoc(GolfDocBytes())->currentPlayer);
        mode = (player + AsCGolfDoc(GolfDocBytes())->currentHoleIndex)[0x28];
    }
    if (mode == 1) {
        ::strcpy(storage.a, golfRuleStrings + kText_albatrossVictoryPose);
        ::strcpy(storage.b, golfRuleStrings + kText_holeInOneSequence);
        ::strcpy(storage.c, golfRuleStrings + kText_holeInOneSequence);
    } else {
        switch (delta) {
        case -3:
            ::strcpy(storage.a, golfRuleStrings + kText_albatrossVictoryPose);
            ::strcpy(storage.b, golfRuleStrings + kText_holeInOneSequence);
            ::strcpy(storage.c, golfRuleStrings + kText_albatrossSequence);
            break;
        case -2:
            ::strcpy(storage.a, golfRuleStrings + kText_holeInOneVictoryPose);
            ::strcpy(storage.b, golfRuleStrings + kText_hole_in);
            ::strcpy(storage.c, golfRuleStrings + kText_eagleSequence);
            break;
        case -1:
            ::strcpy(storage.a, golfRuleStrings + kText_eagleBirdieVictoryPose);
            ::strcpy(storage.b, golfRuleStrings + kText_hole_in);
            ::strcpy(storage.c, golfRuleStrings + kText_birdieSequence);
            break;
        case 0:
            ::strcpy(storage.a, golfRuleStrings + kText_saveParVictoryPose);
            ::strcpy(storage.b, golfRuleStrings + kText_hole_in);
            ::strcpy(storage.c, golfRuleStrings + kText_parSequence);
            break;
        case 1:
            ::strcpy(storage.a, golfRuleStrings + kText_bogeyDisqualifiedDisappointmentPose);
            ::strcpy(storage.b, golfRuleStrings + kText_disqualifiedSequence);
            ::strcpy(storage.c, golfRuleStrings + kText_bogeySequence);
            break;
        case 2:
            ::strcpy(storage.a, golfRuleStrings + kText_bogeyDisqualifiedDisappointmentPose);
            ::strcpy(storage.b, golfRuleStrings + kText_disqualifiedSequence);
            ::strcpy(storage.c, golfRuleStrings + kText_doubleBogeySequence);
            break;
        case 3:
            ::strcpy(storage.a, golfRuleStrings + kText_bogeyDisqualifiedDisappointmentPose);
            ::strcpy(storage.b, golfRuleStrings + kText_disqualifiedSequence);
            ::strcpy(storage.c, golfRuleStrings + kText_tripleBogeySequence);
            break;
        default:
            ::strcpy(storage.a, golfRuleStrings + kText_bogeyDisqualifiedDisappointmentPose);
            ::strcpy(storage.b, golfRuleStrings + kText_disqualifiedSequence);
            unsigned char* state = (unsigned char*)GolfBall__Fv();
            if (C6Decode(state) == 1)
                sprintf(storage.c, golfRuleStrings + kText_overParScoreFormat, delta);
            else
                ::strcpy(storage.c, golfRuleStrings + kText_scoreUnit);
            break;
        }
    }
    if (!arg1) {
        IActor_DispatchMessage(self, self, golfRuleStrings + kText_player, 0x34, (int)&valid, 0, 0, 0);
        angle = valid ? kFloat_0_GolfRule : kFloat_3_1415927_GolfRule;
        position = *(C6WordVec*)(self + 0x84);
        IActor_DispatchMessage(self, self, golfRuleStrings + kText_player, 0x2718, (int)(self + 0x84), 0, 0, 0);
        IActor_DispatchMessage(self, self, golfRuleStrings + kText_player, 0x2717, (int)&angle, 0, 0, 0);
        IActor_DispatchMessage(self, self, golfRuleStrings + kText_player, 10, 0, 0, 0, 0);
        unsigned int character =
            (unsigned char)*(int*)(SharedDocBytes() + AsCGolfDoc(GolfDocBytes())->currentPlayer * 0x1D4 + 0x120);
        int category = C6Category(mode, delta);
        offset.y = kFloat_0_GolfRule;
        table = s_characterResultCameraPresets[character][category];
        angle = table.z;
        offset.x = table.x;
        offset.z = table.y;
        __pl__FRC7WVectorRC7WVector(&sum, self + 0x84, &offset);
        *(WVector*)&position = *(WVector*)&sum;
        copy = position;
        GetGroundPoint__9CQuadTreeF7WVectorbPib(&camera, CQuadTree_GetInstance(), &copy, 0, 0, 0);
        *(WVector*)&position = *(WVector*)&camera;
        IActor_DispatchMessage(self, self, golfRuleStrings + kText_caddie, 5, 0, 0, 0, 0);
        IActor_DispatchMessage(self, self, golfRuleStrings + kText_caddie, 0x14, (int)&angle, 0, 0, 0);
        IActor_DispatchMessage(self, self, golfRuleStrings + kText_caddie, 0x2718, (int)&position, 0, 0, 0);
    }
    if (arg2) {
        if (arg3)
            SoundManager_RequestBgmChange(g_soundManager, arg3, 1, 1);
    } else if (delta <= 0)
        SoundManager_RequestBgmChange(g_soundManager, golfRuleStrings + kText_cupin_underpar_wav, 1, 1);
    else
        SoundManager_RequestBgmChange(g_soundManager, golfRuleStrings + kText_cupin_overpar_wav, 1, 1);
    if (!arg1) {
        IActor_DispatchMessage(self, self, golfRuleStrings + kText_player, 0x2715, (int)storage.a, 0, 0, 0);
        IActor_DispatchMessage(self, self, golfRuleStrings + kText_player, 0x2716,
                               (int)(golfRuleStrings + kText_emptyTaskArgument), 0, 0, 0);
        IActor_DispatchMessage(self, self, golfRuleStrings + kText_caddie, 0x2715, (int)storage.b, 0, 0, 0);
        float duration = ((CCameraWork*)g_cameraWork)->StartGroupAnimationAtPivot(0, storage.a, (int*)(self + 0x84));
        float threshold = kFloat_0_GolfRule;
        *(float*)(self + 0x9C) = duration;
        if (*(float*)(self + 0x9C) < threshold) {
            off1.x = threshold;
            off1.y = kFloat_6_4_GolfRule;
            off1.z = kFloat_neg32_GolfRule;
            __pl__FRC7WVectorRC7WVector(&sum1, self + 0x84, &off1);
            *(WVector*)&end1 = *(WVector*)&sum1;
            off0.x = kFloat_0_GolfRule;
            off0.y = kFloat_6_4_GolfRule;
            off0.z = kFloat_0_GolfRule;
            __pl__FRC7WVectorRC7WVector(&sum0, self + 0x84, &off0);
            *(WVector*)&end0 = *(WVector*)&sum0;
            Matrix_BuildCameraTransform(&storage.matrix, &end1, &end0);
            Matrix_CopyAffine(s_cameraTransform, &storage.matrix);
            *(float*)(self + 0x9C) = kFloat_3_5_GolfRule;
        }
    }
    if (!arg2)
        IActor_DispatchMessage(self, self, golfRuleStrings + kText_letter, 1, (int)storage.c, 0, 0, 0);
    GolfTipsDialog_RequestClose(-1, kFloat_0_5_GolfRule);
}

// Choose the hole-result animation, camera and music from player scores and rules.
// Match-play admission: returns 1 immediately for other rule types, 0 if neither
// player is finished (state 1). With one finished, the unfinished player may be
// completed here when it cannot improve the hole result; an otherwise losing
// finished player waits unless it trails by exactly one stroke and its match score
// exceeds the opponent's plus the remaining holes (roundHoleCount - (hole+1)).
// The admitted unfinished player gets one extra stroke and state 1; completed-hole
// count guards totals from being added twice. State 3 is excluded from settlement.
// After admission, publishes result poses, camera duration, text and win/lose BGM.
// Requires two player records for match play; no pointer/null recovery is provided.
// Static, taking the rule view explicitly: the member form that uses this scores lower.
int CGolfRule::StartHoleResultPresentation(CE00Self* self) {
    using namespace ns4;
    struct Frame {
        bool flags[2];
        CE00Vec offset0, temp0, offset1, temp1, point0, point1;
        char pad54[4];
        float matrix[12];
        char text[32], suffix[32], spoken[32], resultText[32];
    } frame;
    // Phase 1: gate match-play presentation and compare player completion/results.
    if (AsCSharedDoc(SharedDocBytes())->golfRuleType != 1)
        return 1;
    int state = ((GolfDocSnapshot8013*)(GolfPlayerEntry(0)))->playerState;
    unsigned char* control = GolfDocBytes();
    CE00Manager* endManager;
    frame.flags[0] = state == 1;
    frame.flags[1] = ((GolfDocSnapshot8013*)(GolfPlayer_GetEntryByIndex((int)control, 1)))->playerState == 1;
    if (!frame.flags[0] && !frame.flags[1])
        return 0;
    if (((unsigned char*)frame.flags)[0] ^ ((unsigned char*)frame.flags)[1]) {
        if (((unsigned char*)frame.flags)[0]) {
            unsigned char *p0, *p1;
            unsigned int hole0, hole1;
            hole1 = AsCGolfDoc(GolfDocBytes())->currentHoleIndex;
            p1 = GolfPlayerEntry(1);
            hole0 = AsCGolfDoc(GolfDocBytes())->currentHoleIndex;
            p0 = GolfPlayerEntry(0);
            if (AsGolfDocSnapshot(p0)->holeStrokeCounts[hole0] > AsGolfDocSnapshot(p1)->holeStrokeCounts[hole1]) {
                unsigned int h1 = AsCGolfDoc(GolfDocBytes())->currentHoleIndex;
                unsigned char* a = GolfPlayerEntry(1);
                unsigned int h0 = AsCGolfDoc(GolfDocBytes())->currentHoleIndex;
                unsigned char* b = GolfPlayerEntry(0);
                unsigned int limit, h;
                unsigned char *c, *d;
                if (AsGolfDocSnapshot(b)->holeStrokeCounts[h0] != AsGolfDocSnapshot(a)->holeStrokeCounts[h1] + 1 ||
                    (limit = AsCSharedDoc(SharedDocBytes())->roundHoleCount, c = GolfPlayerEntry(1),
                     h = AsCGolfDoc(GolfDocBytes())->currentHoleIndex, d = GolfPlayerEntry(0),
                     *(int*)&((GolfDocSnapshot8013*)(d))->roundResultWords[0] <=
                         *(int*)&((GolfDocSnapshot8013*)(c))->roundResultWords[0] + (int)limit - (int)(h + 1)))
                    return 0;
            }
            unsigned int h = AsCGolfDoc(GolfDocBytes())->currentHoleIndex;
            unsigned char* p = GolfPlayerEntry(1);
            ++AsGolfDocSnapshot(p)->holeStrokeCounts[h];
            if (AsCSharedDoc(SharedDocBytes())->golfRuleType == 1 && AsCGolfDoc(GolfDocBytes())->playerCount > 1 &&
                ((GolfDocSnapshot8013*)(GolfPlayerEntry(1)))->playerState != 3) {
                unsigned char* cleared = GolfPlayerEntry(1);
                ((GolfDocSnapshot8013*)(cleared))->itemId = 0;
                int hole = AsCGolfDoc(GolfDocBytes())->currentHoleIndex;
                unsigned char* current = GolfPlayerEntry(1);
                if (*(int*)&((GolfDocSnapshot8013*)(current))->roundResultWords[3] == hole) {
                    if (!((CGolfDoc*)(GolfDocBytes()))->unk_35C) {
                        unsigned int h = AsCGolfDoc(GolfDocBytes())->currentHoleIndex;
                        unsigned char* a = GolfPlayerEntry(1);
                        unsigned char* b = GolfPlayerEntry(1);
                        ((GolfDocSnapshot8013*)(b))->totalStrokeCount += AsGolfDocSnapshot(a)->holeStrokeCounts[h];
                        if (AsCSharedDoc(SharedDocBytes())->golfRuleType != 1) {
                            unsigned int h = AsCGolfDoc(GolfDocBytes())->currentHoleIndex;
                            unsigned char* a = GolfPlayerEntry(1);
                            unsigned char* record = *(unsigned char**)&((CGolfDoc*)(GolfDocBytes()))->holeRecords +
                                                    (AsCGolfDoc(GolfDocBytes())->currentHoleIndex << 5);
                            unsigned char* b = GolfPlayerEntry(1);
                            *(int*)&((GolfDocSnapshot8013*)(b))->roundResultWords[0] +=
                                AsGolfDocSnapshot(a)->holeStrokeCounts[h] - ((ShotHoleRecord*)(record))->par;
                        }
                    }
                    ++*(int*)&((GolfDocSnapshot8013*)(GolfPlayerEntry(1)))->roundResultWords[3];
                }
                unsigned char* finished = GolfPlayerEntry(1);
                ((GolfDocSnapshot8013*)(finished))->playerState = 1;
                endManager = (CE00Manager*)SharedDocBytes();
                if (endManager->mode == 6) {
                    int value = PartyGame_EvaluatePlayerThresholdTier(1);
                    endManager->result = value;
                }
            }
        } else {
            unsigned int hole1 = AsCGolfDoc(GolfDocBytes())->currentHoleIndex;
            unsigned char* p1 = GolfPlayerEntry(1);
            unsigned int hole0 = AsCGolfDoc(GolfDocBytes())->currentHoleIndex;
            unsigned char* p0 = GolfPlayerEntry(0);
            if (AsGolfDocSnapshot(p0)->holeStrokeCounts[hole0] < AsGolfDocSnapshot(p1)->holeStrokeCounts[hole1]) {
                unsigned int h0 = AsCGolfDoc(GolfDocBytes())->currentHoleIndex;
                unsigned char* a = GolfPlayerEntry(0);
                unsigned int h1 = AsCGolfDoc(GolfDocBytes())->currentHoleIndex;
                unsigned char* b = GolfPlayerEntry(1);
                unsigned int limit, h;
                unsigned char *c, *d;
                if (AsGolfDocSnapshot(b)->holeStrokeCounts[h1] != AsGolfDocSnapshot(a)->holeStrokeCounts[h0] + 1 ||
                    (limit = AsCSharedDoc(SharedDocBytes())->roundHoleCount, c = GolfPlayerEntry(0),
                     h = AsCGolfDoc(GolfDocBytes())->currentHoleIndex, d = GolfPlayerEntry(1),
                     *(int*)&((GolfDocSnapshot8013*)(d))->roundResultWords[0] <=
                         *(int*)&((GolfDocSnapshot8013*)(c))->roundResultWords[0] + (int)limit - (int)(h + 1)))
                    return 0;
            }
            unsigned int h = AsCGolfDoc(GolfDocBytes())->currentHoleIndex;
            unsigned char* p = GolfPlayerEntry(0);
            ++AsGolfDocSnapshot(p)->holeStrokeCounts[h];
            if (AsCSharedDoc(SharedDocBytes())->golfRuleType == 1 && AsCGolfDoc(GolfDocBytes())->playerCount != 0 &&
                ((GolfDocSnapshot8013*)(GolfPlayerEntry(0)))->playerState != 3) {
                unsigned char* cleared = GolfPlayerEntry(0);
                ((GolfDocSnapshot8013*)(cleared))->itemId = 0;
                int hole = AsCGolfDoc(GolfDocBytes())->currentHoleIndex;
                unsigned char* current = GolfPlayerEntry(0);
                if (*(int*)&((GolfDocSnapshot8013*)(current))->roundResultWords[3] == hole) {
                    if (!((CGolfDoc*)(GolfDocBytes()))->unk_35C) {
                        unsigned int h = AsCGolfDoc(GolfDocBytes())->currentHoleIndex;
                        unsigned char* a = GolfPlayerEntry(0);
                        unsigned char* b = GolfPlayerEntry(0);
                        ((GolfDocSnapshot8013*)(b))->totalStrokeCount += AsGolfDocSnapshot(a)->holeStrokeCounts[h];
                        if (AsCSharedDoc(SharedDocBytes())->golfRuleType != 1) {
                            unsigned int h = AsCGolfDoc(GolfDocBytes())->currentHoleIndex;
                            unsigned char* a = GolfPlayerEntry(0);
                            unsigned char* record = *(unsigned char**)&((CGolfDoc*)(GolfDocBytes()))->holeRecords +
                                                    (AsCGolfDoc(GolfDocBytes())->currentHoleIndex << 5);
                            unsigned char* b = GolfPlayerEntry(0);
                            *(int*)&((GolfDocSnapshot8013*)(b))->roundResultWords[0] +=
                                AsGolfDocSnapshot(a)->holeStrokeCounts[h] - ((ShotHoleRecord*)(record))->par;
                        }
                    }
                    ++*(int*)&((GolfDocSnapshot8013*)(GolfPlayerEntry(0)))->roundResultWords[3];
                }
                unsigned char* finished = GolfPlayerEntry(0);
                ((GolfDocSnapshot8013*)(finished))->playerState = 1;
                endManager = (CE00Manager*)SharedDocBytes();
                if (endManager->mode == 6) {
                    int value = PartyGame_EvaluatePlayerThresholdTier(0);
                    endManager->result = value;
                }
            }
        }
    }
    // Phase 2: choose result camera/animation and its display duration.
    IActor_DispatchMessage(self, self, golfRuleStrings + kText_caddie, 0xE, 0, 0, 0, 0);
    unsigned int active = (unsigned char)((CGolfDoc*)GolfDocBytes())->ResolveMatchTurnFromHoleStrokes();
    int result;
    state = ((GolfDocSnapshot8013*)(GolfPlayerEntry(0)))->playerState;
    control = GolfDocBytes();
    frame.flags[0] = state == 1;
    frame.flags[1] = ((GolfDocSnapshot8013*)(GolfPlayer_GetEntryByIndex((int)control, 1)))->playerState == 1;
    if (frame.flags[0] && frame.flags[1]) {
        unsigned char h0, h1, otherHole;
        int other = 1 - active;
        h0 = AsCGolfDoc(GolfDocBytes())->currentHoleIndex;
        unsigned char* a = GolfPlayerEntry((unsigned char)other);
        h1 = AsCGolfDoc(GolfDocBytes())->currentHoleIndex;
        unsigned char* b = GolfPlayerEntry(active);
        if (AsGolfDocSnapshot(b)->holeStrokeCounts[h1] > AsGolfDocSnapshot(a)->holeStrokeCounts[h0])
            result = -1;
        else {
            unsigned char* b;
            otherHole = AsCGolfDoc(GolfDocBytes())->currentHoleIndex;
            unsigned char* a = GolfPlayerEntry((unsigned char)other);
            h0 = AsCGolfDoc(GolfDocBytes())->currentHoleIndex;
            b = GolfPlayerEntry(active);
            if (AsGolfDocSnapshot(b)->holeStrokeCounts[h0] == AsGolfDocSnapshot(a)->holeStrokeCounts[otherHole])
                result = 0;
            else
                result = 1;
        }
    } else {
        result = -1;
        if (frame.flags[active])
            result = 1;
    }
    if (result == -1) {
        ::strcpy(frame.text, golfRuleStrings + kText_bogeyDisqualifiedDisappointmentPose);
        ::strcpy(frame.suffix, golfRuleStrings + kText_thirdPlaceAnimation);
        ::strcpy(frame.spoken, golfRuleStrings + kText_bogeyDisqualifiedDisappointmentPose);
        ::strcpy(frame.resultText, golfRuleStrings + kText_lose);
    } else {
        unsigned char* p;
        unsigned char h = AsCGolfDoc(GolfDocBytes())->currentHoleIndex;
        p = GolfPlayerEntry(active);
        if (AsGolfDocSnapshot(p)->holeStrokeCounts[h] == 1)
            ::strcpy(frame.text, golfRuleStrings + kText_albatrossVictoryPose);
        else {
            unsigned char h = AsCGolfDoc(GolfDocBytes())->currentHoleIndex;
            unsigned char* p = GolfPlayerEntry(active);
            int score = AsGolfDocSnapshot(p)->holeStrokeCounts[h];
            unsigned char* control = GolfDocBytes();
            unsigned char* record = *(unsigned char**)&((CGolfDoc*)(control))->holeRecords +
                                    (((CGolfDoc*)(control))->currentHoleIndex << 5);
            switch (score - ((ShotHoleRecord*)(record))->par) {
            case -3:
                ::strcpy(frame.text, golfRuleStrings + kText_albatrossVictoryPose);
                break;
            case -2:
                ::strcpy(frame.text, golfRuleStrings + kText_holeInOneVictoryPose);
                break;
            case -1:
                ::strcpy(frame.text, golfRuleStrings + kText_eagleBirdieVictoryPose);
                break;
            default:
                ::strcpy(frame.text, golfRuleStrings + kText_saveParVictoryPose);
                break;
            }
        }
        if (result == 0)
            ::strcpy(frame.suffix, golfRuleStrings + kText_secondPlaceAnimation);
        else
            ::strcpy(frame.suffix, golfRuleStrings + kText_firstPlaceAnimation);
        ::strcpy(frame.spoken, frame.text);
        if (result == 0)
            ::strcpy(frame.resultText, golfRuleStrings + kText_draw);
        else
            ::strcpy(frame.resultText, golfRuleStrings + kText_win);
    }
    ((void (*)(void*, void*, const char*, int, void*, void*, void*, int))IActor_DispatchMessage)(
        self, self, golfRuleStrings + kText_player, 0x1A, frame.text, frame.suffix, &self->point, 0);
    float duration = ((CCameraWork*)g_cameraWork)->StartGroupAnimationAtPivot(0, frame.spoken, (int*)&self->point);
    float threshold = kFloat_0_GolfRule;
    self->duration = duration;
    if (self->duration < threshold) {
        frame.offset1.x = threshold;
        frame.offset1.y = kFloat_6_4_GolfRule;
        frame.offset1.z = kFloat_neg32_GolfRule;
        __pl__FRC7WVectorRC7WVector(&frame.temp1, &self->point, &frame.offset1);
        *(WVector*)&frame.point1 = *(WVector*)&frame.temp1;
        frame.offset0.x = kFloat_0_GolfRule;
        frame.offset0.y = kFloat_6_4_GolfRule;
        frame.offset0.z = kFloat_0_GolfRule;
        __pl__FRC7WVectorRC7WVector(&frame.temp0, &self->point, &frame.offset0);
        *(WVector*)&frame.point0 = *(WVector*)&frame.temp0;
        Matrix_BuildCameraTransform(frame.matrix, &frame.point1, &frame.point0);
        Matrix_CopyAffine(s_cameraTransform, frame.matrix);
        AsGolfRuleFrame(self)->stateTimer = kFloat_3_5_GolfRule;
    }
    // Phase 3: publish result text, HUD state and matching audio.
    IActor_DispatchMessage(self, self, golfRuleStrings + kText_letter, 1, (int)frame.resultText, 0, 0, 0);
    IActor_DispatchMessage(self, self, golfRuleStrings + kText_screen, 0x18, 0, 0, 0, 0);
    GolfTipsDialog_RequestClose(-1, kFloat_0_5_GolfRule);
    if (result >= 0)
        SoundManager_RequestBgmChange(g_soundManager, golfRuleStrings + kText_hole_win_wav, 1, 1);
    else
        SoundManager_RequestBgmChange(g_soundManager, golfRuleStrings + kText_hole_lose_wav, 1, 1);
    return 1;
}

// ===== S10 - Ball playback and shot outcomes =====
// ShotSample events, playback clock, stroke/penalty settlement.

// Advance precomputed flight samples, camera policy and completed-shot transitions.
// Entry: constructed rule, live ball/doc/currentPlayer [0,4), loaded course and
// actor table. delta is finite seconds >0 during fast playback: zero can prevent
// cursor progress and spin this loop. Pause or network wait returns before work.
// Callers borrow all objects; courseCameraBounds caches a course-owned record and
// must be cleared on reload. No resources are freed here. Camera-tag lookup copies
// into 64-byte name scratch; authored tokens must fit, since scanf has no bound.
// Decision map:
// 1. Double-power WOOD, classified non-miss: after camera duration, reset once
//    and preserve saved camera transform. presentation bit 2 suppresses free input.
// 2. Without presentation/tutorial blockers, button input action12 or motion
//    action11 retains random camera; otherwise clear its input offsets/request.
// 3. Entering a new course camera volume installs its named anchor; leaving all
//    volumes clears the cache and action 0x15's installed camera only.
// 4. fastPlayback is set by OnProcess's SkipFlight action for nonparty/nonputter
//    shots with >90 samples left and neither tutorial blocker. It begins batch
//    work only AFTER sample 15, and pauses while FadeState::captureRequested.
//    When admitted, disable sound and NEW attached-effect creation, update camera,
//    background, player, caddie (then its extra hook), NPC, ball, guide, trajectory,
//    effect, screen IN THAT ORDER, then unconditionally re-enable both globals.
//    This is not a saved-state restoration. All named actors and the CGolfCaddie
//    dynamic cast must succeed; callbacks may not destroy later actors or throw.
//    No cleanup guard restores globals on failure. Existing effects still update.
// 5. Moving ball -> UpdateBallPlayback; stopped -> ProcessShotOutcome and clear
//    fast mode. Stop batch loop when <=90 samples remain, cursor<=15, capture is
//    pending or fast mode cleared. No iteration cap: sample progress is required.
// Each iteration re-observes globals/singletons; keep repeated lookups and callback
// order. This plays stored physics samples; it does not integrate a new trajectory.
// Requires live rule/doc/input and completed simulation history.
// Pause/sync gates return before playback; accepted updates consume samples,
// handle skip/readiness and route the resulting presentation/outcome state.
void CGolfRule::ProcessBallFlight(float delta) {
    using namespace ns4;
    void* camera;
    void* actor;
    void* current;
    unsigned char* obj;
    obj = (unsigned char*)this;
    if (AsCGolfRule(obj)->processingPaused || AsCGolfRule(obj)->networkSyncPending)
        return;
    for (;;) {
        void* scene = InstanceTable_Get();
        // Double-power camera completion; retain the original nested tests and
        // separate InstanceTable_Get observations across condition evaluation.
        if (((ClubSelectionRuntime*)scene)->powerShotMode == kDoublePowerShotMode) {
            if (!((ClubSelectionRuntime*)InstanceTable_Get())->selectedClub->clubKind) {
                if (AsCGolfDoc((*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc))->impactGrade != kImpactMiss) {
                    if (AsCGolfDoc((*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc))->impactGrade !=
                        kImpactUnclassified) {
                        if (!AsGolfRuleFrame(obj)->doublePowerCameraReset) {
                            int flag = 0;
                            unsigned char* state = (unsigned char*)g_cameraWork;
                            float durationSeconds = AsCCameraWork(state)->duration;

                            if (durationSeconds > kFloat_0_GolfRule) {
                                if (AsCCameraWork(state)->animationTime > durationSeconds)
                                    flag = 1;
                            }
                            if (flag) {
                                ((CCameraWork*)g_cameraWork)->ResetMode(-1);
                                AsGolfRuleFrame(obj)->doublePowerCameraReset = 1;
                                AsCCameraWork(g_cameraWork)->useSavedCameraTransform = 1;
                            }
                        }
                    }
                }
            }
        }
        // 0x8015D800: Join after the optional shot-camera reset.
        do {
            if (!(AsGolfRuleFrame(obj)->shotPresentationFlags & kSpecialShotPresentation)) {
                if (!AsGolfRuleFrame(obj)->presentationStep) {
                    if (!((PlayerSwingInputView*)((char*)g_sharedDoc +
                                                  AsCGolfDoc((*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc))
                                                          ->currentPlayer *
                                                      0x1D4))
                             ->buttonSwingEnabled ||
                        GolfRule_PollShotCameraAction12()) {
                        if (!((PlayerSwingInputView*)((char*)g_sharedDoc +
                                                      AsCGolfDoc((*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc))
                                                              ->currentPlayer *
                                                          0x1D4))
                                 ->buttonSwingEnabled) {
                            if (GetInputManagerSlot() == 0) {
                                if (!s_inputManagerInitialized) {
                                    InputManager_Init(s_gameInputManager);
                                    __register_global_object(s_gameInputManager, InputManager_Dtor,
                                                             s_golfRuleInputDestructorRecord);
                                    s_inputManagerInitialized = 1;
                                }
                                GetInputManagerSlot() = s_gameInputManager;
                            }
                            if (Input_IsActionButtonDown(GetInputManagerSlot(), 11, -1))
                                break;
                        } else
                            break;
                    }
                    if (!AsGolfRuleFrame(obj)->tutorialFeedbackEnabled) {
                        if (!AsGolfRuleFrame(obj)->tutorialAdvancePending) {
                            if (AsCCameraWork(g_cameraWork)->randomModeRequested) {
                                unsigned char* st = (unsigned char*)g_cameraWork;
                                AsCCameraWork(st)->randomModeRequested = 0;
                                AsCCameraWork(st)->inputMoved = 0;
                                AsCCameraWork(st)->inputFovOffset = kFloat_0_GolfRule;
                                Vec2_Zero(AsCCameraWork(st)->inputRotationOffset);
                            }
                        }
                    }
                }
            }
        } while (0);

        camera = g_cameraWork;
        actor = *(void**)((char*)(*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc) + 8);
        void* playerState = GolfBall__Fv();
        current = CourseData_FindBoundsContainingPoint(actor, playerState);
        if (current && AsGolfRuleFrame(obj)->courseCameraBounds != current) {
            // Integerized pointer stages the original record+1 token cursor;
            // this is an address, not a numeric camera ID, and scanner borrows it.
            int id = (unsigned int)current + 1;

            char name[64];
            memset(name, 0xCD, sizeof(name));
            String_ScanTokensAfterSubstring((char*)&id, golfRuleStrings + kText_cameraResourceTag,
                                            golfRuleStrings + kText_stringFormat, name);
            void* found = CourseData_FindNamedRecord(actor, name);
            if (found)
                ((CCameraWork*)camera)->SetInstalledCamera(&((CourseCameraAnchor*)found)->position);
            AsGolfRuleFrame(obj)->courseCameraBounds = current;
        } else if (CourseData_FindBoundsContainingPoint(actor, GolfBall__Fv()) == 0) {
            AsGolfRuleFrame(obj)->courseCameraBounds = 0;
            if (AsCCameraWork(camera)->action == 0x15)
                ((CCameraWork*)camera)->UninstallCamera();
        }
        // Batch actor work is suppressed during capture; no actor pointer is retained.
        if (AsGolfRuleFrame(obj)->fastPlaybackByte) {
            if (AsCGolfBall(GolfBall__Fv())->m_eventPreDataIndex[kBallPlaybackCursor] > kFastPlaybackFirstSample) {
                if (!Sing_805DC608()) {
                    if (!s_fadeOverlayInitialized) {
                        FadeOverlay_Init(s_fadeOverlay);
                        __register_global_object(s_fadeOverlay, ResourceState_Dtor, s_fadeOverlayRegistration_804F72D8);
                        s_fadeOverlayInitialized = 1;
                    }
                    Sing_805DC608() = s_fadeOverlay;
                }
                if (!((::FadeState*)Sing_805DC608())->captureRequested) {
                    SoundManager_SetEnabled(g_soundManager, 0);
                    ((CFx*)g_fx)->SetAttachedEffectCreationEnabled(0);
                    ((ShotUpdateActor*)TaskActorTable_FindByName(Object_GetChild(g_taskManager),
                                                                 golfRuleStrings + kText_camera))
                        ->update(delta);
                    ((ShotUpdateActor*)TaskActorTable_FindByName(Object_GetChild(g_taskManager),
                                                                 golfRuleStrings + kText_golfBg))
                        ->update(delta);
                    ((ShotUpdateActor*)TaskActorTable_FindByName(Object_GetChild(g_taskManager),
                                                                 golfRuleStrings + kText_player))
                        ->update(delta);
                    ((ShotUpdateActor*)TaskActorTable_FindByName(Object_GetChild(g_taskManager),
                                                                 golfRuleStrings + kText_caddie))
                        ->update(delta);
                    ((ShotUpdateActor*)Runtime_DynamicCast(
                         TaskActorTable_FindByName(Object_GetChild(g_taskManager), golfRuleStrings + kText_caddie), 0,
                         &__RTTI__11CGolfCaddie, &__RTTI__6IActor, 0))
                        ->caddie();
                    ((ShotUpdateActor*)TaskActorTable_FindByName(Object_GetChild(g_taskManager),
                                                                 golfRuleStrings + kText_npcActor))
                        ->update(delta);
                    ((ShotUpdateActor*)TaskActorTable_FindByName(Object_GetChild(g_taskManager),
                                                                 golfRuleStrings + kText_ball))
                        ->update(delta);
                    ((ShotUpdateActor*)TaskActorTable_FindByName(Object_GetChild(g_taskManager),
                                                                 golfRuleStrings + kText_guideLine))
                        ->update(delta);
                    ((ShotUpdateActor*)TaskActorTable_FindByName(Object_GetChild(g_taskManager),
                                                                 golfRuleStrings + kText_trajectory))
                        ->update(delta);
                    ((ShotUpdateActor*)TaskActorTable_FindByName(Object_GetChild(g_taskManager),
                                                                 golfRuleStrings + kText_effect))
                        ->update(delta);
                    ((ShotUpdateActor*)TaskActorTable_FindByName(Object_GetChild(g_taskManager),
                                                                 golfRuleStrings + kText_screen))
                        ->update(delta);
                    SoundManager_SetEnabled(g_soundManager, 1);
                    ((CFx*)g_fx)->SetAttachedEffectCreationEnabled(1);
                }
            }
        }
        if (AsCGolfBall(GolfBall__Fv())->m_state)
            ((CGolfRule*)this)->UpdateBallPlayback(delta);
        else {
            ((CGolfRule*)this)->ProcessShotOutcome(delta);
            if (AsGolfRuleFrame(obj)->fastPlaybackByte)
                AsGolfRuleFrame(obj)->fastPlaybackByte = 0;
        }
        if (AsGolfRuleFrame(obj)->fastPlaybackByte) {
            unsigned char* _p = (unsigned char*)GolfBall__Fv();
            int finalSampleIndex = AsCGolfBall(_p)->m_preDataCount;
            _p = (unsigned char*)GolfBall__Fv();

            if (AsCGolfBall(_p)->m_eventPreDataIndex[kBallPlaybackCursor] + kFastPlaybackTailSamples >=
                finalSampleIndex)
                AsGolfRuleFrame(obj)->fastPlaybackByte = 0;
        }
        if (!AsGolfRuleFrame(obj)->fastPlaybackByte ||
            AsCGolfBall(GolfBall__Fv())->m_eventPreDataIndex[kBallPlaybackCursor] <= kFastPlaybackFirstSample)
            break;
        if (!Sing_805DC608()) {
            if (!s_fadeOverlayInitialized) {
                FadeOverlay_Init(s_fadeOverlay);
                __register_global_object(s_fadeOverlay, ResourceState_Dtor, s_fadeOverlayRegistration_804F72D8);
                s_fadeOverlayInitialized = 1;
            }
            Sing_805DC608() = s_fadeOverlay;
        }
        if (((::FadeState*)Sing_805DC608())->captureRequested)
            break;
    }
}

// Kind 5 creates a booster-warp effect; message IDs belong to their recipients.
// Requires live ball, completed sample history, rule and actor recipients.
// stateTimer accumulates seconds; complete 1/30-second ticks advance cursor slot 0.
// The fraction remains for interpolation; scanning stops at an event to dispatch
// its sound/effects at the matching sample. End-of-history clears timer and motion.
void CGolfRule::UpdateBallPlayback(float delta) {
    using namespace ns4;
    int leaving, rollingOnGreen;
    unsigned char* ballBytes = (unsigned char*)GolfBall__Fv();
    float alpha;
    float finalMatrix[12], initialMatrix[12];
    float eventPos[3];
    EPlaybackPosition currentSamplePosition, futurePos;
    float sum[3], current[3], next[3];
    float velocity[3], currentVel[3], nextVel[3];
    float diff[3];
    unsigned long long angle, currentAngle, nextAngle;
    // Phase 1: consume complete simulation ticks, stopping this scan at an event
    // so its effects/sounds can be dispatched at the matching playback index.
    AsGolfRuleFrame(this)->stateTimer += delta;
    if (AsGolfRuleFrame(this)->stateTimer >= 0.03333333507180214f) {
        int frames = (int)(30.0f * AsGolfRuleFrame(this)->stateTimer);
        if (AsCGolfBall(ballBytes)->m_eventPreDataIndex[kBallPlaybackCursor] + frames >
            AsCGolfBall(ballBytes)->m_preDataCount)
            frames = AsCGolfBall(ballBytes)->m_preDataCount -
                     AsCGolfBall(ballBytes)->m_eventPreDataIndex[kBallPlaybackCursor];
        int i;
        for (i = 0; i < frames; ++i) {
            unsigned char* rec = (unsigned char*)PreData__9CGolfBallFi(
                ballBytes, i + AsCGolfBall(ballBytes)->m_eventPreDataIndex[kBallPlaybackCursor] + 1);
            if (((ShotSample*)rec)->eventKind) {
                AsCGolfBall(ballBytes)->m_flags &= ~kSpecialShotMask;
                switch (((ShotSample*)rec)->eventSubkind) {
                case 0:
                    break;
                case 1:
                    IActor_DispatchMessage(this, this, golfRuleStrings + kText_effect, 7, 0, 0, 0, 0);
                    break;
                case 2:
                    if (Magnitude__7WVectorCFv(rec + 0xC) > 30.0f)
                        IActor_DispatchMessage(this, this, golfRuleStrings + kText_effect, 8,
                                               i + AsCGolfBall(ballBytes)->m_eventPreDataIndex[kBallPlaybackCursor] + 1,
                                               0, 0, 0);
                    break;
                case 3:
                    IActor_DispatchMessage(this, this, golfRuleStrings + kText_effect, 5,
                                           i + AsCGolfBall(ballBytes)->m_eventPreDataIndex[kBallPlaybackCursor] + 1, 0,
                                           0, 0);
                    break;
                case 4:
                    IActor_DispatchMessage(this, this, golfRuleStrings + kText_effect, 10, 0, 0, 0, 0);
                    break;
                }
                if (((ShotSample*)rec)->eventKind == 2)
                    IActor_DispatchMessage(this, this, golfRuleStrings + kText_gallery, 6, 0, 0, 0, 0);
                if (5 == ((ShotSample*)rec)->eventKind)
                    ((CFx*)g_fx)->ResetAndCreateBoosterWarp(ballBytes);
                int special = 0;
                if ((int)AsCSharedDoc(g_sharedDoc)->partyGameMode == 1 && ((ShotSample*)rec)->eventKind == 1 &&
                    AsCGolfBall(ballBytes)->m_state == 1)
                    IActor_DispatchMessage(this, this, golfRuleStrings + kText_dartsSystem, 4, (int)&special, 0, 0, 0);
                if (AsCGolfBall(ballBytes)->m_eventPreDataIndex[6] ==
                    i + AsCGolfBall(ballBytes)->m_eventPreDataIndex[kBallPlaybackCursor] + 1)
                    IActor_DispatchMessage(this, this, golfRuleStrings + kText_gallery, 5, 0, 0, 0, 0);
                if ((*(signed char*)AsGolfRuleFrame(((unsigned char*)this))->ballFlightSoundName))
                    Sound_StopByAlias(g_soundManager, AsGolfRuleFrame(((unsigned char*)this))->ballFlightSoundName);
                if ((*(signed char*)(rec + kSimulationEventSoundOffset)) &&
                    !SoundManager_IsAliasPrefixOrStreamStemActive(g_soundManager,
                                                                  ((char*)rec + kSimulationEventSoundOffset))) {
                    const char* name =
                        special ? golfRuleStrings + kText_party_darts_hit : ((char*)rec + kSimulationEventSoundOffset);
                    __mi__FRC7WVectorRC7WVector(eventPos, ballBytes, s_cameraTransform + 0x24);
                    ((int (*)(void*, const char*, void*, int, const char*, int, float, float, float))Sound_PlayByName)(
                        g_soundManager, name, eventPos, 5, name, 0, 0.5f, 200.0f, 1.0f);
                }
                break;
            }
            if (AsCGolfBall(ballBytes)->m_eventPreDataIndex[3] ==
                i + AsCGolfBall(ballBytes)->m_eventPreDataIndex[kBallPlaybackCursor] + 1)
                IActor_DispatchMessage(this, this, golfRuleStrings + kText_gallery, 0x11, 0, 0, 0, 0);
            unsigned char* flags = ballBytes + 0x20000;
            if ((E_I(flags, -0x6764) ^ ~E_I(flags, -0x675C)) != 1 &&
                AsCGolfBall(ballBytes)->m_eventPreDataIndex[5] ==
                    i + AsCGolfBall(ballBytes)->m_eventPreDataIndex[kBallPlaybackCursor] + 1)
                IActor_DispatchMessage(this, this, golfRuleStrings + kText_gallery, 0xD, 0, 0, 0, 0);
        }
        // Phase 2: publish the bounded cursor and retain fractional tick time.
        int end = AsCGolfBall(ballBytes)->m_eventPreDataIndex[kBallPlaybackCursor] + frames;
        if (end > AsCGolfBall(ballBytes)->m_preDataCount)
            end = AsCGolfBall(ballBytes)->m_preDataCount;
        AsCGolfBall(ballBytes)->m_eventPreDataIndex[kBallPlaybackCursor] = end;
        AsGolfRuleFrame(this)->stateTimer -= 0.03333333507180214f * frames;
        leaving = 0;
        rollingOnGreen = 0;
        if (AsCGolfBall(ballBytes)->m_state == 3) {
            EPlaybackPosition* rec = (EPlaybackPosition*)PreData__9CGolfBallFi(
                ballBytes, AsCGolfBall(ballBytes)->m_eventPreDataIndex[kBallPlaybackCursor]);
            currentSamplePosition = *rec;
            if ((unsigned char)GetGroundType__9CQuadTreeF7WVector(CQuadTree_GetInstance(), &currentSamplePosition) == 2)
                rollingOnGreen = 1;
        }
        if (rollingOnGreen) {
            EPlaybackPosition* rec = (EPlaybackPosition*)PreData__9CGolfBallFi(
                ballBytes, AsCGolfBall(ballBytes)->m_eventPreDataIndex[kBallPlaybackCursor] + frames >
                                   AsCGolfBall(ballBytes)->m_preDataCount
                               ? AsCGolfBall(ballBytes)->m_preDataCount
                               : AsCGolfBall(ballBytes)->m_eventPreDataIndex[kBallPlaybackCursor] + frames);
            futurePos = *rec;
            if ((unsigned char)GetGroundType__9CQuadTreeF7WVector(CQuadTree_GetInstance(), &futurePos) != 2)
                leaving = 1;
        }
        if (leaving)
            IActor_DispatchMessage(this, this, golfRuleStrings + kText_gallery, 0xE, 0, 0, 0, 0);
        if (AsCGolfBall(ballBytes)->m_eventPreDataIndex[7] > 0 &&
            AsCGolfBall(ballBytes)->m_eventPreDataIndex[kBallPlaybackCursor] >
                AsCGolfBall(ballBytes)->m_eventPreDataIndex[7] &&
            AsCGolfBall(ballBytes)->m_eventPreDataIndex[kBallPlaybackCursor] <
                AsCGolfBall(ballBytes)->m_eventPreDataIndex[7] + 30) {
            if (AsCGolfBall(ballBytes)->m_eventPreDataIndex[kBallPlaybackCursor] ==
                AsCGolfBall(ballBytes)->m_eventPreDataIndex[7] + 1)
                ((int (*)(void*, const char*, int, int, int, int, float, float, float))Sound_PlayByName)(
                    g_soundManager, golfRuleStrings + kText_spin_super, 0, 0, 0, 0, 0.5f, 200.0f, 1.0f);
            if (!Map_FindByKey(g_fx, s_powerSpinEffectId))
                s_powerSpinEffectId = ((CFx*)g_fx)->CreatePowerSpin(ballBytes);
        }
        if (AsCGolfBall(ballBytes)->m_eventPreDataIndex[kBallPlaybackCursor] ==
            AsCGolfBall(ballBytes)->m_preDataCount) {
            if (AsCGolfBall(ballBytes)->m_lie) {
                if ((unsigned char)SharedDoc_GetCourseId() == 12) {
                    unsigned char* effect =
                        (unsigned char*)((CFx*)g_fx)->CreateSequence(golfRuleStrings + kText_hell_ob_seq, 1);
                    if (effect)
                        __as__7WVectorFRC7WVector(effect + 0x84, GolfBall__Fv());
                } else
                    IActor_DispatchMessage(this, this, golfRuleStrings + kText_effect, 10, 0, 0, 0, 0);
            }
            AsGolfRuleFrame(this)->stateTimer = 0.0f;
            Vec3_Zero(ballBytes + 0xC);
            AsCGolfBall(ballBytes)->m_state = 0;
            Sound_StopByAlias(g_soundManager, golfRuleStrings + kText_shot_dist_count);
            IActor_DispatchMessage(this, this, golfRuleStrings + kText_screen, 0x2D, 0, 0, 0, 0);
            if ((int)AsCSharedDoc(g_sharedDoc)->partyGameMode == 1) {
                unsigned char* p = CurrentGolfPlayerEntry();
                IActor_DispatchMessage(this, this, golfRuleStrings + kText_dartsSystem, 1, (int)(p + 0x174), 0, 0, 0);
                unsigned char* a = CurrentGolfPlayerEntry();
                unsigned char* b = CurrentGolfPlayerEntry();
                E_I(b, 0x178) = E_I(b, 0x178) + E_I(a, 0x174);
                if (E_I(CurrentGolfPlayerEntry(), 0x178) < 0)
                    E_I(CurrentGolfPlayerEntry(), 0x178) = 0;
                a = CurrentGolfPlayerEntry();
                unsigned char slot = AsCGolfDoc(GolfDocBytes())->currentHoleIndex;
                b = CurrentGolfPlayerEntry();
                E_I(b + slot * 4, 0x17C) = E_I(a, 0x178);
            }
            if ((int)AsCSharedDoc(g_sharedDoc)->partyGameMode == 2) {
                float value = 0.0f;
                unsigned char* p = CurrentGolfPlayerEntry();
                IActor_DispatchMessage(this, this, golfRuleStrings + kText_driveSystem, 0, (int)(p + 0x174),
                                       (int)&value, 0, 0);
                p = CurrentGolfPlayerEntry();
                E_F(p, 0x170) = E_F(p, 0x170) + value;
                p = CurrentGolfPlayerEntry();
                unsigned int slot = (p + AsCGolfDoc(GolfDocBytes())->currentHoleIndex)[0x28];
                unsigned char* a = CurrentGolfPlayerEntry();
                unsigned char* b = CurrentGolfPlayerEntry();
                p = CurrentGolfPlayerEntry();
                E_F(p + slot * 4, 0x178) = E_F(b, 0x174) + E_F(a, 0x170);
            }
        }
        if ((AsCGolfRule(this)->shotPresentationFlags & 2)) {
            if ((float)AsCGolfBall(ballBytes)->m_eventPreDataIndex[kBallPlaybackCursor] > 9.0f) {
                ++E_I(((unsigned char*)this), 0x494);
                if (AsGolfRuleFrame(this)->presentationStep == 3) {
                    AsGolfRuleFrame(this)->presentationStep = 0;
                    E_I(((unsigned char*)this), 0x490) &= ~2;
                    IActor_DispatchMessage(this, this, golfRuleStrings + kText_caddie, 1, 0, 0, 0, 0);
                    void* first = PreData__9CGolfBallFi(GolfBall__Fv(), 0);
                    void* state = GolfBall__Fv();
                    void* last = PreData__9CGolfBallFi(GolfBall__Fv(), AsCGolfBall(state)->m_preDataCount);
                    // Same final-sample horizontal distance (including roll) as the
                    // two launch paths; alpha is reused here as world-unit distance.
                    __mi__FRC7WVectorRC7WVector(diff, last, first);
                    diff[1] = 0.0f;
                    alpha = Magnitude__7WVectorCFv(diff);
                    if (!AsCGolfBall(GolfBall__Fv())->m_recoveryRequired && alpha > 416.0f)
                        IActor_DispatchMessage(this, this, golfRuleStrings + kText_caddie, 2, 0, 0, 0, 0);
                    else
                        ((CameraCaddieOption*)g_cameraWork)->caddieCameraEnabled = 0;
                    if (Pointer_IsActionActive(g_inputPointer, golfRuleStrings + kText_spaceBarKey, 1)) {
                        CGolfDoc::BuildCameraMatrix(initialMatrix, GolfDocBytes());
                        Matrix_CopyAffine(s_cameraTransform, initialMatrix);
                    }
                } else {
                    IActor_DispatchMessage(this, this, golfRuleStrings + kText_player, 0x33, 0, 0, 0, 0);
                    ((CGolfRule*)this)->StartShotPlaybackCamera();
                }
            }
        }
    }
    // Phase 3: interpolate cursor/cursor+1 for fractional time (alpha weights
    // the CURRENT sample); at the end copy the final sample directly.
    if (AsCGolfBall(ballBytes)->m_eventPreDataIndex[kBallPlaybackCursor] < AsCGolfBall(ballBytes)->m_preDataCount &&
        (unsigned int)AsCGolfDoc(GolfDocBytes())->gameStateFlags == 0x80) {
        if (AsGolfRuleFrame(this)->stateTimer > 0.0f)
            alpha = 30.0f * (0.03333333507180214f - AsGolfRuleFrame(this)->stateTimer);
        else
            alpha = 1.0f;
        __ml__FRC7WVectorf(
            next,
            PreData__9CGolfBallFi(ballBytes, AsCGolfBall(ballBytes)->m_eventPreDataIndex[kBallPlaybackCursor] + 1),
            1.0f - alpha);
        __ml__FRC7WVectorf(
            current, PreData__9CGolfBallFi(ballBytes, AsCGolfBall(ballBytes)->m_eventPreDataIndex[kBallPlaybackCursor]),
            alpha);
        __pl__FRC7WVectorRC7WVector(sum, current, next);
        __as__7WVectorFRC7WVector(ballBytes, sum);
        __ml__FRC7WVectorf(nextVel,
                           (char*)PreData__9CGolfBallFi(
                               ballBytes, AsCGolfBall(ballBytes)->m_eventPreDataIndex[kBallPlaybackCursor] + 1) +
                               0xC,
                           1.0f - alpha);
        __ml__FRC7WVectorf(
            currentVel,
            (char*)PreData__9CGolfBallFi(ballBytes, AsCGolfBall(ballBytes)->m_eventPreDataIndex[kBallPlaybackCursor]) +
                0xC,
            alpha);
        __pl__FRC7WVectorRC7WVector(velocity, currentVel, nextVel);
        __as__7WVectorFRC7WVector(ballBytes + 0xC, velocity);
        nextAngle =
            CursorVec2_Scale((char*)PreData__9CGolfBallFi(
                                 ballBytes, AsCGolfBall(ballBytes)->m_eventPreDataIndex[kBallPlaybackCursor] + 1) +
                                 0x18,
                             1.0f - alpha);
        currentAngle = CursorVec2_Scale(
            (char*)PreData__9CGolfBallFi(ballBytes, AsCGolfBall(ballBytes)->m_eventPreDataIndex[kBallPlaybackCursor]) +
                0x18,
            alpha);
        angle = Vec2_Add(&currentAngle, &nextAngle);
        Vec2_Copy(ballBytes + 0x2C, &angle);
    } else {
        alpha = 0.0f;
        __as__7WVectorFRC7WVector(
            ballBytes,
            PreData__9CGolfBallFi(ballBytes, AsCGolfBall(ballBytes)->m_eventPreDataIndex[kBallPlaybackCursor]));
        __as__7WVectorFRC7WVector(
            ballBytes + 0xC,
            (char*)PreData__9CGolfBallFi(ballBytes, AsCGolfBall(ballBytes)->m_eventPreDataIndex[kBallPlaybackCursor]) +
                0xC);
        Vec2_Copy(ballBytes + 0x2C, (char*)PreData__9CGolfBallFi(
                                        ballBytes, AsCGolfBall(ballBytes)->m_eventPreDataIndex[kBallPlaybackCursor]) +
                                        0x18);
    }
    if ((unsigned char)SharedDoc_GetCourseId() == 10 &&
        ((ClubSelectionRuntime*)InstanceTable_Get())->selectedClub->clubKind != 3 &&
        AsCGolfBall(ballBytes)->m_eventPreDataIndex[8] > 0 &&
        !((RuleUnresolvedShotBytes*)this)->trajectoryEventDispatchedByte &&
        AsCGolfBall(ballBytes)->m_eventPreDataIndex[kBallPlaybackCursor] >=
            AsCGolfBall(ballBytes)->m_eventPreDataIndex[8]) {
        ((RuleUnresolvedShotBytes*)this)->trajectoryEventDispatchedByte = 1;
        IActor_DispatchMessage(this, this, golfRuleStrings + kText_trajectory, 5, 0, 0, 0, 0);
    }
    if (!s_cameraDirectedShotEnabled && !(AsCGolfRule(this)->shotPresentationFlags & 2))
        ((CCameraWork*)g_cameraWork)->UpdateMotionAndTarget(delta, 1.0f - alpha);
    if (lbl_805DC7F1 && (int)lbl_805DC7F4 < 2000) {
        Matrix_ExtractUniformScaleNormalizedRotation(s_cameraTransform, finalMatrix);
    }
}

// Resolve the simulated shot outcome, penalties, scoring and next presentation state.
// Requires a live rule/ball and current player; delta is seconds. Clears trajectoryEventDispatched.
// Outcome decision table (branches below; no failure return):
// - Balloon Pop pending reply > 0: return before consuming the outcome.
// - Decoded hole-in == 1 and tutorial stage >= 4: hole-in presentation, then results.
// - Otherwise: recoveryRequired adds one stroke only outside meter-reset/party mode;
//   m_lie 2/3 selects water text, other recovery selects out-of-bounds text.
// - No recovery and no bounce: same conditional penalty, then failure feedback.
// - Stroke limit is byte(par + 4); finished state 1 uses >, others >=; state 3 bypasses.
// Entry flags gate one-time sounds/messages; timers wait for UI or a fallback timeout.
// Hole-in decode is encodedValue ^ ~key. Exhibition audio consumes rand()%4.
// Scoring updates total strokes, strokes minus par, completed-hole count, then state 1.
// Simulation and presentation use separate time domains.
void CGolfRule::ProcessShotOutcome(float delta) {
    using namespace ns4;
    unsigned int slot;
    unsigned char* context;
    int strokes;
    unsigned char* self = (unsigned char*)this;
    int state;
    unsigned char* manager;
    int savedHole;
    *(unsigned char*)&((CGolfRule*)(self))->trajectoryEventDispatched = 0;
    if (*(int*)&((CSharedDoc*)(SharedDocBytes()))->partyGameMode == 3) {
        int pending = 0;
        EA_SEND(kText_balloonSystem, 2, &pending);
        if (pending > 0)
            return;
    }
    context = (unsigned char*)GolfBall__Fv();
    ((RuleUnresolvedShotBytes*)(self))->unk_092 = 1;
    unsigned char* g = GolfDocBytes();
    unsigned char* player = (unsigned char*)GolfPlayer_GetEntryByIndex((int)g, ((CGolfDoc*)(g))->currentPlayer);
    strokes = (player + AsCGolfDoc(GolfDocBytes())->currentHoleIndex)[0x28];
    if ((((BallHoleInWordsView*)(context))->encodedValue ^ ~((BallHoleInWordsView*)(context))->key) == 1 &&
        (AsCGolfDoc(GolfDocBytes())->tutorialStage >= 5 || AsCGolfDoc(GolfDocBytes())->tutorialStage == 4)) {
        if (self[0x90]) {
            self[0x90] = 0;
            int sound = NamedEffectList_FindByName(g_sceneManager, golfRuleStrings + kText_ballSequence);
            ((void (*)(int, int))WPuppet_SetOpacityRecursive)(sound, 0xFF);
            ((CCameraWork*)g_cameraWork)->StopBallSound();
            EA_SEND(kText_score, 5, 0);
            Sound_StopByAlias(g_soundManager, golfRuleStrings + kText_shot_dist_count);
            if (((CGolfRule*)(self))->puttAnimationEnabled)
                EA_SEND(kText_player, 0x2715, golfRuleStrings + kText_puttSuccessSequence);
            int flags = ((CGolfRule*)(self))->shotPresentationFlags;
            if (flags & 1) {
                int stage = ((GolfRuleFrameFields*)(self))->presentationStep + 1;
                ((GolfRuleFrameFields*)(self))->stateEntryPending = 1;
                ((GolfRuleFrameFields*)(self))->presentationStep = stage;
                switch (stage) {
                case 1:
                    EA_B(g_cameraWork, 0x1A0) = 1;
                case 2:
                    ((CGolfRule*)self)->SetupLast30FramesPlayback();
                    return;
                case 3:
                    ((CGolfRule*)(self))->shotPresentationFlags = flags & ~1;
                    {
                        struct Timer {
                            char pad[0x10];
                            float time;
                            int vector[2];
                            unsigned char flag;
                            char pad1d[0x183];
                            unsigned char active;
                        };
                        Timer* timer = (Timer*)g_cameraWork;
                        if (timer->active) {
                            timer->active = 0;
                            timer->flag = 0;
                            timer->time = kFloat_0_GolfRule;
                            Vec2_Zero(timer->vector);
                        }
                    }
                    ((GolfRuleFrameFields*)(self))->presentationStep = 0;
                    break;
                }
            } else {
                EA_SEND(kText_effect, 0xD, 0);
                g = GolfDocBytes();
                strokes -= (*(unsigned char**)&((CGolfDoc*)(g))->holeRecords +
                            (((CGolfDoc*)(g))->currentHoleIndex << 5))[0x18];
                if (strokes <= 0)
                    EA_SEND(kText_effect, 0x11, strokes < 0 ? 0xD : 6);
                manager = SharedDocBytes();
                g = EA_PTR(manager, 0xB90);
                unsigned int key = ItemInventory_GetBallIdAt(manager + 0xA04, 0xB);
                if (key == (unsigned int)EA_I(manager + ((CGolfDoc*)(g))->currentPlayer * 0x1D4, 0x88)) {
                    if (strokes < 0) {
                        const char* s =
                            String_FormatTemporary(golfRuleStrings + kText_birdieExhibitionFormat, rand() % 4 + 1);
                        ((int (*)(void*, const char*, int, int, int, int, float, float, float))Sound_PlayByName)(
                            g_soundManager, s, 0, 0, 0, 0, kFloat_0_5_GolfRule, kFloat_200_GolfRule, kFloat_1_805DEA38);
                    } else if (strokes == 0) {
                        const char* s =
                            String_FormatTemporary(golfRuleStrings + kText_parExhibitionFormat, rand() % 4 + 1);
                        ((int (*)(void*, const char*, int, int, int, int, float, float, float))Sound_PlayByName)(
                            g_soundManager, s, 0, 0, 0, 0, kFloat_0_5_GolfRule, kFloat_200_GolfRule, kFloat_1_805DEA38);
                    } else if (strokes > 0) {
                        const char* s =
                            String_FormatTemporary(golfRuleStrings + kText_overParExhibitionFormat, rand() % 4 + 1);
                        ((int (*)(void*, const char*, int, int, int, int, float, float, float))Sound_PlayByName)(
                            g_soundManager, s, 0, 0, 0, 0, kFloat_0_5_GolfRule, kFloat_200_GolfRule, kFloat_1_805DEA38);
                    }
                } else if (strokes < 0) {
                    EA_SOUND(0x61D);
                } else if (strokes == 0) {
                    EA_SOUND(0x624);
                } else if (strokes > 0) {
                    EA_SOUND(0x628);
                }
                if (AsCSharedDoc(SharedDocBytes())->golfRuleType == 1)
                    CGolfRule::StartRoundResultPresentation((char*)self, 1, 0, 0);
                if (*(int*)&((CSharedDoc*)(SharedDocBytes()))->partyGameMode == 3) {
                    unsigned char* p = GolfPlayerEntry(AsCGolfDoc(GolfDocBytes())->currentPlayer);
                    EA_I(p, 0x174) += 10;
                }
            }
        }
        ((GolfRuleFrameFields*)(self))->stateTimer += delta;
        float limit = AsCSharedDoc(SharedDocBytes())->golfRuleType == 1 ? kFloat_3_0 : kFloat_2_805DEA84;
        if ((unsigned int)(*(int*)&((CSharedDoc*)(SharedDocBytes()))->partyGameMode - 1) <= 1)
            limit += kFloat_3_0;
        if (((GolfRuleFrameFields*)(self))->stateTimer > limit) {
            unsigned char ready = 1;
            if (AsCGolfDoc(GolfDocBytes())->tutorialStage == 6)
                EA_SEND(kText_screen, 0x19, &ready);
            limit += kFloat_4_GolfPlayerHelpers;
            if (ready || ((GolfRuleFrameFields*)(self))->stateTimer > limit) {
                int flags = ((CGolfRule*)(self))->shotPresentationFlags;
                if (flags & 4) {
                    ((CGolfRule*)(self))->shotPresentationFlags = flags & ~4;
                    ((CGolfRule*)self)->StartBallFlightFromSample(0);
                    ((CCameraWork*)g_cameraWork)->UpdateMotionAndTarget(delta, kFloat_0_GolfRule);
                    return;
                }
                ((GolfRuleFrameFields*)(self))->stateTimer = kFloat_0_GolfRule;
                ((void (*)(void*))GolfRule_AccumulateCurrentShotStatistics)(self);
                if (AsCGolfDoc(GolfDocBytes())->tutorialStage == 4) {
                    EA_SEND(kText_golfRule, 0xD, 1);
                    EA_SEND(kText_tutorial, 1, 0);
                } else if (*(int*)&((CSharedDoc*)(SharedDocBytes()))->partyGameMode != 0) {
                    unsigned char* c = (unsigned char*)GolfBall__Fv() + 0x20000;
                    int result = (EA_I(c, -0x6764) ^ ~EA_I(c, -0x675C));
                    ((CGolfRule*)self)->AdvanceTurnOrRound(result == 1);
                } else {

                    {
                        slot = ((CGolfDoc*)(GolfDocBytes()))->currentPlayer;
                        if (slot < ((CGolfDoc*)(GolfDocBytes()))->playerCount && EA_I(GolfPlayerEntry(slot), 8) != 3) {
                            unsigned char* record = GolfPlayerEntry(slot);
                            ((GolfDocSnapshot8013*)(record))->itemId = 0;
                            state = ((CGolfDoc*)(GolfDocBytes()))->currentHoleIndex;
                            record = GolfPlayerEntry(slot);
                            if (*(int*)&((GolfDocSnapshot8013*)(record))->roundResultWords[3] == state) {
                                if (!((CGolfDoc*)(GolfDocBytes()))->unk_35C) {
                                    strokes = ((CGolfDoc*)(GolfDocBytes()))->currentHoleIndex;
                                    unsigned char* old = GolfPlayerEntry(slot);
                                    unsigned char* p = GolfPlayerEntry(slot);
                                    ((GolfDocSnapshot8013*)(p))->totalStrokeCount += (old + strokes)[0x28];
                                    if (((CSharedDoc*)(SharedDocBytes()))->golfRuleType != 1) {
                                        strokes = ((CGolfDoc*)(GolfDocBytes()))->currentHoleIndex;
                                        old = GolfPlayerEntry(slot);
                                        manager = *(unsigned char**)&((CGolfDoc*)(GolfDocBytes()))->holeRecords +
                                                  (((CGolfDoc*)(GolfDocBytes()))->currentHoleIndex << 5);
                                        p = GolfPlayerEntry(slot);
                                        *(int*)&((GolfDocSnapshot8013*)(p))->roundResultWords[0] +=
                                            (old + strokes)[0x28] - manager[0x18];
                                    }
                                }
                                unsigned char* p = GolfPlayerEntry(slot);
                                ++*(int*)&((GolfDocSnapshot8013*)(p))->roundResultWords[3];
                            }
                            record = GolfPlayerEntry(slot);
                            ((GolfDocSnapshot8013*)(record))->playerState = 1;
                            unsigned char* manager = SharedDocBytes();
                            if ((unsigned int)EA_I(manager, 0x808) == 6)
                                EA_I(manager, 0xAF0) = PartyGame_EvaluatePlayerThresholdTier(slot);
                        }
                    };
                    ((CGolfRule*)self)->EnterState(kStateDelayedTransition);
                }
            }
        }
    } else {
        if (self[0x90]) {
            self[0x90] = 0;
            int sound = NamedEffectList_FindByName(g_sceneManager, golfRuleStrings + kText_ballSequence);
            ((void (*)(int, int))WPuppet_SetOpacityRecursive)(sound, 0xFF);
            Sound_StopByAlias(g_soundManager, golfRuleStrings + kText_shot_dist_count);
            EA_SEND(kText_npcActor, 1, 0);
            if (((CGolfRule*)(self))->puttAnimationEnabled)
                EA_SEND(kText_player, 0x2715, golfRuleStrings + kText_puttFailureSequence);
            struct Vec {
                int x, y, z;
            };
            float difference[3];
            Vec hole = *(Vec*)(*(unsigned char**)&((CGolfDoc*)(GolfDocBytes()))->holeRecords +
                               (AsCGolfDoc(GolfDocBytes())->currentHoleIndex << 5) + 0xC);
            __mi__FRC7WVectorRC7WVector(difference, PreData__9CGolfBallFi(context, 0), &hole);
            difference[1] = kFloat_0_GolfRule;
            if (((CGolfDoc*)(GolfDocBytes()))->shotReactionActive == 0) {
                int special = (((GolfDocSnapshot8013*)(GolfPlayerEntry(AsCGolfDoc(GolfDocBytes())->currentPlayer)))
                                   ->afterPuttPoseFlags &
                               1) ^
                              1;
                if (special) {
                    unsigned char* c = (unsigned char*)GolfBall__Fv();
                    special = (EA_I(c, 0x1989C) ^ ~EA_I(c, 0x198A4)) != 1;
                }
                if (special)
                    special = !self[0x3D8];
                if (special) {
                    Vec pos = *(Vec*)PreData__9CGolfBallFi(context, 0);
                    special = (unsigned char)GetGroundType__9CQuadTreeF7WVector(CQuadTree_GetInstance(), &pos) == 2;
                }
                if (special) {
                    special = (unsigned int)*(int*)&((CGolfDoc*)(GolfDocBytes()))->remainingTimeMs < 10000U;
                    if (!special)
                        special = Magnitude__7WVectorCFv(difference) < kFloat_32_GolfRule;
                    if (!special) {
                        unsigned char* p = GolfPlayerEntry(AsCGolfDoc(GolfDocBytes())->currentPlayer);
                        special = (p + AsCGolfDoc(GolfDocBytes())->currentHoleIndex)[0x3A] >= 2;
                    }
                }
                if (special) {
                    unsigned char* p = GolfPlayerEntry(AsCGolfDoc(GolfDocBytes())->currentPlayer);
                    ((GolfDocSnapshot8013*)(p))->afterPuttPoseFlags |= 1;
                    EA_SEND(kText_player, 0x2716, golfRuleStrings + kText_afterPuttPose);
                    ((CGolfDoc*)(GolfDocBytes()))->afterPuttPoseShown = 1;
                } else
                    ((CGolfDoc*)(GolfDocBytes()))->afterPuttPoseShown = 0;
            } else if (((CGolfDoc*)(GolfDocBytes()))->afterPuttPoseShown)
                EA_SEND(kText_player, 0x2716, golfRuleStrings + kText_afterPuttPose);
            float relative[3];
            Vec hole2 = *(Vec*)(*(unsigned char**)&((CGolfDoc*)(GolfDocBytes()))->holeRecords +
                                (AsCGolfDoc(GolfDocBytes())->currentHoleIndex << 5) + 0xC);
            __mi__FRC7WVectorRC7WVector(relative, &hole2, context);
            __as__7WVectorFRC7WVector(difference, relative);
            difference[1] = kFloat_0_GolfRule;
            float distance = Magnitude__7WVectorCFv(difference);
            Vec position = *(Vec*)context;
            unsigned int terrain =
                (unsigned char)GetGroundType__9CQuadTreeF7WVector(CQuadTree_GetInstance(), &position);
            if (terrain == 3 || terrain == 12)
                EA_SEND(kText_gallery, 10, 0);
            else if (strokes <= 2 && terrain == 2) {
                if (distance > kFloat_22_4_GolfRule)
                    EA_SEND(kText_gallery, 4, 0);
                else
                    EA_SEND(kText_gallery, 15, 0);
            }
            if (EA_B(EA_PTR(InstanceTable_Get(), 4), 0xA) == 3 && distance < kFloat_3_2_GolfRule &&
                EA_I(context, 0x78) < 0)
                EA_SEND(kText_gallery, 8, 0);
            if (((CGolfBall*)(context))->m_recoveryRequired) {

                if (!((CGolfDoc*)(GolfDocBytes()))->shotMeterFadeResetRequested &&
                    *(int*)&((CSharedDoc*)(SharedDocBytes()))->partyGameMode == 0) {
                    unsigned char* p = GolfPlayerEntry(AsCGolfDoc(GolfDocBytes())->currentPlayer);
                    ++((GolfDocSnapshot8013*)(p))->unk_BC;
                    p = GolfPlayerEntry(AsCGolfDoc(GolfDocBytes())->currentPlayer);
                    unsigned char* score = p + ((CGolfDoc*)(GolfDocBytes()))->currentHoleIndex;
                    ++score[0x28];
                    if (SharedDoc_IsPlayerFlag0Set(((CGolfDoc*)(GolfDocBytes()))->currentPlayer))
                        ((CRival*)g_rival)->SetShotPositionCandidateContext();
                };
                if (AsCGolfBall(GolfBall__Fv())->m_lie == 3 || AsCGolfBall(GolfBall__Fv())->m_lie == 2) {
                    EA_SEND(kText_letter, 1, golfRuleStrings + kText_waterHazardSequence);
                    EA_SOUND(0x66B);
                } else {
                    EA_SEND(kText_letter, 1, golfRuleStrings + kText_outOfBoundsSequence);
                    if (AsCGolfBall(GolfBall__Fv())->m_lie == 1) {
                        EA_SOUND(0x66B);
                    } else {
                        EA_SOUND(0x683);
                    }
                }
                EA_SEND(kText_gallery, 9, 0);
            } else if (!((CGolfBall*)(context))->m_bounceCount) {
                ((CGolfRule*)(self))->shotPresentationFlags &= ~4;

                if (!((CGolfDoc*)(GolfDocBytes()))->shotMeterFadeResetRequested &&
                    *(int*)&((CSharedDoc*)(SharedDocBytes()))->partyGameMode == 0) {
                    unsigned char* p = GolfPlayerEntry(AsCGolfDoc(GolfDocBytes())->currentPlayer);
                    ++((GolfDocSnapshot8013*)(p))->unk_BC;
                    p = GolfPlayerEntry(AsCGolfDoc(GolfDocBytes())->currentPlayer);
                    unsigned char* score = p + ((CGolfDoc*)(GolfDocBytes()))->currentHoleIndex;
                    ++score[0x28];
                    if (SharedDoc_IsPlayerFlag0Set(((CGolfDoc*)(GolfDocBytes()))->currentPlayer))
                        ((CRival*)g_rival)->SetShotPositionCandidateContext();
                };
                EA_SOUND(0x683);
                int handled = 0;
                EA_SEND(kText_screen, 0x56, &handled);
                if (!handled && *(int*)&((CSharedDoc*)(SharedDocBytes()))->partyGameMode != 2 &&
                    *(int*)&((CSharedDoc*)(SharedDocBytes()))->partyGameMode != 1)
                    GolfTips_ShowIndexedMessage(5, golfRuleMessageTags + 0x78, 0, kFloat_0_GolfRule, kFloat_0_GolfRule);
            } else {
                unsigned char* p = GolfPlayerEntry(AsCGolfDoc(GolfDocBytes())->currentPlayer);
                ((GolfDocSnapshot8013*)(p))->unk_BC = 0;
            }
            int over;

            if (((GolfDocSnapshot8013*)(GolfPlayerEntry(AsCGolfDoc(GolfDocBytes())->currentPlayer)))->playerState == 3)
                over = 0;
            else {
                unsigned char* g = GolfDocBytes();
                strokes = (unsigned char)((*(unsigned char**)&((CGolfDoc*)(g))->holeRecords +
                                           (((CGolfDoc*)(g))->currentHoleIndex << 5))[0x18] +
                                          4);
                unsigned char score;
                if (((CSharedDoc*)(SharedDocBytes()))->golfRuleType == 1) {
                    state = ((GolfDocSnapshot8013*)(GolfPlayerEntry(AsCGolfDoc(GolfDocBytes())->currentPlayer)))
                                ->playerState;
                    unsigned char* p = GolfPlayerEntry(AsCGolfDoc(GolfDocBytes())->currentPlayer);
                    score = (p + ((CGolfDoc*)(GolfDocBytes()))->currentHoleIndex)[0x28];
                } else {
                    state = ((GolfDocSnapshot8013*)(GolfPlayerEntry(AsCGolfDoc(GolfDocBytes())->currentPlayer)))
                                ->playerState;
                    unsigned char* p = GolfPlayerEntry(AsCGolfDoc(GolfDocBytes())->currentPlayer);
                    score = (p + ((CGolfDoc*)(GolfDocBytes()))->currentHoleIndex)[0x28];
                }
                over = state == 1 ? score > (unsigned char)strokes : score >= (unsigned char)strokes;
            };
            if (over && AsCSharedDoc(SharedDocBytes())->golfRuleType == 1)
                CGolfRule::StartRoundResultPresentation((char*)self, 1, 0, 0);
            EA_SEND(kText_voice, 5, 0);
        }
        if (((CGolfBall*)(context))->m_lie) {
            float ground[3];
            struct Vec {
                int x, y, z;
            } pos = *(Vec*)context;
            float height = ((CGolfBall*)(context))->m_radius;
            GetGroundPoint__9CQuadTreeF7WVectorbPib(ground, CQuadTree_GetInstance(), &pos, 1, 0, 0);
            float floor = ground[1] + height;
            float y = EA_F(context, 4) - kFloat_3_2_GolfRule * delta;
            if (y > floor)
                EA_F(context, 4) = y;
        }
        ((GolfRuleFrameFields*)(self))->stateTimer += delta;
        float limit = ((CGolfBall*)(context))->m_recoveryRequired ? kFloat_2_805DEA84 : kFloat_3_5_GolfRule;
        if ((unsigned int)(*(int*)&((CSharedDoc*)(SharedDocBytes()))->partyGameMode - 1) <= 1)
            limit += kFloat_3_0;
        if (((GolfRuleFrameFields*)(self))->stateTimer > limit) {
            unsigned char ready = 1;
            if (AsCGolfDoc(GolfDocBytes())->tutorialStage == 6)
                EA_SEND(kText_screen, 0x19, &ready);
            limit += kFloat_4_GolfPlayerHelpers;
            if (ready || ((GolfRuleFrameFields*)(self))->stateTimer > limit) {
                int flags = ((CGolfRule*)(self))->shotPresentationFlags;
                if (flags & 4) {
                    ((CGolfRule*)(self))->shotPresentationFlags = flags & ~4;
                    ((CGolfRule*)self)->StartBallFlightFromSample(0);
                    ((CCameraWork*)g_cameraWork)->UpdateMotionAndTarget(delta, kFloat_0_GolfRule);
                    return;
                }
                if (*(unsigned char*)&((CGolfRule*)(self))->forgetDlg) {
                    if (!*(unsigned char**)&((CGolfRule*)(self))->forgetItemDialog) {
                        struct Effect {
                            char data[0x44];
                            // Construct the Pangya common dialog in mode 15.
                            Effect() {
                                ((CPangyaCommonDialog*)this)->__ct(0, 15);
                            }
                        };
                        Effect* effect = new Effect;
                        *(unsigned char**)&((CGolfRule*)(self))->forgetItemDialog = (unsigned char*)effect;
                        ((CPangyaCommonDialog*)effect)->SetText(golfRuleMessageTags + 0xB4);
                        AsCGolfRule(self)->forgetDlgTime = kFloat_0_GolfRule;
                        CMouseCursor::SetPointerInputEnabled(g_mouseCursor, 1);
                    }
                    AsCGolfRule(self)->forgetDlgTime += delta / EA_F(g_project, 0x64);
                    return;
                }
                ((void (*)(void*))GolfRule_AccumulateCurrentShotStatistics)(self);
                float zero = kFloat_0_GolfRule;
                self[0x92] = 0;
                EA_F(self, 0x9C) = zero;
                int over;

                if (((GolfDocSnapshot8013*)(GolfPlayerEntry(AsCGolfDoc(GolfDocBytes())->currentPlayer)))->playerState ==
                    3)
                    over = 0;
                else {
                    unsigned char* g = GolfDocBytes();
                    strokes = (unsigned char)((*(unsigned char**)&((CGolfDoc*)(g))->holeRecords +
                                               (((CGolfDoc*)(g))->currentHoleIndex << 5))[0x18] +
                                              4);
                    unsigned char score;
                    if (((CSharedDoc*)(SharedDocBytes()))->golfRuleType == 1) {
                        state = ((GolfDocSnapshot8013*)(GolfPlayerEntry(AsCGolfDoc(GolfDocBytes())->currentPlayer)))
                                    ->playerState;
                        unsigned char* p = GolfPlayerEntry(AsCGolfDoc(GolfDocBytes())->currentPlayer);
                        score = (p + ((CGolfDoc*)(GolfDocBytes()))->currentHoleIndex)[0x28];
                    } else {
                        state = ((GolfDocSnapshot8013*)(GolfPlayerEntry(AsCGolfDoc(GolfDocBytes())->currentPlayer)))
                                    ->playerState;
                        unsigned char* p = GolfPlayerEntry(AsCGolfDoc(GolfDocBytes())->currentPlayer);
                        score = (p + ((CGolfDoc*)(GolfDocBytes()))->currentHoleIndex)[0x28];
                    }
                    over = state == 1 ? score > (unsigned char)strokes : score >= (unsigned char)strokes;
                };
                if (over) {
                    EA_SEND(kText_guideLine, 1, 0);
                    state = AsCGolfDoc(GolfDocBytes())->currentPlayer;
                    if ((unsigned int)state < AsCGolfDoc(GolfDocBytes())->playerCount) {
                        strokes = AsCGolfDoc(GolfDocBytes())->currentHoleIndex;
                        slot = (unsigned char)((*(unsigned char**)&((CGolfDoc*)(GolfDocBytes()))->holeRecords +
                                                (strokes << 5))[0x18] +
                                               4);
                        if (AsCSharedDoc(SharedDocBytes())->golfRuleType == 1) {
                            unsigned char* p = (unsigned char*)((
                                void* (*)(void*, unsigned char))GolfPlayer_GetEntryByIndex)(GolfDocBytes(), state);
                            (p + strokes)[0x28] = slot + 1;
                        } else {
                            unsigned char* p = GolfPlayerEntry(state);
                            (p + strokes)[0x28] = slot + 1;
                        }
                        unsigned char* p = GolfPlayerEntry(state);
                        ++((GolfDocSnapshot8013*)(p))->strokeLimitHitCount;
                    }

                    {
                        slot = ((CGolfDoc*)(GolfDocBytes()))->currentPlayer;
                        if (slot < ((CGolfDoc*)(GolfDocBytes()))->playerCount && EA_I(GolfPlayerEntry(slot), 8) != 3) {
                            unsigned char* record = GolfPlayerEntry(slot);
                            ((GolfDocSnapshot8013*)(record))->itemId = 0;
                            state = ((CGolfDoc*)(GolfDocBytes()))->currentHoleIndex;
                            record = GolfPlayerEntry(slot);
                            if (*(int*)&((GolfDocSnapshot8013*)(record))->roundResultWords[3] == state) {
                                if (!((CGolfDoc*)(GolfDocBytes()))->unk_35C) {
                                    savedHole = ((CGolfDoc*)(GolfDocBytes()))->currentHoleIndex;
                                    unsigned char* old = GolfPlayerEntry(slot);
                                    unsigned char* p = GolfPlayerEntry(slot);
                                    ((GolfDocSnapshot8013*)(p))->totalStrokeCount += (old + savedHole)[0x28];
                                    if (((CSharedDoc*)(SharedDocBytes()))->golfRuleType != 1) {
                                        savedHole = ((CGolfDoc*)(GolfDocBytes()))->currentHoleIndex;
                                        old = GolfPlayerEntry(slot);
                                        manager = *(unsigned char**)&((CGolfDoc*)(GolfDocBytes()))->holeRecords +
                                                  (((CGolfDoc*)(GolfDocBytes()))->currentHoleIndex << 5);
                                        p = GolfPlayerEntry(slot);
                                        *(int*)&((GolfDocSnapshot8013*)(p))->roundResultWords[0] +=
                                            (old + savedHole)[0x28] - manager[0x18];
                                    }
                                }
                                unsigned char* p = GolfPlayerEntry(slot);
                                ++*(int*)&((GolfDocSnapshot8013*)(p))->roundResultWords[3];
                            }
                            record = GolfPlayerEntry(slot);
                            ((GolfDocSnapshot8013*)(record))->playerState = 1;
                            unsigned char* manager = SharedDocBytes();
                            if ((unsigned int)EA_I(manager, 0x808) == 6)
                                EA_I(manager, 0xAF0) = PartyGame_EvaluatePlayerThresholdTier(slot);
                        }
                    };
                    if (AsCSharedDoc(SharedDocBytes())->golfRuleType == 1)
                        ((CGolfRule*)self)->EnterState(kStateDelayedTransition);
                    else
                        ((CGolfRule*)self)->EnterState(kStateFightResult);
                } else if (*(int*)&((CSharedDoc*)(SharedDocBytes()))->partyGameMode != 0)
                    ((CGolfRule*)self)->AdvanceTurnOrRound(0);
                else if (AsCSharedDoc(SharedDocBytes())->golfRuleType == 1)
                    ((CGolfRule*)self)->EnterState(kStateDelayedTransition);
                else
                    ((CGolfRule*)self)->EnterState(kStateWaitPlayerAndCaddie);
            }
        }
    }
    if (!s_cameraDirectedShotEnabled)
        ((CCameraWork*)g_cameraWork)->UpdateMotionAndTarget(delta, kFloat_0_GolfRule);
}

#pragma opt_common_subs off
#pragma opt_lifetimes off
#pragma opt_propagation off
// ===== S11 - Aim, swing and motion camera input =====
// Document gauge/input state and motion-controller ABI.

// query 0x40 overwrites the same gate before swing input is examined.
// Requires rule, current player, ball and screen services. CPU input can commit
// a prepared shot; manual input waits for HUD phase and panel readiness replies.
// May switch camera mode, begin button/motion swing or show UI. Borrowed reply
// floats are radians where computed as heading; receiver-specific IDs remain distinct.
// The caller also passes dt; this body does not read it.
void CGolfRule::ProcessAimInput(float dt) {
    using namespace ns4;
    fn_80346F58();
    if (SharedDoc_IsPlayerFlag0Set(((*(unsigned char**)&AsCSharedDoc(g_sharedDoc)->golfDoc))[0x3C1])) {
        void* rival = g_rival;
        if (!((CRival*)rival)->IsUnplayableDropActive() && ((CRival*)rival)->ApplyPreparedShot(this)) {
            int targetPositionBits[3];
            targetPositionBits[0] = (*(int*)&AsCRival(rival)->holePosition[0]);
            targetPositionBits[1] = (*(int*)&AsCRival(rival)->holePosition[1]);
            targetPositionBits[2] = (*(int*)&AsCRival(rival)->holePosition[2]);
            __as__7WVectorFRC7WVector(&AsGolfRuleFrame(this)->aimTarget, targetPositionBits);
            float worldForward[3];
            worldForward[0] = kFloat_0_GolfRule;
            worldForward[1] = kFloat_0_GolfRule;
            worldForward[2] = kFloat_1_805DEA38;
            unsigned char* control = (*(unsigned char**)&AsCSharedDoc(g_sharedDoc)->golfDoc);
            WVector ballToTarget;
            __mi__FRC7WVectorRC7WVector(&ballToTarget, &AsGolfRuleFrame(this)->aimTarget, GolfBall__Fv());
            float headingRadians = Vector_SignedAngleXZ(&ballToTarget, worldForward);
            AsCGolfDoc(control)->headingRadians = headingRadians;
            // Broadcast the computed radians as a borrowed float. The screen's
            // handler copies it as ring height; retain this recipient-specific use.
            float broadcastFloat = headingRadians;
            IActor_DispatchMessage(this, this, golfRuleStrings + kText_camera, kAimFloatBroadcastMessage,
                                   (int)&broadcastFloat, 0, 0, 0);
            IActor_DispatchMessage(this, this, golfRuleStrings + kText_spot, kAimFloatBroadcastMessage,
                                   (int)&broadcastFloat, 0, 0, 0);
            IActor_DispatchMessage(this, this, golfRuleStrings + kText_screen, kAimFloatBroadcastMessage,
                                   (int)&broadcastFloat, 0, 0, 0);
            IActor_DispatchMessage(this, this, golfRuleStrings + kText_grid, kAimFloatBroadcastMessage,
                                   (int)&broadcastFloat, 0, 0, 0);
            IActor_DispatchMessage(this, this, golfRuleStrings + kText_caddie, kAimFloatBroadcastMessage,
                                   (int)&broadcastFloat, 0, 0, 0);

            IActor_DispatchMessage(this, this, golfRuleStrings + kText_player, 6, 0, 0, 0, 0);
            float powerColorBlend = 0.0f;
            IActor_DispatchMessage(this, this, golfRuleStrings + kText_spot, 5, 0, 0, 0, 0);
            IActor_DispatchMessage(this, this, golfRuleStrings + kText_screen, kScreenSetPowerColorMessage,
                                   (int)&powerColorBlend, 0, 0, 0);
            ((CGolfRule*)this)->CommitShotGauge();
            return;
        }
        return;
    }
    int hudPhase;
    int aimInputReady = 0;
    QueryScreenHudPhase(this, &hudPhase);
    if (hudPhase == 1)
        aimInputReady = 1;
    unsigned char* control2 = (*(unsigned char**)&AsCSharedDoc(g_sharedDoc)->golfDoc);
    if (AsCGolfDoc(control2)->cameraMode) {
        if (PollAdvanceOrDismiss()) {
            AsCGolfDoc(control2)->cameraMode = 0;
            return;
        }
    }
    if (PollCameraToggle()) {
        if (2 == hudPhase) {
            unsigned char* c2 = (*(unsigned char**)&AsCSharedDoc(g_sharedDoc)->golfDoc);
            if (AsCGolfDoc(c2)->cameraMode == 0 || AsCGolfDoc(c2)->cameraMode == 1)
                AsCGolfDoc(c2)->cameraMode = 2;
            ((CGolfRule*)this)->EnterState(kStateCameraTransition);
            return;
        }
    }
    unsigned char* c3 = (*(unsigned char**)&AsCSharedDoc(g_sharedDoc)->golfDoc);
    if (AsCGolfDoc(c3)->tutorialStage < 6 && AsCGolfDoc(c3)->tutorialStage != 5) {
        int enabled = 0;
        IActor_DispatchMessage(this, this, golfRuleStrings + kText_tutorial, kTutorialQueryGateMessage, (int)&enabled,
                               0, 0, 0);
        if (enabled) {
            if (aimInputReady)
                IActor_DispatchMessage(this, this, golfRuleStrings + kText_tutorial, 5, 0, 0, 0, 0);
            else
                IActor_DispatchMessage(this, this, golfRuleStrings + kText_tutorial, 5, 1, 0, 0, 0);
        }
    }
    if (aimInputReady)
        QueryScreenPanelsReady(this, &aimInputReady);
    if (!aimInputReady)
        return;
    unsigned char* control3 = (*(unsigned char**)&AsCSharedDoc(g_sharedDoc)->golfDoc);
    AsCGolfDoc(control3)->shotReactionActive = 0;
    AsCGolfDoc(control3)->puttingReactionPending = 0;
    unsigned char* mgr = (unsigned char*)g_sharedDoc;

    AsCGolfDoc(control3)->afterPuttPoseShown = 0;
    if (!*(int*)(mgr + (AsCGolfDoc(control3)->currentPlayer) * 0x1D4 + 0x1DC)) {
        ((CGolfRule*)this)->EnterState(kStateMotionSwing);
        return;
    }
    if (!GetInputManagerSlot()) {
        if (!s_inputManagerInitialized) {
            InputManager_Init(s_gameInputManager);
            __register_global_object(s_gameInputManager, (void*)InputManager_Dtor, s_golfRuleInputDestructorRecord);
            s_inputManagerInitialized = 1;
        }
        GetInputManagerSlot() = s_gameInputManager;
    }
    void* input = GetInputManagerSlot();
    unsigned char* ctrl2 = (*(unsigned char**)&AsCSharedDoc(g_sharedDoc)->golfDoc);
    if (Input_IsActionPressed(input, kActionSwing, kSelectedInputChannel) && !ctrl2[0x35D] &&
        AsCGolfDoc(ctrl2)->tutorialStage >= 2 && *(int*)((char*)InstanceTable_Get() + 0x60) == 2) {
        if (!AsCGolfRule(this)->shotBlock) {
            if (!((unsigned char*)this)[0x75]) {
                Sound_PlayByName(g_soundManager, golfRuleStrings + kText_play_unplayable, 0, 0, 0, 0, 0.5f, 200.0f,
                                 1.0f);
                return;
            } else {
                int value = 0;
                IActor_DispatchMessage(this, this, golfRuleStrings + kText_item, 5, (int)&value, 0, 0, 0);
                if (value == 3) {
                    IActor_DispatchMessage(this, this, golfRuleStrings + kText_player, 0x15, (int)&value, 0, 0, 0);
                    if (value == 3)
                        return;
                    ((CGolfRule*)this)->EnterState(kStateBeginButtonSwing);
                } else {
                    return;
                }
            }
        } else {
            return;
        }
    }
    if (Input_IsActionPressed(input, kActionCancel, kSelectedInputChannel) &&
        !Input_IsActionPressed(input, kActionSwing, kSelectedInputChannel))
        IActor_DispatchMessage(this, this, golfRuleStrings + kText_screen, 0x38, 0, 0, 0, 0);
}

#pragma opt_common_subs reset
#pragma opt_lifetimes reset
#pragma opt_propagation reset
// Poll button swing input, update the selected gauge mode and round the displayed cursor.
// Requires live rule, documents, controller and selected club; deltaSeconds is seconds.
// Writes gauge X values (screen units: start 149, span 360),
// shot phase and millisecond tick fields; samples actions and may commit a shot.
// No failure return; pause/sync admission returns before sampling input.
void CGolfRule::ProcessButtonSwingInput(float deltaSeconds) {
    using namespace ns4;
    float initialPowerWindow;
    float frameStep;
    struct ReplayGaugeScratch {
        int header[2];
        float resetPosition;
        unsigned char resetPhase;
        char reserved[7];
        unsigned char advancePhase;
        char pad[3];
        float advancePosition;
    };
    volatile ReplayGaugeScratch replay;
    unsigned char* ruleBytes = (unsigned char*)this;
    // Phase 1: respect pause/sync gates, initialize input and sample swing.
    if (ruleBytes[0x3B1] || ruleBytes[0x3B2])
        return;

    if (0 == GetInputManagerSlot()) {
        if (s_inputManagerInitialized == 0) {
            InputManager_Init(s_gameInputManager);
            __register_global_object(s_gameInputManager, (void*)InputManager_Dtor, s_golfRuleInputDestructorRecord);
            s_inputManagerInitialized = 1;
        }
        GetInputManagerSlot() = s_gameInputManager;
    }
    void* inputManager = GetInputManagerSlot();
    unsigned int swingPressed = 0;
    swingPressed = Input_IsActionPressed(inputManager, kActionSwing, kSelectedInputChannel) != 0;

    void* shotParameterTable = InstanceTable_Get();
    void* shotParameters = *(void**)((char*)shotParameterTable + 4);

    if (((SelectedClubRuntime*)shotParameters)->clubKind == kClubPutter)
        initialPowerWindow = kFloat_10_GolfRule;
    else {
        shotParameterTable = InstanceTable_Get();
        initialPowerWindow = ShotParameters_GetGoodImpactHalfWidth(shotParameterTable);
    }
    unsigned char* sharedDocBytes = (unsigned char*)g_sharedDoc;
    unsigned char* golfDocBytes = (*(unsigned char**)&AsCSharedDoc(sharedDocBytes)->golfDoc);
    if ((unsigned char)AsCGolfDoc(golfDocBytes)->shotPhase == kShotSelectPower) {
        if (swingPressed) {
            if (AsCGolfDoc(golfDocBytes)->shotGaugeCursorX < kFloat_149_805DEA58 + initialPowerWindow) {
                ((CFx*)g_fx)->SetUpdateEnabled(1);
                if (GolfTask_NoOpStatus()) {
                    replay.resetPhase = 0;
                    unsigned char* replayGolfDocBytes = (*(unsigned char**)&AsCSharedDoc(g_sharedDoc)->golfDoc);
                    replay.resetPosition = AsCGolfDoc(replayGolfDocBytes)->pangyaTargetX;
                }
                ruleBytes[0x3F0] = 0;
                sharedDocBytes = (unsigned char*)g_sharedDoc;
                golfDocBytes = (*(unsigned char**)&AsCSharedDoc(sharedDocBytes)->golfDoc);
                AsCGolfDoc(golfDocBytes)->shotPhase = kShotIdle;
                shotParameterTable = InstanceTable_Get();
                shotParameters = *(void**)((char*)shotParameterTable + 4);
                float fullPower;
                float initial;
                initial = (((SelectedClubRuntime*)shotParameters)->clubKind == kClubPutter) ? kFloat_149_805DEA58
                                                                                            : kFloat_114_GolfRule;
                fullPower = kFloat_149_805DEA58;
                golfDocBytes = (*(unsigned char**)&AsCSharedDoc(g_sharedDoc)->golfDoc);
                AsCGolfDoc(golfDocBytes)->shotGaugeCursorX = initial;
                AsCGolfDoc(AsCSharedDoc(g_sharedDoc)->golfDoc)->roundedShotGaugeCursorX =
                    AsCGolfDoc(AsCSharedDoc(g_sharedDoc)->golfDoc)->shotGaugeCursorX;
                AsCGolfDoc(AsCSharedDoc(g_sharedDoc)->golfDoc)->shotPowerGaugeX = fullPower;
                AsCGolfDoc(AsCSharedDoc(g_sharedDoc)->golfDoc)->shotGaugeDirection = kGaugeStopped;
                ((unsigned char*)AsCSharedDoc(g_sharedDoc)->golfDoc)[0x2D8] = 0;
                IActor_DispatchMessage(this, this, golfRuleStrings + kText_camera, 4, 0, 0, 0, 0);
                AsCGolfDoc((*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc))->gameStateFlags = 8;
                return;
            }
        }
    }

    if (AsCGolfRule(ruleBytes)->autoShot && !Input_IsActionPressed(inputManager, kActionSwing, kSelectedInputChannel))
        swingPressed = 0;
    if (swingPressed) {
        if (GolfTask_NoOpStatus()) {
            golfDocBytes = (*(unsigned char**)&AsCSharedDoc(g_sharedDoc)->golfDoc);
            replay.advancePhase = (unsigned char)(AsCGolfDoc(golfDocBytes)->shotPhase + 1);
            replay.advancePosition = AsCGolfDoc(golfDocBytes)->roundedShotGaugeCursorX;
            shotParameterTable = InstanceTable_Get();
            shotParameters = *(void**)((char*)shotParameterTable + 4);
            if (((SelectedClubRuntime*)shotParameters)->clubKind == kClubPutter) {
                (void)&replay;
            }
        }
    } else if (AsCGolfRule(ruleBytes)->autoShot) {
        golfDocBytes = (*(unsigned char**)&AsCSharedDoc(g_sharedDoc)->golfDoc);
        unsigned int shotPhase = AsCGolfDoc(golfDocBytes)->shotPhase;
        if (((unsigned char)shotPhase == 1 &&
             (AsCGolfDoc(golfDocBytes)->shotGaugeCursorX - *(float*)(ruleBytes + 0x3D0)) > kFloat_0_GolfRule) ||
            ((unsigned char)shotPhase == 2 &&
             (AsCGolfDoc(golfDocBytes)->shotGaugeCursorX - *(float*)(ruleBytes + 0x3D4)) < kFloat_0_GolfRule))
            IActor_DispatchMessage(this, this, golfRuleStrings + kText_tutorial, 0, 0, 0, 0, 0);
    }
    sharedDocBytes = (unsigned char*)g_sharedDoc;
    golfDocBytes = (*(unsigned char**)&AsCSharedDoc(sharedDocBytes)->golfDoc);
    unsigned char* playerRecordBase = sharedDocBytes + AsCGolfDoc(golfDocBytes)->currentPlayer * 0x1D4;
    // Phase 2: record special-shot directions, then advance the power/impact gauge.
    if (*(int*)(playerRecordBase + 0x1DC) && AsCGolfDoc(golfDocBytes)->tutorialStage == 6)
        ((CGolfRule*)this)->RecordDirectionalShotInput();

    if (!*(int*)(ruleBytes + 0x3A8)) {
        unsigned int initialTick = OS_GetTimeMilliseconds();
        *(int*)(ruleBytes + 0x3A8) = initialTick;
        if (!*(int*)(ruleBytes + 0x3AC))
            *(int*)(ruleBytes + 0x3AC) = initialTick;
    }
    unsigned int currentTick = OS_GetTimeMilliseconds();
    unsigned int elapsedMs = currentTick - *(int*)(ruleBytes + 0x3A8);
    *(int*)(ruleBytes + 0x3A8) = currentTick;
    float secondsScale = kGolfRuleMillisecondsToSeconds;
    float elapsedFloat = (float)elapsedMs;
    float elapsedSeconds = secondsScale * elapsedFloat;
    shotParameterTable = InstanceTable_Get();
    shotParameters = *(void**)((char*)shotParameterTable + 4);

    if (((SelectedClubRuntime*)shotParameters)->clubKind == kClubPutter) {
        frameStep = elapsedSeconds;
        ((CGolfRule*)this)->UpdateSingleStageGauge(swingPressed, elapsedSeconds);
    } else {
        golfDocBytes = (*(unsigned char**)&AsCSharedDoc(g_sharedDoc)->golfDoc);
        if (AsCGolfDoc(golfDocBytes)->shotPhase == kShotSelectPower) {
            if (!ButtonSwingNearZero(deltaSeconds)) {
                float old = *(float*)(ruleBytes + 0x40C);
                *(float*)(ruleBytes + 0x40C) = ButtonSwingMinimum(deltaSeconds, old);
            }
            frameStep = elapsedSeconds;
        } else {
            float old = *(float*)(ruleBytes + 0x40C);
            frameStep = old < elapsedSeconds ? elapsedSeconds : old;
        }
        ((CGolfRule*)this)->UpdatePowerAndImpactGauge(swingPressed, frameStep);
    }
    if (AsCGolfDoc(*(unsigned char**)(((char*)&AsCSharedDoc(g_sharedDoc)->golfDoc)))->shotGaugeDirection ==
        kGaugeAdvancing) {
        float gaugePosition =
            AsCGolfDoc(*(unsigned char**)(((char*)&AsCSharedDoc(g_sharedDoc)->golfDoc)))->shotGaugeCursorX;
        AsCGolfDoc(*(unsigned char**)(((char*)&AsCSharedDoc(g_sharedDoc)->golfDoc)))->roundedShotGaugeCursorX =
            (float)floor(gaugePosition);
    } else {
        float gaugePosition =
            AsCGolfDoc(*(unsigned char**)(((char*)&AsCSharedDoc(g_sharedDoc)->golfDoc)))->shotGaugeCursorX;
        AsCGolfDoc(*(unsigned char**)(((char*)&AsCSharedDoc(g_sharedDoc)->golfDoc)))->roundedShotGaugeCursorX =
            (float)ceil(gaugePosition);
    }
    (void)frameStep;
}

// Consume Wii Remote swing-controller results, commit hits and manage pointer input and rumble.
// Requires a constructed rule/controller and live documents; absent player actor returns.
// Writes gauge power/phase and controller state, commits hits or preview simulation,
// and sends animation/pointer/rumble feedback. Gauge X uses start 149 and span 360
// deltaSeconds is seconds. No failure return.
void CGolfRule::ProcessMotionSwingInput(float deltaSeconds) {
    using namespace ns4;
    unsigned char* ruleBytes = (unsigned char*)this;
    void* golfDocAfterUpdate;
    void* actorRoot = g_taskManager ? Object_GetChild(g_taskManager) : 0;
    void* playerActor = TaskActorTable_FindByName(actorRoot, golfRuleStrings + kText_player);
    if (!playerActor)
        return;

    int playerAnimationState = 0;
    // Phase 1: query player animation before interpreting motion-controller results.
    IActor_DispatchMessage(this, this, golfRuleStrings + kText_player, 0x15, (int)&playerAnimationState, 0, 0, 0);

    void* sharedDoc = g_sharedDoc;
    void* golfDoc = (*(void**)&AsCSharedDoc(sharedDoc)->golfDoc);
    void* swingController = AsCGolfRule(ruleBytes)->padSwingController;
    int swingResult =
        ((PadSwingInterface*)swingController)
            ->UpdateSwing(deltaSeconds, &AsCGolfDoc(golfDoc)->shotPowerGaugeX, playerActor, playerAnimationState);

    {
        switch (swingResult) {
        case kMotionCancelled: {
            unsigned char* sharedDocBytes = (unsigned char*)g_sharedDoc;
            AsCGolfDoc((*(unsigned char**)&AsCSharedDoc(sharedDocBytes)->golfDoc))->shotPhase = kShotIdle;
            ((CGolfRule*)this)->EnterState(kStateAim);
            IActor_DispatchMessage(this, this, golfRuleStrings + kText_screen, 0x38, 0, 0, 0, 0);
            ((PlayerSwingInterface*)playerActor)->SetSwingAnimationState(0);
            CMouseCursor::SetPointerInputEnabled(g_mouseCursor, 1);
            break;
        }
        case kMotionIdle: {
            unsigned char* sharedDocBytes = (unsigned char*)g_sharedDoc;
            AsCGolfDoc((*(unsigned char**)&AsCSharedDoc(sharedDocBytes)->golfDoc))->shotPhase = kShotIdle;
            break;
        }
        case kMotionStarted: {
            unsigned char* sharedDocBytes = (unsigned char*)g_sharedDoc;
            unsigned char* golfDocBytes = (*(unsigned char**)&AsCSharedDoc(sharedDocBytes)->golfDoc);
            if (AsCGolfDoc(golfDocBytes)->shotPhase == kShotIdle) {
                ((int (*)(void*, const char*, int, int, int, int, float, float, float))Sound_PlayByName)(
                    g_soundManager, golfRuleStrings + kText_swing_power_decide, 0, 0, 0, 0, kFloat_0_5_GolfRule,
                    kFloat_200_GolfRule, kFloat_1_805DEA38);
            }
            AsCGolfDoc((*(unsigned char**)&AsCSharedDoc(g_sharedDoc)->golfDoc))->shotPhase = kShotSelectPower;
            AsCGolfDoc(*(unsigned char**)(((char*)&AsCSharedDoc(g_sharedDoc)->golfDoc)))->alternateMotionPowerDecided =
                0;
            break;
        }
        case kMotionPowerDecided: {
            unsigned char* sharedDocBytes = (unsigned char*)g_sharedDoc;
            unsigned char* golfDocBytes = (*(unsigned char**)&AsCSharedDoc(sharedDocBytes)->golfDoc);
            if (AsCGolfDoc(golfDocBytes)->shotPhase == kShotSelectPower) {
                ((int (*)(void*, const char*, int, int, int, int, float, float, float))Sound_PlayByName)(
                    g_soundManager, golfRuleStrings + kText_swing_power_decide, 0, 0, 0, 0, kFloat_0_5_GolfRule,
                    kFloat_200_GolfRule, kFloat_1_805DEA38);
            }
            AsCGolfDoc((*(unsigned char**)&AsCSharedDoc(g_sharedDoc)->golfDoc))->shotPhase = kShotSelectImpact;
            AsCGolfDoc(*(unsigned char**)(((char*)&AsCSharedDoc(g_sharedDoc)->golfDoc)))->alternateMotionPowerDecided =
                0;
            break;
        }
        case kMotionAlternatePowerDecided: {
            unsigned char* sharedDocBytes = (unsigned char*)g_sharedDoc;
            unsigned char* golfDocBytes = (*(unsigned char**)&AsCSharedDoc(sharedDocBytes)->golfDoc);
            if (AsCGolfDoc(golfDocBytes)->shotPhase == kShotSelectPower) {
                ((int (*)(void*, const char*, int, int, int, int, float, float, float))Sound_PlayByName)(
                    g_soundManager, golfRuleStrings + kText_swing_power_decide, 0, 0, 0, 0, kFloat_0_5_GolfRule,
                    kFloat_200_GolfRule, kFloat_1_805DEA38);
            }
            AsCGolfDoc((*(unsigned char**)&AsCSharedDoc(g_sharedDoc)->golfDoc))->shotPhase = kShotSelectImpact;
            AsCGolfDoc(*(unsigned char**)(((char*)&AsCSharedDoc(g_sharedDoc)->golfDoc)))->alternateMotionPowerDecided =
                1;
            break;
        }
        case kMotionHit: {
            unsigned char* sharedDocBytes = (unsigned char*)g_sharedDoc;
            AsCGolfDoc((*(unsigned char**)&AsCSharedDoc(sharedDocBytes)->golfDoc))->shotPhase = kShotImpactDecided;
            ((CGolfRule*)this)->BuildShotSimulation(0);
            ((PlayerSwingInterface*)playerActor)->SetSwingAnimationState(0);
            void* anim = InstanceTable_Get();
            if (((ClubSelectionRuntime*)anim)->powerShotMode == kNormalShotMode && playerAnimationState != 3) {
                {
                    void* controllerForMotion = AsCGolfRule(ruleBytes)->padSwingController;
                    void* r = ((PadSwingInterface*)controllerForMotion)->GetPlayerSwingHandle();
                    ((PlayerSwingInterface*)playerActor)->SetSwingMotionHandle(r);
                }
                {
                    float* p = (float*)GolfBall__Fv();
                    void* pv = ((CGolfBall*)p)->GetOrAllocatePreDataAt(0);
                    if (((ShotSample*)pv)->motionState == 1) {
                        if (!GetInputManagerSlot()) {
                            if (!s_inputManagerInitialized) {
                                InputManager_Init(s_gameInputManager);
                                __register_global_object(s_gameInputManager, (void*)InputManager_Dtor,
                                                         s_golfRuleInputDestructorRecord);
                                s_inputManagerInitialized = 1;
                            }
                            GetInputManagerSlot() = s_gameInputManager;
                        }
                        InputManager_StartRumble(GetInputManagerSlot(), kSwingHitRumbleFrames);
                    }
                }
                ((CGolfRule*)this)->EnterState(kStateBallFlight);
            } else {
                ((CGolfRule*)this)->EnterState(kStateMotionShotAnimation);
            }
            break;
        }
        case kMotionPreviewHit: {
            unsigned char* sharedDocBytes = (unsigned char*)g_sharedDoc;
            AsCGolfDoc((*(unsigned char**)&AsCSharedDoc(sharedDocBytes)->golfDoc))->shotPhase = kShotImpactDecided;
            ((CGolfRule*)this)->BuildShotSimulation(1);
            break;
        }
        case kMotionFinished: {
            unsigned char* sharedDocBytes = (unsigned char*)g_sharedDoc;
            AsCGolfDoc((*(unsigned char**)&AsCSharedDoc(sharedDocBytes)->golfDoc))->shotPhase = kShotImpactDecided;
            break;
        }
        case kMotionAutoShot: {
            unsigned char* sharedDocBytes = (unsigned char*)g_sharedDoc;
            AsCGolfDoc((*(unsigned char**)&AsCSharedDoc(sharedDocBytes)->golfDoc))->shotPhase = kShotImpactDecided;
            if (AsCGolfRule(ruleBytes)->autoShot) {
                IActor_DispatchMessage(this, this, golfRuleStrings + kText_tutorial, 0, 0, 0, 0, 0);
            }
            break;
        }
        default:
            break;
        }
    }

    void* swingControllerAfterUpdate = AsCGolfRule(ruleBytes)->padSwingController;
    void* sharedDocBytes = g_sharedDoc;
    golfDocAfterUpdate = (*(void**)&AsCSharedDoc(sharedDocBytes)->golfDoc);
    int controllerShotState = ((PadSwingInterface*)swingControllerAfterUpdate)->QueryShotState();
    // Phase 2: publish the controller result, then apply the automatic-shot override.
    AsCGolfDoc(golfDocAfterUpdate)->motionSwingState = controllerShotState;

    if (AsCGolfRule(ruleBytes)->autoShot) {
        void* autoShotController = AsCGolfRule(ruleBytes)->padSwingController;
        int r = ((PadSwingInterface*)autoShotController)->QueryAutoShotReady();
        if (r != 0) {
            IActor_DispatchMessage(this, this, golfRuleStrings + kText_tutorial, 0, 0, 0, 0, 0);
        }
    }
}

// Update the swing-controller camera interface and leave it when input requests exit.
// Requires rule/controller; delta is seconds. Returns if the player actor is absent.
// Initializes camera once, then calls the motion-controller interface with the actor.
// A completed controller reply updates actor bindings and enters BallFlight;
// otherwise waits in the current state. No success/failure return.
void CGolfRule::ProcessMotionCameraInput(float delta) {
    using namespace ns4;
    unsigned char* object = (unsigned char*)this;
    void* message = g_taskManager ? Object_GetChild(g_taskManager) : 0;
    void* item = TaskActorTable_FindByName(message, golfRuleStrings + kText_player);
    if (!item)
        return;
    if (!object[0x74]) {
        if (!(*(unsigned int*)(object + 0x490) & 2)) {
            if (!GetInputManagerSlot()) {
                if (!s_inputManagerInitialized) {
                    InputManager_Init(s_gameInputManager);
                    __register_global_object(s_gameInputManager, (void*)InputManager_Dtor,
                                             (void*)s_golfRuleInputDestructorRecord);
                    s_inputManagerInitialized = 1;
                }
                GetInputManagerSlot() = s_gameInputManager;
            }
            if (!Input_IsActionButtonDown(GetInputManagerSlot(), 11, -1)) {
                unsigned char* state = (unsigned char*)g_cameraWork;
                if (state[0x1A0]) {
                    state[0x1A0] = 0;
                    state[0x1C] = 0;
                    *(float*)(state + 0x10) = kFloat_0_GolfRule;
                    Vec2_Zero(state + 0x14);
                }
                ((unsigned char*)g_cameraWork)[0x241] = 0;
            }
            if (!s_cameraDirectedShotEnabled)
                ((CCameraWork*)g_cameraWork)->UpdateMotionAndTarget(delta, kFloat_0_GolfRule);
        }
        object[0x74] = 1;
    }
    if (((int (*)(void*, void*, float)) *
         (void**)((char*)*(void**)(*(char**)(object + 0x4C4) + 0x58) + 0x18))(*(void**)(object + 0x4C4), item, delta)) {
        void** item_vtable = *(void***)item;
        ((void (*)(void*, int))item_vtable[0xAC / 4])(item, 0);
        void* next = ((void* (*)(void*)) *
                      (void**)((char*)*(void**)(*(char**)(object + 0x4C4) + 0x58) + 0x28))(*(void**)(object + 0x4C4));
        item_vtable = *(void***)item;
        ((void (*)(void*, void*))item_vtable[0xCC / 4])(item, next);
        ((CGolfRule*)this)->EnterState(kStateBallFlight);
    }
}

#pragma opt_common_subs off
#pragma opt_lifetimes off
namespace ns4 {
extern "C" {
// Final-sample horizontal displacement includes roll; see the caddie-camera gate.

#pragma push
#pragma always_inline on
// Record manual swing count and controller gauge totals before preview or simulation.
static inline void RecordManualSwingStatistics(unsigned char* self) {
    if (AsCGolfRule(self)->autoShot == 0) {
        ((ShotAccountStatisticsView*)(SharedDocBytes() + AsCGolfDoc(GolfDocBytes())->currentPlayer * 0x1D4))
            ->manualSwings =
            ((ShotAccountStatisticsView*)(SharedDocBytes() + AsCGolfDoc(GolfDocBytes())->currentPlayer * 0x1D4))
                ->manualSwings +
            1;
        int r = SharedDoc_IsMode6PartyMode0();
        int gameMode = (int)AsCSharedDoc(SharedDocBytes())->gameMode;
        switch (gameMode) {
        case 0:
        case 3:
        case 4:
        case 5:
            r = 1;
            break;
        default:
            break;
        }
        if (r) {
            void* o = AsCGolfRule(self)->padSwingController;
            unsigned char* mgr = SharedDocBytes();
            unsigned char* g = GolfDocBytes();
            float t = ((ShotGauge54*)o)->value();
            float gaugeTotal =
                ((ShotAccountStatisticsView*)(mgr + AsCGolfDoc(g)->currentPlayer * 0x1D4))->swingGaugeTotal;
            ((ShotAccountStatisticsView*)(mgr + AsCGolfDoc(g)->currentPlayer * 0x1D4))->swingGaugeTotal =
                gaugeTotal + t;
            if (((ShotAccountStatisticsView*)(SharedDocBytes() + AsCGolfDoc(GolfDocBytes())->currentPlayer * 0x1D4))
                    ->swingGaugeTotal > 1000000.0f) {
                ((ShotAccountStatisticsView*)(SharedDocBytes() + AsCGolfDoc(GolfDocBytes())->currentPlayer * 0x1D4))
                    ->swingGaugeTotal = 1000000.0f;
            }
            o = AsCGolfRule(self)->padSwingController;
            ShotGauge_AddClampedTotal(((ShotGauge54*)o)->value());
        }
    }
}
#pragma pop

// Runtime pre-data record (0x74 bytes, same storage as golfball.h::sExpected).
// Position/velocity are world XYZ, +Y up; rotation is the ball's two-float pair.
// Offsets +0x20/+0x60/+0x61 are eventSoundName/eventKind/eventSubkind, cleared
// before stepping and consumed by UpdateBallPlayback for sound/effect dispatch.
// motionState copies live m_state: 0 stopped, 1 airborne, 3 rolling in this path.
// Superseded by the shared ShotSample model above.
typedef char ShotSampleSize[(sizeof(ShotSample) == 0x74) ? 1 : -1];
typedef char ShotSampleStateOffset[((int)&((ShotSample*)0)->motionState == 0x70) ? 1 : -1];

// Simulation-only views for the gaps in shared headers. Each pointer is borrowed
// and biased exactly as the original access; no new object/storage is allocated.
// ShotWorldClockView is in the common model block.
// BallGroundTriangleView is in the common model block.
// BallHoleInWordsView is in the common model block.
// ShotAccountStatisticsView is in the common model block.
// ShotPlayerTotalsView is in the common model block.
// Current per-player checkpoint: RULE + player*0x54 +0x10C; the same 0x54-byte
// payload is copied to the segmented history. Reserved130 is preserved by the
// original S54 word/byte-copy shape. Byte AE/BC effects remain explicitly opaque
// player snapshot state, not guessed counters. Club name must fit 3 chars+NUL.
// RuleShotCheckpointView is in the common model block.
// ShotHistoryHeaderView is in the common model block.
typedef char ShotCheckpointEnd[(int)&((RuleShotCheckpointView*)0)->powerMeterCost == 0x15C ? 1 : -1];
typedef char ShotWorldFrameOffset[(int)&((ShotWorldClockView*)0)->frame == 0x21AC ? 1 : -1];
// All simulation field accesses retain signed word/byte widths; references to
// these views describe live storage, never serialized asset pointers.

// Domain map for the branches below:
// selected club +A: wood0/iron1/wedge2/putter3. Power mode +44: normal0/power1/
// double2. Selected club +8: authored special-shot variant 0..2, choosing launch
// multipliers and distance cutoffs; no individual-club/display name is proven.
// impactGrade: 0 unclassified, 1 miss, 2 poor, 3 good, 4 Pangya. impactDirection
// sent to caddie message 0x10: 0 Pangya, +/-1 good, +/-2 poor, +/-3 miss, sign
// determined by impactGaugeOffset. These are separate from live/sample motion
// states 0 stopped/1 airborne/3 rolling and from power flag bits 0x10/20/40.
// Table +48 command kind 4 enables spin recalculation and the SetShotOffset path;
// other codes skip those branches. Preserve that discriminator, do not assume
// that all power shots use command 4. Sample event bytes 60/61 are separate again:
// UpdateBallPlayback consumes 60 as an effect/event kind and 61 as its subkind.
// Remaining byte-copy boundaries in this pipeline:
// - Player +1C is current ball position; +10 is last safe position; +28[hole]
//   stroke count, +3A[hole] result byte, +4C[hole*4] Pang word. Snapshot header
//   GolfDocSnapshot8013 names the byte arrays; word-copy access keeps their widths.
// - Character ID subtracts category base 0x04000000 before checking indices 1/5/6
//   for miss restrictions. This is an IFF character type ID (RoundPlayerSelection),
//   not a special-shot code. Keep unsigned subtraction/wrap and the per-club cases.
// - Ball +1989C/+198A0/+198A4 are WCrypticValue encoded word/address mix/key.
//   The four-byte XOR loop intentionally includes the ADDRESS of local key storage;
//   keep native Wii pointer width and do not serialize this runtime encoding.
// - Shot history uses blocks of 16 checkpoints of 0x54 bytes. Header head/count,
//   two-word block-map iterator and occupied map count are ShotHistoryHeaderView; it.cur is
//   a block-pointer cursor. The spare block, count-before-increment and copy try/
//   catch preserve container lifetime/exception entries. No payload is published
//   outside the rule; a failure can leave earlier stats/effects/ball samples changed.
// Signed casts from shared unsigned ABI words (party mode below) deliberately
// retain the old signed compare instruction; changing compare signedness is beyond
// matching even though only small domain values are admitted.

// GOLF_CLUB_CATEGORY elsewhere in this merged unit reads the selected club's
// category byte. This pipeline uses the equivalent typed field access.
#define GOLF_SELECTED_CLUB_KIND (((ClubSelectionRuntime*)GOLF_SHOT_PARAMETERS_BYTES)->selectedClub->clubKind)
// ===== S12 - Shot simulation and launch model =====
// ShotSample, ShotCheckpoint, account statistics and world clock.
}
}

// Classify impact, seed launch, build fixed-step history, recover and rewind.
// Required constructed rule and live doc/player/ball/club; gauge power must be
// in the range accepted by sqrt(powerFraction), with nonzero impact denominators.
// previewOnly!=0 still adjusts impact, records manual-swing statistics and emits
// feedback, but returns 1 before sample generation. Full path also returns 1;
// it is not a validation/error status and allocation failures may throw.
// Full path owns samples through the BALL and appends a copied 0x54 shot-history
// record through the RULE; borrowed player/terrain/club pointers never transfer.
// Each loop calls animated-object update THEN ball integration with the same
// binary32 0.03333333507180214 step; no elapsed wall-clock dt changes that step.
// Stop/recovery thresholds, event sample latches and rewind-to-sample-0 form the
// playback contract: completed samples remain while live ball state is rewound.
// Scalar/FP_F* expression order, deterministic assistance, message reply sizes, repeated
// singleton observations and stop-counter comparisons must survive future edits.
int CGolfRule::BuildShotSimulation(int previewOnly) {
    using namespace ns4;
    // Writes: ball powerFlags, shotGaugeDirection and impactGrade.
    // Phase 1: obtain selected spin/curve, classify impact, apply assistance,
    // then classify again against the adjusted gauge. Preserve both passes.
    unsigned char* self = (unsigned char*)this;
    int zero = 0;
    float yawCorrectionRadians, impactGaugeOffset, powerFraction, launchSpeed;
    // impactGaugeOffset starts as a signed gauge-space delta. launchSpeed is
    // a speed through launch and special-shot simulation; finalHorizontalDistanceWorldUnits
    // is a separate later value. World-space vectors use Y up.
    // Independent domain values; registers may be reused when lifetimes end.
    float pangyaHalfWidth, outsidePangyaGaugeDistance, curveOverride;
    float baseLaunchSpeed, weightedGoodImpactLieScale;
    float absoluteNormalizedImpact, poorImpactGaugeDistance, absoluteCurveSetting;
    float collisionRadiusWorldUnits, normalizedImpactRatio, spinAccessoryBonus;
    float poorImpactRangeWidth, weightedLowerLieScale, cobraTravelLimit;
    int impactDirection;
    unsigned char* effectStateBytes;
    unsigned char* firstSampleBytes;
    unsigned char* ballBytes;
    char simulationStopwatch[0x28];
    float ironMissPitch[12], wedgeMissPitch[12], launchPitch[12], launchYaw[12];
    float screenVectorReply[3];
    float cobraTravelXZ[3], spikeTravelXZ[3], totalTravelXZ[3], woodMissRollingVelocity[3], ironMissRollingVelocity[3],
        ironMissAirborneVelocity[3], wedgeMissRollingVelocity[3], wedgeMissAirborneVelocity[3];
    WVector worldLaunchVelocity, shotWind;
    float waterTestPosition[3], outsideTestPosition[3];
    WVector groundRecoveryPosition;
    float groundQueryPosition[3];
    float recoveryPosition[3], lastPlayerPosition[3];
    WVector aimRecoveryPosition;
    float waterRecoveryTestPosition[3];
    float finalHorizontalDisplacement[3];
    int screenPowerFlagsReply;
    float headingRadians, shotKeySeconds, normalModeBaseSpeed, modeOneBaseSpeed, modeTwoBaseSpeed;
    AsCGolfBall(GolfBall__Fv())->m_powerFlags = zero;
    {
        unsigned char* a = GOLF_BALL_BYTES;
        unsigned char* b = GOLF_BALL_BYTES;
        IActor_DispatchMessage(self, self, golfRuleStrings + kText_spin, 1, (int)&AsCGolfBall(b)->m_initCurve,
                               (int)&AsCGolfBall(a)->m_initSpin, 0, 0);
    }
    if (GOLF_SELECTED_CLUB_KIND == kClubPutter) {
        AsCGolfDoc(GolfDocBytes())->shotGaugeDirection = zero;
        AsCGolfDoc(GolfDocBytes())->impactGrade = kImpactPangya;
    } else {
        int hasImpactAssistItem = 0;
        int impactGrade;
        AsCGolfDoc(GolfDocBytes())->shotGaugeDirection = zero;
        pangyaHalfWidth = kFloat_2_805DEA84 * (float)s_pangyaZoneBaseWidthUnits;
        {
            unsigned char* lay = (unsigned char*)AsCSharedDoc(SharedDocBytes())->itemInventory;
            unsigned char* pl = (unsigned char*)GolfPlayer_GetEntryByIndex((int)GolfDocBytes(),
                                                                           AsCGolfDoc(GolfDocBytes())->currentPlayer);
            // Item tests at hit share the following adjustment.
            if ((unsigned)AsGolfDocSnapshot(pl)->itemId == (unsigned)ItemInventory_GetItemIdAt(lay, 9) ||
                (lay = (unsigned char*)AsCSharedDoc(SharedDocBytes())->itemInventory,
                 pl = (unsigned char*)GolfPlayer_GetEntryByIndex((int)GolfDocBytes(),
                                                                 AsCGolfDoc(GolfDocBytes())->currentPlayer),
                 (unsigned)AsGolfDocSnapshot(pl)->itemId == (unsigned)ItemInventory_GetItemIdAt(lay, 1))) {
                hasImpactAssistItem = 1;
                pangyaHalfWidth = pangyaHalfWidth * kFloat_2_805DEA84;
            }
        }
        pangyaHalfWidth = pangyaHalfWidth * AsCGolfDoc(GolfDocBytes())->pangyaZoneScale;
        if ((int)AsCSharedDoc(SharedDocBytes())->partyGameMode == 3) {
            unsigned char* effectStateBytes = (unsigned char*)PartyItemSlots_GetInstance();
            if (hasImpactAssistItem == 0)
                pangyaHalfWidth =
                    pangyaHalfWidth * (1.0f + ((ShotAccessoryModifiers*)effectStateBytes)->impactWidthBonus);
            pangyaHalfWidth = pangyaHalfWidth / ((ShotAccessoryModifiers*)effectStateBytes)->gaugeRateScale;
        }
        {
            unsigned char* g = GolfDocBytes();
            float d = AsCGolfDoc(g)->shotImpactGaugeX - AsCGolfDoc(g)->pangyaTargetX;
            d = (d > 0.0f) ? d : -d;
            if (d < 0.5f + pangyaHalfWidth) {
                impactGrade = kImpactPangya;
            } else {
                outsidePangyaGaugeDistance = AsCGolfDoc(g)->shotImpactGaugeX - 149.0f;
                outsidePangyaGaugeDistance =
                    (outsidePangyaGaugeDistance > 0.0f) ? outsidePangyaGaugeDistance : -outsidePangyaGaugeDistance;
                if (outsidePangyaGaugeDistance < 0.5f + ShotParameters_GetGoodImpactHalfWidth(InstanceTable_Get())) {
                    impactGrade = kImpactGood;
                } else {
                    if (outsidePangyaGaugeDistance < 0.5f + ShotParameters_GetPoorImpactHalfWidth(InstanceTable_Get()))
                        impactGrade = kImpactPoor;
                    else
                        impactGrade = kImpactMiss;
                }
            }
        }
        if (impactGrade <= kImpactMiss) {
            unsigned char* g = GolfDocBytes();
            if (AsCGolfDoc(g)->shotImpactGaugeX - 149.0f < 0.0f) {
                AsCGolfDoc(g)->shotImpactGaugeX =
                    0.01f + (149.0f - (0.5f + ShotParameters_GetPoorImpactHalfWidth(InstanceTable_Get())));
            } else {
                AsCGolfDoc(g)->shotImpactGaugeX =
                    (149.0f + (0.5f + ShotParameters_GetPoorImpactHalfWidth(InstanceTable_Get()))) - 0.01f;
            }
            AsCGolfDoc(GolfDocBytes())->shotGaugeCursorX = AsCGolfDoc(GolfDocBytes())->shotImpactGaugeX;
            AsCGolfDoc(GolfDocBytes())->roundedShotGaugeCursorX =
                (float)ceil(AsCGolfDoc(GolfDocBytes())->shotGaugeCursorX);
        }
        CGolfRule::ApplyImpactAssistance(self, 1);
        hasImpactAssistItem = 0;
        pangyaHalfWidth = kFloat_2_805DEA84 * (float)s_pangyaZoneBaseWidthUnits;
        {
            unsigned char* lay = (unsigned char*)AsCSharedDoc(SharedDocBytes())->itemInventory;
            unsigned char* pl = (unsigned char*)GolfPlayer_GetEntryByIndex((int)GolfDocBytes(),
                                                                           AsCGolfDoc(GolfDocBytes())->currentPlayer);
            // Item tests at hit2 share the following adjustment.
            if ((unsigned)AsGolfDocSnapshot(pl)->itemId == (unsigned)ItemInventory_GetItemIdAt(lay, 9) ||
                (lay = (unsigned char*)AsCSharedDoc(SharedDocBytes())->itemInventory,
                 pl = (unsigned char*)GolfPlayer_GetEntryByIndex((int)GolfDocBytes(),
                                                                 AsCGolfDoc(GolfDocBytes())->currentPlayer),
                 (unsigned)AsGolfDocSnapshot(pl)->itemId == (unsigned)ItemInventory_GetItemIdAt(lay, 1))) {
                hasImpactAssistItem = 1;
                pangyaHalfWidth = pangyaHalfWidth * kFloat_2_805DEA84;
            }
        }
        pangyaHalfWidth = pangyaHalfWidth * AsCGolfDoc(GolfDocBytes())->pangyaZoneScale;
        if ((int)AsCSharedDoc(SharedDocBytes())->partyGameMode == 3) {
            unsigned char* effectStateBytes = (unsigned char*)PartyItemSlots_GetInstance();
            if (hasImpactAssistItem == 0)
                pangyaHalfWidth =
                    pangyaHalfWidth * (1.0f + ((ShotAccessoryModifiers*)effectStateBytes)->impactWidthBonus);
            pangyaHalfWidth = pangyaHalfWidth / ((ShotAccessoryModifiers*)effectStateBytes)->gaugeRateScale;
        }
        {
            unsigned char* g = GolfDocBytes();
            float d = AsCGolfDoc(g)->shotImpactGaugeX - AsCGolfDoc(g)->pangyaTargetX;
            d = (d > 0.0f) ? d : -d;
            if (d < 0.5f + pangyaHalfWidth) {
                impactGrade = kImpactPangya;
            } else {
                outsidePangyaGaugeDistance = AsCGolfDoc(g)->shotImpactGaugeX - 149.0f;
                outsidePangyaGaugeDistance =
                    (outsidePangyaGaugeDistance > 0.0f) ? outsidePangyaGaugeDistance : -outsidePangyaGaugeDistance;
                if (outsidePangyaGaugeDistance < 0.5f + ShotParameters_GetGoodImpactHalfWidth(InstanceTable_Get())) {
                    impactGrade = kImpactGood;
                } else {
                    if (outsidePangyaGaugeDistance < 0.5f + ShotParameters_GetPoorImpactHalfWidth(InstanceTable_Get()))
                        impactGrade = kImpactPoor;
                    else
                        impactGrade = kImpactMiss;
                }
            }
        }
        AsCGolfDoc(GolfDocBytes())->impactGrade = impactGrade;
        if (AsCGolfDoc(GolfDocBytes())->impactGrade == kImpactPangya) {
            if (AsCGolfDoc(GolfDocBytes())->shotPowerGaugeX >= 437.0f) {
                IActor_DispatchMessage(self, self, golfRuleStrings + kText_screen, 0x32, (int)&screenPowerFlagsReply, 0,
                                       0, 0);
                int t = screenPowerFlagsReply;
                AsCGolfBall(GOLF_BALL_BYTES)->m_powerFlags = t;
                IActor_DispatchMessage(self, self, golfRuleStrings + kText_screen, 0x33, (int)&screenPowerFlagsReply, 0,
                                       0, 0);
                {
                    unsigned char* p = GOLF_BALL_BYTES;
                    AsCGolfBall(p)->m_powerFlags = AsCGolfBall(p)->m_powerFlags | screenPowerFlagsReply;
                }
            }
            if (AsCGolfBall(GOLF_BALL_BYTES)->m_powerFlags != 0) {
                if (AsCGolfBall(GOLF_BALL_BYTES)->m_powerFlags & kSpecialShotMask) {
                    if (((ClubSelectionRuntime*)GOLF_SHOT_PARAMETERS_BYTES)->powerShotMode == 0 ||
                        ((AsCGolfBall(GOLF_BALL_BYTES)->m_powerFlags & kShotCobra) &&
                         GOLF_SELECTED_CLUB_KIND != kClubWood)) {
                        int clear = 0;
                        AsCGolfBall(GOLF_BALL_BYTES)->m_powerFlags = clear;
                    }
                    if (((ClubSelectionRuntime*)GOLF_SHOT_PARAMETERS_BYTES)->powerShotMode == 0 ||
                        ((AsCGolfBall(GOLF_BALL_BYTES)->m_powerFlags & kShotSpike) &&
                         GOLF_SELECTED_CLUB_KIND != kClubWood)) {
                        int clear = 0;
                        AsCGolfBall(GOLF_BALL_BYTES)->m_powerFlags = clear;
                    }
                }
                if (AsCGolfBall(GOLF_BALL_BYTES)->m_powerFlags & kShotCurveNegative) {
                    curveOverride = -1.5f;
                    AsCGolfBall(GOLF_BALL_BYTES)->m_initCurve = curveOverride;
                } else if (AsCGolfBall(GOLF_BALL_BYTES)->m_powerFlags & kShotCurvePositive) {
                    curveOverride = 1.5f;
                    AsCGolfBall(GOLF_BALL_BYTES)->m_initCurve = curveOverride;
                }
            }
            {
                unsigned char* p = GOLF_BALL_BYTES;
                AsCGolfBall(p)->m_powerFlags = AsCGolfBall(p)->m_powerFlags | AsCGolfRule(self)->deferredInputButtons;
                AsCGolfRule(self)->deferredInputButtons = 0;
            }
        }
        {
            unsigned char* p = GOLF_BALL_BYTES;
            AsCGolfBall(p)->m_powerFlags = AsCGolfBall(p)->m_powerFlags | AsCGolfRule(self)->deferredInputButtons;
            AsCGolfRule(self)->deferredInputButtons = 0;
        }
    }

    // Writes: manualSwings and swingGaugeTotal.
    // Phase 2: update manual-swing statistics before the preview/full split.
    RecordManualSwingStatistics(self);

    // Writes: feedback only; full simulation remains untouched.
    // Phase 3: feedback-only exit. No sample simulation occurs on this path.
    if (previewOnly != 0) {
        IActor_DispatchMessage(self, self, golfRuleStrings + kText_screen, 0xB, 0, 0, 0, 0);
        unsigned char* manager = SharedDocBytes();
        float d = AsCGolfDoc((*(unsigned char**)&AsCSharedDoc(manager)->golfDoc))->shotPowerGaugeX - 509.0f;
        if (OrderedLess54(WabsHint(d), 0.00001f) &&
            AsCGolfDoc((*(unsigned char**)&AsCSharedDoc(manager)->golfDoc))->impactTimingSpriteIndex == 6) {
            IActor_DispatchMessage(self, self, golfRuleStrings + kText_screen, 0xA, 0, 0, 0, 0);
        }
        if (((ClubSelectionRuntime*)GOLF_SHOT_PARAMETERS_BYTES)->powerShotMode != 0) {
            Sound_PlayByName(g_soundManager, golfRuleStrings + kText_powershot_ball_effect, 0, 0, 0, 0, 0.5f, 200.0f,
                             1.0f);
            IActor_DispatchMessage(self, self, golfRuleStrings + kText_effect, 0x12, 1, 0, 0, 0);
        }
        return 1;
    }

    // Writes: sample position/velocity/motionState and ball curve/spin settings.
    // Phase 4: seed sample 0 from the current ball; derive launch speed from
    // sqrt(power fraction), then adjust heading/speed/spin for impact and lie.
    ballBytes = GOLF_BALL_BYTES;
    {
        unsigned char* g = GolfDocBytes();
        unsigned char* pl = (unsigned char*)GolfPlayer_GetEntryByIndex((int)g, AsCGolfDoc(g)->currentPlayer);
        ((ShotPlayerTotalsView*)pl)->accumulatedShotMilliseconds =
            ((ShotPlayerTotalsView*)pl)->accumulatedShotMilliseconds + AsCGolfDoc(g)->elapsedTimeMs;
    }
    IActor_DispatchMessage(self, self, golfRuleStrings + kText_clock, 1, 0, 0, 0, 0);
    IActor_DispatchMessage(self, self, golfRuleStrings + kText_spot, 4, 0, 0, 0, 0);
    IActor_DispatchMessage(self, self, golfRuleStrings + kText_grid, 3, 0, 0, 0, 0);
    yawCorrectionRadians = 0.0f;
    impactGaugeOffset = AsCGolfDoc(GolfDocBytes())->shotImpactGaugeX - AsCGolfDoc(GolfDocBytes())->pangyaTargetX;
    powerFraction = (AsCGolfDoc(GolfDocBytes())->shotPowerGaugeX - 149.0f) / 360.0f;
    baseLaunchSpeed = ShotParameters_GetBaseLaunchSpeed(InstanceTable_Get());
    float rootPower = Math_SquareRootApprox(powerFraction);
    launchSpeed = rootPower * baseLaunchSpeed;
    if (AsCGolfBall(GOLF_BALL_BYTES)->m_powerFlags & kShotTomahawk)
        launchSpeed = launchSpeed * kFloat_1_3_GolfPowerFlags;
    if (AsCGolfBall(GOLF_BALL_BYTES)->m_powerFlags & kShotSpike)
        launchSpeed = launchSpeed * kFloat_1_3_GolfPowerFlags;
    IActor_DispatchMessage(self, self, golfRuleStrings + kText_player, 3, (int)&headingRadians, 0, 0, 0);
    headingRadians = headingRadians + AsGolfRuleFrame(self)->launchHeadingOffset;
    firstSampleBytes = (unsigned char*)((CGolfBall*)ballBytes)->GetOrAllocatePreDataAt(0);
    ((ShotSample*)firstSampleBytes)->eventKind = 0;
    ((ShotSample*)firstSampleBytes)->m_pos = AsCGolfBall(ballBytes)->m_pos;
    // 0x801611E8: Mode 3 skips the ordinary shot setup and keeps the shared tail.
    do {
        if (GOLF_SELECTED_CLUB_KIND == kClubPutter) {
            ((ShotSample*)firstSampleBytes)->motionState = 3;
            break;
        }
        ((ShotSample*)firstSampleBytes)->motionState = 1;
        if (((ClubSelectionRuntime*)GOLF_SHOT_PARAMETERS_BYTES)->powerShotMode != 0) {
            Sound_PlayByName(g_soundManager, golfRuleStrings + kText_powershot_ball_effect, 0, 0, 0, 0, 0.5f, 200.0f,
                             1.0f);
            IActor_DispatchMessage(self, self, golfRuleStrings + kText_effect, 0x12, 1, 0, 0, 0);
        }
        {
            unsigned char* g = GolfDocBytes();
            switch (AsCGolfDoc(g)->impactGrade) {
            case kImpactPangya:
                if (AsCGolfDoc(g)->pangyaAssistState == 2) {
                    ((ShotAccountStatisticsView*)(SharedDocBytes() + AsCGolfDoc(g)->currentPlayer * 0x1D4))
                        ->assistedPangyas =
                        ((ShotAccountStatisticsView*)(SharedDocBytes() + AsCGolfDoc(g)->currentPlayer * 0x1D4))
                            ->assistedPangyas +
                        1;
                }
                launchSpeed =
                    launchSpeed * Math_SquareRootApprox(((CQuadTree*)CQuadTree_GetInstance())->GetMaxPowerOnGround());
                yawCorrectionRadians =
                    FP_FNMSUBS(0.3490658402442932f, AsCGolfBall(ballBytes)->m_initCurve, yawCorrectionRadians);
                impactDirection = 0;
                break;
            case kImpactGood:
                if (impactGaugeOffset > 0.0f) {
                    normalizedImpactRatio =
                        impactGaugeOffset / ((149.0f + ShotParameters_GetGoodImpactHalfWidth(InstanceTable_Get())) -
                                             AsCGolfDoc(g)->pangyaTargetX);
                } else {
                    normalizedImpactRatio =
                        impactGaugeOffset / (AsCGolfDoc(g)->pangyaTargetX -
                                             (149.0f - ShotParameters_GetGoodImpactHalfWidth(InstanceTable_Get())));
                }
                if (normalizedImpactRatio > 0.0f)
                    weightedGoodImpactLieScale = normalizedImpactRatio;
                else
                    weightedGoodImpactLieScale = -normalizedImpactRatio;
                if (normalizedImpactRatio > 0.0f)
                    absoluteNormalizedImpact = normalizedImpactRatio;
                else
                    absoluteNormalizedImpact = -normalizedImpactRatio;
                weightedGoodImpactLieScale =
                    weightedGoodImpactLieScale * ((CQuadTree*)CQuadTree_GetInstance())->GetMinPowerOnGround();
                launchSpeed = launchSpeed * Math_SquareRootApprox(
                                                FP_FMADDS((1.0f - absoluteNormalizedImpact),
                                                          ((CQuadTree*)CQuadTree_GetInstance())->GetMaxPowerOnGround(),
                                                          weightedGoodImpactLieScale));
                {
                    float old = AsCGolfBall(ballBytes)->m_initCurve;
                    AsCGolfBall(ballBytes)->m_initCurve = FP_FNMSUBS(0.2f, normalizedImpactRatio, old);
                    yawCorrectionRadians = FP_FNMSUBS(0.3490658402442932f, old, yawCorrectionRadians);
                }
                if (impactGaugeOffset > 0.0f)
                    impactDirection = 1;
                else
                    impactDirection = -1;
                break;
            case kImpactPoor:
                poorImpactGaugeDistance = AsCGolfDoc(g)->shotImpactGaugeX - 149.0f;
                poorImpactGaugeDistance =
                    (poorImpactGaugeDistance > 0.0f) ? poorImpactGaugeDistance : -poorImpactGaugeDistance;
                poorImpactRangeWidth = ShotParameters_GetGoodImpactHalfWidth(InstanceTable_Get());
                poorImpactRangeWidth =
                    ShotParameters_GetPoorImpactHalfWidth(InstanceTable_Get()) - poorImpactRangeWidth;
                normalizedImpactRatio =
                    (poorImpactGaugeDistance - ShotParameters_GetGoodImpactHalfWidth(InstanceTable_Get())) /
                    poorImpactRangeWidth;
                weightedLowerLieScale =
                    normalizedImpactRatio * ((CQuadTree*)CQuadTree_GetInstance())->GetMinPowerOnGround();
                launchSpeed = launchSpeed * Math_SquareRootApprox(
                                                FP_FMADDS((1.0f - normalizedImpactRatio),
                                                          ((CQuadTree*)CQuadTree_GetInstance())->GetMinPowerOnGround(),
                                                          weightedLowerLieScale));
                AsCGolfBall(ballBytes)->m_initCurve = 0.2f;
                AsCGolfBall(ballBytes)->m_initSpin = 0.0f;
                if (AsCGolfDoc(GolfDocBytes())->shotImpactGaugeX > 149.0f) {
                    impactDirection = 2;
                    AsCGolfBall(ballBytes)->m_initCurve = AsCGolfBall(ballBytes)->m_initCurve * kFloat_neg1_GolfRule;
                    yawCorrectionRadians = FP_FNMSUBS(0.1745329201221466f, normalizedImpactRatio, yawCorrectionRadians);
                } else {
                    impactDirection = -2;
                    yawCorrectionRadians = FP_FMADDS(0.1745329201221466f, normalizedImpactRatio, yawCorrectionRadians);
                }
                break;
            case kImpactUnclassified:
            case kImpactMiss:
                AsCGolfBall(ballBytes)->m_initCurve = 0.0f;
                AsCGolfBall(ballBytes)->m_initSpin = 0.0f;
                switch (GOLF_SELECTED_CLUB_KIND) {
                case kClubWood:
                    if (AsCGolfDoc(GolfDocBytes())->shotImpactGaugeX > AsCGolfDoc(GolfDocBytes())->pangyaTargetX) {
                        Vec3_Zero(&((ShotSample*)firstSampleBytes)->velocity);
                        ((ShotSample*)firstSampleBytes)->motionState = 0;
                    } else {
                        unsigned t = ((ShotAccountStatisticsView*)(SharedDocBytes() +
                                                                   AsCGolfDoc(GolfDocBytes())->currentPlayer * 0x1D4))
                                         ->characterId -
                                     0x4000000;
                        if (t == 1 || t == 6) {
                            Vec3_Zero(&((ShotSample*)firstSampleBytes)->velocity);
                            ((ShotSample*)firstSampleBytes)->motionState = 0;
                        } else {
                            woodMissRollingVelocity[0] = 0.0f;
                            woodMissRollingVelocity[1] = 0.0f;
                            woodMissRollingVelocity[2] = 15.0f;
                            ((ShotSample*)firstSampleBytes)->velocity = *(WVector*)woodMissRollingVelocity;
                            ((ShotSample*)firstSampleBytes)->motionState = 3;
                            yawCorrectionRadians = yawCorrectionRadians + kFloat_0_69813168_GolfRule;
                        }
                    }
                    break;
                case kClubIron:
                    if (AsCGolfDoc(GolfDocBytes())->shotImpactGaugeX > AsCGolfDoc(GolfDocBytes())->pangyaTargetX) {
                        ironMissRollingVelocity[0] = 0.0f;
                        ironMissRollingVelocity[1] = 0.0f;
                        ironMissRollingVelocity[2] = 20.0f;
                        ((ShotSample*)firstSampleBytes)->velocity = *(WVector*)ironMissRollingVelocity;
                        ((ShotSample*)firstSampleBytes)->motionState = 3;
                    } else {
                        Matrix_SetAxisRotation(ironMissPitch, 1.3962633609771729f, 0);
                        __ml__FfRC7WVector(ironMissAirborneVelocity, ironMissPitch + 6, 50.0f);
                        ((ShotSample*)firstSampleBytes)->velocity = *(WVector*)ironMissAirborneVelocity;
                        ((ShotSample*)firstSampleBytes)->motionState = 1;
                    }
                    {
                        unsigned t = ((ShotAccountStatisticsView*)(SharedDocBytes() +
                                                                   AsCGolfDoc(GolfDocBytes())->currentPlayer * 0x1D4))
                                         ->characterId -
                                     0x4000000;
                        if (t == 6) {
                            Vec3_Zero(&((ShotSample*)firstSampleBytes)->velocity);
                            ((ShotSample*)firstSampleBytes)->motionState = 0;
                        }
                    }
                    break;
                case kClubWedge:
                    if (AsCGolfDoc(GolfDocBytes())->shotImpactGaugeX > AsCGolfDoc(GolfDocBytes())->pangyaTargetX) {
                        wedgeMissRollingVelocity[0] = 0.0f;
                        wedgeMissRollingVelocity[1] = 0.0f;
                        wedgeMissRollingVelocity[2] = 5.0f;
                        ((ShotSample*)firstSampleBytes)->velocity = *(WVector*)wedgeMissRollingVelocity;
                        ((ShotSample*)firstSampleBytes)->motionState = 3;
                    } else {
                        Matrix_SetAxisRotation(wedgeMissPitch, 1.3962633609771729f, 0);
                        __ml__FfRC7WVector(wedgeMissAirborneVelocity, wedgeMissPitch + 6, 20.0f);
                        ((ShotSample*)firstSampleBytes)->velocity = *(WVector*)wedgeMissAirborneVelocity;
                        ((ShotSample*)firstSampleBytes)->motionState = 1;
                    }
                    break;
                default:
                    break;
                }
                if (GOLF_SELECTED_CLUB_KIND != kClubWedge) {
                    unsigned t = ((ShotAccountStatisticsView*)(SharedDocBytes() +
                                                               AsCGolfDoc(GolfDocBytes())->currentPlayer * 0x1D4))
                                     ->characterId -
                                 0x4000000;
                    if (t == 5) {
                        Vec3_Zero(&((ShotSample*)firstSampleBytes)->velocity);
                        ((ShotSample*)firstSampleBytes)->motionState = 0;
                    }
                }
                if (impactGaugeOffset > 0.0f)
                    impactDirection = 3;
                else
                    impactDirection = -3;
                break;
            default:
                break;
            }
        }
        {
            unsigned char* g = GolfDocBytes();
            int impactGrade = AsCGolfDoc(g)->impactGrade;
            if (impactGrade > 1 && AsCGolfDoc(g)->powerShotMeterOverride == 0) {
                if (!(impactGrade == 4 && ((GolfDocShotBytes*)g)->unk_2D8 != 0 &&
                      ((ClubSelectionRuntime*)GOLF_SHOT_PARAMETERS_BYTES)->powerShotMode != 0 &&
                      (AsCGolfBall(ballBytes)->m_powerFlags & kSpecialShotMask) == 0)) {
                    IActor_DispatchMessage(self, self, golfRuleStrings + kText_screen, 0x23, (int)screenVectorReply, 0,
                                           0, 0);
                    if (AsCGolfBall(ballBytes)->m_powerFlags & kShotSpike)
                        AsCGolfBall(ballBytes)->m_initCurve =
                            FP_FMADDS(0.15000000596046448f, screenVectorReply[0], AsCGolfBall(ballBytes)->m_initCurve);
                    else
                        AsCGolfBall(ballBytes)->m_initCurve =
                            FP_FMADDS(0.5f, screenVectorReply[0], AsCGolfBall(ballBytes)->m_initCurve);
                }
            }
        }
        IActor_DispatchMessage(self, self, golfRuleStrings + kText_screen, 0xB, 0, 0, 0, 0);
        {
            unsigned char* manager = SharedDocBytes();
            float d = AsCGolfDoc((*(unsigned char**)&AsCSharedDoc(manager)->golfDoc))->shotPowerGaugeX - 509.0f;
            if (OrderedLess54(WabsHint(d), 0.00001f) &&
                AsCGolfDoc((*(unsigned char**)&AsCSharedDoc(manager)->golfDoc))->impactTimingSpriteIndex == 6) {
                IActor_DispatchMessage(self, self, golfRuleStrings + kText_screen, 0xA, 0, 0, 0, 0);
            }
        }
        IActor_DispatchMessage(self, self, golfRuleStrings + kText_caddie, 0x10, impactDirection, 0, 0, 0);
    } while (0);

    // Writes: sample velocity and launch rotation matrices.
    // Phase 5: rotate the launch vector by elevation then yaw. For a non-miss
    // sample +0C receives velocity; sample +70 carries the initial physics state.
    float negativeOne = kFloat_neg1_GolfRule;
    AsCGolfBall(ballBytes)->m_initCurve = AsCGolfBall(ballBytes)->m_initCurve * negativeOne;
    yawCorrectionRadians = yawCorrectionRadians * negativeOne;
    if (AsCGolfDoc(GolfDocBytes())->impactGrade > 1) {
        absoluteCurveSetting = AsCGolfBall(ballBytes)->m_initCurve;
        absoluteCurveSetting = (absoluteCurveSetting > 0.0f) ? absoluteCurveSetting : -absoluteCurveSetting;
        Matrix_SetAxisRotation(launchYaw, headingRadians + yawCorrectionRadians, 1);
        Matrix_SetAxisRotation(launchPitch, ClubSelection_GetLaunchAngle(InstanceTable_Get(), 0), 0);
        WVector pitchAdjustedVelocity =
            (launchSpeed * FP_FMADDS(0.10000000149011612f, absoluteCurveSetting, 1.0f)) * *(WVector*)(launchPitch + 6);
        Matrix_TransformPoint(&worldLaunchVelocity, &pitchAdjustedVelocity, launchYaw);
        ((ShotSample*)firstSampleBytes)->velocity = worldLaunchVelocity;
    }
    // Writes: checkpoint/replay inputs, world clock and live ball flags/velocity.
    // Phase 6: snapshot shot/weather/player parameters for replay/history, then
    // align the animated-object simulation clock with the shot animation.
    CaptureShotReplayInputs(self, ballBytes, headingRadians, shotWind);
    if (s_cameraDirectedShotEnabled != 0) {
        AsCGolfBall(ballBytes)->m_pos = *(WVector*)(s_cameraTransform + 0x24);
        WVector cameraDirectedVelocity = *(WVector*)(s_cameraTransform + 0x18) * 230.0f;
        ((ShotSample*)firstSampleBytes)->velocity = cameraDirectedVelocity;
        ((ShotSample*)firstSampleBytes)->motionState = 1;
        ((BallGroundTriangleView*)ballBytes)->groundTriangleHandle = -1;
    }
    IActor_DispatchMessage(self, self, golfRuleStrings + kText_player, 0x20, (int)&shotKeySeconds, 0, 0, 0);
    if (GolfTask_NoOpStatus() == 0) {
        if (((ClubSelectionRuntime*)GOLF_SHOT_PARAMETERS_BYTES)->powerShotMode != 0) {
            IActor_DispatchMessage(self, self, golfRuleStrings + kText_player, 0xC, 0, 0, 0, 0);
        }
        ((RuleUnresolvedShotBytes*)self)->stateEntryPending = 1;
        ((RuleUnresolvedShotBytes*)self)->unk_094 = AsCGolfBall(ballBytes)->m_groundType != 2;
    }
    AsGolfRuleFrame(self)->simulationStartFrame = ((ShotWorldClockView*)CQuadTree_GetInstance())->frame;
    if (((ClubSelectionRuntime*)GOLF_SHOT_PARAMETERS_BYTES)->powerShotMode != 0) {
        AsGolfRuleFrame(self)->playbackFrame = AsGolfRuleFrame(self)->simulationStartFrame + 0x1E;
        AsGolfRuleFrame(self)->shotContactFrame =
            AsGolfRuleFrame(self)->playbackFrame + Runtime_ConvertDoubleToUnsigned(30.0f * shotKeySeconds);
    } else {
        void* a = g_taskManager ? Object_GetChild(g_taskManager) : 0;
        void* b = TaskActorTable_FindByName(a, golfRuleStrings + kText_player);
        unsigned t;
        if (b == 0) {
            t = 0x1E;
        } else {
            t = ((ShotGauge54*)AsCGolfRule(self)->padSwingController)->slot1c(b);
        }
        if ((unsigned)AsGolfRuleFrame(self)->simulationStartFrame < t)
            AsGolfRuleFrame(self)->playbackFrame = 0;
        else
            AsGolfRuleFrame(self)->playbackFrame = AsGolfRuleFrame(self)->simulationStartFrame - t;
        AsGolfRuleFrame(self)->shotContactFrame = AsGolfRuleFrame(self)->simulationStartFrame;
    }
    {
        int saved = AsGolfRuleFrame(self)->shotContactFrame;
        unsigned char* ctl = (unsigned char*)CQuadTree_GetInstance();
        ((ShotWorldClockView*)ctl)->frame = saved;
        ((ShotWorldClockView*)ctl)->fractionalSeconds = 0.0f;
    }
    (*(WVector*)(ballBytes + kBallVelocityByteOffset)) = ((ShotSample*)firstSampleBytes)->velocity;
    {
        effectStateBytes = (unsigned char*)PartyItemSlots_GetInstance();
        unsigned char* key = AsCGolfBall(ballBytes)->m_holeInCrypt;
        struct {
            unsigned char* key;
            int sp14;
        } kk;
        unsigned char* kb = (unsigned char*)&kk;
        kk.key = key;
        kk.sp14 = 0;
        AsCGolfBall(ballBytes)->m_velocity.y =
            AsCGolfBall(ballBytes)->m_velocity.y *
            (1.0f + ((ShotAccessoryModifiers*)effectStateBytes)->verticalVelocityBonus);
        AsCGolfBall(ballBytes)->m_state = ((ShotSample*)firstSampleBytes)->motionState;
        AsCGolfBall(ballBytes)->m_flags = AsCGolfBall(ballBytes)->m_powerFlags;
        AsGolfRuleFrame(self)->animationStarted = 0;
        AsGolfRuleFrame(self)->presentationStep = 0;
        AsCGolfBall(ballBytes)->m_lie = 0;
        ((BallHoleInWordsView*)ballBytes)->encodedValue = ~((BallHoleInWordsView*)ballBytes)->key;
        for (int i = 0; i < sizeof(kk.key); i++) {
            ballBytes[0x198A0 + i] = key[i] ^ kb[i];
        }
    }
    {
        unsigned char* pl =
            (unsigned char*)GolfPlayer_GetEntryByIndex((int)GolfDocBytes(), AsCGolfDoc(GolfDocBytes())->currentPlayer);
        AsGolfDocSnapshot(pl)->unk_AC = -1;
        pl = (unsigned char*)GolfPlayer_GetEntryByIndex((int)GolfDocBytes(), AsCGolfDoc(GolfDocBytes())->currentPlayer);
        ((ShotPlayerTotalsView*)pl)->clearedShotWordB4 = 0;
        if (AsCGolfBall(GOLF_BALL_BYTES)->m_state != 0) {
            pl = (unsigned char*)GolfPlayer_GetEntryByIndex((int)GolfDocBytes(),
                                                            AsCGolfDoc(GolfDocBytes())->currentPlayer);
            AsGolfDocSnapshot(pl)->unk_AE = 0;
        }
    }
    // Writes: ball contact fields, sample counters and spin/curve.
    // Phase 7: clear collision/sample/contact state. Event indices start at -1;
    // rolling-start slot 8 is 0 only if the launch already starts rolling (state 3).
    ResetShotCollisionAndSamples(ballBytes);
    unsigned char saved59 = AsCGolfBall(ballBytes)->m_groundType;
    if (AsCGolfDoc(GolfDocBytes())->impactGrade == kImpactMiss) {
        AsCGolfBall(ballBytes)->m_spin = 0.0f;
        AsCGolfBall(ballBytes)->m_curve = 0.0f;
    } else {
        if (((ClubSelectionRuntime*)GOLF_SHOT_PARAMETERS_BYTES)->shotCommandKind == 4) {
            spinAccessoryBonus = 0.5f * ((float (*)(int, int, int))CSharedDoc_ComputeAuxPartModifier)(
                                            (int)SharedDocBytes(), AsCGolfDoc(GolfDocBytes())->currentPlayer, 0);
            float v = (float)((ShotPlayerTotalsView*)GolfPlayer_GetEntryByIndex(
                                  (int)GolfDocBytes(), AsCGolfDoc(GolfDocBytes())->currentPlayer))
                          ->spinStat;
            float t = (spinAccessoryBonus + v) / 170.0f;
            t = (0.0f < t) ? t : 0.0f;
            AsCGolfBall(ballBytes)->m_spin = powerFraction * (powerFraction * (1.5f + t));
        } else {
            AsCGolfBall(ballBytes)->m_spin = 0.0f;
        }
        double curveSetting = AsCGolfBall(ballBytes)->m_initCurve;
        AsCGolfBall(ballBytes)->m_curve = (float)curveSetting * powerFraction;
    }
    gtl_Stopwatch_Initialize(simulationStopwatch);
    gtl_Stopwatch_Start(simulationStopwatch);
    gtl_Stopwatch_Reset(simulationStopwatch);
    // Writes: new samples, event indices and integrated ball state.
    // Phase 8: record each fixed 1/30-second step until stopped. Each iteration
    // writes XYZ/velocity/rotation/state into the new sample, then updates
    // special-shot milestones. At 1800 samples (60 s), or a stalled/invalid
    // shot after 900 (30 s), force the existing ground reset rather than loop forever.
    while (AsCGolfBall(ballBytes)->m_state != 0) {
        unsigned char* obj2 = (unsigned char*)((CGolfBall*)ballBytes)->NewPreData();
        ((ShotSample*)obj2)->eventSoundName[0] = 0;
        ((ShotSample*)obj2)->eventKind = 0;
        ((ShotSample*)obj2)->eventSubkind = 0;
        ((CQuadTree*)CQuadTree_GetInstance())->UpdateAnimatedObjects(0.03333333507180214f, 1);
        ((CQuadTree*)CQuadTree_GetInstance())->StepBallSimulation(0.03333333507180214f);
        if (((ClubSelectionRuntime*)GOLF_SHOT_PARAMETERS_BYTES)->shotCommandKind == 4) {
            int m = AsCGolfBall(ballBytes)->m_flags;
            if (m & kShotCobra) {
                if (AsCGolfBall(ballBytes)->m_eventPreDataIndex[kBallCobraDistanceSample] < 0) {
                    collisionRadiusWorldUnits = AsCGolfBall(ballBytes)->m_radius;
                    *(D3*)(waterTestPosition) = *(D3*)&AsCGolfBall(ballBytes)->m_pos;
                    float r = ((CQuadTree*)CQuadTree_GetInstance())
                                  ->ResolveGroundHeight((Vec3f*)waterTestPosition, 2.0f * collisionRadiusWorldUnits);
                    float c = AsCGolfBall(ballBytes)->m_pos.y;
                    r = (c < r) ? r : c;
                    AsCGolfBall(ballBytes)->m_pos.y = r;
                } else {
                    AsCGolfBall(ballBytes)->m_spin = AsCGolfBall(ballBytes)->m_spin - 0.03333333507180214f;
                }
            } else if (m & kShotSpike) {
                if (AsCGolfBall(ballBytes)->m_eventPreDataIndex[kBallSpikeMotionSample] < 0) {
                    collisionRadiusWorldUnits = AsCGolfBall(ballBytes)->m_radius;
                    *(D3*)(outsideTestPosition) = *(D3*)&AsCGolfBall(ballBytes)->m_pos;
                    float r = ((CQuadTree*)CQuadTree_GetInstance())
                                  ->ResolveGroundHeight((Vec3f*)outsideTestPosition, 2.0f * collisionRadiusWorldUnits);
                    float c = AsCGolfBall(ballBytes)->m_pos.y;
                    r = (c < r) ? r : c;
                    AsCGolfBall(ballBytes)->m_pos.y = r;
                } else {
                    AsCGolfBall(ballBytes)->m_spin = AsCGolfBall(ballBytes)->m_spin - 0.03333333507180214f;
                }
            } else {
                if (AsCGolfBall(ballBytes)->m_eventPreDataIndex[1] < 0 &&
                    ((CQuadTree*)CQuadTree_GetInstance())->GetObjColNum() == 0) {
                    AsCGolfBall(ballBytes)->m_spin =
                        FP_FNMSUBS(0.03333333507180214f,
                                   FP_FNMSUBS(0.10000000149011612f, AsCGolfBall(ballBytes)->m_initSpin, 0.5f),
                                   AsCGolfBall(ballBytes)->m_spin);
                } else {
                    AsCGolfBall(ballBytes)->m_spin = AsCGolfBall(ballBytes)->m_spin - 0.0033333336468786f;
                }
            }
        }
        {
            int t64 = AsCGolfBall(ballBytes)->m_preDataCount;
            // 0x801611E8: timeout or a stalled shot uses the same ground reset.
            if (t64 >= kShotMaximumSamples ||
                (t64 >= kShotStallCheckSamples &&
                 (((*(WVector*)(ballBytes + kBallVelocityByteOffset))).Magnitude() < 0.10000000149011612f ||
                  ((CQuadTree*)CQuadTree_GetInstance())->GetObjColNum() >= kShotMaximumCollisionEvents ||
                  (int)((CQuadTree*)CQuadTree_GetInstance())->GetMeshIndex(ballBytes, 0) == -1))) {
                Vec3_Zero(ballBytes + kBallVelocityByteOffset);
                *(D3*)(groundQueryPosition) = *(D3*)&AsCGolfBall(ballBytes)->m_pos;
                GetGroundPoint__9CQuadTreeF7WVectorbPib(&groundRecoveryPosition, CQuadTree_GetInstance(),
                                                        groundQueryPosition, 0, 0, 0);
                AsCGolfBall(ballBytes)->m_pos = groundRecoveryPosition;
                float radius = AsCGolfBall(ballBytes)->m_radius;
                AsCGolfBall(ballBytes)->m_pos.y = AsCGolfBall(ballBytes)->m_pos.y + radius;
                AsCGolfBall(ballBytes)->m_state = 0;
            }
        }
        ((ShotSample*)obj2)->m_pos = AsCGolfBall(ballBytes)->m_pos;
        ((ShotSample*)obj2)->velocity = (*(WVector*)(ballBytes + kBallVelocityByteOffset));
        Vec2_Copy(((ShotSample*)obj2)->rotation, ballBytes + 0x2C);
        ((ShotSample*)obj2)->motionState = AsCGolfBall(ballBytes)->m_state;
        if (AsCGolfBall(ballBytes)->m_state == 0)
            AsCGolfBall(ballBytes)->m_flags = AsCGolfBall(ballBytes)->m_flags & ~kShotTomahawk;
        if (AsCGolfBall(ballBytes)->m_state == 1 && AsCGolfBall(ballBytes)->m_velocity.y < 0.0f &&
            AsCGolfBall(ballBytes)->m_eventPreDataIndex[kBallFirstDescendingSample] < 0)
            AsCGolfBall(ballBytes)->m_eventPreDataIndex[kBallFirstDescendingSample] =
                AsCGolfBall(ballBytes)->m_preDataCount;
        if (AsCGolfBall(ballBytes)->m_eventPreDataIndex[kBallTomahawkEndSample] < 0 &&
            (AsCGolfBall(ballBytes)->m_powerFlags & kShotTomahawk) &&
            !(AsCGolfBall(ballBytes)->m_flags & kShotTomahawk))
            AsCGolfBall(ballBytes)->m_eventPreDataIndex[kBallTomahawkEndSample] =
                AsCGolfBall(ballBytes)->m_preDataCount;
        if (AsCGolfBall(ballBytes)->m_flags & kShotCobra) {
            if (AsCGolfBall(ballBytes)->m_preDataCount == 1) {
                AsCGolfBall(ballBytes)->m_velocity.y = 0.0f;
                WVector cobraVelocity = launchSpeed * ((*(WVector*)(ballBytes + kBallVelocityByteOffset))).Normalize();
                (*(WVector*)(ballBytes + kBallVelocityByteOffset)) = cobraVelocity;
            }
            __mi__FRC7WVectorRC7WVector(cobraTravelXZ, ballBytes, PreData__9CGolfBallFi(ballBytes, 0));
            cobraTravelXZ[1] = 0.0f;
            if (AsCGolfBall(ballBytes)->m_eventPreDataIndex[kBallCobraDistanceSample] < 0) {
                cobraTravelLimit = GolfParameter_ResolveFloat(InstanceTable_Get(), 0);
                float rootDistance = Math_SquareRootApprox(powerFraction);
                cobraTravelLimit = 3.200000047683716f * FP_FMSUBS(rootDistance, cobraTravelLimit, 100.0f);
                if ((*(WVector*)cobraTravelXZ).Magnitude() > cobraTravelLimit) {
                    AsCGolfBall(ballBytes)->m_eventPreDataIndex[kBallCobraDistanceSample] =
                        AsCGolfBall(ballBytes)->m_preDataCount;
                    ((*(WVector*)(ballBytes + kBallVelocityByteOffset))).Normalize();
                    switch (((ClubSelectionRuntime*)GOLF_SHOT_PARAMETERS_BYTES)->selectedClub->specialShotVariant) {
                    case 0:
                        (*(WVector*)(ballBytes + kBallVelocityByteOffset)) *= 74.0f;
                        break;
                    case 1:
                        (*(WVector*)(ballBytes + kBallVelocityByteOffset)) *= 76.0f;
                        break;
                    case 2:
                        (*(WVector*)(ballBytes + kBallVelocityByteOffset)) *= 80.0f;
                        break;
                    default:
                        break;
                    }
                    (*(WVector*)(ballBytes + kBallVelocityByteOffset)) *= Math_SquareRootApprox(powerFraction);
                    AsCGolfBall(ballBytes)->m_spin = 2.5f;
                }
            }
        }
        if (AsCGolfBall(ballBytes)->m_eventPreDataIndex[kBallSpikeEndSample] < 0 &&
            (AsCGolfBall(ballBytes)->m_powerFlags & kShotSpike) && !(AsCGolfBall(ballBytes)->m_flags & kShotSpike) &&
            AsCGolfBall(ballBytes)->m_eventPreDataIndex[kBallSpikeDescentSample] > 0) {
            AsCGolfBall(ballBytes)->m_eventPreDataIndex[kBallSpikeEndSample] = AsCGolfBall(ballBytes)->m_preDataCount;
        }
        if (AsCGolfBall(ballBytes)->m_flags & kShotSpike) {
            if (AsCGolfBall(ballBytes)->m_preDataCount == 1) {
                AsCGolfBall(ballBytes)->m_velocity.y = 0.0f;
                WVector spikeVelocity = launchSpeed * ((*(WVector*)(ballBytes + kBallVelocityByteOffset))).Normalize();
                (*(WVector*)(ballBytes + kBallVelocityByteOffset)) = spikeVelocity;
            }
            __mi__FRC7WVectorRC7WVector(spikeTravelXZ, ballBytes, PreData__9CGolfBallFi(ballBytes, 0));
            spikeTravelXZ[1] = 0.0f;
            if (AsCGolfBall(ballBytes)->m_eventPreDataIndex[kBallSpikeMotionSample] < 0 &&
                (*(WVector*)spikeTravelXZ).Magnitude() > 0.0f) {
                AsCGolfBall(ballBytes)->m_eventPreDataIndex[kBallSpikeMotionSample] =
                    AsCGolfBall(ballBytes)->m_preDataCount;
                ((*(WVector*)(ballBytes + kBallVelocityByteOffset))).Normalize();
                (*(WVector*)(ballBytes + kBallVelocityByteOffset)) *= 72.5f;
                (*(WVector*)(ballBytes + kBallVelocityByteOffset)) *= 2.0f * Math_SquareRootApprox(powerFraction);
                AsCGolfBall(ballBytes)->m_spin = 3.0999999046325684f;
            }
            if (AsCGolfBall(ballBytes)->m_eventPreDataIndex[kBallFirstDescendingSample] >= 0 &&
                AsCGolfBall(ballBytes)->m_preDataCount >
                    AsCGolfBall(ballBytes)->m_eventPreDataIndex[kBallFirstDescendingSample] +
                        kSpikeDescentDelaySamples &&
                AsCGolfBall(ballBytes)->m_eventPreDataIndex[kBallSpikeDescentSample] < 0) {
                AsCGolfBall(ballBytes)->m_eventPreDataIndex[kBallSpikeDescentSample] =
                    AsCGolfBall(ballBytes)->m_preDataCount;
                switch (((ClubSelectionRuntime*)GOLF_SHOT_PARAMETERS_BYTES)->selectedClub->specialShotVariant) {
                case 0: {
                    float x =
                        Math_SquareRootApprox(powerFraction) * ShotParameters_GetBaseLaunchSpeed(InstanceTable_Get());
                    normalModeBaseSpeed = x;
                    const float* p = &Minimum54(normalModeBaseSpeed, s_golfRuleShotSpeedCap);
                    AsCGolfBall(ballBytes)->m_velocity.y = FP_FNMSUBS_SUB_FIRST(-8.0f, 21.5f, ((344.0f - *p) / 112.0f));
                    break;
                }
                case 1: {
                    float x =
                        Math_SquareRootApprox(powerFraction) * ShotParameters_GetBaseLaunchSpeed(InstanceTable_Get());
                    modeOneBaseSpeed = x;
                    const float* p = &Minimum54(modeOneBaseSpeed, s_golfRuleMode1SpeedCap306);
                    AsCGolfBall(ballBytes)->m_velocity.y =
                        FP_FNMSUBS_SUB_FIRST(-10.299999237060547f, 20.5f, ((306.0f - *p) / 105.0f));
                    break;
                }
                case 2: {
                    float x =
                        Math_SquareRootApprox(powerFraction) * ShotParameters_GetBaseLaunchSpeed(InstanceTable_Get());
                    modeTwoBaseSpeed = x;
                    const float* p = &Minimum54(modeTwoBaseSpeed, s_golfRuleSimulationMode2SpeedCap273);
                    AsCGolfBall(ballBytes)->m_velocity.y =
                        FP_FNMSUBS_SUB_FIRST(-10.799999237060547f, 20.200000762939453f, ((273.0f - *p) / 100.0f));
                    break;
                }
                default:
                    break;
                }
                (*(WVector*)(ballBytes + kBallVelocityByteOffset)) *= 7.0f * Math_SquareRootApprox(powerFraction);
                AsCGolfBall(ballBytes)->m_spin = AsCGolfBall(ballBytes)->m_initSpin;
            }
        }
    }
    // Writes: clock, player position, checkpoint and shot history.
    // Phase 9: retain simulation results, restore the starting ground category
    // and animation clock, classify the shot and append the player's 0x54-byte
    // history entry. Recovery changes the player's next playable position.
    gtl_Stopwatch_GetElapsedSeconds(simulationStopwatch);
    AsCGolfBall(ballBytes)->m_groundType = saved59;
    AsCGolfBall(ballBytes)->m_flags = AsCGolfBall(ballBytes)->m_powerFlags;
    IActor_DispatchMessage(self, self, golfRuleStrings + kText_npcActor, 4, 0, 0, 0, 0);
    if (((CQuadTree*)CQuadTree_GetInstance())->GetObjColNum() > 0)
        AsCGolfBall(ballBytes)->m_eventPreDataIndex[3] = -1;
    {
        int saved = AsGolfRuleFrame(self)->simulationStartFrame - 1;
        unsigned char* ctl = (unsigned char*)CQuadTree_GetInstance();
        ((ShotWorldClockView*)ctl)->frame = saved;
        ((ShotWorldClockView*)ctl)->fractionalSeconds = 0.0f;
    }
    ((CQuadTree*)CQuadTree_GetInstance())->UpdateAnimatedObjects(0.03333333507180214f, 1);
    AsCGolfBall(ballBytes)->m_curve = AsCGolfBall(ballBytes)->m_initCurve;
    AsCGolfBall(ballBytes)->m_spin = AsCGolfBall(ballBytes)->m_initSpin;
    ((BallShotTimingView*)ballBytes)->simulationSeconds =
        0.03333333507180214f * (float)AsCGolfBall(ballBytes)->m_preDataCount;
    if (AsCGolfBall(ballBytes)->m_recoveryRequired == 0)
        AsCGolfBall(ballBytes)->m_recoveryRequired =
            CourseData_IsPointOutsidePlayableRegion(AsCGolfDoc(GolfDocBytes())->courseData, ballBytes);
    ((CGolfRule*)self)->ClassifyShotPresentation();
    IActor_DispatchMessage(self, self, golfRuleStrings + kText_player, 0xB,
                           (int)PreData__9CGolfBallFi(ballBytes, AsCGolfBall(ballBytes)->m_preDataCount), 0, 0, 0);
    ((RuleShotCheckpointView*)(self + AsCGolfDoc(GolfDocBytes())->currentPlayer * 0x54))->playerPosition =
        *(WVector*)AsGolfDocSnapshot(
             GolfPlayer_GetEntryByIndex((int)GolfDocBytes(), AsCGolfDoc(GolfDocBytes())->currentPlayer))
             ->ballPosition;
    ((RuleShotCheckpointView*)(self + AsCGolfDoc(GolfDocBytes())->currentPlayer * 0x54))->headingRadians =
        headingRadians;
    ((RuleShotCheckpointView*)(self + AsCGolfDoc(GolfDocBytes())->currentPlayer * 0x54))->recoveryRequired =
        AsCGolfBall(ballBytes)->m_recoveryRequired;
    ((RuleShotCheckpointView*)(self + AsCGolfDoc(GolfDocBytes())->currentPlayer * 0x54))->playerByteAE =
        AsGolfDocSnapshot(GolfPlayer_GetEntryByIndex((int)GolfDocBytes(), AsCGolfDoc(GolfDocBytes())->currentPlayer))
            ->unk_AE;
    ((RuleShotCheckpointView*)(self + AsCGolfDoc(GolfDocBytes())->currentPlayer * 0x54))->powerShotMeter =
        AsGolfDocSnapshot(GolfPlayer_GetEntryByIndex((int)GolfDocBytes(), AsCGolfDoc(GolfDocBytes())->currentPlayer))
            ->powerShotMeter;
    ((RuleShotCheckpointView*)(self + AsCGolfDoc(GolfDocBytes())->currentPlayer * 0x54))->pang =
        ((ShotPlayerTotalsView*)GolfPlayer_GetEntryByIndex((int)GolfDocBytes(),
                                                           AsCGolfDoc(GolfDocBytes())->currentPlayer))
            ->pang;
    ((RuleShotCheckpointView*)(self + AsCGolfDoc(GolfDocBytes())->currentPlayer * 0x54))->bonusPang =
        ((ShotPlayerTotalsView*)GolfPlayer_GetEntryByIndex((int)GolfDocBytes(),
                                                           AsCGolfDoc(GolfDocBytes())->currentPlayer))
            ->bonusPang;
    {
        unsigned char b6 = AsCGolfDoc(GolfDocBytes())->currentHoleIndex;
        unsigned char* pl =
            (unsigned char*)GolfPlayer_GetEntryByIndex((int)GolfDocBytes(), AsCGolfDoc(GolfDocBytes())->currentPlayer);
        ((RuleShotCheckpointView*)(self + AsCGolfDoc(GolfDocBytes())->currentPlayer * 0x54))->holePang =
            *(int*)(AsGolfDocSnapshot(pl)->holePangTotals + b6 * 4);
    }
    {
        unsigned char* pl =
            (unsigned char*)GolfPlayer_GetEntryByIndex((int)GolfDocBytes(), AsCGolfDoc(GolfDocBytes())->currentPlayer);
        ((RuleShotCheckpointView*)(self + AsCGolfDoc(GolfDocBytes())->currentPlayer * 0x54))->holeStrokes =
            AsGolfDocSnapshot(pl)->holeStrokeCounts[AsCGolfDoc(GolfDocBytes())->currentHoleIndex];
    }
    {
        unsigned char* pl =
            (unsigned char*)GolfPlayer_GetEntryByIndex((int)GolfDocBytes(), AsCGolfDoc(GolfDocBytes())->currentPlayer);
        ((RuleShotCheckpointView*)(self + AsCGolfDoc(GolfDocBytes())->currentPlayer * 0x54))->holeResult =
            AsGolfDocSnapshot(pl)->holeResultValues[AsCGolfDoc(GolfDocBytes())->currentHoleIndex];
    }
    ((RuleShotCheckpointView*)(self + AsCGolfDoc(GolfDocBytes())->currentPlayer * 0x54))->playerByteBC =
        AsGolfDocSnapshot(GolfPlayer_GetEntryByIndex((int)GolfDocBytes(), AsCGolfDoc(GolfDocBytes())->currentPlayer))
            ->unk_BC;
    ::strcpy(((RuleShotCheckpointView*)(self + AsCGolfDoc(GolfDocBytes())->currentPlayer * 0x54))->clubName,
             ((ClubSelectionRuntime*)GOLF_SHOT_PARAMETERS_BYTES)->selectedClub->clubName);
    ((RuleShotCheckpointView*)(self + AsCGolfDoc(GolfDocBytes())->currentPlayer * 0x54))->partyResult =
        ((ShotPlayerTotalsView*)GolfPlayer_GetEntryByIndex((int)GolfDocBytes(),
                                                           AsCGolfDoc(GolfDocBytes())->currentPlayer))
            ->partyResult;
    {
        unsigned char* lay = (unsigned char*)AsCSharedDoc(SharedDocBytes())->itemInventory;
        unsigned char* pl =
            (unsigned char*)GolfPlayer_GetEntryByIndex((int)GolfDocBytes(), AsCGolfDoc(GolfDocBytes())->currentPlayer);
        // Item tests at hit3 share the following adjustment.
        if ((unsigned)AsGolfDocSnapshot(pl)->itemId == (unsigned)ItemInventory_GetItemIdAt(lay, 7) ||
            (lay = (unsigned char*)AsCSharedDoc(SharedDocBytes())->itemInventory,
             pl = (unsigned char*)GolfPlayer_GetEntryByIndex((int)GolfDocBytes(),
                                                             AsCGolfDoc(GolfDocBytes())->currentPlayer),
             (unsigned)AsGolfDocSnapshot(pl)->itemId == (unsigned)ItemInventory_GetItemIdAt(lay, 0xA))) {
            unsigned char* rec = self + AsCGolfDoc(GolfDocBytes())->currentPlayer * 0x54;
            ((RuleShotCheckpointView*)rec)->powerShotMeter =
                ((RuleShotCheckpointView*)rec)->powerShotMeter - (float)((RuleShotCheckpointView*)rec)->powerMeterCost;
        }
    }
    {
        unsigned char* hdr = self + AsCGolfDoc(GolfDocBytes())->currentPlayer * 0x18 + 0xAC;
        unsigned char* rec = self + AsCGolfDoc(GolfDocBytes())->currentPlayer * 0x54;
        int cap = ((ShotHistoryHeaderView*)hdr)->blockMapCount;
        if (((cap == 0) ? 0 : (cap - 1) * 16) -
                (((ShotHistoryHeaderView*)hdr)->head + ((ShotHistoryHeaderView*)hdr)->count) ==
            0)
            ShotHistory_Resize(hdr, 1);
        {
            RingIt54 it = RingBegin54(hdr + 8);
            unsigned total = ((ShotHistoryHeaderView*)hdr)->count + ((ShotHistoryHeaderView*)hdr)->head;
            it.advance(total >> 4);
            unsigned char* blk = (((ShotHistoryHeaderView*)hdr)->blockMapCount == 0) ? 0 : *it.cur;
            unsigned char* elem = blk + (total & 0xF) * 0x54;
            if (elem) {
                try {
                    *(S54*)elem = *(S54*)&((RuleShotCheckpointView*)rec)->playerPosition;
                } catch (...) {
                    throw;
                }
            }
            ((ShotHistoryHeaderView*)hdr)->count = ((ShotHistoryHeaderView*)hdr)->count + 1;
        }
    }
    if (AsCGolfBall(ballBytes)->m_recoveryRequired != 0) {
        float* src;
        if (AsCGolfBall(ballBytes)->m_lie != 0) {
            unsigned char* ctl = (unsigned char*)CQuadTree_GetInstance();
            *(D3*)(recoveryPosition) = *(D3*)(&((QuadTreeRecoveryPoint*)ctl)->position);
            src = recoveryPosition;
        } else {
            src = (float*)PreData__9CGolfBallFi(ballBytes, 0);
        }
        *(WVector*)AsGolfDocSnapshot(
             GolfPlayer_GetEntryByIndex((int)GolfDocBytes(), AsCGolfDoc(GolfDocBytes())->currentPlayer))
             ->ballPosition = *(WVector*)src;
        AsGolfDocSnapshot(GolfPlayer_GetEntryByIndex((int)GolfDocBytes(), AsCGolfDoc(GolfDocBytes())->currentPlayer))
            ->unk_AE = 0;
    } else {
        if (KeyCheck54(GOLF_BALL_BYTES) == 1 || AsCGolfBall(GOLF_BALL_BYTES)->m_bounceCount != 0) {
            unsigned char* pl = (unsigned char*)GolfPlayer_GetEntryByIndex((int)GolfDocBytes(),
                                                                           AsCGolfDoc(GolfDocBytes())->currentPlayer);
            *(WVector*)AsGolfDocSnapshot(pl)->ballPosition =
                *(WVector*)(PreData__9CGolfBallFi(ballBytes, AsCGolfBall(ballBytes)->m_preDataCount));
            pl = (unsigned char*)GolfPlayer_GetEntryByIndex((int)GolfDocBytes(),
                                                            AsCGolfDoc(GolfDocBytes())->currentPlayer);
            *(D3*)(lastPlayerPosition) = *(D3*)(AsGolfDocSnapshot(pl)->ballPosition);
            if ((unsigned char)GetGroundType__9CQuadTreeF7WVector(CQuadTree_GetInstance(), lastPlayerPosition) < 3) {
                unsigned char* a = (unsigned char*)GolfPlayer_GetEntryByIndex(
                    (int)GolfDocBytes(), AsCGolfDoc(GolfDocBytes())->currentPlayer);
                unsigned char* b = (unsigned char*)GolfPlayer_GetEntryByIndex(
                    (int)GolfDocBytes(), AsCGolfDoc(GolfDocBytes())->currentPlayer);
                *(WVector*)AsGolfDocSnapshot(b)->previousBallPosition = *(WVector*)AsGolfDocSnapshot(a)->ballPosition;
            }
        } else {
            unsigned char* g = GolfDocBytes();
            GolfAim_FindPlayableRecoveryPoint(AsCGolfDoc(g)->aimPath,
                                              PreData__9CGolfBallFi(ballBytes, AsCGolfBall(ballBytes)->m_preDataCount));
            {
                unsigned char* q = (unsigned char*)AsCGolfDoc(GolfDocBytes())->aimPath;
                *(D3*)(&aimRecoveryPosition) = *(D3*)(&((AimRecoveryPoint*)q)->position);
                *(WVector*)AsGolfDocSnapshot(
                     GolfPlayer_GetEntryByIndex((int)GolfDocBytes(), AsCGolfDoc(GolfDocBytes())->currentPlayer))
                     ->ballPosition = aimRecoveryPosition;
            }
            AsGolfDocSnapshot(
                GolfPlayer_GetEntryByIndex((int)GolfDocBytes(), AsCGolfDoc(GolfDocBytes())->currentPlayer))
                ->unk_AE = 0;
        }
    }
    {
        int found = 0;
        if (AsCGolfBall(ballBytes)->m_recoveryRequired == 0) {
            unsigned char* pl = (unsigned char*)GolfPlayer_GetEntryByIndex((int)GolfDocBytes(),
                                                                           AsCGolfDoc(GolfDocBytes())->currentPlayer);
            *(D3*)(waterRecoveryTestPosition) = *(D3*)(AsGolfDocSnapshot(pl)->ballPosition);
            if (QuadTree_IsWaterAtPoint(CQuadTree_GetInstance(), waterRecoveryTestPosition) != 0)
                found = 1;
        }
        if (found) {
            int z = 0;
            AsCGolfBall(ballBytes)->m_recoveryRequired = 1;
            AsCGolfBall(ballBytes)->m_lie = z;
            unsigned char* pl = (unsigned char*)GolfPlayer_GetEntryByIndex((int)GolfDocBytes(),
                                                                           AsCGolfDoc(GolfDocBytes())->currentPlayer);
            *(WVector*)AsGolfDocSnapshot(pl)->ballPosition = *(WVector*)(PreData__9CGolfBallFi(ballBytes, 0));
            AsGolfDocSnapshot(
                GolfPlayer_GetEntryByIndex((int)GolfDocBytes(), AsCGolfDoc(GolfDocBytes())->currentPlayer))
                ->unk_AE = z;
        }
    }
    {
        unsigned char* g = GolfDocBytes();
        if (AsCGolfDoc(g)->pangyaAssistState == 2) {
            void* o = AsCGolfRule(self)->padSwingController;
            AsCGolfDoc(g)->powerShotBonusPang =
                ((int (*)(void*))(((void**)AsCPadSwingCtrl(o)->interfaceVtable))[13])(o);
        }
    }
    if ((unsigned)(AsCGolfDoc(GolfDocBytes())->tutorialStage - 5) <= 1) {
        IActor_DispatchMessage(self, self, golfRuleStrings + kText_screen, 0x15, 0, 0, 0, 0);
    }
    *(unsigned char*)&AsCGolfRule(self)->forgetDlg = 0;
    AsCGolfRule(self)->forgetDlgTime = 0.0f;
    if (AsCGolfBall(ballBytes)->m_recoveryRequired != 0) {
        if (FindSlot418(self))
            *(unsigned char*)&AsCGolfRule(self)->forgetDlg = 1;
    }
    // Writes: live ball start state, next player and camera state.
    // Phase 10: rewind live ball position/velocity to sample 0 for playback;
    // completed samples/history remain. Then select the next player and camera.
    AsCGolfBall(ballBytes)->m_pos = ((ShotSample*)firstSampleBytes)->m_pos;
    (*(WVector*)(ballBytes + kBallVelocityByteOffset)) = ((ShotSample*)firstSampleBytes)->velocity;
    AsCGolfBall(ballBytes)->m_velocity.y = AsCGolfBall(ballBytes)->m_velocity.y *
                                           (1.0f + ((ShotAccessoryModifiers*)effectStateBytes)->verticalVelocityBonus);
    ((BallGroundTriangleView*)ballBytes)->groundTriangleHandle =
        (int)((CQuadTree*)CQuadTree_GetInstance())->GetMeshIndex(ballBytes, 0);
    __mi__FRC7WVectorRC7WVector(totalTravelXZ, PreData__9CGolfBallFi(ballBytes, AsCGolfBall(ballBytes)->m_preDataCount),
                                firstSampleBytes);
    totalTravelXZ[1] = 0.0f;
    if (GOLF_SELECTED_CLUB_KIND == kClubPutter) {
        if (AsCGolfBall(ballBytes)->m_groundType == 2) {
            unsigned char* pl = (unsigned char*)GolfPlayer_GetEntryByIndex((int)GolfDocBytes(),
                                                                           AsCGolfDoc(GolfDocBytes())->currentPlayer);
            unsigned char* e = pl + AsCGolfDoc(GolfDocBytes())->currentHoleIndex;
            AsGolfDocSnapshot(e)->holeResultValues[0] = AsGolfDocSnapshot(e)->holeResultValues[0] + 1;
        }
        ((ShotAccountStatisticsView*)(SharedDocBytes() + AsCGolfDoc(GolfDocBytes())->currentPlayer * 0x1D4))
            ->putterShots =
            ((ShotAccountStatisticsView*)(SharedDocBytes() + AsCGolfDoc(GolfDocBytes())->currentPlayer * 0x1D4))
                ->putterShots +
            1;
    } else {
        ((ShotAccountStatisticsView*)(SharedDocBytes() + AsCGolfDoc(GolfDocBytes())->currentPlayer * 0x1D4))
            ->nonPutterShots =
            ((ShotAccountStatisticsView*)(SharedDocBytes() + AsCGolfDoc(GolfDocBytes())->currentPlayer * 0x1D4))
                ->nonPutterShots +
            1;
    }
    if (GOLF_SELECTED_CLUB_KIND != kClubPutter)
        ((void (*)(void*))GolfRule_ApplyShotOutcomeModifier)(self);
    AsCGolfRule(self)->nextPlayerIndex = ((CGolfRule*)self)->SelectNextPlayer(0);
    ((RuleUnresolvedShotBytes*)self)->unk_091 = 0;
    AsCGolfRule(self)->shotUndoEnabled = 1;
    AsCGolfRule(self)->puttAnimationEnabled = 0;
    if (s_cameraDirectedShotEnabled == 0) {
        if (!(AsGolfRuleFrame(self)->shotPresentationFlags & 2)) {
            IActor_DispatchMessage(self, self, golfRuleStrings + kText_caddie, 1, 0, 0, 0, 0);
            void* a = PreData__9CGolfBallFi(GolfBall__Fv(), 0);
            unsigned char* b = GOLF_BALL_BYTES;
            // Last sample minus sample 0, including roll; horizontal endpoint
            // distance in world units, not airborne carry or summed path length.
            __mi__FRC7WVectorRC7WVector(finalHorizontalDisplacement,
                                        PreData__9CGolfBallFi(GolfBall__Fv(), AsCGolfBall(b)->m_preDataCount), a);
            finalHorizontalDisplacement[1] = 0.0f;
            float finalHorizontalDistanceWorldUnits = (*(WVector*)finalHorizontalDisplacement).Magnitude();
            if (AsCGolfBall(GOLF_BALL_BYTES)->m_recoveryRequired == 0 && finalHorizontalDistanceWorldUnits > 416.0f) {
                IActor_DispatchMessage(self, self, golfRuleStrings + kText_caddie, 2, 0, 0, 0, 0);
            } else {
                ((CameraCaddieOption*)g_cameraWork)->caddieCameraEnabled = 0;
            }
        }
        ((CCameraWork*)g_cameraWork)->ResetMode(-1);
    }
    return 1;
}

#pragma opt_common_subs reset
#pragma opt_lifetimes reset
namespace ns4 {
extern "C" {
// ===== S13 - Directional commands, gauges and camera input =====
// Gauge screen units, timing and club runtime.
}
}

// Record directional shot commands, including held-direction timing and conflicting inputs.
// Requires live rule/doc/input state and structurally valid shot-command list.
// Only the impact phase in the admitted screen-X window accepts direction commands.
// Writes held-direction milliseconds and appends commands; >100 ms admits repeats.
// No failure return; conflicting directions append the existing conflict code.
void CGolfRule::RecordDirectionalShotInput() {
    using namespace ns4;
    unsigned char* sharedDocBytes = (unsigned char*)g_sharedDoc;
    unsigned char* golfDocBytes = (*(unsigned char**)&AsCSharedDoc(sharedDocBytes)->golfDoc);
    if (AsCGolfDoc(golfDocBytes)->shotPhase < 2)
        return;
    if (AsCGolfDoc(golfDocBytes)->shotGaugeCursorX > kFloat_354_2_GolfRule)
        return;
    if (AsCGolfDoc(golfDocBytes)->shotPowerGaugeX < kFloat_437_GolfRule)
        return;
    // 0x80163A60: Reject a pending operation before polling action 0x63.
    do {
        if (AsCGolfDoc(golfDocBytes)->shotCommandCount) {
            if ((*(unsigned int*)((*(unsigned char**)&AsCGolfDoc(golfDocBytes)->shotCommandHead) + 8) &
                 kSpecialShotMask) == 0)
                break;
            return;
        }
    } while (0);

    unsigned int directionMask = 0;
    int heldDirectionCount = 0;
    if (!GetInputManagerSlot()) {
        if (!s_inputManagerInitialized) {
            InputManager_Init(s_gameInputManager);
            __register_global_object(s_gameInputManager, (void*)InputManager_Dtor, s_golfRuleInputDestructorRecord);
            s_inputManagerInitialized = 1;
        }
        GetInputManagerSlot() = s_gameInputManager;
    }
    void* inputManager = GetInputManagerSlot();
    if (Input_IsActionButtonDown(inputManager, 0, -1)) {
        directionMask |= 1;
        ++heldDirectionCount;
    }
    if (Input_IsActionButtonDown(inputManager, 1, -1)) {
        directionMask |= 2;
        ++heldDirectionCount;
    }
    if (Input_IsActionButtonDown(inputManager, 2, -1)) {
        directionMask |= 4;
        ++heldDirectionCount;
    }
    if (Input_IsActionButtonDown(inputManager, 3, -1)) {
        directionMask |= 8;
        ++heldDirectionCount;
    }
    unsigned char* ruleBytes = (unsigned char*)this;
    if (heldDirectionCount == 0) {
        ((RuleGaugeInputTiming*)ruleBytes)->heldDirectionTick = 0;
        return;
    }
    if (heldDirectionCount == 1) {
        if (((RuleGaugeInputTiming*)ruleBytes)->heldDirectionTick &&
            (directionMask &
             *(unsigned int*)((*(unsigned char**)&AsCGolfDoc((*(unsigned char**)&AsCSharedDoc(g_sharedDoc)->golfDoc))
                                    ->shotCommandHead) +
                              8))) {
            if ((unsigned int)(OS_GetTimeMilliseconds() - ((RuleGaugeInputTiming*)ruleBytes)->heldDirectionTick) >
                100) {
                INSERT63(s_heldDirectionShotCommandMask);
            }
        } else {
            ((RuleGaugeInputTiming*)ruleBytes)->heldDirectionTick = OS_GetTimeMilliseconds();
            INSERT63(directionMask);
        }
    } else {
        INSERT63(s_conflictingShotDirectionsCommand);
    }
}

namespace ns4 {
extern "C" {
struct ButtonGaugeAccessorySpeed {
    char prefix[0x1C];
    float gaugeSpeedMultiplier;
};
struct ButtonGaugePlayerStat {
    char prefix[0xE0];
    int gaugeSpeedReductionStat;
};
struct Replay3D {
    float x, point;
    unsigned char flag;
    char pad[4];
};
}
}

// Button gauge speed relationship table (UI X units/second, then /30 per call).
// Club kind: 0 wood, 1 iron, 2 wedge, 3 putter (Club table). Ground IDs are the
// live CGolfBall::m_groundType returned by terrain lookup; their exact material
// labels are deliberately not inferred from these speed penalties.
//                         wood   iron   wedge  putter/other
// ground 3 or 12          *1.5   *1.3   *1.2   unchanged
// ground 7                *1.3   *1.2   unchanged
// ground 4                *1.1   *1.1   *1.1   unchanged
// all other grounds       unchanged (retains extra GolfBall__Fv call)
// Editing a row: change that ground branch's per-club multiplier(s) below; this
// table is a source index, not a second runtime table that could drift from code.
// Ordered pipeline, preserving float precision and each multiplication:
// 1 base = -(10*player.gaugeSpeedReductionStat - club.baseGaugeSpeed).
// 2 party mode 3: optional accessory multiplier if nonzero, THEN rule multiplier.
// 3 ground/club row above. 4 returning after power decision: *0.9375 if stopped
//   power latch, otherwise x=(powerX-149)/360: x<.5 -> *.875, else
//   *.5*((x-.5)^2+1.75). 5 item slots 7/8/9/2 -> *.5 ONCE, autoShot -> *.5 again.
// 6 cursor uses speed/kButtonGaugeUpdatesPerSecond. Increasing positive speed
//   makes either cursor direction faster; changing only /30 also changes timing
//   relative to input, animation and assistance. Safe tuning ranges unestablished.
// Shared consumers: kFloat_0_5_GolfRule, kFloat_149_805DEA58 and
// kFloat_360_GolfRule also define gauge geometry/impact fractions throughout
// this unit. Add a dedicated future setting to tune speed without moving geometry.
// value starts as speed. On committed impact it becomes Pangya half-width,
// then optional +/-1.5 curve; that branch returns before cursor motion. In the
// uncommitted return path it becomes the projected rounded cursor X for crossing
// checks, which can return without committing.
// Advance the button gauge, lock power/impact, and validate directional shots.
// Required live rule/doc/player/selected club. pressed is sampled swing action;
// state selects whether it locks power or impact. Writes document gauge/phase,
// ball power flags/curve and deferred commands, plays feedback and may commit a
// shot. This is an update-call API, not delta-seconds integration; no ownership.
// Gauge X is screen units (start 149, span 360); steps use /30.
// Requires live rule/doc/player/club and input/effect recipients. pressed is sampled
// input; the existing update-call ABI and float expressions remain unchanged.
// Writes phase/gauge/commands and may commit the shot. No failure return.
// The caller also passes frameStep; this body does not read it.
void CGolfRule::UpdatePowerAndImpactGauge(int pressed, float frameStep) {
    using namespace ns4;
    int swingPressed = pressed;
    unsigned char* itemInventoryBytes;
    unsigned char* shotParameters = *(unsigned char**)(GAUGE_PARAMETERS_BYTES + 4);
    // Phase 1: derive speed from player stats, lie, party modifiers and equipment.
    float baseGaugeSpeed = ((ButtonGaugeClubSpeed*)shotParameters)->baseGaugeSpeed;
    unsigned char* golfDocBytes = GolfDocBytes();
    unsigned char* playerSnapshot =
        (unsigned char*)GolfPlayer_GetEntryByIndex((int)golfDocBytes, AsCGolfDoc(golfDocBytes)->currentPlayer);
    float value = -(10.0f * (float)((ButtonGaugePlayerStat*)playerSnapshot)->gaugeSpeedReductionStat - baseGaugeSpeed);
    if ((int)AsCSharedDoc(SharedDocBytes())->partyGameMode == 3) {
        float s = ((ButtonGaugeAccessorySpeed*)PartyItemSlots_GetInstance())->gaugeSpeedMultiplier;
        if (s != kFloat_0_GolfRule)
            value *= s;
        value *= ((ButtonGaugeTuningView*)this)->partyMode3GaugeSpeedMultiplier;
    }
    if (AsCGolfBall(GAUGE_BALL_BYTES)->m_groundType == 3 || AsCGolfBall(GAUGE_BALL_BYTES)->m_groundType == 12) {
        switch ((int)GAUGE_CLUB_MODE) {
        case 0:
            value *= 1.5f;
            break;
        case 1:
            value *= 1.3f;
            break;
        case 2:
            value *= 1.2f;
            break;
        }
    } else if (AsCGolfBall(GAUGE_BALL_BYTES)->m_groundType == 7) {
        switch ((int)GAUGE_CLUB_MODE) {
        case 0:
            value *= 1.3f;
            break;
        case 1:
            value *= 1.2f;
            break;
        }
    } else if (AsCGolfBall(GAUGE_BALL_BYTES)->m_groundType == 4) {
        switch ((int)GAUGE_CLUB_MODE) {
        case 0:
            value *= 1.1f;
            break;
        case 1:
            value *= 1.1f;
            break;
        case 2:
            value *= 1.1f;
            break;
        }
    } else
        GolfBall__Fv();
    golfDocBytes = GolfDocBytes();
    if (AsCGolfDoc(golfDocBytes)->shotGaugeDirection != kGaugeAdvancing && 1 < AsCGolfDoc(golfDocBytes)->shotPhase) {
        if (((ButtonGaugePowerLatch*)golfDocBytes)->powerLockedFromStoppedGauge)
            value *= 0.9375f;
        else {
            float x = (AsCGolfDoc(golfDocBytes)->shotPowerGaugeX - kFloat_149_805DEA58) / kFloat_360_GolfRule;
            if (x < kFloat_0_5_GolfRule)
                value *= kFloat_0_875_GolfRule;
            else {
                float d = x - kFloat_0_5_GolfRule;
                value *= kFloat_0_5_GolfRule * (d * d + kFloat_1_75_GolfRule);
            }
        }
    }
    // Item tests refresh the active player before each short-circuit operand.
    // Slots 7/8/9/2 halve speed; slots 9/1 double the Pangya half-width.
    itemInventoryBytes = (unsigned char*)*(unsigned int* volatile*)&g_sharedDoc + 0xA04;
    playerSnapshot =
        (unsigned char*)GolfPlayer_GetEntryByIndex((int)golfDocBytes, AsCGolfDoc(golfDocBytes)->currentPlayer);
    if (*(unsigned int*)&AsGolfDocSnapshot(playerSnapshot)->itemId ==
            (unsigned int)ItemInventory_GetItemIdAt(itemInventoryBytes, kGaugeSlowSlot7) ||
        GaugePlayerHasItem(kGaugeSlowSlot8) || GaugePlayerHasItem(kGaugeSlowAndWideSlot9) ||
        GaugePlayerHasItem(kGaugeSlowSlot2)) {
        value *= 0.5f;
    }
    if (AsCGolfRule(this)->autoShot)
        value *= 0.5f;
    // Phase 2: commit power/impact on the sampled action, with assistance/auto-shot adjustments.
    if (swingPressed) {
        golfDocBytes = GolfDocBytes();
        switch ((int)AsCGolfDoc(golfDocBytes)->shotPhase) {
        case 1:
            AsCGolfDoc(golfDocBytes)->shotPhase++;
            golfDocBytes = GolfDocBytes();
            if (AsCGolfDoc(golfDocBytes)->calipersGaugeX > kButtonGaugeOriginX) {
                AsCGolfDoc(golfDocBytes)->roundedShotGaugeCursorX = AsCGolfDoc(golfDocBytes)->calipersGaugeX;
                AsCGolfDoc(GolfDocBytes())->shotGaugeCursorX = AsCGolfDoc(GolfDocBytes())->calipersGaugeX;
                unsigned char* sharedDocBytes = SharedDocBytes();
                if (near3D(AsCGolfDoc((*(unsigned char**)&AsCSharedDoc(sharedDocBytes)->golfDoc))->calipersGaugeX -
                           kButtonGaugeFullPowerX))
                    AsCGolfDoc(*(unsigned char* volatile*)&AsCSharedDoc(sharedDocBytes)->golfDoc)->shotGaugeDirection =
                        0;
                else
                    AsCGolfDoc(*(unsigned char* volatile*)&AsCSharedDoc(sharedDocBytes)->golfDoc)->shotGaugeDirection =
                        -1;
            }
            AsCGolfDoc(GolfDocBytes())->shotPowerGaugeX = AsCGolfDoc(GolfDocBytes())->roundedShotGaugeCursorX;
            golfDocBytes = GolfDocBytes();
            if (AsCGolfDoc(golfDocBytes)->shotGaugeDirection == kGaugeStopped) {
                AsCGolfDoc(golfDocBytes)->shotGaugeDirection = kGaugeReturning;
                ((int (*)(void*, const char*, int, int, int, int, float, float, float))Sound_PlayByName)(
                    g_soundManager, shotTimingSoundNames[0], 0, 0, 0, 0, 0.5f, 200.0f, 1.0f);
                ((ButtonGaugePowerLatch*)GolfDocBytes())->powerLockedFromStoppedGauge = 1;
                IActor_DispatchMessage(this, this, golfRuleStrings + kText_screen, 10, 0, 0, 0, 0);
            } else {
                ((int (*)(void*, const char*, int, int, int, int, float, float, float))Sound_PlayByName)(
                    g_soundManager, shotTimingSoundNames[2], 0, 0, 0, 0, 0.5f, 200.0f, 1.0f);
            }
            break;
        case 2: {
            unsigned char* itemInventoryBytes;
            if (AsCGolfDoc(golfDocBytes)->shotGaugeDirection != kGaugeReturning)
                break;
            {
                float limit = minimum3D(AsCGolfDoc(golfDocBytes)->shotPowerGaugeX);
                if (!(AsCGolfDoc(golfDocBytes)->shotGaugeCursorX < limit))
                    break;
            }
            if (AsCGolfDoc(golfDocBytes)->powerShotMeterOverride) {
                AsCGolfDoc(golfDocBytes)->shotGaugeCursorX = AsCGolfDoc(golfDocBytes)->pangyaTargetX;
                AsCGolfDoc(GolfDocBytes())->roundedShotGaugeCursorX = AsCGolfDoc(GolfDocBytes())->pangyaTargetX;
            }
            AsCGolfDoc(GolfDocBytes())->shotPhase++;
            AsCGolfDoc(GolfDocBytes())->shotImpactGaugeX = AsCGolfDoc(GolfDocBytes())->roundedShotGaugeCursorX;
            AsCGolfRule(this)->impactGaugeDirection = AsCGolfDoc(GolfDocBytes())->shotGaugeDirection;
            AsCGolfDoc(GolfDocBytes())->shotGaugeDirection = kGaugeStopped;
            CGolfRule::ApplyImpactAssistance(this, 0);
            // From here to grading, value is the impact-zone half-width in UI X units.
            value = 2.0f * (float)s_pangyaZoneBaseWidthUnits;
            if (GaugePlayerHasItem(kGaugeSlowAndWideSlot9) || GaugePlayerHasItem(kGaugeWideSlot1)) {
                value *= 2.0f;
            }
            {
                float d = magnitude3D(AsCGolfDoc(GolfDocBytes())->shotImpactGaugeX -
                                      AsCGolfDoc(GolfDocBytes())->pangyaTargetX);
                int result;

                if (d < 0.5f + value)
                    result = 4;
                else {
                    float impactDistance =
                        magnitude3D(AsCGolfDoc(GolfDocBytes())->shotImpactGaugeX - kButtonGaugeOriginX);
                    if (impactDistance < 0.5f + ShotParameters_GetGoodImpactHalfWidth(InstanceTable_Get()))
                        result = 3;
                    else {
                        if (impactDistance < 0.5f + ShotParameters_GetPoorImpactHalfWidth(InstanceTable_Get()))
                            result = 2;
                        else
                            result = 1;
                    }
                }
                AsCGolfDoc(GolfDocBytes())->impactGrade = result;
            }
            golfDocBytes = GolfDocBytes();
            if (AsCGolfDoc(golfDocBytes)->impactGrade == kImpactPangya) {
                int code = combo3D(golfDocBytes);
                switch (code) {
                case 0x11:
                    AsCGolfBall(GAUGE_BALL_BYTES)->m_powerFlags = 1;
                    break;
                case 0x12:
                    AsCGolfBall(GAUGE_BALL_BYTES)->m_powerFlags = 2;
                    break;
                case 0x14:
                    AsCGolfBall(GAUGE_BALL_BYTES)->m_powerFlags = 4;
                    break;
                case 0x18:
                    AsCGolfBall(GAUGE_BALL_BYTES)->m_powerFlags = 8;
                    break;
                case 0x102:
                    AsCGolfBall(GAUGE_BALL_BYTES)->m_powerFlags = 0x10;
                    break;
                case 0x801:
                    AsCGolfBall(GAUGE_BALL_BYTES)->m_powerFlags = 0x20;
                    break;
                case 0x10211:
                    AsCGolfBall(GAUGE_BALL_BYTES)->m_powerFlags = 0x11;
                    break;
                case 0x10212:
                    AsCGolfBall(GAUGE_BALL_BYTES)->m_powerFlags = 0x12;
                    break;
                case 0x10214:
                    AsCGolfBall(GAUGE_BALL_BYTES)->m_powerFlags = 0x14;
                    break;
                case 0x10218:
                    AsCGolfBall(GAUGE_BALL_BYTES)->m_powerFlags = 0x18;
                    break;
                case 0x80111:
                    AsCGolfBall(GAUGE_BALL_BYTES)->m_powerFlags = 0x21;
                    break;
                case 0x80112:
                    AsCGolfBall(GAUGE_BALL_BYTES)->m_powerFlags = 0x22;
                    break;
                case 0x80114:
                    AsCGolfBall(GAUGE_BALL_BYTES)->m_powerFlags = 0x24;
                    break;
                case 0x80118:
                    AsCGolfBall(GAUGE_BALL_BYTES)->m_powerFlags = 0x28;
                    break;
                case 0x802:
                    AsCGolfBall(GAUGE_BALL_BYTES)->m_powerFlags = 0x40;
                    break;
                case 0x80211:
                    AsCGolfBall(GAUGE_BALL_BYTES)->m_powerFlags = 0x41;
                    break;
                case 0x80212:
                    AsCGolfBall(GAUGE_BALL_BYTES)->m_powerFlags = 0x42;
                    break;
                case 0x80214:
                    AsCGolfBall(GAUGE_BALL_BYTES)->m_powerFlags = 0x44;
                    break;
                case 0x80218:
                    AsCGolfBall(GAUGE_BALL_BYTES)->m_powerFlags = 0x48;
                    break;
                }
                if ((int)AsCSharedDoc(SharedDocBytes())->partyGameMode == 1)
                    AsCGolfBall(GAUGE_BALL_BYTES)->m_powerFlags = 0;
                if (AsCGolfBall(GAUGE_BALL_BYTES)->m_powerFlags) {
                    if (AsCGolfBall(GAUGE_BALL_BYTES)->m_powerFlags & kSpecialShotMask) {
                        if (((ClubSelectionRuntime*)GAUGE_PARAMETERS_BYTES)->powerShotMode == 0 ||
                            ((AsCGolfBall(GAUGE_BALL_BYTES)->m_powerFlags & 0x20) && GAUGE_CLUB_MODE != 0))
                            AsCGolfBall(GAUGE_BALL_BYTES)->m_powerFlags = 0;
                        if (((ClubSelectionRuntime*)GAUGE_PARAMETERS_BYTES)->powerShotMode == 0 ||
                            ((AsCGolfBall(GAUGE_BALL_BYTES)->m_powerFlags & 0x40) && GAUGE_CLUB_MODE != 0))
                            AsCGolfBall(GAUGE_BALL_BYTES)->m_powerFlags = 0;
                    }
                    float x, y;
                    IActor_DispatchMessage(this, this, golfRuleStrings + kText_spin, 2, (int)&x, (int)&y, 0, 0);
                    if (AsCGolfBall(GAUGE_BALL_BYTES)->m_powerFlags & 1) {
                        if (y > 0.0f || nearref3D(y))
                            AsCGolfBall(GAUGE_BALL_BYTES)->m_powerFlags = 0;
                    } else if (AsCGolfBall(GAUGE_BALL_BYTES)->m_powerFlags & 2) {
                        if (y < 0.0f || nearref3D(y))
                            AsCGolfBall(GAUGE_BALL_BYTES)->m_powerFlags = 0;
                    } else if (AsCGolfBall(GAUGE_BALL_BYTES)->m_powerFlags & 4) {
                        if (!near3D(x - s_golfRulePowerFlag4DirectionX))
                            AsCGolfBall(GAUGE_BALL_BYTES)->m_powerFlags = 0;
                    } else if (AsCGolfBall(GAUGE_BALL_BYTES)->m_powerFlags & 8) {
                        if (!near3D(x - s_golfRuleFloat_1))
                            AsCGolfBall(GAUGE_BALL_BYTES)->m_powerFlags = 0;
                    }
                    if (AsCGolfBall(GAUGE_BALL_BYTES)->m_powerFlags & 4) {
                        value = -1.5f;
                        AsCGolfBall(GAUGE_BALL_BYTES)->m_initCurve = value;
                    } else if (AsCGolfBall(GAUGE_BALL_BYTES)->m_powerFlags & 8) {
                        value = 1.5f;
                        AsCGolfBall(GAUGE_BALL_BYTES)->m_initCurve = value;
                    }
                }
                AsCGolfBall(GAUGE_BALL_BYTES)->m_powerFlags |= AsCGolfRule(this)->deferredInputButtons;
                AsCGolfRule(this)->deferredInputButtons = 0;
                ((int (*)(void*, const char*, int, int, int, int, float, float, float))Sound_PlayByName)(
                    g_soundManager, shotTimingSoundNames[1], 0, 0, 0, 0, 0.5f, 200.0f, 1.0f);
            } else if (AsCGolfDoc(golfDocBytes)->impactGrade == kImpactGood) {
                ((int (*)(void*, const char*, int, int, int, int, float, float, float))Sound_PlayByName)(
                    g_soundManager, shotTimingSoundNames[2], 0, 0, 0, 0, 0.5f, 200.0f, 1.0f);
            } else if (AsCGolfDoc(golfDocBytes)->impactGrade == kImpactPoor) {
                ((int (*)(void*, const char*, int, int, int, int, float, float, float))Sound_PlayByName)(
                    g_soundManager, shotTimingSoundNames[3], 0, 0, 0, 0, 0.5f, 200.0f, 1.0f);
            } else {
                ((int (*)(void*, const char*, int, int, int, int, float, float, float))Sound_PlayByName)(
                    g_soundManager, shotTimingSoundNames[4], 0, 0, 0, 0, 0.5f, 200.0f, 1.0f);
            }
            AsCGolfBall(GAUGE_BALL_BYTES)->m_powerFlags |= AsCGolfRule(this)->deferredInputButtons;
            AsCGolfRule(this)->deferredInputButtons = 0;
            ((CGolfRule*)this)->CommitShotGauge();
            return;
        }
        }
    }
    golfDocBytes = GolfDocBytes();
    // Phase 3: advance, return or stop the cursor and apply boundary transitions.
    switch (AsCGolfDoc(golfDocBytes)->shotGaugeDirection) {
    case -1:
        AsCGolfDoc(golfDocBytes)->shotGaugeCursorX -= value / kButtonGaugeUpdatesPerSecond;
        if (AsCGolfRule(this)->autoShot) {
            golfDocBytes = GolfDocBytes();
            if (AsCGolfDoc(golfDocBytes)->shotGaugeCursorX < ((ButtonGaugeTuningView*)this)->autoImpactStopX &&
                AsCGolfDoc(golfDocBytes)->shotPhase == kShotSelectImpact)
                AsCGolfDoc(golfDocBytes)->shotGaugeCursorX = ((ButtonGaugeTuningView*)this)->autoImpactStopX - 1.0f;
        }
        golfDocBytes = GolfDocBytes();
        if (AsCGolfDoc(golfDocBytes)->shotGaugeCursorX < kButtonGaugeReturnLimitX) {
            ((CFx*)g_fx)->SetUpdateEnabled(1);
            if (GolfTask_NoOpStatus()) {
                volatile Replay3D replay;
                replay.flag = 0;
                replay.point = AsCGolfDoc(GolfDocBytes())->pangyaTargetX;
            }
            *(unsigned char*)&AsCGolfRule(this)->unk_3F0 = 0;
            AsCGolfDoc(GolfDocBytes())->shotPhase = kShotIdle;
            float reset = GAUGE_CLUB_MODE == 3 ? kButtonGaugeOriginX : kButtonGaugeReturnLimitX;
            AsCGolfDoc(GolfDocBytes())->shotGaugeCursorX = reset;
            AsCGolfDoc(GolfDocBytes())->roundedShotGaugeCursorX = AsCGolfDoc(GolfDocBytes())->shotGaugeCursorX;
            AsCGolfDoc(GolfDocBytes())->shotPowerGaugeX = kButtonGaugeOriginX;
            AsCGolfDoc(GolfDocBytes())->shotGaugeDirection = kGaugeStopped;
            ((ButtonGaugePowerLatch*)GolfDocBytes())->powerLockedFromStoppedGauge = 0;
            IActor_DispatchMessage(this, this, golfRuleStrings + kText_camera, 4, 0, 0, 0, 0);
            AsCGolfDoc(GolfDocBytes())->gameStateFlags = 8;
        }
        if (AsCGolfDoc(GolfDocBytes())->shotPhase == kShotSelectImpact) {
            unsigned char* itemInventoryBytes;
            float pangyaHalfWidth = 2.0f * (float)s_pangyaZoneBaseWidthUnits;
            if (GaugePlayerHasItem(kGaugeSlowAndWideSlot9) || GaugePlayerHasItem(kGaugeWideSlot1)) {
                pangyaHalfWidth *= 2.0f;
            }
            float rounded;
            if (AsCGolfRule(this)->impactGaugeDirection == 1) {
                value =
                    (float)floor(AsCGolfDoc(GolfDocBytes())->shotGaugeCursorX - value / kButtonGaugeUpdatesPerSecond);
                rounded = (float)floor(AsCGolfDoc(GolfDocBytes())->shotGaugeCursorX);
            } else {
                value =
                    (float)ceil(AsCGolfDoc(GolfDocBytes())->shotGaugeCursorX - value / kButtonGaugeUpdatesPerSecond);
                rounded = (float)ceil(AsCGolfDoc(GolfDocBytes())->shotGaugeCursorX);
            }
            golfDocBytes = GolfDocBytes();
            float d = magnitude3D(rounded - AsCGolfDoc(golfDocBytes)->pangyaTargetX);
            if (d < 0.5f + pangyaHalfWidth)
                return;
            if (!(rounded > AsCGolfDoc(golfDocBytes)->pangyaTargetX))
                return;
            d = magnitude3D(value - AsCGolfDoc(golfDocBytes)->pangyaTargetX);
            if (d < 0.5f + pangyaHalfWidth)
                return;
            if (!(value < AsCGolfDoc(golfDocBytes)->pangyaTargetX))
                return;
            AsCGolfDoc(golfDocBytes)->shotGaugeCursorX = AsCGolfDoc(golfDocBytes)->pangyaTargetX;
        }
        break;
    case 1:
        AsCGolfDoc(golfDocBytes)->shotGaugeCursorX += value / kButtonGaugeUpdatesPerSecond;
        if (AsCGolfRule(this)->autoShot) {
            golfDocBytes = GolfDocBytes();
            if (AsCGolfDoc(golfDocBytes)->shotGaugeCursorX > ((ButtonGaugeTuningView*)this)->autoPowerStopX &&
                AsCGolfDoc(golfDocBytes)->shotPhase == kShotSelectPower)
                AsCGolfDoc(golfDocBytes)->shotGaugeCursorX = 1.0f + ((ButtonGaugeTuningView*)this)->autoPowerStopX;
        }
        golfDocBytes = GolfDocBytes();
        if (AsCGolfDoc(golfDocBytes)->shotGaugeCursorX > kButtonGaugeFullPowerX) {
            AsCGolfDoc(golfDocBytes)->shotGaugeCursorX = kButtonGaugeFullPowerX;
            AsGolfRuleFrame(this)->stateTimer = 0.0f;
            AsCGolfDoc(GolfDocBytes())->shotGaugeDirection = kGaugeStopped;
        }
        break;
    case 0:
        if (AsCGolfDoc(golfDocBytes)->shotPhase) {
            float t = AsGolfRuleFrame(this)->stateTimer + kButtonGaugeStepSeconds;
            AsGolfRuleFrame(this)->stateTimer = t;
            if (t > kButtonGaugeReturnPauseSeconds)
                AsCGolfDoc(GolfDocBytes())->shotGaugeDirection = kGaugeReturning;
        }
        break;
    }
}

namespace ns4 {
extern "C" {
struct Save164 {
    int pad;
    float value;
    unsigned char flag;
    int rest[3];
};
}
}

// Update the single-stage gauge mode and commit power or reset an expired swing.
// Requires live rule/doc/selected club and sampled swingPressed input.
// Advances cursor in screen X units per update (/30, bar start 149 and span 360),
// commits power with Pangya impact or resets expired input and its return timer.
// Actor/sound feedback is ordered as below; no failure return.
// The caller also passes elapsedSeconds; this body does not read it.
void CGolfRule::UpdateSingleStageGauge(int swingPressed, float elapsedSeconds) {
    using namespace ns4;
    unsigned char* ruleBytes = (unsigned char*)this;
    float gaugeRate;
    unsigned char* shotParameterTable = (unsigned char*)InstanceTable_Get();
    unsigned char* shotParameters = *(unsigned char**)(shotParameterTable + 4);
    unsigned char* golfDocBytes = GolfDocBytes();
    float base, baseGaugeSpeed, target;
    baseGaugeSpeed = ((ButtonGaugeClubSpeed*)shotParameters)->baseGaugeSpeed;
    base = kFloat_149_805DEA58;
    float normalizedPower = (AsCGolfDoc(golfDocBytes)->shotGaugeCursorX - base) / kFloat_360_GolfRule;
    gaugeRate = baseGaugeSpeed * (kFloat_0_5_GolfRule * (normalizedPower * normalizedPower + kFloat_1_805DEA38));
    if (swingPressed != 0) {
        switch ((int)AsCGolfDoc(golfDocBytes)->shotPhase) {
        case 1: {
            float calipersGaugeX = AsCGolfDoc(golfDocBytes)->calipersGaugeX;
            if (calipersGaugeX > base) {
                AsCGolfDoc(golfDocBytes)->roundedShotGaugeCursorX = calipersGaugeX;
                AsCGolfDoc(GolfDocBytes())->shotGaugeCursorX = AsCGolfDoc(GolfDocBytes())->calipersGaugeX;
                target = kFloat_509_GolfRule;
                if (near164(AsCGolfDoc(GolfDocBytes())->calipersGaugeX, target))
                    AsCGolfDoc(GolfDocBytes())->shotGaugeDirection = kGaugeStopped;
            }
            if (AsCGolfDoc(GolfDocBytes())->shotGaugeDirection == kGaugeStopped)
                ((int (*)(void*, const char*, int, int, int, int, float, float, float))Sound_PlayByName)(
                    g_soundManager, shotTimingSoundNames[0], 0, 0, 0, 0, kFloat_0_5_GolfRule, kFloat_200_GolfRule,
                    kFloat_1_805DEA38);
            else
                ((int (*)(void*, const char*, int, int, int, int, float, float, float))Sound_PlayByName)(
                    g_soundManager, shotTimingSoundNames[3], 0, 0, 0, 0, kFloat_0_5_GolfRule, kFloat_200_GolfRule,
                    kFloat_1_805DEA38);
            AsCGolfDoc(GolfDocBytes())->shotPhase = AsCGolfDoc(GolfDocBytes())->shotPhase + 1;
            AsCGolfDoc(GolfDocBytes())->shotPowerGaugeX = AsCGolfDoc(GolfDocBytes())->roundedShotGaugeCursorX;
            AsCGolfDoc(GolfDocBytes())->shotGaugeDirection = kGaugeStopped;
            AsCGolfDoc(GolfDocBytes())->impactGrade = kImpactPangya;
            ((CGolfRule*)this)->CommitShotGauge();
            return;
        }
        }
    }
    int gaugeDirection = AsCGolfDoc(golfDocBytes)->shotGaugeDirection;
    if (gaugeDirection != 0) {
        if (gaugeDirection < 0)
            return;
        if (gaugeDirection >= 2)
            return;
        AsCGolfDoc(golfDocBytes)->shotGaugeCursorX =
            AsCGolfDoc(golfDocBytes)->shotGaugeCursorX + gaugeRate / kFloat_30_GolfRule;
        if (AsCGolfRule(ruleBytes)->autoShot != 0) {
            float limit = ((ButtonGaugeTuningView*)ruleBytes)->autoPowerStopX;
            if (AsCGolfDoc(GolfDocBytes())->shotGaugeCursorX > limit &&
                AsCGolfDoc(GolfDocBytes())->shotPhase == kShotSelectPower)
                AsCGolfDoc(GolfDocBytes())->shotGaugeCursorX = kFloat_1_805DEA38 + limit;
        }
        if (AsCGolfDoc(GolfDocBytes())->shotGaugeCursorX > kFloat_509_ShotGauge) {
            AsCGolfDoc(GolfDocBytes())->shotGaugeCursorX = kFloat_509_ShotGauge;
            AsCGolfDoc(GolfDocBytes())->shotGaugeDirection = kGaugeStopped;
            AsGolfRuleFrame(ruleBytes)->stateTimer = kFloat_0_GolfRule;
        }
    } else {
        if (AsCGolfDoc(golfDocBytes)->shotPhase != kShotIdle) {
            float thr = kFloat_0_03_GolfRule;
            float f = AsGolfRuleFrame(ruleBytes)->stateTimer + kGolfSimulationStepSeconds;
            AsGolfRuleFrame(ruleBytes)->stateTimer = f;
            if (f > thr) {
                ((CFx*)g_fx)->SetUpdateEnabled(1);
                if (GolfTask_NoOpStatus() != 0) {
                    volatile Save164 save;
                    save.flag = 0;
                    save.value = AsCGolfDoc(GolfDocBytes())->pangyaTargetX;
                }
                *(unsigned char*)&AsCGolfRule(ruleBytes)->unk_3F0 = 0;
                AsCGolfDoc(GolfDocBytes())->shotPhase = kShotIdle;
                float val, cur, init;
                val = (((ClubSelectionRuntime*)InstanceTable_Get())->selectedClub->clubKind == 3) ? kFloat_149_805DEA58
                                                                                                  : kFloat_114_GolfRule;
                init = kFloat_149_805DEA58;
                AsCGolfDoc(GolfDocBytes())->shotGaugeCursorX = val;
                cur = AsCGolfDoc(GolfDocBytes())->shotGaugeCursorX;
                AsCGolfDoc(GolfDocBytes())->roundedShotGaugeCursorX = cur;
                AsCGolfDoc(GolfDocBytes())->shotPowerGaugeX = init;
                AsCGolfDoc(GolfDocBytes())->shotGaugeDirection = kGaugeStopped;
                ((GolfDocShotBytes*)GolfDocBytes())->unk_2D8 = 0;
                ((void (*)(void*, void*, const char*, int, int, int, int, int, float))IActor_DispatchMessage)(
                    this, this, golfRuleStrings + kText_camera, 4, 0, 0, 0, 0, cur);
                AsCGolfDoc(GolfDocBytes())->gameStateFlags = 8;
            }
        }
    }
}

// Wait for screen replies before completing or cancelling a camera-mode transition.
// Requires rule/documents/screen. Advance/dismiss sets cameraMode 0 and enters Aim.
// Otherwise synchronous HUD/transition replies gate cameraMode 2 -> 3.
// Unready replies preserve the mode; no error return and no direct camera allocation.
// The caller also passes dt; this body does not read it.
void CGolfRule::UpdateCameraModeTransition(float dt) {
    using namespace ns4;
    if (PollAdvanceOrDismiss()) {
        unsigned char* manager = (unsigned char*)g_sharedDoc;
        unsigned char* control = (*(unsigned char**)&AsCSharedDoc(manager)->golfDoc);
        AsCGolfDoc(control)->cameraMode = 0;
        EnterState(kStateAim);
        DrawScene_NoOpHook(g_sceneManager, 0);
        return;
    }
    char* textBase = golfRuleStrings;
    int mode;
    int result;
    IActor_DispatchMessage(this, this, textBase + kText_screen, 0x30, (int)&mode, 0, 0, 0);
    result = 0;
    IActor_DispatchMessage(this, this, textBase + kText_screen, 0x56, (int)&result, 0, 0, 0);
    // 0x801651E4: Apply the transition only after all message replies are ready.
    do {
        if (mode != 2)
            break;
        if (result != 0)
            break;
        {
            int next = 0;
            IActor_DispatchMessage(this, this, textBase + kText_screen, 0x3D, (int)&next, 0, 0, 0);
            if (!next)
                break;
            {
                unsigned char* manager = (unsigned char*)g_sharedDoc;
                unsigned char* control = (*(unsigned char**)&AsCSharedDoc(manager)->golfDoc);
                if (AsCGolfDoc(control)->cameraMode != 2)
                    break;
                AsCGolfDoc(control)->cameraMode = 3;
            }
        }
    } while (0);
    ;
}

// Poll screen replies and logical input to advance a view or dismiss its notice.
// Requires screen replies; returns 0 outside HUD phase 2. Queries several UI gates
// before accepting advance/dismiss, sending screen message 0x39 on that path.
// When the alternate gate is set, cancel instead clears notices and returns 1.
// Lazily initializes input service; reply words remain stack-owned.
int CGolfRule::PollAdvanceOrDismiss() {
    using namespace ns4;
    int value3A;
    int mode;
    IActor_DispatchMessage(this, this, golfRuleStrings + kText_screen, 0x30, (int)&mode, 0, 0, 0);
    if (mode != 2) {
        return 0;
    }

    int result = 0;
    IActor_DispatchMessage(this, this, golfRuleStrings + kText_screen, 0x56, (int)&result, 0, 0, 0);
    int value42 = 0;
    IActor_DispatchMessage(this, this, golfRuleStrings + kText_screen, 0x42, (int)&value42, 0, 0, 0);
    IActor_DispatchMessage(this, this, golfRuleStrings + kText_screen, 0x3A, (int)&value3A, 0, 0, 0);
    int value3B = 0;
    IActor_DispatchMessage(this, this, golfRuleStrings + kText_screen, 0x3B, (int)&value3B, 0, 0, 0);
    int value3E = 0;
    IActor_DispatchMessage(this, this, golfRuleStrings + kText_screen, 0x3E, (int)&value3E, 0, 0, 0);

    if (result == 0) {
        if (value3A != 0 || (value42 == 0 && GolfRule_PollAdvanceAction() && value3B != 0 && value3E == 0)) {
            IActor_DispatchMessage(this, this, golfRuleStrings + kText_screen, 0x39, 0, 0, 0, 0);
            return 1;
        }
    } else {
        if (!GetInputManagerSlot()) {
            if (!s_inputManagerInitialized) {
                InputManager_Init(s_gameInputManager);
                __register_global_object(s_gameInputManager, (void*)InputManager_Dtor,
                                         (void*)s_golfRuleInputDestructorRecord);
                s_inputManagerInitialized = 1;
            }
            GetInputManagerSlot() = s_gameInputManager;
        }
        if (Input_IsActionPressed(GetInputManagerSlot(), kActionCancel, kSelectedInputChannel)) {
            NoticeQueue_Clear(g_helpMsg);
            return 1;
        }
    }
    return 0;
}

// Poll the screen and logical input for a camera-view transition.
// Requires screen/current golf document. Returns 0 outside HUD phase 2.
// Camera mode 0 combines screen admission with action 8; other modes query 0x3D.
// Returns 1 for the admitted toggle. May initialize input and send screen 0x44;
// does not itself enter a new rule state.
int CGolfRule::PollCameraToggle() {
    using namespace ns4;
    int mode;
    IActor_DispatchMessage(this, this, golfRuleStrings + kText_screen, 0x30, (int)&mode, 0, 0, 0);
    if (mode != 2)
        return 0;
    int first = 0;
    int second;
    if (AsCGolfDoc(*(unsigned char**)(((char*)&AsCSharedDoc(g_sharedDoc)->golfDoc)))->cameraMode == 0) {
        second = 0;
        IActor_DispatchMessage(this, this, golfRuleStrings + kText_screen, 0x3B, (int)&second, 0, 0, 0);
        IActor_DispatchMessage(this, this, golfRuleStrings + kText_screen, 0x3C, (int)&first, 0, 0, 0);
        if (GetInputManagerSlot() == 0) {
            if (s_inputManagerInitialized == 0) {
                InputManager_Init(s_gameInputManager);
                __register_global_object(s_gameInputManager, (void*)InputManager_Dtor,
                                         (void*)s_golfRuleInputDestructorRecord);
                s_inputManagerInitialized = 1;
            }
            GetInputManagerSlot() = s_gameInputManager;
        }
        if (Input_IsActionPressed(GetInputManagerSlot(), 8, -1)) {
            first |= 1;
            if (second)
                IActor_DispatchMessage(this, this, golfRuleStrings + kText_screen, 0x44, 0, 0, 0, 0);
        }
        first &= second;
    } else {
        IActor_DispatchMessage(this, this, golfRuleStrings + kText_screen, 0x3D, (int)&first, 0, 0, 0);
    }
    if (first != 0)
        return 1;
    return 0;
}

namespace ns4 {
extern "C" {
// ===== S14 - Shot statistics and turn preparation =====
// Player/document/ball views and course/story continuations.

// Accumulate the completed shot into the active player's account statistics.
// Requires live documents, current player, sample 0 and selected club. No result,
// allocation or ownership transfer. unk_3CC suppresses ALL writes (its wider role
// is unresolved). Only account fields change; repeated document/player lookups
// and their order are retained, including both rule-mode stroke-read sequences.
//
// Stored-statistic map: offsets are from SharedDoc + currentPlayer*0x1D4, the
// biased ShotAccountStatisticsView, not the real player-record start (+0x10).
// H = ~(holeInEncoded ^ holeInKey) == 1, R = ball +58 recoveryRequired,
// B = ball +AC bounceCount != 0, S = player.holeStrokeCounts[currentHole],
// P = holeRecords[currentHole].par, club kind 3 = putter.
// Global admission: unk_3CC == 0. After that, in evaluation order:
// +70 f32 longestHoledOrBouncedShotYards: max(distanceYards), if H || B;
//     this runs BEFORE the R split, so recovery shots can update it too.
// R branch: +6C u32 recoveryShots++; +60 u32 openingShots++ only if S == 2;
//     then exit: no hole-in, putt, terrain or whole-distance updates below.
// !R branch, H only:
//   club 3: +68 u32 madePutts++; +74 f32 longestMadePuttYards = max(distanceYards).
//   other club: +78 f32 longestNonPutterHoleOutYards = max(distanceYards).
//   S == 1: +5C u16 holeInOnes++; skip the albatross test.
//   otherwise S - P == -3: +5E u16 albatrosses++.
// !R branch, independently of H and club:
//   S == 1: +60 u32 openingShots++; also +64 u32 openingShotsOnFairwayOrGreen++
//     if the ground query at the saved END position gives low-byte type 1 or 2.
//   +7C u32 totalNonRecoveryWholeYards += unsigned conversion of rawDistance/3.2.
// "openingShots" includes the S==2 recovery case; it is not a count of all shots.
// It and +64 are gate-based names, not a claim about the save UI's rate labels.
//
// rawDistance is horizontal endpoint displacement: player.ballPosition minus
// sample0.position, Y cleared. It can include roll, not airborne carry or path length.
// distanceYards = powerShotMeterOverride ? 0 : rawDistance/3.2 for the three maxima.
// +7C recomputes rawDistance/3.2, so that override DOES NOT suppress the total.
// Maxima keep the original comparison/NaN behavior; u16/u32 counters wrap at
// their stored widths. Preserve those widths, conversion and the branch order.
void GolfRule_AccumulateCurrentShotStatistics() {
    unsigned char* sharedDocBytes;
    float* ballBytes;
    unsigned char* golfDocBytes;
    void* playerEntry;
    void* firstSample;
    float horizontalDisplacement[3];

    int i8;
    int i12;
    int i16;

    sharedDocBytes = (unsigned char*)g_sharedDoc;
    golfDocBytes = (*(unsigned char**)&AsCSharedDoc(sharedDocBytes)->golfDoc);
    if (((CGolfDoc*)(golfDocBytes))->unk_3CC)
        return;

    ballBytes = GolfBall__Fv();
    sharedDocBytes = (unsigned char*)g_sharedDoc;
    golfDocBytes = (*(unsigned char**)&AsCSharedDoc(sharedDocBytes)->golfDoc);
    playerEntry = GolfPlayer_GetEntryByIndex((int)golfDocBytes, ((CGolfDoc*)(golfDocBytes))->currentPlayer);

    firstSample = PreData__9CGolfBallFi(ballBytes, 0);
    __mi__FRC7WVectorRC7WVector(horizontalDisplacement, AsGolfDocSnapshot(playerEntry)->ballPosition, firstSample);

    float shotDistanceYards;
    shotDistanceYards = kFloat_0_GolfRule;
    horizontalDisplacement[1] = shotDistanceYards;
    sharedDocBytes = (unsigned char*)g_sharedDoc;
    golfDocBytes = (*(unsigned char**)&AsCSharedDoc(sharedDocBytes)->golfDoc);
    shotDistanceYards = (!((CGolfDoc*)(golfDocBytes))->powerShotMeterOverride)
                            ? (*(WVector*)horizontalDisplacement).Magnitude() / kFloat_3_2_GolfRule
                            : shotDistanceYards;

    // Phase 1: hole-in OR bounce admits the distance maximum, even on recovery.
    {
        unsigned int holeInKey = (*(unsigned int*)&AsCGolfBall(ballBytes)->m_holeInCrypt[kHoleInKeyWord]);
        unsigned int holeInEncoded = (*(unsigned int*)&AsCGolfBall(ballBytes)->m_holeInCrypt[kHoleInEncodedWord]);
        unsigned int keyCopy = holeInKey;
        unsigned int holeInXor = keyCopy ^ holeInEncoded;
        int holeInValue = ~(holeInXor);
        if (holeInValue == 1 || AsCGolfBall(ballBytes)->m_bounceCount != 0) {
            sharedDocBytes = (unsigned char*)g_sharedDoc;
            golfDocBytes = (*(unsigned char**)&AsCSharedDoc(sharedDocBytes)->golfDoc);
            unsigned char playerIndex = ((CGolfDoc*)(golfDocBytes))->currentPlayer;
            unsigned char* accountBytes = sharedDocBytes + (unsigned int)playerIndex * 0x1D4;
            float maximumDistance = ((ShotAccountStatisticsView*)accountBytes)->longestHoledOrBouncedShotYards;
            maximumDistance = (shotDistanceYards < maximumDistance) ? maximumDistance : shotDistanceYards;
            ((ShotAccountStatisticsView*)accountBytes)->longestHoledOrBouncedShotYards = maximumDistance;
        }
    }

    // Phase 2: recovery has its own counters and bypasses every !R statistic.
    do {
        if (AsCGolfBall(ballBytes)->m_recoveryRequired != 0) {
            sharedDocBytes = (unsigned char*)g_sharedDoc;
            golfDocBytes = (*(unsigned char**)&AsCSharedDoc(sharedDocBytes)->golfDoc);
            unsigned char playerIndex = ((CGolfDoc*)(golfDocBytes))->currentPlayer;
            unsigned char* accountBytes = sharedDocBytes + (unsigned int)playerIndex * 0x1D4;
            unsigned int* recoveryShots = &((ShotAccountStatisticsView*)accountBytes)->recoveryShots;
            (*recoveryShots)++;
            sharedDocBytes = (unsigned char*)g_sharedDoc;
            golfDocBytes = (*(unsigned char**)&AsCSharedDoc(sharedDocBytes)->golfDoc);
            void* currentPlayer =
                GolfPlayer_GetEntryByIndex((int)golfDocBytes, ((CGolfDoc*)(golfDocBytes))->currentPlayer);
            sharedDocBytes = (unsigned char*)g_sharedDoc;
            golfDocBytes = (*(unsigned char**)&AsCSharedDoc(sharedDocBytes)->golfDoc);
            unsigned char holeIndex = ((CGolfDoc*)(golfDocBytes))->currentHoleIndex;
            unsigned char* playerEntryBytes = (unsigned char*)currentPlayer;
            // GolfDocSnapshot8013::holeStrokeCounts[holeIndex] (+28). Keep
            // index+offset inside []: the member form swaps PowerPC add operands.
            unsigned char holeStrokes = playerEntryBytes[holeIndex + 0x28];
            if (2 != holeStrokes)
                break;
            playerIndex = ((CGolfDoc*)(golfDocBytes))->currentPlayer;
            accountBytes = sharedDocBytes + (unsigned int)playerIndex * 0x1D4;
            unsigned int* openingShots = &((ShotAccountStatisticsView*)accountBytes)->openingShots;
            (*openingShots)++;
            break;
        }

        // Phase 3 (!R): made putt/non-putter hole-out maximum, then HIO/albatross.
        do {
            {
                unsigned int holeInEncoded =
                    (*(unsigned int*)&AsCGolfBall(ballBytes)->m_holeInCrypt[kHoleInEncodedWord]);
                unsigned int holeInKey = (*(unsigned int*)&AsCGolfBall(ballBytes)->m_holeInCrypt[kHoleInKeyWord]);

                int holeInValue = ~((holeInEncoded ^ holeInKey));
                if (1 != holeInValue)
                    break;
            }
            {
                unsigned char* clubRuntime = (unsigned char*)InstanceTable_Get();
                unsigned char* selectedClub = *(unsigned char**)(((clubRuntime + 4)));
                if (3 == ((SelectedClubRuntime*)selectedClub)->clubKind) {
                    sharedDocBytes = (unsigned char*)g_sharedDoc;
                    golfDocBytes = (*(unsigned char**)&AsCSharedDoc(sharedDocBytes)->golfDoc);
                    unsigned char playerIndex = ((CGolfDoc*)(golfDocBytes))->currentPlayer;
                    unsigned char* accountBytes = sharedDocBytes + (unsigned int)playerIndex * 0x1D4;
                    unsigned int* madePutts = &((ShotAccountStatisticsView*)accountBytes)->madePutts;
                    (*madePutts)++;
                    sharedDocBytes = (unsigned char*)g_sharedDoc;
                    golfDocBytes = (*(unsigned char**)&AsCSharedDoc(sharedDocBytes)->golfDoc);
                    playerIndex = ((CGolfDoc*)(golfDocBytes))->currentPlayer;
                    accountBytes = sharedDocBytes + (unsigned int)playerIndex * 0x1D4;
                    float maximumDistance = ((ShotAccountStatisticsView*)accountBytes)->longestMadePuttYards;
                    maximumDistance = (shotDistanceYards < maximumDistance) ? maximumDistance : shotDistanceYards;
                    ((ShotAccountStatisticsView*)accountBytes)->longestMadePuttYards = maximumDistance;
                } else {
                    sharedDocBytes = (unsigned char*)g_sharedDoc;
                    golfDocBytes = (*(unsigned char**)&AsCSharedDoc(sharedDocBytes)->golfDoc);
                    unsigned char playerIndex = ((CGolfDoc*)(golfDocBytes))->currentPlayer;
                    unsigned char* accountBytes = sharedDocBytes + (unsigned int)playerIndex * 0x1D4;
                    float maximumDistance = ((ShotAccountStatisticsView*)accountBytes)->longestNonPutterHoleOutYards;
                    maximumDistance = (shotDistanceYards < maximumDistance) ? maximumDistance : shotDistanceYards;
                    ((ShotAccountStatisticsView*)accountBytes)->longestNonPutterHoleOutYards = maximumDistance;
                }
            }
            sharedDocBytes = (unsigned char*)g_sharedDoc;
            // Both helpers read S; mode 1 refreshes the documents in a different
            // order. Preserve this short-circuit predicate and both call sites.
            if ((((CSharedDoc*)(sharedDocBytes))->golfRuleType != 1 &&
                 GolfRule_ReadPlayerHoleStrokes(sharedDocBytes, golfDocBytes) == 1) ||
                (((CSharedDoc*)(sharedDocBytes))->golfRuleType == 1 &&
                 GolfRule_ReadPlayerHoleStrokesMode1(sharedDocBytes, golfDocBytes) == 1)) {
                unsigned char playerIndex = ((CGolfDoc*)(golfDocBytes))->currentPlayer;
                unsigned char* accountBytes = sharedDocBytes + (unsigned int)playerIndex * 0x1D4;
                unsigned short* holeInOnes = &((ShotAccountStatisticsView*)accountBytes)->holeInOnes;
                (*holeInOnes)++;
                break;
            }
            {
                golfDocBytes = (*(unsigned char**)&AsCSharedDoc(sharedDocBytes)->golfDoc);
                unsigned char playerIndex = ((CGolfDoc*)(golfDocBytes))->currentPlayer;
                void* currentPlayer = GolfPlayer_GetEntryByIndex((int)golfDocBytes, playerIndex);
                sharedDocBytes = (unsigned char*)g_sharedDoc;
                golfDocBytes = (*(unsigned char**)&AsCSharedDoc(sharedDocBytes)->golfDoc);
                unsigned char* playerEntryBytes = (unsigned char*)currentPlayer;
                unsigned char holeIndex = ((CGolfDoc*)(golfDocBytes))->currentHoleIndex;

                unsigned int holeRecordsAddress = *(unsigned int*)&((CGolfDoc*)(golfDocBytes))->holeRecords;
                unsigned int holeRecordOffset = holeIndex << 5;
                unsigned char* holeRecord = (unsigned char*)((holeRecordsAddress + holeRecordOffset));
                // Same +28 holeStrokeCounts field; retain the indexed byte form.
                unsigned char holeStrokes = playerEntryBytes[holeIndex + 0x28];
                unsigned char par = ((ShotHoleRecord*)(holeRecord))->par;
                int strokesRelativeToPar = (int)holeStrokes - (int)par;
                if (strokesRelativeToPar != -3)
                    break;
                playerIndex = ((CGolfDoc*)(golfDocBytes))->currentPlayer;
                unsigned char* accountBytes = sharedDocBytes + (unsigned int)playerIndex * 0x1D4;
                unsigned short* albatrosses = &((ShotAccountStatisticsView*)accountBytes)->albatrosses;
                (*albatrosses)++;
            }
        } while (0);

        // Phase 4 (!R): opening-shot/lie counts, whether holed or not.
        sharedDocBytes = (unsigned char*)g_sharedDoc;
        golfDocBytes = (*(unsigned char**)&AsCSharedDoc(sharedDocBytes)->golfDoc);
        playerEntry = GolfPlayer_GetEntryByIndex((int)golfDocBytes, ((CGolfDoc*)(golfDocBytes))->currentPlayer);
        {
            int endPositionBits[3];

            int positionXBits = *(int*)(AsGolfDocSnapshot(playerEntry)->ballPosition);
            int positionYBits = (*(int*)&AsGolfDocSnapshot(playerEntry)->ballPosition[1]);
            endPositionBits[0] = positionXBits;
            endPositionBits[1] = positionYBits;
            endPositionBits[2] = (*(int*)&AsGolfDocSnapshot(playerEntry)->ballPosition[2]);
            void* world = CQuadTree_GetInstance();
            int groundType = GetGroundType__9CQuadTreeF7WVector(world, endPositionBits);
            sharedDocBytes = (unsigned char*)g_sharedDoc;
            golfDocBytes = (*(unsigned char**)&AsCSharedDoc(sharedDocBytes)->golfDoc);
            void* currentPlayer =
                GolfPlayer_GetEntryByIndex((int)golfDocBytes, ((CGolfDoc*)(golfDocBytes))->currentPlayer);
            sharedDocBytes = (unsigned char*)g_sharedDoc;
            golfDocBytes = (*(unsigned char**)&AsCSharedDoc(sharedDocBytes)->golfDoc);
            unsigned char holeIndex = ((CGolfDoc*)(golfDocBytes))->currentHoleIndex;
            unsigned char* playerEntryBytes = (unsigned char*)currentPlayer;

            // Same +28 holeStrokeCounts field; retain the indexed byte form.
            if ((playerEntryBytes[holeIndex + 0x28]) == 1) {
                unsigned int groundTypeMinusOne = (groundType + 0xFF) & 0xFF;
                if (1u >= groundTypeMinusOne) {
                    unsigned char* accountBytes =
                        sharedDocBytes + (unsigned int)(((CGolfDoc*)(golfDocBytes))->currentPlayer) * 0x1D4;
                    unsigned int* openingShotsOnFairwayOrGreen =
                        &((ShotAccountStatisticsView*)accountBytes)->openingShotsOnFairwayOrGreen;
                    (*openingShotsOnFairwayOrGreen)++;
                }
                sharedDocBytes = (unsigned char*)g_sharedDoc;
                golfDocBytes = (*(unsigned char**)&AsCSharedDoc(sharedDocBytes)->golfDoc);

                unsigned char* accountBytes =
                    sharedDocBytes + (unsigned int)(((CGolfDoc*)(golfDocBytes))->currentPlayer) * 0x1D4;
                unsigned int* openingShots = &((ShotAccountStatisticsView*)accountBytes)->openingShots;
                (*openingShots)++;
            }
        }

        // Phase 5 (!R): add whole endpoint yards, independent of the maxima override.
        {
            unsigned char* sharedDocBytes = (unsigned char*)g_sharedDoc;
            unsigned char* golfDocBytes = (*(unsigned char**)&AsCSharedDoc(sharedDocBytes)->golfDoc);
            float rawDistanceYards = (*(WVector*)horizontalDisplacement).Magnitude();
            rawDistanceYards = rawDistanceYards / kFloat_3_2_GolfRule;
            int wholeDistanceYards = Runtime_ConvertDoubleToUnsigned(rawDistanceYards);
            unsigned char playerIndex = ((CGolfDoc*)(golfDocBytes))->currentPlayer;
            unsigned char* accountBytes = sharedDocBytes + (unsigned int)playerIndex * 0x1D4;
            unsigned int* totalNonRecoveryWholeYards =
                &((ShotAccountStatisticsView*)accountBytes)->totalNonRecoveryWholeYards;
            *totalNonRecoveryWholeYards = *totalNonRecoveryWholeYards + (unsigned int)wholeDistanceYards;
        }
    } while (0);

    return;
}
}
}

// Prepare the next shot or complete the current player/hole after presentation ends.
// Requires live rule/documents/ball; dt drives camera input offsets. Pause/sync gates
// return before shot resets. Tutorial branches may wait for feedback or reset sample 0;
// ordinary turns select player, increment the hole stroke byte and restore ball/club.
// Initializes aim, turn hints, gauge and camera before Aim or the turn-start delay.
// Terrain handles, messages and repeated accessor calls retain their original order.
void CGolfRule::PrepareNextShot(float dt) {
    using namespace ns4;
    unsigned char* o = (unsigned char*)this;
    ((CCameraWork*)g_cameraWork)->UpdateInputOffsets(dt);
    if (((CGolfRule*)(o))->processingPaused || ((CGolfRule*)(o))->networkSyncPending)
        return;
    ((GolfRuleFrameFields*)(o))->stateEntryPending = 0;
    ((RuleUnresolvedShotBytes*)(o))->unk_091 = 0;
    ((RuleUnresolvedShotBytes*)(o))->unk_092 = 0;
    ((RuleUnresolvedShotBytes*)(o))->unk_094 = 0;
    IActor_DispatchMessage(this, this, golfRuleStrings + kText_effect, 11, 0, 0, 0, 0);
    IActor_DispatchMessage(this, this, golfRuleStrings + kText_effect, 12, 0, 0, 0, 0);
    IActor_DispatchMessage(this, this, golfRuleStrings + kText_letter, 0, 0, 0, 0, 0);
    IActor_DispatchMessage(this, this, golfRuleStrings + kText_trajectory, 6, 0, 0, 0, 0);
    ((CFx*)g_fx)->ClearEffects(1);
    Camera_SetFov(g_view, kGolfCameraFovRadians);
    unsigned char* ctx = (unsigned char*)GolfBall__Fv();
    void* actor = ((CGolfBall*)ctx)->GetOrAllocatePreDataAt(0);
    unsigned char* game = B5_GAME;
    ((GolfDocSnapshot8013*)(CurrentGolfPlayerEntryFromDoc(game)))->itemId = 0;
    if (((CGolfDoc*)(B5_GAME))->tutorialStage < 6)
        B5_MSG_THIS(kText_screen, 22, 0);
    ((B5ContextFields*)ctx)->i68 = -1;
    float reset = kFloat_0_GolfRule;
    ((B5ContextFields*)ctx)->i64 = 0;
    ((B5ContextFields*)ctx)->f34 = reset;
    ((B5ContextFields*)ctx)->f38 = reset;
    Vec2_Zero(AsCGolfBall(ctx)->m_rotation);
    game = B5_GAME;
    int mode = ((CGolfDoc*)(game))->tutorialStage;
    if (mode < 6 && mode != 5) {
        if (AsCGolfRule(o)->autoShot) {
            B5_MSG_THIS(kText_tutorial, 0, 0);
        } else if (((CGolfRule*)(o))->tutorialFeedbackEnabled) {
            int value;
            if (((CGolfDoc*)(game))->impactTimingSpriteIndex != 6) {
                void* obj = ((CGolfRule*)(o))->padSwingController;
                int result = ((B5Object*)obj)->check();
                if (result)
                    value = 0;
                else
                    value = 1;
            } else if (((CGolfDoc*)(game))->impactGrade == 4)
                value = 4;
            else if (((CGolfDoc*)(game))->shotImpactGaugeX < ((CGolfDoc*)(game))->pangyaTargetX)
                value = 3;
            else
                value = 2;
            B5_MSG_THIS(kText_tutorial, 4, value);
            B5_MSG_THIS(kText_golfRule, 13, 1);
            return;
        } else if (((CGolfRule*)(o))->tutorialAdvancePending) {
            ((CGolfRule*)(o))->tutorialAdvancePending = 0;
            B5_MSG_THIS(kText_tutorial, 0, 0);
        } else {
            float vec[3];
            __mi__FRC7WVectorRC7WVector(vec, game + 0xAC, ctx);
            vec[1] = kFloat_0_GolfRule;
            float distance = (*(WVector*)vec).Magnitude();
            game = B5_GAME;
            mode = ((CGolfDoc*)(game))->tutorialStage;
            if ((unsigned int)(mode - 2) <= 1) {
                float scale;
                if (mode == 2)
                    scale = kFloat_2_805DEA84;
                else
                    scale = kFloat_1_805DEA38;
                if (distance < kFloat_6_4_GolfRule * scale) {
                    unsigned char* p = CurrentGolfPlayerEntryFromDoc(game);
                    *(int*)&((GolfDocSnapshot8013*)(p))->roundResultWords[1] += 100;
                } else if (distance < kFloat_19_2_GolfRule * scale) {
                    unsigned char* p = CurrentGolfPlayerEntryFromDoc(game);
                    *(int*)&((GolfDocSnapshot8013*)(p))->roundResultWords[1] += 50;
                } else if (distance < kFloat_32_GolfRule * scale) {
                    unsigned char* p = CurrentGolfPlayerEntryFromDoc(game);
                    *(int*)&((GolfDocSnapshot8013*)(p))->roundResultWords[1] += 30;
                } else if (distance < kFloat_44_8_GolfRule * scale) {
                    unsigned char* p = CurrentGolfPlayerEntryFromDoc(game);
                    *(int*)&((GolfDocSnapshot8013*)(p))->roundResultWords[1] += 10;
                }
                if (distance < kFloat_44_8_GolfRule * scale) {
                    B5_MSG_THIS(kText_golfRule, 13, 1);
                    CMouseCursor::SetPointerInputEnabled(g_mouseCursor, 1);
                    B5_MSG_THIS(kText_tutorial, 1, 0);
                    return;
                }
            }
        }
        game = B5_GAME;
        B5PlayerFields* fields = (B5PlayerFields*)CurrentGolfPlayerEntryFromDoc(game);
        fields->flag = 0;
        game = B5_GAME;
        unsigned char* p = CurrentGolfPlayerEntryFromDoc(game);
        *(WVector*)(p + 0x1C) = *(WVector*)(PreData__9CGolfBallFi(ctx, 0));
        *(WVector*)ctx = *(WVector*)(PreData__9CGolfBallFi(ctx, 0));
        *(void**)(ctx + 0x28) = (void*)((CQuadTree*)CQuadTree_GetInstance())->GetMeshIndex(ctx, 0);
        if ((unsigned int)(((CGolfDoc*)(B5_GAME))->tutorialStage - 2) <= 2) {
            B5_MSG_THIS(kText_screen, 1, 0);
            ((CGolfDoc*)(B5_GAME))->headingRadians = B5_F(o, 0x344);
            unsigned char* animation = (unsigned char*)InstanceTable_Get();
            ((ClubSelectionRuntime*)(animation))->powerShotMode = 0;
            float zero;
            float angle = ((CGolfDoc*)(B5_GAME))->headingRadians;
            B5_MSG_THIS(kText_camera, 0x2717, &angle);
            B5_MSG_THIS(kText_spot, 0x2717, &angle);
            B5_MSG_THIS(kText_screen, 0x2717, &angle);
            B5_MSG_THIS(kText_grid, 0x2717, &angle);
            B5_MSG_THIS(kText_caddie, 0x2717, &angle);
            B5_MSG_THIS(kText_player, 6, 0);
            if (((CGolfDoc*)(B5_GAME))->tutorialStage == 2)
                B5_MSG_THIS(kText_caddie, 3, 0);
            zero = kFloat_0_GolfRule;
            B5_MSG_THIS(kText_screen, 9, &zero);
        } else
            InitializeAimHeading();
        ((CMouseCursor*)g_mouseCursor)->Reset(0);
        game = B5_GAME;
        p = CurrentGolfPlayerEntryFromDoc(game);
        (p + ((CGolfDoc*)(B5_GAME))->currentHoleIndex)[0x28] = 1;
        ((CGolfRule*)this)->ResetShotGauge();
        ((CGolfRule*)this)->EnterState(kStateAim);
        B5_MSG_THIS(kText_screen, 15, 0);
        ((unsigned char*)g_cameraWork)[0x241] = 0;
    } else {
        B5_MSG_THIS(kText_clock, 0x2719, 0);
        if (((unsigned char*)g_sharedDoc)[0x8C0] == 1) {
            unsigned char value = (*(unsigned char**)((char*)InstanceTable_Get() + 4))[0xA];
            game = B5_GAME;
            ((GolfDocSnapshot8013*)(CurrentGolfPlayerEntryFromDoc(game)))->unk_BD = value;
        }
        game = B5_GAME;
        if (((CGolfDoc*)(game))->playerCount > 2 &&
            ((CGolfRule*)(o))->nextPlayerIndex != ((CGolfDoc*)(game))->currentPlayer) {
            B5_MSG_THIS(kText_player, 0x24, ((CGolfDoc*)(game))->currentPlayer);
            B5_MSG_THIS(kText_caddie, 0x17, ((CGolfDoc*)(B5_GAME))->currentPlayer);
        }
        if (*(int*)&((CSharedDoc*)(g_sharedDoc))->partyGameMode == 3) {
            void* obj = PartyItemSlots_GetInstance();
            PartyItemSlots_SetSelectedPlayer(obj, ((CGolfRule*)(o))->nextPlayerIndex);
            PartyItemSlots_RebuildEffectParameters(obj);
        }
        ((CGolfDoc*)(B5_GAME))->currentPlayer = ((CGolfRule*)(o))->nextPlayerIndex;
        game = B5_GAME;
        unsigned char* p = CurrentGolfPlayerEntryFromDoc(game);
        (p + ((CGolfDoc*)(B5_GAME))->currentHoleIndex)[0x28]++;
        game = B5_GAME;
        B5PlayerFields* fields = (B5PlayerFields*)CurrentGolfPlayerEntryFromDoc(game);
        fields->field170 = 0;
        game = B5_GAME;
        fields = (B5PlayerFields*)CurrentGolfPlayerEntryFromDoc(game);
        fields->field174 = 0;
        switch (*(int*)&((CSharedDoc*)(g_sharedDoc))->partyGameMode) {
        case 2: {
            unsigned int slot;
            if (*(int*)&((CSharedDoc*)(g_sharedDoc))->gameMode == 0 && ((unsigned char*)g_sharedDoc)[0x8C0] == 0)
                slot = 0;
            else if (((unsigned char*)g_sharedDoc)[0x8C0] == 1)
                slot = AsCGolfRule(o)->turn;
            else {
                slot = 0;
                do {
                    p = (unsigned char*)((void* (*)(void*, int))GolfPlayer_GetEntryByIndex)(B5_GAME,
                                                                                            (unsigned char)slot);
                    if (!((GolfDocSnapshot8013*)(p))->turnOrder)
                        break;
                    ++slot;
                } while (slot < 4);
            }
            if (((CGolfDoc*)(B5_GAME))->currentPlayer == (unsigned char)slot) {
                CWind::GetInstance()->LoadCourseRegions();
                CWind::GetInstance()->InitOverrideFromCourse();
                CourseData_UpdateWindActors(*(void**)(B5_GAME + 8));
            }
        }
        case 1:
            game = B5_GAME;
            *(WVector*)(CurrentGolfPlayerEntryFromDoc(game) + 0x1C) = *(WVector*)(o + 0x4E8);
            break;
        }
        game = B5_GAME;
        *(WVector*)actor = *(WVector*)(CurrentGolfPlayerEntryFromDoc(game) + 0x1C);
        game = B5_GAME;
        *(WVector*)ctx = *(WVector*)(CurrentGolfPlayerEntryFromDoc(game) + 0x1C);
        *(void**)(ctx + 0x28) = (void*)((CQuadTree*)CQuadTree_GetInstance())->GetMeshIndex(ctx, 0);
        B5_MSG_THIS(kText_player, 0x28, 0);
        B5_MSG_THIS(kText_caddie, 0x1A, 0);
        InitializeAimHeading();
        if (((CGolfDoc*)(B5_GAME))->playerCount > 2) {
            B5_MSG_THIS(kText_player, 0x26, (unsigned char)((CGolfRule*)o)->SelectNextPlayer(1));
            B5_MSG_THIS(kText_caddie, 0x18, (unsigned char)((CGolfRule*)o)->SelectNextPlayer(1));
        }
        if (((unsigned char*)g_sharedDoc)[0x8C0] == 1) {
            unsigned char value = (*(unsigned char**)((char*)InstanceTable_Get() + 4))[0xA];
            game = B5_GAME;
            ((GolfDocSnapshot8013*)(CurrentGolfPlayerEntryFromDoc(game)))->unk_BD = value;
        }
        if (PreparePlayerTurn()) {
            ((CGolfRule*)this)->ResetShotGauge();
            ((CGolfDoc*)B5_GAME)->UpdateCamera(1);
            ((CGolfRule*)this)->EnterState(kStateTurnStartDelay);
            B5_MSG_THIS(kText_voice, 8, 0);
        } else {
            ((CMouseCursor*)g_mouseCursor)->Reset(0);
            B5_MSG_THIS(kText_clock, 0, 0);
            ((CGolfRule*)this)->ResetShotGauge();
            ((CGolfDoc*)B5_GAME)->UpdateCamera(1);
            int h = NamedEffectList_FindByName(g_sceneManager, golfRuleStrings + kText_ballSequence);
            if (h)
                ((void (*)(int, int))WPuppet_SetOpacityRecursive)(h, 0xFF);
            Sound_StopByAlias(g_soundManager, golfRuleStrings + kText_shot_dist_count);
            ((int (*)(void*, const char*, int, int, int, int, float, float, float))Sound_PlayByName)(
                g_soundManager, golfRuleStrings + kText_play_my_turn, 0, 0, 0, 0, kFloat_0_5_GolfRule,
                kFloat_200_GolfRule, kFloat_1_805DEA38);
            ((CCameraWork*)g_cameraWork)->StopBallSound();
            ((CGolfRule*)this)->EnterState(kStateAim);
            B5_MSG_THIS(kText_voice, 4, 0);
        }
    }
    B5_MSG_THIS(kText_screen, 3, 0);
}

// Advance the course-reload task and restore camera, player and loading-screen state.
// Requires rule/services; courseReloadStage advances through the staged checks.
// Coordinates loading/fade handles, resource reload and player/background messages.
// Pending gates defer progress; completion enters story or hole-camera state.
// No failure return or local timeout is introduced by this state processor.
void CGolfRule::ProcessCourseReload() {
    using namespace ns4;
    unsigned char* object = (unsigned char*)this;
    int* state = (int*)(object + 0x4D0);
    int result;
    if (*state == 0) {
        if (!Sing_805DC610()) {
            if (!s_nowReloadingInitialized) {
                ((CNowReloading*)(&s_nowReloading))->Init();
                __register_global_object(&s_nowReloading, (void*)__dt__13CNowReloadingFv,
                                         s_nowReloadingDtorRegistration_8014B210);
                s_nowReloadingInitialized = 1;
            }
            Sing_805DC610() = &s_nowReloading;
        }
        LoadingIndicator_CreatePlayer(Sing_805DC610());
        *state = 1;
    }
    // 0x80166778: Stop state progression while the current asynchronous step is pending.
    do {
        if (*state == 1) {
            if (!Sing_805DC610()) {
                if (!s_nowReloadingInitialized) {
                    ((CNowReloading*)(&s_nowReloading))->Init();
                    __register_global_object(&s_nowReloading, (void*)__dt__13CNowReloadingFv,
                                             s_nowReloadingDtorRegistration_8014B210);
                    s_nowReloadingInitialized = 1;
                }
                Sing_805DC610() = &s_nowReloading;
            }
            if (LoadingIndicator_IsAbsentOrState3(Sing_805DC610()))
                *state = 2;
            else
                break;
        }
        if (*state == 2) {
            ((void (*)(void*, int))CQuadTree_SetObjectSoundEnabled)(CQuadTree_GetInstance(), 0);
            void* _p = IActor_GetDispatchObject(this);
            ((GolfTaskActivityFlags*)_p)->sceneHiddenForReload = 1;
            AsCSceneManager(g_sceneManager)->renderEnabled = 0;
            AsCSceneManager(g_sceneManager)->unk_14C = 0;
            IActor_DispatchMessage(this, this, golfRuleStrings + kText_player, 0x30,
                                   (int)(*(int*)(object + 0x4C8) != 3), 0, 0, 0);
            *state = 3;
        }
        if (*state == 3) {
            IActor_DispatchMessage(this, this, golfRuleStrings + kText_player, 0x31, (int)&result, 0, 0, 0);
            if (!result)
                break;
            int value = *(int*)(object + 0x4C8);
            IActor_DispatchMessage(this, this, golfRuleStrings + kText_caddie, 0x1E, (value != 3), 0, 0, 0);
            *state = 4;
        }
        if (*state == 4) {
            IActor_DispatchMessage(this, this, golfRuleStrings + kText_caddie, 0x1F, (int)&result, 0, 0, 0);
            if (!result)
                break;
            IActor_DispatchMessage(this, this, golfRuleStrings + kText_golfBg, 0x0B, *(int*)(object + 0x4C8), 0, 0, 0);
            *state = 5;
        }
        if (*state == 5) {
            IActor_DispatchMessage(this, this, golfRuleStrings + kText_golfBg, 0x0C, (int)&result, 0, 0, 0);
            if (!result)
                break;
            CWind::GetInstance()->LoadCourseRegions();
            CWind::GetInstance()->InitOverrideFromCourse();
            if (!Sing_805DC610()) {
                if (!s_nowReloadingInitialized) {
                    ((CNowReloading*)(&s_nowReloading))->Init();
                    __register_global_object(&s_nowReloading, (void*)__dt__13CNowReloadingFv,
                                             s_nowReloadingDtorRegistration_8014B210);
                    s_nowReloadingInitialized = 1;
                }
                Sing_805DC610() = &s_nowReloading;
            }
            CB2dPlayerHandle::RequestPendingScript(Sing_805DC610());
            *state = 6;
        }
        if (*state == 6) {
            if (!Sing_805DC610()) {
                if (!s_nowReloadingInitialized) {
                    ((CNowReloading*)(&s_nowReloading))->Init();
                    __register_global_object(&s_nowReloading, (void*)__dt__13CNowReloadingFv,
                                             s_nowReloadingDtorRegistration_8014B210);
                    s_nowReloadingInitialized = 1;
                }
                Sing_805DC610() = &s_nowReloading;
            }
            if (!CNowReloading::IsPlayerAbsent(Sing_805DC610()))
                break;
            *state = 7;
        }
        void* _q = IActor_GetDispatchObject(this);
        ((GolfTaskActivityFlags*)_q)->sceneHiddenForReload = 0;
        AsCSceneManager(g_sceneManager)->renderEnabled = 1;
        AsCSceneManager(g_sceneManager)->unk_14C = 1;
        if (*(int*)(object + 0x4C8) != 3) {
            ((CGolfRule*)this)->EnterState(kStateStorySequence);
            break;
        }
        if (!Sing_805DC608()) {
            if (!s_fadeOverlayInitialized) {
                FadeOverlay_Init(s_fadeOverlay);
                __register_global_object(s_fadeOverlay, (void*)ResourceState_Dtor, s_fadeOverlayRegistration_804F72D8);
                s_fadeOverlayInitialized = 1;
            }
            Sing_805DC608() = s_fadeOverlay;
        }
        FadeState_StartFadeOut(Sing_805DC608(), 0x0F);
        for (int i = 0; i < AsCGolfDoc((*(char**)&AsCSharedDoc(g_sharedDoc)->golfDoc))->playerCount; ++i) {
            int a = *(int*)((char*)GolfPlayer_GetEntryByIndex((int)(*(char**)&AsCSharedDoc(g_sharedDoc)->golfDoc),
                                                              (unsigned char)i) +
                            4) == 1;
            int b = SharedDoc_IsPlayerFlag0Set(i) != 0;
            if (a != b)
                IActor_DispatchMessage(this, this, golfRuleStrings, 0x13, i, 0, 0, 0);
        }
        ((CGolfRule*)this)->EnterState(kStateHoleCameraPath);
    } while (0);

    return;
}

// Advance the story sequence and fade before returning to normal hole play.
// Requires rule/scene/terrain services; value is the frame time passed to updates.
// Polls story completion through the stored reply word and coordinates fade state.
// On completion, the stored story result selects course reload or Tour exit;
// otherwise remains pending. No success/failure result.
void CGolfRule::ProcessStorySequence(float value) {
    using namespace ns4;
    unsigned char* object = (unsigned char*)this;
    void* controller = CQuadTree_GetInstance();
    ((CQuadTree*)controller)->UpdateAnimatedObjects(value, 0);
    if (*(int*)(object + 0x4CC) == 0) {
        IActor_DispatchMessage(this, this, golfRuleStrings + kText_storyConte, 3, (int)(object + 0x4CC), 0, 0, 0);
        if (*(int*)(object + 0x4CC) != 0) {
            if (!Sing_805DC608()) {
                if (!s_fadeOverlayInitialized) {
                    FadeOverlay_Init(s_fadeOverlay);
                    __register_global_object(s_fadeOverlay, (void*)ResourceState_Dtor,
                                             s_fadeOverlayRegistration_804F72D8);
                    s_fadeOverlayInitialized = 1;
                }
                Sing_805DC608() = s_fadeOverlay;
            }
            FadeState_Reset(Sing_805DC608());
            if (!Sing_805DC608()) {
                if (!s_fadeOverlayInitialized) {
                    FadeOverlay_Init(s_fadeOverlay);
                    __register_global_object(s_fadeOverlay, (void*)ResourceState_Dtor,
                                             s_fadeOverlayRegistration_804F72D8);
                    s_fadeOverlayInitialized = 1;
                }
                Sing_805DC608() = s_fadeOverlay;
            }
            FadeState_RequestFramebufferCapture(Sing_805DC608());
            if (!Sing_805DC608()) {
                if (!s_fadeOverlayInitialized) {
                    FadeOverlay_Init(s_fadeOverlay);
                    __register_global_object(s_fadeOverlay, (void*)ResourceState_Dtor,
                                             s_fadeOverlayRegistration_804F72D8);
                    s_fadeOverlayInitialized = 1;
                }
                Sing_805DC608() = s_fadeOverlay;
            }
            FadeState_StartFadeIn(Sing_805DC608(), 0);
        }
    } else {
        IActor_DispatchMessage(this, this, golfRuleStrings + kText_camera, 4, 0, 0, 0, 0);
        IActor_DispatchMessage(this, this, golfRuleStrings + kText_screen, 0x5A, 0, 0, 0, 0);
        IActor_DispatchMessage(this, this, golfRuleStrings + kText_player, 0x2F, 0, 0, 0, 0);
        IActor_DispatchMessage(this, this, golfRuleStrings + kText_caddie, 0x1D, 0, 0, 0, 0);
        CQuadTree_SetObjectSoundEnabled(CQuadTree_GetInstance(), 1);
        unsigned char i;
        void* cached = g_sceneManager;
        for (i = 0; i < 3; ++i)
            DrawScene_SetNamedObjectEnabled(cached, golfRuleAmbientCreatureNames[i], 1);
        IActor_DispatchMessage(this, this, golfRuleStrings + kText_storyConte, 1, 0, 0, 0, 0);
        int state = *(int*)(object + 0x4C8);
        switch (state) {
        case 0:
            *(int*)(object + 0x4C8) = 3;
            ((CGolfRule*)this)->EnterState(kStateCourseReload);
            break;
        case 1:
            ((CGolfRule*)this)->EnterState(kStateTourExit);
            break;
        }
    }
}

// Advance round-result animations, result menus, fades and the eventual task transition.
// Requires rule-owned result handles and documents; dt is seconds.
// Phases poll actor readiness, start result presentation, wait for its duration,
// create/process the menu, fade, then release it. Pending replies defer each phase;
// no failure return. Preserve switch shape and lazy singleton registration order.
void CGolfRule::ProcessRoundResultPresentation(float dt) {
    using namespace ns4;
    char* self = (char*)this;
    switch (*(unsigned int*)(self + 0x4E0)) {
    case 0: {

        SINGLETON_A();
        FadeState_Reset(Sing_805DC608());
        SINGLETON_A();
        FadeState_RequestFramebufferCapture(Sing_805DC608());
        SINGLETON_A();
        FadeState_StartFadeIn(Sing_805DC608(), 0);
        unsigned int result;
        IActor_DispatchMessage(self, self, golfRuleStrings + kText_player, 0x29, (int)&result, 0, 0, 0);
        if (0 == result) {
            IActor_DispatchMessage(self, self, golfRuleStrings + kText_player, 0x26, 0, 0, 0, 0);
            IActor_DispatchMessage(self, self, golfRuleStrings + kText_caddie, 0x18, 0, 0, 0, 0);
            *(int*)(self + 0x4E0) = 1;
        } else
            *(int*)(self + 0x4E0) = 2;
        break;
    }
    case 1: {
        int a;
        int b;
        IActor_DispatchMessage(self, self, golfRuleStrings + kText_player, 0x27, (int)&a, 0, 0, 0);
        IActor_DispatchMessage(self, self, golfRuleStrings + kText_caddie, 0x19, (int)&b, 0, 0, 0);
        if (a == 0 && b == 0)
            *(int*)(self + 0x4E0) = 2;
        break;
    }
    case 2: {
        IActor_DispatchMessage(self, self, golfRuleStrings + kText_score, 1, 0, 0, 0, 0);
        IActor_DispatchMessage(self, self, golfRuleStrings + kText_ball, 1, 0, 0, 0, 0);
        IActor_DispatchMessage(self, self, golfRuleStrings + kText_spot, 4, 0, 0, 0, 0);
        IActor_DispatchMessage(self, self, golfRuleStrings + kText_grid, 3, 0, 0, 0, 0);
        IActor_DispatchMessage(self, self, golfRuleStrings + kText_screen, 7, 0, 0, 0, 0);
        IActor_DispatchMessage(self, self, golfRuleStrings + kText_screen, 3, 0, 0, 0, 0);
        IActor_DispatchMessage(self, self, golfRuleStrings + kText_effect, 0xB, 0, 0, 0, 0);
        IActor_DispatchMessage(self, self, golfRuleStrings + kText_trajectory, 6, 0, 0, 0, 0);
        ((CFx*)g_fx)->ClearEffects(1);
        ((*(unsigned char**)&AsCSharedDoc(g_sharedDoc)->golfDoc))[0x3C1] = 0;
        IActor_DispatchMessage(self, self, golfRuleStrings + kText_player, 6, 0, 0, 0, 0);
        IActor_DispatchMessage(self, self, golfRuleStrings + kText_caddie, 3, 0, 0, 0, 0);
        unsigned char* mgr = (unsigned char*)g_sharedDoc;
        unsigned int mode = ((*(unsigned char**)&AsCSharedDoc(mgr)->golfDoc))[0x3C1];
        if (1 == ((CSharedDoc*)(mgr))->golfRuleType)
            mode = ((CGolfDoc*)((*(void**)&AsCSharedDoc(mgr)->golfDoc)))->ResolveMatchTurnFromHoleStrokes();
        IActor_DispatchMessage(self, self, golfRuleStrings + kText_player, 0x32, 2, mode, 0, 0);
        CGolfRule::StartRoundResultPresentation(self, 0, 1, 0);
        IActor_DispatchMessage(self, self, golfRuleStrings + kText_voice, 0xA, 0, 0, 0, 0);
        SINGLETON_A();
        FadeState_StartFadeOut(Sing_805DC608(), 15);
        *(int*)(self + 0x4E0) = 3;
        break;
    }
    case 3:
        if (((GolfRuleFrameFields*)(self))->elapsedTime < ((GolfRuleFrameFields*)(self))->stateTimer) {
            ((CCameraWork*)g_cameraWork)->UpdateMotionAndTarget(dt, kFloat_0_GolfRule);
            ((GolfRuleFrameFields*)(self))->elapsedTime += dt;
        } else {
            if (!*(void**)&((CGolfRule*)(self))->resultMenu)
                *(void**)&((CGolfRule*)(self))->resultMenu = ((CGolfRule*)self)->CreateResultMenu(0);
            *(int*)(self + 0x4E0) = 4;
        }
        break;
    case 4:
        if (MPBase1_Process(*(void**)&((CGolfRule*)(self))->resultMenu)) {
            ((SelfFields*)self)->count = 0;
            volatile void* _sb = Sing_805DC660();
            if (_sb == 0) {
                if (0 == s_effectFadeInitialized) {
                    ((CEffectFade*)s_effectFade)->__ct();
                    __register_global_object(s_effectFade, (void*)__dt__11CEffectFadeFv,
                                             s_effectFadeDtorRegistration_GolfRule);
                    s_effectFadeInitialized = 1;
                }
                Sing_805DC660() = s_effectFade;
            }
            FadeQuad_SetRgb(Sing_805DC660(), 0);
            ((CEffectFade*)GetSingletonB())->SetAlpha(0);
            *(int*)(self + 0x4E0) = 5;
        }
        break;
    case 5:
        if (((SelfFields*)self)->count < 15) {
            unsigned char n = ((SelfFields*)self)->count + 1;
            volatile void* _sb = Sing_805DC660();
            ((SelfFields*)self)->count = n;
            int v = (int)((float)((((n * 255)))) / 15.0f);
            if (!_sb) {
                if (!s_effectFadeInitialized) {
                    ((CEffectFade*)s_effectFade)->__ct();
                    __register_global_object(s_effectFade, (void*)__dt__11CEffectFadeFv,
                                             s_effectFadeDtorRegistration_GolfRule);
                    s_effectFadeInitialized = 1;
                }
                Sing_805DC660() = s_effectFade;
            }
            ((CEffectFade*)Sing_805DC660())->SetAlpha((unsigned char)v);
        } else {
            SINGLETON_A();
            FadeState_Reset(Sing_805DC608());
            SINGLETON_A();
            FadeState_RequestFramebufferCapture(Sing_805DC608());
            SINGLETON_A();
            FadeState_StartFadeIn(Sing_805DC608(), 0);
            *(int*)(self + 0x4E0) = 6;
        }
        break;
    case 6: {
        unsigned char* mgr = (unsigned char*)g_sharedDoc;
        int mode = ((*(unsigned char**)&AsCSharedDoc(mgr)->golfDoc))[0x3C1];
        if (1 == ((CSharedDoc*)(mgr))->golfRuleType)
            mode = ((CGolfDoc*)((*(void**)&AsCSharedDoc(mgr)->golfDoc)))->ResolveMatchTurnFromHoleStrokes();
        IActor_DispatchMessage(self, self, golfRuleStrings + kText_player, 0x32, 1, mode, 0, 0);
        SINGLETON_B();
        ((CEffectFade*)Sing_805DC660())->SetAlpha(0);
        VObj* p = *(VObj**)(self + 0x4AC);
        if (p) {
            delete p;
            *(VObj**)(self + 0x4AC) = 0;
        }
        *(int*)(self + 0x4E0) = 7;
        break;
    }
    default:
        GolfTransition_ClearInputAudioAndEffects(g_project);
        SaveProcess_SetTransitionTargetName(golfRuleStrings + kText_cTourTask);
        SaveProcess_SetFollowUpTaskName(golfRuleStrings + kText_tourMain);
        ((CTask*)g_taskManager)
            ->RequestTransitionWithFade(golfRuleStrings + kText_cSaveProcessTask,
                                        golfRuleStrings + kText_emptyTaskArgument, 0);
        break;
    }
}

// Wait for player and caddie resources, then reset hole positions and start its camera.
// Requires rule/player/caddie and loading-indicator services. Computes the selected
// player from mode/turn-order rules and polls the actor readiness replies.
// When ready, resets hole positions and enters player preview or hole-camera state;
// otherwise waits for readiness. No success/failure return.
// The caller also passes dt; this body does not read it.
void CGolfRule::WaitForHoleActors(float dt) {
    using namespace ns4;
    unsigned char* object = (unsigned char*)this;
    unsigned char* manager = (unsigned char*)g_sharedDoc;
    unsigned int index;
    if ((*(int*)&AsCSharedDoc(manager)->gameMode) == 0 && AsCSharedDoc(manager)->golfRuleType == 0) {
        index = 0;
    } else if (AsCSharedDoc(manager)->golfRuleType == 1) {
        index = AsCGolfRule(object)->turn;
    } else {
        for (index = 0; index < 4; ++index) {
            if (!*((unsigned char*)GolfPlayer_GetEntryByIndex((int)(*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc),
                                                              (unsigned char)index) +
                   0xA4))
                break;
        }
    }
    int a;
    IActor_DispatchMessage(this, this, golfRuleStrings + kText_player, 0x27, (int)&a, (unsigned char)index, 0, 0);
    manager = (unsigned char*)g_sharedDoc;
    if ((*(int*)&AsCSharedDoc(manager)->gameMode) == 0 && AsCSharedDoc(manager)->golfRuleType == 0) {
        index = 0;
    } else if (AsCSharedDoc(manager)->golfRuleType == 1) {
        index = AsCGolfRule(object)->turn;
    } else {
        for (index = 0; index < 4; ++index) {
            if (!*((unsigned char*)GolfPlayer_GetEntryByIndex((int)(*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc),
                                                              (unsigned char)index) +
                   0xA4))
                break;
        }
    }
    int b;
    IActor_DispatchMessage(this, this, golfRuleStrings + kText_caddie, 0x19, (int)&b, (unsigned char)index, 0, 0);
    if (a && b) {
        if (!Sing_805DC610()) {
            if (!s_nowReloadingInitialized) {
                ((CNowReloading*)(&s_nowReloading))->Init();
                __register_global_object(&s_nowReloading, (void*)__dt__13CNowReloadingFv,
                                         s_nowReloadingDtorRegistration_8014B210);
                s_nowReloadingInitialized = 1;
            }
            Sing_805DC610() = &s_nowReloading;
        }
        void* singleton = Sing_805DC610();
        if (singleton && LoadingIndicator_IsAbsentOrState3(singleton))
            CB2dPlayerHandle::RequestPendingScript(singleton);
        IActor_DispatchMessage(this, this, golfRuleStrings + kText_score, 2, 0, 0, 0, 0);
        ((CGolfDoc*)((*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc)))->ResetPlayerBallPositionsForHole();
        if (((CGolfRule*)(object))->roundRestarted == 0 &&
            ((*(unsigned char**)&AsCSharedDoc(g_sharedDoc)->golfDoc))[0x3B6] == 0)
            ((CGolfRule*)this)->EnterState(kStatePlayerPreview);
        else
            ((CGolfRule*)this)->EnterState(kStateHoleCameraPath);
    }
}

// Prepare per-player turn data, instructions and computer-shot thinking state.
// Requires current player and ball; clears its checkpoint and copies eight inventory
// words. Publishes turn hints/panels and optional CPU thinking reset.
// On green, Match compares lead/tie and the remaining holes for a closing opportunity;
// the return value requests a turn-start delay only when that player's option admits it.
// Otherwise returns 0 after score refresh; several hint branches return early.
int CGolfRule::PreparePlayerTurn() {
    using namespace ns4;
    unsigned char* ballBytes = (unsigned char*)GolfBall__Fv();
    float duration = kFloat_2_805DEA84;
    int active, other;
    int result = 0;
    memset((unsigned char*)this + AsCGolfDoc(GolfDocBytes())->currentPlayer * 0x54 + 0x10C, 0, 0x54);
    int index = AsCGolfDoc(GolfDocBytes())->currentPlayer;
    memcpy((unsigned char*)this + index * 0x54 + 0x130, SharedDocBytes() + index * 0x1D4 + 0x8C, 0x20);
    // Phase 1: reset shot panels and restore current-player turn presentation.
    IActor_DispatchMessage(this, this, golfRuleStrings + kText_screen, 0xF, 0, 0, 0, 0);
    ((unsigned char*)g_cameraWork)[0x241] = 0;
    if (SharedDoc_IsPlayerFlag0Set(AsCGolfDoc(GolfDocBytes())->currentPlayer))
        ((CRival*)g_rival)->ResetShotThinkingState();
    // 0x80167A58: Join ordinary behavior after the mode-specific camera setup.
    do {
        if (AsCSharedDoc(SharedDocBytes())->golfRuleType == 1) {
            if (((CGolfBall*)(ballBytes))->m_groundType != 2)
                break;
            int current = AsCGolfDoc(GolfDocBytes())->currentPlayer;
            if (current < 0)
                active = 0;
            else {
                active = 1;
                if (current <= 1)
                    active = current;
            }
            other = 1 - active;
            int lead = 0, tie = 0, done = 0;
            unsigned char* a = GolfPlayerEntryByte(other);
            unsigned char* b = GolfPlayerEntryByte(active);
            int gap = *(int*)&((GolfDocSnapshot8013*)(b))->roundResultWords[0] -
                      *(int*)&((GolfDocSnapshot8013*)(a))->roundResultWords[0];
            if (*(int*)(GolfPlayerEntryByte(other) + 8) == 1) {
                int s1 = AsCGolfDoc(GolfDocBytes())->currentHoleIndex;
                a = GolfPlayerEntryByte(other);
                int s2 = AsCGolfDoc(GolfDocBytes())->currentHoleIndex;
                b = GolfPlayerEntryByte(active);
                if ((b + s2)[0x28] < (a + s1)[0x28])
                    lead = 1;
                else {
                    int s1 = AsCGolfDoc(GolfDocBytes())->currentHoleIndex;
                    a = GolfPlayerEntryByte(other);
                    int s2 = AsCGolfDoc(GolfDocBytes())->currentHoleIndex;
                    b = GolfPlayerEntryByte(active);
                    if ((b + s2)[0x28] == (a + s1)[0x28])
                        tie = 1;
                }
            } else {
                int s1 = AsCGolfDoc(GolfDocBytes())->currentHoleIndex;
                a = GolfPlayerEntryByte(other);
                int s2 = AsCGolfDoc(GolfDocBytes())->currentHoleIndex;
                b = GolfPlayerEntryByte(active);
                if ((b + s2)[0x28] <= (a + s1)[0x28])
                    lead = 1;
                else {
                    int s1 = AsCGolfDoc(GolfDocBytes())->currentHoleIndex;
                    a = GolfPlayerEntryByte(other);
                    int s2 = AsCGolfDoc(GolfDocBytes())->currentHoleIndex;
                    b = GolfPlayerEntryByte(active);
                    if ((b + s2)[0x28] == (a + s1)[0x28] + 1)
                        tie = 1;
                }
            }
            if (lead) {
                if (gap + 1 >=
                    (int)AsCSharedDoc(SharedDocBytes())->roundHoleCount - AsCGolfDoc(GolfDocBytes())->currentHoleIndex)
                    done = 1;
            } else if (tie && gap >= (int)AsCSharedDoc(SharedDocBytes())->roundHoleCount -
                                         AsCGolfDoc(GolfDocBytes())->currentHoleIndex)
                done = 1;
            if ((lead || done) && *(int*)(SharedDocBytes() + 0x1B8 + AsCGolfDoc(GolfDocBytes())->currentPlayer * 0x1D4))
                result = 1;
            if (done) {
                a = GolfPlayerEntryByte(active);
                GolfTips_ShowIndexedMessage(3, *(int*)(a + 4) == 1 ? T7A(0xEC) : T7A(0x128), 0, duration,
                                            kFloat_0_5_GolfRule);
                return result;
            }
            if (lead) {
                a = GolfPlayerEntryByte(active);
                GolfTips_ShowIndexedMessage(3, *(int*)(a + 4) == 1 ? T7A(0x15E) : T7A(0x19A), 0, duration,
                                            kFloat_0_5_GolfRule);
                return result;
            }
        } else {
            unsigned char* p = GolfPlayerEntryByte(AsCGolfDoc(GolfDocBytes())->currentPlayer);
            unsigned char* manager = SharedDocBytes();
            unsigned char* control = (*(unsigned char**)&AsCSharedDoc(manager)->golfDoc);
            int slot = ((CGolfDoc*)(control))->currentHoleIndex;
            if ((p + slot)[0x28] < (*(unsigned char**)&((CGolfDoc*)(control))->holeRecords + slot * 0x20)[0x18] &&
                ((CGolfBall*)(ballBytes))->m_groundType == 2) {
                result = *(int*)(manager + ((CGolfDoc*)(control))->currentPlayer * 0x1D4 + 0x1B8) != 0;
                p = GolfPlayerEntryByte(((CGolfDoc*)(control))->currentPlayer);
                control = GolfDocBytes();
                slot = ((CGolfDoc*)(control))->currentHoleIndex;
                int delta =
                    (p + slot)[0x28] - (*(unsigned char**)&((CGolfDoc*)(control))->holeRecords + slot * 0x20)[0x18];
                switch (delta) {
                case -2:
                    GolfTips_ShowIndexedMessage(2, T7A(0x1D0), 0, duration, kFloat_0_5_GolfRule);
                    return result;
                case -1:
                    GolfTips_ShowIndexedMessage(1, T7A(0x206), 0, duration, kFloat_0_5_GolfRule);
                    return result;
                }
            }
        }
    } while (0);

    // Phase 2: select computer thinking or player input before refreshing the score.
    if (SharedDoc_IsPlayerFlag0Set(AsCGolfDoc(GolfDocBytes())->currentPlayer)) {
        if (!((CRival*)g_rival)->IsUnplayableDropActive())
            GolfTips_ShowIndexedMessage(0xC, T7A(0x23C), 0, duration, kFloat_0_5_GolfRule);
    } else {
        unsigned char* p = GolfPlayerEntryByte(AsCGolfDoc(GolfDocBytes())->currentPlayer);
        if ((p + AsCGolfDoc(GolfDocBytes())->currentHoleIndex)[0x28] == 1 &&
            *(int*)(GolfPlayerEntryByte(AsCGolfDoc(GolfDocBytes())->currentPlayer) + 0x160) == 2) {
            GolfTips_ShowIndexedMessage(0xD, T7A(0x270), 0, duration, kFloat_0_5_GolfRule);
        } else if (((CGolfBall*)(ballBytes))->m_groundType == 2 || ((unsigned char*)InstanceTable_Get())[0x48] == 4) {
            unsigned int count = 0;
            for (unsigned int i = 0; i < AsCGolfDoc(GolfDocBytes())->playerCount; ++i)
                if (!SharedDoc_IsPlayerFlag0Set(i))
                    ++count;
            if (count > 1) {
                switch (AsCGolfDoc(GolfDocBytes())->currentPlayer) {
                case 0:
                    GolfTips_ShowIndexedMessage(8, T7A(0x2A0), 0, duration, kFloat_0_5_GolfRule);
                    break;
                case 1:
                    GolfTips_ShowIndexedMessage(9, T7A(0x2D2), 0, duration, kFloat_0_5_GolfRule);
                    break;
                case 2:
                    GolfTips_ShowIndexedMessage(10, T7A(0x304), 0, duration, kFloat_0_5_GolfRule);
                    break;
                case 3:
                    GolfTips_ShowIndexedMessage(11, T7A(0x336), 0, duration, kFloat_0_5_GolfRule);
                    break;
                }
            } else
                GolfTips_ShowIndexedMessage(4, T7A(0x368), 0, duration, kFloat_0_5_GolfRule);
        } else if (((unsigned char*)InstanceTable_Get())[0x48] <= 2) {
            GolfTips_ShowIndexedMessage(0, T7A(0x39E), 0, duration, kFloat_0_5_GolfRule);
        } else if (((unsigned char*)InstanceTable_Get())[0x48] <= 3) {
            GolfTips_ShowIndexedMessage(0, T7A(0x3E0), 0, duration, kFloat_0_5_GolfRule);
        }
    }
    IActor_DispatchMessage(this, this, golfRuleStrings + kText_score, 1, 0, 0, 0, 0);
    return result;
}

// ===== S15 - Aim reset, shot start and undo =====
// ShotCheckpointDeque and player snapshots; borrowed terrain handles.

// Point the shot heading from the ball toward the current hole target.
// Requires rule aimTarget, ball and current player; chooses target/club by CPU
// and tutorial branches. Computes heading in radians against world +Z, clears
// power-shot mode and broadcasts the borrowed heading float to five recipients.
// Resets power-color UI; no failure result.
void CGolfRule::InitializeAimHeading() {
    using namespace ns4;
    float zero2;
    unsigned char* player;
    IActor_DispatchMessage(this, this, golfRuleStrings + kText_screen, 1, 0, 0, 0, 0);
    if (SharedDoc_IsPlayerFlag0Set(((*(unsigned char**)&AsCSharedDoc(g_sharedDoc)->golfDoc))[0x3C1])) {
        ((CRival*)g_rival)->ResetShotThinkingState();
        struct V3 {
            unsigned int x, y, z;
        };
        V3 vec = *(V3*)AsCRival(g_rival)->holePosition;
        __as__7WVectorFRC7WVector(&AsGolfRuleFrame(this)->aimTarget, &vec);
    } else {
        int mode = AsCGolfDoc((*(unsigned char**)&AsCSharedDoc(g_sharedDoc)->golfDoc))->tutorialStage;
        if (mode == 2 || mode == 3)
            ((CClub*)InstanceTable_Get())->SelectBestClub(&AsGolfRuleFrame(this)->aimTarget);
        else
            ((CClub*)InstanceTable_Get())->SelectBestClubForHole();
        player = (*(unsigned char**)&AsCSharedDoc(g_sharedDoc)->golfDoc);
        if (((CGolfDoc*)(player))->tutorialStage == 5 || ((CGolfDoc*)(player))->tutorialStage == 6) {
            unsigned int value[3];
            GolfAim_FindTargetAlongPath(value, ((CGolfDoc*)(player))->aimPath, 0);
            __as__7WVectorFRC7WVector(&AsGolfRuleFrame(this)->aimTarget, value);
        }
    }
    float out[3];
    float input[3];
    input[0] = kFloat_0_GolfRule;
    input[1] = kFloat_0_GolfRule;
    input[2] = kFloat_1_805DEA38;
    unsigned char* p = (*(unsigned char**)&AsCSharedDoc(g_sharedDoc)->golfDoc);
    __mi__FRC7WVectorRC7WVector(out, &AsGolfRuleFrame(this)->aimTarget, (float*)GolfBall__Fv());
    AsCGolfDoc(p)->headingRadians = Vector_SignedAngleXZ(out, input);
    unsigned char* ea = (unsigned char*)InstanceTable_Get();
    *(int*)(ea + 0x44) = 0;
    float h;
    IActor_DispatchMessage(
        this, this, golfRuleStrings + kText_camera, 0x2717,
        (h = AsCGolfDoc((*(unsigned char**)&AsCSharedDoc(g_sharedDoc)->golfDoc))->headingRadians, (int)&h), 0, 0, 0);
    IActor_DispatchMessage(this, this, golfRuleStrings + kText_spot, 0x2717, (int)&h, 0, 0, 0);
    IActor_DispatchMessage(this, this, golfRuleStrings + kText_screen, 0x2717, (int)&h, 0, 0, 0);
    IActor_DispatchMessage(this, this, golfRuleStrings + kText_grid, 0x2717, (int)&h, 0, 0, 0);
    IActor_DispatchMessage(this, this, golfRuleStrings + kText_caddie, 0x2717, (int)&h, 0, 0, 0);
    IActor_DispatchMessage(this, this, golfRuleStrings + kText_player, 6, 0, 0, 0, 0);
    IActor_DispatchMessage(this, this, golfRuleStrings + kText_caddie, 3, 0, 0, 0, 0);
    zero2 = kFloat_0_GolfRule;
    IActor_DispatchMessage(this, this, golfRuleStrings + kText_spot, 5, 0, 0, 0, 0);
    IActor_DispatchMessage(this, this, golfRuleStrings + kText_screen, 9, (int)&zero2, 0, 0, 0);
}

// Restore the first simulation sample and set up ball-flight camera and timing.
// Requires ball sample 0 and completed history plus live rule/doc/camera/effects.
// Restores ball position/velocity/state, applies vertical accessory bonus and flags,
// then updates playback cursor, camera, sound and actor messages. arg is forwarded
// unchanged to CCameraWork::CycleMode.
// No failure return or random selection occurs in this body.
void CGolfRule::StartBallFlightFromSample(void* arg) {
    using namespace ns4;
    unsigned char camera[0x30];
    unsigned char vec[12];
    void* point;
    void* initialBall = GolfBall__Fv();

    point = PreData__9CGolfBallFi(initialBall, 0);
    AsGolfRuleFrame(this)->animationStarted = 0;
    AsGolfRuleFrame(this)->stateEntryPending = 1;
    AsCGolfBall(initialBall)->m_eventPreDataIndex[kBallPlaybackCursor] = 0;
    AsCGolfBall(initialBall)->m_pos = ((ShotSample*)point)->m_pos;
    AsCGolfBall(initialBall)->m_velocity = ((ShotSample*)point)->velocity;
    float verticalVelocityBonus = ((ShotAccessoryModifiers*)PartyItemSlots_GetInstance())->verticalVelocityBonus;
    float one = kFloat_1_805DEA38;
    float f = AsCGolfBall(initialBall)->m_velocity.y * (one + verticalVelocityBonus);
    AsCGolfBall(initialBall)->m_velocity.y = f;
    AsCGolfBall(initialBall)->m_state = ((ShotSample*)point)->motionState;
    AsCGolfBall(initialBall)->m_flags = AsCGolfBall(initialBall)->m_powerFlags;
    IActor_DispatchMessage(this, this, golfRuleStrings + kText_npcActor, 3, 0, 0, 0, 0);
    IActor_DispatchMessage(this, this, golfRuleStrings + kText_screen, 22, 0, 0, 0, 0);
    IActor_DispatchMessage(this, this, golfRuleStrings + kText_caddie, 3, 1, 0, 0, 0);
    IActor_DispatchMessage(this, this, golfRuleStrings + kText_caddie, 1, 0, 0, 0, 0);
    void* state = GolfBall__Fv();
    void* p = PreData__9CGolfBallFi(state, 0);
    void* selector = GolfBall__Fv();
    void* qbase = GolfBall__Fv();

    // Same final-sample horizontal distance (including roll) as both launch paths.
    void* q = PreData__9CGolfBallFi(qbase, AsCGolfBall(selector)->m_preDataCount);
    __mi__FRC7WVectorRC7WVector(vec, q, p);
    *(float*)(((vec + 4))) = 0.0f;
    float distance = (*(WVector*)vec).Magnitude();
    if (AsCGolfBall(GolfBall__Fv())->m_recoveryRequired == 0 && distance > 416.0f)
        IActor_DispatchMessage(this, this, golfRuleStrings + kText_caddie, 2, 0, 0, 0, 0);
    else
        ((CameraCaddieOption*)g_cameraWork)->caddieCameraEnabled = 0;
    IActor_DispatchMessage(this, this, golfRuleStrings + kText_screen, 6, 0, 0, 0, 0);
    IActor_DispatchMessage(this, this, golfRuleStrings + kText_screen, 38, 0, 0, 0, 0);
    IActor_DispatchMessage(this, this, golfRuleStrings + kText_trajectory, 6, 0, 0, 0, 0);
    IActor_DispatchMessage(this, this, golfRuleStrings + kText_letter, 0, 0, 0, 0, 0);
    IActor_DispatchMessage(this, this, golfRuleStrings + kText_effect, 11, 0, 0, 0, 0);
    IActor_DispatchMessage(this, this, golfRuleStrings + kText_effect, 12, 0, 0, 0, 0);
    ((CFx*)g_fx)->ClearEffects(1);
    IActor_DispatchMessage(this, this, golfRuleStrings + kText_ball, 7, 0, 0, 0, 0);
    Camera_SetFov(g_view, 0.9599310755729675f);

    CGolfDoc::BuildCameraMatrix(camera, (*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc));
    Matrix_CopyAffine(s_cameraTransform, camera);
    Sound_StopByAlias(g_soundManager, golfRuleStrings + kText_shot_dist_count);
    IActor_DispatchMessage(this, this, golfRuleStrings + kText_letter, 6, 0, 0, 0, 0);
    IActor_DispatchMessage(this, this, golfRuleStrings + kText_player, 1, 0, 0, 0, 0);
    AsGolfRuleFrame(this)->elapsedTime = 1.0f;
    void* manager = g_sharedDoc;
    void* control = (*(void**)&AsCSharedDoc(manager)->golfDoc);

    void* slot = (char*)manager + (AsCGolfDoc(control)->currentPlayer) * 0x1D4;
    if (((PlayerSwingOptions*)slot)->buttonSwingSelected == 1)
        AsCGolfDoc(control)->gameStateFlags = 0x40;
    else if (((ClubSelectionRuntime*)InstanceTable_Get())->powerShotMode == 0)
        ((CGolfRule*)this)->EnterState((int)0x80000000);
    else
        AsCGolfDoc((*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc))->gameStateFlags = 0x20000000;
    control = (*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc);
    AsCGolfDoc(control)->shotReactionActive = 1;
    int val478 = AsGolfRuleFrame(this)->playbackFrame;
    void* cb = CQuadTree_GetInstance();
    ((ShotWorldClockFields*)cb)->frame = val478;
    ((ShotWorldClockFields*)cb)->fractionalSeconds = 0.0f;
    ((CCameraWork*)g_cameraWork)->CycleMode(arg);
    AsCGolfDoc((*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc))->shotMeterFadeResetRequested = 1;
    if (g_helpMsg)
        NoticeQueue_ResetAndActivate(g_helpMsg);
}

// Finalize shot statistics and presentation, then choose the next player/result state.
// Requires rule, current player and ball; notifies screen/score/NPC in that order.
// Clears completion bytes, stops shot audio and records shot statistics.
// Hole-in advances directly; otherwise applies recovery/no-bounce stroke handling
// and par+4 policy before selecting delayed transition, fight result or actor wait.
void CGolfRule::FinalizeShotAndAdvanceState() {
    using namespace ns4;
    // Phase 1: clear entry bytes and notify each actor using its own message ID.
    unsigned char* ballBytes;
    ((RuleShotCompletionBytes*)this)->stateEntryPending = 0;
    ((RuleShotCompletionBytes*)this)->unresolved92 = 0;
    ((RuleShotCompletionBytes*)this)->unresolved94 = 0;
    IActor_DispatchMessage(this, this, golfRuleStrings + kText_screen, kScreenFinalizeShotMessage17, 0, 0, 0, 0);
    IActor_DispatchMessage(this, this, golfRuleStrings + kText_score, kScoreFinalizeShotMessage5, 0, 0, 0, 0);
    IActor_DispatchMessage(this, this, golfRuleStrings + kText_npcActor, kNpcFinalizeShotMessage0, 0, 0, 0, 0);
    ballBytes = (unsigned char*)GolfBall__Fv();
    if ((unsigned int)AsCGolfDoc(GolfDocBytes())->gameStateFlags == kStateButtonShotAnimation) {
        if (*(int*)(ballBytes + 0x88) > 0)
            IActor_DispatchMessage(this, this, golfRuleStrings + kText_ball, kBallFinalizeShotMessage3, 0, 0, 0, 0);
        IActor_DispatchMessage(this, this, golfRuleStrings + kText_screen, kScreenFinalizeShotMessage7, 0, 0, 0, 0);
        IActor_DispatchMessage(this, this, golfRuleStrings + kText_spin, kSpinFinalizeShotMessage4, 0, 0, 0, 0);
    }
    WPuppet_SetOpacityRecursive(NamedEffectList_FindByName(g_sceneManager, golfRuleStrings + kText_ballSequence), 0xFF);
    Sound_StopByAlias(g_soundManager, golfRuleStrings + kText_shot_dist_count);
    ((CCameraWork*)g_cameraWork)->StopBallSound();
    ((void (*)(void*))GolfRule_AccumulateCurrentShotStatistics)(this);
    // Phase 2: decoded hole-in status selects immediate advancement or stroke policy.
    if ((int)(*(unsigned int*)&((BallHoleInWordsView*)(ballBytes))->encodedValue ^
              ~*(unsigned int*)&((BallHoleInWordsView*)(ballBytes))->key) == 1) {
        F8168_Advance(AsCGolfDoc(GolfDocBytes())->currentPlayer);
        ((CGolfRule*)this)->EnterState(kStateDelayedTransition);
        GolfPlayerEntry(AsCGolfDoc(GolfDocBytes())->currentPlayer);
    } else {
        if (((CGolfBall*)(ballBytes))->m_recoveryRequired) {
            ++GolfPlayerEntry(AsCGolfDoc(GolfDocBytes())->currentPlayer)[0xBC];
            unsigned char* r1 = GolfPlayerEntry(AsCGolfDoc(GolfDocBytes())->currentPlayer);
            ++*(r1 + AsCGolfDoc(GolfDocBytes())->currentHoleIndex + 0x28);
            if (SharedDoc_IsPlayerFlag0Set(AsCGolfDoc(GolfDocBytes())->currentPlayer))
                ((CRival*)g_rival)->SetShotPositionCandidateContext();
        } else {
            if ((int)(*(unsigned int*)&((BallHoleInWordsView*)(ballBytes))->encodedValue ^
                      ~*(unsigned int*)&((BallHoleInWordsView*)(ballBytes))->key) != 1 &&
                !((CGolfBall*)(ballBytes))->m_bounceCount) {
                unsigned char* r2 = GolfPlayerEntry(AsCGolfDoc(GolfDocBytes())->currentPlayer);
                ++*(r2 + AsCGolfDoc(GolfDocBytes())->currentHoleIndex + 0x28);
            }
            unsigned char* r3 = GolfPlayerEntry(AsCGolfDoc(GolfDocBytes())->currentPlayer);
            ((GolfDocSnapshot8013*)(r3))->unk_BC = 0;
        }
        // Phase 3: apply the existing per-rule stroke cap, then choose the wait/result state.
        int value;
        if (*(int*)(GolfPlayerEntry(AsCGolfDoc(GolfDocBytes())->currentPlayer) + 8) == 3)
            value = 0;
        else {
            unsigned char slot = AsCGolfDoc(GolfDocBytes())->currentHoleIndex;
            unsigned char target =
                (unsigned char)((*(unsigned char**)&((CGolfDoc*)(GolfDocBytes()))->holeRecords)[slot * 32 + 0x18] + 4);
            int mode;
            unsigned char cur;
            if (AsCSharedDoc(SharedDocBytes())->golfRuleType == 1) {
                mode = *(int*)(GolfPlayerEntry(AsCGolfDoc(GolfDocBytes())->currentPlayer) + 8);
                unsigned char* rc = GolfPlayerEntry(AsCGolfDoc(GolfDocBytes())->currentPlayer);
                cur = *(rc + AsCGolfDoc(GolfDocBytes())->currentHoleIndex + 0x28);
            } else {
                mode = *(int*)(GolfPlayerEntry(AsCGolfDoc(GolfDocBytes())->currentPlayer) + 8);
                unsigned char* rc = GolfPlayerEntry(AsCGolfDoc(GolfDocBytes())->currentPlayer);
                cur = *(rc + AsCGolfDoc(GolfDocBytes())->currentHoleIndex + 0x28);
            }
            value = mode == 1 ? cur > target : (unsigned)cur >= (unsigned)target;
        }
        if (value) {
            IActor_DispatchMessage(this, this, golfRuleStrings + kText_guideLine, kGuidelineFinalizeShotMessage1, 0, 0,
                                   0, 0);
            unsigned char idx = AsCGolfDoc(GolfDocBytes())->currentPlayer;
            if (idx < AsCGolfDoc(GolfDocBytes())->playerCount) {
                unsigned char t =
                    (unsigned char)((*(unsigned char**)&((CGolfDoc*)(GolfDocBytes()))
                                          ->holeRecords)[AsCGolfDoc(GolfDocBytes())->currentHoleIndex * 32 + 0x18] +
                                    4);
                GolfPlayerEntry(idx)[AsCGolfDoc(GolfDocBytes())->currentHoleIndex + 0x28] = t + 1;
                ++GolfPlayerEntry(idx)[0xB0];
            }
            F8168_Advance(AsCGolfDoc(GolfDocBytes())->currentPlayer);
            ((CGolfRule*)this)
                ->EnterState(AsCSharedDoc(SharedDocBytes())->golfRuleType == 1 ? kStateDelayedTransition
                                                                               : kStateFightResult);
        } else
            ((CGolfRule*)this)
                ->EnterState(AsCSharedDoc(SharedDocBytes())->golfRuleType == 1 ? kStateDelayedTransition
                                                                               : kStateWaitPlayerAndCaddie);
    }
}

// Restore the shot-start ball position and rebuild aiming, gauge and presentation state.
// Requires arg to address a 12-byte world position and live rule/documents/ball.
// Increments this hole's byte stroke count; copies arg to ball, player and sample 0.
// Stores the terrain lookup result unchanged, including the -1 handle bit pattern.
// Resets club/power and rebuilds heading, turn UI and camera before entering Aim.
// Messages keep their source order: screen 0x57, screen 1, five heading recipients,
// player/caddie, power-color UI, clock, turn preparation, then clock restart.
void CGolfRule::ResetToShotStart(void* arg) {
    using namespace ns4;
    float* firstSample = (float*)((CGolfBall*)((float*)GolfBall__Fv()))->GetOrAllocatePreDataAt(0);

    unsigned char* gm = (unsigned char*)g_sharedDoc;
    void* control = (*(void**)&AsCSharedDoc(gm)->golfDoc);
    void* playerEntry =
        ((void* (*)(int, int))GolfPlayer_GetEntryByIndex)((int)control, AsCGolfDoc(control)->currentPlayer);

    unsigned char* g2 = (unsigned char*)g_sharedDoc;
    void* c2 = (*(void**)&AsCSharedDoc(g2)->golfDoc);

    unsigned char* holeStrokeBase = (unsigned char*)playerEntry + (AsCGolfDoc(c2)->currentHoleIndex);
    unsigned char holeStrokes = holeStrokeBase[0x28];
    holeStrokeBase[0x28] = (((holeStrokes + 1)));

    IActor_DispatchMessage(this, this, golfRuleStrings + kText_screen, 0x57, 0, 0, 0, 0);

    *(WVector*)((void*)GolfBall__Fv()) = *(WVector*)arg;

    gm = (unsigned char*)g_sharedDoc;

    control = (*(void**)&AsCSharedDoc(gm)->golfDoc);
    unsigned char byte = AsCGolfDoc(control)->currentPlayer;
    playerEntry = ((void* (*)(int, int))GolfPlayer_GetEntryByIndex)((int)control, (int)byte);
    (*(WVector*)AsGolfDocSnapshot(playerEntry)->ballPosition) = *(WVector*)arg;

    gm = (unsigned char*)g_sharedDoc;
    control = (*(void**)&AsCSharedDoc(gm)->golfDoc);
    byte = AsCGolfDoc(control)->currentPlayer;
    playerEntry = ((void* (*)(int, int))GolfPlayer_GetEntryByIndex)((int)control, (int)byte);
    AsGolfDocSnapshot(playerEntry)->unk_AE = 0;
    *(WVector*)firstSample = *(WVector*)arg;

    void* groundTriangle = (void*)((CQuadTree*)CQuadTree_GetInstance())->GetMeshIndex(arg, 0);
    *(void**)((char*)GolfBall__Fv() + 0x28) = groundTriangle;

    IActor_DispatchMessage(this, this, golfRuleStrings + kText_screen, 1, 0, 0, 0, 0);

    ((CClub*)InstanceTable_Get())->SelectBestClubForHole();

    unsigned char aimTargetReply[12];
    control = (*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc);
    GolfAim_FindTargetAlongPath(aimTargetReply, AsCGolfDoc(control)->aimPath, 0);
    AsGolfRuleFrame(this)->aimTarget = *(WVector*)aimTargetReply;

    float worldForward[3]; // passed as a Vec3 below; three separate scalars left stack14/stack18 in registers
    worldForward[0] = kFloat_0_GolfRule;
    worldForward[1] = kFloat_0_GolfRule;
    worldForward[2] = kFloat_1_805DEA38;

    CGolfDoc* golfDoc = (CGolfDoc*)(*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc);

    WVector ballToTarget = AsGolfRuleFrame(this)->aimTarget - *(WVector*)arg;

    golfDoc->headingRadians = (Vector_SignedAngleXZ(&ballToTarget, worldForward));

    ((ClubSelectionRuntime*)InstanceTable_Get())->powerShotMode = 0;

    float headingReply = golfDoc->headingRadians;
    IActor_DispatchMessage(this, this, golfRuleStrings + kText_camera, 0x2717, (int)&headingReply, 0, 0, 0);
    IActor_DispatchMessage(this, this, golfRuleStrings + kText_spot, 0x2717, (int)&headingReply, 0, 0, 0);
    IActor_DispatchMessage(this, this, golfRuleStrings + kText_screen, 0x2717, (int)&headingReply, 0, 0, 0);
    IActor_DispatchMessage(this, this, golfRuleStrings + kText_grid, 0x2717, (int)&headingReply, 0, 0, 0);
    IActor_DispatchMessage(this, this, golfRuleStrings + kText_caddie, 0x2717, (int)&headingReply, 0, 0, 0);

    IActor_DispatchMessage(this, this, golfRuleStrings + kText_player, 6, 0, 0, 0, 0);
    IActor_DispatchMessage(this, this, golfRuleStrings + kText_caddie, 3, 0, 0, 0, 0);

    float powerColorOff = kFloat_0_GolfRule;
    IActor_DispatchMessage(this, this, golfRuleStrings + kText_spot, 5, 0, 0, 0, 0);
    IActor_DispatchMessage(this, this, golfRuleStrings + kText_screen, 9, (int)&powerColorOff, 0, 0, 0);

    IActor_DispatchMessage(this, this, golfRuleStrings + kText_clock, 0x2719, 0, 0, 0, 0);

    PreparePlayerTurn();
    ((CMouseCursor*)g_mouseCursor)->Reset(0);

    IActor_DispatchMessage(this, this, golfRuleStrings + kText_clock, 0, 0, 0, 0, 0);

    ((CGolfRule*)this)->ResetShotGauge();

    control = (*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc);
    ((CGolfDoc*)control)->UpdateCamera(1);

    {
        int h = NamedEffectList_FindByName(g_sceneManager, golfRuleStrings + kText_ballSequence);
        if (h)
            WPuppet_SetOpacityRecursive(h, 0xFF);
    }

    Sound_StopByAlias(g_soundManager, golfRuleStrings + kText_shot_dist_count);
    Sound_PlayByName(g_soundManager, golfRuleStrings + kText_play_my_turn, 0, 0, 0, 0, kFloat_0_5_GolfRule,
                     kFloat_200_GolfRule, kFloat_1_805DEA38);

    ((CCameraWork*)g_cameraWork)->StopBallSound();
    ((CGolfRule*)this)->EnterState(kStateAim);
}

// Restore a selected shot-history entry, including ball, gauge and player state.
// index selects a PLAYER history, not an entry within one deque. Require a live
// rule/document, playerCount [1,4], valid current hole [0,18), structurally valid
// per-player deques. Negative index selects 0; indices above count-1 select last.
// copy==0 retains current eight item-slot words; ANY nonzero copies saved words.
// Admission: shotUndoEnabled(+93), no shotMeterFadeResetRequested(doc+3B4),
// selected deque.size>0; ordinary party mode 0 also refuses an already-restored
// slot byte rule[25C+slot]. Rejected admission writes nothing. Empty playerCount
// is invalid: the clamp can become -1 before the deque-size load, not a safe miss.
// Snapshot schema (Elem939C, 0x54 bytes): +00 world XYZ, +0C heading radians,
// +11 byte -> player+AE, +14 float -> player+A8, +18/+1C -> player+CC/+D0,
// +20 -> player per-hole word +4C+4*hole; +24..43 eight item words;
// +44..47 inline NUL-terminated club name (at most 3 characters);
// +48/+49 -> strokes/status arrays
// (+28/+3A+hole), +4A -> player+BC, +4C -> player+178. Other bytes are opaque.
// Output ledger after admission, in publication order:
// - Clear active camera-work input accumulator; restore score-record position/AE.
// - Party mode 3 subtracts min(player+1C4,10) from per-hole word +17C+4*hole
//   and player+1C4 (assumes nonnegative balance); other modes restore schema above
//   and clear player state +8. SharedDoc partyGameMode is at +AD8.
// - Optional item copy writes SharedDoc+8C+slot*1D4 (eight words), clears player+A0;
//   always recount nonzero slots into player+9C. Ball and predata[0] take restored
//   XYZ, ground triangle is recomputed, playback cursor=-1; heading is restored.
// - Select saved club, consume exactly ONE back entry; free spare tail blocks
//   as described beside mutation. Build aim target rule+78 from ball + heading.
// - Broadcast heading/player/caddie/HUD resets; clear effects, prepare turn,
//   cursor/gauge/camera, opacity and sound, then enter Aim. Finally clear rule
//   bytes +90/+92/+94 and float +9C, mark rule[25C+slot]=1. No success return,
//   retained snapshot, or exception rollback; callees can leave partial effects.
// Clamps index to the player range; requires a live rule and documents.
// Returns without restoring if undo is disabled, history empty, meter reset active,
// or the per-player restore byte blocks this non-party request.
// Restores the back checkpoint, optionally the eight reserved inventory words;
// selects the saved club before consuming the record and freeing spare tail blocks.
// Rebuilds ball/sample 0, heading, turn UI and camera, then enters Aim.
// Balloon Pop instead deducts up to 10 from the recorded party result balance.
// Terrain lookup results are retained unchanged; no success/failure return value.
void CGolfRule::RestorePreviousShot(int index, int copy) {
    using namespace ns4;
    long slot;
    int restoreAllowed;
    int clampedPlayerIndex;
    unsigned char* game;
    unsigned char* root;
    float headingMatrix[12];

    float zero;

    unsigned char* ruleBytes = (unsigned char*)this;
    root = (unsigned char*)g_sharedDoc;
    game = (*(unsigned char**)&AsCSharedDoc(root)->golfDoc);
    int last = ((CGolfDoc*)(game))->playerCount - 1;

    if (index < 0)
        slot = 0;
    else {
        if (index > last)
            index = last;
        slot = index;
    }

    if (slot < 0)
        clampedPlayerIndex = 0;
    else {
        clampedPlayerIndex = slot;
        if (slot > last)
            clampedPlayerIndex = last;
    }
    restoreAllowed = 1;
    if (!((CGolfRule*)(ruleBytes))->shotUndoEnabled || ((CGolfDoc*)(game))->shotMeterFadeResetRequested ||
        !*(int*)(ruleBytes + clampedPlayerIndex * 0x18 + 0xB0) ||
        (*(int*)&((CSharedDoc*)(root))->partyGameMode == 0 && *(ruleBytes + clampedPlayerIndex + 0x25C)))
        restoreAllowed = 0;
    // Phase 1: refuse restore unless history and current UI state allow it.
    if (0 == restoreAllowed)
        return;
    unsigned char* ui = (unsigned char*)g_cameraWork;
    if (ui[0x1A0]) {
        ui[0x1A0] = 0;
        ui[0x1C] = 0;
        *(float*)(ui + 0x10) = 0.0f;
        Vec2_Zero(ui + 0x14);
    }
    float* ball = GolfBall__Fv();
    void* firstSample = ((CGolfBall*)ball)->GetOrAllocatePreDataAt(0);
    *(WVector*)(P939C + 0x1C) = *(WVector*)(Back939C((((ShotCheckpointDeque*)(ruleBytes + 0xAC)) + slot)));
    ((GolfDocSnapshot8013*)(P939C))->unk_AE =
        ((ShotCheckpoint*)(Back939C((((ShotCheckpointDeque*)(ruleBytes + 0xAC)) + slot))))->unk_PlayerAE;
    if ((*(int*)&AsCSharedDoc(g_sharedDoc)->partyGameMode) == 3) {
        if (*(int*)(P939C + 0x1C4) - 10 > 0) {
            *Cur939C(GolfDocBytes(), slot, 0x17C) -= 10;
            *(int*)(P939C + 0x1C4) -= 10;
        } else {
            unsigned char* p = P939C;
            *Cur939C(GolfDocBytes(), slot, 0x17C) -= *(int*)(p + 0x1C4);
            *(int*)(P939C + 0x1C4) = 0;
        }
    } else {
        ShotCheckpointDeque* history = (((ShotCheckpointDeque*)(ruleBytes + 0xAC)) + slot);
        ((GolfDocSnapshot8013*)(P939C))->powerShotMeter = ((ShotCheckpoint*)(Back939C(history)))->powerShotMeter;
        *(int*)&((GolfDocSnapshot8013*)(P939C))->roundResultWords[1] = ((ShotCheckpoint*)(Back939C(history)))->pang;
        *(int*)&((GolfDocSnapshot8013*)(P939C))->roundResultWords[2] =
            ((ShotCheckpoint*)(Back939C(history)))->bonusPang;
        *Cur939C(GolfDocBytes(), slot, 0x4C) = ((ShotCheckpoint*)(Back939C(history)))->holePang;
        *CurB939C(GolfDocBytes(), slot, 0x28) = ((ShotCheckpoint*)(Back939C(history)))->holeStrokes;
        *CurB939C(GolfDocBytes(), slot, 0x3A) = ((ShotCheckpoint*)(Back939C(history)))->holeResult;
        ((GolfDocSnapshot8013*)(P939C))->unk_BC = ((ShotCheckpoint*)(Back939C(history)))->unk_PlayerBC;
        *(int*)(P939C + 8) = 0;
        *(int*)(P939C + 0x178) = ((ShotCheckpoint*)(Back939C(history)))->partyResult;
    }
    // Phase 2: restore selected history data, preserving the copy option.
    if (copy) {
        *(int*)((unsigned char*)g_sharedDoc + slot * 0x1D4 + 0x8C) =
            ((ShotCheckpoint*)(Back939C((((ShotCheckpointDeque*)(ruleBytes + 0xAC)) + slot))))->reserved[0];
        *(int*)((unsigned char*)g_sharedDoc + slot * 0x1D4 + 0x90) =
            ((ShotCheckpoint*)(Back939C((((ShotCheckpointDeque*)(ruleBytes + 0xAC)) + slot))))->reserved[1];
        *(int*)((unsigned char*)g_sharedDoc + slot * 0x1D4 + 0x94) =
            ((ShotCheckpoint*)(Back939C((((ShotCheckpointDeque*)(ruleBytes + 0xAC)) + slot))))->reserved[2];
        *(int*)((unsigned char*)g_sharedDoc + slot * 0x1D4 + 0x98) =
            ((ShotCheckpoint*)(Back939C((((ShotCheckpointDeque*)(ruleBytes + 0xAC)) + slot))))->reserved[3];
        *(int*)((unsigned char*)g_sharedDoc + slot * 0x1D4 + 0x9C) =
            ((ShotCheckpoint*)(Back939C((((ShotCheckpointDeque*)(ruleBytes + 0xAC)) + slot))))->reserved[4];
        *(int*)((unsigned char*)g_sharedDoc + slot * 0x1D4 + 0xA0) =
            ((ShotCheckpoint*)(Back939C((((ShotCheckpointDeque*)(ruleBytes + 0xAC)) + slot))))->reserved[5];
        *(int*)((unsigned char*)g_sharedDoc + slot * 0x1D4 + 0xA4) =
            ((ShotCheckpoint*)(Back939C((((ShotCheckpointDeque*)(ruleBytes + 0xAC)) + slot))))->reserved[6];
        *(int*)((unsigned char*)g_sharedDoc + slot * 0x1D4 + 0xA8) =
            ((ShotCheckpoint*)(Back939C((((ShotCheckpointDeque*)(ruleBytes + 0xAC)) + slot))))->reserved[7];
        ((GolfDocSnapshot8013*)(P939C))->itemId = 0;
    }
    ((GolfDocSnapshot8013*)(P939C))->equippedItemCount = 0;
    for (int n = 0; n < 8; n++) {
        if (*(int*)((unsigned char*)g_sharedDoc + slot * 0x1D4 + n * 4 + 0x8C))
            ((GolfDocSnapshot8013*)(P939C))->equippedItemCount++;
    }
    Sound_StopByAlias(g_soundManager, golfRuleStrings + kText_shot_dist_count);
    *(WVector*)firstSample = *(WVector*)(P939C + 0x1C);
    *(WVector*)ball = *(WVector*)(P939C + 0x1C);
    ((BallGroundContact*)ball)->triangle = (void*)((CQuadTree*)CQuadTree_GetInstance())->GetMeshIndex(ball, 0);
    AsCGolfBall(ball)->m_eventPreDataIndex[kBallPlaybackCursor] = -1;
    AsCGolfDoc((*(unsigned char**)&AsCSharedDoc(g_sharedDoc)->golfDoc))->headingRadians =
        ((ShotCheckpoint*)(Back939C((((ShotCheckpointDeque*)(ruleBytes + 0xAC)) + slot))))->headingRadians;
    IActor_DispatchMessage(this, this, golfRuleStrings + kText_screen, 1, 0, 0, 0, 0);
    ((ClubSelectionRuntime*)InstanceTable_Get())->powerShotMode = 0;
    // Back iterator/removal schema: d is rule+AC+slot*18, not a snapshot.
    // {start,size,map,mapStart,blocks,mapSize} are six 32-bit words; map is a
    // circular array of pointers to 16 Elem939C records each. Back absolute
    // index=start+size-1, map slot=(mapStart+(index>>4))%mapSize, element=index&15.
    // Select club BEFORE size--. spareTailEntries counts unused elements before
    // the reserved final block: free tail blocks while >=32, retaining slack.
    // Aliases below are identity snapshots of this schema, not extra iterations.
    // Keep their declarations/order to preserve compiler lifetimes and EH labels.
    __typeof__((((ShotCheckpointDeque*)(ruleBytes + 0xAC)) + slot)->start) startBeforeClubSelection =
        (((ShotCheckpointDeque*)(ruleBytes + 0xAC)) + slot)->start;
    ((CClub*)InstanceTable_Get())->SelectBestClubForHole();
    __typeof__((((ShotCheckpointDeque*)(ruleBytes + 0xAC)) + slot)->start +
               (((ShotCheckpointDeque*)(ruleBytes + 0xAC)) + slot)->size) endOffset =
        (((ShotCheckpointDeque*)(ruleBytes + 0xAC)) + slot)->start +
        (((ShotCheckpointDeque*)(ruleBytes + 0xAC)) + slot)->size;
    __typeof__(startBeforeClubSelection + (((ShotCheckpointDeque*)(ruleBytes + 0xAC)) + slot)->size) endOffsetAlias =
        endOffset;
    unsigned int backOffset = endOffsetAlias - 1;
    __typeof__((((ShotCheckpointDeque*)(ruleBytes + 0xAC)) + slot)->blocks) blocksBeforeRemoval =
        (((ShotCheckpointDeque*)(ruleBytes + 0xAC)) + slot)->blocks;
    __typeof__((((ShotCheckpointDeque*)(ruleBytes + 0xAC)) + slot)->blocks) blocksBeforeRemovalAlias =
        blocksBeforeRemoval;
    __typeof__((((ShotCheckpointDeque*)(ruleBytes + 0xAC)) + slot)->mapStart) firstMapSlot =
        (((ShotCheckpointDeque*)(ruleBytes + 0xAC)) + slot)->mapStart;
    __typeof__((((ShotCheckpointDeque*)(ruleBytes + 0xAC)) + slot)->mapStart) firstMapSlotReload =
        (((ShotCheckpointDeque*)(ruleBytes + 0xAC)) + slot)->mapStart;
    __typeof__((((ShotCheckpointDeque*)(ruleBytes + 0xAC)) + slot)->mapStart) firstMapSlotAlias = firstMapSlot;
    __typeof__(firstMapSlotAlias) backMapOrigin = firstMapSlotReload;
    __typeof__((((ShotCheckpointDeque*)(ruleBytes + 0xAC)) + slot)->size) sizeBeforeRemoval =
        (((ShotCheckpointDeque*)(ruleBytes + 0xAC)) + slot)->size;
    unsigned int backElementInBlock = backOffset & 15;
    __typeof__((((ShotCheckpointDeque*)(ruleBytes + 0xAC)) + slot)->size) savedSize = sizeBeforeRemoval;
    __typeof__(blocksBeforeRemovalAlias) blocksBeforeClubLookup =
        (((ShotCheckpointDeque*)(ruleBytes + 0xAC)) + slot)->blocks;
    __typeof__((((ShotCheckpointDeque*)(ruleBytes + 0xAC)) + slot)->start) startBeforeClubLookup =
        (((ShotCheckpointDeque*)(ruleBytes + 0xAC)) + slot)->start;
    __typeof__((((ShotCheckpointDeque*)(ruleBytes + 0xAC)) + slot)->mapSize) mapSlotCount =
        (((ShotCheckpointDeque*)(ruleBytes + 0xAC)) + slot)->mapSize;
    __typeof__((((ShotCheckpointDeque*)(ruleBytes + 0xAC)) + slot)->mapSize) savedMapSlotCount = mapSlotCount;
    // Phase 3: select equipment, consume back snapshot, then restore presentation.
    ClubSelection_SelectByName(
        InstanceTable_Get(), (((ShotCheckpointDeque*)(ruleBytes + 0xAC)) + slot)
                                     ->map[(backMapOrigin + (backOffset >> 4)) % savedMapSlotCount][backElementInBlock]
                                     .b +
                                 0x44);
    (((ShotCheckpointDeque*)(ruleBytes + 0xAC)) + slot)->size = savedSize - 1;
    __typeof__(startBeforeClubLookup) startAfterRemoval = (((ShotCheckpointDeque*)(ruleBytes + 0xAC)) + slot)->start;
    __typeof__((((ShotCheckpointDeque*)(ruleBytes + 0xAC)) + slot)->blocks) blocksAfterRemoval =
        (((ShotCheckpointDeque*)(ruleBytes + 0xAC)) + slot)->blocks;
    __typeof__((((ShotCheckpointDeque*)(ruleBytes + 0xAC)) + slot)->blocks) blocksForSlack =
        (((ShotCheckpointDeque*)(ruleBytes + 0xAC)) + slot)->blocks;
    __typeof__((((ShotCheckpointDeque*)(ruleBytes + 0xAC)) + slot)->blocks) blocksForSlackType =
        (((ShotCheckpointDeque*)(ruleBytes + 0xAC)) + slot)->blocks;
    __typeof__((((ShotCheckpointDeque*)(ruleBytes + 0xAC)) + slot)->start) startForSlack =
        (((ShotCheckpointDeque*)(ruleBytes + 0xAC)) + slot)->start;
    __typeof__(blocksForSlackType) savedBlocksAfterRemoval = blocksAfterRemoval;
    __typeof__(blocksForSlack) savedBlocksForSlack = savedBlocksAfterRemoval;
    bool noBlocks = (((ShotCheckpointDeque*)(ruleBytes + 0xAC)) + slot)->blocks == 0;
    __typeof__(startForSlack) startForSlackReload = (((ShotCheckpointDeque*)(ruleBytes + 0xAC)) + slot)->start;
    __typeof__((((ShotCheckpointDeque*)(ruleBytes + 0xAC)) + slot)->blocks) blocksForSlackReload =
        (((ShotCheckpointDeque*)(ruleBytes + 0xAC)) + slot)->blocks;
    __typeof__(startForSlackReload) savedStart = startAfterRemoval;
    unsigned int spareTailEntries =
        (noBlocks ? 0 : ((((ShotCheckpointDeque*)(ruleBytes + 0xAC)) + slot)->blocks - 1) * 16) -
        (savedStart + (((ShotCheckpointDeque*)(ruleBytes + 0xAC)) + slot)->size);
    while (spareTailEntries >= 32) {
        (((ShotCheckpointDeque*)(ruleBytes + 0xAC)) + slot)->blocks =
            (((ShotCheckpointDeque*)(ruleBytes + 0xAC)) + slot)->blocks - 1;
        __typeof__((((ShotCheckpointDeque*)(ruleBytes + 0xAC)) + slot)->blocks) remainingBlocks =
            (((ShotCheckpointDeque*)(ruleBytes + 0xAC)) + slot)->blocks;
        __typeof__((((ShotCheckpointDeque*)(ruleBytes + 0xAC)) + slot)->mapStart) mapOrigin =
            (((ShotCheckpointDeque*)(ruleBytes + 0xAC)) + slot)->mapStart;
        __typeof__((((ShotCheckpointDeque*)(ruleBytes + 0xAC)) + slot)->map) blockMap =
            (((ShotCheckpointDeque*)(ruleBytes + 0xAC)) + slot)->map;
        __typeof__((((ShotCheckpointDeque*)(ruleBytes + 0xAC)) + slot)->mapStart) savedMapOrigin = mapOrigin;
        Elem939C** map = blockMap;
        unsigned int k =
            (savedMapOrigin + remainingBlocks - 1) % (((ShotCheckpointDeque*)(ruleBytes + 0xAC)) + slot)->mapSize;
        unsigned int savedK = k;
        appFreeUni__FPv(map[savedK]);
        spareTailEntries = spareTailEntries - 16;
        map[k] = 0;
    }
    Matrix_SetAxisRotation(headingMatrix,
                           AsCGolfDoc((*(unsigned char**)&AsCSharedDoc(g_sharedDoc)->golfDoc))->headingRadians, 1);
    unsigned char aimTargetReply[0x10];
    __pl__FRC7WVectorRC7WVector(aimTargetReply, GolfBall__Fv(), headingMatrix + 6);
    *(WVector*)(ruleBytes + 0x78) = *(WVector*)aimTargetReply;
    float angle;
    angle = AsCGolfDoc((*(unsigned char**)&AsCSharedDoc(g_sharedDoc)->golfDoc))->headingRadians;
    IActor_DispatchMessage(this, this, golfRuleStrings + kText_camera, 0x2717, (int)&angle, 0, 0, 0);
    IActor_DispatchMessage(this, this, golfRuleStrings + kText_spot, 0x2717, (int)&angle, 0, 0, 0);
    IActor_DispatchMessage(this, this, golfRuleStrings + kText_screen, 0x2717, (int)&angle, 0, 0, 0);
    IActor_DispatchMessage(this, this, golfRuleStrings + kText_grid, 0x2717, (int)&angle, 0, 0, 0);
    IActor_DispatchMessage(this, this, golfRuleStrings + kText_caddie, 0x2717, (int)&angle, 0, 0, 0);
    IActor_DispatchMessage(this, this, golfRuleStrings + kText_player, 6, 0, 0, 0, 0);
    IActor_DispatchMessage(this, this, golfRuleStrings + kText_caddie, 3, 0, 0, 0, 0);
    zero = 0.0f;
    IActor_DispatchMessage(this, this, golfRuleStrings + kText_spot, 5, 0, 0, 0, 0);
    IActor_DispatchMessage(this, this, golfRuleStrings + kText_screen, 9, (int)&zero, 0, 0, 0);
    IActor_DispatchMessage(this, this, golfRuleStrings + kText_clock, 0x2719, 0, 0, 0, 0);
    IActor_DispatchMessage(this, this, golfRuleStrings + kText_effect, 11, 0, 0, 0, 0);
    IActor_DispatchMessage(this, this, golfRuleStrings + kText_trajectory, 6, 0, 0, 0, 0);
    ((CFx*)g_fx)->ClearEffects(1);
    IActor_DispatchMessage(this, this, golfRuleStrings + kText_score, 5, 0, 0, 0, 0);
    // Phase 4: rebuild turn UI/camera and re-enter aim after clearing transient effects.
    PreparePlayerTurn();
    ((CMouseCursor*)g_mouseCursor)->Reset(0);
    IActor_DispatchMessage(this, this, golfRuleStrings + kText_clock, 0, 0, 0, 0, 0);
    ((CGolfRule*)this)->ResetShotGauge();
    ((CGolfDoc*)((*(unsigned char**)&AsCSharedDoc(g_sharedDoc)->golfDoc)))->UpdateCamera(1);
    int h = NamedEffectList_FindByName(g_sceneManager, golfRuleStrings + kText_ballSequence);
    if (h)
        ((void (*)(int, int))WPuppet_SetOpacityRecursive)(h, 0xFF);
    Sound_StopByAlias(g_soundManager, golfRuleStrings + kText_shot_dist_count);
    ((int (*)(void*, const char*, int, int, int, int, float, float, float))Sound_PlayByName)(
        g_soundManager, golfRuleStrings + kText_play_my_turn, 0, 0, 0, 0, 0.5f, 200.0f, 1.0f);
    ((CCameraWork*)g_cameraWork)->StopBallSound();
    ((CGolfRule*)this)->EnterState(kStateAim);
    ((RuleUnresolvedShotBytes*)(ruleBytes))->unk_094 = 0;
    ((GolfRuleFrameFields*)(ruleBytes))->stateEntryPending = 0;
    ((RuleUnresolvedShotBytes*)(ruleBytes))->unk_092 = 0;
    ((GolfRuleFrameFields*)(ruleBytes))->stateTimer = 0.0f;
    ruleBytes[slot + 0x25C] = 1;
}

namespace ns4 {
extern "C" {
// Return the count-word address used by shot-history resizing.
// Returns the address of the count word in required ring storage.
// The caller may mutate that word; this accessor itself does not change it.
// No bounds check, allocation or failure branch.
void* RingBuffer32_GetCountAddressForResize(void* self) {
    return &((RingWordStorage*)self)->count;
}
}
}

namespace ns4 {
extern "C" {
// Return the capacity-word address used by shot-history insertion.
// Returns the address of the capacity word in required ring storage.
// The caller may mutate that word; this accessor itself does not change it.
// No bounds check, allocation or failure branch.
void* RingBuffer32_GetCapacityAddressForInsert(void* self) {
    return &((RingWordStorage*)self)->capacity;
}
}
}

// ===== S16 - Camera paths, player order and replay =====
// Rule frame fields, sample cursor and presentation classification.

// Evaluate the current course camera-path segment and build its view transform.
// Requires rule camera points/count and a valid camera transform; delta is seconds.
// Accumulates stateTimer, wraps the path cursor and updates camera interpolation.
// Path-point messages refresh the list on the existing branch; preserves the
// float expression order. Count must be >=3; both frame+2 and the wrapped next+2
// must index allocated 12-byte world-space points. Count/cursor/list validity is
// the provider's responsibility, including after the synchronous refresh message.
// A call subtracts at most one 5-second segment: keep accumulated time in [0,10]
// to retain interpolation t in [0,1]. Larger deltas extrapolate; no failure result.
void CGolfRule::UpdateBezierPathCamera(float delta) {
    using namespace ns4;
    float c[3], d[3], q0[3], q1[3], q2[3], e[3], f[3], p0[3], p1[3], p2[3], a[3], b[3], g[3], value[3];
    float result[12];
    unsigned char* object = (unsigned char*)this;
    float time = ((GolfRuleFrameFields*)(object))->stateTimer + delta;
    ((GolfRuleFrameFields*)(object))->stateTimer = time;
    if (time > kFloat_5_GolfRule) {
        double tmp = time - kDouble_5_GolfRule;
        unsigned int count = *(unsigned int*)&((CGolfRule*)(object))->cameraPathPointCount;
        unsigned int frame = s_bezierCameraPointIndex + 2;
        unsigned int last = count - 1;
        s_bezierCameraPointIndex = frame;
        time = (float)tmp;
        ((GolfRuleFrameFields*)(object))->stateTimer = time;
        if (frame >= last) {
            s_bezierCameraPointIndex = 0;
            IActor_DispatchMessage(this, this, golfRuleStrings + kText_golfBg, 4, (int)(object + 0x394), 0, 0, 0);
        }
    }

    unsigned int frame = s_bezierCameraPointIndex;
    float t = ((GolfRuleFrameFields*)(object))->stateTimer / kFloat_5_GolfRule;
    unsigned int nextFrame = frame + 1;
    unsigned int afterNextFrame = frame + 2;
    unsigned char* points = *(unsigned char**)&((CGolfRule*)(object))->cameraPathPoints;
    float t2 = t * t;
    float u = kFloat_1_805DEA38 - t;
    float u2 = u * u;
    unsigned char* nextPoint = points + nextFrame * 12;
    unsigned char* framePoint = points + frame * 12;
    __ml__FfRC7WVector(p0, points + afterNextFrame * 12, t2);
    __ml__FfRC7WVector(p1, nextPoint, kFloat_2_805DEA84 * u * t);
    __ml__FfRC7WVector(p2, framePoint, u2);
    __pl__FRC7WVectorRC7WVector(a, p2, p1);
    __pl__FRC7WVectorRC7WVector(b, a, p0);
    *(WVector*)value = *(WVector*)b;

    unsigned int next = s_bezierCameraPointIndex + 2;
    if (next >= *(unsigned int*)&((CGolfRule*)(object))->cameraPathPointCount - 1)
        next = 0;
    unsigned char* nextPoints = *(unsigned char**)&((CGolfRule*)(object))->cameraPathPoints;
    __ml__FfRC7WVector(q0, nextPoints + (next + 2) * 12, t2);
    __ml__FfRC7WVector(q1, nextPoints + (next + 1) * 12, kFloat_2_805DEA84 * u * t);
    __ml__FfRC7WVector(q2, nextPoints + next * 12, u2);
    __pl__FRC7WVectorRC7WVector(e, q2, q1);
    __pl__FRC7WVectorRC7WVector(f, e, q0);
    *(WVector*)g = *(WVector*)f;
    __mi__FRC7WVectorRC7WVector(d, g, value);
    *(WVector*)g = (*(WVector*)d).Normalize();
    g[1] = kFloat_neg0_1_GolfRule;

    unsigned int count = *(unsigned int*)&((CGolfRule*)(object))->cameraPathPointCount;
    unsigned int half = (count - 1) >> 1;
    if (s_bezierCameraPointIndex == 0 || s_bezierCameraPointIndex == half - 4 || s_bezierCameraPointIndex == half) {
        g[1] -= kFloat_0_8_GolfRule - ((GolfRuleFrameFields*)(object))->stateTimer / kFloat_6_25_GolfRule;
    } else if (s_bezierCameraPointIndex == half - 6 || s_bezierCameraPointIndex == half - 2 ||
               s_bezierCameraPointIndex == count - 3) {
        g[1] -= ((GolfRuleFrameFields*)(object))->stateTimer / kFloat_6_25_GolfRule;
    }
    float clamped = g[1];
    if (clamped < kFloat_neg0_9_BezierPathCamera)
        clamped = kFloat_neg0_9_BezierPathCamera;
    else if (clamped > kFloat_neg0_1_GolfRule)
        clamped = kFloat_neg0_1_GolfRule;
    g[1] = clamped;
    __pl__FRC7WVectorRC7WVector(c, value, g);
    Matrix_BuildCameraTransform(result, value, c);
    Matrix_CopyAffine(s_cameraTransform, result);
}

// Select the next eligible player according to turn order, distance and game rules.
// Requires player/document state. Normal solo Stroke returns 0; party modes search
// the next turn-order value among state-0 players and may return playerCount.
// Match and Stroke use their separate eligibility/ordering rules, then horizontal
// distance to pin. arg excludes current player in the ordinary distance branch.
// No mutation of player positions.
int CGolfRule::SelectNextPlayer(int arg) {
    using namespace ns4;
    unsigned char* cursor;
    unsigned int count;
    Fn8016A4C0Vec d1, d0, d, t0;
    Arr3 s0;
    Fn8016A4C0Vec t1;
    Arr3 s1, s;
    Fn8016A4C0Vec d2;
    Arr3 s2;
    unsigned char* manager = (unsigned char*)g_sharedDoc;
    unsigned int result;
    if ((*(int*)&AsCSharedDoc(manager)->gameMode) == 0 && ((CSharedDoc*)(manager))->golfRuleType == 0)
        return 0;
    result = 0;
    // Phase 1: apply party-mode selection rules before ordinary ordering.
    if ((*(int*)&AsCSharedDoc(manager)->partyGameMode) != 0) {
        count = AsCGolfDoc(GolfDocBytes())->playerCount;
        cursor = (unsigned char*)(GolfPlayerEntryByte(AsCGolfDoc(GolfDocBytes())->currentPlayer)[0xA4] + 1);
        if ((int)cursor >= (int)count)
            cursor = 0;
        for (arg = result; arg < (int)count; arg++) {
            if ((int)cursor == ((GolfDocSnapshot8013*)(GolfPlayerEntryByte(arg)))->turnOrder) {
                if (*(int*)(GolfPlayerEntryByte(arg) + 8) != 0) {
                    cursor++;
                    if ((int)cursor >= (int)count)
                        cursor = 0;
                    arg = -1;
                } else {
                    return (unsigned char)arg;
                }
            }
        }
        return count;
    }
    // Phase 2: use the golf-rule-specific ordering and eligibility comparisons.
    if (((CSharedDoc*)(manager))->golfRuleType == 1) {
        int found;
        unsigned int i;
        /* Search at done: stop on a match or exhausted input. */
        for (i = 0; ((unsigned char)i < AsCGolfDoc(GolfDocBytes())->playerCount) || (found = 0, false); i++) {
            if (*(int*)(GolfPlayerEntryByte(i) + 8) == 0) {
                unsigned char idx = AsCGolfDoc(GolfDocBytes())->currentHoleIndex;
                if (GolfPlayerEntryByte(i)[idx + 0x28] == 0) {
                    found = 1;
                    break;
                }
            }
        }
        if (found) {
            result = turn ^ 1;
        } else {
            int freeCount = 0;
            for (int j = 0; j < AsCGolfDoc(GolfDocBytes())->playerCount; j++) {
                if (*(int*)(GolfPlayerEntryByte(j) + 8) == 0) {
                    result = (unsigned char)j;
                    freeCount++;
                }
            }
            if (freeCount == 2) {
                unsigned char* a = GolfPlayerEntryByte(0);
                s0 = *(Arr3*)(*(unsigned char**)&((CGolfDoc*)(GolfDocBytes()))->holeRecords +
                              (AsCGolfDoc(GolfDocBytes())->currentHoleIndex << 5) + 0xC);
                __mi__FRC7WVectorRC7WVector(&t0, &s0, a + 0x1C);
                __as__7WVectorFRC7WVector(&d0, &t0);
                d0.y = kFloat_0_GolfRule;
                unsigned char* b = GolfPlayerEntryByte(1);
                s1 = *(Arr3*)(*(unsigned char**)&((CGolfDoc*)(GolfDocBytes()))->holeRecords +
                              (AsCGolfDoc(GolfDocBytes())->currentHoleIndex << 5) + 0xC);
                __mi__FRC7WVectorRC7WVector(&t1, &s1, b + 0x1C);
                __as__7WVectorFRC7WVector(&d1, &t1);
                d1.y = kFloat_0_GolfRule;
                float lb = Magnitude__7WVectorCFv(&d1);
                if (Magnitude__7WVectorCFv(&d0) > lb)
                    result = 0;
                else
                    result = 1;
            } else if (freeCount == 0) {
                return AsCGolfDoc(GolfDocBytes())->currentPlayer;
            }
        }
    } else {
        if (AsCGolfDoc(GolfDocBytes())->playerCount == 1)
            return 0;
        unsigned char idx;
        int found;
        unsigned int k;
        /* Search at done2: stop on a match or exhausted input. */
        for (k = 0; ((unsigned char)k < AsCGolfDoc(GolfDocBytes())->playerCount) || (found = 0, false); k++) {
            if (*(int*)(GolfPlayerEntryByte(k) + 8) == 0) {
                idx = AsCGolfDoc(GolfDocBytes())->currentHoleIndex;
                if (GolfPlayerEntryByte(k)[idx + 0x28] == 0) {
                    found = 1;
                    break;
                }
            }
        }
        if (found) {
            unsigned int next = GolfPlayerEntryByte(AsCGolfDoc(GolfDocBytes())->currentPlayer)[0xA4];
            for (; (unsigned char)result < AsCGolfDoc(GolfDocBytes())->playerCount; result++) {
                if (*(int*)(GolfPlayerEntryByte(result) + 8) != 3) {
                    unsigned char* candidate = GolfPlayerEntryByte(result);
                    int expected = (unsigned char)next + 1;
                    if (expected == ((GolfDocSnapshot8013*)(candidate))->turnOrder) {
                        if (*(int*)(GolfPlayerEntryByte(result) + 8) == 0)
                            break;
                        result = 0xFF;
                        next++;
                    }
                }
            }
        } else {
            for (; (unsigned char)result < AsCGolfDoc(GolfDocBytes())->playerCount; result++) {
                if (*(int*)(GolfPlayerEntryByte(result) + 8) == 0) {
                    if (arg == 0)
                        break;
                    if ((unsigned char)result != AsCGolfDoc(GolfDocBytes())->currentPlayer)
                        break;
                }
            }
            if ((unsigned char)result == AsCGolfDoc(GolfDocBytes())->playerCount)
                return AsCGolfDoc(GolfDocBytes())->currentPlayer;
            unsigned char* p = GolfPlayerEntryByte(result);
            s = *(Arr3*)(*(unsigned char**)&((CGolfDoc*)(GolfDocBytes()))->holeRecords +
                         (AsCGolfDoc(GolfDocBytes())->currentHoleIndex << 5) + 0xC);
            __mi__FRC7WVectorRC7WVector(&d, &s, p + 0x1C);
            d.y = kFloat_0_GolfRule;
            float zero;
            float best = Magnitude__7WVectorCFv(&d);
            zero = kFloat_0_GolfRule;
            for (unsigned int j = 0; (unsigned char)j < AsCGolfDoc(GolfDocBytes())->playerCount; j++) {
                if (*(int*)(GolfPlayerEntryByte(j) + 8) == 0 && (unsigned char)j != (unsigned char)result &&
                    (arg == 0 || (unsigned char)j != AsCGolfDoc(GolfDocBytes())->currentPlayer)) {
                    unsigned char* q = GolfPlayerEntryByte(j);
                    s2 = *(Arr3*)(*(unsigned char**)&((CGolfDoc*)(GolfDocBytes()))->holeRecords +
                                  (AsCGolfDoc(GolfDocBytes())->currentHoleIndex << 5) + 0xC);
                    __mi__FRC7WVectorRC7WVector(&d2, &s2, q + 0x1C);
                    __as__7WVectorFRC7WVector(&d, &d2);
                    d.y = zero;
                    float l = Magnitude__7WVectorCFv(&d);
                    if (l > best) {
                        result = j;
                        best = l;
                    }
                }
            }
        }
    }
    return result;
}

// Set up playback of the final thirty simulated frames with the appropriate camera.
// Requires a history long enough for the existing final-30-sample cursor.
// Writes cursor/state and playback camera; sample indices are 30 Hz ticks,
// and the quadtree frame synchronizes animated objects to this playback point.
void CGolfRule::SetupLast30FramesPlayback() {
    using namespace ns4;
    Fn8016AA98Vec eye;
    int time;
    float mat180[12], mat150[12], mat120[12], matF0[12];
    Fn8016AA98Vec dir;
    Fn8016AA98Vec ground;
    Fn8016AA98Vec delta;
    Fn8016AA98Words pB0;
    Fn8016AA98Vec sumA4;
    Fn8016AA98Words p98;
    Fn8016AA98Vec rotated8C, scaled80, hit74;
    Fn8016AA98Words copy68, p5C;
    Fn8016AA98Vec sum50;
    Fn8016AA98Words p44;
    Fn8016AA98Vec rotated38, scaled2C, hit20;
    Fn8016AA98Words copy14, p08;

    float* state = GolfBall__Fv();
    AsCGolfBall(state)->m_eventPreDataIndex[kBallPlaybackCursor] =
        AsCGolfBall(state)->m_preDataCount - 30; // original +0x68/+0x64
    *(WVector*)state =
        *(WVector*)(PreData__9CGolfBallFi(state, AsCGolfBall(state)->m_eventPreDataIndex[kBallPlaybackCursor]));
    CopyShotSampleVelocity(state,
                           PreData__9CGolfBallFi(state, AsCGolfBall(state)->m_eventPreDataIndex[kBallPlaybackCursor]));
    AsCGolfBall(state)->m_state =
        ((ShotSample*)PreData__9CGolfBallFi(state, AsCGolfBall(state)->m_eventPreDataIndex[kBallPlaybackCursor]))
            ->motionState; // original +0x18/+0x68
    time = ((ShotWorldClockView*)CQuadTree_GetInstance())->frame;
    ShotWorldClockFields* timer = (ShotWorldClockFields*)CQuadTree_GetInstance();
    timer->frame = (int)time - 30;
    timer->fractionalSeconds = kFloat_0_GolfRule;
    unsigned char* ctl = (*(unsigned char**)&AsCSharedDoc(g_sharedDoc)->golfDoc);

    unsigned char* slot =
        (*(unsigned char**)&AsCGolfDoc(ctl)->holeRecords) + ((unsigned int)(AsCGolfDoc(ctl)->currentHoleIndex) << 5);
    pB0 = *(Fn8016AA98Words*)(slot + 0x0C);
    __mi__FRC7WVectorRC7WVector(&delta, &pB0, PreData__9CGolfBallFi(state, 0));
    *(WVector*)&dir = *(WVector*)&delta;
    dir.y = kFloat_0_GolfRule;
    (*(WVector*)&dir).Normalize();
    switch (presentationStep) {
    case 1:
        ctl = (*(unsigned char**)&AsCSharedDoc(g_sharedDoc)->golfDoc);
        slot =
            (*(unsigned char**)&AsCGolfDoc(ctl)->holeRecords) + ((unsigned int)AsCGolfDoc(ctl)->currentHoleIndex << 5);
        p98 = *(Fn8016AA98Words*)(slot + 0xC);
        Matrix_SetAxisRotation(mat180, kFloat_0_7853982_GolfRuleCamera, 1);
        __ml__FfRC7WVector(&scaled80, &dir, kFloat_16_GolfRule);
        Matrix_TransformPoint(&rotated8C, &scaled80, mat180);
        __pl__FRC7WVectorRC7WVector(&sumA4, &p98, &rotated8C);
        *(WVector*)&eye = *(WVector*)&sumA4;
        eye.y += kFloat_9_6_GolfPlayerHelpers;
        copy68 = *(Fn8016AA98Words*)&eye;
        GetGroundPoint__9CQuadTreeF7WVectorbPib(&hit74, CQuadTree_GetInstance(), &copy68, 0, 0, 0);
        *(WVector*)&ground = *(WVector*)&hit74;
        ground.y = (ground.y + kFloat_9_6_GolfPlayerHelpers);
        if (ground.y >= eye.y)
            *(WVector*)&eye = *(WVector*)&ground;
        ctl = (*(unsigned char**)&AsCSharedDoc(g_sharedDoc)->golfDoc);
        slot =
            (*(unsigned char**)&AsCGolfDoc(ctl)->holeRecords) + ((unsigned int)AsCGolfDoc(ctl)->currentHoleIndex << 5);
        p5C = *(Fn8016AA98Words*)(slot + 0xC);
        Matrix_BuildCameraTransform(mat150, &eye, &p5C);
        Matrix_CopyAffine(s_cameraTransform, mat150);
        Matrix_CopyAffine(AsCCameraWork(g_cameraWork)->previousTransform, s_cameraTransform);
        break;
    case 2:
        ctl = (*(unsigned char**)&AsCSharedDoc(g_sharedDoc)->golfDoc);
        slot =
            (*(unsigned char**)&AsCGolfDoc(ctl)->holeRecords) + ((unsigned int)AsCGolfDoc(ctl)->currentHoleIndex << 5);
        p44 = *(Fn8016AA98Words*)(slot + 0xC);
        Matrix_SetAxisRotation(mat120, kFloat_neg0_7853982_GolfRule, 1);
        __ml__FfRC7WVector(&scaled2C, &dir, kFloat_9_6_GolfPlayerHelpers);
        Matrix_TransformPoint(&rotated38, &scaled2C, mat120);
        __pl__FRC7WVectorRC7WVector(&sum50, &p44, &rotated38);
        *(WVector*)&eye = *(WVector*)&sum50;
        copy14 = *(Fn8016AA98Words*)&eye;
        GetGroundPoint__9CQuadTreeF7WVectorbPib(&hit20, CQuadTree_GetInstance(), &copy14, 0, 0, 0);
        *(WVector*)&ground = *(WVector*)&hit20;
        ground.y += kFloat_3_2_GolfRule;
        if (ground.y >= eye.y)
            *(WVector*)&eye = *(WVector*)&ground;
        ctl = (*(unsigned char**)&AsCSharedDoc(g_sharedDoc)->golfDoc);
        slot =
            (*(unsigned char**)&AsCGolfDoc(ctl)->holeRecords) + ((unsigned int)AsCGolfDoc(ctl)->currentHoleIndex << 5);
        p08 = *(Fn8016AA98Words*)(slot + 0xC);
        Matrix_BuildCameraTransform(matF0, &eye, &p08);
        Matrix_CopyAffine(s_cameraTransform, matF0);
        ((float*)s_cameraTransform)[10] += kFloat_1_805DEA38;
        Matrix_CopyAffine(AsCCameraWork(g_cameraWork)->previousTransform, s_cameraTransform);
        break;
    }
}

#pragma opt_lifetimes off
#pragma use_lmw_stmw on
namespace ns4 {
extern "C" {
// CGolfDoc +324 is an affine camera-to-world frame, four rows of three f32s:
// right +324, up +330, forward +33C, eye +348 (total matrix size 0x30).
// Same storage as CGolfDoc::cameraMatrix and camera_input_and_ground_view.cpp's GolfDocCameraFields.
// Producer: golf_doc_turns_and_parameter_xml.cpp CGolfDoc::UpdateCamera builds the basis from the
// ball and its horizontal heading direction, then adjusts only the eye row.
// Replay uses its WORLD-SPACE up/forward basis directions, not two positions,
// and ignores the saved eye row. No ownership or matrix copy is implied by this view.
struct PlaybackCameraDocumentFrame {
    char prefix[0x324];
    float right[3], up[3], forward[3], eye[3];
};
typedef char PlaybackCameraUpOffset[(int)&((PlaybackCameraDocumentFrame*)0)->up == 0x330 ? 1 : -1];
typedef char PlaybackCameraForwardOffset[(int)&((PlaybackCameraDocumentFrame*)0)->forward == 0x33C ? 1 : -1];
typedef char PlaybackCameraEyeOffset[(int)&((PlaybackCameraDocumentFrame*)0)->eye == 0x348 ? 1 : -1];
}
}

// Restore the first simulated ball sample and set up the selected playback camera.
// Requires sample 0, completed history and live camera/doc state.
// Restores playback presentation and clock, then installs the selected camera.
// All positions/offsets use world XYZ (Y up); the document basis is dimensionless.
// presentationStep 1: eye = ball + RotateY(+120deg, 32*(up+forward)); target = ball.
// presentationStep 2: eye = ball + RotateY(-110deg, 32*(0.5*up+forward));
//                     target = ball + (0,2,0).
// Other values keep the existing camera after restoring the ball/clock/effects.
// With the producer's horizontal forward and vertical up, 32 gives 32 world
// units of horizontal standoff in either branch (10 yards); type 1 rises 32,
// type 2 rises 16. The sum is NOT normalized; 32 is not its diagonal length.
// The 0.5 halves only type 2's up contribution. The 2 raises its look-at point
// by 2 world units. These constants share the unit's float pool: a later camera
// mod should introduce separate settings instead of changing all pool consumers.
// Safe tuning ranges are unestablished; keep eye != target and retain all
// add/scale/transform/copy steps and operands to preserve floating-point rounding.
// Matrix scratch is 12 f32s: the same row layout as the document frame above.
// Axis 1 is world Y; its rotation has zero translation. TransformPoint still
// uses w=1; do not substitute TransformDirection or fold/reorder the chain.
// BuildCameraTransform writes [right,up,forward,eye], a camera-to-world pose,
// not a world-to-view inverse; CopyAffine publishes it to s_cameraTransform.
void CGolfRule::StartShotPlaybackCamera() {
    using namespace ns4;
    void* firstSample;
    void* ball;
    ball = GolfBall__Fv();
    firstSample = PreData__9CGolfBallFi(ball, 0);
    IActor_DispatchMessage(this, this, golfRuleStrings + kText_effect, 0xB, 0, 0, 0, 0);
    IActor_DispatchMessage(this, this, golfRuleStrings + kText_effect, 0xC, 0, 0, 0, 0);
    IActor_DispatchMessage(this, this, golfRuleStrings + kText_trajectory, 6, 0, 0, 0, 0);
    ((CFx*)g_fx)->ClearEffects(1);
    IActor_DispatchMessage(this, this, golfRuleStrings + kText_player, 1, 0, 0, 0, 0);
    float initialElapsedSeconds = kFloat_1_805DEA38;
    animationStarted = 0;
    int shotAnimationState = 0x40;
    elapsedTime = initialElapsedSeconds;
    unsigned char* sharedDocBytes = (unsigned char*)g_sharedDoc;
    AsCGolfDoc((*(unsigned char**)&AsCSharedDoc(sharedDocBytes)->golfDoc))->gameStateFlags = shotAnimationState;
    // This temporary carries frame bits, never an address to dereference. Keep
    // the inherited pointer-shaped cast across the singleton call for code matching.
    unsigned int* playbackFrameBits = ((unsigned int*)playbackFrame);
    unsigned char* worldBytes = (unsigned char*)CQuadTree_GetInstance();
    *(unsigned int*)&((ShotWorldClockView*)worldBytes)->frame = (unsigned int)playbackFrameBits;
    ((ShotWorldClockView*)worldBytes)->fractionalSeconds = kFloat_0_GolfRule;
    AsCGolfBall(ball)->m_eventPreDataIndex[kBallPlaybackCursor] = 0;
    __as__7WVectorFRC7WVector(ball, firstSample);
    __as__7WVectorFRC7WVector(&AsCGolfBall(ball)->m_velocity, &((ShotSample*)firstSample)->velocity);
    {
        float* accessoryModifiers = (float*)PartyItemSlots_GetInstance();
        AsCGolfBall(ball)->m_velocity.y *= (kFloat_1_805DEA38 + accessoryModifiers[9]);
    }
    AsCGolfBall(ball)->m_state = ((ShotSample*)firstSample)->motionState;
    int step = presentationStep;
    if (step == 1) {
        sharedDocBytes = (unsigned char*)g_sharedDoc;
        unsigned char* golfDocBytes = (*(unsigned char**)&AsCSharedDoc(sharedDocBytes)->golfDoc);

        // Up/forward -> scaled world offset -> yawed offset -> eye -> look-at pose.
        float yawRotation[12];
        Matrix_SetAxisRotation(yawRotation, kFloat_2_0943952_GolfRule, 1);
        float upPlusForward[3];
        __pl__FRC7WVectorRC7WVector(upPlusForward, ((PlaybackCameraDocumentFrame*)golfDocBytes)->up,
                                    ((PlaybackCameraDocumentFrame*)golfDocBytes)->forward);
        float scaledCameraOffset[3];
        __ml__FfRC7WVector(scaledCameraOffset, upPlusForward, kFloat_32_GolfRule);
        float rotatedCameraOffset[3];
        Matrix_TransformPoint(rotatedCameraOffset, scaledCameraOffset, yawRotation);
        float eyeSum[3];
        __pl__FRC7WVectorRC7WVector(eyeSum, ball, rotatedCameraOffset);
        float eyePosition[3];
        __as__7WVectorFRC7WVector(eyePosition, eyeSum);
        float cameraPose[12];
        Matrix_BuildCameraTransform(cameraPose, eyePosition, ball);
        Matrix_CopyAffine(s_cameraTransform, cameraPose);
    } else if (step == 2) {
        sharedDocBytes = (unsigned char*)g_sharedDoc;
        unsigned char* golfDocBytes = (*(unsigned char**)&AsCSharedDoc(sharedDocBytes)->golfDoc);

        // Half up + forward -> offset -> yaw -> eye; target is raised separately.
        float yawRotation[12];
        Matrix_SetAxisRotation(yawRotation, kFloat_neg1_9198622_GolfRuleCamera, 1);
        float halfUp[3];
        __ml__FfRC7WVector(halfUp, ((PlaybackCameraDocumentFrame*)golfDocBytes)->up, kFloat_0_5_GolfRule);
        float halfUpPlusForward[3];
        __pl__FRC7WVectorRC7WVector(halfUpPlusForward, halfUp, ((PlaybackCameraDocumentFrame*)golfDocBytes)->forward);
        float scaledCameraOffset[3];
        __ml__FfRC7WVector(scaledCameraOffset, halfUpPlusForward, kFloat_32_GolfRule);
        float rotatedCameraOffset[3];
        Matrix_TransformPoint(rotatedCameraOffset, scaledCameraOffset, yawRotation);
        float eyeSum[3];
        __pl__FRC7WVectorRC7WVector(eyeSum, ball, rotatedCameraOffset);
        float eyePosition[3];
        __as__7WVectorFRC7WVector(eyePosition, eyeSum);
        float targetRise[3];
        float lookAtPosition[3];
        targetRise[0] = kFloat_0_GolfRule;
        targetRise[1] = kFloat_2_805DEA84;
        targetRise[2] = kFloat_0_GolfRule;
        __pl__FRC7WVectorRC7WVector(lookAtPosition, ball, targetRise);
        float cameraPose[12];
        Matrix_BuildCameraTransform(cameraPose, eyePosition, lookAtPosition);
        Matrix_CopyAffine(s_cameraTransform, cameraPose);
    }
}

#pragma opt_lifetimes reset
#pragma use_lmw_stmw reset
// Set presentation flags from shot distance, special-shot bits, impact and landing state.
// Requires live ball/doc/player/club and current-hole records.
// Writes presentation flag bits from distance, impact, hole-in and recovery state.
// This selects playback presentation; it does not integrate the ball.
void CGolfRule::ClassifyShotPresentation() {
    using namespace ns4;
    shotPresentationFlags = 0;
    if ((*(int*)&AsCSharedDoc(g_sharedDoc)->partyGameMode) != 0)
        return;
    if ((unsigned char)SharedDoc_GetCourseId() == 7)
        return;
    unsigned char* control = (*(unsigned char**)&AsCSharedDoc(g_sharedDoc)->golfDoc);
    int mode = AsCGolfDoc(control)->tutorialStage;
    if (mode < 6) {
        if (mode != 5)
            return;
    }
    unsigned char index = AsCGolfDoc(control)->currentHoleIndex;
    unsigned int* record = (*(unsigned int**)&AsCGolfDoc(control)->holeRecords) + index * 8;
    float vec[3];
    unsigned int pos[3] = {record[3], record[4], record[5]};
    void* value = (void*)PreData__9CGolfBallFi(GolfBall__Fv(), 0);
    __mi__FRC7WVectorRC7WVector(vec, pos, value);
    vec[1] = kFloat_0_GolfRule;
    float distance = (*(WVector*)vec).Magnitude();
    if (((unsigned int)AsCGolfBall(GolfBall__Fv())->m_powerFlags & kSpecialShotMask) != 0)
        shotPresentationFlags |= 2;
    if (((ClubSelectionRuntime*)InstanceTable_Get())->powerShotMode != 0) {
        control = (*(unsigned char**)&AsCSharedDoc(g_sharedDoc)->golfDoc);
        if (AsCGolfDoc(control)->impactGrade == kImpactPangya && ((GolfDocShotBytes*)control)->unk_2D8)
            shotPresentationFlags |= 2;
    }
    unsigned char* ballBytes = (unsigned char*)GolfBall__Fv();
    int _eqv;
    _eqv = ~(((BallHoleInWordsView*)ballBytes)->encodedValue ^ ((BallHoleInWordsView*)ballBytes)->key);
    if (_eqv == 1) {
        unsigned char* anim = (unsigned char*)InstanceTable_Get();
        unsigned char* animObj = *(unsigned char**)(anim + 4);
        if (((SelectedClubRuntime*)animObj)->clubKind == 3 && distance > kFloat_64_GolfRule)
            shotPresentationFlags |= 4;
        if (distance > kFloat_320_GolfRule)
            shotPresentationFlags |= 4;
        unsigned int flags = *(unsigned int*)&shotPresentationFlags;
        if (!(flags & 4)) {
            if (distance > kFloat_121_6_GolfRule && distance < kFloat_320_GolfRule)
                shotPresentationFlags |= 1;
            if (((unsigned int)AsCGolfBall(GolfBall__Fv())->m_flags & 3) != 0)
                shotPresentationFlags |= 1;
        }
    }
    ballBytes = (unsigned char*)GolfBall__Fv();
    if (AsCGolfBall(ballBytes)->m_recoveryRequired)
        shotPresentationFlags &= ~4; // original +0x58
    if (((ClubSelectionRuntime*)InstanceTable_Get())->powerShotMode == 2 ||
        (float)AsCGolfBall(GolfBall__Fv())->m_preDataCount < kFloat_9_GolfRule)
        shotPresentationFlags &= ~2;
}

// ===== S17 - Gauge commit, reset and mode exit =====
// Document gauges, power meter and state-dialog callbacks.

// Commit the gauge result through network synchronization or start the shot animation.
// Requires live rule, ball, documents, club runtime and quadtree clock.
// Reads power/impact X in screen units (149 start, 360 span).
// Publishes network wait in replay/network mode; otherwise enters shot animation.
// Enables effects first. No failure return.
void CGolfRule::CommitShotGauge() {
    using namespace ns4;
    ((CFx*)g_fx)->SetUpdateEnabled(1);
    if (GolfTask_NoOpStatus()) {
        float* point = GolfBall__Fv();

        int vals[16];
        *(float*)((2 + vals)) = AsCGolfBall(point)->m_initCurve; // original +0x34
        point = GolfBall__Fv();
        *(float*)((3 + vals)) = AsCGolfBall(point)->m_initSpin; // original +0x38
        point = GolfBall__Fv();
        unsigned char* manager = (unsigned char*)g_sharedDoc;
        vals[5] = AsCGolfBall(point)->m_powerFlags; // original +0x44
        *(float*)(vals + 1) = AsCGolfDoc((*(unsigned char**)&AsCSharedDoc(manager)->golfDoc))->shotImpactGaugeX;
        *(float*)(vals + 0) = AsCGolfDoc((*(unsigned char**)&AsCSharedDoc(manager)->golfDoc))->shotPowerGaugeX;
        ((unsigned char*)vals)[0x10] =
            (unsigned char)AsCGolfDoc((*(unsigned char**)&AsCSharedDoc(manager)->golfDoc))->impactGrade;
        *(float*)(((9 + vals))) = AsCGolfDoc((*(unsigned char**)&AsCSharedDoc(manager)->golfDoc))->pangyaTargetX;
        vals[8] = (*(int*)&AsCGolfDoc((*(unsigned char**)&AsCSharedDoc(manager)->golfDoc))->elapsedTimeMs);
        vals[6] = ((ShotWorldClockView*)CQuadTree_GetInstance())->frame;
        IActor_DispatchMessage(this, this, golfRuleStrings + kText_player, 3, (int)((vals + 7)), 0, 0, 0);
        unsigned char* anim = (unsigned char*)InstanceTable_Get();
        unsigned char* obj = *(unsigned char**)(4 + anim);
        ((unsigned char*)vals)[0x28] = obj[9];
        anim = (unsigned char*)InstanceTable_Get();
        (*(float*)&vals[11]) = *(float*)(anim + 0x70);
        anim = (unsigned char*)InstanceTable_Get();
        anim[0x74] = 0;

        (*(float*)&vals[12]) = launchHeadingOffset;

        GolfRule_StartNetworkWait(this, 1, 0x3628, GolfRuleWaitMessage(golfRuleStrings + kText_hitShot),
                                  kFloat_20_GolfRule);
    } else {
        EnterState(kStateButtonShotAnimation);
    }
}

// Reset power, impact, Pangya assistance and gauge direction for a new shot.
// Requires live rule/doc/player/ball plus sound/input/camera recipients.
// Resets gauge X (149 start; cursor 114 for non-putter), phase and assistance,
// clears hole-in encoding and updates actor presentation. On green, rand()%2
// selects putting BGM; other ground uses the selected list entry. No failure return.
void CGolfRule::ResetShotGauge() {
    using namespace ns4;
    void* s = this;
    char* strings = golfRuleStrings;
    IActor_DispatchMessage(s, s, strings + kText_spin, 0x2719, 0, 0, 0, 0);
    IActor_DispatchMessage(s, s, strings + kText_grid, 2, 0, 0, 0, 0);
    IActor_DispatchMessage(s, s, strings + kText_ball, 0, 0, 0, 0, 0);
    IActor_DispatchMessage(s, s, strings + kText_ball, 2, 0, 0, 0, 0);
    IActor_DispatchMessage(s, s, strings + kText_ball, 4, 0, 0, 0, 0);
    IActor_DispatchMessage(s, s, strings + kText_screen, 14, 0, 0, 0, 0);
    IActor_DispatchMessage(s, s, strings + kText_item, 11, 0, 0, 0, 0);
    IActor_DispatchMessage(s, s, strings + kText_ball, 6, 0, 0, 0, 0);
    IActor_DispatchMessage(s, s, strings + kText_score, 4, 0, 0, 0, 0);
    IActor_DispatchMessage(s, s, strings + kText_guideLine, 2, 0, 0, 0, 0);
    IActor_DispatchMessage(s, s, strings + kText_npcActor, 0, 0, 0, 0, 0);
    IActor_DispatchMessage(s, s, strings + kText_caddie, 3, 0, 0, 0, 0);
    unsigned char index = AsCGolfDoc((*(unsigned char**)&AsCSharedDoc(g_sharedDoc)->golfDoc))->currentPlayer;
    IActor_DispatchMessage(s, s, strings + kText_caddie, 22,
                           *(int*)((char*)g_sharedDoc + (unsigned int)index * 0x1D4 + 0x1B8),
                           *(int*)((char*)g_sharedDoc + (unsigned int)index * 0x1D4 + 0x1BC), 0, 0);
    float v1 = kFloat_1_805DEA38;
    float v114 = kFloat_114_GolfRule;
    float v149 = kFloat_149_805DEA58;
    unsigned char* p = (*(unsigned char**)&AsCSharedDoc(g_sharedDoc)->golfDoc);
    AsCGolfDoc(p)->shotPhase = 0;
    p = (*(unsigned char**)&AsCSharedDoc(g_sharedDoc)->golfDoc);
    AsCGolfDoc(p)->alternateMotionPowerDecided = 0;
    p = (*(unsigned char**)&AsCSharedDoc(g_sharedDoc)->golfDoc);
    AsCGolfDoc(p)->pangyaTargetX = v149;
    AsCGolfDoc(p)->shotPowerGaugeX = v149;
    AsCGolfDoc(p)->shotImpactGaugeX = v149;
    AsCGolfDoc(p)->shotGaugeCursorX = v114;
    AsCGolfDoc(p)->roundedShotGaugeCursorX = v114;
    ((GolfDocShotBytes*)p)->unk_2D8 = 0;
    AsCGolfDoc(p)->shotGaugeDirection = kGaugeStopped;
    AsCGolfDoc(p)->calipersGaugeX = v149;
    AsCGolfDoc(p)->pangyaZoneScale = v1;
    AsCGolfDoc(p)->impactTimingSpriteIndex = 6;
    AsCGolfDoc(p)->impactFeedbackGaugeX = v149;
    AsCGolfDoc(p)->powerShotBonusPang = 0;
    AsCGolfDoc(p)->pangyaAssistState = 0;
    AsCGolfDoc(p)->unassistedImpactGaugeX = v149;
    AsCGolfDoc(p)->unassistedImpactGrade = 4;
    unsigned char* mode = *(unsigned char**)((char*)InstanceTable_Get() + 4);
    float value = ((SelectedClubRuntime*)mode)->clubKind == 3 ? kFloat_149_805DEA58 : kFloat_114_GolfRule;
    AsCGolfDoc(*(unsigned char**)(((char*)&AsCSharedDoc(g_sharedDoc)->golfDoc)))->shotGaugeCursorX = value;
    AsCGolfDoc(*(unsigned char**)(((char*)&AsCSharedDoc(g_sharedDoc)->golfDoc)))->roundedShotGaugeCursorX =
        AsCGolfDoc(*(unsigned char**)(((char*)&AsCSharedDoc(g_sharedDoc)->golfDoc)))->shotGaugeCursorX;
    unsigned char* crypto = (unsigned char*)GolfBall__Fv();
    unsigned int addr = (unsigned int)(crypto + 0x1989C);
    ((BallHoleInWordsView*)crypto)->encodedValue = ((BallHoleInWordsView*)crypto)->key ^ -1;
    unsigned char* dst = crypto + 0x198A0;
    unsigned char* src = crypto + 0x1989C;
    unsigned char* ab = (unsigned char*)&addr;
    for (int i = 0; i < 4; ++i)
        dst[i] = src[i] ^ ab[i];
    ((CQuadTree*)CQuadTree_GetInstance())->CalcGreenPlane();
    ShotParameters_UpdateControlWindow(InstanceTable_Get());
    unsigned char* control = (unsigned char*)GolfBall__Fv();
    if (AsCGolfBall(control)->m_groundType == 2) {
        SoundManager_RequestBgmChange(g_soundManager, (const char*)puttingBgmFileNames[rand() % 2], 0, 1);
        FloatTween_Start(s_actorRenderScaleTween, -1, kFloat_0_7_GolfRule, kFloat_1_805DEA38);
    } else {
        unsigned int item = selectedBgmIndex;
        if ((*(unsigned int*)&bgmNameCount) > item) {
            unsigned char* entry = ((unsigned char*)bgmNames) + item * 12;
            void* arg = g_soundManager;
            if ((*(unsigned int*)entry >> 31) == 0)
                ++entry;
            else
                entry = *(unsigned char**)(entry + 8);
            SoundManager_RequestBgmChange(arg, (const char*)entry, 0, 1);
        }
        FloatTween_Start(s_actorRenderScaleTween, -1, kFloat_1_805DEA38, kFloat_1_805DEA38);
    }
    AsCGolfDoc(*(unsigned char**)(((char*)&AsCSharedDoc(g_sharedDoc)->golfDoc)))->shotMeterFadeResetRequested = 0;
    if (!GetInputManagerSlot()) {
        if (!s_inputManagerInitialized) {
            InputManager_Init(s_gameInputManager);
            __register_global_object(s_gameInputManager, (void*)InputManager_Dtor, s_golfRuleInputDestructorRecord);
            s_inputManagerInitialized = 1;
        }
        GetInputManagerSlot() = s_gameInputManager;
    }
    void* singleton = GetInputManagerSlot();
    void* valuePtr =
        GolfPlayer_GetEntryByIndex((int)((*(unsigned char**)&AsCSharedDoc(g_sharedDoc)->golfDoc)),
                                   AsCGolfDoc((*(unsigned char**)&AsCSharedDoc(g_sharedDoc)->golfDoc))->currentPlayer);
    InputManager_SetCursorDrawOptions(
        singleton, 0, 2, AsCGolfDoc((*(unsigned char**)&AsCSharedDoc(g_sharedDoc)->golfDoc))->currentPlayer,
        *(int*)valuePtr);
    if (!GetInputManagerSlot()) {
        if (!s_inputManagerInitialized) {
            InputManager_Init(s_gameInputManager);
            __register_global_object(s_gameInputManager, (void*)InputManager_Dtor, s_golfRuleInputDestructorRecord);
            s_inputManagerInitialized = 1;
        }
        GetInputManagerSlot() = s_gameInputManager;
    }
    singleton = GetInputManagerSlot();
    valuePtr =
        GolfPlayer_GetEntryByIndex((int)((*(unsigned char**)&AsCSharedDoc(g_sharedDoc)->golfDoc)),
                                   AsCGolfDoc((*(unsigned char**)&AsCSharedDoc(g_sharedDoc)->golfDoc))->currentPlayer);
    InputManager_SetCursorColorOverride(singleton, 0, 0xD6, 0x48, 0x5C, *(int*)valuePtr);
    if (!GetInputManagerSlot()) {
        if (!s_inputManagerInitialized) {
            InputManager_Init(s_gameInputManager);
            __register_global_object(s_gameInputManager, (void*)InputManager_Dtor, s_golfRuleInputDestructorRecord);
            s_inputManagerInitialized = 1;
        }
        GetInputManagerSlot() = s_gameInputManager;
    }
    singleton = GetInputManagerSlot();
    valuePtr =
        GolfPlayer_GetEntryByIndex((int)((*(unsigned char**)&AsCSharedDoc(g_sharedDoc)->golfDoc)),
                                   AsCGolfDoc((*(unsigned char**)&AsCSharedDoc(g_sharedDoc)->golfDoc))->currentPlayer);
    InputManager_SetSelectedPlayer(singleton, *(void**)valuePtr);
}

// Request the exit transition appropriate to the active game mode.
// Uses shared save/task services; the integer parameter is ignored.
// Enables course records and clears transition input/audio/effects, then chooses
// Title directly, Title via SaveProcess in normal mode, or Tour via SaveProcess
// in Tour mode (6). Shared-record mode takes the direct Title route first.
// Appends follow-up requests;
// no success/failure result or local wait for completion.
void CGolfRule::RequestModeExitTransition(int unused) {
    using namespace ns4;
    AsCSharedDoc(g_sharedDoc)->courseRecordUpdatesEnabled = 1;
    GolfTransition_ClearInputAudioAndEffects(g_project);
    void* receiver4359_fn_80073DF0 = (void*)SaveData_AcquireInitializedInstance();
    if (SaveData_IsSharedRecordMode(receiver4359_fn_80073DF0) != 0) {
        unsigned char* stringPoolBytes = (unsigned char*)golfRuleStrings;
        ((CTask*)g_taskManager)
            ->RequestTransitionWithFade((char*)stringPoolBytes + kText_cTitleTask,
                                        (char*)stringPoolBytes + kText_emptyTaskArgument, 1);
        TaskCommandQueue_AppendNamedRequest(g_taskManager, 0, (char*)stringPoolBytes + kText_titleMain2, 3, 0, 0, 0, 0);
    } else {
        unsigned int mode = AsCSharedDoc(g_sharedDoc)->gameMode;
        if (mode == 0) {
            unsigned char* stringPoolBytes = (unsigned char*)golfRuleStrings;
            SaveProcess_SetTransitionTargetName((char*)stringPoolBytes + kText_cTitleTask);
            SaveProcess_SetFollowUpTaskName((char*)stringPoolBytes + kText_titleMain2);
            ((CTask*)g_taskManager)
                ->RequestTransitionWithFade((char*)stringPoolBytes + kText_cSaveProcessTask,
                                            (char*)stringPoolBytes + kText_emptyTaskArgument, 1);
            TaskCommandQueue_AppendNamedRequest(g_taskManager, 0, (char*)stringPoolBytes + kText_saveProcessMain, 1, 0,
                                                0, 0, 0);
        } else if (mode == 6) {
            unsigned char* stringPoolBytes = (unsigned char*)golfRuleStrings;
            SaveProcess_SetTransitionTargetName((char*)stringPoolBytes + kText_cTourTask);
            SaveProcess_SetFollowUpTaskName((char*)stringPoolBytes + kText_tourMain);
            ((CTask*)g_taskManager)
                ->RequestTransitionWithFade((char*)stringPoolBytes + kText_cSaveProcessTask,
                                            (char*)stringPoolBytes + kText_emptyTaskArgument, 1);
            TaskCommandQueue_AppendNamedRequest(g_taskManager, 0, (char*)stringPoolBytes + kText_saveProcessMain, 1, 0,
                                                0, 0, 0);
        } else {
            unsigned char* stringPoolBytes = (unsigned char*)golfRuleStrings;
            ((CTask*)g_taskManager)
                ->RequestTransitionWithFade((char*)stringPoolBytes + kText_cTitleTask,
                                            (char*)stringPoolBytes + kText_emptyTaskArgument, 1);
            TaskCommandQueue_AppendNamedRequest(g_taskManager, 0, (char*)stringPoolBytes + kText_titleMain2, 1, 0, 0, 0,
                                                0);
        }
    }
}

// Release the state-transition dialog and resume the selected turn state on acceptance.
// Requires a nonnull stored stateTransitionDialog: decrements its reference count
// before the null test, destroys it at zero and clears the stored handle.
// state == 1 clears dropRequest, sends screen 0x58 and re-enters Aim.
// Returns 1 for every state; no null-dialog recovery is provided.
int CGolfRule::OnStateDialogResult(int state) {
    using namespace ns4;
    int* ref = (int*)stateTransitionDialog;
    int count = --ref[0];
    if (count == 0 && ref) {
        ((Rf4*)ref)->v08(1);
    }
    stateTransitionDialog = 0;
    if (state == 1) {
        unsigned char* manager = (unsigned char*)g_sharedDoc;
        unsigned char* control = (*(unsigned char**)&AsCSharedDoc(manager)->golfDoc);
        AsCGolfDoc(control)->dropRequest = 0;
        IActor_DispatchMessage(this, this, golfRuleStrings + kText_screen, 0x58, 0, 0, 0, 0);
        EnterState(kStateAim);
    }
    return 1;
}

// ===== S18 - BGM playlist and sequence preloads =====
// Rule-owned name/task containers and lazy singleton registrations.

// Build the background-music playlist for the active course and party rules.
// Requires the rule's constructed name vector. Destroys previous strings, then
// copies fixed party tracks or the course-specific names selected by course ID.
// Unknown/default course IDs leave the list empty. Does not play a track or use RNG;
// callers using rand()%bgmNameCount require a nonempty list.
void CGolfRule::BuildBgmPlaylist(const char* directory) {
    using namespace ns4;
    Fn8016BCB4Object* bgm = (Fn8016BCB4Object*)this;
    // Phase 1: destroy old strings; party modes use the three fixed tracks.
    StringArray_ClearElements(bgm->list);
    if (*(int*)((char*)SharedDoc_GetCurrent() + 0xAD8)) {
        {
            BgmTrackName text(golfRuleStrings + kText_skip_skip_skip_wav);
            StringVector_AppendCopy(bgm->list, &text);
        }
        {
            BgmTrackName text(golfRuleStrings + kText_so_nice_wav);
            StringVector_AppendCopy(bgm->list, &text);
        }
        {
            BgmTrackName text(golfRuleStrings + kText_fun_time_wav);
            StringVector_AppendCopy(bgm->list, &text);
        }
        return;
    }
    // Phase 2: append course-specific tracks, copying each temporary string.
    switch ((int)(unsigned char)SharedDoc_GetCourseId()) {
    case 0:
    case 1: {
        BgmTrackName text(golfRuleStrings + kText_daydream_wav);
        StringVector_AppendCopy(bgm->list, &text);
    }
        {
            BgmTrackName text(golfRuleStrings + kText_breeze_wav);
            StringVector_AppendCopy(bgm->list, &text);
        }
        {
            BgmTrackName text(golfRuleStrings + kText_frog_wav);
            StringVector_AppendCopy(bgm->list, &text);
        }
        break;
    case 2:
    case 3: {
        BgmTrackName text(golfRuleStrings + kText_spring_wav);
        StringVector_AppendCopy(bgm->list, &text);
    }
        {
            BgmTrackName text(golfRuleStrings + kText_twilight_mood_wav);
            StringVector_AppendCopy(bgm->list, &text);
        }
        {
            BgmTrackName text(golfRuleStrings + kText_autumn_colors_wav);
            StringVector_AppendCopy(bgm->list, &text);
        }
        break;
    case 4:
    case 5: {
        BgmTrackName text(golfRuleStrings + kText_bunny_picnic_wav);
        StringVector_AppendCopy(bgm->list, &text);
    }
        {
            BgmTrackName text(golfRuleStrings + kText_magic_wand_wav);
            StringVector_AppendCopy(bgm->list, &text);
        }
        {
            BgmTrackName text(golfRuleStrings + kText_a_shiny_day_wav);
            StringVector_AppendCopy(bgm->list, &text);
        }
        break;
    case 6: {
        BgmTrackName text(golfRuleStrings + kText_ruffles_wav);
        StringVector_AppendCopy(bgm->list, &text);
    }
        {
            BgmTrackName text(golfRuleStrings + kText_southern_cross_wav);
            StringVector_AppendCopy(bgm->list, &text);
        }
        break;
    case 7: {
        BgmTrackName text(golfRuleStrings + kText_navy_blue_memory_wav);
        StringVector_AppendCopy(bgm->list, &text);
    }
        {
            BgmTrackName text(golfRuleStrings + kText_rising_sun_wav);
            StringVector_AppendCopy(bgm->list, &text);
        }
        {
            BgmTrackName text(golfRuleStrings + kText_stratosphere_wav);
            StringVector_AppendCopy(bgm->list, &text);
        }
        break;
    case 9: {
        BgmTrackName text(golfRuleStrings + kText_distant_bells_wav);
        StringVector_AppendCopy(bgm->list, &text);
    }
        {
            BgmTrackName text(golfRuleStrings + kText_snowscape_wav);
            StringVector_AppendCopy(bgm->list, &text);
        }
        {
            BgmTrackName text(golfRuleStrings + kText_winter_ride_wav);
            StringVector_AppendCopy(bgm->list, &text);
        }
        break;
    case 10: {
        BgmTrackName text(golfRuleStrings + kText_somewhere_wav);
        StringVector_AppendCopy(bgm->list, &text);
    }
        {
            BgmTrackName text(golfRuleStrings + kText_nowhere_wav);
            StringVector_AppendCopy(bgm->list, &text);
        }
        {
            BgmTrackName text(golfRuleStrings + kText_journey_for_treasure_wav);
            StringVector_AppendCopy(bgm->list, &text);
        }
        break;
    case 8: {
        BgmTrackName text(golfRuleStrings + kText_crystal_waver_wav);
        StringVector_AppendCopy(bgm->list, &text);
    }
        {
            BgmTrackName text(golfRuleStrings + kText_happy_flight_wav);
            StringVector_AppendCopy(bgm->list, &text);
        }
        {
            BgmTrackName text(golfRuleStrings + kText_polaris_wav);
            StringVector_AppendCopy(bgm->list, &text);
        }
        break;
    case 11: {
        BgmTrackName text(golfRuleStrings + kText_sakura_front_wav);
        StringVector_AppendCopy(bgm->list, &text);
    }
        {
            BgmTrackName text(golfRuleStrings + kText_sweet_blossom_wav);
            StringVector_AppendCopy(bgm->list, &text);
        }
        break;
    case 12: {
        BgmTrackName text(golfRuleStrings + kText_dive_into_volcano_wav);
        StringVector_AppendCopy(bgm->list, &text);
    }
        {
            BgmTrackName text(golfRuleStrings + kText_vermilion_sunset_wav);
            StringVector_AppendCopy(bgm->list, &text);
        }
        break;
    case 13: {
        BgmTrackName text(golfRuleStrings + kText_crystal_lake_wav);
        StringVector_AppendCopy(bgm->list, &text);
    }
        {
            BgmTrackName text(golfRuleStrings + kText_fade_into_white_wav);
            StringVector_AppendCopy(bgm->list, &text);
        }
        break;
    case 253: {
        BgmTrackName text(golfRuleStrings + kText_tutorial_wav);
        StringVector_AppendCopy(bgm->list, &text);
    } break;
    case 127:
    default:
        break;
    }
}

#pragma use_lmw_stmw on
namespace ns4 {
extern "C" {
// Queue the shared shot-effect sequence resources.
// Uses the global preload-task vector; creates/appends 28 asynchronous sequence tasks.
// Vector growth retains its overflow exception paths and storage-swap order.
// Returns 1 after queueing all names; completion is polled separately.
int GolfRule_PreloadEffectSequences() {
    struct StringList {
        unsigned int* data;
        unsigned int size, capacity;
    };
    struct Names {
        unsigned int name[28];
    };
    Names localNames = *(Names*)s_ballEffectPreloadSequenceNames;
    StringList* list = (StringList*)&s_shotEffectPreloadTasks;
    unsigned int* names = localNames.name;
    int i = 0;
    do {
        unsigned int value = (unsigned int)CPreloadSeqAsync::Create((void*)*names);
        unsigned int size = list->size;
        unsigned int cap = list->capacity;
        if (size < cap) {
            unsigned int next = size + 1;
            unsigned int* data = s_shotEffectPreloadTasks.data;
            list->size = next;
            (data + next)[-1] = value;
        } else {
            if (0x3FFFFFFFU - list->capacity < 1U) {
                const char* source = golfRuleStrings + kText_vector_length_error;
                Fn8016C2B8LengthError error(source);
            }
            Fn8016C2B8Storage tmp(&list->capacity);
            unsigned int growthA, growthB;
            unsigned int capacity = list->capacity;
            unsigned int needed = list->size + 1 - capacity;
            if (needed > 0x3FFFFFFFU - capacity) {
                FN8016C2B8_ERROR();
            }
            if (capacity < 0x15555555U) {
                growthA = ((capacity + 1) * 3) / 5;
                capacity += *(growthA < needed ? &needed : &growthA);
            } else if (capacity < 0x2AAAAAAAU) {
                growthB = (capacity + 1) / 2;
                capacity += *(growthB < needed ? &needed : &growthB);
            } else {
                capacity = 0x3FFFFFFF;
            }
            if (capacity > 0x3FFFFFFFU) {
                FN8016C2B8_ERROR();
            }
            unsigned int* data = (unsigned int*)appMallocDummy__FUl(capacity * 4);
            tmp.offset = list->size;
            (data + list->size)[tmp.size] = value;
            tmp.capacity = capacity;
            tmp.data = data;
            tmp.size++;
            unsigned int* old = s_shotEffectPreloadTasks.data;
            unsigned int count = ((int)(old + list->size) - (int)old) / 4;
            tmp.offset -= count;
            memcpy(data + tmp.offset, old, count * 4);
            memset(old, 0, count * 4);
            unsigned int newSize = tmp.size + count;
            tmp.size = newSize;
            unsigned int* oldData = s_shotEffectPreloadTasks.data;
            unsigned int* newData = tmp.data;
            unsigned int oldCapacity = list->capacity;
            list->size = 0;
            list->capacity = tmp.capacity;
            tmp.capacity = oldCapacity;
            s_shotEffectPreloadTasks.data = newData;
            tmp.data = oldData;
            list->size = newSize;
            tmp.size = 0;
        }
        ++i;
        ++names;
    } while (i < 28);
    return 1;
}
}
}

#pragma opt_strength_reduction off
#pragma use_lmw_stmw reset
namespace ns4 {
extern "C" {
// Update each queued preload task and report whether every task is complete.
// Polls the global task range, reloading its count/base with volatile reads.
// Calls each task's update then done; returns 0 at the first unfinished task.
// Returns 1 when the whole range is done (also for empty range); does not destroy tasks.
int GolfRule_PollSequencePreloads() {
    unsigned int* base;
    unsigned int count;
    unsigned int* end;
    unsigned int* current = s_shotEffectPreloadTasks;
    while ((count = *(volatile unsigned int*)&s_shotEffectPreloadTasks.size,
            base = *(unsigned int* volatile*)&s_shotEffectPreloadTasks.data, end = base + count, !(end == current))) {
        ((Fn8016C700Task*)*current)->update();
        if (!((Fn8016C700Task*)*current)->done())
            return 0;
        current++;
    }
    return 1;
}
}
}

#pragma opt_strength_reduction reset
#pragma use_lmw_stmw on
// Queue player-shot sequences and the resources required by the selected party rules.
// Requires a rule child-task vector. If a course suffix exists, tries six sequence
// formats, skipping names whose calculated length does not fit the 64-byte buffer.
// Balloon Pop also appends seven fixed synchronous sequence tasks.
// Returns 1 after queueing; absent suffix simply skips the course-name loop.
int CGolfRule::PreloadShotAndPartySequences() {
    using namespace ns4;
    unsigned char* base = (*(unsigned char**)&AsCSharedDoc(g_sharedDoc)->selectedCourseData);
    Fn8016C79CObject* obj = (Fn8016C79CObject*)this;
    const char* suffix = base ? ((IFF_STRUCT::sCourse*)base)->mapName : 0;
    char text[64];
    Fn8016C79CNames6 firstNames;
    Fn8016C79CNames7 secondNames;
    if (suffix) {
        firstNames = s_courseShotSequenceFormats;
        const char** name = firstNames.name;
        int i = 0;
        do {
            int suffixLength;
            const char* format = *name;
            suffixLength = strlen(suffix);
            int totalLength = strlen(format);
            totalLength += suffixLength;
            if ((unsigned int)(totalLength - 2) < 0x40U) {
                sprintf(text, format, suffix);
                void* value = CPreloadSeqSync::Create(text);
                FN8016C79C_APPEND(value, 0);
            }
            int savedI2 = i;
            ++name;
            int savedValue = 1 + savedI2;
            i = savedValue;
        } while (i < 6);
    }
    if ((*(int*)&AsCSharedDoc(g_sharedDoc)->partyGameMode) == 3) {
        secondNames = s_partyMode3PreloadSequenceNames;
        const char** name = secondNames.name;
        int i = 0;
        do {
            void* value = CPreloadSeqSync::Create((void*)*name);
            FN8016C79C_APPEND(value, 1);
            name++;
            i++;
        } while (7 > i);
    }
    return 1;
}

#pragma inline_depth(8)
#pragma inline_max_size(10000)
#pragma inline_max_total_size(100000)
// Queue effect resources for each equipped ball.
// Requires rule/shared inventory. For each selected player, looks up ball data
// and examines seven effect names; only recognized .seq/.spr suffixes queue tasks.
// Missing ball data, empty names, missing suffix or null task results are skipped.
// Returns 1 after traversal; queued child handles belong to the rule's task vector.
int CGolfRule::PreloadBallEffects() {
    using namespace ns4;
    Fn8016D08CList* list = (Fn8016D08CList*)&childObjects;
    struct Iterator {
        unsigned int count, limit, offset;
        char padding[8];
        // Snapshot the round-player list limit and start at its first entry.
        Iterator() {
            limit = *s_roundPlayerList;
            count = 0;
            offset = 0;
        }
        void advance() volatile {
            offset += 0x1D4;
            ++count;
        }
    };
    volatile Iterator it;
    for (; it.count < it.limit; it.advance()) {
        char* manager = (char*)g_sharedDoc;
        void* key = *(void**)(manager + it.offset + 0x88);
        IFF_STRUCT::sBall* found = (IFF_STRUCT::sBall*)ItemInventory_FindBallData(manager + 0xA04, key);
        if (found) {
            char* name = found->effectFiles[0];
            unsigned int i = 0;
            do {
                if (*(signed char*)name) {
                    char* dot = strrchr(name, 0x2E);
                    if (dot) {
                        unsigned int value = 0;
                        if (strcmp(dot, golfRuleStrings + kText_seq) == 0)
                            value = (unsigned int)CPreloadSeqSync::Create(name);
                        else if (strcmp(dot, golfRuleStrings + kText_spr) == 0)
                            value = (unsigned int)CPreloadSprSync::Create(name);
                        if (value)
                            Fn8016D08CPush(list, value);
                    }
                }
                ++i;
                name += 0x28;
            } while (i < 7);
        }
    }
    return 1;
}

#pragma inline_depth reset
#pragma inline_max_size reset
#pragma inline_max_total_size reset
#pragma opt_common_subs off
#pragma opt_lifetimes off
#pragma opt_propagation off
#pragma opt_strength_reduction off
// ===== S19 - Turn advancement, assistance and result menus =====
// Player results, gauge assistance and shared course-clear records.

// Select the next player or round-result state according to player completion and rules.
// Requires current player/documents. Darts checks its target reply first;
// otherwise par reached, or flagged Balloon Pop, admits settlement for this player.
// Completed-hole count guards totals; sets state 1 and optional party threshold tier.
// Chooses hole results when the round-completion query succeeds, otherwise actor wait.
void CGolfRule::AdvanceTurnOrRound(int flag) {
    using namespace ns4;
    if (*(int*)&((CSharedDoc*)(SharedDocBytes()))->partyGameMode == 1) {
        int result;
        IActor_DispatchMessage(this, this, golfRuleStrings + kText_dartsSystem, 0, (int)&result,
                               AsCGolfDoc(GolfDocBytes())->currentHoleIndex, 0, 0);
        if (*(int*)(GolfPlayerEntryAbi(AsCGolfDoc(GolfDocBytes())->currentPlayer) + 0x178) > result) {
            ((GolfDocSnapshot8013*)(GolfPlayerEntryAbi(AsCGolfDoc(GolfDocBytes())->currentPlayer)))->itemId = 0;
            EnterState(kStateFightResult);
            return;
        }
    }

    unsigned char active;
    active = 0;
    unsigned char* state = GolfPlayerEntryAbi(AsCGolfDoc(GolfDocBytes())->currentPlayer);
    unsigned char* control = (*(unsigned char**)&AsCSharedDoc(SharedDocBytes())->golfDoc);
    if ((state + ((CGolfDoc*)(control))->currentHoleIndex)[0x28] >=
            (*(unsigned char**)&((CGolfDoc*)(control))->holeRecords +
             (((CGolfDoc*)(control))->currentHoleIndex << 5))[0x18] ||
        (flag && *(int*)&((CSharedDoc*)(SharedDocBytes()))->partyGameMode == 3)) {
        unsigned char slot = ((CGolfDoc*)(control))->currentPlayer;
        if (slot < ((CGolfDoc*)(control))->playerCount) {
            if (*(int*)((unsigned char*)GolfPlayer_GetEntryByIndex((int)control, slot) + 8) != 3) {
                ((GolfDocSnapshot8013*)(GolfPlayerEntryAbi(slot)))->itemId = 0;
                unsigned char round = AsCGolfDoc(GolfDocBytes())->currentHoleIndex;
                if (*(int*)&((GolfDocSnapshot8013*)(GolfPlayerEntryAbi(slot)))->roundResultWords[3] == round) {
                    if (((CGolfDoc*)(GolfDocBytes()))->unk_35C == 0) {
                        unsigned char round2 = AsCGolfDoc(GolfDocBytes())->currentHoleIndex;
                        unsigned char* src = GolfPlayerEntryAbi(slot);
                        unsigned char* dst = GolfPlayerEntryAbi(slot);
                        ((GolfDocSnapshot8013*)(dst))->totalStrokeCount += (src + round2)[0x28];
                        if (AsCSharedDoc(SharedDocBytes())->golfRuleType != 1) {
                            unsigned char round3 = AsCGolfDoc(GolfDocBytes())->currentHoleIndex;
                            unsigned char* src2 = GolfPlayerEntryAbi(slot);
                            unsigned char* byround = *(unsigned char**)&((CGolfDoc*)(GolfDocBytes()))->holeRecords +
                                                     (AsCGolfDoc(GolfDocBytes())->currentHoleIndex << 5);
                            unsigned char* dst2 = GolfPlayerEntryAbi(slot);
                            *(int*)&((GolfDocSnapshot8013*)(dst2))->roundResultWords[0] +=
                                (int)(src2 + round3)[0x28] - (int)((ShotHoleRecord*)(byround))->par;
                        }
                    }
                    ++*(int*)&((GolfDocSnapshot8013*)(GolfPlayerEntryAbi(slot)))->roundResultWords[3];
                }
                *(int*)(GolfPlayerEntryAbi(slot) + 8) = 1;
                unsigned char* mgr = SharedDocBytes();
                if (((CSharedDoc*)(mgr))->gameMode == 6U)
                    *(int*)&((CSharedDoc*)(mgr))->partyResultTier = PartyGame_EvaluatePlayerThresholdTier(slot);
            }
        }
        active = 1;
    }
    if (active) {
        if (IsCurrentHoleComplete()) {
            EnterState(kStateHoleResults);
            return;
        }
        IActor_DispatchMessage(this, this, golfRuleStrings + kText_score, 1, 0, 0, 0, 0);
    }
    EnterState(kStateWaitPlayerAndCaddie);
}

#pragma opt_common_subs reset
#pragma opt_lifetimes reset
#pragma opt_propagation reset
#pragma opt_strength_reduction reset
#pragma use_lmw_stmw reset
namespace ns4 {
extern "C" {
// Apply impact, equipment and effect modifiers to the current player shot-outcome value.
// Requires active player, inventory, club runtime and live ball/document state.
// Computes the table/item/accessory delta and clamps the powerShotMeter result.
// Returns early for excluded item/mode outcomes; no failure return and no RNG
// call in this body. Auxiliary slots and unresolved account words retain raw views.
void GolfRule_ApplyShotOutcomeModifier() {
    if ((((unsigned int*)s_shotActionRestrictionState)[0x48 / sizeof(unsigned int)] & 0x8) ||
        AsCGolfDoc(GolfDocBytes())->tutorialStage < 6)
        return;
    {
        int amount = 0;
        int mode = AsCGolfDoc(GolfDocBytes())->unassistedImpactGrade;
        if (((ClubSelectionRuntime*)InstanceTable_Get())->powerShotMode == 0) {
            switch (mode) {
            case 4: {
                void* layout;
                unsigned char* pl;
                if ((layout = ((char*)&AsCSharedDoc(g_sharedDoc)->itemInventory), pl = CurrentGolfPlayerEntryAbi(),
                     *(unsigned int*)&AsGolfDocSnapshot(pl)->itemId ==
                         (unsigned int)ItemInventory_GetItemIdAt(layout, 9)) ||
                    (layout = ((char*)&AsCSharedDoc(g_sharedDoc)->itemInventory), pl = CurrentGolfPlayerEntryAbi(),
                     *(unsigned int*)&AsGolfDocSnapshot(pl)->itemId ==
                         (unsigned int)ItemInventory_GetItemIdAt(layout, 1)))
                    return;
                {
                    T16 t = *(T16*)s_shotOutcomeClubSetModifiers;
                    amount = (int)(t.v + 0)[D834_IDX()];
                }
                if ((unsigned int)AsCGolfBall(GolfBall__Fv())->m_powerFlags & 3) {
                    T16 t = *(T16*)s_shotOutcomeClubSetModifiers;
                    amount = (int)(t.v + 8)[D834_IDX()];
                } else if ((unsigned int)AsCGolfBall(GolfBall__Fv())->m_powerFlags & 0xC) {
                    T16 t = *(T16*)s_shotOutcomeClubSetModifiers;
                    amount = (int)(t.v + 10)[D834_IDX()];
                }
                unsigned int rankTier = PlayerRank_FindTierForValue(
                    *(int*)((char*)g_sharedDoc + (int)AsCGolfDoc(GolfDocBytes())->currentPlayer * 0x1D4 + 0x44));
                int rankGrade = rankTier < 6 ? 1 : (int)((rankTier - 1) / 5) + 1;
                int rest = (AsCGolfDoc(GolfDocBytes())->maxGrade - (unsigned char)rankGrade) * 2;
                if (rest > 0)
                    amount += rest;
                break;
            }
            case 3: {
                T16 t = *(T16*)s_shotOutcomeClubSetModifiers;
                amount = (int)(t.v + 2)[D834_IDX()];
                break;
            }
            case 2: {
                T16 t = *(T16*)s_shotOutcomeClubSetModifiers;
                amount = (int)(t.v + 4)[D834_IDX()];
                break;
            }
            case 0:
            case 1: {
                T16 t = *(T16*)s_shotOutcomeClubSetModifiers;
                amount = (int)(t.v + 6)[D834_IDX()];
                break;
            }
            }
            {
                unsigned int entry = (unsigned int)g_sharedDoc + AsCGolfDoc(GolfDocBytes())->currentPlayer * 0x1D4;
                unsigned int* slot = (unsigned int*)(entry + 0x190);
                for (int i = 0; i < 5; ++i, ++slot) {
                    if (*slot == (unsigned int)ItemInventory_GetAuxPartIdAt(
                                     ((char*)&AsCSharedDoc(g_sharedDoc)->itemInventory), 4)) {
                        amount += (mode == 4) ? 1 : 3;
                        break;
                    }
                }
            }
        } else {
            {
                T16 t = *(T16*)s_shotOutcomeClubSetModifiers;
                amount = (int)(t.v + 14)[D834_IDX()];
            }
            if (((ClubSelectionRuntime*)InstanceTable_Get())->powerShotMode == 2) {
                T16 t = *(T16*)s_shotOutcomeClubSetModifiers;
                amount = (int)((float)amount + (t.v + 14)[D834_IDX()]);
            }
        }
        if (amount == 0)
            return;
        if ((*(int*)&AsCSharedDoc(g_sharedDoc)->partyGameMode) == 2 && amount > 0)
            return;
        float delta;
        if (amount < 0)
            delta = (float)amount;
        else {
            float multiplier = ((float (*)(int, int, int))CSharedDoc_ComputeAuxPartModifier)(
                (int)g_sharedDoc, AsCGolfDoc(GolfDocBytes())->currentPlayer, 2);
            delta = (float)amount * multiplier;
        }
        if ((*(int*)&AsCSharedDoc(g_sharedDoc)->partyGameMode) == 3) {
            char* extra = (char*)PartyItemSlots_GetInstance();
            if (kFloat_0_GolfRule < delta)
                delta *= kFloat_1_805DEA38 + *(float*)(extra + 0x20);
        }
        float result = D834_clamp2(delta + AsGolfDocSnapshot(CurrentGolfPlayerEntryAbi())->powerShotMeter);
        AsGolfDocSnapshot(CurrentGolfPlayerEntryAbi())->powerShotMeter = result;
    }
}
}
}

// Apply equipped-item impact assistance while retaining the unassisted gauge result.
// Requires live documents, player entry, selected club and item inventory.
// Preserves unassisted impact X/grade, then may move impact X within the assisted
// Pangya window. X is in screen units (149 start, 360 span).
// adjust controls width scaling. Both width helpers are deterministic; no RNG
// is consumed here or in those helpers. Early exits have no failure return.
void CGolfRule::ApplyImpactAssistance(void* unused, int adjust) {
    using namespace ns4;
    unsigned char* state = (*(unsigned char**)&AsCSharedDoc(g_sharedDoc)->golfDoc);
    AsCGolfDoc(state)->pangyaAssistState = 0;
    float step = fn_8016E124_step(adjust);
    unsigned char* control = (*(unsigned char**)&AsCSharedDoc(g_sharedDoc)->golfDoc);
    float delta = AsCGolfDoc(control)->shotImpactGaugeX - AsCGolfDoc(control)->pangyaTargetX;
    delta = delta > 0.0f ? delta : -delta;
    int mode;
    if (delta < 0.5f + step)
        mode = 4;
    else {
        float dist = AsCGolfDoc(control)->shotImpactGaugeX - 149.0f;
        dist = dist > 0.0f ? dist : -dist;
        if (dist < 0.5f + ShotParameters_GetGoodImpactHalfWidth(InstanceTable_Get()))
            mode = 3;
        else {
            mode = dist < 0.5f + ShotParameters_GetPoorImpactHalfWidth(InstanceTable_Get()) ? 2 : 1;
        }
    }
    AsCGolfDoc(state)->unassistedImpactGrade = mode;
    control = (*(unsigned char**)&AsCSharedDoc(g_sharedDoc)->golfDoc);
    AsCGolfDoc(state)->unassistedImpactGaugeX = AsCGolfDoc(control)->shotImpactGaugeX;
    if (mode == 1 && !adjust)
        return;
    if (mode == 4) {
        AsCGolfDoc(state)->pangyaAssistState = 2;
        return;
    }
    unsigned char* gm = (unsigned char*)g_sharedDoc;
    control = (*(unsigned char**)&AsCSharedDoc(gm)->golfDoc);
    if (AsCGolfDoc(control)->tutorialStage < 6)
        return;
    if (!ItemInventory_IsSlot14ClubSetId(*(void**)(gm + AsCGolfDoc(control)->currentPlayer * 0x1D4 + 0x1CC)))
        return;
    step = fn_8016E124_step2(adjust);
    control = (*(unsigned char**)&AsCSharedDoc(g_sharedDoc)->golfDoc);
    float amount = (0.5f + step) - 0.01f;
    if (AsCGolfDoc(control)->shotImpactGaugeX > AsCGolfDoc(control)->pangyaTargetX)
        AsCGolfDoc(control)->shotImpactGaugeX = amount + AsCGolfDoc(control)->pangyaTargetX;
    else
        AsCGolfDoc(control)->shotImpactGaugeX = AsCGolfDoc(control)->pangyaTargetX - amount;
    AsCGolfDoc(state)->pangyaAssistState = 1;
}

namespace ns4 {
extern "C" {
// Producer-side ABI for ResultMenuData in result_menu_and_fight_result.cpp. The setter copies each
// scalar to CResultMenu (+9C..+CC) synchronously and retains no payload address.
// Signed producer words/short are intentional: the consumer reads their unsigned
// bit patterns, then formats score as signed. Padding +2E is zeroed, not consumed.
struct GolfResultMenuPayload {
    int placing;           // +00: player+C4 high nibble; party overrides with shared+AF0
    int textureIndex;      // +04: player+168 rankRewardIndex, selects one of three images
    int score;             // +08: score-to-par, or mode-dependent party value (see below)
    int earnedPang;        // +0C: player+CC, already awarded base Pang
    int bonusPang;         // +10: player+D0, already awarded bonus Pang
    int hasDoublePang;     // +14: player+16E byte; Boolean reveal flag, not a multiplier
    int remainingProgress; // +18: player+164 roundRankReward, counted progress units
    int totalProgress;     // +1C: sign-extended player+16C short, final displayed total
    int roundHoleCount;    // +20: shared+9C5
    int currentHoleIndex;  // +24: document+3B6, zero-based
    int messageIndex;      // +28: shared+B80 courseRecordIndex, result message suffix
    short specialTotal;    // +2C: special win's SharedStringState short; zero otherwise
    short pad_02E;
    int specialMode; // +30: gameMode==6, independent of party/result menu mode
};
// Both menu producers read the completed party value at player+1C4. Modes 1/3
// store integer points; mode 2 stores a float distance, converted to int for UI.
// This ABI alone does not establish the distance's physical/display unit.
struct GolfPartyResultPayload {
    char prefix[0x1C4];
    union {
        int points;
        float distance;
    } result;
};
typedef char PartyResultOffset[(int)&((GolfPartyResultPayload*)0)->result == 0x1C4 ? 1 : -1];
typedef char ResultPayloadSize[(sizeof(GolfResultMenuPayload) == 0x34) ? 1 : -1];
#define RESULT_PAYLOAD_OFF(f, o) typedef char ResultPayload_##f[((int)&((GolfResultMenuPayload*)0)->f == (o)) ? 1 : -1]
RESULT_PAYLOAD_OFF(placing, 0x00);
RESULT_PAYLOAD_OFF(textureIndex, 0x04);
RESULT_PAYLOAD_OFF(score, 0x08);
RESULT_PAYLOAD_OFF(earnedPang, 0x0C);
RESULT_PAYLOAD_OFF(bonusPang, 0x10);
RESULT_PAYLOAD_OFF(hasDoublePang, 0x14);
RESULT_PAYLOAD_OFF(remainingProgress, 0x18);
RESULT_PAYLOAD_OFF(totalProgress, 0x1C);
RESULT_PAYLOAD_OFF(roundHoleCount, 0x20);
RESULT_PAYLOAD_OFF(currentHoleIndex, 0x24);
RESULT_PAYLOAD_OFF(messageIndex, 0x28);
RESULT_PAYLOAD_OFF(specialTotal, 0x2C);
RESULT_PAYLOAD_OFF(specialMode, 0x30);
#undef RESULT_PAYLOAD_OFF
}
}

// Create and initialize the result menu for the active game mode.
// Requires documents/player reward records; the rule is the actor-message sender.
// Menu mode: 0 stroke, 1 match, 2 party. specialMode instead means gameMode==6.
// arg selects win BGM/specialTotal only in specialMode; party replaces arg with
// (partyResultTier!=0). It does not select the player or recompute any award.
// Selection: every slot with player+4 !=1, plus slot 0 in specialMode, overwrites
// ALL player-derived fields. Thus the highest eligible slot wins; special mode
// does NOT force slot 0 if a later slot passes. If none pass those fields stay 0;
// that zero fallback is not a valid normal stroke placing. Caller must arrange
// an eligible player and valid reward/rank data for the intended menu mode.
// Score units: ordinary signed strokes relative to par; party 1 Darts points,
// party 3 Balloon Pop points; party 2 Driving Contest distance truncated from
// float to int (the distance unit is not established by this producer).
// Values/Pang/progress are separate units; the setter awards nothing.
// Consumer domain (result_menu_and_fight_result.cpp SelectStateScript/SetResultData): stroke placing
// 1..8, match placing==1 wins, party placing 0..3; textureIndex 0..2, holes 1..18
// and currentHoleIndex<roundHoleCount, existing course message suffix. Progress
// animation requires nonnegative reward/initial progress with their sum<=2^24,
// positive rank intervals<=2^24, and Pang totals/sums fitting signed UI integers.
// These are caller/resource requirements, not validation added here.
// Stack payload is borrowed only for SetResultData's synchronous scalar copy.
// Returns a newly allocated owned menu; callers store it in rule.resultMenu,
// poll MPBase1_Process, then delete through its virtual interface and clear it
// (ProcessPostRoundMenus / ProcessRoundResultPresentation). Allocation may throw;
// this function provides no local failure recovery. Voice message 7 follows copy.
void* CGolfRule::CreateResultMenu(int arg) {
    using namespace ns4;
    int kind = 1;
    if ((*(int*)&AsCSharedDoc(g_sharedDoc)->partyGameMode))
        kind = 2;
    else if (!AsCSharedDoc(g_sharedDoc)->golfRuleType)
        kind = 0;
    struct Obj8016 {
        char pad[0xD0];
        // Construct the result-menu object using the caller mode.
        Obj8016(int k) {
            __ct__11CResultMenuFPv(this, k);
        }
    };
    Obj8016* result = new Obj8016(kind);
    GolfResultMenuPayload data;
    memset(&data, 0, 0x34);
    data.specialMode = *(int*)&((CSharedDoc*)(SharedDocBytes()))->gameMode == 6;
    if (data.specialMode) {
        if (*(int*)&((CSharedDoc*)(SharedDocBytes()))->partyGameMode)
            arg = *(int*)&((CSharedDoc*)(SharedDocBytes()))->partyResultTier != 0;
        if (arg) {
            data.specialTotal = SharedStringState_GetShortValue();
            SoundManager_RequestBgmChange(g_soundManager, golfRuleStrings + kText_story_win_wav, 1, 1);
        } else {
            SoundManager_RequestBgmChange(g_soundManager, golfRuleStrings + kText_story_lose_wav, 1, 1);
        }
    }
    for (int slot = 0; slot < AsCGolfDoc(GolfDocBytes())->playerCount; ++slot) {
        if (((RoundRewardPlayerView*)GolfPlayerEntryAbiByte(slot))->isComputer != 1 ||
            (slot == 0 && data.specialMode)) {
            data.placing = (((GolfDocSnapshot8013*)(GolfPlayerEntryAbiByte(slot)))->rankFlags >> 4) & 0xF;
            data.score =
                *(int*)&((GolfDocSnapshot8013*)(GolfPlayerEntryAbiByte(slot)))->roundResultWords[kRoundScoreWord];
            switch (*(int*)&((CSharedDoc*)(SharedDocBytes()))->partyGameMode) {
            case 1:
            case 3:
                data.score = ((GolfPartyResultPayload*)GolfPlayerEntryAbiByte(slot))->result.points;
                break;
            case 2:
                data.score = (int)((GolfPartyResultPayload*)GolfPlayerEntryAbiByte(slot))->result.distance;
                break;
            }
            data.textureIndex = ((RoundRewardPlayerView*)(GolfPlayerEntryAbiByte(slot)))->rankRewardIndex;
            data.earnedPang =
                *(int*)&((GolfDocSnapshot8013*)(GolfPlayerEntryAbiByte(slot)))->roundResultWords[kRoundPangWord];
            data.bonusPang =
                *(int*)&((GolfDocSnapshot8013*)(GolfPlayerEntryAbiByte(slot)))->roundResultWords[kRoundBonusPangWord];
            data.remainingProgress = ((RoundRewardPlayerView*)(GolfPlayerEntryAbiByte(slot)))->roundRankReward;
            data.totalProgress = ((RoundRewardPlayerView*)(GolfPlayerEntryAbiByte(slot)))->rankTableBonus;
            data.hasDoublePang = ((RoundRewardPlayerView*)(GolfPlayerEntryAbiByte(slot)))->repeatPangAward;
        }
    }
    if (*(int*)&((CSharedDoc*)(SharedDocBytes()))->partyGameMode)
        data.placing = *(int*)&((CSharedDoc*)(SharedDocBytes()))->partyResultTier;
    data.roundHoleCount = AsCSharedDoc(SharedDocBytes())->roundHoleCount;
    data.currentHoleIndex = AsCGolfDoc(GolfDocBytes())->currentHoleIndex;
    data.messageIndex = *(int*)&((CSharedDoc*)(SharedDocBytes()))->courseRecordIndex;
    SetResultData__11CResultMenuFP14ResultMenuData(result, &data);
    IActor_DispatchMessage(this, this, golfRuleStrings + kText_voice, 7, 0, 0, 0, 0);
    return result;
}

namespace ns4 {
extern "C" {
// ABI consumed by CResultFightMenu::SetEntries in view_rank_and_fight_result_menu.cpp: count rows are
// copied by value to menu+3C, 12 bytes per row, without sorting or retaining src.
struct GolfFightResultEntry {
    int playerIndex; // +0: slot in the golf document, used to select that player's texture
    int score;       // +4: same signed score / party points / truncated distance as above
    int totalPang;   // +8: earned Pang + bonus Pang; no additional multiplier
};
typedef char FightResultEntrySize[(sizeof(GolfFightResultEntry) == 12) ? 1 : -1];
typedef char FightResultEntryScoreOffset[(int)&((GolfFightResultEntry*)0)->score == 4 ? 1 : -1];
typedef char FightResultEntryPangOffset[(int)&((GolfFightResultEntry*)0)->totalPang == 8 ? 1 : -1];
}
}

// Create the fight result or retry menu appropriate to the round outcome.
// Requires rule/doc/player records. If every player+4 equals 1 (also true for an
// empty list), creates/stores rule.retryMenu, sets postRoundMenuState=2, returns 0.
// Otherwise returns a newly allocated owned fight menu; caller installs it in
// rule.resultMenu, polls MPBase1_Process and deletes/clears it on completion in
// ProcessPostRoundMenus. The retryMenu is likewise rule-owned and deleted there.
// Allocation may throw; 0 denotes the retry branch, not allocation failure.
// Non-retry input requires 1<=playerCount<=4 and high rank nibbles forming exactly
// the permutation 1..playerCount. Unique AND dense ranks are required: duplicates
// overwrite a row, gaps leave uninitialized rows, rank 0 underflows the array,
// and rank>count leaves a row read by SetEntries uninitialized (even if rank<=4).
// UpdatePlayerRanksAndHoleOrder computes high-nibble ranks with tie-break order
// separately from tied low-nibble places. Its normal ranking branches rely on
// distinct tie-break keys; playerState==3 can retain a prior rank. Party 1/2
// delegate ranking to their actors. Callers must supply a complete permutation
// after those paths too; neither producer nor setter checks or repairs it here.
// Exactly the first count entries must be initialized. Rows [count,4) may remain
// uninitialized because SetEntries copies only [0,count). Stack addresses do not
// escape that synchronous copy. Pang sum must fit signed int; score units and
// menu-mode selection match CreateResultMenu. Voice message 7 follows the copy.
void* CGolfRule::CreateFightResultOrRetryMenu() {
    using namespace ns4;
    int ready = 1;
    for (int i = 0; i < AsCGolfDoc((*(unsigned char**)&AsCSharedDoc(SharedDocBytes())->golfDoc))->playerCount; ++i) {
        if (((RoundRewardPlayerView*)GolfPlayer_GetEntryByIndex(CTL, (unsigned char)i))->isComputer != 1) {
            ready = 0;
            break;
        }
    }
    if (ready) {
        struct Panel {
            unsigned char bytes[0x28];
            // Construct the retry panel in mode 1.
            Panel() {
                ((CRetryMenu*)this)->__ct(1);
            }
        };
        void* panel = new Panel;
        (*(void**)&retryMenu) = panel;
        postRoundMenuState = 2;
        return 0;
    }
    int flag = 1;
    if (*(int*)&((CSharedDoc*)(SharedDocBytes()))->partyGameMode)
        flag = 2;
    else if (!AsCSharedDoc(SharedDocBytes())->golfRuleType)
        flag = 0;
    struct Box {
        unsigned char bytes[0x11C];
        // Construct the fight-result menu using the supplied mode.
        Box(int f) {
            __ct__16CResultFightMenuFPv(this, f);
        }
    };
    Box* box = new Box(flag);
    GolfFightResultEntry entries[4];
    unsigned int count = AsCGolfDoc((*(unsigned char**)&AsCSharedDoc(SharedDocBytes())->golfDoc))->playerCount;
    GolfFightResultEntry* e;
    unsigned int i;
    for (i = 0; i < count; ++i) {
        e = &entries[((((GolfDocSnapshot8013*)GolfPlayer_GetEntryByIndex(CTL, (unsigned char)i))->rankFlags >> 4) &
                      0xF) -
                     1];
        e->playerIndex = i;
        e->score = *(int*)&((GolfDocSnapshot8013*)GolfPlayer_GetEntryByIndex(CTL, (unsigned char)i))
                        ->roundResultWords[kRoundScoreWord];
        switch (*(int*)&((CSharedDoc*)(SharedDocBytes()))->partyGameMode) {
        case 1:
        case 3:
            e->score = ((GolfPartyResultPayload*)GolfPlayer_GetEntryByIndex(CTL, (unsigned char)i))->result.points;
            break;
        case 2:
            e->score =
                (int)((GolfPartyResultPayload*)GolfPlayer_GetEntryByIndex(CTL, (unsigned char)i))->result.distance;
            break;
        }
        unsigned char* a = (unsigned char*)GolfPlayer_GetEntryByIndex(CTL, (unsigned char)i);
        unsigned char* b = (unsigned char*)GolfPlayer_GetEntryByIndex(CTL, (unsigned char)i);
        e->totalPang = *(int*)&((GolfDocSnapshot8013*)(b))->roundResultWords[kRoundPangWord] +
                       *(int*)&((GolfDocSnapshot8013*)(a))->roundResultWords[kRoundBonusPangWord];
    }
    ((CResultFightMenu*)box)->SetEntries(entries, count);
    IActor_DispatchMessage(this, this, golfRuleStrings + kText_voice, 7, 0, 0, 0, 0);
    return box;
}

#pragma opt_common_subs off
#pragma opt_lifetimes off
#pragma opt_propagation off
namespace ns4 {
extern "C" {
// Update course-clear status after applying the best rank-reward category to a copied save record.
// Returns 0 outside normal gameMode. Copies the current save record to stack,
// checks the selected 32-category result set and marks the current category.
// Returns 1 only when that update completes the checked set, publishing clear status
// 1/2 in shared state. Existing all-complete records return 0; no save write here.
int GolfRule_UpdateCourseClearStatus() {
    unsigned char* mgr;
    unsigned char mode;
    int result;
    unsigned char* update;
    int value;
    int index;
    int i;
    int k;
    int clear;
    bool selected;
    unsigned char* entries;
    if ((*(int*)&AsCSharedDoc(g_sharedDoc)->gameMode) != 0)
        return 0;
    if (0 == GetSaveDataSlot()) {
        if (!s_saveDataInitialized) {
            SaveData_Ctor(s_saveData);
            s_saveDataInitialized = 1;
        }
        GetSaveDataSlot() = s_saveData;
    }
    void* source = SaveData_GetCurrentRecord(GetSaveDataSlot());
    unsigned char buffer[0x828];
    SaveRecord_CopyCtor(buffer, source);
    mgr = (unsigned char*)g_sharedDoc;
    result = 0;
    mode = ((CSharedDoc*)(mgr))->golfRuleType;
    selected = (mode == 1);
    entries = buffer + (selected ? 0x20 : 0);
    clear = 1;
    for (k = 0; k < 32; entries++, k++) {
        if (*(signed char*)(entries + 0x218)) {
            clear = 0;
            break;
        }
    }
    if (!clear) {
        update = (unsigned char*)g_sharedDoc;
        value = *(int*)(update + 0x3C);
        index = *(int*)&((CSharedDoc*)(update))->courseRecordIndex;
        (buffer + 0x218)[index] = (unsigned char)value;
        entries = buffer + (selected ? 0x20 : 0);
        result = 1;
        for (i = 0; i < 32; entries++, i++) {
            if (*(signed char*)(entries + 0x218)) {
                result = 0;
                break;
            }
        }
        if (result) {
            if (selected)
                *(int*)&((CSharedDoc*)(mgr))->pendingCourseClearNotification = 2;
            else
                *(int*)&((CSharedDoc*)(mgr))->pendingCourseClearNotification = 1;
        }
    }
    return result;
}
}
}

#pragma pop

// In-round screen/HUD coordinator: owns child panels and named overlays, dispatches
// actor messages, presents shot input/rewards/tutorial prompts, and draws overlays.
// Entry index for local mods (ABI names/addresses remain in symbols.txt):
// - Lifecycle/ownership: CScreen_Ctor -> OnPreLoadInit -> OnLoad/PollLoadComplete
//   -> InitializeLoadedLayouts -> OnProcess/OnDisplay -> OnDestroy. Children and
//   registry overlay payloads are released before effect attachments on teardown.
// - Controller/shot gauge: ScreenMessageId + CScreen_HandleMsg route commands;
//   HudPhase/HudPanel and OnProcess's named phases coordinate panel admission,
//   input, player/equipment reconciliation and feedback. Query payloads are
//   caller-owned writable storage; follow the individual switch arm's type.
// - Camera buttons: CScreen_UpdateCameraAndShotHints and HudMapCameraInterface;
//   CMapCamera2d implementations in map_camera_2d_panel.cpp define its latches/transition clock.
// - Frame rate/animations: OnProcess dt drives timers/blends, while panel update
//   methods may count calls. Preserve both clocks; this unit sets no retrace rate.
// - Widescreen/placement: OnDisplay, DrawClubModelOverlay, UpdateSwingCaption and
//   tutorial title code use WView viewport pixels. Change layout anchors with
//   their hit areas/projection consumers; the shared scalar pool is u80179180_data.
// - Overlay resources: ScreenOverlayRegistry_CreateNamedOverlay and local Lookup
//   and OnDestroy use the node schema below; resource keys live in screenOverlayStrings.
// Shared views: CScreen/CGolfDoc/CSharedDoc/WView; local Hud* views overlap these
// objects and borrow their storage. They do not confer ownership or new vtables.
// Callers supply constructed receivers, initialized globals and serialized actor
// dispatch. Keep resource polling, message order, float order and RNG calls intact.
// Keep nested phase helpers inline; their caller-owned scratch preserves the original code.
#pragma inline_max_size(100000)
#pragma inline_max_total_size(100000)
#include "game/golfball.h"
// Field offsets are checked in the class headers; original function addresses stay below.
#include "classes/CGolfDoc.h"
#define AsCGolfDoc(p) ((CGolfDoc*)(p))
namespace ScreenFields {
#include "classes/CScreen.h"
}
#define AsCScreen(p) ((ScreenFields::CScreen*)(p))
#include "classes/CMouseCursor.h"
#include "classes/PartyItemModifiers.h"
#include "classes/COption.h"
#include "classes/WView.h"
#include "classes/CSharedDoc.h"
#define AsCSharedDoc(p) ((CSharedDoc*)(p))

#include "sdk_protos.h"
#include "game/wvector.h"
#include "game/types.h"
#include "game/singletons.h"

extern "C" {
void ScreenOverlayRegistry_CreateNamedOverlay(void*, const char*, void*, void*);
int CScreen_PollLoadComplete(void*);
int CScreen_AreHudTransitionsComplete(void*);
void CScreen_InitializeLoadedLayouts(void*);
struct UpdateData;
void CScreen_UpdateShotMeterPresentation(UpdateData*);
void CScreen_UpdatePanelTransitions(void*);
void CScreen_UpdateCameraAndShotHints(void*);
void CScreen_UpdateClubChangeDisplay(void*);
void CScreen_UpdateSwingCaption(void*);
void CScreen_DrawImpactTimingSprite(void*);
void CScreen_StartDirectionalSwingFeedback(void*);
extern CSharedDoc* g_sharedDoc;
extern WView* g_view;
extern void *g_drawDriveLine, *g_mouseCursor;
COption* g_option;
}

extern "C" {

void* g_chatMsg;

extern "C" void* CHoleInfo_ShowCornerLabels(void*);

struct S8017A418 { float x, y, z, w; };

signed char s_bpCtrlInitialized;
extern char s_bpCtrl[];
extern char s_bpCtrlDtorRegistration_AuxMenu[];
void __ct__7CBpCtrlFv(void*);
extern "C" void CBpCtrl_dtor(void*, int);

extern "C" void* InstanceTable_Get();
extern "C" void* ClubModel_FindCachedForItem(void*, int);
extern "C" void* FxInterpreter_GetInstance();
extern "C" void* FxAttachments_CreateFromResourceTags(void*, void*, int);

extern "C" const float kFloat_0_AuxMenu, kFloat_6_2831855_Screen;

extern char screenOverlayStrings[];

// Byte offsets in the existing resource pool; retain its storage symbol and relocations.
// Message IDs handled by this screen; payload members preserve the original integer/pointer ABI.
enum ScreenMessageId {
    ScreenMsg_AddBallRingHeight = 0x0,
    ScreenMsg_RefreshGroundLie = 0x1,
    ScreenMsg_SetFirstHoleInfoDisplayEnabled = 0x2,
    ScreenMsg_SetSecondHoleInfoDisplayEnabled = 0x3,
    ScreenMsg_ShowTimedNotification = 0x4,
    ScreenMsg_SetHudDisplayEnabled = 0x5,
    ScreenMsg_ShowShotGauge = 0x6,
    ScreenMsg_HideShotGauge = 0x7,
    ScreenMsg_SetWindStrengthAndHeading = 0x8,
    ScreenMsg_SetShotPowerColor = 0x9,
    ScreenMsg_ShowFullPower = 0xa,
    ScreenMsg_ShowShotFeedback = 0xb,
    ScreenMsg_SetFeedbackOpacity = 0xc,
    ScreenMsg_SetTerrainProfile = 0xd,
    ScreenMsg_ResetForCurrentPlayer = 0xe,
    ScreenMsg_ResetShotPanels = 0xf,
    ScreenMsg_EnablePointerShotInput = 0x10,
    ScreenMsg_DisablePointerShotInput = 0x11,
    ScreenMsg_ResetShotFeedbackBlend = 0x12,
    ScreenMsg_ResetHoleInfo = 0x13,
    ScreenMsg_ShowCornerLabels = 0x14,
    ScreenMsg_CalculateShotRewards = 0x15,
    ScreenMsg_ResetRewardAnimations = 0x16,
    ScreenMsg_FinishRewardTotals = 0x17,
    ScreenMsg_CalculateHoleRewards = 0x18,
    ScreenMsg_QueryRewardDisplayComplete = 0x19,
    ScreenMsg_LoadBallResource = 0x1a,
    ScreenMsg_RequestShotTransition = 0x1b,
    ScreenMsg_StartTutorialStage = 0x1c,
    ScreenMsg_ResetTutorialTitle = 0x1d,
    ScreenMsg_SetTutorialCompletion = 0x1e,
    ScreenMsg_ResetIngameStatus = 0x1f,
    ScreenMsg_FlashHoleInfo = 0x20,
    ScreenMsg_SelectAuxiliaryMenuEntry = 0x21,
    ScreenMsg_ClearAuxiliaryMenuSelection = 0x22,
    ScreenMsg_QueryBallPreviewBasisRow = 0x23,
    ScreenMsg_QueryGroundLieBlend = 0x24,
    ScreenMsg_ShowIngameStatus = 0x25,
    ScreenMsg_HideIngameStatus = 0x26,
    ScreenMsg_SnapHoleDistance = 0x27,
    ScreenMsg_SetHoleInfoValues = 0x28,
    ScreenMsg_RefreshIngamePlayer = 0x29,
    ScreenMsg_StartSkinsResult = 0x2a,
    ScreenMsg_ResetShotCounterFade = 0x2b,
    ScreenMsg_SetCurrentPlayerDisplayValue = 0x2c,
    ScreenMsg_RefreshNearestPinPlayer = 0x2d,
    ScreenMsg_EnterDropMode = 0x2e,
    ScreenMsg_QueryHudPhase = 0x30,
    ScreenMsg_QueryStrokePanelReady = 0x31,
    ScreenMsg_QueryShotOptionResult = 0x32,
    ScreenMsg_QueryShotDirectionResult = 0x33,
    ScreenMsg_ConsumeShotOptionChanged = 0x34,
    ScreenMsg_QueryStrokeOption = 0x35,
    ScreenMsg_QueryShotOptionEnabled = 0x36,
    ScreenMsg_SetShotOptionEnabled = 0x37,
    ScreenMsg_ResumeAim = 0x38,
    ScreenMsg_BlockCameraInput = 0x39,
    ScreenMsg_QueryMapCameraState = 0x3a,
    ScreenMsg_QueryPanelsIdle = 0x3b,
    ScreenMsg_QueryAimPanelState = 0x3c,
    ScreenMsg_QueryMapCameraSelection = 0x3d,
    ScreenMsg_QueryCalipersPreview = 0x3e,
    ScreenMsg_QueryAlwaysZero = 0x3f,
    ScreenMsg_QueryPanelTransitionsReady = 0x40,
    ScreenMsg_SetAimControlsEnabled = 0x41,
    ScreenMsg_QueryCameraOrBallControlActive = 0x42,
    ScreenMsg_QueryItemPanelReady = 0x43,
    ScreenMsg_ResetAimPanelAction = 0x44,
    ScreenMsg_ResetMapCameraAction = 0x45,
    ScreenMsg_ConfirmMapCameraAction = 0x46,
    ScreenMsg_ShowTutorialMeter = 0x47,
    ScreenMsg_ShowTutorialMeterImmediately = 0x48,
    ScreenMsg_ShowTutorialGauge = 0x49,
    ScreenMsg_ShowTutorialGaugeImmediately = 0x4a,
    ScreenMsg_ShowTutorialAim = 0x4b,
    ScreenMsg_ShowTutorialAimImmediately = 0x4c,
    ScreenMsg_HideTutorialMeter = 0x4d,
    ScreenMsg_HideTutorialGauge = 0x4e,
    ScreenMsg_HideTutorialAim = 0x4f,
    ScreenMsg_SetTutorialClubChangeEnabled = 0x50,
    ScreenMsg_SetTutorialAimControlEnabled = 0x51,
    ScreenMsg_SetTutorialSpinControlEnabled = 0x52,
    ScreenMsg_ShowTutorialIllustration = 0x54,
    ScreenMsg_SetCameraControlsEnabled = 0x55,
    ScreenMsg_QueryDropMode = 0x56,
    ScreenMsg_ExitDropMode = 0x57,
    ScreenMsg_CancelDropMode = 0x58,
    ScreenMsg_SuspendHudDrawing = 0x59,
    ScreenMsg_ResumeHudDrawing = 0x5a,
    ScreenMsg_SetBallRingHeight = 0x2717,
};
enum TutorialTitlePhase { TutorialTitleFadingIn = 0, TutorialTitleMoving = 1, TutorialTitleArrived = 2 };
enum TutorialCompletionCommand { TutorialCompletionRestart = 0, TutorialCompletionFinishFlashing = 1, TutorialCompletionMarkVisible = 2 };
enum HudPhase { HudInactive = 0, HudShot = 1, HudAim = 2, HudItems = 3, HudSpin = 4, HudShotTransition = 5 };
// Panel selection is separate from HudPhase: requestedPanel is the desired widget
// group; previousPanel tracks the group being retired. 0 hides panels, 1 opens
// the aim group, 2 closes aim for a shot, 4 opens camera, 1000 items, 2000 spin.
// High item/spin bits survive low-nibble panel changes in OnProcess. Value 3
// occurs as an ignored previous-panel case; its independent role is unresolved.
enum HudPanel { HudPanelNone = 0, HudPanelAim = 1, HudPanelAimClosing = 2, HudPanelCamera = 4, HudPanelItems = 0x1000, HudPanelSpin = 0x2000 };
// Bits of requestedPanel above the low panel nibble (items/spin); preserved when the nibble is replaced.
enum HudPanelMasks { HudPanelHighBitsMask = 0xFFF0 };
// Tutorial aim/meter/gauge blink counters stop counting once they reach this value.
enum HudBlink { TutorialBlinkLimit = 6 };
enum HudFlag { HudShotGaugeVisible = 1, HudPointerShotInputEnabled = 2 };
enum HudGameStateMask {
    HudActiveLayoutState = 0x08000000, HudSuppressDrawingState = 0x2000,
    HudMeterFadeStates = 0x1080, HudItemMenuAllowedStates = 0x44308, HudCommandGlyphStates = 0x60
};
enum HudLimits { TutorialStageCount = 6, ClubOverlayCount = 17, TimebombFramesPerTick = 18, FullOpacity = 255 };
enum HudLogicalAction {
    HudActionOpenItems = 7, HudActionCamera = 0x18, HudActionHoldCamera = 0x19,
    HudActionAim = 0x1A, HudActionSpin = 0x1F, HudActionCancelPanel = 0x23
};
// The blink cycles below count update calls; do not convert them to elapsed-time timers.
enum PululuAnimationPhase { PululuHidden = 0, PululuEntering = 1, PululuLooping = 2, PululuLeaving = 3 };

enum ScreenStringOffset {
    ScreenText_BallWindScript = 0x0,
    ScreenText_PartyMeterScript = 0x12,
    ScreenText_CountdownScript = 0x29,
    ScreenText_PululuJarashiScript = 0x37,
    ScreenText_FeatherScript = 0x4a,
    ScreenText_SwingCaptionLayout = 0x5d,
    ScreenText_BallConditionKey = 0x6f,
    ScreenText_BallConditionResource = 0x7a,
    ScreenText_PlayMode0Key = 0x88,
    ScreenText_PlayMode0Resource = 0x93,
    ScreenText_PlayMode1Key = 0xa4,
    ScreenText_PlayMode1Resource = 0xb3,
    ScreenText_PlayMode4Key = 0xc4,
    ScreenText_PlayMode4Resource = 0xcf,
    ScreenText_SpinPointKey = 0xe0,
    ScreenText_SpinPointResource = 0xef,
    ScreenText_WoodClubKey = 0x105,
    ScreenText_WoodClubResource = 0x113,
    ScreenText_IronClubKey = 0x124,
    ScreenText_IronClubResource = 0x134,
    ScreenText_PutterClubKey = 0x145,
    ScreenText_PutterClubResource = 0x152,
    ScreenText_TopCameraKey = 0x163,
    ScreenText_TopCameraResource = 0x172,
    ScreenText_FreeCameraKey = 0x185,
    ScreenText_FreeCameraResource = 0x196,
    ScreenText_TargetCameraKey = 0x1aa,
    ScreenText_TargetCameraResource = 0x1bb,
    ScreenText_ClickIndicatorKey = 0x1d1,
    ScreenText_ClickIndicatorResource = 0x1de,
    ScreenText_CommandKeysResource = 0x1ec,
    ScreenText_DownSmallGlyph = 0x1f5,
    ScreenText_LeftSmallGlyph = 0x1fc,
    ScreenText_RightSmallGlyph = 0x203,
    ScreenText_UpSmallGlyph = 0x20b,
    ScreenText_DownGlyph = 0x210,
    ScreenText_LeftGlyph = 0x215,
    ScreenText_RightGlyph = 0x21a,
    ScreenText_UpGlyph = 0x220,
    ScreenText_InvalidGlyph = 0x223,
    ScreenText_WindResource = 0x22b,
    ScreenText_TutorialTitleResource = 0x234,
    ScreenText_TutorialTitleFormat = 0x247,
    ScreenText_CancelClickSound = 0x253,
    ScreenText_CameraActor = 0x26a,
    ScreenText_IconClickSound = 0x271,
    ScreenText_SpinActor = 0x27f,
    ScreenText_OpenSpinDialogSound = 0x284,
    ScreenText_ItemActor = 0x299,
    ScreenText_TutorialActor = 0x29e,
    ScreenText_CloseSpinDialogSound = 0x2a7,
    ScreenText_TimebombTickSound = 0x2bd,
    ScreenText_TimebombFailureSound = 0x2d4,
    ScreenText_PlayerActor = 0x2e8,
    ScreenText_GolfRuleActor = 0x2ef,
    ScreenText_ScoreActor = 0x2f8,
    ScreenText_OpenPlayMenuSound = 0x2fe,
    ScreenText_SpotActor = 0x30d,
    ScreenText_PululuEnterAnimation = 0x312,
    ScreenText_PululuExitAnimation = 0x31f,
    ScreenText_PululuLoopAnimation = 0x32d,
    ScreenText_CountdownNumberGroup = 0x33b,
    ScreenText_PatternKey = 0x350,
    ScreenText_PracticeOptionsKey = 0x35b,
    ScreenText_SmallGlyphSuffix = 0x370,
    ScreenText_UpdatedText = 0x373,
    ScreenText_CountdownFontResource = 0x37d,
    ScreenText_CountdownAnimation = 0x390,
    ScreenText_EmptyText = 0x39b,
    ScreenText_ClubChangeOutAnimation = 0x39c,
    ScreenText_ClubChangeInAnimation = 0x3ab,
    ScreenText_RedMeterAnimation = 0x3b9,
    ScreenText_BlueMeterAnimation = 0x3ca,
    ScreenText_YellowMeterAnimation = 0x3dc,
    ScreenText_FeatherAnimation = 0x3f0,
};

extern void* SharedDoc_GetCurrent();
extern void* GolfDoc_GetCurrent();
extern void* SharedDoc_GetGolfRuleType();
extern void CBpCtrl_CreateEntry(void*, char*, void**, int);
extern void* IdMap_Lookup(void*, void*);
extern void* Layout_LoadExtended(void*, int, char*);
extern void* OverlayPlayer_Init(void*, char*);
extern void* Register_NormalizedNamedRectangle(void*, char*, void*, int);
extern void* ResourceManager_LoadByExtension(void*, char*, int, int, int);
extern void WPuppet_ReservedHook(void*, float, float, void*);
extern "C" char* String_FormatTemporary(const char*, ...);
extern void Layout_Update(void*);
extern "C" void* PartyItemSlots_GetInstance();
extern void PartyItemSlots_RefreshDisplay(void*);
extern void* __nw__FUl(unsigned int);
extern void CTutorialIllust2d_Ctor(void*);
extern void CHoleInfoSet_Init(void*);
extern void ScreenBallPreview_Ctor(void*);
extern void CPangInfo_Ctor(void*);
extern void CIngameStatus_Ctor(void*);
extern void CWindInfo_Ctor(void*);
extern void CAccuracyInfo_Ctor(void*);
extern void __ct__10CPowerInfoFv(void*);
extern void __ct__8CFlyInfoFv(void*);
extern void __ct__12CBalloonInfoFv(void*);
extern void CDriveInfo_Ctor(void*);
extern void __ct__10CAuxMenuUIFv(void*);
extern void __ct__15CRemainTimeInfoFv(void*);
extern void __ct__9CCalipersFv(void*);
extern void CHelpMsg_Ctor(void*);
extern void* g_resourceManager;

extern const float kScreenPulseWrapThreshold; // 20.0f: positive wrap threshold for the shot indicator pulse
extern const float kScreenPulseWrapSpan; // 40.0f: pulse wrap span / command glyph Y offset
extern const float kFloat_50_Screen;
extern const float kFloat_5_Screen;
extern const float kScreenTutorialTitleRowHeight; // 24.0f: tutorial title row height
extern const float kScreenTutorialTitleWidth; // 177.0f: tutorial title rectangle width
extern const float kScreenTutorialBlinkInterval; // 0.25f: tutorial blink interval

extern void Layout_Destroy(void*, int);
extern void CBpCtrl_DeleteEntryById(void*, int);
extern void UITextTable_ClearEntries(void*);
extern void PointerArray_DeleteValue(void*, void*);
extern void CHoleInfoSet_Dtor(void*, int);
extern void ScreenBallPreview_dtor(void*, int);
extern void CIngameStatus_Dtor(void*, int);
extern void OverlayPlayer_Destroy(void*, int);
extern void ResourceManager_ReleaseObject(void*, void*, int);
extern void Object_DeleteVirtual(void*, void*);

extern "C" void FxInterpreter_RemoveEntriesByKey(void*, int);

extern void* g_helpMsg;

extern signed char s_pointerVectorInitialized;
extern char s_fontNumDrawRegistry[];
extern char s_fontNumDrawRegistryDtorRegistration[];
extern void PointerVector_InitReserved(char*);
extern void PointerVector_DestroyStorage();
extern void __dl__FPv(void*);
void CScreen_OnDestroy(void* p);
void* CBpCtrl_GetInstance();
void CScreen_OnPreLoadInit(void* arg0);
void CScreen_OnLoad(void* p);
int Option_UseIntegratedClubOverlay();
S8017A418* Vec4_Set(S8017A418* arg0, float a, float b, float c, float d);
float Option_GetHudBaselineY(void* arg0);
int Option_PlaceWindIndicatorOnLoad();
float WView_GetViewportWidth(void* arg0);
void* ChatMessage_GetDefaultFontResource(void* arg0);
void* HoleInfoSet_ShowCornerLabels(void* arg0);
void* Option_GetInstance();
void* ChatMessage_GetCurrent();
void* CScreen_OnInit(void* p);

struct InstanceLock_u80179180 {
    void* lock;
    // Hold one registry mutex level for the lifetime of this guard.
    InstanceLock_u80179180(void* l) { lock = l; OSLockMutex((OSMutex*)(l)); }
    // Release the registry mutex level acquired by this guard.
    ~InstanceLock_u80179180() { OSUnlockMutex((OSMutex*)(lock)); }
};

// Fields absent from the shared header; no storage, constructors, or virtual members.
struct ScreenExtraState {
    char padding0[0x5c];
    float ballRingHeight; // 0x5c
    char padding60[0x58];
    unsigned char initialDisplayEnabled; // 0xb8
    char paddingb9[0x7];
    int feedbackColor0; // 0xc0
    int feedbackColor1; // 0xc4
    char paddingc8[0x4];
    int terrainMarkerIndex; // 0xcc
    int terrainSampleCount; // 0xd0
    char paddingd4[0x4];
    float shotIndicatorPulse; // 0xd8
    char paddingdc[0xa8];
    float terrainProfileScale; // 0x184
    char padding188[0x4];
    float shotFeedbackBlend; // 0x18c
    float groundLieBlend; // 0x190
    unsigned char tutorialStageVisible[8]; // 0x194
    char padding19c[0x18];
    int previousTutorialStage; // 0x1b4
    float cameraCaptionOpacity; // 0x1b8
    char padding1bc[0x18];
    unsigned char tutorialPointerDelayed; // 0x1d4
    char padding1d5[0x13];
    float clubChangeAngle; // 0x1e8
    int clubChangeAnimationDirection; // 0x1ec
    float clubChangeAnimationTime; // 0x1f0
    char padding1f4[0x18];
    char tutorialTitleKeys[5][16]; // 0x20c
    char padding25c[0x4];
    int panelOpenRequested; // 0x260
    int itemPanelOpenRequested; // 0x264
    int itemPanelReady; // 0x268
    char padding26c[0x4];
    int shotOptionChanged; // 0x270
    int resumeAimRequested; // 0x274
    int hudInputBlocked; // 0x278
    int panelsReady; // 0x27c
    int tutorialMeterEnabled; // 0x280
    char padding284[0x4];
    int tutorialAimEnabled; // 0x288
    char padding28c[0x4];
    int tutorialAimControlEnabled; // 0x290
    int tutorialSpinControlEnabled; // 0x294
    char padding298[0x4];
    int tutorialIllustrationActive; // 0x29c
    int cameraControlsEnabled; // 0x2a0
    int aimControlsEnabled; // 0x2a4
    int dropModeActive; // 0x2a8
    int shotTransitionRequested; // 0x2ac
    char padding2b0[0x2c];
    int pululuAnimationPhase; // 0x2dc
    int meterFadeFrame; // 0x2e0
    int impactFeedbackStarted; // 0x2e4
    int tutorialAimBlinkFrame; // 0x2e8
    int tutorialAimBlinkCount; // 0x2ec
    int tutorialMeterBlinkFrame; // 0x2f0
    int tutorialMeterBlinkCount; // 0x2f4
    int tutorialGaugeBlinkFrame; // 0x2f8
    int tutorialGaugeBlinkCount; // 0x2fc
    char padding300[0x8];
    int requestedPanel; // 0x308
    int savedPanel; // 0x30c
    int previousPanel; // 0x310
    char padding314[0x4];
    int previousHudPhase; // 0x318
    int shotOptionResult; // 0x31c
    int shotDirectionResult; // 0x320
    int partyMeterIndex; // 0x324
    int meterOpacity; // 0x328
};
struct ScreenRegistryLock { char padding[0x84]; OSMutex mutex; };
struct OverlayRegistryNodeView { void* value; const char* key; unsigned char flag; char padding[3]; void* next; void* previous; void* bucketNext; };
struct NamedOverlayView { char name[32]; void* resource; };
// The instance-table selection is a club snapshot, not a GameInputManager or CGolfPlayer actor.
struct ClubSelectionSnapshot {
    char padding0[8];
    unsigned char clubIndex;
    char padding9;
    unsigned char clubKind;
    char paddingB[0x15];
    float baseImpactRange;
};
struct ClubSelectionState {
    char padding0[4];
    ClubSelectionSnapshot* selection;
    char padding8[0x3C];
    int shotOptionEnabled;
    char padding48[4];
    unsigned int changeFlags;
    char padding50[0x10];
    int clubMenuState;
};
struct PowerInfoView { char padding0[0xC]; unsigned int color; char padding10[8]; WVector position; };
struct BallPreviewMaterialView { char padding[0x1AC]; void* material; };
struct ClubResourceSlots { char padding[0x14]; void* byClubKind[4]; };

struct ChatMessageView { char padding[0x10]; void* defaultFontResource; };
struct HoleInfoSetView { void* cup; void* distance; void* cornerLabels; void* accuracy; };
template<int VtableOffset> struct ScreenVtableView { char padding[VtableOffset]; void** vtable; };

struct VDel0 { 
virtual void del(int);
};
struct VDel330 { char pad[0xe0]; 
virtual void del(int);
};
struct VDel334 { char pad[0x9c]; 
virtual void del(int);
};
struct VDel338 { char pad[0xa4]; 
virtual void del(int);
};
struct VDel33C { char pad[0x164]; 
virtual void del(int);
};
struct VDel340 { char pad[0x74]; 
virtual void del(int);
};
struct VDel344 { char pad[0x78]; 
virtual void del(int);
};

#pragma use_lmw_stmw on
#pragma opt_propagation off
#pragma opt_common_subs off
// Destroy HUD layouts and scripts, release resources, and clear the named-overlay registry.
extern void CScreen_OnDestroy(void* p) {
    char* a = (char*)p;
    void* self = p;
    /* 0x32c */
    // Phase 1: destroy core HUD children and clear their retained pointers.
    if (AsCScreen(a)->layout) { Layout_Destroy(AsCScreen(a)->layout,1); AsCScreen(a)->layout=0; }
    /* 0x330 */
    if (AsCScreen(a)->strokePointDisplay) { if (AsCScreen(a)->strokePointDisplay) ((VDel330*)AsCScreen(a)->strokePointDisplay)->del(1); AsCScreen(a)->strokePointDisplay=0; }
    /* 0x334 */
    if (AsCScreen(a)->mapCameraDisplay) { if (AsCScreen(a)->mapCameraDisplay) ((VDel334*)AsCScreen(a)->mapCameraDisplay)->del(1); AsCScreen(a)->mapCameraDisplay=0; }
    /* 0x338 */
    if (AsCScreen(a)->ballWindDisplay) { if (AsCScreen(a)->ballWindDisplay) ((VDel338*)AsCScreen(a)->ballWindDisplay)->del(1); AsCScreen(a)->ballWindDisplay=0; }
    /* 0x33c */
    if (AsCScreen(a)->golfBaseDisplay) { if (AsCScreen(a)->golfBaseDisplay) ((VDel33C*)AsCScreen(a)->golfBaseDisplay)->del(1); AsCScreen(a)->golfBaseDisplay=0; }
    /* 0x340 */
    if (AsCScreen(a)->swingResultDisplay) { if (AsCScreen(a)->swingResultDisplay) ((VDel340*)AsCScreen(a)->swingResultDisplay)->del(1); AsCScreen(a)->swingResultDisplay=0; }
    /* 0x344 */
    if (AsCScreen(a)->tutorialIllustration) { if (AsCScreen(a)->tutorialIllustration) ((VDel344*)AsCScreen(a)->tutorialIllustration)->del(1); AsCScreen(a)->tutorialIllustration=0; }
    /* 0x2b4 */
    // Phase 2: remove BP script entries, then release optional panels/resources.
    if (AsCScreen(a)->ballWindScriptId!=-1) {
        if (!((int&)Sing_805DC5F0())) {
            if (!s_bpCtrlInitialized) {
                __ct__7CBpCtrlFv(s_bpCtrl);
                __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, s_bpCtrlDtorRegistration_AuxMenu);
                s_bpCtrlInitialized = 1;
            }
            ((int&)Sing_805DC5F0()) = (int)s_bpCtrl;
        }
        CBpCtrl_DeleteEntryById((void*)((int&)Sing_805DC5F0()), AsCScreen(a)->ballWindScriptId);
        AsCScreen(a)->ballWindScriptId = -1;
    }
    /* 0x2b8 */
    if (AsCScreen(a)->partyGolfBaseScriptId!=-1) {
        if (!((int&)Sing_805DC5F0())) {
            if (!s_bpCtrlInitialized) {
                __ct__7CBpCtrlFv(s_bpCtrl);
                __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, s_bpCtrlDtorRegistration_AuxMenu);
                s_bpCtrlInitialized = 1;
            }
            ((int&)Sing_805DC5F0()) = (int)s_bpCtrl;
        }
        CBpCtrl_DeleteEntryById((void*)((int&)Sing_805DC5F0()), AsCScreen(a)->partyGolfBaseScriptId);
        AsCScreen(a)->partyGolfBaseScriptId = -1;
    }
    /* 0x2bc */
    if (AsCScreen(a)->timebombScriptId!=-1) {
        if (!((int&)Sing_805DC5F0())) {
            if (!s_bpCtrlInitialized) {
                __ct__7CBpCtrlFv(s_bpCtrl);
                __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, s_bpCtrlDtorRegistration_AuxMenu);
                s_bpCtrlInitialized = 1;
            }
            ((int&)Sing_805DC5F0()) = (int)s_bpCtrl;
        }
        CBpCtrl_DeleteEntryById((void*)((int&)Sing_805DC5F0()), AsCScreen(a)->timebombScriptId);
        AsCScreen(a)->timebombScriptId = -1;
    }
    /* 0x2c0 */
    if (AsCScreen(a)->pululuJarashiScriptId!=-1) {
        if (!((int&)Sing_805DC5F0())) {
            if (!s_bpCtrlInitialized) {
                __ct__7CBpCtrlFv(s_bpCtrl);
                __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, s_bpCtrlDtorRegistration_AuxMenu);
                s_bpCtrlInitialized = 1;
            }
            ((int&)Sing_805DC5F0()) = (int)s_bpCtrl;
        }
        CBpCtrl_DeleteEntryById((void*)((int&)Sing_805DC5F0()), AsCScreen(a)->pululuJarashiScriptId);
        AsCScreen(a)->pululuJarashiScriptId = -1;
    }
    /* 0x2c4 */
    if (AsCScreen(a)->featherBadgeScriptId!=-1) {
        if (!((int&)Sing_805DC5F0())) {
            if (!s_bpCtrlInitialized) {
                __ct__7CBpCtrlFv(s_bpCtrl);
                __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, s_bpCtrlDtorRegistration_AuxMenu);
                s_bpCtrlInitialized = 1;
            }
            ((int&)Sing_805DC5F0()) = (int)s_bpCtrl;
        }
        CBpCtrl_DeleteEntryById((void*)((int&)Sing_805DC5F0()), AsCScreen(a)->featherBadgeScriptId);
        AsCScreen(a)->featherBadgeScriptId = -1;
    }
    /* 0xad8 check */
    if ((*(int*)&AsCSharedDoc(g_sharedDoc)->partyGameMode)==3) {
        UITextTable_ClearEntries(AsCScreen(a)->textTable);
        if (AsCScreen(a)->textTable) {
            if (!Sing_805DC830()) {
                if (!s_pointerVectorInitialized) {
                    PointerVector_InitReserved(s_fontNumDrawRegistry);
                    __register_global_object(s_fontNumDrawRegistry, (void*)PointerVector_DestroyStorage, s_fontNumDrawRegistryDtorRegistration);
                    s_pointerVectorInitialized = 1;
                }
                Sing_805DC830() = s_fontNumDrawRegistry;
            }
            PointerArray_DeleteValue(Sing_805DC830(), AsCScreen(a)->textTable);
        }
    }
    /* 0x1c */
    if (AsCScreen(a)->holeInfoSet) { CHoleInfoSet_Dtor(AsCScreen(a)->holeInfoSet,1); AsCScreen(a)->holeInfoSet=0; }
    /* 0x24 */
    if (AsCScreen(a)->ballPreview) { ScreenBallPreview_dtor(AsCScreen(a)->ballPreview,1); AsCScreen(a)->ballPreview=0; }
    /* 0x28 */
    { void* o=AsCScreen(a)->windInfo; if(o){ if(o) ((VDel0*)o)->del(1); AsCScreen(a)->windInfo=0; } }
    /* 0x2c */
    { void* o=AsCScreen(a)->remainingTimeInfo; if(o){ if(o) ((VDel0*)o)->del(1); AsCScreen(a)->remainingTimeInfo=0; } }
    /* 0x3c */
    if (AsCScreen(a)->ingameStatus) { CIngameStatus_Dtor(AsCScreen(a)->ingameStatus,1); AsCScreen(a)->ingameStatus=0; }
    /* 0x40 */
    { void* o=AsCScreen(a)->auxiliaryMenu; if(o){ if(o) ((VDel0*)o)->del(1); AsCScreen(a)->auxiliaryMenu=0; } }
    /* 0x34 */
    { void* o=AsCScreen(a)->accuracyInfo; if(o){ if(o) ((VDel0*)o)->del(1); AsCScreen(a)->accuracyInfo=0; } }
    /* 0x44 */
    { void* o=AsCScreen(a)->pangInfo; if(o){ if(o) ((VDel0*)o)->del(1); AsCScreen(a)->pangInfo=0; } }
    /* 0x30 */
    { void* o=AsCScreen(a)->powerInfo; if(o){ if(o) ((VDel0*)o)->del(1); AsCScreen(a)->powerInfo=0; } }
    /* 0x20 */
    { void* o=AsCScreen(a)->flyInfo; if(o){ if(o) ((VDel0*)o)->del(1); AsCScreen(a)->flyInfo=0; } }
    /* 0x50 */
    { void* o=AsCScreen(a)->modeInfo; if(o){ if(o) ((VDel0*)o)->del(1); AsCScreen(a)->modeInfo=0; } }
    /* 0x54 */
    if (AsCScreen(a)->modeLabelRenderer) { OverlayPlayer_Destroy(AsCScreen(a)->modeLabelRenderer,1); AsCScreen(a)->modeLabelRenderer=0; }
    /* 0x58 */
    if (AsCScreen(a)->statusTextRenderer) { OverlayPlayer_Destroy(AsCScreen(a)->statusTextRenderer,1); AsCScreen(a)->statusTextRenderer=0; }
    /* 0x1f4 */
    { void* o=AsCScreen(a)->calipers; if(o){ if(o) ((VDel0*)o)->del(1); AsCScreen(a)->calipers=0; } }
    /* 0x48 */
    { void* o=AsCScreen(a)->skinsInfo; if(o){ if(o) ((VDel0*)o)->del(1); AsCScreen(a)->skinsInfo=0; } }
    /* 0x4c */
    { void* o=AsCScreen(a)->nearestPinInfo; if(o){ if(o) ((VDel0*)o)->del(1); AsCScreen(a)->nearestPinInfo=0; } }
    /* g_helpMsg */
    if (g_helpMsg) { if (g_helpMsg) ((VDel0*)g_helpMsg)->del(1); }
    /* g_resourceManager */
    if (g_resourceManager && AsCScreen(a)->windIndicatorResource) { ResourceManager_ReleaseObject(g_resourceManager, AsCScreen(a)->windIndicatorResource, 0); AsCScreen(a)->windIndicatorResource=0; }
    // Phase 3: unlink named-overlay registry rings/buckets under the recursive mutex.
    // Registry nodes and their overlay values have separate cleanup steps below.
    // 1. Traverse previous links; a return to head ends iteration. Destroy overlay
    //    resources outside the iterator lock, then release each overlay wrapper.
    // 2. Splice the active circular ring into the free ring when both exist;
    //    otherwise move nodes one by one, repairing both neighboring links.
    // 3. Clear every bucket pointer. Recycled nodes remain owned by the registry;
    //    they are not overlay objects and must not be freed as overlay values.
    // The outer lock and explicit inner mutex calls intentionally retain the
    // existing recursive-lock nesting. This is serial teardown, not concurrent
    // mutation support; callback/resource destruction must keep this ordering.
    /* WInstanceLock block 0x84 */
    {
        void* overlay;
        {
            InstanceLock_u80179180 iteratorLock((char*)&((ScreenRegistryLock*)self)->mutex);
            void* mutex = (char*)&((ScreenRegistryLock*)self)->mutex;
            AsCScreen(a)->nameHashIterator = AsCScreen(a)->nameHashHead;
            OSLockMutex((OSMutex*)mutex);
            {
                void* node = AsCScreen(a)->nameHashIterator;
                if (node) {
                    void* previousNode = ((OverlayRegistryNodeView*)node)->previous;
                    void* head = AsCScreen(a)->nameHashHead;
                    overlay = ((OverlayRegistryNodeView*)node)->value;
                    if (previousNode == head) previousNode = 0;
                    AsCScreen(a)->nameHashIterator = previousNode;
                } else {
                    overlay = 0;
                }
            }
            OSUnlockMutex((OSMutex*)mutex);
        }
        if (overlay) {
            void* mutex = (char*)&((ScreenRegistryLock*)self)->mutex;
            int unusedZero = 0;
            do {
                if (overlay) {
                    if (g_resourceManager && ((NamedOverlayView*)overlay)->resource) {
                        Object_DeleteVirtual(g_resourceManager, ((NamedOverlayView*)overlay)->resource);
                        ((NamedOverlayView*)overlay)->resource = 0;
                    }
                    __dl__FPv(overlay);
                }
                {
                    InstanceLock_u80179180 iteratorLock(mutex);
                    void* node = AsCScreen(a)->nameHashIterator;
                    if (node) {
                        void* previousNode = ((OverlayRegistryNodeView*)node)->previous;
                        void* head = AsCScreen(a)->nameHashHead;
                        overlay = ((OverlayRegistryNodeView*)node)->value;
                        if (previousNode == head) previousNode = 0;
                        AsCScreen(a)->nameHashIterator = previousNode;
                    } else {
                        overlay = 0;
                    }
                }
            } while (overlay);
        }
        // Recycle the now payload-free node ring.
        InstanceLock_u80179180 ringLock((char*)&((ScreenRegistryLock*)self)->mutex);
        void* activeHead = AsCScreen(a)->nameHashHead;
        if (activeHead && AsCScreen(a)->nameHashFreeList) {
            void* activeAnchor = activeHead;
            void* freeAnchor = AsCScreen(a)->nameHashFreeList;
            void* activeNext = ((OverlayRegistryNodeView*)activeAnchor)->next;
            void* freePrevious = ((OverlayRegistryNodeView*)freeAnchor)->previous;
            ((OverlayRegistryNodeView*)activeNext)->previous = freePrevious;
            ((OverlayRegistryNodeView*)freePrevious)->next = activeNext;
            ((OverlayRegistryNodeView*)freeAnchor)->previous = activeAnchor;
            ((OverlayRegistryNodeView*)activeAnchor)->next = freeAnchor;
            AsCScreen(a)->nameHashHead = 0;
            activeHead = 0;
        }
        if (activeHead) {
            void* mutex = (char*)&((ScreenRegistryLock*)self)->mutex;
            void* remainingHead = 0;
            void* node = AsCScreen(a)->nameHashHead;
            void* freeHead;
            while (node) {
                {
                    InstanceLock_u80179180 nodeLock(mutex);
                    void* nextNode = ((OverlayRegistryNodeView*)node)->next;
                    void* previousNode = ((OverlayRegistryNodeView*)node)->previous;
                    ((OverlayRegistryNodeView*)previousNode)->next = nextNode;
                    ((OverlayRegistryNodeView*)nextNode)->previous = previousNode;
                    remainingHead = ((OverlayRegistryNodeView*)node)->previous;
                    if (remainingHead == node) remainingHead = 0;
                }
                node = AsCScreen(a)->nameHashHead;
                freeHead = AsCScreen(a)->nameHashFreeList;
                {
                    InstanceLock_u80179180 nodeLock(mutex);
                    if (!freeHead) {
                        ((OverlayRegistryNodeView*)node)->next = node;
                        ((OverlayRegistryNodeView*)node)->previous = node;
                    } else {
                        ((OverlayRegistryNodeView*)node)->previous = freeHead;
                        void* nextNode = ((OverlayRegistryNodeView*)freeHead)->next;
                        ((OverlayRegistryNodeView*)node)->next = nextNode;
                        ((OverlayRegistryNodeView*)freeHead)->next = node;
                        void* relinkedNext = ((OverlayRegistryNodeView*)node)->next;
                        ((OverlayRegistryNodeView*)relinkedNext)->previous = node;
                    }
                }
                if (freeHead) node = freeHead;
                AsCScreen(a)->nameHashFreeList = node;
                AsCScreen(a)->nameHashHead = remainingHead;
                node = AsCScreen(a)->nameHashHead;
            }
        }
        // Bucket storage is a target pointer array (four bytes per entry).
        {
            int bucketIndex = 0;
            int bucketByteOffset = 0;
            int unusedZero = 0;
            while (bucketIndex < AsCScreen(a)->nameHashBucketCount) {
                *(int*)((char*)AsCScreen(a)->nameHashBuckets + bucketByteOffset) = 0; /* stwx */
                bucketIndex++;
                bucketByteOffset += 4;
            }
        }
    }
    /* trailing */
    void* _r = FxInterpreter_GetInstance();
    // Phase 4: remove any remaining effect attachments owned by this screen key.
    FxInterpreter_RemoveEntriesByKey(_r, AsCScreen(a)->fxInterpreterKey);
}
#pragma opt_common_subs reset
#pragma opt_propagation reset
#pragma use_lmw_stmw reset

// Lazily construct the shared BP script controller and register its destructor.
void* CBpCtrl_GetInstance() {
	void* result;
	if (((int&)Sing_805DC5F0()) == 0) {
		if (s_bpCtrlInitialized == 0) {
			__ct__7CBpCtrlFv(s_bpCtrl);
			__register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, s_bpCtrlDtorRegistration_AuxMenu);
			s_bpCtrlInitialized = 1;
		}
		result = s_bpCtrl;
		((int&)Sing_805DC5F0()) = (int)result;
	}
	return (void*)((int&)Sing_805DC5F0());
}

// Enable HUD display before loading the screen resources.
void CScreen_OnPreLoadInit(void* screen) {
	AsCScreen(screen)->hudDisplayEnabled = 1;
}

// Load HUD scripts and overlays, construct optional panels, and wait for all layout resources.
void CScreen_OnLoad(void* p) {
	char* a=(char*)p;
	void* r27=((void*(*)())CBpCtrl_GetInstance)();
	// Phase 1: load shared/party scripts and begin loading core child layouts.
	CBpCtrl_CreateEntry(r27,screenOverlayStrings,(void**)(a+0x2b4),0);
	AsCScreen(a)->ballWindScript=IdMap_Lookup(r27,(*(void**)&AsCScreen(a)->ballWindScriptId));
	if ((*(int*)&AsCSharedDoc(SharedDoc_GetCurrent())->partyGameMode)==3) {
		CBpCtrl_CreateEntry(r27,screenOverlayStrings + ScreenText_PartyMeterScript,(void**)(a+0x2b8),0);
		CBpCtrl_CreateEntry(r27,screenOverlayStrings + ScreenText_CountdownScript,(void**)(a+0x2bc),0);
		CBpCtrl_CreateEntry(r27,screenOverlayStrings + ScreenText_PululuJarashiScript,(void**)(a+0x2c0),0);
		CBpCtrl_CreateEntry(r27,screenOverlayStrings + ScreenText_FeatherScript,(void**)(a+0x2c4),0);
		AsCScreen(a)->partyGolfBaseScript=IdMap_Lookup(r27,(*(void**)&AsCScreen(a)->partyGolfBaseScriptId));
		AsCScreen(a)->timebombScript=IdMap_Lookup(r27,(*(void**)&AsCScreen(a)->timebombScriptId));
		AsCScreen(a)->pululuJarashiScript=IdMap_Lookup(r27,(*(void**)&AsCScreen(a)->pululuJarashiScriptId));
		AsCScreen(a)->featherBadgeScript=IdMap_Lookup(r27,(*(void**)&AsCScreen(a)->featherBadgeScriptId));
	}
	if (AsCGolfDoc(GolfDoc_GetCurrent())->tutorialStage<TutorialStageCount) { if (!(*(int*)&AsCScreen(a)->tutorialIllustration)) {
		void* r29=__nw__FUl(0x7c);
		if (r29) CTutorialIllust2d_Ctor(r29);
		AsCScreen(a)->tutorialIllustration=r29;
	} }
	Layout_LoadExtended(AsCScreen(a)->layout,0,screenOverlayStrings + ScreenText_SwingCaptionLayout);
	{
		void* o=AsCScreen(a)->strokePointDisplay; ((void(*)(void*,int))(((ScreenVtableView<0xe0>*)o)->vtable)[3])(o,0);
	}
	{
		void* o=AsCScreen(a)->mapCameraDisplay; ((void(*)(void*,int))(((ScreenVtableView<0x9c>*)o)->vtable)[3])(o,0);
	}
	{
		void* o=AsCScreen(a)->ballWindDisplay; ((void(*)(void*,int))(((ScreenVtableView<0xa4>*)o)->vtable)[3])(o,0);
	}
	{
		void* o=AsCScreen(a)->golfBaseDisplay; ((void(*)(void*,int))(((ScreenVtableView<0x164>*)o)->vtable)[3])(o,0);
	}
	{
		void* o=AsCScreen(a)->swingResultDisplay; ((void(*)(void*,int))(((ScreenVtableView<0x74>*)o)->vtable)[3])(o,0);
	}
	if (AsCScreen(a)->tutorialIllustration) {
		void* o=AsCScreen(a)->tutorialIllustration; ((void(*)(void*,int))(((ScreenVtableView<0x78>*)o)->vtable)[3])(o,0);
	}
	// Phase 2: populate named overlays; registry entries retain the created resources.
	ScreenOverlayRegistry_CreateNamedOverlay(p,screenOverlayStrings + ScreenText_BallConditionKey,screenOverlayStrings + ScreenText_BallConditionResource,(void*)(0));
	ScreenOverlayRegistry_CreateNamedOverlay(p,screenOverlayStrings + ScreenText_PlayMode0Key,screenOverlayStrings + ScreenText_PlayMode0Resource,(void*)(0x80000000));
	ScreenOverlayRegistry_CreateNamedOverlay(p,screenOverlayStrings + ScreenText_PlayMode1Key,screenOverlayStrings + ScreenText_PlayMode1Resource,(void*)(0x80000000));
	ScreenOverlayRegistry_CreateNamedOverlay(p,screenOverlayStrings + ScreenText_PlayMode4Key,screenOverlayStrings + ScreenText_PlayMode4Resource,(void*)(0x80000000));
	ScreenOverlayRegistry_CreateNamedOverlay(p,screenOverlayStrings + ScreenText_SpinPointKey,screenOverlayStrings + ScreenText_SpinPointResource,(void*)(0));
	((void(*)())Option_GetInstance)();
	if(!((int(*)())Option_UseIntegratedClubOverlay)()) {
		ScreenOverlayRegistry_CreateNamedOverlay(p,screenOverlayStrings + ScreenText_WoodClubKey,screenOverlayStrings + ScreenText_WoodClubResource,(void*)(0));
		ScreenOverlayRegistry_CreateNamedOverlay(p,screenOverlayStrings + ScreenText_IronClubKey,screenOverlayStrings + ScreenText_IronClubResource,(void*)(0));
		ScreenOverlayRegistry_CreateNamedOverlay(p,screenOverlayStrings + ScreenText_PutterClubKey,screenOverlayStrings + ScreenText_PutterClubResource,(void*)(0));
	}
	ScreenOverlayRegistry_CreateNamedOverlay(p,screenOverlayStrings + ScreenText_TopCameraKey,screenOverlayStrings + ScreenText_TopCameraResource,(void*)(0));
	ScreenOverlayRegistry_CreateNamedOverlay(p,screenOverlayStrings + ScreenText_FreeCameraKey,screenOverlayStrings + ScreenText_FreeCameraResource,(void*)(0));
	ScreenOverlayRegistry_CreateNamedOverlay(p,screenOverlayStrings + ScreenText_TargetCameraKey,screenOverlayStrings + ScreenText_TargetCameraResource,(void*)(0));
	ScreenOverlayRegistry_CreateNamedOverlay(p,screenOverlayStrings + ScreenText_ClickIndicatorKey,screenOverlayStrings + ScreenText_ClickIndicatorResource,(void*)(0));
	void* commandResourcePool=__nw__FUl(0x50);
	if (commandResourcePool) {
	    try {
	        OverlayPlayer_Init(commandResourcePool,screenOverlayStrings + ScreenText_CommandKeysResource);
	    } catch (...) {
	        __dl__FPv(commandResourcePool);
	        throw;
	    }
	}
	AsCScreen(a)->statusTextRenderer=commandResourcePool;
	{
		S8017A418 t; void* _tr = ((void*(*)(S8017A418*,float,float,float,float))Vec4_Set)(&t, 0.0f, 0.0f, 20.0f, 20.0f);
		Register_NormalizedNamedRectangle(AsCScreen(a)->statusTextRenderer, screenOverlayStrings + ScreenText_DownSmallGlyph, _tr, 0);
	}
	{
		S8017A418 t; void* _tr = ((void*(*)(S8017A418*,float,float,float,float))Vec4_Set)(&t, 20.0f, 0.0f, 20.0f, 20.0f);
		Register_NormalizedNamedRectangle(AsCScreen(a)->statusTextRenderer, screenOverlayStrings + ScreenText_LeftSmallGlyph, _tr, 0);
	}
	{
		S8017A418 t; void* _tr = ((void*(*)(S8017A418*,float,float,float,float))Vec4_Set)(&t, 40.0f, 0.0f, 20.0f, 20.0f);
		Register_NormalizedNamedRectangle(AsCScreen(a)->statusTextRenderer, screenOverlayStrings + ScreenText_RightSmallGlyph, _tr, 0);
	}
	{
		S8017A418 t; void* _tr = ((void*(*)(S8017A418*,float,float,float,float))Vec4_Set)(&t, 0.0f, 20.0f, 20.0f, 20.0f);
		Register_NormalizedNamedRectangle(AsCScreen(a)->statusTextRenderer, screenOverlayStrings + ScreenText_UpSmallGlyph, _tr, 0);
	}
	{
		S8017A418 t; void* _tr = ((void*(*)(S8017A418*,float,float,float,float))Vec4_Set)(&t, 20.0f, 20.0f, 20.0f, 20.0f);
		Register_NormalizedNamedRectangle(AsCScreen(a)->statusTextRenderer, screenOverlayStrings + ScreenText_DownGlyph, _tr, 0);
	}
	{
		S8017A418 t; void* _tr = ((void*(*)(S8017A418*,float,float,float,float))Vec4_Set)(&t, 40.0f, 20.0f, 20.0f, 20.0f);
		Register_NormalizedNamedRectangle(AsCScreen(a)->statusTextRenderer, screenOverlayStrings + ScreenText_LeftGlyph, _tr, 0);
	}
	{
		S8017A418 t; void* _tr = ((void*(*)(S8017A418*,float,float,float,float))Vec4_Set)(&t, 0.0f, 40.0f, 20.0f, 20.0f);
		Register_NormalizedNamedRectangle(AsCScreen(a)->statusTextRenderer, screenOverlayStrings + ScreenText_RightGlyph, _tr, 0);
	}
	{
		S8017A418 t; void* _tr = ((void*(*)(S8017A418*,float,float,float,float))Vec4_Set)(&t, 20.0f, 40.0f, 20.0f, 20.0f);
		Register_NormalizedNamedRectangle(AsCScreen(a)->statusTextRenderer, screenOverlayStrings + ScreenText_UpGlyph, _tr, 0);
	}
	{
		S8017A418 t; void* _tr = ((void*(*)(S8017A418*,float,float,float,float))Vec4_Set)(&t, 40.0f, 40.0f, 20.0f, 20.0f);
		Register_NormalizedNamedRectangle(AsCScreen(a)->statusTextRenderer, screenOverlayStrings + ScreenText_InvalidGlyph, _tr, 0);
	}
	float f30=((float(*)(void*))Option_GetHudBaselineY)(((void*(*)())Option_GetInstance)());
	AsCScreen(a)->windIndicatorResource=ResourceManager_LoadByExtension(g_resourceManager,screenOverlayStrings + ScreenText_WindResource,0,1,0);
	((void(*)())Option_GetInstance)();
	if (((int(*)())Option_PlaceWindIndicatorOnLoad)()) {
		float f1=((float(*)(void*))WView_GetViewportWidth)(g_view);
		f1 = f1 - 50.0f;

		WPuppet_ReservedHook(AsCScreen(a)->windIndicatorResource, f1, ((f30 + 5.0f)), g_view);
	}
	void* r29b;
	r29b=__nw__FUl(0x1c); if (r29b) {
	    try {
	        CHoleInfoSet_Init(r29b);
	    } catch (...) {
	        __dl__FPv(r29b);
	        throw;
	    }
	} AsCScreen(a)->holeInfoSet=r29b;
	r29b=__nw__FUl(0x690); if (r29b) {
	    try {
	        ScreenBallPreview_Ctor(r29b);
	    } catch (...) {
	        __dl__FPv(r29b);
	        throw;
	    }
	} AsCScreen(a)->ballPreview=r29b;
	r29b=__nw__FUl(0x470); if (r29b) {
	    try {
	        CPangInfo_Ctor(r29b);
	    } catch (...) {
	        __dl__FPv(r29b);
	        throw;
	    }
	} AsCScreen(a)->pangInfo=r29b;
	r29b=__nw__FUl(0x28); if (r29b) {
	    try {
	        CIngameStatus_Ctor(r29b);
	    } catch (...) {
	        __dl__FPv(r29b);
	        throw;
	    }
	} AsCScreen(a)->ingameStatus=r29b;
	r29b=__nw__FUl(0x18); if (r29b) {
	    try {
	        CWindInfo_Ctor(r29b);
	    } catch (...) {
	        __dl__FPv(r29b);
	        throw;
	    }
	} AsCScreen(a)->windInfo=r29b;
	r29b=__nw__FUl(0x2c); if (r29b) {
	    try {
	        CAccuracyInfo_Ctor(r29b);
	    } catch (...) {
	        __dl__FPv(r29b);
	        throw;
	    }
	} AsCScreen(a)->accuracyInfo=r29b;
	r29b=__nw__FUl(0x24); if (r29b) {
	    try {
	        __ct__10CPowerInfoFv(r29b);
	    } catch (...) {
	        __dl__FPv(r29b);
	        throw;
	    }
	} AsCScreen(a)->powerInfo=r29b;
	r29b=__nw__FUl(0x88); if (r29b) {
	    try {
	        __ct__8CFlyInfoFv(r29b);
	    } catch (...) {
	        __dl__FPv(r29b);
	        throw;
	    }
	} AsCScreen(a)->flyInfo=r29b;
	if ((*(int*)&AsCSharedDoc(SharedDoc_GetCurrent())->partyGameMode)==3) {
		r29b=__nw__FUl(0x4c); if (r29b) {
		    try {
		        __ct__12CBalloonInfoFv(r29b);
		    } catch (...) {
		        __dl__FPv(r29b);
		        throw;
		    }
		} AsCScreen(a)->modeInfo=r29b;
	}
	if ((*(int*)&AsCSharedDoc(SharedDoc_GetCurrent())->partyGameMode)==2) {
		r29b=__nw__FUl(0x38); if (r29b) {
		    try {
		        CDriveInfo_Ctor(r29b);
		    } catch (...) {
		        __dl__FPv(r29b);
		        throw;
		    }
		} AsCScreen(a)->modeInfo=r29b;
	}
	if (AsCGolfDoc(GolfDoc_GetCurrent())->tutorialStage==TutorialStageCount) {
		r29b=__nw__FUl(0x24); if (r29b) {
		    try {
		        __ct__10CAuxMenuUIFv(r29b);
		    } catch (...) {
		        __dl__FPv(r29b);
		        throw;
		    }
		} AsCScreen(a)->auxiliaryMenu=r29b;
		r29b=__nw__FUl(0x18); if (r29b) {
		    try {
		        __ct__15CRemainTimeInfoFv(r29b);
		    } catch (...) {
		        __dl__FPv(r29b);
		        throw;
		    }
		} AsCScreen(a)->remainingTimeInfo=r29b;
		r29b=__nw__FUl(0x2c); if (r29b) {
		    try {
		        __ct__9CCalipersFv(r29b);
		    } catch (...) {
		        __dl__FPv(r29b);
		        throw;
		    }
		} AsCScreen(a)->calipers=r29b;
		r29b=__nw__FUl(0x28); if (r29b) {
		    try {
		        CHelpMsg_Ctor(r29b);
		    } catch (...) {
		        __dl__FPv(r29b);
		        throw;
		    }
		}
		// 0x8017A1BC/0x8017A1C8: this branch jumps to 0x8017A2D0 (bl ChatMessage_GetCurrent), past the
		// 0x19C-0x1C0 stores at 0x8017A298-0x8017A2CC; only the else branch makes them.
	} else {
		if ((unsigned char)SharedDoc_GetGolfRuleType()==2) { float f31v=24.0f;
			void* r29=__nw__FUl(0x50); if (r29) {
			    try {
			        OverlayPlayer_Init(r29,screenOverlayStrings + ScreenText_TutorialTitleResource);
			    } catch (...) {
			        __dl__FPv(r29);
			        throw;
			    }
			} AsCScreen(a)->modeLabelRenderer=r29;
			char* commandResourcePool=screenOverlayStrings;
			char* r28=((ScreenExtraState*)p)->tutorialTitleKeys[0];
			char* r27=r28;

			for (int r26=1; r26<=5; r26++) { S8017A418 t;
				char* s=(char*)String_FormatTemporary(commandResourcePool + ScreenText_TutorialTitleFormat, ((r26+1)));
				strcpy(r28, s);
				   void* _tr = ((void*(*)(S8017A418*,float,float,float,float))Vec4_Set)(&t, 0.0f, f31v*(float)(r26-1), 177.0f, f31v);
				Register_NormalizedNamedRectangle(AsCScreen(a)->modeLabelRenderer, r27, _tr, 0);
				r28 += 0x10;
				r27 += 0x10;
			}
		}
	AsCScreen(a)->tutorialFlashOn=0;
	AsCScreen(a)->tutorialFlashCount=2;
	AsCScreen(a)->tutorialFlashInterval=0.25f;
	AsCScreen(a)->tutorialFlashTimer=0.0f;
	AsCScreen(a)->tutorialCompletionPhase=3;
	AsCScreen(a)->tutorialCompletionInterval=0.25f;
	AsCScreen(a)->tutorialTitlePhase=2;
	AsCScreen(a)->tutorialTitleTimer=0.0f;
	AsCScreen(a)->modeLabelOpacity=0;
	}
	void* _t5c = ((void*(*)())ChatMessage_GetCurrent)();
	(*(void**)&AsCScreen(a)->chatMessageFontResource)=((void*(*)(void*))ChatMessage_GetDefaultFontResource)(_t5c);
	for (int r26=0; 6>r26; r26++) {
		if (r26 < AsCGolfDoc(GolfDoc_GetCurrent())->tutorialStage) ((ScreenExtraState*)(r26+a))->tutorialStageVisible[0]=1;
		else ((ScreenExtraState*)(r26+a))->tutorialStageVisible[0]=0;
	}
	if (AsCGolfDoc(GolfDoc_GetCurrent())->tutorialStage<TutorialStageCount && AsCScreen(a)->holeInfoSet) ((void*(*)(void*))HoleInfoSet_ShowCornerLabels)(AsCScreen(a)->holeInfoSet);
	// Phase 3: keep updating child loaders until all required resources are ready.
	while (1) {
		Layout_Update(AsCScreen(a)->layout);
		{
			void* o=AsCScreen(a)->strokePointDisplay; ((void(*)(void*))(((ScreenVtableView<0xe0>*)o)->vtable)[12])(o);
		}
		{
			void* o=AsCScreen(a)->mapCameraDisplay; ((void(*)(void*))(((ScreenVtableView<0x9c>*)o)->vtable)[5])(o);
		}
		{
			void* o=AsCScreen(a)->ballWindDisplay; ((void(*)(void*))(((ScreenVtableView<0xa4>*)o)->vtable)[5])(o);
		}
		{
			void* o=AsCScreen(a)->golfBaseDisplay; ((void(*)(void*))(((ScreenVtableView<0x164>*)o)->vtable)[5])(o);
		}
		{
			void* o=AsCScreen(a)->swingResultDisplay; ((void(*)(void*))(((ScreenVtableView<0x74>*)o)->vtable)[5])(o);
		}
		if (AsCScreen(a)->tutorialIllustration) {
			void* o=AsCScreen(a)->tutorialIllustration; ((void(*)(void*))(((ScreenVtableView<0x78>*)o)->vtable)[6])(o);
		}
// Require the base HUD resources and any party-mode resources to finish loading.
		if (CScreen_PollLoadComplete(p)) {
			if ((*(int*)&AsCSharedDoc(SharedDoc_GetCurrent())->partyGameMode)==3) {
				void* receiver696_fn_800A6218 = (void*)PartyItemSlots_GetInstance();
				PartyItemSlots_RefreshDisplay(receiver696_fn_800A6218);
			}
			break;
		}
	}
}

// Return the fixed option that suppresses the three separate club overlays.
int Option_UseIntegratedClubOverlay() {
	return 1;
}

// Assign all four components and return the supplied vector.
S8017A418* Vec4_Set(S8017A418* vector, float x, float y, float z, float w) {
	vector->x = x;
	vector->y = y;
	vector->z = z;
	vector->w = w;
	return vector;
}

// Read the option-controlled HUD baseline Y coordinate.
float Option_GetHudBaselineY(void* options) {
	return AsCOption(options)->hudBaselineY;
}

// Return the fixed option that disables initial wind placement.
int Option_PlaceWindIndicatorOnLoad() {
	return 0;
}

// Read the view width used to position HUD elements.
float WView_GetViewportWidth(void* view) {
	return ((WView*)view)->viewportWidth;
}

// Return the chat-message default font resource used by the HUD.
void* ChatMessage_GetDefaultFontResource(void* chat) {
	return ((ChatMessageView*)chat)->defaultFontResource;
}

// Forward corner-label display to the embedded hole-information object.
void* HoleInfoSet_ShowCornerLabels(void* holeInfo) {
	return CHoleInfo_ShowCornerLabels(((HoleInfoSetView*)holeInfo)->cornerLabels);
}

// Return the shared option object without transferring ownership.
void* Option_GetInstance() {
	return g_option;
}

// Return the current shared chat-message object without retaining it.
void* ChatMessage_GetCurrent() {
	return g_chatMsg;
}

// Reset HUD animation and panel state, then attach the current club resource effects.
extern "C" void* CScreen_OnInit(void* screenObject) {
	char* screenBytes = (char*)screenObject;
	((ScreenExtraState*)screenBytes)->panelOpenRequested=0;
	((ScreenExtraState*)screenBytes)->itemPanelOpenRequested=0;
	((ScreenExtraState*)screenBytes)->itemPanelReady=0;
	AsCScreen(screenBytes)->shotOptionEnabled=0;
	((ScreenExtraState*)screenBytes)->shotOptionChanged=0;
	((ScreenExtraState*)screenBytes)->resumeAimRequested=0;
	((ScreenExtraState*)screenBytes)->hudInputBlocked=0;
	((ScreenExtraState*)screenBytes)->tutorialMeterEnabled=0;
	(*(int*)&AsCScreen(screenBytes)->tutorialGaugeEnabled)=0;
	((ScreenExtraState*)screenBytes)->tutorialAimEnabled=0;
	((ScreenExtraState*)screenBytes)->aimControlsEnabled=0;
	((ScreenExtraState*)screenBytes)->dropModeActive=0;
	(*(int*)&AsCScreen(screenBytes)->tutorialClubChangeEnabled)=1;
	((ScreenExtraState*)screenBytes)->tutorialAimControlEnabled=1;
	((ScreenExtraState*)screenBytes)->tutorialSpinControlEnabled=1;
	((ScreenExtraState*)screenBytes)->tutorialIllustrationActive=0;
	((ScreenExtraState*)screenBytes)->shotTransitionRequested=0;
	(*(int*)&AsCScreen(screenBytes)->swingFeedbackPending)=0;
	((ScreenExtraState*)screenBytes)->cameraControlsEnabled=1;
	((ScreenExtraState*)screenBytes)->meterFadeFrame=0;
	((ScreenExtraState*)screenBytes)->impactFeedbackStarted=0;
	((ScreenExtraState*)screenBytes)->partyMeterIndex=-1;
	((ScreenExtraState*)screenBytes)->pululuAnimationPhase=0;
	((ScreenExtraState*)screenBytes)->tutorialAimBlinkFrame=0;
	((ScreenExtraState*)screenBytes)->tutorialAimBlinkCount=0;
	((ScreenExtraState*)screenBytes)->tutorialMeterBlinkFrame=0;
	((ScreenExtraState*)screenBytes)->tutorialMeterBlinkCount=0;
	((ScreenExtraState*)screenBytes)->tutorialGaugeBlinkFrame=0;
	((ScreenExtraState*)screenBytes)->tutorialGaugeBlinkCount=0;
	((ScreenExtraState*)screenBytes)->meterOpacity=255;
	(*(int*)&AsCScreen(screenBytes)->hudDisplayMask)=0;
	((ScreenExtraState*)screenBytes)->requestedPanel=0;
	((ScreenExtraState*)screenBytes)->previousPanel=0;
	((ScreenExtraState*)screenBytes)->savedPanel=0;
	(*(int*)&AsCScreen(screenBytes)->hudPhase)=HudInactive;
	((ScreenExtraState*)screenBytes)->previousHudPhase=0;
	((ScreenExtraState*)screenBytes)->shotOptionResult=0;
	((ScreenExtraState*)screenBytes)->shotDirectionResult=0;
	AsCScreen(screenBytes)->clubChangeDisplayState=0;
	((ScreenExtraState*)screenBytes)->cameraCaptionOpacity=0;
	((ScreenExtraState*)screenBytes)->terrainSampleCount=10;
	((ScreenExtraState*)screenBytes)->terrainMarkerIndex=0;
	((ScreenExtraState*)screenBytes)->shotIndicatorPulse=0;
	((ScreenExtraState*)screenBytes)->previousTutorialStage=-1;
	((ScreenExtraState*)screenBytes)->tutorialPointerDelayed=0;
	(*(int*)&AsCScreen(screenBytes)->previousGameStateFlags)=0;
	((ScreenExtraState*)screenBytes)->ballRingHeight=0;
	(*(float*)&AsCScreen(screenBytes)->displayTimeRemaining)=0;
	((ScreenExtraState*)screenBytes)->terrainProfileScale=0;
	((ScreenExtraState*)screenBytes)->initialDisplayEnabled=1;
	((ScreenExtraState*)screenBytes)->clubChangeAngle=6.2831855f;
	((ScreenExtraState*)screenBytes)->clubChangeAnimationDirection=0;
	((ScreenExtraState*)screenBytes)->clubChangeAnimationTime=0;
	((ScreenExtraState*)screenBytes)->feedbackColor0=0;
	((ScreenExtraState*)screenBytes)->feedbackColor1=0;
	((ScreenExtraState*)screenBytes)->shotFeedbackBlend=0;
	AsCScreen(screenBytes)->hudFlags |= 2;
	// 0x8017A588-0x8017A594: clubKind = *(u8*)(*(InstanceTable_Get()+4) + 0xA) (two loads). 0x8017A598-0x8017A5B0:
	// ClubModel_FindCachedForItem(r3 = InstanceTable_Get(), r4 = g_sharedDoc[playerIndex*0x1D4 + 0x1CC]). Ours read +4 directly and
	// passed the player entry as the self argument (r3) with garbage r4.
	unsigned char clubKind = ((ClubSelectionState*)InstanceTable_Get())->selection->clubKind;
	void* sharedDoc=g_sharedDoc;
	void* clubTable=InstanceTable_Get();
	unsigned char playerIndex=AsCGolfDoc(AsCSharedDoc(sharedDoc)->golfDoc)->currentPlayer;
	void* clubResources=ClubModel_FindCachedForItem(clubTable,*(int*)((char*)sharedDoc+(unsigned int)playerIndex*0x1d4+0x1cc));
	void* clubAsset=((ClubResourceSlots*)clubResources)->byClubKind[(unsigned int)clubKind];
	void* fxInterpreter=FxInterpreter_GetInstance();
	void* fxKey=FxAttachments_CreateFromResourceTags(fxInterpreter,clubAsset,0);
	(*(void**)&AsCScreen(screenBytes)->fxInterpreterKey)=fxKey;
	return fxKey; // 0x8017A5D0: no mr r3 after the store; r3 stays FxAttachments_CreateFromResourceTags's result
}

} // extern "C"

template <int N> struct ScreenUI {
    char data[N];
    virtual void slot08();
    virtual void slot0C();
    virtual void slot10();
    virtual void slot14();
    virtual void slot18();
    virtual void slot1C();
    virtual void slot20();
    virtual void slot24();
    virtual void slot28();
    virtual void slot2C();
    virtual void slot30();
};
// Process-only view preserves the exact scalar types used by the update path.
struct HudProcessState {
    char padding000[0x194];
    bool flashes[8];
    unsigned char tutorialFlashCount,tutorialFlashOn;
    char padding19E[2];
    float tutorialFlashInterval,tutorialFlashTimer;
    unsigned char tutorialCompletionPhase;
    char padding1A9[3];
    float tutorialCompletionInterval,tutorialCompletionTimer;
    int previousTutorialStage; // +0x1b4: initialized to -1; message handlers store integer stage ids.
    float cameraCaptionOpacity;
    int tutorialTitlePhase;
    unsigned char modeLabelOpacity;
    char padding1C1[3];
    float tutorialTitleX,tutorialTitleY,tutorialTitleTimer,tutorialPointerDelay;
    unsigned char tutorialPointerDelayed;
    char padding1D5[3];
    int previousGameStateFlags;
    float pangyaFeedbackRed,pangyaFeedbackGreen,pangyaFeedbackBlue,clubChangeAngle;
    int clubChangeAnimationDirection;
    char padding1F0[8];
    int fxInterpreterKey;
    char padding1FC[0x60];
    int hudInitialized,panelOpenRequested,itemPanelOpenRequested,itemPanelReady,shotOptionEnabled;
    int shotOptionChanged,resumeAimRequested,hudInputBlocked,panelsReady;
    char padding280[8];
    int tutorialAimEnabled;
    char padding28C[4];
    int tutorialAimControlEnabled;
    char padding294[8];
    int tutorialIllustrationActive,cameraControlsEnabled,aimControlsEnabled,dropModeActive,shotTransitionRequested;
    char padding2B0[0x3C];
    int tutorialAimBlinkCount;
    char padding2F0[0x18];
    int requestedPanel,savedPanel,previousPanel,hudPhase;
    int previousHudPhase,shotOptionResult,shotDirectionResult;
    char padding324[0x14];
    void* ballWindDisplay;
    char padding33C[0x18];
    int hudEnabled;
};
typedef char HudPreviousTutorialStageOffsetCheck[
    ((int)&((HudProcessState*)0)->previousTutorialStage == (int)&((ScreenExtraState*)0)->previousTutorialStage) ? 1 : -1];

// Borrowed field views used only by the HUD coordinator; offsets preserve Wii storage.
struct HudPlayerFields {
    char padding00[8]; int status08;
    char padding0C[0x90]; unsigned char itemMenuAvailable;
    char padding9D[3]; unsigned int equippedItemId;
    char paddingA4[4]; float gaugeValue;
    char paddingAC[0x34]; int impactRangeAdjustment;
};
struct HudPartyEffectsView { char padding00[0x10]; PartyItemModifiers modifiers; };
struct HudPlayerDocumentSlot {
    char padding000[0x1cc]; int clubItemId;
    char padding1D0[0xc]; int activityWord;
};
struct HudHoleRecordView { char padding00[0xc]; WVector position; char padding18[8]; };
struct HudHeadingReferenceView { char padding000[0x33c]; WVector headingReference; };
struct HudHoleInfoChildren { void* cornerGroup; char padding04[8]; void* accuracyInfo; };
struct HudCornerGroupChildren { char padding00[0x34]; void* distanceToHole; };
enum HudPointerRegion { HudRegionClubMenu = 5, HudRegionPlayerFirst = 10,
    HudRegionPlayerLast = 13, HudRegionTutorialAction = 31,
    HudRegionDelayedTutorialControl = 32, HudRegionOneShotClose = 35 };
enum HudPointerButtonState { HudPrimaryPointerButton = 0, HudPointerPressed = 1, HudPointerReleased = 2 };
enum HudCoordinatorConstants { kSharedPlayerRecordStride = 0x1d4, kPartyItemMode = 3,
    kTimebombPartyItem = 10, kTimebombTickStateMask = 0x08000008 };

// Read one player's document words; the view is offset-biased and never owns storage.
static inline HudPlayerDocumentSlot* HudDocumentPlayer(void* doc, int playerIndex) {
    return (HudPlayerDocumentSlot*)((char*)doc + playerIndex*kSharedPlayerRecordStride);
}

// Wii layouts, including the vtable pointers embedded in each UI object.
// Vtable pointers are embedded at these offsets within separate display objects.
enum ScreenDisplayVptrOffset {
    kTutorialIllustrationVptr = 0x78,
    kMapCameraDisplayVptr = 0x9C,
    kBallWindDisplayVptr = 0xA4,
    kStrokePointDisplayVptr = 0xE0,
    kGolfBaseDisplayVptr = 0x164
};
// Compatibility declarations used by the later draw path.
#define CALL_SLOTS(args) \
    virtual int slot0x08 args; virtual int slot0xC args; \
    virtual int slot0x10 args; virtual int slot0x14 args; \
    virtual int slot0x18 args; virtual int slot0x1C args; \
    virtual int slot0x20 args; virtual int slot0x24 args; \
    virtual int slot0x28 args; virtual int slot0x2C args; \
    virtual int slot0x30 args; virtual int slot0x34 args; \
    virtual int slot0x38 args; virtual int slot0x3C args; \
    virtual int slot0x40 args; virtual int slot0x44 args; \
    virtual int slot0x48 args; virtual int slot0x4C args; \
    virtual int slot0x50 args; virtual int slot0x54 args; \
    virtual int slot0x58 args; virtual int slot0x5C args; \
    virtual int slot0x60 args; virtual int slot0x64 args; \
    virtual int slot0x68 args; virtual int slot0x6C args; \
    virtual int slot0x70 args;
template<int N> struct ScreenCall0 { char data[N]; CALL_SLOTS(()) };
template<int N> struct ScreenCallI { char data[N]; CALL_SLOTS((int)) };
template<int N> struct ScreenCallF { char data[N]; CALL_SLOTS((float)) };
template<> struct ScreenCallF<0> { CALL_SLOTS((float)) };
template<int N> struct ScreenCallFF { char data[N]; CALL_SLOTS((float,float)) };
template<int N> struct ScreenCallPP { char data[N]; CALL_SLOTS((void*,void*)) };
template<int N> struct ScreenCallP { char data[N]; CALL_SLOTS((void*)) };
#undef CALL_SLOTS
#define GAME (*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc)
#define PLAYER AsCGolfDoc(GAME)->currentPlayer
#define STAGE AsCGolfDoc(GAME)->tutorialStage

extern "C" {
void CDrawDriveLine_UpdateShotPrediction(void*);

void InputManager_Init(void*);
void InputManager_Dtor();
void InputManager_RequestPointerInput(void*,int);
int Input_IsActionPressed(void*,int,int);
extern "C" int SharedDoc_IsPlayerFlag0Set(int);
void GolfTipsDialog_RequestClose(int,float);
void CDistToHoleInfo_BeginMoveToPin(void*);
void CMouseCursor_SetPointerInputEnabled(void*,int);
void CMouseCursor_SetRadialMenuEnabled(void*,int);
void Sound_PlayByName(void*,const char*,int,int,int,int,float,float,float);
extern "C" void IActor_DispatchMessage(void*, void*, const char*, int, void*, void*, void*, void*);
void* GolfPlayer_GetEntryByIndex(void*,int);
unsigned int ItemInventory_GetItemIdAt(void*,int);
void CCalipers_ResetMeasurement(void*);
int CCalipers_IsPreviewActive(void*);
int GolfState_AllowsStatusTransition(void*);
void CIngameStatus_BeginHide(void*);
void* GolfBall__Fv();
void NoticeQueue_Clear(void*);
void NoticeQueue_UpdateTimers(void*,float);
void HeadIcon_UpdatePlayerTimers(void*,float);
void Vec2_Copy(void*,void*);
unsigned long long Vec2_ScaleByFloat(void*,float);
unsigned long long Vec2_Add(void*,void*);
extern "C" int PartyItemSlots_GetItemType(void*, int);
void* CWind_GetInstance();
void WindIndicator_UpdateDirectionAndPhase(void*,float);
void ScreenBallPreview_Update(void*,float);

void CAccuracyInfo_ShowShotFlagFeedback(void*,int,int);
void CHoleInfoSet_Update(void*,float);
void CIngameStatus_Update(void*,float);
void __mi__FRC7WVectorRC7WVector(void*,void*,void*);
float Vector_SignedAngleXZ(void*,void*);
int Pointer_GetButtonState(void*,int);
int GolfTask_NoOpStatus();
extern "C" float ShotParameters_GetGoodImpactHalfWidth(void*);
float Random_RangeFloat(int,float,float);
extern void  *g_soundManager;
extern void *g_headIcon,*g_inputPointer;
extern signed char s_inputManagerInitialized;
extern char s_gameInputManager[],s_gameInputManagerDestructorRegistration_Screen[];
extern unsigned int jumptable_804AEBA8[];
extern const float kFloat_0_5_AuxMenu;
extern const float kScreenHudSoundRange; // 200.0f: HUD sound range
extern const float kFloat_1_AuxMenu;
extern const float kScreenTutorialTitleInsetX; // 30.0f: tutorial title horizontal inset
extern const float kFloat_0_3_AuxMenu;
extern const float kScreenMarkerInsetX; // 14.0f: marker/title horizontal inset
extern const float kFloat_0_05_Screen; // 0.05f: ground-lie adjustment and viewport height fraction
extern const float kScreenTutorialTitleDestinationInsetX; // 250.0f: tutorial title destination inset
extern const float kFloat_255_Screen;
extern const float kFloat_100_Screen; // 100.0f: shot indicator animation speed / update text position
extern const float kScreenShotFeedbackBlendRate; // 7.0f: shot feedback blend rate
extern const float kScreenClubChangeAngularSpeed; // 21.9911499f: club-change angular animation speed
extern const float kFloat_2_805DED94;
extern const float kScreenClubChangeAngleUpperLimit; // 6.80678415f: club-change angle upper limit
extern const float kScreenCameraCaptionFadeStart; // -255.0f: camera-caption fade start
extern const float kScreenCameraCaptionFadeRate; // 512.0f: camera-caption fade rate
extern const float kScreenTerrainIndicatorAngleLimit; // 0.69813168f: terrain indicator angle limit (40 degrees)
extern const float kFloat_128_ShotGauge;
extern const float kPangyaTargetCenterX;
extern const float kFloat_10_Screen; // 10.0f: impact range and marker coordinate scale
extern const float kScreenGroundLieRangeScale1_5; // 1.5f: ground-lie impact-range multiplier
extern const float kScreenGroundLieRangeScale1_3; // 1.3f: ground-lie impact-range multiplier
extern const float kFloat_1_2_ScreenOverlay;
extern const float kFloat_1_1_ScreenRange;
extern const float kScreenImpactRangeNormalization; // 460.0f: impact-range normalization numerator
extern const float kScreenRadiansToDegrees; // 57.29578f: radians-to-degrees scale
extern const float kScreenImpactHeadingDeadZone; // 2.5f: impact heading dead zone
extern const float kFloat_neg1_Screen;
extern const float kScreenColorEqualityEpsilon; // 0.00001f: color equality tolerance
extern const float kScreenImpactFeedbackMinimumIntensity; // 225.0f: minimum randomized impact-feedback color
extern float g_screenFullFeedbackIntensity; // 255.0f: full-intensity feedback color comparison

// Keep singleton construction inline: its local-static symbol and initialization order are part of the ABI.
#pragma push
#pragma always_inline on
static inline void EnsureHudInput() {
    if (!Sing_805DC5C8()) {
        if (!s_inputManagerInitialized) {
            InputManager_Init(s_gameInputManager);
            __register_global_object(s_gameInputManager,(void*)InputManager_Dtor,s_gameInputManagerDestructorRegistration_Screen);
            s_inputManagerInitialized=1;
        }
        Sing_805DC5C8()=s_gameInputManager;
    }
}
// Test a logical HUD action through the initialized shared input manager.
static inline int IsHudActionPressed(int action) {
    return Input_IsActionPressed(Sing_805DC5C8(),action,-1);
}
// Play a HUD sound selected by a byte offset in the shared string pool.
static inline void PlayHudSound(int stringOffset) {
    Sound_PlayByName(g_soundManager,screenOverlayStrings+stringOffset,0,0,0,0,kFloat_0_5_AuxMenu,kScreenHudSoundRange,kFloat_1_AuxMenu);
}
// Send two raw payload words synchronously to the named HUD actor.
static inline void DispatchHudMessage(void* sender,int actorNameOffset,int messageId,unsigned int payload0,unsigned int payload1) {
    IActor_DispatchMessage(sender,sender,screenOverlayStrings+actorNameOffset,messageId,(void*)payload0,(void*)payload1,0,0);
}
#pragma pop

// Ensure input initialization, then test the requested logical action.
static inline int ScreenInputPress(int button) {
    EnsureHudInput();
    return IsHudActionPressed(button);
}

// Borrowed display interfaces: named operations keep the original virtual dispatch.
// Setter/getter meanings are checked in ball_wind_2d_hud.cpp, golf_base_2d_hud_layout.cpp, stroke_point_2d_selector.cpp and stroke_point_2d_dialog.cpp.
// Map camera uses CMapCamera2d (map_camera_2d_panel.cpp): slot +0x14 updates layout/visibility/
// input/elements, +0x24 latches its first action with click feedback, and +0x4C
// starts closing (seven updates, full initial opacity). No interface owns its object.
struct HudBallWindInterface {
    char prefix[kBallWindDisplayVptr];
    virtual int reserved8();
    virtual int reservedC();
    virtual int reserved10();
    virtual int UpdateFrame();
    virtual int reserved18();
    virtual int reserved1C();
    virtual int SetSelectionActive(int arg0);
    virtual int SetSelectionState(int arg0);
    virtual int IsBallSelectorActivated();
    virtual int reserved2C();
    virtual int reserved30();
    virtual int reserved34();
    virtual int ShowElements26And27(int arg0);
    virtual int ShowElements22And25(int arg0);
    virtual int ShowElement22(int arg0);
    virtual int SetElements25And26Bright(int arg0);
    virtual int ShowElement28(int arg0);
    virtual int ShowElement29(int arg0);
    virtual int reserved50();
    virtual int reserved54();
    virtual int reserved58();
    virtual int reserved5C();
    virtual int reserved60();
    virtual int reserved64();
    virtual int reserved68();
    virtual int reserved6C();
    virtual int reserved70();
};
struct HudGolfBaseInterface {
    char prefix[kGolfBaseDisplayVptr];
    virtual int reserved8();
    virtual int reservedC();
    virtual int reserved10();
    virtual int UpdateFrame();
    virtual int reserved18();
    virtual int ShowExtraMeterImage(int arg0);
    virtual int reserved20();
    virtual int reserved24();
    virtual int reserved28();
    virtual int reserved2C();
    virtual int reserved30();
    virtual int EnableConfirmButton(int arg0);
    virtual int EnableActionButtons(int arg0);
    virtual int reserved3C();
    virtual int IsConfirmActivated();
    virtual int IsCancelActivated();
    virtual int LatchConfirmAction();
    virtual int reserved4C();
    virtual int reserved50();
    virtual int reserved54();
    virtual int reserved58();
    virtual int reserved5C();
    virtual int SetPangyaTargetX(float arg0);
    virtual int SetPangyaZoneWidth(float arg0);
    virtual int reserved68();
    virtual int reserved6C();
    virtual int BeginFadeOut();
};
struct HudStrokePointInterface {
    char prefix[kStrokePointDisplayVptr];
    virtual int reserved8();
    virtual int reservedC();
    virtual int reserved10();
    virtual int reserved14();
    virtual int reserved18();
    virtual int EnablePowerMode(int arg0);
    virtual int EnableDirectionButtons(int arg0);
    virtual int SetGaugeValue(int arg0);
    virtual int EnableExtendedSpecialShots(int arg0);
    virtual int reserved2C();
    virtual int UpdateFrame();
    virtual int reserved34();
    virtual int SetDirectionThresholds(float arg0, float arg1);
    virtual int reserved3C();
    virtual int IsConfirmationRequested();
    virtual int GetDirectionSelection();
    virtual int GetSpecialShotSelection();
    virtual int GetPowerMode();
    virtual int ClearDirectionSelection();
    virtual int ClearSpecialShotSelection();
    virtual int ClearPowerMode();
    virtual int RestoreSavedSelection();
    virtual int GetCommittedPoint(void* arg0, void* arg1);
    virtual int reserved64();
    virtual int BeginClosing();
    virtual int reserved6C();
    virtual int reserved70();
};
struct HudMapCameraInterface {
    char prefix[kMapCameraDisplayVptr];
    virtual int reserved8();
    virtual int reservedC();
    virtual int reserved10();
    virtual int UpdateFrame();
    virtual int reserved18();
    virtual int reserved1C();
    virtual int reserved20();
    virtual int LatchFirstActionAndPlayClick();
    virtual int reserved28();
    virtual int reserved2C();
    virtual int reserved30();
    virtual int reserved34();
    virtual int reserved38();
    virtual int reserved3C();
    virtual int reserved40();
    virtual int reserved44();
    virtual int reserved48();
    virtual int BeginClosing();
    virtual int reserved50();
    virtual int reserved54();
    virtual int reserved58();
    virtual int reserved5C();
    virtual int reserved60();
    virtual int reserved64();
    virtual int reserved68();
    virtual int reserved6C();
    virtual int reserved70();
};
// CSwingResult2d's vtable is embedded at +0x74; slot 0x14 advances layout,
// transition/fade counters and result-element visibility (swing_result_hud_and_equipped_stats.cpp).
struct HudSwingResultFrameInterface {
    char prefix[0x74];
    virtual void reserved08();
    virtual void reserved0C();
    virtual void reserved10();
    virtual void UpdateLayoutAndFeedback();
};

struct HudTutorialIllustrationInterface {
    char prefix[kTutorialIllustrationVptr];
    virtual int reserved8();
    virtual int reservedC();
    virtual int reserved10();
    virtual int reserved14();
    virtual int ProcessFrameSlot18();
    virtual int reserved1C();
    virtual int reserved20();
    virtual int reserved24();
    virtual int OnCameraAction();
    virtual int reserved2C();
    virtual int reserved30();
    virtual int reserved34();
    virtual int reserved38();
    virtual int reserved3C();
    virtual int reserved40();
    virtual int reserved44();
    virtual int reserved48();
    virtual int reserved4C();
    virtual int reserved50();
    virtual int reserved54();
    virtual int reserved58();
    virtual int reserved5C();
    virtual int reserved60();
    virtual int reserved64();
    virtual int reserved68();
    virtual int reserved6C();
    virtual int reserved70();
};
struct HudTimedChildInterface {
    virtual int reserved8();
    virtual int UpdateSeconds(float arg0);
    virtual int reserved10();
    virtual int reserved14();
    virtual int reserved18();
    virtual int reserved1C();
    virtual int reserved20();
    virtual int reserved24();
    virtual int reserved28();
    virtual int reserved2C();
    virtual int reserved30();
    virtual int reserved34();
    virtual int reserved38();
    virtual int reserved3C();
    virtual int reserved40();
    virtual int reserved44();
    virtual int reserved48();
    virtual int reserved4C();
    virtual int reserved50();
    virtual int reserved54();
    virtual int reserved58();
    virtual int reserved5C();
    virtual int reserved60();
    virtual int reserved64();
    virtual int reserved68();
    virtual int reserved6C();
    virtual int reserved70();
};

#pragma push
#pragma always_inline on
// Set ball/wind selector, item and camera controls from aim, tutorial and drop state.
static inline void UpdateHudBallWindControls(HudProcessState* self) {
    if (TutorialStageCount>STAGE || ((ClubSelectionState*)InstanceTable_Get())->selection->clubKind==3 || (self->hudPhase)!=HudAim) {
        ((HudBallWindInterface*)self->ballWindDisplay)->SetSelectionActive(0);
        if ((self->hudPhase)==HudAim) ((HudBallWindInterface*)self->ballWindDisplay)->SetSelectionState(0);
    } else {
        ((HudBallWindInterface*)self->ballWindDisplay)->SetSelectionActive(1);
        if ((self->shotDirectionResult) || (self->shotOptionResult)) ((HudBallWindInterface*)self->ballWindDisplay)->SetSelectionState(2);
        else ((HudBallWindInterface*)self->ballWindDisplay)->SetSelectionState(1);
    }
    if ((self->hudPhase)!=HudAim || (self->dropModeActive)) {
        ((HudBallWindInterface*)self->ballWindDisplay)->ShowElements22And25(0);
        ((HudBallWindInterface*)self->ballWindDisplay)->ShowElements26And27(0);
    } else if (6==STAGE) {
        if (((ClubSelectionState*)InstanceTable_Get())->selection->clubKind==3 && ((CGolfBall*)GolfBall__Fv())->m_groundType==2) {
            ((HudBallWindInterface*)self->ballWindDisplay)->ShowElements22And25(1);
            ((HudBallWindInterface*)self->ballWindDisplay)->ShowElements26And27(0);
        } else {
            ((HudBallWindInterface*)self->ballWindDisplay)->ShowElements26And27(1);
            ((HudBallWindInterface*)self->ballWindDisplay)->ShowElements22And25(0);
        }
    } else if (4<=STAGE) {
        if (((ClubSelectionState*)InstanceTable_Get())->selection->clubKind==3 && ((CGolfBall*)GolfBall__Fv())->m_groundType==2) ((HudBallWindInterface*)self->ballWindDisplay)->ShowElement22(1);
        else ((HudBallWindInterface*)self->ballWindDisplay)->ShowElement22(0);
    }
    if ((self->hudPhase)!=HudAim || (self->dropModeActive) || !(self->cameraControlsEnabled)) {
        ((HudBallWindInterface*)self->ballWindDisplay)->ShowElement28(0);
        ((HudBallWindInterface*)self->ballWindDisplay)->ShowElement29(0);
    } else if (AsCGolfDoc(GAME)->cameraMode==0 || AsCGolfDoc(GAME)->cameraMode==4) ((HudBallWindInterface*)self->ballWindDisplay)->ShowElement28(1);
    else ((HudBallWindInterface*)self->ballWindDisplay)->ShowElement29(1);
    if (((HudPlayerFields*)GolfPlayer_GetEntryByIndex(GAME,PLAYER))->itemMenuAvailable && !(int)((HudPlayerFields*)GolfPlayer_GetEntryByIndex(GAME,PLAYER))->equippedItemId && !(self->dropModeActive)) ((HudBallWindInterface*)self->ballWindDisplay)->SetElements25And26Bright(1);
    else ((HudBallWindInterface*)self->ballWindDisplay)->SetElements25And26Bright(0);
}


// Advance the club-change angle in radians; dt is seconds and the idle timer uses its own rate.
static inline void UpdateHudClubChangeAngle(HudProcessState* self,float dt) {
    if (((ClubSelectionState*)InstanceTable_Get())->changeFlags&1) {
        float zero=kFloat_0_AuxMenu;
        self->clubChangeAngle=zero;
        (self->clubChangeAnimationDirection)=1;
        ((ScreenExtraState*)self)->clubChangeAnimationTime=zero;
    } else {
        switch ((self->clubChangeAnimationDirection)) {
        case -1: {
            float t=self->clubChangeAngle-kScreenClubChangeAngularSpeed*dt; self->clubChangeAngle=t;
            if (t<kFloat_6_2831855_Screen) {
                self->clubChangeAngle=kFloat_6_2831855_Screen;
                (self->clubChangeAnimationDirection)=0;
            }
            break;
        }
        case 0: ((ScreenExtraState*)self)->clubChangeAnimationTime=kFloat_2_805DED94*dt+((ScreenExtraState*)self)->clubChangeAnimationTime; break;
        case 1: { float t=kScreenClubChangeAngularSpeed*dt+self->clubChangeAngle;
            self->clubChangeAngle=t;
            if (t>kScreenClubChangeAngleUpperLimit) {
                self->clubChangeAngle=kScreenClubChangeAngleUpperLimit;
                (self->clubChangeAnimationDirection)=-1;
            }
            break;
        }
        }
    }
}


// Advance completion and flash timers without changing the separate title-motion clock.
// flashDirection/flashCount are caller scratch only; references preserve register lifetimes.
// The caller retains ownership of the screen and passes dt in seconds.
static inline void UpdateHudTutorialFlashClocks(HudProcessState* self,float dt,unsigned int& flashDirection,unsigned char& flashCount) {
    // Completion phases below 3 blink; the separate title phases stay in OnProcess.
    if (self->tutorialCompletionPhase<3) {
        float t=(self->tutorialCompletionTimer+dt);
        self->tutorialCompletionTimer=t;
        if (t>self->tutorialCompletionInterval) {
            self->tutorialCompletionTimer=t-self->tutorialCompletionInterval;
            self->flashes[STAGE]=((0 == ((self->flashes[STAGE]))));
            if (self->flashes[STAGE]) ++self->tutorialCompletionPhase;
        }
    }
    flashCount=self->tutorialFlashCount;
    if (2>flashCount) {
        float t=((self->tutorialFlashTimer+dt));
        self->tutorialFlashTimer=t;
        if (t>self->tutorialFlashInterval) {
            flashDirection=1-self->tutorialFlashOn;
            self->tutorialFlashTimer=t-self->tutorialFlashInterval;
            self->tutorialFlashOn=flashDirection;
            if ((unsigned char)flashDirection) self->tutorialFlashCount=flashCount+1;
        }
    }
}

#pragma pop

#pragma push
#pragma always_inline on
// Advance loaded child presentations in dependency order; no input or panel requests here.
static inline void UpdateLoadedHudChildren(HudProcessState* self) {
    Layout_Update(AsCScreen(self)->layout);
    ((HudStrokePointInterface*)AsCScreen(self)->strokePointDisplay)->UpdateFrame();
    ((HudMapCameraInterface*)AsCScreen(self)->mapCameraDisplay)->UpdateFrame();
    CScreen_UpdateShotMeterPresentation((UpdateData*)self);
    ((HudBallWindInterface*)self->ballWindDisplay)->UpdateFrame();
    ((HudGolfBaseInterface*)AsCScreen(self)->golfBaseDisplay)->UpdateFrame();
    ((HudSwingResultFrameInterface*)AsCScreen(self)->swingResultDisplay)->UpdateLayoutAndFeedback();
    // CTutorialIllust2d slot 0x18 updates layout, page controls, and both message
    // reveals when present; its aggregate ready result is ignored by this coordinator.
    if (AsCScreen(self)->tutorialIllustration) ((HudTutorialIllustrationInterface*)AsCScreen(self)->tutorialIllustration)->ProcessFrameSlot18();
}

#pragma pop

#pragma push
#pragma always_inline on
// Request the aim panel only when input, panel history, HUD phase and tutorial gates agree.
// Gate table (PLAYER/STAGE remain live singleton reads; preserve short-circuit order):
// Outer gate: input allowed && no panel && previous panel unchanged && HudAim && no resume.
// Outer | Tutorial gate                                | Result             | History
// pass  | STAGE == TutorialStageCount                   | request/open aim   | unchanged
// pass  | STAGE < count && aim enabled && blinks > limit| request/open aim   | unchanged
// pass  | any other tutorial state                     | wait               | unchanged
// fail  | not evaluated                                | no new aim request | snapshot panel/phase
// Closing is a later independent gate, handled by FinishHudAimPanelClosing.
static inline void RequestHudAimPanelWhenReady(HudProcessState* self) {
    if (!SharedDoc_IsPlayerFlag0Set(PLAYER) && (self->requestedPanel)==0 && (self->previousPanel)==(self->requestedPanel) && (self->hudPhase)==HudAim && !(self->resumeAimRequested)) {
        if (STAGE==TutorialStageCount || ((self->tutorialAimEnabled) && TutorialStageCount>STAGE && (self->tutorialAimBlinkCount)>TutorialBlinkLimit)) {
            (self->requestedPanel)=HudPanelAim;
            (self->panelOpenRequested)=1;
        }
    } else {
        (self->previousPanel)=(self->requestedPanel);
        (self->previousHudPhase)=(self->hudPhase);
    }
}

#pragma pop

#pragma push
#pragma always_inline on
// Consume the aim-close request, close tips and start the distance-to-pin overlay motion.
static inline void FinishHudAimPanelClosing(HudProcessState* self) {
    if ((self->requestedPanel)&HudPanelAimClosing) {
        (self->requestedPanel)=0;
        GolfTipsDialog_RequestClose(-2,(kFloat_0_5_AuxMenu));
        void* holeInfo=AsCScreen(self)->holeInfoSet;
        if (holeInfo) {
            void* cornerGroup=((HudHoleInfoChildren*)holeInfo)->cornerGroup;
            if (cornerGroup && ((HudCornerGroupChildren*)cornerGroup)->distanceToHole)
                CDistToHoleInfo_BeginMoveToPin(((HudCornerGroupChildren*)cornerGroup)->distanceToHole);
        }
    }
}

#pragma pop

#pragma push
#pragma always_inline on
// Handle aim confirmation and requests to open spin/items; scratch preserves the original ABI lifetimes.
// Actor queries write synchronously into caller-owned scratch; no destination is retained.
// Camera reply 0/1 permits aim input; other values defer it. Item replies use
// CItemWindow states: open=0, opening=1, closing=2, closed=3. Spin query 9 is RANGE.
enum HudAimActorQuery {
    kHudCameraQueryInput = 8, kHudSpinQueryRange = 9,
    kHudItemQueryNoSelection = 12, kHudItemQueryTransitionState = 13,
    kHudTutorialQueryDialog = 6
};
enum HudAimActorReply { kHudItemPanelOpening = 1, kHudItemPanelClosing = 2 };

// Coordinate resumed aim, camera permission, and spin/item panel requests.
static inline void ProcessHudAimPanelInput(
    HudProcessState* self, void*& inventory, void*& activePlayer, int& cameraInputReply, float& spinRangeX,
    float& spinRangeY, int& itemPanelStateReply, int& allowed, int& blocked) {

    // Resume restores the saved panel and requests its opening on the next reconciliation.
    if ((self->resumeAimRequested)) {
        (self->requestedPanel)=((self->savedPanel));
        (self->panelOpenRequested)=1;
        (self->resumeAimRequested)=0;
        PlayHudSound(ScreenText_CancelClickSound);
        if (AsCScreen(self)->calipers) {
            inventory=AsCSharedDoc(g_sharedDoc)->itemInventory;
            activePlayer=GolfPlayer_GetEntryByIndex(GAME,PLAYER);
            if (((HudPlayerFields*)activePlayer)->equippedItemId==ItemInventory_GetItemIdAt(inventory,5) || AsCGolfDoc(GAME)->powerShotMeterOverride) CCalipers_ResetMeasurement(AsCScreen(self)->calipers);
        }
        return;
    }
    if (!(self->aimControlsEnabled)) ((HudGolfBaseInterface*)AsCScreen(self)->golfBaseDisplay)->EnableConfirmButton(0);
    else ((HudGolfBaseInterface*)AsCScreen(self)->golfBaseDisplay)->EnableConfirmButton(1);
    if (TutorialStageCount>STAGE) ((HudGolfBaseInterface*)AsCScreen(self)->golfBaseDisplay)->EnableActionButtons((self->tutorialAimControlEnabled));
    if (!((ClubSelectionState*)InstanceTable_Get())->shotOptionEnabled && ((HudStrokePointInterface*)AsCScreen(self)->strokePointDisplay)->GetPowerMode()) ((HudStrokePointInterface*)AsCScreen(self)->strokePointDisplay)->ClearPowerMode();
    // Query camera input before any confirm, spin, or item transition.
    DispatchHudMessage(self,ScreenText_CameraActor,kHudCameraQueryInput,(unsigned int)&cameraInputReply,0);
    if (cameraInputReply) {
        if (1!=cameraInputReply) {
            ((HudGolfBaseInterface*)AsCScreen(self)->golfBaseDisplay)->EnableConfirmButton(0);
            return;
        }
    }
    if (((HudGolfBaseInterface*)AsCScreen(self)->golfBaseDisplay)->IsConfirmActivated()) {
        if (!AsCGolfDoc(GAME)->cameraMode) {
            (self->hudPhase)=HudShot;
            unsigned int action=(self->requestedPanel);
            (self->savedPanel)=action;
            (self->requestedPanel)=(action&HudPanelHighBitsMask)|HudPanelAimClosing;
            CMouseCursor_SetPointerInputEnabled(g_mouseCursor,0);
            PlayHudSound(ScreenText_IconClickSound);
        }
    }
    // previousPanel is the acknowledged presentation; defer new panels until it catches up.
    if ((self->previousPanel)!=(self->requestedPanel)) return;
    if (((HudBallWindInterface*)self->ballWindDisplay)->IsBallSelectorActivated() || ScreenInputPress(HudActionSpin)) {
        if (((ClubSelectionState*)InstanceTable_Get())->selection->clubKind!=3) {
            if (STAGE==TutorialStageCount) {
                (self->hudPhase)=HudSpin;
                (self->savedPanel)=(self->requestedPanel);
                (self->requestedPanel)=HudPanelSpin;
                if (AsCScreen(self)->ingameStatus && GolfState_AllowsStatusTransition(AsCScreen(self)->ingameStatus)) CIngameStatus_BeginHide(AsCScreen(self)->ingameStatus);
                spinRangeX=kFloat_0_AuxMenu;
                spinRangeY=kFloat_0_AuxMenu;
                DispatchHudMessage(self,ScreenText_SpinActor,kHudSpinQueryRange,(unsigned int)&spinRangeX,(unsigned int)&spinRangeY);
                ((HudStrokePointInterface*)AsCScreen(self)->strokePointDisplay)->SetDirectionThresholds(spinRangeX,spinRangeY);
                void* activePlayer=GolfPlayer_GetEntryByIndex(GAME,PLAYER);
                ((HudStrokePointInterface*)AsCScreen(self)->strokePointDisplay)->SetGaugeValue((int)((HudPlayerFields*)activePlayer)->gaugeValue);
                if ((*(int*)&AsCSharedDoc(g_sharedDoc)->partyGameMode)==1) {
                    ((HudStrokePointInterface*)AsCScreen(self)->strokePointDisplay)->EnablePowerMode(0);
                    ((HudStrokePointInterface*)AsCScreen(self)->strokePointDisplay)->EnableDirectionButtons(0);
                } else {
                    ((HudStrokePointInterface*)AsCScreen(self)->strokePointDisplay)->EnableDirectionButtons(1);
                    if (((ClubSelectionState*)InstanceTable_Get())->selection->clubKind==3 || ((CGolfBall*)GolfBall__Fv())->m_groundType==2) ((HudStrokePointInterface*)AsCScreen(self)->strokePointDisplay)->EnablePowerMode(0);
                    else ((HudStrokePointInterface*)AsCScreen(self)->strokePointDisplay)->EnablePowerMode(1);
                }
                if (((ClubSelectionState*)InstanceTable_Get())->selection->clubIndex<3) ((HudStrokePointInterface*)AsCScreen(self)->strokePointDisplay)->EnableExtendedSpecialShots(1);
                else ((HudStrokePointInterface*)AsCScreen(self)->strokePointDisplay)->EnableExtendedSpecialShots(0);
                PlayHudSound(ScreenText_OpenSpinDialogSound);
            }
        }
    }
    if ((self->hudPhase)==HudSpin) return;
    if (((HudPlayerFields*)GolfPlayer_GetEntryByIndex(GAME,PLAYER))->itemMenuAvailable) {
        if (((*(unsigned int*)&AsCGolfDoc(GAME)->gameStateFlags)&HudItemMenuAllowedStates)) {
            if (STAGE==TutorialStageCount) {

                DispatchHudMessage(self,ScreenText_ItemActor,kHudItemQueryTransitionState,(unsigned int)&itemPanelStateReply,0);
                EnsureHudInput();
                if (IsHudActionPressed(HudActionOpenItems) || itemPanelStateReply==kHudItemPanelOpening) {

                    DispatchHudMessage(self,ScreenText_ItemActor,kHudItemQueryNoSelection,(unsigned int)&allowed,0);
                    if (0 == allowed) return;
                    if (g_helpMsg) NoticeQueue_Clear(g_helpMsg);
                    PlayHudSound(ScreenText_IconClickSound);
                    (self->hudPhase)=HudItems;
                    (self->savedPanel)=(self->requestedPanel);
                    (self->requestedPanel)=HudPanelItems;
                    (self->itemPanelOpenRequested)=1;
                    if (AsCScreen(self)->ingameStatus && GolfState_AllowsStatusTransition(AsCScreen(self)->ingameStatus)) CIngameStatus_BeginHide(AsCScreen(self)->ingameStatus);
                    return;
                }
            }
        }
    }
    EnsureHudInput();
    if (!IsHudActionPressed(HudActionAim) || AsCGolfDoc(GAME)->cameraMode || !(self->aimControlsEnabled) || ((HudGolfBaseInterface*)AsCScreen(self)->golfBaseDisplay)->IsCancelActivated()) return;
    blocked=0;
    if (TutorialStageCount>STAGE) DispatchHudMessage(self,ScreenText_TutorialActor,kHudTutorialQueryDialog,(unsigned int)&blocked,0);
    if (AsCScreen(self)->calipers && CCalipers_IsPreviewActive(AsCScreen(self)->calipers)) blocked=1;
    if (blocked) return;
    ((HudGolfBaseInterface*)AsCScreen(self)->golfBaseDisplay)->LatchConfirmAction();
    unsigned int action=(self->requestedPanel);
    (self->hudPhase)=HudShot;
    (self->savedPanel)=action;
    (self->requestedPanel)=(action&HudPanelHighBitsMask)|HudPanelAimClosing;
    CMouseCursor_SetPointerInputEnabled(g_mouseCursor,0);
    PlayHudSound(ScreenText_IconClickSound);
    return;
}

// Close the item panel on cancellation or its actor's close status.
static inline void ProcessHudItemPanelInput(HudProcessState* self, int& itemStatus) {

    DispatchHudMessage(self,ScreenText_ItemActor,kHudItemQueryTransitionState,(unsigned int)&itemStatus,0);
    EnsureHudInput();
    if (IsHudActionPressed(HudActionCancelPanel) || kHudItemPanelClosing==itemStatus) {
        (self->hudPhase)=HudAim;
        (self->requestedPanel)=(self->savedPanel);
        (self->panelOpenRequested)=1;
        (self->itemPanelReady)=0;
        EnsureHudInput();
        if (IsHudActionPressed(HudActionCancelPanel) && g_helpMsg) NoticeQueue_Clear(g_helpMsg);
    }
    return;
}

// Commit or restore spin selection, then update shot-option feedback and return to aiming.
static inline void ProcessHudSpinPanelInput(HudProcessState* self, unsigned int& spinDone, float& spinCommitX, float& spinCommitY) {
    spinDone=0;
    if (((HudStrokePointInterface*)AsCScreen(self)->strokePointDisplay)->IsConfirmationRequested()) spinDone=1;
    else {
        EnsureHudInput();
        if (IsHudActionPressed(HudActionCancelPanel)) {
            ((HudStrokePointInterface*)AsCScreen(self)->strokePointDisplay)->RestoreSavedSelection();

            ((HudStrokePointInterface*)AsCScreen(self)->strokePointDisplay)->GetCommittedPoint(&spinCommitX,&spinCommitY);
            // Preserve float-to-integer payload evaluation order in this spin-dialog commit.
            IActor_DispatchMessage(self,self,screenOverlayStrings + ScreenText_SpinActor,10,(void*)((int)spinCommitX),(void*)((int)spinCommitY),0,0);
            spinDone=1;
        }
    }
    if (spinDone) {
        (self->hudPhase)=HudAim;
        (self->requestedPanel)=(self->savedPanel);
        (self->panelOpenRequested)=1;
        ((HudStrokePointInterface*)AsCScreen(self)->strokePointDisplay)->BeginClosing();
        (self->shotOptionResult)=((HudStrokePointInterface*)AsCScreen(self)->strokePointDisplay)->GetDirectionSelection();
        int option=((HudStrokePointInterface*)AsCScreen(self)->strokePointDisplay)->GetPowerMode();
        if (option!=((ClubSelectionState*)InstanceTable_Get())->shotOptionEnabled) (self->shotOptionChanged)=1;
        if (option) (self->shotOptionEnabled)=1;
        else (self->shotOptionEnabled)=0;
        (self->shotDirectionResult)=((HudStrokePointInterface*)AsCScreen(self)->strokePointDisplay)->GetSpecialShotSelection();
        ((HudGolfBaseInterface*)AsCScreen(self)->golfBaseDisplay)->ShowExtraMeterImage((self->shotOptionEnabled));
    }
    return;
}

// Retire the previous aim/camera/item/spin panel before the shot transition.
static inline void ProcessHudShotTransitionInput(HudProcessState* self) {
    (self->panelsReady)=0;
    int previousPanel=(self->previousPanel);
    (self->panelOpenRequested)=0;
    if (previousPanel==HudPanelAim) ((HudGolfBaseInterface*)AsCScreen(self)->golfBaseDisplay)->BeginFadeOut();
    else if (previousPanel==HudPanelCamera) ((HudMapCameraInterface*)AsCScreen(self)->mapCameraDisplay)->BeginClosing();
    if ((self->itemPanelReady)) {
        (self->itemPanelReady)=0;
        DispatchHudMessage(self,ScreenText_ItemActor,1,0,0);
    } else if ((self->previousHudPhase)==HudSpin) {
        ((HudStrokePointInterface*)AsCScreen(self)->strokePointDisplay)->BeginClosing();
        PlayHudSound(ScreenText_CloseSpinDialogSound);
    }
    return;
}

// Poll layout loading, update loaded children and dispatch the active input phase.
static inline void UpdateHudLoadedPanelsAndInput(
    HudProcessState* self, void*& inventory, void*& activePlayer, int& cameraInputReply, float& spinRangeX,
    float& spinRangeY, int& itemPanelStateReply, int& allowed, int& blocked, int& itemStatus, unsigned int& spinDone,
    float& spinCommitX, float& spinCommitY) {
    if (g_drawDriveLine) CDrawDriveLine_UpdateShotPrediction(g_drawDriveLine);
    if (CScreen_PollLoadComplete(self)) {
        if (!(self->hudInitialized)) {
            CScreen_InitializeLoadedLayouts(self);
            (self->hudInitialized)=1;
        }
    }
    if ((self->hudInitialized)) {
        UpdateLoadedHudChildren(self);
        EnsureHudInput();
        (self->shotOptionChanged)=0;
        RequestHudAimPanelWhenReady(self);
        FinishHudAimPanelClosing(self);
        if ((self->hudPhase)==HudAim && !SharedDoc_IsPlayerFlag0Set(PLAYER)) ((HudBallWindInterface*)self->ballWindDisplay)->SetSelectionActive(1);
        else ((HudBallWindInterface*)self->ballWindDisplay)->SetSelectionActive(0);
        if ((self->dropModeActive)) {
            EnsureHudInput();
            InputManager_RequestPointerInput(Sing_805DC5C8(),1);
            if ((self->requestedPanel)) {
                (self->savedPanel)=(self->requestedPanel);
                (self->requestedPanel)=0;
                (self->panelsReady)=0;
            }
        }
        if (SharedDoc_IsPlayerFlag0Set(PLAYER)) {
            if ((self->hudPhase)==HudAim) {
                (self->hudPhase)=HudShot;
                (self->panelsReady)=0;
            }
        }
        if ((self->shotTransitionRequested)) {
            unsigned int zero;
            unsigned int previous;
            previous=(self->hudPhase);
            zero=0;
            (self->previousHudPhase)=previous;
            (self->requestedPanel)=zero;
            (self->hudPhase)=HudShotTransition;
            (self->savedPanel)=zero;
            AsCGolfDoc(GAME)->cameraMode=zero;
            (self->panelsReady)=1;
            (self->shotTransitionRequested)=zero;
        }
        if ((self->panelsReady)) {
            if (((self->hudPhase)==HudShotTransition || CScreen_AreHudTransitionsComplete(self))) {
                if (!(self->hudInputBlocked)) {
                    switch ((self->hudPhase)) {
                    case HudInactive: case HudShot: default: break;
                    case HudAim: // Handle aim confirmation and requests to open spin/items; scratch preserves the original ABI lifetimes.
                        ProcessHudAimPanelInput(
                            self, inventory, activePlayer, cameraInputReply, spinRangeX, spinRangeY, itemPanelStateReply,
                            allowed, blocked);
                        break;
                    case HudItems: // Close the item panel on cancellation or its actor's close status.
                        ProcessHudItemPanelInput(self, itemStatus);
                        break;
                    case HudSpin: // Commit or restore spin selection, then update shot-option feedback and return to aiming.
                        ProcessHudSpinPanelInput(self, spinDone, spinCommitX, spinCommitY);
                        break;
                    case HudShotTransition: // Retire the previous aim/camera/item/spin panel before the shot transition.
                        ProcessHudShotTransitionInput(self);
                        break;
                    }
                }
            }
        }
    }
}

// Reconcile camera requests and remove selections unsupported by the current club.
static inline void ReconcileHudCameraAndEquipment(HudProcessState* self) {
    // Phase 2: reconcile camera mode, blocked input and equipment restrictions;
    // this shared path also runs before layout initialization.
    if (AsCGolfDoc(GAME)->cameraMode) {
        if (!(self->dropModeActive)) {
            int action=(self->requestedPanel);
            if ((action&1) || (0 == action && !(self->tutorialAimEnabled))) {
                (self->requestedPanel)=(action&HudPanelHighBitsMask)|HudPanelCamera;
                (self->panelsReady)=0;
                (self->panelOpenRequested)=1;
            }
        }
    }
    if ((self->hudInputBlocked)) {
        ((HudMapCameraInterface*)AsCScreen(self)->mapCameraDisplay)->LatchFirstActionAndPlayClick();
        ((HudMapCameraInterface*)AsCScreen(self)->mapCameraDisplay)->BeginClosing();
        if (!(self->tutorialAimEnabled) && STAGE<TutorialStageCount) (self->requestedPanel)=0;
        else (self->requestedPanel)=((self->requestedPanel)&HudPanelHighBitsMask)|HudPanelAim;
        (self->hudInputBlocked)=0;
        (self->panelsReady)=0;
        (self->panelOpenRequested)=1;
    }
    if (((self->shotDirectionResult)==0x20 || (self->shotDirectionResult)==0x40)) {
        if (((ClubSelectionState*)InstanceTable_Get())->selection->clubIndex>=3) {
            (self->shotDirectionResult)=0;
            ((HudStrokePointInterface*)AsCScreen(self)->strokePointDisplay)->ClearSpecialShotSelection();
        }
    }
    if ((self->shotOptionResult)) {
        if (((ClubSelectionState*)InstanceTable_Get())->selection->clubKind==3) {
            (self->shotOptionResult)=0;
            ((HudStrokePointInterface*)AsCScreen(self)->strokePointDisplay)->ClearDirectionSelection();
        }
    }
}

// Update ball controls, timebomb frame cues and the dt-driven shot pulse.
static inline void UpdateHudPartyClocksAndShotPulse(HudProcessState* self, float dt, unsigned int& otherPlayerHasTimebomb) {
    UpdateHudBallWindControls(self);
    if ((*(int*)&AsCSharedDoc(g_sharedDoc)->partyGameMode)==kPartyItemMode) {
        otherPlayerHasTimebomb=0;
        for (int playerIndex=0;playerIndex<AsCGolfDoc(GAME)->playerCount;++playerIndex) {
            if (PartyItemSlots_GetItemType(PartyItemSlots_GetInstance(),playerIndex)==kTimebombPartyItem) {
                if (playerIndex==PLAYER) {
                    otherPlayerHasTimebomb=0;
                    break;
                }
                otherPlayerHasTimebomb=1;
            }
        }
        if (otherPlayerHasTimebomb) {
            void* partyEffects=PartyItemSlots_GetInstance();
            if (((HudPartyEffectsView*)partyEffects)->modifiers.timebombTicks) {
                if (((HudPartyEffectsView*)partyEffects)->modifiers.timebombTicks%TimebombFramesPerTick==0) {
                    void* golfDocument=GAME;
                    unsigned int playerActivity;
                    if ((((unsigned int)AsCGolfDoc(golfDocument)->gameStateFlags&kTimebombTickStateMask) &&
                    (((playerActivity=HudDocumentPlayer(g_sharedDoc,PLAYER)->activityWord) && !AsCGolfDoc(golfDocument)->shotPhase) ||
                    (playerActivity == 0 && AsCGolfDoc(golfDocument)->shotPhase<2))) ||
                    AsCGolfDoc(golfDocument)->cameraMode) PlayHudSound(ScreenText_TimebombTickSound);
                }
            }
            if (((HudPartyEffectsView*)partyEffects)->modifiers.timebombTriggered && ((HudPartyEffectsView*)partyEffects)->modifiers.timebombTicks==((HudPartyEffectsView*)partyEffects)->modifiers.timebombPeriod*TimebombFramesPerTick) PlayHudSound(ScreenText_TimebombFailureSound);
        }
    }
    if (AsCGolfDoc(GAME)->shotPhase==2) {
        float shotPulse=kFloat_100_Screen*dt+((ScreenExtraState*)self)->shotIndicatorPulse;
        ((ScreenExtraState*)self)->shotIndicatorPulse=shotPulse;
        if (shotPulse>kScreenPulseWrapThreshold) ((ScreenExtraState*)self)->shotIndicatorPulse=shotPulse-kScreenPulseWrapSpan;
    }
}

// Advance wind, ball preview, reward display and shot-feedback blend; dt is seconds.
static inline void UpdateHudWindAndPreview(HudProcessState* self, float dt) {
    // Phase 4: update wind, preview/rewards, club effects and optional HUD children.
    WindIndicator_UpdateDirectionAndPhase(CWind_GetInstance(),dt);
    if (AsCScreen(self)->ballPreview) ScreenBallPreview_Update(AsCScreen(self)->ballPreview,dt);
    if (AsCScreen(self)->pangInfo) ((HudTimedChildInterface*)AsCScreen(self)->pangInfo)->UpdateSeconds(dt);
    float feedbackBlend=kScreenShotFeedbackBlendRate*dt+((ScreenExtraState*)self)->shotFeedbackBlend;
    ((ScreenExtraState*)self)->shotFeedbackBlend=feedbackBlend>kFloat_1_AuxMenu ? kFloat_1_AuxMenu:feedbackBlend;
}

// Replace interpreter attachments after the equipped club changes; the interpreter owns returned FX.
static inline void RefreshHudClubAttachments(
    HudProcessState* self, unsigned char& clubKind, void*& sharedDocument, void*& clubModel, void*& clubAsset) {
    if (((ClubSelectionState*)InstanceTable_Get())->changeFlags&2) {
        FxInterpreter_RemoveEntriesByKey(FxInterpreter_GetInstance(),(self->fxInterpreterKey));

        clubKind=((ClubSelectionState*)InstanceTable_Get())->selection->clubKind;
        sharedDocument=g_sharedDoc;
        clubModel=ClubModel_FindCachedForItem(InstanceTable_Get(),HudDocumentPlayer(sharedDocument,AsCGolfDoc(AsCSharedDoc(sharedDocument)->golfDoc)->currentPlayer)->clubItemId);
        clubAsset = *(void* volatile*)&((ClubResourceSlots*)clubModel)->byClubKind[clubKind];
        (self->fxInterpreterKey)=(int)FxAttachments_CreateFromResourceTags(FxInterpreter_GetInstance(),clubAsset,0);
    }
}

// Update club angle and optional child displays; calipers run only for an enabled aim preview.
static inline void UpdateHudTimedChildren(HudProcessState* self, float dt, void*& inventory, void*& activePlayer) {
    UpdateHudClubChangeAngle(self,dt);
    if (AsCScreen(self)->calipers) {
        if ((self->hudPhase)==HudAim) {
            inventory=AsCSharedDoc(g_sharedDoc)->itemInventory;
            activePlayer=GolfPlayer_GetEntryByIndex(GAME,PLAYER);
            if (((HudPlayerFields*)activePlayer)->equippedItemId==ItemInventory_GetItemIdAt(inventory,5) || AsCGolfDoc(GAME)->powerShotMeterOverride) ((HudTimedChildInterface*)AsCScreen(self)->calipers)->UpdateSeconds(dt);
        }
    }
    if (AsCScreen(self)->holeInfoSet) {
        if (AsCGolfDoc(GAME)->cameraMode || ((*(unsigned int*)&AsCGolfDoc(GAME)->gameStateFlags)&8)) {
            CAccuracyInfo_ShowShotFlagFeedback(((HudHoleInfoChildren*)AsCScreen(self)->holeInfoSet)->accuracyInfo,(self->shotDirectionResult)|(self->shotOptionResult),(((HudStrokePointInterface*)AsCScreen(self)->strokePointDisplay)->GetPowerMode()));
        }
        CHoleInfoSet_Update(AsCScreen(self)->holeInfoSet,dt);
    }
    if (AsCScreen(self)->flyInfo) ((HudTimedChildInterface*)AsCScreen(self)->flyInfo)->UpdateSeconds(dt);
    if (AsCScreen(self)->accuracyInfo) ((HudTimedChildInterface*)AsCScreen(self)->accuracyInfo)->UpdateSeconds(dt);
    if (AsCScreen(self)->modeInfo) ((HudTimedChildInterface*)AsCScreen(self)->modeInfo)->UpdateSeconds(dt);
    if (AsCScreen(self)->ingameStatus && 5<=STAGE) CIngameStatus_Update(AsCScreen(self)->ingameStatus,dt);
}

// Restart or advance caption opacity when the camera gameplay state changes.
static inline void UpdateHudCameraCaption(HudProcessState* self, float dt) {
    if (AsCGolfDoc(GAME)->cameraMode) {
        unsigned int previousStates=AsCScreen(self)->previousGameStateFlags;
        int currentStates=(*(unsigned int*)&AsCGolfDoc(GAME)->gameStateFlags);
        if (previousStates!=currentStates) {
            if (!(previousStates&8) || !(currentStates&0x100) || !(8&currentStates) || !(previousStates&0x100)) self->cameraCaptionOpacity=kScreenCameraCaptionFadeStart;
            AsCScreen(self)->previousGameStateFlags=(*(unsigned int*)&AsCGolfDoc(GAME)->gameStateFlags);
        } else {
            float captionOpacity=kScreenCameraCaptionFadeRate*dt+self->cameraCaptionOpacity;
            self->cameraCaptionOpacity=captionOpacity>kFloat_255_Screen ? kFloat_255_Screen:captionOpacity;
        }
    }
}

// Re-enable the delayed tutorial pointer region after its seconds-based delay.
static inline void UpdateHudTutorialPointerDelay(HudProcessState* self, float dt) {
    if (self->tutorialPointerDelayed) {
        float pointerDelaySeconds=self->tutorialPointerDelay+dt;
        self->tutorialPointerDelay=pointerDelaySeconds;
        if (pointerDelaySeconds > (kFloat_5_Screen)) {
            self->tutorialPointerDelayed=0;
            ((CMouseCursor*)g_mouseCursor)->regionEnabled[HudRegionDelayedTutorialControl]=1;
        }
    }
}

// Dispatch pointer actions; release actions must match their button-down region.
static inline void ProcessHudPointerRegions(HudProcessState* self, int& panelStatus, unsigned char& scorePanelBlocksInput) {
    // Phase 5: pointer-region messages. Region 5=club menu, 10..13=activePlayer
    // entries, 31=tutorial action, 32=delayed tutorial control, 35=one-shot close.
    // A release must match the button-down region before activating a control.
    // The original hitRegion-to-handler table lives in another data unit.
    unsigned int hitRegion=((CMouseCursor*)g_mouseCursor)->hitRegion;
    {
        switch (hitRegion) {
        case 6: case 7: case 8: case 9:
        case 14: case 15: case 16: case 17: case 18: case 19:
        case 20: case 21: case 22: case 23: case 24: case 25:
        case 26: case 27: case 28: case 29: case 30:
        case 33: case 34:
        default: break;
        case HudRegionClubMenu: {

                DispatchHudMessage(self,ScreenText_ItemActor,5,(unsigned int)&panelStatus,0);
                if (3!=panelStatus) break;
                DispatchHudMessage(self,ScreenText_PlayerActor,21,(unsigned int)&panelStatus,0);
                if (3==panelStatus || Pointer_GetButtonState(g_inputPointer,HudPrimaryPointerButton)!=HudPointerPressed || !((*(unsigned int*)&AsCScreen(self)->hudFlags)&2)) break;
                // The same document byte also suspends the shot clock; keep it in the club-menu gate.
                if (AsCGolfDoc(GAME)->shotPhase || AsCGolfDoc(GAME)->unk_35D || 2>STAGE || ((ClubSelectionState*)InstanceTable_Get())->clubMenuState!=2) break;
                DispatchHudMessage(self,ScreenText_GolfRuleActor,5,0,0);
                break;
            }
        case HudRegionPlayerFirst: case 11: case 12: case HudRegionPlayerLast: {

                DispatchHudMessage(self,ScreenText_ScoreActor,13,(unsigned int)&scorePanelBlocksInput,0);
                if (scorePanelBlocksInput || Pointer_GetButtonState(g_inputPointer,HudPrimaryPointerButton)!=HudPointerReleased || ((CMouseCursor*)g_mouseCursor)->hitRegion!=((CMouseCursor*)g_mouseCursor)->buttonDownRegions[0] || !GolfTask_NoOpStatus()) break;
                if (((HudPlayerFields*)GolfPlayer_GetEntryByIndex(GAME,(unsigned char)(((CMouseCursor*)g_mouseCursor)->hitRegion-HudRegionPlayerFirst)))->status08==3) break;
                PlayHudSound(ScreenText_OpenPlayMenuSound);
                CMouseCursor_SetRadialMenuEnabled(g_mouseCursor,0);
                break;
            }
        case HudRegionTutorialAction:
            if (Pointer_GetButtonState(g_inputPointer,HudPrimaryPointerButton)==HudPointerReleased && ((CMouseCursor*)g_mouseCursor)->hitRegion==((CMouseCursor*)g_mouseCursor)->buttonDownRegions[0]) DispatchHudMessage(self,ScreenText_TutorialActor,3,0,0);
            break;
        case HudRegionDelayedTutorialControl:
            if (Pointer_GetButtonState(g_inputPointer,HudPrimaryPointerButton)==HudPointerReleased) {
                if (((CMouseCursor*)g_mouseCursor)->hitRegion==((CMouseCursor*)g_mouseCursor)->buttonDownRegions[0]) {
                    if (STAGE<TutorialStageCount) {
                        if (self->tutorialPointerDelayed == 0) {
                            self->tutorialPointerDelayed=1;
                            self->tutorialPointerDelay=kFloat_0_AuxMenu;
                            ((CMouseCursor*)g_mouseCursor)->regionEnabled[HudRegionDelayedTutorialControl]=0;
                        }
                    }
                }
            }
            break;
        case HudRegionOneShotClose:
            if (Pointer_GetButtonState(g_inputPointer,HudPrimaryPointerButton)==HudPointerReleased && ((CMouseCursor*)g_mouseCursor)->hitRegion==((CMouseCursor*)g_mouseCursor)->buttonDownRegions[0]) ((CMouseCursor*)g_mouseCursor)->regionEnabled[HudRegionOneShotClose]=0;
            break;
        case 3: break;
        }
    }
}

// Alias a feedback difference as IEEE-754 bits for the original absolute-value test.
union HudFeedbackChannelBits { int u; float f; };

// Derive and clamp the impact zone in gauge coordinates, then animate feedback RGB.
static inline void UpdateHudImpactZone(
    HudProcessState* self, void*& inventory, void*& activePlayer, float& colorEqualityTolerance,
    float& proposedTargetOffset, float& baseImpactRange, float& maximumTargetOffset, float& adjustedImpactRange,
    float& heading, void*& partyEffects, void*& impactWidthInventory, void*& impactWidthPlayer,
    float& targetDistance, char*& channelOwnerCursor, HudFeedbackChannelBits& intensityDifferenceBits) {
    // Phase 6: derive Pangya impactZoneWidth/target from lie, equipment and heading.
    // Gauge positions are HUD coordinates, not world distances; clamp the target
    // so its half-impactZoneWidth stays inside the good-impact interval. Then age notices.
    if (((ClubSelectionState*)InstanceTable_Get())->selection->clubKind==3) AsCGolfDoc(GAME)->pangyaTargetX=kPangyaTargetCenterX;
    else {

        if (!HudDocumentPlayer(g_sharedDoc,PLAYER)->activityWord) {
            baseImpactRange=((ClubSelectionState*)InstanceTable_Get())->selection->baseImpactRange;
            adjustedImpactRange=baseImpactRange-kFloat_10_Screen*(float)((HudPlayerFields*)GolfPlayer_GetEntryByIndex(GAME,PLAYER))->impactRangeAdjustment;
            if (((CGolfBall*)GolfBall__Fv())->m_groundType==3) {
                switch (((ClubSelectionState*)InstanceTable_Get())->selection->clubKind) {
                case 0: adjustedImpactRange*=kScreenGroundLieRangeScale1_5;
                    break;
                case 1: adjustedImpactRange*=kScreenGroundLieRangeScale1_3;
                    break;
                case 2: adjustedImpactRange*=kFloat_1_2_ScreenOverlay;
                    break;
                }
            } else if (((CGolfBall*)GolfBall__Fv())->m_groundType==7) {
                switch (((ClubSelectionState*)InstanceTable_Get())->selection->clubKind) {
                case 0: adjustedImpactRange*=kScreenGroundLieRangeScale1_3;
                    break;
                case 1: adjustedImpactRange*=kFloat_1_2_ScreenOverlay;
                    break;
                case 2: default: break;
                }
            } else if (((CGolfBall*)GolfBall__Fv())->m_groundType==4) {
                switch (((ClubSelectionState*)InstanceTable_Get())->selection->clubKind) {
                case 0: adjustedImpactRange*=kFloat_1_1_ScreenRange;
                    break;
                case 1: adjustedImpactRange*=kFloat_1_1_ScreenRange;
                    break;
                case 2: adjustedImpactRange*=kFloat_1_1_ScreenRange;
                    break;
                }
            } else GolfBall__Fv();
            inventory=AsCSharedDoc(g_sharedDoc)->itemInventory;
            activePlayer=GolfPlayer_GetEntryByIndex(GAME,PLAYER);
            if (((HudPlayerFields*)activePlayer)->equippedItemId==ItemInventory_GetItemIdAt(inventory,7) ||
            (inventory=AsCSharedDoc(g_sharedDoc)->itemInventory, activePlayer=GolfPlayer_GetEntryByIndex(GAME,PLAYER),
            ((HudPlayerFields*)activePlayer)->equippedItemId==ItemInventory_GetItemIdAt(inventory,8)) ||
            (inventory=AsCSharedDoc(g_sharedDoc)->itemInventory, activePlayer=GolfPlayer_GetEntryByIndex(GAME,PLAYER),
            ((HudPlayerFields*)activePlayer)->equippedItemId==ItemInventory_GetItemIdAt(inventory,9)) ||
            (inventory=AsCSharedDoc(g_sharedDoc)->itemInventory, activePlayer=GolfPlayer_GetEntryByIndex(GAME,PLAYER),
            ((HudPlayerFields*)activePlayer)->equippedItemId==ItemInventory_GetItemIdAt(inventory,2)))
            adjustedImpactRange*=kFloat_0_5_AuxMenu;
            if (kFloat_0_AuxMenu!=adjustedImpactRange) AsCGolfDoc(GAME)->pangyaZoneScale=kScreenImpactRangeNormalization/adjustedImpactRange;
        }
        float impactZoneWidth=kFloat_2_805DED94;
        partyEffects=PartyItemSlots_GetInstance();
        impactWidthInventory=AsCSharedDoc(g_sharedDoc)->itemInventory;
        impactWidthPlayer=GolfPlayer_GetEntryByIndex(GAME,PLAYER);
        if (((HudPlayerFields*)impactWidthPlayer)->equippedItemId==ItemInventory_GetItemIdAt(impactWidthInventory,9) ||
        (impactWidthInventory=AsCSharedDoc(g_sharedDoc)->itemInventory, impactWidthPlayer=GolfPlayer_GetEntryByIndex(GAME,PLAYER),
        ((HudPlayerFields*)impactWidthPlayer)->equippedItemId==ItemInventory_GetItemIdAt(impactWidthInventory,1)))
        impactZoneWidth*=kFloat_2_805DED94;
        else impactZoneWidth*=kFloat_1_AuxMenu+((HudPartyEffectsView*)partyEffects)->modifiers.pangyaZoneWidthBonus;
        impactZoneWidth*=AsCGolfDoc(GAME)->pangyaZoneScale;
        if ((*(int*)&AsCSharedDoc(g_sharedDoc)->partyGameMode)==3 && !HudDocumentPlayer(g_sharedDoc,PLAYER)->activityWord) impactZoneWidth/=((HudPartyEffectsView*)partyEffects)->modifiers.impactZoneWidthDivisor;
        DispatchHudMessage(self,ScreenText_PlayerActor,9,(unsigned int)&heading,0);
        heading=(float)floor(kScreenRadiansToDegrees*(heading*kFloat_0_5_AuxMenu));
        float absoluteHeading=heading>kFloat_0_AuxMenu ? heading:-heading;
        if (absoluteHeading<kScreenImpactHeadingDeadZone) heading=kFloat_0_AuxMenu;
        else if (heading>kFloat_0_AuxMenu) *(volatile float*)&heading=heading-kFloat_2_805DED94;
        else heading=heading+kFloat_2_805DED94;
        if (!HudDocumentPlayer(g_sharedDoc,PLAYER)->activityWord) {
            if (((float(*)(void*))ShotParameters_GetGoodImpactHalfWidth)(InstanceTable_Get())<impactZoneWidth) {
                AsCGolfDoc(GAME)->pangyaZoneScale*=(((float(*)(void*))ShotParameters_GetGoodImpactHalfWidth)(InstanceTable_Get())/impactZoneWidth);
                impactZoneWidth=((float(*)(void*))ShotParameters_GetGoodImpactHalfWidth)(InstanceTable_Get());
            }
        }
        maximumTargetOffset=((float(*)(void*))ShotParameters_GetGoodImpactHalfWidth)(InstanceTable_Get())-impactZoneWidth;
        proposedTargetOffset=heading;
        float minimumTargetOffset=kFloat_neg1_Screen*((float(*)(void*))ShotParameters_GetGoodImpactHalfWidth)(InstanceTable_Get())+impactZoneWidth;
        float clampedTargetOffset=proposedTargetOffset<minimumTargetOffset ? minimumTargetOffset:(proposedTargetOffset>maximumTargetOffset ? maximumTargetOffset:proposedTargetOffset);
        AsCGolfDoc(GAME)->pangyaTargetX=(kPangyaTargetCenterX+clampedTargetOffset);
        ((HudGolfBaseInterface*)AsCScreen(self)->golfBaseDisplay)->SetPangyaTargetX(AsCGolfDoc(GAME)->pangyaTargetX);
        ((HudGolfBaseInterface*)AsCScreen(self)->golfBaseDisplay)->SetPangyaZoneWidth((((impactZoneWidth*kFloat_0_5_AuxMenu))));
        targetDistance=AsCGolfDoc(GAME)->roundedShotGaugeCursorX-AsCGolfDoc(GAME)->pangyaTargetX;
        targetDistance=targetDistance>kFloat_0_AuxMenu ? targetDistance:-targetDistance;
        if (targetDistance<((float(*)(void*))ShotParameters_GetGoodImpactHalfWidth)(InstanceTable_Get()) && AsCGolfDoc(GAME)->shotPhase==2) {
            channelOwnerCursor=(char*)self;
            colorEqualityTolerance=kScreenColorEqualityEpsilon;
            for (int channelIndex=0;3>channelIndex;++channelIndex,channelOwnerCursor+=4) {

                intensityDifferenceBits.f=AsCScreen(channelOwnerCursor)->pangyaFeedbackRed-g_screenFullFeedbackIntensity;
                __typeof__(intensityDifferenceBits.f) channelDifference = intensityDifferenceBits.f;
                double absoluteChannelDifference=channelDifference;
                if (intensityDifferenceBits.u&0x80000000) absoluteChannelDifference=-absoluteChannelDifference;
                double absoluteDifference = absoluteChannelDifference;
                bool channelIsAtFullIntensity = absoluteDifference<colorEqualityTolerance;
                if ((channelIsAtFullIntensity)) AsCScreen(channelOwnerCursor)->pangyaFeedbackRed=Random_RangeFloat(-1,kScreenImpactFeedbackMinimumIntensity,kFloat_255_Screen);
            }
        } else {
            float fullIntensity=kFloat_255_Screen;
            self->pangyaFeedbackRed=fullIntensity;
            float retainedFullIntensity = fullIntensity;
            self->pangyaFeedbackGreen=retainedFullIntensity;
            self->pangyaFeedbackBlue=fullIntensity;
        }
    }
}

#pragma pop

// Advance HUD panels, handle aiming/menu input, and update tutorial, shot, wind, and impact feedback.
// dt is seconds. Frame counters (blink counts, party ticks, meter transitions)
// advance per call; timer/angle/fade fields use dt. Keep those clocks distinct.
// This coordinator intentionally runs the shared phases even before layouts
// finish loading; only the first panel/input phase requires hudInitialized.
// Order: loaded child displays and panel input; camera/equipment reconciliation;
// tutorial clocks and control visibility; wind/club/feedback updates; pointer actions; impact width/target.
// requestedPanel holds a HudPanel value in its low nibble; bits above (HudPanelHighBitsMask) keep item/spin panel state.
void CScreen_OnProcess(HudProcessState* self,float dt) {
    // These scratch scopes preserve CodeWarrior stack/register allocation across inlined phases.
    // Helpers borrow each reference for the call and never retain it.
    // Phase 1: load completion, child updates and panel input. HudPhase decides
    // which input protocol is active; requestedPanel decides which widgets open.
    void* inventory;
    void* activePlayer;
    float colorEqualityTolerance;
    float proposedTargetOffset;
    float baseImpactRange;
    float maximumTargetOffset;
    float terrainAngleLimit;
    float adjustedImpactRange;
    if (!(self->hudEnabled)) return;
    {
        int cameraInputReply;
        float spinRangeX;
        float spinRangeY;
        int itemPanelStateReply;
        int allowed;
        int blocked;
        int itemStatus;
        unsigned int spinDone;
        float spinCommitX;
        float spinCommitY;
        UpdateHudLoadedPanelsAndInput(
            self, inventory, activePlayer, cameraInputReply, spinRangeX, spinRangeY, itemPanelStateReply, allowed, blocked,
            itemStatus, spinDone, spinCommitX, spinCommitY);
    }
    ReconcileHudCameraAndEquipment(self);
    if (STAGE<TutorialStageCount) {
        unsigned int flashDirectionScratch;
        float titleOriginPixels[2];
        if ((self->tutorialIllustrationActive)) {
            EnsureHudInput();
            if (IsHudActionPressed(HudActionCamera)) {
                ((HudTutorialIllustrationInterface*)AsCScreen(self)->tutorialIllustration)->OnCameraAction();
                (self->tutorialIllustrationActive)=0;
                DispatchHudMessage(self,ScreenText_TutorialActor,0,0,0);
            }
        }
        unsigned char flashCountScratch;
        UpdateHudTutorialFlashClocks(self,dt,flashDirectionScratch,flashCountScratch);
        float titleDestinationPixels[2];
        if (STAGE==1) {
            float viewportHeight;
            float titleOriginX;
            viewportHeight=g_view->viewportHeight;
            float titleOriginY;
            titleOriginY=kFloat_0_3_AuxMenu*viewportHeight;
            float halfViewportScale=kFloat_0_5_AuxMenu;
            float viewportWidth=g_view->viewportWidth;
            titleOriginX=(halfViewportScale*viewportWidth-kFloat_50_Screen)-kScreenTutorialTitleInsetX;
            float titleOriginScratch[2];
            titleOriginScratch[1]=titleOriginY;
            titleOriginScratch[0]=titleOriginX-kScreenMarkerInsetX;
            Vec2_Copy(titleOriginPixels,titleOriginScratch);
        } else {
            float halfViewportScale;
            float titleOriginY;
            float viewportHeight;
            float viewportWidth;
            float titleOriginX;
            halfViewportScale=kFloat_0_5_AuxMenu;
            viewportWidth=g_view->viewportWidth;
            titleOriginX=halfViewportScale*viewportWidth-kFloat_50_Screen;
            viewportHeight=g_view->viewportHeight;
            titleOriginY=kFloat_0_3_AuxMenu*viewportHeight;
            float titleOriginScratch[2];
            titleOriginScratch[1]=titleOriginY;
            titleOriginScratch[0]=titleOriginX-kScreenTutorialTitleInsetX;
            Vec2_Copy(titleOriginPixels,titleOriginScratch);
        }
        float destinationHeightFraction;
        float viewportHeight;
        float titleDestinationX;
        float titleDestinationY;
        destinationHeightFraction=kFloat_0_05_Screen;
        viewportHeight=g_view->viewportHeight;
        titleDestinationX=g_view->viewportWidth;
        titleDestinationY=destinationHeightFraction*viewportHeight;
        titleDestinationX=titleDestinationX-kScreenTutorialTitleDestinationInsetX;
        titleDestinationPixels[1]=titleDestinationY;
        titleDestinationPixels[0]=titleDestinationX;
        switch ((self->tutorialTitlePhase)) {
        case TutorialTitleFadingIn: { // Fade in at the stage-specific origin.
                float titleTime;
                float maximumTitleTime;
                titleTime=self->tutorialTitleTimer;
                titleTime+=dt;
                maximumTitleTime=kFloat_1_AuxMenu;
                self->tutorialTitleTimer=titleTime;
                if (titleTime>maximumTitleTime) {
                    self->modeLabelOpacity=255;
                    self->tutorialTitleTimer=(kFloat_0_AuxMenu);
                    (self->tutorialTitlePhase)=TutorialTitleMoving;
                } else self->modeLabelOpacity=(int)(kFloat_255_Screen*titleTime);
                Vec2_Copy(&self->tutorialTitleX,titleOriginPixels);
                break;
            }
        case TutorialTitleMoving: { // Move from the origin to the destination in viewport pixels.
                float maximumTitleTime=kFloat_1_AuxMenu;
                __typeof__(self->tutorialTitleTimer) previousTitleTime = self->tutorialTitleTimer;
                __typeof__(self->tutorialTitleTimer+dt) titleTimeAfterStep = self->tutorialTitleTimer+dt;
                __typeof__(self->tutorialTitleTimer+dt) retainedTitleTimeAfterStep = titleTimeAfterStep;
                __typeof__(retainedTitleTimeAfterStep) nextTitleTime = previousTitleTime+dt;
                float titleTime=nextTitleTime;
                self->tutorialTitleTimer=titleTime;
                if (titleTime>maximumTitleTime) {
                    float savedMaximum = maximumTitleTime;
                    self->tutorialTitleTimer=savedMaximum;
                    (self->tutorialTitlePhase)=TutorialTitleArrived;
                    DispatchHudMessage(self,ScreenText_TutorialActor,0,0,0);
                }
                __typeof__(self->tutorialTitleTimer) titleMoveWeight = self->tutorialTitleTimer;
                unsigned long long interpolatedTitlePosition,weightedOrigin,weightedDestination;
                weightedDestination=Vec2_ScaleByFloat(titleDestinationPixels,titleMoveWeight);
                weightedOrigin=Vec2_ScaleByFloat(titleOriginPixels,kFloat_1_AuxMenu-self->tutorialTitleTimer);
                interpolatedTitlePosition=Vec2_Add(&weightedOrigin,&weightedDestination);
                Vec2_Copy(&self->tutorialTitleX,&interpolatedTitlePosition);
                break;
            }
        case TutorialTitleArrived: default: break; // Hold the completed title position.
        }
    }
    {
        unsigned int otherPlayerHasTimebomb;
        UpdateHudPartyClocksAndShotPulse(self, dt, otherPlayerHasTimebomb);
    }
    UpdateHudWindAndPreview(self, dt);
    {
        unsigned char clubKind;
        void* sharedDocument;
        void* clubModel;
        void* clubAsset;
        RefreshHudClubAttachments(self, clubKind, sharedDocument, clubModel, clubAsset);
    }
    UpdateHudTimedChildren(self, dt, inventory, activePlayer);
    UpdateHudCameraCaption(self, dt);
    if (((CGolfBall*)GolfBall__Fv())->m_groundType==2) {
        terrainAngleLimit=kScreenTerrainIndicatorAngleLimit;
        void* golfDocument=GAME;
        void* holeRecord=(char*)((HudHoleRecordView*)AsCGolfDoc(golfDocument)->holeRecords + AsCGolfDoc(golfDocument)->currentHoleIndex);
        float ballToHole[3];
        int holePositionWords[3];
        struct Position {
            float v[3];
        }
        ;
        *(Position*)holePositionWords=*(Position*)&((HudHoleRecordView*)holeRecord)->position;
        __mi__FRC7WVectorRC7WVector(ballToHole,holePositionWords,GolfBall__Fv());
        float angle=Vector_SignedAngleXZ(ballToHole,&((HudHeadingReferenceView*)golfDocument)->headingReference);
        angle=angle>kFloat_0_AuxMenu ? angle:-angle;
        float terrainHeadingWeight=terrainAngleLimit-angle;
        terrainHeadingWeight=kFloat_0_AuxMenu<terrainHeadingWeight ? terrainHeadingWeight:kFloat_0_AuxMenu;
        int terrainIndicatorAlpha=(int)(kFloat_128_ShotGauge*(terrainHeadingWeight/terrainAngleLimit));
        if (((ClubSelectionState*)InstanceTable_Get())->shotOptionEnabled) AsCScreen(self)->terrainIndicatorColor=((unsigned int)terrainIndicatorAlpha<<24)|0xFFF600;
        else AsCScreen(self)->terrainIndicatorColor=((unsigned int)terrainIndicatorAlpha<<24)|0xFF96;
    }
    UpdateHudTutorialPointerDelay(self, dt);
    {
        int panelStatus;
        unsigned char scorePanelBlocksInput;
        ProcessHudPointerRegions(self, panelStatus, scorePanelBlocksInput);
    }
    if (g_helpMsg) NoticeQueue_UpdateTimers(g_helpMsg,dt);
    if (g_headIcon) HeadIcon_UpdatePlayerTimers(g_headIcon,dt);
    {
        float heading;
        void* partyEffects;
        void* impactWidthInventory;
        void* impactWidthPlayer;
        float targetDistance;
        char* channelOwnerCursor;
        HudFeedbackChannelBits intensityDifferenceBits;
        UpdateHudImpactZone(
            self, inventory, activePlayer, colorEqualityTolerance, proposedTargetOffset, baseImpactRange,
            maximumTargetOffset, adjustedImpactRange, heading, partyEffects, impactWidthInventory,
            impactWidthPlayer, targetDistance, channelOwnerCursor, intensityDifferenceBits);
    }
    if (AsCScreen(self)->skinsInfo) ((HudTimedChildInterface*)AsCScreen(self)->skinsInfo)->UpdateSeconds(dt);
    float remainingDisplaySeconds=(*(float*)&AsCScreen(self)->displayTimeRemaining)-dt;
    (*(float*)&AsCScreen(self)->displayTimeRemaining)=kFloat_0_AuxMenu<remainingDisplaySeconds ? remainingDisplaySeconds:kFloat_0_AuxMenu;
}
}

#undef GAME
#undef PLAYER
#undef STAGE

extern "C" {

// The draw path deliberately reads the initialized word as a pointer-sized status.
struct HudDisplayState {
    char padding000[0x25C];
    void* hudInitialized;
};

extern const float kScreenUpdatedTextX; // 550.0f: updated-text X coordinate

void GridSurface_DiagnosticStub(void*, const char*, ...);
void BpObject_TryInvokeCurrentCallback(void*, int);
void BpObject_Draw(void*);
void Layout_DrawWithOpacity(void*, float);
void __as__7WVectorFRC7WVector(void*, void*);
void CScreen_DrawClubModelOverlay(HudDisplayState*);
void ScreenBallPreview_Draw(void*);
void GolfTips_ResetActiveDialogForRoundState();
void WOverlay_DrawTransformedRect(void*, void*, void*, float, void*, int, unsigned int, int);
void CDrawDriveLine_DrawRibbon(HudDisplayState*);
void Engine_ClearFramebuffer(int, int, int, int, int, float);
void BpObject_SetEnabled(void*, int);
void UIRender_SetDefaultState();
void CHoleInfoSet_Draw(void*);
int strcmp(const char*, const char*);

void CIngameStatus_UpdateAndDraw(void*);

void CWind_DrawDirectionModel(void*, void*, float, float);
void PartyItemSlots_Draw(int, int);

extern "C" void BpObject_StartScriptByName(void*, const char*, int);
int BpObject_IsCurrentScriptComplete(void*);
extern "C" void* BpObject_GetOrCreateNamedGroup(void*, const char*);
void UIGroup_SetTintColor(void*,unsigned int);
void UIGroup_SetOpacity(void*,int);
void UITextTable_SetInteger(void*,int,int);
void UITextTable_Draw(void*);

extern unsigned int kScreenFeatherCallbackFloat2Bits,kScreenFeatherCallbackFloat0Bits; // float bit patterns 2.0f and 0.0f for the feather callback
extern const float kScreenSpinMarkerInsetY; // 26.0f: spin marker Y inset
extern const float kFloat_32_screen;
extern const float kScreenBallConditionBaselineInset; // 28.0f: ball-condition marker baseline inset
extern const float kFloat_64_Screen;
extern const float kScreenBallConditionX; // 35.0f: ball-condition marker X coordinate
extern const float kFloat_55_Screen;
extern const float kFloat_256_GolfScreen;
extern const float kScreenShotOverlayX; // 21.0f: shot overlay X coordinate
extern const float kScreenPatternBaselineInset; // 60.0f: pattern overlay baseline inset
extern const float kScreenPatternX; // 3.0f: pattern overlay X coordinate
extern const float kFloat_51_ShotGauge;
extern const float kScreenClickIndicatorX; // 99.0f: click indicator X coordinate
extern const float kScreenGaugeClickInsetX; // 23.0f: gauge click indicator inset
extern const float kFloat_8_GolfScreen;
extern const float kScreenImpactClickBaselineInset; // 71.0f: impact click indicator baseline inset
extern const float kScreenImpactClickOffsetY; // 12.0f: impact click indicator Y offset
extern const float kScreenImpactClickX; // 134.0f: impact click indicator X coordinate
extern const float kScreenPracticeOptionsY; // 245.0f: practice-options Y coordinate
extern const float kScreenCommandGlyphStartX; // 209.0f: command glyph initial X coordinate
extern const float kScreenCommandAndMenuRowSpacing; // 22.0f: command/menu row spacing
void NamedText_DrawAlignedScaled(void*,const char*,int,float,float,float);
extern "C" void String_ReplaceRange(void*, unsigned int, unsigned int, const char*, const char*, void*);
void appFreeUni__FPv(void*);

#define FIELD(T,p,o) (*(T*)((char*)(p)+(o)))
#define STATE (*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc)
#define PLAYER AsCGolfDoc(STATE)->currentPlayer
#define CALL(p,v,o) Invoke<v,o>(p)
#define QUERY(p,v,o) ((*(int (**)(void*))((char*)FIELD(void*,p,v)+o))(p))
}

struct Renderable {
    virtual void slot0();
    virtual void slot4();
    virtual void slot8();
    virtual void slotC();
    virtual void slot10();
};
// Invoke the no-argument virtual slot O at the interface vtable stored at offset V.
template<int V,int O> static inline void Invoke(void* p) {
    (*(void (**)(void*))((char*)FIELD(void*,p,V)+O))(p);
}
// Invoke the common draw hook through its typed renderable interface.
template<> inline void Invoke<0,16>(void* p) { ((Renderable*)p)->slot8(); }

struct Rect4 {
    float x,y,w,h;
    // Leave rectangle components uninitialized until assigned by the caller.
    Rect4() {}
    // Initialize the rectangle as x/y/width/height in HUD coordinates.
    Rect4(float a,float b,float c,float d) : x(a),y(b),w(c),h(d) {}
    // Access one rectangle component; requires an index in 0..3.
    float& operator[](int i) { return (&x)[i]; }
    // Expose the rectangle storage as the raw pointer expected by layout calls.
    operator void*() { return this; }
};

struct HudString {
    unsigned int bits;
    unsigned int length;
    char* data;
    // Initialize an empty inline string with no heap allocation.
    HudString() : bits(0),length(0),data(0) {}
    // Free string storage only when the heap-ownership bit is set.
    ~HudString() { if(bits>>31) appFreeUni__FPv(data); }
    // Return the length from the inline prefix or heap-string length word.
    unsigned int size() const { return !(bits>>31) ? *(const unsigned char*)this & 0x7f : length; }
    // Return borrowed NUL-terminated inline or heap characters.
    const char* c_str() const { return !(bits>>31) ? (const char*)this+1 : data; }
    // Replace the whole string: legacy append name is reversed.
    void append(const char* const& text) { unsigned int n=size(); unsigned int allocator; volatile unsigned int source; *(char*)&allocator=*(volatile char*)&source; String_ReplaceRange(this,0,n,text,text+strlen(text),&allocator); }
    // Append at the current end: legacy assign name is reversed.
    void assign(const char* const& text) { unsigned int n=size(); unsigned int allocator; volatile unsigned int source; *(char*)&allocator=*(volatile char*)&source; String_ReplaceRange(this,n,0,text,text+strlen(text),&allocator); }
};

// Return a borrowed named overlay under the registry lock, or null for a missing key/table.
static inline void* Lookup(HudDisplayState* const& self, const char* const& name) {
    InstanceLock_u80179180 lock((char*)&((ScreenRegistryLock*)self)->mutex);
    if (!name || !AsCScreen(self)->nameHashBuckets) return 0;
    unsigned int hash;
    {
        void* mutex=(char*)&((ScreenRegistryLock*)self)->mutex;
        OSLockMutex((OSMutex*)(mutex));
        int n = strlen(name);
        unsigned int h = (unsigned char)name[0] & 0x2a;
        int index = n > 0 ? n-1 : 0;
        h |= (unsigned char)name[index] & 0x15;
        h = (h & 0x3f) | ((unsigned int)n<<6);
        hash = h & (*(unsigned int*)&AsCScreen(self)->nameHashMask);
        OSUnlockMutex((OSMutex*)(mutex));
    }
    void* node = ((void**)AsCScreen(self)->nameHashBuckets)[hash];
    while (node) {
        if (!strcmp(((OverlayRegistryNodeView*)node)->key,name)) return ((OverlayRegistryNodeView*)node)->value;
        node = ((OverlayRegistryNodeView*)node)->bucketNext;
    }
    return 0;
}


#define DRAWKEY(key,rect,color) { void* entry = Lookup(self,key); if(entry && ((NamedOverlayView*)entry)->resource) { WOverlay_DrawTransformedRect(((NamedOverlayView*)entry)->resource,g_view,Rect4(kFloat_0_AuxMenu,kFloat_0_AuxMenu,kFloat_1_AuxMenu,kFloat_1_AuxMenu),kFloat_0_AuxMenu,rect,0,color,0); } }
#define DRAW(off,rect,color) DRAWKEY(screenOverlayStrings+(off),rect,color)
#define RECT(a,b,c,d) Rect4 rect((a),(b),(c),(d))
extern "C" {

#pragma opt_common_subs off
#pragma opt_strength_reduction off
// Draw active HUD panels, party-item effects, shot feedback, tutorial labels, and command glyphs.
extern "C" void CScreen_OnDisplay(register HudDisplayState* r3) {
    register HudDisplayState* r29 = r3;
    register HudDisplayState* self = r29;
    volatile double pad[32];
    volatile char pad2[0x60];
    (void)pad; (void)pad2;
    // Phase 1: require ready/visible HUD state, then establish rendering state.
    if (0 == self->hudInitialized || !AsCScreen(self)->hudDisplayEnabled ||
        (*(void**)&AsCScreen(self)->hudEnabled) == 0)
        return;
    if (!CScreen_PollLoadComplete(self))
        return;
    if (g_drawDriveLine)
        CDrawDriveLine_DrawRibbon((HudDisplayState*)g_drawDriveLine);
    Engine_ClearFramebuffer(0, 0, 2, 0, 0, kFloat_1_AuxMenu);
    HudDisplayState* state = (*(HudDisplayState**)&AsCSharedDoc(g_sharedDoc)->golfDoc);

    if (((*(unsigned int*)&AsCGolfDoc(state)->gameStateFlags)) & HudSuppressDrawingState) {
        (*(int*)&AsCScreen(self)->hudDisplayMask) = 1;
        if ((*(int*)&AsCSharedDoc(g_sharedDoc)->partyGameMode) != 3)
            return;
        BpObject_SetEnabled(AsCScreen(self)->partyGolfBaseScript, 0);
        BpObject_SetEnabled(AsCScreen(self)->timebombScript, 0);
        BpObject_SetEnabled(AsCScreen(self)->pululuJarashiScript, 0);
        BpObject_SetEnabled(AsCScreen(self)->featherBadgeScript, 0);
        *(int*)((*(char**)&AsCScreen(self)->textTable) + 0x28) = 0;
        return;
    }
    UIRender_SetDefaultState();
    void* global=g_sharedDoc;
    int active = 0;
    if (AsCGolfDoc((void*)AsCSharedDoc(global)->golfDoc)->gameStateFlags & HudActiveLayoutState) active = 1;

    // Phase 2: draw the widgets selected by HUD phase, tutorial and party mode.
    switch (((*(int*)&AsCScreen(self)->hudPhase))) {
    case HudInactive:
        if ((*(int*)&AsCSharedDoc(global)->partyGameMode) == 3)
            BpObject_SetEnabled(AsCScreen(self)->partyGolfBaseScript, 0);
        if (AsCScreen(self)->holeInfoSet)
            CHoleInfoSet_Draw(AsCScreen(self)->holeInfoSet);
        return;
    case HudShot:
        CScreen_UpdatePanelTransitions(self); CScreen_UpdateCameraAndShotHints(self); CScreen_UpdateClubChangeDisplay(self); CScreen_UpdateSwingCaption(self);
        break;
    case HudAim:
        CScreen_UpdatePanelTransitions(self); CScreen_UpdateClubChangeDisplay(self); CScreen_UpdateCameraAndShotHints(self);
        break;
    case HudItems:
        CScreen_UpdatePanelTransitions(self); CScreen_UpdateClubChangeDisplay(self);
        break;
    case HudSpin: { float x; float y;
        CScreen_UpdatePanelTransitions(self);
        x = kFloat_0_AuxMenu; y = kFloat_0_AuxMenu;
        IActor_DispatchMessage(self, self, screenOverlayStrings + ScreenText_SpinActor, 8,(void*)(&x),(void*)(&y),(void*)(0),(void*)(0));
        ((ScreenCallFF<0xE0>*)AsCScreen(self)->strokePointDisplay)->slot0x3C(x,y);
        CScreen_UpdateClubChangeDisplay(self);
        break;
    }
    case HudShotTransition:
        CScreen_UpdatePanelTransitions(self); CScreen_UpdateClubChangeDisplay(self);
        break;
    default:
        break;
    }

    global = g_sharedDoc;
    if (AsCGolfDoc(STATE)->tutorialStage < TutorialStageCount) {
        if ((*(int*)&AsCScreen(self)->tutorialGaugeEnabled) == 0) active = 0;
    } else if (SharedDoc_IsPlayerFlag0Set(PLAYER)) {
        BpObject_SetEnabled(AsCScreen(self)->ballWindScript, 0);
        active = 0;
    } else {
        BpObject_SetEnabled(AsCScreen(self)->ballWindScript, 1);
    }
    global=g_sharedDoc;
    if ((*(int*)&AsCSharedDoc(global)->partyGameMode) == 3) {
        unsigned int index = PartyItemSlots_GetItemType(PartyItemSlots_GetInstance(), FIELD(unsigned char,(*(void**)&AsCSharedDoc(global)->golfDoc),0x3c1));
        void* ui = InstanceTable_Get();

        if ((*(unsigned char*)((char*)*(void**)((char*)ui + 4) + 10)) == 3) BpObject_SetEnabled(AsCScreen(self)->partyGolfBaseScript, 0);
        else if (2 >= index) BpObject_SetEnabled(AsCScreen(self)->partyGolfBaseScript, 1);
        BpObject_TryInvokeCurrentCallback(AsCScreen(self)->partyGolfBaseScript, 0);
    }
    void* obj = AsCScreen(self)->golfBaseDisplay;
    ((ScreenCallP<0x164>*)AsCScreen(self)->golfBaseDisplay)->slot0x18(AsCScreen(self)->partyGolfBaseScript);
    if (((*(unsigned int*)&AsCScreen(self)->hudFlags) & HudShotGaugeVisible)) { if (!SharedDoc_IsPlayerFlag0Set(PLAYER)) { if (AsCScreen(self)->powerInfo) {
        if (((ScreenCall0<0x164>*)AsCScreen(self)->golfBaseDisplay)->slot0x24()) {
            unsigned int color = -1;
            if (AsCGolfDoc(STATE)->cameraMode) color = 0x7fffffff;

            WVector pos;
            IActor_DispatchMessage(self,self,screenOverlayStrings + ScreenText_SpotActor,2,(void*)(&pos),(void*)(0),(void*)(0),(void*)(0));
            ((PowerInfoView*)AsCScreen(self)->powerInfo)->color = color;
            ((PowerInfoView*)AsCScreen(self)->powerInfo)->position = pos;
            CALL(AsCScreen(self)->powerInfo,0,0x10);
        }
    } } }
    if (active) Layout_DrawWithOpacity(AsCScreen(self)->layout,kFloat_1_AuxMenu);
    float height = g_option->hudBaselineY;
    if (((ScreenCall0<0xA4>*)AsCScreen(self)->ballWindDisplay)->slot0x30() && !SharedDoc_IsPlayerFlag0Set(PLAYER)) CScreen_DrawClubModelOverlay(self);
    if (((*(unsigned int*)&AsCScreen(self)->hudFlags)&1)) { if (AsCGolfDoc(STATE)->tutorialStage>=2) { if (((ScreenExtraState*)self)->tutorialStageVisible[2]) { if (AsCScreen(self)->calipers) {
        void* manager = AsCSharedDoc(g_sharedDoc)->itemInventory;
        void* data = GolfPlayer_GetEntryByIndex(STATE,PLAYER);
        if (FIELD(unsigned int,data,0xa0)==ItemInventory_GetItemIdAt(manager,5) || AsCGolfDoc(STATE)->powerShotMeterOverride) CALL(AsCScreen(self)->calipers,0,0x10);
    } } } }
    BpObject_TryInvokeCurrentCallback(AsCScreen(self)->ballWindScript, 0);
    obj = AsCScreen(self)->ballWindDisplay;
    ((ScreenCallP<0xA4>*)AsCScreen(self)->ballWindDisplay)->slot0x18(AsCScreen(self)->ballWindScript);
    if (!SharedDoc_IsPlayerFlag0Set(PLAYER) && AsCScreen(self)->ballPreview) ScreenBallPreview_Draw(AsCScreen(self)->ballPreview);
    if (!SharedDoc_IsPlayerFlag0Set(PLAYER)) { if (((ClubSelectionState*)InstanceTable_Get())->selection->clubKind!=3) { if (!SharedDoc_IsPlayerFlag0Set(PLAYER)) { if (AsCGolfDoc(STATE)->tutorialStage==TutorialStageCount) { float x;  int color;
         float y;
        IActor_DispatchMessage(self,self,screenOverlayStrings + ScreenText_SpinActor,7,(void*)(&x),(void*)(&y),(void*)(&color),(void*)(0));
        const char* key=screenOverlayStrings + ScreenText_SpinPointKey;
        int drawColor=color;
        RECT(x-kScreenMarkerInsetX,y-kScreenSpinMarkerInsetY,kFloat_32_screen,kFloat_32_screen);
        if (drawColor>>24) DRAWKEY(key,rect,drawColor);
    } } } }
    if ((*(int*)&AsCSharedDoc(g_sharedDoc)->partyGameMode)==3) {
        unsigned int manager = (unsigned int)PartyItemSlots_GetInstance();
        if (PartyItemSlots_GetItemType(PartyItemSlots_GetInstance(),PLAYER)==9) {
            unsigned int update[2]={kScreenFeatherCallbackFloat2Bits,kScreenFeatherCallbackFloat0Bits};
            BpObject_TryInvokeCurrentCallback(AsCScreen(self)->featherBadgeScript,(int)update);
            BpObject_Draw(AsCScreen(self)->featherBadgeScript);
        } else BpObject_SetEnabled(AsCScreen(self)->featherBadgeScript,0);
        unsigned int found=0;
        for(int i=0;i<FIELD(unsigned char,(state=(HudDisplayState*)STATE),0x2b9);++i) {
            if(i!=FIELD(unsigned char,state,0x3c1)) { if (PartyItemSlots_GetItemType(PartyItemSlots_GetInstance(),i)==6) {found=1;break;} }
        }
        if(found) {
            EnsureHudInput();
            int value=FIELD(int,(void*)manager,0x64);
            switch(((ScreenExtraState*)self)->pululuAnimationPhase) {
            case PululuHidden:
                if(value) {BpObject_SetEnabled(AsCScreen(self)->pululuJarashiScript,1);BpObject_StartScriptByName(AsCScreen(self)->pululuJarashiScript,screenOverlayStrings + ScreenText_PululuEnterAnimation,1);((ScreenExtraState*)self)->pululuAnimationPhase=PululuEntering;} break;
            case PululuEntering:
                if(!value) {BpObject_StartScriptByName(AsCScreen(self)->pululuJarashiScript,screenOverlayStrings + ScreenText_PululuExitAnimation,1);((ScreenExtraState*)self)->pululuAnimationPhase=PululuLeaving;}
                else if(BpObject_IsCurrentScriptComplete(AsCScreen(self)->pululuJarashiScript)) {BpObject_StartScriptByName(AsCScreen(self)->pululuJarashiScript,screenOverlayStrings + ScreenText_PululuLoopAnimation,1);((ScreenExtraState*)self)->pululuAnimationPhase=PululuLooping;} break;
            case PululuLooping:
                if(0 == value) {BpObject_StartScriptByName(AsCScreen(self)->pululuJarashiScript,screenOverlayStrings + ScreenText_PululuExitAnimation,1);((ScreenExtraState*)self)->pululuAnimationPhase=PululuLeaving;} break;
            case PululuLeaving:
                if(BpObject_IsCurrentScriptComplete(AsCScreen(self)->pululuJarashiScript)) {BpObject_SetEnabled(AsCScreen(self)->pululuJarashiScript,0);((ScreenExtraState*)self)->pululuAnimationPhase=PululuHidden;} break;
            }
            BpObject_TryInvokeCurrentCallback(AsCScreen(self)->pululuJarashiScript,0);BpObject_Draw(AsCScreen(self)->pululuJarashiScript);
        } else {BpObject_SetEnabled(AsCScreen(self)->pululuJarashiScript,0);((ScreenExtraState*)self)->pululuAnimationPhase=0;}
        found=0;
        for(int j=0;j<AsCGolfDoc(STATE)->playerCount;++j) {
            if(PartyItemSlots_GetItemType(PartyItemSlots_GetInstance(),j)==10) {
                if(j==PLAYER) {found=0;break;}
                found=1;
            }
        }
        if(found) {
            FIELD(int,AsCScreen(self)->textTable,0x28)=1;
            int remaining=FIELD(int,(void*)manager,0x4c)-FIELD(int,(void*)manager,0x48)/TimebombFramesPerTick;
            if(remaining<0) remaining=0;
            void* pane=BpObject_GetOrCreateNamedGroup(AsCScreen(self)->timebombScript,screenOverlayStrings + ScreenText_CountdownNumberGroup);
            if(remaining>14) UIGroup_SetTintColor(pane,0xffff9632);
            else UIGroup_SetTintColor(pane,0xff5050ff);
            UIGroup_SetOpacity(pane,128);
            UITextTable_SetInteger(AsCScreen(self)->textTable,0,remaining);UITextTable_Draw(AsCScreen(self)->textTable);
        } else FIELD(int,AsCScreen(self)->textTable,0x28)=0;
    }
    if(((ScreenExtraState*)self)->tutorialStageVisible[1]) { if (!SharedDoc_IsPlayerFlag0Set(PLAYER)) { if (((CGolfBall*)GolfBall__Fv())->m_eventPreDataIndex[0]<0) {
        unsigned int color;
        switch(((CGolfBall*)GolfBall__Fv())->m_groundType) {
        case 0: case 1: case 2: case 6: case 7: color=0;break;
        case 4: color=0x8078e577;break;
        case 3: case 10: color=0x80e5ca77;break;
        case 5: color=0x80ffffff;break;
        case 12: color=0x808080ff;break;
        case 8: case 9: case 11:
default: color=0x8078e577;break;
        }
        if(color) {
            void* entry=Lookup(self,screenOverlayStrings + ScreenText_BallConditionKey);
            if(entry) { if (((NamedOverlayView*)entry)->resource) { Rect4 uv; Rect4 rect;
                float y = height-kScreenBallConditionBaselineInset;
                float t = ((ScreenExtraState*)self)->groundLieBlend;
                float remain = kFloat_1_AuxMenu-t;
                y = kFloat_64_Screen*t+y;
                rect.w=kFloat_64_Screen;
                rect.x=kScreenBallConditionX;
                rect.h=kFloat_64_Screen*remain;
                rect.y=y-kFloat_10_Screen;
                uv.x=kFloat_0_AuxMenu; uv.y=((ScreenExtraState*)self)->groundLieBlend; uv.w=kFloat_1_AuxMenu; uv.h=kFloat_1_AuxMenu-uv.y;
                WOverlay_DrawTransformedRect(((NamedOverlayView*)entry)->resource,g_view,uv,kFloat_0_AuxMenu,rect,0,color,0);
            } }
        }
    } } }
    // Phase 3: draw shot/impact results and optional status, distance and reward overlays.
    Layout_DrawWithOpacity(AsCScreen(self)->swingResultDisplay, kFloat_1_AuxMenu);
    CScreen_DrawImpactTimingSprite(self);
    if (AsCScreen(self)->flyInfo) CALL(AsCScreen(self)->flyInfo,0,0x10);
    if (AsCScreen(self)->modeInfo) CALL(AsCScreen(self)->modeInfo,0,0x10);
    if (!AsCGolfDoc(STATE)->cameraMode && AsCScreen(self)->pangInfo) CALL(AsCScreen(self)->pangInfo,0,0x10);
    GolfTips_ResetActiveDialogForRoundState();
    if (AsCScreen(self)->holeInfoSet) CHoleInfoSet_Draw(AsCScreen(self)->holeInfoSet);
    if(((ScreenExtraState*)self)->cameraCaptionOpacity>kFloat_0_AuxMenu) {
        switch(AsCGolfDoc(STATE)->gameStateFlags) {
        case 0x200: {
            int alpha=(int)((ScreenExtraState*)self)->cameraCaptionOpacity;
            const char* key=screenOverlayStrings + ScreenText_TopCameraKey;
            RECT(kFloat_0_AuxMenu,kFloat_55_Screen,kFloat_256_GolfScreen,kFloat_32_screen);
            if((unsigned char)alpha) DRAWKEY(key,rect,((unsigned int)alpha<<24)|0xffffff);
            break;
        }
        case 0x40000: {
            unsigned int alpha=(int)((ScreenExtraState*)self)->cameraCaptionOpacity;
            const char* key=screenOverlayStrings + ScreenText_TargetCameraKey;
            RECT(kFloat_0_AuxMenu,kFloat_55_Screen,kFloat_256_GolfScreen,kFloat_32_screen);
            if((unsigned char)alpha) DRAWKEY(key,rect,((unsigned int)alpha<<24)|0xffffff);
            break;
        }
        case 0x100: {
            unsigned int alpha=(int)((ScreenExtraState*)self)->cameraCaptionOpacity;
            const char* key=screenOverlayStrings + ScreenText_FreeCameraKey;
            RECT(kFloat_0_AuxMenu,kFloat_55_Screen,kFloat_256_GolfScreen,kFloat_32_screen);
            if((unsigned char)alpha) DRAWKEY(key,rect,((unsigned int)alpha<<24)|0xffffff);
            break;
        }
        }
        if(((*(unsigned int*)&AsCGolfDoc(STATE)->gameStateFlags)&8)) { if (AsCGolfDoc(STATE)->cameraMode) {
            unsigned int alpha=(int)((ScreenExtraState*)self)->cameraCaptionOpacity;
            const char* key=screenOverlayStrings + ScreenText_FreeCameraKey;
            RECT(kFloat_0_AuxMenu,kFloat_55_Screen,kFloat_256_GolfScreen,kFloat_32_screen);
            if((unsigned char)alpha) DRAWKEY(key,rect,((unsigned int)alpha<<24)|0xffffff);
        } }
    }
    if(AsCScreen(self)->ingameStatus && ((ScreenExtraState*)self)->tutorialStageVisible[5]) CIngameStatus_UpdateAndDraw(AsCScreen(self)->ingameStatus);
    if(((ClubSelectionState*)InstanceTable_Get())->selection->clubKind!=3) { if (AsCGolfDoc(STATE)->tutorialStage==TutorialStageCount) { float x=g_view->viewportWidth;

        void* camera=CWind_GetInstance();
        CWind_DrawDirectionModel(camera,AsCScreen(self)->windIndicatorResource,x-kFloat_50_Screen,kFloat_5_Screen+height);
        if(AsCScreen(self)->windInfo) CALL(AsCScreen(self)->windInfo,0,0x10);
    } }
    // Phase 4: compose party items and directional command glyphs after the main HUD.
    PartyItemSlots_Draw((int)PartyItemSlots_GetInstance(),AsCGolfDoc(STATE)->gameStateFlags);
    if(((ScreenCall0<0x9C>*)AsCScreen(self)->mapCameraDisplay)->slot0x1C()) ((ScreenCall0<0x9C>*)AsCScreen(self)->mapCameraDisplay)->slot0x18();
    if(((ScreenCall0<0xE0>*)AsCScreen(self)->strokePointDisplay)->slot0x34()) {
        ((ScreenCallP<0xE0>*)AsCScreen(self)->strokePointDisplay)->slot0x2C(AsCScreen(self)->ballPreview);
        EnsureHudInput();InputManager_RequestPointerInput(Sing_805DC5C8(),1);
    }
    if(AsCGolfDoc(STATE)->tutorialStage<TutorialStageCount && AsCScreen(self)->tutorialIllustration) ((ScreenCall0<0x78>*)AsCScreen(self)->tutorialIllustration)->slot0x14();
    float baseH=kFloat_64_Screen; float baseX=kScreenShotOverlayX; float baseY=height-baseH; float baseW=kFloat_128_ShotGauge;
    volatile float unusedRect[4]={baseX,baseY,baseW,baseH};
    volatile float copyX=baseX;
    volatile float copyY=baseY;
    volatile float copyW=baseW;
    volatile float copyH=baseH;
    if(((ScreenExtraState*)self)->tutorialStageVisible[1]) {
        RECT(kScreenPatternX,height-kScreenPatternBaselineInset,kFloat_128_ShotGauge,kFloat_128_ShotGauge);
        DRAW(ScreenText_PatternKey,rect,0xffffffff);
    }
    if(((*(unsigned int*)&AsCScreen(self)->hudFlags)&1)) { if (AsCGolfDoc(STATE)->tutorialStage>=2) { if (((ScreenExtraState*)self)->tutorialStageVisible[2]) { if (!SharedDoc_IsPlayerFlag0Set(PLAYER)) {
        if((((ScreenExtraState*)self)->tutorialGaugeBlinkCount>=TutorialBlinkLimit && FIELD(int,(state=(HudDisplayState*)FIELD(void*,(global=g_sharedDoc),0xb90)),0x98)<6) || FIELD(int,(state=(HudDisplayState*)FIELD(void*,(global=g_sharedDoc),0xb90)),0x98)==6) {
            if(FIELD(int,(char*)global+FIELD(unsigned char,state,0x3c1)*0x1d4,0x1dc)) {
                if(((ClubSelectionState*)InstanceTable_Get())->selection->clubKind!=3) {
                    void* gameState=STATE;
                    unsigned char kind=FIELD(unsigned char,gameState,0x3c0);
                    if(kind == 0 && (*(int*)&AsCScreen(self)->hudPhase)==HudShot) {
                        RECT(kScreenClickIndicatorX,kFloat_2_805DED94+(height-kFloat_51_ShotGauge),kFloat_32_screen,kFloat_32_screen);
                        DRAW(ScreenText_ClickIndicatorKey,rect,0xffffffff);
                    } else if(kind==1) {
                        RECT(kFloat_8_GolfScreen+(FIELD(float,gameState,0x2d0)-kScreenGaugeClickInsetX),kFloat_2_805DED94+(height-kFloat_51_ShotGauge),kFloat_32_screen,kFloat_32_screen);
                        DRAW(ScreenText_ClickIndicatorKey,rect,0xffffffff);
                    } else if(2==kind) {
                        float delta=((ScreenExtraState*)self)->shotIndicatorPulse*kFloat_0_5_AuxMenu;
                        delta = delta>kFloat_0_AuxMenu ? delta : -delta;
                        RECT(kScreenImpactClickX,(kScreenImpactClickOffsetY+(height-kScreenImpactClickBaselineInset))+delta,kFloat_32_screen,kFloat_32_screen);
                        DRAW(ScreenText_ClickIndicatorKey,rect,0xffffffff);
                    }
                } else if((*(int*)&AsCScreen(self)->hudPhase)==HudShot) {
                    RECT(kFloat_8_GolfScreen+(AsCGolfDoc(STATE)->roundedShotGaugeCursorX-kScreenGaugeClickInsetX),(height-kFloat_51_ShotGauge)-kFloat_8_GolfScreen,kFloat_32_screen,kFloat_32_screen);
                    DRAW(ScreenText_ClickIndicatorKey,rect,0xffffffff);
                }
            }
        }
    } } } }
    if(!((*(unsigned int*)&AsCGolfDoc(STATE)->gameStateFlags)&16) && AsCGolfDoc(STATE)->tutorialStage==TutorialStageCount)
        IActor_DispatchMessage(self,self,screenOverlayStrings + ScreenText_ItemActor,4,(void*)(0),(void*)(0),(void*)(0),(void*)(0));
    if(AsCGolfDoc(STATE)->tutorialStage<TutorialStageCount) {
        if(AsCScreen(self)->modeLabelRenderer) { if (AsCSharedDoc(g_sharedDoc)->gameMode!=4) {
            FIELD(unsigned int,AsCScreen(self)->modeLabelRenderer,0xc)=((unsigned int)AsCScreen(self)->modeLabelOpacity<<24)|0xffffff;
            const char* text=String_FormatTemporary(screenOverlayStrings + ScreenText_TutorialTitleFormat,AsCGolfDoc(STATE)->tutorialStage+1);
            NamedText_DrawAlignedScaled(AsCScreen(self)->modeLabelRenderer,text,0,FIELD(float,self,0x1c4),FIELD(float,self,0x1c8),kFloat_1_AuxMenu);
        } }
    } else if(AsCSharedDoc(g_sharedDoc)->gameMode==5) {
        RECT(kFloat_0_AuxMenu,kScreenPracticeOptionsY,kFloat_32_screen,kFloat_32_screen);
        DRAW(ScreenText_PracticeOptionsKey,rect,0xffffffff);
    }
    if((*(unsigned int*)&AsCGolfDoc(STATE)->gameStateFlags)&HudCommandGlyphStates) { float x=kScreenCommandGlyphStartX;  HudString text;    float spacing=kScreenCommandAndMenuRowSpacing;

        float yoffset=kScreenPulseWrapSpan;

        void* node=AsCGolfDoc(STATE)->shotCommandTail;
        while(node!=&AsCGolfDoc(STATE)->shotCommandHead) {
            int kind=FIELD(unsigned int,node,8);
            if(16u==(unsigned int)kind) {node=FIELD(void*,node,4);continue;}
            switch(kind) {
            case 1: text.append(screenOverlayStrings + ScreenText_UpGlyph);break;
            case 2: text.append(screenOverlayStrings + ScreenText_DownGlyph);break;
            case 4: text.append(screenOverlayStrings + ScreenText_LeftGlyph);break;
            case 8: text.append(screenOverlayStrings + ScreenText_RightGlyph);break;
            case 64: text.append(screenOverlayStrings + ScreenText_InvalidGlyph);break;
            case 32: text.append(screenOverlayStrings + ScreenText_InvalidGlyph);
            float savedSpacing = spacing;
            float savedX = x;
            x = savedX - savedSpacing;
            break;
}
            node=FIELD(void*,node,4);
            if(node!=&AsCGolfDoc(STATE)->shotCommandHead && FIELD(unsigned int,node,8)==16) text.assign(screenOverlayStrings + ScreenText_SmallGlyphSuffix);
            if(AsCScreen(self)->statusTextRenderer) NamedText_DrawAlignedScaled(AsCScreen(self)->statusTextRenderer,text.c_str(),0,x,height-yoffset,kFloat_1_AuxMenu);
                                        float savedSpacing2 = spacing;
                                        float savedValue = x + savedSpacing2;
                                        x = savedValue;
        }
    }
    if(AsCScreen(self)->accuracyInfo && !SharedDoc_IsPlayerFlag0Set(PLAYER)) CALL(AsCScreen(self)->accuracyInfo,0,0x10);
    if(AsCScreen(self)->skinsInfo) CALL(AsCScreen(self)->skinsInfo,0,0x10);
    if(AsCScreen(self)->nearestPinInfo) CALL(AsCScreen(self)->nearestPinInfo,0,0x10);
    if ((*(float*)&AsCScreen(self)->displayTimeRemaining) > kFloat_0_AuxMenu)
        GridSurface_DiagnosticStub(g_view, screenOverlayStrings + ScreenText_UpdatedText, kScreenUpdatedTextX, kFloat_100_Screen);
}

#pragma opt_common_subs reset
#pragma opt_strength_reduction reset
} // extern "C"

#undef FIELD
#undef STATE
#undef PLAYER
#undef CALL
#undef QUERY
#undef DRAW
#undef DRAWKEY
#undef RECT

extern "C" {

struct ScreenMutex { char data[4]; };
extern void* WOverlay_Create(void*, void*, void*);

extern void* appMallocUni__FUl(unsigned int);

// A ring link and a separate bucket-chain link share each 24-byte registry node.
struct RingNode { void* value; void* key; unsigned char flag; char pad1[3]; RingNode* next; RingNode* prev; unsigned int bucketNextAddress; };
struct Entry { char pad[0x64]; RingNode* secondary; char pad68[4]; RingNode* primary; RingNode* free; int capacity; unsigned int mask; char pad7C[4]; unsigned int* table; ScreenMutex lock; };

#pragma use_lmw_stmw on
struct StringEntry {
    char text[32];
    void* manager;
    // Initialize an overlay-name entry without an owned resource yet.
    StringEntry() { manager = 0; }
    // Release the overlay resource through the shared manager if both remain valid.
    ~StringEntry() {
        if (g_resourceManager && manager) {
            Object_DeleteVirtual(g_resourceManager, manager);
            manager = 0;
        }
    }
};

// Create an overlay and insert its name into the locked registry rings and hash buckets.
void ScreenOverlayRegistry_CreateNamedOverlay(void* object, const char* key, void* arg, void* arg2) {
    Entry* self = (Entry*)object;
    union { StringEntry* value; unsigned int index; RingNode* tail; } scratch;
    StringEntry*& value = scratch.value;
    // Phase 1: create an owned name/resource pair; failed resource creation is cleaned up.
    value = new StringEntry;
    value->manager = WOverlay_Create(g_resourceManager, arg, arg2);
    if (value->manager) {
        strcpy(value->text, key);
        RingNode* n;
        // Phase 2: allocate/reuse a registry node and splice its ring/hash links while locked.
        InstanceLock_u80179180 outer(&self->lock);
        {
            InstanceLock_u80179180 inner(&self->lock);
            if (!self->free) {
                RingNode* node = (RingNode*)appMallocUni__FUl(self->capacity * 0x18);
                void* mutex = &self->lock;
                RingNode* head = self->primary;
                OSLockMutex((OSMutex*)mutex);
                if (!head) {
                    node->next = node;
                    node->prev = node;
                    OSUnlockMutex((OSMutex*)mutex);
                    head = node;
                } else {
                    node->prev = head;
                    node->next = head->next;
                    head->next = node;
                    node->next->prev = node;
                    OSUnlockMutex((OSMutex*)mutex);
                }
                self->primary = head;
                int i = 1;
                if (self->capacity > 1) {
                    RingNode* loopHead;
                    void* mutex = &self->lock;
                    ++node;
                    for (; i < self->capacity; ++node, ++i) {
                        loopHead = self->free;
                        OSLockMutex((OSMutex*)mutex);
                        if (!loopHead) {
                            node->next = node;
                            node->prev = node;
                            OSUnlockMutex((OSMutex*)mutex);
                            loopHead = node;
                        } else {
                            node->prev = loopHead;
                            node->next = loopHead->next;
                            loopHead->next = node;
                            node->next->prev = node;
                            OSUnlockMutex((OSMutex*)mutex);
                        }
                        self->free = loopHead;
                    }
                }
            }
            void* mutex = &self->lock;
            n = self->free;
            OSLockMutex((OSMutex*)mutex);
            n->prev->next = n->next;
            n->next->prev = n->prev;
            RingNode* next = n->prev;
            if (next == n) next = 0;
            OSUnlockMutex((OSMutex*)mutex);
            self->free = next;
        }
        n->value = value;
        n->flag = 0;
        n->key = value;
        if (value) {
            void* mutex = &self->lock;
            OSLockMutex((OSMutex*)mutex);
            int hash;
            int last;
            int len = strlen(value->text);
            int first = (unsigned char)value->text[0] & 0x2A;
            last = len > 0 ? len - 1 : 0;
            hash = first | ((unsigned char)value->text[last] & 0x15);
            hash = (hash & 0x3F) | (len << 6);
            scratch.index = hash & self->mask;
            OSUnlockMutex((OSMutex*)mutex);
            OSLockMutex((OSMutex*)mutex);
            n->bucketNextAddress = self->table[scratch.index];
            self->table[scratch.index] = (unsigned int)n;
            OSUnlockMutex((OSMutex*)mutex);
        }
        {
            void* mutex = &self->lock;
            scratch.tail = self->secondary;
            OSLockMutex((OSMutex*)mutex);
            RingNode* tail = scratch.tail;
            if (!tail) {
                n->next = n; n->prev = n;
                OSUnlockMutex((OSMutex*)mutex);
            } else {
                n->prev = tail; n->next = tail->next;
                tail->next = n; n->next->prev = n;
                OSUnlockMutex((OSMutex*)mutex);
                n = tail;
            }
            self->secondary = n;
        }
    } else {
        if (value) delete value;
    }
}
#pragma use_lmw_stmw reset

struct HudLoadState {
    char pad0[0x2C8];
    void* ballWindScript;
    void* partyGolfBaseScript;
    void* timebombScript;
    void* pululuJarashiScript;
    void* featherBadgeScript;
    char pad2DC[0x50];
    void* layout;
    void* strokePointDisplay;
    void* mapCameraDisplay;
    void* ballWindDisplay;
    void* golfBaseDisplay;
    void* swingResultDisplay;
    void* tutorialIllustration;
    void* textTable;
    void* n;
};

extern int BpObject_PollLoad(void*);
extern int Layout_PollLoadComplete(void*);

// Return 1 only when every required script/layout has loaded; optional tutorial is checked if present.
int CScreen_PollLoadComplete(void* object) {
    HudLoadState* self = (HudLoadState*)object;
    if (!BpObject_PollLoad(self->ballWindScript)) return 0;
    if ((*(int*)&g_sharedDoc->partyGameMode) == 3) {
        if (!BpObject_PollLoad(self->partyGolfBaseScript)) return 0;
        if (!BpObject_PollLoad(self->timebombScript)) return 0;
        if (!BpObject_PollLoad(self->pululuJarashiScript)) return 0;
        if (!BpObject_PollLoad(self->featherBadgeScript)) return 0;
    }
    if (!Layout_PollLoadComplete(self->layout)) return 0;
    if (!Layout_PollLoadComplete(self->strokePointDisplay)) return 0;
    if (!Layout_PollLoadComplete(self->mapCameraDisplay)) return 0;
    if (!Layout_PollLoadComplete(self->ballWindDisplay)) return 0;
    if (!Layout_PollLoadComplete(self->golfBaseDisplay)) return 0;
    if (!Layout_PollLoadComplete(self->swingResultDisplay)) return 0;
    if (self->tutorialIllustration && !Layout_PollLoadComplete(self->tutorialIllustration)) return 0;
    return 1;
}

struct ObjE0 { char pad[0xE0]; void** vtable; };
struct Obj9C { char pad[0x9C]; void** vtable; };
struct Obj164 { char pad[0x164]; void** vtable; };

struct VE0B { char pad[224]; };
class VE0 : public VE0B { public: 
virtual void v08(); virtual void v0C(); virtual void v10(); virtual void v14(); virtual void v18(); virtual void v1C(); virtual void v20(); virtual void v24(); virtual void v28(); virtual void v2C(); virtual void v30(); virtual void v34(); virtual void v38(); virtual void v3C(); virtual void v40(); virtual void v44(); virtual void v48(); virtual void v4C(); virtual void v50(); virtual void v54(); virtual void v58(); virtual void v5C(); virtual void v60(); virtual void v64(); virtual void v68(); virtual int v6C();
};
struct V9CB { char pad[156]; };
class V9C : public V9CB { public: 
virtual void v08(); virtual void v0C(); virtual void v10(); virtual void v14(); virtual void v18(); virtual void v1C(); virtual void v20(); virtual void v24(); virtual void v28(); virtual void v2C(); virtual void v30(); virtual void v34(); virtual void v38(); virtual void v3C(); virtual void v40(); virtual void v44(); virtual void v48(); virtual void v4C(); virtual int v50();
};
struct V164B { char pad[356]; };
class V164 : public V164B { public: 
virtual void v08(); virtual void v0C(); virtual void v10(); virtual void v14(); virtual void v18(); virtual void v1C(); virtual void v20(); virtual void v24(); virtual void v28(); virtual void v2C(); virtual void v30(); virtual void v34(); virtual void v38(); virtual void v3C(); virtual void v40(); virtual void v44(); virtual void v48(); virtual void v4C(); virtual void v50(); virtual void v54(); virtual void v58(); virtual void v5C(); virtual void v60(); virtual void v64(); virtual void v68(); virtual void v6C(); virtual void v70(); virtual int v74();
};
// Require script completion and idle panel transitions before accepting HUD input.
int CScreen_AreHudTransitionsComplete(void* object) {
    HudLoadState* self = (HudLoadState*)object;
    if (!BpObject_IsCurrentScriptComplete(self->ballWindScript)) return 0;
    if ((*(int*)&g_sharedDoc->partyGameMode) == 3) {
        if (!BpObject_IsCurrentScriptComplete(self->partyGolfBaseScript)) return 0;
        if (!BpObject_IsCurrentScriptComplete(self->timebombScript)) return 0;
        if (!BpObject_IsCurrentScriptComplete(self->pululuJarashiScript)) return 0;
        if (!BpObject_IsCurrentScriptComplete(self->featherBadgeScript)) return 0;
    }
    if (!((VE0*)self->strokePointDisplay)->v6C()) return 0;
    if (!((V9C*)self->mapCameraDisplay)->v50()) return 0;
    int c = ((V164*)self->golfBaseDisplay)->v74();
    return c != 0;
}

struct HudLoadedLayoutState {
    char pad0[0x25C];
    int state;
    int panelOpenRequested;
    int itemPanelOpenRequested;
    char pad268[0x60];
    int ballWindScript;
    int partyGolfBaseScript;
    int timebombScript;
    int pululuJarashiScript;
    char pad2D8[0x2C];
    int hudDisplayMask;
    char pad308[0xC];
    int hudPhase;
    char pad318[0xC];
    int partyMeterIndex;
    int meterOpacity;
    int layout;
    int strokePointDisplay;
    int mapCameraDisplay;
    int ballWindDisplay;
    int golfBaseDisplay;
    int swingResultDisplay;
    int tutorialIllustration;
    int textTable;
};

struct ObjA4 { char pad[0xA4]; void** vtable; };
struct Obj78 { char pad[0x78]; void** vtable; };
struct Obj74 { char pad[0x74]; void** vtable; };
struct Obj10 { char pad[0x10]; void** vtable; };
struct ObjE0B { char pad[0xE0]; void** vtable; };
struct Obj9CB { char pad[0x9C]; void** vtable; };
struct Obj164B { char pad[0x164]; void** vtable; };
extern unsigned int partyMeterGroupNames[];
extern char s_clubOverlayGroupNames[];

extern char s_timebombCountdownTextEntries[];

extern void Layout_SetElementEnabled(void*, int, int);
extern int CFontNumDraw_RegisterEntries(void*, const char*, int, void*, const char*);
extern "C" int UIGroup_SetEnabled(void*, int);

#pragma use_lmw_stmw on
#pragma opt_propagation off
// Initialize loaded HUD layouts, meter opacity, party countdown text, and club group visibility.
void CScreen_InitializeLoadedLayouts(void* object) {
    char* item;
    HudLoadedLayoutState* self = (HudLoadedLayoutState*)object;
    self->hudDisplayMask = 1;
    self->panelOpenRequested = 0;
    self->itemPanelOpenRequested = 0;
    if (self->state != 0) {
        if ((*(int*)&AsCSharedDoc(g_sharedDoc)->partyGameMode) == 3) {
            struct Names { unsigned int value[3]; };
            Names names = *(Names*)partyMeterGroupNames;
            int index = self->partyMeterIndex;
            if (index >= 0 && (unsigned int)index < 3) {
                UIGroup_SetOpacity(BpObject_GetOrCreateNamedGroup((void*)self->partyGolfBaseScript, (const char*)names.value[index]), 0x7F);
            }
        }
        self->meterOpacity = 0xFF;
        ((ScreenCallI<0xA4>*)self->ballWindDisplay)->slot0x64(-1);
        ((ScreenCallI<0x164>*)self->golfBaseDisplay)->slot0x50(-1);
    }
    void* mgr = (*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc);
    if (AsCGolfDoc(mgr)->tutorialStage < TutorialStageCount) self->hudPhase = HudAim;
    for (int slot = 0; slot < 4; ++slot) Layout_SetElementEnabled((void*)self->layout, slot, 0);
    ((ScreenCall0<0xE0>*)self->strokePointDisplay)->slot0x10();
    ((ScreenCall0<0x9C>*)self->mapCameraDisplay)->slot0x10();
    ((ScreenCall0<0xA4>*)self->ballWindDisplay)->slot0x10();
    ((ScreenCall0<0x164>*)self->golfBaseDisplay)->slot0x10();
    ((ScreenCall0<0x74>*)self->swingResultDisplay)->slot0x10();
    if (self->tutorialIllustration) {
        ((ScreenCall0<0x78>*)self->tutorialIllustration)->slot0x10();
    }
    ((ScreenCallI<0x164>*)self->golfBaseDisplay)->slot0x34(1);
    ((ScreenCallI<0x164>*)self->golfBaseDisplay)->slot0x38(1);
    if ((*(int*)&AsCSharedDoc(g_sharedDoc)->partyGameMode) == 3) {
        CFontNumDraw_RegisterEntries((void*)self->textTable, s_timebombCountdownTextEntries, 1, (void*)self->timebombScript, screenOverlayStrings + ScreenText_CountdownFontResource);
        BpObject_StartScriptByName((void*)self->timebombScript, screenOverlayStrings + ScreenText_CountdownAnimation, 1);
        BpObject_SetEnabled((void*)self->pululuJarashiScript, 0);
    }
    struct Table { char bytes[34][16]; };
    Table name;
    for (unsigned int i = 0; (int)i < ClubOverlayCount; ++i) {
        name = *(Table*)s_clubOverlayGroupNames;
        if ((unsigned int)i <= 0x11) {
            item = name.bytes[i * 2];
            if (strcmp(item, screenOverlayStrings + ScreenText_EmptyText)) {
                UIGroup_SetEnabled(BpObject_GetOrCreateNamedGroup((void*)self->ballWindScript, item), 0);
            }
            item += 0x10;
            if (strcmp(item, screenOverlayStrings + ScreenText_EmptyText)) {
                UIGroup_SetEnabled(BpObject_GetOrCreateNamedGroup((void*)self->ballWindScript, item), 0);
            }
        }
    }
}
#pragma opt_propagation reset
#pragma use_lmw_stmw reset

// Vtables of the three sub-objects CScreen_UpdateShotMeterPresentation drives: the one at 0x338 (vtable pointer at +0xA4), the one at 0x33C
// (vtable pointer at +0x164) and the one at 0x340 (vtable pointer at +0x74).
// The classes are polymorphic: the vtable pointer follows the members, a vtable has an 8-byte header and 4-byte
// slots, so the slot at vtable offset 0x58 is the 21st virtual function. The x* functions only fill unused slots.
struct BallWindMeterInterface {
    char pad[0xA4];
    virtual void x0(); virtual void x1(); virtual void x2(); virtual void x3();
    virtual void x4(); virtual void x5(); virtual void x6(); virtual void x7(); virtual void x8();
    virtual void f2C(int);
    virtual void x10();
    virtual void f34(int);
    virtual void x12(); virtual void x13(); virtual void x14(); virtual void x15();
    virtual void x16(); virtual void x17(); virtual void x18(); virtual void x19();
    virtual void f58(int);
    virtual void f5C(int);
    virtual void f60(float);
    virtual void f64(int);
};
struct GolfBaseMeterInterface {
    char pad[0x164];
    virtual void x0(); virtual void x1(); virtual void x2(); virtual void x3();
    virtual void x4(); virtual void x5();
    virtual void f20(int);
    virtual void x7(); virtual void x8();
    virtual void f2C(int, int);
    virtual void f30(void*);
    virtual void x11(); virtual void x12(); virtual void x13(); virtual void x14();
    virtual void x15(); virtual void x16(); virtual void x17();
    virtual void f50(int);
    virtual void f54(float);
    virtual void f58(float);
    virtual void f5C();
    virtual void x22(); virtual void x23();
    virtual void f68(float);
};
struct SwingResultMeterInterface {
    char pad[0x74];
    virtual void x0(); virtual void x1(); virtual void x2(); virtual void x3();
    virtual void x4(); virtual void x5(); virtual void x6();
    virtual void f24();
    virtual int f28();
};

struct Elem {
    float f0; int i0; float f1; int i1; float f2; int i2; float f3; int i3;
    float f4; int i4; float f5; int i5; float f6; int i6;
};

struct UpdateData {
    char pad00[0xA0];
    int meterPresentation;
    char pad_a4[0xCC - 0xA4];
    int terrainMarkerIndex;
    int terrainSampleCount;
    char pad_d4[0xDC - 0xD4];
    Elem terrainProfile[3];
    char pad_184[0x188 - 0x184];
    unsigned int hudFlags;
    char pad_18c[0x25C - 0x18C];
    int hudInitialized;
    char pad_260[0x280 - 0x260];
    int tutorialMeterEnabled;
    int tutorialGaugeEnabled;
    char pad_288[0x2B0 - 0x288];
    int swingFeedbackPending;
    char pad_2b4[0x2CC - 0x2B4];
    int partyGolfBaseScript;
    char pad_2d0[0x2E0 - 0x2D0];
    int meterFadeFrame;
    int impactFeedbackStarted;
    char pad_2e8[0x2F0 - 0x2E8];
    int tutorialMeterBlinkFrame;
    int tutorialMeterBlinkCount;
    int tutorialGaugeBlinkFrame;
    int tutorialGaugeBlinkCount;
    int c300;
    int hudDisplayMask;
    char pad_308[0x324 - 0x308];
    int partyMeterIndex;
    int meterOpacity;
    char pad_32c[0x338 - 0x32C];
    BallWindMeterInterface* ballWindDisplay;
    GolfBaseMeterInterface* golfBaseDisplay;
    SwingResultMeterInterface* swingResultDisplay;
};

typedef CGolfDoc HudMeterGolfStateView;

struct NameTbl { const char* s[3]; };

struct GameP { char p[0xA]; unsigned char mode; };
struct Game { char p[4]; GameP* gp; };

#define MGR ((*(HudMeterGolfStateView**)&AsCSharedDoc(g_sharedDoc)->golfDoc))
#define GLB_AD8 ((*(int*)&AsCSharedDoc(g_sharedDoc)->partyGameMode))

// Presentation modes are separate from CScreen's cached display-mask values.
enum ShotGolfMeterDisplay { kGolfMeterHidden = 0, kGolfMeterPower = 1,
    kGolfMeterTerrain10 = 2, kGolfMeterTerrain15 = 3, kGolfMeterTerrain20 = 4 };
enum ShotHudMeterMask { kHudMeterInactive = 0, kHudMeterTransparent = 1,
    kHudMeterPower = 2, kHudMeterTerrain10 = 4, kHudMeterTerrain15 = 8, kHudMeterTerrain20 = 16 };

// Select the shot-meter presentation and update tutorial blinking, impact feedback, and meter fading.
#pragma push
#pragma always_inline on
// Select ordinary power or sampled terrain-marker display (CGolfBase2d slot 6).
static inline void SelectGolfMeterDisplay(GolfBaseMeterInterface* meter, int mode) { meter->f20(mode); }
// Set the selected terrain marker and sample count (CGolfBase2d slot 9).
static inline void SetGolfTerrainMarkerRange(GolfBaseMeterInterface* meter, int index, int count) { meter->f2C(index, count); }
// Copy 21 Y-offset/color pairs from the caller-owned style buffer (slot 10).
static inline void CopyGolfTerrainMarkerStyles(GolfBaseMeterInterface* meter, void* styles) { meter->f30(styles); }
// Set the packed RGBA tint of the golf meter (slot 18).
static inline void SetGolfMeterTint(GolfBaseMeterInterface* meter, int rgba) { meter->f50(rgba); }
// Show/hide the ball-selector controls (CBallWind2d slot 9).
static inline void ShowBallSelectorControls(BallWindMeterInterface* meter, int visible) { meter->f2C(visible); }
// Show/hide layout element 0; its artwork role is unresolved (slot 11).
static inline void ShowBallWindElement0(BallWindMeterInterface* meter, int visible) { meter->f34(visible); }
// Draw indicator element 3 separately from the clipped script (slot 20).
static inline void DrawBallWindIndicator3Separately(BallWindMeterInterface* meter, int enabled) { meter->f58(enabled); }
// Draw indicator element 1 separately from the clipped script (slot 21).
static inline void DrawBallWindIndicator1Separately(BallWindMeterInterface* meter, int enabled) { meter->f5C(enabled); }
// Set both ball/wind indicators' packed RGBA tint (slot 23).
static inline void SetBallWindIndicatorTint(BallWindMeterInterface* meter, int rgba) { meter->f64(rgba); }
#pragma pop

// Select meter widgets and update tutorial blinking, shot feedback, and tint.
void CScreen_UpdateShotMeterPresentation(UpdateData* hud) {
    // Phase 1: choose terrain/ordinary meter and initial opacity from camera state.
    int usesTerrainMeter = ((Game*)InstanceTable_Get())->gp->mode == 3; // putting uses the sampled terrain meter
    unsigned int alpha = 0xFF;
    if (MGR->cameraMode) alpha = 0x7F;

    DrawBallWindIndicator3Separately(hud->ballWindDisplay, 0);
    DrawBallWindIndicator1Separately(hud->ballWindDisplay, 0);
    ShowBallSelectorControls(hud->ballWindDisplay, 0);
    ShowBallWindElement0(hud->ballWindDisplay, 0);
    SelectGolfMeterDisplay(hud->golfBaseDisplay, kGolfMeterHidden);

    if (!(hud->hudFlags & HudShotGaugeVisible)) {
        if (hud->hudDisplayMask != kHudMeterTransparent) hud->hudDisplayMask = kHudMeterTransparent;
    } else {
        hud->hudDisplayMask = kHudMeterInactive;
    }

    if ((MGR->gameStateFlags & HudMeterFadeStates) || (hud->hudFlags & HudShotGaugeVisible) || MGR->shotPhase) {
        if (usesTerrainMeter) {
            switch (hud->terrainSampleCount) {
            case 10:
                if (hud->hudDisplayMask != kHudMeterTerrain10) {
                    SelectGolfMeterDisplay(hud->golfBaseDisplay, kGolfMeterTerrain10);
                    hud->hudDisplayMask = kHudMeterTerrain10;
                }
                break;
            case 15:
                if (hud->hudDisplayMask != kHudMeterTerrain15) {
                    SelectGolfMeterDisplay(hud->golfBaseDisplay, kGolfMeterTerrain15);
                    hud->hudDisplayMask = kHudMeterTerrain15;
                }
                break;
            case 20:
                if (hud->hudDisplayMask != kHudMeterTerrain20) {
                    SelectGolfMeterDisplay(hud->golfBaseDisplay, kGolfMeterTerrain20);
                    hud->hudDisplayMask = kHudMeterTerrain20;
                }
                break;
            }
            SetGolfTerrainMarkerRange(hud->golfBaseDisplay, hud->terrainMarkerIndex, hud->terrainSampleCount);
            Elem tmp[3];
            for (int i = 0; i < 3; i++) {
                tmp[i].f0 = hud->terrainProfile[i].f0; tmp[i].i0 = hud->terrainProfile[i].i0;
                tmp[i].f1 = hud->terrainProfile[i].f1; tmp[i].i1 = hud->terrainProfile[i].i1;
                tmp[i].f2 = hud->terrainProfile[i].f2; tmp[i].i2 = hud->terrainProfile[i].i2;
                tmp[i].f3 = hud->terrainProfile[i].f3; tmp[i].i3 = hud->terrainProfile[i].i3;
                tmp[i].f4 = hud->terrainProfile[i].f4; tmp[i].i4 = hud->terrainProfile[i].i4;
                tmp[i].f5 = hud->terrainProfile[i].f5; tmp[i].i5 = hud->terrainProfile[i].i5;
                tmp[i].f6 = hud->terrainProfile[i].f6; tmp[i].i6 = hud->terrainProfile[i].i6;
            }
            CopyGolfTerrainMarkerStyles(hud->golfBaseDisplay, tmp);
        } else if (hud->hudDisplayMask != kHudMeterPower) {
            SelectGolfMeterDisplay(hud->golfBaseDisplay, kGolfMeterPower);
            hud->hudDisplayMask = kHudMeterPower;
        }
    } else if (hud->hudDisplayMask != kHudMeterTransparent) {
        SelectGolfMeterDisplay(hud->golfBaseDisplay, kGolfMeterPower);
        hud->hudDisplayMask = kHudMeterTransparent;
    }

    if (SharedDoc_IsPlayerFlag0Set(MGR->currentPlayer)) {
        SelectGolfMeterDisplay(hud->golfBaseDisplay, kGolfMeterHidden);
        hud->hudDisplayMask = kHudMeterTransparent;
    }

    // Phase 2: run meter/gauge tutorial blink counters (updates, not seconds).
    if (hud->tutorialMeterEnabled && MGR->tutorialStage < 6) {
        if (hud->tutorialMeterBlinkCount < TutorialBlinkLimit) {
            if (hud->tutorialGaugeEnabled) SelectGolfMeterDisplay(hud->golfBaseDisplay, kGolfMeterPower);
            else SelectGolfMeterDisplay(hud->golfBaseDisplay, kGolfMeterHidden);
            if (hud->tutorialMeterBlinkFrame < 2) {
                if (hud->tutorialMeterBlinkCount % 2 != 0) {
                    if (hud->meterPresentation != 1) hud->meterPresentation = 1;
                    ShowBallSelectorControls(hud->ballWindDisplay, 1);
                }
                hud->tutorialMeterBlinkFrame = hud->tutorialMeterBlinkFrame + 1;
                return;
            }
            hud->tutorialMeterBlinkFrame = 0;
            hud->tutorialMeterBlinkCount = hud->tutorialMeterBlinkCount + 1;
            ShowBallSelectorControls(hud->ballWindDisplay, 1);
            return;
        } else if (!hud->tutorialGaugeEnabled) {
            ShowBallSelectorControls(hud->ballWindDisplay, 1);
        }
    }

    if (hud->tutorialGaugeEnabled && MGR->tutorialStage < 6) {
        if (hud->tutorialGaugeBlinkCount < TutorialBlinkLimit) {
            if (hud->tutorialGaugeBlinkFrame < 2) {
                if (hud->tutorialGaugeBlinkCount % 2 != 0) {
                    if (hud->meterPresentation != 2) hud->meterPresentation = 2;
                    ShowBallSelectorControls(hud->ballWindDisplay, 1);
                    SelectGolfMeterDisplay(hud->golfBaseDisplay, kGolfMeterPower);
                    void* receiver2429_fn_800ED6BC = (void*)InstanceTable_Get();
                    hud->golfBaseDisplay->f68(kFloat_2_805DED94 * ShotParameters_GetGoodImpactHalfWidth(receiver2429_fn_800ED6BC));
                    DrawBallWindIndicator3Separately(hud->ballWindDisplay, 1);
                    hud->ballWindDisplay->f60(MGR->roundedShotGaugeCursorX);
                    hud->golfBaseDisplay->f54(MGR->roundedShotGaugeCursorX);
                } else {
                    if (hud->meterPresentation != 1) hud->meterPresentation = 1;
                    ShowBallSelectorControls(hud->ballWindDisplay, 1);
                    SelectGolfMeterDisplay(hud->golfBaseDisplay, kGolfMeterHidden);
                }
                hud->tutorialGaugeBlinkFrame = hud->tutorialGaugeBlinkFrame + 1;
                return;
            }
            hud->tutorialGaugeBlinkFrame = 0;
            hud->tutorialGaugeBlinkCount = hud->tutorialGaugeBlinkCount + 1;
        } else if (hud->tutorialGaugeBlinkCount == 6) {
            SelectGolfMeterDisplay(hud->golfBaseDisplay, kGolfMeterPower);
            void* receiver2446_fn_800ED6BC = (void*)InstanceTable_Get();
            hud->golfBaseDisplay->f68(kFloat_2_805DED94 * ShotParameters_GetGoodImpactHalfWidth(receiver2446_fn_800ED6BC));
            DrawBallWindIndicator3Separately(hud->ballWindDisplay, 1);
            hud->ballWindDisplay->f60(MGR->roundedShotGaugeCursorX);
            hud->golfBaseDisplay->f54(MGR->roundedShotGaugeCursorX);
            ShowBallSelectorControls(hud->ballWindDisplay, 1);
            hud->tutorialGaugeBlinkCount = hud->tutorialGaugeBlinkCount + 1;
            return;
        }
    } else if (!hud->tutorialGaugeEnabled && MGR->tutorialStage < 6 && hud->tutorialMeterEnabled) {
        if (hud->meterPresentation != 1) hud->meterPresentation = 1;
        ShowBallSelectorControls(hud->ballWindDisplay, 1);
        SelectGolfMeterDisplay(hud->golfBaseDisplay, kGolfMeterHidden);
    } else if (!hud->tutorialGaugeEnabled && !hud->tutorialMeterEnabled && MGR->tutorialStage < 6) {
        SelectGolfMeterDisplay(hud->golfBaseDisplay, kGolfMeterHidden);
        ShowBallSelectorControls(hud->ballWindDisplay, 0);
    }

    if ((!hud->tutorialMeterEnabled || !hud->tutorialGaugeEnabled) && MGR->tutorialStage < 6) return;

    // Phase 3: select the active meter representation after tutorial gates.
    switch (hud->hudDisplayMask) {
    case kHudMeterInactive:
        hud->meterPresentation = 0;
        break;
    case kHudMeterTransparent:
        if (hud->meterPresentation != 1) hud->meterPresentation = 1;
        if (((Game*)InstanceTable_Get())->gp->mode != 3 && MGR->tutorialStage == 6) ShowBallWindElement0(hud->ballWindDisplay, 1);
        if (!SharedDoc_IsPlayerFlag0Set(MGR->currentPlayer)) ShowBallSelectorControls(hud->ballWindDisplay, 1);
        alpha = 0;
        break;
    case kHudMeterPower:
        SelectGolfMeterDisplay(hud->golfBaseDisplay, kGolfMeterPower);
        if (((Game*)InstanceTable_Get())->gp->mode != 3 && MGR->tutorialStage == 6) ShowBallWindElement0(hud->ballWindDisplay, 1);
        if (!SharedDoc_IsPlayerFlag0Set(MGR->currentPlayer)) ShowBallSelectorControls(hud->ballWindDisplay, 1);
        break;
    case kHudMeterTerrain10:
        SelectGolfMeterDisplay(hud->golfBaseDisplay, kGolfMeterTerrain10);
        if (!SharedDoc_IsPlayerFlag0Set(MGR->currentPlayer)) ShowBallSelectorControls(hud->ballWindDisplay, 1);
        break;
    case kHudMeterTerrain15:
        SelectGolfMeterDisplay(hud->golfBaseDisplay, kGolfMeterTerrain15);
        if (!SharedDoc_IsPlayerFlag0Set(MGR->currentPlayer)) ShowBallSelectorControls(hud->ballWindDisplay, 1);
        break;
    case kHudMeterTerrain20:
        SelectGolfMeterDisplay(hud->golfBaseDisplay, kGolfMeterTerrain20);
        if (!SharedDoc_IsPlayerFlag0Set(MGR->currentPlayer)) ShowBallSelectorControls(hud->ballWindDisplay, 1);
        break;
    }

    if (hud->meterFadeFrame == 0 && hud->hudInitialized != 0) {
        if (GLB_AD8 == 3) {
            NameTbl names = *(NameTbl*)partyMeterGroupNames;
            int idx = hud->partyMeterIndex;
            if (idx >= 0 && (unsigned int)idx < 3) {
                UIGroup_SetOpacity(BpObject_GetOrCreateNamedGroup((void*)hud->partyGolfBaseScript, names.s[idx]), alpha >> 1);
            }
        }
        hud->meterOpacity = alpha;
        alpha |= 0xFFFFFF00;
        SetBallWindIndicatorTint(hud->ballWindDisplay, alpha);
        SetGolfMeterTint(hud->golfBaseDisplay, alpha);
    }

    if (usesTerrainMeter) {
        if (MGR->shotPhase < 2) hud->golfBaseDisplay->f54(MGR->roundedShotGaugeCursorX);
        else hud->golfBaseDisplay->f54(MGR->shotPowerGaugeX);
        if (!SharedDoc_IsPlayerFlag0Set(MGR->currentPlayer)) {
            if ((hud->hudFlags & HudShotGaugeVisible) || MGR->shotPhase) DrawBallWindIndicator1Separately(hud->ballWindDisplay, 1);
        }
        hud->ballWindDisplay->f60(MGR->roundedShotGaugeCursorX);
    } else {
        if (MGR->shotPhase == 0 && hud->swingFeedbackPending) {
            hud->swingResultDisplay->f24();
            hud->swingFeedbackPending = 0;
        }
        HudMeterGolfStateView* m = MGR;
        if (m->shotPhase < 2) {
            if (hud->impactFeedbackStarted) {
                hud->golfBaseDisplay->f5C();
                hud->impactFeedbackStarted = 0;
            }
            hud->golfBaseDisplay->f54(MGR->roundedShotGaugeCursorX);
        } else if (m->shotPhase == 2) {
            hud->golfBaseDisplay->f54(m->shotPowerGaugeX);
        } else if (m->shotPhase == 3) {
            if (*(int*)((char*)g_sharedDoc + m->currentPlayer * 0x1D4 + 0x1DC) == 0 && hud->impactFeedbackStarted == 0) {
                hud->golfBaseDisplay->f58(m->impactFeedbackGaugeX); // Prepared impact-feedback gauge position, also saved/restored by CRival.
                hud->impactFeedbackStarted = hud->impactFeedbackStarted + 1;
            }
            if (!hud->swingResultDisplay->f28()) CScreen_StartDirectionalSwingFeedback(hud);
            hud->golfBaseDisplay->f54(MGR->shotPowerGaugeX);
        }
        void* receiver2538_fn_800ED6BC = (void*)InstanceTable_Get();
        hud->golfBaseDisplay->f68(kFloat_2_805DED94 * ShotParameters_GetGoodImpactHalfWidth(receiver2538_fn_800ED6BC));
        if (!SharedDoc_IsPlayerFlag0Set(MGR->currentPlayer) && hud->hudDisplayMask != kHudMeterTransparent) DrawBallWindIndicator3Separately(hud->ballWindDisplay, 1);
        hud->ballWindDisplay->f60(MGR->roundedShotGaugeCursorX);
    }

    // A shot-completion request cancels the meter fade and restores full opacity.
    // Phase 4: consume fade-reset requests and update the visible meter fade.
    if (MGR->shotMeterFadeResetRequested && (hud->hudFlags & 1) && hud->meterFadeFrame) {
        if (hud->hudInitialized) {
            if (GLB_AD8 == 3) {
                NameTbl names = *(NameTbl*)partyMeterGroupNames;
                int idx = hud->partyMeterIndex;
                if (idx >= 0 && (unsigned int)idx < 3) {
                    UIGroup_SetOpacity(BpObject_GetOrCreateNamedGroup((void*)hud->partyGolfBaseScript, names.s[idx]), 0x7F);
                }
            }
            hud->meterOpacity = 0xFF;
            SetBallWindIndicatorTint(hud->ballWindDisplay, -1);
            SetGolfMeterTint(hud->golfBaseDisplay, -1);
        }
        hud->meterFadeFrame = 0;
        hud->impactFeedbackStarted = 0;
    }

    if (MGR->gameStateFlags & HudMeterFadeStates) {
        int n = hud->meterFadeFrame;
        if (n < 0x23) {
            if (n >= 0x1E) {
                if (n == 0x22) alpha = 0;
                else alpha = 255 - (n - 0x1D) * 255 / 5;
                if (hud->hudInitialized) {
                    if (GLB_AD8 == 3) {
                        NameTbl names = *(NameTbl*)partyMeterGroupNames;
                        int idx = hud->partyMeterIndex;
                        if (idx >= 0 && (unsigned int)idx < 3) {
                            UIGroup_SetOpacity(BpObject_GetOrCreateNamedGroup((void*)hud->partyGolfBaseScript, names.s[idx]), alpha >> 1);
                        }
                    }
                    hud->meterOpacity = alpha;
                    alpha |= 0xFFFFFF00;
                    SetBallWindIndicatorTint(hud->ballWindDisplay, alpha);
                    SetGolfMeterTint(hud->golfBaseDisplay, alpha);
                }
            }
            hud->meterFadeFrame = hud->meterFadeFrame + 1;
        }
    }
}

} // extern "C"

#undef MGR
#undef GLB_AD8

extern "C" {

struct U80180710_Menu { char pad[0x334]; void* controls; void* other; };

extern int Input_IsActionButtonDown(void*, int, int);

typedef CGolfDoc HudGolfStateView;
typedef CGolfDoc HudImpactStateView;
struct HudSharedStateView { char pad[0xB90]; HudGolfStateView* game; };
struct U80180710_Root { char pad[0xB90]; HudSharedStateView* manager; };

extern const float kScreenImpactAtlasSpriteWidth; // 39.0f: impact atlas sprite width
extern const float kScreenImpactAtlasUStep; // 0.609375f: impact atlas U step (39/64)
extern const float kScreenImpactAtlasVStep; // 0.1328125f: impact atlas V step (17/128)
extern double kScreenSignedIntToDoubleBias;
extern int s_impactTimingQuadTemplate[];

void Engine_SetTextureStageResource(int, void*);
void Engine_SetTexturePalette(int, void*);
void Engine_SetFlexibleVertexFormat(int);
void DrawPrimitiveUP(int, int, void*, int);

void Layout_SetElementValueState(void*, int, int);

struct U80180710_VT { char pad[0x18]; void (*set)(void*, int, int); void (*finish)(void*); };
struct U80180710_Widget { char pad[0x74]; U80180710_VT* vt; };
struct U80180710_Object { char pad0[0x24]; void* child; char pad1[0x160]; int done; char pad2[0x124]; void* list; char pad3[0x74]; int color; char pad4[0x14]; void* panel; };

struct U80180710_ActionVT { char pad[0x34]; void (*action44)(void*); void (*action)(void*, int); };
struct U80180710_Action { char pad[0x9C]; U80180710_ActionVT* vt; };
struct U80180710_ChoiceVT { char pad[0x50]; void (*choice)(void*, int); };
struct U80180710_Choice { char pad[0xA4]; U80180710_ChoiceVT* vt; };

struct U80180710_OuterVT { char pad[0x1C]; int (*call)(void*, int); };
struct U80180710_Outer { char pad[0xA4]; U80180710_OuterVT* vt; };

struct U80180710_InputVT { char pad[0x3C]; void (*m3c)(void*, int); char pad2[0x2C]; int (*m6c)(void*); int (*m70)(void*); int (*m74)(void*); char pad3[0x4]; void (*m7c)(void*); };
struct U80180710_Obj330VT { char pad[0x64]; void (*m64)(void*); char pad2[0x4]; int (*m6c)(void*); int (*m70)(void*); };
struct U80180710_Input334VT { char pad[0x48]; void (*m48)(void*); void (*m4c)(void*); int (*m50)(void*); };
struct U80180710_VirtualObj330Prefix { char pad[0xE0]; };
struct U80180710_VirtualObj330 : U80180710_VirtualObj330Prefix {
    virtual void v00(); virtual void v01(); virtual void v02(); virtual void v03();
    virtual void v04(); virtual void v05(); virtual void v06(); virtual void v07();
    virtual void v08(); virtual void v09(); virtual void v0a(); virtual void v0b();
    virtual void v0c(); virtual void v0d(); virtual void v0e(); virtual void v0f();
    virtual void v10(); virtual void v11(); virtual void v12(); virtual void v13();
    virtual void v14(); virtual void v15(); virtual void v16();
    virtual void m64(); virtual void m68(); virtual int m6c(); virtual int m70();
};
struct HudMapCameraTransitionInterfacePrefix { char pad[0x9C]; };
struct HudMapCameraTransitionInterface : HudMapCameraTransitionInterfacePrefix {
    virtual void v00(); virtual void v01(); virtual void v02(); virtual void v03();
    virtual void v04(); virtual void v05(); virtual void v06(); virtual void v07();
    virtual void v08(); virtual void v09(); virtual void v0a(); virtual void v0b();
    virtual void v0c(); virtual void v0d(); virtual void v0e(); virtual void v0f();
    virtual void BeginOpening(); virtual void BeginClosing(); virtual int IsTransitionIdle();
};
struct U80180710_Input { char pad[0x164]; U80180710_InputVT* vt; };
struct U80180710_Obj330 { char pad[0xE0]; U80180710_Obj330VT* vt; };
struct U80180710_Input334 { char pad[0x9C]; U80180710_Input334VT* vt; };
struct HudPanelTransitionState {
    char pad0[0x260]; int panelOpenRequested; int itemPanelOpenRequested; int itemPanelReady;
    char pad26c[0x10]; int panelsReady;
    char pad280[8]; int tutorialAimEnabled;
    char pad28c[0x5C]; int tutorialAimBlinkFrame; int tutorialAimBlinkCount;
    char pad2f0[0x18]; int requestedPanel; int savedPanel; int previousPanel; int hudPhase;
    char pad318[0x18]; void* strokePointDisplay; void* mapCameraDisplay; void* ballWindDisplay; void* golfBaseDisplay; void* swingResultDisplay;
};

extern int CIngameStatus_IsShownOrShowing(void*);
extern void CIngameStatus_BeginShow(void*);

struct U80180710_InputPrefix { char pad[0x164]; };
struct U80180710_VirtualToggle : U80180710_InputPrefix {
    virtual void t00(); virtual void t01(); virtual void t02(); virtual void t03();
    virtual void t04(); virtual void t05(); virtual void t06(); virtual void t07();
    virtual void t08(); virtual void t09(); virtual void t0a(); virtual void t0b();
    virtual void t0c();
    virtual void m3c(int);
    virtual void t10(); virtual void t11(); virtual void t12(); virtual void t13();
    virtual void t14(); virtual void t15(); virtual void t16(); virtual void t17();
    virtual void t18(); virtual void t19(); virtual void t1a();
    virtual void m6c();
    virtual void m70();
    virtual int m74();
    virtual void t1b();
    virtual void m7c();
};
// Advance requested panel transitions once all affected widgets report readiness.
// Protocol for offset-named interface calls below (shared method names remain):
// stroke m6c / camera m50 / gauge m74 = transition-idle queries;
// gauge m6c / m70 = begin open / close. Named camera transitions are CMapCamera2d
// vfunc_16/17/18: open resets action latches and fades from zero over five update
// calls; close starts at full opacity over seven calls; idle means neither opening
// nor closing (it does not mean fully open). These counts belong to map_camera_2d_panel.cpp.
// stroke m70 = fully open query, m64 = request open. A query is not an action.
// panelsReady is cleared while waiting and raised only by the matching stable
// case. Changing requestedPanel does not make its widgets ready in that call.
void CScreen_UpdatePanelTransitions(void* object) {
    HudPanelTransitionState* self = (HudPanelTransitionState*)object;
    if (((HudSharedStateView*)g_sharedDoc)->game->tutorialStage < 6 && self->tutorialAimEnabled != 0) {
        int count = self->tutorialAimBlinkCount;
        if (count < 6) {
            if (self->tutorialAimBlinkFrame < 2) {
                if (count % 2) {
                    ((U80180710_VirtualToggle*)self->golfBaseDisplay)->m7c();
                    ((U80180710_VirtualToggle*)self->golfBaseDisplay)->m3c(0);
                } else {
                    ((U80180710_VirtualToggle*)self->golfBaseDisplay)->m3c(1);
                }
                ++self->tutorialAimBlinkFrame;
                return;
            }
            self->tutorialAimBlinkFrame = 0;
            ++self->tutorialAimBlinkCount;
            return;
        }
        if (count == 6) {
            ((U80180710_VirtualToggle*)self->golfBaseDisplay)->m3c(1);
            self->requestedPanel = 1;
            self->previousPanel = 1;
            ++self->tutorialAimBlinkCount;
            self->panelOpenRequested = 0;
        }
    }
    if (self->panelsReady && self->requestedPanel == self->previousPanel) {
        if (((HudSharedStateView*)g_sharedDoc)->game->tutorialStage == 6) return;
    }
    if (self->requestedPanel != self->previousPanel) self->panelsReady = 0;
    if (!((U80180710_VirtualObj330*)self->strokePointDisplay)->m6c()) return;
    if (!((HudMapCameraTransitionInterface*)self->mapCameraDisplay)->IsTransitionIdle()) return;
    if (!((U80180710_VirtualToggle*)self->golfBaseDisplay)->m74()) return;
    switch (SharedDoc_IsPlayerFlag0Set(((HudSharedStateView*)g_sharedDoc)->game->currentPlayer)) {
    case 0: break;
    default: return;
    }
    switch (self->requestedPanel) {
    case HudPanelNone:
        switch (self->previousPanel) {
        case 3: break;
        case HudPanelAim: { ((U80180710_VirtualToggle*)self->golfBaseDisplay)->m70(); } break;
        case HudPanelAimClosing: { ((U80180710_VirtualToggle*)self->golfBaseDisplay)->m70(); } break;
        case HudPanelCamera: { ((HudMapCameraTransitionInterface*)self->mapCameraDisplay)->BeginClosing(); } break;
        }
        self->panelsReady = 1;
        break;
    case HudPanelAim:
        switch (self->previousPanel) {
        case HudPanelNone:
            if (AsCScreen(self)->ingameStatus && GolfState_AllowsStatusTransition(AsCScreen(self)->ingameStatus) && !CIngameStatus_IsShownOrShowing(AsCScreen(self)->ingameStatus)) CIngameStatus_BeginShow(AsCScreen(self)->ingameStatus);
            break;
        case HudPanelAim:
          if (self->panelOpenRequested) {
            if (AsCScreen(self)->ingameStatus && GolfState_AllowsStatusTransition(AsCScreen(self)->ingameStatus) && !CIngameStatus_IsShownOrShowing(AsCScreen(self)->ingameStatus)) CIngameStatus_BeginShow(AsCScreen(self)->ingameStatus);
            if (self->hudPhase != HudShotTransition) { ((U80180710_VirtualToggle*)self->golfBaseDisplay)->m6c(); }
            self->panelOpenRequested = 0;
          } else self->panelsReady = 1;
          break;
        case HudPanelCamera: ((HudMapCameraTransitionInterface*)self->mapCameraDisplay)->BeginClosing(); break;
        }
        break;
    case HudPanelAimClosing:
        switch (self->previousPanel) { case 2: self->panelsReady = 1; break; }
        break;
    case HudPanelItems:
        switch (self->previousPanel) {
        case HudPanelAim: ((U80180710_VirtualToggle*)self->golfBaseDisplay)->m70(); break;
        case HudPanelCamera: ((HudMapCameraTransitionInterface*)self->mapCameraDisplay)->BeginClosing(); break;
        case HudPanelItems: if (self->itemPanelOpenRequested) { self->itemPanelOpenRequested=0; self->itemPanelReady=1; } else self->panelsReady=1; break;
        }
        break;
    case HudPanelSpin:
        switch (self->previousPanel) {
        case HudPanelAim: ((U80180710_VirtualToggle*)self->golfBaseDisplay)->m70(); break;
        case HudPanelCamera: ((HudMapCameraTransitionInterface*)self->mapCameraDisplay)->BeginClosing(); break;
        case HudPanelSpin: if (!((U80180710_VirtualObj330*)self->strokePointDisplay)->m70()) ((U80180710_VirtualObj330*)self->strokePointDisplay)->m64(); else self->panelsReady=1; break;
        }
        break;
    case HudPanelCamera:
        switch (self->previousPanel) {
        case HudPanelAim: ((U80180710_VirtualToggle*)self->golfBaseDisplay)->m70(); break;
        case HudPanelCamera:
          if (self->panelOpenRequested) {
            if (self->hudPhase != HudShotTransition) { ((HudMapCameraTransitionInterface*)self->mapCameraDisplay)->BeginOpening(); }
            if (AsCScreen(self)->ingameStatus && GolfState_AllowsStatusTransition(AsCScreen(self)->ingameStatus) && !CIngameStatus_IsShownOrShowing(AsCScreen(self)->ingameStatus)) CIngameStatus_BeginShow(AsCScreen(self)->ingameStatus);
            self->panelOpenRequested=0;
          } else self->panelsReady=1;
          break;
        }
        break;
    }
}

#pragma use_lmw_stmw on
// Animate club changes and enable the named overlay groups for the selected club.
void CScreen_UpdateClubChangeDisplay(void* object) {
    void* self = (void*)object;
    struct Table { char bytes[0x220]; };
    int i;
    char* item;
    void* menu = InstanceTable_Get();
    void* user = ((ClubSelectionState*)menu)->selection;
    ((U80180710_Outer*)AsCScreen(self)->ballWindDisplay)->vt->call(AsCScreen(self)->ballWindDisplay, ((ClubSelectionSnapshot*)user)->clubIndex);

    int phase = (*(int*)&AsCScreen(self)->hudPhase);
    // Phase 1: shot transition suppresses club-change controls and retires old state.
    if (phase == HudShotTransition) {
        int status = AsCScreen(self)->clubChangeDisplayState;
        if (status == 1 || status == 2) {
            char* names = (char*)screenOverlayStrings;
            BpObject_StartScriptByName(AsCScreen(self)->ballWindScript, names + ScreenText_ClubChangeOutAnimation, 1);
            for (i = 0; i < ClubOverlayCount; ++i) {
                char buffer[0x220];
                *(Table*)buffer = *(Table*)s_clubOverlayGroupNames;
                if ((unsigned)i <= 0x11) {
                    item = buffer + i * 0x20;
                    if (strcmp(item, names + ScreenText_EmptyText)) {
                        UIGroup_SetEnabled(BpObject_GetOrCreateNamedGroup(AsCScreen(self)->ballWindScript, item), 0);
                    }
                    char* second = item + 0x10;
                    if (strcmp(second, names + ScreenText_EmptyText)) {
                        UIGroup_SetEnabled(BpObject_GetOrCreateNamedGroup(AsCScreen(self)->ballWindScript, second), 0);
                    }
                }
            }
        }
        menu = InstanceTable_Get();
        ((ClubSelectionState*)menu)->clubMenuState = 2;
        AsCScreen(self)->clubChangeDisplayState = 0;
    } else {
        menu = InstanceTable_Get();
        int menuState = ((ClubSelectionState*)menu)->clubMenuState;
        if (menuState == 3) {
            if (AsCScreen(self)->clubChangeDisplayState != 1) {
                AsCScreen(self)->clubChangeDisplayState = 1;
                BpObject_StartScriptByName(AsCScreen(self)->ballWindScript, screenOverlayStrings + ScreenText_ClubChangeInAnimation, 1);
            }
        } else if (((ClubSelectionState*)InstanceTable_Get())->clubMenuState == 0 && AsCScreen(self)->clubChangeDisplayState != 0) {
                AsCScreen(self)->clubChangeDisplayState = 2;
                menu = InstanceTable_Get();
                user = ((ClubSelectionState*)menu)->selection;
                int selected = *((unsigned char*)user + 8);
                if (AsCScreen(self)->displayedClubIndex != selected) {
                    menu = InstanceTable_Get();
                    user = ((ClubSelectionState*)menu)->selection;
                    AsCScreen(self)->displayedClubIndex = *((unsigned char*)user + 8);
                    char buffer[0x220];
                    for (i = 0; i < ClubOverlayCount; ++i) {
                        *(Table*)buffer = *(Table*)s_clubOverlayGroupNames;
                        if ((unsigned)i <= 0x11) {
                        item = buffer + i * 0x20;
                        if (strcmp(item, screenOverlayStrings + ScreenText_EmptyText)) {
                            UIGroup_SetEnabled(BpObject_GetOrCreateNamedGroup(AsCScreen(self)->ballWindScript, item), 0);
                        }
                        char* second = item + 0x10;
                        if (strcmp(second, screenOverlayStrings + ScreenText_EmptyText)) {
                            UIGroup_SetEnabled(BpObject_GetOrCreateNamedGroup(AsCScreen(self)->ballWindScript, second), 0);
                        }
                        }
                    }
                    char current[0x220];
                    selected = AsCScreen(self)->displayedClubIndex;
                    *(Table*)current = *(Table*)s_clubOverlayGroupNames;
                    if ((unsigned)selected <= 0x11) {
                        item = current + selected * 0x20;
                        if (strcmp(item, screenOverlayStrings + ScreenText_EmptyText)) {
                            UIGroup_SetEnabled(BpObject_GetOrCreateNamedGroup(AsCScreen(self)->ballWindScript, item), 1);
                        }
                        char* second = item + 0x10;
                        if (strcmp(second, screenOverlayStrings + ScreenText_EmptyText)) {
                            UIGroup_SetEnabled(BpObject_GetOrCreateNamedGroup(AsCScreen(self)->ballWindScript, second), 1);
                        }
                    }
                }
        // Display-state cases below drive named club groups; keep each enable/disable
        // pair together so an outgoing club does not leave its companion highlight visible.
        } else if (((ClubSelectionState*)InstanceTable_Get())->clubMenuState == 1 && AsCScreen(self)->clubChangeDisplayState != 0) {
            int status = AsCScreen(self)->clubChangeDisplayState;
            if (status != 3) {
                AsCScreen(self)->clubChangeDisplayState = 3;
                BpObject_StartScriptByName(AsCScreen(self)->ballWindScript, screenOverlayStrings + ScreenText_ClubChangeOutAnimation, 1);
                char buffer[0x220];
                for (i = 0; i < ClubOverlayCount; ++i) {
                    *(Table*)buffer = *(Table*)s_clubOverlayGroupNames;
                    if ((unsigned)i <= 0x11) {
                    item = buffer + i * 0x20;
                    if (strcmp(item, screenOverlayStrings + ScreenText_EmptyText)) {
                        UIGroup_SetEnabled(BpObject_GetOrCreateNamedGroup(AsCScreen(self)->ballWindScript, item), 0);
                    }
                    char* second = item + 0x10;
                    if (strcmp(second, screenOverlayStrings + ScreenText_EmptyText)) {
                        UIGroup_SetEnabled(BpObject_GetOrCreateNamedGroup(AsCScreen(self)->ballWindScript, second), 0);
                    }
                    }
                }
            }
        } else {
            AsCScreen(self)->clubChangeDisplayState = 0;
        }
    }
}
#pragma use_lmw_stmw reset

// CMapCamera2d's secondary vptr is at +0x9C. These are per-layout-update
// requests: setters mark layoutDirty; UpdateFrame consumes/resets the requests.
// +34 edge indicators, +38 highlighted mode (0 none, 1..4 mode indicators),
// +3C controls, +44 action input. See map_camera_2d_panel.cpp vfunc_11/12/13/15.
struct HudMapCameraHintPrefix { char pad[0x9C]; };
struct HudMapCameraHintInterface : HudMapCameraHintPrefix {
    virtual void v08(); virtual void v0c(); virtual void v10(); virtual void v14();
    virtual void v18(); virtual void v1c(); virtual void v20(); virtual void v24();
    virtual void v28(); virtual void v2c(); virtual void v30(); virtual void RequestEdgeIndicators();
    virtual void SetHighlightedMode(int); virtual void RequestControls(); virtual void v40(); virtual void EnableInputForFrame();
};
struct U80180710_ChoicePrefix { char pad[0xA4]; };
struct U80180710_VirtualChoice : U80180710_ChoicePrefix {
    virtual void v08(); virtual void v0c(); virtual void v10(); virtual void v14();
    virtual void v18(); virtual void v1c(); virtual void v20(); virtual void v24();
    virtual void v28(); virtual void v2c(); virtual void v30(); virtual void v34();
    virtual void v38(); virtual void v3c(); virtual void v40(); virtual void v44();
    virtual void v48(); virtual void v4c(); virtual void m50(int);
};

// Update camera-action hints and the shot-control prompt from input and actor state.
void CScreen_UpdateCameraAndShotHints(void* object) {
    HudPanelTransitionState* self = (HudPanelTransitionState*)object;
    if (!((U80180710_Menu *&)Sing_805DC5C8())) {
        if (!s_inputManagerInitialized) {
            InputManager_Init(s_gameInputManager);
            __register_global_object(s_gameInputManager, (void*)InputManager_Dtor, s_gameInputManagerDestructorRegistration_Screen);
            s_inputManagerInitialized = 1;
        }
        ((U80180710_Menu *&)Sing_805DC5C8()) = (U80180710_Menu*)s_gameInputManager;
    }
    if (!Input_IsActionButtonDown(((U80180710_Menu *&)Sing_805DC5C8()), HudActionHoldCamera, -1)) {
        int mode = ((HudSharedStateView*)g_sharedDoc)->game->cameraMode;
        if (mode && mode != 4) {
            ((HudMapCameraHintInterface*)self->mapCameraDisplay)->RequestEdgeIndicators();
        }
    }
    int result;
    IActor_DispatchMessage(self, self, screenOverlayStrings + ScreenText_CameraActor, 8,(void*)(&result),(void*)(0),(void*)(0),(void*)(0));
    // Camera actor replies -> highlighted indicator: 1->none, 2->4, 3->1,
    // 7->2, 8->3, other->none. These are layout mode indicators, not camera
    // world coordinates; changing actor modes requires keeping this mapping.
    switch (result) {
    case 1: ((HudMapCameraHintInterface*)self->mapCameraDisplay)->SetHighlightedMode(0); break;
    case 2: ((HudMapCameraHintInterface*)self->mapCameraDisplay)->SetHighlightedMode(4); break;
    case 3: ((HudMapCameraHintInterface*)self->mapCameraDisplay)->SetHighlightedMode(1); break;
    case 7: ((HudMapCameraHintInterface*)self->mapCameraDisplay)->SetHighlightedMode(2); break;
    case 8: ((HudMapCameraHintInterface*)self->mapCameraDisplay)->SetHighlightedMode(3); break;
    default: ((HudMapCameraHintInterface*)self->mapCameraDisplay)->SetHighlightedMode(0); break;
    }
    int phase = (*(int*)&AsCScreen(self)->hudPhase);
    if (phase != HudItems && phase != HudSpin) {
        HudGolfStateView* game = ((HudSharedStateView*)g_sharedDoc)->game;
        if (game->tutorialStage < 6) {
            if ((*(int*)&AsCScreen(self)->tutorialClubChangeEnabled)) ((HudMapCameraHintInterface*)self->mapCameraDisplay)->EnableInputForFrame();
        } else {
            ((HudMapCameraHintInterface*)self->mapCameraDisplay)->EnableInputForFrame();
        }
    }
    if (((HudSharedStateView*)g_sharedDoc)->game->cameraMode != 4) ((HudMapCameraHintInterface*)self->mapCameraDisplay)->RequestControls();
    ((U80180710_VirtualChoice*)self->ballWindDisplay)->m50(0);

    HudGolfStateView* game = ((HudSharedStateView*)g_sharedDoc)->game;
    if (game->gameStateFlags & 0x80) {
        void* p12 = GolfBall__Fv();
        int* state = &((CGolfBall*)p12)->m_state;
        if (!*state && !(*(int*)&AsCSharedDoc(g_sharedDoc)->partyGameMode)) {
            HudGolfStateView* managerGame = ((HudSharedStateView*)g_sharedDoc)->game;
            if (!SharedDoc_IsPlayerFlag0Set(managerGame->currentPlayer) && ((HudSharedStateView*)g_sharedDoc)->game->tutorialStage == 6) {
                int held;
                unsigned char pressed;
                IActor_DispatchMessage(self, self, screenOverlayStrings + ScreenText_GolfRuleActor, 0x1D,(void*)(&held),(void*)(0),(void*)(0),(void*)(0));
                IActor_DispatchMessage(self, self, screenOverlayStrings + ScreenText_GolfRuleActor, 0x2F,(void*)((int*)&pressed),(void*)(0),(void*)(0),(void*)(0));
                if (!(held & 4) && !pressed) ((U80180710_VirtualChoice*)self->ballWindDisplay)->m50(1);
            }
        }
    }
}

struct U80180710_DrawVirtual {
    virtual void v00(); virtual void v01(); virtual void v02(); virtual void v03();
    virtual void v04(); virtual void v05(); virtual void v06(); virtual void v07();
    virtual void v08(); virtual void v09(); virtual void v0a(); virtual void v0b();
    virtual void v0c(); virtual void v0d(); virtual void v0e(); virtual void v0f();
    virtual void v10(); virtual void v11(); virtual void v12(); virtual void v13();
    virtual void v14(); virtual void v15(); virtual void v16(); virtual void v17();
    virtual void v18(); virtual void v19();
    virtual void* get(void*); virtual void* get2(void*);
};
struct U80180710_DrawRoot { char pad[0xA4]; U80180710_DrawVirtual* obj; };

// View of the swing-result display object (member swingResultDisplay, vtable at +0x74); only set()/finish() are used here.
struct SwingResultDisplayPrefix { char pad[0x74]; };
struct SwingResultDisplayView : SwingResultDisplayPrefix {
    virtual void v08(); virtual void v0c(); virtual void v10(); virtual void v14();
    virtual void set(int, int);
    virtual void finish();
};
struct U80180710_VirtualInput : U80180710_InputPrefix {
// CGolfBase2d_IsFadeSettled (golf_base_2d_hud_layout.cpp): 0 during fade states 1/3, 1 in hidden/settled states 0/2; this is not an
// input-ready/visible test. +64/+68 mark the layout dirty on change.
    virtual void v08(); virtual void v0c(); virtual void v10(); virtual void v14();
    virtual void v18(); virtual void v1c(); virtual void v20(); virtual void v24();
    virtual void v28(); virtual void v2c(); virtual void v30(); virtual void v34();
    virtual void v38(); virtual void v3c(); virtual void v40(); virtual void v44();
    virtual void v48(); virtual void v4c(); virtual void v50(); virtual void v54();
    virtual void v58(); virtual void v5c(); virtual void v60(); virtual void v64();
    virtual void v68(); virtual void v6c(); virtual void v70();
    // CGolfBase2d_IsFadeSettled (golf_base_2d_hud_layout.cpp): 0 during fade states 1/3,
    // 1 in hidden/settled states 0/2. This is NOT an input-ready/visible test.
    virtual int IsCaptionFadeSettled();
};

struct U80180710_13DC { char pad0[0x32c]; void* list; char pad1[0xc]; void* panel; };

enum SwingCaptionElement {
    kShotPhaseCaption = 0, kMotionCaptionFirst = 1,
    kMotionCaptionSecond = 2, kNonPutterMotionCaption = 3,
    kSwingCaptionElementCount = 4
};
enum SwingCaptionValue {
    kAimCaptionValue = 0, kPowerCaptionValue = 1,
    kImpactCaptionValue = 2, kAlternatePowerDecidedCaptionValue = 3
};
// Select the four caption elements from shot phase, motion state, and meter fade.
// list owns four loaded layout elements; panel is the golfBaseDisplay receiver
// at owner+33C, with its vptr at receiver+164. All pointers are required/borrowed.
// Admission clears elements 0..3 FIRST, then requires HudActiveLayoutState and
// +74 IsCaptionFadeSettled. A hidden but settled panel passes that latter test.
// Element 0's value states: shotPhase 0 -> aim=0, 1 -> power=1, 2 -> impact=2
// (or 3 when alternateMotionPowerDecided). Phase 1 plus motionSwingState enables
// elements 1/2 and element 3 unless clubKind==3 (putter). Other phases leave
// element 0 enabled with its previous value state: there is no default reset.
// The names describe selection policy; artwork/text lives in the loaded caption
// layout, not these enum values. Changing captions requires keeping its element
// and value-state domains together. No resources or gameplay state are changed.
void CScreen_UpdateSwingCaption(void* arg) {
    int i;
    i = 0;
    U80180710_13DC* self = (U80180710_13DC*)arg;
    for (; i < kSwingCaptionElementCount; ++i) Layout_SetElementEnabled(self->list, i, 0);
    HudGolfStateView* game = ((HudSharedStateView*)g_sharedDoc)->game;
    if (!(game->gameStateFlags & HudActiveLayoutState)) return;
    if (!((U80180710_VirtualInput*)self->panel)->IsCaptionFadeSettled()) return;
    Layout_SetElementEnabled(self->list, kShotPhaseCaption, 1);
    game = ((HudSharedStateView*)g_sharedDoc)->game;
    unsigned char state = game->shotPhase;
    switch (state) {
    case 0:
        Layout_SetElementValueState(self->list, kShotPhaseCaption, kAimCaptionValue);
        return;
    case 1:
        Layout_SetElementValueState(self->list, kShotPhaseCaption, kPowerCaptionValue);
        game = ((HudSharedStateView*)g_sharedDoc)->game;
        if (!game->motionSwingState) return;
        Layout_SetElementEnabled(self->list, kMotionCaptionFirst, 1);
        Layout_SetElementEnabled(self->list, kMotionCaptionSecond, 1);
        {
            void* p = InstanceTable_Get();
            p = *(void**)((char*)p + 4);
            if (((ClubSelectionSnapshot*)p)->clubKind != 3)
                Layout_SetElementEnabled(self->list, kNonPutterMotionCaption, 1);
        }
        return;
    case 2:
        if (game->alternateMotionPowerDecided) Layout_SetElementValueState(self->list, kShotPhaseCaption, kAlternatePowerDecidedCaptionValue);
        else Layout_SetElementValueState(self->list, kShotPhaseCaption, kImpactCaptionValue);
        break;
    }
}

// Draw the impact timing sprite from its atlas during the unassisted impact phase.
void CScreen_DrawImpactTimingSprite(void* object) {
    U80180710_Object* self = (U80180710_Object*)object;
    U80180710_Root* root = (U80180710_Root*)g_sharedDoc;
    HudGolfStateView* game = ((HudSharedStateView*)root)->game;
    int player = game->currentPlayer;
    char* playerData = (char*)root + player * 0x1D4;
    if (*(int*)(playerData + 0x1DC) != 0) return;
    if (game->shotPhase != 3) return;
    if (!(self->done & HudShotGaugeVisible)) {
        if ((*(unsigned int*)&AsCGolfDoc(game)->gameStateFlags) != 0x80) return;
    }

    int index = AsCGolfDoc(game)->impactTimingSpriteIndex;
    void* panel = self->child;
    void* material = ((BallPreviewMaterialView*)panel)->material;
    int frame = index * 5 + 70;
    if (frame < 40) return;
    if (frame >= 100) return;
    UIRender_SetDefaultState();

    int quarter;
    int remainder;
    int third = (100 - frame) / 5;
    quarter = third / 7;
    remainder = third % 7;
    game = ((HudSharedStateView*)g_sharedDoc)->game;
    float x0 = AsCGolfDoc(game)->shotPowerGaugeX - kFloat_50_Screen;
    float x1 = kScreenImpactAtlasSpriteWidth + x0;
    int ix0 = (int)x0;
    int ix1 = (int)x1;
    float uv[4];
    uv[0] = kScreenImpactAtlasUStep * (float)quarter;
    uv[2] = kScreenImpactAtlasUStep * (float)(quarter + 1);
    uv[1] = kScreenImpactAtlasVStep * (float)remainder;
    uv[3] = kScreenImpactAtlasVStep * (float)(remainder + 1);
    struct Vertex {
        float x, y, z;
        union { unsigned int packed; unsigned char color[4]; };
        float u, v;
    };
    struct Vertices { Vertex v[4]; } verts;
    verts = *(Vertices*)s_impactTimingQuadTemplate;
    Vertex* v = verts.v;
    int top = 383;
    int bottom = 400;
    v[0].x = ix0; v[0].y = top; v[0].color[3] = self->color; v[0].u = uv[0]; v[0].v = uv[1];
    v[1].x = ix1; v[1].y = top; v[1].color[3] = self->color; v[1].u = uv[2]; v[1].v = uv[1];
    v[2].x = ix0; v[2].y = bottom; v[2].color[3] = self->color; v[2].u = uv[0]; v[2].v = uv[3];
    v[3].x = ix1; v[3].y = bottom; v[3].color[3] = self->color; v[3].u = uv[2]; v[3].v = uv[3];
    Engine_SetTextureStageResource(0, ((U80180710_DrawRoot*)g_view)->obj->get(material));
    Engine_SetTexturePalette(0, ((U80180710_DrawRoot*)g_view)->obj->get2(material));
    Engine_SetFlexibleVertexFormat(0x11F005);
    DrawPrimitiveUP(0x98, 2, &verts, 0x18);
}

// Classify impact direction relative to the Pangya target and start the swing feedback display.
void CScreen_StartDirectionalSwingFeedback(void* object) {
    HudPanelTransitionState* self = (HudPanelTransitionState*)object;
    HudImpactStateView* game = (HudImpactStateView*)((HudSharedStateView*)g_sharedDoc)->game;
    int result = 0;
    if (game->pangyaAssistState != 1 && game->impactGrade == 4) result = 1;
    int direction;
    if (result) {
        direction = 0;
    } else {
        float delta = game->roundedShotGaugeCursorX - kPangyaTargetCenterX;
        delta = delta > kFloat_0_AuxMenu ? delta : -delta;
        void* receiver3104_fn_800ED6BC = (void*)InstanceTable_Get();
        float ed = ShotParameters_GetGoodImpactHalfWidth(receiver3104_fn_800ED6BC);
        float threshold = ed * kFloat_0_5_AuxMenu + kFloat_0_5_AuxMenu;
        if (delta < threshold) {
            game = (HudImpactStateView*)((HudSharedStateView*)g_sharedDoc)->game;
            direction = game->roundedShotGaugeCursorX - game->pangyaTargetX < kFloat_0_AuxMenu ? 2 : 1;
        } else {
            game = (HudImpactStateView*)((HudSharedStateView*)g_sharedDoc)->game;
            direction = game->roundedShotGaugeCursorX - game->pangyaTargetX < kFloat_0_AuxMenu ? 4 : 3;
        }
    }
    int mf = game->pangyaAssistState;
    ((SwingResultDisplayView*)self->swingResultDisplay)->set(direction, mf == 1);
    ((SwingResultDisplayView*)self->swingResultDisplay)->finish();
    (*(int*)&AsCScreen(self)->swingFeedbackPending) = 1;
}

} // extern "C"

extern "C" {

typedef unsigned char u8;
typedef unsigned int u32;

union U {
    int i;
    int* pi;
    float* pf;
    u8* pb;
    void* p;
};

// Message ABI: +4 ID, +8/+C/+10 p0/p1/p2, each one 32-bit word. The union
// is a view selected by ID, not a tagged value. Per-case Payload comments state
// immediate values versus input/output pointers. All pointed-to data is borrowed
// for this synchronous dispatch; arrays/vectors are copied and never retained.
// Output pointers must be valid; most queries do not validate them. No return
// value carries query data. Unlisted payload words are unused by that case.
struct ScreenMessage {
    int pad0;
    int id;
    U payload0;
    U payload1;
    U payload2;
};

struct Vec3 { int x, y, z; };
struct Tri { int v[3]; };

struct V0;
struct StrokePointMessageInterface;
struct MapCameraMessageInterface;
struct BallWindMessageInterface;
struct GolfBaseMessageInterface;
struct SwingResultMessageInterface;
struct TutorialIllustrationMessageInterface;

struct HoleInfoMessageView {
    V0* cupInfo;
    char pad4[4];
    V0* holeInfo;
    void* accuracyInfo;
    V0* shotTransitionInfo;
    u8 firstDisplayEnabled;
    u8 secondDisplayEnabled;
};

struct BallPreviewMessageView {
    char pad[0x1a4];
    float ballRingHeight;
};

struct WindInfoMessageView {
    char pad[0xc];
    int windStrength;
};

struct AuxiliaryMenuMessageView {
    char pad[0x14];
    int selectionActive;
    int selectedEntry;
    float selectionTimer;
};

struct PangInfoMessageView {
    char pad[0x20];
    u32 displayFlags;
};

struct CursorRegionStateView {
    char pad[0xdd];
    u8 tutorialPointerRegionEnabled;
    char pade[5];
    u8 auxiliaryMenuRegionEnabled[6];
};

struct ViewportSizeView {
    char pad[0x188];
    float viewportWidth;
    float viewportHeight;
};

struct HudMessageGolfStateView {
    char pad0[0x98];
    int tutorialStage;
    char pad1[0x2d0 - 0x9c];
    float roundedShotGaugeCursorX;
    char pad2[0x358 - 0x2d4];
    u32 gameStateFlags;
    char pad3[0x368 - 0x35c];
    int cameraMode;
    char pad4[0x3bc - 0x36c];
    void* f3bc;
    u8 pad5;
    u8 currentPlayer;
    char pad6[6];
    u8 dropRequest;
};

struct HudMessageSharedStateView {
    char pad0[0x8c0];
    u8 golfRuleType;
    char pad1[0xad8 - 0x8c1];
    int partyGameMode;
    char pad2[0xb90 - 0xadc];
    HudMessageGolfStateView* h;
};

struct TerrainProfilePoint {
    float v;
    u32 col;
};

struct HudMessageState {
    void* vt;
    char pad04[0x18];
    HoleInfoMessageView* holeInfoSet;
    char pad20[4];
    BallPreviewMessageView* ballPreview;
    char pad28[4];
    WindInfoMessageView* windInfo;
    char pad30[4];
    void* accuracyInfo;
    char pad38[4];
    void* ingameStatus;
    AuxiliaryMenuMessageView* auxiliaryMenu;
    PangInfoMessageView* pangInfo;
    void* skinsInfo;
    void* nearestPinInfo;
    V0* modeInfo;
    char pad54[8];
    float ballRingHeight;
    float displayTimeRemaining;
    char pad64[0x4c];
    float windHeading;
    int windStrength;
    char padb8[4];
    u32 powerFeedbackColor;
    u32 fc0;
    u32 fc4;
    char padc8[4];
    int terrainMarkerIndex;
    int terrainSampleCount;
    u32 terrainIndicatorColor;
    float shotIndicatorPulse;
    TerrainProfilePoint terrainProfile[21];
    char pad184[4];
    u32 hudFlags;
    float shotFeedbackBlend;
    float groundLieBlend;
    u8 tutorialStageVisible[8];
    u8 tutorialFlashCount;
    u8 tutorialFlashOn;
    char pad19e[6];
    float tutorialFlashTimer;
    u8 tutorialCompletionPhase;
    char pad1a9[7];
    float tutorialCompletionTimer;
    int previousTutorialStage;
    char pad1b8[4];
    int tutorialTitlePhase;
    u8 modeLabelOpacity;
    char pad1c1[11];
    float tutorialTitleTimer;
    float tutorialPointerDelay;
    u8 tutorialPointerDelayed;
    bool hudDisplayEnabled;
    char pad1d6[30];
    void* calipers;
    char pad1f8[100];
    int hudInitialized;
    char pad260[8];
    int itemPanelReady;
    int shotOptionEnabled;
    int shotOptionChanged;
    int resumeAimRequested;
    int hudInputBlocked;
    int panelsReady;
    int tutorialMeterEnabled;
    int tutorialGaugeEnabled;
    int tutorialAimEnabled;
    int tutorialClubChangeEnabled;
    int tutorialAimControlEnabled;
    int tutorialSpinControlEnabled;
    char pad298[4];
    int tutorialIllustrationActive;
    int cameraControlsEnabled;
    int aimControlsEnabled;
    int dropModeActive;
    int shotTransitionRequested;
    int swingFeedbackPending;
    char pad2b4[24];
    void* partyGolfBaseScript;
    char pad2d0[8];
    void* featherBadgeScript;
    char pad2dc[4];
    int meterFadeFrame;
    char pad2e4[4];
    int tutorialAimBlinkFrame;
    int tutorialAimBlinkCount;
    int tutorialMeterBlinkFrame;
    int tutorialMeterBlinkCount;
    int tutorialGaugeBlinkFrame;
    int tutorialGaugeBlinkCount;
    char pad300[8];
    int requestedPanel;
    char pad30c[8];
    int hudPhase;
    char pad318[4];
    int shotOptionResult;
    int shotDirectionResult;
    int partyMeterIndex;
    int meterOpacity;
    char pad32c[4];
    StrokePointMessageInterface* strokePointDisplay;
    MapCameraMessageInterface* mapCameraDisplay;
    BallWindMessageInterface* ballWindDisplay;
    GolfBaseMessageInterface* golfBaseDisplay;
    SwingResultMessageInterface* swingResultDisplay;
    TutorialIllustrationMessageInterface* tutorialIllustration;
    char pad348[12];
    int hudEnabled;
};

struct F9c { char d[0x9c]; };
struct Fa4 { char d[0xa4]; };
struct F164 { char d[0x164]; };
struct F74 { char d[0x74]; };
struct F78 { char d[0x78]; };
struct Fe0 { char d[0xe0]; };

struct V0 {
    virtual void u0();
    virtual void u1();
    virtual void u2();
    virtual void u3();
    virtual void m18(int, int);
    virtual void m1c();
    virtual void m20();
    virtual int m24();
    virtual void u8_();
    virtual void m2c(void*);
};

struct StrokePointMessageInterface : Fe0 {
    virtual void u0();
    virtual void u1();
    virtual void u2();
    virtual void ResetSelections(); // +14, return status ignored by this ABI view
    virtual void SetInputLockWord(void*); // +18, integer word in pointer-shaped ABI slot
    virtual void u5();
    virtual void u6();
    virtual void u7();
    virtual void u8_();
    virtual void u9();
    virtual void u10();
    virtual void u11();
    virtual void u12();
    virtual void u13();
    virtual void u14();
    virtual void u15();
    virtual void u16();
    virtual int GetStrokeOption();
    virtual void u18();
    virtual void u19();
    virtual void u20();
    virtual void u21();
    virtual void u22();
    virtual void u23();
    virtual void u24();
    virtual void u25();
    virtual int m70();
};

struct MapCameraMessageInterface : F9c {
    virtual void u0();
    virtual void u1();
    virtual void u2();
    virtual void u3();
    virtual void u4();
    virtual void u5();
    virtual int GetMapCameraState();
    virtual void ConfirmAction();
    virtual int GetMapCameraSelection();
    virtual void ResetAction();
    virtual int m30();
    virtual void u11();
    virtual void u12();
    virtual void u13();
    virtual void m40(int);
    virtual void u15();
    virtual void u16();
    virtual void u17();
    virtual int IsPanelIdle();
};

struct BallWindMessageInterface : Fa4 {
    virtual void u0();
    virtual void u1();
    virtual void u2();
    virtual void u3();
    virtual void u4();
    virtual void u5();
    virtual void u6();
    virtual void m24(int);
    virtual int m28();
    virtual void u9();
    virtual void u10();
    virtual void u11();
    virtual void u12();
    virtual void u13();
    virtual void u14();
    virtual void u15();
    virtual void u16();
    virtual void u17();
    virtual void u18();
    virtual void u19();
    virtual void u20();
    virtual void u21();
    virtual void u22();
    virtual void m64(int);
};

struct GolfBaseMessageInterface : F164 {
    virtual void u0();
    virtual void u1();
    virtual void u2();
    virtual void u3();
    virtual void u4();
    virtual void m1c(int);
    virtual void m20(int);
    virtual void u7();
    virtual void m28(int);
    virtual void u9();
    virtual void u10();
    virtual void u11();
    virtual void u12();
    virtual void u13();
    virtual void u14();
    virtual int GetAimPanelState();
    virtual void u16();
    virtual void ResetAimPanelAction();
    virtual void m50(int);
    virtual void u19();
    virtual void u20();
    virtual void m5c();
    virtual void m60(float);
    virtual void SetPangyaZoneWidth(float); // +64, marks layout dirty on change
    virtual void SetMeterOverlayWidth(float); // +68, marks layout dirty on change
    virtual void u25();
    virtual void u26();
    virtual int IsPanelIdle();
    virtual void u28();
    virtual void m7c();
};

struct SwingResultMessageInterface : F74 {
    virtual void u0();
    virtual void u1();
    virtual void u2();
    virtual void u3();
    virtual void u4();
    virtual void u5();
    virtual void m20();
};

struct TutorialIllustrationMessageInterface : F78 {
    virtual void u0();
    virtual void u1();
    virtual void u2();
    virtual void u3();
    virtual void u4();
    virtual void m1c(int, void*);
    virtual void u6();
    virtual void m24();
};

extern void* g_taskManager;
struct CamObj {
    char pad[0x24];
    float x;
    float y;
};
extern CamObj s_cameraTransform;

extern const float kScreenDropCameraClearance; // 192.0f: drop-camera vertical clearance
extern const float kScreenTerrainProfileRangeLimit; // 1.6f: terrain-profile normalization range
extern const float kScreenTerrainProfileColorMinDelta; // -8.0f: terrain-profile color delta lower bound
extern const float kScreenTerrainProfileColorScale; // 0.125f: terrain-profile color delta scale

void* CQuadTree_GetInstance(void);
void CQuadTree_OverlappedWithObject(void*, void*, float);
void NoticeQueue_ShowById(void*, int, int);
void* Object_GetChild(void*);
V0* TaskActorTable_FindByName(void*, char*);
void GetGroundPoint__9CQuadTreeF7WVectorbPib(void*, void*, void*, int, int, int);

void CQuadTree_GetMeshNormal(void*, void*, int);
void BallRing_UpdateGroundLie(void*, void*);
float CQuadTree_GetMaxPowerOnGround(void*);
void Hud_SetCurrentPlayerDisplayValue(void*, void*);
void Hud_UpdateNearestPinPlayer(void*);
void CAccuracyInfo_ShowCurrentShotFeedback(void*);
void CAccuracyInfo_ShowFullPower(void*);
void NoticeQueue_ResetAndActivate(void*);
void CPangInfo_ResetFromCurrentPlayer(void*);
void ScreenBallPreview_SelectCurrentPlayerBall(void*);

void CPangInfo_CalculateShotRewards(void*, int);
void CPangInfo_ResetBonusAnimations(void*);
void CPangInfo_FinishDisplayedTotals(void*);
void CPangInfo_CalculateMatchPlayHoleRewards(void*);
void Screen_GetOrLoadBallResource(void*, int);
void CIngameStatus_ResetState(void*);
void CHoleInfo_StartFlash(void*);
void CMouseCursor_MoveTo(void*, float, float, float);
void CScreen_BuildOrientedBasisMatrix(void*, void*);

void CHoleCupInfo_SnapDistanceOverlay(void*);
void CIngameStatus_RefreshPlayer(void*);
void SkinsInfo_StartResultDisplay(void*, int, int);
void CIngameStatus_ResetShotCounterFade(void*);

void CBpCtrl_SetDrawingEnabled(void*, int);

// Synchronous actor packet used by CGolfRule_HandleMsg (golf_rule_round_flow_and_shot_sim.cpp):
// message 7 calls CGolfRule_EnterState with word 2; the selector and remaining
// words are unused by that receiver case. No pointer in this packet is retained.
struct GolfRuleStateRequestPacket {
    unsigned int selector;
    unsigned int messageId;
    unsigned int requestedGameState;
    unsigned int payload1, payload2;
    unsigned int reserved5, reserved6;
    unsigned int completionWord;
};
enum GolfRuleDropTransition {
    kGolfRuleRequestStateMessage = 7,
    kGolfRuleCameraTransitionState = 0x200u
};
typedef char GolfRuleStatePacketSizeCheck[sizeof(GolfRuleStateRequestPacket) == 32 ? 1 : -1];

// Integer-load copies of three world-space float words preserve the original vector ABI.
struct HudMessagePositionWords { float v[3]; };
struct HudBallTriangleView { char precedingFields[0x28]; int groundTriangleIndex; };
// Offset-biased view anchored at document + playerIndex*stride, not a standalone player object.
struct HudMessagePlayerSlotView {
    char precedingHandles[0x1DC];
    void* strokePointHandle;
    void* illustrationHandle;
};
// Borrow the two player-specific handles forwarded to the stroke and tutorial displays.
static inline HudMessagePlayerSlotView* HudMessagePlayerSlot(void* document, int playerIndex) {
    return (HudMessagePlayerSlotView*)((char*)document + playerIndex*kSharedPlayerRecordStride);
}
enum HudColorBlendLanes {
    kPowerColorStartUnshiftedLanes = 0xFF006A, kPowerColorEndUnshiftedLanes = 0xFF001E,
    kPowerColorStartShiftedLanes = 0x10096, kPowerColorEndShiftedLanes = 0xC80042,
    kPowerColorSelectedBytes = 0xFF000000, kPowerColorSelectedMiddleByte = 0xFF00
};

#pragma push
#pragma always_inline on
// Set up drop bounds/camera and activate the map panel; world-space scratch stays caller-owned.
// Payload pointers are borrowed for this synchronous call; no pointer is retained.
static inline void EnterHudDropMode(
    HudMessageState* self, const ScreenMessage* message, HudMessagePositionWords& dropBoundsTestPosition,
    HudMessagePositionWords& dropGroundQueryPosition, WVector& dropGroundPosition) {
    // Payload: none; uses current HUD/shared state.
    if (0 != self->auxiliaryMenu) { if (0 == ((HudMessageSharedStateView*)g_sharedDoc)->h->dropRequest) {
            ((CursorRegionStateView*)g_mouseCursor)->auxiliaryMenuRegionEnabled[2] = 0;
            Vec3* ballPosition = (Vec3*)GolfBall__Fv();
            dropBoundsTestPosition = *(HudMessagePositionWords*)ballPosition;
            CQuadTree_OverlappedWithObject(CQuadTree_GetInstance(), &dropBoundsTestPosition, kFloat_0_AuxMenu);
            ((HudMessageSharedStateView*)g_sharedDoc)->h->dropRequest = 1;
            if (g_helpMsg != 0) {
                NoticeQueue_ShowById(g_helpMsg, 7, 0);
            }
            // Request a camera transition when ANY state bit other than 0x200 is set.
            // Both zero and exactly 0x200 skip it; mixed states such as 0x201 request it.
            if (((HudMessageSharedStateView*)g_sharedDoc)->h->gameStateFlags & ~kGolfRuleCameraTransitionState) {
                void* actorTable;
                if (0 != g_taskManager) {
                    actorTable = Object_GetChild(g_taskManager);
                } else {
                    actorTable = 0;
                }

                char* golfRuleActorName = screenOverlayStrings + ScreenText_GolfRuleActor;
                unsigned int zero = 0;
                GolfRuleStateRequestPacket packet;
                packet.selector = zero;
                packet.messageId = kGolfRuleRequestStateMessage;
                packet.requestedGameState = kGolfRuleCameraTransitionState;

                packet.payload1 = zero;
                packet.payload2 = zero;
                packet.reserved5 = zero;
                packet.reserved6 = zero;

                packet.completionWord = zero; V0* golfRuleActor = TaskActorTable_FindByName(actorTable, golfRuleActorName);
                if (0 != golfRuleActor) {
                    golfRuleActor->m2c(&packet);
                }
            }
            ballPosition = (Vec3*)GolfBall__Fv();

            dropGroundQueryPosition = *(HudMessagePositionWords*)ballPosition;
            GetGroundPoint__9CQuadTreeF7WVectorbPib(&dropGroundPosition, CQuadTree_GetInstance(), &dropGroundQueryPosition, 0, 0, 0);
            *(WVector*)(&s_cameraTransform.x) = dropGroundPosition;
            s_cameraTransform.y += kScreenDropCameraClearance;
        } }
    self->dropModeActive = 1;
    if (self->mapCameraDisplay) {
        self->mapCameraDisplay->m40(1);
    }
    GolfTipsDialog_RequestClose(5, kFloat_0_5_AuxMenu);
}

// Refresh the ball ring normal and lie blend from the current ground triangle.
// Payload pointers are borrowed for this synchronous call; no pointer is retained.
static inline void RefreshHudGroundLie(HudMessageState* self, const ScreenMessage* message, Vec3& groundNormal) {
    // Payload: none; uses current HUD/shared state.
    if (self->ballPreview) {
        char* ball = (char*)GolfBall__Fv();
        CQuadTree_GetMeshNormal(&groundNormal, CQuadTree_GetInstance(), ((HudBallTriangleView*)ball)->groundTriangleIndex);
        BallRing_UpdateGroundLie(self->ballPreview, &groundNormal);
    }
    if (self->holeInfoSet != 0) {
        self->holeInfoSet->cupInfo->m1c();
    }
    self->terrainIndicatorColor = 0x8000FF96;
    {
        self->groundLieBlend = CQuadTree_GetMaxPowerOnGround(CQuadTree_GetInstance());
        if (self->groundLieBlend < kFloat_1_AuxMenu) {
            self->groundLieBlend = self->groundLieBlend - kFloat_0_05_Screen;
        }
    }
}

// Interpolate the packed feedback color; a zero blend also clears the shot option.
// Payload pointers are borrowed for this synchronous call; no pointer is retained.
static inline void SetHudShotPowerColor(
    HudMessageState* self, const ScreenMessage* message, float*& blendPayload, u8& blendWeight,
    u32& inverseBlendWeight) {

    // Payload: p0=float* input blend (0..1 expected, no clamp); zero disables shot option.

    float scaledBlend;
    blendPayload = message->payload0.pf;
    scaledBlend = kFloat_255_Screen * *blendPayload;
    blendWeight = (int)scaledBlend;
    inverseBlendWeight = FullOpacity - blendWeight;

    u32 unshiftedColorLanes = inverseBlendWeight * kPowerColorStartUnshiftedLanes + blendWeight * kPowerColorEndUnshiftedLanes;
    u32 shiftedColorLanes = inverseBlendWeight * kPowerColorStartShiftedLanes + blendWeight * kPowerColorEndShiftedLanes;
    self->powerFeedbackColor = (((shiftedColorLanes & kPowerColorSelectedBytes) | (shiftedColorLanes & kPowerColorSelectedMiddleByte)) >> 8) | ((unshiftedColorLanes & kPowerColorSelectedBytes) | (unshiftedColorLanes & kPowerColorSelectedMiddleByte));
    if (*blendPayload == kFloat_0_AuxMenu) {
        if (self->shotOptionEnabled) {
            self->shotOptionEnabled = 0;
        }
    }
}

// Copy two borrowed opacity payloads into the feedback colors' alpha bytes.
// Payload pointers are borrowed for this synchronous call; no pointer is retained.
static inline void SetHudFeedbackOpacity(HudMessageState* self, const ScreenMessage* message) {
    // Payload: p0/p1=float* input opacity for the two feedback colors (0..1 expected).
    {
        int firstAlpha; float* firstOpacity; float* secondOpacity; int firstColor;
        firstOpacity = message->payload0.pf;
        firstAlpha = (int)(kFloat_255_Screen * *firstOpacity);
        firstColor = (firstAlpha << 24) + 0x1000000 - 1;
        secondOpacity = message->payload1.pf;
        self->fc0 = firstColor;
        self->fc4 = (((int)(kFloat_255_Screen * *secondOpacity)) << 24) + 0x1000000 - 1;
    }
}

// Copy height samples, compress their range and color offsets relative to the first sample.
// Payload pointers are borrowed for this synchronous call; no pointer is retained.
// Phase 1 copies raw heights and measures their range; phase 2 maps heights/colors into the HUD profile.
static inline void BuildHudTerrainProfile(
    HudMessageState* self, const ScreenMessage* message, int& sampleIndex, int& profileIndex,
    int& additionalSampleCount, int& profileKind, float*& heights, float& displayScale, float& heightRange,
    float& minimumHeight, float& heightMidpoint) {
    heights = message->payload2.pf; profileKind = message->payload0.i; additionalSampleCount = message->payload1.i;
    // Payload: p0=immediate profile kind; p1=additional sample count 0..20; p2=optional float[count+1] input, copied. No bounds check.

    self->terrainSampleCount = additionalSampleCount;
    self->terrainMarkerIndex = profileKind;
    if (0 != heights) {   float maximumHeight = heights[0];
        float initialMinimumHeight = maximumHeight;
        minimumHeight = initialMinimumHeight;

        float firstHeight = maximumHeight;

        self->terrainProfile[0].v = firstHeight;
        for (sampleIndex = 1; sampleIndex < 1 + self->terrainSampleCount; sampleIndex++) {
            self->terrainProfile[sampleIndex].v = message->payload2.pf[sampleIndex];
            if (self->terrainProfile[sampleIndex].v < minimumHeight) {
                __typeof__(self->terrainProfile[sampleIndex].v) sampledMinimumHeight = self->terrainProfile[sampleIndex].v; __typeof__(self->terrainProfile[sampleIndex].v) retainedMinimumHeight = sampledMinimumHeight; minimumHeight = retainedMinimumHeight;
            }
            if (self->terrainProfile[sampleIndex].v > maximumHeight) {
                __typeof__(self->terrainProfile[sampleIndex].v) sampledMaximumHeight = self->terrainProfile[sampleIndex].v; maximumHeight = sampledMaximumHeight;
            }
        }

        heightRange = maximumHeight - minimumHeight;
        heightMidpoint = kFloat_0_5_AuxMenu * heightRange + minimumHeight;
        if (heightRange > kScreenTerrainProfileRangeLimit) {
            float rangeForScaling = heightRange;
            displayScale = kScreenTerrainProfileRangeLimit / rangeForScaling;
        } else {
            displayScale = kFloat_1_AuxMenu;
        }
        __typeof__(self->terrainProfile[0].v) firstHeightForScaling = self->terrainProfile[0].v;
        __typeof__(self->terrainProfile[0].v) retainedFirstHeight = firstHeightForScaling;
        float midpointForScaling = heightMidpoint;
        float scaleForFirstHeight = displayScale;
        self->terrainProfile[0].v = scaleForFirstHeight * (kFloat_5_Screen * (kFloat_neg1_Screen * (retainedFirstHeight - midpointForScaling)));
        for (profileIndex = 1; profileIndex < 1 + self->terrainSampleCount; profileIndex++) {
            self->terrainProfile[profileIndex].v = displayScale * (kFloat_5_Screen * (kFloat_neg1_Screen * (self->terrainProfile[profileIndex].v - heightMidpoint)));
            float relativeHeight = self->terrainProfile[profileIndex].v - self->terrainProfile[0].v;
            if (relativeHeight < kScreenTerrainProfileColorMinDelta) {
                relativeHeight = kScreenTerrainProfileColorMinDelta;
            } else if (relativeHeight > kFloat_8_GolfScreen) {
                relativeHeight = kFloat_8_GolfScreen;
            }
            if (relativeHeight > kFloat_0_AuxMenu) {
                float colorProduct = kFloat_255_Screen * relativeHeight; int colorComponent = (int)(kFloat_255_Screen - colorProduct * kScreenTerrainProfileColorScale);
                self->terrainProfile[profileIndex].col = 0xFF00E500 | (colorComponent << 16) | colorComponent;
            } else {
                float colorProduct = kFloat_255_Screen * relativeHeight; unsigned int colorComponent = (int)(colorProduct * kScreenTerrainProfileColorScale + kFloat_255_Screen);
                self->terrainProfile[profileIndex].col = (colorComponent << 8) | (0xFFFF0000 | colorComponent);
            }
        }
    }
}

// Reset notices, reward/ball displays and auxiliary selection for the active player.
// Payload pointers are borrowed for this synchronous call; no pointer is retained.
static inline void ResetHudCurrentPlayerDisplays(HudMessageState* self, const ScreenMessage* message) {
    // Payload: none; uses current HUD/shared state.
    self->shotIndicatorPulse = kFloat_0_AuxMenu;
    if (g_helpMsg != 0) {
        NoticeQueue_ResetAndActivate(g_helpMsg);
    }
    if (self->pangInfo) {
        CPangInfo_ResetFromCurrentPlayer(self->pangInfo);
    }
    if (self->modeInfo) {
        self->modeInfo->m20();
    }
    if (self->ballPreview) {
        ScreenBallPreview_SelectCurrentPlayerBall(self->ballPreview);
    }
    {

        if (AuxiliaryMenuMessageView* auxiliaryMenu = self->auxiliaryMenu) {
            int zero = 0; auxiliaryMenu->selectionActive = zero;
            auxiliaryMenu->selectionTimer = kFloat_0_AuxMenu;
            auxiliaryMenu->selectedEntry = -1;
            CursorRegionStateView* cursorRegions = (CursorRegionStateView*)g_mouseCursor;
            cursorRegions->auxiliaryMenuRegionEnabled[0] = zero;

            cursorRegions->auxiliaryMenuRegionEnabled[1] = zero;
            cursorRegions->auxiliaryMenuRegionEnabled[2] = zero;
            cursorRegions->auxiliaryMenuRegionEnabled[3] = zero;
            cursorRegions->auxiliaryMenuRegionEnabled[4] = zero;
            cursorRegions->auxiliaryMenuRegionEnabled[5] = zero;
        }
    }
    self->meterFadeFrame = 0;
}

// Reset shot controls, party meter scripts and their loaded widget opacity.
// Payload pointers are borrowed for this synchronous call; no pointer is retained.
// Required live receivers are CStrokePoint2d, CBallWind2d, CGolfBase2d and
// CSwingResult2d; the optional calipers/status pointers are checked separately.
// Virtual protocol (byte slot -> implementation -> receiver effect):
// stroke +14 -> CStrokePoint2d_ClearShotSelections (stroke_point_2d_selector.cpp), clears directionSelection,
// confirmRequested, powerMode, specialShotSelection; +18 -> vfunc_4 stores its
// integer argument in inputLocked. The older local name strokePointHandle is a
// pointer-shaped ABI WORD, NOT a retained object: the player's +1DC value is
// copied to that integer lock. Neither call takes ownership; both return 1,
// ignored here. Do not turn the +18 argument into a dereference or retain.
// golfBase +64 -> CGolfBase2d_SetPangyaZoneWidth (golf_base_2d_hud_layout.cpp), sets pangyaZoneWidth=1;
// +68 -> vfunc_24, sets meterOverlayWidth=0. Both mark layoutDirty only on a
// changed float value and return 1 (ignored). These are layout scale/width values,
// not opacity or seconds; the receiver's refresh consumes them. Keep +60 target-X
// assignment and +5C action reset after these setters, as in the stock protocol.
// This reset does not destroy/recreate these receivers. A reset with payload0=0
// also enters HudAim; nonzero preserves phase. Party scripts and opacity updates
// below remain ordered after the widget state reset.
static inline void ResetHudShotPanels(HudMessageState* self, const ScreenMessage* message) {
    // Payload: p0=immediate int; zero also resets HUD phase to aim, nonzero preserves it.
    if (message->payload0.i == 0) {
        self->hudPhase = HudAim;
    }
    self->requestedPanel = 0;
    self->shotOptionResult = 0;
    self->shotDirectionResult = 0;
    self->shotOptionEnabled = 0;
    ((HudMessageSharedStateView*)g_sharedDoc)->h->cameraMode = 0;
    if (0 != self->calipers) {
        CCalipers_ResetMeasurement(self->calipers);
    }
    self->strokePointDisplay->ResetSelections();
    self->strokePointDisplay->SetInputLockWord(HudMessagePlayerSlot(g_sharedDoc, ((((HudMessageSharedStateView*)g_sharedDoc)->h->currentPlayer)))->strokePointHandle);
    if (6 > ((HudMessageSharedStateView*)g_sharedDoc)->h->tutorialStage || *(u8*)(*(char**)((char*)InstanceTable_Get() + 4) + 0xa) == 3) {
        self->ballWindDisplay->m24(0);
    } else {
        self->ballWindDisplay->m24(1);
    }
    self->golfBaseDisplay->SetPangyaZoneWidth(kFloat_1_AuxMenu);
    self->golfBaseDisplay->SetMeterOverlayWidth(kFloat_0_AuxMenu);
    self->golfBaseDisplay->m60(((HudMessageSharedStateView*)g_sharedDoc)->h->roundedShotGaugeCursorX);
    self->golfBaseDisplay->m5c();
    if (((CGolfBall*)GolfBall__Fv())->m_groundType == 2) {
        self->golfBaseDisplay->m28(1);
    } else {
        self->golfBaseDisplay->m28(0);
    }
    self->golfBaseDisplay->m1c(0);
    self->golfBaseDisplay->m20(0);
    self->swingResultDisplay->m20();
    self->swingFeedbackPending = 0;
    if (0 != self->ingameStatus) {
        CIngameStatus_RefreshPlayer(self->ingameStatus);
    }
    HudMessageSharedStateView* sharedState = (HudMessageSharedStateView*)g_sharedDoc;
    if (3 == sharedState->partyGameMode) {
        int partyItemType = PartyItemSlots_GetItemType(PartyItemSlots_GetInstance(), sharedState->h->currentPlayer);
        BpObject_SetEnabled(self->partyGolfBaseScript, 0);
        BpObject_SetEnabled(self->featherBadgeScript, 0);
        self->partyMeterIndex = partyItemType;
        switch (partyItemType) {
        case 0:
            BpObject_StartScriptByName(self->partyGolfBaseScript, screenOverlayStrings + ScreenText_RedMeterAnimation, 1);
            BpObject_SetEnabled(self->partyGolfBaseScript, 1);
            break;
        case 1:
            BpObject_StartScriptByName(self->partyGolfBaseScript, screenOverlayStrings + ScreenText_BlueMeterAnimation, 1);
            BpObject_SetEnabled(self->partyGolfBaseScript, 1);
            break;
        case 2:
            BpObject_StartScriptByName(self->partyGolfBaseScript, screenOverlayStrings + ScreenText_YellowMeterAnimation, 1);
            BpObject_SetEnabled(self->partyGolfBaseScript, 1);
            break;
        case 9:
            BpObject_StartScriptByName(self->featherBadgeScript, screenOverlayStrings + ScreenText_FeatherAnimation, 1);
            BpObject_SetEnabled(self->featherBadgeScript, 1);
            break;
        }
    }
    if (self->hudInitialized) {
        if (3 == ((HudMessageSharedStateView*)g_sharedDoc)->partyGameMode) { int meterGroupIndex = self->partyMeterIndex;
            Tri meterGroupNames; meterGroupNames = *(Tri*)partyMeterGroupNames;

            if (0 <= meterGroupIndex) { if ((u32)meterGroupIndex < 3) {
                    UIGroup_SetOpacity(BpObject_GetOrCreateNamedGroup(self->partyGolfBaseScript, (const char*)meterGroupNames.v[meterGroupIndex]), 0x7f);
                } }
        }
        self->meterOpacity = 0xff;
        self->ballWindDisplay->m64(-1);
        self->golfBaseDisplay->m50(-1);
    }
}

// Select match-play rewards when applicable, then notify the score actor.
// Payload pointers are borrowed for this synchronous call; no pointer is retained.
static inline void CalculateHudHoleRewards(HudMessageState* self, const ScreenMessage* message) {
    // Payload: none; uses current HUD/shared state.
    if (self->pangInfo) {
        switch (((HudMessageSharedStateView*)g_sharedDoc)->golfRuleType) {
        case 1:
            CPangInfo_CalculateMatchPlayHoleRewards(self->pangInfo);
            break;
        }
        IActor_DispatchMessage(self, self, screenOverlayStrings + ScreenText_ScoreActor, 5,(void*)(0),(void*)(0),(void*)(0),(void*)(0));
    }
}

// Start stage flashing or select the preceding stage; tutorial input may be delayed.
// Payload pointers are borrowed for this synchronous call; no pointer is retained.
static inline void StartHudTutorialStage(HudMessageState* self, const ScreenMessage* message) {
    // Payload: p0=immediate int: nonzero starts current stage flashing; zero selects previous stage.
    if (message->payload0.i != 0) {
        self->tutorialFlashOn = 0;
        self->tutorialFlashCount = 0;
        self->tutorialFlashTimer = kFloat_0_AuxMenu;
        self->previousTutorialStage = ((HudMessageSharedStateView*)g_sharedDoc)->h->tutorialStage;
        if (((HudMessageSharedStateView*)g_sharedDoc)->h->tutorialStage < 5) {
            if (((HudMessageSharedStateView*)g_sharedDoc)->golfRuleType == 2) {
                self->tutorialPointerDelay = kFloat_0_AuxMenu;
                self->tutorialPointerDelayed = 1;
                ((CursorRegionStateView*)g_mouseCursor)->tutorialPointerRegionEnabled = 0;
            }
        }
    } else {
        self->tutorialFlashOn = 0;
        self->previousTutorialStage = ((HudMessageSharedStateView*)g_sharedDoc)->h->tutorialStage - 1;
    }
}

// Apply the tutorial completion command without retaining payload pointers.
// Payload pointers are borrowed for this synchronous call; no pointer is retained.
static inline void SetHudTutorialCompletion(HudMessageState* self, const ScreenMessage* message) {
    // Payload: p0=immediate int: 0 clear/restart, 1 clear/finish flashing, 2 mark visible.
    switch (message->payload0.i) {
    case TutorialCompletionRestart: // Clear this stage and restart its completion timer.
        self->tutorialStageVisible[((HudMessageSharedStateView*)g_sharedDoc)->h->tutorialStage] = 0;
        self->tutorialCompletionTimer = kFloat_0_AuxMenu;
        self->tutorialCompletionPhase = 0;
        break;
    case TutorialCompletionFinishFlashing: // Clear this stage and finish the flashing phase.
        self->tutorialStageVisible[((HudMessageSharedStateView*)g_sharedDoc)->h->tutorialStage] = 0;
        self->tutorialCompletionPhase = 3;
        break;
    case TutorialCompletionMarkVisible: // Mark the current stage visible.
        self->tutorialStageVisible[((HudMessageSharedStateView*)g_sharedDoc)->h->tutorialStage] = 1;
        break;
    }
}

// Activate the requested menu cursor region and move the pointer in viewport coordinates.
// Payload pointers are borrowed for this synchronous call; no pointer is retained.
static inline void SelectHudAuxiliaryMenuEntry(
    HudMessageState* self, const ScreenMessage* message, int& entryIndex, AuxiliaryMenuMessageView*& auxiliaryMenu) {
    // Payload: p0=immediate entry index; used to index cursor regions without validation.
    {   auxiliaryMenu = self->auxiliaryMenu; if (auxiliaryMenu) { int enabled = 1; entryIndex = message->payload0.i;
            auxiliaryMenu->selectionActive = enabled;
            auxiliaryMenu->selectedEntry = entryIndex;
            int regionEntryIndex = entryIndex;
            int regionEnabled = enabled;
            ((CMouseCursor*)g_mouseCursor)->regionEnabled[0xe3 - 0xbd + regionEntryIndex] = regionEnabled;
            float viewportHeight = ((ViewportSizeView*)g_view)->viewportHeight;
            float viewportWidth = ((ViewportSizeView*)g_view)->viewportWidth;  CMouseCursor_MoveTo(g_mouseCursor,
            viewportWidth - kFloat_50_Screen,
            kScreenCommandAndMenuRowSpacing * (kFloat_0_5_AuxMenu + (float)(entryIndex - 3)) + kFloat_0_5_AuxMenu * viewportHeight,
            kFloat_neg1_Screen);
        }
    }
}

#pragma pop

// Dispatch HUD messages for shot feedback, rewards, tutorial panels, drop mode, and visibility.
void CScreen_HandleMsg(HudMessageState* self, const ScreenMessage* message) {
    // Keep these four distinct vector roles in original declaration order for the Wii stack ABI.

    Vec3 groundNormal;
    HudMessagePositionWords dropGroundQueryPosition;
    WVector dropGroundPosition;
    HudMessagePositionWords dropBoundsTestPosition;

    switch (message->id) {
    case ScreenMsg_EnterDropMode: // Set up drop bounds/camera and activate the map panel; world-space scratch stays caller-owned.
        EnterHudDropMode(self, message, dropBoundsTestPosition, dropGroundQueryPosition, dropGroundPosition);
        break;
    case ScreenMsg_QueryDropMode:
        // Payload: p0=int* output DropMode; not retained.
        *message->payload0.pi = self->dropModeActive;
        break;
    case ScreenMsg_ExitDropMode:
        // Payload: none; uses current HUD/shared state.
    case ScreenMsg_CancelDropMode:
        // Payload: none; uses current HUD/shared state.
        self->dropModeActive = 0;
        if (0 != self->mapCameraDisplay) {
            self->mapCameraDisplay->m40(0);
        }
        ((HudMessageSharedStateView*)g_sharedDoc)->h->cameraMode = 0;
        break;
    case ScreenMsg_SetCurrentPlayerDisplayValue:
        // Payload: none; uses current HUD/shared state.
        if (self->nearestPinInfo != 0) {
            Hud_SetCurrentPlayerDisplayValue(self->nearestPinInfo, (((HudMessageSharedStateView*)g_sharedDoc)->h->f3bc));
        }
        break;
    case ScreenMsg_RefreshNearestPinPlayer:
        // Payload: none; uses current HUD/shared state.
        if (self->nearestPinInfo != 0) {
            Hud_UpdateNearestPinPlayer(self->nearestPinInfo);
        }
        break;
    case ScreenMsg_RequestShotTransition:
        // Payload: none; uses current HUD/shared state.
        self->shotTransitionRequested = 1;
        if (0 != self->holeInfoSet) {
            self->holeInfoSet->shotTransitionInfo->m1c();
        }
        break;
    case ScreenMsg_SetBallRingHeight:
        // Payload: p0=float* input height, copied only when ballPreview exists.
        if (self->ballPreview != 0) {
            float v = *message->payload0.pf;
            self->ballRingHeight = v;
            self->ballPreview->ballRingHeight = v;
        }
        break;
    case ScreenMsg_AddBallRingHeight:
        // Payload: p0=float* input height increment, applied only when preview exists.
        if (0 != self->ballPreview) {
            float d = *message->payload0.pf;
            float v = self->ballRingHeight + d;
            self->ballRingHeight = v;
            self->ballPreview->ballRingHeight = v;
        }
        break;
    case ScreenMsg_RefreshGroundLie: // Refresh the ball ring normal and lie blend from the current ground triangle.
        RefreshHudGroundLie(self, message, groundNormal);
        break;
    case ScreenMsg_SetFirstHoleInfoDisplayEnabled:
        // Payload: p0=immediate int enable (nonzero).
        if (0 != self->holeInfoSet) {
            self->holeInfoSet->firstDisplayEnabled = ((message->payload0.i != 0));
        }
        break;
    case ScreenMsg_SetSecondHoleInfoDisplayEnabled:
        // Payload: p0=immediate int enable (nonzero).
        if (self->holeInfoSet) {
            self->holeInfoSet->secondDisplayEnabled = (((0 != message->payload0.i)));
        }
        break;
    case ScreenMsg_ShowShotGauge:
        // Payload: none; uses current HUD/shared state.
        self->hudFlags |= HudShotGaugeVisible;
        break;
    case ScreenMsg_HideShotGauge:
        // Payload: none; uses current HUD/shared state.
        self->hudFlags &= ~HudShotGaugeVisible;
        break;
    case ScreenMsg_ShowTimedNotification:
        // Payload: none; uses current HUD/shared state.
        self->displayTimeRemaining = kFloat_1_AuxMenu;
        break;
    case ScreenMsg_SetWindStrengthAndHeading: {
            // Payload: p0=immediate int strength; p1=float* input heading.
            int v = message->payload0.i;
            self->windStrength = v;
            self->windHeading = *message->payload1.pf;
            if (self->windInfo != 0) {
                self->windInfo->windStrength = v;
            }
            break;
        }
    case ScreenMsg_SetShotPowerColor: // Interpolate the packed feedback color; a zero blend also clears the shot option.
        {
            float* blendPayload;
            u8 blendWeight;
            u32 inverseBlendWeight;
            SetHudShotPowerColor(self, message, blendPayload, blendWeight, inverseBlendWeight);
            break;
        }
    case ScreenMsg_ShowShotFeedback:
        // Payload: none; uses current HUD/shared state.
        if (self->holeInfoSet != 0) {
            CAccuracyInfo_ShowCurrentShotFeedback(self->holeInfoSet->accuracyInfo);
        }
        break;
    case ScreenMsg_ShowFullPower:
        // Payload: none; uses current HUD/shared state.
        if (self->accuracyInfo != 0) {
            CAccuracyInfo_ShowFullPower(self->accuracyInfo);
        }
        break;
    case ScreenMsg_SetFeedbackOpacity: // Copy two borrowed opacity payloads into the feedback colors' alpha bytes.
        SetHudFeedbackOpacity(self, message);
        break;
    case ScreenMsg_SetTerrainProfile: // Copy height samples, compress their range and color offsets relative to the first sample.
        {
            int sampleIndex;
            int profileIndex;
            int additionalSampleCount;
            int profileKind;
            float* heights;
            float displayScale;
            float heightRange;
            float minimumHeight;
            float heightMidpoint;
            BuildHudTerrainProfile(
                self, message, sampleIndex, profileIndex, additionalSampleCount, profileKind, heights, displayScale,
                heightRange, minimumHeight, heightMidpoint);
            break;
        }
    case ScreenMsg_ResetForCurrentPlayer: // Reset notices, reward/ball displays and auxiliary selection for the active player.
        ResetHudCurrentPlayerDisplays(self, message);
        break;
    case ScreenMsg_ResetShotPanels: // Reset shot controls, party meter scripts and their loaded widget opacity.
        ResetHudShotPanels(self, message);
        break;
    case ScreenMsg_EnablePointerShotInput:
        // Payload: none; uses current HUD/shared state.
        self->hudFlags |= HudPointerShotInputEnabled;
        break;
    case ScreenMsg_DisablePointerShotInput:
        // Payload: none; uses current HUD/shared state.
        self->hudFlags &= ~HudPointerShotInputEnabled;
        break;
    case ScreenMsg_ResetShotFeedbackBlend:
        // Payload: none; uses current HUD/shared state.
        self->shotFeedbackBlend = kFloat_0_AuxMenu;
        break;
    case ScreenMsg_ResetHoleInfo:
        // Payload: none; uses current HUD/shared state.
        if (self->holeInfoSet) {
            self->holeInfoSet->holeInfo->m1c();
        }
        break;
    case ScreenMsg_CalculateShotRewards:
        // Payload: p0=immediate int option, forwarded as nonzero boolean.
        if (0 != self->pangInfo) {
            CPangInfo_CalculateShotRewards(self->pangInfo, (((message->payload0.i != 0))));
        }
        break;
    case ScreenMsg_ResetRewardAnimations:
        // Payload: none; uses current HUD/shared state.
        if (self->pangInfo) {
            CPangInfo_ResetBonusAnimations(self->pangInfo);
        }
        break;
    case ScreenMsg_FinishRewardTotals:
        // Payload: none; uses current HUD/shared state.
        if (self->pangInfo) {
            CPangInfo_FinishDisplayedTotals(self->pangInfo);
        }
        break;
    case ScreenMsg_CalculateHoleRewards: // Select match-play rewards when applicable, then notify the score actor.
        CalculateHudHoleRewards(self, message);
        break;
    case ScreenMsg_QueryRewardDisplayComplete:
        // Payload: p0=unsigned char* output completion flag (one byte).
        *message->payload0.pb = 0;
        if (!self->pangInfo || (self->pangInfo->displayFlags & 0x10)) {
            if (0 == self->modeInfo || self->modeInfo->m24() != 0) {
                *message->payload0.pb = 1;
            }
        }
        break;
    case ScreenMsg_LoadBallResource:
        // Payload: p0=immediate int resource ID, forwarded if preview exists.
        if (self->ballPreview) {
            Screen_GetOrLoadBallResource(self->ballPreview, message->payload0.i);
        }
        break;
    case ScreenMsg_StartTutorialStage: // Start stage flashing or select the preceding stage; tutorial input may be delayed.
        StartHudTutorialStage(self, message);
        break;
    case ScreenMsg_ResetTutorialTitle:
        // Payload: none; uses current HUD/shared state.
        self->tutorialTitlePhase = 0;
        self->tutorialTitleTimer = kFloat_0_AuxMenu;
        self->modeLabelOpacity = 0;
        break;
    case ScreenMsg_SetTutorialCompletion: // Apply the tutorial completion command without retaining payload pointers.
        SetHudTutorialCompletion(self, message);
        break;
    case ScreenMsg_ResetIngameStatus:
        // Payload: none; uses current HUD/shared state.
        if (self->ingameStatus) {
            CIngameStatus_ResetState(self->ingameStatus);
        }
        break;
    case ScreenMsg_FlashHoleInfo:
        // Payload: none; uses current HUD/shared state.
        if (0 != self->holeInfoSet) {
            CHoleInfo_StartFlash(self->holeInfoSet->holeInfo);
        }
        break;
    case ScreenMsg_SelectAuxiliaryMenuEntry: // Activate the requested menu cursor region and move the pointer in viewport coordinates.
        {
            int entryIndex;
            AuxiliaryMenuMessageView* auxiliaryMenu;
            SelectHudAuxiliaryMenuEntry(self, message, entryIndex, auxiliaryMenu);
            break;
        }
    case ScreenMsg_ClearAuxiliaryMenuSelection:
        // Payload: none; uses current HUD/shared state.
        if (0 != self->auxiliaryMenu) {
            self->auxiliaryMenu->selectedEntry = -1;
        }
        break;
    case ScreenMsg_QueryBallPreviewBasisRow:
        // Payload: p0=WVector* output second basis row (m[3..5]), NOT translation; left untouched if preview absent.
        if (self->ballPreview) {
            float m[12];
            CScreen_BuildOrientedBasisMatrix(m, self->ballPreview);
            *(WVector*)message->payload0.p = *(WVector*)(3 + m);
        }
        break;
    case ScreenMsg_QueryGroundLieBlend:
        // Payload: p0=float* output ground-lie blend.
        *message->payload0.pf = self->groundLieBlend;
        break;
    case ScreenMsg_SetHudDisplayEnabled:
        // Payload: p0=immediate int; only exactly 1 enables display.
        self->hudDisplayEnabled = 1 == message->payload0.i;
        break;
    case ScreenMsg_ShowIngameStatus:
        // Payload: none; uses current HUD/shared state.
        if (0 != self->ingameStatus) {
            CIngameStatus_BeginShow(self->ingameStatus);
        }
        break;
    case ScreenMsg_HideIngameStatus:
        // Payload: none; uses current HUD/shared state.
        if (0 != self->ingameStatus) {
            CIngameStatus_BeginHide(self->ingameStatus);
        }
        break;
    case ScreenMsg_ShowCornerLabels:
        // Payload: none; uses current HUD/shared state.
        if (0 != self->holeInfoSet) {
            CHoleInfo_ShowCornerLabels(self->holeInfoSet->holeInfo);
        }
        break;
    case ScreenMsg_SnapHoleDistance:
        // Payload: none; uses current HUD/shared state.
        if (0 != self->holeInfoSet) {
            CHoleCupInfo_SnapDistanceOverlay(self->holeInfoSet->cupInfo);
        }
        break;
    case ScreenMsg_SetHoleInfoValues:
        // Payload: p0/p1=immediate ints forwarded to cup-info m18; their units remain unresolved.
        if (0 != self->holeInfoSet) {
            self->holeInfoSet->cupInfo->m18(message->payload0.i, message->payload1.i);
        }
        break;
    case ScreenMsg_RefreshIngamePlayer:
        // Payload: none; uses current HUD/shared state.
        if (self->ingameStatus) {
            CIngameStatus_RefreshPlayer(self->ingameStatus);
        }
        break;
    case ScreenMsg_StartSkinsResult:
        // Payload: p0/p1=immediate ints forwarded to SkinsInfo_StartResultDisplay.
        if (self->skinsInfo) {
            SkinsInfo_StartResultDisplay(self->skinsInfo, message->payload0.i, message->payload1.i);
        }
        break;
    case ScreenMsg_ResetShotCounterFade:
        // Payload: none; uses current HUD/shared state.
        if (0 != self->ingameStatus) {
            CIngameStatus_ResetShotCounterFade(self->ingameStatus);
        }
        break;
    case ScreenMsg_QueryHudPhase:
        // Payload: p0=int* output HudPhase; not retained.
        *message->payload0.pi = self->hudPhase;
        break;
    case ScreenMsg_QueryStrokePanelReady:
        // Payload: p0=int* output StrokePanelReady; not retained.
        if (self->strokePointDisplay != 0) {
            if (self->strokePointDisplay->m70() != 0) {
                *message->payload0.pi = 1;
            } else {
                *message->payload0.pi = 0;
            }
        } else {
            *message->payload0.pi = 0;
        }
        break;
    case ScreenMsg_QueryShotOptionResult:
        // Payload: p0=int* output ShotOptionResult; not retained.
        *message->payload0.pi = self->shotOptionResult;
        break;
    case ScreenMsg_QueryShotDirectionResult:
        // Payload: p0=int* output ShotDirectionResult; not retained.
        *message->payload0.pi = self->shotDirectionResult;
        break;
    case ScreenMsg_ConsumeShotOptionChanged: {
            // Payload: p0=int* output previous change latch; clears that latch after writing it.
            int* p = message->payload0.pi;
            *p = self->shotOptionChanged;
            self->shotOptionChanged = 0;
            break;
        }
    case ScreenMsg_QueryStrokeOption:
        // Payload: p0=int* output StrokeOption; not retained.
        *message->payload0.pi = self->strokePointDisplay->GetStrokeOption();
        break;
    case ScreenMsg_QueryShotOptionEnabled:
        // Payload: p0=int* output ShotOptionEnabled; not retained.
        *message->payload0.pi = self->shotOptionEnabled;
        break;
    case ScreenMsg_SetShotOptionEnabled: {
            // Payload: p0=immediate int nonzero enable; also sets shotOptionChanged to that boolean.
            int v = ((((message->payload0.i != 0))));
            self->shotOptionEnabled = v;
            self->shotOptionChanged = v;
            break;
        }
    case ScreenMsg_ResumeAim:
        // Payload: none; uses current HUD/shared state.
        self->hudPhase = HudAim;
        CMouseCursor_SetPointerInputEnabled(g_mouseCursor, 1);
        self->resumeAimRequested = 1;
        break;
    case ScreenMsg_BlockCameraInput:
        // Payload: none; uses current HUD/shared state.
        if (0 != ((HudMessageSharedStateView*)g_sharedDoc)->h->cameraMode) {
            self->hudInputBlocked = 1;
        }
        break;
    case ScreenMsg_QueryMapCameraState:
        // Payload: p0=int* output MapCameraState; not retained.
        *message->payload0.pi = self->mapCameraDisplay->GetMapCameraState();
        break;
    case ScreenMsg_QueryPanelsIdle: {
            // Payload: p0=int* output PanelsIdle; not retained.
            unsigned int a = self->golfBaseDisplay->IsPanelIdle();

            *message->payload0.pi = a & (self->mapCameraDisplay->IsPanelIdle());
            break;
        }
    case ScreenMsg_QueryAimPanelState:
        // Payload: p0=int* output AimPanelState; not retained.
        *message->payload0.pi = self->golfBaseDisplay->GetAimPanelState();
        break;
    case ScreenMsg_QueryMapCameraSelection:
        // Payload: p0=int* output MapCameraSelection; not retained.
        *message->payload0.pi = self->mapCameraDisplay->GetMapCameraSelection();
        break;
    case ScreenMsg_QueryCalipersPreview:
        // Payload: p0=int* output CalipersPreview; not retained.
        if (self->calipers) {
            *message->payload0.pi = CCalipers_IsPreviewActive(self->calipers);
        } else {
            *message->payload0.pi = 0;
        }
        break;
    case ScreenMsg_QueryAlwaysZero:
        // Payload: p0=int* output; this query always reports 0 (no state is read). Not retained.
        *message->payload0.pi = 0;
        break;
    case ScreenMsg_QueryPanelTransitionsReady:
        // Payload: p0=int* output PanelTransitionsReady; not retained.
        *message->payload0.pi = self->panelsReady;
        break;
    case ScreenMsg_SetAimControlsEnabled:
        // Payload: p0=immediate int aim-control gate.
        self->aimControlsEnabled = message->payload0.i;
        break;
    case ScreenMsg_QueryCameraOrBallControlActive: {
            // Payload: p0=int* output CameraOrBallControlActive; not retained.
            unsigned int a = self->mapCameraDisplay->m30();

            *message->payload0.pi = a | (self->ballWindDisplay->m28());
            break;
        }
    case ScreenMsg_QueryItemPanelReady:
        // Payload: p0=int* output ItemPanelReady; not retained.
        *message->payload0.pi = self->itemPanelReady;
        break;
    case ScreenMsg_ResetAimPanelAction:
        // Payload: none; uses current HUD/shared state.
        self->golfBaseDisplay->ResetAimPanelAction();
        break;
    case ScreenMsg_ResetMapCameraAction:
        // Payload: none; uses current HUD/shared state.
        self->mapCameraDisplay->ResetAction();
        break;
    case ScreenMsg_ConfirmMapCameraAction:
        // Payload: none; uses current HUD/shared state.
        self->mapCameraDisplay->ConfirmAction();
        break;
    case ScreenMsg_ShowTutorialMeter:
        // Payload: none; uses current HUD/shared state.
        self->tutorialMeterEnabled = 1;
        break;
    case ScreenMsg_ShowTutorialMeterImmediately:
        // Payload: none; uses current HUD/shared state.
        self->tutorialMeterEnabled = 1;
        self->tutorialMeterBlinkCount = 6;
        break;
    case ScreenMsg_ShowTutorialGauge:
        // Payload: none; uses current HUD/shared state.
        self->tutorialGaugeEnabled = 1;
        break;
    case ScreenMsg_ShowTutorialGaugeImmediately:
        // Payload: none; uses current HUD/shared state.
        self->tutorialGaugeEnabled = 1;
        self->tutorialGaugeBlinkCount = 6;
        break;
    case ScreenMsg_ShowTutorialAim:
        // Payload: none; uses current HUD/shared state.
        self->tutorialAimEnabled = 1;
        break;
    case ScreenMsg_ShowTutorialAimImmediately:
        // Payload: none; uses current HUD/shared state.
        self->tutorialAimEnabled = 1;
        self->tutorialAimBlinkCount = 6;
        self->golfBaseDisplay->m7c();
        break;
    case ScreenMsg_HideTutorialMeter:
        // Payload: none; uses current HUD/shared state.
        self->tutorialMeterEnabled = 0;
        self->tutorialMeterBlinkFrame = 0;
        self->tutorialMeterBlinkCount = 0;
        break;
    case ScreenMsg_HideTutorialGauge:
        // Payload: none; uses current HUD/shared state.
        self->tutorialGaugeEnabled = 0;
        self->tutorialGaugeBlinkFrame = 0;
        self->tutorialGaugeBlinkCount = 0;
        break;
    case ScreenMsg_HideTutorialAim:
        // Payload: none; uses current HUD/shared state.
        self->tutorialAimEnabled = 0;
        self->tutorialAimBlinkFrame = 0;
        self->tutorialAimBlinkCount = 0;
        break;
    case ScreenMsg_SetTutorialClubChangeEnabled:
        // Payload: p0=immediate int club-change gate.
        self->tutorialClubChangeEnabled = message->payload0.i;
        break;
    case ScreenMsg_SetTutorialAimControlEnabled:
        // Payload: p0=immediate int tutorial aim gate.
        self->tutorialAimControlEnabled = message->payload0.i;
        break;
    case ScreenMsg_SetTutorialSpinControlEnabled:
        // Payload: p0=immediate int tutorial spin gate.
        self->tutorialSpinControlEnabled = message->payload0.i;
        break;
    case ScreenMsg_ShowTutorialIllustration:
        // Payload: p0=immediate illustration selector; current-player shared handle is supplied internally.
        if (self->tutorialIllustration) {
            self->tutorialIllustration->m1c(message->payload0.i,
            HudMessagePlayerSlot(g_sharedDoc, ((HudMessageSharedStateView*)g_sharedDoc)->h->currentPlayer)->illustrationHandle);
            self->tutorialIllustration->m24();
            self->tutorialIllustrationActive = 1;
        }
        break;
    case ScreenMsg_SetCameraControlsEnabled:
        // Payload: p0=immediate int camera-control gate.
        self->cameraControlsEnabled = message->payload0.i;
        break;
    case ScreenMsg_SuspendHudDrawing:
        // Payload: none; uses current HUD/shared state.
        {

            unsigned int zero;
            zero = 0; self->hudEnabled = zero;
            if (0 == Sing_805DC5F0()) {
                if (0 == s_bpCtrlInitialized) {
                    __ct__7CBpCtrlFv(s_bpCtrl);
                    __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, s_bpCtrlDtorRegistration_AuxMenu);
                    s_bpCtrlInitialized = 1;
                }
                Sing_805DC5F0() = s_bpCtrl;
            }
            CBpCtrl_SetDrawingEnabled(Sing_805DC5F0(), zero);
        }
        break;
    case ScreenMsg_ResumeHudDrawing: {
            // Payload: none; uses current HUD/shared state.
            void* ctrl = Sing_805DC5F0();
            self->hudEnabled = 1;
            if (0 == ctrl) {
                if (0 == s_bpCtrlInitialized) {
                    __ct__7CBpCtrlFv(s_bpCtrl);
                    __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, s_bpCtrlDtorRegistration_AuxMenu);
                    s_bpCtrlInitialized = 1;
                }
                Sing_805DC5F0() = s_bpCtrl;
            }
            CBpCtrl_SetDrawingEnabled(Sing_805DC5F0(), 1);
            break;
        }
    }
}

}

// In-round screen/HUD coordinator: owns child panels and named overlays, dispatches
// actor messages, presents shot input/rewards/tutorial prompts, and draws overlays.
// Entry index for local mods (ABI names/addresses remain in symbols.txt):
// - Lifecycle/ownership: CScreen::CScreen() -> OnPreLoadInit -> OnLoad/PollLoadComplete
//   -> InitializeLoadedLayouts -> OnProcess/OnDisplay -> OnDestroy. Children and
//   registry overlay payloads are released before effect attachments on teardown.
// - Controller/shot gauge: ScreenMessageId + CScreen::HandleMsg route commands;
//   HudPhase/HudPanel and OnProcess's named phases coordinate panel admission,
//   input, player/equipment reconciliation and feedback. Query payloads are
//   caller-owned writable storage; follow the individual switch arm's type.
// - Camera buttons: CScreen::UpdateCameraAndShotHints and HudMapCameraInterface;
//   CMapCamera2d implementations in map_camera_2d_panel.cpp define its latches/transition clock.
// - Frame rate/animations: OnProcess dt drives timers/blends, while panel update
//   methods may count calls. Preserve both clocks; this unit sets no retrace rate.
// - Widescreen/placement: OnDisplay, DrawClubModelOverlay, UpdateSwingCaption and
//   tutorial title code use WView viewport pixels. Change layout anchors with
//   their hit areas/projection consumers; the shared scalar pool is the data section.
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
#include "classes/CScreen.h"
#define AsCScreen(p) ((CScreen*)(p))
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
#include "classes/CWind.h"
#include "classes/CQuadTree.h"
#include "classes/CHoleInfo.h"
#include "classes/CPangInfo.h"
#include "classes/CIngameStatus.h"
#include "game/draw_drive_line_match.h"
#include "classes/CAccuracyInfo.h"
#include "classes/CDistToHoleInfo.h"
#include "classes/CHoleCupInfo.h"
#include "classes/CHoleInfoSet.h"
#include "classes/CCalipers.h"
#include "classes/CBpCtrl.h"
#include "classes/CFontNumDraw.h"
#include "classes/CAuxMenuUI.h"
#include "classes/CBalloonInfo.h"
#include "classes/CDriveInfo.h"
#include "classes/CFlyInfo.h"
#include "classes/CHelpMsg.h"
#include "classes/CPowerInfo.h"
#include "classes/CRemainTimeInfo.h"
#include "classes/CTutorialIllust2d.h"
#include "classes/CWindInfo.h"

extern "C" {
void ScreenOverlayRegistry_CreateNamedOverlay(void*, const char*, void*, void*);
extern CSharedDoc* g_sharedDoc;
extern WView* g_view;
extern void *g_drawDriveLine, *g_mouseCursor;
COption* g_option;
}

extern "C" {

void* g_chatMsg;

struct S8017A418 {
    float x, y, z, w;
};

signed char s_bpCtrlInitialized;
extern char s_bpCtrl[];
extern char s_bpCtrlDtorRegistration_AuxMenu[];
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
enum TutorialTitlePhase {
    TutorialTitleFadingIn = 0,
    TutorialTitleMoving = 1,
    TutorialTitleArrived = 2
};
enum TutorialCompletionCommand {
    TutorialCompletionRestart = 0,
    TutorialCompletionFinishFlashing = 1,
    TutorialCompletionMarkVisible = 2
};
enum HudPhase {
    HudInactive = 0,
    HudShot = 1,
    HudAim = 2,
    HudItems = 3,
    HudSpin = 4,
    HudShotTransition = 5
};
// Panel selection is separate from HudPhase: requestedPanel is the desired widget
// group; previousPanel tracks the group being retired. 0 hides panels, 1 opens
// the aim group, 2 closes aim for a shot, 4 opens camera, 1000 items, 2000 spin.
// High item/spin bits survive low-nibble panel changes in OnProcess. Value 3
// occurs as an ignored previous-panel case; its independent role is unresolved.
enum HudPanel {
    HudPanelNone = 0,
    HudPanelAim = 1,
    HudPanelAimClosing = 2,
    HudPanelCamera = 4,
    HudPanelItems = 0x1000,
    HudPanelSpin = 0x2000
};
// Bits of requestedPanel above the low panel nibble (items/spin); preserved when the nibble is replaced.
enum HudPanelMasks {
    HudPanelHighBitsMask = 0xFFF0
};
// Tutorial aim/meter/gauge blink counters stop counting once they reach this value.
enum HudBlink {
    TutorialBlinkLimit = 6
};
enum HudFlag {
    HudShotGaugeVisible = 1,
    HudPointerShotInputEnabled = 2
};
enum HudGameStateMask {
    HudActiveLayoutState = 0x08000000,
    HudSuppressDrawingState = 0x2000,
    HudMeterFadeStates = 0x1080,
    HudItemMenuAllowedStates = 0x44308,
    HudCommandGlyphStates = 0x60
};
enum HudLimits {
    TutorialStageCount = 6,
    ClubOverlayCount = 17,
    TimebombFramesPerTick = 18,
    FullOpacity = 255
};
enum HudLogicalAction {
    HudActionOpenItems = 7,
    HudActionCamera = 0x18,
    HudActionHoldCamera = 0x19,
    HudActionAim = 0x1A,
    HudActionSpin = 0x1F,
    HudActionCancelPanel = 0x23
};
// The blink cycles below count update calls; do not convert them to elapsed-time timers.
enum PululuAnimationPhase {
    PululuHidden = 0,
    PululuEntering = 1,
    PululuLooping = 2,
    PululuLeaving = 3
};

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
extern void ScreenBallPreview_Ctor(void*);
extern void* g_resourceManager;

extern const float kScreenPulseWrapThreshold; // 20.0f: positive wrap threshold for the shot indicator pulse
extern const float kScreenPulseWrapSpan;      // 40.0f: pulse wrap span / command glyph Y offset
extern const float kFloat_50_Screen;
extern const float kFloat_5_Screen;
extern const float kScreenTutorialTitleRowHeight; // 24.0f: tutorial title row height
extern const float kScreenTutorialTitleWidth;     // 177.0f: tutorial title rectangle width
extern const float kScreenTutorialBlinkInterval;  // 0.25f: tutorial blink interval

extern void Layout_Destroy(void*, int);
extern void UITextTable_ClearEntries(void*);
extern void PointerArray_DeleteValue(void*, void*);
extern void ScreenBallPreview_dtor(void*, int);
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
void* CBpCtrl_GetInstance();
int Option_UseIntegratedClubOverlay();
S8017A418* Vec4_Set(S8017A418* arg0, float a, float b, float c, float d);
float Option_GetHudBaselineY(void* arg0);
int Option_PlaceWindIndicatorOnLoad();
float WView_GetViewportWidth(void* arg0);
void* ChatMessage_GetDefaultFontResource(void* arg0);
void* HoleInfoSet_ShowCornerLabels(void* arg0);
void* Option_GetInstance();
void* ChatMessage_GetCurrent();

struct InstanceLock_u80179180 {
    void* lock;
    // Hold one registry mutex level for the lifetime of this guard.
    InstanceLock_u80179180(void* l) {
        lock = l;
        OSLockMutex((OSMutex*)(l));
    }
    // Release the registry mutex level acquired by this guard.
    ~InstanceLock_u80179180() {
        OSUnlockMutex((OSMutex*)(lock));
    }
};

struct OverlayRegistryNodeView {
    void* value;
    const char* key;
    unsigned char flag;
    char padding[3];
    void* next;
    void* previous;
    void* bucketNext;
};
struct NamedOverlayView {
    char name[32];
    void* resource;
};
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
struct PowerInfoView {
    char padding0[0xC];
    unsigned int color;
    char padding10[8];
    WVector position;
};
struct BallPreviewMaterialView {
    char padding[0x1AC];
    void* material;
};
struct ClubResourceSlots {
    char padding[0x14];
    void* byClubKind[4];
};

struct ChatMessageView {
    char padding[0x10];
    void* defaultFontResource;
};
struct HoleInfoSetView {
    void* cup;
    void* distance;
    void* cornerLabels;
    void* accuracy;
};
template <int VtableOffset> struct ScreenVtableView {
    char padding[VtableOffset];
    void** vtable;
};

struct VDel0 {
    virtual void del(int);
};
struct VDel330 {
    char pad[0xe0];
    virtual void del(int);
};
struct VDel334 {
    char pad[0x9c];
    virtual void del(int);
};
struct VDel338 {
    char pad[0xa4];
    virtual void del(int);
};
struct VDel33C {
    char pad[0x164];
    virtual void del(int);
};
struct VDel340 {
    char pad[0x74];
    virtual void del(int);
};
struct VDel344 {
    char pad[0x78];
    virtual void del(int);
};

#pragma use_lmw_stmw on
#pragma opt_propagation off
#pragma opt_common_subs off
} // extern "C"

// Destroy HUD layouts and scripts, release resources, and clear the named-overlay registry.
void CScreen::OnDestroy() {
    // Phase 1: destroy core HUD children and clear their retained pointers.
    if (layout) {
        Layout_Destroy(layout, 1);
        layout = 0;
    }
    if (strokePointDisplay) {
        if (strokePointDisplay)
            ((VDel330*)strokePointDisplay)->del(1);
        strokePointDisplay = 0;
    }
    if (mapCameraDisplay) {
        if (mapCameraDisplay)
            ((VDel334*)mapCameraDisplay)->del(1);
        mapCameraDisplay = 0;
    }
    if (ballWindDisplay) {
        if (ballWindDisplay)
            ((VDel338*)ballWindDisplay)->del(1);
        ballWindDisplay = 0;
    }
    if (golfBaseDisplay) {
        if (golfBaseDisplay)
            ((VDel33C*)golfBaseDisplay)->del(1);
        golfBaseDisplay = 0;
    }
    if (swingResultDisplay) {
        if (swingResultDisplay)
            ((VDel340*)swingResultDisplay)->del(1);
        swingResultDisplay = 0;
    }
    if (tutorialIllustration) {
        if (tutorialIllustration)
            ((VDel344*)tutorialIllustration)->del(1);
        tutorialIllustration = 0;
    }
    // Phase 2: remove BP script entries, then release optional panels/resources.
    if (ballWindScriptId != -1) {
        if (!((int&)Sing_805DC5F0())) {
            if (!s_bpCtrlInitialized) {
                ((CBpCtrl*)s_bpCtrl)->__ct();
                __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, s_bpCtrlDtorRegistration_AuxMenu);
                s_bpCtrlInitialized = 1;
            }
            ((int&)Sing_805DC5F0()) = (int)s_bpCtrl;
        }
        ((CBpCtrl*)(((int&)Sing_805DC5F0())))->DeleteEntryById(ballWindScriptId);
        ballWindScriptId = -1;
    }
    if (partyGolfBaseScriptId != -1) {
        if (!((int&)Sing_805DC5F0())) {
            if (!s_bpCtrlInitialized) {
                ((CBpCtrl*)s_bpCtrl)->__ct();
                __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, s_bpCtrlDtorRegistration_AuxMenu);
                s_bpCtrlInitialized = 1;
            }
            ((int&)Sing_805DC5F0()) = (int)s_bpCtrl;
        }
        ((CBpCtrl*)(((int&)Sing_805DC5F0())))->DeleteEntryById(partyGolfBaseScriptId);
        partyGolfBaseScriptId = -1;
    }
    if (timebombScriptId != -1) {
        if (!((int&)Sing_805DC5F0())) {
            if (!s_bpCtrlInitialized) {
                ((CBpCtrl*)s_bpCtrl)->__ct();
                __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, s_bpCtrlDtorRegistration_AuxMenu);
                s_bpCtrlInitialized = 1;
            }
            ((int&)Sing_805DC5F0()) = (int)s_bpCtrl;
        }
        ((CBpCtrl*)(((int&)Sing_805DC5F0())))->DeleteEntryById(timebombScriptId);
        timebombScriptId = -1;
    }
    if (pululuJarashiScriptId != -1) {
        if (!((int&)Sing_805DC5F0())) {
            if (!s_bpCtrlInitialized) {
                ((CBpCtrl*)s_bpCtrl)->__ct();
                __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, s_bpCtrlDtorRegistration_AuxMenu);
                s_bpCtrlInitialized = 1;
            }
            ((int&)Sing_805DC5F0()) = (int)s_bpCtrl;
        }
        ((CBpCtrl*)(((int&)Sing_805DC5F0())))->DeleteEntryById(pululuJarashiScriptId);
        pululuJarashiScriptId = -1;
    }
    if (featherBadgeScriptId != -1) {
        if (!((int&)Sing_805DC5F0())) {
            if (!s_bpCtrlInitialized) {
                ((CBpCtrl*)s_bpCtrl)->__ct();
                __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, s_bpCtrlDtorRegistration_AuxMenu);
                s_bpCtrlInitialized = 1;
            }
            ((int&)Sing_805DC5F0()) = (int)s_bpCtrl;
        }
        ((CBpCtrl*)(((int&)Sing_805DC5F0())))->DeleteEntryById(featherBadgeScriptId);
        featherBadgeScriptId = -1;
    }
    if ((*(int*)&AsCSharedDoc(g_sharedDoc)->partyGameMode) == 3) {
        UITextTable_ClearEntries(textTable);
        if (textTable) {
            if (!Sing_805DC830()) {
                if (!s_pointerVectorInitialized) {
                    PointerVector_InitReserved(s_fontNumDrawRegistry);
                    __register_global_object(s_fontNumDrawRegistry, (void*)PointerVector_DestroyStorage,
                                             s_fontNumDrawRegistryDtorRegistration);
                    s_pointerVectorInitialized = 1;
                }
                Sing_805DC830() = s_fontNumDrawRegistry;
            }
            PointerArray_DeleteValue(Sing_805DC830(), textTable);
        }
    }
    if (holeInfoSet) {
        delete (CHoleInfoSet*)holeInfoSet;
        holeInfoSet = 0;
    }
    if (ballPreview) {
        ScreenBallPreview_dtor(ballPreview, 1);
        ballPreview = 0;
    }
    {
        void* o = windInfo;
        if (o) {
            if (o)
                ((VDel0*)o)->del(1);
            windInfo = 0;
        }
    }
    {
        void* o = remainingTimeInfo;
        if (o) {
            if (o)
                ((VDel0*)o)->del(1);
            remainingTimeInfo = 0;
        }
    }
    if (ingameStatus) {
        delete (CIngameStatus*)ingameStatus;
        ingameStatus = 0;
    }
    {
        void* o = auxiliaryMenu;
        if (o) {
            if (o)
                ((VDel0*)o)->del(1);
            auxiliaryMenu = 0;
        }
    }
    {
        void* o = accuracyInfo;
        if (o) {
            if (o)
                ((VDel0*)o)->del(1);
            accuracyInfo = 0;
        }
    }
    {
        void* o = pangInfo;
        if (o) {
            if (o)
                ((VDel0*)o)->del(1);
            pangInfo = 0;
        }
    }
    {
        void* o = powerInfo;
        if (o) {
            if (o)
                ((VDel0*)o)->del(1);
            powerInfo = 0;
        }
    }
    {
        void* o = flyInfo;
        if (o) {
            if (o)
                ((VDel0*)o)->del(1);
            flyInfo = 0;
        }
    }
    {
        void* o = modeInfo;
        if (o) {
            if (o)
                ((VDel0*)o)->del(1);
            modeInfo = 0;
        }
    }
    if (modeLabelRenderer) {
        OverlayPlayer_Destroy(modeLabelRenderer, 1);
        modeLabelRenderer = 0;
    }
    if (statusTextRenderer) {
        OverlayPlayer_Destroy(statusTextRenderer, 1);
        statusTextRenderer = 0;
    }
    {
        void* o = calipers;
        if (o) {
            if (o)
                ((VDel0*)o)->del(1);
            calipers = 0;
        }
    }
    {
        void* o = skinsInfo;
        if (o) {
            if (o)
                ((VDel0*)o)->del(1);
            skinsInfo = 0;
        }
    }
    {
        void* o = nearestPinInfo;
        if (o) {
            if (o)
                ((VDel0*)o)->del(1);
            nearestPinInfo = 0;
        }
    }
    /* g_helpMsg */
    if (g_helpMsg) {
        if (g_helpMsg)
            ((VDel0*)g_helpMsg)->del(1);
    }
    /* g_resourceManager */
    if (g_resourceManager && windIndicatorResource) {
        ResourceManager_ReleaseObject(g_resourceManager, windIndicatorResource, 0);
        windIndicatorResource = 0;
    }
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
            InstanceLock_u80179180 iteratorLock((char*)&nameMutex);
            void* mutex = (char*)&nameMutex;
            nameHashIterator = nameHashHead;
            OSLockMutex((OSMutex*)mutex);
            {
                void* node = nameHashIterator;
                if (node) {
                    void* previousNode = ((OverlayRegistryNodeView*)node)->previous;
                    void* head = nameHashHead;
                    overlay = ((OverlayRegistryNodeView*)node)->value;
                    if (previousNode == head)
                        previousNode = 0;
                    nameHashIterator = previousNode;
                } else {
                    overlay = 0;
                }
            }
            OSUnlockMutex((OSMutex*)mutex);
        }
        if (overlay) {
            void* mutex = (char*)&nameMutex;
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
                    void* node = nameHashIterator;
                    if (node) {
                        void* previousNode = ((OverlayRegistryNodeView*)node)->previous;
                        void* head = nameHashHead;
                        overlay = ((OverlayRegistryNodeView*)node)->value;
                        if (previousNode == head)
                            previousNode = 0;
                        nameHashIterator = previousNode;
                    } else {
                        overlay = 0;
                    }
                }
            } while (overlay);
        }
        // Recycle the now payload-free node ring.
        InstanceLock_u80179180 ringLock((char*)&nameMutex);
        void* activeHead = nameHashHead;
        if (activeHead && nameHashFreeList) {
            void* activeAnchor = activeHead;
            void* freeAnchor = nameHashFreeList;
            void* activeNext = ((OverlayRegistryNodeView*)activeAnchor)->next;
            void* freePrevious = ((OverlayRegistryNodeView*)freeAnchor)->previous;
            ((OverlayRegistryNodeView*)activeNext)->previous = freePrevious;
            ((OverlayRegistryNodeView*)freePrevious)->next = activeNext;
            ((OverlayRegistryNodeView*)freeAnchor)->previous = activeAnchor;
            ((OverlayRegistryNodeView*)activeAnchor)->next = freeAnchor;
            nameHashHead = 0;
            activeHead = 0;
        }
        if (activeHead) {
            void* mutex = (char*)&nameMutex;
            void* remainingHead = 0;
            void* node = nameHashHead;
            void* freeHead;
            while (node) {
                {
                    InstanceLock_u80179180 nodeLock(mutex);
                    void* nextNode = ((OverlayRegistryNodeView*)node)->next;
                    void* previousNode = ((OverlayRegistryNodeView*)node)->previous;
                    ((OverlayRegistryNodeView*)previousNode)->next = nextNode;
                    ((OverlayRegistryNodeView*)nextNode)->previous = previousNode;
                    remainingHead = ((OverlayRegistryNodeView*)node)->previous;
                    if (remainingHead == node)
                        remainingHead = 0;
                }
                node = nameHashHead;
                freeHead = nameHashFreeList;
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
                if (freeHead)
                    node = freeHead;
                nameHashFreeList = node;
                nameHashHead = remainingHead;
                node = nameHashHead;
            }
        }
        // Bucket storage is a target pointer array (four bytes per entry).
        {
            int bucketIndex = 0;
            int bucketByteOffset = 0;
            int unusedZero = 0;
            while (bucketIndex < nameHashBucketCount) {
                *(int*)((char*)nameHashBuckets + bucketByteOffset) = 0; /* stwx */
                bucketIndex++;
                bucketByteOffset += 4;
            }
        }
    }
    /* trailing */
    void* _r = FxInterpreter_GetInstance();
    // Phase 4: remove any remaining effect attachments owned by this screen key.
    FxInterpreter_RemoveEntriesByKey(_r, fxInterpreterKey);
}
#pragma opt_common_subs reset
#pragma opt_propagation reset
#pragma use_lmw_stmw reset

extern "C" {

// Lazily construct the shared BP script controller and register its destructor.
void* CBpCtrl_GetInstance() {
    void* result;
    if (((int&)Sing_805DC5F0()) == 0) {
        if (s_bpCtrlInitialized == 0) {
            ((CBpCtrl*)s_bpCtrl)->__ct();
            __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, s_bpCtrlDtorRegistration_AuxMenu);
            s_bpCtrlInitialized = 1;
        }
        result = s_bpCtrl;
        ((int&)Sing_805DC5F0()) = (int)result;
    }
    return (void*)((int&)Sing_805DC5F0());
}

} // extern "C"

// Enable HUD display before loading the screen resources.
void CScreen::OnPreLoadInit() {
    hudDisplayEnabled = 1;
}

extern "C" {} // extern "C"

// Load HUD scripts and overlays, construct optional panels, and wait for all layout resources.
void CScreen::OnLoad() {
    void* r27 = ((void* (*)())CBpCtrl_GetInstance)();
    // Phase 1: load shared/party scripts and begin loading core child layouts.
    ((CBpCtrl*)r27)->CreateEntry(screenOverlayStrings, (void**)&ballWindScriptId, 0);
    ballWindScript = IdMap_Lookup(r27, (void*)ballWindScriptId);
    if ((*(int*)&AsCSharedDoc(SharedDoc_GetCurrent())->partyGameMode) == 3) {
        ((CBpCtrl*)r27)
            ->CreateEntry(screenOverlayStrings + ScreenText_PartyMeterScript, (void**)&partyGolfBaseScriptId, 0);
        ((CBpCtrl*)r27)->CreateEntry(screenOverlayStrings + ScreenText_CountdownScript, (void**)&timebombScriptId, 0);
        ((CBpCtrl*)r27)
            ->CreateEntry(screenOverlayStrings + ScreenText_PululuJarashiScript, (void**)&pululuJarashiScriptId, 0);
        ((CBpCtrl*)r27)->CreateEntry(screenOverlayStrings + ScreenText_FeatherScript, (void**)&featherBadgeScriptId, 0);
        partyGolfBaseScript = IdMap_Lookup(r27, (void*)partyGolfBaseScriptId);
        timebombScript = IdMap_Lookup(r27, (void*)timebombScriptId);
        pululuJarashiScript = IdMap_Lookup(r27, (void*)pululuJarashiScriptId);
        featherBadgeScript = IdMap_Lookup(r27, (void*)featherBadgeScriptId);
    }
    if (AsCGolfDoc(GolfDoc_GetCurrent())->tutorialStage < TutorialStageCount) {
        if (!tutorialIllustration) {
            void* r29 = __nw__FUl(0x7c);
            if (r29)
                ((CTutorialIllust2d*)r29)->__ct();
            tutorialIllustration = r29;
        }
    }
    Layout_LoadExtended(layout, 0, screenOverlayStrings + ScreenText_SwingCaptionLayout);
    {
        void* o = strokePointDisplay;
        ((void (*)(void*, int))(((ScreenVtableView<0xe0>*)o)->vtable)[3])(o, 0);
    }
    {
        void* o = mapCameraDisplay;
        ((void (*)(void*, int))(((ScreenVtableView<0x9c>*)o)->vtable)[3])(o, 0);
    }
    {
        void* o = ballWindDisplay;
        ((void (*)(void*, int))(((ScreenVtableView<0xa4>*)o)->vtable)[3])(o, 0);
    }
    {
        void* o = golfBaseDisplay;
        ((void (*)(void*, int))(((ScreenVtableView<0x164>*)o)->vtable)[3])(o, 0);
    }
    {
        void* o = swingResultDisplay;
        ((void (*)(void*, int))(((ScreenVtableView<0x74>*)o)->vtable)[3])(o, 0);
    }
    if (tutorialIllustration) {
        void* o = tutorialIllustration;
        ((void (*)(void*, int))(((ScreenVtableView<0x78>*)o)->vtable)[3])(o, 0);
    }
    // Phase 2: populate named overlays; registry entries retain the created resources.
    ScreenOverlayRegistry_CreateNamedOverlay(this, screenOverlayStrings + ScreenText_BallConditionKey,
                                             screenOverlayStrings + ScreenText_BallConditionResource, (void*)(0));
    ScreenOverlayRegistry_CreateNamedOverlay(this, screenOverlayStrings + ScreenText_PlayMode0Key,
                                             screenOverlayStrings + ScreenText_PlayMode0Resource, (void*)(0x80000000));
    ScreenOverlayRegistry_CreateNamedOverlay(this, screenOverlayStrings + ScreenText_PlayMode1Key,
                                             screenOverlayStrings + ScreenText_PlayMode1Resource, (void*)(0x80000000));
    ScreenOverlayRegistry_CreateNamedOverlay(this, screenOverlayStrings + ScreenText_PlayMode4Key,
                                             screenOverlayStrings + ScreenText_PlayMode4Resource, (void*)(0x80000000));
    ScreenOverlayRegistry_CreateNamedOverlay(this, screenOverlayStrings + ScreenText_SpinPointKey,
                                             screenOverlayStrings + ScreenText_SpinPointResource, (void*)(0));
    ((void (*)())Option_GetInstance)();
    if (!((int (*)())Option_UseIntegratedClubOverlay)()) {
        ScreenOverlayRegistry_CreateNamedOverlay(this, screenOverlayStrings + ScreenText_WoodClubKey,
                                                 screenOverlayStrings + ScreenText_WoodClubResource, (void*)(0));
        ScreenOverlayRegistry_CreateNamedOverlay(this, screenOverlayStrings + ScreenText_IronClubKey,
                                                 screenOverlayStrings + ScreenText_IronClubResource, (void*)(0));
        ScreenOverlayRegistry_CreateNamedOverlay(this, screenOverlayStrings + ScreenText_PutterClubKey,
                                                 screenOverlayStrings + ScreenText_PutterClubResource, (void*)(0));
    }
    ScreenOverlayRegistry_CreateNamedOverlay(this, screenOverlayStrings + ScreenText_TopCameraKey,
                                             screenOverlayStrings + ScreenText_TopCameraResource, (void*)(0));
    ScreenOverlayRegistry_CreateNamedOverlay(this, screenOverlayStrings + ScreenText_FreeCameraKey,
                                             screenOverlayStrings + ScreenText_FreeCameraResource, (void*)(0));
    ScreenOverlayRegistry_CreateNamedOverlay(this, screenOverlayStrings + ScreenText_TargetCameraKey,
                                             screenOverlayStrings + ScreenText_TargetCameraResource, (void*)(0));
    ScreenOverlayRegistry_CreateNamedOverlay(this, screenOverlayStrings + ScreenText_ClickIndicatorKey,
                                             screenOverlayStrings + ScreenText_ClickIndicatorResource, (void*)(0));
    void* commandResourcePool = __nw__FUl(0x50);
    if (commandResourcePool) {
        try {
            OverlayPlayer_Init(commandResourcePool, screenOverlayStrings + ScreenText_CommandKeysResource);
        } catch (...) {
            __dl__FPv(commandResourcePool);
            throw;
        }
    }
    statusTextRenderer = commandResourcePool;
    {
        S8017A418 t;
        void* _tr = ((void* (*)(S8017A418*, float, float, float, float))Vec4_Set)(&t, 0.0f, 0.0f, 20.0f, 20.0f);
        Register_NormalizedNamedRectangle(statusTextRenderer, screenOverlayStrings + ScreenText_DownSmallGlyph, _tr, 0);
    }
    {
        S8017A418 t;
        void* _tr = ((void* (*)(S8017A418*, float, float, float, float))Vec4_Set)(&t, 20.0f, 0.0f, 20.0f, 20.0f);
        Register_NormalizedNamedRectangle(statusTextRenderer, screenOverlayStrings + ScreenText_LeftSmallGlyph, _tr, 0);
    }
    {
        S8017A418 t;
        void* _tr = ((void* (*)(S8017A418*, float, float, float, float))Vec4_Set)(&t, 40.0f, 0.0f, 20.0f, 20.0f);
        Register_NormalizedNamedRectangle(statusTextRenderer, screenOverlayStrings + ScreenText_RightSmallGlyph, _tr,
                                          0);
    }
    {
        S8017A418 t;
        void* _tr = ((void* (*)(S8017A418*, float, float, float, float))Vec4_Set)(&t, 0.0f, 20.0f, 20.0f, 20.0f);
        Register_NormalizedNamedRectangle(statusTextRenderer, screenOverlayStrings + ScreenText_UpSmallGlyph, _tr, 0);
    }
    {
        S8017A418 t;
        void* _tr = ((void* (*)(S8017A418*, float, float, float, float))Vec4_Set)(&t, 20.0f, 20.0f, 20.0f, 20.0f);
        Register_NormalizedNamedRectangle(statusTextRenderer, screenOverlayStrings + ScreenText_DownGlyph, _tr, 0);
    }
    {
        S8017A418 t;
        void* _tr = ((void* (*)(S8017A418*, float, float, float, float))Vec4_Set)(&t, 40.0f, 20.0f, 20.0f, 20.0f);
        Register_NormalizedNamedRectangle(statusTextRenderer, screenOverlayStrings + ScreenText_LeftGlyph, _tr, 0);
    }
    {
        S8017A418 t;
        void* _tr = ((void* (*)(S8017A418*, float, float, float, float))Vec4_Set)(&t, 0.0f, 40.0f, 20.0f, 20.0f);
        Register_NormalizedNamedRectangle(statusTextRenderer, screenOverlayStrings + ScreenText_RightGlyph, _tr, 0);
    }
    {
        S8017A418 t;
        void* _tr = ((void* (*)(S8017A418*, float, float, float, float))Vec4_Set)(&t, 20.0f, 40.0f, 20.0f, 20.0f);
        Register_NormalizedNamedRectangle(statusTextRenderer, screenOverlayStrings + ScreenText_UpGlyph, _tr, 0);
    }
    {
        S8017A418 t;
        void* _tr = ((void* (*)(S8017A418*, float, float, float, float))Vec4_Set)(&t, 40.0f, 40.0f, 20.0f, 20.0f);
        Register_NormalizedNamedRectangle(statusTextRenderer, screenOverlayStrings + ScreenText_InvalidGlyph, _tr, 0);
    }
    float f30 = ((float (*)(void*))Option_GetHudBaselineY)(((void* (*)())Option_GetInstance)());
    windIndicatorResource =
        ResourceManager_LoadByExtension(g_resourceManager, screenOverlayStrings + ScreenText_WindResource, 0, 1, 0);
    ((void (*)())Option_GetInstance)();
    if (((int (*)())Option_PlaceWindIndicatorOnLoad)()) {
        float f1 = ((float (*)(void*))WView_GetViewportWidth)(g_view);
        f1 = f1 - 50.0f;

        WPuppet_ReservedHook(windIndicatorResource, f1, ((f30 + 5.0f)), g_view);
    }
    void* r29b;
    r29b = __nw__FUl(0x1c);
    if (r29b) {
        try {
            ((CHoleInfoSet*)r29b)->Init();
        } catch (...) {
            __dl__FPv(r29b);
            throw;
        }
    }
    holeInfoSet = r29b;
    r29b = __nw__FUl(0x690);
    if (r29b) {
        try {
            ScreenBallPreview_Ctor(r29b);
        } catch (...) {
            __dl__FPv(r29b);
            throw;
        }
    }
    ballPreview = r29b;
    r29b = __nw__FUl(0x470);
    if (r29b) {
        try {
            ((CPangInfo*)r29b)->__ct();
        } catch (...) {
            __dl__FPv(r29b);
            throw;
        }
    }
    pangInfo = r29b;
    r29b = __nw__FUl(0x28);
    if (r29b) {
        try {
            ((CIngameStatus*)r29b)->Init();
        } catch (...) {
            __dl__FPv(r29b);
            throw;
        }
    }
    ingameStatus = r29b;
    r29b = __nw__FUl(0x18);
    if (r29b) {
        try {
            ((CWindInfo*)r29b)->__ct();
        } catch (...) {
            __dl__FPv(r29b);
            throw;
        }
    }
    windInfo = r29b;
    r29b = __nw__FUl(0x2c);
    if (r29b) {
        try {
            ((CAccuracyInfo*)r29b)->__ct();
        } catch (...) {
            __dl__FPv(r29b);
            throw;
        }
    }
    accuracyInfo = r29b;
    r29b = __nw__FUl(0x24);
    if (r29b) {
        try {
            ((CPowerInfo*)r29b)->__ct();
        } catch (...) {
            __dl__FPv(r29b);
            throw;
        }
    }
    powerInfo = r29b;
    r29b = __nw__FUl(0x88);
    if (r29b) {
        try {
            ((CFlyInfo*)r29b)->__ct();
        } catch (...) {
            __dl__FPv(r29b);
            throw;
        }
    }
    flyInfo = r29b;
    if ((*(int*)&AsCSharedDoc(SharedDoc_GetCurrent())->partyGameMode) == 3) {
        r29b = __nw__FUl(0x4c);
        if (r29b) {
            try {
                ((CBalloonInfo*)r29b)->__ct();
            } catch (...) {
                __dl__FPv(r29b);
                throw;
            }
        }
        modeInfo = r29b;
    }
    if ((*(int*)&AsCSharedDoc(SharedDoc_GetCurrent())->partyGameMode) == 2) {
        r29b = __nw__FUl(0x38);
        if (r29b) {
            try {
                ((CDriveInfo*)r29b)->__ct();
            } catch (...) {
                __dl__FPv(r29b);
                throw;
            }
        }
        modeInfo = r29b;
    }
    if (AsCGolfDoc(GolfDoc_GetCurrent())->tutorialStage == TutorialStageCount) {
        r29b = __nw__FUl(0x24);
        if (r29b) {
            try {
                ((CAuxMenuUI*)r29b)->__ct();
            } catch (...) {
                __dl__FPv(r29b);
                throw;
            }
        }
        auxiliaryMenu = r29b;
        r29b = __nw__FUl(0x18);
        if (r29b) {
            try {
                ((CRemainTimeInfo*)r29b)->__ct();
            } catch (...) {
                __dl__FPv(r29b);
                throw;
            }
        }
        remainingTimeInfo = r29b;
        r29b = __nw__FUl(0x2c);
        if (r29b) {
            try {
                ((CCalipers*)r29b)->__ct();
            } catch (...) {
                __dl__FPv(r29b);
                throw;
            }
        }
        calipers = r29b;
        r29b = __nw__FUl(0x28);
        if (r29b) {
            try {
                ((CHelpMsg*)r29b)->__ct();
            } catch (...) {
                __dl__FPv(r29b);
                throw;
            }
        }
        // The tutorial timers and flags below are set only in the else branch.
    } else {
        if ((unsigned char)SharedDoc_GetGolfRuleType() == 2) {
            float f31v = 24.0f;
            void* r29 = __nw__FUl(0x50);
            if (r29) {
                try {
                    OverlayPlayer_Init(r29, screenOverlayStrings + ScreenText_TutorialTitleResource);
                } catch (...) {
                    __dl__FPv(r29);
                    throw;
                }
            }
            modeLabelRenderer = r29;
            char* commandResourcePool = screenOverlayStrings;
            char* r28 = tutorialTitleKeys[0];
            char* r27 = r28;

            for (int r26 = 1; r26 <= 5; r26++) {
                S8017A418 t;
                char* s =
                    (char*)String_FormatTemporary(commandResourcePool + ScreenText_TutorialTitleFormat, ((r26 + 1)));
                strcpy(r28, s);
                void* _tr = ((void* (*)(S8017A418*, float, float, float, float))Vec4_Set)(
                    &t, 0.0f, f31v * (float)(r26 - 1), 177.0f, f31v);
                Register_NormalizedNamedRectangle(modeLabelRenderer, r27, _tr, 0);
                r28 += 0x10;
                r27 += 0x10;
            }
        }
        tutorialFlashOn = 0;
        tutorialFlashCount = 2;
        tutorialFlashInterval = 0.25f;
        tutorialFlashTimer = 0.0f;
        tutorialCompletionPhase = 3;
        tutorialCompletionInterval = 0.25f;
        tutorialTitlePhase = 2;
        tutorialTitleTimer = 0.0f;
        modeLabelOpacity = 0;
    }
    void* _t5c = ((void* (*)())ChatMessage_GetCurrent)();
    chatMessageFontResource = (int)((void* (*)(void*))ChatMessage_GetDefaultFontResource)(_t5c);
    for (int r26 = 0; 6 > r26; r26++) {
        if (r26 < AsCGolfDoc(GolfDoc_GetCurrent())->tutorialStage)
            tutorialStageVisible[r26] = 1;
        else
            tutorialStageVisible[r26] = 0;
    }
    if (AsCGolfDoc(GolfDoc_GetCurrent())->tutorialStage < TutorialStageCount && holeInfoSet)
        ((void* (*)(void*))HoleInfoSet_ShowCornerLabels)(holeInfoSet);
    // Phase 3: keep updating child loaders until all required resources are ready.
    while (1) {
        Layout_Update(layout);
        {
            void* o = strokePointDisplay;
            ((void (*)(void*))(((ScreenVtableView<0xe0>*)o)->vtable)[12])(o);
        }
        {
            void* o = mapCameraDisplay;
            ((void (*)(void*))(((ScreenVtableView<0x9c>*)o)->vtable)[5])(o);
        }
        {
            void* o = ballWindDisplay;
            ((void (*)(void*))(((ScreenVtableView<0xa4>*)o)->vtable)[5])(o);
        }
        {
            void* o = golfBaseDisplay;
            ((void (*)(void*))(((ScreenVtableView<0x164>*)o)->vtable)[5])(o);
        }
        {
            void* o = swingResultDisplay;
            ((void (*)(void*))(((ScreenVtableView<0x74>*)o)->vtable)[5])(o);
        }
        if (tutorialIllustration) {
            void* o = tutorialIllustration;
            ((void (*)(void*))(((ScreenVtableView<0x78>*)o)->vtable)[6])(o);
        }
        // Require the base HUD resources and any party-mode resources to finish loading.
        if (PollLoadComplete()) {
            if ((*(int*)&AsCSharedDoc(SharedDoc_GetCurrent())->partyGameMode) == 3) {
                void* receiver696_fn_800A6218 = (void*)PartyItemSlots_GetInstance();
                PartyItemSlots_RefreshDisplay(receiver696_fn_800A6218);
            }
            break;
        }
    }
}

extern "C" {

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
    ((CHoleInfo*)((HoleInfoSetView*)holeInfo)->cornerLabels)->ShowCornerLabels();
}

// Return the shared option object without transferring ownership.
void* Option_GetInstance() {
    return g_option;
}

// Return the current shared chat-message object without retaining it.
void* ChatMessage_GetCurrent() {
    return g_chatMsg;
}

} // extern "C"

// Reset HUD animation and panel state, then attach the current club resource effects.
void* CScreen::OnInit() {
    panelOpenRequested = 0;
    itemPanelOpenRequested = 0;
    itemPanelReady = 0;
    shotOptionEnabled = 0;
    shotOptionChanged = 0;
    resumeAimRequested = 0;
    hudInputBlocked = 0;
    tutorialMeterEnabled = 0;
    tutorialGaugeEnabled = 0;
    tutorialAimEnabled = 0;
    aimControlsEnabled = 0;
    dropModeActive = 0;
    tutorialClubChangeEnabled = 1;
    tutorialAimControlEnabled = 1;
    tutorialSpinControlEnabled = 1;
    tutorialIllustrationActive = 0;
    shotTransitionRequested = 0;
    swingFeedbackPending = 0;
    cameraControlsEnabled = 1;
    meterFadeFrame = 0;
    impactFeedbackStarted = 0;
    partyMeterIndex = -1;
    pululuAnimationPhase = 0;
    tutorialAimBlinkFrame = 0;
    tutorialAimBlinkCount = 0;
    tutorialMeterBlinkFrame = 0;
    tutorialMeterBlinkCount = 0;
    tutorialGaugeBlinkFrame = 0;
    tutorialGaugeBlinkCount = 0;
    meterOpacity = 255;
    hudDisplayMask = 0;
    requestedPanel = 0;
    previousPanel = 0;
    savedPanel = 0;
    hudPhase = HudInactive;
    previousHudPhase = 0;
    shotOptionResult = 0;
    shotDirectionResult = 0;
    clubChangeDisplayState = 0;
    cameraCaptionOpacity = 0;
    terrainSampleCount = 10;
    terrainMarkerIndex = 0;
    shotIndicatorPulse = 0;
    previousTutorialStage = -1;
    tutorialPointerDelayed = 0;
    previousGameStateFlags = 0;
    ballRingHeight = 0;
    displayTimeRemaining = 0;
    terrainProfileScale = 0;
    initialDisplayEnabled = 1;
    clubChangeAngle = 6.2831855f;
    clubChangeAnimationDirection = 0;
    clubChangeAnimationTime = 0;
    feedbackColor0 = 0;
    feedbackColor1 = 0;
    shotFeedbackBlend = 0;
    hudFlags |= 2;
    // The club kind comes from the selected club; its resource is looked up by the current player's item.
    unsigned char clubKind = ((ClubSelectionState*)InstanceTable_Get())->selection->clubKind;
    void* sharedDoc = g_sharedDoc;
    void* clubTable = InstanceTable_Get();
    unsigned char playerIndex = AsCGolfDoc(AsCSharedDoc(sharedDoc)->golfDoc)->currentPlayer;
    void* clubResources =
        ClubModel_FindCachedForItem(clubTable, *(int*)((char*)sharedDoc + (unsigned int)playerIndex * 0x1d4 + 0x1cc));
    void* clubAsset = ((ClubResourceSlots*)clubResources)->byClubKind[(unsigned int)clubKind];
    void* fxInterpreter = FxInterpreter_GetInstance();
    void* fxKey = FxAttachments_CreateFromResourceTags(fxInterpreter, clubAsset, 0);
    (*(void**)&fxInterpreterKey) = fxKey;
    return fxKey;
}

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
// Borrowed field views used only by the HUD coordinator; offsets preserve Wii storage.
struct HudPlayerFields {
    char padding00[8];
    int status08;
    char padding0C[0x90];
    unsigned char itemMenuAvailable;
    char padding9D[3];
    unsigned int equippedItemId;
    char paddingA4[4];
    float gaugeValue;
    char paddingAC[0x34];
    int impactRangeAdjustment;
};
struct HudPartyEffectsView {
    char padding00[0x10];
    PartyItemModifiers modifiers;
};
struct HudPlayerDocumentSlot {
    char padding000[0x1cc];
    int clubItemId;
    char padding1D0[0xc];
    int activityWord;
};
struct HudHoleRecordView {
    char padding00[0xc];
    WVector position;
    char padding18[8];
};
struct HudHeadingReferenceView {
    char padding000[0x33c];
    WVector headingReference;
};
struct HudHoleInfoChildren {
    void* cornerGroup;
    char padding04[8];
    void* accuracyInfo;
};
struct HudCornerGroupChildren {
    char padding00[0x34];
    void* distanceToHole;
};
enum HudPointerRegion {
    HudRegionClubMenu = 5,
    HudRegionPlayerFirst = 10,
    HudRegionPlayerLast = 13,
    HudRegionTutorialAction = 31,
    HudRegionDelayedTutorialControl = 32,
    HudRegionOneShotClose = 35
};
enum HudPointerButtonState {
    HudPrimaryPointerButton = 0,
    HudPointerPressed = 1,
    HudPointerReleased = 2
};
enum HudCoordinatorConstants {
    kSharedPlayerRecordStride = 0x1d4,
    kPartyItemMode = 3,
    kTimebombPartyItem = 10,
    kTimebombTickStateMask = 0x08000008
};

// Read one player's document words; the view is offset-biased and never owns storage.
static inline HudPlayerDocumentSlot* HudDocumentPlayer(void* doc, int playerIndex) {
    return (HudPlayerDocumentSlot*)((char*)doc + playerIndex * kSharedPlayerRecordStride);
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
#define CALL_SLOTS(args)       \
    virtual int slot0x08 args; \
    virtual int slot0xC args;  \
    virtual int slot0x10 args; \
    virtual int slot0x14 args; \
    virtual int slot0x18 args; \
    virtual int slot0x1C args; \
    virtual int slot0x20 args; \
    virtual int slot0x24 args; \
    virtual int slot0x28 args; \
    virtual int slot0x2C args; \
    virtual int slot0x30 args; \
    virtual int slot0x34 args; \
    virtual int slot0x38 args; \
    virtual int slot0x3C args; \
    virtual int slot0x40 args; \
    virtual int slot0x44 args; \
    virtual int slot0x48 args; \
    virtual int slot0x4C args; \
    virtual int slot0x50 args; \
    virtual int slot0x54 args; \
    virtual int slot0x58 args; \
    virtual int slot0x5C args; \
    virtual int slot0x60 args; \
    virtual int slot0x64 args; \
    virtual int slot0x68 args; \
    virtual int slot0x6C args; \
    virtual int slot0x70 args;
template <int N> struct ScreenCall0 {
    char data[N];
    CALL_SLOTS(())
};
template <int N> struct ScreenCallI {
    char data[N];
    CALL_SLOTS((int))
};
template <int N> struct ScreenCallF {
    char data[N];
    CALL_SLOTS((float))
};
template <> struct ScreenCallF<0> {
    CALL_SLOTS((float))
};
template <int N> struct ScreenCallFF {
    char data[N];
    CALL_SLOTS((float, float))
};
template <int N> struct ScreenCallPP {
    char data[N];
    CALL_SLOTS((void*, void*))
};
template <int N> struct ScreenCallP {
    char data[N];
    CALL_SLOTS((void*))
};
#undef CALL_SLOTS
#define GAME (*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc)
#define PLAYER AsCGolfDoc(GAME)->currentPlayer
#define STAGE AsCGolfDoc(GAME)->tutorialStage

extern "C" {
void InputManager_Init(void*);
void InputManager_Dtor();
void InputManager_RequestPointerInput(void*, int);
int Input_IsActionPressed(void*, int, int);
extern "C" int SharedDoc_IsPlayerFlag0Set(int);
void GolfTipsDialog_RequestClose(int, float);
void Sound_PlayByName(void*, const char*, int, int, int, int, float, float, float);
extern "C" void IActor_DispatchMessage(void*, void*, const char*, int, void*, void*, void*, void*);
void* GolfPlayer_GetEntryByIndex(void*, int);
unsigned int ItemInventory_GetItemIdAt(void*, int);
int GolfState_AllowsStatusTransition(void*);
void* GolfBall__Fv();
void NoticeQueue_Clear(void*);
void NoticeQueue_UpdateTimers(void*, float);
void HeadIcon_UpdatePlayerTimers(void*, float);
void Vec2_Copy(void*, void*);
unsigned long long Vec2_ScaleByFloat(void*, float);
unsigned long long Vec2_Add(void*, void*);
extern "C" int PartyItemSlots_GetItemType(void*, int);
void ScreenBallPreview_Update(void*, float);

void __mi__FRC7WVectorRC7WVector(void*, void*, void*);
float Vector_SignedAngleXZ(void*, void*);
int Pointer_GetButtonState(void*, int);
int GolfTask_NoOpStatus();
extern "C" float ShotParameters_GetGoodImpactHalfWidth(void*);
float Random_RangeFloat(int, float, float);
extern void* g_soundManager;
extern void *g_headIcon, *g_inputPointer;
extern signed char s_inputManagerInitialized;
extern char s_gameInputManager[], s_gameInputManagerDestructorRegistration_Screen[];
extern unsigned int jumptable_804AEBA8[];
extern const float kFloat_0_5_AuxMenu;
extern const float kScreenHudSoundRange; // 200.0f: HUD sound range
extern const float kFloat_1_AuxMenu;
extern const float kScreenTutorialTitleInsetX; // 30.0f: tutorial title horizontal inset
extern const float kFloat_0_3_AuxMenu;
extern const float kScreenMarkerInsetX;                   // 14.0f: marker/title horizontal inset
extern const float kFloat_0_05_Screen;                    // 0.05f: ground-lie adjustment and viewport height fraction
extern const float kScreenTutorialTitleDestinationInsetX; // 250.0f: tutorial title destination inset
extern const float kFloat_255_Screen;
extern const float kFloat_100_Screen;             // 100.0f: shot indicator animation speed / update text position
extern const float kScreenShotFeedbackBlendRate;  // 7.0f: shot feedback blend rate
extern const float kScreenClubChangeAngularSpeed; // 21.9911499f: club-change angular animation speed
extern const float kFloat_2_805DED94;
extern const float kScreenClubChangeAngleUpperLimit;  // 6.80678415f: club-change angle upper limit
extern const float kScreenCameraCaptionFadeStart;     // -255.0f: camera-caption fade start
extern const float kScreenCameraCaptionFadeRate;      // 512.0f: camera-caption fade rate
extern const float kScreenTerrainIndicatorAngleLimit; // 0.69813168f: terrain indicator angle limit (40 degrees)
extern const float kFloat_128_ShotGauge;
extern const float kPangyaTargetCenterX;
extern const float kFloat_10_Screen;              // 10.0f: impact range and marker coordinate scale
extern const float kScreenGroundLieRangeScale1_5; // 1.5f: ground-lie impact-range multiplier
extern const float kScreenGroundLieRangeScale1_3; // 1.3f: ground-lie impact-range multiplier
extern const float kFloat_1_2_ScreenOverlay;
extern const float kFloat_1_1_ScreenRange;
extern const float kScreenImpactRangeNormalization; // 460.0f: impact-range normalization numerator
extern const float kScreenRadiansToDegrees;         // 57.29578f: radians-to-degrees scale
extern const float kScreenImpactHeadingDeadZone;    // 2.5f: impact heading dead zone
extern const float kFloat_neg1_Screen;
extern const float kScreenColorEqualityEpsilon;           // 0.00001f: color equality tolerance
extern const float kScreenImpactFeedbackMinimumIntensity; // 225.0f: minimum randomized impact-feedback color
extern float g_screenFullFeedbackIntensity;               // 255.0f: full-intensity feedback color comparison

// Keep singleton construction inline: its local-static symbol and initialization order are part of the ABI.
#pragma push
#pragma always_inline on
static inline void EnsureHudInput() {
    if (!GetInputManagerSlot()) {
        if (!s_inputManagerInitialized) {
            InputManager_Init(s_gameInputManager);
            __register_global_object(s_gameInputManager, (void*)InputManager_Dtor,
                                     s_gameInputManagerDestructorRegistration_Screen);
            s_inputManagerInitialized = 1;
        }
        GetInputManagerSlot() = s_gameInputManager;
    }
}
// Test a logical HUD action through the initialized shared input manager.
static inline int IsHudActionPressed(int action) {
    return Input_IsActionPressed(GetInputManagerSlot(), action, -1);
}
// Play a HUD sound selected by a byte offset in the shared string pool.
static inline void PlayHudSound(int stringOffset) {
    Sound_PlayByName(g_soundManager, screenOverlayStrings + stringOffset, 0, 0, 0, 0, kFloat_0_5_AuxMenu,
                     kScreenHudSoundRange, kFloat_1_AuxMenu);
}
// Send two raw payload words synchronously to the named HUD actor.
static inline void DispatchHudMessage(void* sender, int actorNameOffset, int messageId, unsigned int payload0,
                                      unsigned int payload1) {
    IActor_DispatchMessage(sender, sender, screenOverlayStrings + actorNameOffset, messageId, (void*)payload0,
                           (void*)payload1, 0, 0);
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
static inline void UpdateHudBallWindControls(CScreen* self) {
    if (TutorialStageCount > STAGE || ((ClubSelectionState*)InstanceTable_Get())->selection->clubKind == 3 ||
        (self->hudPhase) != HudAim) {
        ((HudBallWindInterface*)self->ballWindDisplay)->SetSelectionActive(0);
        if ((self->hudPhase) == HudAim)
            ((HudBallWindInterface*)self->ballWindDisplay)->SetSelectionState(0);
    } else {
        ((HudBallWindInterface*)self->ballWindDisplay)->SetSelectionActive(1);
        if ((self->shotDirectionResult) || (self->shotOptionResult))
            ((HudBallWindInterface*)self->ballWindDisplay)->SetSelectionState(2);
        else
            ((HudBallWindInterface*)self->ballWindDisplay)->SetSelectionState(1);
    }
    if ((self->hudPhase) != HudAim || (self->dropModeActive)) {
        ((HudBallWindInterface*)self->ballWindDisplay)->ShowElements22And25(0);
        ((HudBallWindInterface*)self->ballWindDisplay)->ShowElements26And27(0);
    } else if (6 == STAGE) {
        if (((ClubSelectionState*)InstanceTable_Get())->selection->clubKind == 3 &&
            ((CGolfBall*)GolfBall__Fv())->m_groundType == 2) {
            ((HudBallWindInterface*)self->ballWindDisplay)->ShowElements22And25(1);
            ((HudBallWindInterface*)self->ballWindDisplay)->ShowElements26And27(0);
        } else {
            ((HudBallWindInterface*)self->ballWindDisplay)->ShowElements26And27(1);
            ((HudBallWindInterface*)self->ballWindDisplay)->ShowElements22And25(0);
        }
    } else if (4 <= STAGE) {
        if (((ClubSelectionState*)InstanceTable_Get())->selection->clubKind == 3 &&
            ((CGolfBall*)GolfBall__Fv())->m_groundType == 2)
            ((HudBallWindInterface*)self->ballWindDisplay)->ShowElement22(1);
        else
            ((HudBallWindInterface*)self->ballWindDisplay)->ShowElement22(0);
    }
    if ((self->hudPhase) != HudAim || (self->dropModeActive) || !(self->cameraControlsEnabled)) {
        ((HudBallWindInterface*)self->ballWindDisplay)->ShowElement28(0);
        ((HudBallWindInterface*)self->ballWindDisplay)->ShowElement29(0);
    } else if (AsCGolfDoc(GAME)->cameraMode == 0 || AsCGolfDoc(GAME)->cameraMode == 4)
        ((HudBallWindInterface*)self->ballWindDisplay)->ShowElement28(1);
    else
        ((HudBallWindInterface*)self->ballWindDisplay)->ShowElement29(1);
    if (((HudPlayerFields*)GolfPlayer_GetEntryByIndex(GAME, PLAYER))->itemMenuAvailable &&
        !(int)((HudPlayerFields*)GolfPlayer_GetEntryByIndex(GAME, PLAYER))->equippedItemId && !(self->dropModeActive))
        ((HudBallWindInterface*)self->ballWindDisplay)->SetElements25And26Bright(1);
    else
        ((HudBallWindInterface*)self->ballWindDisplay)->SetElements25And26Bright(0);
}

// Advance the club-change angle in radians; dt is seconds and the idle timer uses its own rate.
static inline void UpdateHudClubChangeAngle(CScreen* self, float dt) {
    if (((ClubSelectionState*)InstanceTable_Get())->changeFlags & 1) {
        float zero = kFloat_0_AuxMenu;
        self->clubChangeAngle = zero;
        (self->clubChangeAnimationDirection) = 1;
        self->clubChangeAnimationTime = zero;
    } else {
        switch ((self->clubChangeAnimationDirection)) {
        case -1: {
            float t = self->clubChangeAngle - kScreenClubChangeAngularSpeed * dt;
            self->clubChangeAngle = t;
            if (t < kFloat_6_2831855_Screen) {
                self->clubChangeAngle = kFloat_6_2831855_Screen;
                (self->clubChangeAnimationDirection) = 0;
            }
            break;
        }
        case 0:
            self->clubChangeAnimationTime = kFloat_2_805DED94 * dt + self->clubChangeAnimationTime;
            break;
        case 1: {
            float t = kScreenClubChangeAngularSpeed * dt + self->clubChangeAngle;
            self->clubChangeAngle = t;
            if (t > kScreenClubChangeAngleUpperLimit) {
                self->clubChangeAngle = kScreenClubChangeAngleUpperLimit;
                (self->clubChangeAnimationDirection) = -1;
            }
            break;
        }
        }
    }
}

// Advance completion and flash timers without changing the separate title-motion clock.
// flashDirection/flashCount are caller scratch only; references preserve register lifetimes.
// The caller retains ownership of the screen and passes dt in seconds.
static inline void UpdateHudTutorialFlashClocks(CScreen* self, float dt, unsigned int& flashDirection,
                                                unsigned char& flashCount) {
    // Completion phases below 3 blink; the separate title phases stay in OnProcess.
    if (self->tutorialCompletionPhase < 3) {
        float t = (self->tutorialCompletionTimer + dt);
        self->tutorialCompletionTimer = t;
        if (t > self->tutorialCompletionInterval) {
            self->tutorialCompletionTimer = t - self->tutorialCompletionInterval;
            // The original reads these flash flags as bool.
            ((bool*)self->tutorialStageVisible)[STAGE] = ((0 == (((bool*)self->tutorialStageVisible)[STAGE])));
            if (((bool*)self->tutorialStageVisible)[STAGE])
                ++self->tutorialCompletionPhase;
        }
    }
    flashCount = self->tutorialFlashCount;
    if (2 > flashCount) {
        float t = ((self->tutorialFlashTimer + dt));
        self->tutorialFlashTimer = t;
        if (t > self->tutorialFlashInterval) {
            flashDirection = 1 - self->tutorialFlashOn;
            self->tutorialFlashTimer = t - self->tutorialFlashInterval;
            self->tutorialFlashOn = flashDirection;
            if ((unsigned char)flashDirection)
                self->tutorialFlashCount = flashCount + 1;
        }
    }
}

#pragma pop

#pragma push
#pragma always_inline on
// Advance loaded child presentations in dependency order; no input or panel requests here.
static inline void UpdateLoadedHudChildren(CScreen* self) {
    Layout_Update(AsCScreen(self)->layout);
    ((HudStrokePointInterface*)AsCScreen(self)->strokePointDisplay)->UpdateFrame();
    ((HudMapCameraInterface*)AsCScreen(self)->mapCameraDisplay)->UpdateFrame();
    AsCScreen(self)->UpdateShotMeterPresentation();
    ((HudBallWindInterface*)self->ballWindDisplay)->UpdateFrame();
    ((HudGolfBaseInterface*)AsCScreen(self)->golfBaseDisplay)->UpdateFrame();
    ((HudSwingResultFrameInterface*)AsCScreen(self)->swingResultDisplay)->UpdateLayoutAndFeedback();
    // CTutorialIllust2d slot 0x18 updates layout, page controls, and both message
    // reveals when present; its aggregate ready result is ignored by this coordinator.
    if (AsCScreen(self)->tutorialIllustration)
        ((HudTutorialIllustrationInterface*)AsCScreen(self)->tutorialIllustration)->ProcessFrameSlot18();
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
static inline void RequestHudAimPanelWhenReady(CScreen* self) {
    if (!SharedDoc_IsPlayerFlag0Set(PLAYER) && (self->requestedPanel) == 0 &&
        (self->previousPanel) == (self->requestedPanel) && (self->hudPhase) == HudAim && !(self->resumeAimRequested)) {
        if (STAGE == TutorialStageCount || ((self->tutorialAimEnabled) && TutorialStageCount > STAGE &&
                                            (self->tutorialAimBlinkCount) > TutorialBlinkLimit)) {
            (self->requestedPanel) = HudPanelAim;
            (self->panelOpenRequested) = 1;
        }
    } else {
        (self->previousPanel) = (self->requestedPanel);
        (self->previousHudPhase) = (self->hudPhase);
    }
}

#pragma pop

#pragma push
#pragma always_inline on
// Consume the aim-close request, close tips and start the distance-to-pin overlay motion.
static inline void FinishHudAimPanelClosing(CScreen* self) {
    if ((self->requestedPanel) & HudPanelAimClosing) {
        (self->requestedPanel) = 0;
        GolfTipsDialog_RequestClose(-2, (kFloat_0_5_AuxMenu));
        void* holeInfo = AsCScreen(self)->holeInfoSet;
        if (holeInfo) {
            void* cornerGroup = ((HudHoleInfoChildren*)holeInfo)->cornerGroup;
            if (cornerGroup && ((HudCornerGroupChildren*)cornerGroup)->distanceToHole)
                ((CDistToHoleInfo*)(((HudCornerGroupChildren*)cornerGroup)->distanceToHole))->BeginMoveToPin();
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
    kHudCameraQueryInput = 8,
    kHudSpinQueryRange = 9,
    kHudItemQueryNoSelection = 12,
    kHudItemQueryTransitionState = 13,
    kHudTutorialQueryDialog = 6
};
enum HudAimActorReply {
    kHudItemPanelOpening = 1,
    kHudItemPanelClosing = 2
};

// Coordinate resumed aim, camera permission, and spin/item panel requests.
static inline void ProcessHudAimPanelInput(CScreen* self, void*& inventory, void*& activePlayer, int& cameraInputReply,
                                           float& spinRangeX, float& spinRangeY, int& itemPanelStateReply, int& allowed,
                                           int& blocked) {

    // Resume restores the saved panel and requests its opening on the next reconciliation.
    if ((self->resumeAimRequested)) {
        (self->requestedPanel) = ((self->savedPanel));
        (self->panelOpenRequested) = 1;
        (self->resumeAimRequested) = 0;
        PlayHudSound(ScreenText_CancelClickSound);
        if (AsCScreen(self)->calipers) {
            inventory = AsCSharedDoc(g_sharedDoc)->itemInventory;
            activePlayer = GolfPlayer_GetEntryByIndex(GAME, PLAYER);
            if (((HudPlayerFields*)activePlayer)->equippedItemId == ItemInventory_GetItemIdAt(inventory, 5) ||
                AsCGolfDoc(GAME)->powerShotMeterOverride)
                ((CCalipers*)(AsCScreen(self)->calipers))->ResetMeasurement();
        }
        return;
    }
    if (!(self->aimControlsEnabled))
        ((HudGolfBaseInterface*)AsCScreen(self)->golfBaseDisplay)->EnableConfirmButton(0);
    else
        ((HudGolfBaseInterface*)AsCScreen(self)->golfBaseDisplay)->EnableConfirmButton(1);
    if (TutorialStageCount > STAGE)
        ((HudGolfBaseInterface*)AsCScreen(self)->golfBaseDisplay)
            ->EnableActionButtons((self->tutorialAimControlEnabled));
    if (!((ClubSelectionState*)InstanceTable_Get())->shotOptionEnabled &&
        ((HudStrokePointInterface*)AsCScreen(self)->strokePointDisplay)->GetPowerMode())
        ((HudStrokePointInterface*)AsCScreen(self)->strokePointDisplay)->ClearPowerMode();
    // Query camera input before any confirm, spin, or item transition.
    DispatchHudMessage(self, ScreenText_CameraActor, kHudCameraQueryInput, (unsigned int)&cameraInputReply, 0);
    if (cameraInputReply) {
        if (1 != cameraInputReply) {
            ((HudGolfBaseInterface*)AsCScreen(self)->golfBaseDisplay)->EnableConfirmButton(0);
            return;
        }
    }
    if (((HudGolfBaseInterface*)AsCScreen(self)->golfBaseDisplay)->IsConfirmActivated()) {
        if (!AsCGolfDoc(GAME)->cameraMode) {
            (self->hudPhase) = HudShot;
            unsigned int action = (self->requestedPanel);
            (self->savedPanel) = action;
            (self->requestedPanel) = (action & HudPanelHighBitsMask) | HudPanelAimClosing;
            CMouseCursor::SetPointerInputEnabled(g_mouseCursor, 0);
            PlayHudSound(ScreenText_IconClickSound);
        }
    }
    // previousPanel is the acknowledged presentation; defer new panels until it catches up.
    if ((self->previousPanel) != (self->requestedPanel))
        return;
    if (((HudBallWindInterface*)self->ballWindDisplay)->IsBallSelectorActivated() || ScreenInputPress(HudActionSpin)) {
        if (((ClubSelectionState*)InstanceTable_Get())->selection->clubKind != 3) {
            if (STAGE == TutorialStageCount) {
                (self->hudPhase) = HudSpin;
                (self->savedPanel) = (self->requestedPanel);
                (self->requestedPanel) = HudPanelSpin;
                if (AsCScreen(self)->ingameStatus && GolfState_AllowsStatusTransition(AsCScreen(self)->ingameStatus))
                    ((CIngameStatus*)(AsCScreen(self)->ingameStatus))->BeginHide();
                spinRangeX = kFloat_0_AuxMenu;
                spinRangeY = kFloat_0_AuxMenu;
                DispatchHudMessage(self, ScreenText_SpinActor, kHudSpinQueryRange, (unsigned int)&spinRangeX,
                                   (unsigned int)&spinRangeY);
                ((HudStrokePointInterface*)AsCScreen(self)->strokePointDisplay)
                    ->SetDirectionThresholds(spinRangeX, spinRangeY);
                void* activePlayer = GolfPlayer_GetEntryByIndex(GAME, PLAYER);
                ((HudStrokePointInterface*)AsCScreen(self)->strokePointDisplay)
                    ->SetGaugeValue((int)((HudPlayerFields*)activePlayer)->gaugeValue);
                if ((*(int*)&AsCSharedDoc(g_sharedDoc)->partyGameMode) == 1) {
                    ((HudStrokePointInterface*)AsCScreen(self)->strokePointDisplay)->EnablePowerMode(0);
                    ((HudStrokePointInterface*)AsCScreen(self)->strokePointDisplay)->EnableDirectionButtons(0);
                } else {
                    ((HudStrokePointInterface*)AsCScreen(self)->strokePointDisplay)->EnableDirectionButtons(1);
                    if (((ClubSelectionState*)InstanceTable_Get())->selection->clubKind == 3 ||
                        ((CGolfBall*)GolfBall__Fv())->m_groundType == 2)
                        ((HudStrokePointInterface*)AsCScreen(self)->strokePointDisplay)->EnablePowerMode(0);
                    else
                        ((HudStrokePointInterface*)AsCScreen(self)->strokePointDisplay)->EnablePowerMode(1);
                }
                if (((ClubSelectionState*)InstanceTable_Get())->selection->clubIndex < 3)
                    ((HudStrokePointInterface*)AsCScreen(self)->strokePointDisplay)->EnableExtendedSpecialShots(1);
                else
                    ((HudStrokePointInterface*)AsCScreen(self)->strokePointDisplay)->EnableExtendedSpecialShots(0);
                PlayHudSound(ScreenText_OpenSpinDialogSound);
            }
        }
    }
    if ((self->hudPhase) == HudSpin)
        return;
    if (((HudPlayerFields*)GolfPlayer_GetEntryByIndex(GAME, PLAYER))->itemMenuAvailable) {
        if (((*(unsigned int*)&AsCGolfDoc(GAME)->gameStateFlags) & HudItemMenuAllowedStates)) {
            if (STAGE == TutorialStageCount) {

                DispatchHudMessage(self, ScreenText_ItemActor, kHudItemQueryTransitionState,
                                   (unsigned int)&itemPanelStateReply, 0);
                EnsureHudInput();
                if (IsHudActionPressed(HudActionOpenItems) || itemPanelStateReply == kHudItemPanelOpening) {

                    DispatchHudMessage(self, ScreenText_ItemActor, kHudItemQueryNoSelection, (unsigned int)&allowed, 0);
                    if (0 == allowed)
                        return;
                    if (g_helpMsg)
                        NoticeQueue_Clear(g_helpMsg);
                    PlayHudSound(ScreenText_IconClickSound);
                    (self->hudPhase) = HudItems;
                    (self->savedPanel) = (self->requestedPanel);
                    (self->requestedPanel) = HudPanelItems;
                    (self->itemPanelOpenRequested) = 1;
                    if (AsCScreen(self)->ingameStatus &&
                        GolfState_AllowsStatusTransition(AsCScreen(self)->ingameStatus))
                        ((CIngameStatus*)(AsCScreen(self)->ingameStatus))->BeginHide();
                    return;
                }
            }
        }
    }
    EnsureHudInput();
    if (!IsHudActionPressed(HudActionAim) || AsCGolfDoc(GAME)->cameraMode || !(self->aimControlsEnabled) ||
        ((HudGolfBaseInterface*)AsCScreen(self)->golfBaseDisplay)->IsCancelActivated())
        return;
    blocked = 0;
    if (TutorialStageCount > STAGE)
        DispatchHudMessage(self, ScreenText_TutorialActor, kHudTutorialQueryDialog, (unsigned int)&blocked, 0);
    if (AsCScreen(self)->calipers && ((CCalipers*)(AsCScreen(self)->calipers))->IsPreviewActive())
        blocked = 1;
    if (blocked)
        return;
    ((HudGolfBaseInterface*)AsCScreen(self)->golfBaseDisplay)->LatchConfirmAction();
    unsigned int action = (self->requestedPanel);
    (self->hudPhase) = HudShot;
    (self->savedPanel) = action;
    (self->requestedPanel) = (action & HudPanelHighBitsMask) | HudPanelAimClosing;
    CMouseCursor::SetPointerInputEnabled(g_mouseCursor, 0);
    PlayHudSound(ScreenText_IconClickSound);
    return;
}

// Close the item panel on cancellation or its actor's close status.
static inline void ProcessHudItemPanelInput(CScreen* self, int& itemStatus) {

    DispatchHudMessage(self, ScreenText_ItemActor, kHudItemQueryTransitionState, (unsigned int)&itemStatus, 0);
    EnsureHudInput();
    if (IsHudActionPressed(HudActionCancelPanel) || kHudItemPanelClosing == itemStatus) {
        (self->hudPhase) = HudAim;
        (self->requestedPanel) = (self->savedPanel);
        (self->panelOpenRequested) = 1;
        (self->itemPanelReady) = 0;
        EnsureHudInput();
        if (IsHudActionPressed(HudActionCancelPanel) && g_helpMsg)
            NoticeQueue_Clear(g_helpMsg);
    }
    return;
}

// Commit or restore spin selection, then update shot-option feedback and return to aiming.
static inline void ProcessHudSpinPanelInput(CScreen* self, unsigned int& spinDone, float& spinCommitX,
                                            float& spinCommitY) {
    spinDone = 0;
    if (((HudStrokePointInterface*)AsCScreen(self)->strokePointDisplay)->IsConfirmationRequested())
        spinDone = 1;
    else {
        EnsureHudInput();
        if (IsHudActionPressed(HudActionCancelPanel)) {
            ((HudStrokePointInterface*)AsCScreen(self)->strokePointDisplay)->RestoreSavedSelection();

            ((HudStrokePointInterface*)AsCScreen(self)->strokePointDisplay)
                ->GetCommittedPoint(&spinCommitX, &spinCommitY);
            // Preserve float-to-integer payload evaluation order in this spin-dialog commit.
            IActor_DispatchMessage(self, self, screenOverlayStrings + ScreenText_SpinActor, 10,
                                   (void*)((int)spinCommitX), (void*)((int)spinCommitY), 0, 0);
            spinDone = 1;
        }
    }
    if (spinDone) {
        (self->hudPhase) = HudAim;
        (self->requestedPanel) = (self->savedPanel);
        (self->panelOpenRequested) = 1;
        ((HudStrokePointInterface*)AsCScreen(self)->strokePointDisplay)->BeginClosing();
        (self->shotOptionResult) =
            ((HudStrokePointInterface*)AsCScreen(self)->strokePointDisplay)->GetDirectionSelection();
        int option = ((HudStrokePointInterface*)AsCScreen(self)->strokePointDisplay)->GetPowerMode();
        if (option != ((ClubSelectionState*)InstanceTable_Get())->shotOptionEnabled)
            (self->shotOptionChanged) = 1;
        if (option)
            (self->shotOptionEnabled) = 1;
        else
            (self->shotOptionEnabled) = 0;
        (self->shotDirectionResult) =
            ((HudStrokePointInterface*)AsCScreen(self)->strokePointDisplay)->GetSpecialShotSelection();
        ((HudGolfBaseInterface*)AsCScreen(self)->golfBaseDisplay)->ShowExtraMeterImage((self->shotOptionEnabled));
    }
    return;
}

// Retire the previous aim/camera/item/spin panel before the shot transition.
static inline void ProcessHudShotTransitionInput(CScreen* self) {
    (self->panelsReady) = 0;
    int previousPanel = (self->previousPanel);
    (self->panelOpenRequested) = 0;
    if (previousPanel == HudPanelAim)
        ((HudGolfBaseInterface*)AsCScreen(self)->golfBaseDisplay)->BeginFadeOut();
    else if (previousPanel == HudPanelCamera)
        ((HudMapCameraInterface*)AsCScreen(self)->mapCameraDisplay)->BeginClosing();
    if ((self->itemPanelReady)) {
        (self->itemPanelReady) = 0;
        DispatchHudMessage(self, ScreenText_ItemActor, 1, 0, 0);
    } else if ((self->previousHudPhase) == HudSpin) {
        ((HudStrokePointInterface*)AsCScreen(self)->strokePointDisplay)->BeginClosing();
        PlayHudSound(ScreenText_CloseSpinDialogSound);
    }
    return;
}

// Poll layout loading, update loaded children and dispatch the active input phase.
static inline void UpdateHudLoadedPanelsAndInput(CScreen* self, void*& inventory, void*& activePlayer,
                                                 int& cameraInputReply, float& spinRangeX, float& spinRangeY,
                                                 int& itemPanelStateReply, int& allowed, int& blocked, int& itemStatus,
                                                 unsigned int& spinDone, float& spinCommitX, float& spinCommitY) {
    if (g_drawDriveLine)
        ((CDrawDriveLine*)g_drawDriveLine)->UpdateShotPrediction();
    if (((CScreen*)self)->PollLoadComplete()) {
        if (!(self->hudInitialized)) {
            ((CScreen*)self)->InitializeLoadedLayouts();
            (self->hudInitialized) = 1;
        }
    }
    if ((self->hudInitialized)) {
        UpdateLoadedHudChildren(self);
        EnsureHudInput();
        (self->shotOptionChanged) = 0;
        RequestHudAimPanelWhenReady(self);
        FinishHudAimPanelClosing(self);
        if ((self->hudPhase) == HudAim && !SharedDoc_IsPlayerFlag0Set(PLAYER))
            ((HudBallWindInterface*)self->ballWindDisplay)->SetSelectionActive(1);
        else
            ((HudBallWindInterface*)self->ballWindDisplay)->SetSelectionActive(0);
        if ((self->dropModeActive)) {
            EnsureHudInput();
            InputManager_RequestPointerInput(GetInputManagerSlot(), 1);
            if ((self->requestedPanel)) {
                (self->savedPanel) = (self->requestedPanel);
                (self->requestedPanel) = 0;
                (self->panelsReady) = 0;
            }
        }
        if (SharedDoc_IsPlayerFlag0Set(PLAYER)) {
            if ((self->hudPhase) == HudAim) {
                (self->hudPhase) = HudShot;
                (self->panelsReady) = 0;
            }
        }
        if ((self->shotTransitionRequested)) {
            unsigned int zero;
            unsigned int previous;
            previous = (self->hudPhase);
            zero = 0;
            (self->previousHudPhase) = previous;
            (self->requestedPanel) = zero;
            (self->hudPhase) = HudShotTransition;
            (self->savedPanel) = zero;
            AsCGolfDoc(GAME)->cameraMode = zero;
            (self->panelsReady) = 1;
            (self->shotTransitionRequested) = zero;
        }
        if ((self->panelsReady)) {
            if (((self->hudPhase) == HudShotTransition || ((CScreen*)self)->AreHudTransitionsComplete())) {
                if (!(self->hudInputBlocked)) {
                    switch ((self->hudPhase)) {
                    case HudInactive:
                    case HudShot:
                    default:
                        break;
                    case HudAim: // Handle aim confirmation and requests to open spin/items; scratch preserves the original ABI lifetimes.
                        ProcessHudAimPanelInput(self, inventory, activePlayer, cameraInputReply, spinRangeX, spinRangeY,
                                                itemPanelStateReply, allowed, blocked);
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
static inline void ReconcileHudCameraAndEquipment(CScreen* self) {
    // Phase 2: reconcile camera mode, blocked input and equipment restrictions;
    // this shared path also runs before layout initialization.
    if (AsCGolfDoc(GAME)->cameraMode) {
        if (!(self->dropModeActive)) {
            int action = (self->requestedPanel);
            if ((action & 1) || (0 == action && !(self->tutorialAimEnabled))) {
                (self->requestedPanel) = (action & HudPanelHighBitsMask) | HudPanelCamera;
                (self->panelsReady) = 0;
                (self->panelOpenRequested) = 1;
            }
        }
    }
    if ((self->hudInputBlocked)) {
        ((HudMapCameraInterface*)AsCScreen(self)->mapCameraDisplay)->LatchFirstActionAndPlayClick();
        ((HudMapCameraInterface*)AsCScreen(self)->mapCameraDisplay)->BeginClosing();
        if (!(self->tutorialAimEnabled) && STAGE < TutorialStageCount)
            (self->requestedPanel) = 0;
        else
            (self->requestedPanel) = ((self->requestedPanel) & HudPanelHighBitsMask) | HudPanelAim;
        (self->hudInputBlocked) = 0;
        (self->panelsReady) = 0;
        (self->panelOpenRequested) = 1;
    }
    if (((self->shotDirectionResult) == 0x20 || (self->shotDirectionResult) == 0x40)) {
        if (((ClubSelectionState*)InstanceTable_Get())->selection->clubIndex >= 3) {
            (self->shotDirectionResult) = 0;
            ((HudStrokePointInterface*)AsCScreen(self)->strokePointDisplay)->ClearSpecialShotSelection();
        }
    }
    if ((self->shotOptionResult)) {
        if (((ClubSelectionState*)InstanceTable_Get())->selection->clubKind == 3) {
            (self->shotOptionResult) = 0;
            ((HudStrokePointInterface*)AsCScreen(self)->strokePointDisplay)->ClearDirectionSelection();
        }
    }
}

// Update ball controls, timebomb frame cues and the dt-driven shot pulse.
static inline void UpdateHudPartyClocksAndShotPulse(CScreen* self, float dt, unsigned int& otherPlayerHasTimebomb) {
    UpdateHudBallWindControls(self);
    if ((*(int*)&AsCSharedDoc(g_sharedDoc)->partyGameMode) == kPartyItemMode) {
        otherPlayerHasTimebomb = 0;
        for (int playerIndex = 0; playerIndex < AsCGolfDoc(GAME)->playerCount; ++playerIndex) {
            if (PartyItemSlots_GetItemType(PartyItemSlots_GetInstance(), playerIndex) == kTimebombPartyItem) {
                if (playerIndex == PLAYER) {
                    otherPlayerHasTimebomb = 0;
                    break;
                }
                otherPlayerHasTimebomb = 1;
            }
        }
        if (otherPlayerHasTimebomb) {
            void* partyEffects = PartyItemSlots_GetInstance();
            if (((HudPartyEffectsView*)partyEffects)->modifiers.timebombTicks) {
                if (((HudPartyEffectsView*)partyEffects)->modifiers.timebombTicks % TimebombFramesPerTick == 0) {
                    void* golfDocument = GAME;
                    unsigned int playerActivity;
                    if ((((unsigned int)AsCGolfDoc(golfDocument)->gameStateFlags & kTimebombTickStateMask) &&
                         (((playerActivity = HudDocumentPlayer(g_sharedDoc, PLAYER)->activityWord) &&
                           !AsCGolfDoc(golfDocument)->shotPhase) ||
                          (playerActivity == 0 && AsCGolfDoc(golfDocument)->shotPhase < 2))) ||
                        AsCGolfDoc(golfDocument)->cameraMode)
                        PlayHudSound(ScreenText_TimebombTickSound);
                }
            }
            if (((HudPartyEffectsView*)partyEffects)->modifiers.timebombTriggered &&
                ((HudPartyEffectsView*)partyEffects)->modifiers.timebombTicks ==
                    ((HudPartyEffectsView*)partyEffects)->modifiers.timebombPeriod * TimebombFramesPerTick)
                PlayHudSound(ScreenText_TimebombFailureSound);
        }
    }
    if (AsCGolfDoc(GAME)->shotPhase == 2) {
        float shotPulse = kFloat_100_Screen * dt + self->shotIndicatorPulse;
        self->shotIndicatorPulse = shotPulse;
        if (shotPulse > kScreenPulseWrapThreshold)
            self->shotIndicatorPulse = shotPulse - kScreenPulseWrapSpan;
    }
}

// Advance wind, ball preview, reward display and shot-feedback blend; dt is seconds.
static inline void UpdateHudWindAndPreview(CScreen* self, float dt) {
    // Phase 4: update wind, preview/rewards, club effects and optional HUD children.
    CWind::GetInstance()->UpdateIndicator(dt);
    if (AsCScreen(self)->ballPreview)
        ScreenBallPreview_Update(AsCScreen(self)->ballPreview, dt);
    if (AsCScreen(self)->pangInfo)
        ((HudTimedChildInterface*)AsCScreen(self)->pangInfo)->UpdateSeconds(dt);
    float feedbackBlend = kScreenShotFeedbackBlendRate * dt + self->shotFeedbackBlend;
    self->shotFeedbackBlend = feedbackBlend > kFloat_1_AuxMenu ? kFloat_1_AuxMenu : feedbackBlend;
}

// Replace interpreter attachments after the equipped club changes; the interpreter owns returned FX.
static inline void RefreshHudClubAttachments(CScreen* self, unsigned char& clubKind, void*& sharedDocument,
                                             void*& clubModel, void*& clubAsset) {
    if (((ClubSelectionState*)InstanceTable_Get())->changeFlags & 2) {
        FxInterpreter_RemoveEntriesByKey(FxInterpreter_GetInstance(), (self->fxInterpreterKey));

        clubKind = ((ClubSelectionState*)InstanceTable_Get())->selection->clubKind;
        sharedDocument = g_sharedDoc;
        clubModel = ClubModel_FindCachedForItem(
            InstanceTable_Get(),
            HudDocumentPlayer(sharedDocument, AsCGolfDoc(AsCSharedDoc(sharedDocument)->golfDoc)->currentPlayer)
                ->clubItemId);
        clubAsset = *(void* volatile*)&((ClubResourceSlots*)clubModel)->byClubKind[clubKind];
        (self->fxInterpreterKey) = (int)FxAttachments_CreateFromResourceTags(FxInterpreter_GetInstance(), clubAsset, 0);
    }
}

// Update club angle and optional child displays; calipers run only for an enabled aim preview.
static inline void UpdateHudTimedChildren(CScreen* self, float dt, void*& inventory, void*& activePlayer) {
    UpdateHudClubChangeAngle(self, dt);
    if (AsCScreen(self)->calipers) {
        if ((self->hudPhase) == HudAim) {
            inventory = AsCSharedDoc(g_sharedDoc)->itemInventory;
            activePlayer = GolfPlayer_GetEntryByIndex(GAME, PLAYER);
            if (((HudPlayerFields*)activePlayer)->equippedItemId == ItemInventory_GetItemIdAt(inventory, 5) ||
                AsCGolfDoc(GAME)->powerShotMeterOverride)
                ((HudTimedChildInterface*)AsCScreen(self)->calipers)->UpdateSeconds(dt);
        }
    }
    if (AsCScreen(self)->holeInfoSet) {
        if (AsCGolfDoc(GAME)->cameraMode || ((*(unsigned int*)&AsCGolfDoc(GAME)->gameStateFlags) & 8)) {
            ((CAccuracyInfo*)(((HudHoleInfoChildren*)AsCScreen(self)->holeInfoSet)->accuracyInfo))
                ->ShowShotFlagFeedback(
                    (self->shotDirectionResult) | (self->shotOptionResult),
                    (((HudStrokePointInterface*)AsCScreen(self)->strokePointDisplay)->GetPowerMode()));
        }
        ((CHoleInfoSet*)(AsCScreen(self)->holeInfoSet))->Update(dt);
    }
    if (AsCScreen(self)->flyInfo)
        ((HudTimedChildInterface*)AsCScreen(self)->flyInfo)->UpdateSeconds(dt);
    if (AsCScreen(self)->accuracyInfo)
        ((HudTimedChildInterface*)AsCScreen(self)->accuracyInfo)->UpdateSeconds(dt);
    if (AsCScreen(self)->modeInfo)
        ((HudTimedChildInterface*)AsCScreen(self)->modeInfo)->UpdateSeconds(dt);
    if (AsCScreen(self)->ingameStatus && 5 <= STAGE)
        CIngameStatus::Update(AsCScreen(self)->ingameStatus, dt);
}

// Restart or advance caption opacity when the camera gameplay state changes.
static inline void UpdateHudCameraCaption(CScreen* self, float dt) {
    if (AsCGolfDoc(GAME)->cameraMode) {
        unsigned int previousStates = AsCScreen(self)->previousGameStateFlags;
        int currentStates = (*(unsigned int*)&AsCGolfDoc(GAME)->gameStateFlags);
        if (previousStates != currentStates) {
            if (!(previousStates & 8) || !(currentStates & 0x100) || !(8 & currentStates) || !(previousStates & 0x100))
                self->cameraCaptionOpacity = kScreenCameraCaptionFadeStart;
            AsCScreen(self)->previousGameStateFlags = (*(unsigned int*)&AsCGolfDoc(GAME)->gameStateFlags);
        } else {
            float captionOpacity = kScreenCameraCaptionFadeRate * dt + self->cameraCaptionOpacity;
            self->cameraCaptionOpacity = captionOpacity > kFloat_255_Screen ? kFloat_255_Screen : captionOpacity;
        }
    }
}

// Re-enable the delayed tutorial pointer region after its seconds-based delay.
static inline void UpdateHudTutorialPointerDelay(CScreen* self, float dt) {
    if (self->tutorialPointerDelayed) {
        float pointerDelaySeconds = self->tutorialPointerDelay + dt;
        self->tutorialPointerDelay = pointerDelaySeconds;
        if (pointerDelaySeconds > (kFloat_5_Screen)) {
            self->tutorialPointerDelayed = 0;
            ((CMouseCursor*)g_mouseCursor)->regionEnabled[HudRegionDelayedTutorialControl] = 1;
        }
    }
}

// Dispatch pointer actions; release actions must match their button-down region.
static inline void ProcessHudPointerRegions(CScreen* self, int& panelStatus, unsigned char& scorePanelBlocksInput) {
    // Phase 5: pointer-region messages. Region 5=club menu, 10..13=activePlayer
    // entries, 31=tutorial action, 32=delayed tutorial control, 35=one-shot close.
    // A release must match the button-down region before activating a control.
    // The original hitRegion-to-handler table lives in another data unit.
    unsigned int hitRegion = ((CMouseCursor*)g_mouseCursor)->hitRegion;
    {
        switch (hitRegion) {
        case 6:
        case 7:
        case 8:
        case 9:
        case 14:
        case 15:
        case 16:
        case 17:
        case 18:
        case 19:
        case 20:
        case 21:
        case 22:
        case 23:
        case 24:
        case 25:
        case 26:
        case 27:
        case 28:
        case 29:
        case 30:
        case 33:
        case 34:
        default:
            break;
        case HudRegionClubMenu: {

            DispatchHudMessage(self, ScreenText_ItemActor, 5, (unsigned int)&panelStatus, 0);
            if (3 != panelStatus)
                break;
            DispatchHudMessage(self, ScreenText_PlayerActor, 21, (unsigned int)&panelStatus, 0);
            if (3 == panelStatus ||
                Pointer_GetButtonState(g_inputPointer, HudPrimaryPointerButton) != HudPointerPressed ||
                !((*(unsigned int*)&AsCScreen(self)->hudFlags) & 2))
                break;
            // The same document byte also suspends the shot clock; keep it in the club-menu gate.
            if (AsCGolfDoc(GAME)->shotPhase || AsCGolfDoc(GAME)->unk_35D || 2 > STAGE ||
                ((ClubSelectionState*)InstanceTable_Get())->clubMenuState != 2)
                break;
            DispatchHudMessage(self, ScreenText_GolfRuleActor, 5, 0, 0);
            break;
        }
        case HudRegionPlayerFirst:
        case 11:
        case 12:
        case HudRegionPlayerLast: {

            DispatchHudMessage(self, ScreenText_ScoreActor, 13, (unsigned int)&scorePanelBlocksInput, 0);
            if (scorePanelBlocksInput ||
                Pointer_GetButtonState(g_inputPointer, HudPrimaryPointerButton) != HudPointerReleased ||
                ((CMouseCursor*)g_mouseCursor)->hitRegion != ((CMouseCursor*)g_mouseCursor)->buttonDownRegions[0] ||
                !GolfTask_NoOpStatus())
                break;
            if (((HudPlayerFields*)GolfPlayer_GetEntryByIndex(
                     GAME, (unsigned char)(((CMouseCursor*)g_mouseCursor)->hitRegion - HudRegionPlayerFirst)))
                    ->status08 == 3)
                break;
            PlayHudSound(ScreenText_OpenPlayMenuSound);
            ((CMouseCursor*)g_mouseCursor)->SetRadialMenuEnabled(0);
            break;
        }
        case HudRegionTutorialAction:
            if (Pointer_GetButtonState(g_inputPointer, HudPrimaryPointerButton) == HudPointerReleased &&
                ((CMouseCursor*)g_mouseCursor)->hitRegion == ((CMouseCursor*)g_mouseCursor)->buttonDownRegions[0])
                DispatchHudMessage(self, ScreenText_TutorialActor, 3, 0, 0);
            break;
        case HudRegionDelayedTutorialControl:
            if (Pointer_GetButtonState(g_inputPointer, HudPrimaryPointerButton) == HudPointerReleased) {
                if (((CMouseCursor*)g_mouseCursor)->hitRegion == ((CMouseCursor*)g_mouseCursor)->buttonDownRegions[0]) {
                    if (STAGE < TutorialStageCount) {
                        if (self->tutorialPointerDelayed == 0) {
                            self->tutorialPointerDelayed = 1;
                            self->tutorialPointerDelay = kFloat_0_AuxMenu;
                            ((CMouseCursor*)g_mouseCursor)->regionEnabled[HudRegionDelayedTutorialControl] = 0;
                        }
                    }
                }
            }
            break;
        case HudRegionOneShotClose:
            if (Pointer_GetButtonState(g_inputPointer, HudPrimaryPointerButton) == HudPointerReleased &&
                ((CMouseCursor*)g_mouseCursor)->hitRegion == ((CMouseCursor*)g_mouseCursor)->buttonDownRegions[0])
                ((CMouseCursor*)g_mouseCursor)->regionEnabled[HudRegionOneShotClose] = 0;
            break;
        case 3:
            break;
        }
    }
}

// Alias a feedback difference as IEEE-754 bits for the original absolute-value test.
union HudFeedbackChannelBits {
    int u;
    float f;
};

// Derive and clamp the impact zone in gauge coordinates, then animate feedback RGB.
static inline void UpdateHudImpactZone(CScreen* self, void*& inventory, void*& activePlayer,
                                       float& colorEqualityTolerance, float& proposedTargetOffset,
                                       float& baseImpactRange, float& maximumTargetOffset, float& adjustedImpactRange,
                                       float& heading, void*& partyEffects, void*& impactWidthInventory,
                                       void*& impactWidthPlayer, float& targetDistance, char*& channelOwnerCursor,
                                       HudFeedbackChannelBits& intensityDifferenceBits) {
    // Phase 6: derive Pangya impactZoneWidth/target from lie, equipment and heading.
    // Gauge positions are HUD coordinates, not world distances; clamp the target
    // so its half-impactZoneWidth stays inside the good-impact interval. Then age notices.
    if (((ClubSelectionState*)InstanceTable_Get())->selection->clubKind == 3)
        AsCGolfDoc(GAME)->pangyaTargetX = kPangyaTargetCenterX;
    else {

        if (!HudDocumentPlayer(g_sharedDoc, PLAYER)->activityWord) {
            baseImpactRange = ((ClubSelectionState*)InstanceTable_Get())->selection->baseImpactRange;
            adjustedImpactRange =
                baseImpactRange -
                kFloat_10_Screen *
                    (float)((HudPlayerFields*)GolfPlayer_GetEntryByIndex(GAME, PLAYER))->impactRangeAdjustment;
            if (((CGolfBall*)GolfBall__Fv())->m_groundType == 3) {
                switch (((ClubSelectionState*)InstanceTable_Get())->selection->clubKind) {
                case 0:
                    adjustedImpactRange *= kScreenGroundLieRangeScale1_5;
                    break;
                case 1:
                    adjustedImpactRange *= kScreenGroundLieRangeScale1_3;
                    break;
                case 2:
                    adjustedImpactRange *= kFloat_1_2_ScreenOverlay;
                    break;
                }
            } else if (((CGolfBall*)GolfBall__Fv())->m_groundType == 7) {
                switch (((ClubSelectionState*)InstanceTable_Get())->selection->clubKind) {
                case 0:
                    adjustedImpactRange *= kScreenGroundLieRangeScale1_3;
                    break;
                case 1:
                    adjustedImpactRange *= kFloat_1_2_ScreenOverlay;
                    break;
                case 2:
                default:
                    break;
                }
            } else if (((CGolfBall*)GolfBall__Fv())->m_groundType == 4) {
                switch (((ClubSelectionState*)InstanceTable_Get())->selection->clubKind) {
                case 0:
                    adjustedImpactRange *= kFloat_1_1_ScreenRange;
                    break;
                case 1:
                    adjustedImpactRange *= kFloat_1_1_ScreenRange;
                    break;
                case 2:
                    adjustedImpactRange *= kFloat_1_1_ScreenRange;
                    break;
                }
            } else
                GolfBall__Fv();
            inventory = AsCSharedDoc(g_sharedDoc)->itemInventory;
            activePlayer = GolfPlayer_GetEntryByIndex(GAME, PLAYER);
            if (((HudPlayerFields*)activePlayer)->equippedItemId == ItemInventory_GetItemIdAt(inventory, 7) ||
                (inventory = AsCSharedDoc(g_sharedDoc)->itemInventory,
                 activePlayer = GolfPlayer_GetEntryByIndex(GAME, PLAYER),
                 ((HudPlayerFields*)activePlayer)->equippedItemId == ItemInventory_GetItemIdAt(inventory, 8)) ||
                (inventory = AsCSharedDoc(g_sharedDoc)->itemInventory,
                 activePlayer = GolfPlayer_GetEntryByIndex(GAME, PLAYER),
                 ((HudPlayerFields*)activePlayer)->equippedItemId == ItemInventory_GetItemIdAt(inventory, 9)) ||
                (inventory = AsCSharedDoc(g_sharedDoc)->itemInventory,
                 activePlayer = GolfPlayer_GetEntryByIndex(GAME, PLAYER),
                 ((HudPlayerFields*)activePlayer)->equippedItemId == ItemInventory_GetItemIdAt(inventory, 2)))
                adjustedImpactRange *= kFloat_0_5_AuxMenu;
            if (kFloat_0_AuxMenu != adjustedImpactRange)
                AsCGolfDoc(GAME)->pangyaZoneScale = kScreenImpactRangeNormalization / adjustedImpactRange;
        }
        float impactZoneWidth = kFloat_2_805DED94;
        partyEffects = PartyItemSlots_GetInstance();
        impactWidthInventory = AsCSharedDoc(g_sharedDoc)->itemInventory;
        impactWidthPlayer = GolfPlayer_GetEntryByIndex(GAME, PLAYER);
        if (((HudPlayerFields*)impactWidthPlayer)->equippedItemId ==
                ItemInventory_GetItemIdAt(impactWidthInventory, 9) ||
            (impactWidthInventory = AsCSharedDoc(g_sharedDoc)->itemInventory,
             impactWidthPlayer = GolfPlayer_GetEntryByIndex(GAME, PLAYER),
             ((HudPlayerFields*)impactWidthPlayer)->equippedItemId ==
                 ItemInventory_GetItemIdAt(impactWidthInventory, 1)))
            impactZoneWidth *= kFloat_2_805DED94;
        else
            impactZoneWidth *= kFloat_1_AuxMenu + ((HudPartyEffectsView*)partyEffects)->modifiers.pangyaZoneWidthBonus;
        impactZoneWidth *= AsCGolfDoc(GAME)->pangyaZoneScale;
        if ((*(int*)&AsCSharedDoc(g_sharedDoc)->partyGameMode) == 3 &&
            !HudDocumentPlayer(g_sharedDoc, PLAYER)->activityWord)
            impactZoneWidth /= ((HudPartyEffectsView*)partyEffects)->modifiers.impactZoneWidthDivisor;
        DispatchHudMessage(self, ScreenText_PlayerActor, 9, (unsigned int)&heading, 0);
        heading = (float)floor(kScreenRadiansToDegrees * (heading * kFloat_0_5_AuxMenu));
        float absoluteHeading = heading > kFloat_0_AuxMenu ? heading : -heading;
        if (absoluteHeading < kScreenImpactHeadingDeadZone)
            heading = kFloat_0_AuxMenu;
        else if (heading > kFloat_0_AuxMenu)
            *(volatile float*)&heading = heading - kFloat_2_805DED94;
        else
            heading = heading + kFloat_2_805DED94;
        if (!HudDocumentPlayer(g_sharedDoc, PLAYER)->activityWord) {
            if (((float (*)(void*))ShotParameters_GetGoodImpactHalfWidth)(InstanceTable_Get()) < impactZoneWidth) {
                AsCGolfDoc(GAME)->pangyaZoneScale *=
                    (((float (*)(void*))ShotParameters_GetGoodImpactHalfWidth)(InstanceTable_Get()) / impactZoneWidth);
                impactZoneWidth = ((float (*)(void*))ShotParameters_GetGoodImpactHalfWidth)(InstanceTable_Get());
            }
        }
        maximumTargetOffset =
            ((float (*)(void*))ShotParameters_GetGoodImpactHalfWidth)(InstanceTable_Get()) - impactZoneWidth;
        proposedTargetOffset = heading;
        float minimumTargetOffset =
            kFloat_neg1_Screen * ((float (*)(void*))ShotParameters_GetGoodImpactHalfWidth)(InstanceTable_Get()) +
            impactZoneWidth;
        float clampedTargetOffset =
            proposedTargetOffset < minimumTargetOffset
                ? minimumTargetOffset
                : (proposedTargetOffset > maximumTargetOffset ? maximumTargetOffset : proposedTargetOffset);
        AsCGolfDoc(GAME)->pangyaTargetX = (kPangyaTargetCenterX + clampedTargetOffset);
        ((HudGolfBaseInterface*)AsCScreen(self)->golfBaseDisplay)->SetPangyaTargetX(AsCGolfDoc(GAME)->pangyaTargetX);
        ((HudGolfBaseInterface*)AsCScreen(self)->golfBaseDisplay)
            ->SetPangyaZoneWidth((((impactZoneWidth * kFloat_0_5_AuxMenu))));
        targetDistance = AsCGolfDoc(GAME)->roundedShotGaugeCursorX - AsCGolfDoc(GAME)->pangyaTargetX;
        targetDistance = targetDistance > kFloat_0_AuxMenu ? targetDistance : -targetDistance;
        if (targetDistance < ((float (*)(void*))ShotParameters_GetGoodImpactHalfWidth)(InstanceTable_Get()) &&
            AsCGolfDoc(GAME)->shotPhase == 2) {
            channelOwnerCursor = (char*)self;
            colorEqualityTolerance = kScreenColorEqualityEpsilon;
            for (int channelIndex = 0; 3 > channelIndex; ++channelIndex, channelOwnerCursor += 4) {

                intensityDifferenceBits.f =
                    AsCScreen(channelOwnerCursor)->pangyaFeedbackRed - g_screenFullFeedbackIntensity;
                __typeof__(intensityDifferenceBits.f) channelDifference = intensityDifferenceBits.f;
                double absoluteChannelDifference = channelDifference;
                if (intensityDifferenceBits.u & 0x80000000)
                    absoluteChannelDifference = -absoluteChannelDifference;
                double absoluteDifference = absoluteChannelDifference;
                bool channelIsAtFullIntensity = absoluteDifference < colorEqualityTolerance;
                if ((channelIsAtFullIntensity))
                    AsCScreen(channelOwnerCursor)->pangyaFeedbackRed =
                        Random_RangeFloat(-1, kScreenImpactFeedbackMinimumIntensity, kFloat_255_Screen);
            }
        } else {
            float fullIntensity = kFloat_255_Screen;
            self->pangyaFeedbackRed = fullIntensity;
            float retainedFullIntensity = fullIntensity;
            self->pangyaFeedbackGreen = retainedFullIntensity;
            self->pangyaFeedbackBlue = fullIntensity;
        }
    }
}

#pragma pop

} // extern "C"

// Advance HUD panels, handle aiming/menu input, and update tutorial, shot, wind, and impact feedback.
// dt is seconds. Frame counters (blink counts, party ticks, meter transitions)
// advance per call; timer/angle/fade fields use dt. Keep those clocks distinct.
// This coordinator intentionally runs the shared phases even before layouts
// finish loading; only the first panel/input phase requires hudInitialized.
// Order: loaded child displays and panel input; camera/equipment reconciliation;
// tutorial clocks and control visibility; wind/club/feedback updates; pointer actions; impact width/target.
// requestedPanel holds a HudPanel value in its low nibble; bits above (HudPanelHighBitsMask) keep item/spin panel state.
void CScreen::OnProcess(float dt) {
    CScreen* self = this;
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
    if (!(self->hudEnabled))
        return;
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
        UpdateHudLoadedPanelsAndInput(self, inventory, activePlayer, cameraInputReply, spinRangeX, spinRangeY,
                                      itemPanelStateReply, allowed, blocked, itemStatus, spinDone, spinCommitX,
                                      spinCommitY);
    }
    ReconcileHudCameraAndEquipment(self);
    if (STAGE < TutorialStageCount) {
        unsigned int flashDirectionScratch;
        float titleOriginPixels[2];
        if ((self->tutorialIllustrationActive)) {
            EnsureHudInput();
            if (IsHudActionPressed(HudActionCamera)) {
                ((HudTutorialIllustrationInterface*)self->tutorialIllustration)->OnCameraAction();
                (self->tutorialIllustrationActive) = 0;
                DispatchHudMessage(self, ScreenText_TutorialActor, 0, 0, 0);
            }
        }
        unsigned char flashCountScratch;
        UpdateHudTutorialFlashClocks(self, dt, flashDirectionScratch, flashCountScratch);
        float titleDestinationPixels[2];
        if (STAGE == 1) {
            float viewportHeight;
            float titleOriginX;
            viewportHeight = g_view->viewportHeight;
            float titleOriginY;
            titleOriginY = kFloat_0_3_AuxMenu * viewportHeight;
            float halfViewportScale = kFloat_0_5_AuxMenu;
            float viewportWidth = g_view->viewportWidth;
            titleOriginX = (halfViewportScale * viewportWidth - kFloat_50_Screen) - kScreenTutorialTitleInsetX;
            float titleOriginScratch[2];
            titleOriginScratch[1] = titleOriginY;
            titleOriginScratch[0] = titleOriginX - kScreenMarkerInsetX;
            Vec2_Copy(titleOriginPixels, titleOriginScratch);
        } else {
            float halfViewportScale;
            float titleOriginY;
            float viewportHeight;
            float viewportWidth;
            float titleOriginX;
            halfViewportScale = kFloat_0_5_AuxMenu;
            viewportWidth = g_view->viewportWidth;
            titleOriginX = halfViewportScale * viewportWidth - kFloat_50_Screen;
            viewportHeight = g_view->viewportHeight;
            titleOriginY = kFloat_0_3_AuxMenu * viewportHeight;
            float titleOriginScratch[2];
            titleOriginScratch[1] = titleOriginY;
            titleOriginScratch[0] = titleOriginX - kScreenTutorialTitleInsetX;
            Vec2_Copy(titleOriginPixels, titleOriginScratch);
        }
        float destinationHeightFraction;
        float viewportHeight;
        float titleDestinationX;
        float titleDestinationY;
        destinationHeightFraction = kFloat_0_05_Screen;
        viewportHeight = g_view->viewportHeight;
        titleDestinationX = g_view->viewportWidth;
        titleDestinationY = destinationHeightFraction * viewportHeight;
        titleDestinationX = titleDestinationX - kScreenTutorialTitleDestinationInsetX;
        titleDestinationPixels[1] = titleDestinationY;
        titleDestinationPixels[0] = titleDestinationX;
        switch ((self->tutorialTitlePhase)) {
        case TutorialTitleFadingIn: { // Fade in at the stage-specific origin.
            float titleTime;
            float maximumTitleTime;
            titleTime = self->tutorialTitleTimer;
            titleTime += dt;
            maximumTitleTime = kFloat_1_AuxMenu;
            self->tutorialTitleTimer = titleTime;
            if (titleTime > maximumTitleTime) {
                self->modeLabelOpacity = 255;
                self->tutorialTitleTimer = (kFloat_0_AuxMenu);
                (self->tutorialTitlePhase) = TutorialTitleMoving;
            } else
                self->modeLabelOpacity = (int)(kFloat_255_Screen * titleTime);
            Vec2_Copy(&self->tutorialTitleX, titleOriginPixels);
            break;
        }
        case TutorialTitleMoving: { // Move from the origin to the destination in viewport pixels.
            float maximumTitleTime = kFloat_1_AuxMenu;
            __typeof__(self->tutorialTitleTimer) previousTitleTime = self->tutorialTitleTimer;
            __typeof__(self->tutorialTitleTimer + dt) titleTimeAfterStep = self->tutorialTitleTimer + dt;
            __typeof__(self->tutorialTitleTimer + dt) retainedTitleTimeAfterStep = titleTimeAfterStep;
            __typeof__(retainedTitleTimeAfterStep) nextTitleTime = previousTitleTime + dt;
            float titleTime = nextTitleTime;
            self->tutorialTitleTimer = titleTime;
            if (titleTime > maximumTitleTime) {
                float savedMaximum = maximumTitleTime;
                self->tutorialTitleTimer = savedMaximum;
                (self->tutorialTitlePhase) = TutorialTitleArrived;
                DispatchHudMessage(self, ScreenText_TutorialActor, 0, 0, 0);
            }
            __typeof__(self->tutorialTitleTimer) titleMoveWeight = self->tutorialTitleTimer;
            unsigned long long interpolatedTitlePosition, weightedOrigin, weightedDestination;
            weightedDestination = Vec2_ScaleByFloat(titleDestinationPixels, titleMoveWeight);
            weightedOrigin = Vec2_ScaleByFloat(titleOriginPixels, kFloat_1_AuxMenu - self->tutorialTitleTimer);
            interpolatedTitlePosition = Vec2_Add(&weightedOrigin, &weightedDestination);
            Vec2_Copy(&self->tutorialTitleX, &interpolatedTitlePosition);
            break;
        }
        case TutorialTitleArrived:
        default:
            break; // Hold the completed title position.
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
    if (((CGolfBall*)GolfBall__Fv())->m_groundType == 2) {
        terrainAngleLimit = kScreenTerrainIndicatorAngleLimit;
        void* golfDocument = GAME;
        void* holeRecord = (char*)((HudHoleRecordView*)AsCGolfDoc(golfDocument)->holeRecords +
                                   AsCGolfDoc(golfDocument)->currentHoleIndex);
        float ballToHole[3];
        int holePositionWords[3];
        struct Position {
            float v[3];
        };
        *(Position*)holePositionWords = *(Position*)&((HudHoleRecordView*)holeRecord)->position;
        __mi__FRC7WVectorRC7WVector(ballToHole, holePositionWords, GolfBall__Fv());
        float angle = Vector_SignedAngleXZ(ballToHole, &((HudHeadingReferenceView*)golfDocument)->headingReference);
        angle = angle > kFloat_0_AuxMenu ? angle : -angle;
        float terrainHeadingWeight = terrainAngleLimit - angle;
        terrainHeadingWeight = kFloat_0_AuxMenu < terrainHeadingWeight ? terrainHeadingWeight : kFloat_0_AuxMenu;
        int terrainIndicatorAlpha = (int)(kFloat_128_ShotGauge * (terrainHeadingWeight / terrainAngleLimit));
        if (((ClubSelectionState*)InstanceTable_Get())->shotOptionEnabled)
            self->terrainIndicatorColor = ((unsigned int)terrainIndicatorAlpha << 24) | 0xFFF600;
        else
            self->terrainIndicatorColor = ((unsigned int)terrainIndicatorAlpha << 24) | 0xFF96;
    }
    UpdateHudTutorialPointerDelay(self, dt);
    {
        int panelStatus;
        unsigned char scorePanelBlocksInput;
        ProcessHudPointerRegions(self, panelStatus, scorePanelBlocksInput);
    }
    if (g_helpMsg)
        NoticeQueue_UpdateTimers(g_helpMsg, dt);
    if (g_headIcon)
        HeadIcon_UpdatePlayerTimers(g_headIcon, dt);
    {
        float heading;
        void* partyEffects;
        void* impactWidthInventory;
        void* impactWidthPlayer;
        float targetDistance;
        char* channelOwnerCursor;
        HudFeedbackChannelBits intensityDifferenceBits;
        UpdateHudImpactZone(self, inventory, activePlayer, colorEqualityTolerance, proposedTargetOffset,
                            baseImpactRange, maximumTargetOffset, adjustedImpactRange, heading, partyEffects,
                            impactWidthInventory, impactWidthPlayer, targetDistance, channelOwnerCursor,
                            intensityDifferenceBits);
    }
    if (self->skinsInfo)
        ((HudTimedChildInterface*)self->skinsInfo)->UpdateSeconds(dt);
    float remainingDisplaySeconds = (*(float*)&self->displayTimeRemaining) - dt;
    (*(float*)&self->displayTimeRemaining) =
        kFloat_0_AuxMenu < remainingDisplaySeconds ? remainingDisplaySeconds : kFloat_0_AuxMenu;
}

extern "C" {}

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
void ScreenBallPreview_Draw(void*);
void GolfTips_ResetActiveDialogForRoundState();
void WOverlay_DrawTransformedRect(void*, void*, void*, float, void*, int, unsigned int, int);
void Engine_ClearFramebuffer(int, int, int, int, int, float);
void BpObject_SetEnabled(void*, int);
void UIRender_SetDefaultState();
int strcmp(const char*, const char*);

// CWind::DrawDirectionModel takes only the arrow model. This caller also passes the arrow's screen
// position, which the function ignores; it is declared with those arguments so the call stays as compiled.
extern "C" void DrawDirectionModel__5CWindFPv(CWind* self, void* arrowModel, float x, float y);
void PartyItemSlots_Draw(int, int);

extern "C" void BpObject_StartScriptByName(void*, const char*, int);
int BpObject_IsCurrentScriptComplete(void*);
extern "C" void* BpObject_GetOrCreateNamedGroup(void*, const char*);
void UIGroup_SetTintColor(void*, unsigned int);
void UIGroup_SetOpacity(void*, int);
void UITextTable_SetInteger(void*, int, int);
void UITextTable_Draw(void*);

extern unsigned int kScreenFeatherCallbackFloat2Bits,
    kScreenFeatherCallbackFloat0Bits;       // float bit patterns 2.0f and 0.0f for the feather callback
extern const float kScreenSpinMarkerInsetY; // 26.0f: spin marker Y inset
extern const float kFloat_32_screen;
extern const float kScreenBallConditionBaselineInset; // 28.0f: ball-condition marker baseline inset
extern const float kFloat_64_Screen;
extern const float kScreenBallConditionX; // 35.0f: ball-condition marker X coordinate
extern const float kFloat_55_Screen;
extern const float kFloat_256_GolfScreen;
extern const float kScreenShotOverlayX;         // 21.0f: shot overlay X coordinate
extern const float kScreenPatternBaselineInset; // 60.0f: pattern overlay baseline inset
extern const float kScreenPatternX;             // 3.0f: pattern overlay X coordinate
extern const float kFloat_51_ShotGauge;
extern const float kScreenClickIndicatorX;  // 99.0f: click indicator X coordinate
extern const float kScreenGaugeClickInsetX; // 23.0f: gauge click indicator inset
extern const float kFloat_8_GolfScreen;
extern const float kScreenImpactClickBaselineInset; // 71.0f: impact click indicator baseline inset
extern const float kScreenImpactClickOffsetY;       // 12.0f: impact click indicator Y offset
extern const float kScreenImpactClickX;             // 134.0f: impact click indicator X coordinate
extern const float kScreenPracticeOptionsY;         // 245.0f: practice-options Y coordinate
extern const float kScreenCommandGlyphStartX;       // 209.0f: command glyph initial X coordinate
extern const float kScreenCommandAndMenuRowSpacing; // 22.0f: command/menu row spacing
void NamedText_DrawAlignedScaled(void*, const char*, int, float, float, float);
extern "C" void String_ReplaceRange(void*, unsigned int, unsigned int, const char*, const char*, void*);
void appFreeUni__FPv(void*);

#define FIELD(T, p, o) (*(T*)((char*)(p) + (o)))
#define STATE (*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc)
#define PLAYER AsCGolfDoc(STATE)->currentPlayer
#define CALL(p, v, o) Invoke<v, o>(p)
#define QUERY(p, v, o) ((*(int (**)(void*))((char*)FIELD(void*, p, v) + o))(p))
}

struct Renderable {
    virtual void slot0();
    virtual void slot4();
    virtual void slot8();
    virtual void slotC();
    virtual void slot10();
};
// Invoke the no-argument virtual slot O at the interface vtable stored at offset V.
template <int V, int O> static inline void Invoke(void* p) {
    (*(void (**)(void*))((char*)FIELD(void*, p, V) + O))(p);
}
// Invoke the common draw hook through its typed renderable interface.
template <> inline void Invoke<0, 16>(void* p) {
    ((Renderable*)p)->slot8();
}

struct Rect4 {
    float x, y, w, h;
    // Leave rectangle components uninitialized until assigned by the caller.
    Rect4() {}
    // Initialize the rectangle as x/y/width/height in HUD coordinates.
    Rect4(float a, float b, float c, float d) : x(a), y(b), w(c), h(d) {}
    // Access one rectangle component; requires an index in 0..3.
    float& operator[](int i) {
        return (&x)[i];
    }
    // Expose the rectangle storage as the raw pointer expected by layout calls.
    operator void*() {
        return this;
    }
};

struct HudString {
    unsigned int bits;
    unsigned int length;
    char* data;
    // Initialize an empty inline string with no heap allocation.
    HudString() : bits(0), length(0), data(0) {}
    // Free string storage only when the heap-ownership bit is set.
    ~HudString() {
        if (bits >> 31)
            appFreeUni__FPv(data);
    }
    // Return the length from the inline prefix or heap-string length word.
    unsigned int size() const {
        return !(bits >> 31) ? *(const unsigned char*)this & 0x7f : length;
    }
    // Return borrowed NUL-terminated inline or heap characters.
    const char* c_str() const {
        return !(bits >> 31) ? (const char*)this + 1 : data;
    }
    // Replace the whole string: legacy append name is reversed.
    void append(const char* const& text) {
        unsigned int n = size();
        unsigned int allocator;
        volatile unsigned int source;
        *(char*)&allocator = *(volatile char*)&source;
        String_ReplaceRange(this, 0, n, text, text + strlen(text), &allocator);
    }
    // Append at the current end: legacy assign name is reversed.
    void assign(const char* const& text) {
        unsigned int n = size();
        unsigned int allocator;
        volatile unsigned int source;
        *(char*)&allocator = *(volatile char*)&source;
        String_ReplaceRange(this, n, 0, text, text + strlen(text), &allocator);
    }
};

// Return a borrowed named overlay under the registry lock, or null for a missing key/table.
static inline void* Lookup(CScreen* const& self, const char* const& name) {
    InstanceLock_u80179180 lock((char*)&self->nameMutex);
    if (!name || !AsCScreen(self)->nameHashBuckets)
        return 0;
    unsigned int hash;
    {
        void* mutex = (char*)&self->nameMutex;
        OSLockMutex((OSMutex*)(mutex));
        int n = strlen(name);
        unsigned int h = (unsigned char)name[0] & 0x2a;
        int index = n > 0 ? n - 1 : 0;
        h |= (unsigned char)name[index] & 0x15;
        h = (h & 0x3f) | ((unsigned int)n << 6);
        hash = h & (*(unsigned int*)&AsCScreen(self)->nameHashMask);
        OSUnlockMutex((OSMutex*)(mutex));
    }
    void* node = ((void**)AsCScreen(self)->nameHashBuckets)[hash];
    while (node) {
        if (!strcmp(((OverlayRegistryNodeView*)node)->key, name))
            return ((OverlayRegistryNodeView*)node)->value;
        node = ((OverlayRegistryNodeView*)node)->bucketNext;
    }
    return 0;
}

#define DRAWKEY(key, rect, color)                                                                                      \
    {                                                                                                                  \
        void* entry = Lookup(self, key);                                                                               \
        if (entry && ((NamedOverlayView*)entry)->resource) {                                                           \
            WOverlay_DrawTransformedRect(                                                                              \
                ((NamedOverlayView*)entry)->resource, g_view,                                                          \
                Rect4(kFloat_0_AuxMenu, kFloat_0_AuxMenu, kFloat_1_AuxMenu, kFloat_1_AuxMenu), kFloat_0_AuxMenu, rect, \
                0, color, 0);                                                                                          \
        }                                                                                                              \
    }
#define DRAW(off, rect, color) DRAWKEY(screenOverlayStrings + (off), rect, color)
#define RECT(a, b, c, d) Rect4 rect((a), (b), (c), (d))
extern "C" {} // extern "C"

#pragma opt_common_subs off
#pragma opt_strength_reduction off
// Draw active HUD panels, party-item effects, shot feedback, tutorial labels, and command glyphs.
void CScreen::OnDisplay() {
    CScreen* self = this;
    volatile double pad[32];
    volatile char pad2[0x60];
    (void)pad;
    (void)pad2;
    // Phase 1: require ready/visible HUD state, then establish rendering state.
    if (0 == self->hudInitialized || !self->hudDisplayEnabled || self->hudEnabled == 0)
        return;
    if (!self->PollLoadComplete())
        return;
    if (g_drawDriveLine)
        ((CDrawDriveLine*)((HudDisplayState*)g_drawDriveLine))->DrawRibbon();
    Engine_ClearFramebuffer(0, 0, 2, 0, 0, kFloat_1_AuxMenu);
    HudDisplayState* state = (*(HudDisplayState**)&AsCSharedDoc(g_sharedDoc)->golfDoc);

    if (((*(unsigned int*)&AsCGolfDoc(state)->gameStateFlags)) & HudSuppressDrawingState) {
        self->hudDisplayMask = 1;
        if ((*(int*)&AsCSharedDoc(g_sharedDoc)->partyGameMode) != 3)
            return;
        BpObject_SetEnabled(self->partyGolfBaseScript, 0);
        BpObject_SetEnabled(self->timebombScript, 0);
        BpObject_SetEnabled(self->pululuJarashiScript, 0);
        BpObject_SetEnabled(self->featherBadgeScript, 0);
        *(int*)((*(char**)&self->textTable) + 0x28) = 0;
        return;
    }
    UIRender_SetDefaultState();
    void* global = g_sharedDoc;
    int active = 0;
    if (AsCGolfDoc((void*)AsCSharedDoc(global)->golfDoc)->gameStateFlags & HudActiveLayoutState)
        active = 1;

    // Phase 2: draw the widgets selected by HUD phase, tutorial and party mode.
    switch ((self->hudPhase)) {
    case HudInactive:
        if ((*(int*)&AsCSharedDoc(global)->partyGameMode) == 3)
            BpObject_SetEnabled(self->partyGolfBaseScript, 0);
        if (self->holeInfoSet)
            ((CHoleInfoSet*)(self->holeInfoSet))->Draw();
        return;
    case HudShot:
        self->UpdatePanelTransitions();
        self->UpdateCameraAndShotHints();
        self->UpdateClubChangeDisplay();
        self->UpdateSwingCaption();
        break;
    case HudAim:
        self->UpdatePanelTransitions();
        self->UpdateClubChangeDisplay();
        self->UpdateCameraAndShotHints();
        break;
    case HudItems:
        self->UpdatePanelTransitions();
        self->UpdateClubChangeDisplay();
        break;
    case HudSpin: {
        float x;
        float y;
        self->UpdatePanelTransitions();
        x = kFloat_0_AuxMenu;
        y = kFloat_0_AuxMenu;
        IActor_DispatchMessage(self, self, screenOverlayStrings + ScreenText_SpinActor, 8, (void*)(&x), (void*)(&y),
                               (void*)(0), (void*)(0));
        ((ScreenCallFF<0xE0>*)self->strokePointDisplay)->slot0x3C(x, y);
        self->UpdateClubChangeDisplay();
        break;
    }
    case HudShotTransition:
        self->UpdatePanelTransitions();
        self->UpdateClubChangeDisplay();
        break;
    default:
        break;
    }

    global = g_sharedDoc;
    if (AsCGolfDoc(STATE)->tutorialStage < TutorialStageCount) {
        if (self->tutorialGaugeEnabled == 0)
            active = 0;
    } else if (SharedDoc_IsPlayerFlag0Set(PLAYER)) {
        BpObject_SetEnabled(self->ballWindScript, 0);
        active = 0;
    } else {
        BpObject_SetEnabled(self->ballWindScript, 1);
    }
    global = g_sharedDoc;
    if ((*(int*)&AsCSharedDoc(global)->partyGameMode) == 3) {
        unsigned int index = PartyItemSlots_GetItemType(
            PartyItemSlots_GetInstance(), FIELD(unsigned char, (*(void**)&AsCSharedDoc(global)->golfDoc), 0x3c1));
        void* ui = InstanceTable_Get();

        if ((*(unsigned char*)((char*)*(void**)((char*)ui + 4) + 10)) == 3)
            BpObject_SetEnabled(self->partyGolfBaseScript, 0);
        else if (2 >= index)
            BpObject_SetEnabled(self->partyGolfBaseScript, 1);
        BpObject_TryInvokeCurrentCallback(self->partyGolfBaseScript, 0);
    }
    void* obj = self->golfBaseDisplay;
    ((ScreenCallP<0x164>*)self->golfBaseDisplay)->slot0x18(self->partyGolfBaseScript);
    if ((self->hudFlags & HudShotGaugeVisible)) {
        if (!SharedDoc_IsPlayerFlag0Set(PLAYER)) {
            if (self->powerInfo) {
                if (((ScreenCall0<0x164>*)self->golfBaseDisplay)->slot0x24()) {
                    unsigned int color = -1;
                    if (AsCGolfDoc(STATE)->cameraMode)
                        color = 0x7fffffff;

                    WVector pos;
                    IActor_DispatchMessage(self, self, screenOverlayStrings + ScreenText_SpotActor, 2, (void*)(&pos),
                                           (void*)(0), (void*)(0), (void*)(0));
                    ((PowerInfoView*)self->powerInfo)->color = color;
                    ((PowerInfoView*)self->powerInfo)->position = pos;
                    CALL(self->powerInfo, 0, 0x10);
                }
            }
        }
    }
    if (active)
        Layout_DrawWithOpacity(self->layout, kFloat_1_AuxMenu);
    float height = g_option->hudBaselineY;
    if (((ScreenCall0<0xA4>*)self->ballWindDisplay)->slot0x30() && !SharedDoc_IsPlayerFlag0Set(PLAYER))
        self->DrawClubModelOverlay();
    if ((self->hudFlags & 1)) {
        if (AsCGolfDoc(STATE)->tutorialStage >= 2) {
            if (self->tutorialStageVisible[2]) {
                if (self->calipers) {
                    void* manager = AsCSharedDoc(g_sharedDoc)->itemInventory;
                    void* data = GolfPlayer_GetEntryByIndex(STATE, PLAYER);
                    if (FIELD(unsigned int, data, 0xa0) == ItemInventory_GetItemIdAt(manager, 5) ||
                        AsCGolfDoc(STATE)->powerShotMeterOverride)
                        CALL(self->calipers, 0, 0x10);
                }
            }
        }
    }
    BpObject_TryInvokeCurrentCallback(self->ballWindScript, 0);
    obj = self->ballWindDisplay;
    ((ScreenCallP<0xA4>*)self->ballWindDisplay)->slot0x18(self->ballWindScript);
    if (!SharedDoc_IsPlayerFlag0Set(PLAYER) && self->ballPreview)
        ScreenBallPreview_Draw(self->ballPreview);
    if (!SharedDoc_IsPlayerFlag0Set(PLAYER)) {
        if (((ClubSelectionState*)InstanceTable_Get())->selection->clubKind != 3) {
            if (!SharedDoc_IsPlayerFlag0Set(PLAYER)) {
                if (AsCGolfDoc(STATE)->tutorialStage == TutorialStageCount) {
                    float x;
                    int color;
                    float y;
                    IActor_DispatchMessage(self, self, screenOverlayStrings + ScreenText_SpinActor, 7, (void*)(&x),
                                           (void*)(&y), (void*)(&color), (void*)(0));
                    const char* key = screenOverlayStrings + ScreenText_SpinPointKey;
                    int drawColor = color;
                    RECT(x - kScreenMarkerInsetX, y - kScreenSpinMarkerInsetY, kFloat_32_screen, kFloat_32_screen);
                    if (drawColor >> 24)
                        DRAWKEY(key, rect, drawColor);
                }
            }
        }
    }
    if ((*(int*)&AsCSharedDoc(g_sharedDoc)->partyGameMode) == 3) {
        unsigned int manager = (unsigned int)PartyItemSlots_GetInstance();
        if (PartyItemSlots_GetItemType(PartyItemSlots_GetInstance(), PLAYER) == 9) {
            unsigned int update[2] = {kScreenFeatherCallbackFloat2Bits, kScreenFeatherCallbackFloat0Bits};
            BpObject_TryInvokeCurrentCallback(self->featherBadgeScript, (int)update);
            BpObject_Draw(self->featherBadgeScript);
        } else
            BpObject_SetEnabled(self->featherBadgeScript, 0);
        unsigned int found = 0;
        for (int i = 0; i < FIELD(unsigned char, (state = (HudDisplayState*)STATE), 0x2b9); ++i) {
            if (i != FIELD(unsigned char, state, 0x3c1)) {
                if (PartyItemSlots_GetItemType(PartyItemSlots_GetInstance(), i) == 6) {
                    found = 1;
                    break;
                }
            }
        }
        if (found) {
            EnsureHudInput();
            int value = FIELD(int, (void*)manager, 0x64);
            switch (self->pululuAnimationPhase) {
            case PululuHidden:
                if (value) {
                    BpObject_SetEnabled(self->pululuJarashiScript, 1);
                    BpObject_StartScriptByName(self->pululuJarashiScript,
                                               screenOverlayStrings + ScreenText_PululuEnterAnimation, 1);
                    self->pululuAnimationPhase = PululuEntering;
                }
                break;
            case PululuEntering:
                if (!value) {
                    BpObject_StartScriptByName(self->pululuJarashiScript,
                                               screenOverlayStrings + ScreenText_PululuExitAnimation, 1);
                    self->pululuAnimationPhase = PululuLeaving;
                } else if (BpObject_IsCurrentScriptComplete(self->pululuJarashiScript)) {
                    BpObject_StartScriptByName(self->pululuJarashiScript,
                                               screenOverlayStrings + ScreenText_PululuLoopAnimation, 1);
                    self->pululuAnimationPhase = PululuLooping;
                }
                break;
            case PululuLooping:
                if (0 == value) {
                    BpObject_StartScriptByName(self->pululuJarashiScript,
                                               screenOverlayStrings + ScreenText_PululuExitAnimation, 1);
                    self->pululuAnimationPhase = PululuLeaving;
                }
                break;
            case PululuLeaving:
                if (BpObject_IsCurrentScriptComplete(self->pululuJarashiScript)) {
                    BpObject_SetEnabled(self->pululuJarashiScript, 0);
                    self->pululuAnimationPhase = PululuHidden;
                }
                break;
            }
            BpObject_TryInvokeCurrentCallback(self->pululuJarashiScript, 0);
            BpObject_Draw(self->pululuJarashiScript);
        } else {
            BpObject_SetEnabled(self->pululuJarashiScript, 0);
            self->pululuAnimationPhase = 0;
        }
        found = 0;
        for (int j = 0; j < AsCGolfDoc(STATE)->playerCount; ++j) {
            if (PartyItemSlots_GetItemType(PartyItemSlots_GetInstance(), j) == 10) {
                if (j == PLAYER) {
                    found = 0;
                    break;
                }
                found = 1;
            }
        }
        if (found) {
            FIELD(int, self->textTable, 0x28) = 1;
            int remaining = FIELD(int, (void*)manager, 0x4c) - FIELD(int, (void*)manager, 0x48) / TimebombFramesPerTick;
            if (remaining < 0)
                remaining = 0;
            void* pane = BpObject_GetOrCreateNamedGroup(self->timebombScript,
                                                        screenOverlayStrings + ScreenText_CountdownNumberGroup);
            if (remaining > 14)
                UIGroup_SetTintColor(pane, 0xffff9632);
            else
                UIGroup_SetTintColor(pane, 0xff5050ff);
            UIGroup_SetOpacity(pane, 128);
            UITextTable_SetInteger(self->textTable, 0, remaining);
            UITextTable_Draw(self->textTable);
        } else
            FIELD(int, self->textTable, 0x28) = 0;
    }
    if (self->tutorialStageVisible[1]) {
        if (!SharedDoc_IsPlayerFlag0Set(PLAYER)) {
            if (((CGolfBall*)GolfBall__Fv())->m_eventPreDataIndex[0] < 0) {
                unsigned int color;
                switch (((CGolfBall*)GolfBall__Fv())->m_groundType) {
                case 0:
                case 1:
                case 2:
                case 6:
                case 7:
                    color = 0;
                    break;
                case 4:
                    color = 0x8078e577;
                    break;
                case 3:
                case 10:
                    color = 0x80e5ca77;
                    break;
                case 5:
                    color = 0x80ffffff;
                    break;
                case 12:
                    color = 0x808080ff;
                    break;
                case 8:
                case 9:
                case 11:
                default:
                    color = 0x8078e577;
                    break;
                }
                if (color) {
                    void* entry = Lookup(self, screenOverlayStrings + ScreenText_BallConditionKey);
                    if (entry) {
                        if (((NamedOverlayView*)entry)->resource) {
                            Rect4 uv;
                            Rect4 rect;
                            float y = height - kScreenBallConditionBaselineInset;
                            float t = self->groundLieBlend;
                            float remain = kFloat_1_AuxMenu - t;
                            y = kFloat_64_Screen * t + y;
                            rect.w = kFloat_64_Screen;
                            rect.x = kScreenBallConditionX;
                            rect.h = kFloat_64_Screen * remain;
                            rect.y = y - kFloat_10_Screen;
                            uv.x = kFloat_0_AuxMenu;
                            uv.y = self->groundLieBlend;
                            uv.w = kFloat_1_AuxMenu;
                            uv.h = kFloat_1_AuxMenu - uv.y;
                            WOverlay_DrawTransformedRect(((NamedOverlayView*)entry)->resource, g_view, uv,
                                                         kFloat_0_AuxMenu, rect, 0, color, 0);
                        }
                    }
                }
            }
        }
    }
    // Phase 3: draw shot/impact results and optional status, distance and reward overlays.
    Layout_DrawWithOpacity(self->swingResultDisplay, kFloat_1_AuxMenu);
    self->DrawImpactTimingSprite();
    if (self->flyInfo)
        CALL(self->flyInfo, 0, 0x10);
    if (self->modeInfo)
        CALL(self->modeInfo, 0, 0x10);
    if (!AsCGolfDoc(STATE)->cameraMode && self->pangInfo)
        CALL(self->pangInfo, 0, 0x10);
    GolfTips_ResetActiveDialogForRoundState();
    if (self->holeInfoSet)
        ((CHoleInfoSet*)(self->holeInfoSet))->Draw();
    if (self->cameraCaptionOpacity > kFloat_0_AuxMenu) {
        switch (AsCGolfDoc(STATE)->gameStateFlags) {
        case 0x200: {
            int alpha = (int)self->cameraCaptionOpacity;
            const char* key = screenOverlayStrings + ScreenText_TopCameraKey;
            RECT(kFloat_0_AuxMenu, kFloat_55_Screen, kFloat_256_GolfScreen, kFloat_32_screen);
            if ((unsigned char)alpha)
                DRAWKEY(key, rect, ((unsigned int)alpha << 24) | 0xffffff);
            break;
        }
        case 0x40000: {
            unsigned int alpha = (int)self->cameraCaptionOpacity;
            const char* key = screenOverlayStrings + ScreenText_TargetCameraKey;
            RECT(kFloat_0_AuxMenu, kFloat_55_Screen, kFloat_256_GolfScreen, kFloat_32_screen);
            if ((unsigned char)alpha)
                DRAWKEY(key, rect, ((unsigned int)alpha << 24) | 0xffffff);
            break;
        }
        case 0x100: {
            unsigned int alpha = (int)self->cameraCaptionOpacity;
            const char* key = screenOverlayStrings + ScreenText_FreeCameraKey;
            RECT(kFloat_0_AuxMenu, kFloat_55_Screen, kFloat_256_GolfScreen, kFloat_32_screen);
            if ((unsigned char)alpha)
                DRAWKEY(key, rect, ((unsigned int)alpha << 24) | 0xffffff);
            break;
        }
        }
        if (((*(unsigned int*)&AsCGolfDoc(STATE)->gameStateFlags) & 8)) {
            if (AsCGolfDoc(STATE)->cameraMode) {
                unsigned int alpha = (int)self->cameraCaptionOpacity;
                const char* key = screenOverlayStrings + ScreenText_FreeCameraKey;
                RECT(kFloat_0_AuxMenu, kFloat_55_Screen, kFloat_256_GolfScreen, kFloat_32_screen);
                if ((unsigned char)alpha)
                    DRAWKEY(key, rect, ((unsigned int)alpha << 24) | 0xffffff);
            }
        }
    }
    if (self->ingameStatus && self->tutorialStageVisible[5])
        ((CIngameStatus*)(self->ingameStatus))->UpdateAndDraw();
    if (((ClubSelectionState*)InstanceTable_Get())->selection->clubKind != 3) {
        if (AsCGolfDoc(STATE)->tutorialStage == TutorialStageCount) {
            float x = g_view->viewportWidth;

            CWind* wind = CWind::GetInstance();
            DrawDirectionModel__5CWindFPv(wind, self->windIndicatorResource, x - kFloat_50_Screen,
                                          kFloat_5_Screen + height);
            if (self->windInfo)
                CALL(self->windInfo, 0, 0x10);
        }
    }
    // Phase 4: compose party items and directional command glyphs after the main HUD.
    PartyItemSlots_Draw((int)PartyItemSlots_GetInstance(), AsCGolfDoc(STATE)->gameStateFlags);
    if (((ScreenCall0<0x9C>*)self->mapCameraDisplay)->slot0x1C())
        ((ScreenCall0<0x9C>*)self->mapCameraDisplay)->slot0x18();
    if (((ScreenCall0<0xE0>*)self->strokePointDisplay)->slot0x34()) {
        ((ScreenCallP<0xE0>*)self->strokePointDisplay)->slot0x2C(self->ballPreview);
        EnsureHudInput();
        InputManager_RequestPointerInput(GetInputManagerSlot(), 1);
    }
    if (AsCGolfDoc(STATE)->tutorialStage < TutorialStageCount && self->tutorialIllustration)
        ((ScreenCall0<0x78>*)self->tutorialIllustration)->slot0x14();
    float baseH = kFloat_64_Screen;
    float baseX = kScreenShotOverlayX;
    float baseY = height - baseH;
    float baseW = kFloat_128_ShotGauge;
    volatile float unusedRect[4] = {baseX, baseY, baseW, baseH};
    volatile float copyX = baseX;
    volatile float copyY = baseY;
    volatile float copyW = baseW;
    volatile float copyH = baseH;
    if (self->tutorialStageVisible[1]) {
        RECT(kScreenPatternX, height - kScreenPatternBaselineInset, kFloat_128_ShotGauge, kFloat_128_ShotGauge);
        DRAW(ScreenText_PatternKey, rect, 0xffffffff);
    }
    if ((self->hudFlags & 1)) {
        if (AsCGolfDoc(STATE)->tutorialStage >= 2) {
            if (self->tutorialStageVisible[2]) {
                if (!SharedDoc_IsPlayerFlag0Set(PLAYER)) {
                    if ((self->tutorialGaugeBlinkCount >= TutorialBlinkLimit &&
                         FIELD(int, (state = (HudDisplayState*)FIELD(void*, (global = g_sharedDoc), 0xb90)), 0x98) <
                             6) ||
                        FIELD(int, (state = (HudDisplayState*)FIELD(void*, (global = g_sharedDoc), 0xb90)), 0x98) ==
                            6) {
                        if (FIELD(int, (char*)global + FIELD(unsigned char, state, 0x3c1) * 0x1d4, 0x1dc)) {
                            if (((ClubSelectionState*)InstanceTable_Get())->selection->clubKind != 3) {
                                void* gameState = STATE;
                                unsigned char kind = FIELD(unsigned char, gameState, 0x3c0);
                                if (kind == 0 && self->hudPhase == HudShot) {
                                    RECT(kScreenClickIndicatorX, kFloat_2_805DED94 + (height - kFloat_51_ShotGauge),
                                         kFloat_32_screen, kFloat_32_screen);
                                    DRAW(ScreenText_ClickIndicatorKey, rect, 0xffffffff);
                                } else if (kind == 1) {
                                    RECT(kFloat_8_GolfScreen +
                                             (FIELD(float, gameState, 0x2d0) - kScreenGaugeClickInsetX),
                                         kFloat_2_805DED94 + (height - kFloat_51_ShotGauge), kFloat_32_screen,
                                         kFloat_32_screen);
                                    DRAW(ScreenText_ClickIndicatorKey, rect, 0xffffffff);
                                } else if (2 == kind) {
                                    float delta = self->shotIndicatorPulse * kFloat_0_5_AuxMenu;
                                    delta = delta > kFloat_0_AuxMenu ? delta : -delta;
                                    RECT(kScreenImpactClickX,
                                         (kScreenImpactClickOffsetY + (height - kScreenImpactClickBaselineInset)) +
                                             delta,
                                         kFloat_32_screen, kFloat_32_screen);
                                    DRAW(ScreenText_ClickIndicatorKey, rect, 0xffffffff);
                                }
                            } else if (self->hudPhase == HudShot) {
                                RECT(kFloat_8_GolfScreen +
                                         (AsCGolfDoc(STATE)->roundedShotGaugeCursorX - kScreenGaugeClickInsetX),
                                     (height - kFloat_51_ShotGauge) - kFloat_8_GolfScreen, kFloat_32_screen,
                                     kFloat_32_screen);
                                DRAW(ScreenText_ClickIndicatorKey, rect, 0xffffffff);
                            }
                        }
                    }
                }
            }
        }
    }
    if (!((*(unsigned int*)&AsCGolfDoc(STATE)->gameStateFlags) & 16) &&
        AsCGolfDoc(STATE)->tutorialStage == TutorialStageCount)
        IActor_DispatchMessage(self, self, screenOverlayStrings + ScreenText_ItemActor, 4, (void*)(0), (void*)(0),
                               (void*)(0), (void*)(0));
    if (AsCGolfDoc(STATE)->tutorialStage < TutorialStageCount) {
        if (self->modeLabelRenderer) {
            if (AsCSharedDoc(g_sharedDoc)->gameMode != 4) {
                FIELD(unsigned int, self->modeLabelRenderer, 0xc) =
                    ((unsigned int)self->modeLabelOpacity << 24) | 0xffffff;
                const char* text = String_FormatTemporary(screenOverlayStrings + ScreenText_TutorialTitleFormat,
                                                          AsCGolfDoc(STATE)->tutorialStage + 1);
                NamedText_DrawAlignedScaled(self->modeLabelRenderer, text, 0, FIELD(float, self, 0x1c4),
                                            FIELD(float, self, 0x1c8), kFloat_1_AuxMenu);
            }
        }
    } else if (AsCSharedDoc(g_sharedDoc)->gameMode == 5) {
        RECT(kFloat_0_AuxMenu, kScreenPracticeOptionsY, kFloat_32_screen, kFloat_32_screen);
        DRAW(ScreenText_PracticeOptionsKey, rect, 0xffffffff);
    }
    if ((*(unsigned int*)&AsCGolfDoc(STATE)->gameStateFlags) & HudCommandGlyphStates) {
        float x = kScreenCommandGlyphStartX;
        HudString text;
        float spacing = kScreenCommandAndMenuRowSpacing;

        float yoffset = kScreenPulseWrapSpan;

        void* node = AsCGolfDoc(STATE)->shotCommandTail;
        while (node != &AsCGolfDoc(STATE)->shotCommandHead) {
            int kind = FIELD(unsigned int, node, 8);
            if (16u == (unsigned int)kind) {
                node = FIELD(void*, node, 4);
                continue;
            }
            switch (kind) {
            case 1:
                text.append(screenOverlayStrings + ScreenText_UpGlyph);
                break;
            case 2:
                text.append(screenOverlayStrings + ScreenText_DownGlyph);
                break;
            case 4:
                text.append(screenOverlayStrings + ScreenText_LeftGlyph);
                break;
            case 8:
                text.append(screenOverlayStrings + ScreenText_RightGlyph);
                break;
            case 64:
                text.append(screenOverlayStrings + ScreenText_InvalidGlyph);
                break;
            case 32:
                text.append(screenOverlayStrings + ScreenText_InvalidGlyph);
                float savedSpacing = spacing;
                float savedX = x;
                x = savedX - savedSpacing;
                break;
            }
            node = FIELD(void*, node, 4);
            if (node != &AsCGolfDoc(STATE)->shotCommandHead && FIELD(unsigned int, node, 8) == 16)
                text.assign(screenOverlayStrings + ScreenText_SmallGlyphSuffix);
            if (self->statusTextRenderer)
                NamedText_DrawAlignedScaled(self->statusTextRenderer, text.c_str(), 0, x, height - yoffset,
                                            kFloat_1_AuxMenu);
            float savedSpacing2 = spacing;
            float savedValue = x + savedSpacing2;
            x = savedValue;
        }
    }
    if (self->accuracyInfo && !SharedDoc_IsPlayerFlag0Set(PLAYER))
        CALL(self->accuracyInfo, 0, 0x10);
    if (self->skinsInfo)
        CALL(self->skinsInfo, 0, 0x10);
    if (self->nearestPinInfo)
        CALL(self->nearestPinInfo, 0, 0x10);
    if (self->displayTimeRemaining > kFloat_0_AuxMenu)
        GridSurface_DiagnosticStub(g_view, screenOverlayStrings + ScreenText_UpdatedText, kScreenUpdatedTextX,
                                   kFloat_100_Screen);
}

extern "C" {

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

struct ScreenMutex {
    char data[4];
};
extern void* WOverlay_Create(void*, void*, void*);

extern void* appMallocUni__FUl(unsigned int);

// A ring link and a separate bucket-chain link share each 24-byte registry node.
struct RingNode {
    void* value;
    void* key;
    unsigned char flag;
    char pad1[3];
    RingNode* next;
    RingNode* prev;
    unsigned int bucketNextAddress;
};
struct Entry {
    char pad[0x64];
    RingNode* secondary;
    char pad68[4];
    RingNode* primary;
    RingNode* free;
    int capacity;
    unsigned int mask;
    char pad7C[4];
    unsigned int* table;
    ScreenMutex lock;
};

#pragma use_lmw_stmw on
struct StringEntry {
    char text[32];
    void* manager;
    // Initialize an overlay-name entry without an owned resource yet.
    StringEntry() {
        manager = 0;
    }
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
    union {
        StringEntry* value;
        unsigned int index;
        RingNode* tail;
    } scratch;
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
            if (next == n)
                next = 0;
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
                n->next = n;
                n->prev = n;
                OSUnlockMutex((OSMutex*)mutex);
            } else {
                n->prev = tail;
                n->next = tail->next;
                tail->next = n;
                n->next->prev = n;
                OSUnlockMutex((OSMutex*)mutex);
                n = tail;
            }
            self->secondary = n;
        }
    } else {
        if (value)
            delete value;
    }
}
#pragma use_lmw_stmw reset

extern int BpObject_PollLoad(void*);
extern int Layout_PollLoadComplete(void*);

// Return 1 only when every required script/layout has loaded; optional tutorial is checked if present.
int CScreen::PollLoadComplete() {
    if (!BpObject_PollLoad(ballWindScript))
        return 0;
    if ((*(int*)&g_sharedDoc->partyGameMode) == 3) {
        if (!BpObject_PollLoad(partyGolfBaseScript))
            return 0;
        if (!BpObject_PollLoad(timebombScript))
            return 0;
        if (!BpObject_PollLoad(pululuJarashiScript))
            return 0;
        if (!BpObject_PollLoad(featherBadgeScript))
            return 0;
    }
    if (!Layout_PollLoadComplete(layout))
        return 0;
    if (!Layout_PollLoadComplete(strokePointDisplay))
        return 0;
    if (!Layout_PollLoadComplete(mapCameraDisplay))
        return 0;
    if (!Layout_PollLoadComplete(ballWindDisplay))
        return 0;
    if (!Layout_PollLoadComplete(golfBaseDisplay))
        return 0;
    if (!Layout_PollLoadComplete(swingResultDisplay))
        return 0;
    if (tutorialIllustration && !Layout_PollLoadComplete(tutorialIllustration))
        return 0;
    return 1;
}

struct ObjE0 {
    char pad[0xE0];
    void** vtable;
};
struct Obj9C {
    char pad[0x9C];
    void** vtable;
};
struct Obj164 {
    char pad[0x164];
    void** vtable;
};

struct VE0B {
    char pad[224];
};
class VE0 : public VE0B {
public:
    virtual void v08();
    virtual void v0C();
    virtual void v10();
    virtual void v14();
    virtual void v18();
    virtual void v1C();
    virtual void v20();
    virtual void v24();
    virtual void v28();
    virtual void v2C();
    virtual void v30();
    virtual void v34();
    virtual void v38();
    virtual void v3C();
    virtual void v40();
    virtual void v44();
    virtual void v48();
    virtual void v4C();
    virtual void v50();
    virtual void v54();
    virtual void v58();
    virtual void v5C();
    virtual void v60();
    virtual void v64();
    virtual void v68();
    virtual int v6C();
};
struct V9CB {
    char pad[156];
};
class V9C : public V9CB {
public:
    virtual void v08();
    virtual void v0C();
    virtual void v10();
    virtual void v14();
    virtual void v18();
    virtual void v1C();
    virtual void v20();
    virtual void v24();
    virtual void v28();
    virtual void v2C();
    virtual void v30();
    virtual void v34();
    virtual void v38();
    virtual void v3C();
    virtual void v40();
    virtual void v44();
    virtual void v48();
    virtual void v4C();
    virtual int v50();
};
struct V164B {
    char pad[356];
};
class V164 : public V164B {
public:
    virtual void v08();
    virtual void v0C();
    virtual void v10();
    virtual void v14();
    virtual void v18();
    virtual void v1C();
    virtual void v20();
    virtual void v24();
    virtual void v28();
    virtual void v2C();
    virtual void v30();
    virtual void v34();
    virtual void v38();
    virtual void v3C();
    virtual void v40();
    virtual void v44();
    virtual void v48();
    virtual void v4C();
    virtual void v50();
    virtual void v54();
    virtual void v58();
    virtual void v5C();
    virtual void v60();
    virtual void v64();
    virtual void v68();
    virtual void v6C();
    virtual void v70();
    virtual int v74();
};
// Require script completion and idle panel transitions before accepting HUD input.
int CScreen::AreHudTransitionsComplete() {
    if (!BpObject_IsCurrentScriptComplete(ballWindScript))
        return 0;
    if ((*(int*)&g_sharedDoc->partyGameMode) == 3) {
        if (!BpObject_IsCurrentScriptComplete(partyGolfBaseScript))
            return 0;
        if (!BpObject_IsCurrentScriptComplete(timebombScript))
            return 0;
        if (!BpObject_IsCurrentScriptComplete(pululuJarashiScript))
            return 0;
        if (!BpObject_IsCurrentScriptComplete(featherBadgeScript))
            return 0;
    }
    if (!((VE0*)strokePointDisplay)->v6C())
        return 0;
    if (!((V9C*)mapCameraDisplay)->v50())
        return 0;
    int c = ((V164*)golfBaseDisplay)->v74();
    return c != 0;
}

struct ObjA4 {
    char pad[0xA4];
    void** vtable;
};
struct Obj78 {
    char pad[0x78];
    void** vtable;
};
struct Obj74 {
    char pad[0x74];
    void** vtable;
};
struct Obj10 {
    char pad[0x10];
    void** vtable;
};
struct ObjE0B {
    char pad[0xE0];
    void** vtable;
};
struct Obj9CB {
    char pad[0x9C];
    void** vtable;
};
struct Obj164B {
    char pad[0x164];
    void** vtable;
};
extern unsigned int partyMeterGroupNames[];
extern char s_clubOverlayGroupNames[];

extern char s_timebombCountdownTextEntries[];

extern void Layout_SetElementEnabled(void*, int, int);
extern "C" int UIGroup_SetEnabled(void*, int);

#pragma use_lmw_stmw on
#pragma opt_propagation off
// Initialize loaded HUD layouts, meter opacity, party countdown text, and club group visibility.
void CScreen::InitializeLoadedLayouts() {
    char* item;
    hudDisplayMask = 1;
    panelOpenRequested = 0;
    itemPanelOpenRequested = 0;
    if (hudInitialized != 0) {
        if ((*(int*)&AsCSharedDoc(g_sharedDoc)->partyGameMode) == 3) {
            struct Names {
                unsigned int value[3];
            };
            Names names = *(Names*)partyMeterGroupNames;
            int index = partyMeterIndex;
            if (index >= 0 && (unsigned int)index < 3) {
                UIGroup_SetOpacity(
                    BpObject_GetOrCreateNamedGroup((void*)partyGolfBaseScript, (const char*)names.value[index]), 0x7F);
            }
        }
        meterOpacity = 0xFF;
        ((ScreenCallI<0xA4>*)ballWindDisplay)->slot0x64(-1);
        ((ScreenCallI<0x164>*)golfBaseDisplay)->slot0x50(-1);
    }
    void* mgr = (*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc);
    if (AsCGolfDoc(mgr)->tutorialStage < TutorialStageCount)
        hudPhase = HudAim;
    for (int slot = 0; slot < 4; ++slot)
        Layout_SetElementEnabled((void*)layout, slot, 0);
    ((ScreenCall0<0xE0>*)strokePointDisplay)->slot0x10();
    ((ScreenCall0<0x9C>*)mapCameraDisplay)->slot0x10();
    ((ScreenCall0<0xA4>*)ballWindDisplay)->slot0x10();
    ((ScreenCall0<0x164>*)golfBaseDisplay)->slot0x10();
    ((ScreenCall0<0x74>*)swingResultDisplay)->slot0x10();
    if (tutorialIllustration) {
        ((ScreenCall0<0x78>*)tutorialIllustration)->slot0x10();
    }
    ((ScreenCallI<0x164>*)golfBaseDisplay)->slot0x34(1);
    ((ScreenCallI<0x164>*)golfBaseDisplay)->slot0x38(1);
    if ((*(int*)&AsCSharedDoc(g_sharedDoc)->partyGameMode) == 3) {
        ((CFontNumDraw*)textTable)
            ->RegisterEntries(s_timebombCountdownTextEntries, 1, (void*)timebombScript,
                              screenOverlayStrings + ScreenText_CountdownFontResource);
        BpObject_StartScriptByName((void*)timebombScript, screenOverlayStrings + ScreenText_CountdownAnimation, 1);
        BpObject_SetEnabled((void*)pululuJarashiScript, 0);
    }
    struct Table {
        char bytes[34][16];
    };
    Table name;
    for (unsigned int i = 0; (int)i < ClubOverlayCount; ++i) {
        name = *(Table*)s_clubOverlayGroupNames;
        if ((unsigned int)i <= 0x11) {
            item = name.bytes[i * 2];
            if (strcmp(item, screenOverlayStrings + ScreenText_EmptyText)) {
                UIGroup_SetEnabled(BpObject_GetOrCreateNamedGroup((void*)ballWindScript, item), 0);
            }
            item += 0x10;
            if (strcmp(item, screenOverlayStrings + ScreenText_EmptyText)) {
                UIGroup_SetEnabled(BpObject_GetOrCreateNamedGroup((void*)ballWindScript, item), 0);
            }
        }
    }
}
#pragma opt_propagation reset
#pragma use_lmw_stmw reset

// Vtables of the three sub-objects CScreen::UpdateShotMeterPresentation drives: the one at 0x338 (vtable pointer at +0xA4), the one at 0x33C
// (vtable pointer at +0x164) and the one at 0x340 (vtable pointer at +0x74).
// The classes are polymorphic: the vtable pointer follows the members, a vtable has an 8-byte header and 4-byte
// slots, so the slot at vtable offset 0x58 is the 21st virtual function. The x* functions only fill unused slots.
struct BallWindMeterInterface {
    char pad[0xA4];
    virtual void x0();
    virtual void x1();
    virtual void x2();
    virtual void x3();
    virtual void x4();
    virtual void x5();
    virtual void x6();
    virtual void x7();
    virtual void x8();
    virtual void f2C(int);
    virtual void x10();
    virtual void f34(int);
    virtual void x12();
    virtual void x13();
    virtual void x14();
    virtual void x15();
    virtual void x16();
    virtual void x17();
    virtual void x18();
    virtual void x19();
    virtual void f58(int);
    virtual void f5C(int);
    virtual void f60(float);
    virtual void f64(int);
};
struct GolfBaseMeterInterface {
    char pad[0x164];
    virtual void x0();
    virtual void x1();
    virtual void x2();
    virtual void x3();
    virtual void x4();
    virtual void x5();
    virtual void f20(int);
    virtual void x7();
    virtual void x8();
    virtual void f2C(int, int);
    virtual void f30(void*);
    virtual void x11();
    virtual void x12();
    virtual void x13();
    virtual void x14();
    virtual void x15();
    virtual void x16();
    virtual void x17();
    virtual void f50(int);
    virtual void f54(float);
    virtual void f58(float);
    virtual void f5C();
    virtual void x22();
    virtual void x23();
    virtual void f68(float);
};
struct SwingResultMeterInterface {
    char pad[0x74];
    virtual void x0();
    virtual void x1();
    virtual void x2();
    virtual void x3();
    virtual void x4();
    virtual void x5();
    virtual void x6();
    virtual void f24();
    virtual int f28();
};

typedef CGolfDoc HudMeterGolfStateView;

struct NameTbl {
    const char* s[3];
};

struct GameP {
    char p[0xA];
    unsigned char mode;
};
struct Game {
    char p[4];
    GameP* gp;
};

#define MGR ((*(HudMeterGolfStateView**)&AsCSharedDoc(g_sharedDoc)->golfDoc))
#define GLB_AD8 ((*(int*)&AsCSharedDoc(g_sharedDoc)->partyGameMode))

// Presentation modes are separate from CScreen's cached display-mask values.
enum ShotGolfMeterDisplay {
    kGolfMeterHidden = 0,
    kGolfMeterPower = 1,
    kGolfMeterTerrain10 = 2,
    kGolfMeterTerrain15 = 3,
    kGolfMeterTerrain20 = 4
};
enum ShotHudMeterMask {
    kHudMeterInactive = 0,
    kHudMeterTransparent = 1,
    kHudMeterPower = 2,
    kHudMeterTerrain10 = 4,
    kHudMeterTerrain15 = 8,
    kHudMeterTerrain20 = 16
};

// Select the shot-meter presentation and update tutorial blinking, impact feedback, and meter fading.
#pragma push
#pragma always_inline on
// Select ordinary power or sampled terrain-marker display (CGolfBase2d slot 6).
static inline void SelectGolfMeterDisplay(GolfBaseMeterInterface* meter, int mode) {
    meter->f20(mode);
}
// Set the selected terrain marker and sample count (CGolfBase2d slot 9).
static inline void SetGolfTerrainMarkerRange(GolfBaseMeterInterface* meter, int index, int count) {
    meter->f2C(index, count);
}
// Copy 21 Y-offset/color pairs from the caller-owned style buffer (slot 10).
static inline void CopyGolfTerrainMarkerStyles(GolfBaseMeterInterface* meter, void* styles) {
    meter->f30(styles);
}
// Set the packed RGBA tint of the golf meter (slot 18).
static inline void SetGolfMeterTint(GolfBaseMeterInterface* meter, int rgba) {
    meter->f50(rgba);
}
// Show/hide the ball-selector controls (CBallWind2d slot 9).
static inline void ShowBallSelectorControls(BallWindMeterInterface* meter, int visible) {
    meter->f2C(visible);
}
// Show/hide layout element 0; its artwork role is unresolved (slot 11).
static inline void ShowBallWindElement0(BallWindMeterInterface* meter, int visible) {
    meter->f34(visible);
}
// Draw indicator element 3 separately from the clipped script (slot 20).
static inline void DrawBallWindIndicator3Separately(BallWindMeterInterface* meter, int enabled) {
    meter->f58(enabled);
}
// Draw indicator element 1 separately from the clipped script (slot 21).
static inline void DrawBallWindIndicator1Separately(BallWindMeterInterface* meter, int enabled) {
    meter->f5C(enabled);
}
// Set both ball/wind indicators' packed RGBA tint (slot 23).
static inline void SetBallWindIndicatorTint(BallWindMeterInterface* meter, int rgba) {
    meter->f64(rgba);
}
#pragma pop

// Select meter widgets and update tutorial blinking, shot feedback, and tint.
void CScreen::UpdateShotMeterPresentation() {
    // Phase 1: choose terrain/ordinary meter and initial opacity from camera state.
    int usesTerrainMeter = ((Game*)InstanceTable_Get())->gp->mode == 3; // putting uses the sampled terrain meter
    unsigned int alpha = 0xFF;
    if (MGR->cameraMode)
        alpha = 0x7F;

    DrawBallWindIndicator3Separately((BallWindMeterInterface*)ballWindDisplay, 0);
    DrawBallWindIndicator1Separately((BallWindMeterInterface*)ballWindDisplay, 0);
    ShowBallSelectorControls((BallWindMeterInterface*)ballWindDisplay, 0);
    ShowBallWindElement0((BallWindMeterInterface*)ballWindDisplay, 0);
    SelectGolfMeterDisplay((GolfBaseMeterInterface*)golfBaseDisplay, kGolfMeterHidden);

    if (!(hudFlags & HudShotGaugeVisible)) {
        if (hudDisplayMask != kHudMeterTransparent)
            hudDisplayMask = kHudMeterTransparent;
    } else {
        hudDisplayMask = kHudMeterInactive;
    }

    if ((MGR->gameStateFlags & HudMeterFadeStates) || (hudFlags & HudShotGaugeVisible) || MGR->shotPhase) {
        if (usesTerrainMeter) {
            switch (terrainSampleCount) {
            case 10:
                if (hudDisplayMask != kHudMeterTerrain10) {
                    SelectGolfMeterDisplay((GolfBaseMeterInterface*)golfBaseDisplay, kGolfMeterTerrain10);
                    hudDisplayMask = kHudMeterTerrain10;
                }
                break;
            case 15:
                if (hudDisplayMask != kHudMeterTerrain15) {
                    SelectGolfMeterDisplay((GolfBaseMeterInterface*)golfBaseDisplay, kGolfMeterTerrain15);
                    hudDisplayMask = kHudMeterTerrain15;
                }
                break;
            case 20:
                if (hudDisplayMask != kHudMeterTerrain20) {
                    SelectGolfMeterDisplay((GolfBaseMeterInterface*)golfBaseDisplay, kGolfMeterTerrain20);
                    hudDisplayMask = kHudMeterTerrain20;
                }
                break;
            }
            SetGolfTerrainMarkerRange((GolfBaseMeterInterface*)golfBaseDisplay, terrainMarkerIndex, terrainSampleCount);
            TerrainProfileSlot tmp[3];
            for (int i = 0; i < 3; i++) {
                tmp[i].f0 = terrainProfile[i].f0;
                tmp[i].i0 = terrainProfile[i].i0;
                tmp[i].f1 = terrainProfile[i].f1;
                tmp[i].i1 = terrainProfile[i].i1;
                tmp[i].f2 = terrainProfile[i].f2;
                tmp[i].i2 = terrainProfile[i].i2;
                tmp[i].f3 = terrainProfile[i].f3;
                tmp[i].i3 = terrainProfile[i].i3;
                tmp[i].f4 = terrainProfile[i].f4;
                tmp[i].i4 = terrainProfile[i].i4;
                tmp[i].f5 = terrainProfile[i].f5;
                tmp[i].i5 = terrainProfile[i].i5;
                tmp[i].f6 = terrainProfile[i].f6;
                tmp[i].i6 = terrainProfile[i].i6;
            }
            CopyGolfTerrainMarkerStyles((GolfBaseMeterInterface*)golfBaseDisplay, tmp);
        } else if (hudDisplayMask != kHudMeterPower) {
            SelectGolfMeterDisplay((GolfBaseMeterInterface*)golfBaseDisplay, kGolfMeterPower);
            hudDisplayMask = kHudMeterPower;
        }
    } else if (hudDisplayMask != kHudMeterTransparent) {
        SelectGolfMeterDisplay((GolfBaseMeterInterface*)golfBaseDisplay, kGolfMeterPower);
        hudDisplayMask = kHudMeterTransparent;
    }

    if (SharedDoc_IsPlayerFlag0Set(MGR->currentPlayer)) {
        SelectGolfMeterDisplay((GolfBaseMeterInterface*)golfBaseDisplay, kGolfMeterHidden);
        hudDisplayMask = kHudMeterTransparent;
    }

    // Phase 2: run meter/gauge tutorial blink counters (updates, not seconds).
    if (tutorialMeterEnabled && MGR->tutorialStage < 6) {
        if (tutorialMeterBlinkCount < TutorialBlinkLimit) {
            if (tutorialGaugeEnabled)
                SelectGolfMeterDisplay((GolfBaseMeterInterface*)golfBaseDisplay, kGolfMeterPower);
            else
                SelectGolfMeterDisplay((GolfBaseMeterInterface*)golfBaseDisplay, kGolfMeterHidden);
            if (tutorialMeterBlinkFrame < 2) {
                if (tutorialMeterBlinkCount % 2 != 0) {
                    if (meterPresentation != 1)
                        meterPresentation = 1;
                    ShowBallSelectorControls((BallWindMeterInterface*)ballWindDisplay, 1);
                }
                tutorialMeterBlinkFrame = tutorialMeterBlinkFrame + 1;
                return;
            }
            tutorialMeterBlinkFrame = 0;
            tutorialMeterBlinkCount = tutorialMeterBlinkCount + 1;
            ShowBallSelectorControls((BallWindMeterInterface*)ballWindDisplay, 1);
            return;
        } else if (!tutorialGaugeEnabled) {
            ShowBallSelectorControls((BallWindMeterInterface*)ballWindDisplay, 1);
        }
    }

    if (tutorialGaugeEnabled && MGR->tutorialStage < 6) {
        if (tutorialGaugeBlinkCount < TutorialBlinkLimit) {
            if (tutorialGaugeBlinkFrame < 2) {
                if (tutorialGaugeBlinkCount % 2 != 0) {
                    if (meterPresentation != 2)
                        meterPresentation = 2;
                    ShowBallSelectorControls((BallWindMeterInterface*)ballWindDisplay, 1);
                    SelectGolfMeterDisplay((GolfBaseMeterInterface*)golfBaseDisplay, kGolfMeterPower);
                    void* receiver2429_fn_800ED6BC = (void*)InstanceTable_Get();
                    ((GolfBaseMeterInterface*)golfBaseDisplay)
                        ->f68(kFloat_2_805DED94 * ShotParameters_GetGoodImpactHalfWidth(receiver2429_fn_800ED6BC));
                    DrawBallWindIndicator3Separately((BallWindMeterInterface*)ballWindDisplay, 1);
                    ((BallWindMeterInterface*)ballWindDisplay)->f60(MGR->roundedShotGaugeCursorX);
                    ((GolfBaseMeterInterface*)golfBaseDisplay)->f54(MGR->roundedShotGaugeCursorX);
                } else {
                    if (meterPresentation != 1)
                        meterPresentation = 1;
                    ShowBallSelectorControls((BallWindMeterInterface*)ballWindDisplay, 1);
                    SelectGolfMeterDisplay((GolfBaseMeterInterface*)golfBaseDisplay, kGolfMeterHidden);
                }
                tutorialGaugeBlinkFrame = tutorialGaugeBlinkFrame + 1;
                return;
            }
            tutorialGaugeBlinkFrame = 0;
            tutorialGaugeBlinkCount = tutorialGaugeBlinkCount + 1;
        } else if (tutorialGaugeBlinkCount == 6) {
            SelectGolfMeterDisplay((GolfBaseMeterInterface*)golfBaseDisplay, kGolfMeterPower);
            void* receiver2446_fn_800ED6BC = (void*)InstanceTable_Get();
            ((GolfBaseMeterInterface*)golfBaseDisplay)
                ->f68(kFloat_2_805DED94 * ShotParameters_GetGoodImpactHalfWidth(receiver2446_fn_800ED6BC));
            DrawBallWindIndicator3Separately((BallWindMeterInterface*)ballWindDisplay, 1);
            ((BallWindMeterInterface*)ballWindDisplay)->f60(MGR->roundedShotGaugeCursorX);
            ((GolfBaseMeterInterface*)golfBaseDisplay)->f54(MGR->roundedShotGaugeCursorX);
            ShowBallSelectorControls((BallWindMeterInterface*)ballWindDisplay, 1);
            tutorialGaugeBlinkCount = tutorialGaugeBlinkCount + 1;
            return;
        }
    } else if (!tutorialGaugeEnabled && MGR->tutorialStage < 6 && tutorialMeterEnabled) {
        if (meterPresentation != 1)
            meterPresentation = 1;
        ShowBallSelectorControls((BallWindMeterInterface*)ballWindDisplay, 1);
        SelectGolfMeterDisplay((GolfBaseMeterInterface*)golfBaseDisplay, kGolfMeterHidden);
    } else if (!tutorialGaugeEnabled && !tutorialMeterEnabled && MGR->tutorialStage < 6) {
        SelectGolfMeterDisplay((GolfBaseMeterInterface*)golfBaseDisplay, kGolfMeterHidden);
        ShowBallSelectorControls((BallWindMeterInterface*)ballWindDisplay, 0);
    }

    if ((!tutorialMeterEnabled || !tutorialGaugeEnabled) && MGR->tutorialStage < 6)
        return;

    // Phase 3: select the active meter representation after tutorial gates.
    switch (hudDisplayMask) {
    case kHudMeterInactive:
        meterPresentation = 0;
        break;
    case kHudMeterTransparent:
        if (meterPresentation != 1)
            meterPresentation = 1;
        if (((Game*)InstanceTable_Get())->gp->mode != 3 && MGR->tutorialStage == 6)
            ShowBallWindElement0((BallWindMeterInterface*)ballWindDisplay, 1);
        if (!SharedDoc_IsPlayerFlag0Set(MGR->currentPlayer))
            ShowBallSelectorControls((BallWindMeterInterface*)ballWindDisplay, 1);
        alpha = 0;
        break;
    case kHudMeterPower:
        SelectGolfMeterDisplay((GolfBaseMeterInterface*)golfBaseDisplay, kGolfMeterPower);
        if (((Game*)InstanceTable_Get())->gp->mode != 3 && MGR->tutorialStage == 6)
            ShowBallWindElement0((BallWindMeterInterface*)ballWindDisplay, 1);
        if (!SharedDoc_IsPlayerFlag0Set(MGR->currentPlayer))
            ShowBallSelectorControls((BallWindMeterInterface*)ballWindDisplay, 1);
        break;
    case kHudMeterTerrain10:
        SelectGolfMeterDisplay((GolfBaseMeterInterface*)golfBaseDisplay, kGolfMeterTerrain10);
        if (!SharedDoc_IsPlayerFlag0Set(MGR->currentPlayer))
            ShowBallSelectorControls((BallWindMeterInterface*)ballWindDisplay, 1);
        break;
    case kHudMeterTerrain15:
        SelectGolfMeterDisplay((GolfBaseMeterInterface*)golfBaseDisplay, kGolfMeterTerrain15);
        if (!SharedDoc_IsPlayerFlag0Set(MGR->currentPlayer))
            ShowBallSelectorControls((BallWindMeterInterface*)ballWindDisplay, 1);
        break;
    case kHudMeterTerrain20:
        SelectGolfMeterDisplay((GolfBaseMeterInterface*)golfBaseDisplay, kGolfMeterTerrain20);
        if (!SharedDoc_IsPlayerFlag0Set(MGR->currentPlayer))
            ShowBallSelectorControls((BallWindMeterInterface*)ballWindDisplay, 1);
        break;
    }

    if (meterFadeFrame == 0 && hudInitialized != 0) {
        if (GLB_AD8 == 3) {
            NameTbl names = *(NameTbl*)partyMeterGroupNames;
            int idx = partyMeterIndex;
            if (idx >= 0 && (unsigned int)idx < 3) {
                UIGroup_SetOpacity(BpObject_GetOrCreateNamedGroup((void*)partyGolfBaseScript, names.s[idx]),
                                   alpha >> 1);
            }
        }
        meterOpacity = alpha;
        alpha |= 0xFFFFFF00;
        SetBallWindIndicatorTint((BallWindMeterInterface*)ballWindDisplay, alpha);
        SetGolfMeterTint((GolfBaseMeterInterface*)golfBaseDisplay, alpha);
    }

    if (usesTerrainMeter) {
        if (MGR->shotPhase < 2)
            ((GolfBaseMeterInterface*)golfBaseDisplay)->f54(MGR->roundedShotGaugeCursorX);
        else
            ((GolfBaseMeterInterface*)golfBaseDisplay)->f54(MGR->shotPowerGaugeX);
        if (!SharedDoc_IsPlayerFlag0Set(MGR->currentPlayer)) {
            if ((hudFlags & HudShotGaugeVisible) || MGR->shotPhase)
                DrawBallWindIndicator1Separately((BallWindMeterInterface*)ballWindDisplay, 1);
        }
        ((BallWindMeterInterface*)ballWindDisplay)->f60(MGR->roundedShotGaugeCursorX);
    } else {
        if (MGR->shotPhase == 0 && swingFeedbackPending) {
            ((SwingResultMeterInterface*)swingResultDisplay)->f24();
            swingFeedbackPending = 0;
        }
        HudMeterGolfStateView* m = MGR;
        if (m->shotPhase < 2) {
            if (impactFeedbackStarted) {
                ((GolfBaseMeterInterface*)golfBaseDisplay)->f5C();
                impactFeedbackStarted = 0;
            }
            ((GolfBaseMeterInterface*)golfBaseDisplay)->f54(MGR->roundedShotGaugeCursorX);
        } else if (m->shotPhase == 2) {
            ((GolfBaseMeterInterface*)golfBaseDisplay)->f54(m->shotPowerGaugeX);
        } else if (m->shotPhase == 3) {
            if (*(int*)((char*)g_sharedDoc + m->currentPlayer * 0x1D4 + 0x1DC) == 0 && impactFeedbackStarted == 0) {
                ((GolfBaseMeterInterface*)golfBaseDisplay)
                    ->f58(
                        m->impactFeedbackGaugeX); // Prepared impact-feedback gauge position, also saved/restored by CRival.
                impactFeedbackStarted = impactFeedbackStarted + 1;
            }
            if (!((SwingResultMeterInterface*)swingResultDisplay)->f28())
                StartDirectionalSwingFeedback();
            ((GolfBaseMeterInterface*)golfBaseDisplay)->f54(MGR->shotPowerGaugeX);
        }
        void* receiver2538_fn_800ED6BC = (void*)InstanceTable_Get();
        ((GolfBaseMeterInterface*)golfBaseDisplay)
            ->f68(kFloat_2_805DED94 * ShotParameters_GetGoodImpactHalfWidth(receiver2538_fn_800ED6BC));
        if (!SharedDoc_IsPlayerFlag0Set(MGR->currentPlayer) && hudDisplayMask != kHudMeterTransparent)
            DrawBallWindIndicator3Separately((BallWindMeterInterface*)ballWindDisplay, 1);
        ((BallWindMeterInterface*)ballWindDisplay)->f60(MGR->roundedShotGaugeCursorX);
    }

    // A shot-completion request cancels the meter fade and restores full opacity.
    // Phase 4: consume fade-reset requests and update the visible meter fade.
    if (MGR->shotMeterFadeResetRequested && (hudFlags & 1) && meterFadeFrame) {
        if (hudInitialized) {
            if (GLB_AD8 == 3) {
                NameTbl names = *(NameTbl*)partyMeterGroupNames;
                int idx = partyMeterIndex;
                if (idx >= 0 && (unsigned int)idx < 3) {
                    UIGroup_SetOpacity(BpObject_GetOrCreateNamedGroup((void*)partyGolfBaseScript, names.s[idx]), 0x7F);
                }
            }
            meterOpacity = 0xFF;
            SetBallWindIndicatorTint((BallWindMeterInterface*)ballWindDisplay, -1);
            SetGolfMeterTint((GolfBaseMeterInterface*)golfBaseDisplay, -1);
        }
        meterFadeFrame = 0;
        impactFeedbackStarted = 0;
    }

    if (MGR->gameStateFlags & HudMeterFadeStates) {
        int n = meterFadeFrame;
        if (n < 0x23) {
            if (n >= 0x1E) {
                if (n == 0x22)
                    alpha = 0;
                else
                    alpha = 255 - (n - 0x1D) * 255 / 5;
                if (hudInitialized) {
                    if (GLB_AD8 == 3) {
                        NameTbl names = *(NameTbl*)partyMeterGroupNames;
                        int idx = partyMeterIndex;
                        if (idx >= 0 && (unsigned int)idx < 3) {
                            UIGroup_SetOpacity(BpObject_GetOrCreateNamedGroup((void*)partyGolfBaseScript, names.s[idx]),
                                               alpha >> 1);
                        }
                    }
                    meterOpacity = alpha;
                    alpha |= 0xFFFFFF00;
                    SetBallWindIndicatorTint((BallWindMeterInterface*)ballWindDisplay, alpha);
                    SetGolfMeterTint((GolfBaseMeterInterface*)golfBaseDisplay, alpha);
                }
            }
            meterFadeFrame = meterFadeFrame + 1;
        }
    }
}

} // extern "C"

#undef MGR
#undef GLB_AD8

extern "C" {

struct GameInputManagerView {
    char pad[0x334];
    void* controls;
    void* other;
};

extern int Input_IsActionButtonDown(void*, int, int);

typedef CGolfDoc HudGolfStateView;
typedef CGolfDoc HudImpactStateView;
struct HudSharedStateView {
    char pad[0xB90];
    HudGolfStateView* game;
};
struct U80180710_Root {
    char pad[0xB90];
    HudSharedStateView* manager;
};

extern const float kScreenImpactAtlasSpriteWidth; // 39.0f: impact atlas sprite width
extern const float kScreenImpactAtlasUStep;       // 0.609375f: impact atlas U step (39/64)
extern const float kScreenImpactAtlasVStep;       // 0.1328125f: impact atlas V step (17/128)
extern double kScreenSignedIntToDoubleBias;
extern int s_impactTimingQuadTemplate[];

void Engine_SetTextureStageResource(int, void*);
void Engine_SetTexturePalette(int, void*);
void Engine_SetFlexibleVertexFormat(int);
void DrawPrimitiveUP(int, int, void*, int);

void Layout_SetElementValueState(void*, int, int);

struct U80180710_VT {
    char pad[0x18];
    void (*set)(void*, int, int);
    void (*finish)(void*);
};
struct U80180710_Widget {
    char pad[0x74];
    U80180710_VT* vt;
};
struct U80180710_Object {
    char pad0[0x24];
    void* child;
    char pad1[0x160];
    int done;
    char pad2[0x124];
    void* list;
    char pad3[0x74];
    int color;
    char pad4[0x14];
    void* panel;
};

struct U80180710_ActionVT {
    char pad[0x34];
    void (*action44)(void*);
    void (*action)(void*, int);
};
struct U80180710_Action {
    char pad[0x9C];
    U80180710_ActionVT* vt;
};
struct U80180710_ChoiceVT {
    char pad[0x50];
    void (*choice)(void*, int);
};
struct U80180710_Choice {
    char pad[0xA4];
    U80180710_ChoiceVT* vt;
};

struct U80180710_OuterVT {
    char pad[0x1C];
    int (*call)(void*, int);
};
struct U80180710_Outer {
    char pad[0xA4];
    U80180710_OuterVT* vt;
};

struct U80180710_InputVT {
    char pad[0x3C];
    void (*m3c)(void*, int);
    char pad2[0x2C];
    int (*m6c)(void*);
    int (*m70)(void*);
    int (*m74)(void*);
    char pad3[0x4];
    void (*m7c)(void*);
};
struct U80180710_Obj330VT {
    char pad[0x64];
    void (*m64)(void*);
    char pad2[0x4];
    int (*m6c)(void*);
    int (*m70)(void*);
};
struct U80180710_Input334VT {
    char pad[0x48];
    void (*m48)(void*);
    void (*m4c)(void*);
    int (*m50)(void*);
};
struct U80180710_VirtualObj330Prefix {
    char pad[0xE0];
};
struct U80180710_VirtualObj330 : U80180710_VirtualObj330Prefix {
    virtual void v00();
    virtual void v01();
    virtual void v02();
    virtual void v03();
    virtual void v04();
    virtual void v05();
    virtual void v06();
    virtual void v07();
    virtual void v08();
    virtual void v09();
    virtual void v0a();
    virtual void v0b();
    virtual void v0c();
    virtual void v0d();
    virtual void v0e();
    virtual void v0f();
    virtual void v10();
    virtual void v11();
    virtual void v12();
    virtual void v13();
    virtual void v14();
    virtual void v15();
    virtual void v16();
    virtual void m64();
    virtual void m68();
    virtual int m6c();
    virtual int m70();
};
struct HudMapCameraTransitionInterfacePrefix {
    char pad[0x9C];
};
struct HudMapCameraTransitionInterface : HudMapCameraTransitionInterfacePrefix {
    virtual void v00();
    virtual void v01();
    virtual void v02();
    virtual void v03();
    virtual void v04();
    virtual void v05();
    virtual void v06();
    virtual void v07();
    virtual void v08();
    virtual void v09();
    virtual void v0a();
    virtual void v0b();
    virtual void v0c();
    virtual void v0d();
    virtual void v0e();
    virtual void v0f();
    virtual void BeginOpening();
    virtual void BeginClosing();
    virtual int IsTransitionIdle();
};
struct U80180710_Input {
    char pad[0x164];
    U80180710_InputVT* vt;
};
struct U80180710_Obj330 {
    char pad[0xE0];
    U80180710_Obj330VT* vt;
};
struct U80180710_Input334 {
    char pad[0x9C];
    U80180710_Input334VT* vt;
};
struct U80180710_InputPrefix {
    char pad[0x164];
};
struct U80180710_VirtualToggle : U80180710_InputPrefix {
    virtual void t00();
    virtual void t01();
    virtual void t02();
    virtual void t03();
    virtual void t04();
    virtual void t05();
    virtual void t06();
    virtual void t07();
    virtual void t08();
    virtual void t09();
    virtual void t0a();
    virtual void t0b();
    virtual void t0c();
    virtual void m3c(int);
    virtual void t10();
    virtual void t11();
    virtual void t12();
    virtual void t13();
    virtual void t14();
    virtual void t15();
    virtual void t16();
    virtual void t17();
    virtual void t18();
    virtual void t19();
    virtual void t1a();
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
// (StartOpeningTransition, StartClosingTransition, IsTransitionIdle): open resets action
// latches and fades from zero over five update
// calls; close starts at full opacity over seven calls; idle means neither opening
// nor closing (it does not mean fully open). These counts belong to map_camera_2d_panel.cpp.
// stroke m70 = fully open query, m64 = request open. A query is not an action.
// panelsReady is cleared while waiting and raised only by the matching stable
// case. Changing requestedPanel does not make its widgets ready in that call.
void CScreen::UpdatePanelTransitions() {
    if (((HudSharedStateView*)g_sharedDoc)->game->tutorialStage < 6 && tutorialAimEnabled != 0) {
        int count = tutorialAimBlinkCount;
        if (count < 6) {
            if (tutorialAimBlinkFrame < 2) {
                if (count % 2) {
                    ((U80180710_VirtualToggle*)golfBaseDisplay)->m7c();
                    ((U80180710_VirtualToggle*)golfBaseDisplay)->m3c(0);
                } else {
                    ((U80180710_VirtualToggle*)golfBaseDisplay)->m3c(1);
                }
                ++tutorialAimBlinkFrame;
                return;
            }
            tutorialAimBlinkFrame = 0;
            ++tutorialAimBlinkCount;
            return;
        }
        if (count == 6) {
            ((U80180710_VirtualToggle*)golfBaseDisplay)->m3c(1);
            requestedPanel = 1;
            previousPanel = 1;
            ++tutorialAimBlinkCount;
            panelOpenRequested = 0;
        }
    }
    if (panelsReady && requestedPanel == previousPanel) {
        if (((HudSharedStateView*)g_sharedDoc)->game->tutorialStage == 6)
            return;
    }
    if (requestedPanel != previousPanel)
        panelsReady = 0;
    if (!((U80180710_VirtualObj330*)strokePointDisplay)->m6c())
        return;
    if (!((HudMapCameraTransitionInterface*)mapCameraDisplay)->IsTransitionIdle())
        return;
    if (!((U80180710_VirtualToggle*)golfBaseDisplay)->m74())
        return;
    switch (SharedDoc_IsPlayerFlag0Set(((HudSharedStateView*)g_sharedDoc)->game->currentPlayer)) {
    case 0:
        break;
    default:
        return;
    }
    switch (requestedPanel) {
    case HudPanelNone:
        switch (previousPanel) {
        case 3:
            break;
        case HudPanelAim: {
            ((U80180710_VirtualToggle*)golfBaseDisplay)->m70();
        } break;
        case HudPanelAimClosing: {
            ((U80180710_VirtualToggle*)golfBaseDisplay)->m70();
        } break;
        case HudPanelCamera: {
            ((HudMapCameraTransitionInterface*)mapCameraDisplay)->BeginClosing();
        } break;
        }
        panelsReady = 1;
        break;
    case HudPanelAim:
        switch (previousPanel) {
        case HudPanelNone:
            if (ingameStatus && GolfState_AllowsStatusTransition(ingameStatus) &&
                !((CIngameStatus*)(ingameStatus))->IsShownOrShowing())
                ((CIngameStatus*)(ingameStatus))->BeginShow();
            break;
        case HudPanelAim:
            if (panelOpenRequested) {
                if (ingameStatus && GolfState_AllowsStatusTransition(ingameStatus) &&
                    !((CIngameStatus*)(ingameStatus))->IsShownOrShowing())
                    ((CIngameStatus*)(ingameStatus))->BeginShow();
                if (hudPhase != HudShotTransition) {
                    ((U80180710_VirtualToggle*)golfBaseDisplay)->m6c();
                }
                panelOpenRequested = 0;
            } else
                panelsReady = 1;
            break;
        case HudPanelCamera:
            ((HudMapCameraTransitionInterface*)mapCameraDisplay)->BeginClosing();
            break;
        }
        break;
    case HudPanelAimClosing:
        switch (previousPanel) {
        case 2:
            panelsReady = 1;
            break;
        }
        break;
    case HudPanelItems:
        switch (previousPanel) {
        case HudPanelAim:
            ((U80180710_VirtualToggle*)golfBaseDisplay)->m70();
            break;
        case HudPanelCamera:
            ((HudMapCameraTransitionInterface*)mapCameraDisplay)->BeginClosing();
            break;
        case HudPanelItems:
            if (itemPanelOpenRequested) {
                itemPanelOpenRequested = 0;
                itemPanelReady = 1;
            } else
                panelsReady = 1;
            break;
        }
        break;
    case HudPanelSpin:
        switch (previousPanel) {
        case HudPanelAim:
            ((U80180710_VirtualToggle*)golfBaseDisplay)->m70();
            break;
        case HudPanelCamera:
            ((HudMapCameraTransitionInterface*)mapCameraDisplay)->BeginClosing();
            break;
        case HudPanelSpin:
            if (!((U80180710_VirtualObj330*)strokePointDisplay)->m70())
                ((U80180710_VirtualObj330*)strokePointDisplay)->m64();
            else
                panelsReady = 1;
            break;
        }
        break;
    case HudPanelCamera:
        switch (previousPanel) {
        case HudPanelAim:
            ((U80180710_VirtualToggle*)golfBaseDisplay)->m70();
            break;
        case HudPanelCamera:
            if (panelOpenRequested) {
                if (hudPhase != HudShotTransition) {
                    ((HudMapCameraTransitionInterface*)mapCameraDisplay)->BeginOpening();
                }
                if (ingameStatus && GolfState_AllowsStatusTransition(ingameStatus) &&
                    !((CIngameStatus*)(ingameStatus))->IsShownOrShowing())
                    ((CIngameStatus*)(ingameStatus))->BeginShow();
                panelOpenRequested = 0;
            } else
                panelsReady = 1;
            break;
        }
        break;
    }
}

#pragma use_lmw_stmw on
// Animate club changes and enable the named overlay groups for the selected club.
void CScreen::UpdateClubChangeDisplay() {
    struct Table {
        char bytes[0x220];
    };
    int i;
    char* item;
    void* menu = InstanceTable_Get();
    void* user = ((ClubSelectionState*)menu)->selection;
    ((U80180710_Outer*)ballWindDisplay)->vt->call(ballWindDisplay, ((ClubSelectionSnapshot*)user)->clubIndex);

    int phase = hudPhase;
    // Phase 1: shot transition suppresses club-change controls and retires old state.
    if (phase == HudShotTransition) {
        int status = clubChangeDisplayState;
        if (status == 1 || status == 2) {
            char* names = (char*)screenOverlayStrings;
            BpObject_StartScriptByName(ballWindScript, names + ScreenText_ClubChangeOutAnimation, 1);
            for (i = 0; i < ClubOverlayCount; ++i) {
                char buffer[0x220];
                *(Table*)buffer = *(Table*)s_clubOverlayGroupNames;
                if ((unsigned)i <= 0x11) {
                    item = buffer + i * 0x20;
                    if (strcmp(item, names + ScreenText_EmptyText)) {
                        UIGroup_SetEnabled(BpObject_GetOrCreateNamedGroup(ballWindScript, item), 0);
                    }
                    char* second = item + 0x10;
                    if (strcmp(second, names + ScreenText_EmptyText)) {
                        UIGroup_SetEnabled(BpObject_GetOrCreateNamedGroup(ballWindScript, second), 0);
                    }
                }
            }
        }
        menu = InstanceTable_Get();
        ((ClubSelectionState*)menu)->clubMenuState = 2;
        clubChangeDisplayState = 0;
    } else {
        menu = InstanceTable_Get();
        int menuState = ((ClubSelectionState*)menu)->clubMenuState;
        if (menuState == 3) {
            if (clubChangeDisplayState != 1) {
                clubChangeDisplayState = 1;
                BpObject_StartScriptByName(ballWindScript, screenOverlayStrings + ScreenText_ClubChangeInAnimation, 1);
            }
        } else if (((ClubSelectionState*)InstanceTable_Get())->clubMenuState == 0 && clubChangeDisplayState != 0) {
            clubChangeDisplayState = 2;
            menu = InstanceTable_Get();
            user = ((ClubSelectionState*)menu)->selection;
            int selected = *((unsigned char*)user + 8);
            if (displayedClubIndex != selected) {
                menu = InstanceTable_Get();
                user = ((ClubSelectionState*)menu)->selection;
                displayedClubIndex = *((unsigned char*)user + 8);
                char buffer[0x220];
                for (i = 0; i < ClubOverlayCount; ++i) {
                    *(Table*)buffer = *(Table*)s_clubOverlayGroupNames;
                    if ((unsigned)i <= 0x11) {
                        item = buffer + i * 0x20;
                        if (strcmp(item, screenOverlayStrings + ScreenText_EmptyText)) {
                            UIGroup_SetEnabled(BpObject_GetOrCreateNamedGroup(ballWindScript, item), 0);
                        }
                        char* second = item + 0x10;
                        if (strcmp(second, screenOverlayStrings + ScreenText_EmptyText)) {
                            UIGroup_SetEnabled(BpObject_GetOrCreateNamedGroup(ballWindScript, second), 0);
                        }
                    }
                }
                char current[0x220];
                selected = displayedClubIndex;
                *(Table*)current = *(Table*)s_clubOverlayGroupNames;
                if ((unsigned)selected <= 0x11) {
                    item = current + selected * 0x20;
                    if (strcmp(item, screenOverlayStrings + ScreenText_EmptyText)) {
                        UIGroup_SetEnabled(BpObject_GetOrCreateNamedGroup(ballWindScript, item), 1);
                    }
                    char* second = item + 0x10;
                    if (strcmp(second, screenOverlayStrings + ScreenText_EmptyText)) {
                        UIGroup_SetEnabled(BpObject_GetOrCreateNamedGroup(ballWindScript, second), 1);
                    }
                }
            }
            // Display-state cases below drive named club groups; keep each enable/disable
            // pair together so an outgoing club does not leave its companion highlight visible.
        } else if (((ClubSelectionState*)InstanceTable_Get())->clubMenuState == 1 && clubChangeDisplayState != 0) {
            int status = clubChangeDisplayState;
            if (status != 3) {
                clubChangeDisplayState = 3;
                BpObject_StartScriptByName(ballWindScript, screenOverlayStrings + ScreenText_ClubChangeOutAnimation, 1);
                char buffer[0x220];
                for (i = 0; i < ClubOverlayCount; ++i) {
                    *(Table*)buffer = *(Table*)s_clubOverlayGroupNames;
                    if ((unsigned)i <= 0x11) {
                        item = buffer + i * 0x20;
                        if (strcmp(item, screenOverlayStrings + ScreenText_EmptyText)) {
                            UIGroup_SetEnabled(BpObject_GetOrCreateNamedGroup(ballWindScript, item), 0);
                        }
                        char* second = item + 0x10;
                        if (strcmp(second, screenOverlayStrings + ScreenText_EmptyText)) {
                            UIGroup_SetEnabled(BpObject_GetOrCreateNamedGroup(ballWindScript, second), 0);
                        }
                    }
                }
            }
        } else {
            clubChangeDisplayState = 0;
        }
    }
}
#pragma use_lmw_stmw reset

// CMapCamera2d's secondary vptr is at +0x9C. These are per-layout-update
// requests: setters mark layoutDirty; UpdateFrame consumes/resets the requests.
// +34 edge indicators, +38 highlighted mode (0 none, 1..4 mode indicators),
// +3C controls, +44 action input. See map_camera_2d_panel.cpp for the CMapCamera2d methods that read them.
struct HudMapCameraHintPrefix {
    char pad[0x9C];
};
struct HudMapCameraHintInterface : HudMapCameraHintPrefix {
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
    virtual void RequestEdgeIndicators();
    virtual void SetHighlightedMode(int);
    virtual void RequestControls();
    virtual void v40();
    virtual void EnableInputForFrame();
};
struct U80180710_ChoicePrefix {
    char pad[0xA4];
};
struct U80180710_VirtualChoice : U80180710_ChoicePrefix {
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
    virtual void m50(int);
};

// Update camera-action hints and the shot-control prompt from input and actor state.
void CScreen::UpdateCameraAndShotHints() {
    if (!((GameInputManagerView*&)GetInputManagerSlot())) {
        if (!s_inputManagerInitialized) {
            InputManager_Init(s_gameInputManager);
            __register_global_object(s_gameInputManager, (void*)InputManager_Dtor,
                                     s_gameInputManagerDestructorRegistration_Screen);
            s_inputManagerInitialized = 1;
        }
        ((GameInputManagerView*&)GetInputManagerSlot()) = (GameInputManagerView*)s_gameInputManager;
    }
    if (!Input_IsActionButtonDown(((GameInputManagerView*&)GetInputManagerSlot()), HudActionHoldCamera, -1)) {
        int mode = ((HudSharedStateView*)g_sharedDoc)->game->cameraMode;
        if (mode && mode != 4) {
            ((HudMapCameraHintInterface*)mapCameraDisplay)->RequestEdgeIndicators();
        }
    }
    int result;
    IActor_DispatchMessage(this, this, screenOverlayStrings + ScreenText_CameraActor, 8, (void*)(&result), (void*)(0),
                           (void*)(0), (void*)(0));
    // Camera actor replies -> highlighted indicator: 1->none, 2->4, 3->1,
    // 7->2, 8->3, other->none. These are layout mode indicators, not camera
    // world coordinates; changing actor modes requires keeping this mapping.
    switch (result) {
    case 1:
        ((HudMapCameraHintInterface*)mapCameraDisplay)->SetHighlightedMode(0);
        break;
    case 2:
        ((HudMapCameraHintInterface*)mapCameraDisplay)->SetHighlightedMode(4);
        break;
    case 3:
        ((HudMapCameraHintInterface*)mapCameraDisplay)->SetHighlightedMode(1);
        break;
    case 7:
        ((HudMapCameraHintInterface*)mapCameraDisplay)->SetHighlightedMode(2);
        break;
    case 8:
        ((HudMapCameraHintInterface*)mapCameraDisplay)->SetHighlightedMode(3);
        break;
    default:
        ((HudMapCameraHintInterface*)mapCameraDisplay)->SetHighlightedMode(0);
        break;
    }
    int phase = hudPhase;
    if (phase != HudItems && phase != HudSpin) {
        HudGolfStateView* game = ((HudSharedStateView*)g_sharedDoc)->game;
        if (game->tutorialStage < 6) {
            if (tutorialClubChangeEnabled)
                ((HudMapCameraHintInterface*)mapCameraDisplay)->EnableInputForFrame();
        } else {
            ((HudMapCameraHintInterface*)mapCameraDisplay)->EnableInputForFrame();
        }
    }
    if (((HudSharedStateView*)g_sharedDoc)->game->cameraMode != 4)
        ((HudMapCameraHintInterface*)mapCameraDisplay)->RequestControls();
    ((U80180710_VirtualChoice*)ballWindDisplay)->m50(0);

    HudGolfStateView* game = ((HudSharedStateView*)g_sharedDoc)->game;
    if (game->gameStateFlags & 0x80) {
        void* p12 = GolfBall__Fv();
        int* state = &((CGolfBall*)p12)->m_state;
        if (!*state && !(*(int*)&AsCSharedDoc(g_sharedDoc)->partyGameMode)) {
            HudGolfStateView* managerGame = ((HudSharedStateView*)g_sharedDoc)->game;
            if (!SharedDoc_IsPlayerFlag0Set(managerGame->currentPlayer) &&
                ((HudSharedStateView*)g_sharedDoc)->game->tutorialStage == 6) {
                int held;
                unsigned char pressed;
                IActor_DispatchMessage(this, this, screenOverlayStrings + ScreenText_GolfRuleActor, 0x1D,
                                       (void*)(&held), (void*)(0), (void*)(0), (void*)(0));
                IActor_DispatchMessage(this, this, screenOverlayStrings + ScreenText_GolfRuleActor, 0x2F,
                                       (void*)((int*)&pressed), (void*)(0), (void*)(0), (void*)(0));
                if (!(held & 4) && !pressed)
                    ((U80180710_VirtualChoice*)ballWindDisplay)->m50(1);
            }
        }
    }
}

struct U80180710_DrawVirtual {
    virtual void v00();
    virtual void v01();
    virtual void v02();
    virtual void v03();
    virtual void v04();
    virtual void v05();
    virtual void v06();
    virtual void v07();
    virtual void v08();
    virtual void v09();
    virtual void v0a();
    virtual void v0b();
    virtual void v0c();
    virtual void v0d();
    virtual void v0e();
    virtual void v0f();
    virtual void v10();
    virtual void v11();
    virtual void v12();
    virtual void v13();
    virtual void v14();
    virtual void v15();
    virtual void v16();
    virtual void v17();
    virtual void v18();
    virtual void v19();
    virtual void* get(void*);
    virtual void* get2(void*);
};
struct U80180710_DrawRoot {
    char pad[0xA4];
    U80180710_DrawVirtual* obj;
};

// View of the swing-result display object (member swingResultDisplay, vtable at +0x74); only set()/finish() are used here.
struct SwingResultDisplayPrefix {
    char pad[0x74];
};
struct SwingResultDisplayView : SwingResultDisplayPrefix {
    virtual void v08();
    virtual void v0c();
    virtual void v10();
    virtual void v14();
    virtual void set(int, int);
    virtual void finish();
};
struct U80180710_VirtualInput : U80180710_InputPrefix {
    // CGolfBase2d::IsFadeSettled (golf_base_2d_hud_layout.cpp): 0 during fade states 1/3, 1 in hidden/settled states 0/2; this is not an
    // input-ready/visible test. +64/+68 mark the layout dirty on change.
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
    // CGolfBase2d::IsFadeSettled (golf_base_2d_hud_layout.cpp): 0 during fade states 1/3,
    // 1 in hidden/settled states 0/2. This is NOT an input-ready/visible test.
    virtual int IsCaptionFadeSettled();
};

struct U80180710_13DC {
    char pad0[0x32c];
    void* list;
    char pad1[0xc];
    void* panel;
};

enum SwingCaptionElement {
    kShotPhaseCaption = 0,
    kMotionCaptionFirst = 1,
    kMotionCaptionSecond = 2,
    kNonPutterMotionCaption = 3,
    kSwingCaptionElementCount = 4
};
enum SwingCaptionValue {
    kAimCaptionValue = 0,
    kPowerCaptionValue = 1,
    kImpactCaptionValue = 2,
    kAlternatePowerDecidedCaptionValue = 3
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
void CScreen::UpdateSwingCaption() {
    int i;
    i = 0;
    for (; i < kSwingCaptionElementCount; ++i)
        Layout_SetElementEnabled(layout, i, 0);
    HudGolfStateView* game = ((HudSharedStateView*)g_sharedDoc)->game;
    if (!(game->gameStateFlags & HudActiveLayoutState))
        return;
    if (!((U80180710_VirtualInput*)golfBaseDisplay)->IsCaptionFadeSettled())
        return;
    Layout_SetElementEnabled(layout, kShotPhaseCaption, 1);
    game = ((HudSharedStateView*)g_sharedDoc)->game;
    unsigned char state = game->shotPhase;
    switch (state) {
    case 0:
        Layout_SetElementValueState(layout, kShotPhaseCaption, kAimCaptionValue);
        return;
    case 1:
        Layout_SetElementValueState(layout, kShotPhaseCaption, kPowerCaptionValue);
        game = ((HudSharedStateView*)g_sharedDoc)->game;
        if (!game->motionSwingState)
            return;
        Layout_SetElementEnabled(layout, kMotionCaptionFirst, 1);
        Layout_SetElementEnabled(layout, kMotionCaptionSecond, 1);
        {
            void* p = InstanceTable_Get();
            p = *(void**)((char*)p + 4);
            if (((ClubSelectionSnapshot*)p)->clubKind != 3)
                Layout_SetElementEnabled(layout, kNonPutterMotionCaption, 1);
        }
        return;
    case 2:
        if (game->alternateMotionPowerDecided)
            Layout_SetElementValueState(layout, kShotPhaseCaption, kAlternatePowerDecidedCaptionValue);
        else
            Layout_SetElementValueState(layout, kShotPhaseCaption, kImpactCaptionValue);
        break;
    }
}

// Draw the impact timing sprite from its atlas during the unassisted impact phase.
void CScreen::DrawImpactTimingSprite() {
    U80180710_Root* root = (U80180710_Root*)g_sharedDoc;
    HudGolfStateView* game = ((HudSharedStateView*)root)->game;
    int player = game->currentPlayer;
    char* playerData = (char*)root + player * 0x1D4;
    if (*(int*)(playerData + 0x1DC) != 0)
        return;
    if (game->shotPhase != 3)
        return;
    if (!(hudFlags & HudShotGaugeVisible)) {
        if ((*(unsigned int*)&AsCGolfDoc(game)->gameStateFlags) != 0x80)
            return;
    }

    int index = AsCGolfDoc(game)->impactTimingSpriteIndex;
    void* panel = ballPreview;
    void* material = ((BallPreviewMaterialView*)panel)->material;
    int frame = index * 5 + 70;
    if (frame < 40)
        return;
    if (frame >= 100)
        return;
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
        union {
            unsigned int packed;
            unsigned char color[4];
        };
        float u, v;
    };
    struct Vertices {
        Vertex v[4];
    } verts;
    verts = *(Vertices*)s_impactTimingQuadTemplate;
    Vertex* v = verts.v;
    int top = 383;
    int bottom = 400;
    v[0].x = ix0;
    v[0].y = top;
    v[0].color[3] = meterOpacity;
    v[0].u = uv[0];
    v[0].v = uv[1];
    v[1].x = ix1;
    v[1].y = top;
    v[1].color[3] = meterOpacity;
    v[1].u = uv[2];
    v[1].v = uv[1];
    v[2].x = ix0;
    v[2].y = bottom;
    v[2].color[3] = meterOpacity;
    v[2].u = uv[0];
    v[2].v = uv[3];
    v[3].x = ix1;
    v[3].y = bottom;
    v[3].color[3] = meterOpacity;
    v[3].u = uv[2];
    v[3].v = uv[3];
    Engine_SetTextureStageResource(0, ((U80180710_DrawRoot*)g_view)->obj->get(material));
    Engine_SetTexturePalette(0, ((U80180710_DrawRoot*)g_view)->obj->get2(material));
    Engine_SetFlexibleVertexFormat(0x11F005);
    DrawPrimitiveUP(0x98, 2, &verts, 0x18);
}

// Classify impact direction relative to the Pangya target and start the swing feedback display.
void CScreen::StartDirectionalSwingFeedback() {
    HudImpactStateView* game = (HudImpactStateView*)((HudSharedStateView*)g_sharedDoc)->game;
    int result = 0;
    if (game->pangyaAssistState != 1 && game->impactGrade == 4)
        result = 1;
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
    ((SwingResultDisplayView*)swingResultDisplay)->set(direction, mf == 1);
    ((SwingResultDisplayView*)swingResultDisplay)->finish();
    swingFeedbackPending = 1;
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

struct Vec3 {
    int x, y, z;
};
struct Tri {
    int v[3];
};

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

struct F9c {
    char d[0x9c];
};
struct Fa4 {
    char d[0xa4];
};
struct F164 {
    char d[0x164];
};
struct F74 {
    char d[0x74];
};
struct F78 {
    char d[0x78];
};
struct Fe0 {
    char d[0xe0];
};

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
    virtual void ResetSelections();       // +14, return status ignored by this ABI view
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
    virtual void SetPangyaZoneWidth(float);   // +64, marks layout dirty on change
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

extern const float kScreenDropCameraClearance;         // 192.0f: drop-camera vertical clearance
extern const float kScreenTerrainProfileRangeLimit;    // 1.6f: terrain-profile normalization range
extern const float kScreenTerrainProfileColorMinDelta; // -8.0f: terrain-profile color delta lower bound
extern const float kScreenTerrainProfileColorScale;    // 0.125f: terrain-profile color delta scale

void* CQuadTree_GetInstance(void);
void CQuadTree_OverlappedWithObject(void*, void*, float);
void NoticeQueue_ShowById(void*, int, int);
void* Object_GetChild(void*);
V0* TaskActorTable_FindByName(void*, char*);
void GetGroundPoint__9CQuadTreeF7WVectorbPib(void*, void*, void*, int, int, int);

void BallRing_UpdateGroundLie(void*, void*);
void Hud_SetCurrentPlayerDisplayValue(void*, void*);
void Hud_UpdateNearestPinPlayer(void*);
void NoticeQueue_ResetAndActivate(void*);
void ScreenBallPreview_SelectCurrentPlayerBall(void*);

void Screen_GetOrLoadBallResource(void*, int);

void SkinsInfo_StartResultDisplay(void*, int, int);

// Synchronous actor packet used by CGolfRule::HandleMsg (golf_rule_round_flow_and_shot_sim.cpp):
// message 7 calls CGolfRule::EnterState with word 2; the selector and remaining
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
struct HudMessagePositionWords {
    float v[3];
};
struct HudBallTriangleView {
    char precedingFields[0x28];
    int groundTriangleIndex;
};
// Offset-biased view anchored at document + playerIndex*stride, not a standalone player object.
struct HudMessagePlayerSlotView {
    char precedingHandles[0x1DC];
    void* strokePointHandle;
    void* illustrationHandle;
};
// Borrow the two player-specific handles forwarded to the stroke and tutorial displays.
static inline HudMessagePlayerSlotView* HudMessagePlayerSlot(void* document, int playerIndex) {
    return (HudMessagePlayerSlotView*)((char*)document + playerIndex * kSharedPlayerRecordStride);
}
enum HudColorBlendLanes {
    kPowerColorStartUnshiftedLanes = 0xFF006A,
    kPowerColorEndUnshiftedLanes = 0xFF001E,
    kPowerColorStartShiftedLanes = 0x10096,
    kPowerColorEndShiftedLanes = 0xC80042,
    kPowerColorSelectedBytes = 0xFF000000,
    kPowerColorSelectedMiddleByte = 0xFF00
};

#pragma push
#pragma always_inline on
// Set up drop bounds/camera and activate the map panel; world-space scratch stays caller-owned.
// Payload pointers are borrowed for this synchronous call; no pointer is retained.
static inline void EnterHudDropMode(CScreen* self, const ScreenMessage* message,
                                    HudMessagePositionWords& dropBoundsTestPosition,
                                    HudMessagePositionWords& dropGroundQueryPosition, WVector& dropGroundPosition) {
    // Payload: none; uses current HUD/shared state.
    if (0 != ((AuxiliaryMenuMessageView*)self->auxiliaryMenu)) {
        if (0 == ((HudMessageSharedStateView*)g_sharedDoc)->h->dropRequest) {
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

                packet.completionWord = zero;
                V0* golfRuleActor = TaskActorTable_FindByName(actorTable, golfRuleActorName);
                if (0 != golfRuleActor) {
                    golfRuleActor->m2c(&packet);
                }
            }
            ballPosition = (Vec3*)GolfBall__Fv();

            dropGroundQueryPosition = *(HudMessagePositionWords*)ballPosition;
            GetGroundPoint__9CQuadTreeF7WVectorbPib(&dropGroundPosition, CQuadTree_GetInstance(),
                                                    &dropGroundQueryPosition, 0, 0, 0);
            *(WVector*)(&s_cameraTransform.x) = dropGroundPosition;
            s_cameraTransform.y += kScreenDropCameraClearance;
        }
    }
    self->dropModeActive = 1;
    if (((MapCameraMessageInterface*)self->mapCameraDisplay)) {
        ((MapCameraMessageInterface*)self->mapCameraDisplay)->m40(1);
    }
    GolfTipsDialog_RequestClose(5, kFloat_0_5_AuxMenu);
}

// Refresh the ball ring normal and lie blend from the current ground triangle.
// Payload pointers are borrowed for this synchronous call; no pointer is retained.
static inline void RefreshHudGroundLie(CScreen* self, const ScreenMessage* message, Vec3& groundNormal) {
    // Payload: none; uses current HUD/shared state.
    if (((BallPreviewMessageView*)self->ballPreview)) {
        char* ball = (char*)GolfBall__Fv();
        CQuadTree::GetMeshNormal(&groundNormal, CQuadTree_GetInstance(),
                                 ((HudBallTriangleView*)ball)->groundTriangleIndex);
        BallRing_UpdateGroundLie(((BallPreviewMessageView*)self->ballPreview), &groundNormal);
    }
    if (((HoleInfoMessageView*)self->holeInfoSet) != 0) {
        ((HoleInfoMessageView*)self->holeInfoSet)->cupInfo->m1c();
    }
    self->terrainIndicatorColor = 0x8000FF96;
    {
        self->groundLieBlend = ((CQuadTree*)CQuadTree_GetInstance())->GetMaxPowerOnGround();
        if (self->groundLieBlend < kFloat_1_AuxMenu) {
            self->groundLieBlend = self->groundLieBlend - kFloat_0_05_Screen;
        }
    }
}

// Interpolate the packed feedback color; a zero blend also clears the shot option.
// Payload pointers are borrowed for this synchronous call; no pointer is retained.
static inline void SetHudShotPowerColor(CScreen* self, const ScreenMessage* message, float*& blendPayload,
                                        u8& blendWeight, u32& inverseBlendWeight) {

    // Payload: p0=float* input blend (0..1 expected, no clamp); zero disables shot option.

    float scaledBlend;
    blendPayload = message->payload0.pf;
    scaledBlend = kFloat_255_Screen * *blendPayload;
    blendWeight = (int)scaledBlend;
    inverseBlendWeight = FullOpacity - blendWeight;

    u32 unshiftedColorLanes =
        inverseBlendWeight * kPowerColorStartUnshiftedLanes + blendWeight * kPowerColorEndUnshiftedLanes;
    u32 shiftedColorLanes =
        inverseBlendWeight * kPowerColorStartShiftedLanes + blendWeight * kPowerColorEndShiftedLanes;
    self->powerFeedbackColor =
        (((shiftedColorLanes & kPowerColorSelectedBytes) | (shiftedColorLanes & kPowerColorSelectedMiddleByte)) >> 8) |
        ((unshiftedColorLanes & kPowerColorSelectedBytes) | (unshiftedColorLanes & kPowerColorSelectedMiddleByte));
    if (*blendPayload == kFloat_0_AuxMenu) {
        if (self->shotOptionEnabled) {
            self->shotOptionEnabled = 0;
        }
    }
}

// Copy two borrowed opacity payloads into the feedback colors' alpha bytes.
// Payload pointers are borrowed for this synchronous call; no pointer is retained.
static inline void SetHudFeedbackOpacity(CScreen* self, const ScreenMessage* message) {
    // Payload: p0/p1=float* input opacity for the two feedback colors (0..1 expected).
    {
        int firstAlpha;
        float* firstOpacity;
        float* secondOpacity;
        int firstColor;
        firstOpacity = message->payload0.pf;
        firstAlpha = (int)(kFloat_255_Screen * *firstOpacity);
        firstColor = (firstAlpha << 24) + 0x1000000 - 1;
        secondOpacity = message->payload1.pf;
        self->feedbackColor0 = firstColor;
        self->feedbackColor1 = (((int)(kFloat_255_Screen * *secondOpacity)) << 24) + 0x1000000 - 1;
    }
}

// Copy height samples, compress their range and color offsets relative to the first sample.
// Payload pointers are borrowed for this synchronous call; no pointer is retained.
// Phase 1 copies raw heights and measures their range; phase 2 maps heights/colors into the HUD profile.
static inline void BuildHudTerrainProfile(CScreen* self, const ScreenMessage* message, int& sampleIndex,
                                          int& profileIndex, int& additionalSampleCount, int& profileKind,
                                          float*& heights, float& displayScale, float& heightRange,
                                          float& minimumHeight, float& heightMidpoint) {
    heights = message->payload2.pf;
    profileKind = message->payload0.i;
    additionalSampleCount = message->payload1.i;
    // Payload: p0=immediate profile kind; p1=additional sample count 0..20; p2=optional float[count+1] input, copied. No bounds check.

    self->terrainSampleCount = additionalSampleCount;
    self->terrainMarkerIndex = profileKind;
    if (0 != heights) {
        float maximumHeight = heights[0];
        float initialMinimumHeight = maximumHeight;
        minimumHeight = initialMinimumHeight;

        float firstHeight = maximumHeight;

        self->terrainPoints[0].v = firstHeight;
        for (sampleIndex = 1; sampleIndex < 1 + self->terrainSampleCount; sampleIndex++) {
            self->terrainPoints[sampleIndex].v = message->payload2.pf[sampleIndex];
            if (self->terrainPoints[sampleIndex].v < minimumHeight) {
                __typeof__(self->terrainPoints[sampleIndex].v) sampledMinimumHeight =
                    self->terrainPoints[sampleIndex].v;
                __typeof__(self->terrainPoints[sampleIndex].v) retainedMinimumHeight = sampledMinimumHeight;
                minimumHeight = retainedMinimumHeight;
            }
            if (self->terrainPoints[sampleIndex].v > maximumHeight) {
                __typeof__(self->terrainPoints[sampleIndex].v) sampledMaximumHeight =
                    self->terrainPoints[sampleIndex].v;
                maximumHeight = sampledMaximumHeight;
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
        __typeof__(self->terrainPoints[0].v) firstHeightForScaling = self->terrainPoints[0].v;
        __typeof__(self->terrainPoints[0].v) retainedFirstHeight = firstHeightForScaling;
        float midpointForScaling = heightMidpoint;
        float scaleForFirstHeight = displayScale;
        self->terrainPoints[0].v =
            scaleForFirstHeight * (kFloat_5_Screen * (kFloat_neg1_Screen * (retainedFirstHeight - midpointForScaling)));
        for (profileIndex = 1; profileIndex < 1 + self->terrainSampleCount; profileIndex++) {
            self->terrainPoints[profileIndex].v =
                displayScale *
                (kFloat_5_Screen * (kFloat_neg1_Screen * (self->terrainPoints[profileIndex].v - heightMidpoint)));
            float relativeHeight = self->terrainPoints[profileIndex].v - self->terrainPoints[0].v;
            if (relativeHeight < kScreenTerrainProfileColorMinDelta) {
                relativeHeight = kScreenTerrainProfileColorMinDelta;
            } else if (relativeHeight > kFloat_8_GolfScreen) {
                relativeHeight = kFloat_8_GolfScreen;
            }
            if (relativeHeight > kFloat_0_AuxMenu) {
                float colorProduct = kFloat_255_Screen * relativeHeight;
                int colorComponent = (int)(kFloat_255_Screen - colorProduct * kScreenTerrainProfileColorScale);
                self->terrainPoints[profileIndex].col = 0xFF00E500 | (colorComponent << 16) | colorComponent;
            } else {
                float colorProduct = kFloat_255_Screen * relativeHeight;
                unsigned int colorComponent = (int)(colorProduct * kScreenTerrainProfileColorScale + kFloat_255_Screen);
                self->terrainPoints[profileIndex].col = (colorComponent << 8) | (0xFFFF0000 | colorComponent);
            }
        }
    }
}

// Reset notices, reward/ball displays and auxiliary selection for the active player.
// Payload pointers are borrowed for this synchronous call; no pointer is retained.
static inline void ResetHudCurrentPlayerDisplays(CScreen* self, const ScreenMessage* message) {
    // Payload: none; uses current HUD/shared state.
    self->shotIndicatorPulse = kFloat_0_AuxMenu;
    if (g_helpMsg != 0) {
        NoticeQueue_ResetAndActivate(g_helpMsg);
    }
    if (((PangInfoMessageView*)self->pangInfo)) {
        ((CPangInfo*)(((PangInfoMessageView*)self->pangInfo)))->ResetFromCurrentPlayer();
    }
    if (((V0*)self->modeInfo)) {
        ((V0*)self->modeInfo)->m20();
    }
    if (((BallPreviewMessageView*)self->ballPreview)) {
        ScreenBallPreview_SelectCurrentPlayerBall(((BallPreviewMessageView*)self->ballPreview));
    }
    {

        if (AuxiliaryMenuMessageView* auxiliaryMenu = ((AuxiliaryMenuMessageView*)self->auxiliaryMenu)) {
            int zero = 0;
            auxiliaryMenu->selectionActive = zero;
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
// confirmRequested, powerMode, specialShotSelection; +18 -> SetPointerSelectionDisabled stores its
// integer argument in inputLocked. The older local name strokePointHandle is a
// pointer-shaped ABI WORD, NOT a retained object: the player's +1DC value is
// copied to that integer lock. Neither call takes ownership; both return 1,
// ignored here. Do not turn the +18 argument into a dereference or retain.
// golfBase +64 -> CGolfBase2d::SetPangyaZoneWidth (golf_base_2d_hud_layout.cpp), sets pangyaZoneWidth=1;
// +68 -> CGolfBase2d::SetMeterOverlayWidth, sets meterOverlayWidth=0. Both mark layoutDirty only on a
// changed float value and return 1 (ignored). These are layout scale/width values,
// not opacity or seconds; the receiver's refresh consumes them. Keep +60 target-X
// assignment and +5C action reset after these setters, as in the stock protocol.
// This reset does not destroy/recreate these receivers. A reset with payload0=0
// also enters HudAim; nonzero preserves phase. Party scripts and opacity updates
// below remain ordered after the widget state reset.
static inline void ResetHudShotPanels(CScreen* self, const ScreenMessage* message) {
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
        ((CCalipers*)(self->calipers))->ResetMeasurement();
    }
    ((StrokePointMessageInterface*)self->strokePointDisplay)->ResetSelections();
    ((StrokePointMessageInterface*)self->strokePointDisplay)
        ->SetInputLockWord(
            HudMessagePlayerSlot(g_sharedDoc, ((((HudMessageSharedStateView*)g_sharedDoc)->h->currentPlayer)))
                ->strokePointHandle);
    if (6 > ((HudMessageSharedStateView*)g_sharedDoc)->h->tutorialStage ||
        *(u8*)(*(char**)((char*)InstanceTable_Get() + 4) + 0xa) == 3) {
        ((BallWindMessageInterface*)self->ballWindDisplay)->m24(0);
    } else {
        ((BallWindMessageInterface*)self->ballWindDisplay)->m24(1);
    }
    ((GolfBaseMessageInterface*)self->golfBaseDisplay)->SetPangyaZoneWidth(kFloat_1_AuxMenu);
    ((GolfBaseMessageInterface*)self->golfBaseDisplay)->SetMeterOverlayWidth(kFloat_0_AuxMenu);
    ((GolfBaseMessageInterface*)self->golfBaseDisplay)
        ->m60(((HudMessageSharedStateView*)g_sharedDoc)->h->roundedShotGaugeCursorX);
    ((GolfBaseMessageInterface*)self->golfBaseDisplay)->m5c();
    if (((CGolfBall*)GolfBall__Fv())->m_groundType == 2) {
        ((GolfBaseMessageInterface*)self->golfBaseDisplay)->m28(1);
    } else {
        ((GolfBaseMessageInterface*)self->golfBaseDisplay)->m28(0);
    }
    ((GolfBaseMessageInterface*)self->golfBaseDisplay)->m1c(0);
    ((GolfBaseMessageInterface*)self->golfBaseDisplay)->m20(0);
    ((SwingResultMessageInterface*)self->swingResultDisplay)->m20();
    self->swingFeedbackPending = 0;
    if (0 != self->ingameStatus) {
        ((CIngameStatus*)(self->ingameStatus))->RefreshPlayer();
    }
    HudMessageSharedStateView* sharedState = (HudMessageSharedStateView*)g_sharedDoc;
    if (3 == sharedState->partyGameMode) {
        int partyItemType = PartyItemSlots_GetItemType(PartyItemSlots_GetInstance(), sharedState->h->currentPlayer);
        BpObject_SetEnabled(self->partyGolfBaseScript, 0);
        BpObject_SetEnabled(self->featherBadgeScript, 0);
        self->partyMeterIndex = partyItemType;
        switch (partyItemType) {
        case 0:
            BpObject_StartScriptByName(self->partyGolfBaseScript, screenOverlayStrings + ScreenText_RedMeterAnimation,
                                       1);
            BpObject_SetEnabled(self->partyGolfBaseScript, 1);
            break;
        case 1:
            BpObject_StartScriptByName(self->partyGolfBaseScript, screenOverlayStrings + ScreenText_BlueMeterAnimation,
                                       1);
            BpObject_SetEnabled(self->partyGolfBaseScript, 1);
            break;
        case 2:
            BpObject_StartScriptByName(self->partyGolfBaseScript,
                                       screenOverlayStrings + ScreenText_YellowMeterAnimation, 1);
            BpObject_SetEnabled(self->partyGolfBaseScript, 1);
            break;
        case 9:
            BpObject_StartScriptByName(self->featherBadgeScript, screenOverlayStrings + ScreenText_FeatherAnimation, 1);
            BpObject_SetEnabled(self->featherBadgeScript, 1);
            break;
        }
    }
    if (self->hudInitialized) {
        if (3 == ((HudMessageSharedStateView*)g_sharedDoc)->partyGameMode) {
            int meterGroupIndex = self->partyMeterIndex;
            Tri meterGroupNames;
            meterGroupNames = *(Tri*)partyMeterGroupNames;

            if (0 <= meterGroupIndex) {
                if ((u32)meterGroupIndex < 3) {
                    UIGroup_SetOpacity(BpObject_GetOrCreateNamedGroup(self->partyGolfBaseScript,
                                                                      (const char*)meterGroupNames.v[meterGroupIndex]),
                                       0x7f);
                }
            }
        }
        self->meterOpacity = 0xff;
        ((BallWindMessageInterface*)self->ballWindDisplay)->m64(-1);
        ((GolfBaseMessageInterface*)self->golfBaseDisplay)->m50(-1);
    }
}

// Select match-play rewards when applicable, then notify the score actor.
// Payload pointers are borrowed for this synchronous call; no pointer is retained.
static inline void CalculateHudHoleRewards(CScreen* self, const ScreenMessage* message) {
    // Payload: none; uses current HUD/shared state.
    if (((PangInfoMessageView*)self->pangInfo)) {
        switch (((HudMessageSharedStateView*)g_sharedDoc)->golfRuleType) {
        case 1:
            ((CPangInfo*)(((PangInfoMessageView*)self->pangInfo)))->CalculateMatchPlayHoleRewards();
            break;
        }
        IActor_DispatchMessage(self, self, screenOverlayStrings + ScreenText_ScoreActor, 5, (void*)(0), (void*)(0),
                               (void*)(0), (void*)(0));
    }
}

// Start stage flashing or select the preceding stage; tutorial input may be delayed.
// Payload pointers are borrowed for this synchronous call; no pointer is retained.
static inline void StartHudTutorialStage(CScreen* self, const ScreenMessage* message) {
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
static inline void SetHudTutorialCompletion(CScreen* self, const ScreenMessage* message) {
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
static inline void SelectHudAuxiliaryMenuEntry(CScreen* self, const ScreenMessage* message, int& entryIndex,
                                               AuxiliaryMenuMessageView*& auxiliaryMenu) {
    // Payload: p0=immediate entry index; used to index cursor regions without validation.
    {
        auxiliaryMenu = ((AuxiliaryMenuMessageView*)self->auxiliaryMenu);
        if (auxiliaryMenu) {
            int enabled = 1;
            entryIndex = message->payload0.i;
            auxiliaryMenu->selectionActive = enabled;
            auxiliaryMenu->selectedEntry = entryIndex;
            int regionEntryIndex = entryIndex;
            int regionEnabled = enabled;
            ((CMouseCursor*)g_mouseCursor)->regionEnabled[0xe3 - 0xbd + regionEntryIndex] = regionEnabled;
            float viewportHeight = ((ViewportSizeView*)g_view)->viewportHeight;
            float viewportWidth = ((ViewportSizeView*)g_view)->viewportWidth;
            ((CMouseCursor*)g_mouseCursor)
                ->MoveTo(viewportWidth - kFloat_50_Screen,
                         kScreenCommandAndMenuRowSpacing * (kFloat_0_5_AuxMenu + (float)(entryIndex - 3)) +
                             kFloat_0_5_AuxMenu * viewportHeight,
                         kFloat_neg1_Screen);
        }
    }
}

#pragma pop

// Dispatch HUD messages for shot feedback, rewards, tutorial panels, drop mode, and visibility.
void CScreen::HandleMsg(const ScreenMessage* message) {
    // Keep these four distinct vector roles in original declaration order for the Wii stack ABI.

    Vec3 groundNormal;
    HudMessagePositionWords dropGroundQueryPosition;
    WVector dropGroundPosition;
    HudMessagePositionWords dropBoundsTestPosition;

    switch (message->id) {
    case ScreenMsg_EnterDropMode: // Set up drop bounds/camera and activate the map panel; world-space scratch stays caller-owned.
        EnterHudDropMode(this, message, dropBoundsTestPosition, dropGroundQueryPosition, dropGroundPosition);
        break;
    case ScreenMsg_QueryDropMode:
        // Payload: p0=int* output DropMode; not retained.
        *message->payload0.pi = dropModeActive;
        break;
    case ScreenMsg_ExitDropMode:
        // Payload: none; uses current HUD/shared state.
    case ScreenMsg_CancelDropMode:
        // Payload: none; uses current HUD/shared state.
        dropModeActive = 0;
        if (0 != mapCameraDisplay) {
            ((MapCameraMessageInterface*)mapCameraDisplay)->m40(0);
        }
        ((HudMessageSharedStateView*)g_sharedDoc)->h->cameraMode = 0;
        break;
    case ScreenMsg_SetCurrentPlayerDisplayValue:
        // Payload: none; uses current HUD/shared state.
        if (nearestPinInfo != 0) {
            Hud_SetCurrentPlayerDisplayValue(nearestPinInfo, (((HudMessageSharedStateView*)g_sharedDoc)->h->f3bc));
        }
        break;
    case ScreenMsg_RefreshNearestPinPlayer:
        // Payload: none; uses current HUD/shared state.
        if (nearestPinInfo != 0) {
            Hud_UpdateNearestPinPlayer(nearestPinInfo);
        }
        break;
    case ScreenMsg_RequestShotTransition:
        // Payload: none; uses current HUD/shared state.
        shotTransitionRequested = 1;
        if (0 != holeInfoSet) {
            ((HoleInfoMessageView*)holeInfoSet)->shotTransitionInfo->m1c();
        }
        break;
    case ScreenMsg_SetBallRingHeight:
        // Payload: p0=float* input height, copied only when ballPreview exists.
        if (ballPreview != 0) {
            float v = *message->payload0.pf;
            ballRingHeight = v;
            ((BallPreviewMessageView*)ballPreview)->ballRingHeight = v;
        }
        break;
    case ScreenMsg_AddBallRingHeight:
        // Payload: p0=float* input height increment, applied only when preview exists.
        if (0 != ballPreview) {
            float d = *message->payload0.pf;
            float v = ballRingHeight + d;
            ballRingHeight = v;
            ((BallPreviewMessageView*)ballPreview)->ballRingHeight = v;
        }
        break;
    case ScreenMsg_RefreshGroundLie: // Refresh the ball ring normal and lie blend from the current ground triangle.
        RefreshHudGroundLie(this, message, groundNormal);
        break;
    case ScreenMsg_SetFirstHoleInfoDisplayEnabled:
        // Payload: p0=immediate int enable (nonzero).
        if (0 != holeInfoSet) {
            ((HoleInfoMessageView*)holeInfoSet)->firstDisplayEnabled = ((message->payload0.i != 0));
        }
        break;
    case ScreenMsg_SetSecondHoleInfoDisplayEnabled:
        // Payload: p0=immediate int enable (nonzero).
        if (holeInfoSet) {
            ((HoleInfoMessageView*)holeInfoSet)->secondDisplayEnabled = (((0 != message->payload0.i)));
        }
        break;
    case ScreenMsg_ShowShotGauge:
        // Payload: none; uses current HUD/shared state.
        hudFlags |= HudShotGaugeVisible;
        break;
    case ScreenMsg_HideShotGauge:
        // Payload: none; uses current HUD/shared state.
        hudFlags &= ~HudShotGaugeVisible;
        break;
    case ScreenMsg_ShowTimedNotification:
        // Payload: none; uses current HUD/shared state.
        displayTimeRemaining = kFloat_1_AuxMenu;
        break;
    case ScreenMsg_SetWindStrengthAndHeading: {
        // Payload: p0=immediate int strength; p1=float* input heading.
        int v = message->payload0.i;
        windStrength = v;
        windHeading = *message->payload1.pf;
        // The wind-strength view sits in the 0x2C slot (remainingTimeInfo in CScreen.h).
        if (remainingTimeInfo != 0) {
            ((WindInfoMessageView*)remainingTimeInfo)->windStrength = v;
        }
        break;
    }
    case ScreenMsg_SetShotPowerColor: // Interpolate the packed feedback color; a zero blend also clears the shot option.
    {
        float* blendPayload;
        u8 blendWeight;
        u32 inverseBlendWeight;
        SetHudShotPowerColor(this, message, blendPayload, blendWeight, inverseBlendWeight);
        break;
    }
    case ScreenMsg_ShowShotFeedback:
        // Payload: none; uses current HUD/shared state.
        if (holeInfoSet != 0) {
            ((CAccuracyInfo*)(((HoleInfoMessageView*)holeInfoSet)->accuracyInfo))->ShowCurrentShotFeedback();
        }
        break;
    case ScreenMsg_ShowFullPower:
        // Payload: none; uses current HUD/shared state.
        if (accuracyInfo != 0) {
            ((CAccuracyInfo*)(accuracyInfo))->ShowFullPower();
        }
        break;
    case ScreenMsg_SetFeedbackOpacity: // Copy two borrowed opacity payloads into the feedback colors' alpha bytes.
        SetHudFeedbackOpacity(this, message);
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
        BuildHudTerrainProfile(this, message, sampleIndex, profileIndex, additionalSampleCount, profileKind, heights,
                               displayScale, heightRange, minimumHeight, heightMidpoint);
        break;
    }
    case ScreenMsg_ResetForCurrentPlayer: // Reset notices, reward/ball displays and auxiliary selection for the active player.
        ResetHudCurrentPlayerDisplays(this, message);
        break;
    case ScreenMsg_ResetShotPanels: // Reset shot controls, party meter scripts and their loaded widget opacity.
        ResetHudShotPanels(this, message);
        break;
    case ScreenMsg_EnablePointerShotInput:
        // Payload: none; uses current HUD/shared state.
        hudFlags |= HudPointerShotInputEnabled;
        break;
    case ScreenMsg_DisablePointerShotInput:
        // Payload: none; uses current HUD/shared state.
        hudFlags &= ~HudPointerShotInputEnabled;
        break;
    case ScreenMsg_ResetShotFeedbackBlend:
        // Payload: none; uses current HUD/shared state.
        shotFeedbackBlend = kFloat_0_AuxMenu;
        break;
    case ScreenMsg_ResetHoleInfo:
        // Payload: none; uses current HUD/shared state.
        if (holeInfoSet) {
            ((HoleInfoMessageView*)holeInfoSet)->holeInfo->m1c();
        }
        break;
    case ScreenMsg_CalculateShotRewards:
        // Payload: p0=immediate int option, forwarded as nonzero boolean.
        if (0 != pangInfo) {
            ((CPangInfo*)(pangInfo))->CalculateShotRewards((((message->payload0.i != 0))));
        }
        break;
    case ScreenMsg_ResetRewardAnimations:
        // Payload: none; uses current HUD/shared state.
        if (pangInfo) {
            ((CPangInfo*)(pangInfo))->ResetBonusAnimations();
        }
        break;
    case ScreenMsg_FinishRewardTotals:
        // Payload: none; uses current HUD/shared state.
        if (pangInfo) {
            ((CPangInfo*)(pangInfo))->FinishDisplayedTotals();
        }
        break;
    case ScreenMsg_CalculateHoleRewards: // Select match-play rewards when applicable, then notify the score actor.
        CalculateHudHoleRewards(this, message);
        break;
    case ScreenMsg_QueryRewardDisplayComplete:
        // Payload: p0=unsigned char* output completion flag (one byte).
        *message->payload0.pb = 0;
        if (!pangInfo || (((PangInfoMessageView*)pangInfo)->displayFlags & 0x10)) {
            if (0 == modeInfo || ((V0*)modeInfo)->m24() != 0) {
                *message->payload0.pb = 1;
            }
        }
        break;
    case ScreenMsg_LoadBallResource:
        // Payload: p0=immediate int resource ID, forwarded if preview exists.
        if (ballPreview) {
            Screen_GetOrLoadBallResource(ballPreview, message->payload0.i);
        }
        break;
    case ScreenMsg_StartTutorialStage: // Start stage flashing or select the preceding stage; tutorial input may be delayed.
        StartHudTutorialStage(this, message);
        break;
    case ScreenMsg_ResetTutorialTitle:
        // Payload: none; uses current HUD/shared state.
        tutorialTitlePhase = 0;
        tutorialTitleTimer = kFloat_0_AuxMenu;
        modeLabelOpacity = 0;
        break;
    case ScreenMsg_SetTutorialCompletion: // Apply the tutorial completion command without retaining payload pointers.
        SetHudTutorialCompletion(this, message);
        break;
    case ScreenMsg_ResetIngameStatus:
        // Payload: none; uses current HUD/shared state.
        if (ingameStatus) {
            ((CIngameStatus*)(ingameStatus))->ResetState();
        }
        break;
    case ScreenMsg_FlashHoleInfo:
        // Payload: none; uses current HUD/shared state.
        if (0 != holeInfoSet) {
            ((CHoleInfo*)(((HoleInfoMessageView*)holeInfoSet)->holeInfo))->StartFlash();
        }
        break;
    case ScreenMsg_SelectAuxiliaryMenuEntry: // Activate the requested menu cursor region and move the pointer in viewport coordinates.
    {
        int entryIndex;
        AuxiliaryMenuMessageView* auxiliaryMenu;
        SelectHudAuxiliaryMenuEntry(this, message, entryIndex, auxiliaryMenu);
        break;
    }
    case ScreenMsg_ClearAuxiliaryMenuSelection:
        // Payload: none; uses current HUD/shared state.
        if (0 != auxiliaryMenu) {
            ((AuxiliaryMenuMessageView*)auxiliaryMenu)->selectedEntry = -1;
        }
        break;
    case ScreenMsg_QueryBallPreviewBasisRow:
        // Payload: p0=WVector* output second basis row (m[3..5]), NOT translation; left untouched if preview absent.
        if (ballPreview) {
            float m[12];
            CScreen::BuildOrientedBasisMatrix((char*)m, ballPreview);
            *(WVector*)message->payload0.p = *(WVector*)(3 + m);
        }
        break;
    case ScreenMsg_QueryGroundLieBlend:
        // Payload: p0=float* output ground-lie blend.
        *message->payload0.pf = groundLieBlend;
        break;
    case ScreenMsg_SetHudDisplayEnabled:
        // Payload: p0=immediate int; only exactly 1 enables display.
        // Stored as a bool: the original compiles this comparison to a bool store.
        *(bool*)&hudDisplayEnabled = 1 == message->payload0.i;
        break;
    case ScreenMsg_ShowIngameStatus:
        // Payload: none; uses current HUD/shared state.
        if (0 != ingameStatus) {
            ((CIngameStatus*)(ingameStatus))->BeginShow();
        }
        break;
    case ScreenMsg_HideIngameStatus:
        // Payload: none; uses current HUD/shared state.
        if (0 != ingameStatus) {
            ((CIngameStatus*)(ingameStatus))->BeginHide();
        }
        break;
    case ScreenMsg_ShowCornerLabels:
        // Payload: none; uses current HUD/shared state.
        if (0 != holeInfoSet) {
            ((CHoleInfo*)((HoleInfoMessageView*)holeInfoSet)->holeInfo)->ShowCornerLabels();
        }
        break;
    case ScreenMsg_SnapHoleDistance:
        // Payload: none; uses current HUD/shared state.
        if (0 != holeInfoSet) {
            ((CHoleCupInfo*)(((HoleInfoMessageView*)holeInfoSet)->cupInfo))->SnapDistanceOverlay();
        }
        break;
    case ScreenMsg_SetHoleInfoValues:
        // Payload: p0/p1=immediate ints forwarded to cup-info m18; their units remain unresolved.
        if (0 != holeInfoSet) {
            ((HoleInfoMessageView*)holeInfoSet)->cupInfo->m18(message->payload0.i, message->payload1.i);
        }
        break;
    case ScreenMsg_RefreshIngamePlayer:
        // Payload: none; uses current HUD/shared state.
        if (ingameStatus) {
            ((CIngameStatus*)(ingameStatus))->RefreshPlayer();
        }
        break;
    case ScreenMsg_StartSkinsResult:
        // Payload: p0/p1=immediate ints forwarded to SkinsInfo_StartResultDisplay.
        if (skinsInfo) {
            SkinsInfo_StartResultDisplay(skinsInfo, message->payload0.i, message->payload1.i);
        }
        break;
    case ScreenMsg_ResetShotCounterFade:
        // Payload: none; uses current HUD/shared state.
        if (0 != ingameStatus) {
            ((CIngameStatus*)(ingameStatus))->ResetShotCounterFade();
        }
        break;
    case ScreenMsg_QueryHudPhase:
        // Payload: p0=int* output HudPhase; not retained.
        *message->payload0.pi = hudPhase;
        break;
    case ScreenMsg_QueryStrokePanelReady:
        // Payload: p0=int* output StrokePanelReady; not retained.
        if (strokePointDisplay != 0) {
            if (((StrokePointMessageInterface*)strokePointDisplay)->m70() != 0) {
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
        *message->payload0.pi = shotOptionResult;
        break;
    case ScreenMsg_QueryShotDirectionResult:
        // Payload: p0=int* output ShotDirectionResult; not retained.
        *message->payload0.pi = shotDirectionResult;
        break;
    case ScreenMsg_ConsumeShotOptionChanged: {
        // Payload: p0=int* output previous change latch; clears that latch after writing it.
        int* p = message->payload0.pi;
        *p = shotOptionChanged;
        shotOptionChanged = 0;
        break;
    }
    case ScreenMsg_QueryStrokeOption:
        // Payload: p0=int* output StrokeOption; not retained.
        *message->payload0.pi = ((StrokePointMessageInterface*)strokePointDisplay)->GetStrokeOption();
        break;
    case ScreenMsg_QueryShotOptionEnabled:
        // Payload: p0=int* output ShotOptionEnabled; not retained.
        *message->payload0.pi = shotOptionEnabled;
        break;
    case ScreenMsg_SetShotOptionEnabled: {
        // Payload: p0=immediate int nonzero enable; also sets shotOptionChanged to that boolean.
        int v = ((((message->payload0.i != 0))));
        shotOptionEnabled = v;
        shotOptionChanged = v;
        break;
    }
    case ScreenMsg_ResumeAim:
        // Payload: none; uses current HUD/shared state.
        hudPhase = HudAim;
        CMouseCursor::SetPointerInputEnabled(g_mouseCursor, 1);
        resumeAimRequested = 1;
        break;
    case ScreenMsg_BlockCameraInput:
        // Payload: none; uses current HUD/shared state.
        if (0 != ((HudMessageSharedStateView*)g_sharedDoc)->h->cameraMode) {
            hudInputBlocked = 1;
        }
        break;
    case ScreenMsg_QueryMapCameraState:
        // Payload: p0=int* output MapCameraState; not retained.
        *message->payload0.pi = ((MapCameraMessageInterface*)mapCameraDisplay)->GetMapCameraState();
        break;
    case ScreenMsg_QueryPanelsIdle: {
        // Payload: p0=int* output PanelsIdle; not retained.
        unsigned int a = ((GolfBaseMessageInterface*)golfBaseDisplay)->IsPanelIdle();

        *message->payload0.pi = a & (((MapCameraMessageInterface*)mapCameraDisplay)->IsPanelIdle());
        break;
    }
    case ScreenMsg_QueryAimPanelState:
        // Payload: p0=int* output AimPanelState; not retained.
        *message->payload0.pi = ((GolfBaseMessageInterface*)golfBaseDisplay)->GetAimPanelState();
        break;
    case ScreenMsg_QueryMapCameraSelection:
        // Payload: p0=int* output MapCameraSelection; not retained.
        *message->payload0.pi = ((MapCameraMessageInterface*)mapCameraDisplay)->GetMapCameraSelection();
        break;
    case ScreenMsg_QueryCalipersPreview:
        // Payload: p0=int* output CalipersPreview; not retained.
        if (calipers) {
            *message->payload0.pi = ((CCalipers*)(calipers))->IsPreviewActive();
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
        *message->payload0.pi = panelsReady;
        break;
    case ScreenMsg_SetAimControlsEnabled:
        // Payload: p0=immediate int aim-control gate.
        aimControlsEnabled = message->payload0.i;
        break;
    case ScreenMsg_QueryCameraOrBallControlActive: {
        // Payload: p0=int* output CameraOrBallControlActive; not retained.
        unsigned int a = ((MapCameraMessageInterface*)mapCameraDisplay)->m30();

        *message->payload0.pi = a | (((BallWindMessageInterface*)ballWindDisplay)->m28());
        break;
    }
    case ScreenMsg_QueryItemPanelReady:
        // Payload: p0=int* output ItemPanelReady; not retained.
        *message->payload0.pi = itemPanelReady;
        break;
    case ScreenMsg_ResetAimPanelAction:
        // Payload: none; uses current HUD/shared state.
        ((GolfBaseMessageInterface*)golfBaseDisplay)->ResetAimPanelAction();
        break;
    case ScreenMsg_ResetMapCameraAction:
        // Payload: none; uses current HUD/shared state.
        ((MapCameraMessageInterface*)mapCameraDisplay)->ResetAction();
        break;
    case ScreenMsg_ConfirmMapCameraAction:
        // Payload: none; uses current HUD/shared state.
        ((MapCameraMessageInterface*)mapCameraDisplay)->ConfirmAction();
        break;
    case ScreenMsg_ShowTutorialMeter:
        // Payload: none; uses current HUD/shared state.
        tutorialMeterEnabled = 1;
        break;
    case ScreenMsg_ShowTutorialMeterImmediately:
        // Payload: none; uses current HUD/shared state.
        tutorialMeterEnabled = 1;
        tutorialMeterBlinkCount = 6;
        break;
    case ScreenMsg_ShowTutorialGauge:
        // Payload: none; uses current HUD/shared state.
        tutorialGaugeEnabled = 1;
        break;
    case ScreenMsg_ShowTutorialGaugeImmediately:
        // Payload: none; uses current HUD/shared state.
        tutorialGaugeEnabled = 1;
        tutorialGaugeBlinkCount = 6;
        break;
    case ScreenMsg_ShowTutorialAim:
        // Payload: none; uses current HUD/shared state.
        tutorialAimEnabled = 1;
        break;
    case ScreenMsg_ShowTutorialAimImmediately:
        // Payload: none; uses current HUD/shared state.
        tutorialAimEnabled = 1;
        tutorialAimBlinkCount = 6;
        ((GolfBaseMessageInterface*)golfBaseDisplay)->m7c();
        break;
    case ScreenMsg_HideTutorialMeter:
        // Payload: none; uses current HUD/shared state.
        tutorialMeterEnabled = 0;
        tutorialMeterBlinkFrame = 0;
        tutorialMeterBlinkCount = 0;
        break;
    case ScreenMsg_HideTutorialGauge:
        // Payload: none; uses current HUD/shared state.
        tutorialGaugeEnabled = 0;
        tutorialGaugeBlinkFrame = 0;
        tutorialGaugeBlinkCount = 0;
        break;
    case ScreenMsg_HideTutorialAim:
        // Payload: none; uses current HUD/shared state.
        tutorialAimEnabled = 0;
        tutorialAimBlinkFrame = 0;
        tutorialAimBlinkCount = 0;
        break;
    case ScreenMsg_SetTutorialClubChangeEnabled:
        // Payload: p0=immediate int club-change gate.
        tutorialClubChangeEnabled = message->payload0.i;
        break;
    case ScreenMsg_SetTutorialAimControlEnabled:
        // Payload: p0=immediate int tutorial aim gate.
        tutorialAimControlEnabled = message->payload0.i;
        break;
    case ScreenMsg_SetTutorialSpinControlEnabled:
        // Payload: p0=immediate int tutorial spin gate.
        tutorialSpinControlEnabled = message->payload0.i;
        break;
    case ScreenMsg_ShowTutorialIllustration:
        // Payload: p0=immediate illustration selector; current-player shared handle is supplied internally.
        if (tutorialIllustration) {
            ((TutorialIllustrationMessageInterface*)tutorialIllustration)
                ->m1c(message->payload0.i,
                      HudMessagePlayerSlot(g_sharedDoc, ((HudMessageSharedStateView*)g_sharedDoc)->h->currentPlayer)
                          ->illustrationHandle);
            ((TutorialIllustrationMessageInterface*)tutorialIllustration)->m24();
            tutorialIllustrationActive = 1;
        }
        break;
    case ScreenMsg_SetCameraControlsEnabled:
        // Payload: p0=immediate int camera-control gate.
        cameraControlsEnabled = message->payload0.i;
        break;
    case ScreenMsg_SuspendHudDrawing:
        // Payload: none; uses current HUD/shared state.
        {

            unsigned int zero;
            zero = 0;
            hudEnabled = zero;
            if (0 == Sing_805DC5F0()) {
                if (0 == s_bpCtrlInitialized) {
                    ((CBpCtrl*)s_bpCtrl)->__ct();
                    __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, s_bpCtrlDtorRegistration_AuxMenu);
                    s_bpCtrlInitialized = 1;
                }
                Sing_805DC5F0() = s_bpCtrl;
            }
            ((CBpCtrl*)Sing_805DC5F0())->SetDrawingEnabled(zero);
        }
        break;
    case ScreenMsg_ResumeHudDrawing: {
        // Payload: none; uses current HUD/shared state.
        void* ctrl = Sing_805DC5F0();
        hudEnabled = 1;
        if (0 == ctrl) {
            if (0 == s_bpCtrlInitialized) {
                ((CBpCtrl*)s_bpCtrl)->__ct();
                __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, s_bpCtrlDtorRegistration_AuxMenu);
                s_bpCtrlInitialized = 1;
            }
            Sing_805DC5F0() = s_bpCtrl;
        }
        ((CBpCtrl*)Sing_805DC5F0())->SetDrawingEnabled(1);
        break;
    }
    }
}
}

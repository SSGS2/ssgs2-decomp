// The golf HUD panel drawn over the round (CGolfBase2d): its layout, power
// meter, terrain markers, action buttons and dialog fade. The file also holds
// the round task hooks, the round-position unit initialiser and the BGM
// collection constructor, which share the unit.
#include <decomp/local_class.h>
#include <decomp/static_init.h>
#include "game/chatmsg_match.h"
#include "classes/CBgmCollection.h"
#include "classes/CGolfBase2d.h"
#include "game/singletons.h"
#include "sdk_protos.h"
#include "game/types.h"
#include "classes/CRoundDolMain.h"
#include "classes/CRoundDolTask.h"
#include "classes/CTask.h"
#include "classes/CBpCtrl.h"
#include "classes/CPackTextureResource.h"

// Layout element numbers. 0, 4..8 and 0x09..0x33 are artwork groups; 0x34..0x39
// are the meter, 0x3A..0x44 the power-meter and Pangya art, 0x45..0x48 the
// action buttons and 0x49..0x4A the marker row. Marker n uses element 0x4C+n.
enum {
    kGolfLayoutResourceCount = 5,
    kGolfLayoutElementCount = 0x61,
    kMarkerStyleCount = 21,
    kMeterBackdrop0 = 0x34,
    kMeterBackdrop1 = 0x35,
    kExtraMeterImage = 0x36,
    kTransitionStartMeter = 0x37,
    kInterpolatedMeter = 0x38,
    kMeterOverlay = 0x39,
    kPangyaZone = 0x43,
    kCancelButton = 0x45,
    kCancelButtonImage = 0x46,
    kConfirmButton = 0x47,
    kConfirmButtonImage = 0x48,
    kMarkerRangeEnd = 0x49,
    kMarkerRangeStart = 0x4A,
    kMarkerElementBase = 0x4C,
    kButtonAnimationFlag = 5
};

// Artwork shown in each display mode.
enum {
    kPowerArtworkFirst = 0x3A,
    kPowerArtworkLast = 0x42,
    kMode2ArtworkFirst = 0x09,
    kMode2ArtworkLast = 0x11,
    kMode3ArtworkFirst = 0x12,
    kMode3ArtworkLast = 0x20,
    kMode4ArtworkFirst = 0x21,
    kMode4ArtworkLast = 0x33,
    kPowerMeterEndImage = 0x44
};

enum GolfDisplayMode {
    kNoMeterDisplay = 0,
    kPowerMeterDisplay = 1,
    kMarkerDisplay2 = 2,
    kMarkerDisplay3 = 3,
    kMarkerDisplay4 = 4
};

enum GolfFadeState {
    kFadeHidden = 0,
    kFadeIn = 1,
    kFadeVisible = 2,
    kFadeOut = 3
};

enum {
    kFadeInFrames = 5,
    kFadeOutFrames = 7,
    kFadeOutSoundFrame = 4,
    kButtonIdle = 0,
    kButtonActivated = 2
};

// Offsets into golfPositionDialogStrings.
enum {
    kLayoutFilename = 0x00, // "p2_pay_golf_base.psd"
    kButtonSound = 0x16,    // "ui_icon_click"
    kFadeSound = 0x24,      // "close_position_dialog"
    kRangeErrorText = 0x3A, // "vector::at index out of range"
    kRangeErrorType = 0x58  // "!std::exception!!std::logic_error!!std::out_of_range!!"
};

// The BP archive names of the BGM collection.
struct BgmCollectionResourcePool {
    char archive[19], controller[22], layout[22], messageTable[19];
};

enum {
    kBgmControllerOffset = (int)&((BgmCollectionResourcePool*)0)->controller,
    kBgmLayoutOffset = (int)&((BgmCollectionResourcePool*)0)->layout,
    kBgmMessagesOffset = (int)&((BgmCollectionResourcePool*)0)->messageTable
};

// The vtable slot that scales one element horizontally.
enum {
    kScaleElementVtableSlot = 0x8C / sizeof(void*)
};

// A pointer cursor at the style boundary of the marker row.
struct GolfMarkerStyleCursor {
    char layoutPrefix[0x70];
    CGolfBase2d::MarkerStyle style;
};

// An empty allocator argument: String_ReplaceRange only needs its address.
struct StringAllocator {};

// The HUD interface as seen through its vtable. Only the three slots called at
// the end of an update are named.
struct HudInterfaceBase {
    char layoutStorage[0x164];
};
class HudInterface : public HudInterfaceBase {
public:
    virtual void Slot0() DECOMP_LOCAL_VIRTUAL;
    virtual void Slot1() DECOMP_LOCAL_VIRTUAL;
    virtual void Slot2() DECOMP_LOCAL_VIRTUAL;
    virtual void Slot3() DECOMP_LOCAL_VIRTUAL;
    virtual void Slot4() DECOMP_LOCAL_VIRTUAL;
    virtual void Slot5() DECOMP_LOCAL_VIRTUAL;
    virtual void Slot6() DECOMP_LOCAL_VIRTUAL;
    virtual void Slot7() DECOMP_LOCAL_VIRTUAL;
    virtual void Slot8() DECOMP_LOCAL_VIRTUAL;
    virtual void Slot9() DECOMP_LOCAL_VIRTUAL;
    virtual void Slot10() DECOMP_LOCAL_VIRTUAL;
    virtual void Slot11() DECOMP_LOCAL_VIRTUAL;
    virtual void Slot12() DECOMP_LOCAL_VIRTUAL;
    virtual void Slot13() DECOMP_LOCAL_VIRTUAL;
    virtual void Slot14() DECOMP_LOCAL_VIRTUAL;
    virtual void Slot15() DECOMP_LOCAL_VIRTUAL;
    virtual void Slot16() DECOMP_LOCAL_VIRTUAL;
    virtual void Slot17() DECOMP_LOCAL_VIRTUAL;
    virtual void Slot18() DECOMP_LOCAL_VIRTUAL;
    virtual void Slot19() DECOMP_LOCAL_VIRTUAL;
    virtual void Slot20() DECOMP_LOCAL_VIRTUAL;
    virtual void Slot21() DECOMP_LOCAL_VIRTUAL;
    virtual void Slot22() DECOMP_LOCAL_VIRTUAL;
    virtual void Slot23() DECOMP_LOCAL_VIRTUAL;
    virtual void Slot24() DECOMP_LOCAL_VIRTUAL;
    virtual void Slot25() DECOMP_LOCAL_VIRTUAL;
    virtual void Slot26() DECOMP_LOCAL_VIRTUAL;
    virtual void Slot27() DECOMP_LOCAL_VIRTUAL;
    virtual void Slot28() DECOMP_LOCAL_VIRTUAL;
    virtual void Slot29() DECOMP_LOCAL_VIRTUAL;
    virtual int UpdateFadeAndMeterTransition() DECOMP_LOCAL_VIRTUAL;
    virtual int ProcessActivatedActionButtons() DECOMP_LOCAL_VIRTUAL;
    virtual int RebuildDirtyLayout() DECOMP_LOCAL_VIRTUAL;
};

extern "C" {

extern char golfPositionDialogStrings[];
extern char bgmCollectionStrings[];
extern char s_saveData[];
extern char s_bpCtrl[];
extern void* s_resourceTableStorage;
extern signed char s_resourceTableInitialized;
extern signed char s_saveDataInitialized;
extern signed char s_bpCtrlInitialized;
extern char s_resourceTableDtorRegistration_BgmCollection[];
extern char s_bpCtrlDtorRegistration_80316848[];

extern unsigned char s_roundDolTaskTypeInfo[1];
extern unsigned char s_roundDolMainTypeInfo[1];

extern void* g_soundManager;
extern void* g_chatMsg;

extern char __vt__11CGolfBase2d[];
extern char __vt__14CBgmCollection[];
extern char __vt__Q23std11logic_error[];
extern char __vt__Q23std12out_of_range[];
extern char s_sharedCharDeleterVtable[];
extern void __dt__Q23std12out_of_rangeFv();
extern void __throw(const char* typeName, void* object, void* destroy);

// Float constants, linked from the original data section.
extern const float kFloat_0_GolfBase2d;
extern float kFloat_1_GolfBase2d;
extern float kFloat_0_5_GolfBase2d;
extern float kFloat_200_GolfBase2d;
extern float kGolfBase2dTransitionFrameDivisor;
extern float kDialogFadeOutOpacityStep;
extern float kDialogEnabledButtonOpacityScale;
extern float kDialogDisabledButtonOpacityScale;
extern float kFloat_6_2831855;
extern float s_twoPi_GolfBase2d;
extern float s_twoPi_RoundPositionDialog;

extern void Layout_Init(void*, int);
extern void Layout_LoadExtended(void*, int, const char*);
extern void Layout_Destroy(void*, int);
extern int Layout_Update(void*);
extern void Layout_ReadElementRect(void*, int, void*);
extern void Layout_WriteElementRect(void*, int, void*);
extern void Layout_GetElementPosition(void*, int, void*);
extern void Layout_GetElementSize(void*, int, void*);
extern void Layout_GetElementScale(void*, int, void*);
extern void Layout_SetElementScale(void*, int, void*);
extern void Layout_SetElementPosition(void*, int, void*);
extern void Layout_SetElementEnabled(void*, int, int);
extern void Layout_SetElementColor(void*, int, int);
extern void Layout_SetElementValueState(void*, int, int);
extern void Layout_SetElementPlayerFlag(void*, int, int, int);
extern void Layout_SetElementPlayerFlagsAndState(void*, int, int);
extern int Layout_CollectActivated(void*, void*, int);
extern void Layout_DrawElementWithOpacity(void*, unsigned int, float);
extern void Layout_DrawWithOpacity(void*, float);
extern void BpObject_Draw(void*);
extern void Sound_PlayByName(void*, const char*, int, int, int, int, float, float, float);

extern void __dt__5CTaskFv(void*, int);
extern void* __dt__13CRoundDolTaskFv(void*, int);
extern void* __dt__13CRoundDolMainFv(void*, int);

extern void appFreeUni__FPv(void*);
extern void* __nwa__FUl(unsigned);
extern void* __nw__FUl(unsigned);
extern void __dla__FPv(void*);
extern void __dl__FPv(void*);
extern char* strcpy(char*, const char*);
extern void* __register_global_object(void*, void*, void*);
extern void Allocation_Dtor(void*);

extern void MPBase1_Ctor(void*);
extern void MPBase1_SetAutoStart(void*, int);
extern void* SaveData_InitPlayerRecord(void*, void*);
extern void* ResourceTable_FindEntryByName(void*, char*, void*);
extern void String_ReplaceRange(void*, int, int, const char*, const char*, const StringAllocator&);
extern void SoundManager_RequestBgmStop(void*, int);
extern void SaveData_Ctor(void*);
extern void CBpCtrl_dtor(void*);
extern void WFixedFont_CreateMsgEcbResource(void*, void*, int, int);
extern void WFixedFont_CreateAndRegisterMsgDraw(void*, void*, int);
extern "C" void* __ct__6CBpTexFv(void*);
extern void* SharedFile_Create(const char*, int);
extern void* SaveData_GetCurrentRecord(void*);

} // extern "C"

// Forward the round task's phases to the common task implementation.
void CRoundDolTask::StartActorLoading(void* forwarded0) {
    ((CTask*)forwarded0)->StartActorLoading();
}
void CRoundDolTask::ProcessActors(void* forwarded0, float forwarded1) {
    ((CTask*)forwarded0)->ProcessActors(forwarded1);
}
void CRoundDolTask::DisplayActorsByPriority(void* forwarded0) {
    ((CTask*)forwarded0)->DisplayActorsByPriority();
}

// Return the round task's runtime type descriptor.
void* CRoundDolTask::GetRTTI() {
    return s_roundDolTaskTypeInfo;
}
// Reserved round task preload hook; it performs no work.
void CRoundDolMain::OnPreLoadInit() {}
// Reserved round task initialization hook; it performs no work.
void CRoundDolMain::OnInit() {}
// Return the round main object's runtime type descriptor.
void* CRoundDolMain::GetRTTI() {
    return s_roundDolMainTypeInfo;
}
// Cache the unit's full-turn angle.
extern "C" void CGolfBase2d_InitializeUnitConstants() {
    s_twoPi_GolfBase2d = kFloat_6_2831855;
}
// Adjust the +0x18 interface pointer back to the owning object before destruction.
void CRoundDolMain::DtorThunk18(int deleting) {
    __dt__13CRoundDolMainFv((char*)this - 0x18, deleting);
}
// Adjust the +8 interface pointer back to the owning task before destruction.
void CRoundDolTask::DtorThunk8(int deleting) {
    __dt__13CRoundDolTaskFv((char*)this - 0x8, deleting);
}
// Destroy the round task base and optionally free its storage.
CRoundDolTask::~CRoundDolTask() {
    __dt__5CTaskFv(this, 0);
}

// Initialize the HUD's markers, action buttons, meter values and fade state.
CGolfBase2d* CGolfBase2d::Init() {
    Layout_Init(this, kGolfLayoutResourceCount);
    float fone;
    float fzero = kFloat_0_GolfBase2d;
    showExtraMeterImage = 0;
    fone = kFloat_1_GolfBase2d;
    interfaceVtable = __vt__11CGolfBase2d;
    displayMode = kPowerMeterDisplay;
    markersEnabled = 0;
    markerIndex = 0;
    markerCount = 0;
    markerStyles[0].offsetY = fzero;
    markerStyles[0].color = -1;
    markerStyles[1].offsetY = fzero;
    markerStyles[1].color = -1;
    markerStyles[2].offsetY = fzero;
    markerStyles[2].color = -1;
    markerStyles[3].offsetY = fzero;
    markerStyles[3].color = -1;
    markerStyles[4].offsetY = fzero;
    markerStyles[4].color = -1;
    markerStyles[5].offsetY = fzero;
    markerStyles[5].color = -1;
    markerStyles[6].offsetY = fzero;
    markerStyles[6].color = -1;
    markerStyles[7].offsetY = fzero;
    markerStyles[7].color = -1;
    markerStyles[8].offsetY = fzero;
    markerStyles[8].color = -1;
    markerStyles[9].offsetY = fzero;
    markerStyles[9].color = -1;
    markerStyles[10].offsetY = fzero;
    markerStyles[10].color = -1;
    markerStyles[11].offsetY = fzero;
    markerStyles[11].color = -1;
    markerStyles[12].offsetY = fzero;
    markerStyles[12].color = -1;
    markerStyles[13].offsetY = fzero;
    markerStyles[13].color = -1;
    markerStyles[14].offsetY = fzero;
    markerStyles[14].color = -1;
    markerStyles[15].offsetY = fzero;
    markerStyles[15].color = -1;
    markerStyles[16].offsetY = fzero;
    markerStyles[16].color = -1;
    markerStyles[17].offsetY = fzero;
    markerStyles[17].color = -1;
    markerStyles[18].offsetY = fzero;
    markerStyles[18].color = -1;
    markerStyles[19].offsetY = fzero;
    markerStyles[19].color = -1;
    markerStyles[20].offsetY = fzero;
    markerStyles[20].color = -1;
    confirmButtonEnabled = 0;
    actionButtonsEnabled = 1;
    actionButtonsVisible = 1;
    confirmButtonActivated = 0;
    cancelButtonActivated = 0;
    tintColor = -1;
    displayValue = fzero;
    pangyaTargetX = fzero;
    pangyaZoneWidth = fone;
    meterOverlayWidth = fzero;
    transitionStartValue = fzero;
    valueTransitionFramesRemaining = 0;
    valueTransitionActive = 0;
    fadeFramesRemaining = 0;
    fadeState = 0;
    fadeOpacity = fzero;
    layoutReady = 0;
    layoutDirty = 1;
    markerBaseY = fzero;
    return this;
}

// Destroy the HUD layout and optionally free its storage. The null check is
// the original's guard on the receiver.
CGolfBase2d* CGolfBase2d::Destroy(int flag) {
    if (this != 0) {
        Layout_Destroy(this, 0);
        if (flag > 0)
            __dl__FPv(this);
    }
    return this;
}

// Load the golf HUD layout for the requested resource index.
int CGolfBase2d::LoadLayout(int resourceIndex) {
    Layout_LoadExtended(this, resourceIndex, golfPositionDialogStrings + kLayoutFilename);
    return 1;
}

// Copy the action-button rectangles and record the centre of the marker row.
int CGolfBase2d::InitializeLayoutGeometry() {
    float markerTop;
    float halfScale;
    float markerHeight;
    unsigned char elementRect[16];
    unsigned char b[8];
    Layout_ReadElementRect(this, kCancelButton, elementRect);
    Layout_WriteElementRect(this, kCancelButtonImage, elementRect);
    Layout_ReadElementRect(this, kConfirmButton, elementRect);
    Layout_WriteElementRect(this, kConfirmButtonImage, elementRect);
    Layout_GetElementPosition(this, kMarkerRangeStart, b);
    markerBaseY = *(float*)(b + 4);
    Layout_GetElementSize(this, kMarkerRangeStart, b);
    markerHeight = *(float*)(b + 4);
    halfScale = kFloat_0_5_GolfBase2d;
    markerTop = markerBaseY;
    layoutReady = 1;
    markerBaseY = markerTop + markerHeight * halfScale;
    return 1;
}

// Update the layout, then the fade, the activated elements and the HUD itself.
int CGolfBase2d::Update() {
    int updated = Layout_Update(this);
    int result = updated & 1;
    HudInterface* hud = (HudInterface*)this;
    result &= hud->UpdateFadeAndMeterTransition();
    result &= hud->ProcessActivatedActionButtons();
    result &= hud->RebuildDirtyLayout();
    return result;
}

// Draw the meter backdrop, the optional HUD object and the layout.
int CGolfBase2d::Draw(void* hudObject) {
    if (displayMode == kPowerMeterDisplay) {
        Layout_DrawElementWithOpacity(this, kMeterBackdrop0, kFloat_1_GolfBase2d);
        Layout_DrawElementWithOpacity(this, kMeterBackdrop1, kFloat_1_GolfBase2d);
        if (showExtraMeterImage != 0) {
            Layout_DrawElementWithOpacity(this, kExtraMeterImage, kFloat_1_GolfBase2d);
        }
    }
    if (hudObject != 0) {
        BpObject_Draw(hudObject);
    }
    Layout_DrawWithOpacity(this, kFloat_1_GolfBase2d);
    return 1;
}

// Show or hide the extra meter image and refresh the layout on a change.
int CGolfBase2d::SetShowExtraMeterImage(int show) {
    if (showExtraMeterImage == show)
        return 1;
    showExtraMeterImage = show;
    layoutDirty = 1;
    return 1;
}

// Validate and change the display mode, refreshing the layout only on a change.
int CGolfBase2d::SetDisplayMode(int mode) {
    if (mode > kMarkerDisplay4 || mode < kNoMeterDisplay)
        return 0;
    if (displayMode == mode)
        return 1;
    displayMode = mode;
    layoutDirty = 1;
    return 1;
}

int CGolfBase2d::GetDisplayMode() {
    return displayMode;
}

// Enable or disable marker display and refresh the layout when it changes.
int CGolfBase2d::SetMarkersEnabled(int enabled) {
    if (markersEnabled != enabled) {
        markersEnabled = enabled;
        layoutDirty = 1;
    }
    return 1;
}

// Store the selected marker index and the marker count without validation.
// markerIndex is zero-based; the drawing loop reads style n for marker n, so a
// count above 20 would read past the style storage.
int CGolfBase2d::SetMarkerSelection(int index, int count) {
    markerIndex = index;
    markerCount = count;
    layoutDirty = 1;
    return 1;
}

// Copy the marker styles in and request a layout refresh. The caller supplies
// 21 {vertical offset, packed colour} pairs; a null source is rejected.
int CGolfBase2d::SetMarkerStyles(void* styles) {
    if (styles == 0)
        return 0;
    MarkerStyle* in = (MarkerStyle*)styles;
    for (int i = 0; i < kMarkerStyleCount; ++i) {
        markerStyles[i].offsetY = in[i].offsetY;
        markerStyles[i].color = in[i].color;
    }
    layoutDirty = 1;
    return 1;
}

// Enable the confirm button and clear its activation latch when disabling it.
int CGolfBase2d::SetConfirmButtonEnabled(int enabled) {
    if (confirmButtonEnabled == enabled) {
        return 1;
    }
    confirmButtonEnabled = enabled;
    layoutDirty = 1;
    Layout_SetElementPlayerFlag(this, kConfirmButton, enabled, kButtonAnimationFlag);
    int value = confirmButtonEnabled;
    Layout_SetElementPlayerFlag(this, kConfirmButtonImage, value, kButtonAnimationFlag);
    if (confirmButtonEnabled == 0) {
        confirmButtonActivated = 0;
    }
    return 1;
}

// Enable or disable action-button input and refresh the layout on a change.
int CGolfBase2d::SetActionButtonsEnabled(int enabled) {
    if (actionButtonsEnabled == enabled)
        return 1;
    actionButtonsEnabled = enabled;
    layoutDirty = 1;
    return 1;
}

// Show or hide the action buttons and refresh the layout on a change.
int CGolfBase2d::SetActionButtonsVisible(int visible) {
    if (actionButtonsVisible == visible)
        return 1;
    actionButtonsVisible = visible;
    layoutDirty = 1;
    return 1;
}

// Return the latched confirm action.
int CGolfBase2d::IsConfirmButtonActivated() {
    return confirmButtonActivated;
}

// Return the latched cancel action.
int CGolfBase2d::IsCancelButtonActivated() {
    return cancelButtonActivated;
}

// Latch the confirm action when the dialog is visible and play its click sound.
int CGolfBase2d::ActivateConfirmButton() {
    if (confirmButtonActivated == 0) {
        if (fadeState != kFadeHidden) {
            confirmButtonActivated = 1;
            layoutDirty = 1;
            const char* sound = golfPositionDialogStrings + kButtonSound;
            Sound_PlayByName(g_soundManager, sound, 0, 0, 0, 0, 0.5f, 200.0f, 1.0f);
        }
    }
    return 1;
}

// Latch the cancel action when the dialog is visible and play its click sound.
int CGolfBase2d::ActivateCancelButton() {
    if (cancelButtonActivated == 0) {
        if (fadeState != kFadeHidden) {
            cancelButtonActivated = 1;
            layoutDirty = 1;
            const char* sound = golfPositionDialogStrings + kButtonSound;
            Sound_PlayByName(g_soundManager, sound, 0, 0, 0, 0, 0.5f, 200.0f, 1.0f);
        }
    }
    return 1;
}

// Set the packed HUD tint and request a layout refresh.
int CGolfBase2d::SetTintColor(int color) {
    tintColor = color;
    layoutDirty = 1;
    return 1;
}

// Set the displayed meter value and refresh the layout on a change.
int CGolfBase2d::SetDisplayValue(float value) {
    if (displayValue != value) {
        displayValue = value;
        layoutDirty = 1;
    }
    return 1;
}

// Start a five-frame meter transition from the supplied value.
int CGolfBase2d::StartValueTransition(float value) {
    valueTransitionActive = 1;
    valueTransitionFramesRemaining = kFadeInFrames;
    transitionStartValue = value;
    layoutDirty = 1;
    return 1;
}

// Stop the meter-value transition.
int CGolfBase2d::StopValueTransition() {
    valueTransitionActive = 0;
    return 1;
}

// Set the horizontal target position of the Pangya zone.
int CGolfBase2d::SetPangyaTargetX(float x) {
    if (pangyaTargetX != x) {
        pangyaTargetX = x;
        layoutDirty = 1;
    }
    return 1;
}

// Set the width scale of the Pangya zone.
int CGolfBase2d::SetPangyaZoneWidth(float width) {
    if (pangyaZoneWidth != width) {
        pangyaZoneWidth = width;
        layoutDirty = 1;
    }
    return 1;
}

// Set the requested width of the meter overlay and refresh on a change.
int CGolfBase2d::SetMeterOverlayWidth(float width) {
    if (meterOverlayWidth != width) {
        meterOverlayWidth = width;
        layoutDirty = 1;
    }
    return 1;
}

// Start the five-frame dialog fade-in and clear both action latches.
int CGolfBase2d::StartFadeIn() {
    float zeroFloat = kFloat_0_GolfBase2d;
    int one = 1;
    int zero = 0;
    int five = kFadeInFrames;
    fadeState = one;
    fadeFramesRemaining = five;
    fadeOpacity = zeroFloat;
    layoutDirty = one;
    confirmButtonActivated = zero;
    cancelButtonActivated = zero;
    return 1;
}

// Start the seven-frame dialog fade-out when the dialog is visible.
int CGolfBase2d::StartFadeOut() {
    if (fadeState != kFadeHidden) {
        float oneFloat = kFloat_1_GolfBase2d;
        int three = kFadeOut;
        int seven = kFadeOutFrames;
        fadeState = three;
        fadeFramesRemaining = seven;
        fadeOpacity = oneFloat;
    }
    return 1;
}

// Report whether neither fade-in nor fade-out is in progress.
int CGolfBase2d::IsFadeSettled() {
    int value = fadeState;
    if (value != kFadeIn && value != kFadeOut)
        return 1;
    return 0;
}

// Report whether the dialog is fully visible.
int CGolfBase2d::IsFadeVisible() {
    return (*(unsigned int*)&fadeState) == kFadeVisible;
}

// Make a hidden dialog fully visible immediately and clear the action latches.
int CGolfBase2d::ShowImmediately() {
    if (fadeState == kFadeHidden) {
        float oneFloat = kFloat_1_GolfBase2d;
        int zero = 0;
        int two = kFadeVisible;
        int one = 1;
        fadeFramesRemaining = zero;
        fadeOpacity = oneFloat;
        fadeState = two;
        layoutDirty = one;
        confirmButtonActivated = zero;
        cancelButtonActivated = zero;
    }
    return 1;
}

// Advance the dialog fade and the meter transition by one update, playing the
// close sound at the frame the dialog is no longer fully opaque.
int CGolfBase2d::UpdateFadeAndMeterTransition() {
    int fadeFrames = fadeFramesRemaining;
    if (fadeFrames != 0) {
        int remainingFrames = fadeFrames - 1;
        int state = fadeState;
        fadeFramesRemaining = remainingFrames;
        layoutDirty = 1;
        if (state == kFadeIn) {
            float updated = (float)(kFadeInFrames - remainingFrames) / kGolfBase2dTransitionFrameDivisor;
            fadeOpacity = updated;
            if (remainingFrames == 0) {
                fadeState = kFadeVisible;
            }
        } else if (state == kFadeOut) {
            if (remainingFrames < kFadeOutSoundFrame) {
                float updated = (float)remainingFrames * kDialogFadeOutOpacityStep;
                fadeOpacity = updated;
            }
            if (fadeFramesRemaining == kFadeOutSoundFrame) {
                void* manager = g_soundManager;
                const char* sound = golfPositionDialogStrings + kFadeSound;
                Sound_PlayByName(manager, sound, 0, 0, 0, 0, kFloat_0_5_GolfBase2d, kFloat_200_GolfBase2d,
                                 kFloat_1_GolfBase2d);
            }
            if (fadeFramesRemaining == 0) {
                fadeState = kFadeHidden;
                confirmButtonActivated = 0;
                cancelButtonActivated = 0;
                layoutDirty = 1;
            }
        }
    }
    // The interpolation countdown is separate; valueTransitionActive stays set
    // until StopValueTransition clears it, even when the countdown reaches zero.
    int transitionFrames = valueTransitionFramesRemaining;
    if (transitionFrames != 0) {
        layoutDirty = 1;
        valueTransitionFramesRemaining = transitionFrames - 1;
    }
    return 1;
}

// The shared string block of the runtime exception object: a vtable, two
// reference counts and the text.
struct SharedCharRef {
    int vt;
    int c1;
    int c2;
    char* s;
};

// std::logic_error as this unit builds it. The text is copied into a new char[]
// that the reference-counted block owns.
struct LogicErrorBase {
    void* vt;
    char* buf;
    SharedCharRef* ref;
    LogicErrorBase(const char* msg) {
        char done = 0;
        vt = __vt__Q23std11logic_error;
        const char* m = msg;
        unsigned n = strlen(m) + 1;
        unsigned bytes = (n << 1) - n;
        char* b = (char*)__nwa__FUl(bytes);
        buf = b;
        {
            // Holds the text until the reference-counted block is built.
            struct Holder {
                char* p;
                char* flag;
                Holder(char* pp, char* f) {
                    p = pp;
                    flag = f;
                }
                ~Holder() {
                    if (p)
                        __dla__FPv(p);
                }
            } h(b, &done);
            SharedCharRef* r = (SharedCharRef*)__nw__FUl(16);
            if (r) {
                int table = (int)s_sharedCharDeleterVtable;
                r->c1 = 1;
                r->c2 = 1;
                r->vt = table;
                r->s = b;
            }
            ref = r;
            h.p = 0;
        }
        strcpy(buf, m);
    }
    ~LogicErrorBase() {}
};

// std::out_of_range: completes the exception object and throws it through the
// runtime. The local destructor is empty to keep the original unwind structure.
struct OutOfRangeError : LogicErrorBase {
    OutOfRangeError(const char* msg) : LogicErrorBase(msg) {
        vt = __vt__Q23std12out_of_range;
        __throw(golfPositionDialogStrings + kRangeErrorType, this, (void*)__dt__Q23std12out_of_rangeFv);
    }
    ~OutOfRangeError() {}
};

#pragma opt_common_subs off
#pragma opt_strength_reduction off
#pragma use_lmw_stmw on
// Read the activated layout elements and latch the enabled confirm or cancel action.
int CGolfBase2d::ProcessActivatedActionButtons() {
    if (fadeState != kFadeVisible) {
        return 0;
    }
    int zeroReg = 0;
    unsigned activationByteOffset;
    unsigned activationIndex;
    int activated[3];
    activated[0] = zeroReg;
    activated[1] = zeroReg;
    activated[2] = zeroReg;
    Layout_CollectActivated(this, activated, zeroReg);
    // CollectActivated returns an owned {data, size, capacity} array of element IDs.
    activationByteOffset = zeroReg;
    activationIndex = zeroReg;
    for (activationIndex = 0; activationIndex < (unsigned)activated[1]; ++activationIndex, activationByteOffset += 4) {
        if ((unsigned)activationIndex >= (unsigned)activated[1]) {
            OutOfRangeError error(golfPositionDialogStrings + kRangeErrorText);
        }
        int value = *(int*)((char*)activated[0] + activationByteOffset);
        if (value == kConfirmButton) {
            if (confirmButtonEnabled != 0) {
                confirmButtonActivated = 1;
                layoutDirty = 1;
            }
        }
        if ((unsigned)activationIndex >= (unsigned)activated[1]) {
            OutOfRangeError error(golfPositionDialogStrings + kRangeErrorText);
        }
        if (*(int*)((char*)activated[0] + activationByteOffset) == kCancelButton) {
            cancelButtonActivated = 1;
            layoutDirty = 1;
        }
    }
    if (activated[0] != 0) {
        ((volatile int*)activated)[1] = ((volatile int*)activated)[1] - ((volatile int*)activated)[1];
        appFreeUni__FPv((void*)activated[0]);
    }
    return 1;
}
#pragma opt_common_subs reset
#pragma opt_strength_reduction reset

// Load the BGM collection resources, snapshot the save record and initialize the
// menu controller.
CBgmCollection::CBgmCollection() {
    struct Resource {
        char data[0x20];
        // Wrap the resource-table archive bytes in an owned texture-resource object.
        Resource(char* name, int size, void* data) {
            ((CPackTextureResource*)this)->Init((const char*)name, (void**)size, (unsigned int)data);
        }
    };
    struct Controller {
        char data[0x5C];
        // Initialize the layout owner for the given resource count.
        Controller(int count) {
            Layout_Init(this, count);
        }
    };
    struct State {
        char data[0x28];
        // Initialize the owned BP texture state.
        State() {
            __ct__6CBpTexFv(this);
        }
    };
    char* selfc = (char*)this;
    int resourceSize;
    StringAllocator argument;
    StringAllocator allocator;
    // Phase 1: construct base/string/save storage, then load texture and layout owners.
    MPBase1_Ctor(this);
    (*(void**)&((CBgmCollection*)selfc)->stringFlags) = 0;
    char* vtable = __vt__14CBgmCollection;
    ((CBgmCollection*)selfc)->vtbl = (void*)vtable;
    (*(void**)&((CBgmCollection*)selfc)->stringLength) = 0;
    ((CBgmCollection*)selfc)->stringData = 0;
    SaveData_InitPlayerRecord(((CBgmCollection*)selfc)->playerRecord, vtable);
    void* resourceManager = Sing_805DC620();
    (*(int*)&((CBgmCollection*)selfc)->sharedFile) = 0;
    (*(int*)&((CBgmCollection*)selfc)->textureResource) = 0;
    resourceSize = 0;
    if (resourceManager == 0) {
        if (!s_resourceTableInitialized) {
            *(void**)((char*)&s_resourceTableStorage) = 0;
            __register_global_object(&s_resourceTableStorage, (void*)Allocation_Dtor,
                                     s_resourceTableDtorRegistration_BgmCollection);
            s_resourceTableInitialized = 1;
        }
        Sing_805DC620() = &s_resourceTableStorage;
    }
    void* found;
    void* entry = ResourceTable_FindEntryByName(Sing_805DC620(), bgmCollectionStrings, &resourceSize);
    found = new Resource(bgmCollectionStrings, resourceSize, entry);
    ((CBgmCollection*)selfc)->textureResource = found;
    ((CBgmCollection*)selfc)->bpEntryId = -1;
    (*(int*)&((CBgmCollection*)selfc)->trackTable) = 0;
    (*(int*)&((CBgmCollection*)selfc)->messageResource) = 0;
    (*(int*)&((CBgmCollection*)selfc)->chatResource) = 0;
    (*(int*)&((CBgmCollection*)selfc)->messageDraw) = 0;
    (*(int*)&((CBgmCollection*)selfc)->textureState) = 0;
    ((CBgmCollection*)selfc)->selectedTrackIndex = 0;
    found = new Controller(kGolfLayoutResourceCount);
    ((CBgmCollection*)selfc)->layout = found;
    ((CBgmCollection*)selfc)->finished = 0;
    ((CBgmCollection*)selfc)->buttonDelayActive = 0;
    ((CBgmCollection*)selfc)->buttonDelayUpdateCount = 0;
    *(unsigned char*)&argument = *(volatile unsigned char*)&allocator;
    String_ReplaceRange(selfc + (int)&((CBgmCollection*)0)->stringFlags, 0, -1, 0, 0, argument);
    ((CBgmCollection*)selfc)->inputInitialized = 0;
    ((CBgmCollection*)selfc)->exitRequested = 0;
    MPBase1_SetAutoStart(this, 1);
    SoundManager_RequestBgmStop(g_soundManager, 1);
    if (GetSaveDataSlot() == 0) {
        if (!s_saveDataInitialized) {
            SaveData_Ctor(s_saveData);
            s_saveDataInitialized = 1;
        }
        GetSaveDataSlot() = (void*)s_saveData;
    }
    // Phase 2: snapshot persisted save regions, keeping the original copy widths.
    void* src = SaveData_GetCurrentRecord(GetSaveDataSlot());
    struct Data {
        // Boundaries match the player record in the save data. Keep the scalar
        // widths: changing these groups changes the generated copies.
        unsigned headerAndInventoryWords[151];           // +000..25B
        unsigned courseRecordWords[28];                  // +25C..2CB
        unsigned short characterEquipmentHalfwords[208]; // +2CC..46B
        unsigned caddieEquipmentWords[16];               // +46C..4AB
        unsigned char clubSetPrefixBytes[24];            // +4AC..4C3
        unsigned char clubSetByte24, clubSetByte25, clubSetByte26;
        unsigned char clubSetByte27, clubSetByte28, clubSetByte29;
        unsigned char ballPrefixBytes[8];               // +4CA..4D1
        unsigned char ballByte8, ballByte9, ballByte10; // then 3 implicit padding bytes
        unsigned systemSettingsWords[5];                // +4D8..4EB
        unsigned short modeRecordHalfwords[414];        // +4EC..827
    };
    *(Data*)(selfc + (int)&((CBgmCollection*)0)->playerRecord) = *(Data*)src;
    if (Sing_805DC5F0() == 0) {
        if (!s_bpCtrlInitialized) {
            ((CBpCtrl*)s_bpCtrl)->__ct();
            __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, s_bpCtrlDtorRegistration_80316848);
            s_bpCtrlInitialized = 1;
        }
        Sing_805DC5F0() = (void*)s_bpCtrl;
    }
    // Phase 3: register the BP controller, borrow chat resources, create state.
    ((CBpCtrl*)Sing_805DC5F0())->CreateEntry(bgmCollectionStrings + kBgmControllerOffset, selfc + 0x18, 0);
    Layout_LoadExtended(((CBgmCollection*)selfc)->layout, 0, bgmCollectionStrings + kBgmLayoutOffset);
    void* chat = g_chatMsg;
    void* font = ((CChatMsg*)chat)->defaultFixedFont;
    ((CBgmCollection*)selfc)->chatResource = font;
    WFixedFont_CreateMsgEcbResource(font, selfc + 0x24, 0, 0);
    WFixedFont_CreateAndRegisterMsgDraw(*(void**)(selfc + 0x28), selfc + 0x2C, 1);
    found = new State;
    ((CBgmCollection*)selfc)->textureState = found;
    void* file = SharedFile_Create(bgmCollectionStrings + kBgmMessagesOffset, 0);
    ((CBgmCollection*)selfc)->sharedFile = file;
}

#pragma opt_propagation off
#pragma push
#pragma always_inline on
// Refresh the action-button colours, input gates and latched value states.
// The scalar references borrow caller locals so the inline split keeps float
// register allocation; the layout pointer and the HUD pointer stay separate.
static inline void RefreshGolfHudButtons(void* layout, char* hud, int& packedButtonColor, float& enabledScale,
                                         float& enabledOpacity, float& disabledScale, float& disabledOpacity) {
    CGolfBase2d* object = (CGolfBase2d*)hud;
    // Button colours use rotated packed bytes: opacityByte - 0x100 gives white
    // RGB with the wanted low alpha byte.
    enabledScale = kDialogEnabledButtonOpacityScale;
    enabledOpacity = object->fadeOpacity;
    enabledOpacity = enabledScale * enabledOpacity;
    int opacityByte = (int)enabledOpacity;
    if (opacityByte > 0xff)
        opacityByte = 0xff;
    packedButtonColor = opacityByte - 0x100;
    Layout_SetElementColor(layout, kCancelButton, packedButtonColor);
    Layout_SetElementColor(layout, kCancelButtonImage, packedButtonColor);
    Layout_SetElementColor(layout, kConfirmButton, packedButtonColor);
    Layout_SetElementColor(layout, kConfirmButtonImage, packedButtonColor);
    if (object->cancelButtonActivated != 0) {
        Layout_SetElementPlayerFlag(layout, kCancelButton, 0, kButtonAnimationFlag);
        Layout_SetElementPlayerFlag(layout, kCancelButtonImage, 0, kButtonAnimationFlag);
        Layout_SetElementValueState(layout, kCancelButton, kButtonActivated);
        Layout_SetElementValueState(layout, kCancelButtonImage, kButtonActivated);
    } else if (object->actionButtonsEnabled != 0) {
        Layout_SetElementPlayerFlag(layout, kCancelButton, 1, kButtonAnimationFlag);
        Layout_SetElementPlayerFlag(layout, kCancelButtonImage, 1, kButtonAnimationFlag);
        Layout_SetElementValueState(layout, kCancelButton, kButtonIdle);
        Layout_SetElementValueState(layout, kCancelButtonImage, kButtonIdle);
    } else {
        Layout_SetElementPlayerFlag(layout, kCancelButton, 0, kButtonAnimationFlag);
        Layout_SetElementPlayerFlag(layout, kCancelButtonImage, 0, kButtonAnimationFlag);
    }
    if (object->confirmButtonActivated != 0) {
        Layout_SetElementPlayerFlag(layout, kConfirmButton, 0, kButtonAnimationFlag);
        Layout_SetElementPlayerFlag(layout, kConfirmButtonImage, 0, kButtonAnimationFlag);
        Layout_SetElementValueState(layout, kConfirmButton, kButtonActivated);
        Layout_SetElementValueState(layout, kConfirmButtonImage, kButtonActivated);
    } else {
        if (object->confirmButtonEnabled != 0 && object->actionButtonsEnabled != 0) {
            Layout_SetElementPlayerFlag(layout, kConfirmButton, 1, kButtonAnimationFlag);
            Layout_SetElementPlayerFlag(layout, kConfirmButtonImage, 1, kButtonAnimationFlag);
        } else {
            Layout_SetElementPlayerFlag(layout, kConfirmButton, 0, kButtonAnimationFlag);
            Layout_SetElementPlayerFlag(layout, kConfirmButtonImage, 0, kButtonAnimationFlag);
            if (object->confirmButtonEnabled == 0) {
                disabledScale = kDialogDisabledButtonOpacityScale;
                disabledOpacity = object->fadeOpacity;
                disabledOpacity = disabledScale * disabledOpacity;
                int disabledOpacityByte = (int)disabledOpacity;
                if (disabledOpacityByte > 0xff)
                    disabledOpacityByte = 0xff;
                int disabledButtonColor = disabledOpacityByte - 0x100;
                Layout_SetElementColor(layout, kConfirmButton, disabledButtonColor);
                Layout_SetElementColor(layout, kConfirmButtonImage, disabledButtonColor);
            }
        }
    }
    Layout_SetElementValueState(layout, kConfirmButton, kButtonIdle);
    Layout_SetElementValueState(layout, kConfirmButtonImage, kButtonIdle);
}
#pragma pop

// Rebuild the dirty HUD elements from the meter, button, marker and fade state.
int CGolfBase2d::RebuildDirtyLayout() {
    char* s = (char*)this;
    if (layoutReady == 0)
        return 0;
    if (layoutDirty == 0)
        return 0;
    layoutDirty = 0;
    // Phase 1: reset all element state and tint; later phases reveal groups.
    int elementOrColor = 0;
    for (elementOrColor = 0; elementOrColor < kGolfLayoutElementCount; elementOrColor++) {
        Layout_SetElementPlayerFlagsAndState(this, elementOrColor, 0);
        int tint = tintColor;
        Layout_SetElementColor(this, elementOrColor, tint);
    }
    Layout_SetElementColor(this, kCancelButton, -1);
    Layout_SetElementColor(this, kCancelButtonImage, -1);
    Layout_SetElementColor(this, kConfirmButton, -1);
    Layout_SetElementColor(this, kConfirmButtonImage, -1);
    if (fadeState != kFadeHidden && actionButtonsVisible != 0) {
        Layout_SetElementEnabled(this, kCancelButton, 1);
        Layout_SetElementEnabled(this, kCancelButtonImage, 1);
        Layout_SetElementEnabled(this, kConfirmButton, 1);
        Layout_SetElementEnabled(this, kConfirmButtonImage, 1);
    }
    // Phase 2: refresh the button colours and gates with caller-owned scalars.
    float buttonScale, buttonOpacity, disabledScale, disabledOpacity;
    RefreshGolfHudButtons(this, s, elementOrColor, buttonScale, buttonOpacity, disabledScale, disabledOpacity);
    // Phase 3: display-mode artwork and meter width. The interpolated value is
    // target + (start - target) * remaining / 5, in layout X coordinates.
    int mode = displayMode;
    if (mode == kPowerMeterDisplay) {
        Layout_SetElementEnabled(this, kPowerMeterEndImage, 1);
        Layout_SetElementEnabled(this, kInterpolatedMeter, 1);
        if (valueTransitionActive != 0) {
            Layout_SetElementEnabled(this, kTransitionStartMeter, 1);
            void* vt = interfaceVtable;
            float startValue = transitionStartValue;
            ((void (*)(void*, int, float))(((void**)vt)[kScaleElementVtableSlot]))(this, kTransitionStartMeter,
                                                                                   startValue);
            int transitionFrames = valueTransitionFramesRemaining;
            void* vt2 = interfaceVtable;
            float interpolatedValue = transitionStartValue;
            float framesAsFloat = (float)(int)transitionFrames;
            float targetValue = displayValue;
            float weightedDifference = interpolatedValue - targetValue;
            weightedDifference = framesAsFloat * weightedDifference;
            weightedDifference = weightedDifference / kGolfBase2dTransitionFrameDivisor;
            interpolatedValue = targetValue + weightedDifference;
            ((void (*)(void*, int, float))(((void**)vt2)[kScaleElementVtableSlot]))(this, kInterpolatedMeter,
                                                                                    interpolatedValue);
        } else {
            void* vt = interfaceVtable;
            float meterValue = displayValue;
            ((void (*)(void*, int, float))(((void**)vt)[kScaleElementVtableSlot]))(this, kInterpolatedMeter,
                                                                                   meterValue);
        }
    } else if (mode != kNoMeterDisplay) {
        Layout_SetElementEnabled(this, 0, 1);
        Layout_SetElementEnabled(this, 5, 1);
        Layout_SetElementEnabled(this, 6, 1);
        Layout_SetElementEnabled(this, 7, 1);
        Layout_SetElementEnabled(this, 8, 1);
        Layout_SetElementEnabled(this, 4, 1);
        void* vt = interfaceVtable;
        float alternateMeterValue = displayValue;
        ((void (*)(void*, int, float))(((void**)vt)[kScaleElementVtableSlot]))(this, 4, alternateMeterValue);
    }
    mode = displayMode;
    switch (mode) {
    case kPowerMeterDisplay: {
        int groupElement = kPowerArtworkFirst;
        while (groupElement <= kPowerArtworkLast) {
            Layout_SetElementEnabled(this, groupElement, 1);
            groupElement++;
        }
        break;
    }
    case kMarkerDisplay2: {
        int groupElement = kMode2ArtworkFirst;
        while (groupElement <= kMode2ArtworkLast) {
            Layout_SetElementEnabled(this, groupElement, 1);
            groupElement++;
        }
        break;
    }
    case kMarkerDisplay3: {
        int groupElement = kMode3ArtworkFirst;
        while (groupElement <= kMode3ArtworkLast) {
            Layout_SetElementEnabled(this, groupElement, 1);
            groupElement++;
        }
        break;
    }
    case kMarkerDisplay4: {
        int groupElement = kMode4ArtworkFirst;
        while (groupElement <= kMode4ArtworkLast) {
            Layout_SetElementEnabled(this, groupElement, 1);
            groupElement++;
        }
        break;
    }
    }
    mode = displayMode;
    if (mode == kPowerMeterDisplay) {
        Layout_SetElementEnabled(this, kPangyaZone, 1);
        Layout_SetElementEnabled(this, kMeterOverlay, 1);
        float overlayScale = kFloat_1_GolfBase2d;
        float buf0[2];
        Layout_GetElementPosition(this, kPangyaZone, buf0);
        float targetX = pangyaTargetX;
        buf0[0] = targetX;
        Layout_SetElementPosition(this, kPangyaZone, buf0);
        Layout_GetElementScale(this, kPangyaZone, buf0);
        float targetWidth = pangyaZoneWidth;
        buf0[0] = targetWidth;
        Layout_SetElementScale(this, kPangyaZone, buf0);
        Layout_GetElementSize(this, kMeterOverlay, buf0);
        if (kFloat_0_GolfBase2d != buf0[0]) {
            float overlayWidth = meterOverlayWidth;
            overlayScale = overlayWidth / buf0[0];
        }
        Layout_GetElementScale(this, kMeterOverlay, buf0);
        buf0[0] = overlayScale;
        Layout_SetElementScale(this, kMeterOverlay, buf0);
    }
    // Phase 4: the selected marker sits at start + index * spacing; style n at
    // start + (n - 0.5) * spacing - 1, Y = markerBaseY + style offset. Colours
    // combine the owner's tint with each style and rotate the packed bytes.
    if (markersEnabled != 0) {
        int selectedIndex = markerIndex;
        int count = markerCount;
        if (count > selectedIndex) {
            float bufA[2];
            Layout_GetElementPosition(this, kMarkerRangeStart, bufA);
            float markerStartX = bufA[0];
            Layout_GetElementPosition(this, kMarkerRangeEnd, bufA);
            float markerEndX = bufA[0];
            mode = displayMode;
            unsigned int markerCountUnsigned = (*(unsigned int*)&markerCount);
            float markerSpan = markerEndX - markerStartX;
            float markerCountAsFloat = (float)(int)markerCountUnsigned;
            float markerSpacing = markerSpan / markerCountAsFloat;
            switch (mode) {
            case kMarkerDisplay2: {
                Layout_SetElementEnabled(this, 1, 1);
                float bufB[2];
                Layout_GetElementPosition(this, 1, bufB);
                int index = markerIndex;
                float markerX = (float)(int)index;
                markerX = markerSpacing * markerX + markerStartX;
                bufB[0] = markerX;
                Layout_SetElementPosition(this, 1, bufB);
                break;
            }
            case kMarkerDisplay3: {
                Layout_SetElementEnabled(this, 2, 1);
                float bufB[2];
                Layout_GetElementPosition(this, 2, bufB);
                int index = markerIndex;
                float markerX = (float)(int)index;
                markerX = markerSpacing * markerX + markerStartX;
                bufB[0] = markerX;
                Layout_SetElementPosition(this, 2, bufB);
                break;
            }
            case kMarkerDisplay4: {
                Layout_SetElementEnabled(this, 3, 1);
                float bufB[2];
                Layout_GetElementPosition(this, 3, bufB);
                int index = markerIndex;
                float markerX = (float)(int)index;
                markerX = markerSpacing * markerX + markerStartX;
                bufB[0] = markerX;
                Layout_SetElementPosition(this, 3, bufB);
                break;
            }
            }
            mode = displayMode;
            if (mode > kPowerMeterDisplay) {
                int markerTint = tintColor;
                // The cursor keeps its byte base at the start of marker style 1.
                char* styleCursor = (char*)this + sizeof(MarkerStyle);
                float halfMarker = kFloat_0_5_GolfBase2d;
                unsigned int packedTint = ((unsigned int)markerTint << 24) | 0x00FFFFFFu;
                float markerInset = kFloat_1_GolfBase2d;
                int markerNumber = 1;
                while (markerNumber < markerCount + 1) {
                    int styleColor = ((GolfMarkerStyleCursor*)styleCursor)->style.color;
                    unsigned int tintedColor = packedTint & (unsigned int)styleColor;
                    unsigned int rotatedColor = (tintedColor << 8) | (tintedColor >> 24);
                    float styleX = (float)(int)markerNumber;
                    styleX = styleX - halfMarker;
                    styleX = markerSpacing * styleX + markerStartX;
                    styleX = styleX - markerInset;
                    float bufC[2];
                    bufC[0] = styleX;
                    float baseY = markerBaseY;
                    float styleY = ((GolfMarkerStyleCursor*)styleCursor)->style.offsetY;
                    styleY = baseY + styleY;
                    bufC[1] = styleY;
                    Layout_SetElementPosition(this, markerNumber + kMarkerElementBase, bufC);
                    Layout_SetElementColor(this, markerNumber + kMarkerElementBase, (int)rotatedColor);
                    Layout_SetElementEnabled(this, markerNumber + kMarkerElementBase, 1);
                    styleCursor += sizeof(MarkerStyle);
                    markerNumber++;
                }
            }
        }
    }
    // Phase 5: a hidden dialog cannot accept input, including on revealed elements.
    if (fadeState == kFadeHidden && layoutReady != 0) {
        for (int i = 0; i < kGolfLayoutElementCount; i++) {
            Layout_SetElementPlayerFlag(this, i, 0, kButtonAnimationFlag);
        }
    }
    return 1;
}
#pragma opt_propagation reset

// Scale an element horizontally to reach the supplied X coordinate, clamping
// the ratio below zero. The element's Y scale is left alone; the values are
// layout units, not normalized meter values.
int CGolfBase2d::ScaleElementToX(int index, float x) {
    float buf[2];
    float start;
    float ratio = kFloat_0_GolfBase2d;
    Layout_GetElementPosition(this, index, buf);
    start = buf[0];
    Layout_GetElementSize(this, index, buf);
    float span = buf[0];
    if (kFloat_0_GolfBase2d != span) {
        ratio = (x - start) / span;
    }
    if (ratio < kFloat_0_GolfBase2d) {
        ratio = kFloat_0_GolfBase2d;
    }
    Layout_GetElementScale(this, index, buf);
    buf[0] = ratio;
    Layout_SetElementScale(this, index, buf);
    return 1;
}

// Cache the round-position unit's full-turn angle.
extern "C" void RoundPositionDialog_InitializeUnitConstants() {
    s_twoPi_RoundPositionDialog = kFloat_6_2831855;
}

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU804625C8", _ctors_u80316848_0, CGolfBase2d_InitializeUnitConstants);
STATIC_INIT_ENTRY(".CRT$XCU804625C8", _ctors_u80316848_1, RoundPositionDialog_InitializeUnitConstants);
#pragma pop

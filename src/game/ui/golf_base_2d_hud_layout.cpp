#include <decomp/static_init.h>
#include "game/chatmsg_match.h"
#include "classes/CBgmCollection.h"
#include "game/singletons.h"
#include "sdk_protos.h"
#include "game/types.h"
#include "classes/CGolfBase2d.h"

// Borrow the HUD layout; a cast macro preserves comparison operand order in MWCC.
#define AsCGolfBase2d(p) ((CGolfBase2d*)(p))



// Flags shared by the two button images in each action. These names apply to this HUD layout.
// Access the optional third meter-backdrop image toggle.
inline int& showExtraMeterImage(void* self) { return AsCGolfBase2d(self)->showExtraMeterImage; }
// Access the confirm-specific input gate (independent of the shared button gate).
inline int& confirmButtonEnabled(void* self) { return AsCGolfBase2d(self)->confirmButtonEnabled; }
// Access the shared action-button input gate.
inline int& actionButtonsEnabled(void* self) { return AsCGolfBase2d(self)->actionButtonsEnabled; }
// Access the visibility gate for both action-button image pairs.
inline int& actionButtonsVisible(void* self) { return AsCGolfBase2d(self)->actionButtonsVisible; }
// Access the latched confirm action; it persists until explicitly cleared.
inline int& confirmButtonActivated(void* self) { return AsCGolfBase2d(self)->confirmButtonActivated; }
// Access the latched cancel action; it persists until explicitly cleared.
inline int& cancelButtonActivated(void* self) { return AsCGolfBase2d(self)->cancelButtonActivated; }
// Access the overlay's requested layout width, before division by its authored width.
inline float& meterOverlayWidth(void* self) { return AsCGolfBase2d(self)->meterOverlayWidth; }
struct GolfHudStringPool {
    char layoutFilename[22], clickSound[14], closeSound[22];
    char vectorRangeError[30], outOfRangeType[55];
};
struct BgmCollectionResourcePool {
    char archive[19], controller[22], layout[22], messageTable[19];
};
enum GolfBaseLayoutConstants {
    kBgmStringOffset = (int)&((CBgmCollection*)0)->stringFlags,
    kBgmPlayerRecordOffset = (int)&((CBgmCollection*)0)->playerRecord,
    kGolfLayoutResourceCount = 5, kMarkerStyleCount = 21,
    kMeterBackdrop0 = 0x34, kMeterBackdrop1 = 0x35, kExtraMeterImage = 0x36,
    kTransitionStartMeter = 0x37, kInterpolatedMeter = 0x38,
    kMeterOverlay = 0x39, kPangyaZone = 0x43,
    kCancelButton = 0x45, kCancelButtonImage = 0x46,
    kConfirmButton = 0x47, kConfirmButtonImage = 0x48,
    kMarkerRangeEnd = 0x49, kMarkerRangeStart = 0x4a,
    kMarkerElementBase = 0x4c, kGolfLayoutElementCount = 0x61,
    kFadeHidden = 0, kFadeIn = 1, kFadeVisible = 2, kFadeOut = 3,
    kFadeInFrames = 5, kFadeOutFrames = 7, kFadeOutSoundFrame = 4,
    kButtonAnimationFlag = 5,
    kButtonSoundOffset = (int)&((GolfHudStringPool*)0)->clickSound, kFadeSoundOffset = (int)&((GolfHudStringPool*)0)->closeSound,
    kRangeErrorTextOffset = (int)&((GolfHudStringPool*)0)->vectorRangeError, kRangeErrorTypeOffset = (int)&((GolfHudStringPool*)0)->outOfRangeType,
    kMarkerStyleStride = sizeof(float) + sizeof(int),
    kBgmControllerOffset = (int)&((BgmCollectionResourcePool*)0)->controller,
    kBgmLayoutOffset = (int)&((BgmCollectionResourcePool*)0)->layout,
    kBgmMessagesOffset = (int)&((BgmCollectionResourcePool*)0)->messageTable,
    kScaleElementVtableSlot = 0x8c / sizeof(void*)
};
// Word-index setters retain the original int* interface; these match CGolfBase2d.h.
enum GolfHudWordFields { kExtraMeterToggleWord = 23, kActionInputGateWord = 71,
                        kActionVisibilityWord = 72, kLayoutDirtyWord = 87 };

// HUD mode/element map (layout coordinates, not world coordinates):
//   0: no meter group; 1: power meter (backdrops 34/35/36, start 37,
//      interpolated 38, overlay 39, group 3A..42, Pangya zone 43, element 44).
//   2: base elements 0,4..8 + group 09..11, selected marker 1.
//   3: same base + group 12..20, selected marker 2.
//   4: same base + group 21..33, selected marker 3.
// Modes 2..4 have distinct artwork; their game-facing names are unresolved here.
// Cancel 45/46 and confirm 47/48 are paired control/image elements; flag 5
// enables input/animation, value state 2 denotes activation, state 0 is reset.
// Marker endpoints 4A(start)/49(end); marker n (1-based) uses 4C+n. There are
// 21 styles in storage: the loop reads style n, so callers must keep count <=20.
// Fade states: 0 hidden, 1 entering (5 update calls), 2 shown, 3 leaving (7 calls).
// Fade-out holds full opacity until remaining<4; sound plays at remaining==4.
enum GolfDisplayMode { kNoMeterDisplay = 0, kPowerMeterDisplay = 1,
                       kMarkerDisplay2 = 2, kMarkerDisplay3 = 3, kMarkerDisplay4 = 4 };

// ---- merged from golf_base_2d_hud_layout.cpp ----
namespace ns0 {
// Translation unit 0x80316848-0x80316FA0 (30 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

extern "C" {
extern void Layout_Init(void*, int);
extern void Layout_ReadElementRect(void*, int, void*);
extern void Layout_WriteElementRect(void*, int, void*);
extern void Layout_GetElementPosition(void*, int, void*);
extern void Layout_GetElementSize(void*, int, void*);
extern const float kFloat_0_GolfBase2d;
extern float kFloat_1_GolfBase2d;
extern float kFloat_0_5_GolfBase2d;
extern char __vt__11CGolfBase2d[];
extern void CTask_StartActorLoading(void*);
extern void CTask_ProcessActors(void*, float);
extern void CTask_DisplayActorsByPriority(void*);
// Forward round-task actor drawing to the common priority-ordered task renderer.
void CRoundDolTask_DisplayActorsByPriority(void* forwarded0) { CTask_DisplayActorsByPriority(forwarded0); }
// Copy 21 marker offset/color styles into the HUD and request a layout refresh.
// src borrows 21 {float Y-offset, packed color} pairs; returns 0 only for null src.
int CGolfBase2d_vfunc_10(void* self, void* src) {
    if (src == 0) return 0;
    struct Pair { float markerOffsetY; int markerColor; };
    Pair* dst = (Pair*)self; // 14 pairs = +0x70, the first style boundary.
    Pair* in = (Pair*)src;
    for (int i = 0; i < kMarkerStyleCount; ++i) { dst[14 + i].markerOffsetY = in[i].markerOffsetY; dst[14 + i].markerColor = in[i].markerColor; }
    AsCGolfBase2d(self)->layoutDirty = 1;
    return 1;
}

// Copy action-button rectangles and record the center Y coordinate of the marker range.
int CGolfBase2d_vfunc_2(void* self) {
    float markerTop;
    float halfScale;
    float markerHeight;
    unsigned char a[16];
    unsigned char b[8];
    Layout_ReadElementRect(self, kCancelButton, a);
    Layout_WriteElementRect(self, kCancelButtonImage, a);
    Layout_ReadElementRect(self, kConfirmButton, a);
    Layout_WriteElementRect(self, kConfirmButtonImage, a);
    Layout_GetElementPosition(self, kMarkerRangeStart, b);
    AsCGolfBase2d(self)->markerBaseY = *(float*)(b + 4);
    Layout_GetElementSize(self, kMarkerRangeStart, b);
    markerHeight = *(float*)(b + 4);
    halfScale = kFloat_0_5_GolfBase2d;
    markerTop = AsCGolfBase2d(self)->markerBaseY;
    AsCGolfBase2d(self)->layoutReady = 1;
    AsCGolfBase2d(self)->markerBaseY = markerTop + markerHeight * halfScale;
    return 1;
}

// Initialize the golf HUD's markers, action buttons, meter values and fade state.
void* CGolfBase2d_Ctor(void* self) {
    Layout_Init(self, 5);
    float fone;
    float fzero = kFloat_0_GolfBase2d;
    showExtraMeterImage(self) = 0;
    fone = kFloat_1_GolfBase2d;
    AsCGolfBase2d(self)->golfBaseVtable = __vt__11CGolfBase2d;
    AsCGolfBase2d(self)->displayMode = 1;
    AsCGolfBase2d(self)->markersEnabled = 0;
    AsCGolfBase2d(self)->markerIndex = 0;
    AsCGolfBase2d(self)->markerCount = 0;
    AsCGolfBase2d(self)->markerOffsetY0 = fzero; AsCGolfBase2d(self)->markerColor0 = -1;
    AsCGolfBase2d(self)->markerOffsetY1 = fzero; AsCGolfBase2d(self)->markerColor1 = -1;
    AsCGolfBase2d(self)->markerOffsetY2 = fzero; AsCGolfBase2d(self)->markerColor2 = -1;
    AsCGolfBase2d(self)->markerOffsetY3 = fzero; AsCGolfBase2d(self)->markerColor3 = -1;
    AsCGolfBase2d(self)->markerOffsetY4 = fzero; AsCGolfBase2d(self)->markerColor4 = -1;
    AsCGolfBase2d(self)->markerOffsetY5 = fzero; AsCGolfBase2d(self)->markerColor5 = -1;
    AsCGolfBase2d(self)->markerOffsetY6 = fzero; AsCGolfBase2d(self)->markerColor6 = -1;
    AsCGolfBase2d(self)->markerOffsetY7 = fzero; AsCGolfBase2d(self)->markerColor7 = -1;
    AsCGolfBase2d(self)->markerOffsetY8 = fzero; AsCGolfBase2d(self)->markerColor8 = -1;
    AsCGolfBase2d(self)->markerOffsetY9 = fzero; AsCGolfBase2d(self)->markerColor9 = -1;
    AsCGolfBase2d(self)->markerOffsetY10 = fzero; AsCGolfBase2d(self)->markerColor10 = -1;
    AsCGolfBase2d(self)->markerOffsetY11 = fzero; AsCGolfBase2d(self)->markerColor11 = -1;
    AsCGolfBase2d(self)->markerOffsetY12 = fzero; AsCGolfBase2d(self)->markerColor12 = -1;
    AsCGolfBase2d(self)->markerOffsetY13 = fzero; AsCGolfBase2d(self)->markerColor13 = -1;
    AsCGolfBase2d(self)->markerOffsetY14 = fzero; AsCGolfBase2d(self)->markerColor14 = -1;
    AsCGolfBase2d(self)->markerOffsetY15 = fzero; AsCGolfBase2d(self)->markerColor15 = -1;
    AsCGolfBase2d(self)->markerOffsetY16 = fzero; AsCGolfBase2d(self)->markerColor16 = -1;
    AsCGolfBase2d(self)->markerOffsetY17 = fzero; AsCGolfBase2d(self)->markerColor17 = -1;
    AsCGolfBase2d(self)->markerOffsetY18 = fzero; AsCGolfBase2d(self)->markerColor18 = -1;
    AsCGolfBase2d(self)->markerOffsetY19 = fzero; AsCGolfBase2d(self)->markerColor19 = -1;
    AsCGolfBase2d(self)->markerOffsetY20 = fzero; AsCGolfBase2d(self)->markerColor20 = -1;
    confirmButtonEnabled(self) = 0;
    actionButtonsEnabled(self) = 1;
    actionButtonsVisible(self) = 1;
    confirmButtonActivated(self) = 0;
    cancelButtonActivated(self) = 0;
    AsCGolfBase2d(self)->tintColor = -1;
    AsCGolfBase2d(self)->displayValue = fzero;
    AsCGolfBase2d(self)->pangyaTargetX = fzero;
    AsCGolfBase2d(self)->pangyaZoneWidth = fone;
    meterOverlayWidth(self) = fzero;
    AsCGolfBase2d(self)->transitionStartValue = fzero;
    for (int off = 0x144; off <= 0x150; off += 4) *(int*)((char*)self + off) = 0;
    AsCGolfBase2d(self)->fadeOpacity = fzero;
    AsCGolfBase2d(self)->layoutReady = 0;
    AsCGolfBase2d(self)->layoutDirty = 1;
    AsCGolfBase2d(self)->markerBaseY = fzero;
    return self;
}
extern char golfPositionDialogStrings[];
extern int Layout_LoadExtended(void*, int, int);
// Reserved round task preload hook; it performs no work.
void CRoundDolMain_OnPreLoadInit() {}
// Reserved round task initialization hook; it performs no work.
void CRoundDolMain_OnInit() {}

extern unsigned char s_roundDolTaskTypeInfo[1];
// Return borrowed RTTI storage for the round task.
void* CRoundDolTask_GetRTTI() { return s_roundDolTaskTypeInfo; }

extern unsigned char s_roundDolMainTypeInfo[1];
// Return borrowed RTTI storage for the round main object.
void* CRoundDolMain_GetRTTI() { return s_roundDolMainTypeInfo; }

extern void* CRoundDolMain_dtor(void*, int);
// Adjust the +0x18 interface pointer to its complete object before destruction.
void* CRoundDolMain_DtorThunk18(void* p, int flag) { return CRoundDolMain_dtor((char*)p - 0x18, flag); }

extern void Layout_Destroy(void*, int);
extern void __dl__FPv(void*);
// Destroy the HUD layout and optionally free its storage.
void* CGolfBase2d_vfunc_0(void* p, int flag) {
    if (p != 0) {
        Layout_Destroy(p, 0);
        if (flag > 0) __dl__FPv(p);
    }
    return p;
}

extern void* CRoundDolTask_dtor(void*, int);
// Adjust the +8 interface pointer to its complete task before destruction.
void* CRoundDolTask_DtorThunk8(void* p, int flag) { return CRoundDolTask_dtor((char*)p - 0x8, flag); }

// Load the golf-position HUD layout for the requested resource index.
int CGolfBase2d_vfunc_1(void* layout, int arg) {
    Layout_LoadExtended(layout, arg, (int)golfPositionDialogStrings);
    return 1;
}

// Validate and change the HUD display mode, marking the layout dirty only when it changes.
int CGolfBase2d_vfunc_6(int* p, int x) {
    if (x > kMarkerDisplay4 || x < kNoMeterDisplay) return 0;
    if (AsCGolfBase2d(p)->displayMode == x) return 1;
    AsCGolfBase2d(p)->displayMode = x;
    AsCGolfBase2d(p)->layoutDirty = 1;
    return 1;
}

extern float kFloat_6_2831855;
extern float s_twoPi_GolfBase2d;

// Initialize the golf HUD unit's cached full-turn angle.
void CGolfBase2d_InitializeUnitConstants() { s_twoPi_GolfBase2d = kFloat_6_2831855; }

// Return the current HUD display mode.
int CGolfBase2d_vfunc_7(int* p) { return AsCGolfBase2d(p)->displayMode; }
// Enable or disable marker display and refresh the layout when it changes.
int CGolfBase2d_vfunc_8(int* p, int x) {
    if (AsCGolfBase2d(p)->markersEnabled != x) {
        AsCGolfBase2d(p)->markersEnabled = x;
        AsCGolfBase2d(p)->layoutDirty = 1;
    }
    return 1;
}
// Set the selected marker index and marker count, then request a layout refresh.
// markerIndex is zero-based. count must be <=20: style 0 is skipped by the
// one-based drawing loop. This setter stores both values without validation.
int CGolfBase2d_vfunc_9(int* p, int markerIndex, int count) {
    AsCGolfBase2d(p)->markerIndex = markerIndex;
    AsCGolfBase2d(p)->markerCount = count;
    AsCGolfBase2d(p)->layoutDirty = 1;
    return 1;
}
extern void CTask_dtor(void*, int);
extern void __dl__FPv(void*);
// Destroy the round task base and optionally free its storage.
void* CRoundDolTask_dtor(void* p, int flag) {
    if (p != 0) {
        CTask_dtor(p, 0);
        if (flag > 0) __dl__FPv(p);
    }
    return p;
}
// Return the latched confirm action.
int CGolfBase2d_GetConfirmButtonActivated(int* p) { return confirmButtonActivated(p); }
// Return the latched cancel action.
int CGolfBase2d_GetCancelButtonActivated(int* p) { return cancelButtonActivated(p); }

// Show or hide the extra meter image and refresh the layout on a change.
int CGolfBase2d_SetShowExtraMeterImage(int* p, int x) {
    if (p[kExtraMeterToggleWord] == x) return 1;
    p[kExtraMeterToggleWord] = x;
    p[kLayoutDirtyWord] = 1;
    return 1;
}
// Enable or disable action-button input and refresh the layout on a change.
int CGolfBase2d_SetActionButtonsEnabled(int* p, int x) {
    if (p[kActionInputGateWord] == x) return 1;
    p[kActionInputGateWord] = x;
    p[kLayoutDirtyWord] = 1;
    return 1;
}
// Show or hide action buttons and refresh the layout on a change.
int CGolfBase2d_SetActionButtonsVisible(int* p, int x) {
    if (p[kActionVisibilityWord] == x) return 1;
    p[kActionVisibilityWord] = x;
    p[kLayoutDirtyWord] = 1;
    return 1;
}

extern void* g_soundManager;
extern float kFloat_0_5_GolfBase2d;
extern float kFloat_200_GolfBase2d;
extern float kFloat_1_GolfBase2d;
extern void Sound_PlayByName(void*, const char*, int, int, int, int, float, float, float);
// Latch the confirm action when the dialog is visible and play its click sound once.
int CGolfBase2d_ActivateConfirmButton(void* p) {
    if (confirmButtonActivated(p) == 0) {
        if (AsCGolfBase2d(p)->fadeState != kFadeHidden) {
            confirmButtonActivated(p) = 1;
            AsCGolfBase2d(p)->layoutDirty = 1;
            char* __s = (char*)golfPositionDialogStrings + kButtonSoundOffset;
            Sound_PlayByName(g_soundManager, __s, 0, 0, 0, 0, 0.5f, 200.0f, 1.0f);
        }
    }
    return 1;
}
// Latch the cancel action when the dialog is visible and play its click sound once.
int CGolfBase2d_ActivateCancelButton(void* p) {
    if (cancelButtonActivated(p) == 0) {
        if (AsCGolfBase2d(p)->fadeState != kFadeHidden) {
            cancelButtonActivated(p) = 1;
            AsCGolfBase2d(p)->layoutDirty = 1;
            char* __s = (char*)golfPositionDialogStrings + kButtonSoundOffset;
            Sound_PlayByName(g_soundManager, __s, 0, 0, 0, 0, 0.5f, 200.0f, 1.0f);
        }
    }
    return 1;
}

extern int Layout_Update(void*);
extern void Layout_DrawElementWithOpacity(void*, unsigned int, float);
extern void Layout_DrawWithOpacity(void*, float);
extern void BpObject_Draw(void*);
extern void Layout_SetElementPlayerFlag(void*, int, int, int);

struct CGolfBase2dBase { char layoutStorage[0x164]; };
class CGolfBase2dV : public CGolfBase2dBase {
public:
    virtual void v0();
    virtual void v1();
    virtual void v2();
    virtual void v3();
    virtual void v4();
    virtual void v5();
    virtual void v6();
    virtual void v7();
    virtual void v8();
    virtual void v9();
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
    virtual void v20();
    virtual void v21();
    virtual void v22();
    virtual void v23();
    virtual void v24();
    virtual void v25();
    virtual void v26();
    virtual void v27();
    virtual void v28();
    virtual void v29();
    virtual int vfunc80();
    virtual int vfunc84();
    virtual int vfunc88();
};
// Update the base layout, fade timers, activated elements and derived HUD presentation.
int CGolfBase2d_UpdateLayoutAndHudState(void* self) {
    int updated = Layout_Update(self);
    int result = updated & 1;
    CGolfBase2dV* obj = (CGolfBase2dV*)self;
    result &= obj->vfunc80();
    result &= obj->vfunc84();
    result &= obj->vfunc88();
    return result;
}

// Enable the confirm button and clear its activation latch when disabling it.
int CGolfBase2d_SetConfirmButtonEnabled(void* self, int arg) {
    if (AsCGolfBase2d(self)->confirmButtonEnabled == arg) {
        return 1;
    }
    AsCGolfBase2d(self)->confirmButtonEnabled = arg;
    AsCGolfBase2d(self)->layoutDirty = 1;
    Layout_SetElementPlayerFlag(self, 0x47, arg, 5);
    int v = AsCGolfBase2d(self)->confirmButtonEnabled;
    Layout_SetElementPlayerFlag(self, 0x48, v, 5);
    if (AsCGolfBase2d(self)->confirmButtonEnabled == 0) {
        AsCGolfBase2d(self)->confirmButtonActivated = 0;
    }
    return 1;
}

extern float kFloat_1_GolfBase2d;

// Draw the meter backdrop, optional BP object and the complete HUD layout.
int CGolfBase2d_Draw(void* self, void* arg) {
    if (AsCGolfBase2d(self)->displayMode == 1) {
        Layout_DrawElementWithOpacity(self, kMeterBackdrop0, kFloat_1_GolfBase2d);
        Layout_DrawElementWithOpacity(self, kMeterBackdrop1, kFloat_1_GolfBase2d);
        if (showExtraMeterImage(self) != 0) {
            Layout_DrawElementWithOpacity(self, kExtraMeterImage, kFloat_1_GolfBase2d);
        }
    }
    if (arg != 0) {
        BpObject_Draw(arg);
    }
    Layout_DrawWithOpacity(self, kFloat_1_GolfBase2d);
    return 1;
}
// Forward the round task's actor-loading phase to the common task implementation.
void CRoundDolTask_StartActorLoading(void* forwarded0) { CTask_StartActorLoading(forwarded0); }
// Forward actor processing and its unchanged time step to the common task implementation.
void CRoundDolTask_ProcessActors(void* forwarded0, float forwarded1) { CTask_ProcessActors(forwarded0, forwarded1); }

} // extern "C"

} // namespace ns0

// ---- merged from golf_base_2d_hud_layout.cpp ----
namespace ns1 {

// Translation unit 0x80316FA0-0x80318428 (18 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

extern "C" {

extern float kFloat_6_2831855;
extern float s_twoPi_RoundPositionDialog;
extern float kFloat_1_GolfBase2d;
extern const float kFloat_0_GolfBase2d;
extern float kFloat_0_5_GolfBase2d;
extern float kFloat_200_GolfBase2d;
extern float kGolfBase2dTransitionFrameDivisor;
extern float kDialogFadeOutOpacityStep;
extern float kDialogEnabledButtonOpacityScale;
extern float kDialogDisabledButtonOpacityScale;
extern void* g_soundManager;
extern char golfPositionDialogStrings[];
extern char __vt__Q23std12out_of_range[];
extern char __vt__Q23std11logic_error[];
extern char s_sharedCharDeleterVtable[];
extern "C" int Layout_CollectActivated(void*, void*, int);
extern "C" void appFreeUni__FPv(void*);
extern "C" void* __nwa__FUl(unsigned);
extern "C" void* __nw__FUl(unsigned);
extern "C" void __dla__FPv(void*);
extern "C" char* strcpy(char*, const char*);
extern "C" void __dt__Q23std12out_of_rangeFv();
extern "C" void Layout_GetElementPosition(void*, int, void*);
extern "C" void Layout_GetElementSize(void*, int, void*);
extern "C" void Layout_GetElementScale(void*, int, void*);
extern "C" void Layout_SetElementScale(void*, int, void*);
extern "C" void Layout_SetElementEnabled(void*, int, int);
extern "C" void Layout_SetElementColor(void*, int, int);
extern "C" void Layout_SetElementValueState(void*, int, int);
extern "C" void Layout_SetElementPlayerFlag(void*, int, int, int);
extern "C" void Layout_SetElementPosition(void*, int, void*);
extern "C" void Layout_SetElementPlayerFlagsAndState(void*, int, int);
extern "C" void Sound_PlayByName(void*, void*, int, int, int, int, float, float, float);

// Set the packed HUD tint and request a layout refresh.
int CGolfBase2d_SetTintColor(void* self, void* arg) {
    (*(void**)&AsCGolfBase2d(self)->tintColor) = arg;
    AsCGolfBase2d(self)->layoutDirty = 1;
    return 1;
}

// Stop the meter-value transition.
int CGolfBase2d_StopValueTransition(void* self) {
    AsCGolfBase2d(self)->valueTransitionActive = 0;
    return 1;
}

// Report whether the position dialog has finished fading in.
int CGolfBase2d_vfunc_28(void* self) {
    return (*(unsigned int*)&AsCGolfBase2d(self)->fadeState) == 2;
}

// Start the five-frame dialog fade-in and clear both action latches.
int CGolfBase2d_StartFadeInAndClearActionButtons(void* self) {
    float zeroFloat = kFloat_0_GolfBase2d;
    int one = 1;
    int zero = 0;
    int five = kFadeInFrames;
    AsCGolfBase2d(self)->fadeState = one;
    AsCGolfBase2d(self)->fadeFramesRemaining = five;
    AsCGolfBase2d(self)->fadeOpacity = zeroFloat;
    AsCGolfBase2d(self)->layoutDirty = one;
    confirmButtonActivated(self) = zero;
    cancelButtonActivated(self) = zero;
    return 1;
}

// Start the seven-frame dialog fade-out when the dialog is currently visible.
int CGolfBase2d_StartFadeOut(void* self) {
    if (AsCGolfBase2d(self)->fadeState != kFadeHidden) {
        float oneFloat = kFloat_1_GolfBase2d;
        int three = 3;
        int seven = kFadeOutFrames;
        AsCGolfBase2d(self)->fadeState = three;
        AsCGolfBase2d(self)->fadeFramesRemaining = seven;
        AsCGolfBase2d(self)->fadeOpacity = oneFloat;
    }
    return 1;
}

// Report whether neither fade-in nor fade-out is in progress.
int CGolfBase2d_IsFadeSettled(void* self) {
    int value = AsCGolfBase2d(self)->fadeState;
    if (value != 1 && value != 3)
        return 1;
    return 0;
}

// Set the displayed meter value and refresh the layout on a change.
int CGolfBase2d_SetDisplayValue(void* self, float value) {
    if (AsCGolfBase2d(self)->displayValue != value) {
        AsCGolfBase2d(self)->displayValue = value;
        AsCGolfBase2d(self)->layoutDirty = 1;
    }
    return 1;
}

// Set the horizontal target position of the Pangya zone.
int CGolfBase2d_SetPangyaTargetX(void* self, float value) {
    if (AsCGolfBase2d(self)->pangyaTargetX != value) {
        AsCGolfBase2d(self)->pangyaTargetX = value;
        AsCGolfBase2d(self)->layoutDirty = 1;
    }
    return 1;
}

// Set the width scale of the Pangya zone.
int CGolfBase2d_SetPangyaZoneWidth(void* self, float value) {
    if (AsCGolfBase2d(self)->pangyaZoneWidth != value) {
        AsCGolfBase2d(self)->pangyaZoneWidth = value;
        AsCGolfBase2d(self)->layoutDirty = 1;
    }
    return 1;
}

// Set the requested width of the meter overlay and refresh the layout on a change.
int CGolfBase2d_SetMeterOverlayWidth(void* self, float value) {
    if (AsCGolfBase2d(self)->meterOverlayWidth != value) {
        AsCGolfBase2d(self)->meterOverlayWidth = value;
        AsCGolfBase2d(self)->layoutDirty = 1;
    }
    return 1;
}

// Start a five-frame meter transition from the supplied value.
int CGolfBase2d_StartMeterValueTransition(void* self, float value) {
    AsCGolfBase2d(self)->valueTransitionActive = 1;
    AsCGolfBase2d(self)->valueTransitionFramesRemaining = kFadeInFrames;
    AsCGolfBase2d(self)->transitionStartValue = value;
    AsCGolfBase2d(self)->layoutDirty = 1;
    return 1;
}

// Initialize the round-dialog unit's cached full-turn angle.
void RoundPositionDialog_InitializeUnitConstants() {
    s_twoPi_RoundPositionDialog = kFloat_6_2831855;
}

// Make a hidden dialog fully visible immediately and clear action latches.
int CGolfBase2d_ShowImmediately(void* self) {
    if (AsCGolfBase2d(self)->fadeState == kFadeHidden) {
        float oneFloat = kFloat_1_GolfBase2d;
        int zero = 0;
        int two = 2;
        int one = 1;
        AsCGolfBase2d(self)->fadeFramesRemaining = zero;
        AsCGolfBase2d(self)->fadeOpacity = oneFloat;
        AsCGolfBase2d(self)->fadeState = two;
        AsCGolfBase2d(self)->layoutDirty = one;
        confirmButtonActivated(self) = zero;
        cancelButtonActivated(self) = zero;
    }
    return 1;
}

// Scale an element horizontally to reach the supplied X coordinate, clamping below zero.
// ratio = max(0, (targetX - authoredX) / authoredWidth), or 0 for zero width.
// Preserve the current Y scale. Inputs are layout units, not normalized meter values.
int CGolfBase2d_ScaleElementToX(void* self, int index, float arg) {
    float buf[2];
    float start;
    float ratio = kFloat_0_GolfBase2d;
    Layout_GetElementPosition(self, index, buf);
    start = buf[0];
    Layout_GetElementSize(self, index, buf);
    float span = buf[0];
    if (kFloat_0_GolfBase2d != span) {
        ratio = (arg - start) / span;
    }
    if (ratio < kFloat_0_GolfBase2d) {
        ratio = kFloat_0_GolfBase2d;
    }
    Layout_GetElementScale(self, index, buf);
    buf[0] = ratio;
    Layout_SetElementScale(self, index, buf);
    return 1;
}

// Advance dialog fades and meter transitions, playing the close sound at its designated frame.
int CGolfBase2d_UpdateFadeAndMeterTransition(void* self) {
    // Phase 1: advance opacity and close-sound timing by one update, not elapsed seconds.
    int fadeFrames = AsCGolfBase2d(self)->fadeFramesRemaining;
    if (fadeFrames != 0) {
        int remainingFrames = fadeFrames - 1;
        int fadeState = AsCGolfBase2d(self)->fadeState;
        AsCGolfBase2d(self)->fadeFramesRemaining = remainingFrames;
        AsCGolfBase2d(self)->layoutDirty = 1;
        if (fadeState == kFadeIn) {
            float f0 = (float)(kFadeInFrames - remainingFrames) / kGolfBase2dTransitionFrameDivisor;
            AsCGolfBase2d(self)->fadeOpacity = f0;
            if (remainingFrames == 0) {
                AsCGolfBase2d(self)->fadeState = kFadeVisible;
            }
        } else if (fadeState == kFadeOut) {
            if (remainingFrames < 4) {
                float f0 = (float)remainingFrames * kDialogFadeOutOpacityStep; // 0.25
                AsCGolfBase2d(self)->fadeOpacity = f0;
            }
            if (AsCGolfBase2d(self)->fadeFramesRemaining == 4) {
                void* p = g_soundManager;
                void* q = (void*)(golfPositionDialogStrings + kFadeSoundOffset);
                Sound_PlayByName(p, q, 0, 0, 0, 0, kFloat_0_5_GolfBase2d, kFloat_200_GolfBase2d, kFloat_1_GolfBase2d);
            }
            if (AsCGolfBase2d(self)->fadeFramesRemaining == 0) {
                AsCGolfBase2d(self)->fadeState = kFadeHidden;
                confirmButtonActivated(self) = 0;
                cancelButtonActivated(self) = 0;
                AsCGolfBase2d(self)->layoutDirty = 1;
            }
        }
    }
    // Phase 2: advance the separate interpolation countdown; active stays set
    // until vfunc_21 explicitly clears it, even when the countdown reaches zero.
    int transitionFrames = AsCGolfBase2d(self)->valueTransitionFramesRemaining;
    if (transitionFrames != 0) {
        AsCGolfBase2d(self)->layoutDirty = 1;
        AsCGolfBase2d(self)->valueTransitionFramesRemaining = transitionFrames - 1;
    }
    return 1;
}

struct Ref_8031727C { int vt; int c1; int c2; char* s; };
struct ErrBase_8031727C {
    void* vt; char* buf; Ref_8031727C* ref;
    // Copy a diagnostic into the original shared-string exception representation.
    ErrBase_8031727C(const char* msg) {
        char done = 0;
        vt = __vt__Q23std11logic_error;
        const char* m = msg;
        unsigned n = strlen(m) + 1;
        unsigned bytes = (n << 1) - n;
        char* b = (char*)__nwa__FUl(bytes);
        buf = b;
        {
            struct Holder {
                char* p; char* flag;
                // Protect text until construction of its shared owner succeeds.
                Holder(char* pp, char* f) { p = pp; flag = f; }
                // Release text only while ownership has not been transferred.
                ~Holder() { if (p) __dla__FPv(p); }
            } h(b, &done);
            Ref_8031727C* r = (Ref_8031727C*)__nw__FUl(16);
            if (r) { int table = (int)s_sharedCharDeleterVtable; r->c1 = 1; r->c2 = 1; r->vt = table; r->s = b; }
            ref = r;
            h.p = 0;
        }
        strcpy(buf, m);
    }
    // Local wrapper has no extra cleanup beyond the runtime exception representation.
    ~ErrBase_8031727C() {}
};
struct Err_8031727C : ErrBase_8031727C {
    // Complete the out_of_range object and throw it through the original runtime.
    Err_8031727C(const char* msg) : ErrBase_8031727C(msg) { vt = __vt__Q23std12out_of_range; __throw((char*)golfPositionDialogStrings + kRangeErrorTypeOffset, this, (void*)__dt__Q23std12out_of_rangeFv); }
    // Empty local destructor preserves the original unwind structure.
    ~Err_8031727C() {}
};

#pragma opt_common_subs off
#pragma opt_strength_reduction off
#pragma use_lmw_stmw on
// Read activated layout elements and latch enabled confirm or cancel actions.
int CGolfBase2d_ProcessActivatedActionButtons(void* self) {
    if (AsCGolfBase2d(self)->fadeState != kFadeVisible) {
        return 0;
    }
    int zeroReg = 0;
    unsigned activationByteOffset;
    unsigned activationIndex;
    int activated[3];
    activated[0] = zeroReg;
    activated[1] = zeroReg;
    activated[2] = zeroReg;
    Layout_CollectActivated(self, activated, zeroReg);
    // CollectActivated returns an owned {data, size, capacity} array of element IDs.
    activationByteOffset = zeroReg;
    activationIndex = zeroReg;
    for (activationIndex = 0; activationIndex < (unsigned)activated[1]; ++activationIndex, activationByteOffset += 4) {
        if ((unsigned)activationIndex >= (unsigned)activated[1]) {
            Err_8031727C e((char*)golfPositionDialogStrings + kRangeErrorTextOffset);
        }
        int value = *(int*)((char*)activated[0] + activationByteOffset);
        if (value == kConfirmButton) {
            if (confirmButtonEnabled(self) != 0) {
                confirmButtonActivated(self) = 1;
                AsCGolfBase2d(self)->layoutDirty = 1;
            }
        }
        if ((unsigned)activationIndex >= (unsigned)activated[1]) {
            Err_8031727C e((char*)golfPositionDialogStrings + kRangeErrorTextOffset);
        }
        if (*(int*)((char*)activated[0] + activationByteOffset) == kCancelButton) {
            cancelButtonActivated(self) = 1;
            AsCGolfBase2d(self)->layoutDirty = 1;
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

struct Allocator_80317EF0 {};

// Load the BGM collection resources, snapshot the save record and initialize the menu controller.
extern "C" void* CBgmCollection_Ctor(void* self) {
    extern void MPBase1_Ctor(void*);
    extern void* SaveData_InitPlayerRecord(void*, void*);
    extern void* ResourceTable_FindEntryByName(void*, char*, void*);
    extern void* __nw__FUl(unsigned);
    extern void* CPackTextureResource_Init(void*, char*, int, void*);
    extern void* Layout_Init(void*, int);
    extern void* String_ReplaceRange(void*, int, int, const char*, const char*, const Allocator_80317EF0&);
    extern void MPBase1_SetAutoStart(void*, int);
    extern void SoundManager_RequestBgmStop(void*, int);
    extern void SaveData_Ctor(void*);
    extern void __ct__7CBpCtrlFv(void*);
    extern void CBpCtrl_dtor(void*);
    extern void CBpCtrl_CreateEntry(void*, char*, void*, int);
    extern void Layout_LoadExtended(void*, int, char*);
    extern void WFixedFont_CreateMsgEcbResource(void*, void*, int, int);
    extern void WFixedFont_CreateAndRegisterMsgDraw(void*, void*, int);
    extern void* CBpTex_Init(void*);
    extern void* SharedFile_Create(const char*, int);
    extern void* SaveData_GetCurrentRecord(void*);
    extern void* g_soundManager;
    extern char bgmCollectionStrings[];
    extern char __vt__14CBgmCollection[];
    extern char s_saveData[];
    extern char s_bpCtrl[];
    extern signed char s_resourceTableInitialized;
    extern void* s_resourceTableStorage;
    extern signed char s_saveDataInitialized;
    extern signed char s_bpCtrlInitialized;
    extern void* g_chatMsg;
    extern char s_resourceTableDtorRegistration_BgmCollection[];
    extern char s_bpCtrlDtorRegistration_80316848[];

    struct Resource {
        char data[0x20];
        // Wrap the resource-table archive bytes in an owned texture-resource object.
        Resource(char* name, int size, void* data) { CPackTextureResource_Init(this, name, size, data); }
    };
    struct Controller {
        char data[0x5C];
        // Initialize the layout owner for the given resource count.
        Controller(int count) { Layout_Init(this, count); }
    };
    struct State {
        char data[0x28];
        // Initialize the owned BP texture state.
        State() { CBpTex_Init(this); }
    };
    char* selfc = (char*)self;
    int resourceSize;
    Allocator_80317EF0 argument;
    Allocator_80317EF0 allocator;
    // Phase 1: construct base/string/save storage, then load texture and layout owners.
    MPBase1_Ctor(self);
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
            *(void**)( (char*)&s_resourceTableStorage ) = 0;
            extern void Allocation_Dtor(void*);
            __register_global_object(&s_resourceTableStorage, (void*)Allocation_Dtor, s_resourceTableDtorRegistration_BgmCollection);
            s_resourceTableInitialized = 1;
        }
        Sing_805DC620() = &s_resourceTableStorage;
    }
    void* r29;
    void* r28 = ResourceTable_FindEntryByName(Sing_805DC620(), bgmCollectionStrings, &resourceSize);
    r29 = new Resource(bgmCollectionStrings, resourceSize, r28);
    ((CBgmCollection*)selfc)->textureResource = r29;
    ((CBgmCollection*)selfc)->bpEntryId = -1;
    (*(int*)&((CBgmCollection*)selfc)->trackTable) = 0;
    (*(int*)&((CBgmCollection*)selfc)->messageResource) = 0;
    (*(int*)&((CBgmCollection*)selfc)->chatResource) = 0;
    (*(int*)&((CBgmCollection*)selfc)->messageDraw) = 0;
    (*(int*)&((CBgmCollection*)selfc)->textureState) = 0;
    ((CBgmCollection*)selfc)->selectedTrackIndex = 0;
    r29 = new Controller(5);
    ((CBgmCollection*)selfc)->layout = r29;
    ((CBgmCollection*)selfc)->finished = 0;
    ((CBgmCollection*)selfc)->buttonDelayActive = 0;
    ((CBgmCollection*)selfc)->buttonDelayUpdateCount = 0;
    *(unsigned char*)&argument = *(volatile unsigned char*)&allocator;
    String_ReplaceRange(selfc + kBgmStringOffset, 0, -1, 0, 0, argument);
    ((CBgmCollection*)selfc)->inputInitialized = 0;
    ((CBgmCollection*)selfc)->exitRequested = 0;
    MPBase1_SetAutoStart(self, 1);
    SoundManager_RequestBgmStop(g_soundManager, 1);
    if (Sing_805DC5E0() == 0) {
        if (!s_saveDataInitialized) {
            SaveData_Ctor(s_saveData);
            s_saveDataInitialized = 1;
        }
        Sing_805DC5E0() = (void*)s_saveData;
    }
    // Phase 2: snapshot persisted save regions, retaining the original copy granularity.
    void* src = SaveData_GetCurrentRecord(Sing_805DC5E0());
    struct Data {
        // Boundaries verified against save_data_records_and_defaults.cpp::PlayerRecordView. Keep the
        // original scalar widths: changing these groups changes generated copies.
        unsigned headerAndInventoryWords[151]; // +000..25B
        unsigned courseRecordWords[28];        // +25C..2CB
        unsigned short characterEquipmentHalfwords[208]; // +2CC..46B
        unsigned caddieEquipmentWords[16];     // +46C..4AB
        unsigned char clubSetPrefixBytes[24];  // +4AC..4C3
        unsigned char clubSetByte24, clubSetByte25, clubSetByte26;
        unsigned char clubSetByte27, clubSetByte28, clubSetByte29;
        unsigned char ballPrefixBytes[8];      // +4CA..4D1
        unsigned char ballByte8, ballByte9, ballByte10; // then 3 implicit padding bytes
        unsigned systemSettingsWords[5];       // +4D8..4EB
        unsigned short modeRecordHalfwords[414]; // +4EC..827
    };
    *(Data*)(selfc + kBgmPlayerRecordOffset) = *(Data*)src;
    if (Sing_805DC5F0() == 0) {
        if (!s_bpCtrlInitialized) {
            __ct__7CBpCtrlFv(s_bpCtrl);
            __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, s_bpCtrlDtorRegistration_80316848);
            s_bpCtrlInitialized = 1;
        }
        Sing_805DC5F0() = (void*)s_bpCtrl;
    }
    // Phase 3: register the BP controller, borrow chat resources, create text/texture state.
    CBpCtrl_CreateEntry(Sing_805DC5F0(), bgmCollectionStrings + kBgmControllerOffset, selfc+0x18, 0);
    Layout_LoadExtended(((CBgmCollection*)selfc)->layout, 0, bgmCollectionStrings + kBgmLayoutOffset);
    void* r3 = g_chatMsg;
    void* r3p = ((CChatMsg*)r3)->defaultFixedFont;
    ((CBgmCollection*)selfc)->chatResource = r3p;
    WFixedFont_CreateMsgEcbResource(r3p, selfc+0x24, 0, 0);
    WFixedFont_CreateAndRegisterMsgDraw(*(void**)(selfc+0x28), selfc+0x2C, 1);
    r29 = new State;
    ((CBgmCollection*)selfc)->textureState = r29;
    void* h = SharedFile_Create(bgmCollectionStrings + kBgmMessagesOffset, 0);
    ((CBgmCollection*)selfc)->sharedFile = h;
    return self;
}

struct GolfMarkerStyle { float offsetY; int packedColor; };
// styleZero is authored storage only; visible marker n consumes style n, n=1..20.
// Preserve a byte cursor's original base while exposing only its style boundary.
struct GolfMarkerStyleCursor { char layoutPrefix[0x70]; GolfMarkerStyle style; };
enum GolfHudButtonState { kButtonIdle = 0, kButtonActivated = 2 };
enum GolfHudArtwork {
    kPowerArtworkFirst = 0x3a, kPowerArtworkLast = 0x42,
    kMode2ArtworkFirst = 0x09, kMode2ArtworkLast = 0x11,
    kMode3ArtworkFirst = 0x12, kMode3ArtworkLast = 0x20,
    kMode4ArtworkFirst = 0x21, kMode4ArtworkLast = 0x33,
    kPowerMeterEndImage = 0x44
};

#pragma opt_propagation off
#pragma push
#pragma always_inline on
// Refresh action-button opacity, input gates and latched value states.
// Scratch references borrow caller locals so the inline split keeps float register allocation.
static inline void RefreshGolfHudButtons(void* self, char* hud, int& packedButtonColor,
        float& enabledScale, float& enabledOpacity, float& disabledScale, float& disabledOpacity) {
    // Phase 2: button visibility, opacity and input gates. Colors use rotated packed
    // bytes: opacityByte-256 yields white RGB with the desired low alpha byte.
    enabledScale = kDialogEnabledButtonOpacityScale;
    enabledOpacity = AsCGolfBase2d(hud)->fadeOpacity;
    enabledOpacity = enabledScale * enabledOpacity;
    int opacityByte = (int)enabledOpacity;
    if (opacityByte > 0xff) opacityByte = 0xff;
    packedButtonColor = opacityByte - 0x100;
    Layout_SetElementColor(self, kCancelButton, packedButtonColor);
    Layout_SetElementColor(self, kCancelButtonImage, packedButtonColor);
    Layout_SetElementColor(self, kConfirmButton, packedButtonColor);
    Layout_SetElementColor(self, kConfirmButtonImage, packedButtonColor);
    if (cancelButtonActivated(hud) != 0) {
        Layout_SetElementPlayerFlag(self, kCancelButton, 0, kButtonAnimationFlag);
        Layout_SetElementPlayerFlag(self, kCancelButtonImage, 0, kButtonAnimationFlag);
        Layout_SetElementValueState(self, kCancelButton, kButtonActivated);
        Layout_SetElementValueState(self, kCancelButtonImage, kButtonActivated);
    } else if (actionButtonsEnabled(hud) != 0) {
        Layout_SetElementPlayerFlag(self, kCancelButton, 1, kButtonAnimationFlag);
        Layout_SetElementPlayerFlag(self, kCancelButtonImage, 1, kButtonAnimationFlag);
        Layout_SetElementValueState(self, kCancelButton, kButtonIdle);
        Layout_SetElementValueState(self, kCancelButtonImage, kButtonIdle);
    } else {
        Layout_SetElementPlayerFlag(self, kCancelButton, 0, kButtonAnimationFlag);
        Layout_SetElementPlayerFlag(self, kCancelButtonImage, 0, kButtonAnimationFlag);
    }
    if (confirmButtonActivated(hud) != 0) {
        Layout_SetElementPlayerFlag(self, kConfirmButton, 0, kButtonAnimationFlag);
        Layout_SetElementPlayerFlag(self, kConfirmButtonImage, 0, kButtonAnimationFlag);
        Layout_SetElementValueState(self, kConfirmButton, kButtonActivated);
        Layout_SetElementValueState(self, kConfirmButtonImage, kButtonActivated);
    } else {
        if (confirmButtonEnabled(hud) != 0 && actionButtonsEnabled(hud) != 0) {
            Layout_SetElementPlayerFlag(self, kConfirmButton, 1, kButtonAnimationFlag);
            Layout_SetElementPlayerFlag(self, kConfirmButtonImage, 1, kButtonAnimationFlag);
        } else {
            Layout_SetElementPlayerFlag(self, kConfirmButton, 0, kButtonAnimationFlag);
            Layout_SetElementPlayerFlag(self, kConfirmButtonImage, 0, kButtonAnimationFlag);
            if (confirmButtonEnabled(hud) == 0) {
                disabledScale = kDialogDisabledButtonOpacityScale;
                disabledOpacity = AsCGolfBase2d(hud)->fadeOpacity;
                disabledOpacity = disabledScale * disabledOpacity;
                int disabledOpacityByte = (int)disabledOpacity;
                if (disabledOpacityByte > 0xff) disabledOpacityByte = 0xff;
                int disabledButtonColor = disabledOpacityByte - 0x100;
                Layout_SetElementColor(self, kConfirmButton, disabledButtonColor);
                Layout_SetElementColor(self, kConfirmButtonImage, disabledButtonColor);
            }
        }
    }
    Layout_SetElementValueState(self, kConfirmButton, kButtonIdle);
    Layout_SetElementValueState(self, kConfirmButtonImage, kButtonIdle);
}
#pragma pop

// Rebuild dirty HUD elements from meter, dialog-button, marker and fade state.
int CGolfBase2d_RebuildDirtyLayout(void* self) {
    char* s = (char*)self;
    if (AsCGolfBase2d(s)->layoutReady == 0) return 0;
    if (AsCGolfBase2d(s)->layoutDirty == 0) return 0;
    AsCGolfBase2d(s)->layoutDirty = 0;
    // Phase 1: reset all element state/tint; subsequent phases selectively reveal groups.
    int elementOrColor = 0;
    for (elementOrColor = 0; elementOrColor < kGolfLayoutElementCount; elementOrColor++) {
        Layout_SetElementPlayerFlagsAndState(self, elementOrColor, 0);
        int tintColor = AsCGolfBase2d(s)->tintColor;
        Layout_SetElementColor(self, elementOrColor, tintColor);
    }
    Layout_SetElementColor(self, kCancelButton, -1);
    Layout_SetElementColor(self, kCancelButtonImage, -1);
    Layout_SetElementColor(self, kConfirmButton, -1);
    Layout_SetElementColor(self, kConfirmButtonImage, -1);
    if (AsCGolfBase2d(s)->fadeState != kFadeHidden && actionButtonsVisible(s) != 0) {
        Layout_SetElementEnabled(self, kCancelButton, 1);
        Layout_SetElementEnabled(self, kCancelButtonImage, 1);
        Layout_SetElementEnabled(self, kConfirmButton, 1);
        Layout_SetElementEnabled(self, kConfirmButtonImage, 1);
    }
    // Phase 2: refresh button colors and gates using caller-owned scalar scratch.
    float buttonScale, buttonOpacity, disabledScale, disabledOpacity;
    RefreshGolfHudButtons(self, s, elementOrColor,
        buttonScale, buttonOpacity, disabledScale, disabledOpacity);
    // Phase 3: display-mode artwork and meter width. Interpolation is
    // target + (start-target)*remaining/5, in authored layout X coordinates.
    int displayMode = AsCGolfBase2d(s)->displayMode;
    if (displayMode == kPowerMeterDisplay) {
        Layout_SetElementEnabled(self, kPowerMeterEndImage, 1);
        Layout_SetElementEnabled(self, kInterpolatedMeter, 1);
        if (AsCGolfBase2d(s)->valueTransitionActive != 0) {
            Layout_SetElementEnabled(self, kTransitionStartMeter, 1);
            void* vt = AsCGolfBase2d(s)->golfBaseVtable;
            float startValue = AsCGolfBase2d(s)->transitionStartValue;
            ((void(*)(void*, int, float))(((void**)vt)[kScaleElementVtableSlot]))(self, kTransitionStartMeter, startValue);
            int transitionFrames = AsCGolfBase2d(s)->valueTransitionFramesRemaining;
            void* vt2 = AsCGolfBase2d(s)->golfBaseVtable;
            float interpolatedValue = AsCGolfBase2d(s)->transitionStartValue;
            float framesAsFloat = (float)(int)transitionFrames;
            float targetValue = AsCGolfBase2d(s)->displayValue;
            float weightedDifference = interpolatedValue - targetValue;
            weightedDifference = framesAsFloat * weightedDifference;
            weightedDifference = weightedDifference / kGolfBase2dTransitionFrameDivisor;
            interpolatedValue = targetValue + weightedDifference;
            ((void(*)(void*, int, float))(((void**)vt2)[kScaleElementVtableSlot]))(self, kInterpolatedMeter, interpolatedValue);
        } else {
            void* vt = AsCGolfBase2d(s)->golfBaseVtable;
            float meterValue = AsCGolfBase2d(s)->displayValue;
            ((void(*)(void*, int, float))(((void**)vt)[kScaleElementVtableSlot]))(self, kInterpolatedMeter, meterValue);
        }
    } else if (displayMode != 0) {
        Layout_SetElementEnabled(self, 0, 1);
        Layout_SetElementEnabled(self, 5, 1);
        Layout_SetElementEnabled(self, 6, 1);
        Layout_SetElementEnabled(self, 7, 1);
        Layout_SetElementEnabled(self, 8, 1);
        Layout_SetElementEnabled(self, 4, 1);
        void* vt = AsCGolfBase2d(s)->golfBaseVtable;
        float alternateMeterValue = AsCGolfBase2d(s)->displayValue;
        ((void(*)(void*, int, float))(((void**)vt)[kScaleElementVtableSlot]))(self, 4, alternateMeterValue);
    }
    displayMode = AsCGolfBase2d(s)->displayMode;
    switch (displayMode) {
        case kPowerMeterDisplay: {
            int groupElement = kPowerArtworkFirst;
            while (groupElement <= kPowerArtworkLast) {
                Layout_SetElementEnabled(self, groupElement, 1);
                groupElement++;
            }
            break;
        }
        case kMarkerDisplay2: {
            int groupElement = kMode2ArtworkFirst;
            while (groupElement <= kMode2ArtworkLast) {
                Layout_SetElementEnabled(self, groupElement, 1);
                groupElement++;
            }
            break;
        }
        case kMarkerDisplay3: {
            int groupElement = kMode3ArtworkFirst;
            while (groupElement <= kMode3ArtworkLast) {
                Layout_SetElementEnabled(self, groupElement, 1);
                groupElement++;
            }
            break;
        }
        case kMarkerDisplay4: {
            int groupElement = kMode4ArtworkFirst;
            while (groupElement <= kMode4ArtworkLast) {
                Layout_SetElementEnabled(self, groupElement, 1);
                groupElement++;
            }
            break;
        }
    }
    displayMode = AsCGolfBase2d(s)->displayMode;
    if (displayMode == 1) {
        Layout_SetElementEnabled(self, kPangyaZone, 1);
        Layout_SetElementEnabled(self, kMeterOverlay, 1);
        float overlayScale = kFloat_1_GolfBase2d;
        float buf0[2];
        Layout_GetElementPosition(self, kPangyaZone, buf0);
        float targetX = AsCGolfBase2d(s)->pangyaTargetX;
        buf0[0] = targetX;
        Layout_SetElementPosition(self, kPangyaZone, buf0);
        Layout_GetElementScale(self, kPangyaZone, buf0);
        float targetWidth = AsCGolfBase2d(s)->pangyaZoneWidth;
        buf0[0] = targetWidth;
        Layout_SetElementScale(self, kPangyaZone, buf0);
        Layout_GetElementSize(self, kMeterOverlay, buf0);
        if (kFloat_0_GolfBase2d != buf0[0]) {
            float overlayWidth = meterOverlayWidth(s);
            overlayScale = overlayWidth / buf0[0];
        }
        Layout_GetElementScale(self, kMeterOverlay, buf0);
        buf0[0] = overlayScale;
        Layout_SetElementScale(self, kMeterOverlay, buf0);
    }
    // Phase 4: selected marker at start+index*spacing; style n at
    // start+(n-0.5)*spacing-1, Y=markerBaseY+styleOffset. Colors combine
    // the owner's tint with each style, then rotate the packed byte order.
    if (AsCGolfBase2d(s)->markersEnabled != 0) {
        int markerIndex = AsCGolfBase2d(s)->markerIndex;
        int markerCount = AsCGolfBase2d(s)->markerCount;
        if (markerCount > markerIndex) {
            float bufA[2];
            Layout_GetElementPosition(self, kMarkerRangeStart, bufA);
            float markerStartX = bufA[0];
            Layout_GetElementPosition(self, kMarkerRangeEnd, bufA);
            float markerEndX = bufA[0];
            displayMode = AsCGolfBase2d(s)->displayMode;
            unsigned int markerCountUnsigned = (*(unsigned int*)&AsCGolfBase2d(s)->markerCount);
            float markerSpan = markerEndX - markerStartX;
            float markerCountAsFloat = (float)(int)markerCountUnsigned;
            float markerSpacing = markerSpan / markerCountAsFloat;
            switch (displayMode) {
                case 2: {
                    Layout_SetElementEnabled(self, 1, 1);
                    float bufB[2];
                    Layout_GetElementPosition(self, 1, bufB);
                    int selectedMarkerIndex = AsCGolfBase2d(s)->markerIndex;
                    float markerX = (float)(int)selectedMarkerIndex;
                    markerX = markerSpacing * markerX + markerStartX;
                    bufB[0] = markerX;
                    Layout_SetElementPosition(self, 1, bufB);
                    break;
                }
                case 3: {
                    Layout_SetElementEnabled(self, 2, 1);
                    float bufB[2];
                    Layout_GetElementPosition(self, 2, bufB);
                    int selectedMarkerIndex = AsCGolfBase2d(s)->markerIndex;
                    float markerX = (float)(int)selectedMarkerIndex;
                    markerX = markerSpacing * markerX + markerStartX;
                    bufB[0] = markerX;
                    Layout_SetElementPosition(self, 2, bufB);
                    break;
                }
                case 4: {
                    Layout_SetElementEnabled(self, 3, 1);
                    float bufB[2];
                    Layout_GetElementPosition(self, 3, bufB);
                    int selectedMarkerIndex = AsCGolfBase2d(s)->markerIndex;
                    float markerX = (float)(int)selectedMarkerIndex;
                    markerX = markerSpacing * markerX + markerStartX;
                    bufB[0] = markerX;
                    Layout_SetElementPosition(self, 3, bufB);
                    break;
                }
            }
            displayMode = AsCGolfBase2d(s)->displayMode;
            if (displayMode > 1) {
                int markerTint = AsCGolfBase2d(s)->tintColor;
                char* styleCursor = (char*)self + kMarkerStyleStride;
                float halfMarker = kFloat_0_5_GolfBase2d;
                unsigned int packedTint = ((unsigned int)markerTint << 24) | 0x00FFFFFFu;
                float markerInset = kFloat_1_GolfBase2d;
                int markerNumber = 1;
                while (markerNumber < AsCGolfBase2d(s)->markerCount + 1) {
                    unsigned int unusedConvertedIndex = (unsigned int)markerNumber ^ 0x8000u;
                    int styleColor = ((GolfMarkerStyleCursor*)styleCursor)->style.packedColor;
                    unsigned int tintedColor = packedTint & (unsigned int)styleColor;
                    unsigned int rotatedColor = (tintedColor << 8) | (tintedColor >> 24);
                    float styleX = (float)(int)markerNumber;
                    styleX = styleX - halfMarker;
                    styleX = markerSpacing * styleX + markerStartX;
                    styleX = styleX - markerInset;
                    float bufC[2];
                    bufC[0] = styleX;
                    float markerBaseY = AsCGolfBase2d(s)->markerBaseY;
                    float styleY = ((GolfMarkerStyleCursor*)styleCursor)->style.offsetY;
                    styleY = markerBaseY + styleY;
                    bufC[1] = styleY;
                    Layout_SetElementPosition(self, markerNumber + kMarkerElementBase, bufC);
                    Layout_SetElementColor(self, markerNumber + kMarkerElementBase, (int)rotatedColor);
                    Layout_SetElementEnabled(self, markerNumber + kMarkerElementBase, 1);
                    styleCursor += kMarkerStyleStride;
                    markerNumber++;
                }
            }
        }
    }
    // Phase 5: a hidden dialog cannot accept input, including on revealed meter elements.
    if (AsCGolfBase2d(s)->fadeState == kFadeHidden && AsCGolfBase2d(s)->layoutReady != 0) {
        for (int i = 0; i < kGolfLayoutElementCount; i++) {
            Layout_SetElementPlayerFlag(self, i, 0, 5);
        }
    }
    return 1;
}
#pragma opt_propagation reset
 
// placeholder fn
extern "C" void Allocation_Dtor(void*);

} // extern "C"

} // namespace ns1

extern "C" { void CGolfBase2d_InitializeUnitConstants(); void RoundPositionDialog_InitializeUnitConstants(); }
/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU804625C8", _ctors_u80316848_0, CGolfBase2d_InitializeUnitConstants);
STATIC_INIT_ENTRY(".CRT$XCU804625C8", _ctors_u80316848_1, RoundPositionDialog_InitializeUnitConstants);
#pragma pop

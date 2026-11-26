// Map camera panel: the camera controls and mode indicators on the map screen, with their open
// and close fades, action input and per-frame element updates.
// Also has the stroke-point hit tests, the hover help text, and the option dialog constructor.

#include <decomp/static_init.h>
#include <decomp/ct_member.h>
extern "C" {
extern char s_bpCtrlDtorRegistration_80519F20[];
extern char s_bpCtrlHandleDtorRegistration_OptionMenu[];

extern unsigned char strokePointCharacterIconRows[];
}
namespace exception_runtime {
extern "C" {
void __throw(const char*, void*, void*) __attribute__((noreturn));
void __dt__Q23std12out_of_rangeFv(void*, int);
extern const char mapCameraDialogStrings[];
}
}

#include "game/singletons.h"
#include "classes/COptionDialog.h"
#include "classes/CStrokePoint2d.h"
#include "classes/CMapCamera2d.h"
#include "classes/CBpCtrl.h"
#include "classes/CPackTextureResource.h"

#define AsCOptionDialog(p) ((COptionDialog*)(p))
// Panel visibility is independent of the selected camera mode (slots 1..4).
enum MapCameraVisibility {
    kCameraPanelHidden = 0,
    kCameraPanelOpening = 1,
    kCameraPanelOpen = 2,
    kCameraPanelClosing = 3
};
enum {
    kCameraPanelOpenFrames = 5,
    kCameraPanelCloseFrames = 7,
    kCameraPanelFadeOutFrames = 3,
    kCameraPanelCloseSoundFrame = 4,
    kCameraActionFeedbackFrames = 3,
    kCameraPanelElementCount = 12,
    kStrokeSelectionCount = 8,
    kPointerActivateAction = 5,
    kAllPlayers = -1
};
// Layout map: each action has a display/hit pair whose rectangles are aligned
// by AlignActionHitAreas. Exact icon meanings remain unknown; numbering is not a camera mode.
enum MapCameraElementId {
    kCameraControl0 = 0,
    kCameraControl1 = 1,
    kCameraForcedControl2 = 2,
    kCameraForcedControl3 = 3,
    kCameraFirstActionDisplay = 4,
    kCameraFirstActionHit = 5,
    kCameraSecondActionDisplay = 6,
    kCameraSecondActionHit = 7,
    kCameraModeIndicator1 = 8,
    kCameraModeIndicator2 = 9,
    kCameraModeIndicator3 = 10,
    kCameraModeIndicator4 = 11
};
// Vtable guide (symbol names kept as-is):
// 4 draw; 5 visible/forced; 6 read first latch; 7 latch first action;
// 8 read second one-frame request; 9 queue second feedback; 10 collect activation;
// 11 request edge indicators; 12 select mode highlight; 13 request controls;
// 14 force visibility; 15 request input; 16 begin opening; 17 begin closing;
// 18 transition idle; 19 fully open; 20 advance fade; 21 consume actions;
// 22 apply element state. Request flags 11/13/15 are consumed each update and
// must be refreshed by callers. First latch survives until reopening/closing;
// second latch lasts three updates while its request pulse clears each update.
// Mode values 1..4 select indicator elements 8..11; their camera roles are unresolved.

// Three eight-word tables share one pool; +0x20 is not used by selection here.
struct StrokePointHelpTableView {
    int elementIds[kStrokeSelectionCount];
    int unresolved20[kStrokeSelectionCount];
    int defaultMessageIds[kStrokeSelectionCount];
    int alternateMessageIds[kStrokeSelectionCount];
};
enum OptionDialogStringOffset {
    kOptionTexturePackName = 0,
    kOptionExtendedLayoutName = 0x0B,
    kOptionBasicLayoutName = 0x1A,
    kOptionExtendedScriptName = 0x28,
    kOptionBasicScriptName = 0x37
};
// Packed string pool; field sizes include each terminating NUL.
struct MapCameraStrings {
    char layout[0x11];
    char clickSound[0x0E];
    char indexRangeError[0x1E];
    char rangeErrorType[0x37];
    char closeSound[0x16];
    char padding[6];
};
#define CAMERA_STRING_OFFSET(member) ((int)&((MapCameraStrings*)0)->member)

extern "C" {

struct MapCameraUpdateVtable {
    char padding00[0x58];
    int (*updateVisibility)(void*);
    int (*updateInput)(void*);
    int (*updateElements)(void*);
};

struct MapCameraPanelFields {
    char padding00[0x5C];
    void* firstActionLatched;
    int secondActionLatched;
    int secondActionRequested;
    int highlightedMode;
    int edgeIndicatorsRequested;
    int controlsRequested;
    int forceVisible;
    int inputEnabledForFrame;
    int transitionFramesRemaining;
    int visibilityState;
    float opacity;
    int visible;
    int layoutReady;
    int layoutDirty;
    int secondActionFeedbackFrames;
    int secondActionPending;
    MapCameraUpdateVtable* cameraVtable;
};
struct MapCameraFirstActionView {
    char padding00[0x5C];
    void* firstActionLatched;
};

struct MapCameraSecondActionView {
    char padding00[0x64];
    void* secondActionRequested;
};

struct MapCameraPendingActionView {
    char padding00[0x60];
    int secondActionLatched;
    char padding64[0x34];
    int secondActionPending;
};

struct MapCameraTransitionView {
    char padding00[0x7C];
    int transitionFramesRemaining;
    int visibilityState;
    float opacity;
};

struct MapCameraEdgeIndicatorView {
    char padding00[0x6C];
    int edgeIndicatorsRequested;
    char padding70[0x20];
    int layoutDirty;
};

struct MapCameraModeView {
    char padding00[0x68];
    void* highlightedMode;
    char padding6C[0x24];
    int layoutDirty;
};

struct MapCameraControlsView {
    char padding00[0x70];
    int controlsRequested;
    char padding74[0x1C];
    int layoutDirty;
};

struct MapCameraInputEnableView {
    char padding00[0x78];
    int inputEnabledForFrame;
    char padding7C[0x14];
    int layoutDirty;
};

struct MapCameraVisibilityView {
    char padding00[0x80];
    int visibilityState;
};

struct MapCameraOpenView {
    char padding00[0x5C];
    int firstActionLatched;
    int secondActionLatched;
    char padding64[0x4];
    int highlightedMode;
    char padding6C[0x10];
    int transitionFramesRemaining;
    int visibilityState;
    float opacity;
    char padding88[0x8];
    int layoutDirty;
    char padding94[0x4];
    int secondActionPending;
};
struct MapCameraOpacityView {
    char padding00[0x84];
    float opacity;
};

struct MapCameraVisibleView {
    char padding00[0x74];
    int forceVisible;
    char padding78[0x10];
    int visible;
};

struct MapCameraForceVisibleView {
    char padding00[0x74];
    int forceVisible;
    char padding78[0x18];
    int layoutDirty;
};

void Layout_DrawWithOpacity(void* layout, float value);
void Layout_Destroy(void* self, int flag);
int Layout_LoadExtended(void* layout, int arg, int name);
void __dl__FPv(void* ptr);
void __dla__FPv(void* ptr);
void Layout_Init(void* self, int value);
void MPBase1_Ctor(void* self);
void SaveData_InitPlayerRecord(void* self);
void* __nw__FUl(unsigned long size);
void Allocation_Dtor(void* self);
void CBpCtrl_dtor(void* self);
int ResourceTable_FindEntryByName(void* obj, const char* name, int* out);
int Layout_Update(void* layout);
void Sound_PlayByName(void* obj, const char* name, const void* position, unsigned int type, const char* alias,
                      const void* distances, float minimumGain, float attenuationRange, float volumeScale);

extern const char mapCameraDialogStrings[];
extern MapCameraUpdateVtable __vt__12CMapCamera2d;
extern char __vt__13COptionDialog[];
extern char optionMenuStrings[];
char s_bpCtrlDtorRegistration_80519F20[12];
char s_bpCtrlHandleDtorRegistration_OptionMenu[12];
char lbl_80519F38[12]; // Input-manager destructor-registration record.
char lbl_80519F44[12]; // Resource-table destructor-registration record.
extern signed char s_resourceTableInitialized;
extern void* s_resourceTableStorage;
extern signed char s_bpCtrlInitialized;
extern char s_bpCtrl[];
extern void* g_soundManager;
extern const float kFloat_0_MapCameraDialog;
extern const float kFloat_0_5_MapCameraSound;
extern const float kFloat_200_MapCameraSound;
extern const float kFloat_1_MapCameraDialog;
extern const float kMapCameraCloseFadeScale;
extern const float kFloat_0_OptionDialog;
extern const float kFloat_6_2831855;
extern float s_twoPi_MapCameraPanel;
extern float s_twoPi_OptionDialog;

void Layout_SetElementEnabled(void*, int, int);
void Layout_SetElementPlayerFlag(void*, int, int, int);
void Layout_SetElementValueState(void*, int, int);
void Layout_SetElementColor(void*, int, int);
void Layout_SetElementPlayerFlagsAndState(void*, int, int);
void Layout_ReadElementRect(void*, int, int*);
void Layout_WriteElementRect(void*, int, int*);
void Layout_GetElementByte58(void*, int, void*);
void Layout_GetElementEnabled(void*, int, int*);
void Layout_GetElementPlayerFlags(void*, int, int*, int);
int Layout_HitTestElement(void*, int, int);
int Layout_HitTestPointerPointInElement(void*, int, int);
int Input_IsActionPressed(void*, int, int);
int Layout_HitTest(void*, int, int);
int InputManager_GetSelectedPlayer(void*);
void InputManager_Init(void*);
void InputManager_Dtor(void*);
void __register_global_object(void*, void*, void*);
int MenuMessage_SetText(void*, int);
int MenuMessage_ResetTextState(void*);
int MenuMessage_UpdateReveal(void*);

extern char s_gameInputManager[];
extern signed char s_inputManagerInitialized;
extern char s_strokePointInputDestructorRegistration[];
extern char s_strokePointHelpTables[];

// Return the first camera action latch.
void* CMapCamera2d::GetFirstActionLatch() {
    MapCameraFirstActionView* self = (MapCameraFirstActionView*)this;
    return self->firstActionLatched;
}

// Return the second camera action request.
void* CMapCamera2d::GetSecondActionRequest() {
    MapCameraSecondActionView* self = (MapCameraSecondActionView*)this;
    return self->secondActionRequested;
}

// Queue second-action feedback unless that action is already latched.
int CMapCamera2d::RequestSecondActionFeedback() {
    MapCameraPendingActionView* self = (MapCameraPendingActionView*)this;
    if (self->secondActionLatched == 0) {
        self->secondActionPending = 1;
    }
    return 1;
}

// Initialize the camera-panel angle constant to one full turn in radians.
void MapCameraPanel_InitAngleConstant() {
    s_twoPi_MapCameraPanel = kFloat_6_2831855;
}

// Draw the camera panel at its current transition opacity.
int CMapCamera2d::DrawWithOpacity() {
    MapCameraOpacityView* self = (MapCameraOpacityView*)this;
    Layout_DrawWithOpacity(self, self->opacity);
    return 1;
}

// Report whether the panel is visible or forced visible.
int CMapCamera2d::IsVisibleOrForced() {
    MapCameraVisibleView* self = (MapCameraVisibleView*)this;
    if (self->visible != 0 || self->forceVisible != 0) {
        return 1;
    }
    return 0;
}

// Show the edge camera indicators for the next layout update.
int CMapCamera2d::RequestModeIndicators1And4() {
    MapCameraEdgeIndicatorView* self = (MapCameraEdgeIndicatorView*)this;
    self->edgeIndicatorsRequested = 1;
    self->layoutDirty = 1;
    return 1;
}

// Select which of the four mode indicators is highlighted.
int CMapCamera2d::SetHighlightedMode(void* value) {
    MapCameraModeView* self = (MapCameraModeView*)this;
    self->highlightedMode = value;
    self->layoutDirty = 1;
    return 1;
}

// Show the camera controls for the next layout update.
int CMapCamera2d::RequestControlsDisplay() {
    MapCameraControlsView* self = (MapCameraControlsView*)this;
    self->controlsRequested = 1;
    self->layoutDirty = 1;
    return 1;
}

// Set the forced-visible override and invalidate the layout when it changes.
int CMapCamera2d::SetForceVisible(int value) {
    MapCameraForceVisibleView* self = (MapCameraForceVisibleView*)this;
    if (self->forceVisible == value) {
        return 1;
    }
    self->forceVisible = value;
    self->layoutDirty = 1;
    return 1;
}

// Enable camera action input for the next layout update.
int CMapCamera2d::EnableInputForFrame() {
    MapCameraInputEnableView* self = (MapCameraInputEnableView*)this;
    self->inputEnabledForFrame = 1;
    self->layoutDirty = 1;
    return 1;
}

// Start the seven-frame closing sequence at full opacity.
int CMapCamera2d::StartClosingTransition() {
    MapCameraTransitionView* self = (MapCameraTransitionView*)this;
    float tmp = kFloat_1_MapCameraDialog;
    self->visibilityState = kCameraPanelClosing;
    self->transitionFramesRemaining = kCameraPanelCloseFrames;
    self->opacity = tmp;
    return 1;
}

// Report that no opening or closing transition is active.
int CMapCamera2d::IsTransitionIdle() {
    MapCameraVisibilityView* self = (MapCameraVisibilityView*)this;
    if (self->visibilityState != kCameraPanelOpening && self->visibilityState != kCameraPanelClosing) {
        return 1;
    }
    return 0;
}

// Report that the camera panel is fully open.
int CMapCamera2d::IsFullyOpen() {
    MapCameraVisibilityView* self = (MapCameraVisibilityView*)this;
    return self->visibilityState == kCameraPanelOpen;
}

// Initialize the option-dialog angle constant to one full turn in radians.
void MapCamera2dPanel_InitTwoPiConstant() {
    s_twoPi_OptionDialog = kFloat_6_2831855;
}

// Start the five-frame opening fade and reset camera action latches.
int CMapCamera2d::StartOpeningTransition() {
    MapCameraOpenView* self = (MapCameraOpenView*)this;
    float tmp = kFloat_0_MapCameraDialog;
    self->visibilityState = kCameraPanelOpening;
    self->transitionFramesRemaining = kCameraPanelOpenFrames;
    self->opacity = tmp;
    self->layoutDirty = 1;
    self->firstActionLatched = 0;
    self->secondActionLatched = 0;
    self->secondActionPending = 0;
    self->highlightedMode = 0;
    return 1;
}

// Destroy the panel layout and optionally free its allocation.
CMapCamera2d::~CMapCamera2d() {
    Layout_Destroy(this, 0);
}

// Load the map-camera layout for the requested player.
int CMapCamera2d::LoadLayout(void* layout, int arg) {
    Layout_LoadExtended(layout, arg, (int)mapCameraDialogStrings);
    return 1;
}

// Initialize the camera panel with no pending actions and zero opacity.
CMapCamera2d::CMapCamera2d() {
    MapCameraPanelFields* self = (MapCameraPanelFields*)this;
    Layout_Init(self, 5);
    float f0 = kFloat_0_MapCameraDialog;
    self->cameraVtable = &__vt__12CMapCamera2d;
    self->firstActionLatched = 0;
    self->secondActionLatched = 0;
    self->secondActionPending = 0;
    self->secondActionRequested = 0;
    self->highlightedMode = 0;
    self->edgeIndicatorsRequested = 0;
    self->controlsRequested = 0;
    self->forceVisible = 0;
    self->inputEnabledForFrame = 1;
    self->transitionFramesRemaining = 0;
    self->visibilityState = kCameraPanelHidden;
    self->opacity = f0;
    self->visible = 0;
    self->layoutReady = 0;
    self->layoutDirty = 0;
    self->secondActionFeedbackFrames = 0;
}

struct MapCameraLayoutStorage {
    char padding00[0x9C];
};
class MapCameraUpdateInterface : public MapCameraLayoutStorage {
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
    virtual int updateVisibility();
    virtual int updateInput();
    virtual int updateElements();
};
// Update the base layout, transition, input and element state; combine success bits.
int CMapCamera2d::UpdateFrame() {
    int lu = Layout_Update(this);
    int updateResult = lu & 1;
    MapCameraUpdateInterface* cam = (MapCameraUpdateInterface*)this;
    updateResult &= cam->updateVisibility();
    updateResult &= cam->updateInput();
    updateResult &= cam->updateElements();
    return updateResult;
}

// Latch the first action once, play its click sound and refresh element state.
int CMapCamera2d::LatchFirstActionAndPlayClick() {
    MapCameraPanelFields* self = (MapCameraPanelFields*)this;
    if (self->firstActionLatched == 0) {
        self->firstActionLatched = (void*)1;
        const char* base = mapCameraDialogStrings;
        float f1 = kFloat_0_5_MapCameraSound;
        const char* str = base + CAMERA_STRING_OFFSET(clickSound);
        self->layoutDirty = 1;
        int r5 = 0;
        float f2 = kFloat_200_MapCameraSound;
        int r6 = 0;
        void* obj = g_soundManager;
        int r7 = 0;
        float f3 = kFloat_1_MapCameraDialog;
        int r8 = 0;
        Sound_PlayByName(obj, str, (const void*)r5, r6, (const char*)r7, (const void*)r8, f1, f2, f3);
    }
    return 1;
}

// Align action hit areas with their display elements and mark the layout ready.
int CMapCamera2d::AlignActionHitAreas() {
    MapCameraPanelFields* self = (MapCameraPanelFields*)this;
    Layout_SetElementEnabled(self, kCameraModeIndicator1, 0);
    Layout_SetElementEnabled(self, kCameraModeIndicator4, 0);
    int tmp[4]; // Layout_ReadElementRect/WriteElementRect move 4 ints
    Layout_ReadElementRect(self, kCameraFirstActionDisplay, tmp);
    Layout_WriteElementRect(self, kCameraFirstActionHit, tmp);
    Layout_ReadElementRect(self, kCameraSecondActionDisplay, tmp);
    Layout_WriteElementRect(self, kCameraSecondActionHit, tmp);
    self->layoutReady = 1;
    return 1;
}

// Hit-test an enabled type-2 stroke element for the selected player.
int CStrokePoint2d::HitTestAvailableElement(int elementId) {
    if (GetInputManagerSlot() == 0) {
        if (s_inputManagerInitialized == 0) {
            InputManager_Init(s_gameInputManager);
            __register_global_object(s_gameInputManager, (void*)InputManager_Dtor,
                                     (void*)s_strokePointInputDestructorRegistration);
            s_inputManagerInitialized = 1;
        }
        GetInputManagerSlot() = (void*)s_gameInputManager;
    }
    void* obj = GetInputManagerSlot();
    char flag = 0;
    Layout_GetElementByte58(this, elementId, &flag);
    if (flag != 2) {
        return 0;
    }
    int enabled = 0;
    int zero = 0;
    Layout_GetElementEnabled(this, elementId, &enabled);
    if (enabled != 0) {
        int selectedPlayer = InputManager_GetSelectedPlayer(obj);
        return Layout_HitTestElement(this, elementId, selectedPlayer);
    }
    return zero;
}

// Test pointer activation of an enabled, unlocked stroke element.
int CStrokePoint2d::HitTestPointerActivation(int elementId) {
    if (GetInputManagerSlot() == 0) {
        if (s_inputManagerInitialized == 0) {
            InputManager_Init(s_gameInputManager);
            __register_global_object(s_gameInputManager, (void*)InputManager_Dtor,
                                     (void*)s_strokePointInputDestructorRegistration);
            s_inputManagerInitialized = 1;
        }
        GetInputManagerSlot() = (void*)s_gameInputManager;
    }
    void* obj = GetInputManagerSlot();
    if (Input_IsActionPressed(obj, kPointerActivateAction, kAllPlayers) != 0) {
        int playerFlags = 0;
        int enabled = 0;
        Layout_GetElementPlayerFlags(this, elementId, &playerFlags, 5);
        Layout_GetElementEnabled(this, elementId, &enabled);
        if (enabled != 0 && playerFlags == 0) {
            int selectedPlayer = InputManager_GetSelectedPlayer(obj);
            return Layout_HitTestPointerPointInElement(this, elementId, selectedPlayer);
        }
    }
    return 0;
}

// Advance panel fades, close feedback and visibility; honor the visibility override.
int CMapCamera2d::UpdateFadeAndVisibility() {
    MapCameraPanelFields* self = (MapCameraPanelFields*)this;
    if (self->transitionFramesRemaining != 0) {
        int framesRemaining = self->transitionFramesRemaining - 1;
        self->transitionFramesRemaining = framesRemaining;
        if (self->visibilityState == kCameraPanelOpening) {
            int tmp = 5 - framesRemaining;
            self->opacity = (float)tmp / 5.0f;
            if (framesRemaining == 0) {
                self->visibilityState = kCameraPanelOpen;
            }
        } else if (self->visibilityState == kCameraPanelClosing) {
            if (framesRemaining < 4) {
                self->opacity = (float)framesRemaining * kMapCameraCloseFadeScale;
            }
            if (self->transitionFramesRemaining == 4) {
                void* obj = g_soundManager;
                const char* base = mapCameraDialogStrings;
                Sound_PlayByName(obj, base + CAMERA_STRING_OFFSET(closeSound), 0, 0, 0, 0, kFloat_0_5_MapCameraSound,
                                 kFloat_200_MapCameraSound, kFloat_1_MapCameraDialog);
            }
            if (self->transitionFramesRemaining == 0) {
                self->visibilityState = kCameraPanelHidden;
                self->firstActionLatched = 0;
                self->layoutDirty = 1;
            }
        }
    }
    if (kFloat_0_MapCameraDialog == self->opacity) {
        self->visible = 0;
    } else {
        self->visible = 1;
    }
    if (self->forceVisible != 0) {
        self->opacity = kFloat_1_MapCameraDialog;
    }
    return 1;
}

struct VBase32 {
    char pad[0xE0];
};
struct VDerived32 : VBase32 {
    virtual void s00();
    virtual void s01();
    virtual void s02();
    virtual void s03();
    virtual void s04();
    virtual void s05();
    virtual void s06();
    virtual void s07();
    virtual void s08();
    virtual void s09();
    virtual void s10();
    virtual void s11();
    virtual void s12();
    virtual void s13();
    virtual void s14();
    virtual void s15();
    virtual void s16();
    virtual void s17();
    virtual void s18();
    virtual void s19();
    virtual void s20();
    virtual void s21();
    virtual void s22();
    virtual void s23();
    virtual void s24();
    virtual void s25();
    virtual void s26();
    virtual void s27();
    virtual void s28();
    virtual void s29();
    virtual void s30();
    virtual void s31();
    virtual void s32();
    virtual void s33();
    virtual int vtmethod(int);
};
#pragma opt_propagation off
#pragma opt_strength_reduction off
#pragma use_lmw_stmw on
// Select the first hit stroke item, update its help text and signal selection changes.
// Selection index 0..7 indexes all three named tables. -1 means no enabled hit;
// selectionChanged is a latch set only when the selected index differs.
int CStrokePoint2d::UpdateHoveredHelpText() {
    char* base = s_strokePointHelpTables;
    int* p = (int*)base + 0;
    int idx = 0;
    int previousSelection = selectedIndex;
    while (idx < kStrokeSelectionCount) {
        int item = *p;
        int hit = Layout_HitTest(this, item, 0);
        // Original CStrokePoint2d::UpdateHoveredHelpText: either hit source selects this item.
        if (hit != 0 || ((VDerived32*)this)->vtmethod(item) != 0) {
            if (inputLock != 0) {
                void* layout = message;
                int scaled = idx << 2;
                int* arr = (int*)((char*)(((StrokePointHelpTableView*)base)->alternateMessageIds));
                int val = *(int*)((char*)arr + scaled);
                MenuMessage_SetText(layout, val);
            } else {
                void* layout = message;
                int scaled = idx << 2;
                int* arr = (int*)((char*)(((StrokePointHelpTableView*)base)->defaultMessageIds));
                int val = *(int*)((char*)arr + scaled);
                MenuMessage_SetText(layout, val);
            }
            selectedIndex = idx;
            break;
        }
        idx++;
        p++;
        continue;
    }
    if (idx == kStrokeSelectionCount || dialogState == 0) {
        void* layout = message;
        MenuMessage_ResetTextState(layout);
        selectedIndex = -1;
    }
    if (selectedIndex != previousSelection) {
        controlsDirty = 1;
    }
    {
        void* layout = message;
        MenuMessage_UpdateReveal(layout);
    }
    return 1;
}
#pragma opt_strength_reduction reset
#pragma opt_propagation reset
#pragma use_lmw_stmw reset

// Allocate option textures, choose the extended or basic layout and create its script entry.
// arg selects extended options (stored as a byte). The dialog retains its texture
// wrapper/layout/script entry; singleton providers remain borrowed/shared.
COptionDialog::COptionDialog(int arg) {
    // Phase 1: base/save-record defaults and lazy shared resource-table setup.
    char* self = (char*)this;
    int negOne = -1;
    MPBase1_Ctor(self);
    float f0 = kFloat_0_OptionDialog;
    *(void**)self = (void*)__vt__13COptionDialog;
    AsCOptionDialog(self)->bpEntryId = negOne;
    AsCOptionDialog(self)->controlModeOptionsVisible = (unsigned char)arg;
    AsCOptionDialog(self)->inputDelay = f0;
    SaveData_InitPlayerRecord(self + 0x24);
    void* resourceTable = Sing_805DC620();
    (*(int*)&AsCOptionDialog(self)->layout) = 0;
    (*(int*)&AsCOptionDialog(self)->saveMenu) = 0;
    (*(int*)&AsCOptionDialog(self)->textureResource) = 0;
    AsCOptionDialog(self)->previousScriptIndex = negOne;
    AsCOptionDialog(self)->closePending = 0;
    int resourceArchive = 0;
    if (resourceTable == 0) {
        if (s_resourceTableInitialized == 0) {
            *(void**)(&s_resourceTableStorage) = 0;
            __register_global_object(&s_resourceTableStorage, (void*)Allocation_Dtor, lbl_80519F44);
            s_resourceTableInitialized = 1;
        }
        Sing_805DC620() = &s_resourceTableStorage;
    }
    void* textureResource;
    // Phase 2: find the pack, then construct its owned texture wrapper. Failure
    // during construction frees that wrapper before propagating the exception.
    int resourceEntry =
        ResourceTable_FindEntryByName(Sing_805DC620(), optionMenuStrings + kOptionTexturePackName, &resourceArchive);
    textureResource = __nw__FUl(0x20);
    if (textureResource != 0) {
        {
            try {
                ((CPackTextureResource*)textureResource)
                    ->Init((const char*)optionMenuStrings, (void**)resourceArchive, (unsigned int)resourceEntry);
            } catch (...) {
                __dl__FPv(textureResource);
                throw;
            }
        }
    }
    unsigned char extendedOptions = AsCOptionDialog(self)->controlModeOptionsVisible;
    AsCOptionDialog(self)->textureResource = textureResource;
    // Phase 3: choose the paired basic/extended layout and script names.
    char* layoutName = optionMenuStrings + kOptionBasicLayoutName;
    if (extendedOptions != 0) {
        layoutName = optionMenuStrings + kOptionExtendedLayoutName;
    }
    void* layout = __nw__FUl(0x5C);
    if (layout != 0) {
        Layout_Init(layout, 5);
    }
    AsCOptionDialog(self)->layout = layout;
    if (GetInputManagerSlot() == 0) {
        if (s_inputManagerInitialized == 0) {
            InputManager_Init(s_gameInputManager);
            __register_global_object(s_gameInputManager, (void*)InputManager_Dtor, lbl_80519F38);
            s_inputManagerInitialized = 1;
        }
        GetInputManagerSlot() = (void*)s_gameInputManager;
    }
    void* obj = GetInputManagerSlot();
    int selectedPlayer = InputManager_GetSelectedPlayer(obj);
    Layout_LoadExtended(AsCOptionDialog(self)->layout, selectedPlayer, (int)layoutName);
    extendedOptions = AsCOptionDialog(self)->controlModeOptionsVisible;
    char* scriptName = optionMenuStrings + kOptionBasicScriptName;
    if (extendedOptions != 0) {
        scriptName = optionMenuStrings + kOptionExtendedScriptName;
    }
    if (Sing_805DC5F0() == 0) {
        if (s_bpCtrlInitialized == 0) {
            ((CBpCtrl*)s_bpCtrl)->__ct();
            __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, s_bpCtrlDtorRegistration_80519F20);
            s_bpCtrlInitialized = 1;
        }
        Sing_805DC5F0() = (void*)s_bpCtrl;
    }
    ((CBpCtrl*)Sing_805DC5F0())->CreateEntry(scriptName, (int*)(self + 0x18), 0);
}

extern char __vt__Q23std12out_of_range[];
extern char __vt__Q23std12length_error[];
extern char __vt__Q23std11logic_error[];
extern char s_sharedCharDeleterVtable[];

void* __nwa__FUl(unsigned long size);
void dtor_80043A2C(void* ptr);
extern "C" void __dt__Q23std9exceptionFv(void* ptr, int n);
void __dt__Q33std6detail16char_array_guardFv(void* ptr, int n);

void appFreeUni__FPv(void* p);
void __throw(const char*, void*, void (*)());
void __dt__Q23std12out_of_rangeFv();
unsigned strlen(const char* s);
char* strcpy(char* dst, const char* src);
int Layout_CollectActivated(void* layout, void* indices, void* players);

struct ActivatedElementList {
    void* data;
    volatile int size;
    int capacity;
};

struct LayoutVectorErrorBase_u802E8A04 {
    void* table;
    // Initialize the logic-error base vtable.
    LayoutVectorErrorBase_u802E8A04() {
        table = __vt__Q23std11logic_error;
    }
    // Destroy the standard exception base during cleanup.
    ~LayoutVectorErrorBase_u802E8A04() {
        __dt__Q23std9exceptionFv(this, -1);
    }
};
struct LayoutVectorAllocation {
    char* pointer;
    char* allocator;
    // Guard the allocated message bytes until ownership is transferred.
    LayoutVectorAllocation(char* p, char* a) : pointer(p), allocator(a) {}
    // Delete guarded message bytes if construction did not transfer them.
    ~LayoutVectorAllocation() {
        if (pointer)
            __dla__FPv(pointer);
    }
};
struct LayoutVectorControlBase {
    void* table;
    unsigned strong, weak;
    // Initialize both message-storage reference counts to one.
    LayoutVectorControlBase() : strong(1), weak(1) {}
};
struct LayoutVectorControl : LayoutVectorControlBase {
    char* pointer;
    // Install the shared character deleter for the supplied message allocation.
    LayoutVectorControl(char* p) {
        table = s_sharedCharDeleterVtable;
        pointer = p;
    }
};
struct LayoutVectorError : LayoutVectorErrorBase_u802E8A04 {
    char* text;
    void* control;
    // Copy the error message and attach reference-counted storage.
    LayoutVectorError(const char* message) {
        struct Allocator {
            char value;
            char pad[3];
        } allocator;
        allocator.value = 0;
        unsigned n = strlen(message) + 1;
        text = (char*)__nwa__FUl(2 * n - n);
        {
            char* p = text;
            LayoutVectorAllocation allocation(p, &allocator.value);
            control = new LayoutVectorControl(p);
            allocation.pointer = 0;
        }
        strcpy(text, message);
        table = __vt__Q23std12length_error;
    }
};

namespace std {
struct out_of_range : LayoutVectorErrorBase_u802E8A04 {
    char* text;
    void* control;
    // Construct the range-error object from a copied diagnostic message.
    out_of_range(const char* message) {
        struct Allocator {
            char value;
            char pad[3];
        } allocator;
        allocator.value = 0;
        unsigned n = strlen(message) + 1;
        text = (char*)__nwa__FUl(2 * n - n);
        {
            char* p = text;
            LayoutVectorAllocation allocation(p, &allocator.value);
            control = new LayoutVectorControl(p);
            allocation.pointer = 0;
        }
        strcpy(text, message);
        table = __vt__Q23std12out_of_range;
    }
    DECOMP_CT_MEMBER(out_of_range)
};
}

#pragma use_lmw_stmw on
// Report activation of either camera action element (4 or 6).
int CMapCamera2d::IsEitherActionActivated(void* layout) {
    ActivatedElementList indices;
    indices.data = 0;
    indices.size = 0;
    indices.capacity = 0;
    Layout_CollectActivated(layout, &indices, 0);
    unsigned index = 0;
    int byteOffset = 0;
    const char* base = mapCameraDialogStrings;
    while (index < indices.size) {
        if (index >= indices.size) {
            const char* msg = base + CAMERA_STRING_OFFSET(indexRangeError);
            {
                union {
                    double alignment;
                    unsigned char bytes[sizeof(std::out_of_range)];
                } storage;
                std::out_of_range* error = (std::out_of_range*)storage.bytes;
                error->__ct(msg);
                exception_runtime::__throw(exception_runtime::mapCameraDialogStrings +
                                               CAMERA_STRING_OFFSET(rangeErrorType),
                                           error, (void*)exception_runtime::__dt__Q23std12out_of_rangeFv);
            }
        }
        int val = ((int*)indices.data)[byteOffset / 4];
        if (val == 4) {
            if (indices.data != 0) {
                int sz = indices.size;
                indices.size = sz - sz;
                appFreeUni__FPv(indices.data);
            }
            return 1;
        } else {
            if (index >= indices.size) {
                const char* msg = base + CAMERA_STRING_OFFSET(indexRangeError);
                {
                    union {
                        double alignment;
                        unsigned char bytes[sizeof(std::out_of_range)];
                    } storage;
                    std::out_of_range* error = (std::out_of_range*)storage.bytes;
                    error->__ct(msg);
                    exception_runtime::__throw(exception_runtime::mapCameraDialogStrings +
                                                   CAMERA_STRING_OFFSET(rangeErrorType),
                                               error, (void*)exception_runtime::__dt__Q23std12out_of_rangeFv);
                }
            }
            val = ((int*)indices.data)[byteOffset / 4];
            if (val == 6) {
                if (indices.data != 0) {
                    int sz = indices.size;
                    indices.size = sz - sz;
                    appFreeUni__FPv(indices.data);
                }
                return 1;
            }
        }
        index++;
        byteOffset += 4;
    }
    if (indices.data != 0) {
        int sz = indices.size;
        indices.size = sz - sz;
        appFreeUni__FPv(indices.data);
    }
    return 0;
}
#pragma use_lmw_stmw reset

// Collect camera actions and maintain three-frame pressed feedback.
int CMapCamera2d::UpdateActionInput() {
    MapCameraPanelFields* self = (MapCameraPanelFields*)this;
    int r0 = self->visibilityState;
    self->secondActionRequested = 0;
    if (r0 != 2) {
        return 0;
    }
    ActivatedElementList indices;
    indices.data = 0;
    indices.size = 0;
    indices.capacity = 0;
    Layout_CollectActivated(self, &indices, 0);

    const char* base = mapCameraDialogStrings;
    int byteOffset = 0;
    unsigned index = 0;
    while (index < indices.size) {
        if (index >= indices.size) {
            const char* msg = base + CAMERA_STRING_OFFSET(indexRangeError);
            {
                union {
                    double alignment;
                    unsigned char bytes[sizeof(std::out_of_range)];
                } storage;
                std::out_of_range* error = (std::out_of_range*)storage.bytes;
                error->__ct(msg);
                exception_runtime::__throw(exception_runtime::mapCameraDialogStrings +
                                               CAMERA_STRING_OFFSET(rangeErrorType),
                                           error, (void*)exception_runtime::__dt__Q23std12out_of_rangeFv);
            }
        }
        int val = ((int*)indices.data)[byteOffset / 4];
        if (val == 4) {
            self->firstActionLatched = (void*)1;
            self->layoutDirty = 1;
        } else {
            if (index >= indices.size) {
                const char* msg = base + CAMERA_STRING_OFFSET(indexRangeError);
                {
                    union {
                        double alignment;
                        unsigned char bytes[sizeof(std::out_of_range)];
                    } storage;
                    std::out_of_range* error = (std::out_of_range*)storage.bytes;
                    error->__ct(msg);
                    exception_runtime::__throw(exception_runtime::mapCameraDialogStrings +
                                                   CAMERA_STRING_OFFSET(rangeErrorType),
                                               error, (void*)exception_runtime::__dt__Q23std12out_of_rangeFv);
                }
            }
            val = ((int*)indices.data)[byteOffset / 4];
            if (val == 6) {
                self->secondActionLatched = 1;
                self->secondActionRequested = 1;
                self->secondActionFeedbackFrames = kCameraActionFeedbackFrames;
                self->layoutDirty = 1;
            }
        }
        index++;
        byteOffset += 4;
    }

    if (self->secondActionPending != 0) {
        self->secondActionPending = 0;
        if (self->secondActionLatched == 0) {
            void* obj = g_soundManager;
            const char* str = mapCameraDialogStrings + CAMERA_STRING_OFFSET(clickSound);
            Sound_PlayByName(obj, str, 0, 0, 0, 0, kFloat_0_5_MapCameraSound, kFloat_200_MapCameraSound,
                             kFloat_1_MapCameraDialog);
            self->secondActionLatched = 1;
            self->secondActionRequested = 1;
            self->secondActionFeedbackFrames = kCameraActionFeedbackFrames;
            self->layoutDirty = 1;
        }
    }
    if (self->secondActionFeedbackFrames > 0) {
        self->secondActionFeedbackFrames--;
        if (self->secondActionFeedbackFrames == 0) {
            self->secondActionLatched = 0;
            self->layoutDirty = 1;
        }
    }
    if (indices.data != 0) {
        int sz = indices.size;
        indices.size = sz - sz;
        appFreeUni__FPv(indices.data);
    }
    return 1;
}

// Apply pending visibility, mode highlight and per-frame control requests to elements.
int CMapCamera2d::ApplyPanelLayout() {
    MapCameraPanelFields* self = (MapCameraPanelFields*)this;
    // Phase 1: apply changes only to a ready, dirty layout.
    if (self->layoutReady == 0) {
        return 0;
    }
    if (self->layoutDirty == 0) {
        return 0;
    }
    self->layoutDirty = 0;
    int enabled = 1;
    if (self->visibilityState == kCameraPanelHidden) {
        enabled = 0;
    }
    Layout_SetElementEnabled(self, kCameraFirstActionDisplay, enabled);
    Layout_SetElementEnabled(self, kCameraFirstActionHit, enabled);
    Layout_SetElementEnabled(self, kCameraSecondActionDisplay, enabled);
    Layout_SetElementEnabled(self, kCameraSecondActionHit, enabled);
    if (self->firstActionLatched != 0) {
        Layout_SetElementPlayerFlag(self, kCameraFirstActionDisplay, 0, 5);
        Layout_SetElementPlayerFlag(self, kCameraFirstActionHit, 0, 5);
        Layout_SetElementValueState(self, kCameraFirstActionDisplay, 2);
        Layout_SetElementValueState(self, kCameraFirstActionHit, 2);
    } else {
        if (self->inputEnabledForFrame != 0) {
            Layout_SetElementPlayerFlag(self, kCameraFirstActionDisplay, 1, 5);
            Layout_SetElementPlayerFlag(self, kCameraFirstActionHit, 1, 5);
        } else {
            Layout_SetElementPlayerFlag(self, kCameraFirstActionDisplay, 0, 5);
            Layout_SetElementPlayerFlag(self, kCameraFirstActionHit, 0, 5);
        }
        Layout_SetElementValueState(self, kCameraFirstActionDisplay, 0);
        Layout_SetElementValueState(self, kCameraFirstActionHit, 0);
    }
    if (self->secondActionLatched != 0) {
        Layout_SetElementPlayerFlag(self, kCameraSecondActionDisplay, 0, 5);
        Layout_SetElementPlayerFlag(self, kCameraSecondActionHit, 0, 5);
        Layout_SetElementValueState(self, kCameraSecondActionDisplay, 2);
        Layout_SetElementValueState(self, kCameraSecondActionHit, 2);
    } else {
        if (self->inputEnabledForFrame != 0) {
            Layout_SetElementPlayerFlag(self, kCameraSecondActionDisplay, 1, 5);
            Layout_SetElementPlayerFlag(self, kCameraSecondActionHit, 1, 5);
        } else {
            Layout_SetElementPlayerFlag(self, kCameraSecondActionDisplay, 0, 5);
            Layout_SetElementPlayerFlag(self, kCameraSecondActionHit, 0, 5);
        }
        Layout_SetElementValueState(self, kCameraSecondActionDisplay, 0);
        Layout_SetElementValueState(self, kCameraSecondActionHit, 0);
    }
    if (self->inputEnabledForFrame != 0) {
        // Phase 2: consume this frame input request and schedule its next clear.
        self->inputEnabledForFrame = 0;
        self->layoutDirty = 1;
    }
    if (self->edgeIndicatorsRequested != 0) {
        Layout_SetElementEnabled(self, kCameraModeIndicator1, 1);
        Layout_SetElementEnabled(self, kCameraModeIndicator4, 1);
        self->edgeIndicatorsRequested = 0;
        self->layoutDirty = 1;
    } else {
        Layout_SetElementEnabled(self, kCameraModeIndicator1, 0);
        Layout_SetElementEnabled(self, kCameraModeIndicator4, 0);
    }
    Layout_SetElementEnabled(self, kCameraModeIndicator2, 1);
    Layout_SetElementEnabled(self, kCameraModeIndicator3, 1);
    Layout_SetElementColor(self, kCameraModeIndicator1, -0x80);
    Layout_SetElementColor(self, kCameraModeIndicator2, -0x80);
    Layout_SetElementColor(self, kCameraModeIndicator3, -0x80);
    Layout_SetElementColor(self, kCameraModeIndicator4, -0x80);
    // Phase 3: highlight one indicator; forced visibility is applied last.
    switch (self->highlightedMode) {
    case 1:
        Layout_SetElementColor(self, kCameraModeIndicator1, -1);
        break;
    case 2:
        Layout_SetElementColor(self, kCameraModeIndicator2, -1);
        break;
    case 3:
        Layout_SetElementColor(self, kCameraModeIndicator3, -1);
        break;
    case 4:
        Layout_SetElementColor(self, kCameraModeIndicator4, -1);
        break;
    default:
        break;
    }
    if (self->controlsRequested != 0) {
        Layout_SetElementEnabled(self, kCameraControl0, 1);
        Layout_SetElementEnabled(self, kCameraControl1, 1);
        self->controlsRequested = 0;
        self->layoutDirty = 1;
    } else {
        Layout_SetElementEnabled(self, kCameraControl0, 0);
        Layout_SetElementEnabled(self, kCameraControl1, 0);
    }
    Layout_SetElementEnabled(self, kCameraForcedControl2, 0);
    Layout_SetElementEnabled(self, kCameraForcedControl3, 0);
    if (self->visibilityState == kCameraPanelHidden && self->layoutReady != 0) {
        int i = 0;
        while (i < kCameraPanelElementCount) {
            Layout_SetElementPlayerFlagsAndState(self, i, 0);
            i++;
        }
    }
    if (self->forceVisible != 0) {
        Layout_SetElementEnabled(self, kCameraModeIndicator1, 1);
        Layout_SetElementEnabled(self, kCameraModeIndicator2, 1);
        Layout_SetElementEnabled(self, kCameraModeIndicator3, 1);
        Layout_SetElementEnabled(self, kCameraModeIndicator4, 1);
        Layout_SetElementEnabled(self, kCameraControl0, 1);
        Layout_SetElementEnabled(self, kCameraControl1, 1);
        Layout_SetElementEnabled(self, kCameraForcedControl2, 1);
        Layout_SetElementEnabled(self, kCameraForcedControl3, 1);
    }
    return 1;
}

} // extern "C"

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU8046257C", _ctors_u802E8A04_0, MapCameraPanel_InitAngleConstant);
STATIC_INIT_ENTRY(".CRT$XCU8046257C", _ctors_u802E8A04_1, MapCamera2dPanel_InitTwoPiConstant);
#pragma pop

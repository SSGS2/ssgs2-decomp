// Tour change-character window (CTourWindow): the character and caddie choice screen, the power,
// control and spin stat bars, the shared file handles it holds, and the Confirm commit.

#include <decomp/static_init.h>
#include "ppc_double_words.h"
// Tour Change Character window: loads BP/layout resources, previews character
// and caddie choices, displays Power/Control/Spin, commits only on Confirm.
// CTourMain_ProcessItemMenu owns this window; save record pointers are borrowed.
// Modding lookup:
//   return-address rule). Draw order is BP -> layout hover -> numbers using the
//   caller's projection; no local viewport, scissor or aspect scalar is set here.
// - Portrait atlas: InitializeCharacterSelection, 54-pixel X stride, rows 0/58;
//   kTourPortraitStep and the data section. Crop and layout must stay coherent.
// - Stat bars: SetThreeStatBars, maximum 50 Power / 30 Control / 30 Spin;
//   X=(stat/maximum)*100/3, Y=1, scalar definitions in the data section.
// - Controls: ProcessSelectionInput uses game actions 4/6 (confirm/cancel);
//   eight character + eight caddie controls and paired availability overlays.
// Resource paths are in tourWindowStrings; layouts define the actual geometry.
// No texture pixel-format or frame-rate policy is introduced by this window.
// Methods require a live initialized self except nullable deleting destructors;
// load completion must be polled before layout/BP consumers are called.
#define GAME_VECTOR_AGGREGATE

#include "game/vector.h"

// Stack-local storage view; external vector operations retain the original ABI.
struct LocalIntVector_u802FBA74 {
    int* items;
    volatile unsigned int size;
    unsigned int capacity;
    // Free the collected hover IDs on normal or exceptional exit; entries are plain integers.
    ~LocalIntVector_u802FBA74() {
        if (items) {
            size = *(unsigned int*)&size - *(int*)&size;
            appFreeUni__FPv(items);
        }
    }
};

#define char_array_guard CharArrayGuard_u802FBA74
#include "game/stdexcept_local.h"
#undef char_array_guard
#include "game/singletons.h"
#define __throw SDKHeaderThrow
#define strlen SDKHeaderStrlen
#include "sdk_protos.h"
#include "classes/CTourWindow.h"
#include "classes/SavePlayerRecord.h"
#include "classes/UITextTable.h"
#include "classes/CSharedFileHandleOwner.h"
#include "classes/CSharedFileHandle.h"
#include "classes/CSharedFileHandleHolder.h"
#include "classes/CBpCtrl.h"
#include "classes/CFontNumDraw.h"

// Borrow the shared-file owner's field view without changing handle ownership.
static inline CSharedFileHandleOwner* AsCSharedFileHandleOwner(void* object) {
    return (CSharedFileHandleOwner*)object;
}

// Borrow the Tour character/caddie window view, retaining the original layout.
static inline CTourWindow* AsCTourWindow(void* object) {
    return (CTourWindow*)object;
}
#undef strlen
#undef __throw
extern "C" {
void* __nw__FUl(unsigned long);
void __dl__FPv(void*);
}
// Allocate the small owned handle/stat wrappers through the game's object allocator.
inline void* operator new(size_t size) {
    return __nw__FUl(size);
}
void operator delete(void*) throw();

extern "C" {

extern float kFloat_6_2831855;
extern float s_tourWindowFullTurn;
extern float kFloat_1_805E1310;
extern int kFloatBits_1_TourScaleX;
extern int kFloatBits_1_TourScaleY;
extern float kTourStatBarScaleNumerator;
extern float kFloat_50_805E1324;
extern float kTourStatBarScaleDivisor;
extern float kFloat_30_805E132C;
extern double kDouble_IntToFloatBias_805E1300;
extern int CSharedFilePtr_GetLoadedBuffer(unsigned int);
extern void* SharedFile_Create(void*, int);
extern void __dl__FPv(void*);
extern void* __nw__FUl(unsigned long);

extern char s_bpCtrl[];
extern char s_bpCtrlDtorRegistration_tourWindow[];
extern signed char s_bpCtrlInitialized;
extern signed char s_pointerVectorInitialized;
struct TourWindowStringPool {
    char characterSelectScene[15];
    char characterSelectLayout[15];
    char numberSprites[17];
    char defaultSelectionScript[15];
    char vectorRangeError[30];
    char outOfRangeType[55];
    char clickSound[14];
    char powerBar[10];
    char controlBar[10];
    char spinBar[10];
    char padding;
};
extern TourWindowStringPool tourWindowStrings;
typedef char TourWindowStringPool_size[(sizeof(TourWindowStringPool) == 0xC0) ? 1 : -1];
extern char s_fontNumDrawRegistry[];
extern char s_fontNumDrawRegistryDtorRegistration_Tour[];

extern void CBpCtrl_dtor(void*);
extern void* IdMap_Lookup(void*, void*);
extern void* BpObject_GetOrCreateNamedGroup(void*, const char*);
extern void UIGroup_SetScale(void*, float*);
extern void BpObject_Draw(void*);
extern void Layout_UpdateCursorHover(void*, float);
extern void UITextTable_Draw(void*);
extern void PointerVector_DestroyStorage(void*);
extern void UITextTable_ClearEntries(void*);
extern void PointerVector_InitReserved(void*);
extern void PointerArray_DeleteValue(void*, void*);
extern void* Layout_LoadExtended(void*, void*, void*);
extern void* BpObject_PollLoad(void*);
extern void Layout_UpdateExtended(void*);
extern int Layout_PollLoadComplete(void*);
extern void* IdMap_Lookup(void*, void*);
extern void SaveData_InitPlayerRecord(void*);
extern void PlayerStats_InitEquippedAttributeState(void*, void*);
extern void PlayerStats_ComputeEquippedAttributeTotals(void*, void*);
extern void UITextTable_SetInteger(void*, int, int);

// Integer view of the screen-state word at +0x00.
struct TourScreenState {
    int state;
};
enum TourScreenMode {
    TourScreen_Selecting = 1,
    TourScreen_Closing = 2
};
enum TourElement {
    TourElement_CaddieFirst = 8,
    TourElement_CharacterOverlayFirst = 0x10,
    TourElement_CaddieOverlayFirst = 0x18,
    TourElement_Confirm = 0x20,
    TourElement_PowerValue = 0x3D,
    TourElement_ControlValue = 0x3E,
    TourElement_SpinValue = 0x3F
};
// Layout ID map: 0..7 character controls, 8..15 caddie controls; +16 gives each control's
// paired availability overlay (16..23 / 24..31). 32 confirms; 61/62/63 draw Power/Control/Spin.
// selectedCharacterIndex/characterAvailable are CHARACTER index/flags; selectedCaddieIndex/caddieAvailable are CADDIE.
// The playerRecord pointer is borrowed. Preview changes stay in those two fields until Confirm.
// Screen states: 1 accepts hover/input; 2 disables the BP object and returns completion.
enum TourPortraitGeometry {
    kTourPortraitStep = 0x36
};
// Negative channel substitutes selectedPlayer (game_input_manager_cursor_rumble.cpp), the currently
// selected controller channel. It does not scan all controllers.
enum TourSelectionConstants {
    InputAction_Confirm = 4,
    InputAction_Cancel = 6,
    InputPlayer_Selected = -1,
    TourCharacterCount = 8,
    TourCaddieCount = 8,
    Availability_Unlocked = 0x40,
    TourPowerMaximum = 50,
    TourControlMaximum = 30,
    TourSpinMaximum = 30,
    SavedCharacterEquipmentStride = 0x34,
    SavedCaddieEquipmentStride = 8
};
// SavePlayerRecord describes selection indices; these views expose the adjacent accessory byte and ownership flags.
struct TourSaveAccessory {
    char prefix[0x30];
    signed char caddieItemIndex;
};
struct TourCharacterAvailability {
    unsigned char flags;
    char equipment[0x33];
};
struct TourCaddieAvailability {
    unsigned char flags;
    char equipment[7];
};
struct TourAvailabilityView {
    char prefix[0x2CC];
    TourCharacterAvailability characters[TourCharacterCount];
    TourCaddieAvailability caddies[TourCaddieCount];
};
typedef char TourCaddieFlagsOffset[((int)&((TourAvailabilityView*)0)->caddies == 0x46C) ? 1 : -1];

// Start loading the character-select scene and layout.
// BP owns its registered scene entry; self keeps the ID and owns its layout.
// Starts asynchronous work; IsLoadComplete must succeed before drawing/input.
// Entry requires a constructed layout and no active scene load owned by this
// window. There is no failure transaction: CreateEntry's status and LoadExtended's
// return are ignored, so scene creation can fail while layout work starts (or
// vice versa). bpEntryId is written only as the controller specifies; it is not
// cleared first. Polling looks it up and passes the result to PollLoad without a
// null guard. A zero poll is only pending, with no timeout/error/retry signal.
// Calling this again registers another scene request; it does not remove the
// previous ID or implement retry cleanup. On failure tear down through the owner
// before restarting; no readiness or drawing guarantee exists for partial setup.
// Producer/consumer evidence: bp_ctrl_entry_manager.cpp::CBpCtrl::CreateEntry registers/publishes
// before path setup (so return0 can still own an entry); layout.cpp::LoadExtended
// resets progress and overwrites its shared-file handle without a retry release;
// bp_object_sprite_resources.cpp::BpObject_PollLoad dereferences its receiver and returns1 only when
// already ready at entry. A missing scene entry is not a safe pending-load object.
void CTourWindow::LoadResources() {
    if (Sing_805DC5F0() == 0) {
        if (!s_bpCtrlInitialized) {
            ((CBpCtrl*)s_bpCtrl)->__ct();
            __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, s_bpCtrlDtorRegistration_tourWindow);
            s_bpCtrlInitialized = 1;
        }
        Sing_805DC5F0() = s_bpCtrl;
    }
    ((CBpCtrl*)Sing_805DC5F0())->CreateEntry(tourWindowStrings.characterSelectScene, &bpEntryId, 0);
    Layout_LoadExtended(layout, 0, tourWindowStrings.characterSelectLayout);
}

// Poll both the scene and extended layout before accepting selection input.
// Returns 0 at the first pending stage, 1 only when both are ready; no resources
// are transferred. Layout advances only after the BP object reports ready.
int CTourWindow::IsLoadComplete() {
    if (Sing_805DC5F0() == 0) {
        if (!s_bpCtrlInitialized) {
            ((CBpCtrl*)s_bpCtrl)->__ct();
            __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, s_bpCtrlDtorRegistration_tourWindow);
            s_bpCtrlInitialized = 1;
        }
        Sing_805DC5F0() = s_bpCtrl;
    }
    void* obj = IdMap_Lookup(Sing_805DC5F0(), bpEntryId);
    if (!BpObject_PollLoad(obj))
        return 0;
    Layout_UpdateExtended(layout);
    return Layout_PollLoadComplete(layout) != 0;
}

// Replace the registered number renderer and clear its previous entries.
void CTourWindow::ResetNumberDraw() {
    if (numberDraw != 0) {
        AsUITextTable(numberDraw)->drawEnabled = 0;
        UITextTable_ClearEntries(numberDraw);
        if (Sing_805DC830() == 0) {
            if (!s_pointerVectorInitialized) {
                PointerVector_InitReserved(s_fontNumDrawRegistry);
                __register_global_object(s_fontNumDrawRegistry, (void*)PointerVector_DestroyStorage,
                                         s_fontNumDrawRegistryDtorRegistration_Tour);
                s_pointerVectorInitialized = 1;
            }
            Sing_805DC830() = s_fontNumDrawRegistry;
        }
        PointerArray_DeleteValue(Sing_805DC830(), numberDraw);
    }
    if (Sing_805DC830() == 0) {
        if (!s_pointerVectorInitialized) {
            PointerVector_InitReserved(s_fontNumDrawRegistry);
            __register_global_object(s_fontNumDrawRegistry, (void*)PointerVector_DestroyStorage,
                                     s_fontNumDrawRegistryDtorRegistration_Tour);
            s_pointerVectorInitialized = 1;
        }
        Sing_805DC830() = s_fontNumDrawRegistry;
    }
    numberDraw = CFontNumDraw::CreateInVector((PtrVector*)Sing_805DC830());
}

// Acquire an asynchronous cached-file handle for pathString (a borrowed CString object).
// The handle is null on failure; a nonnull handle need not have a loaded buffer yet.
CSharedFileHandleHolder::CSharedFileHandleHolder(void* pathString) {
    handle = SharedFile_Create(pathString, 0);
}

// Release the shared file handle.
CSharedFileHandleHolder::~CSharedFileHandleHolder() {
    if (handle != 0) {
        CSharedFileHandle::Destroy(handle, 1);
        handle = 0;
    }
}

// Test whether the owned shared file has a loaded buffer.
int CSharedFileHandleOwner::IsLoaded() {
    return CSharedFilePtr_GetLoadedBuffer(**(unsigned int**)&sharedFilePtr) != 0;
}

// Initialize this unit's full-turn constant; callers ignore its return register.
// Deliberately has no C++ return expression despite the recovered int ABI;
// this is not a success/status result and must not be tested by new callers.
int TourWindow_InitializeFullTurnConstant() {
    s_tourWindowFullTurn = kFloat_6_2831855;
    // The original returns no value: its callers never read one.
}

// Return the loaded-buffer address through the shared-file wrapper; this is not a byte count.
// value is a required live handle-holder; returned Wii address is borrowed as
// an int, zero while not loaded, and valid only while the handle stays retained.
int SharedFilePtr_GetLoadedBuffer(unsigned int* value) {
    return CSharedFilePtr_GetLoadedBuffer(*value);
}

// Test the underlying loaded-buffer query for a nonzero result.
// Required initialized holder; zero means pending/absent buffer, not a byte size.
int SharedFilePtr_HasLoadedBuffer(unsigned int* value) {
    return CSharedFilePtr_GetLoadedBuffer(*value) != 0;
}

// Owns one shared file handle; type name is referenced by exception cleanup.
struct CSharedFileHandleWrapper {
    void* p;
    // Acquire one shared-file handle; its enclosing owner releases it before deleting this wrapper.
    CSharedFileHandleWrapper(void* v, int x) {
        p = SharedFile_Create(v, x);
    }
};

// Allocate the handle holder and initialize the owner's state word.
CSharedFileHandleOwner::CSharedFileHandleOwner(void* value) {
    CSharedFileHandleWrapper* ptr = new CSharedFileHandleWrapper(value, 0);
    (*(CSharedFileHandleWrapper**)&sharedFilePtr) = ptr;
    *(int*)this = 0;
}

// Destroy the owned handle holder and optionally delete the owner.
CSharedFileHandleOwner::~CSharedFileHandleOwner() {
    CSharedFileHandleWrapper* ptr = (*(CSharedFileHandleWrapper**)&sharedFilePtr);
    if (ptr != 0) {
        if (ptr != 0) {
            if (ptr->p != 0) {
                CSharedFileHandle::Destroy(ptr->p, 1);
                ptr->p = 0;
            }
            __dl__FPv(ptr);
        }
        sharedFilePtr = 0;
    }
}

// Draw the character-select scene, cursor hover and numeric stats.
void CTourWindow::Draw() {
    // BP draw's return address to adjust the Change Character backdrop; keep the
    // following HUD layout/numbers in the same screen coordinate convention.
    if (Sing_805DC5F0() == 0) {
        if (!s_bpCtrlInitialized) {
            char* base = s_bpCtrl;
            ((CBpCtrl*)base)->__ct();
            __register_global_object(base, (void*)CBpCtrl_dtor, s_bpCtrlDtorRegistration_tourWindow);
            s_bpCtrlInitialized = 1;
        }
        Sing_805DC5F0() = s_bpCtrl;
    }
    BpObject_Draw(IdMap_Lookup(Sing_805DC5F0(), bpEntryId));
    Layout_UpdateCursorHover(layout, kFloat_1_805E1310);
    UITextTable_Draw(numberDraw);
}

// Equipment-stat calculator storage; keep its compiler-visible type name for EH relocations.
struct CPreviewStatCalculator {
    char data[8];
    // Initialize temporary equipped-stat calculation from a borrowed preview save record.
    CPreviewStatCalculator(void* buf) {
        PlayerStats_InitEquippedAttributeState(this, buf);
    }
};

// Preview character/caddie selection in a temporary save record and recompute its stats.
// elementId modulo 16 selects character 0..7 or caddie 8..15, including paired overlay IDs.
// Negative IDs do nothing; callers exclude Confirm (32). The save record itself is not modified.
// Lifetime/exception ledger, identical for both branches: previewRecord is a
// stack copy; statCalculator is a raw heap allocation borrowing that copy only
// during this call. Normal completion publishes bars then directly deletes the
// calculator without a destructor. Constructor failure uses new-expression
// cleanup; after acquisition, ComputeEquippedAttributeTotals or SetThreeStatBars
// throwing would bypass that delete (no RAII owner/catch), leaking the calculator.
// Its borrowed preview pointer then outlives the stack storage if retained.
// The selected index is published BEFORE allocation; failures do not roll it
// back and can leave previous bars. This code preserves that existing path.
void CTourWindow::UpdateSelectionAndStatBars(int elementId) {
    if (elementId < 0)
        return;
    int selectionIndex = elementId % 16;
    if (selectionIndex < 8) {
        // Character branch: update the preview character, retain the preview caddie.
        selectedCharacterIndex = (signed char)selectionIndex;
        char previewRecord[0x828];
        SaveData_InitPlayerRecord(previewRecord);
        Memory_Copy(previewRecord, playerRecord, 0x828);
        *(char*)&AsSavePlayerRecord(previewRecord)->characterIndex = (*(char*)&selectedCharacterIndex);
        *(char*)&AsSavePlayerRecord(previewRecord)->caddieIndex = (*(char*)&selectedCaddieIndex);
        signed char equippedStats[3];
        equippedStats[0] = 0;
        equippedStats[1] = 0;
        equippedStats[2] = 0;
        CPreviewStatCalculator* statCalculator = new CPreviewStatCalculator(previewRecord);
        PlayerStats_ComputeEquippedAttributeTotals(statCalculator, equippedStats);
        int power = equippedStats[0];
        int control = equippedStats[1];
        int spin = equippedStats[2];
        SetThreeStatBars((int)(signed char)power, (int)(signed char)control, (int)(signed char)spin);
        if (statCalculator != 0)
            __dl__FPv(statCalculator);
    } else {
        // Caddie branch: update the preview caddie, retain the preview character.
        // The repeated save-copy/calculator sequence deliberately preserves compiler lifetimes.
        selectedCaddieIndex = (signed char)(selectionIndex - 8);
        char previewRecord[0x828];
        SaveData_InitPlayerRecord(previewRecord);
        Memory_Copy(previewRecord, playerRecord, 0x828);
        *(char*)&AsSavePlayerRecord(previewRecord)->characterIndex = (*(char*)&selectedCharacterIndex);
        *(char*)&AsSavePlayerRecord(previewRecord)->caddieIndex = (*(char*)&selectedCaddieIndex);
        signed char equippedStats[3];
        equippedStats[0] = 0;
        equippedStats[1] = 0;
        equippedStats[2] = 0;
        CPreviewStatCalculator* statCalculator = new CPreviewStatCalculator(previewRecord);
        PlayerStats_ComputeEquippedAttributeTotals(statCalculator, equippedStats);
        int power = equippedStats[0];
        int control = equippedStats[1];
        int spin = equippedStats[2];
        SetThreeStatBars((int)(signed char)power, (int)(signed char)control, (int)(signed char)spin);
        if (statCalculator != 0)
            __dl__FPv(statCalculator);
    }
}

// Acquire the BP scene used by the stat-bar groups.
inline void* StatController(void* self) {
    if (Sing_805DC5F0() == 0) {
        if (!s_bpCtrlInitialized) {
            ((CBpCtrl*)s_bpCtrl)->__ct();
            __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, s_bpCtrlDtorRegistration_tourWindow);
            s_bpCtrlInitialized = 1;
        }
        Sing_805DC5F0() = s_bpCtrl;
    }
    return IdMap_Lookup(Sing_805DC5F0(), AsCTourWindow(self)->bpEntryId);
}

#pragma opt_lifetimes off
// Clamp Power/Control/Spin for display and update bar scales and numbers.
// Only upper bounds are clamped. Dimensionless X scale = stat/max * 100/3; Y stays 1.
// Signed-byte inputs may be negative; this display intentionally has no lower
// clamp. Missing BP/group skips only that bar; numeric entries are still written.
// Larger numerator/ smaller divisor lengthens ALL bars, not the stats themselves.
// Keep separate maximums and FP divide/multiply/divide order and signed casts.
void CTourWindow::SetThreeStatBars(signed char power, signed char control, signed char spin) {
    int scaleComponents[2] = {kFloatBits_1_TourScaleX, kFloatBits_1_TourScaleY};
    int displayPower = (signed char)(power > TourPowerMaximum ? TourPowerMaximum : power);
    int displayControl = (signed char)(control > TourControlMaximum ? TourControlMaximum : control);
    int displaySpin = (signed char)(spin > TourSpinMaximum ? TourSpinMaximum : spin);
    const char* name = tourWindowStrings.powerBar;
    void* pane = StatController(this);
    void* node;
    if (pane == 0) {
        node = 0;
    } else {
        node = BpObject_GetOrCreateNamedGroup(pane, name);
    }
    if (node) {
        float value =
            ((float)displayPower / kFloat_50_805E1324) * kTourStatBarScaleNumerator / kTourStatBarScaleDivisor;
        *(float*)&scaleComponents[0] = value;
        UIGroup_SetScale(node, (float*)scaleComponents);
    }
    UITextTable_SetInteger(numberDraw, TourElement_PowerValue, displayPower);

    name = tourWindowStrings.controlBar;
    pane = StatController(this);
    if (pane == 0) {
        node = 0;
    } else {
        node = BpObject_GetOrCreateNamedGroup(pane, name);
    }
    if (node) {
        float value =
            ((float)displayControl / kFloat_30_805E132C) * kTourStatBarScaleNumerator / kTourStatBarScaleDivisor;
        *(float*)&scaleComponents[0] = value;
        UIGroup_SetScale(node, (float*)scaleComponents);
    }
    UITextTable_SetInteger(numberDraw, TourElement_ControlValue, displayControl);

    name = tourWindowStrings.spinBar;
    pane = StatController(this);
    if (pane == 0) {
        node = 0;
    } else {
        node = BpObject_GetOrCreateNamedGroup(pane, name);
    }
    if (node) {
        float value = ((float)displaySpin / kFloat_30_805E132C) * kTourStatBarScaleNumerator / kTourStatBarScaleDivisor;
        *(float*)&scaleComponents[0] = value;
        UIGroup_SetScale(node, (float*)scaleComponents);
    }
    UITextTable_SetInteger(numberDraw, TourElement_SpinValue, displaySpin);
}
#pragma opt_lifetimes reset

extern signed char s_inputManagerInitialized;
extern signed char s_fadeOverlayInitialized;
extern void* g_soundManager;
extern float kFloat_0_5_805E1308;
extern float kFloat_200_805E130C;
extern char __vt__Q23std12out_of_range[];
extern char __vt__Q23std11logic_error[];
extern char s_sharedCharDeleterVtable[];
extern char s_fadeOverlay[];
extern char s_gameInputManager[];
extern char s_inputManagerDtorRegistration_Tour[];
extern char s_fadeOverlayDtorRegistration_Tour[];

extern void* __nwa__FUl(unsigned long);
extern void appFreeUni__FPv(void*);
extern void Layout_CollectHovered(void*, void*, int);
extern void InputManager_Init(void*);
extern void InputManager_Dtor(void*);
extern int Input_IsActionPressed(void*, int, int);
extern void Layout_SetElementValueState(void*, int, int);
extern void Layout_SetElementFeedbackState(void*, int, int);
extern void SharedStringState_SetCharacterIndex(int);
extern void SharedStringState_SetCaddieIndex(int);
extern void FadeOverlay_Init(void*);
extern void ResourceState_Dtor(void*);
extern void FadeState_RequestFramebufferCapture(void*);
extern void FadeState_StartFadeIn(void*, int);
extern void Sound_PlayByName(void*, const char*, int, int, int, int, float, float, float);
extern void __dt__Q23std12out_of_rangeFv(void*);
extern void __dla__FPv(void*);
extern void Layout_SetElementPlayerFlag(void*, int, int, int);
extern void* Layout_GetElementSprite(void*, int);
extern void Sprite_SetTextureRegionSize2D(void*, void*);
extern void UIGroup_SetSourcePosition(void*, void*);
extern void BpObject_SetEnabled(void*, int);
extern void BpObject_TryInvokeCurrentCallback(void*, int);
extern void Layout_SetElementEnabled(void*, int, int);
extern void BpObject_StartScriptByName(void*, void*, int);
extern void FadeState_StartFadeOut(void*, int);
extern int kFloatBits_1_TourInitialX;
extern int kFloatBits_1_TourInitialY;
extern float kFloat_0_805E12F8;
extern float kFloat_58_805E12FC;
extern int s_characterPortraitGroupNames[];
extern int s_caddiePortraitGroupNames[];
extern char s_tourStatNumberDescriptors[];

// Apply confirm/cancel input, saving selected character/caddie indices before closing.
// Input flow: first hovered ID -> preview choice or Confirm commit -> fade/closing request.
// hoveredElements is an owned temporary vector prefix {data, count-as-word, capacity-as-word},
// not three element pointers. Only the first collected ID is consumed; its storage is freed here.
// Publication contract: preview handlers update self->selectedCharacterIndex/selectedCaddieIndex and
// compute stats from a temporary initialized SavePlayerRecord, leaving the
// borrowed playerRecord intact. Confirm writes saved character, resets saved
// caddieItemIndex to -1 only if caddie changed, writes saved caddie, THEN publishes
// SharedStringState_SetCharacterIndex/SetCaddieIndex, and finally starts closing.
// These setters update the shared UI string substitution state, not disk storage;
// this function performs no SaveData file write. Cancel alone publishes neither
// selection. Confirm and Cancel are independent tests: simultaneous inputs can
// commit and then request closing again. There is no rollback across those writes.
void CTourWindow::ProcessSelectionInput() {
    void* hoveredElements[3] = {0, 0, 0};
    void* menuAlias = this;
    int hoveredElement;
    Layout_CollectHovered(layout, hoveredElements, 5);
    if (hoveredElements[1] == 0) {
        hoveredElement = -1;
    } else {
        if (hoveredElements[1] == 0) {
            std::out_of_range error(tourWindowStrings.vectorRangeError, tourWindowStrings.outOfRangeType);
        } else {
            hoveredElement = *(int*)hoveredElements[0];
        }
    }
    if (hoveredElements[0] != 0) {
        hoveredElements[1] = 0;
        appFreeUni__FPv(hoveredElements[0]);
    }
    // Lazily acquire the logical input manager.
    if (GetInputManagerSlot() == 0) {
        if (!s_inputManagerInitialized) {
            InputManager_Init(s_gameInputManager);
            __register_global_object(s_gameInputManager, (void*)InputManager_Dtor, s_inputManagerDtorRegistration_Tour);
            s_inputManagerInitialized = 1;
        }
        GetInputManagerSlot() = s_gameInputManager;
    }
    void* manager = GetInputManagerSlot();
    if (manager == 0)
        return;
    if (manager != 0) {
        if (Input_IsActionPressed(manager, InputAction_Confirm, InputPlayer_Selected) != 0) {
            if (hoveredElement != -1) {
                Sound_PlayByName(g_soundManager, (const char*)(tourWindowStrings.clickSound), 0, 0, 0, 0,
                                 kFloat_0_5_805E1308, kFloat_200_805E130C, kFloat_1_805E1310);
                if (hoveredElement != TourElement_Confirm) {
                    UpdateSelectionAndStatBars(hoveredElement);
                } else {
                    // Commit both indices; changing caddie invalidates the saved accessory selection.
                    Layout_SetElementValueState(layout, TourElement_Confirm, 2);
                    Layout_SetElementFeedbackState(layout, TourElement_Confirm, 2);
                    AsSavePlayerRecord(playerRecord)->characterIndex = (*(unsigned char*)&selectedCharacterIndex);
                    {
                        void* savedRecord = playerRecord;
                        signed char selectedCaddie = selectedCaddieIndex;
                        signed char previousCaddie = AsSavePlayerRecord(savedRecord)->caddieIndex;
                        if (previousCaddie != selectedCaddie) {
                            ((TourSaveAccessory*)savedRecord)->caddieItemIndex = -1;
                        }
                    }
                    AsSavePlayerRecord(playerRecord)->caddieIndex = selectedCaddieIndex;
                    SharedStringState_SetCharacterIndex((signed char)(*(unsigned char*)&selectedCharacterIndex));
                    SharedStringState_SetCaddieIndex((signed char)(*(unsigned char*)&selectedCaddieIndex));
                    if (Sing_805DC608() == 0) {
                        if (!s_fadeOverlayInitialized) {
                            FadeOverlay_Init(s_fadeOverlay);
                            __register_global_object(s_fadeOverlay, (void*)ResourceState_Dtor,
                                                     s_fadeOverlayDtorRegistration_Tour);
                            s_fadeOverlayInitialized = 1;
                        }
                        Sing_805DC608() = s_fadeOverlay;
                    }
                    FadeState_RequestFramebufferCapture(Sing_805DC608());
                    if (Sing_805DC608() == 0) {
                        if (!s_fadeOverlayInitialized) {
                            FadeOverlay_Init(s_fadeOverlay);
                            __register_global_object(s_fadeOverlay, (void*)ResourceState_Dtor,
                                                     s_fadeOverlayDtorRegistration_Tour);
                            s_fadeOverlayInitialized = 1;
                        }
                        Sing_805DC608() = s_fadeOverlay;
                    }
                    FadeState_StartFadeIn(Sing_805DC608(), 0);
                    ((TourScreenState*)this)->state = TourScreen_Closing;
                }
            }
        }
    }
    // Cancel requests closing without a new commit. It is tested independently after Confirm;
    // simultaneous actions can therefore reach this path after a commit already occurred.
    if (Input_IsActionPressed(manager, InputAction_Cancel, InputPlayer_Selected) != 0) {
        if (Sing_805DC608() == 0) {
            if (!s_fadeOverlayInitialized) {
                FadeOverlay_Init(s_fadeOverlay);
                __register_global_object(s_fadeOverlay, (void*)ResourceState_Dtor, s_fadeOverlayDtorRegistration_Tour);
                s_fadeOverlayInitialized = 1;
            }
            Sing_805DC608() = s_fadeOverlay;
        }
        void* fadeOverlay = Sing_805DC608();
        FadeState_RequestFramebufferCapture(fadeOverlay);
        if (Sing_805DC608() == 0) {
            if (!s_fadeOverlayInitialized) {
                FadeOverlay_Init(s_fadeOverlay);
                __register_global_object(s_fadeOverlay, (void*)ResourceState_Dtor, s_fadeOverlayDtorRegistration_Tour);
                s_fadeOverlayInitialized = 1;
            }
            Sing_805DC608() = s_fadeOverlay;
        }
        FadeState_StartFadeIn(Sing_805DC608(), 0);
        ((TourScreenState*)this)->state = TourScreen_Closing;
    }
}

#pragma use_lmw_stmw on
// Update selection highlights and return 1 once the screen is closing.
// Indices are -1 or 0..7 and availability arrays cover eight entries. Closing
// disables the BP object without waiting for the fade to finish. The caller passes
// a frame delta that this function does not read. Hovered IDs are collected in a
// temporary list; a failed bounds check throws out_of_range.
int CTourWindow::UpdateSelectionHover(float delta) {
    int result;
    char* text;
    void* excPtr;
    char* message;
    int i;
    char active;
    struct OutOfRangeView {
        void* vtable;
        void* text;
        void* rep;
    };
    volatile struct OutOfRangeView exc;
    struct CExceptionMessageGuard {
        void* first;
        void* second;
        // Release an exception-message allocation unless its shared owner has adopted it.
        ~CExceptionMessageGuard() {
            if (first)
                __dla__FPv(first);
        }
    };
    int hovered;
    result = 0;
    if (Sing_805DC5F0() == 0) {
        if (!s_bpCtrlInitialized) {
            ((CBpCtrl*)s_bpCtrl)->__ct();
            __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, s_bpCtrlDtorRegistration_tourWindow);
            s_bpCtrlInitialized = 1;
        }
        Sing_805DC5F0() = s_bpCtrl;
    }
    BpObject_TryInvokeCurrentCallback(IdMap_Lookup(Sing_805DC5F0(), bpEntryId), 0);
    // Phase 1: update the BP callback/layout before reading this frame's hover IDs.
    Layout_UpdateExtended(layout);
    switch (screenState) {
    case TourScreen_Selecting: {
        // Phase 2: clear all choice/overlay states, then restore available selected choices.
        for (i = 0; i < 16; i++) {
            Layout_SetElementEnabled(layout, i, 0);
            Layout_SetElementEnabled(layout, i + TourElement_CharacterOverlayFirst, 0);
        }
        signed char selectedCharacter = selectedCharacterIndex;
        if (selectedCharacter != -1 && characterAvailable[selectedCharacter] != 0) {
            Layout_SetElementEnabled(layout, selectedCharacter, 1);
        }
        signed char selectedCaddie = selectedCaddieIndex;
        if (selectedCaddie != -1 && caddieAvailable[selectedCaddie] != 0) {
            Layout_SetElementEnabled(layout, selectedCaddie + 8, 1);
        }
        {
            LocalIntVector_u802FBA74 list = {0, 0, 0};
            Layout_CollectHovered(layout, &list, 5);
            if (list.size != 0) {
                if (list.size == 0) {
                    // Reconstructed vector::at(0) exception path nested under its nonempty check.
                    // The redundancy and allocator cleanup lifetimes preserve the original code.
                    active = 0;
                    excPtr = (void*)&exc;
                    message = tourWindowStrings.vectorRangeError;
                    exc.vtable = __vt__Q23std11logic_error;
                    unsigned int n = strlen(message) + 1;
                    n = (n << 1) - n;
                    text = (char*)__nwa__FUl(n);
                    exc.text = text;
                    {
                        CExceptionMessageGuard pair;
                        pair.first = text;
                        pair.second = (void*)&active;
                        void* rep = __nw__FUl(0x10);
                        if (rep != 0) {
                            ((int*)rep)[1] = 1;
                            ((int*)rep)[2] = 1;
                            ((void**)rep)[0] = s_sharedCharDeleterVtable;
                            ((void**)rep)[3] = text;
                        }
                        exc.rep = rep;
                        pair.first = 0;
                    }
                    strcpy((char*)exc.text, message);
                    exc.vtable = __vt__Q23std12out_of_range;
                    __throw(tourWindowStrings.outOfRangeType, excPtr, (void*)__dt__Q23std12out_of_rangeFv);
                }
                hovered = *(int*)list.items;
            } else {
                hovered = -1;
            }
        }
        // Phase 3: overlay the first hovered choice, then process confirm/cancel for this frame.
        if (hovered != -1 && hovered != TourElement_Confirm) {
            Layout_SetElementEnabled(layout, hovered, 1);
        }
        ProcessSelectionInput();
        break;
    }
    case TourScreen_Closing:
        if (Sing_805DC5F0() == 0) {
            if (!s_bpCtrlInitialized) {
                ((CBpCtrl*)s_bpCtrl)->__ct();
                __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, s_bpCtrlDtorRegistration_tourWindow);
                s_bpCtrlInitialized = 1;
            }
            Sing_805DC5F0() = s_bpCtrl;
        }
        BpObject_SetEnabled(IdMap_Lookup(Sing_805DC5F0(), bpEntryId), 0);
        result = 1;
        break;
    }
    return result;
}
#pragma use_lmw_stmw reset

// Equipment-stat calculator used for initial selection.
struct CSavedStatCalculator {
    char data[8];
    // Initialize the initial equipped-stat calculation from the borrowed saved selection.
    CSavedStatCalculator(void* buf) {
        PlayerStats_InitEquippedAttributeState(this, buf);
    }
};

// Load selection indices, ownership flags, portrait positions and initial stat bars.
// self owns loaded layout and registered numberDraw; playerRecord is borrowed.
// Saved characterIndex must be 0..7; caddieIndex is -1 (byte 0xFF: absent) or
// 0..7. SavePlayerRecord.h defines the signed byte encoding, and
// CPlayerStatus_InitEquipmentDisplay (player_status_hud.cpp) tests absent caddie before
// inventory lookup. The UI copies bytes without range repair; -1 is meaningful
// for caddie, not a supported character choice. With availability bit 0x40 set,
// character portraits use atlas ((index+1)*54, 0), caddies use (index*54, 58).
// Unavailable portraits use (0,0). Neither atlas stride nor row coordinate is a
// screen-position/widescreen setting. Publishes
// Selecting after setup, leaving the saved record unchanged until Confirm.
// The hover path also tolerates selectedCharacterIndex==-1 by omitting its highlight, but
// this initializer's stat calculation does not establish a valid character for
// that sentinel. Confirm copies preview bytes unchanged; do not rely on it to
// validate a corrupt/absent character. Default inventory initialization writes
// character0/caddie0 (SaveData_InitDefaultInventoryRecord, save_data_records_and_inventory_init.cpp).
void CTourWindow::InitializeCharacterSelection() {
    // Phase 1: enable scene/numbers, load saved indices, reset portrait region sizes.
    if (Sing_805DC5F0() == 0) {
        if (!s_bpCtrlInitialized) {
            ((CBpCtrl*)s_bpCtrl)->__ct();
            __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, s_bpCtrlDtorRegistration_tourWindow);
            s_bpCtrlInitialized = 1;
        }
        Sing_805DC5F0() = s_bpCtrl;
    }
    BpObject_SetEnabled(IdMap_Lookup(Sing_805DC5F0(), bpEntryId), 1);
    UITextTable_ClearEntries(numberDraw);

    if (Sing_805DC5F0() == 0) {
        if (!s_bpCtrlInitialized) {
            ((CBpCtrl*)s_bpCtrl)->__ct();
            __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, s_bpCtrlDtorRegistration_tourWindow);
            s_bpCtrlInitialized = 1;
        }
        Sing_805DC5F0() = s_bpCtrl;
    }
    void* pane0 = IdMap_Lookup(Sing_805DC5F0(), bpEntryId);
    ((CFontNumDraw*)numberDraw)
        ->RegisterEntries(s_tourStatNumberDescriptors, 3, pane0, (void*)(tourWindowStrings.numberSprites));
    AsUITextTable(numberDraw)->drawEnabled = 1;
    Layout_SetElementValueState(layout, TourElement_Confirm, 0);
    Layout_SetElementFeedbackState(layout, TourElement_Confirm, 1);

    void* savedRecord = playerRecord;
    selectedCharacterIndex = *(unsigned char*)&AsSavePlayerRecord(savedRecord)->characterIndex;
    selectedCaddieIndex = *(unsigned char*)&AsSavePlayerRecord(savedRecord)->caddieIndex;

    int initialElementValues[4];
    initialElementValues[0] = kFloatBits_1_TourInitialX;
    initialElementValues[1] = kFloatBits_1_TourInitialY;
    initialElementValues[2] = 0;
    initialElementValues[3] = 0;

    for (int i = 0; i < 0x20; i++) {
        void* node = Layout_GetElementSprite(layout, i);
        if (node != 0) {
            Sprite_SetTextureRegionSize2D(node, initialElementValues);
        }
    }

    // Phase 2: build the character row and availability overlays from 0x34-byte save entries.
    // Portrait source coordinates use X=(i+1)*54, Y=0; unavailable portraits use (0,0).
    // The 0x43300000/XOR/subtract sequence converts signed integer X to floating point;
    // its bias is compiler conversion machinery, not an atlas coordinate.
    int* portraitNames = s_characterPortraitGroupNames;
    double rowOriginOrConversionBias = kDouble_IntToFloatBias_805E1300;
    double rowYOrConversionBias = kFloat_0_805E12F8;
    int characterPortraitX = 0;
    int characterEquipmentOffset = 0;
    for (int i = 0; i < TourCharacterCount; i++) {
        Layout_SetElementPlayerFlag(layout, i, 0, 5);
        char* equipmentRecord = (char*)(playerRecord) + characterEquipmentOffset;
        int isUnlocked = (((TourAvailabilityView*)equipmentRecord)->characters[0].flags & Availability_Unlocked) != 0;
        characterAvailable[i] = (signed char)isUnlocked;
        if (isUnlocked) {
            if (Sing_805DC5F0() == 0) {
                if (!s_bpCtrlInitialized) {
                    ((CBpCtrl*)s_bpCtrl)->__ct();
                    __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, s_bpCtrlDtorRegistration_tourWindow);
                    s_bpCtrlInitialized = 1;
                }
                Sing_805DC5F0() = s_bpCtrl;
            }
            int portraitNameAddress = *portraitNames;
            void* pane = IdMap_Lookup(Sing_805DC5F0(), bpEntryId);
            void* node = pane ? BpObject_GetOrCreateNamedGroup(pane, (const char*)portraitNameAddress) : 0;
            if (node != 0) {
                int portraitX = characterPortraitX + kTourPortraitStep;
                unsigned int biasedPortraitX = (unsigned int)portraitX ^ 0x80000000;
                union {
                    double d;
                    unsigned int u[2];
                } integerToDouble;
                integerToDouble.u[PPC_DW_HI] = 0x43300000;
                integerToDouble.u[PPC_DW_LO] = biasedPortraitX;
                float portraitXFloat = (float)(integerToDouble.d - rowOriginOrConversionBias);
                float sourcePosition[2];
                sourcePosition[0] = portraitXFloat;
                sourcePosition[1] = rowYOrConversionBias;
                UIGroup_SetSourcePosition(node, sourcePosition);
            }
            Layout_SetElementPlayerFlag(layout, i + TourElement_CharacterOverlayFirst, 1, 5);
        } else {
            if (Sing_805DC5F0() == 0) {
                if (!s_bpCtrlInitialized) {
                    ((CBpCtrl*)s_bpCtrl)->__ct();
                    __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, s_bpCtrlDtorRegistration_tourWindow);
                    s_bpCtrlInitialized = 1;
                }
                Sing_805DC5F0() = s_bpCtrl;
            }
            int portraitNameAddress = *portraitNames;
            void* pane = IdMap_Lookup(Sing_805DC5F0(), bpEntryId);
            void* node = pane ? BpObject_GetOrCreateNamedGroup(pane, (const char*)portraitNameAddress) : 0;
            if (node != 0) {
                float sourcePosition[2];
                sourcePosition[0] = 0.0f;
                sourcePosition[1] = 0.0f;
                UIGroup_SetSourcePosition(node, sourcePosition);
            }
            Layout_SetElementPlayerFlag(layout, i + TourElement_CharacterOverlayFirst, 0, 5);
        }
        portraitNames++;
        characterEquipmentOffset += SavedCharacterEquipmentStride;
        characterPortraitX += kTourPortraitStep;
    }

    // Phase 3: caddie row uses 8-byte save entries, X=i*54 and Y=58.
    // The two double temporaries exchange roles here (conversion bias vs atlas-row Y).
    portraitNames = s_caddiePortraitGroupNames;
    rowYOrConversionBias = kDouble_IntToFloatBias_805E1300; // double const
    rowOriginOrConversionBias = kFloat_58_805E12FC;         // 58.0f
    int unusedCaddieCounter = 0;
    int caddieEquipmentOffset = 0;
    int caddiePortraitX = 0;
    for (int i = 0; i < TourCaddieCount; i++) {
        Layout_SetElementPlayerFlag(layout, i + TourElement_CaddieFirst, 0, 5);
        char* equipmentRecord = (char*)(playerRecord) + caddieEquipmentOffset;
        int isUnlocked = (((TourAvailabilityView*)equipmentRecord)->caddies[0].flags & Availability_Unlocked) != 0;
        caddieAvailable[i] = (signed char)isUnlocked;
        if (isUnlocked) {
            if (Sing_805DC5F0() == 0) {
                if (!s_bpCtrlInitialized) {
                    ((CBpCtrl*)s_bpCtrl)->__ct();
                    __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, s_bpCtrlDtorRegistration_tourWindow);
                    s_bpCtrlInitialized = 1;
                }
                Sing_805DC5F0() = s_bpCtrl;
            }
            int portraitNameAddress = *portraitNames;
            void* pane = IdMap_Lookup(Sing_805DC5F0(), bpEntryId);
            void* node = pane ? BpObject_GetOrCreateNamedGroup(pane, (const char*)portraitNameAddress) : 0;
            if (node != 0) {
                int portraitX = caddiePortraitX;
                unsigned int biasedPortraitX = (unsigned int)portraitX ^ 0x80000000;
                union {
                    double d;
                    unsigned int u[2];
                } integerToDouble;
                integerToDouble.u[PPC_DW_HI] = 0x43300000;
                integerToDouble.u[PPC_DW_LO] = biasedPortraitX;
                float portraitXFloat = (float)(integerToDouble.d - rowYOrConversionBias);
                float sourcePosition[2];
                sourcePosition[0] = portraitXFloat;
                sourcePosition[1] = rowOriginOrConversionBias;
                UIGroup_SetSourcePosition(node, sourcePosition);
            }
            Layout_SetElementPlayerFlag(layout, i + TourElement_CaddieOverlayFirst, 1, 5);
        } else {
            if (Sing_805DC5F0() == 0) {
                if (!s_bpCtrlInitialized) {
                    ((CBpCtrl*)s_bpCtrl)->__ct();
                    __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, s_bpCtrlDtorRegistration_tourWindow);
                    s_bpCtrlInitialized = 1;
                }
                Sing_805DC5F0() = s_bpCtrl;
            }
            int portraitNameAddress = *portraitNames;
            void* pane = IdMap_Lookup(Sing_805DC5F0(), bpEntryId);
            void* node = pane ? BpObject_GetOrCreateNamedGroup(pane, (const char*)portraitNameAddress) : 0;
            if (node != 0) {
                float sourcePosition[2];
                sourcePosition[0] = 0.0f;
                sourcePosition[1] = 0.0f;
                UIGroup_SetSourcePosition(node, sourcePosition);
            }
            Layout_SetElementPlayerFlag(layout, i + TourElement_CaddieOverlayFirst, 0, 5);
        }
        portraitNames++;
        caddiePortraitX += kTourPortraitStep;
        caddieEquipmentOffset += SavedCaddieEquipmentStride;
    }

    // Phase 4: compute saved-selection stats, start the default BP script, then enter Selecting.
    signed char equippedStats[3];
    equippedStats[0] = 0;
    equippedStats[1] = 0;
    equippedStats[2] = 0;
    CSavedStatCalculator* statCalculator = new CSavedStatCalculator(playerRecord);
    PlayerStats_ComputeEquippedAttributeTotals(statCalculator, equippedStats);
    SetThreeStatBars((signed char)equippedStats[0], (signed char)equippedStats[1], (signed char)equippedStats[2]);
    if (statCalculator != 0)
        __dl__FPv(statCalculator);

    if (Sing_805DC5F0() == 0) {
        if (!s_bpCtrlInitialized) {
            ((CBpCtrl*)s_bpCtrl)->__ct();
            __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, s_bpCtrlDtorRegistration_tourWindow);
            s_bpCtrlInitialized = 1;
        }
        Sing_805DC5F0() = s_bpCtrl;
    }
    void* pane = IdMap_Lookup(Sing_805DC5F0(), bpEntryId);
    BpObject_StartScriptByName(pane, (void*)(tourWindowStrings.defaultSelectionScript), 1);

    if (Sing_805DC608() == 0) {
        if (!s_fadeOverlayInitialized) {
            FadeOverlay_Init(s_fadeOverlay);
            __register_global_object(s_fadeOverlay, (void*)ResourceState_Dtor, s_fadeOverlayDtorRegistration_Tour);
            s_fadeOverlayInitialized = 1;
        }
        Sing_805DC608() = s_fadeOverlay;
    }
    FadeState_StartFadeOut(Sing_805DC608(), 8);

    ((TourScreenState*)this)->state = TourScreen_Selecting;
}

} // extern "C"

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU804625A8", _ctors_u802FBA74_0, (void (*)())TourWindow_InitializeFullTurnConstant);
#pragma pop

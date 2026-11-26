// Options dialog: the control mode choice (button or swing), audio output and volume settings,
// and the save-record fields they write. Also the tour map preview setup.

#include <decomp/static_init.h>
#include <decomp/local_class.h>
// Options / saved Control Mode defaults (0x802E9EE8), plus the Tour context init.
// - Button default: COptionDialog::UpdateAndProcessInput, ResetOptions case. Out-of-match only,
//   it clears BOTH high bits using kKeepNonExtendedOptions (Swing Righty).
//   A future mod must also consider SaveData_InitBlankRecord (save_data_records_and_defaults.cpp,
//   site 0x80073768); a reset action and creation of a new user are separate paths.
// - Save word: record+0x4E8, copied to dialog+0x50C. Bit31=Button (takes priority),
//   bit30=Lefty, bits28..29=audio output (0 Mono, 1 Stereo, 2 Dolby Pro Logic II).
//   ExtendedChoice0/1/2 are Swing Righty / Swing Lefty / Button, with odd caption IDs.
//   below agree. Preserve all unrelated save bits and the complete 0x828-byte copy.
// - BGM/SE: streamVolumePercent / bufferVolumePercent at record+0x4E0/+0x4E4;
//   kDefaultVolumePercent=80 and kMaxVolumePercent=100. Mixer receives percent/100.
// - Repeat pacing: kOptionSelectionDelaySeconds=0.1, volume repeat=0.02 in
//   the data section. Each held tick changes one percentage point; the countdown
//   returns early and carries no extra repeats after a large delta.
// - UI masks: same companion's double kOptionVolumeMaskUnitsPerPercent=2.55;
//   mask X is round(2.55*(100-volume)), a dimensionless scale in [0,255],
//   not screen width. UIGroup_SetScale (ui_group_and_bp_edit_sprite.cpp) stores scale*1024 in its items.
// - Tour model/course context: ModeRecordUiContext_Initialize only seeds borrowed
//   save/progress pointers and empty handles; resource loading lives in tour_character_map_board.cpp.
// COptionDialog methods require a live receiver and initialized BP/layout where
// unguarded. Texture, layout and saveMenu are owned; BP objects are borrowed by ID.
// Cancel previews saved audio without committing. Apply copies the entire scratch
// save record, then opens a NAND save modal for non-shared users. Selection alone
// never persists. Safe test cases: normal/shared records, in/out-of-match Options,
// Button with either hand bit, cancel after Default, and volume 0/100 endpoints.
struct SavedCharacterModeRecord {
    unsigned char progress[0x4A];
};
// Saved per-character progress is borrowed by the tour-map initializer; no copy/retain.
struct SavedModeDataView {
    SavedCharacterModeRecord characters[11];
    unsigned char characterOrder[8];
    signed char orderSelectionIndex;
};
struct SavedRecordModeView {
    char playerData[0x4EC];
    SavedModeDataView modeData;
};
// These words belong to the 0x828-byte player save record, not a UI object.
struct SavedAudioOptionsView {
    char playerData[0x4E0];
    int streamVolumePercent;
    int bufferVolumePercent;
    unsigned int optionFlags;
};
typedef char SavedAudioOptions_flags_offset[((int)&((SavedAudioOptionsView*)0)->optionFlags == 0x4E8) ? 1 : -1];
typedef char
    SavedRecordMode_selection_offset[((int)&((SavedRecordModeView*)0)->modeData.orderSelectionIndex == 0x822) ? 1 : -1];
// Layout map: 0..3 are independent stream/buffer decrement/increment buttons.
// IDs 4..19 use even control/odd caption pairs. Standard mode stops at ID 13.
// Value states: 0 normal, 2 selected/pressed, 3 volume limit reached.
// Output codes 0/1/2 are Mono/Stereo/Dolby Pro Logic II. ExtendedChoice0/1/2
// are Swing Righty/Swing Lefty/Button (legacy local names kept beside the layout map).
enum OptionElement {
    StreamDown = 0,
    StreamUp = 1,
    BufferDown = 2,
    BufferUp = 3,
    ApplyOptions = 4,
    ApplyOptionsLabel = 5,
    ResetOptions = 6,
    ResetOptionsLabel = 7,
    OutputMode0 = 8,
    OutputMode0Label = 9,
    OutputMode1 = 10,
    OutputMode1Label = 11,
    OutputMode2 = 12,
    OutputMode2Label = 13,
    ExtendedChoice0 = 14,
    ExtendedChoice0Label = 15,
    ExtendedChoice1 = 16,
    ExtendedChoice1Label = 17,
    ExtendedChoice2 = 18,
    ExtendedChoice2Label = 19,
    StandardOptionElementCount = 14,
    ExtendedOptionElementCount = 20
};
// Bit 31 chooses Button; bit 30 chooses Lefty only when Button is clear.
// Keep both masks coordinated with the separate blank-record initializer.
static const unsigned kExtendedChoice2Mask = 0x80000000u;
static const unsigned kExtendedChoice1Mask = 0x40000000u;
static const unsigned kKeepNonExtendedOptions = 0x3FFFFFFFu;
enum {
    kDefaultVolumePercent = 80,
    kMaxVolumePercent = 100,
    kConfirmInputAction = 4,
    kCancelInputAction = 6,
    kLayoutSelected = 2,
    kLayoutDisabled = 3,
    kLayoutInputFlagSelector = 5,
    kAllPlayers = -1
};
struct OptionResourceStrings {
    char archive[11];
    char outerLayout[15];
    char innerLayout[14];
    char outerAnimation[15];
    char innerAnimation[14];
    char outerScript[11];
    char innerScript[10];
    char cancelSound[23];
    char volumeSound[14];
    char streamVolumeMask[16];
    char bufferVolumeMask[16];
};

#include "classes/OpenItemMenuState.h"
// Preserve the initializer's integer BP IDs and typed preview-tree root.
// Using the canonical menu view directly swaps two registers in this leaf function.
struct TourMapPreviewTree {
    int header;
    int root;
    int* end;
};
struct TourMapInitializationView {
    void* mapFile;
    void* itemFile;
    void* messageFile;
    void* cameraController;
    TourMapPreviewTree previews;
    void* tourWindow;
    void* tourHintWindow;
    void* tourVsWindow;
    void* particleEffects;
    void* messageResourceHolder;
    void* flashMemorySaveMenu;
    void* mapData;
    void* coinItemData;
    void* eventResource;
    void* shop;
    void* numberDraw;
    void* menuContext;
    void* tourProgress;
    void* selectionRecord;
    void* packedResources[4]; // map file, item file, map texture, item texture
    char reserved64[8];
    int modeGroup;
    int itemGroup;
    int numberGroup;
    void* layout;
    int state;
    int pendingState;
    int windowState;
    float cameraOffsetX;
    float cameraOffsetY;
    unsigned char tourCharacterIndex; // Character-table index, also used to select that character's map resources.
    char reserved91[7];
    short initialSelection;
};
typedef char TourMapInitializationView_tree_offset[(int)&((TourMapInitializationView*)0)->previews == 0x10 ? 1 : -1];
typedef char TourMapInitializationView_context_offset
    [(int)&((TourMapInitializationView*)0)->menuContext == (int)&((OpenItemMenuState*)0)->menuContext ? 1 : -1];
typedef char TourMapInitializationView_group_offset
    [(int)&((TourMapInitializationView*)0)->modeGroup == (int)&((OpenItemMenuState*)0)->modeGroup ? 1 : -1];
typedef char TourMapInitializationView_mode_offset[(int)&((TourMapInitializationView*)0)->tourCharacterIndex ==
                                                           (int)&((OpenItemMenuState*)0)->tourCharacterIndex
                                                       ? 1
                                                       : -1];
enum {
    kTourMapLoadArchives = 1,
    kInitialTourMapWindowState = 4
};
#include "classes/CBpCtrlHandle.h"

#include "game/singletons.h"
#include "sdk_protos.h"
#include "classes/COptionDialog.h"
#include <decomp/cw_bitfields.h>
#include "classes/CFlashMemoryMenuSave.h"
#include "classes/CPackTextureResource.h"
#include "classes/CBpCtrl.h"

// View the existing storage as COptionDialog without transferring ownership.
static inline ::COptionDialog* AsCOptionDialog(const volatile void* p) {
    return (::COptionDialog*)p;
}
enum {
    kOptionRecordOffset = (int)&((::COptionDialog*)0)->savedRecordCopy
};

extern "C" {

extern float kFloat_6_2831855;
extern float s_optionSoundFullTurnAngleStorage;
extern float kFloat_1_OptionMenu;
extern float kFloat_0_OptionDialog;
extern float kFloat_0_5_OptionSound;
extern float kFloat_200_OptionSound;
extern float kOptionSelectionDelaySeconds;
extern float kOptionVolumeRepeatDelaySeconds;
extern signed char s_fadeOverlayInitialized;
extern signed char s_inputManagerInitialized;
extern void* g_soundManager;
extern unsigned char s_fadeOverlay[];
extern unsigned char s_gameInputManager[];
extern float kFloat_100_OptionDialog;
extern double kOptionVolumeMaskUnitsPerPercent;
extern float kFloat_0_OpenItemMenu;
extern signed char s_bpCtrlInitialized;
extern signed char s_bpCtrlHandleInitialized;
extern int s_bpCtrlHandle[2];
extern signed char s_saveDataInitialized;
extern unsigned char s_bpCtrl[];
extern unsigned char s_saveData[];
extern unsigned char s_bpCtrlDtorRegistration_80519F20[];
extern unsigned char __vt__13COptionDialog[];
extern unsigned char s_bpCtrlHandleDtorRegistration_OptionMenu[];
extern unsigned char optionMenuStrings[];
extern int kOptionVolumeMaskInitialXBits;
extern int kOptionVolumeMaskInitialYBits;
extern int optionVolumeButtonIds[4];

void Layout_Update(void*);
void __ct__7CBpCtrlFv(void*);
void SaveData_Ctor(void*);
void* SaveData_GetCurrentRecord(void*);
void BpObject_StartScriptByName(void*, const char*, int);
void BpObject_TryInvokeCurrentCallback(void*, int);
void Layout_ReadElementRect(void*, int, int*);
void Layout_WriteElementRect(void*, int, int*);
void Layout_SetElementSecondaryState(void*, int, int);
void CBpCtrl_dtor(void);
int MPBase1_Process(void*, float);
void BpObject_TryInvokeCurrentCallback(void*, int);
void* IdMap_Lookup(void*, void*);
void __ct__7CBpCtrlFv(void*);
void CBpCtrl_dtor(void);
void FadeOverlay_Init(void*);
void ResourceState_Dtor(void);
void FadeState_StartFadeOut(void*, int);
void InputManager_Init(void*);
void InputManager_Dtor(void);
int Input_IsActionPressed(void*, int, int);
int Input_IsActionButtonDown(void*, int, int);
void Sound_PlayByName(void*, const char*, int, int, const char*, int, float, float, float);
void SoundManager_ApplySavedAudioSettings(void*, float);
int Layout_CollectActivated(void*, void*, int);
void Layout_Update(void*);
void Layout_SetElementValueState(void*, int, int);
void Layout_SetElementPlayerFlag(void*, int, int, int);
void Layout_SetElementFeedbackState(void*, int, int);
void SaveData_SetCurrentRecord(void*, void*);
int SaveData_IsSharedRecordMode(void*);
void appFreeUni__FPv(void*);
int InputManager_GetSelectedPlayer(void*);
int Layout_HitTest(void*, int, int);
int SoundManager_IsAliasPrefixOrStreamStemActive(void*, const char*);
void SoundManager_SetOutputMode(void*, int);
void SoundManager_SetStreamVolume(void*, float, float);
void SoundManager_SetBufferVolume(void*, float, float);
void COptionDialog_UpdateVolumeControls(void*);
void CBpCtrl_dtor(void);
void CBpCtrlHandle_Dtor(void);
void* IdMap_Lookup(void*, void*);
void BpObject_Draw(void*);
void Layout_DrawWithOpacity(void*, float);
void MPBase1_DrawIfInitialized(void*);
int Layout_PollLoadComplete(void*);
void Layout_Destroy(void*, int);
void __dl__FPv(void*);
void Layout_SetElementValueState(void*, int, int);
void Layout_SetElementFeedbackState(void*, int, int);
void Layout_SetElementPlayerFlagsAndState(void*, int, int);
void Layout_SetElementPlayerFlag(void*, int, int, int);
void* BpObject_GetOrCreateNamedGroup(void*, const char*);
int UIGroup_SetScale(void*, float*);
double Math_RoundDoubleHalfAwayFromZero(double); // Round to nearest, with half values rounded away from zero.
}
// Adapt a no-argument destructor callback to the runtime registration overload.
inline void* __register_global_object(void* object, void (*destructor)(), void* regmem) {
    return __register_global_object(object, (void*)destructor, regmem);
}
extern "C" {

// Highlight one of three extended choices from the saved high option bits.
// Controls the out-of-match Control Mode row; Button takes precedence over Lefty.
// saveRecord points to a complete 0x828-byte save record, not its option-flags word.
// Bit 31 comes from saveRecord; bit 30 comes from this dialog. Current callers pass the dialog's copied record.
void COptionDialog::RefreshControlModeSelection(void* saveRecord) {
    if (controlModeOptionsVisible != 0) {
        int i;
        for (i = ExtendedChoice0; i <= ExtendedChoice2Label; ++i) {
            Layout_SetElementValueState(layout, i, 0);
            Layout_SetElementFeedbackState(layout, i, 1);
            Layout_SetElementPlayerFlagsAndState(layout, i, 1);
        }
        if ((((SavedAudioOptionsView*)saveRecord)->optionFlags >> 31) == 0) {
            if (((optionFlags >> 30) & 1) != 0) {
                Layout_SetElementValueState(layout, ExtendedChoice1, 2);
                Layout_SetElementValueState(layout, ExtendedChoice1Label, 2);
                Layout_SetElementPlayerFlag(layout, ExtendedChoice1, 0, 5);
                Layout_SetElementPlayerFlag(layout, ExtendedChoice1Label, 0, 5);
            } else {
                Layout_SetElementValueState(layout, ExtendedChoice0, 2);
                Layout_SetElementValueState(layout, ExtendedChoice0Label, 2);
                Layout_SetElementPlayerFlag(layout, ExtendedChoice0, 0, 5);
                Layout_SetElementPlayerFlag(layout, ExtendedChoice0Label, 0, 5);
            }
        } else {
            Layout_SetElementValueState(layout, ExtendedChoice2, 2);
            Layout_SetElementValueState(layout, ExtendedChoice2Label, 2);
            Layout_SetElementPlayerFlag(layout, ExtendedChoice2, 0, 5);
            Layout_SetElementPlayerFlag(layout, ExtendedChoice2Label, 0, 5);
        }
    }
}

// Highlight the selected audio output mode stored in option bits 28 and 29.
// saveRecord is a full save record; mode 3 clears highlights without selecting any output button.
void COptionDialog::RefreshAudioOutputModeButtons(void* saveRecord) {
    int i;
    for (i = OutputMode0; i <= OutputMode2Label; ++i) {
        Layout_SetElementValueState(layout, i, 0);
        Layout_SetElementFeedbackState(layout, i, 1);
        Layout_SetElementPlayerFlagsAndState(layout, i, 1);
    }
    unsigned mode = (((SavedAudioOptionsView*)saveRecord)->optionFlags >> 28) & 3;
    switch (mode) {
    case 0:
        Layout_SetElementValueState(layout, OutputMode0, 2);
        Layout_SetElementValueState(layout, OutputMode0Label, 2);
        Layout_SetElementPlayerFlag(layout, OutputMode0, 0, 5);
        Layout_SetElementPlayerFlag(layout, OutputMode0Label, 0, 5);
        break;
    case 1:
        Layout_SetElementValueState(layout, OutputMode1, 2);
        Layout_SetElementValueState(layout, OutputMode1Label, 2);
        Layout_SetElementPlayerFlag(layout, OutputMode1, 0, 5);
        Layout_SetElementPlayerFlag(layout, OutputMode1Label, 0, 5);
        break;
    case 2:
        Layout_SetElementValueState(layout, OutputMode2, 2);
        Layout_SetElementValueState(layout, OutputMode2Label, 2);
        Layout_SetElementPlayerFlag(layout, OutputMode2, 0, 5);
        Layout_SetElementPlayerFlag(layout, OutputMode2Label, 0, 5);
        break;
    }
}

// Initialize this module's cached 2-pi constant during static initialization.
void OptionMenu_InitFullTurnConstant(void) {
    s_optionSoundFullTurnAngleStorage = kFloat_6_2831855;
}

struct OptionActivated {
    int* begin;
    unsigned count;
    unsigned cap;
    // Initialize an empty owned array of activated layout IDs.
    OptionActivated() : begin(0), count(0), cap(0) {}
    // Release the owned activation array.
    ~OptionActivated() {
        if (begin) {
            count -= ((char*)begin + count) - (char*)begin;
            appFreeUni__FPv(begin);
        }
    }
};

struct OptionFlags {
    CW_BITS3(unsigned first : 2, unsigned mode : 2, unsigned rest : 28)
};
struct OptionOwned {
    virtual ~OptionOwned();
};
struct OptionRegistration {
    int control;
    char pad04[0x14];
    char input[0x18];
    char dialog[0xC];
};
struct OptionStrings {
    char prefix[0x71];
    char volumeSound[14];
};

// Handle apply/reset, audio-mode selection, held volume controls, and the modal save menu.
// dt and inputDelay use seconds. Returns 0 while active; 1 closes the caller's dialog.
// Audio changes preview immediately. Apply copies the full save slot; cancel restores saved audio.
int COptionDialog::UpdateAndProcessInput(float dt) {
    OptionRegistration& regmem = *(OptionRegistration*)s_bpCtrlDtorRegistration_80519F20;
    unsigned eventIndex;
    int* volumeButton;
    int changed;
    unsigned volumeButtonIndex;
    int index;
    unsigned char* p = (unsigned char*)this;
    OptionStrings* strings;
    if (!Sing_805DC5F0()) {
        if (!s_bpCtrlInitialized) {
            ((CBpCtrl*)s_bpCtrl)->__ct();
            __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, &regmem.control);
            s_bpCtrlInitialized = 1;
        }
        Sing_805DC5F0() = s_bpCtrl;
    }
    BpObject_TryInvokeCurrentCallback(IdMap_Lookup(Sing_805DC5F0(), (*(void**)&AsCOptionDialog(p)->bpEntryId)), 0);
    // Phase 1: an active save menu owns input until it finishes.
    if (AsCOptionDialog(p)->saveMenu) {
        if (MPBase1_Process(AsCOptionDialog(p)->saveMenu, dt)) {
            OptionOwned* obj = (*(OptionOwned**)&AsCOptionDialog(p)->saveMenu);
            if (obj) {
                delete obj;
                AsCOptionDialog(p)->saveMenu = 0;
            }
            if (!Sing_805DC608()) {
                if (!s_fadeOverlayInitialized) {
                    FadeOverlay_Init(s_fadeOverlay);
                    __register_global_object(s_fadeOverlay, (void*)ResourceState_Dtor, regmem.dialog);
                    s_fadeOverlayInitialized = 1;
                }
                Sing_805DC608() = s_fadeOverlay;
            }
            FadeState_StartFadeOut(Sing_805DC608(), 0);
            return 1;
        }
        return 0;
    }
    // Phase 2: expire selection/repeat feedback before processing a new layout activation.
    if (AsCOptionDialog(p)->inputDelay > 0.0f) {
        AsCOptionDialog(p)->inputDelay -= dt;
        if (AsCOptionDialog(p)->inputDelay > 0.0f)
            return 0;
        if (AsCOptionDialog(p)->closePending)
            return 1;
        ((void (*)(void*, void*))COptionDialog_UpdateVolumeControls)(this, p + kOptionRecordOffset);
        Layout_SetElementValueState(AsCOptionDialog(p)->layout, ResetOptions, 0);
        Layout_SetElementValueState(AsCOptionDialog(p)->layout, ResetOptionsLabel, 0);
        Layout_SetElementPlayerFlag(AsCOptionDialog(p)->layout, ResetOptions, 1, 5);
        Layout_SetElementPlayerFlag(AsCOptionDialog(p)->layout, ResetOptionsLabel, 1, 5);
    }
    if (!GetInputManagerSlot()) {
        if (!s_inputManagerInitialized) {
            InputManager_Init(s_gameInputManager);
            __register_global_object(s_gameInputManager, (void*)InputManager_Dtor, regmem.input);
            s_inputManagerInitialized = 1;
        }
        GetInputManagerSlot() = s_gameInputManager;
    }
    if (Input_IsActionPressed(GetInputManagerSlot(), kCancelInputAction, kAllPlayers)) {
        Sound_PlayByName(g_soundManager, ((OptionResourceStrings*)optionMenuStrings)->cancelSound, 0, 0, 0, 0,
                         kFloat_0_5_OptionSound, kFloat_200_OptionSound, kFloat_1_OptionMenu);
        SoundManager_ApplySavedAudioSettings(g_soundManager, kFloat_1_OptionMenu);
        return 1;
    }
    Layout_Update(AsCOptionDialog(p)->layout);
    changed = 0;
    OptionActivated activated;
    // Phase 3: process apply/reset and output/extended-choice buttons.
    if (Layout_CollectActivated(AsCOptionDialog(p)->layout, &activated, 0)) {
        float delay = kOptionSelectionDelaySeconds;
        for (eventIndex = 0, index = 0; eventIndex < activated.count; ++eventIndex, index += 4) {
            int event = *(int*)((char*)activated.begin + index);
            switch (event) {
            case ApplyOptions:
                Layout_SetElementValueState(AsCOptionDialog(p)->layout, ApplyOptions, 2);
                Layout_SetElementValueState(AsCOptionDialog(p)->layout, ApplyOptionsLabel, 2);
                {
                    unsigned char controlModeOptionsVisible = AsCOptionDialog(p)->controlModeOptionsVisible;
                    int limit = ExtendedOptionElementCount;
                    if (!controlModeOptionsVisible)
                        limit = StandardOptionElementCount;
                    for (int i = 0; i < limit; ++i) {
                        Layout_SetElementPlayerFlag(AsCOptionDialog(p)->layout, i, 0, 5);
                    }
                    if (!GetSaveDataSlot()) {
                        if (!s_saveDataInitialized) {
                            SaveData_Ctor(s_saveData);
                            s_saveDataInitialized = 1;
                        }
                        GetSaveDataSlot() = s_saveData;
                    }
                    SaveData_SetCurrentRecord(GetSaveDataSlot(), p + kOptionRecordOffset);
                    if (!GetSaveDataSlot()) {
                        if (!s_saveDataInitialized) {
                            SaveData_Ctor(s_saveData);
                            s_saveDataInitialized = 1;
                        }
                        GetSaveDataSlot() = s_saveData;
                    }
                    if (SaveData_IsSharedRecordMode(GetSaveDataSlot())) {
                        // This stores close-delay state but still returns 1 below in
                        // this call; preserve that existing immediate completion.
                        float closeDelay = kOptionSelectionDelaySeconds;
                        AsCOptionDialog(p)->closePending = 1;
                        AsCOptionDialog(p)->inputDelay = closeDelay;
                    } else {
                        AsCOptionDialog(p)->saveMenu = CFlashMemoryMenuSave::Create();
                        return 0;
                    }
                    return 1;
                }
            case ResetOptions: {
                // Retail Default: Swing Righty outside a match, Stereo, BGM/SE 80%.
                // In-match sound-only Options preserves both Control Mode/hand bits.
                if (AsCOptionDialog(p)->controlModeOptionsVisible) {
                    AsCOptionDialog(p)->optionFlags &= kKeepNonExtendedOptions;
                }
                (*(unsigned int*)&AsCOptionDialog(p)->bufferVolumePercent) = kDefaultVolumePercent;
                (*(unsigned int*)&AsCOptionDialog(p)->streamVolumePercent) = kDefaultVolumePercent;
                ((OptionFlags*)(&AsCOptionDialog(p)->optionFlags))->mode = 1;
                changed = 1;
                RefreshAudioOutputModeButtons(p + kOptionRecordOffset);
                ((void (*)(void*, void*))COptionDialog_UpdateVolumeControls)(this, p + kOptionRecordOffset);
                RefreshControlModeSelection(p + kOptionRecordOffset);
                Layout_SetElementValueState(AsCOptionDialog(p)->layout, ResetOptions, 2);
                Layout_SetElementValueState(AsCOptionDialog(p)->layout, ResetOptionsLabel, 2);
                Layout_SetElementPlayerFlag(AsCOptionDialog(p)->layout, ResetOptions, 0, 5);
                Layout_SetElementPlayerFlag(AsCOptionDialog(p)->layout, ResetOptionsLabel, 0, 5);
                AsCOptionDialog(p)->inputDelay = delay;
                break;
            }
            case OutputMode0: {
                ((OptionFlags*)(&AsCOptionDialog(p)->optionFlags))->mode = 0;
                changed = 1;
                RefreshAudioOutputModeButtons(p + kOptionRecordOffset);
                break;
            }
            case OutputMode1: {
                ((OptionFlags*)(&AsCOptionDialog(p)->optionFlags))->mode = 1;
                changed = 1;
                RefreshAudioOutputModeButtons(p + kOptionRecordOffset);
                break;
            }
            case OutputMode2: {
                ((OptionFlags*)(&AsCOptionDialog(p)->optionFlags))->mode = 2;
                changed = 1;
                RefreshAudioOutputModeButtons(p + kOptionRecordOffset);
                break;
            }
            case ExtendedChoice1: {
                unsigned optionBits = AsCOptionDialog(p)->optionFlags;
                optionBits = (optionBits & 0x7FFFFFFF) | kExtendedChoice1Mask;
                AsCOptionDialog(p)->optionFlags = optionBits;
                RefreshControlModeSelection(p + kOptionRecordOffset);
                break;
            }
            case ExtendedChoice0: {
                unsigned optionBits = AsCOptionDialog(p)->optionFlags;
                optionBits &= kKeepNonExtendedOptions;
                AsCOptionDialog(p)->optionFlags = optionBits;
                RefreshControlModeSelection(p + kOptionRecordOffset);
                break;
            }
            case ExtendedChoice2: {
                unsigned optionBits = AsCOptionDialog(p)->optionFlags;
                optionBits = (optionBits | kExtendedChoice2Mask) & ~kExtendedChoice1Mask;
                AsCOptionDialog(p)->optionFlags = optionBits;
                RefreshControlModeSelection(p + kOptionRecordOffset);
                break;
            }
            default:
                break;
            }
        }
    }
    // Repeat the four volume controls while the input action is held.
    struct FourInts {
        int values[4];
    };
    strings = (OptionStrings*)optionMenuStrings;
    FourInts vals = *(FourInts*)optionVolumeButtonIds;
    volumeButton = vals.values;
    float repeatDelay = kOptionVolumeRepeatDelaySeconds;
    for (volumeButtonIndex = 0; volumeButtonIndex < 4; ++volumeButtonIndex, ++volumeButton) {
        index = *volumeButton;
        int& idx = index;
        if (!GetInputManagerSlot()) {
            if (!s_inputManagerInitialized) {
                InputManager_Init(s_gameInputManager);
                __register_global_object(s_gameInputManager, (void*)InputManager_Dtor, regmem.input);
                s_inputManagerInitialized = 1;
            }
            GetInputManagerSlot() = s_gameInputManager;
        }
        void* mgr = (void*)InputManager_GetSelectedPlayer(GetInputManagerSlot());
        if (Layout_HitTest(AsCOptionDialog(p)->layout, idx, (int)mgr)) {
            if (!GetInputManagerSlot()) {
                if (!s_inputManagerInitialized) {
                    InputManager_Init(s_gameInputManager);
                    __register_global_object(s_gameInputManager, (void*)InputManager_Dtor, regmem.input);
                    s_inputManagerInitialized = 1;
                }
                GetInputManagerSlot() = s_gameInputManager;
            }
            if (Input_IsActionButtonDown(GetInputManagerSlot(), kConfirmInputAction, -1)) {
                switch (idx) {
                case StreamDown: {
                    int v = AsCOptionDialog(p)->streamVolumePercent;
                    AsCOptionDialog(p)->streamVolumePercent = v - 1;
                    break;
                }
                case StreamUp: {
                    int v = AsCOptionDialog(p)->streamVolumePercent;
                    AsCOptionDialog(p)->streamVolumePercent = v + 1;
                    break;
                }
                case BufferDown: {
                    int v = AsCOptionDialog(p)->bufferVolumePercent;
                    AsCOptionDialog(p)->bufferVolumePercent = v - 1;
                    break;
                }
                case BufferUp: {
                    int v = AsCOptionDialog(p)->bufferVolumePercent;
                    AsCOptionDialog(p)->bufferVolumePercent = v + 1;
                    break;
                }
                }
                int ret = SoundManager_IsAliasPrefixOrStreamStemActive(g_soundManager, strings->volumeSound);
                if (!ret) {
                    Sound_PlayByName(g_soundManager, strings->volumeSound, 0, 0, strings->volumeSound, 0,
                                     kFloat_0_5_OptionSound, kFloat_200_OptionSound, kFloat_1_OptionMenu);
                }
                AsCOptionDialog(p)->inputDelay = repeatDelay;
                changed = 1;
                Layout_SetElementValueState(AsCOptionDialog(p)->layout, idx, 2);
                Layout_SetElementFeedbackState(AsCOptionDialog(p)->layout, idx, 2);
            }
        }
    }
    // Phase 5: push changed output/volume settings to the live mixer (percent -> [0,1]).
    // Extended-choice edits do not set changed; they are persisted by Apply instead.
    if (changed) {
        unsigned optionBits = AsCOptionDialog(p)->optionFlags;
        SoundManager_SetOutputMode(g_soundManager, (optionBits >> 28) & 3);
        SoundManager_SetStreamVolume(g_soundManager,
                                     (float)AsCOptionDialog(p)->streamVolumePercent / kFloat_100_OptionDialog,
                                     kFloat_0_OptionDialog);
        SoundManager_SetBufferVolume(g_soundManager,
                                     (float)AsCOptionDialog(p)->bufferVolumePercent / kFloat_100_OptionDialog,
                                     kFloat_0_OptionDialog);
    }
    return 0;
}

#pragma push
#pragma opt_common_subs off
// Copy the active save record into the dialog and initialize button and caption state.
// Initialization snapshots the entire save slot so Apply can preserve unrelated fields.
void COptionDialog::InitializeFromSaveData() {
    // Phase 1: discard the old texture request and snapshot the active save slot.
    void* owned = textureResource;
    if (owned != 0) {
        if (owned != 0) {
            struct VBase {
                virtual void destroy(int) DECOMP_LOCAL_VIRTUAL;
            };
            ((VBase*)owned)->destroy(1);
        }
        textureResource = 0;
    }
    void* state = GetSaveDataSlot();
    if (state == 0) {
        if (!s_saveDataInitialized) {
            SaveData_Ctor(s_saveData);
            s_saveDataInitialized = 1;
        }
        GetSaveDataSlot() = s_saveData;
    }
    void* source = SaveData_GetCurrentRecord(GetSaveDataSlot());
    // Preserve the original copy grouping for the complete 0x828-byte save slot.
    struct Settings {
        unsigned first[151];
        unsigned second[28];
        unsigned short third[208];
        unsigned words[16];
        unsigned char bytes0[24];
        unsigned char byte24, byte25, byte26, byte27, byte28, byte29;
        unsigned char bytes1[8];
        unsigned char byte38, byte39, byte40;
        unsigned setting[5];
        unsigned short final[414];
    };
    *(Settings*)savedRecordCopy = *(Settings*)source;
    // Phase 2: refresh controls from the snapshot, then select inner/outer BP script.
    RefreshAudioOutputModeButtons(savedRecordCopy);
    ((void (*)(void*, void*))COptionDialog_UpdateVolumeControls)(this, savedRecordCopy);
    RefreshControlModeSelection(savedRecordCopy);
    void* manager = Sing_805DC5F0();
    if (!manager) {
        if (!s_bpCtrlInitialized) {
            ((CBpCtrl*)s_bpCtrl)->__ct();
            __register_global_object(s_bpCtrl, CBpCtrl_dtor, s_bpCtrlDtorRegistration_80519F20);
            s_bpCtrlInitialized = 1;
        }
        Sing_805DC5F0() = s_bpCtrl;
    }
    int index;
    int limit;
    void* obj = IdMap_Lookup(Sing_805DC5F0(), (*(void**)&bpEntryId));
    if (controlModeOptionsVisible != 0) {
        BpObject_StartScriptByName(obj, ((OptionResourceStrings*)optionMenuStrings)->outerScript, 1);
    } else {
        BpObject_StartScriptByName(obj, ((OptionResourceStrings*)optionMenuStrings)->innerScript, 1);
    }
    BpObject_TryInvokeCurrentCallback(obj, 0);
    limit = ExtendedOptionElementCount;
    if (controlModeOptionsVisible == 0)
        limit = StandardOptionElementCount;
    int values[4] = {};
    // Phase 3: copy control rectangles to captions; both share hit/feedback geometry.
    for (index = 4; index < limit; index += 2) {
        Layout_ReadElementRect(layout, index, values);
        Layout_WriteElementRect(layout, index + 1, values);
    }
    Layout_SetElementSecondaryState(layout, 1, 0);
    Layout_SetElementSecondaryState(layout, 0, 0);
    Layout_SetElementSecondaryState(layout, 3, 0);
    Layout_SetElementSecondaryState(layout, 2, 0);
    void* entries = Sing_805DC5F8();
    if (!entries) {
        if (!s_bpCtrlHandleInitialized) {
            s_bpCtrlHandle[0] = -1;
            s_bpCtrlHandle[1] = -1;
            __register_global_object(s_bpCtrlHandle, CBpCtrlHandle_Dtor, s_bpCtrlHandleDtorRegistration_OptionMenu);
            s_bpCtrlHandleInitialized = 1;
        }
        Sing_805DC5F8() = s_bpCtrlHandle;
    }
    entries = Sing_805DC5F8();
    bool missingEntries = entries == 0;
    previousScriptIndex = ((CBpCtrlHandle*)entries)->scriptIndex;
    if (missingEntries) {
        if (!s_bpCtrlHandleInitialized) {
            s_bpCtrlHandle[0] = -1;
            s_bpCtrlHandle[1] = -1;
            __register_global_object(s_bpCtrlHandle, CBpCtrlHandle_Dtor, s_bpCtrlHandleDtorRegistration_OptionMenu);
            s_bpCtrlHandleInitialized = 1;
        }
        Sing_805DC5F8() = s_bpCtrlHandle;
    }
    ((CBpCtrlHandle*)Sing_805DC5F8())->ReleaseEntry();
    closePending = 0;
}

#pragma pop
// Poll the option texture and update the option layout.
void COptionDialog::UpdateLayoutAndTexture() {
    void* ptr = textureResource;
    if (ptr != 0)
        ((CPackTextureResource*)ptr)->PollLoad();
    ptr = layout;
    if (ptr != 0)
        Layout_Update(ptr);
}

// Initialize a mode-record UI context and resolve the selected character progress record.
// menuContext begins with a borrowed save-record pointer. orderSelectionIndex must index characterOrder,
// whose result must index characters[11]. +0x90 stores that character index despite its mode name.
// Required ranges: orderSelectionIndex 0..7, characterOrder value 0..10. The
// context and save record must outlive this initialized Tour object. It neither
// releases old handles nor allocates replacements: call only on fresh/cleared state.
void ModeRecordUiContext_Initialize(void* self, void* menuContext) {
    TourMapInitializationView* s = (TourMapInitializationView*)self;
    float zero = kFloat_0_OpenItemMenu;
    (*(volatile int*)&s->previews.header) = 0;
    s->previews.root = 0;
    s->previews.end = &s->previews.root;
    s->initialSelection = 0;
    s->cameraOffsetX = zero;
    s->cameraOffsetY = zero;
    s->state = kTourMapLoadArchives;
    s->pendingState = 0;
    s->windowState = kInitialTourMapWindowState;
    s->menuContext = menuContext;
    void* saveRecord = *(void**)menuContext;
    unsigned char characterIndex;
    signed char orderIndex;
    void* modeData = (void*)&((SavedRecordModeView*)saveRecord)->modeData;
    s->tourProgress = modeData;
    orderIndex = ((SavedRecordModeView*)saveRecord)->modeData.orderSelectionIndex;
    characterIndex = ((SavedModeDataView*)modeData)->characterOrder[orderIndex];
    s->tourCharacterIndex = characterIndex;
    *(void**)&s->selectionRecord =
        (char*)((SavedModeDataView*)modeData)->characters + characterIndex * sizeof(SavedCharacterModeRecord);
    *(int*)&s->eventResource = 0;
    *(int*)&s->flashMemorySaveMenu = 0;
    *(int*)&s->shop = 0;
    *(int*)&s->messageResourceHolder = 0;
    *(int*)&s->packedResources[0] = 0;
    *(int*)&s->packedResources[1] = 0;
    *(int*)&s->cameraController = 0;
    *(int*)&s->mapFile = 0;
    *(int*)&s->messageFile = 0;
    *(int*)&s->itemFile = 0;
    *(int*)&s->tourWindow = 0;
    *(int*)&s->tourHintWindow = 0;
    *(int*)&s->tourVsWindow = 0;
    *(int*)&s->particleEffects = 0;
    *(int*)&s->layout = 0;
    *(int*)&s->numberDraw = 0;
    *(int*)&s->mapData = 0;
    *(int*)&s->coinItemData = 0;
    *(int*)&s->packedResources[2] = 0;
    *(int*)&s->packedResources[3] = 0;
    s->modeGroup = -1;
    s->itemGroup = -1;
    s->numberGroup = -1;
}

// Wait for the texture, option layout, and all BP controller entries to finish loading.
int COptionDialog::IsLoaded() {
    void* ptr = textureResource;
    if (ptr != 0 && !((CPackTextureResource*)ptr)->IsLoadFinished())
        return 0;
    if (!Layout_PollLoadComplete(layout))
        return 0;
    void* state = Sing_805DC5F0();
    if (state == 0) {
        if (!s_bpCtrlInitialized) {
            ((CBpCtrl*)s_bpCtrl)->__ct();
            __register_global_object(s_bpCtrl, CBpCtrl_dtor, s_bpCtrlDtorRegistration_80519F20);
            s_bpCtrlInitialized = 1;
        }
        Sing_805DC5F0() = s_bpCtrl;
    }
    int ok = ((CBpCtrl*)Sing_805DC5F0())->PollAllEntriesLoaded();
    if (ok == 0)
        return 0;
    return 1;
}

// Release option texture, BP entry, and layout, then restore the prior caption script.
// should_delete > 0 frees self; returned address then must not be dereferenced.
// saveMenu is released by the process-completion path, not this destructor;
// preserve the caller's requirement that it has completed before dialog teardown.
COptionDialog::~COptionDialog() {
    *(void**)this = __vt__13COptionDialog;
    void* ptr = textureResource;
    if (ptr != 0) {
        if (ptr != 0) {
            struct VBase {
                virtual void call0(int) DECOMP_LOCAL_VIRTUAL;
                virtual void call1(int) DECOMP_LOCAL_VIRTUAL;
            };
            ((VBase*)ptr)->call0(1);
        }
        textureResource = 0;
    }
    if (bpEntryId != -1) {
        void* state = Sing_805DC5F0();
        if (state == 0) {
            if (!s_bpCtrlInitialized) {
                ((CBpCtrl*)s_bpCtrl)->__ct();
                __register_global_object(s_bpCtrl, CBpCtrl_dtor, s_bpCtrlDtorRegistration_80519F20);
                s_bpCtrlInitialized = 1;
            }
            Sing_805DC5F0() = s_bpCtrl;
        }
        ((CBpCtrl*)Sing_805DC5F0())->DeleteEntryById(bpEntryId);
        bpEntryId = -1;
    }
    ptr = layout;
    if (ptr != 0) {
        Layout_Destroy(ptr, 1);
        layout = 0;
    }
    void* state2 = Sing_805DC5F8();
    if (state2 == 0) {
        if (!s_bpCtrlHandleInitialized) {
            s_bpCtrlHandle[0] = -1;
            s_bpCtrlHandle[1] = -1;
            __register_global_object(s_bpCtrlHandle, (void (*)())CBpCtrlHandle_Dtor,
                                     s_bpCtrlHandleDtorRegistration_OptionMenu);
            s_bpCtrlHandleInitialized = 1;
        }
        Sing_805DC5F8() = s_bpCtrlHandle;
    }
    ((CBpCtrlHandle*)Sing_805DC5F8())->SetScriptIndex(previousScriptIndex);
}

// Disable volume buttons at their limits and scale the two volume-mask groups.
// Percent values are expected in [0,100]; this routine disables endpoints, it does not clamp.
// Mask X = round(unitsPerPercent * (100 - percent)); mask Y keeps its initial pool value.
// Both arguments are scale factors; the UI setter converts them to 1/1024 units.
void COptionDialog_UpdateVolumeControls(void* self) {
    union {
        double d;
        unsigned int u[2];
    } conv2;
    union {
        double d;
        unsigned int u[2];
    } conv;
    float maskScale[2];
    int i;
    void* obj;
    conv.u[0] = 0x43300000u;
    conv2.u[0] = 0x43300000u;
    for (i = 0; i <= 3; ++i) {
        void* layout = AsCOptionDialog(self)->layout;
        Layout_SetElementValueState(layout, i, 0);
        layout = AsCOptionDialog(self)->layout;
        Layout_SetElementFeedbackState(layout, i, 1);
        layout = AsCOptionDialog(self)->layout;
        Layout_SetElementPlayerFlagsAndState(layout, i, 1);
    }
    if (AsCOptionDialog(self)->streamVolumePercent >= kMaxVolumePercent) {
        void* layout = AsCOptionDialog(self)->layout;
        Layout_SetElementValueState(layout, StreamUp, 3);
        layout = AsCOptionDialog(self)->layout;
        Layout_SetElementPlayerFlag(layout, StreamUp, 0, 5);
    }
    if (AsCOptionDialog(self)->streamVolumePercent <= 0) {
        void* layout = AsCOptionDialog(self)->layout;
        Layout_SetElementValueState(layout, StreamDown, 3);
        layout = AsCOptionDialog(self)->layout;
        Layout_SetElementPlayerFlag(layout, StreamDown, 0, 5);
    }
    if (AsCOptionDialog(self)->bufferVolumePercent >= kMaxVolumePercent) {
        void* layout = AsCOptionDialog(self)->layout;
        Layout_SetElementValueState(layout, BufferUp, 3);
        layout = AsCOptionDialog(self)->layout;
        Layout_SetElementPlayerFlag(layout, BufferUp, 0, 5);
    }
    if (AsCOptionDialog(self)->bufferVolumePercent <= 0) {
        void* layout = AsCOptionDialog(self)->layout;
        Layout_SetElementValueState(layout, BufferDown, 3);
        layout = AsCOptionDialog(self)->layout;
        Layout_SetElementPlayerFlag(layout, BufferDown, 0, 5);
    }
    void* state = Sing_805DC5F0();
    if (state == 0) {
        if (!s_bpCtrlInitialized) {
            ((CBpCtrl*)s_bpCtrl)->__ct();
            __register_global_object(s_bpCtrl, CBpCtrl_dtor, s_bpCtrlDtorRegistration_80519F20);
            s_bpCtrlInitialized = 1;
        }
        Sing_805DC5F0() = s_bpCtrl;
    }
    obj = IdMap_Lookup(Sing_805DC5F0(), (*(void**)&AsCOptionDialog(self)->bpEntryId));
    *(int*)&maskScale[0] = kOptionVolumeMaskInitialXBits;
    *(int*)&maskScale[1] = kOptionVolumeMaskInitialYBits;
    {
        int volumePercent = AsCOptionDialog(self)->streamVolumePercent;
        float volumeAsFloat = (float)volumePercent;
        float missingPercent = kFloat_100_OptionDialog - volumeAsFloat;
        double unroundedMaskX = kOptionVolumeMaskUnitsPerPercent * (double)missingPercent;
        int roundedMaskX = (int)Math_RoundDoubleHalfAwayFromZero(unroundedMaskX);
        maskScale[0] = (float)roundedMaskX;
    }
    void* style = BpObject_GetOrCreateNamedGroup(obj, ((OptionResourceStrings*)optionMenuStrings)->streamVolumeMask);
    UIGroup_SetScale(style, maskScale);
    {
        int volumePercent = AsCOptionDialog(self)->bufferVolumePercent;
        float volumeAsFloat = (float)volumePercent;
        float missingPercent = kFloat_100_OptionDialog - volumeAsFloat;
        double unroundedMaskX = kOptionVolumeMaskUnitsPerPercent * (double)missingPercent;
        int roundedMaskX = (int)Math_RoundDoubleHalfAwayFromZero(unroundedMaskX);
        maskScale[0] = (float)roundedMaskX;
    }
    style = BpObject_GetOrCreateNamedGroup(obj, ((OptionResourceStrings*)optionMenuStrings)->bufferVolumeMask);
    UIGroup_SetScale(style, maskScale);
}

// Draw option animation and layout, then draw the active save menu when present.
void COptionDialog::Draw() {
    if (*(volatile void**)&Sing_805DC5F0() == 0) {
        if (!s_bpCtrlInitialized) {
            ((CBpCtrl*)s_bpCtrl)->__ct();
            __register_global_object(s_bpCtrl, CBpCtrl_dtor, s_bpCtrlDtorRegistration_80519F20);
            s_bpCtrlInitialized = 1;
        }
        Sing_805DC5F0() = s_bpCtrl;
    }
    void* optionAnimation = (void*)IdMap_Lookup(Sing_805DC5F0(), (*(void**)&bpEntryId));
    BpObject_Draw(optionAnimation);
    Layout_DrawWithOpacity(layout, kFloat_1_OptionMenu);
    void* ptr = saveMenu;
    if (ptr != 0)
        MPBase1_DrawIfInitialized(ptr);
}

} // extern "C"

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU80462584", _ctors_u802E9EE8_0, OptionMenu_InitFullTurnConstant);
#pragma pop

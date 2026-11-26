// The club submenu (CClubSubMenu), the player status panel (CPlayerStatus) and the club
// level-up dialog (CClubLvUpDialog) in the clubhouse shop.
//
// Covers menu navigation and confirmation for each menu variant, and the club upgrade levels
// and prices that the level-up dialog shows.

#include <decomp/static_init.h>

#include "game/singletons.h"
#include "classes/CPangya2Shop.h"
#include "classes/CPlayerStatus.h"
#include "classes/CClubSubMenu.h"
#include "classes/CClubLvUpDialog.h"
#include "classes/CClubCustomTbl.h"
#include "classes/CBpCtrl.h"
#include "classes/CFontNumDraw.h"
#define AsCClubLvUpDialog(p) ((CClubLvUpDialog*)(p))

#define AsCClubSubMenu(p) ((CClubSubMenu*)(p))

#define AsCPlayerStatus(p) ((CPlayerStatus*)(p))

extern "C" {

// The action constants below are IDs consumed by Input_IsActionPressed, not hardware button bits.
// Replacing the Wii Remote mapping belongs in the action/device layer; menu behavior uses these actions.
static const int kStatusReverseTextureCursorOffset =
    0x0C; // Four slots are addressed at cursor[19] while moving backward.
static const int kSubMenuMarkedChoiceOffset =
    0x18;                                  // Marked-choice byte; address form preserves local-class symbol bookkeeping.
static const int kSubMenuLayoutOffset = 4; // CClubSubMenu::layout

static const int kActionPrevious = 0;
static const int kActionNext = 1;
static const int kActionConfirm = 4;
static const int kActionCancel = 0x17;
static const int kAnyInputPlayer = -1;
static const int kFirstClubChoice = 0xF;
static const int kSecondClubChoice = 0x10;
static const int kThirdClubChoice = 0x11;
static const int kNoSelectionResult = -1;
static const int kCancelSelectionResult = 0;
static const int kSelectionFeedbackFrames = 3;
static const int kLevelUpAccept = 0;
static const int kLevelUpCancel = 1;
static const int kClubChoiceCount = 3;
// Variants are named by their observable layout, not by guessed localized labels.
// Choice hit targets are 0x0F/0x10/0x11. Separate label/art IDs are 0x12-0x16:
//   variant 0: primary group, 12/13/16; all three hit targets enabled
//   variant 1: alternate group, 12/13; first two hit targets enabled
//   variant 2: alternate group, 12/14; first two hit targets enabled
//   variant 3: alternate group, 12/15; first two hit targets enabled
// 0x12's value state reflects firstChoiceMarked for variants 0-2 only.
// Text/action names for these art IDs remain unresolved in this unit.
enum ClubSubMenuVariant {
    kClubThreeChoices = 0,
    kClubTwoChoicesLabel13 = 1,
    kClubTwoChoicesLabel14 = 2,
    kClubTwoChoicesLabel15 = 3
};
enum ClubSubMenuArt {
    kClubMarkedFirstLabel = 0x12,
    kClubVariantLabel13 = 0x13,
    kClubVariantLabel14 = 0x14,
    kClubVariantLabel15 = 0x15,
    kClubThirdChoiceLabel = 0x16
};

static const int kSubMenuLayoutName = 0;
static const int kSubMenuCloseSound = 0x11;
static const int kSubMenuOpenSound = 0x21;
static const int kSubMenuMoveSound = 0x30;
static const int kSubMenuClickSound = 0x3F;
static const int kSubMenuCancelSound = 0x4D;

static const int kLevelClickSound = 0x5F;
static const int kLevelCancelSound = 0x6D;
static const int kLevelUpgradeSound = 0x84;
static const int kLevelPurchaseSound = 0x8F;
static const int kPlayerStatusScriptName = 0xF;

// Partial views of the save record and customization-price row; retain signed level bytes.
struct ClubUpgradeLevel {
    signed char pad;
    signed char level;
};
struct ClubUpgradePlayerView {
    char pad_000[0x28];
    int pang;
    char pad_02C[0x480];
    ClubUpgradeLevel clubUpgrades[5];
};
struct ClubUpgradePrice {
    char pad_000[8];
    int pangCost;
};

extern const float kFloat_1_ShopDialog;
extern const float kFloat_0_5_ClubSubMenu;         // 0.5: club submenu sound parameter
extern const float kFloat_200_ClubSubMenu;         // 200: club submenu sound parameter
extern const float kClubDialogSoundParameter0_0_5; // 0.5: level-up dialog sound parameter
extern const float kClubDialogSoundParameter1_200; // 200: level-up dialog sound parameter
extern const float kFloat_1_ClubLevelDialog;
extern const float kFloat_6_2831855; // 2*pi: shared full-turn constant
extern float s_clubSubMenuFullTurnAngle;
extern float s_playerStatusFullTurnAngle;

extern void Layout_Update(void*);
extern int Layout_SetModeByte(void*, int);
extern void Layout_SetElementValueState(void*, int, int);
extern void Layout_SetElementPlayerFlag(void*, int, int, int);
extern void Layout_GetElementPosition(void*, int, float*);
extern void Layout_SetElementPosition(void*, int, float*);
extern void Layout_SetElementFeedbackState(void*, int, int);
extern void Layout_DrawWithOpacity(void*, float);
extern int Layout_PollLoadComplete(void*);
extern void Layout_Destroy(void*, int);
extern void Layout_Init(void*, int);
extern void Layout_LoadExtended(void*, int, void*);
extern void* __nw__FUl(unsigned int);
extern void Memory_Clear(void*, int);
extern void* __nwa__FUl(unsigned int);
extern int IdMap_Lookup(void*, int);
extern void __ct__13CClubHouseMsgFv(void*);
extern void __destroy_new_array(void*, void*);
extern void UITextTable_ClearEntries(void*);
extern void PointerVector_InitReserved(void*);
extern void PointerArray_DeleteValue(void*, void*);
extern void __register_global_object(void*, void*, void*);
extern void PointerVector_DestroyStorage();
extern void CBpCtrl_dtor();
extern void Layout_SetElementEnabled(void*, int, int);
extern void Layout_SetElementPlayerFlagsAndState(void*, int, int);
extern int Layout_HitTestPlayer(void*, int);
extern int Input_IsActionPressed(void*, int, int);
extern void InputManager_Init(void*);
extern int Layout_CollectActivated(void*, int**, int*);
extern void appFreeUni__FPv(void*);
extern void PlayerData_SetClubUpgradeLevel(int, int, void*);
extern void Layout_GetElementEnabled(void*, int, int*);
extern void InputManager_Dtor();
extern void Sound_PlayByName(void*, const char*, int, int, int, int, float, float, float);
extern void* g_soundManager;
extern unsigned char kClubSubMenuLabelElements[]; // int labelElement[variant][choice], three choices per variant
extern unsigned char __vt__13CPlayerStatus[];
extern unsigned char s_fontNumDrawRegistry[];
extern unsigned char s_bpCtrl[];
extern unsigned char s_playerStatusNumberRegistryDtorRegistration[];
extern unsigned char s_playerStatusBpCtrlDtorRegistration[];
extern "C" void __dt__6CBpTexFv(void*);
extern "C" void __ct__6CBpTexFv(void*);
extern void* __construct_new_array(void*, void*, void*, int, int);
extern void __dl__FPv(void*);
extern unsigned char __vt__12CClubSubMenu[];
extern unsigned char clubSubMenuStrings[]; // club submenu layout and sound-name pool
extern const unsigned char clubLevelUpDialogStrings[];
extern const unsigned char kPlayerStatusEquipmentTextureCounts[]; // four equipment texture-array counts
extern unsigned char playerStatusResourceStrings[];               // player-status layout and script-name pool
extern signed char s_pointerVectorInitialized;
extern signed char s_bpCtrlInitialized;
extern signed char s_inputManagerInitialized;
extern unsigned char s_gameInputManager[];
extern unsigned char s_clubSubMenuInputManagerDtorRegistration[];
extern unsigned char s_clubLevelInputManagerDtorRegistration[];

struct Fn802DED7C_Object {
    char data[0x5C];
    // Construct the embedded layout with the supplied layout mode.
    Fn802DED7C_Object(int arg) {
        Layout_Init(this, arg);
    }
};

// Report whether the selected second/third menu choice is valid for the current variant.
int CClubSubMenu::IsSecondaryActionSelected() {
    // Original 0x802DF2DC: the two selectable states share the true result.
    int state = selectedElementIndex;
    switch (state) {
    case kSecondClubChoice:
        if (menuVariant != 0)
            break;
        return 0;
    case kThirdClubChoice:
        break;
    default:
        return 0;
    }
Lret1: // Preserve the original label's compiler bookkeeping for local-class symbols.
    return 1;
}

// Prefer pointer hover, then previous/next actions; wrap selection and play movement feedback.
int CClubSubMenu::ProcessNavigation() {
    // Phase 1: a changed pointer hover wins; a stable hover suppresses button navigation.
    int selectionChanged = 0;
    int elementIndex = kFirstClubChoice;
    for (;;) {
        if (Layout_HitTestPlayer(layout, elementIndex)) {
            if (elementIndex == selectedElementIndex) {
                selectionChanged = 0;
            } else {
                selectedElementIndex = elementIndex;
                CClubSubMenu::UpdateSelectionHighlight(this);
                selectionChanged = 1;
            }
            break;
        }
        ++elementIndex;
        if (elementIndex > kThirdClubChoice) {
            selectionChanged = 0;
            break;
        }
    }
    if (selectionChanged)
        return 1;

    elementIndex = kFirstClubChoice;
    for (;;) {
        if (Layout_HitTestPlayer(layout, elementIndex))
            return 0;
        ++elementIndex;
        if (elementIndex > kThirdClubChoice)
            break;
    }

    // Phase 2: outside all hit targets, use previous/next with variant-dependent wrapping.
    if (GetInputManagerSlot() == 0) {
        if (!s_inputManagerInitialized) {
            InputManager_Init(&s_gameInputManager);
            __register_global_object(&s_gameInputManager, (void*)InputManager_Dtor,
                                     s_clubSubMenuInputManagerDtorRegistration);
            s_inputManagerInitialized = 1;
        }
        GetInputManagerSlot() = &s_gameInputManager;
    }
    // Original 0x802DF31C: either navigation direction shares the feedback tail.
    if (Input_IsActionPressed(GetInputManagerSlot(), kActionPrevious, kAnyInputPlayer)) {
        // Move to the previous eligible menu choice; result translation occurs on confirmation.
        int selectedElement = selectedElementIndex;
        if (selectedElement >= kSecondClubChoice) {
            selectedElementIndex = selectedElement - 1;
        } else {
            int thirdChoiceAvailable = 0;
            Layout_GetElementEnabled(layout, 0x11, &thirdChoiceAvailable);
            if (thirdChoiceAvailable)
                selectedElementIndex = kThirdClubChoice;
            else
                selectedElementIndex = kSecondClubChoice;
        }
    } else {
        if (GetInputManagerSlot() == 0) {
            if (!s_inputManagerInitialized) {
                InputManager_Init(&s_gameInputManager);
                __register_global_object(&s_gameInputManager, (void*)InputManager_Dtor,
                                         s_clubSubMenuInputManagerDtorRegistration);
                s_inputManagerInitialized = 1;
            }
            GetInputManagerSlot() = &s_gameInputManager;
        }
        if (Input_IsActionPressed(GetInputManagerSlot(), kActionNext, kAnyInputPlayer)) {
            int selectedElement = selectedElementIndex;
            if (selectedElement < kSecondClubChoice) {
                selectedElementIndex = selectedElement + 1;
            } else if (selectedElement == kSecondClubChoice) {
                int thirdChoiceAvailable = 0;
                Layout_GetElementEnabled(layout, 0x11, &thirdChoiceAvailable);
                if (thirdChoiceAvailable)
                    selectedElementIndex = kThirdClubChoice;
                else
                    selectedElementIndex = kFirstClubChoice;
            } else {
                selectedElementIndex = kFirstClubChoice;
            }
        } else {
            return 0;
        }
    }
    CClubSubMenu::UpdateSelectionHighlight(this);
    Sound_PlayByName(g_soundManager, (const char*)(clubSubMenuStrings + kSubMenuMoveSound), 0, 0, 0, 0,
                     kFloat_0_5_ClubSubMenu, kFloat_200_ClubSubMenu, kFloat_1_ShopDialog);
    return 1;
}

#pragma opt_propagation off
// Handle cancel/confirm actions, translate the choice to a result, and start pressed feedback.
int CClubSubMenu::ProcessConfirmation() {
    if (GetInputManagerSlot() == 0) {
        if (!s_inputManagerInitialized) {
            InputManager_Init(&s_gameInputManager);
            __register_global_object(&s_gameInputManager, (void*)InputManager_Dtor,
                                     s_clubSubMenuInputManagerDtorRegistration);
            s_inputManagerInitialized = 1;
        }
        GetInputManagerSlot() = &s_gameInputManager;
    }
    if (Input_IsActionPressed(GetInputManagerSlot(), kActionCancel, kAnyInputPlayer)) {
        selectionResult = 0;
        Sound_PlayByName(g_soundManager, (const char*)(clubSubMenuStrings + kSubMenuCancelSound), 0, 0, 0, 0,
                         kFloat_0_5_ClubSubMenu, kFloat_200_ClubSubMenu, kFloat_1_ShopDialog);
        return 1;
    }

    if (GetInputManagerSlot() == 0) {
        if (!s_inputManagerInitialized) {
            InputManager_Init(&s_gameInputManager);
            __register_global_object(&s_gameInputManager, (void*)InputManager_Dtor,
                                     s_clubSubMenuInputManagerDtorRegistration);
            s_inputManagerInitialized = 1;
        }
        GetInputManagerSlot() = &s_gameInputManager;
    }
    if (!Input_IsActionPressed(GetInputManagerSlot(), kActionConfirm, kAnyInputPlayer))
        return 0;

    int selectedElement = selectedElementIndex;
    switch (selectedElement) {
    case kFirstClubChoice:
        selectionResult = 1;
        break;
    case kSecondClubChoice: {
        int variant = menuVariant;
        if ((unsigned int)variant <= 1)
            selectionResult = 2;
        else if (variant == 2)
            selectionResult = 3;
        else
            selectionResult = 4;
        break;
    }
    case kThirdClubChoice:
        selectionResult = 3;
        break;
    }

    // Latch pressed feedback and move the paired label by one layout unit for three frames.
    Layout_SetElementValueState(layout, selectedElementIndex, 3);
    Layout_SetElementPlayerFlag(layout, selectedElementIndex, 0, 5);
    float labelPosition[2] = {0.0f, 0.0f};
    unsigned char* labelElementTable = kClubSubMenuLabelElements;
    int variantIndex = menuVariant;
    int labelElement = *(int*)(labelElementTable + variantIndex * 0xC + selectedElementIndex * 4 - 0x3C);
    Layout_GetElementPosition(layout, labelElement, labelPosition);
    labelPosition[0] += kFloat_1_ShopDialog;
    labelPosition[1] += kFloat_1_ShopDialog;
    labelElement = *(int*)(labelElementTable + variantIndex * 0xC + selectedElementIndex * 4 - 0x3C);
    Layout_SetElementPosition(layout, labelElement, labelPosition);
    selectionDelayFrames = kSelectionFeedbackFrames;
    Sound_PlayByName(g_soundManager, (const char*)(clubSubMenuStrings + kSubMenuClickSound), 0, 0, 0, 0,
                     kFloat_0_5_ClubSubMenu, kFloat_200_ClubSubMenu, kFloat_1_ShopDialog);
    return 1;
}
#pragma opt_propagation reset

// Count down pressed feedback, restore the selected label's position, and unlock input.
int CClubSubMenu::UpdateSelectionDelay() {
    int remainingFrames = selectionDelayFrames;
    if (remainingFrames == -1)
        return 0;
    selectionDelayFrames = remainingFrames - 1;
    if (remainingFrames > 0)
        return 1;
    float pressedOffset = kFloat_1_ShopDialog;
    int elementIndex = kFirstClubChoice;
    do {
        if (elementIndex == selectedElementIndex) {
            Layout_SetElementValueState(layout, elementIndex, 2);
            float labelPosition[2] = {0.0f, 0.0f};
            Layout_GetElementPosition(
                layout, *(int*)(kClubSubMenuLabelElements + menuVariant * 0xC + selectedElementIndex * 4 - 0x3C),
                labelPosition);
            labelPosition[0] -= pressedOffset;
            labelPosition[1] -= pressedOffset;
            Layout_SetElementPosition(
                layout, *(int*)(kClubSubMenuLabelElements + menuVariant * 0xC + selectedElementIndex * 4 - 0x3C),
                labelPosition);
        } else if (firstChoiceMarked && elementIndex == kFirstClubChoice) {
            Layout_SetElementValueState(layout, elementIndex, 1);
        } else {
            Layout_SetElementValueState(layout, elementIndex, 0);
        }
        ++elementIndex;
    } while (elementIndex <= kThirdClubChoice);
    selectionDelayFrames = -1;
    return 1;
}

// Set focused/unfocused visuals for the choices available in the current menu variant.
int CClubSubMenu::UpdateSelectionHighlight(void* menu) {
    void* self = menu;
    int elementIndex = kFirstClubChoice;
    for (;;) {
        if (elementIndex == kThirdClubChoice && AsCClubSubMenu(self)->menuVariant != 0)
            break;
        if (elementIndex == AsCClubSubMenu(self)->selectedElementIndex) {
            Layout_SetElementValueState(AsCClubSubMenu(self)->layout, elementIndex, 2);
            Layout_SetElementFeedbackState(AsCClubSubMenu(self)->layout, elementIndex, 2);
            if (AsCClubSubMenu(self)->firstChoiceMarked && elementIndex == kFirstClubChoice) {
                Layout_SetElementValueState(AsCClubSubMenu(self)->layout,
                                            ((int (*)[3])kClubSubMenuLabelElements)[AsCClubSubMenu(self)->menuVariant]
                                                                                   [elementIndex - kFirstClubChoice],
                                            3);
            } else {
                Layout_SetElementValueState(AsCClubSubMenu(self)->layout,
                                            ((int (*)[3])kClubSubMenuLabelElements)[AsCClubSubMenu(self)->menuVariant]
                                                                                   [elementIndex - kFirstClubChoice],
                                            2);
            }
        } else {
            Layout_SetElementValueState(AsCClubSubMenu(self)->layout, elementIndex, 0);
            Layout_SetElementFeedbackState(AsCClubSubMenu(self)->layout, elementIndex, 1);
            if (AsCClubSubMenu(self)->firstChoiceMarked && elementIndex == kFirstClubChoice) {
                Layout_SetElementValueState(AsCClubSubMenu(self)->layout,
                                            ((int (*)[3])kClubSubMenuLabelElements)[AsCClubSubMenu(self)->menuVariant]
                                                                                   [elementIndex - kFirstClubChoice],
                                            1);
            } else {
                Layout_SetElementValueState(AsCClubSubMenu(self)->layout,
                                            ((int (*)[3])kClubSubMenuLabelElements)[AsCClubSubMenu(self)->menuVariant]
                                                                                   [elementIndex - kFirstClubChoice],
                                            0);
            }
        }
        ++elementIndex;
        if (elementIndex > kThirdClubChoice)
            break;
    }
    return 1;
}

#pragma use_lmw_stmw on
struct VDel802DEA60 {
    virtual void d0(int);
};

// Release the status layout, equipment textures, number renderer, script entry, and message object.
CPlayerStatus::~CPlayerStatus() {
    void** self = (void**)this;
    self[0] = __vt__13CPlayerStatus;
    if (self[2] != 0) {
        Layout_Destroy(self[2], 1);
        self[2] = 0;
    }
    void** textureSlotCursor = (void**)((char*)this + kStatusReverseTextureCursorOffset);
    for (int i = 3; i >= 0; --i, --textureSlotCursor) {
        if (textureSlotCursor[19] != 0) {
            __destroy_new_array(textureSlotCursor[19], (void*)__dt__6CBpTexFv);
            textureSlotCursor[19] = 0;
        }
    }
    UITextTable_ClearEntries(numberDraw);
    if (numberDraw != 0) {
        if (Sing_805DC830() == 0) {
            if (s_pointerVectorInitialized == 0) {
                PointerVector_InitReserved(s_fontNumDrawRegistry);
                __register_global_object(s_fontNumDrawRegistry, (void*)PointerVector_DestroyStorage,
                                         s_playerStatusNumberRegistryDtorRegistration);
                s_pointerVectorInitialized = 1;
            }
            Sing_805DC830() = s_fontNumDrawRegistry;
        }
        PointerArray_DeleteValue(Sing_805DC830(), numberDraw);
    }
    if (scriptEntryId != -1) {
        if (Sing_805DC5F0() == 0) {
            if (s_bpCtrlInitialized == 0) {
                ((CBpCtrl*)s_bpCtrl)->__ct();
                __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, s_playerStatusBpCtrlDtorRegistration);
                s_bpCtrlInitialized = 1;
            }
            Sing_805DC5F0() = s_bpCtrl;
        }
        ((CBpCtrl*)Sing_805DC5F0())->DeleteEntryById(scriptEntryId);
        scriptEntryId = -1;
    }
    void* message = clubHouseMessage;
    if (message != 0) {
        if (message != 0) {
            ((VDel802DEA60*)message)->d0(1);
        }
        clubHouseMessage = 0;
    }
}
#pragma use_lmw_stmw reset

// Return a completed menu result only after its pressed-feedback delay has ended.
int CClubSubMenu::GetSelectionResult(int* resultOut) {
    int selectionResult = this->selectionResult;
    if (selectionResult == -1 || selectionDelayFrames != -1)
        return 0;
    *resultOut = selectionResult;
    if (this->selectionResult == 0) {
        Sound_PlayByName(g_soundManager, (const char*)(clubSubMenuStrings + kSubMenuCloseSound), 0, 0, 0, 0,
                         kFloat_0_5_ClubSubMenu, kFloat_200_ClubSubMenu, kFloat_1_ShopDialog);
    }
    return 1;
}

// Allocate/load the club submenu layout and initialize an unconfirmed first choice.
CClubSubMenu::CClubSubMenu() {
    *(void**)this = __vt__12CClubSubMenu;
    Fn802DED7C_Object* child = new Fn802DED7C_Object(5);
    layout = child;
    int minusOne = -1;
    int mask = kFirstClubChoice;
    unsigned char* name = clubSubMenuStrings;
    selectedElementIndex = mask;
    menuVariant = minusOne;
    selectionDelayFrames = minusOne;
    selectionResult = minusOne;
    firstChoiceMarked = 0;
    Layout_LoadExtended(child, 0, name);
}

#pragma opt_lifetimes off
// Switch the two menu element groups and enable the choices appropriate to that group.
int CClubSubMenu::SetPrimaryElementGroupEnabled(void* menu, int firstGroupEnabled) {
    void* self = menu;
    int mode = firstGroupEnabled;
    int enabled = 0;
    int i = 3;
    while (i <= 8) {
        Layout_SetElementEnabled(AsCClubSubMenu(self)->layout, i, mode);
        ++i;
    }
    i = 9;
    enabled = (mode == 0);
    while (i <= 0xE) {
        Layout_SetElementEnabled(AsCClubSubMenu(self)->layout, i, enabled);
        ++i;
    }
    enabled = kFirstClubChoice;
    while (enabled <= kSecondClubChoice) {
        Layout_SetElementPlayerFlagsAndState(AsCClubSubMenu(self)->layout, enabled, 1);
        ++enabled;
    }
    Layout_SetElementPlayerFlagsAndState(AsCClubSubMenu(self)->layout, kThirdClubChoice, mode);
    return 1;
}
#pragma opt_lifetimes reset

// Update the layout and process navigation/confirmation while pressed feedback is idle.
int CClubSubMenu::Update() {
    Layout_Update(layout);
    if (UpdateSelectionDelay() == 0) {
        if (ProcessNavigation() != 0)
            return 1;
        ProcessConfirmation();
    }
    return 1;
}

// Release the submenu layout and optionally delete the menu object.
CClubSubMenu::~CClubSubMenu() {
    int* field = (int*)this;
    int child = field[1];
    *(void**)this = __vt__12CClubSubMenu;
    if (child != 0) {
        Layout_Destroy((void*)child, 1);
        field[1] = 0;
    }
}

// Initialize the submenu's cached full-turn angle from the shared constant.
void CClubSubMenu_InitFullTurnAngle() {
    s_clubSubMenuFullTurnAngle = kFloat_6_2831855;
}

// Initialize the player-status unit's cached full-turn angle from the shared constant.
void CPlayerStatus_InitFullTurnAngle() {
    s_playerStatusFullTurnAngle = kFloat_6_2831855;
}

// Advance the submenu layout without processing menu input.
int CClubSubMenu::UpdateLayout() {
    Layout_Update(layout);
    return 1;
}

// Reset the submenu layout's mode byte to zero.
int CClubSubMenu::ResetLayoutMode() {
    Layout_SetModeByte(layout, 0);
    return 1;
}

// Draw the submenu layout at full opacity.
int CClubSubMenu::Draw() {
    void* layout = this->layout;
    float f = kFloat_1_ShopDialog;
    Layout_DrawWithOpacity(layout, f);
    return 1;
}

#pragma use_lmw_stmw on
// Build the player-status layout, four equipment texture arrays, message object, and script/number renderer.
// A named member rather than a constructor: the constructor form of this body scores lower.
void* CPlayerStatus::Init(void* playerRecord) {
    // Phase 1: initialize the borrowed player record, owned layout and club-house message.
    char* self = (char*)this;
    *(void**)self = __vt__13CPlayerStatus;
    AsCPlayerStatus(self)->playerRecord = playerRecord;
    void* layout = __nw__FUl(0x5c);
    if (layout != 0)
        Layout_Init(layout, 5);
    AsCPlayerStatus(self)->layout = layout;
    AsCPlayerStatus(self)->power = 0;
    AsCPlayerStatus(self)->control = 0;
    AsCPlayerStatus(self)->spin = 0;
    Memory_Clear(self + 0x6F, 0x28);
    AsCPlayerStatus(self)->scriptObject = 0;
    AsCPlayerStatus(self)->scriptEntryId = -1;
    (*(int*)&AsCPlayerStatus(self)->numberDraw) = 0;
    void* message = __nw__FUl(0x20);
    if (message != 0) {
        try {
            __ct__13CClubHouseMsgFv(message);
        } catch (...) {
            __dl__FPv(message);
            throw;
        }
    }
    AsCPlayerStatus(self)->clubHouseMessage = message;
    Layout_LoadExtended(*(void**)(self + 8), 0, playerStatusResourceStrings);
    // Phase 2: construct owned CBpTex arrays (0x28 bytes/entry, 0x10-byte array header).
    // category index -> array field / initially-null selected texture:
    // 0 -> +0x4C equipmentTextures / +0x5C selectedEquipmentTexture (category unresolved)
    // 1 -> +0x50 caddieTextures / +0x60 selectedCaddieTexture
    // 2 -> +0x54 clubSetTextures / +0x64 selectedClubSetTexture
    // 3 -> +0x58 ballTextures / +0x68 selectedBallTexture
    // textureSlotCursor is a shifted address, NOT another CPlayerStatus object.
    // The cast deliberately adds the same field offsets on each four-byte step.
    int textureCounts[4];
    int* textureCountSource = (int*)kPlayerStatusEquipmentTextureCounts;
    textureCounts[0] = *textureCountSource++;
    textureCounts[1] = *textureCountSource++;
    textureCounts[2] = *textureCountSource++;
    textureCounts[3] = *textureCountSource++;
    int* textureCountCursor = textureCounts;
    char* textureSlotCursor = self;
    int equipmentKind = 0;
    int arrayConstructed = 0;
    do {
        int count = *textureCountCursor;
        arrayConstructed = 0;
        void* block = __nwa__FUl((unsigned int)(count * 0x28 + 0x10));
        void* items = block;
        if (block != 0) {
            arrayConstructed = 1;
            // the array pointer is __construct_new_array's result (mr r27,r3 at 0x802DFD08), past the header
            items = __construct_new_array(block, (void*)__ct__6CBpTexFv, (void*)__dt__6CBpTexFv, 0x28, count);
        }
        AsCPlayerStatus(textureSlotCursor)->equipmentTextures = items;
        equipmentKind = equipmentKind + 1;
        textureCountCursor = textureCountCursor + 1;
        AsCPlayerStatus(textureSlotCursor)->selectedEquipmentTexture = 0;
        textureSlotCursor = textureSlotCursor + 4;
    } while (equipmentKind < 4);
    // Phase 3: register the number renderer, then retain a BP script entry ID
    // and its resolved script object. The global registries own their entries.
    if (Sing_805DC830() == 0) {
        if (s_pointerVectorInitialized == 0) {
            PointerVector_InitReserved(s_fontNumDrawRegistry);
            __register_global_object(s_fontNumDrawRegistry, (void*)PointerVector_DestroyStorage,
                                     s_playerStatusNumberRegistryDtorRegistration);
            s_pointerVectorInitialized = 1;
        }
        Sing_805DC830() = s_fontNumDrawRegistry;
    }
    void* numberRenderer = CFontNumDraw::CreateInVector((PtrVector*)Sing_805DC830());
    volatile void* scriptController = Sing_805DC5F0();
    *(void**)(self + 0xA0) = (scriptController, numberRenderer);
    if (Sing_805DC5F0() == 0) {
        if (s_bpCtrlInitialized == 0) {
            ((CBpCtrl*)s_bpCtrl)->__ct();
            __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, s_playerStatusBpCtrlDtorRegistration);
            s_bpCtrlInitialized = 1;
        }
        Sing_805DC5F0() = s_bpCtrl;
    }
    ((CBpCtrl*)Sing_805DC5F0())
        ->CreateEntry((const char*)(playerStatusResourceStrings + kPlayerStatusScriptName), (int*)(self + 0x9C), 0);
    if (Sing_805DC5F0() == 0) {
        if (s_bpCtrlInitialized == 0) {
            ((CBpCtrl*)s_bpCtrl)->__ct();
            __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, s_playerStatusBpCtrlDtorRegistration);
            s_bpCtrlInitialized = 1;
        }
        Sing_805DC5F0() = s_bpCtrl;
    }
    *(int*)(self + 0x98) = IdMap_Lookup(Sing_805DC5F0(), *(int*)(self + 0x9C));
    return self;
}
#pragma use_lmw_stmw reset

// Report whether the shop layout has completed loading.
int CPangya2Shop::IsLayoutLoaded() {
    return Layout_PollLoadComplete(layout) != 0;
}

// Reset the choice/result and configure visible menu groups for one of four variants.
// firstChoiceMarked selects label state; it is not an ownership flag. Returns 1.
// An unknown variant still resets selection/result, highlights and plays open sound,
// but leaves the old menuVariant/visibility intact (there is no validation fallback).
int CClubSubMenu::OpenVariant(int variant, unsigned char firstChoiceMarked) {
    char* self = (char*)this;
    *(unsigned char*)&AsCClubSubMenu(self)->firstChoiceMarked = firstChoiceMarked;
    *(int*)&AsCClubSubMenu(self)->selectionResult = -1;
    *(int*)&AsCClubSubMenu(self)->selectedElementIndex = kFirstClubChoice;
    switch (variant) {
    case kClubThreeChoices:
        *(int*)&AsCClubSubMenu(self)->menuVariant = kClubThreeChoices;
        if (*(unsigned char*)(self + kSubMenuMarkedChoiceOffset))
            Layout_SetElementValueState(*(void**)&AsCClubSubMenu(self)->layout, kClubMarkedFirstLabel, 1);
        else
            Layout_SetElementValueState(*(void**)&AsCClubSubMenu(self)->layout, kClubMarkedFirstLabel, 0);
        CClubSubMenu::SetPrimaryElementGroupEnabled(self, 1);
        Layout_SetElementEnabled(*(void**)(self + kSubMenuLayoutOffset), kClubMarkedFirstLabel, 1);
        Layout_SetElementEnabled(*(void**)(self + kSubMenuLayoutOffset), kClubVariantLabel13, 1);
        Layout_SetElementEnabled(*(void**)(self + kSubMenuLayoutOffset), kClubVariantLabel14, 0);
        Layout_SetElementEnabled(*(void**)(self + kSubMenuLayoutOffset), kClubVariantLabel15, 0);
        Layout_SetElementEnabled(*(void**)(self + kSubMenuLayoutOffset), kClubThirdChoiceLabel, 1);
        break;
    case kClubTwoChoicesLabel13:
        *(int*)&AsCClubSubMenu(self)->menuVariant = kClubTwoChoicesLabel13;
        if (*(unsigned char*)(self + kSubMenuMarkedChoiceOffset))
            Layout_SetElementValueState(*(void**)&AsCClubSubMenu(self)->layout, kClubMarkedFirstLabel, 1);
        else
            Layout_SetElementValueState(*(void**)&AsCClubSubMenu(self)->layout, kClubMarkedFirstLabel, 0);
        CClubSubMenu::SetPrimaryElementGroupEnabled(self, 0);
        Layout_SetElementEnabled(*(void**)(self + kSubMenuLayoutOffset), kClubMarkedFirstLabel, 1);
        Layout_SetElementEnabled(*(void**)(self + kSubMenuLayoutOffset), kClubVariantLabel13, 1);
        Layout_SetElementEnabled(*(void**)(self + kSubMenuLayoutOffset), kClubVariantLabel14, 0);
        Layout_SetElementEnabled(*(void**)(self + kSubMenuLayoutOffset), kClubVariantLabel15, 0);
        Layout_SetElementEnabled(*(void**)(self + kSubMenuLayoutOffset), kClubThirdChoiceLabel, 0);
        break;
    case kClubTwoChoicesLabel14:
        *(int*)&AsCClubSubMenu(self)->menuVariant = kClubTwoChoicesLabel14;
        if (*(unsigned char*)(self + kSubMenuMarkedChoiceOffset))
            Layout_SetElementValueState(*(void**)&AsCClubSubMenu(self)->layout, kClubMarkedFirstLabel, 1);
        else
            Layout_SetElementValueState(*(void**)&AsCClubSubMenu(self)->layout, kClubMarkedFirstLabel, 0);
        CClubSubMenu::SetPrimaryElementGroupEnabled(self, 0);
        Layout_SetElementEnabled(*(void**)(self + kSubMenuLayoutOffset), kClubMarkedFirstLabel, 1);
        Layout_SetElementEnabled(*(void**)(self + kSubMenuLayoutOffset), kClubVariantLabel13, 0);
        Layout_SetElementEnabled(*(void**)(self + kSubMenuLayoutOffset), kClubVariantLabel14, 1);
        Layout_SetElementEnabled(*(void**)(self + kSubMenuLayoutOffset), kClubVariantLabel15, 0);
        Layout_SetElementEnabled(*(void**)(self + kSubMenuLayoutOffset), kClubThirdChoiceLabel, 0);
        break;
    case kClubTwoChoicesLabel15:
        *(int*)&AsCClubSubMenu(self)->menuVariant = kClubTwoChoicesLabel15;
        CClubSubMenu::SetPrimaryElementGroupEnabled(self, 0);
        Layout_SetElementEnabled(*(void**)&AsCClubSubMenu(self)->layout, kClubMarkedFirstLabel, 1);
        Layout_SetElementEnabled(*(void**)(self + kSubMenuLayoutOffset), kClubVariantLabel13, 0);
        Layout_SetElementEnabled(*(void**)(self + kSubMenuLayoutOffset), kClubVariantLabel14, 0);
        Layout_SetElementEnabled(*(void**)(self + kSubMenuLayoutOffset), kClubVariantLabel15, 1);
        Layout_SetElementEnabled(*(void**)(self + kSubMenuLayoutOffset), kClubThirdChoiceLabel, 0);
        break;
    default:
        break;
    }
    CClubSubMenu::UpdateSelectionHighlight(self);
    Sound_PlayByName(g_soundManager, (const char*)(clubSubMenuStrings + kSubMenuOpenSound), 0, 0, 0, 0,
                     kFloat_0_5_ClubSubMenu, kFloat_200_ClubSubMenu, kFloat_1_ShopDialog);
    return 1;
}

#pragma opt_strength_reduction off
// Handle cancel or activated choices; acceptance pays the upgrade cost and advances the club level.
// Returns 0 when no activation was collected, 1 after cancel or processing a list.
// playerData is borrowed and modified in place; UI setup must already enforce
// affordability/index validity. This routine does not validate the price lookup.
// choice 0 accepts, other choices cancel; if no ID in [0,2] is found, -1 still
// reaches feedback calls. Preserve that behavior when changing the UI contract.
int CClubLvUpDialog::ProcessInput() {
    char* self = (char*)this;
    if (GetInputManagerSlot() == 0) {
        if (!s_inputManagerInitialized) {
            InputManager_Init(&s_gameInputManager);
            __register_global_object(&s_gameInputManager, (void*)InputManager_Dtor,
                                     s_clubLevelInputManagerDtorRegistration);
            s_inputManagerInitialized = 1;
        }
        GetInputManagerSlot() = &s_gameInputManager;
    }
    if (Input_IsActionPressed(GetInputManagerSlot(), kActionCancel, kAnyInputPlayer)) {
        AsCClubLvUpDialog(self)->result = kLevelUpCancel;
        Sound_PlayByName(g_soundManager, (const char*)(clubLevelUpDialogStrings + kLevelCancelSound), 0, 0, 0, 0,
                         kClubDialogSoundParameter0_0_5, kClubDialogSoundParameter1_200, kFloat_1_ClubLevelDialog);
        return 1;
    }
    // Own the temporary activated-element array, including early-return paths.
    struct ActivatedList {
        int* activatedElements;
        unsigned int count;
        int extra;
        // Free the collected activation IDs on normal return and exception unwind.
        ~ActivatedList() {
            if (activatedElements) {
                unsigned int storedCount = *(volatile unsigned int*)&count;
                count -= storedCount;
                appFreeUni__FPv(activatedElements);
            }
        }
    } activated;
    activated.activatedElements = 0;
    activated.count = 0;
    activated.extra = 0;
    if (!Layout_CollectActivated(AsCClubLvUpDialog(self)->layout, (int**)&activated, 0)) {
        return 0;
    }
    // Phase 2: choose the first recognized activated element, in collected order.
    unsigned int count = activated.count;
    int choice = -1;
    int* activatedElements = activated.activatedElements;
    for (int i = 0; i < count; ++i) {
        int elementIndex = activatedElements[i];
        if (elementIndex >= 0 && elementIndex <= 2) {
            choice = elementIndex;
            break;
        }
    }
    // Phase 3: apply payment/level mutation before pressed feedback. Cancellation
    // above exits immediately, whereas a clicked cancel receives delayed feedback.
    if (choice == kLevelUpAccept) {
        AsCClubLvUpDialog(self)->result = kLevelUpAccept;
        ClubUpgradePlayerView* upgradeLevels = (ClubUpgradePlayerView*)AsCClubLvUpDialog(self)->playerData;
        int oldLevel = upgradeLevels->clubUpgrades[AsCClubLvUpDialog(self)->clubIndex].level;
        void* upgradePrice = CClubCustomTbl::FindUpgradeEntry((signed char)AsCClubLvUpDialog(self)->clubIndex,
                                                              (signed char)(oldLevel + 1));
        // This OR is true for every integer oldLevel; it provides no bounds validation.
        // Preserve the original OR: changing it to AND changes purchase behavior.
        if (oldLevel > 0 || oldLevel <= 4) {
            int nextLevel = (signed char)(oldLevel + 1);
            int* playerRecord = (*(int**)&AsCClubLvUpDialog(self)->playerData);
            ((ClubUpgradePlayerView*)playerRecord)->pang -= ((ClubUpgradePrice*)upgradePrice)->pangCost;
            PlayerData_SetClubUpgradeLevel((signed char)AsCClubLvUpDialog(self)->clubIndex, nextLevel,
                                           (*(int**)&AsCClubLvUpDialog(self)->playerData));
            Sound_PlayByName(g_soundManager, (const char*)(clubLevelUpDialogStrings + kLevelUpgradeSound), 0, 0, 0, 0,
                             kClubDialogSoundParameter0_0_5, kClubDialogSoundParameter1_200, kFloat_1_ClubLevelDialog);
            Sound_PlayByName(g_soundManager, (const char*)(clubLevelUpDialogStrings + kLevelPurchaseSound), 0, 0, 0, 0,
                             kClubDialogSoundParameter0_0_5, kClubDialogSoundParameter1_200, kFloat_1_ClubLevelDialog);
        }
    } else {
        AsCClubLvUpDialog(self)->result = kLevelUpCancel;
    }
    // Phase 4: latch the result and offset the paired label by one layout unit
    // in X/Y for three update frames. The temporary activation array is freed.
    AsCClubLvUpDialog(self)->selectedElement = choice;
    Layout_SetElementValueState(AsCClubLvUpDialog(self)->layout, choice, 2);
    Layout_SetElementPlayerFlag(AsCClubLvUpDialog(self)->layout, AsCClubLvUpDialog(self)->selectedElement, 0, 5);
    float labelPosition[2] = {0.0f, 0.0f};
    Layout_GetElementPosition(AsCClubLvUpDialog(self)->layout, AsCClubLvUpDialog(self)->selectedElement + 3,
                              labelPosition);
    labelPosition[0] += kFloat_1_ClubLevelDialog;
    labelPosition[1] += kFloat_1_ClubLevelDialog;
    Layout_SetElementPosition(AsCClubLvUpDialog(self)->layout, AsCClubLvUpDialog(self)->selectedElement + 3,
                              labelPosition);
    AsCClubLvUpDialog(self)->selectionDelayFrames = kSelectionFeedbackFrames;
    Sound_PlayByName(g_soundManager, (const char*)(clubLevelUpDialogStrings + kLevelClickSound), 0, 0, 0, 0,
                     kClubDialogSoundParameter0_0_5, kClubDialogSoundParameter1_200, kFloat_1_ClubLevelDialog);
    return 1;
}
#pragma opt_lifetimes reset
#pragma opt_strength_reduction reset

} // extern "C"

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU8046255C", _ctors_u802DEA60_0, CClubSubMenu_InitFullTurnAngle);
STATIC_INIT_ENTRY(".CRT$XCU8046255C", _ctors_u802DEA60_1, CPlayerStatus_InitFullTurnAngle);
#pragma pop

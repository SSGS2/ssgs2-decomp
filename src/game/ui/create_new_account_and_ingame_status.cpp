#include <decomp/static_init.h>
#include <decomp/cw_bitfields.h>
#include "iff/IffStruct.h"
#include "classes/CSharedDoc.h"
#include "classes/CCreateNewAccount.h"
#include "classes/SavePlayerRecord.h"
#define AsCCreateNewAccount(p) ((CCreateNewAccount*)(p))
#define AsCSharedDoc(p) ((CSharedDoc*)(p))
#define GAME_VECTOR_AGGREGATE

#include "game/vector.h"

// Stack-local storage view; external vector operations retain the original ABI.
struct LocalIntVector_u80326994 {
    int* items;
    volatile unsigned int size;
    unsigned int capacity;
    // Free the owned list of activated element IDs.
    ~LocalIntVector_u80326994() {
        if (items) {
            size = *(unsigned int*)&size - *(int*)&size;
            appFreeUni__FPv(items);
        }
    }
};

// Translation unit 0x80326994-0x803280FC (23 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

#include "game/singletons.h"
#include "classes/CIngameStatus2d.h"

#define AsCIngameStatus2d(p) ((CIngameStatus2d*)(p))
extern "C" {

// The two preference pairs occupy bits 7/6 of the menu byte. Bit 7 maps to
// save preference bit 30; bit 6 chooses club-set inventory index 14 instead of 0.
enum AccountMenuConstants {
    kAccountElementCount = 17, kAccountSelectionOverlayOffset = 6,
    kAccountConfirmElement = 14, kAccountMessageElement = 15,
    kAccountAvatarElement = 16, kSelectedElementState = 2,
    kNeutralPlayerColor = 5, kActionCancel = 6,
    kSavePreferenceOptionBit = 0x80, kAlternateStarterClubOptionBit = 0x40,
    kAlternateStarterClubSetIndex = 14, kStarterCharacterGrantMask = 0xC0,
    kSavedPreferenceMask = 0x40000000,
    kCharacter0Group = 0x3F3, kCharacter1Group = 0x3F4,
    kPowerGaugeGroup = 0x3F6, kControlGaugeGroup = 0x3F8, kSpinGaugeGroup = 0x3FA,
    kCharacterNameGroup = 0x428
};
// Layout map: 0/1 character choices; 2/3 preference bit7; 4/5 starter club bit6.
// Choices 0..5 have overlays at +6; 12/13 are opposite-character indicators;
// 14 confirm, 15 message anchor, 16 avatar rect. States 0/2 mean reset/selected.
// Gauge groups display IFF stats[0]=power, [1]=control, [3]=spin.
enum AccountStatIndex { kAccountPowerStat = 0, kAccountControlStat = 1, kAccountSpinStat = 3 };
struct AccountSelectionState {
    char beforeSelection[0x30];
    unsigned char accepted, cancelled;
    signed char selectedCharacterIndex;
    unsigned char optionBits;
};
struct NewAccountSaveFields {
    char beforeCharacter[0x2C];
    unsigned char characterIndex, clubSetIndex;
    char beforeCharacter0[0x2CC - 0x2E];
    unsigned char character0Status;
    char beforeCharacter1[0x300 - 0x2CD];
    unsigned char character1Status;
    char beforePreference[0x4E8 - 0x301];
    unsigned int preferenceFlags;
};
typedef char NewAccountSaveFields_preference[((int)&((NewAccountSaveFields*)0)->preferenceFlags == 0x4E8) ? 1 : -1];

// Account views: all offsets are relative to the same menu object. Layout +0x18,
// primary script +0x1C, selection/options +0x30..0x33, avatar +0x34, message +0x38.
// Constructor view adds packed-file +0x3C and texture +0x40; status views below
// describe a different CIngameStatus2d object, not trailing account fields.
// Option-byte signedness is retained per call site; the physical schema is shared.
} // extern "C"
template <class OptionByte> struct AccountMenuFields {
    char prefix[0x18]; void* layout; void* primaryBpObject;
    char betweenBpAndOptions[0x10]; unsigned char accepted, cancelled;
    signed char selectedCharacterIndex; OptionByte optionBits;
    void* avatar; void* menuMessage; char packedFileStorage[4];
    union { void* textureResource; void* child; };
};
extern "C" {
typedef AccountMenuFields<char> Fn80326F4CData;
typedef AccountMenuFields<unsigned char> Fn80326994Data;

extern const float kNewAccountPowerGaugeMaximum, kFloat_25_Stats, kFloat_30_805E1518;
extern signed char s_fadeOverlayInitialized;
extern CSharedDoc* g_sharedDoc;
extern int s_newAccountGaugeScaleX, s_newAccountGaugeScaleY;
extern int s_newAccountCharacterNameMessageIds;
extern char s_newAccountFadeOverlayRegistration[];
extern void* g_soundManager;
extern int kNewAccountAvatarLoadingPositionX;
extern int kNewAccountAvatarLoadingPositionY;
struct NewAccountMenuStringPool {
    char playerScripts[0x10], playerLayout[0x10], backgroundLayout[0x11];
    char captionLayout[0x15], resourceArchive[0x0D];
    char playerScript[9], backgroundScript[3], tutorialMusic[0x0D], titleScript[6];
};
extern NewAccountMenuStringPool newAccountMenuStrings;
typedef char NewAccountStrings_script[((int)&((NewAccountMenuStringPool*)0)->playerScript == 0x53) ? 1 : -1];
void BpObject_StartScriptByName(void*, const char*, int);
void UIGroup_SetEnabled(void*, int);
int CCreateNewAccount_InitBpGroups(void*);
void Layout_SetElementEnabled(void*, int, int);
int Layout_SetElementValueState(void*, int, int);
void Layout_SetElementPlayerFlag(void*, int, int, int);
void Layout_GetElementPosition(void*, int, float*);
void MenuMessage_SetPosition(void*, float*);
void* UIGroup_GetOrCreateById(void*, int);
int UIGroup_GetCombinedOpacity(void*);
int UIGroup_GetTintColor(void*);
void* CAsyncExhibitionAvatar_Create(float*, int, float);
void* SaveData_AcquireInitializedInstance();
void* ItemSelection_Init(void*);
int SaveData_LoadValidatedItemSelection(void*, void*, signed char);
int CAsyncExhibitionAvatar_SetEquipment(void*, void*, int);
void SoundManager_RequestBgmChange(void*, const char*, int, int);
void CAsyncExhibition_SetLoadingIndicatorPosition(void*, void*);
void IFadeMenuBase_StartMenuFadeOut(void*);
extern const char s_fadeOverlay[];
void FadeOverlay_Init(void*);
void ResourceState_Dtor();
int FadeState_GetState(void*);
int CAsyncExhibition_PollReady(void*);
int UIGroup_SetScale(void*, float*);
int ItemInventory_GetCharacterIdAt(void*, int);
void* ItemInventory_FindCharacterData(void*, int);
int UIGroup_GetTintColor(void*);
int UIGroup_GetCombinedOpacity(void*);
void MenuMessage_ResetTextState(void*);
void MenuMessage_SetColorMultiplier(void*, void*);
void MenuMessage_SetText(void*, int);
int CCreateNewAccount_HandleConfirmCancel(void*);
int CCreateNewAccount_UpdateCharacterSelection(void*);
void Layout_ReadElementRect(void*, int, float*);
void Layout_WriteElementRect(void*, int, float*);
void* CAsyncExhibitionAvatar_Create(float*, int, float);
void CAsyncExhibition_SetLoadingIndicatorPosition(void*, void*);
void IFadeMenuBase_StartMenuFadeOut(void*);
void SoundManager_RequestBgmChange(void*, const char*, int, int);
void MenuMessage_SetPosition(void*, float*);

// Select a choice and its paired overlay; confirm has no paired overlay.
static inline void SelectAccountChoice(Fn80326994Data* self, int index) {
    if (index < kAccountElementCount) {
        Layout_SetElementValueState(self->layout, index, kSelectedElementState);
        Layout_SetElementPlayerFlag(self->layout, index, 0, kNeutralPlayerColor);
        if (index != kAccountConfirmElement && index + kAccountSelectionOverlayOffset < kAccountElementCount) {
            Layout_SetElementValueState(self->layout, index + kAccountSelectionOverlayOffset, kSelectedElementState);
            Layout_SetElementPlayerFlag(self->layout, index + kAccountSelectionOverlayOffset, 0, kNeutralPlayerColor);
        }
    }
}

// Initialize selection buttons, character stat gauges, name text and the equipped avatar preview.
void CCreateNewAccount_InitializeScreen(Fn80326994Data* self) {
    int index;
    int selected;
    int oppositeCharacterBit;
    int nonzeroOppositeCharacter;
    int oppositeCharacterEnabled;
    void* inventory;
    void* characterData;
    void* powerGauge;
    void* controlGauge;
    void* spinGauge;
    void* nameGroup;
    int color;
    int combinedOpacity;
    int opacityScratch;
    unsigned int alpha;
    unsigned int packed;
    float rect[4];
    float avatarRect[4];
    float size[2];
    int pair[2];
    char equipmentSelection[0x68];
    // Phase 1: release the previous texture, start scripts and reset button layers.
    if (self->textureResource != 0) {
        // Phase 1: release the previous texture, start scripts and reset button layers.
    if (self->textureResource != 0) {
            ((void (**)(void*, int))(*(void***)self->textureResource))[2](self->textureResource, 1);
        }
        self->textureResource = 0;
    }
    BpObject_StartScriptByName(AsCCreateNewAccount(self)->primaryBpObject, newAccountMenuStrings.playerScript, 1);
    BpObject_StartScriptByName(AsCCreateNewAccount(self)->secondaryBpObject, newAccountMenuStrings.backgroundScript, 1);
    CCreateNewAccount_InitBpGroups(self);
    Layout_SetElementEnabled(self->layout, kAccountMessageElement, 0);

    index = (unsigned int)((-self->selectedCharacterIndex) | self->selectedCharacterIndex) >> 31;
    SelectAccountChoice(self, index);
    index = 3;
    if ((self->optionBits >> 7) & 1) index = 2;
    SelectAccountChoice(self, index);
    index = 5;
    if ((self->optionBits >> 6) & 1) index = 4;
    SelectAccountChoice(self, index);
    for (index = 2; index <= 5; ++index) {
        Layout_ReadElementRect(self->layout, index, rect);
        Layout_WriteElementRect(self->layout, index + 6, rect);
    }
    Layout_GetElementPosition(self->layout, kAccountMessageElement, size);
    MenuMessage_SetPosition(AsCCreateNewAccount(self)->menuMessage, size);
    if ((unsigned int)self->selectedCharacterIndex < 2) {
        UIGroup_SetEnabled(UIGroup_GetOrCreateById(AsCCreateNewAccount(self)->primaryBpObject, kCharacter0Group), self->selectedCharacterIndex == 0);
        UIGroup_SetEnabled(UIGroup_GetOrCreateById(AsCCreateNewAccount(self)->primaryBpObject, kCharacter1Group), self->selectedCharacterIndex == 1);
    }
    selected = (unsigned int)((-self->selectedCharacterIndex) | self->selectedCharacterIndex) >> 31;
    Layout_SetElementEnabled(self->layout, 0xC, selected);
    oppositeCharacterBit = selected ^ 1;
    nonzeroOppositeCharacter = (-oppositeCharacterBit) | oppositeCharacterBit;
    oppositeCharacterEnabled = (unsigned int)nonzeroOppositeCharacter >> 31;
    Layout_SetElementEnabled(self->layout, 0xD, oppositeCharacterEnabled);
    // Phase 2: resolve the selected character and scale its three stat gauges.
    inventory = AsCSharedDoc(g_sharedDoc)->itemInventory;
    characterData = ItemInventory_FindCharacterData(inventory, ItemInventory_GetCharacterIdAt(inventory, self->selectedCharacterIndex));
    // The pair holds float bit patterns loaded as integer words. Only the first
    // word is overwritten with horizontal gauge scale; the second retains Y.
    // Power is proportional; control and spin saturate at their 30-point maximum.
    struct Pair { int powerStat, controlStat; };
    Pair gaugeScaleWords = { s_newAccountGaugeScaleX, s_newAccountGaugeScaleY };
    powerGauge = UIGroup_GetOrCreateById(AsCCreateNewAccount(self)->primaryBpObject, kPowerGaugeGroup);
    unsigned int powerStat = ((IFF_STRUCT::sChar*)characterData)->stats[kAccountPowerStat];
    *(float*)&gaugeScaleWords = kFloat_25_Stats * ((float)powerStat / kNewAccountPowerGaugeMaximum);
    UIGroup_SetScale(powerGauge, (float*)&gaugeScaleWords);
    controlGauge = UIGroup_GetOrCreateById(AsCCreateNewAccount(self)->primaryBpObject, kControlGaugeGroup);
    unsigned int controlStat = ((IFF_STRUCT::sChar*)characterData)->stats[kAccountControlStat];
    float controlScale;
    if ((float)controlStat >= kFloat_30_805E1518) controlScale = kFloat_25_Stats;
    else controlScale = kFloat_25_Stats * ((float)controlStat / kFloat_30_805E1518);
    *(float*)&gaugeScaleWords = controlScale;
    UIGroup_SetScale(controlGauge, (float*)&gaugeScaleWords);
    spinGauge = UIGroup_GetOrCreateById(AsCCreateNewAccount(self)->primaryBpObject, kSpinGaugeGroup);
    unsigned int spinStat = ((IFF_STRUCT::sChar*)characterData)->stats[kAccountSpinStat];
    float spinScale;
    if ((float)spinStat >= kFloat_30_805E1518) spinScale = kFloat_25_Stats;
    else spinScale = kFloat_25_Stats * ((float)spinStat / kFloat_30_805E1518);
    *(float*)&gaugeScaleWords = spinScale;
    UIGroup_SetScale(spinGauge, (float*)&gaugeScaleWords);
    // Phase 3: refresh the selected name and its script-controlled color/opacity.
    MenuMessage_ResetTextState(AsCCreateNewAccount(self)->menuMessage);
    if ((unsigned int)self->selectedCharacterIndex < 2) {
        nameGroup = UIGroup_GetOrCreateById(AsCCreateNewAccount(self)->primaryBpObject, kCharacterNameGroup);
        if (nameGroup) {
            color = UIGroup_GetTintColor(nameGroup);
            combinedOpacity = UIGroup_GetCombinedOpacity(nameGroup);
            // Tint is 0x00BBGGRR; renderer consumes 0xRRGGBBAA (PPC byte order).
            // For combinedOpacity=0..128, 0 maps to 0; otherwise 2*combinedOpacity-1 yields odd alpha 1..255.
            // This is the byte-safe input range; no clamping occurs here.
            int alphaByte = combinedOpacity ? 2*combinedOpacity - 1 : 0;
            unsigned int alpha = (unsigned int)alphaByte;
            unsigned int packed = alpha | ((color << 24) | ((color << 8) & 0xff0000) | ((color >> 8) & 0xff00));
            // Expand the group\'s combined opacity into the message renderer\'s packed color.
            MenuMessage_SetColorMultiplier(self->menuMessage, &packed);
            MenuMessage_SetText(self->menuMessage, ((int*)&s_newAccountCharacterNameMessageIds)[self->selectedCharacterIndex]);
        }
    }
    // Phase 4: rebuild the avatar from its rect and load validated saved equipment.
    int hasAvatarRect;
    if (self->layout != 0) {
        Layout_ReadElementRect(self->layout, kAccountAvatarElement, avatarRect);
        hasAvatarRect = 1;
    } else {
        hasAvatarRect = 0;
    }
    if (hasAvatarRect != 0) {
        if (self->avatar != 0) {
            ((void (**)(void*, int))(*(void***)self->avatar))[2](self->avatar, 1);
            self->avatar = 0;
        }
        self->avatar = (void*)CAsyncExhibitionAvatar_Create(avatarRect, 0, 10.0f);
        volatile int avatarCreated = self->avatar ? 1 : 0;
        if (avatarCreated != 0) {
            if (self->avatar != 0) {
                ItemSelection_Init(equipmentSelection);
                void* saveData = SaveData_AcquireInitializedInstance();
                if (SaveData_LoadValidatedItemSelection(saveData, equipmentSelection, self->selectedCharacterIndex)) CAsyncExhibitionAvatar_SetEquipment(self->avatar, equipmentSelection, 0);
            }
        }
    }
    // Phase 5: install loading-indicator position, request music and enter the fade menu.
    SoundManager_RequestBgmChange(g_soundManager, newAccountMenuStrings.tutorialMusic, 0, 1);
    pair[0] = kNewAccountAvatarLoadingPositionX;
    pair[1] = kNewAccountAvatarLoadingPositionY;
    CAsyncExhibition_SetLoadingIndicatorPosition(self->avatar, pair);
    IFadeMenuBase_StartMenuFadeOut(self);
}

typedef Fn80326994Data Fn8032735CData; // accepted/cancelled at +0x30/+0x31.

struct Fn80327E84Data {
    char prefix[0x60];
    unsigned int gaugeEnabled;
};

struct Fn80327E8CData {
    char prefix[0x64];
    float gaugeFill;
};

extern const float kFloat_6_2831855;
extern const float kIngameStatusInitialGaugeFill;
extern float s_newAccountFullTurn;
extern float s_ingameStatusFullTurn;
extern const char s_ingameStatusLayoutFilename[];
extern const char __vt__15CIngameStatus2d[];
extern int s_newAccountHiddenPrimaryGroupIds[];
extern int s_newAccountHiddenTitleGroupIds[];
extern NewAccountMenuStringPool newAccountMenuStrings;
extern char s_bpCtrl[];
extern char s_bpCtrlDtorRegistration_IngameStatus[];

void Layout_LoadExtended(void*, int, const char*);
void CPackTextureResource_PollLoad(void*);
int Layout_Update(void*);
void Layout_Init(void*, int);
void Layout_Destroy(void*, int);
void __dl__FPv(void*);
void Layout_SetElementEnabled(void*, int, int);
void Layout_SetElementPosition(void*, int, void*);
void Layout_GetElementPosition(void*, int, float*);
void Layout_GetElementSize(void*, int, float*);
void Layout_SetElementScale(void*, int, float*);
typedef LocalIntVector_u80326994 ActivatedList;
int Layout_CollectActivated(void*, ActivatedList&, int = 0);
void appFreeUni__FPv(void*);
void SaveData_Ctor(void*);
void* SaveData_GetCurrentRecord(void*);
void SaveData_SetCurrentRecord(void*, void*);
void SaveRecord_CopyCtor(void*, void*);
void* UIGroup_GetOrCreateById(void*, int);
void UIGroup_SetEnabled(void*, int);
void* IdMap_Lookup(void*, int);
void BpObject_StartScriptByName(void*, const char*, int);
void* BpObject_GetOrCreateNamedGroup(void*, int);
void __ct__7CBpCtrlFv(void*);
void CBpCtrl_dtor();
void* ItemSelection_Init(void*);
void* SaveData_AcquireInitializedInstance();
int SaveData_LoadValidatedItemSelection(void*, void*, signed char);
int CAsyncExhibitionAvatar_SetEquipment(void*, void*, int);
extern signed char s_saveDataInitialized;
extern char s_saveData[];
extern int s_ingameStatusIconIds[];
extern signed char s_inputManagerInitialized;
extern char s_gameInputManager[];
extern char s_newAccountInputManagerRegistration[];
extern signed char s_bpCtrlInitialized;
void InputManager_Init(void*);
void InputManager_Dtor();
void __register_global_object(void*, void (*)(), void*);
int Input_IsActionPressed(void*, int, int);
int Layout_SetElementValueState(void*, int, int);
void Layout_SetElementPlayerFlag(void*, int, int, int);
int CCreateNewAccount_ApplySelectionToSaveRecord(void*);


typedef Fn80326994Data Fn80327318Data; // child aliases textureResource at +0x40.

struct Fn80327CA0Pre { char prefix[0x88]; };
struct Fn80327CA0Data : Fn80327CA0Pre {
    virtual void v0(); virtual void v1(); virtual void v2(); virtual void v3(); virtual void v4(); virtual void v5(); virtual void v6(); virtual void v7(); virtual int check();
};

struct Fn80327CA0VTable {
    char prefix[0x28];
    int (*check)(void*);
};

// Load the in-game status layout resource.
int CIngameStatus2d_LoadLayout(void* arg0, int arg1) {
    Layout_LoadExtended(arg0, arg1, s_ingameStatusLayoutFilename);
    return 1;
}

// Poll the auxiliary texture resource, then update the account layout.
void CCreateNewAccount_UpdateWhileWaiting(Fn80327318Data* self) {
    if (self->child != 0)
        CPackTextureResource_PollLoad(self->child);
    Layout_Update(self->layout);
}

#pragma use_lmw_stmw on
#pragma opt_propagation off
// Combine layout readiness with the status component\'s virtual readiness check.
int CIngameStatus2d_UpdateLayoutAndGauge(Fn80327CA0Data* self) {
    int result = Layout_Update(self);
    result = result & 1;
    return result & self->check();
}
#pragma opt_propagation reset
#pragma opt_lifetimes reset
#pragma use_lmw_stmw reset

// Return whether account creation was accepted.
unsigned char Object_GetByte30(Fn8032735CData* self) {
    return self->accepted;
}

// Return whether account creation was cancelled.
unsigned char CCreateNewAccount_vfunc_1(Fn8032735CData* self) {
    return self->cancelled;
}

// Set whether the three-segment status gauge is enabled.
void CIngameStatus2d_vfunc_7(Fn80327E84Data* self, unsigned int value) {
    self->gaugeEnabled = value;
}

// Set the status gauge fill value.
void CIngameStatus2d_vfunc_5(Fn80327E8CData* self, float value) {
    self->gaugeFill = value;
}

struct Fn80326F4CController {
    virtual void vfunc0();
    virtual void update(float delta);
};

// Lazily initialize and borrow the shared fade-overlay instance.
static inline void* Fn80326F4CInstance() {
    if (Sing_805DC608() == 0) {
        if (!s_fadeOverlayInitialized) {
            FadeOverlay_Init((void*)s_fadeOverlay);
            __register_global_object((void*)s_fadeOverlay, ResourceState_Dtor, (void*)s_newAccountFadeOverlayRegistration);
            s_fadeOverlayInitialized = 1;
        }
        Sing_805DC608() = (void*)s_fadeOverlay;
    }
    return Sing_805DC608();
}

// Return true when no avatar exists or readiness polling completes.
static inline bool Fn80326F4CSkip(void* c) {
    if (c == 0 || CAsyncExhibition_PollReady(c) != 0) return true;
    return false;
}

// Handle confirm/cancel and preference changes, refreshing the selected character\'s gauges and avatar.
int CCreateNewAccount_UpdateSelectionAndAvatar(Fn80326F4CData* self, float delta) {
    void* entry;
    void* item;
    int sel;
    if (FadeState_GetState(Fn80326F4CInstance())) return 0;
    if (self->accepted || self->cancelled) {
        if (Fn80326F4CSkip(self->avatar)) return 1;
        ((Fn80326F4CController*)self->avatar)->update(delta);
        return 0;
    }
    Layout_Update(self->layout);
    int result = 0;
    // A selection change refreshes gauges and name; avatar updates continue below.
    if (!CCreateNewAccount_HandleConfirmCancel(self) && CCreateNewAccount_UpdateCharacterSelection(self)) {
        if ((unsigned int)self->selectedCharacterIndex < 2) {
            void* p = UIGroup_GetOrCreateById(self->primaryBpObject, kCharacter0Group);
            UIGroup_SetEnabled(p, self->selectedCharacterIndex == 0);
            p = UIGroup_GetOrCreateById(self->primaryBpObject, kCharacter1Group);
            UIGroup_SetEnabled(p, self->selectedCharacterIndex == 1);
        }
        sel = self->selectedCharacterIndex != 0;
        Layout_SetElementEnabled(self->layout, 0x0c, sel);
        int t = sel ^ 1;
        Layout_SetElementEnabled(self->layout, 0x0d, t != 0);
        // Refresh the selected character's three stat gauges.
        entry = AsCSharedDoc(g_sharedDoc)->itemInventory;
        item = ItemInventory_FindCharacterData(entry, ItemInventory_GetCharacterIdAt(entry, self->selectedCharacterIndex));
        // The pair holds float bit patterns loaded as integer words. Only the first
        // word is overwritten with horizontal gauge scale; the second retains Y.
        // Power is proportional; control and spin saturate at their 30-point maximum.
        struct Pair { int a, b; };
        Pair pr = { s_newAccountGaugeScaleX, s_newAccountGaugeScaleY };
        void* o = UIGroup_GetOrCreateById(self->primaryBpObject, kPowerGaugeGroup);
        unsigned int a = ((IFF_STRUCT::sChar*)item)->stats[kAccountPowerStat];
        *(float*)&pr = kFloat_25_Stats * ((float)a / kNewAccountPowerGaugeMaximum);
        UIGroup_SetScale(o, (float*)&pr);
        o = UIGroup_GetOrCreateById(self->primaryBpObject, kControlGaugeGroup);
        unsigned int b = ((IFF_STRUCT::sChar*)item)->stats[kAccountControlStat];
        float fb;
        if ((float)b >= kFloat_30_805E1518) fb = kFloat_25_Stats;
        else fb = kFloat_25_Stats * ((float)b / kFloat_30_805E1518);
        *(float*)&pr = fb;
        UIGroup_SetScale(o, (float*)&pr);
        o = UIGroup_GetOrCreateById(self->primaryBpObject, kSpinGaugeGroup);
        unsigned int c = ((IFF_STRUCT::sChar*)item)->stats[kAccountSpinStat];
        float fc;
        if ((float)c >= kFloat_30_805E1518) fc = kFloat_25_Stats;
        else fc = kFloat_25_Stats * ((float)c / kFloat_30_805E1518);
        *(float*)&pr = fc;
        UIGroup_SetScale(o, (float*)&pr);
        MenuMessage_ResetTextState(self->menuMessage);
        if ((unsigned int)self->selectedCharacterIndex < 2) {
            entry = UIGroup_GetOrCreateById(self->primaryBpObject, kCharacterNameGroup);
            if (entry) {
                int color = UIGroup_GetTintColor(entry);
                int n = UIGroup_GetCombinedOpacity(entry);
                // Convert 0x00BBGGRR to 0xRRGGBBAA; n=0..128 maps to
                // alpha 0 or 2*n-1. Values outside that range are not clamped.
                unsigned int alpha = n ? 2*n-1 : 0;
                unsigned int packed = alpha | ((color << 24) | ((color << 8) & 0xff0000) | ((color >> 8) & 0xff00));
                MenuMessage_SetColorMultiplier(self->menuMessage, &packed);
                MenuMessage_SetText(self->menuMessage, ((int*)&s_newAccountCharacterNameMessageIds)[self->selectedCharacterIndex]);
            }
        }
    }
    if (self->avatar) ((Fn80326F4CController*)self->avatar)->update(delta);
    return result;
}

struct Fn80327B5CData {
    char prefix[0x5C];
    int enabled;
    int gaugeEnabled;
    float gaugeFill;
    int iconElement;
    int labelElement;
    char beforeSecondaryVtable[0x18];
    const char* secondaryVtable;
};

// Initialize the status layout, visibility flags, gauge fill and secondary interface.
Fn80327B5CData* CIngameStatus2d_Ctor(Fn80327B5CData* self) {
    Layout_Init(self, 5);
    self->secondaryVtable = __vt__15CIngameStatus2d;
    self->enabled = 0;
    self->gaugeEnabled = 1;
    self->gaugeFill = kIngameStatusInitialGaugeFill;
    self->iconElement = 0;
    self->labelElement = 0;
    return self;
}

// Destroy the status layout and optionally free its allocation.
void* CIngameStatus2d_dtor(void* self, int shouldDelete) {
    if (self != 0) {
        Layout_Destroy(self, 0);
        if (shouldDelete > 0)
            __dl__FPv(self);
    }
    return self;
}

// Hide all 21 status elements, then enable the component and gauge.
int CIngameStatus2d_ResetElementsAndEnableGauge(void* self) {
    for (int i = 0; i <= 20; ++i)
        Layout_SetElementEnabled(self, i, 0);
    ((Fn80327B5CData*)self)->gaugeEnabled = 1;
    ((Fn80327B5CData*)self)->enabled = 1;
    return 1;
}

// Initialize this source group\'s full-circle constant.
void NewAccountMenu_InitFullTurnConstant() {
    s_newAccountFullTurn = kFloat_6_2831855;
}

// Initialize the status-display source group\'s full-circle constant.
void IngameStatus2d_InitFullTurnConstant() {
    s_ingameStatusFullTurn = kFloat_6_2831855;
}

#pragma use_lmw_stmw on
#pragma opt_propagation off
// Choose the status icon from its ID table and select the label for the supplied mode.
void CIngameStatus2d_SetIconAndLabelElements(void* self, int id, int base, int mode, int flag) {
    int a0, a1, a2, a3, a4, a5, a6, a7;
    int* p;
    int idx = 0;
    p = (int*)s_ingameStatusIconIds;
    a0 = *p++;
    a1 = *p++;
    a2 = *p++;
    a3 = *p++;
    a4 = *p++;
    a5 = *p++;
    a6 = *p++;
    a7 = *p;
    if (id == a0) {
        ((Fn80327B5CData*)self)->iconElement = idx;
    } else {
        idx = 1;
        if (id == a1) {
            ((Fn80327B5CData*)self)->iconElement = idx;
        } else {
            idx = 2;
            if (id == a2) {
                ((Fn80327B5CData*)self)->iconElement = idx;
            } else {
                idx = 3;
                if (id == a3) {
                    ((Fn80327B5CData*)self)->iconElement = idx;
                } else {
                    idx = 4;
                    if (id == a4) {
                        ((Fn80327B5CData*)self)->iconElement = idx;
                    } else {
                        idx = 5;
                        if (id == a5) {
                            ((Fn80327B5CData*)self)->iconElement = idx;
                        } else {
                            idx = 6;
                            if (id == a6) {
                                ((Fn80327B5CData*)self)->iconElement = idx;
                            } else {
                                idx = 7;
                                if (id == a7) {
                                    ((Fn80327B5CData*)self)->iconElement = idx;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    if (mode == 0) ((Fn80327B5CData*)self)->labelElement = base + 8;
    else if (flag != 0) ((Fn80327B5CData*)self)->labelElement = base + 13;
    else ((Fn80327B5CData*)self)->labelElement = 12;
}
#pragma opt_propagation reset
#pragma opt_lifetimes reset
#pragma use_lmw_stmw reset

// Reset the other button and its overlay in the same mutually exclusive option pair.
int CCreateNewAccount_ResetOtherButtonsInGroup(void* self, int value) {
    int first = 0, last = 0;
    if (value >= 0 && value <= 1) { first = 0; last = 1; }
    else if (value >= 2 && value <= 3) { first = 2; last = 3; }
    else if (value >= 4 && value <= 5) { first = 4; last = 5; }
    for (int i = first; i <= last; ++i) {
        if (value == i || i >= 17) continue;
        Layout_SetElementValueState(AsCCreateNewAccount(self)->layout, i, 0);
        Layout_SetElementPlayerFlag(AsCCreateNewAccount(self)->layout, i, 1, 5);
        if (i != 14 && i + 6 < 17) {
            Layout_SetElementValueState(AsCCreateNewAccount(self)->layout, i + 6, 0);
            Layout_SetElementPlayerFlag(AsCCreateNewAccount(self)->layout, i + 6, 1, 5);
        }
    }
    return 1;
}

#pragma opt_propagation off
#pragma opt_lifetimes off
// Publish cancellation or apply the confirmed choices; return 1 when either action is handled.
int CCreateNewAccount_HandleConfirmCancel(void* self) {
    if (Sing_805DC5C8() == 0) {
        if (!s_inputManagerInitialized) {
            InputManager_Init(s_gameInputManager);
            __register_global_object(s_gameInputManager, InputManager_Dtor, s_newAccountInputManagerRegistration);
            s_inputManagerInitialized = 1;
        }
        Sing_805DC5C8() = (void*)s_gameInputManager;
    }
    if (Input_IsActionPressed(Sing_805DC5C8(), kActionCancel, -1)) { ((AccountSelectionState*)self)->cancelled = 1; return 1; }
    ActivatedList list = {0, 0, 0};
    if (Layout_CollectActivated(AsCCreateNewAccount(self)->layout, list) && list.items[0] == kAccountConfirmElement) {
        ((AccountSelectionState*)self)->accepted = 1;
        CCreateNewAccount_ApplySelectionToSaveRecord(self);
        Layout_SetElementValueState(AsCCreateNewAccount(self)->layout, kAccountConfirmElement, 2);
        Layout_SetElementPlayerFlag(AsCCreateNewAccount(self)->layout, kAccountConfirmElement, 0, 5);
        return 1;
    }
    return 0;
}
#pragma opt_propagation reset
#pragma opt_lifetimes reset

// Position the current icon, label and all gauge elements.
void CIngameStatus2d_SetIconLabelGaugePositions(void* self, int first, int second, int third) {
    Layout_SetElementPosition(self, ((Fn80327B5CData*)self)->iconElement, (void*)first);
    Layout_SetElementPosition(self, ((Fn80327B5CData*)self)->labelElement, (void*)second);
    for (int i = 0x11; i <= 0x14; ++i)
        Layout_SetElementPosition(self, i, (void*)third);
}

#pragma use_lmw_stmw on
#pragma opt_propagation off
#pragma opt_lifetimes off
#pragma opt_strength_reduction off
#pragma opt_common_subs off
#pragma opt_loop_invariants off
// Return the first activated element in the six option buttons, or -1.
int CCreateNewAccount_GetActivatedOptionIndex(void* self) {
    ActivatedList list;
    unsigned int count;
    unsigned int invalid;
    int* items;
    list.items = 0;
    list.size = 0;
    list.capacity = 0;
    if (Layout_CollectActivated(AsCCreateNewAccount(self)->layout, list, 0) == 0) {
        return -1;
    }
    count = list.size;
    items = list.items;
    for (invalid = 0; invalid < count; ++invalid) {
        int val = items[invalid];
        if (val >= 0 && val <= 5)
            break;
    }
    if (invalid == list.size) {
        return -1;
    }
    items = list.items;
    int result = items[invalid];
    return result;
}
#pragma opt_strength_reduction reset
#pragma opt_loop_invariants reset
#pragma opt_propagation reset
#pragma opt_lifetimes reset
#pragma opt_common_subs reset
#pragma use_lmw_stmw reset

// Hide account script groups and initialize the auxiliary title script.
int CCreateNewAccount_InitBpGroups(void* self) {
    int* p = s_newAccountHiddenPrimaryGroupIds;
    unsigned int i = 0;
    do {
        void* obj = UIGroup_GetOrCreateById(AsCCreateNewAccount(self)->primaryBpObject, *p);
        UIGroup_SetEnabled(obj, 0);
        ++i;
        ++p;
    } while (i < 15);

    struct IdBlock { int v[9]; };
    int ids[9];
    *(IdBlock*)ids = *(const IdBlock*)s_newAccountHiddenTitleGroupIds;

    if (Sing_805DC5F0() == 0) {
        if (!s_bpCtrlInitialized) {
            __ct__7CBpCtrlFv((void*)s_bpCtrl);
            __register_global_object((void*)s_bpCtrl, CBpCtrl_dtor, (void*)s_bpCtrlDtorRegistration_IngameStatus);
            s_bpCtrlInitialized = 1;
        }
        Sing_805DC5F0() = (void*)s_bpCtrl;
    }
    void* manager = IdMap_Lookup(Sing_805DC5F0(), AsCCreateNewAccount(self)->auxiliaryBpId);
    BpObject_StartScriptByName(manager, newAccountMenuStrings.titleScript, 1);

    int* q = ids;
    unsigned int j = 0;
    do {
        void* item = BpObject_GetOrCreateNamedGroup(manager, *q);
        UIGroup_SetEnabled(item, 0);
        ++j;
        ++q;
    } while (j < 9);

    return 1;
}

struct Fn8032761CData {
    char prefix[0x18]; void* layout; void* primaryBpObject;
    char betweenBpAndOptions[0x12]; signed char selectedCharacterIndex;
    CW_BITS3(unsigned char flagA : 1, unsigned char flagB : 1, unsigned char rest : 6)   // CodeWarrior order: flagA = 0x80, flagB = 0x40 (the unit reads them with >>7 / >>6)
    void* avatar;
};

// Accept character choices 0/1 and load saved equipment into an existing avatar.
static inline int fn_8032761C_mode(Fn8032761CData* self, int value) {
    if (value < 0 || value > 1) return 0;
    self->selectedCharacterIndex = value;
    if (self->avatar != 0) {
        char local[0x68];
        ItemSelection_Init(local);
        void* v = SaveData_AcquireInitializedInstance();
        if (SaveData_LoadValidatedItemSelection(v, local, self->selectedCharacterIndex) != 0) {
            CAsyncExhibitionAvatar_SetEquipment(self->avatar, local, 0);
        }
    }
    return 1;
}

// Accept preference choices 2/3 and write option bit 7.
static inline int fn_8032761C_bit80(Fn8032761CData* self, int value) {
    if (value < 2 || value > 3) return 0;
    self->flagA = (value == 2);
    return 1;
}

// Accept starter-club choices 4/5 and write option bit 6.
static inline int fn_8032761C_bit40(Fn8032761CData* self, int value) {
    if (value < 4 || value > 5) return 0;
    self->flagB = (value == 4);
    return 1;
}

// Apply an activated option and its paired overlay; return 0 if no option was activated.
int CCreateNewAccount_UpdateCharacterSelection(void* p) {
    int sel;
    Fn8032761CData* self = (Fn8032761CData*)p;
    ActivatedList list = {0, 0, 0};
    if (Layout_CollectActivated(self->layout, list, 0) == 0)
        return 0;
    sel = CCreateNewAccount_GetActivatedOptionIndex(self);
    if (sel == -1)
        return 0;
    if (fn_8032761C_mode(self, sel) || fn_8032761C_bit80(self, sel) || fn_8032761C_bit40(self, sel)) {
        if (sel < kAccountElementCount) {
            Layout_SetElementValueState(self->layout, sel, 2);
            Layout_SetElementPlayerFlag(self->layout, sel, 0, 5);
            if (sel != kAccountConfirmElement && sel + kAccountSelectionOverlayOffset < kAccountElementCount) {
                Layout_SetElementValueState(self->layout, sel + 6, 2);
                Layout_SetElementPlayerFlag(self->layout, sel + 6, 0, 5);
            }
        }
        CCreateNewAccount_ResetOtherButtonsInGroup(self, sel);
    }
    return 1;
}

// Copy the save record, grant both starter characters, and store character, preference and starter club choices.
int CCreateNewAccount_ApplySelectionToSaveRecord(void* self) {
    unsigned char local[0x830];
    if (Sing_805DC5E0() == 0) {
        if (s_saveDataInitialized == 0) {
            SaveData_Ctor(s_saveData);
            s_saveDataInitialized = 1;
        }
        Sing_805DC5E0() = s_saveData;
    }
    SaveRecord_CopyCtor(local, SaveData_GetCurrentRecord(Sing_805DC5E0()));
    {
        unsigned char character0Status = ((NewAccountSaveFields*)local)->character0Status;
        unsigned char selectedCharacter = ((AccountSelectionState*)self)->selectedCharacterIndex;
        unsigned char grantedCharacter0Status = character0Status | kStarterCharacterGrantMask;
        unsigned char character1Status = ((NewAccountSaveFields*)local)->character1Status;
        AsSavePlayerRecord(local)->characterIndex = selectedCharacter;
        unsigned char grantedCharacter1Status = character1Status | kStarterCharacterGrantMask;
        int preferenceFlags = ((NewAccountSaveFields*)local)->preferenceFlags;
        ((NewAccountSaveFields*)local)->character0Status = grantedCharacter0Status;
        ((NewAccountSaveFields*)local)->character1Status = grantedCharacter1Status;
        unsigned char menuOptions = ((AccountSelectionState*)self)->optionBits;
        preferenceFlags = (preferenceFlags & ~kSavedPreferenceMask) | ((menuOptions << 23) & kSavedPreferenceMask);
        ((NewAccountSaveFields*)local)->preferenceFlags = preferenceFlags;
    }
    {
        unsigned char menuOptions = ((AccountSelectionState*)self)->optionBits;
        unsigned char starterClubSet = 0;
        if ((menuOptions >> 6) & 1) starterClubSet = kAlternateStarterClubSetIndex;
        AsSavePlayerRecord(local)->clubSetIndex = starterClubSet;
    }
    if (Sing_805DC5E0() == 0) {
        if (s_saveDataInitialized == 0) {
            SaveData_Ctor(s_saveData);
            s_saveDataInitialized = 1;
        }
        Sing_805DC5E0() = s_saveData;
    }
    SaveData_SetCurrentRecord(Sing_805DC5E0(), local);
    return 1;
}

// Status layout: 0..20 are reset; 17 is the reusable partial segment,
// 18..20 are complete segments. Each segment covers 33 points in 20 pixels,
// with a 21-pixel origin stride. Fill is truncated to whole points before scaling.
enum StatusGaugeLayout { kStatusLastElement = 20, kPartialGaugeElement = 17,
    kFirstFullGaugeElement = 18, kGaugeSegmentCount = 3 };
static const float kGaugePointsPerSegment = 33.0f;
static const float kGaugeSegmentWidthPixels = 20.0f;
static const float kGaugeSegmentStridePixels = 21.0f;

// Show the selected icon/label and draw up to three 33-point gauge segments.
int CIngameStatus2d_UpdateIconLabelAndGauge(void* self) {
    if (((Fn80327B5CData*)self)->enabled == 0)
        return 0;
    for (int i = 0; i <= kStatusLastElement; ++i)
        Layout_SetElementEnabled(self, i, 0);
    Layout_SetElementEnabled(self, ((Fn80327B5CData*)self)->iconElement, 1);
    Layout_SetElementEnabled(self, ((Fn80327B5CData*)self)->labelElement, 1);
    if (((Fn80327B5CData*)self)->gaugeEnabled == 0)
        return 1;
    for (int i = 0; i < kGaugeSegmentCount; ++i) {
        float raw = AsCIngameStatus2d(self)->gaugeFill - kGaugePointsPerSegment * (float)i;
        raw = (kGaugePointsPerSegment > raw) ? raw : kGaugePointsPerSegment;
        int value = (int)raw;
        if (value < 0)
            break;
        float converted = (float)value;
        if (converted == kGaugePointsPerSegment) {
            Layout_SetElementEnabled(self, i + kFirstFullGaugeElement, 1);
            float temp[2];
            Layout_GetElementPosition(self, i + kFirstFullGaugeElement, temp);
            temp[0] += kGaugeSegmentStridePixels * (float)i;
            Layout_SetElementPosition(self, i + kFirstFullGaugeElement, temp);
        } else {
            Layout_SetElementEnabled(self, kPartialGaugeElement, 1);
            float temp[2];
            Layout_GetElementSize(self, kPartialGaugeElement, temp);
            temp[1] = 1.0f;
            temp[0] = (kGaugeSegmentWidthPixels * (float)value) / kGaugePointsPerSegment / temp[0];
            Layout_SetElementScale(self, kPartialGaugeElement, temp);
            Layout_GetElementPosition(self, kPartialGaugeElement, temp);
            temp[0] += kGaugeSegmentStridePixels * (float)i;
            Layout_SetElementPosition(self, kPartialGaugeElement, temp);
        }
    }
    return 1;
}

} // extern "C"

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU80462618", _ctors_u80326994_0, NewAccountMenu_InitFullTurnConstant);
STATIC_INIT_ENTRY(".CRT$XCU80462618", _ctors_u80326994_1, IngameStatus2d_InitFullTurnConstant);
#pragma pop

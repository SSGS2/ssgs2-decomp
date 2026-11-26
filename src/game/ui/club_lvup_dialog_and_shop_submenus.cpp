// The club level-up dialog and the shop's equipment submenus: the panel that
// sells stat upgrades, and the item cart and equipped-item lists it is reached
// from. The dialog's numeric text and the shared string pools it reads are here
// as well.
#include <decomp/static_init.h>
#include <decomp/local_class.h>
#include "classes/UITextTable.h"
#include "classes/CItemCart.h"
#include "classes/SavePlayerRecord.h"
#include "classes/CSharedDoc.h"
#include "classes/CClubCustomTbl.h"
#define AsCSharedDoc(p) ((CSharedDoc*)(p))
#define GAME_VECTOR_AGGREGATE

#include "game/vector.h"

// Stack-local storage view; external vector operations retain the original ABI.
struct LocalIntVector_u802DD5C0 {
    int* items;
    volatile unsigned int size;
    unsigned int capacity;
    ~LocalIntVector_u802DD5C0() {
        if (items) {
            size = *(unsigned int*)&size - *(int*)&size;
            appFreeUni__FPv(items);
        }
    }
};

#include "game/singletons.h"
#include "classes/CClubLvUpDialog.h"
#include "classes/CShopSubMenuC.h"
#include "classes/CShopSubMenuB.h"
#include "classes/CClubHouseMsg.h"
#include "classes/CBpCtrl.h"
#include "classes/CFontNumDraw.h"
#define AsCShopSubMenuB(p) ((CShopSubMenuB*)(p))

#define AsCClubLvUpDialog(p) ((CClubLvUpDialog*)(p))
extern "C" {

// Relevant save-record regions. Shared SavePlayerRecord does not yet expose these fields.
struct ClubUpgradeLevel {
    unsigned char reserved;
    signed char level;
};
struct ShopPlayerRecord {
    char reserved_00[0x28];
    int pang;
    char reserved_2C[5];
    signed char equippedItems[8];
    char reserved_39[0xE3];
    // Addressable short-count region up to the club-level records; only item indices are used here.
    short itemCounts[0x390 / 2];
    // Addressable tail view; the inventory determines which club indices are valid.
    ClubUpgradeLevel clubLevels[(sizeof(SavePlayerRecord) - 0x4AC) / sizeof(ClubUpgradeLevel)];
};
typedef char ShopPlayerRecord_size[(sizeof(ShopPlayerRecord) == sizeof(SavePlayerRecord)) ? 1 : -1];
typedef char ShopPlayerRecord_club_level_offset[((int)&((ShopPlayerRecord*)0)->clubLevels == 0x4AC) ? 1 : -1];
struct ClubUpgradeEntry {
    short entryCode;
    short stats[3]; // Three displayed stat columns; their individual meanings are unresolved.
    int pangCost;
};
enum {
    kEquippedItemSlots = 8,
    kEmptyItemSlot = -1,
    kItemSlotEnabledFlag = 5,
    kItemRefreshDelayFrames = 3,
    kNoSelection = -1,
    kClubIconElement = 21,
    kUpgradeAllowedGroup = 0x42C,
    kUpgradeDeniedGroup = 0x459,
    kUpgradeDeniedMessageGroup = 0x46A,
    kClubUpgradeMessageBase = 0xE
};
enum UpgradeTextId {
    kTextCurrentPang = 0x25,
    kTextUpgradeCost = 0x26,
    kTextRemainingPang = 0x27,
    kTextCurrentStat0 = 0x28,
    kTextCurrentStat1 = 0x29,
    kTextCurrentStat2 = 0x2A,
    kTextNextStat0 = 0x3B,
    kTextNextStat1 = 0x3A,
    kTextNextStat2 = 0x39,
    kTextNextLevel = 0x3C
};

struct P2 {
    float x, y;
};
struct ClubIconInterface {
    virtual void unusedVirtual0();
    virtual void unusedVirtual1();
    virtual void unusedVirtual2();
    virtual void bindTexture();
    virtual int isReady();
};
extern int s_clubLevelDialogScaleZeroBits;
extern int s_clubLevelDialogScaleOneBits;
extern const float kFloat_6_2831855;
extern float s_sharedDialogTau;

extern const float kClubDialogSoundParameter0_0_5;
extern const float kClubDialogSoundParameter1_200;
extern const float kFloat_1_ClubLevelDialog;
extern const float kFloat_0_5_ItemCart;
extern const float kFloat_200_ItemCart;
extern const float kFloat_1_ItemCart;

extern void* g_sharedDoc;
extern void* g_soundManager;
extern signed char s_bpCtrlInitialized;
extern char s_bpCtrl[];
extern char s_bpCtrlDestructorRegistration[];
extern const char clubLevelUpDialogStrings[];

// Packed string-pool view; sizes include terminators and intervening bytes.
struct ClubDialogStrings {
    char layout[0xC];         // +0x0
    char scriptResource[0xC]; // +0xC
    char scriptName[0x8];     // +0x18
    char numberFont[0x10];    // +0x20
    char closeSound[0x10];    // +0x30
    char openSound[0xF];      // +0x40
    char itemNameGroup[0x10]; // +0x4F
};
inline const ClubDialogStrings& ClubStrings() {
    return *(const ClubDialogStrings*)clubLevelUpDialogStrings;
}

extern int kClubUpgradeInitiallyHiddenGroups[];
extern const char itemCartStrings[];

// Packed string-pool view; sizes include terminators and intervening bytes.
struct ItemCartStringPool {
    char prefix[0x2F];
    char clickSound[0xE];   // +0x2F
    char cancelSound[0x17]; // +0x3D
};
inline const ItemCartStringPool& CartStrings() {
    return *(const ItemCartStringPool*)itemCartStrings;
}

extern char kClubUpgradeNumericEntries[];
extern char __vt__15CClubLvUpDialog[];
extern char s_fontNumDrawRegistry[];
extern unsigned int kClubUpgradeIconQuadTemplate[24];
extern char s_fontNumDrawRegistryDestructorRegistration[];
extern signed char s_pointerVectorInitialized;
extern void* g_itemCartDescriptions[];

void Layout_Update(void* self);
void BpObject_SetEnabled(void* self, int value);
void Layout_SetModeByte(void* self, int value);
void BpObject_StartScriptByName(void* self, const char* name, int value);
void* UIGroup_GetOrCreateById(void* self, int id);
void UIGroup_SetEnabled(void* self, int value);
void Layout_SetElementEnabled(void* self, int index, int value);
void Layout_ReadElementRect(void* self, int index, int* value);
void Layout_WriteElementRect(void* self, int index, int* value);
void Layout_SetElementScale(void* self, int index, float* value);
void Layout_SetElementPlayerFlagsAndState(void* self, int index, int value);
void* BpObject_GetOrCreateNamedGroup(void* self, const char* name);
void Layout_SetElementPlayerFlag(void* self, int index, int value, int slot);
int ItemInventory_FindItemIndex(void* self, int value);
void MenuMessage_ResetTextState(void* self);
void MenuMessage_SetText(void* self, void* value);
void Layout_Destroy(void*, int);
void __ct__7CBpCtrlFv(void*);
void CBpCtrl_dtor(void);
void UITextTable_ClearEntries(void*);
void UITextTable_Draw(void*);
void PointerVector_InitReserved(void*);
void PointerArray_DeleteValue(void*, void*);
void PointerVector_DestroyStorage(void);
void __register_global_object(void*, void (*)(void), void*);
void __dl__FPv(void*);
void BpObject_TryInvokeCurrentCallback(void*, int);
void BpObject_Draw(void*);
void Layout_DrawWithOpacity(void*, float);
void Engine_SetFlexibleVertexFormat(int, float, float, float, float, float, float, float);
void* GX_GetPrimitiveCount(int, int);
void DrawPrimitiveUP(int, void*, void*, int);
// Release the dialog layout, script entry, message object and registered numeric text.
CClubLvUpDialog::~CClubLvUpDialog() {
    vtbl = __vt__15CClubLvUpDialog;
    if (layout) {
        Layout_Destroy(layout, 1);
        layout = 0;
    }
    if (bpEntryId != -1) {
        if (!Sing_805DC5F0()) {
            if (!s_bpCtrlInitialized) {
                ((CBpCtrl*)s_bpCtrl)->__ct();
                __register_global_object(s_bpCtrl, CBpCtrl_dtor, s_bpCtrlDestructorRegistration);
                s_bpCtrlInitialized = 1;
            }
            Sing_805DC5F0() = s_bpCtrl;
        }
        ((CBpCtrl*)Sing_805DC5F0())->DeleteEntryById(bpEntryId);
        bpEntryId = -1;
    }
    if (clubHouseMessage) {
        if (clubHouseMessage) {
            struct __V {
                virtual void c(int) DECOMP_LOCAL_VIRTUAL;
            };
            ((__V*)clubHouseMessage)->c(1);
        }
        clubHouseMessage = 0;
    }
    UITextTable_ClearEntries(textTable);
    if (textTable) {
        if (!Sing_805DC830()) {
            if (!s_pointerVectorInitialized) {
                PointerVector_InitReserved(s_fontNumDrawRegistry);
                __register_global_object(s_fontNumDrawRegistry, PointerVector_DestroyStorage,
                                         s_fontNumDrawRegistryDestructorRegistration);
                s_pointerVectorInitialized = 1;
            }
            Sing_805DC830() = s_fontNumDrawRegistry;
        }
        PointerArray_DeleteValue(Sing_805DC830(), textTable);
    }
}
int Layout_PollLoadComplete(void* self);
void __ct__7CBpCtrlFv(void* self);
void CBpCtrl_dtor(void);
void BpObject_TryInvokeCurrentCallback(void*, int);
void BpObject_Draw(void*);
void Layout_DrawWithOpacity(void*, float);
void Layout_ReadElementRect(void*, int, int*);
void UITextTable_ClearEntries(void*);
void UITextTable_Draw(void*);
void PointerVector_InitReserved(void*);
void PointerArray_DeleteValue(void*, void*);
void* GX_GetPrimitiveCount(int, int);
void Engine_SetFlexibleVertexFormat(int, float, float, float, float, float, float, float);
void DrawPrimitiveUP(int, void*, void*, int);
void Layout_Destroy(void*, int);
void PointerVector_DestroyStorage(void);
void __dl__FPv(void*);
void __register_global_object(void* a, void (*b)(void), void* c);
void UITextTable_SetInteger(void* self, int id, int value);
void NumericTextTable_ClearEntryText(void* self, int id);
void Layout_GetElementPosition(void* self, int index, float* value);
void Layout_SetElementPosition(void* self, int index, float* value);
void Layout_SetElementValueState(void* self, int index, int value);
void Sound_PlayByName(void* a, const char* b, int c, int d, int e, int f, float g, float h, float i);
void* __nw__FUl(unsigned int size);
void Layout_Init(void* self, int value);
void Layout_LoadExtended(void* self, int a, const char* name);
int IdMap_Lookup(void* self, int value);
int Layout_CollectActivated(void* layout, void* out, int flag);
void appFreeUni__FPv(void*);
int ItemInventory_FindItemIndex(void*, int);
void* memcpy(void*, const void*, unsigned int);

#pragma opt_propagation off
typedef LocalIntVector_u802DD5C0 Activated;
// Return the activated consumable to inventory, compact equipped slots, and refresh slot flags.
int CShopSubMenuB::RemoveActivatedItemSlot() {
    char* menu = (char*)this;
    Activated activated = {0, 0, 0};
    if (!Layout_CollectActivated(AsCShopSubMenuB(menu)->layout, &activated, 0)) {
        return 0;
    }
    AsCShopSubMenuB(menu)->activatedElementIndex = *(int*)activated.items;
    memcpy(AsCShopSubMenuB(menu)->previousItemSlots,
           ((ShopPlayerRecord*)AsCShopSubMenuB(menu)->itemRecord)->equippedItems, 8);
    int index = *(int*)activated.items;
    signed char inventoryIndex = ((ShopPlayerRecord*)AsCShopSubMenuB(menu)->itemRecord)->equippedItems[index];
    ((ShopPlayerRecord*)AsCShopSubMenuB(menu)->itemRecord)->itemCounts[inventoryIndex]++;
    ((ShopPlayerRecord*)AsCShopSubMenuB(menu)->itemRecord)->equippedItems[index] = kEmptyItemSlot;
    // Field-derived byte cursors preserve the original unrolled slot-copy instructions.
    for (int j = index; j < kEquippedItemSlots - 1; ++j)
        ((unsigned char*)AsCShopSubMenuB(menu)->itemRecord)[j + (int)&((ShopPlayerRecord*)0)->equippedItems] =
            ((unsigned char*)AsCShopSubMenuB(menu)->itemRecord)[j + (int)&((ShopPlayerRecord*)0)->equippedItems + 1];
    ((ShopPlayerRecord*)AsCShopSubMenuB(menu)->itemRecord)->equippedItems[kEquippedItemSlots - 1] = kEmptyItemSlot;
    for (int j = 0; j < kEquippedItemSlots; ++j) {
        if (((ShopPlayerRecord*)AsCShopSubMenuB(menu)->itemRecord)->equippedItems[j] == kEmptyItemSlot)
            Layout_SetElementPlayerFlag(AsCShopSubMenuB(menu)->layout, j, 0, kItemSlotEnabledFlag);
        else
            Layout_SetElementPlayerFlag(AsCShopSubMenuB(menu)->layout, j, 1, kItemSlotEnabledFlag);
    }
    Sound_PlayByName(g_soundManager, CartStrings().cancelSound, 0, 0, 0, 0, kFloat_0_5_ItemCart, kFloat_200_ItemCart,
                     kFloat_1_ItemCart);
    int act0 = *(int*)activated.items;
    Layout_SetElementValueState(AsCShopSubMenuB(menu)->layout, act0, 2);
    Layout_SetElementPlayerFlag(AsCShopSubMenuB(menu)->layout, act0, 0, kItemSlotEnabledFlag);
    ((CItemCart*)menu)->refreshDelayFrames = kItemRefreshDelayFrames;
    Sound_PlayByName(g_soundManager, CartStrings().clickSound, 0, 0, 0, 0, kFloat_0_5_ItemCart, kFloat_200_ItemCart,
                     kFloat_1_ItemCart);
    return 1;
}
#pragma opt_propagation reset

// Create the club-upgrade layout, script instance, message object and numeric text table.
CClubLvUpDialog::CClubLvUpDialog(void* playerRecord) {
    vtbl = __vt__15CClubLvUpDialog;
    void* obj = __nw__FUl(0x5c);
    if (obj)
        Layout_Init(obj, 5);
    layout = obj;
    bpObject = 0;
    bpEntryId = -1;
    clubIcon = 0;
    playerData = playerRecord;
    obj = __nw__FUl(0x20);
    if (obj)
        ((CClubHouseMsg*)obj)->__ct();
    (*(int*)&textTable) = 0;
    clubHouseMessage = obj;
    selectionDelayFrames = -1;
    selectedElement = -1;
    result = -1;
    clubIndex = -1;
    Layout_LoadExtended(layout, 0, clubLevelUpDialogStrings);
    if (!Sing_805DC5F0()) {
        if (!s_bpCtrlInitialized) {
            ((CBpCtrl*)s_bpCtrl)->__ct();
            __register_global_object(s_bpCtrl, CBpCtrl_dtor, s_bpCtrlDestructorRegistration);
            s_bpCtrlInitialized = 1;
        }
        Sing_805DC5F0() = s_bpCtrl;
    }
    // Keep byte-address formation here: taking &bpEntryId directly changes register allocation.
    ((CBpCtrl*)Sing_805DC5F0())
        ->CreateEntry((char*)clubLevelUpDialogStrings + (int)&((ClubDialogStrings*)0)->scriptResource,
                      (int*)((char*)this + (int)&((CClubLvUpDialog*)0)->bpEntryId), 0);
    if (!Sing_805DC5F0()) {
        if (!s_bpCtrlInitialized) {
            ((CBpCtrl*)s_bpCtrl)->__ct();
            __register_global_object(s_bpCtrl, CBpCtrl_dtor, s_bpCtrlDestructorRegistration);
            s_bpCtrlInitialized = 1;
        }
        Sing_805DC5F0() = s_bpCtrl;
    }
    bpObject = IdMap_Lookup(Sing_805DC5F0(), bpEntryId);
    if (!Sing_805DC830()) {
        if (!s_pointerVectorInitialized) {
            PointerVector_InitReserved(s_fontNumDrawRegistry);
            __register_global_object(s_fontNumDrawRegistry, PointerVector_DestroyStorage,
                                     s_fontNumDrawRegistryDestructorRegistration);
            s_pointerVectorInitialized = 1;
        }
        Sing_805DC830() = s_fontNumDrawRegistry;
    }
    textTable = CFontNumDraw::CreateInVector((PtrVector*)Sing_805DC830());
}

// Find an equipped consumable by inventory key, return one to stock, and compact the slots.
int CShopSubMenuB::RemoveItemSlotByKey(int value) {
    char* menu = (char*)this;
    for (int i = kEquippedItemSlots - 1; i >= 0; --i) {
        signed char inventoryIndex = ((ShopPlayerRecord*)AsCShopSubMenuB(menu)->itemRecord)->equippedItems[i];
        if (inventoryIndex != ItemInventory_FindItemIndex(AsCSharedDoc(g_sharedDoc)->itemInventory, value))
            continue;
        ((ShopPlayerRecord*)AsCShopSubMenuB(menu)->itemRecord)->itemCounts[inventoryIndex]++;
        ((ShopPlayerRecord*)AsCShopSubMenuB(menu)->itemRecord)->equippedItems[i] = kEmptyItemSlot;
        for (int j = i; j < kEquippedItemSlots - 1; ++j)
            ((unsigned char*)AsCShopSubMenuB(menu)->itemRecord)[j + (int)&((ShopPlayerRecord*)0)->equippedItems] =
                ((unsigned char*)AsCShopSubMenuB(menu)
                     ->itemRecord)[j + (int)&((ShopPlayerRecord*)0)->equippedItems + 1];
        ((ShopPlayerRecord*)AsCShopSubMenuB(menu)->itemRecord)->equippedItems[kEquippedItemSlots - 1] = kEmptyItemSlot;
        for (i = 0; i < kEquippedItemSlots; ++i) {
            if (((ShopPlayerRecord*)AsCShopSubMenuB(menu)->itemRecord)->equippedItems[i] == kEmptyItemSlot)
                Layout_SetElementPlayerFlag(AsCShopSubMenuB(menu)->layout, i, 0, kItemSlotEnabledFlag);
            else
                Layout_SetElementPlayerFlag(AsCShopSubMenuB(menu)->layout, i, 1, kItemSlotEnabledFlag);
        }
        Sound_PlayByName(g_soundManager, CartStrings().cancelSound, 0, 0, 0, 0, kFloat_0_5_ItemCart,
                         kFloat_200_ItemCart, kFloat_1_ItemCart);
        return 1;
    }
    return 0;
}

// Draw the dialog, optional loaded club icon, numeric values and explanatory message.
int CClubLvUpDialog::Draw() {
    struct W24 {
        unsigned int w[24];
    };
    char* p = (char*)this;
    BpObject_TryInvokeCurrentCallback((*(void**)&AsCClubLvUpDialog(p)->bpObject), 0);
    BpObject_Draw((*(void**)&AsCClubLvUpDialog(p)->bpObject));
    Layout_DrawWithOpacity(AsCClubLvUpDialog(p)->layout, kFloat_1_ClubLevelDialog);
    void* obj = (*(void**)&AsCClubLvUpDialog(p)->clubIcon);
    if (obj) {
        int got = ((ClubIconInterface*)obj)->isReady();
        if (got) {
            int rect[4] = {};
            Layout_ReadElementRect(AsCClubLvUpDialog(p)->layout, kClubIconElement, rect);
            unsigned int vertices[24];
            *(W24*)vertices = *(W24*)kClubUpgradeIconQuadTemplate;
            float a0 = (float)rect[0];
            float a1 = (float)rect[1];
            float a2 = (float)rect[2];
            float a3 = (float)rect[1];
            float a4 = (float)rect[0];
            float a5 = (float)rect[3];
            float a6 = (float)rect[2];
            float a7 = (float)rect[3];
            *(float*)&vertices[0] = a0;
            *(float*)&vertices[1] = a1;
            *(float*)&vertices[6] = a2;
            *(float*)&vertices[7] = a3;
            *(float*)&vertices[12] = a4;
            *(float*)&vertices[13] = a5;
            *(float*)&vertices[18] = a6;
            *(float*)&vertices[19] = a7;
            Engine_SetFlexibleVertexFormat(0x11F005, a6, a5, a4, a3, a2, a1, a0);
            ClubIconInterface* clubIconInterface = (*(ClubIconInterface**)&AsCClubLvUpDialog(p)->clubIcon);
            clubIconInterface->bindTexture();
            void* primitiveCount = GX_GetPrimitiveCount(4, 0x98);
            DrawPrimitiveUP(0x98, primitiveCount, vertices, 0x18);
        }
    }
    UITextTable_Draw(AsCClubLvUpDialog(p)->textTable);
    ((CClubHouseMsg*)(AsCClubLvUpDialog(p)->clubHouseMessage))->DrawText();
    return 1;
}

// Advance the club-upgrade layout animation.
int CClubLvUpDialog::UpdateLayout() {
    Layout_Update(layout);
    return 1;
}

// Initialize script groups, button rectangles, stat labels and message text.
int CClubLvUpDialog::InitLayout() {
    char* p = (char*)this;
    int j;
    Layout_SetModeByte(AsCClubLvUpDialog(p)->layout, 0);
    BpObject_StartScriptByName((*(void**)&AsCClubLvUpDialog(p)->bpObject), ClubStrings().scriptName, 1);
    BpObject_SetEnabled((*(void**)&AsCClubLvUpDialog(p)->bpObject), 0);
    unsigned int i = 0;
    int* arr = kClubUpgradeInitiallyHiddenGroups;
    while (i < 9) {
        UIGroup_SetEnabled(UIGroup_GetOrCreateById((*(void**)&AsCClubLvUpDialog(p)->bpObject), *arr), 0);
        ++i;
        ++arr;
    }
    for (int i = 9; i < 22; ++i)
        Layout_SetElementEnabled(AsCClubLvUpDialog(p)->layout, i, 0);
    int vals[4] = {};
    for (j = 0; j <= 2; ++j) {
        Layout_ReadElementRect(AsCClubLvUpDialog(p)->layout, j, vals);
        Layout_WriteElementRect(AsCClubLvUpDialog(p)->layout, j + 3, vals);
    }
    ((CFontNumDraw*)(AsCClubLvUpDialog(p)->textTable))
        ->RegisterEntries(kClubUpgradeNumericEntries, 10, (*(void**)&AsCClubLvUpDialog(p)->bpObject),
                          ClubStrings().numberFont);
    AsUITextTable(AsCClubLvUpDialog(p)->textTable)->drawEnabled = 0; // original +0x28
    ((CClubHouseMsg*)(AsCClubLvUpDialog(p)->clubHouseMessage))->CreateTextFromResource();
    return 1;
}

// Enable upgrade controls only when the player can afford the next level, then show the dialog.
int CClubLvUpDialog::Show() {
    int upgradeDenied;
    int canAffordUpgrade = 1;
    int clubIndex = this->clubIndex;
    ShopPlayerRecord* playerRecord = (ShopPlayerRecord*)playerData;
    signed char nextLevel = (unsigned char)playerRecord->clubLevels[clubIndex].level + 1;
    ClubUpgradeEntry* nextUpgrade =
        (ClubUpgradeEntry*)CClubCustomTbl::FindUpgradeEntry((signed char)clubIndex, nextLevel);
    int upgradeCost = nextUpgrade->pangCost;
    if (((ShopPlayerRecord*)playerData)->pang < upgradeCost)
        canAffordUpgrade = 0;
    result = -1;
    Layout_SetElementPlayerFlagsAndState(layout, 0, canAffordUpgrade);
    Layout_SetElementPlayerFlagsAndState(layout, 1, canAffordUpgrade);
    upgradeDenied = !canAffordUpgrade;
    Layout_SetElementPlayerFlagsAndState(layout, 2, upgradeDenied);
    Layout_SetElementPlayerFlagsAndState(layout, 3, canAffordUpgrade);
    Layout_SetElementPlayerFlagsAndState(layout, 4, canAffordUpgrade);
    Layout_SetElementPlayerFlagsAndState(layout, 5, upgradeDenied);
    UIGroup_SetEnabled(UIGroup_GetOrCreateById((*(void**)&bpObject), kUpgradeAllowedGroup), canAffordUpgrade);
    UIGroup_SetEnabled(UIGroup_GetOrCreateById((*(void**)&bpObject), kUpgradeDeniedGroup), upgradeDenied);
    UIGroup_SetEnabled(UIGroup_GetOrCreateById((*(void**)&bpObject), kUpgradeDeniedMessageGroup), upgradeDenied);
    BpObject_SetEnabled((*(void**)&bpObject), 1);
    AsUITextTable(textTable)->drawEnabled = 1; // original +0x28
    Layout_Update(layout);
    Sound_PlayByName(g_soundManager, ClubStrings().openSound, 0, 0, 0, 0, kClubDialogSoundParameter0_0_5,
                     kClubDialogSoundParameter1_200, kFloat_1_ClubLevelDialog);
    return 1;
}

#pragma opt_propagation off
// Refresh upgrade values, current stat-column widths, and the selected explanation message.
int CClubLvUpDialog::RefreshStatsAndMessage(int value) {
    UpdateUpgradeValues();
    int slot = clubIndex;
    ShopPlayerRecord* playerRecord = (ShopPlayerRecord*)playerData;
    signed char idx = playerRecord->clubLevels[slot].level;
    ClubUpgradeEntry* rec = (ClubUpgradeEntry*)CClubCustomTbl::FindUpgradeEntry((signed char)slot, idx);
    P2 v;
    ((int*)&v)[0] = s_clubLevelDialogScaleZeroBits;
    ((int*)&v)[1] = s_clubLevelDialogScaleOneBits;
    v.x = rec->stats[0];
    Layout_SetElementScale(layout, 6, &v.x);
    v.x = rec->stats[1];
    Layout_SetElementScale(layout, 7, &v.x);
    v.x = rec->stats[2];
    Layout_SetElementScale(layout, 8, &v.x);
    void* text = BpObject_GetOrCreateNamedGroup((*(void**)&bpObject), ClubStrings().itemNameGroup);
    ((CClubHouseMsg*)clubHouseMessage)->SetTextFromGroup((void*)(value + kClubUpgradeMessageBase), text, 0);
    return 1;
}
#pragma opt_propagation reset

// Wait for the shop layout, script entries and message resource to finish loading.
int CShopSubMenuC::IsLoaded() {
    if (Layout_PollLoadComplete(layout) == 0)
        return 0;
    if (Sing_805DC5F0() == 0) {
        if (s_bpCtrlInitialized == 0) {
            ((CBpCtrl*)s_bpCtrl)->__ct();
            __register_global_object(s_bpCtrl, CBpCtrl_dtor, s_bpCtrlDestructorRegistration);
            s_bpCtrlInitialized = 1;
        }
        Sing_805DC5F0() = s_bpCtrl;
    }
    if (((CBpCtrl*)Sing_805DC5F0())->PollAllEntriesLoaded() == 0)
        return 0;
    return ((CClubHouseMsg*)clubHouseMessage)->IsResourceLoaded();
}

// Count down click feedback; restore the selected button position when it finishes.
int CClubLvUpDialog::UpdateSelectionDelay() {
    int value = selectionDelayFrames;
    if (value == -1)
        return 0;
    selectionDelayFrames = value - 1;
    if (value > 0)
        return 1;
    float one = kFloat_1_ClubLevelDialog;
    for (int i = 0; i <= 2; ++i) {
        if (i == selectedElement) {
            float pos[2] = {0.0f, 0.0f};
            Layout_GetElementPosition(layout, i + 3, pos);
            pos[0] -= one;
            pos[1] -= one;
            Layout_SetElementPosition(layout, i + 3, pos);
        }
        Layout_SetElementValueState(layout, i, 0);
    }
    selectionDelayFrames = -1;
    return 1;
}

#pragma use_lmw_stmw on
// Populate current/next levels, stat columns, upgrade price and remaining Pang.
int CClubLvUpDialog::UpdateUpgradeValues() {
    int currentLevel = ((ShopPlayerRecord*)playerData)->clubLevels[clubIndex].level;
    ClubUpgradeEntry* currentUpgrade =
        (ClubUpgradeEntry*)CClubCustomTbl::FindUpgradeEntry((signed char)clubIndex, currentLevel);
    ClubUpgradeEntry* nextUpgrade = (ClubUpgradeEntry*)CClubCustomTbl::FindUpgradeEntry(
        (signed char)clubIndex, (signed char)((int)currentLevel + 1));
    int upgradeCost = nextUpgrade->pangCost;
    UITextTable_SetInteger(textTable, kTextNextLevel, currentLevel + 1);
    UITextTable_SetInteger(textTable, kTextCurrentPang, ((ShopPlayerRecord*)playerData)->pang);
    UITextTable_SetInteger(textTable, kTextUpgradeCost, upgradeCost);
    if (((ShopPlayerRecord*)playerData)->pang < upgradeCost)
        NumericTextTable_ClearEntryText(textTable, kTextRemainingPang);
    else
        UITextTable_SetInteger(textTable, kTextRemainingPang, ((ShopPlayerRecord*)playerData)->pang - upgradeCost);
    UITextTable_SetInteger(textTable, kTextCurrentStat0, currentUpgrade->stats[0]);
    UITextTable_SetInteger(textTable, kTextCurrentStat1, currentUpgrade->stats[1]);
    UITextTable_SetInteger(textTable, kTextCurrentStat2, currentUpgrade->stats[2]);
    UITextTable_SetInteger(textTable, kTextNextStat0, nextUpgrade->stats[0]);
    UITextTable_SetInteger(textTable, kTextNextStat1, nextUpgrade->stats[1]);
    UITextTable_SetInteger(textTable, kTextNextStat2, nextUpgrade->stats[2]);
    return 1;
}
#pragma use_lmw_stmw reset

// Enable or disable the shop script; clear message state when disabling it.
int CShopSubMenuB::SetEnabled(int enabled) {
    BpObject_SetEnabled(bpObject, enabled);
    if (enabled == 0)
        MenuMessage_ResetTextState(menuMessage);
    return 1;
}

// Advance layout and message reveal; process input after click feedback finishes.
int CClubLvUpDialog::Update() {
    Layout_Update(layout);
    ((CClubHouseMsg*)clubHouseMessage)->UpdateReveal();
    if (UpdateSelectionDelay() == 0)
        ProcessInput();
    return 1;
}

// Initialize the shared dialog float from its compiled default.
void InitializeSharedDialogTau(void) {
    s_sharedDialogTau = kFloat_6_2831855;
}

// Store the externally owned item texture table.
int CItemCart::SetItemIcons(void* value) {
    AsCShopSubMenuB(this)->productTextures = value;
    return 1;
}

// Store the externally owned product record table.
int CItemCart::SetItemRecords(void* value) {
    AsCShopSubMenuB(this)->products = value;
    return 1;
}

// Store the externally owned product count pointer.
int CItemCart::SetItemCount(void* value) {
    AsCShopSubMenuB(this)->productCount = value;
    return 1;
}

// Assign a non-null club-icon resource handle.
int CClubLvUpDialog::SetClubIcon(int icon) {
    if (icon == 0)
        return 0;
    clubIcon = icon;
    return 1;
}

// Select a nonnegative club index for upgrade queries.
int CClubLvUpDialog::SetClubIndex(int index) {
    if (index < 0)
        return 0;
    clubIndex = index;
    return 1;
}

// Resolve an inventory key and display its item description.
int CShopSubMenuB::SetItemMessage(int itemId) {
    if (itemId == 0)
        return 0;
    int idx = ItemInventory_FindItemIndex((void*)(AsCSharedDoc(g_sharedDoc)->itemInventory), itemId);
    MenuMessage_ResetTextState(menuMessage);
    MenuMessage_SetText(menuMessage, g_itemCartDescriptions[idx]);
    return 1;
}

// Enable each equipped-item icon only when its signed inventory index is present.
int CShopSubMenuB::UpdateItemSlotFlags() {
    for (int i = 0; i < kEquippedItemSlots; i++) {
        signed char inventoryIndex = ((ShopPlayerRecord*)itemRecord)->equippedItems[i];
        if (inventoryIndex == kEmptyItemSlot)
            Layout_SetElementPlayerFlag(layout, i, 0, kItemSlotEnabledFlag);
        else
            Layout_SetElementPlayerFlag(layout, i, 1, kItemSlotEnabledFlag);
    }
    return 1;
}

// Close only after a result is chosen and click feedback has finished.
int CClubLvUpDialog::Close() {
    if (result == -1 || selectionDelayFrames != -1)
        return 0;
    ((CClubHouseMsg*)clubHouseMessage)->ResetTextState();
    BpObject_SetEnabled((*(void**)&bpObject), 0);
    AsUITextTable(textTable)->drawEnabled = 0; // original +0x28
    Sound_PlayByName(g_soundManager, ClubStrings().closeSound, 0, 0, 0, 0, kClubDialogSoundParameter0_0_5,
                     kClubDialogSoundParameter1_200, kFloat_1_ClubLevelDialog);
    return 1;
}

} // extern "C"

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU80462558", _ctors_u802DD5C0_0, InitializeSharedDialogTau);
#pragma pop

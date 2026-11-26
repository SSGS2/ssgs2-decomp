// Shop screen selection and ownership checks.
//
// CPangya2Shop restores the saved outfit, handles product selection and its values, and tests
// whether an item is owned or equipped. It also handles profile save input and camera presets.

#include <decomp/static_init.h>
// Byte offsets into the existing string pool; retain its storage and encoding.
enum {
    kShopEquipClothSound = 0xAE,
    kShopRangeErrorType = 0xF4,
};

// Shop ownership snapshot: offsets include the shop's 0x18-byte prefix before the save profile.
// Parts are inventory indices, not IFF type IDs; -1 denotes an unequipped slot.
struct ShopCharacterEquipment {
    unsigned short ownershipFlags;
    short partIndices[24];
    signed char hairStyleIndex;
    signed char auxPartIndex;
};
struct ShopCaddieOwnership {
    unsigned char flags;
    char remainingRecord[7];
};
struct ShopClubOwnership {
    unsigned char flags;
    char remainingRecord;
};
struct ShopOwnershipView {
    char beforePartOwnership[0xC4];
    unsigned int ownedParts[25];
    unsigned int ownedCaddieItems;
    unsigned int ownedHairStyles[2];
    short consumableCounts[13];
    // Only the indexed prefix at +0x14E is known to hold auxiliary-part counts.
    // Its length is unresolved; the remaining save span is not an array of counts.
    signed char auxPartCountPrefix[1];
    char unclassifiedSaveBytes[0x195];
    ShopCharacterEquipment characters[8];
    ShopCaddieOwnership caddies[8];
    ShopClubOwnership clubSets[15];
    unsigned char ballFlags[1]; // Variable-length tail; only indexed access is used here.
};
struct SavedOutfitView {
    char beforeCharacters[0x2CC];
    ShopCharacterEquipment characters[8];
};
#define SHOP_OFFSET(field, offset) \
    typedef char ShopOffset_##field[((int)&((ShopOwnershipView*)0)->field == offset) ? 1 : -1]
SHOP_OFFSET(ownedParts, 0xC4);
SHOP_OFFSET(ownedCaddieItems, 0x128);
SHOP_OFFSET(ownedHairStyles, 0x12C);
SHOP_OFFSET(consumableCounts, 0x134);
SHOP_OFFSET(auxPartCountPrefix, 0x14E);
SHOP_OFFSET(characters, 0x2E4);
SHOP_OFFSET(caddies, 0x484);
SHOP_OFFSET(clubSets, 0x4C4);
SHOP_OFFSET(ballFlags, 0x4E2);
#undef SHOP_OFFSET
typedef char ShopCharacterEquipment_size[sizeof(ShopCharacterEquipment) == 0x34 ? 1 : -1];
enum ShopCategory {
    kShopCharacter = 0,
    kShopItems = 1,
    kShopCaddie = 2,
    kShopCategoryCount = 3
};
enum ItemKind {
    kItemCharacter = 1,
    kItemPart = 2,
    kItemClubSet = 4,
    kItemBall = 5,
    kItemConsumable = 6,
    kItemCaddie = 7,
    kItemCaddieEquipment = 8,
    kItemHairStyle = 15,
    kItemAuxPart = 28
};
enum ShopLimits {
    kPartSlotCount = 24,
    kPartOwnershipBits = 0x30E,
    kHairStyleOwnershipBits = 0x33,
    kCaddieItemOwnershipBits = 0x20
};
enum ShopTextEntry {
    kSelectedValueFirst = 0x2D,
    kSelectedValueSecond = 0x2E,
    kSelectedValueThird = 0x2F,
    kPurchaseAmountPang = 0x30
};
enum ShopInputAction {
    kCameraPresetAction = 0x1B,
    kCameraPresetAlternateAction = 0x25,
    kCameraPresetThirdAction = 0x17,
    kSaveProfileAction = 0x1D
};

// Script IDs describe the dispatch conditions here; the actual button-label text lives in assets.
// Each ordinary/entry pair uses characterEntryMode==0/!=0 respectively.
enum ShopControlHelpScript {
    kHelpProduct = 0,
    kHelpEmptyList = 1,
    kHelpInteractiveRotation = 2,
    kHelpItemTab2Product = 3,
    kHelpItemTab2Empty = 4,
    kHelpEntryProduct = 5,
    kHelpEntryEmptyList = 6,
    kHelpEntryItemTab2Product = 7,
    kHelpEntryItemTab2Empty = 8,
    kHelpAbsentPreviewProduct = 0x14,
    kHelpAbsentPreviewEmpty = 0x15,
    kHelpEntryAbsentPreviewProduct = 0x16,
    kHelpEntryAbsentPreviewEmpty = 0x17
};

#include "classes/CShopProductsTab.h"

#include "iff/IffStruct.h"
#include "classes/CSharedDoc.h"
#define AsCSharedDoc(p) ((CSharedDoc*)(p))
#include "classes/CPangya2Shop.h"

#define char_array_guard CharArrayGuard_u802CBF2C
#include "game/stdexcept_local.h"
#undef char_array_guard
#include "game/singletons.h"
#define __throw __throw_sdk_declaration
#include "sdk_protos.h"
#include "classes/CPadMemoryMenuSave.h"
#include "classes/CClubDialog.h"
#include "classes/CPlayerStatus.h"
#include "classes/CModelOperationChara.h"
#include "classes/CBpCtrlHandle.h"
extern "C" char s_saveData[];
extern "C" char s_inputManagerDtorRegistration_80516C10[];
#undef __throw
#define __register_global_object(a, b, c) __register_global_object(a, (void*)(b), c)

extern "C" {

void* __nw__FUl(unsigned long);
extern const char bitsetTestOutOfRangeMessage[];
extern const char shopHairOwnershipBoundsError[];
extern const char bitsetTestRangeError[];

extern float kFloat_6_2831855;
extern float s_shopTau;

struct EntryList {
    void* entries;
    unsigned int count;
};

int ShopProductTable_GetRecordAt(void*, int, int);
int UIGroup_GetOrCreateById(int, int);
void UIGroup_SetEnabled(int, int);
void UITextTable_SetInteger(void*, int, int);
void NumericTextTable_ClearEntryText(void*, int);
int LayoutManager_GetInstance();
int Input_IsActionPressed(void*, int, int);
int CPangya2Shop_AreSelectedPreviewsReady(void*);
int SaveRecordPrefix_ClearCtor(void*);
int SaveData_AcquireInitializedInstance();
int SaveData_PackProfileSections(void*, void*, void*);
int InputManager_GetSelectedPlayer(void*);
int ItemInventory_GetPricePang(void*, void*);
extern void* g_sharedDoc;
extern signed char s_saveDataInitialized;
extern void* g_soundManager;
extern signed char s_inputManagerInitialized;
extern char s_gameInputManager[0x408];

int InputManager_Init(void*);
void InputManager_Dtor();
int ModelOperationBase_IsInteractiveRotationEnabled(void*);
void ModelOperationBase_ApplyFixedCameraPreset(void*, int);
int ModelOperationBase_ApplyCameraPreset(void*, int);
int ModelOperationBase_EnableInteractiveRotation(void*);
int ItemInventory_FindCharacterIndex(void*, unsigned int);
int ItemInventory_FindPartIndex(void*, unsigned int);
int ItemInventory_FindAuxPartIndex(void*, unsigned int);
int ItemInventory_FindItemIndex(void*, unsigned int);
void* ItemInventory_GetPartIdAt(void*, int);
int ItemInventory_FindHairStyleIndex(void*, unsigned int);
int ItemInventory_FindBallIndex(void*, unsigned int);
int ItemInventory_FindClubSetIndex(void*, unsigned int);
int ItemInventory_FindCaddieIndex(void*, unsigned int);
int ItemInventory_FindCaddieItemIndex(void*, unsigned int);
int SaveData_Ctor(void*);
void* SaveData_GetCurrentRecord(void*);
int ItemInventory_GetAuxPartIdAt(void*, int);
int ItemInventory_GetHairStyleIdAt(void*, int);
void* ItemInventory_FindPartData(void*, int);
void Sound_PlayByName(void*, const char*, int, int, int, int, float, float, float);
extern const char shopStrings[];
extern float kFloat_1_805E0918;
extern float kFloat_0_5_shopSound;
extern float kFloat_200_Shop;
extern signed char s_bpCtrlHandleInitialized;
extern int s_bpCtrlHandle[2];
extern char s_bpCtrlHandleRegistration_80516C34[];
void CBpCtrlHandle_Dtor();

// Restore saved clothing, hair and auxiliary parts without overlapping body-slot hide masks.
int CPangya2Shop::RestoreSavedOutfit() {
    // Phase 1: borrow the saved character outfit and restore its auxiliary part/hair choice.
    int characterIndex = currentCharaIndex;
    short* previewParts = (short*)&((ShopOwnershipView*)this)->characters[characterIndex];
    if (!GetSaveDataSlot()) {
        if (!s_saveDataInitialized) {
            SaveData_Ctor((void*)((char*)&s_saveData));
            s_saveDataInitialized = 1;
        }
        GetSaveDataSlot() = (void*)((char*)&s_saveData);
    }
    void* global = SaveData_GetCurrentRecord(GetSaveDataSlot());
    short* savedParts = (short*)&((SavedOutfitView*)global)->characters[characterIndex];
    ((CModelOperationChara*)charaModelOperation)
        ->ToggleAttachment(0, ItemInventory_GetAuxPartIdAt(AsCSharedDoc(g_sharedDoc)->itemInventory,
                                                           ((ShopCharacterEquipment*)savedParts)->auxPartIndex));
    int value = ItemInventory_GetHairStyleIdAt(AsCSharedDoc(g_sharedDoc)->itemInventory,
                                               ((ShopCharacterEquipment*)savedParts)->hairStyleIndex);
    if (value)
        ((CModelOperationChara*)charaModelOperation)->SetHairStyle(value);
    // Phase 2: restore body parts while accumulating occupied hide-mask slots.
    unsigned int unusedMask = 0;
    unsigned int restoredSlots = 0;
    /* 0x802CC53C-0x802CC694: the item comes from the saved record (r31), the -1 test and the
       fallback id from self (r30); the inner scan starts at entry i with mask = restoredSlots and only
       ORs partHideMask that do not overlap the mask */
    for (int i = 0; i < kPartSlotCount; ++i, ++previewParts, ++savedParts) {
        void* item = ItemInventory_GetPartIdAt(AsCSharedDoc(g_sharedDoc)->itemInventory, savedParts[1]);
        if (!item) {
            if (previewParts[1] != -1) {
                unsigned int mask = restoredSlots;
                short* scan = savedParts;
                for (int j = i; j < kPartSlotCount; ++j, ++scan) {
                    if (scan[1] != -1) {
                        void* ent = ItemInventory_GetPartIdAt(AsCSharedDoc(g_sharedDoc)->itemInventory, scan[1]);
                        unsigned int savedPartHideMask = ((IFF_STRUCT::sPart*)ItemInventory_FindPartData(
                                                              AsCSharedDoc(g_sharedDoc)->itemInventory, (int)ent))
                                                             ->hideMask;
                        if (!(mask & savedPartHideMask))
                            mask |= savedPartHideMask;
                    }
                }
                void* id = ItemInventory_GetPartIdAt(AsCSharedDoc(g_sharedDoc)->itemInventory, previewParts[1]);
                unsigned int partHideMask =
                    ((IFF_STRUCT::sPart*)ItemInventory_FindPartData(AsCSharedDoc(g_sharedDoc)->itemInventory, (int)id))
                        ->hideMask;
                if (!(mask & partHideMask) && id && charaModelOperation) {
                    int p = ((CModelOperationChara*)charaModelOperation)->ApplyPartChanges((int)id);
                    ((CModelOperationChara*)charaModelOperation)->RequestModelUpdate(p);
                }
            }
        } else {
            unsigned int partHideMask =
                ((IFF_STRUCT::sPart*)ItemInventory_FindPartData(AsCSharedDoc(g_sharedDoc)->itemInventory, (int)item))
                    ->hideMask;
            if (!(restoredSlots & partHideMask)) {
                restoredSlots |= partHideMask;
                if (savedParts[1] != previewParts[1] && item && charaModelOperation) {
                    int p = ((CModelOperationChara*)charaModelOperation)->ApplyPartChanges((int)item);
                    ((CModelOperationChara*)charaModelOperation)->RequestModelUpdate(p);
                }
            }
        }
    }
    // Phase 3: persist the resulting preview and refresh status, product list and sound feedback.
    ((CModelOperationChara*)charaModelOperation)->WriteEquipmentToSave();
    CPlayerStatus::RefreshEquipmentDisplay(playerStatus, categoryIndex);
    int selected = selectedListIndex;
    if (selected != -1)
        ((CShopProductsTab*)(((void**)productLists)[selected]))->RefreshVisibleProducts();
    UpdateUnownedSelectedOutfitCategoryMarkers();
    Sound_PlayByName(g_soundManager, (char*)shopStrings + kShopEquipClothSound, 0, 0, 0, 0, kFloat_0_5_shopSound,
                     kFloat_200_Shop, kFloat_1_805E0918);
    return 1;
}

// Publish/register the control-help singleton without choosing or starting a script.
// Call at the original branch site so registration still precedes entry-mode reads.
static inline void EnsureShopControlHelpHandle() {
    if (!Sing_805DC5F8()) {
        if (!s_bpCtrlHandleInitialized) {
            s_bpCtrlHandle[0] = -1;
            s_bpCtrlHandle[1] = -1;
            __register_global_object(s_bpCtrlHandle, CBpCtrlHandle_Dtor, s_bpCtrlHandleRegistration_80516C34);
            s_bpCtrlHandleInitialized = 1;
        }
        Sing_805DC5F8() = s_bpCtrlHandle;
    }
}

// Select the control-help script for the active product tab, preview and entry mode.
// Always returns 1. Priority: item-info hides help; interactive rotation uses script 2;
// otherwise choose from item-tab-2, absent-preview, or ordinary product/empty-list pairs.
// Repeated singleton setup preserves the original lazy-init/registration timing on each exit.
int CPangya2Shop::UpdateControlScriptSelection() {
    // Original 0x802CD154: all script choices share the successful return.
    do {
        // Phase 1: suppress control help while the item-information overlay owns the UI.
        if (itemInfoOpen) {
            EnsureShopControlHelpHandle();
            ((CBpCtrlHandle*)Sing_805DC5F8())->ReleaseEntry();
            break;
        }

        // Phase 2: query rotation mode of the active preview, independently of product ownership.
        int interactiveRotationEnabled = 0; /* No rotation for absent caddie, item tabs >1, or unknown category. */
        int categoryForPreview = categoryIndex;
        if (categoryForPreview == kShopCharacter)
            interactiveRotationEnabled = ModelOperationBase_IsInteractiveRotationEnabled(charaModelOperation);
        else if (categoryForPreview == kShopCaddie) {
            if (!noCaddieSelected)
                interactiveRotationEnabled = ModelOperationBase_IsInteractiveRotationEnabled(caddieModelOperation);
        } else if (categoryForPreview == kShopItems) {
            int mode = selectedListIndex;
            if (mode == 0)
                interactiveRotationEnabled = ModelOperationBase_IsInteractiveRotationEnabled(primaryItemModelOperation);
            else if (mode == 1)
                interactiveRotationEnabled =
                    ModelOperationBase_IsInteractiveRotationEnabled(secondaryItemModelOperation);
        }
        if (interactiveRotationEnabled) {
            EnsureShopControlHelpHandle();
            ((CBpCtrlHandle*)Sing_805DC5F8())->SetScriptIndex(kHelpInteractiveRotation);
            break;
        }

        // Phase 3: dispatch by category/tab and list availability, then by character entry mode.
        int category = categoryIndex;
        int listIndex = selectedListIndex;
        if (category == kShopItems && listIndex == 2) {
            void* item = ((void**)productLists)[listIndex];
            if (((CShopProductsTab*)item)->productCount == 0) {
                EnsureShopControlHelpHandle();
                {
                    int entryMode = characterEntryMode;
                    ((CBpCtrlHandle*)Sing_805DC5F8())
                        ->SetScriptIndex(entryMode ? kHelpEntryItemTab2Empty : kHelpItemTab2Empty);
                }
                break;
            } else {
                EnsureShopControlHelpHandle();
                {
                    int entryMode = characterEntryMode;
                    ((CBpCtrlHandle*)Sing_805DC5F8())
                        ->SetScriptIndex(entryMode ? kHelpEntryItemTab2Product : kHelpItemTab2Product);
                }
                break;
            }
        }
        if ((category == kShopCaddie && noCaddieSelected) || (category == kShopItems && listIndex == -1)) {
            if (listIndex == -1 || ((CShopProductsTab*)((void**)productLists)[listIndex])->productCount == 0) {
                EnsureShopControlHelpHandle();
                {
                    int entryMode = characterEntryMode;
                    ((CBpCtrlHandle*)Sing_805DC5F8())
                        ->SetScriptIndex(entryMode ? kHelpEntryAbsentPreviewEmpty : kHelpAbsentPreviewEmpty);
                }
                break;
            } else {
                EnsureShopControlHelpHandle();
                {
                    int entryMode = characterEntryMode;
                    ((CBpCtrlHandle*)Sing_805DC5F8())
                        ->SetScriptIndex(entryMode ? kHelpEntryAbsentPreviewProduct : kHelpAbsentPreviewProduct);
                }
                break;
            }
        }
        if (listIndex == -1 || ((CShopProductsTab*)((void**)productLists)[listIndex])->productCount == 0) {
            EnsureShopControlHelpHandle();
            {
                int entryMode = characterEntryMode;
                ((CBpCtrlHandle*)Sing_805DC5F8())->SetScriptIndex(entryMode ? kHelpEntryEmptyList : kHelpEmptyList);
            }
            break;
        } else {
            EnsureShopControlHelpHandle();
            {
                int entryMode = characterEntryMode;
                int scriptIndex = kHelpProduct;
                if (entryMode)
                    scriptIndex = kHelpEntryProduct;
                ((CBpCtrlHandle*)Sing_805DC5F8())->SetScriptIndex(scriptIndex);
            }
            break;
        }
    } while (0);
    return 1;
}

// Activate camera control for the selected character, caddie or item preview.
int CPangya2Shop::ActivateModelCameraPreset() {
    if (!GetInputManagerSlot()) {
        if (!s_inputManagerInitialized) {
            InputManager_Init((void*)((char*)&s_gameInputManager));
            __register_global_object((void*)((char*)&s_gameInputManager), InputManager_Dtor,
                                     (void*)((char*)&s_inputManagerDtorRegistration_80516C10));
            s_inputManagerInitialized = 1;
        }
        GetInputManagerSlot() = (void*)((char*)&s_gameInputManager);
    }
    void* input = GetInputManagerSlot();
    if (categoryIndex == kShopCaddie && noCaddieSelected)
        return 0;
    if (categoryIndex == kShopItems && selectedListIndex == -1)
        return 0;
    int interactiveRotationEnabled = 0;
    if (categoryIndex == kShopCharacter)
        interactiveRotationEnabled = ModelOperationBase_IsInteractiveRotationEnabled(charaModelOperation);
    else if (categoryIndex == kShopCaddie) {
        if (!noCaddieSelected)
            interactiveRotationEnabled = ModelOperationBase_IsInteractiveRotationEnabled(caddieModelOperation);
    } else if (categoryIndex == kShopItems) {
        int mode = selectedListIndex;
        if (mode == 0)
            interactiveRotationEnabled = ModelOperationBase_IsInteractiveRotationEnabled(primaryItemModelOperation);
        else
            interactiveRotationEnabled =
                (mode == 1) ? ModelOperationBase_IsInteractiveRotationEnabled(secondaryItemModelOperation)
                            : interactiveRotationEnabled;
    }
    /* 0x802CC91C: bne -> return 0 when the flag is SET (CPangya2Shop::HandleFixedCameraPresetInput uses beq) */
    if (interactiveRotationEnabled || !Input_IsActionPressed(input, kCameraPresetAction, -1))
        return 0;
    if (categoryIndex == kShopCharacter) {
        void* p = charaModelOperation;
        ModelOperationBase_ApplyCameraPreset(p, (int)currentCharaIndex + 8);
        ModelOperationBase_EnableInteractiveRotation(p);
    } else if (categoryIndex == kShopCaddie) {
        void* p = caddieModelOperation;
        ModelOperationBase_ApplyCameraPreset(p, (int)currentCaddieIndex + 8);
        ModelOperationBase_EnableInteractiveRotation(p);
    } else if (categoryIndex == kShopItems) {
        int mode = selectedListIndex;
        if (mode == 0) {
            void* p = primaryItemModelOperation;
            ModelOperationBase_ApplyCameraPreset(p, 1);
            ModelOperationBase_EnableInteractiveRotation(p);
        } else if (mode == 1) {
            void* p = secondaryItemModelOperation;
            ModelOperationBase_ApplyCameraPreset(p, 1);
            ModelOperationBase_EnableInteractiveRotation(p);
        } /* 0x802CC9C8: other modes fall through to CPangya2Shop::UpdateControlScriptSelection and return 1 */
    } else
        return 0;
    UpdateControlScriptSelection();
    return 1;
}

// Test whether the type ID is equipped in the current shop preview; preserve the existing club/ball index choice.
int CPangya2Shop::IsItemSelected(unsigned int typeId) {
    if (!typeId)
        return 0;
    unsigned int kind = typeId >> 26;
    if (kind > 15)
        return 0;
    if (kind == 1) {
        int value = ItemInventory_FindCharacterIndex(AsCSharedDoc(g_sharedDoc)->itemInventory, typeId);
        return currentCharaIndex == value;
    }
    if (kind == 2) {
        int index = currentCharaIndex;
        // Keep byte-addition order: a direct array address changes register allocation here.
        short* values = (short*)((char*)this + index * sizeof(ShopCharacterEquipment) +
                                 (int)&((ShopOwnershipView*)0)->characters[0].partIndices);
        for (int i = 0; i < kPartSlotCount; ++i) {
            if (values[i] != -1 &&
                (void*)typeId == ItemInventory_GetPartIdAt(AsCSharedDoc(g_sharedDoc)->itemInventory, values[i]))
                return 1;
        }
        return 0;
    }
    if (kind ==
        15) { /* jumptable_804B69E4 slot 15 -> 0x802CCFF4 (ItemInventory_FindHairStyleIndex); slot 3 is the default return 0 */
        int index = currentCharaIndex;
        return ItemInventory_FindHairStyleIndex(AsCSharedDoc(g_sharedDoc)->itemInventory, typeId) ==
               ((ShopOwnershipView*)this)->characters[index].hairStyleIndex;
    }
    // ballIndex is written by ball selection (shop_product_selection_and_purchase.cpp:234); club
    // selection writes primaryItemIndex (:217). Both tests below nevertheless use
    // ballIndex, as does shop_product_selection_and_purchase.cpp:630-633. Thus the club test compares
    // across two inventory-index domains; it is not proof of the equipped club.
    if (kind == 5)
        return ItemInventory_FindBallIndex(AsCSharedDoc(g_sharedDoc)->itemInventory, typeId) == ballIndex;
    if (kind == 4)
        return ItemInventory_FindClubSetIndex(AsCSharedDoc(g_sharedDoc)->itemInventory, typeId) == ballIndex;
    if (kind == 7)
        return ItemInventory_FindCaddieIndex(AsCSharedDoc(g_sharedDoc)->itemInventory, typeId) == currentCaddieIndex;
    if (kind == 8) {
        int index = (typeId >> 21) & 0x1F;
        if (currentCaddieIndex != index)
            return 0;
        return ItemInventory_FindCaddieItemIndex(AsCSharedDoc(g_sharedDoc)->itemInventory, typeId) == caddieItemIndex;
    }
    return 0;
}

// Initialize this shop unit's angle-conversion constant at startup.
void Shop_InitTauConstant() {
    s_shopTau = kFloat_6_2831855;
}

// Refresh the active product tab when a tab is selected.
int CPangya2Shop::RefreshSelectedProductList() {
    int index = selectedListIndex;
    if (index == -1)
        return 0;
    ((CShopProductsTab*)(((void**)(productLists))[index]))->RefreshVisibleProducts();
    return 1;
}

// Resolve the selected row through the character, item or caddie product builder.
int CPangya2Shop::GetSelectedProductRecord() {
    int index = selectedListIndex;
    if (index == -1)
        return 0;
    int productIndex = (int)((CShopProductsTab*)(((void**)(productLists))[index]))->GetSelectedProductIndex();
    int category = categoryIndex;
    int productRecord = 0;
    // Retain this dispatch tree and body order: native switches remove a required comparison.
    do {
        if (category == kShopItems)
            goto BuildItemRecord;
        if (category < kShopCaddie) {
            if (category >= kShopCharacter) {
                // Valid character category reaches the first builder below.
            } else {
                break;
            }
        } else {
            if (category >= kShopCategoryCount)
                break;
            goto BuildCaddieRecord;
        }
        productRecord = ShopProductTable_GetRecordAt(charaProductBuilder, selectedListIndex, productIndex);
        break;
    BuildItemRecord:
        productRecord = ShopProductTable_GetRecordAt(itemProductBuilder, selectedListIndex, productIndex);
        break;
    BuildCaddieRecord:
        productRecord = ShopProductTable_GetRecordAt(caddieProductBuilder, selectedListIndex, productIndex);
    } while (0);
    return productRecord;
}

// Show the selected product values only when product information is available.
int CPangya2Shop::RefreshSelectedProductValues() {
    struct Locals {
        int temp;
        short selectedValueFirst;
        short selectedValueSecond;
        short selectedValueThird;
        char x[4];
        char d[40];
        char e[40];
    } locals;
    int result = 1;
    locals.temp = 0;
    Memory_Clear(&locals.selectedValueFirst, 10);
    Memory_Clear(&locals.d, 40);
    Memory_Clear(&locals.e, 40);
    if (!((CPangya2Shop*)this)->GetSelectedProductInfo(&locals.temp) ||
        (locals.selectedValueFirst + locals.selectedValueSecond + locals.selectedValueThird) == 0)
        result = 0;
    for (int id = 0x405; id <= 0x407; ++id) {
        int found = UIGroup_GetOrCreateById((int)uiGroup,
                                            id); // CPangya2Shop::RefreshSelectedProductValues: self + 0x860
        if (found)
            UIGroup_SetEnabled(found, result);
    }
    if (result) {
        UITextTable_SetInteger(textTable, kSelectedValueFirst, locals.selectedValueFirst);
        UITextTable_SetInteger(textTable, kSelectedValueSecond, locals.selectedValueSecond);
        UITextTable_SetInteger(textTable, kSelectedValueThird, locals.selectedValueThird);
    } else {
        NumericTextTable_ClearEntryText(textTable, kSelectedValueFirst);
        NumericTextTable_ClearEntryText(textTable, kSelectedValueSecond);
        NumericTextTable_ClearEntryText(textTable, kSelectedValueThird);
    }
    return 1;
}

// Apply a fixed camera preset when a preview is active and a camera action is pressed.
int CPangya2Shop::HandleFixedCameraPresetInput() {
    if (!GetInputManagerSlot()) {
        if (!s_inputManagerInitialized) {
            InputManager_Init((void*)((char*)&s_gameInputManager));
            __register_global_object((void*)((char*)&s_gameInputManager), InputManager_Dtor,
                                     (void*)((char*)&s_inputManagerDtorRegistration_80516C10));
            s_inputManagerInitialized = 1;
        }
        GetInputManagerSlot() = (void*)((char*)&s_gameInputManager);
    }
    void* input = GetInputManagerSlot();
    int interactiveRotationEnabled = 0;
    int t = categoryIndex;
    if (t == kShopCharacter)
        interactiveRotationEnabled = ModelOperationBase_IsInteractiveRotationEnabled(charaModelOperation);
    else if (t == kShopCaddie) {
        if (!noCaddieSelected)
            interactiveRotationEnabled = ModelOperationBase_IsInteractiveRotationEnabled(caddieModelOperation);
    } else if (t == kShopItems) {
        int mode = selectedListIndex;
        if (mode == 0)
            interactiveRotationEnabled = ModelOperationBase_IsInteractiveRotationEnabled(primaryItemModelOperation);
        else
            interactiveRotationEnabled =
                (mode == 1) ? ModelOperationBase_IsInteractiveRotationEnabled(secondaryItemModelOperation)
                            : interactiveRotationEnabled;
    }
    if (!interactiveRotationEnabled || (!Input_IsActionPressed(input, kCameraPresetAction, -1) &&
                                        !Input_IsActionPressed(input, kCameraPresetAlternateAction, -1) &&
                                        !Input_IsActionPressed(input, kCameraPresetThirdAction, -1)))
        return 0;
    t = categoryIndex;
    if (t == kShopCharacter)
        ModelOperationBase_ApplyFixedCameraPreset(charaModelOperation, currentCharaIndex);
    else if (t == kShopCaddie) {
        if (!noCaddieSelected)
            ModelOperationBase_ApplyFixedCameraPreset(caddieModelOperation, currentCaddieIndex);
    } else if (t == kShopItems) {
        int mode = selectedListIndex;
        if (mode == 0)
            ModelOperationBase_ApplyFixedCameraPreset(primaryItemModelOperation, 0);
        else if (mode == 1)
            ModelOperationBase_ApplyFixedCameraPreset(secondaryItemModelOperation, 0);
    } else
        return 0;
    UpdateControlScriptSelection();
    return 1;
}

// Show the Pang purchase amount for a selected, unowned product outside character entry.
int CPangya2Shop::UpdateSelectedPurchaseAmount() {
    struct Locals {
        int pricePang;
        char a[10];
        char b[40];
        char c[40];
    } locals;
    int result = 1;
    locals.pricePang = 0;
    Memory_Clear(locals.a, 10);
    Memory_Clear(locals.b, 40);
    Memory_Clear(locals.c, 40);
    // Original 0x802CCE08: all validation failures clear the result.
    do {
        if (characterEntryMode == 0 && selectedListIndex != -1) {
            int index = selectedListIndex;
            void* entry = ((void**)(productLists))[index];
            if (((CShopProductsTab*)entry)->IsSelectedProductOwned() == 0 &&
                ((CPangya2Shop*)this)->GetSelectedProductInfo(&locals.pricePang) != 0)
                break;
        }
        result = 0;
    } while (0);
    int found = UIGroup_GetOrCreateById((int)uiGroup,
                                        0x3F0); // CPangya2Shop::UpdateSelectedPurchaseAmount: self + 0x860
    if (!found)
        return 0;
    UIGroup_SetEnabled(found, result);
    if (result)
        UITextTable_SetInteger(textTable, kPurchaseAmountPang, locals.pricePang);
    else
        NumericTextTable_ClearEntryText(textTable, kPurchaseAmountPang);
    return 1;
}

class Y4 {
public:
    virtual void v08(int);
};
// Save the preview outfit, or open the unowned-parts dialog before allowing a profile save.
int CPangya2Shop::HandleProfileSaveInput() {
    void* input = (void*)LayoutManager_GetInstance();
    if (characterEntryMode || !Input_IsActionPressed(input, kSaveProfileAction, -1))
        return 0;
    if (!CPangya2Shop_AreSelectedPreviewsReady(this))
        return 0;
    if (HasUnownedSelectedParts()) {
        clubDialogOpen = 1;
        ((CClubDialog*)(clubDialog))->SetMode(2);
        return 1;
    }
    void* old = saveMenu;
    if (old) {
        if (old) {
            ((Y4*)old)->v08(1);
        }
        saveMenu = 0;
    }
    char data[0x480];
    SaveRecordPrefix_ClearCtor(data);
    SaveData_PackProfileSections((void*)SaveData_AcquireInitializedInstance(), data, ((char*)&saveRecordSnapshotStart));
    void* extraReceiver3 = (void*)LayoutManager_GetInstance();
    int value = (int)((CPadMemoryMenuSave*)(InputManager_GetSelectedPlayer(extraReceiver3)))->Create(data);
    saveMenu = (void*)value; // CPangya2Shop::HandleProfileSaveInput: self + 0x8C8
    saveMenuActive = 1;
    return 1;
}

struct BitReference {
    unsigned int* words;
    unsigned int index;
};
}

// Test a type ID against the shop snapshot: unlock flags, ownership bitsets or positive item counts.
// Kind -> snapshot storage / ownership test:
// 1 character -> characters[index], bit 6 of first byte; 2 part -> ownedParts, bit at inventory index;
// 4 club set -> clubSets[index].flags bit 6; 5 ball -> ballFlags[index] bit 6;
// 6 consumable -> consumableCounts[index] > 0; 7 caddie -> caddies[index].flags bit 6;
// 8 equipment -> zero low-13-bit serial uses caddie unlock (index from bits 21..25),
//                otherwise ownedCaddieItems bit at inventory index;
// 15 hairstyle -> ownedHairStyles bit at inventory index; 28 aux part -> indexed byte at auxPartCountPrefix != 0.
// All other kinds and typeId 0 return false. This is a snapshot test, with no inventory mutation.
// Requires IDs resolvable to valid snapshot indices; the existing bounds-error construction is
// preserved verbatim. Do not treat the bitsets, byte flags and signed counts as interchangeable.
int CPangya2Shop::IsItemOwned(unsigned int typeId) {
    if (!typeId)
        return 0;
    // Character/caddie/club/ball ownership uses bit 6 of the first record byte.
    // Keep the original shift-and-sign-extension form of that flag test.
    ShopOwnershipView* ownership = (ShopOwnershipView*)this;
    // IFF type IDs store the item kind in the upper six bits.
    switch (typeId >> 26) {
    case kItemCharacter: {
        int index = ItemInventory_FindCharacterIndex(AsCSharedDoc(g_sharedDoc)->itemInventory, typeId);
        if (((int)((*(unsigned char*)&ownership->characters[index].ownershipFlags << 25) & 0xC0000000) >> 31) != 0)
            return 1;
        break;
    }
    case kItemPart: {
        unsigned int index = ItemInventory_FindPartIndex(AsCSharedDoc(g_sharedDoc)->itemInventory, typeId);
        BitReference ref = {ownership->ownedParts, index};
        BitReference probe = ref;
        if (index >= kPartOwnershipBits) {
            std::out_of_range error(bitsetTestOutOfRangeMessage, shopStrings + kShopRangeErrorType);
        }
        if ((1U << (index & 31)) & probe.words[index >> 5])
            return 1;
        break;
    }
    case kItemHairStyle: {
        unsigned int index = ItemInventory_FindHairStyleIndex(AsCSharedDoc(g_sharedDoc)->itemInventory, typeId);
        BitReference ref = {ownership->ownedHairStyles, index};
        BitReference probe = ref;
        if (index >= kHairStyleOwnershipBits) {
            std::out_of_range error(shopHairOwnershipBoundsError, shopStrings + kShopRangeErrorType);
        }
        if ((1U << (index & 31)) & probe.words[index >> 5])
            return 1;
        break;
    }
    case kItemAuxPart: {
        int index = ItemInventory_FindAuxPartIndex(AsCSharedDoc(g_sharedDoc)->itemInventory, typeId);
        // Read the whole snapshot byte representation, beyond the one-byte layout marker.
        if (((signed char*)((char*)ownership + (int)&((ShopOwnershipView*)0)->auxPartCountPrefix))[index] != 0)
            return 1;
        break;
    }
    case kItemClubSet: {
        int index = ItemInventory_FindClubSetIndex(AsCSharedDoc(g_sharedDoc)->itemInventory, typeId);
        if (((int)((ownership->clubSets[index].flags << 25) & 0xC0000000) >> 31) != 0)
            return 1;
        break;
    }
    case kItemBall: {
        int index = ItemInventory_FindBallIndex(AsCSharedDoc(g_sharedDoc)->itemInventory, typeId);
        if (((int)((ownership->ballFlags[index] << 25) & 0xC0000000) >> 31) != 0)
            return 1;
        break;
    }
    case kItemConsumable: {
        int index = ItemInventory_FindItemIndex(AsCSharedDoc(g_sharedDoc)->itemInventory, typeId);
        if (ownership->consumableCounts[index] > 0)
            return 1;
        break;
    }
    case kItemCaddie: {
        int index = ItemInventory_FindCaddieIndex(AsCSharedDoc(g_sharedDoc)->itemInventory, typeId);
        if (((int)((ownership->caddies[index].flags << 25) & 0xC0000000) >> 31) != 0)
            return 1;
        break;
    }
    case kItemCaddieEquipment:
        // A zero serial denotes a caddie unlock rather than a separate accessory.
        if ((typeId & 0x1FFF) == 0) {
            int index = (int)(typeId & 0x03E00000) >> 21;
            if (((int)((ownership->caddies[index].flags << 25) & 0xC0000000) >> 31) != 0)
                return 1;
        } else {
            unsigned int index = ItemInventory_FindCaddieItemIndex(AsCSharedDoc(g_sharedDoc)->itemInventory, typeId);
            BitReference ref = {&ownership->ownedCaddieItems, index};
            BitReference probe = ref;
            if (index >= kCaddieItemOwnershipBits) {
                std::out_of_range error(bitsetTestRangeError, shopStrings + kShopRangeErrorType);
            }
            if (probe.words[0] & (1U << index))
                return 1;
        }
        break;
    }
    return 0;
}

#pragma push
#pragma use_lmw_stmw on
// Sum Pang prices only for item IDs absent from the ownership snapshot.
int CPangya2Shop::SumUnownedItemValues(EntryList* list) {
    unsigned int offset;
    int totalPang;
    unsigned int count;
    totalPang = 0;
    count = 0;
    offset = 0;
    while (count < list->count) {
        void* itemId = *(void**)((char*)list->entries + offset);
        if (!IsItemOwned((unsigned int)itemId)) {
            void* pricedItemId = *(void**)((char*)list->entries + offset);
            totalPang += ItemInventory_GetPricePang(AsCSharedDoc(g_sharedDoc)->itemInventory, pricedItemId);
        }
        ++count;
        offset += 4;
    }
    return totalPang;
}

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU804624F0", _ctors_u802CBF2C_0, Shop_InitTauConstant);
#pragma pop

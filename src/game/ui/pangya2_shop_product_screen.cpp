#include "classes/CPangya2Shop.h"
#include "classes/CSharedDoc.h"
#include "classes/ShopProductRecord.h"

// Borrow typed views without changing ownership or layout.
static inline CPangya2Shop* AsCPangya2Shop(void* p) { return (CPangya2Shop*)p; }
// Borrow the shared document view.
static inline CSharedDoc* AsCSharedDoc(void* p) { return (CSharedDoc*)p; }
// Borrow one product-table record.
static inline ShopProductRecord* AsShopProductRecord(void* p) { return (ShopProductRecord*)p; }
// Decode a legacy 32-bit product address without treating it as a type id or count.
static inline ShopProductRecord* AsShopProductRecord(int address) { return (ShopProductRecord*)address; }


enum ShopCategory { kCharacterCategory, kItemCategory, kCaddieCategory };
enum ShopItemKind {
    kCharacterItem = 1, kPartItem = 2, kClubSetItem = 4, kBallItem = 5,
    kConsumableItem = 6, kCaddieItem = 7, kCaddieEquipmentItem = 8,
    kHairStyleItem = 15, kAuxPartItem = 28, kItemKindShift = 26
};
// Logical input action ids; negative player selects GameInputManager::selectedPlayer.
enum ShopInputAction { kActionPreviousList = 0, kActionNextList = 1, kActionActivate = 4, kActionRemoveCartItem = 0x17, kSelectedInputChannel = -1 };
// List indices are category-relative, not one global equipment enum:
// character 0 = character, 1..4 = part lists (exact artwork/group split unresolved);
// item 0 = club set, 1 = ball, 2 = consumable/cart; caddie 0 = caddie, 1 = equipment.
// Character restore scans all 24 saved outfit-part slots until each list accepts a type id.
// Save view at self+0x2e4+characterIndex*0x34: leading short precedes the 24 part shorts;
// hence partCursor[1] is the first equipped-part inventory index, not an IFF type id.
enum ShopList { kFirstList = 0, kBallList = 1, kConsumableList = 2, kNoList = -1 };
enum ShopStringOffset { kUiCursorMoveSound = 0x96, kUiErrorSound = 0xA5 };
enum ShopDialogMode { kSinglePurchaseDialog = 1, kOutfitPurchaseDialog = 2, kCaddiePurchaseDialog = 6 };
enum ShopSelectionStorage { kSavedOutfitStride = 0x34, kSavedOutfitsOffset = 0x2e4, kProductNameMessageGroup = 0x40e };
enum ShopLimits { kCharacterListCount = 5, kItemListCount = 3, kCaddieListCount = 2, kOutfitPartCount = 24, kClubUpgradeLevelCount = 4 };
struct SavedShopClubSet { unsigned char flags; signed char upgradeLevel; };
struct ShopClubUpgradeView { char prefix[0x4C4]; SavedShopClubSet clubSets[15]; };

// Translation unit 0x802C5C60-0x802C7308 (9 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

#include "game/singletons.h"
extern "C" {
void appFreeUni__FPv(void*);

// The raw vector's explicit normal cleanup releases this unwind guard first.
struct ActivatedUnwindCleanup {
    volatile unsigned int* words;
    bool active;
    // Arm ownership of the collected temporary index-vector allocation.
    explicit ActivatedUnwindCleanup(volatile void* vector)
        : words((volatile unsigned int*)vector), active(true) {}
    // Disarm before the existing explicit normal-exit release.
    void release() { active = false; }
    // Free live temporary storage on exceptional exit.
    ~ActivatedUnwindCleanup() {
        if (active && words[0]) {
            words[1] = 0;
            appFreeUni__FPv((void*)words[0]);
        }
    }
};


int CPangya2Shop_HandleProductActivation(void*);

extern signed char s_fadeOverlayInitialized;
extern char s_fadeOverlay[256];
extern char s_fadeOverlayDtorRegistration[256];
extern char shopCategoryDescriptors[];
void FadeOverlay_Init(void*); void ResourceState_Dtor(void*); int FadeState_GetState(void*);
int CPangya2Shop_TryOpenClubDialog(void*); int CPangya2Shop_ActivateModelCameraPreset(void*); int CPangya2Shop_HandleProfileSaveInput(void*); int CPangya2Shop_OpenSelectedItemInfo(void*);
int CPangya2Shop_HandleCategoryNavigation(void*); void CProductsCategory_SetCategoryCount(void*,void*); void CProductsCategory_SetLayoutMode(void*,int); void CProductsCategory_SetSelectedIndex(void*,int);
void CClubHouseMsg_ResetTextState(void*); void Memory_Clear(void*,int); int CPangya2Shop_GetSelectedProductInfo(void*,int*); void CClubHouseMsg_SetTextFromGroup(void*,void*,int,int);
int UIGroup_GetOrCreateById(void*,int); void CPangya2Shop_RefreshSelectedProductValues(void*); void CPangya2Shop_UpdateSelectedPurchaseAmount(void*); void CPlayerStatus_SetEnabled(void*,int); void CShopSubMenuB_SetEnabled(void*,int);
void CShopProductsTab_SetEnabled(void*,int); void CPangya2Shop_UpdatePurchaseButton(void*); void CPangya2Shop_UpdateUnownedSelectedOutfitCategoryMarkers(void*); void CPlayerStatus_RefreshEquipmentDisplay(void*,int);
int CShopProductsTab_RefreshVisibleSlotIndicators(void*); void CPangya2Shop_UpdateControlScriptSelection(void*);

void Layout_SetElementPlayerFlag(void*, int, int, int);
void Layout_SetElementValueState(void*, int, char);
int ShopPreview_PollReady(void*);
void InputManager_Init(void*);
void InputManager_Dtor(void*);
void __register_global_object(void*, void*, void*);
int Input_IsActionButtonDown(void*, int, int);
int Input_IsActionPressed(void*, int, int);
int CPangya2Shop_HandleCategorySelection(void*);
void Sound_PlayByName(void*, const char*, int, int, int, int, float, float, float);
void CPangya2Shop_SelectCharaProduct(void*);
void CPangya2Shop_SelectItemProduct(void*);
void CPangya2Shop_SelectCaddieProduct(void*);
void CShopProductsTab_RefreshVisibleProducts(void*);
void* CProductsCategory_CollectActivatedIndices(void*, void*, int);
void appFreeUni__FPv(void*);
int CPangya2Shop_IsItemOwned(void*, int);
void CClubDialog_SetMode(void*, int);
void* CShopProductsTab_GetSelectedProductIndex(void*);
int CShopSubMenuB_RemoveActivatedItemSlot(void*);
int CShopSubMenuB_RemoveItemSlotByKey(void*, int);
void CShopSubMenuB_SetItemMessage(void*, int);
void* ShopProductTable_GetRecordAt(void*, int, void*);
void* CPangya2Shop_GetSelectedProductRecord(void*);
int CPangya2Shop_HasUnownedSelectedParts(void*);
int CBuildProductChara_FindSetContainingItem(void*, unsigned int);
int CItemCart_IsFull(void*);
int ItemInventory_FindClubSetIndex(void*, unsigned int);
int CPangya2Shop_IsItemSelected(void*, int);
void CClubSubMenu_OpenVariant(void*, int, int);
int CBuildProduct_GetProductCount(void*, int);
int CBuildProduct_GetProductRecords(void*, int);
int CBuildProduct_GetProductDrawEntries(void*, int);
int ItemInventory_GetCharacterIdAt(void*, int);
int ItemInventory_GetPartIdAt(void*, int);
int ItemInventory_GetCaddieIdAt(void*, int);
int ItemInventory_GetCaddieItemIdAt(void*, int);
int ItemInventory_GetClubSetIdAt(void*, int);
int ItemInventory_GetBallIdAt(void*, int);
void CShopProductsTab_SetProductCount(void*, int);
void CShopProductsTab_SetProductArrays(void*, int, int);
int CShopProductsTab_SelectProductByTypeId(void*, int);
extern CSharedDoc* g_sharedDoc;
extern signed char s_inputManagerInitialized;
extern char s_gameInputManager[256];
extern char s_inputManagerDtorRegistration_80516C10[256];
extern char shopStrings[];
extern float kFloat_0_5_shopSound, kFloat_200_Shop, kFloat_1_805E0918;
extern void* g_soundManager;
// Owns a temporary index allocation collected from a category; entries are indices, not objects.
// Normal cleanup releases the raw-vector guard before freeing the allocation explicitly.
struct ActivatedCategoryItems {
    int* indices;
    int size;
    int capacity;
};

// Poll the category's current preview; callbacks may change category/list, so re-read them.
// Character -> caddie -> primary item -> secondary item; stop at the first unfinished poll.
#pragma push
#pragma always_inline on
static inline int PollNavigationPreviewReady(void* self) {
    int category;
    int previewReady;
    category = AsCPangya2Shop(self)->categoryIndex;
    // 0x802C6070: Stop at the first model operation that is still loading.
    do {
        if (category == kCharacterCategory) {
            if (ShopPreview_PollReady(AsCPangya2Shop(self)->charaModelOperation) == 0) {
                previewReady = 0;
                break;
            }
        }
        category = AsCPangya2Shop(self)->categoryIndex;
        if (category == kCaddieCategory) {
            if (ShopPreview_PollReady(AsCPangya2Shop(self)->caddieModelOperation) == 0) {
                previewReady = 0;
                break;
            }
        }
        category = AsCPangya2Shop(self)->categoryIndex;
        if (category == kItemCategory) {
            if (AsCPangya2Shop(self)->selectedListIndex == 0) {
                if (ShopPreview_PollReady(AsCPangya2Shop(self)->primaryItemModelOperation) == 0) {
                    previewReady = 0;
                    break;
                }
            }
        }
        category = AsCPangya2Shop(self)->categoryIndex;
        if (category == kItemCategory) {
            if (AsCPangya2Shop(self)->selectedListIndex == 1) {
                if (ShopPreview_PollReady(AsCPangya2Shop(self)->secondaryItemModelOperation) == 0) {
                    previewReady = 0;
                    break;
                }
            }
        }
        previewReady = 1;
    } while (0);

    return previewReady;
}
#pragma pop

// Move between product lists and the category strip after the required preview has loaded.
int CPangya2Shop_HandleProductListNavigation(void* self) {
    // Navigation polls previews only when entered from the item category.
    // Activation below has a separate readiness gate for every category.
    int entryCategory = AsCPangya2Shop(self)->categoryIndex;
    if (entryCategory == kItemCategory) {
        if (PollNavigationPreviewReady(self) == 0) return 0;
    }
    // Convert an activated category element id into its product-list index.
    if (CPangya2Shop_HandleCategorySelection(self) == 0) {
        if (Sing_805DC5C8() == 0) {
            if (s_inputManagerInitialized == 0) {
                InputManager_Init(s_gameInputManager);
                __register_global_object(s_gameInputManager, (void*)InputManager_Dtor, s_inputManagerDtorRegistration_80516C10);
                s_inputManagerInitialized = 1;
            }
            Sing_805DC5C8() = s_gameInputManager;
        }
        if (Input_IsActionPressed(Sing_805DC5C8(), kActionPreviousList, kSelectedInputChannel)) {
            int listIndex = AsCPangya2Shop(self)->selectedListIndex;
            if (listIndex >= 0) {
                AsCPangya2Shop(self)->selectedListIndex = listIndex - 1;
            } else {
                int category = AsCPangya2Shop(self)->categoryIndex;
                int listIndex;
                switch (category) {
                case kCharacterCategory: listIndex = kCharacterListCount; break;
                case kItemCategory: listIndex = kItemListCount; break;
                case kCaddieCategory: listIndex = kCaddieListCount; break;
                default: listIndex = 0; break;
                }
                AsCPangya2Shop(self)->selectedListIndex = listIndex - 1;
            }
        } else {
            if (Sing_805DC5C8() == 0) {
                if (s_inputManagerInitialized == 0) {
                    InputManager_Init(s_gameInputManager);
                    __register_global_object(s_gameInputManager, (void*)InputManager_Dtor, s_inputManagerDtorRegistration_80516C10);
                    s_inputManagerInitialized = 1;
                }
                Sing_805DC5C8() = s_gameInputManager;
            }
            if (!Input_IsActionPressed(Sing_805DC5C8(), kActionNextList, kSelectedInputChannel)) return 0;
            int category = AsCPangya2Shop(self)->categoryIndex;
            int listCount;
            switch (category) {
            case kCharacterCategory: listCount = kCharacterListCount; break;
            case kItemCategory: listCount = kItemListCount; break;
            case kCaddieCategory: listCount = kCaddieListCount; break;
            default: listCount = 0; break;
            }
            int listIndex = AsCPangya2Shop(self)->selectedListIndex;
            if (listIndex < listCount - 1) AsCPangya2Shop(self)->selectedListIndex = listIndex + 1;
            else AsCPangya2Shop(self)->selectedListIndex = -1;
        }
    }
    Sound_PlayByName(g_soundManager, shopStrings + kUiCursorMoveSound, 0, 0,
        0, 0, kFloat_0_5_shopSound, kFloat_200_Shop, kFloat_1_805E0918);
    return 1;
}

// Apply selected, focused and dimmed states to both sets of category layout elements.
int CPangya2Shop_UpdateCategoryHighlights(void* self) {
    for (int category = 0; category < 3; category++) {
        int dimmed;
        signed char highlightState;
        int selectedCategory = AsCPangya2Shop(self)->categoryIndex;
        if (selectedCategory == category) {
            highlightState = 2;
            if (AsCPangya2Shop(self)->selectedListIndex == kNoList) highlightState = 3;
            dimmed = 0;
        } else {
            highlightState = 0;
            dimmed = 1;
        }
        Layout_SetElementPlayerFlag(AsCPangya2Shop(self)->shopLayout, category, dimmed, 5);
        Layout_SetElementPlayerFlag(AsCPangya2Shop(self)->shopLayout, category + 3, dimmed, 5);
        Layout_SetElementValueState(AsCPangya2Shop(self)->shopLayout, category, highlightState);
        Layout_SetElementValueState(AsCPangya2Shop(self)->shopLayout, category + 3, highlightState);
    }
    return 1;
}

// Map activated strip elements to list indices; return 1 for a collected activation, 0 otherwise.
// Element ids 1/4/7/10/13 select lists 0/1/2/3/4; other ids leave the list unchanged.
int CPangya2Shop_HandleCategorySelection(void* self) {
    ActivatedCategoryItems local;
    local.indices = 0;
    local.size = 0;
    local.capacity = 0;
    ActivatedUnwindCleanup activatedCleanup(&local);
    void* r = CProductsCategory_CollectActivatedIndices(AsCPangya2Shop(self)->productsCategory, &local, 0);
    if (r == 0) {
        if (local.indices != 0) {
            local.size = local.size - local.size;
            activatedCleanup.release(); appFreeUni__FPv(local.indices);
        }
        return 0;
    }
    int v = *(int*)local.indices;
    if (v == 1) {
        AsCPangya2Shop(self)->selectedListIndex = 0;
    } else if (v - 3 == 1) {
        AsCPangya2Shop(self)->selectedListIndex = 1;
    } else if (v - 6 == 1) {
        AsCPangya2Shop(self)->selectedListIndex = 2;
    } else if (v - 9 == 1) {
        AsCPangya2Shop(self)->selectedListIndex = 3;
    } else AsCPangya2Shop(self)->selectedListIndex = (v - 12 == 1) ? 4 : AsCPangya2Shop(self)->selectedListIndex;
    if (local.indices != 0) {
        local.size = local.size - local.size;
        activatedCleanup.release(); appFreeUni__FPv(local.indices);
    }
    return 1;
}

#pragma use_lmw_stmw on
// Bind category product arrays and restore the equipped character, parts, caddie, club set or ball.
int CPangya2Shop_RefreshProductLists(void* self) {
    int productCount;
    // Legacy APIs carry table addresses in 32-bit integers, separately from productCount.
    int productRecordsAddress;
    int drawEntriesAddress;
    void** productLists;
    int equippedTypeId;
    int savedPartSlot;
    short* savedPartIndices;
    int listIndex;
    int category = AsCPangya2Shop(self)->categoryIndex;
    // Reset the five category scratch words before rebuilding the product arrays.
    // These words are also cleared by the shop constructor; no other use is established.
    AsCPangya2Shop(self)->unk_848 = 0;
    AsCPangya2Shop(self)->unk_84C = 0;
    AsCPangya2Shop(self)->unk_850 = 0;
    AsCPangya2Shop(self)->unk_854 = 0;
    AsCPangya2Shop(self)->unk_858 = 0;
    if (category == kCharacterCategory) {
        for (listIndex = 0; listIndex < kCharacterListCount; listIndex++) {
            productCount = CBuildProduct_GetProductCount(AsCPangya2Shop(self)->charaProductBuilder, listIndex);
            productLists = (void**)AsCPangya2Shop(self)->productLists;
            CShopProductsTab_SetProductCount(productLists[listIndex], productCount);
            productRecordsAddress = CBuildProduct_GetProductRecords(AsCPangya2Shop(self)->charaProductBuilder, listIndex);
            drawEntriesAddress = CBuildProduct_GetProductDrawEntries(AsCPangya2Shop(self)->charaProductBuilder, listIndex);
            productLists = (void**)AsCPangya2Shop(self)->productLists;
            CShopProductsTab_SetProductArrays(productLists[listIndex], drawEntriesAddress, productRecordsAddress);
            if (listIndex >= 0 && listIndex < kCharacterListCount) {
            switch (listIndex) {
            case 0: {
                equippedTypeId = ItemInventory_GetCharacterIdAt(AsCSharedDoc(g_sharedDoc)->itemInventory, AsCPangya2Shop(self)->currentCharaIndex);
                productLists = (void**)AsCPangya2Shop(self)->productLists;
                CShopProductsTab_SelectProductByTypeId(productLists[0], equippedTypeId);
                break;
            }
            case 1:
            case 2:
            case 3:
            case 4: {
                if (listIndex >= 1 && listIndex < kCharacterListCount) {
                // Each 0x34-byte outfit record starts with a short before its 24 part indices.
                // Inventory indices select saved equipment; inventory lookup then yields an IFF type id.
                savedPartIndices = (short*)((char*)self + AsCPangya2Shop(self)->currentCharaIndex * kSavedOutfitStride + kSavedOutfitsOffset);
                savedPartSlot = 0;
                do {
                    equippedTypeId = ItemInventory_GetPartIdAt(AsCSharedDoc(g_sharedDoc)->itemInventory, savedPartIndices[1]);
                    productLists = (void**)AsCPangya2Shop(self)->productLists;
                    if (CShopProductsTab_SelectProductByTypeId(productLists[listIndex], equippedTypeId)) break;
                    savedPartSlot++;
                    savedPartIndices++;
                } while (savedPartSlot < kOutfitPartCount);
                }
                break;
            }
            }
            }
        }
    } else if (category == kCaddieCategory) {
        for (listIndex = 0; listIndex < kCaddieListCount; listIndex++) {
            if (AsCPangya2Shop(self)->noCaddieSelected && listIndex == 1) {
                productLists = (void**)AsCPangya2Shop(self)->productLists;
                CShopProductsTab_SetProductCount(productLists[listIndex], 0);
            } else {
                productCount = CBuildProduct_GetProductCount(AsCPangya2Shop(self)->caddieProductBuilder, listIndex);
                productLists = (void**)AsCPangya2Shop(self)->productLists;
                CShopProductsTab_SetProductCount(productLists[listIndex], productCount);
            }
            productRecordsAddress = CBuildProduct_GetProductRecords(AsCPangya2Shop(self)->caddieProductBuilder, listIndex);
            drawEntriesAddress = CBuildProduct_GetProductDrawEntries(AsCPangya2Shop(self)->caddieProductBuilder, listIndex);
            productLists = (void**)AsCPangya2Shop(self)->productLists;
            CShopProductsTab_SetProductArrays(productLists[listIndex], drawEntriesAddress, productRecordsAddress);
            if (listIndex >= 0 && listIndex < kCaddieListCount) {
            switch (listIndex) {
            case 0: {
                equippedTypeId = ItemInventory_GetCaddieIdAt(AsCSharedDoc(g_sharedDoc)->itemInventory, AsCPangya2Shop(self)->currentCaddieIndex);
                productLists = (void**)AsCPangya2Shop(self)->productLists;
                CShopProductsTab_SelectProductByTypeId(productLists[0], equippedTypeId);
                break;
            }
            case 1: {
                equippedTypeId = ItemInventory_GetCaddieItemIdAt(AsCSharedDoc(g_sharedDoc)->itemInventory, AsCPangya2Shop(self)->caddieItemIndex);
                productLists = (void**)AsCPangya2Shop(self)->productLists;
                CShopProductsTab_SelectProductByTypeId(productLists[1], equippedTypeId);
                break;
            }
            }
            }
        }
    } else if (category == kItemCategory) {
        for (listIndex = 0; listIndex < kItemListCount; listIndex++) {
            productCount = CBuildProduct_GetProductCount(AsCPangya2Shop(self)->itemProductBuilder, listIndex);
            productLists = (void**)AsCPangya2Shop(self)->productLists;
            CShopProductsTab_SetProductCount(productLists[listIndex], productCount);
            productRecordsAddress = CBuildProduct_GetProductRecords(AsCPangya2Shop(self)->itemProductBuilder, listIndex);
            drawEntriesAddress = CBuildProduct_GetProductDrawEntries(AsCPangya2Shop(self)->itemProductBuilder, listIndex);
            productLists = (void**)AsCPangya2Shop(self)->productLists;
            CShopProductsTab_SetProductArrays(productLists[listIndex], drawEntriesAddress, productRecordsAddress);
            if (listIndex >= 0 && listIndex < kItemListCount) {
                switch (listIndex) {
                case 2: break;
                case 0: {
                    equippedTypeId = ItemInventory_GetClubSetIdAt(AsCSharedDoc(g_sharedDoc)->itemInventory, AsCPangya2Shop(self)->clubSetIndex);
                    productLists = (void**)AsCPangya2Shop(self)->productLists;
                    CShopProductsTab_SelectProductByTypeId(productLists[0], equippedTypeId);
                    break;
                }
                case 1: {
                    equippedTypeId = ItemInventory_GetBallIdAt(AsCSharedDoc(g_sharedDoc)->itemInventory, AsCPangya2Shop(self)->ballIndex);
                    productLists = (void**)AsCPangya2Shop(self)->productLists;
                    CShopProductsTab_SelectProductByTypeId(productLists[1], equippedTypeId);
                    break;
                }
                }
            }
        }
    }
    return 1;
}
#pragma use_lmw_stmw reset

// Offer a purchase dialog for unowned caddies or caddie equipment.
int CPangya2Shop_TryOpenCaddiePurchaseDialog(void* self) {
    void* productRecord;
    if (AsCPangya2Shop(self)->selectedListIndex == kNoList) {
        productRecord = 0;
    } else {
        void* productList = ((void**)AsCPangya2Shop(self)->productLists)[AsCPangya2Shop(self)->selectedListIndex];
        void* productIndex = CShopProductsTab_GetSelectedProductIndex(productList);
        int category = AsCPangya2Shop(self)->categoryIndex;
        productRecord = 0;
        switch (category) {
        case kCharacterCategory: productRecord = ShopProductTable_GetRecordAt(AsCPangya2Shop(self)->charaProductBuilder, AsCPangya2Shop(self)->selectedListIndex, productIndex); break;
        case kItemCategory: productRecord = ShopProductTable_GetRecordAt(AsCPangya2Shop(self)->itemProductBuilder, AsCPangya2Shop(self)->selectedListIndex, productIndex); break;
        case kCaddieCategory: productRecord = ShopProductTable_GetRecordAt(AsCPangya2Shop(self)->caddieProductBuilder, AsCPangya2Shop(self)->selectedListIndex, productIndex); break;
        default: break;
        }
    }
    if (productRecord == 0) {
        return 0;
    }
    int itemKind = (int)(((unsigned int)(int)AsShopProductRecord(productRecord)->typeId) >> kItemKindShift);
    // 0x802C719C: successful product checks join after the type switch.
    switch (itemKind) {
    case kCaddieItem:
        if (CPangya2Shop_IsItemOwned(self, (int)AsShopProductRecord(productRecord)->typeId) != 0) {
            return 0;
        }
        CClubDialog_SetMode(AsCPangya2Shop(self)->clubDialog, kCaddiePurchaseDialog);
        break;
    case kCaddieEquipmentItem:
        if (CPangya2Shop_IsItemOwned(self, (int)AsShopProductRecord(productRecord)->typeId) != 0) {
            return 0;
        }
        CClubDialog_SetMode(AsCPangya2Shop(self)->clubDialog, kSinglePurchaseDialog);
        break;
    default:
        return 0;
    }
    AsCPangya2Shop(self)->clubDialogOpen = 1;
    return 1;
}

// Choose the character/part/hair/ring purchase or outfit submenu from ownership and set membership.
int CPangya2Shop_OpenSelectedProductActions(void* self) {
    void* productRecord = CPangya2Shop_GetSelectedProductRecord(self);
    if (productRecord == 0) {
        return 0;
    }
    int submenuMode = 0;
    unsigned int itemKind = ((unsigned int)(int)AsShopProductRecord(productRecord)->typeId) >> kItemKindShift;
    switch (itemKind) {
        case kCharacterItem: {
            if (CPangya2Shop_HasUnownedSelectedParts(self)) {
                CClubDialog_SetMode(AsCPangya2Shop(self)->clubDialog, kOutfitPurchaseDialog);
                AsCPangya2Shop(self)->clubDialogOpen = 1;
                return 1;
            }
            if (CPangya2Shop_IsItemOwned(self, (int)AsShopProductRecord(productRecord)->typeId) != 0) {
                return 0;
            }
            CClubDialog_SetMode(AsCPangya2Shop(self)->clubDialog, kSinglePurchaseDialog);
            AsCPangya2Shop(self)->clubDialogOpen = 1;
            return 1;
        }
        case kPartItem: {
            if (CBuildProductChara_FindSetContainingItem(AsCPangya2Shop(self)->charaProductBuilder, AsShopProductRecord(productRecord)->typeId)) {
                if (CPangya2Shop_IsItemOwned(self, (int)AsShopProductRecord(productRecord)->typeId) != 0) {
                    submenuMode = 2;
                } else {
                    submenuMode = 0;
                }
            } else {
                if (CPangya2Shop_IsItemOwned(self, (int)AsShopProductRecord(productRecord)->typeId) != 0) {
                    return 0;
                }
                submenuMode = 1;
            }
            break;
        }
        case kHairStyleItem: {
            if (CPangya2Shop_IsItemOwned(self, (int)AsShopProductRecord(productRecord)->typeId) != 0) {
                return 0;
            }
            CClubDialog_SetMode(AsCPangya2Shop(self)->clubDialog, kSinglePurchaseDialog);
            AsCPangya2Shop(self)->clubDialogOpen = 1;
            return 1;
        }
        case kAuxPartItem: {
            if (CPangya2Shop_IsItemOwned(self, (int)AsShopProductRecord(productRecord)->typeId) != 0) {
                return 0;
            }
            submenuMode = 1;
            break;
        }
        default: {
            return 0;
        }
    }
    AsCPangya2Shop(self)->clubSubMenuOpen = 1;
    int selected = CPangya2Shop_IsItemSelected(self, (int)AsShopProductRecord(productRecord)->typeId);
    CClubSubMenu_OpenVariant(AsCPangya2Shop(self)->clubSubMenu, submenuMode, selected);
    return 1;
}

// Offer club upgrades, ball purchases or consumable purchases, respecting entry mode and cart capacity.
int CPangya2Shop_OpenItemPurchaseOrUpgrade(void* self) {
    int productRecordAddress = 0;
    int listIndex = AsCPangya2Shop(self)->selectedListIndex;
    if (listIndex == -1) {
        productRecordAddress = 0;
    } else {
        void* ptr = ((void**)AsCPangya2Shop(self)->productLists)[listIndex];
        void* productIndex = CShopProductsTab_GetSelectedProductIndex(ptr);
        int category = AsCPangya2Shop(self)->categoryIndex;
        void* r = 0;
        switch (category) {
        case kCharacterCategory: r = ShopProductTable_GetRecordAt(AsCPangya2Shop(self)->charaProductBuilder, AsCPangya2Shop(self)->selectedListIndex, productIndex); break;
        case kItemCategory: r = ShopProductTable_GetRecordAt(AsCPangya2Shop(self)->itemProductBuilder, AsCPangya2Shop(self)->selectedListIndex, productIndex); break;
        case kCaddieCategory: r = ShopProductTable_GetRecordAt(AsCPangya2Shop(self)->caddieProductBuilder, AsCPangya2Shop(self)->selectedListIndex, productIndex); break;
        default: break;
        }
        productRecordAddress = (int)r;
    }
    if (productRecordAddress == 0) {
        return 0;
    }
    int itemKind = (int)(((unsigned int)(int)AsShopProductRecord(productRecordAddress)->typeId) >> kItemKindShift);
    switch (itemKind) {
    case kClubSetItem: {
            if (AsCPangya2Shop(self)->characterEntryMode != 0) {
                return 0;
            }
            if (CPangya2Shop_IsItemOwned(self, (int)AsShopProductRecord(productRecordAddress)->typeId) == 0) {
                break;
            }
            int canUpgrade = 0;
            unsigned int typeId = AsShopProductRecord(productRecordAddress)->typeId;
            unsigned int kind = typeId >> kItemKindShift;
            if (kind != kClubSetItem) {
                canUpgrade = 0;
            } else {
                int clubSetIndex = ItemInventory_FindClubSetIndex(AsCSharedDoc(g_sharedDoc)->itemInventory, typeId);
                if (clubSetIndex == -1) {
                    canUpgrade = 0;
                } else {
                    signed char upgradeLevel = ((ShopClubUpgradeView*)self)->clubSets[clubSetIndex].upgradeLevel;
                    // The sign-bit test implements the existing upgrade-level limit.
                    int x = upgradeLevel + 1;
                    x = x ^ kClubUpgradeLevelCount;
                    int hi = x >> 1;
                    canUpgrade = (int)((unsigned int)(hi - (x & kClubUpgradeLevelCount)) >> 31);
                }
            }
            if (canUpgrade != 0) {
                CClubSubMenu_OpenVariant(AsCPangya2Shop(self)->clubSubMenu, 3, 0);
                AsCPangya2Shop(self)->clubSubMenuOpen = 1;
                return 1;
            }
            return 0;
        }
    case kBallItem:
        if (CPangya2Shop_IsItemOwned(self, (int)AsShopProductRecord(productRecordAddress)->typeId) != 0) {
            return 0;
        }
        break;
    case kConsumableItem:
        if (AsCPangya2Shop(self)->characterEntryMode != 0) {
            return 0;
        }
        if (CPangya2Shop_IsItemOwned(self, (int)AsShopProductRecord(productRecordAddress)->typeId) == 0) {
            if (CItemCart_IsFull(AsCPangya2Shop(self)->itemCart) == 0) {
                break;
            }
        }
        return 0;
    default:
        return 0;
    }
    CClubDialog_SetMode(AsCPangya2Shop(self)->clubDialog, kSinglePurchaseDialog);
    AsCPangya2Shop(self)->clubDialogOpen = 1;
    return 1;
}

// ABI shared with CPangya2Shop_GetSelectedProductInfo in pangya2_shop_screen.cpp.
struct SelectedProductInfo {
    int pangPrice;
    char statBonuses[10];
    char nameMessageId[40];
    char descriptionMessageId[40];
};

// Rebuild the selected product's name, derived values and purchase amount in that order.
// info is caller-owned scratch; failed lookup leaves the message reset and the zeroed record.
#pragma push
#pragma always_inline on
static inline void RefreshSelectedProductMessageAndValues(void* self, SelectedProductInfo& info) {
    CClubHouseMsg_ResetTextState(AsCPangya2Shop(self)->productMessage);
    info.pangPrice = 0;
    Memory_Clear(info.statBonuses, sizeof(info.statBonuses));
    Memory_Clear(info.nameMessageId, sizeof(info.nameMessageId));
    Memory_Clear(info.descriptionMessageId, sizeof(info.descriptionMessageId));
    if (CPangya2Shop_GetSelectedProductInfo(self, &info.pangPrice) != 0) {
        CClubHouseMsg_SetTextFromGroup(AsCPangya2Shop(self)->productMessage, info.nameMessageId,
            UIGroup_GetOrCreateById(AsCPangya2Shop(self)->uiGroup, kProductNameMessageGroup), 0);
    }
    CPangya2Shop_RefreshSelectedProductValues(self);
    CPangya2Shop_UpdateSelectedPurchaseAmount(self);
}
#pragma pop

// Refresh dependencies (preserve order in the three branches below):
// event       category/layout  message/values/amount  purchase  outfit  equipment/controls
// navigation  rebuild          refresh                after UI  yes     yes
// highlight   unchanged        refresh                after msg yes     unchanged
// removal     products only    after purchase          first     no      unchanged
#pragma opt_common_subs off
// Process shop selection, cart removal and purchase actions, then refresh messages, prices and controls.
int CPangya2Shop_ProcessProductSelection(void* self) {
    SelectedProductInfo newSelectionInfo;
    SelectedProductInfo highlightedProductInfo;
    SelectedProductInfo cartChangeInfo;
    // Phase 1: modal/input gates. Stop for fades, dialogs, camera presets, save or info handling.
    if (Sing_805DC608() == 0) {
        if (s_fadeOverlayInitialized == 0) {
            FadeOverlay_Init(s_fadeOverlay);
            __register_global_object(s_fadeOverlay, (void*)ResourceState_Dtor, s_fadeOverlayDtorRegistration);
            s_fadeOverlayInitialized = 1;
        }
        Sing_805DC608() = s_fadeOverlay;
    }
    if (FadeState_GetState(Sing_805DC608()) != 0) {
        return 0;
    }
    if (CPangya2Shop_TryOpenClubDialog(self) != 0) {
        return 0;
    }
    if (CPangya2Shop_ActivateModelCameraPreset(self) != 0) {
        return 0;
    }
    if (CPangya2Shop_HandleProfileSaveInput(self) != 0) {
        return 0;
    }
    if (CPangya2Shop_OpenSelectedItemInfo(self) != 0) {
        return 0;
    }
    // Phase 2: category/list navigation rebuilds the active controls, message and prices.
    // Each handled branch returns 0: ProcessProductSelection has no success/result flag.
    if (CPangya2Shop_HandleCategoryNavigation(self) != 0 || CPangya2Shop_HandleProductListNavigation(self) != 0) {
        CPangya2Shop_UpdateCategoryHighlights(self);
        CProductsCategory_SetCategoryCount(AsCPangya2Shop(self)->productsCategory,
            ((void**)shopCategoryDescriptors)[AsCPangya2Shop(self)->categoryIndex]);
        CProductsCategory_SetLayoutMode(AsCPangya2Shop(self)->productsCategory, AsCPangya2Shop(self)->categoryIndex);
        CProductsCategory_SetSelectedIndex(AsCPangya2Shop(self)->productsCategory, AsCPangya2Shop(self)->selectedListIndex);
        RefreshSelectedProductMessageAndValues(self, newSelectionInfo);
        if (AsCPangya2Shop(self)->categoryIndex == kItemCategory && AsCPangya2Shop(self)->selectedListIndex == kConsumableList) {
            void* item;
            CPlayerStatus_SetEnabled(AsCPangya2Shop(self)->playerStatus, 0);
            CShopSubMenuB_SetEnabled(AsCPangya2Shop(self)->itemCart, 1);
            if (AsCPangya2Shop(self)->selectedListIndex == kNoList) {
                item = 0;
            } else {
                void* data = CShopProductsTab_GetSelectedProductIndex(
                    ((void**)AsCPangya2Shop(self)->productLists)[AsCPangya2Shop(self)->selectedListIndex]);
                void* res = 0;
                switch (AsCPangya2Shop(self)->categoryIndex) {
                case 0:
                    res = ShopProductTable_GetRecordAt(AsCPangya2Shop(self)->charaProductBuilder,
                        AsCPangya2Shop(self)->selectedListIndex, data);
                    break;
                case 1:
                    res = ShopProductTable_GetRecordAt(AsCPangya2Shop(self)->itemProductBuilder,
                        AsCPangya2Shop(self)->selectedListIndex, data);
                    break;
                case 2:
                    res = ShopProductTable_GetRecordAt(AsCPangya2Shop(self)->caddieProductBuilder,
                        AsCPangya2Shop(self)->selectedListIndex, data);
                    break;
                }
                item = res;
            }
            if (item != 0) {
                CShopSubMenuB_SetItemMessage(AsCPangya2Shop(self)->itemCart, (int)AsShopProductRecord(item)->typeId);
            }
        } else {
            CPlayerStatus_SetEnabled(AsCPangya2Shop(self)->playerStatus, 1);
            CShopSubMenuB_SetEnabled(AsCPangya2Shop(self)->itemCart, 0);
        }
        int listIndex = 0;
        int listByteOffset = 0;
        do {
            if (listIndex == AsCPangya2Shop(self)->selectedListIndex) {
                CShopProductsTab_SetEnabled(*(void**)((char*)AsCPangya2Shop(self)->productLists + listByteOffset), 1);
            } else {
                CShopProductsTab_SetEnabled(*(void**)((char*)AsCPangya2Shop(self)->productLists + listByteOffset), 0);
            }
            listIndex++;
            listByteOffset += 4;
        } while (listIndex < kCharacterListCount);
        CPangya2Shop_UpdatePurchaseButton(self);
        CPangya2Shop_UpdateUnownedSelectedOutfitCategoryMarkers(self);
        CPlayerStatus_RefreshEquipmentDisplay(AsCPangya2Shop(self)->playerStatus, AsCPangya2Shop(self)->categoryIndex);
        CPangya2Shop_UpdateControlScriptSelection(self);
        return 0;
    }
    {
        // Phase 3: changed product highlight refreshes message/prices without navigating.
        int active;
        if (AsCPangya2Shop(self)->selectedListIndex == kNoList) {
            active = 0;
        } else {
            active = CShopProductsTab_RefreshVisibleSlotIndicators(
                ((void**)AsCPangya2Shop(self)->productLists)[AsCPangya2Shop(self)->selectedListIndex]);
        }
        if (active != 0) {
            RefreshSelectedProductMessageAndValues(self, highlightedProductInfo);
            CPangya2Shop_UpdatePurchaseButton(self);
            CPangya2Shop_UpdateUnownedSelectedOutfitCategoryMarkers(self);
            if (AsCPangya2Shop(self)->categoryIndex == kItemCategory && AsCPangya2Shop(self)->selectedListIndex == kConsumableList) {
                void* item;
                if (AsCPangya2Shop(self)->selectedListIndex == kNoList) {
                    item = 0;
                } else {
                    void* data = CShopProductsTab_GetSelectedProductIndex(
                        ((void**)AsCPangya2Shop(self)->productLists)[AsCPangya2Shop(self)->selectedListIndex]);
                    void* res = 0;
                    switch (AsCPangya2Shop(self)->categoryIndex) {
                    case 0:
                        res = ShopProductTable_GetRecordAt(AsCPangya2Shop(self)->charaProductBuilder,
                            AsCPangya2Shop(self)->selectedListIndex, data);
                        break;
                    case 1:
                        res = ShopProductTable_GetRecordAt(AsCPangya2Shop(self)->itemProductBuilder,
                            AsCPangya2Shop(self)->selectedListIndex, data);
                        break;
                    case 2:
                        res = ShopProductTable_GetRecordAt(AsCPangya2Shop(self)->caddieProductBuilder,
                            AsCPangya2Shop(self)->selectedListIndex, data);
                        break;
                    }
                    item = res;
                }
                CShopSubMenuB_SetItemMessage(AsCPangya2Shop(self)->itemCart, (int)AsShopProductRecord(item)->typeId);
            }
            return 0;
        }
    }
    {
        // Phase 4: consumable-cart removal via activated slot or remove-action input.
        // A successful removal refreshes visible products and all price/message dependents.
        int flag;
        if (AsCPangya2Shop(self)->categoryIndex != kItemCategory || AsCPangya2Shop(self)->selectedListIndex != kConsumableList) {
            flag = 0;
        } else {
            if (CShopSubMenuB_RemoveActivatedItemSlot(AsCPangya2Shop(self)->itemCart) != 0) {
                flag = 1;
            } else {
                void* item;
                if (Sing_805DC5C8() == 0) {
                    if (s_inputManagerInitialized == 0) {
                        InputManager_Init(s_gameInputManager);
                        __register_global_object(s_gameInputManager, (void*)InputManager_Dtor, s_inputManagerDtorRegistration_80516C10);
                        s_inputManagerInitialized = 1;
                    }
                    Sing_805DC5C8() = s_gameInputManager;
                }
                if (Input_IsActionPressed(Sing_805DC5C8(), kActionRemoveCartItem, kSelectedInputChannel) == 0) {
                    flag = 0;
                } else {
                    if (AsCPangya2Shop(self)->selectedListIndex == kNoList) {
                        item = 0;
                    } else {
                        void* data = CShopProductsTab_GetSelectedProductIndex(
                            ((void**)AsCPangya2Shop(self)->productLists)[AsCPangya2Shop(self)->selectedListIndex]);
                        void* res = 0;
                        switch (AsCPangya2Shop(self)->categoryIndex) {
                        case 0:
                            res = ShopProductTable_GetRecordAt(AsCPangya2Shop(self)->charaProductBuilder,
                                AsCPangya2Shop(self)->selectedListIndex, data);
                            break;
                        case 1:
                            res = ShopProductTable_GetRecordAt(AsCPangya2Shop(self)->itemProductBuilder,
                                AsCPangya2Shop(self)->selectedListIndex, data);
                            break;
                        case 2:
                            res = ShopProductTable_GetRecordAt(AsCPangya2Shop(self)->caddieProductBuilder,
                                AsCPangya2Shop(self)->selectedListIndex, data);
                            break;
                        }
                        item = res;
                    }
                    if (item == 0) {
                        flag = 0;
                    } else if (CShopSubMenuB_RemoveItemSlotByKey(AsCPangya2Shop(self)->itemCart,
                                   (int)AsShopProductRecord(item)->typeId) != 0) {
                        flag = 1;
                    } else {
                        Sound_PlayByName(g_soundManager, shopStrings + kUiErrorSound, 0,
                            0, 0, 0, kFloat_0_5_shopSound, kFloat_200_Shop, kFloat_1_805E0918);
                        flag = 0;
                    }
                }
            }
        }
        if (flag != 0) {
            if (AsCPangya2Shop(self)->selectedListIndex != kNoList) {
                CShopProductsTab_RefreshVisibleProducts(
                    ((void**)AsCPangya2Shop(self)->productLists)[AsCPangya2Shop(self)->selectedListIndex]);
            }
            CPangya2Shop_UpdatePurchaseButton(self);
            RefreshSelectedProductMessageAndValues(self, cartChangeInfo);
            return 0;
        }
    }
    // Phase 5: activate only after all earlier gates/navigation/cart work declines the frame.
    if (CPangya2Shop_HandleProductActivation(self) != 0) {
        CPangya2Shop_UpdateControlScriptSelection(self);
        return 0;
    }
    return 0;
}
#pragma opt_common_subs reset

// Activate a ready selection; return 1 when an action/dialog was handled, otherwise 0.
// Product records and list/model objects are borrowed from the shop; no ownership transfers here.
int CPangya2Shop_HandleProductActivation(void* self) {
    // Scratch word phases: category -> optional list index -> readiness boolean.
    // 0/1/2 comparisons before the final assignment are category/list codes, not readiness.
    // A failed model poll writes 0 and breaks; passing every required poll writes 1.
    int previewGateWord;
    previewGateWord = AsCPangya2Shop(self)->categoryIndex;
    // 0x802C6C0C: Stop at the first model operation that is still loading.
    do {
        if (previewGateWord == 0) {
            if (ShopPreview_PollReady(AsCPangya2Shop(self)->charaModelOperation) == 0) {
                previewGateWord = 0;
                break;
            }
        }
        previewGateWord = AsCPangya2Shop(self)->categoryIndex;
        if (previewGateWord == 2) {
            if (ShopPreview_PollReady(AsCPangya2Shop(self)->caddieModelOperation) == 0) {
                previewGateWord = 0;
                break;
            }
        }
        previewGateWord = AsCPangya2Shop(self)->categoryIndex;
        if (previewGateWord == 1) {
            previewGateWord = AsCPangya2Shop(self)->selectedListIndex;
            if (previewGateWord == 0) {
                if (ShopPreview_PollReady(AsCPangya2Shop(self)->primaryItemModelOperation) == 0) {
                    previewGateWord = 0;
                    break;
                }
            }
        }
        previewGateWord = AsCPangya2Shop(self)->categoryIndex;
        if (previewGateWord == 1) {
            previewGateWord = AsCPangya2Shop(self)->selectedListIndex;
            if (previewGateWord == 1) {
                if (ShopPreview_PollReady(AsCPangya2Shop(self)->secondaryItemModelOperation) == 0) {
                    previewGateWord = 0;
                    break;
                }
            }
        }
        previewGateWord = 1;
    } while (0);

    if (previewGateWord == 0) {
        return 0;
    }
    if (Sing_805DC5C8() == 0) {
        if (s_inputManagerInitialized == 0) {
            char* base = s_gameInputManager;
            InputManager_Init(base);
            __register_global_object(base, (void*)InputManager_Dtor, s_inputManagerDtorRegistration_80516C10);
            s_inputManagerInitialized = 1;
        }
        Sing_805DC5C8() = s_gameInputManager;
    }
    // Removal has priority over activation while its action is held.
    if (Input_IsActionButtonDown(Sing_805DC5C8(), kActionRemoveCartItem, kSelectedInputChannel)) {
        return 0;
    }
    // 0x802C6D20: Skip activation polling for an unavailable selection.
    do {
        if (AsCPangya2Shop(self)->selectedListIndex == kNoList) break;
        if (AsCPangya2Shop(self)->categoryIndex == kCaddieCategory) {
            if (AsCPangya2Shop(self)->noCaddieSelected != 0) {
                if (AsCPangya2Shop(self)->selectedListIndex == 1) break;
            }
        }
        if (Sing_805DC5C8() == 0) {
            if (s_inputManagerInitialized == 0) {
                char* base = s_gameInputManager;
                InputManager_Init(base);
                __register_global_object(base, (void*)InputManager_Dtor, s_inputManagerDtorRegistration_80516C10);
                s_inputManagerInitialized = 1;
            }
            Sing_805DC5C8() = s_gameInputManager;
        }
        if (Input_IsActionPressed(Sing_805DC5C8(), kActionActivate, kSelectedInputChannel) == 0) {
            return 0;
        }
        {
            int state = AsCPangya2Shop(self)->categoryIndex;
            switch (state) {
            case 0: {
                    if (CPangya2Shop_OpenSelectedProductActions(self)) {
                        return 1;
                    }
                    CPangya2Shop_SelectCharaProduct(self);
                    break;
                }
            case 1: {
                    if (CPangya2Shop_OpenItemPurchaseOrUpgrade(self)) {
                        return 1;
                    }
                    CPangya2Shop_SelectItemProduct(self);
                    break;
                }
            case 2: {
                    if (CPangya2Shop_TryOpenCaddiePurchaseDialog(self)) {
                        return 1;
                    }
                    CPangya2Shop_SelectCaddieProduct(self);
                    break;
                }
            default: {
                    break;
                }
            }
        }
        if (AsCPangya2Shop(self)->selectedListIndex != kNoList) {
            void* base = AsCPangya2Shop(self)->productLists;
            int idx = AsCPangya2Shop(self)->selectedListIndex;
            void* p = ((void**)base)[idx];
            CShopProductsTab_RefreshVisibleProducts(p);
        }
        return 1;
    } while (0);

    return 0;
}

} // extern "C"

extern "C" char s_bpCtrlDtorRegistration_802C470C[];
extern "C" char s_bpCtrlHandleRegistration_80516C34[];
#include "iff/IffStruct.h"
#include "classes/CSharedDoc.h"
#include "classes/ShopProductRecord.h"
#include "classes/UITextTable.h"
// Translation unit 0x802C3258-0x802C470C (7 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

#include "game/singletons.h"
#include "sdk_protos.h"
#include "classes/CPangya2Shop.h"

#define AsCPangya2Shop(p) ((CPangya2Shop*)(p))
extern "C" {

void Layout_Update(void*);
void CProductsCategory_UpdateLayout(void*);
void CShopProductsTab_UpdateLayouts(void*);
void CPlayerStatus_UpdateLayout(void*);
void CItemCart_UpdateLayout(void*);
void CClubSubMenu_UpdateLayout(void*);
void CClubDialog_UpdateLayout(void*);
void CPurchaseDialog_UpdateLayoutAndScript(void*);
void CItemInfo_UpdateLayout(void*);
void CClubLvUpDialog_UpdateLayout(void*);
void CPackTextureResource_PollLoad(void*);
void BpObject_SetEnabled(void*, void*);
void CPlayerStatus_SetEnabled(void*, void*);
void CShopSubMenuB_SetEnabled(void*, void*);
void CShopProductsTab_SetEnabled(void*, void*);
void CClubHouseMsg_SetDrawEnabled(void*, int);
void* CShopProductsTab_GetSelectedProductIndex(void*);
void* ShopProductTable_GetRecordAt(void*, int, void*);
void* ItemInventory_GetCaddieIdAt(void*, int);
void* ItemInventory_FindCaddieData(void*, void*);
int ItemInventory_FindClubSetIndex(void*, unsigned int);
void* CClubCustomTbl_FindUpgradeEntry(int, int);
void* ItemInventory_FindDescriptionMessageKeyText(void*, int);
void CProductsCategory_SetCategoryCount(void*, void*);
void CProductsCategory_SetModeAndSelection(void*, int, int);
void* ItemInventory_GetCharacterIdAt(void*, int);
void CModelOperationCaddie_SetItem(void*, void*);
void* ItemInventory_GetCaddieItemIdAt(void*, int);
void* ItemInventory_GetCaddieIdAt(void*, int);
void* ItemInventory_GetClubSetIdAt(void*, int);
void* ItemInventory_GetBallIdAt(void*, int);
void CBuildProduct_SelectSaveRecord(void*, void*);
void CShopProductsTab_SetProductCount(void*, void*);
void* CBuildProduct_GetProductCount(void*, int);
void* CBuildProduct_GetProductRecords(void*, int);
void* CBuildProduct_GetProductDrawEntries(void*, int);
void CShopProductsTab_SetProductArrays(void*, void*, void*);
int CShopProductsTab_SelectProductByTypeId(void*, void*);
void* ItemInventory_GetPartIdAt(void*, int);
void CBuildProductCaddie_LoadIconGroups(void*, int);
void ShopItemList_BuildProductTextures(void*);
void CItemCart_SetItemCount(void*, void*);
void CItemCart_SetItemIcons(void*, void*);
void CItemCart_SetItemRecords(void*, void*);
void CShopSubMenuB_UpdateItemSlotFlags(void*);
void CClubHouseMsg_ResetTextState(void*);
void CClubHouseMsg_SetTextFromGroup(void*, void*, void*, int);
void CShopSubMenuB_SetItemMessage(void*, int);
void CPangya2Shop_UpdateCategoryHighlights(void*);
void CPangya2Shop_RefreshSelectedProductValues(void*);
void CPangya2Shop_UpdateSelectedPurchaseAmount(void*);
void CPangya2Shop_UpdatePurchaseButton(void*);
void CPlayerStatus_RefreshEquipmentDisplay(void*, int);
void* UIGroup_GetOrCreateById(void*, int);
void CBuildProductChara_LoadProductTextures(void*, int);
void CShopProductsTab_RefreshVisibleProducts(void*);
void UITextTable_SetInteger(void*, int, int);

typedef CPangya2Shop ShopScreen;

// Display price in Pang and the three stat bonuses shown by the shop UI.
// The remaining four stat bytes are cleared but not populated here.
struct SelectedProductInfo {
    int pangPrice;
    short power, control, spin;
    char reservedStats[4];
    char nameMessageId[40];
    char descriptionMessageId[40];
};

enum ShopCategory { kCharacterCategory, kItemCategory, kCaddieCategory };
enum ItemKind {
    kCharacterItem = 1, kPartItem = 2, kClubSetItem = 4, kCaddieItem = 7,
    kCaddieEquipmentItem = 8, kAuxPartItem = 28, kItemKindShift = 26
};
// Saved equipment encoding: 26-bit kind shift; caddie parent bits 21..25.
// Club upgrade bytes are at snapshot +0x4AD, i.e. shop +0x4C5, stride two.
enum SavedEquipmentLayout {
    kCaddieParentShift = 21, kCaddieParentMask = 0x1f,
    kSavedCharacterEquipmentBytes = 0x34, kSavedEquipmentIndexBytes = 2,
    kShopClubUpgradeByteOffset = 0x4c5
};
enum ShopLimits { kProductListCount = 5, kOutfitPartCount = 24, kUnsetCaddieDescription = 0x3F };
enum ShopUiId { kProductNameMessageGroup = 0x40e, kPangBalanceText = 0x2b };
// Category/list map used here: category 0=character, 1=item, 2=caddie.
// All categories share five tab slots. Character list 0 selects the character;
// lists 1..4 select equipped outfit parts (their clothing labels are unresolved).
// Item list 2 feeds the consumable cart; caddie list 1 is unavailable when no
// caddie is selected. selectedListIndex==-1 means no active tab.
// UI group 0x40E supplies the product-name message layout; numeric entry 0x2B
// displays Pang balance. These IDs belong to different UI registries.
enum IffStat { kPowerStat = 0, kControlStat = 1, kSpinStat = 3 };
// The club-upgrade table exposes these three display bonuses after a two-byte prefix.
struct ClubUpgradeDisplayRecord { char prefix[2]; short power, control, spin; };

extern CSharedDoc* g_sharedDoc;
extern char shopStrings[];
extern void* shopCategoryDescriptors[];

// Update every shop child layout and poll an optional texture resource.
void CPangya2Shop_UpdateLayoutsAndPollTexture(ShopScreen* shop) {
    Layout_Update(AsCPangya2Shop(shop)->shopLayout);
    CProductsCategory_UpdateLayout(AsCPangya2Shop(shop)->productsCategory);
    int listByteOffset, listIndex = 0;
    listByteOffset = 0;
    do {
        CShopProductsTab_UpdateLayouts(*(void**)((char*)AsCPangya2Shop(shop)->productLists + listByteOffset));
        listIndex++;
        listByteOffset += 4;
    } while (listIndex < kProductListCount);
    CPlayerStatus_UpdateLayout(AsCPangya2Shop(shop)->playerStatus);
    CItemCart_UpdateLayout(AsCPangya2Shop(shop)->itemCart);
    CClubSubMenu_UpdateLayout(AsCPangya2Shop(shop)->clubSubMenu);
    CClubDialog_UpdateLayout(AsCPangya2Shop(shop)->clubDialog);
    CPurchaseDialog_UpdateLayoutAndScript(AsCPangya2Shop(shop)->purchaseDialog);
    CItemInfo_UpdateLayout(AsCPangya2Shop(shop)->itemInfo);
    CClubLvUpDialog_UpdateLayout(AsCPangya2Shop(shop)->clubLvUpDialog);
    void* textureResource = AsCPangya2Shop(shop)->packTextureResource; if (textureResource) CPackTextureResource_PollLoad(textureResource);
}

// Enable or disable shop controls and hide the product messages and numeric text.
int CPangya2Shop_SetControlsEnabled(ShopScreen* shop, void* enabled) {
    BpObject_SetEnabled(((CPangya2Shop*)shop)->uiGroup, enabled);
    BpObject_SetEnabled(((CPangya2Shop*)shop)->scriptObject, enabled);
    CPlayerStatus_SetEnabled(((CPangya2Shop*)shop)->playerStatus, enabled);
    CShopSubMenuB_SetEnabled(((CPangya2Shop*)shop)->itemCart, enabled);
    int listByteOffset, listIndex = 0;
    listByteOffset = 0;
    do {
        CShopProductsTab_SetEnabled(*(void**)((char*)((CPangya2Shop*)shop)->productLists + listByteOffset), enabled);
        listIndex++;
        listByteOffset += 4;
    } while (listIndex < kProductListCount);
    CClubHouseMsg_SetDrawEnabled(((CPangya2Shop*)shop)->productMessage, 0);
    CClubHouseMsg_SetDrawEnabled(((CPangya2Shop*)shop)->subMenuMessage, 0);
    AsUITextTable(((CPangya2Shop*)shop)->textTable)->drawEnabled = 0;
    return 1;
}

struct CpW604 { int a[151]; };
struct CpW112 { int a[28]; };
struct CpH416 { short a[208]; };
struct CpW64 { int a[16]; };
struct CpB24 { char a[24]; };
struct CpB6 { char a[6]; };
struct CpB11 { char a[11]; };
struct CpW20 { int a[5]; };
struct CpH828 { short a[414]; };

// Copy blocks retain the original word/halfword/byte copy widths and padding.
// A shop works on a snapshot at +0x18 and commits it back only when requested.
struct PlayerRecordCopy {
    CpW604 inventoryAndStats;       // 0x000
    CpW112 courseRecords;           // 0x25C
    CpH416 characterEquipment;      // 0x2CC
    CpW64 caddieEquipment;          // 0x46C
    CpB24 clubSets;                 // 0x4AC
    CpB6 clubSetTail;               // 0x4C4
    CpB11 balls;                    // 0x4CA
    char alignmentPadding[3];
    CpW20 systemSettings;           // 0x4D8
    CpH828 modeRecords;             // 0x4EC
};
struct ShopRecordCopy { char prefix[0x18]; PlayerRecordCopy record; };
typedef char PlayerRecordCopy_size[(sizeof(PlayerRecordCopy) == 0x828) ? 1 : -1];

// Copy a player save record into the shop snapshot without changing the copy granularity.
// Both buffers borrow at least 0x828 bytes. Copy preserves the alignment gap
// before systemSettings; this is a value snapshot, with no ownership transfer.
int CPangya2Shop_CopyRecordIntoSnapshot(void* shop, const void* savedRecord) {
    ((ShopRecordCopy*)shop)->record.inventoryAndStats = ((const PlayerRecordCopy*)savedRecord)->inventoryAndStats;
    ((ShopRecordCopy*)shop)->record.courseRecords = ((const PlayerRecordCopy*)savedRecord)->courseRecords;
    ((ShopRecordCopy*)shop)->record.characterEquipment = ((const PlayerRecordCopy*)savedRecord)->characterEquipment;
    ((ShopRecordCopy*)shop)->record.caddieEquipment = ((const PlayerRecordCopy*)savedRecord)->caddieEquipment;
    ((ShopRecordCopy*)shop)->record.clubSets = ((const PlayerRecordCopy*)savedRecord)->clubSets;
    ((ShopRecordCopy*)shop)->record.clubSetTail = ((const PlayerRecordCopy*)savedRecord)->clubSetTail;
    ((ShopRecordCopy*)shop)->record.balls = ((const PlayerRecordCopy*)savedRecord)->balls;
    ((ShopRecordCopy*)shop)->record.systemSettings = ((const PlayerRecordCopy*)savedRecord)->systemSettings;
    ((ShopRecordCopy*)shop)->record.modeRecords = ((const PlayerRecordCopy*)savedRecord)->modeRecords;
    return 1;
}

// Copy the shop snapshot back into a player save record without changing the copy granularity.
int CPangya2Shop_CopySnapshotToRecord(const void* shop, void* savedRecord) {
    ((PlayerRecordCopy*)savedRecord)->inventoryAndStats = ((const ShopRecordCopy*)shop)->record.inventoryAndStats;
    ((PlayerRecordCopy*)savedRecord)->courseRecords = ((const ShopRecordCopy*)shop)->record.courseRecords;
    ((PlayerRecordCopy*)savedRecord)->characterEquipment = ((const ShopRecordCopy*)shop)->record.characterEquipment;
    ((PlayerRecordCopy*)savedRecord)->caddieEquipment = ((const ShopRecordCopy*)shop)->record.caddieEquipment;
    ((PlayerRecordCopy*)savedRecord)->clubSets = ((const ShopRecordCopy*)shop)->record.clubSets;
    ((PlayerRecordCopy*)savedRecord)->clubSetTail = ((const ShopRecordCopy*)shop)->record.clubSetTail;
    ((PlayerRecordCopy*)savedRecord)->balls = ((const ShopRecordCopy*)shop)->record.balls;
    ((PlayerRecordCopy*)savedRecord)->systemSettings = ((const ShopRecordCopy*)shop)->record.systemSettings;
    ((PlayerRecordCopy*)savedRecord)->modeRecords = ((const ShopRecordCopy*)shop)->record.modeRecords;
    return 1;
}

// Return the selected product price in Pang, displayed power/control/spin bonuses and message ids.
// productInfo must be writable. Returns 0 with a cleared result when selection
// is absent, 1 on success. Message IDs are fixed 40-byte fields copied with
// strncpy and are not guaranteed to end in NUL when the source fills the field.
int CPangya2Shop_GetSelectedProductInfo(ShopScreen* shop, SelectedProductInfo* productInfo) {
    // Phase 1: clear caller output and borrow the selected builder record.
    productInfo->pangPrice = 0;
    Memory_Clear(&productInfo->power, 0xa);
    Memory_Clear(productInfo->nameMessageId, 0x28);
    Memory_Clear(productInfo->descriptionMessageId, 0x28);

    int listIndex = AsCPangya2Shop(shop)->selectedListIndex;
    if (listIndex == -1 || (AsCPangya2Shop(shop)->noCaddieSelected != 0 && AsCPangya2Shop(shop)->categoryIndex == 2 && listIndex == 1)) {
        return 0;
    }

    void* productRecord;
    if (listIndex == -1) {
        productRecord = 0;
    } else {
        void** productLists = *(void***)&((CPangya2Shop*)shop)->productLists;
        void* productIndex = CShopProductsTab_GetSelectedProductIndex(productLists[listIndex]);
        void* record = 0;
        switch (AsCPangya2Shop(shop)->categoryIndex) {
            case 0: record = ShopProductTable_GetRecordAt(AsCPangya2Shop(shop)->charaProductBuilder, AsCPangya2Shop(shop)->selectedListIndex, productIndex); break;
            case kItemCategory: record = ShopProductTable_GetRecordAt(AsCPangya2Shop(shop)->itemProductBuilder, AsCPangya2Shop(shop)->selectedListIndex, productIndex); break;
            case kCaddieCategory: record = ShopProductTable_GetRecordAt(AsCPangya2Shop(shop)->caddieProductBuilder, AsCPangya2Shop(shop)->selectedListIndex, productIndex); break;
        }
        productRecord = record;
    }

    if (productRecord == 0) return 0;

    // Phase 2: derive the three displayed stat bonuses for the concrete IFF kind.
    // Caddie equipment uses its parent caddie; clubs use the saved upgrade level.
    int typeId;
    typeId = (int)((ShopProductRecord*)productRecord)->typeId;
    // IFF kind is the high six bits; subfields below are kind-specific.
    switch ((unsigned int)typeId >> kItemKindShift) {
        case 1:
            productInfo->power = (short)((IFF_STRUCT::sChar*)productRecord)->stats[kPowerStat];
            productInfo->control = (short)((IFF_STRUCT::sChar*)productRecord)->stats[kControlStat];
            productInfo->spin = (short)((IFF_STRUCT::sChar*)productRecord)->stats[kSpinStat];
            break;
        case 2:
            productInfo->power = ((IFF_STRUCT::sPart*)productRecord)->stats[kPowerStat];
            productInfo->control = ((IFF_STRUCT::sPart*)productRecord)->stats[kControlStat];
            productInfo->spin = ((IFF_STRUCT::sPart*)productRecord)->stats[kSpinStat];
            break;
        case kAuxPartItem:
            productInfo->power = (signed char)((IFF_STRUCT::sAuxPart*)productRecord)->stats[kPowerStat];
            productInfo->control = (signed char)((IFF_STRUCT::sAuxPart*)productRecord)->stats[kControlStat];
            productInfo->spin = (signed char)((IFF_STRUCT::sAuxPart*)productRecord)->stats[kSpinStat];
            break;
        case kCaddieItem:
            productInfo->power = ((IFF_STRUCT::sCaddie*)productRecord)->stats[kPowerStat];
            productInfo->control = ((IFF_STRUCT::sCaddie*)productRecord)->stats[kControlStat];
            productInfo->spin = ((IFF_STRUCT::sCaddie*)productRecord)->stats[kSpinStat];
            break;
        case kCaddieEquipmentItem: {
            // Caddie-equipment parent index occupies bits 21..25 (five bits).
            void* caddieTypeId = ItemInventory_GetCaddieIdAt(((CSharedDoc*)g_sharedDoc)->itemInventory, ((unsigned int)typeId >> kCaddieParentShift) & kCaddieParentMask);
            IFF_STRUCT::sCaddie* caddieRecord = (IFF_STRUCT::sCaddie*)ItemInventory_FindCaddieData(((CSharedDoc*)g_sharedDoc)->itemInventory, caddieTypeId);
            productInfo->power = (short)caddieRecord->stats[0];
            productInfo->control = (short)caddieRecord->stats[1];
            productInfo->spin = (short)caddieRecord->stats[3];
            break;
        }
        case kClubSetItem: {
            int clubSetIndex = (signed char)ItemInventory_FindClubSetIndex(((CSharedDoc*)g_sharedDoc)->itemInventory, typeId);
            void* upgradeRecord = CClubCustomTbl_FindUpgradeEntry(clubSetIndex, (signed char)*((unsigned char*)shop + clubSetIndex * kSavedEquipmentIndexBytes + kShopClubUpgradeByteOffset));
            productInfo->power = ((ClubUpgradeDisplayRecord*)upgradeRecord)->power;
            productInfo->control = ((ClubUpgradeDisplayRecord*)upgradeRecord)->control;
            productInfo->spin = ((ClubUpgradeDisplayRecord*)upgradeRecord)->spin;
            break;
        }
        case 0: case 3: case 5: case 6: case 9: case 10: case 11: case 12: case 13: case 14:
        case 15: case 16: case 17: case 18: case 19: case 20: case 21: case 22: case 23: case 24:
        case 25: case 26: case 27:
        default:
            productInfo->power = 0;
            productInfo->control = 0;
            productInfo->spin = 0;
            break;
    }

    // Phase 3: copy message keys and Pang price from the borrowed record.
    strncpy(productInfo->nameMessageId, (const char*)productRecord, 0x28);

    if ((int)((ShopProductRecord*)productRecord)->typeId == 0) {
        strncpy(productInfo->descriptionMessageId, (char*)shopStrings + kUnsetCaddieDescription, 0x28);
    } else {
        strncpy(productInfo->descriptionMessageId, (const char*)ItemInventory_FindDescriptionMessageKeyText(((CSharedDoc*)g_sharedDoc)->itemInventory, (int)((ShopProductRecord*)productRecord)->typeId), 0x28);
    }

    productInfo->pangPrice = ((ShopProductRecord*)productRecord)->price;
    return 1;
}

struct VObj { 
virtual void v0(); virtual void v1(); virtual void v2(); virtual void v3(void*);
};

#pragma use_lmw_stmw on
#pragma opt_lifetimes off
#pragma opt_common_subs off
#pragma opt_strength_reduction off
// Restore equipped products into the previews, lists and cart, then refresh shop controls and prices.
int CPangya2Shop_InitializeProductSelection(ShopScreen* shop) {
    // Phase 1: select character category/tab 1 and restore all four preview models.
    // The snapshot stores inventory row indices, whereas previews consume type IDs.
    // Compiler-shared scratch: unused record address, then outfit-part index,
    // then tab index. Splitting these lifetimes changes .text with these pragmas.
    // Each domain starts with an explicit assignment; never dereference this value.
    int selectionScratch;
    int listIndex;
    char* partCursor;
    int listByteOffset;
    ShopScreen* shopSelf;
    selectionScratch = 0;
    shopSelf = shop;
    ((CPangya2Shop*)shop)->categoryIndex = 0;
    ((CPangya2Shop*)shop)->selectedListIndex = 1;
    BpObject_SetEnabled(((CPangya2Shop*)shop)->uiGroup, (void*)1);
    BpObject_SetEnabled(((CPangya2Shop*)shop)->scriptObject, (void*)0);
    int categoryIndex = ((CPangya2Shop*)shop)->categoryIndex;
    void** categoryDescriptors = (void**)shopCategoryDescriptors;
    void* previewArgument = categoryDescriptors[categoryIndex];
    CProductsCategory_SetCategoryCount(((CPangya2Shop*)shop)->productsCategory, previewArgument);
    CProductsCategory_SetModeAndSelection(((CPangya2Shop*)shop)->productsCategory, ((CPangya2Shop*)shop)->categoryIndex, 1);
    void* inventory = ((CSharedDoc*)g_sharedDoc)->itemInventory;
    signed char c = ((CPangya2Shop*)shop)->currentCharaIndex;
    void* resolvedItemIdOrCharacterBase = ItemInventory_GetCharacterIdAt(inventory, c);
    previewArgument = resolvedItemIdOrCharacterBase;
    ((VObj*)((CPangya2Shop*)shop)->charaModelOperation)->v3(previewArgument);

    // Original unit 0x802C3258: an equipped item takes precedence over the caddie.
    if (((CPangya2Shop*)shop)->caddieItemIndex != -1 &&
        ((CPangya2Shop*)shop)->currentCaddieIndex != -1) {
        ((CPangya2Shop*)shop)->noCaddieSelected = 0;
        inventory = ((CSharedDoc*)g_sharedDoc)->itemInventory;
        resolvedItemIdOrCharacterBase = ItemInventory_GetCaddieItemIdAt(inventory, ((CPangya2Shop*)shop)->caddieItemIndex);
        previewArgument = resolvedItemIdOrCharacterBase;
        CModelOperationCaddie_SetItem(((CPangya2Shop*)shop)->caddieModelOperation, previewArgument);
    } else {
        if (((CPangya2Shop*)shop)->currentCaddieIndex == -1) {
            ((CPangya2Shop*)shop)->noCaddieSelected = 1;
        } else {
            ((CPangya2Shop*)shop)->noCaddieSelected = 0;
            inventory = ((CSharedDoc*)g_sharedDoc)->itemInventory;
            resolvedItemIdOrCharacterBase = ItemInventory_GetCaddieIdAt(inventory, ((CPangya2Shop*)shop)->currentCaddieIndex);
            previewArgument = resolvedItemIdOrCharacterBase;
            ((VObj*)((CPangya2Shop*)shop)->caddieModelOperation)->v3(previewArgument);
        }
    }
    inventory = ((CSharedDoc*)g_sharedDoc)->itemInventory;
    signed char clubSetIndex = ((CPangya2Shop*)shop)->clubSetIndex;
    resolvedItemIdOrCharacterBase = ItemInventory_GetClubSetIdAt(inventory, clubSetIndex);
    previewArgument = resolvedItemIdOrCharacterBase;
    ((VObj*)((CPangya2Shop*)shop)->primaryItemModelOperation)->v3(previewArgument);

    inventory = ((CSharedDoc*)g_sharedDoc)->itemInventory;
    signed char ballIndex = ((CPangya2Shop*)shop)->ballIndex;
    resolvedItemIdOrCharacterBase = ItemInventory_GetBallIdAt(inventory, ballIndex);
    previewArgument = resolvedItemIdOrCharacterBase;
    ((VObj*)((CPangya2Shop*)shop)->secondaryItemModelOperation)->v3(previewArgument);

    // Phase 2: builders borrow the shop snapshot; tabs borrow their record/draw arrays.
    // Keep the builders alive until tabs are destroyed. Load character icons first.
    CBuildProduct_SelectSaveRecord(((CPangya2Shop*)shop)->charaProductBuilder, &((ShopRecordCopy*)shop)->record);
    CBuildProduct_SelectSaveRecord(((CPangya2Shop*)shop)->caddieProductBuilder, &((ShopRecordCopy*)shop)->record);
    CBuildProduct_SelectSaveRecord(((CPangya2Shop*)shop)->itemProductBuilder, &((ShopRecordCopy*)shop)->record);

    signed char characterIndexForIcons = ((CPangya2Shop*)shop)->currentCharaIndex;
    CBuildProductChara_LoadProductTextures(((CPangya2Shop*)shop)->charaProductBuilder, characterIndexForIcons);

    // Phase 3: populate five character tabs and select their equipped type IDs.
    // For clothing tabs, the first of 24 part slots accepted by the tab wins.
    listIndex = 0;
    listByteOffset = 0;
    for (listIndex = 0; listIndex < kProductListCount; listIndex++, listByteOffset += 4) {
        void* productCount = CBuildProduct_GetProductCount(((CPangya2Shop*)shop)->charaProductBuilder, listIndex);
        void** arr = *(void***)&((CPangya2Shop*)shop)->productLists;
        void* productTab = *(void**)((char*)arr + listByteOffset);
        CShopProductsTab_SetProductCount(productTab, productCount);

        void* productRecords = CBuildProduct_GetProductRecords(((CPangya2Shop*)shop)->charaProductBuilder, listIndex);
        selectionScratch = (int)productRecords; // Dead address copy; never dereferenced.
        void* productDrawEntries = CBuildProduct_GetProductDrawEntries(((CPangya2Shop*)shop)->charaProductBuilder, listIndex);
        void** arr2 = *(void***)&((CPangya2Shop*)shop)->productLists;
        void* arraysTab = *(void**)((char*)arr2 + listByteOffset);
        CShopProductsTab_SetProductArrays(arraysTab, productDrawEntries, productRecords);

        if (listIndex >= 0 && listIndex < kProductListCount) {
            if (listIndex == 0) {
                inventory = ((CSharedDoc*)g_sharedDoc)->itemInventory;
                signed char characterIndexForSelection = ((CPangya2Shop*)shop)->currentCharaIndex;
                void* r = ItemInventory_GetCharacterIdAt(inventory, characterIndexForSelection);
                void** arr3 = *(void***)&((CPangya2Shop*)shop)->productLists;
                CShopProductsTab_SelectProductByTypeId(arr3[0], r);
            } else {
                signed char characterIndexForParts = ((CPangya2Shop*)shop)->currentCharaIndex;
                selectionScratch = 0; // Domain: outfit part slot, 0..23.
                // Saved equipment row: 0x34 bytes; first halfword is the character,
                // followed by 24 signed inventory part indices at +02, stride 2.
                int characterByteOffset = (int)characterIndexForParts * kSavedCharacterEquipmentBytes;
                resolvedItemIdOrCharacterBase = (char*)shop + characterByteOffset;
                partCursor = (char*)&((ShopRecordCopy*)resolvedItemIdOrCharacterBase)->record.characterEquipment;
                for (selectionScratch = 0; selectionScratch < kOutfitPartCount; selectionScratch++) {
                    inventory = ((CSharedDoc*)g_sharedDoc)->itemInventory;
                    short h = *(short*)(partCursor + kSavedEquipmentIndexBytes);
                    void* r = ItemInventory_GetPartIdAt(inventory, h);
                    void** arr4 = *(void***)&((CPangya2Shop*)shop)->productLists;
                    void* o2 = *(void**)((char*)arr4 + listByteOffset);
                    int res = CShopProductsTab_SelectProductByTypeId(o2, r);
                    if (res != 0) break;
                    partCursor += kSavedEquipmentIndexBytes;
                }
            }
        }
    }

    // Phase 4: load caddie/item icons and bind item-list 2 as the consumable cart.
    signed char caddieIndex = ((CPangya2Shop*)shop)->currentCaddieIndex;
    CBuildProductCaddie_LoadIconGroups(((CPangya2Shop*)shop)->caddieProductBuilder, caddieIndex);
    ShopItemList_BuildProductTextures(((CPangya2Shop*)shop)->itemProductBuilder);

    void* cartItemCount = CBuildProduct_GetProductCount(((CPangya2Shop*)shop)->itemProductBuilder, 2);
    CItemCart_SetItemCount(((CPangya2Shop*)shop)->itemCart, cartItemCount);

    void* cartIcons = CBuildProduct_GetProductDrawEntries(((CPangya2Shop*)shop)->itemProductBuilder, 2);
    CItemCart_SetItemIcons(((CPangya2Shop*)shop)->itemCart, cartIcons);

    void* cartRecords = CBuildProduct_GetProductRecords(((CPangya2Shop*)shop)->itemProductBuilder, 2);
    CItemCart_SetItemRecords(((CPangya2Shop*)shop)->itemCart, cartRecords);

    CShopSubMenuB_UpdateItemSlotFlags(((CPangya2Shop*)shop)->itemCart);

    // Phase 5: enable messages, resolve the selected product, and refresh prices.
    CClubHouseMsg_SetDrawEnabled(((CPangya2Shop*)shop)->productMessage, 1);
    CClubHouseMsg_SetDrawEnabled(((CPangya2Shop*)shop)->subMenuMessage, 1);

    AsUITextTable(((CPangya2Shop*)shop)->textTable)->drawEnabled = 1;
    CPlayerStatus_RefreshEquipmentDisplay(((CPangya2Shop*)shop)->playerStatus, ((CPangya2Shop*)shop)->categoryIndex);

    CPangya2Shop_UpdateCategoryHighlights(shopSelf);

    CClubHouseMsg_ResetTextState(((CPangya2Shop*)shop)->productMessage);

    SelectedProductInfo buf;
    buf.pangPrice = 0;
    Memory_Clear((char*)&buf.power, 0xa);
    Memory_Clear(buf.nameMessageId, 0x28);
    Memory_Clear(buf.descriptionMessageId, 0x28);

    int ret = CPangya2Shop_GetSelectedProductInfo(shopSelf, &buf);
    if (ret != 0) {
        void* nameMessageGroup = UIGroup_GetOrCreateById(((CPangya2Shop*)shop)->uiGroup, kProductNameMessageGroup);
        CClubHouseMsg_SetTextFromGroup(((CPangya2Shop*)shop)->productMessage, buf.nameMessageId, nameMessageGroup, 0);
    }

    CPangya2Shop_RefreshSelectedProductValues(shopSelf);
    CPangya2Shop_UpdateSelectedPurchaseAmount(shopSelf);

    // Phase 6: route input to either the cart or equipment status, then activate one tab.
    int currentCategory = ((CPangya2Shop*)shop)->categoryIndex;
    if (currentCategory == kItemCategory && ((CPangya2Shop*)shop)->selectedListIndex == 2) {
        CPlayerStatus_SetEnabled(((CPangya2Shop*)shop)->playerStatus, (void*)0);
        CShopSubMenuB_SetEnabled(((CPangya2Shop*)shop)->itemCart, (void*)1);

        int selectedCartRecordAddress;
        int selectedCartListIndex = ((CPangya2Shop*)shop)->selectedListIndex;
        if (selectedCartListIndex == -1) {
            selectedCartRecordAddress = 0;
        } else {
            void** arr = *(void***)&((CPangya2Shop*)shop)->productLists;
            void* cartTab = arr[selectedCartListIndex];
            void* selectedCartProductIndex = CShopProductsTab_GetSelectedProductIndex(cartTab);
            int cartCategoryIndex = ((CPangya2Shop*)shop)->categoryIndex;
            int cartListIndex = ((CPangya2Shop*)shop)->selectedListIndex;
            void* selectedCartRecord = (void*)0;
            switch (cartCategoryIndex) {
            case 0: selectedCartRecord = ShopProductTable_GetRecordAt(((CPangya2Shop*)shop)->charaProductBuilder, cartListIndex, selectedCartProductIndex); break;
            case 1: selectedCartRecord = ShopProductTable_GetRecordAt(((CPangya2Shop*)shop)->itemProductBuilder, cartListIndex, selectedCartProductIndex); break;
            case 2: selectedCartRecord = ShopProductTable_GetRecordAt(((CPangya2Shop*)shop)->caddieProductBuilder, cartListIndex, selectedCartProductIndex); break;
            }
            selectedCartRecordAddress = (int)selectedCartRecord;
        }
        if (selectedCartRecordAddress != 0) {
            CShopSubMenuB_SetItemMessage(((CPangya2Shop*)shop)->itemCart, (int)((ShopProductRecord*)selectedCartRecordAddress)->typeId);
        }
    } else {
        CPlayerStatus_SetEnabled(((CPangya2Shop*)shop)->playerStatus, (void*)1);
        CShopSubMenuB_SetEnabled(((CPangya2Shop*)shop)->itemCart, (void*)0);
    }

    selectionScratch = 0; // Domain: product tab index, 0..4.
    listByteOffset = 0;
    for (selectionScratch = 0; selectionScratch < 5; selectionScratch++, listByteOffset += 4) {
        if (selectionScratch == ((CPangya2Shop*)shop)->selectedListIndex) {
            void** arr = *(void***)&((CPangya2Shop*)shop)->productLists;
            CShopProductsTab_SetEnabled(*(void**)((char*)arr + listByteOffset), (void*)1);
        } else {
            void** arr = *(void***)&((CPangya2Shop*)shop)->productLists;
            CShopProductsTab_SetEnabled(*(void**)((char*)arr + listByteOffset), (void*)0);
        }
    }

    CPangya2Shop_UpdatePurchaseButton(shopSelf);

    int activeListIndex = ((CPangya2Shop*)shop)->selectedListIndex;
    if (activeListIndex != -1) {
        void** arr = *(void***)&((CPangya2Shop*)shop)->productLists;
        void* o = arr[activeListIndex];
        CShopProductsTab_RefreshVisibleProducts(o);
    }

    UITextTable_SetInteger(((CPangya2Shop*)shop)->textTable, kPangBalanceText, *(int*)&((CPangya2Shop*)shop)->pangBalance);

    return 1;
}
#pragma opt_strength_reduction reset
#pragma opt_common_subs reset
#pragma opt_lifetimes reset
#pragma use_lmw_stmw reset
extern char __vt__12CPangya2Shop[];
extern char s_bpCtrl[];
extern char s_fontNumDrawRegistry[];
extern char s_inputManagerDtorRegistration_80516C10[];
extern signed char s_bpCtrlInitialized;
extern signed char s_bpCtrlHandleInitialized;
extern int s_bpCtrlHandle[2];
extern signed char s_pointerVectorInitialized;
extern void Layout_Destroy(void*, int);
extern void __ct__7CBpCtrlFv(void*);
extern void CBpCtrl_dtor(void*);
extern void CBpCtrl_DeleteEntryById(void*, int);
extern void CBpCtrlHandle_Dtor(void*);
extern void UITextTable_ClearEntries(void*);
extern void PointerVector_InitReserved(void*);
extern void PointerVector_DestroyStorage(void*);
extern void PointerArray_DeleteValue(void*, void*);
extern void CBpCtrlHandle_ReleaseEntry(void*);
extern void* __register_global_object(void*, void*, void*);
extern void __dla__FPv(void*);
extern void __dl__FPv(void*) throw();

struct ShopChild { 
virtual void destroy(int);
};

#pragma use_lmw_stmw on
// Destroy owned shop UI, previews and product builders, unregister handles, and optionally free the shop.
void* CPangya2Shop_dtor(ShopScreen* shop, int deleteStorage) {
    char* inputDtorRegistration = (char*)s_inputManagerDtorRegistration_80516C10;
    if (shop != 0) { int zero = 0; int listIndex = 0; int listByteOffset;
    *(void**)shop = (void*)__vt__12CPangya2Shop;

    // Phase 1: destroy tabs and layout before releasing the arrays/providers they borrow.
       listByteOffset = 0;
    do {
        void* child = *(void**)((char*)AsCPangya2Shop(shop)->productLists + listByteOffset);
        if (child) {
            if (0 != child) ((ShopChild*)child)->destroy(1);
            *(void**)((char*)AsCPangya2Shop(shop)->productLists + listByteOffset) = (void*)zero;
        }
        listByteOffset += 4;
        __typeof__(listIndex + 1) savedR26Copy = (listIndex + 1); listIndex = savedR26Copy;
    } while (kProductListCount > listIndex);
    {
        void** arr = (*(void***)&AsCPangya2Shop(shop)->productLists);
        if (arr != 0) {
            __dla__FPv(arr);
            AsCPangya2Shop(shop)->productLists = 0;
        }
    }
    {
        void* child = AsCPangya2Shop(shop)->shopLayout;
        if (child != 0) {
            Layout_Destroy(child, 1);
            AsCPangya2Shop(shop)->shopLayout = 0;
        }
    }

    // Ownership order below (each child is destroyed with deleting=1 then nulled):
    // category -> character/caddie/club/ball previews -> character/caddie/item
    // builders -> player status -> cart -> numeric-table registry -> messages
    // -> club/purchase/info/save dialogs -> packed files -> texture -> BP handle.
    // Singleton checks repeat because every unregister/delete may call external code.
    // Phase 2: unregister both BP entries before destroying previews and product builders.
    if (AsCPangya2Shop(shop)->uiGroupEntryId != -1) {
        if (Sing_805DC5F0() == 0) {
            if (!s_bpCtrlInitialized) {
                __ct__7CBpCtrlFv(s_bpCtrl);
                __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, (void*)((inputDtorRegistration + 0xc)));
                s_bpCtrlInitialized = 1;
            }
            Sing_805DC5F0() = s_bpCtrl;
        }
        CBpCtrl_DeleteEntryById(Sing_805DC5F0(), AsCPangya2Shop(shop)->uiGroupEntryId);
        AsCPangya2Shop(shop)->uiGroupEntryId = -1;
    }

    if (AsCPangya2Shop(shop)->scriptEntryId != -1) {
        if (Sing_805DC5F0() == 0) {
            if (0 == s_bpCtrlInitialized) {
                __ct__7CBpCtrlFv(s_bpCtrl);
                __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, (void*)((inputDtorRegistration + 0xc)));
                s_bpCtrlInitialized = 1;
            }
            Sing_805DC5F0() = s_bpCtrl;
        }
        CBpCtrl_DeleteEntryById(Sing_805DC5F0(), AsCPangya2Shop(shop)->scriptEntryId);
        AsCPangya2Shop(shop)->scriptEntryId = -1;
    }

    {
        void* child = AsCPangya2Shop(shop)->productsCategory;
        if (child != 0) {
            if (0 != child) ((ShopChild*)child)->destroy(1);
            AsCPangya2Shop(shop)->productsCategory = 0;
        }
    }
    {
        void* child = AsCPangya2Shop(shop)->charaModelOperation;
        if (0 != child) {
            if (0 != child) ((ShopChild*)child)->destroy(1);
            AsCPangya2Shop(shop)->charaModelOperation = 0;
        }
    }
    {
        void* child = AsCPangya2Shop(shop)->caddieModelOperation;
        if (child != 0) {
            if (child != 0) ((ShopChild*)child)->destroy(1);
            AsCPangya2Shop(shop)->caddieModelOperation = 0;
        }
    }
    {
        void* child = AsCPangya2Shop(shop)->primaryItemModelOperation;
        if (child != 0) {
            if (child != 0) ((ShopChild*)child)->destroy(1);
            AsCPangya2Shop(shop)->primaryItemModelOperation = 0;
        }
    }
    {
        void* child = AsCPangya2Shop(shop)->secondaryItemModelOperation;
        if (0 != child) {
            if (child != 0) ((ShopChild*)child)->destroy(1);
            AsCPangya2Shop(shop)->secondaryItemModelOperation = 0;
        }
    }
    {
        void* child = AsCPangya2Shop(shop)->charaProductBuilder;
        if (child != 0) {
            if (0 != child) ((ShopChild*)child)->destroy(1);
            AsCPangya2Shop(shop)->charaProductBuilder = 0;
        }
    }
    {
        void* child = AsCPangya2Shop(shop)->caddieProductBuilder;
        if (child != 0) {
            if (child != 0) ((ShopChild*)child)->destroy(1);
            AsCPangya2Shop(shop)->caddieProductBuilder = 0;
        }
    }
    {
        void* child = AsCPangya2Shop(shop)->itemProductBuilder;
        if (child != 0) {
            if (child) ((ShopChild*)child)->destroy(1);
            AsCPangya2Shop(shop)->itemProductBuilder = 0;
        }
    }
    {
        void* child = AsCPangya2Shop(shop)->playerStatus;
        if (child) {
            if (child != 0) ((ShopChild*)child)->destroy(1);
            AsCPangya2Shop(shop)->playerStatus = 0;
        }
    }
    {
        void* child = AsCPangya2Shop(shop)->itemCart;
        if (child != 0) {
            if (child != 0) ((ShopChild*)child)->destroy(1);
            AsCPangya2Shop(shop)->itemCart = 0;
        }
    }

    {
        void* child = AsCPangya2Shop(shop)->textTable;
        // Phase 3: clear the numeric table and remove it through its owning registry.
        UITextTable_ClearEntries(child);
        if (AsCPangya2Shop(shop)->textTable != 0) {
                if (Sing_805DC830() == 0) {
                    if (!s_pointerVectorInitialized) {
                        PointerVector_InitReserved(s_fontNumDrawRegistry);
                        __register_global_object(s_fontNumDrawRegistry, (void*)PointerVector_DestroyStorage, (void*)((inputDtorRegistration + 0x18)));
                        s_pointerVectorInitialized = 1;
                    }
                    Sing_805DC830() = s_fontNumDrawRegistry;
                }
                PointerArray_DeleteValue(Sing_805DC830(), AsCPangya2Shop(shop)->textTable);
            }
    }

    {
        void* child = AsCPangya2Shop(shop)->productMessage;
        if (child) {
            if (child != 0) ((ShopChild*)child)->destroy(1);
            AsCPangya2Shop(shop)->productMessage = 0;
        }
    }
    {
        void* child = AsCPangya2Shop(shop)->subMenuMessage;
        if (0 != child) {
            if (0 != child) ((ShopChild*)child)->destroy(1);
            AsCPangya2Shop(shop)->subMenuMessage = 0;
        }
    }
    {
        void* child = AsCPangya2Shop(shop)->clubSubMenu;
        if (child != 0) {
            if (child != 0) ((ShopChild*)child)->destroy(1);
            AsCPangya2Shop(shop)->clubSubMenu = 0;
        }
    }
    {
        void* child = AsCPangya2Shop(shop)->clubDialog;
        if (0 != child) {
            if (child != 0) ((ShopChild*)child)->destroy(1);
            AsCPangya2Shop(shop)->clubDialog = 0;
        }
    }
    {
        void* child = AsCPangya2Shop(shop)->purchaseDialog;
        if (child) {
            if (0 != child) ((ShopChild*)child)->destroy(1);
            AsCPangya2Shop(shop)->purchaseDialog = 0;
        }
    }
    {
        void* child = AsCPangya2Shop(shop)->itemInfo;
        if (child != 0) {
            if (0 != child) ((ShopChild*)child)->destroy(1);
            AsCPangya2Shop(shop)->itemInfo = 0;
        }
    }
    {
        void* child = AsCPangya2Shop(shop)->clubLvUpDialog;
        if (child != 0) {
            if (0 != child) ((ShopChild*)child)->destroy(1);
            AsCPangya2Shop(shop)->clubLvUpDialog = 0;
        }
    }
    {
        void* child = AsCPangya2Shop(shop)->saveMenu;
        if (child != 0) {
            if (child) ((ShopChild*)child)->destroy(1);
            AsCPangya2Shop(shop)->saveMenu = 0;
        }
    }

    // Phase 4: release the three packed-file owners and texture resource last.
    listIndex = 0; listByteOffset = 0; zero = 0;
    do {
        void* child = *(void**)((char*)AsCPangya2Shop(shop)->packedFiles + listByteOffset);
        if (child) {
            if (child != 0) ((ShopChild*)child)->destroy(1);
            *(void**)((char*)AsCPangya2Shop(shop)->packedFiles + listByteOffset) = (void*)zero;
        }
        listByteOffset += 4;
        __typeof__(listIndex + 1) savedR26 = listIndex + 1; listIndex = savedR26;
    } while ((unsigned int)listIndex < 3);
    {
        void** arr = (*(void***)&AsCPangya2Shop(shop)->packedFiles);
        if (arr) {
            __dla__FPv(arr);
            AsCPangya2Shop(shop)->packedFiles = 0;
        }
    }

    {
        void* child = AsCPangya2Shop(shop)->packTextureResource;
        if (0 != child) {
            if (child != 0) ((ShopChild*)child)->destroy(1);
            AsCPangya2Shop(shop)->packTextureResource = 0;
        }
    }

    if (Sing_805DC5F8() == 0) {
        if (!s_bpCtrlHandleInitialized) {
            s_bpCtrlHandle[0] = -1;
            s_bpCtrlHandle[1] = -1;
            __register_global_object(s_bpCtrlHandle, (void*)CBpCtrlHandle_Dtor, (void*)((inputDtorRegistration + 0x24)));
            s_bpCtrlHandleInitialized = 1;
        }
        Sing_805DC5F8() = (void*)s_bpCtrlHandle;
    }
    // Phase 5: drop the shared controller handle, then optionally free this object.
    CBpCtrlHandle_ReleaseEntry(Sing_805DC5F8());

    if (deleteStorage > 0) {
        __dl__FPv(shop);
    }
    }
    return shop;
}
#pragma use_lmw_stmw reset
} // extern "C"

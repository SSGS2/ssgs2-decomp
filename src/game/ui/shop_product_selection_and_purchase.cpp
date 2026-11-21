// Byte offsets into the existing string pool; keep its storage and encoding.
enum {
    kShopStrings_item_select_cloth = 0xae, // item_select_cloth
    kShopStrings_item_select_hair = 0xc0, // item_select_hair
    kShopStrings_item_select_club = 0xd1, // item_select_club
    kShopStrings_item_select_aztec = 0xe2, // item_select_aztec
    kShopStrings_std_exception_std_logic_error_std_out_of_range = 0xf4, // !std::exception!!std::logic_error!!std::out_of_range!!
    kShopStrings_shop_cart_buy = 0x12b, // shop_cart_buy
};

enum { kShopSavePayloadOffset = 0x18, kCaddieOwnedFlag = 0x40 };

// IFF type IDs encode the record kind in bits 26..31 (docs/iff-formats.md).
enum ItemKind {
    kItemCharacter = 1, kItemPart = 2, kItemClubSet = 4, kItemBall = 5,
    kItemConsumable = 6, kItemCaddie = 7, kItemCaddieEquipment = 8,
    kItemHairStyle = 15, kItemAuxPart = 28
};
enum ShopCategory { kShopCharacter = 0, kShopItem = 1, kShopCaddie = 2 };
enum ShopControl { kPurchaseButton = 13, kPangText = 43, kProductMessageGroup = 0x40e };
enum { kItemKindShift = 26, kCharacterEquipmentSlots = 24, kInsufficientPangDialog = 5,
    kOutfitCompatibilityHalfwordCount = kCharacterEquipmentSlots + 1 };

// Save payload embedded in the shop; these are inventory indices, not IFF type IDs.
struct ShopCharacterEquipment {
    short prefix;
    short partIndices[kCharacterEquipmentSlots];
    signed char hairStyleIndex;
    char padding;
};
struct ShopSaveEquipment {
    char padding000[0x128];
    unsigned int ownedCaddieEquipment;
    char padding12c[0x2e4 - 0x12c];
    ShopCharacterEquipment characters[8];
    struct Caddie { unsigned char flags; char padding[7]; } caddies[11];
};
typedef char ShopCharacterEquipment_size[(sizeof(ShopCharacterEquipment) == 0x34) ? 1 : -1];
// A shifted cursor view for the original compatibility scan; not another row.
struct OutfitCompatibilityCursor { short precedingHalfword; short inventoryIndex; };

#include "iff/IffStruct.h"
#include "classes/CSharedDoc.h"
#define AsCSharedDoc(p) ((CSharedDoc*)(p))
#include "classes/CPangya2Shop.h"
#include "classes/ShopProductRecord.h"
#include "classes/SavePlayerRecord.h"
// CPangya2Shop_SelectCharaProduct and selection helpers: ShopProductTable_GetRecordAt returns the product record.
#define AsCPangya2Shop(p) ((CPangya2Shop*)(p))

// Translation unit 0x802C7308-0x802C874C (8 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

#include "sdk_protos.h"

// Use the caller-provided placement address without allocating.
inline void* operator new(size_t, void* p) { return p; }
#pragma use_simple_stdpnew on

extern "C" {

extern void UITextTable_SetInteger(void*, int, int);
extern void CClubHouseMsg_ResetTextState(void*);
extern int CPangya2Shop_GetSelectedProductInfo(void*, int*);
extern void* UIGroup_GetOrCreateById(void*, int);
extern void CClubHouseMsg_SetTextFromGroup(void*, void*, void*, int);
extern void CPangya2Shop_RefreshSelectedProductValues(void*);
extern void CPangya2Shop_UpdateSelectedPurchaseAmount(void*);
extern int CPangya2Shop_PurchaseCharacterOrOutfitItem(void*, int);
extern int CPangya2Shop_ApplyNonOutfitPurchase(void*, int);
extern void CPangya2Shop_UpdateControlScriptSelection(void*);
extern int CShopProductsTab_IsSelectedProductOwned(void*);
extern void Layout_SetElementValueState(void*, int, int);
extern void Layout_SetElementEnabled(void*, int, int);
extern int CShopProductsTab_GetSelectedProductIndex(void*);
extern int ShopProductTable_GetRecordAt(void*, int, int);
extern int ItemInventory_FindCaddieIndex(void*, int);
extern void* ItemInventory_GetCaddieItemIdAt(void*, int);
extern void CBuildProductCaddie_LoadIconGroups(void*, int);
extern void* CBuildProduct_GetProductCount(void*, int);
extern void* CBuildProduct_GetProductRecords(void*, int);
extern void* CBuildProduct_GetProductDrawEntries(void*, int);
extern void CShopProductsTab_SetProductArrays(void*, void*, void*);
extern int CModelOperationCaddie_ToggleItem(void*, unsigned int);
extern int CShopProductsTab_SetProductCount(void*, void*);
extern int CShopProductsTab_SelectProductByTypeId(void*, void*);
extern int CBuildProductChara_FindSetContainingItem(void*, int);
extern void* ItemInventory_FindSetItemData(void*, void*);
extern void* ItemInventory_GetHairStyleIdAt(void*, int);
extern void CModelOperationChara_SetHairStyle(void*, void*);
extern unsigned int ItemInventory_GetPartIdAt(void*, int);
extern void* CModelOperationChara_ApplyPartChanges(void*, unsigned int);
extern void CModelOperationChara_RequestModelUpdate(void*, void*);
extern void CModelOperationChara_WriteEquipmentToSave(void*);
extern void CPlayerStatus_RefreshEquipmentDisplay(void*, int);
extern void CShopProductsTab_RefreshVisibleProducts(void*);
extern void CPangya2Shop_UpdateUnownedSelectedOutfitCategoryMarkers(void*);
extern int ItemInventory_FindCharacterIndex(void*, int);
extern int ItemInventory_GetCharacterIdAt(void*, int);
extern void CBuildProductChara_LoadProductTextures(void*, int);
extern void CModelOperationChara_ToggleAttachment(void*, int, int);
extern void* g_sharedDoc;
extern void* g_soundManager;
extern char shopStrings[];
extern float kFloat_1_805E0918;
extern float kFloat_0_5_shopSound;
extern float kFloat_200_Shop;
extern void* ItemInventory_GetPricePang(void*, void*);
extern int ItemInventory_FindCaddieItemIndex(void*, void*);
extern void* ItemInventory_GetCaddieItemIdAt(void*, int);
extern void CClubDialog_SetMode(void*, int);
extern void* __nw__FUl(unsigned long);
extern void __dla__FPv(void*);
extern void* __nwa__FUl(unsigned long);
extern char __vt__Q23std11logic_error[];
extern char s_sharedCharDeleterVtable[];
extern char __vt__Q23std12out_of_range[];
extern char s_shopPurchaseBitRangeErrorText[];
extern void __dt__Q23std12out_of_rangeFv(void*, int);
extern void __throw(const char*, void*, void*) __attribute__((noreturn));
#pragma does_not_return __throw
extern "C" void __dt__Q23std9exceptionFv(void*, int);
extern void Sound_PlayByName(void*, const char*, int, int, int, int, float, float, float);
extern int CItemCart_AddItem(void*, int);
extern void CPlayerStatus_UpdateEquipmentTextures(void*);
extern int ItemInventory_FindItemIndex(void*, void*);
extern int ItemInventory_FindClubSetIndex(void*, void*);
extern int ItemInventory_FindBallIndex(void*, void*);
extern int ItemInventory_FindHairStyleIndex(void*, int);
typedef CPangya2Shop Obj;

// Refresh the displayed Pang balance from the shop save payload.
int CPangya2Shop_UpdateMoneyDisplay(Obj* self) {
    UITextTable_SetInteger(self->textTable, kPangText, (*(int*)&self->pangBalance));
    return 1;
}

// Refresh the selected product message, values and purchase quantity.
int CPangya2Shop_RefreshProductDetails(Obj* self) {
    struct Frame {
        int itemId;
        char shortDescription[0xa];
        char description[0x28];
        char detail[0x28];
    } productInfo;

    CClubHouseMsg_ResetTextState(self->productMessage);
    productInfo.itemId = 0;
    Memory_Clear(productInfo.shortDescription, 0xa);
    Memory_Clear(productInfo.description, 0x28);
    Memory_Clear(productInfo.detail, 0x28);

    if (CPangya2Shop_GetSelectedProductInfo(self, &productInfo.itemId)) {
        void* r = UIGroup_GetOrCreateById(self->uiGroup, kProductMessageGroup);
        CClubHouseMsg_SetTextFromGroup(self->productMessage, productInfo.description, r, 0);
    }

    CPangya2Shop_RefreshSelectedProductValues(self);
    CPangya2Shop_UpdateSelectedPurchaseAmount(self);
    return 1;
}

// Enable buying only for a valid, unowned product outside character entry and the item-cart list.
int CPangya2Shop_UpdatePurchaseButton(Obj* self) {
    int canPurchase = 0;
    if (self->characterEntryMode == 0) {
        int listIndex = self->selectedListIndex;
        if (listIndex != -1) {
            if (!(self->categoryIndex == kShopItem && listIndex == 2)) {
                if (CShopProductsTab_IsSelectedProductOwned(((void**)self->productLists)[listIndex]) == 0) {
                    canPurchase = 1;
                }
            }
        }
    }
    if (!(self->categoryIndex == kShopCaddie && self->selectedListIndex == 0)) {
        Layout_SetElementValueState(self->shopLayout, kPurchaseButton, 0);
    } else {
        Layout_SetElementValueState(self->shopLayout, kPurchaseButton, 2);
    }
    Layout_SetElementEnabled(self->shopLayout, kPurchaseButton, canPurchase);
    return 1;
}

// Preview a club set or ball, or add an owned consumable to the item cart.
int CPangya2Shop_SelectItemProduct(Obj* self) {
    Obj* shop = self;
    void* selectedProductOrId;
    char* equipmentBase;
    void* productIndex = 0;
    int categoryOrList = shop->selectedListIndex;
    if (categoryOrList == -1) {
        selectedProductOrId = 0;
    } else {
        void* p = ((void**)shop->productLists)[categoryOrList];
        productIndex = (void*)CShopProductsTab_GetSelectedProductIndex(p);
        categoryOrList = shop->categoryIndex;
        void* r = 0;
        switch (categoryOrList) {
            case kShopCharacter: r = (void*)((void*(*)(void*, int, void*))ShopProductTable_GetRecordAt)(shop->charaProductBuilder, shop->selectedListIndex, productIndex); break;
            case kShopItem: r = (void*)((void*(*)(void*, int, void*))ShopProductTable_GetRecordAt)(shop->itemProductBuilder, shop->selectedListIndex, productIndex); break;
            case kShopCaddie: r = (void*)((void*(*)(void*, int, void*))ShopProductTable_GetRecordAt)(shop->caddieProductBuilder, shop->selectedListIndex, productIndex); break;
            default: break;
        }
        selectedProductOrId = r;
    }
    if (selectedProductOrId == 0) return 0;

    equipmentBase = (char*)shop + kShopSavePayloadOffset;
    int itemId = (int)((ShopProductRecord*)selectedProductOrId)->typeId;
    unsigned int itemKind = (unsigned int)itemId >> kItemKindShift;
    switch (itemKind) {
        case kItemClubSet: {
            int inventoryIndex = ItemInventory_FindClubSetIndex(AsCSharedDoc(g_sharedDoc)->itemInventory, (void*)itemId);
            signed char equippedIndex = AsSavePlayerRecord(equipmentBase)->clubSetIndex;
            if (equippedIndex == inventoryIndex) return 0;
            selectedProductOrId = (void*)itemId;
            if (selectedProductOrId != 0) {
                int idx = ItemInventory_FindClubSetIndex(AsCSharedDoc(g_sharedDoc)->itemInventory, selectedProductOrId);
                shop->clubSetIndex = (signed char)idx;
                void* obj = shop->primaryItemModelOperation;
                void** vt = *(void***)obj;
                void (*vf)(void*, int) = (void(*)(void*, int))vt[5];
                vf(obj, (int)selectedProductOrId);
                Sound_PlayByName(g_soundManager, (char*)shopStrings + kShopStrings_item_select_club, 0, 0, 0, 0, kFloat_0_5_shopSound, kFloat_200_Shop, kFloat_1_805E0918);
            }
            CPlayerStatus_RefreshEquipmentDisplay(shop->playerStatus, shop->categoryIndex);
            return 1;
        }
        case kItemBall: {
            int inventoryIndex = ItemInventory_FindBallIndex(AsCSharedDoc(g_sharedDoc)->itemInventory, (void*)itemId);
            signed char equippedIndex = AsSavePlayerRecord(equipmentBase)->ballIndex;
            if (equippedIndex == inventoryIndex) return 0;
            selectedProductOrId = (void*)itemId;
            if (selectedProductOrId != 0) {
                int idx = ItemInventory_FindBallIndex(AsCSharedDoc(g_sharedDoc)->itemInventory, selectedProductOrId);
                shop->ballIndex = (signed char)idx;
                void* obj = shop->secondaryItemModelOperation;
                void** vt = *(void***)obj;
                void (*vf)(void*, int) = (void(*)(void*, int))vt[5];
                vf(obj, (int)selectedProductOrId);
                Sound_PlayByName(g_soundManager, (char*)shopStrings + kShopStrings_item_select_aztec, 0, 0, 0, 0, kFloat_0_5_shopSound, kFloat_200_Shop, kFloat_1_805E0918);
            }
            CPlayerStatus_UpdateEquipmentTextures(shop->playerStatus);
            return 1;
        }
        case kItemConsumable: {
            int idx = ItemInventory_FindItemIndex(AsCSharedDoc(g_sharedDoc)->itemInventory, (void*)itemId);
            int i = idx << 1;
            short quantity = *(short*)(equipmentBase + i + 0x11c);
            if (quantity <= 0) return 0;
            CItemCart_AddItem(shop->itemCart, itemId);
            int (*_f)(Obj*) = CPangya2Shop_UpdatePurchaseButton;
            _f(shop);
            CClubHouseMsg_ResetTextState(shop->productMessage);
            struct Frame {
                int itemId;
                char shortDescription[0xa];
                char description[0x28];
                char detail[0x28];
            } productInfo;
            productInfo.itemId = 0;
            Memory_Clear(productInfo.shortDescription, 0xa);
            Memory_Clear(productInfo.description, 0x28);
            Memory_Clear(productInfo.detail, 0x28);
            if (CPangya2Shop_GetSelectedProductInfo(shop, &productInfo.itemId)) {
                void* rr = UIGroup_GetOrCreateById(shop->uiGroup, kProductMessageGroup);
                CClubHouseMsg_SetTextFromGroup(shop->productMessage, productInfo.description, rr, 0);
            }
            CPangya2Shop_RefreshSelectedProductValues(shop);
            CPangya2Shop_UpdateSelectedPurchaseAmount(shop);
            return 1;
        }
        default:
            return 0;
    }
}

// Apply the selected outfit set, including its hair style and any missing parts.
int CPangya2Shop_ApplySelectedSet(Obj* self) {
    void* setItemCursor = 0;
    void* equipmentSlotCursor = 0;
    void* setRecord = 0;
    void* characterEquipment = 0;
    int alreadyEquipped = 0;
    int setItemIndex = 0;
    int equipmentSlotIndex = 0;

    // Phase 1: resolve the selected product and its containing set; 0 means absent.
    int selectedProductAddress;
    if (self->selectedListIndex == -1) {
        selectedProductAddress = 0;
    } else {
        void* p = ((void**)self->productLists)[self->selectedListIndex];
        void* selectedProductIndex = (void*)CShopProductsTab_GetSelectedProductIndex(p);
        (void)selectedProductIndex;
        int shopCategory = self->categoryIndex;
        int productAddress = 0;
        switch (shopCategory) {
            case kShopCharacter: productAddress = ShopProductTable_GetRecordAt(self->charaProductBuilder, self->selectedListIndex, (int)selectedProductIndex); break;
            case kShopItem: productAddress = ShopProductTable_GetRecordAt(self->itemProductBuilder, self->selectedListIndex, (int)selectedProductIndex); break;
            case kShopCaddie: productAddress = ShopProductTable_GetRecordAt(self->caddieProductBuilder, self->selectedListIndex, (int)selectedProductIndex); break;
            default: break;
        case 24: break;   // value 24 is accepted and yields no product (same as default); its meaning is not known
}
        selectedProductAddress = productAddress;
    }
    if (selectedProductAddress == 0) return 0;
    int setTypeId = CBuildProductChara_FindSetContainingItem(self->charaProductBuilder, ((ShopProductRecord*)selectedProductAddress)->typeId);
    if (setTypeId == 0) return 0;

    // Phase 2: apply the optional set hairstyle (-1 means no hairstyle).
    setRecord = ItemInventory_FindSetItemData(AsCSharedDoc(g_sharedDoc)->itemInventory, (void*)setTypeId);
    signed char hairStyleIndexByte = *(signed char*)&((IFF_STRUCT::sSetItem*)setRecord)->hairStyleInventoryIndex;
    int hairStyleIndex = hairStyleIndexByte;
    if (hairStyleIndex != -1) {
        void* hairStyleTypeId = ItemInventory_GetHairStyleIdAt(AsCSharedDoc(g_sharedDoc)->itemInventory, hairStyleIndex);
        if (hairStyleTypeId != 0) {
            CModelOperationChara_SetHairStyle(self->charaModelOperation, hairStyleTypeId);
        }
    }

    // Strides in this phase: the character's equipment row is 0x34 bytes per character slot;
    // inside it equipmentSlotCursor walks 2-byte entries (short at +2 is an inventory index
    // turned into a part id by ItemInventory_GetPartIdAt); setItemCursor walks the set's
    // 4-byte item ids. The 25th read is at row+0x32: the hairStyleIndex byte and
    // its following padding byte interpreted together as a signed big-endian short.
    // It is NOT a 25th part slot or the next character's row. Why the original
    // feeds this packed halfword to GetPartIdAt is unresolved; retain the read.
    // Phase 3: compare each set item with the saved equipment indices, applying
    // only missing nonzero parts. Integer addresses retain the 32-bit ABI arithmetic.
    signed char characterIndex = self->currentCharaIndex;
    int characterRowOffset = (int)characterIndex * 0x34;
    int characterRowAddress = (int)self + characterRowOffset;
    characterEquipment = (char*)characterRowAddress + IFF_OFFSET(ShopSaveEquipment, characters);
    setItemCursor = setRecord;
    setItemIndex = 0;
    while (setItemIndex < ((IFF_STRUCT::sSetItem*)setRecord)->itemCount) {
        equipmentSlotCursor = characterEquipment;
        alreadyEquipped = 0;
        equipmentSlotIndex = 0;
        while (equipmentSlotIndex <= kOutfitCompatibilityHalfwordCount - 1) {
            unsigned int val = ItemInventory_GetPartIdAt(AsCSharedDoc(g_sharedDoc)->itemInventory, *(short*)((char*)equipmentSlotCursor + (int)&((OutfitCompatibilityCursor*)0)->inventoryIndex));
            if (((IFF_STRUCT::sSetItem*)setItemCursor)->itemIds[0] == val) {
                alreadyEquipped = 1;
                break;
            }
            equipmentSlotIndex++;
            equipmentSlotCursor = (char*)equipmentSlotCursor + 2;
        }
        if (alreadyEquipped == 0) {
            unsigned int v = ((IFF_STRUCT::sSetItem*)setItemCursor)->itemIds[0];
            if (v != 0 && self->charaModelOperation != 0) {
                void* rr = CModelOperationChara_ApplyPartChanges(self->charaModelOperation, v);
                CModelOperationChara_RequestModelUpdate(self->charaModelOperation, rr);
            }
        }
        setItemCursor = (char*)setItemCursor + 4;
        setItemIndex++;
    }

    // Phase 4: write the preview equipment back and refresh category/list indicators.
    CModelOperationChara_WriteEquipmentToSave(self->charaModelOperation);
    CPlayerStatus_RefreshEquipmentDisplay(self->playerStatus, self->categoryIndex);
    if (self->selectedListIndex != -1) {
        void* pp = ((void**)self->productLists)[self->selectedListIndex];
        CShopProductsTab_RefreshVisibleProducts(pp);
    }
    CPangya2Shop_UpdateUnownedSelectedOutfitCategoryMarkers(self);
    return 1;
}

// Preview a caddie or toggle its equipment; a zero ID selects no caddie.
int CPangya2Shop_SelectCaddieProduct(Obj* self) {
    Obj* shop = self;
    void* selectedProduct = 0;
    // Phase 1: resolve a borrowed product record through the active category.
    if (self->selectedListIndex != -1) {
        void* productTab = ((void**)self->productLists)[self->selectedListIndex];
        void* selectedProductIndex = (void*)CShopProductsTab_GetSelectedProductIndex(productTab);
        int category = self->categoryIndex;
        void* categoryProduct = 0;
        switch (category) {
            case kShopCharacter: categoryProduct = (void*)((void*(*)(void*, int, void*))ShopProductTable_GetRecordAt)(self->charaProductBuilder, self->selectedListIndex, selectedProductIndex); break;
            case kShopItem: categoryProduct = (void*)((void*(*)(void*, int, void*))ShopProductTable_GetRecordAt)(self->itemProductBuilder, self->selectedListIndex, selectedProductIndex); break;
            case kShopCaddie: categoryProduct = (void*)((void*(*)(void*, int, void*))ShopProductTable_GetRecordAt)(self->caddieProductBuilder, self->selectedListIndex, selectedProductIndex); break;
            default: break;
        }
        selectedProduct = categoryProduct;
    }
    if (selectedProduct == 0) return 0;

    // Phase 2a: equipment changes the current caddie; it does not replace it.
    void* selectedTypeId = (void*)((ShopProductRecord*)selectedProduct)->typeId;
    if (selectedTypeId != 0) {
        int itemKind = (int)((unsigned int)selectedTypeId >> kItemKindShift);
        if (itemKind == kItemCaddieEquipment) {
            void* toggleResult = 0;
            if (selectedTypeId != 0) toggleResult = (void*)CModelOperationCaddie_ToggleItem(self->caddieModelOperation, (unsigned int)selectedTypeId);
            if (toggleResult == 0) return 0;
            Sound_PlayByName(g_soundManager, (char*)shopStrings + kShopStrings_item_select_cloth, 0, 0, 0, 0, kFloat_0_5_shopSound, kFloat_200_Shop, kFloat_1_805E0918);
            return 1;
        }
        if (itemKind != kItemCaddie) return 0;

        // Phase 2b: selecting a caddie publishes the inventory index and model.
        int selectedCaddieIndex = ItemInventory_FindCaddieIndex(AsCSharedDoc(g_sharedDoc)->itemInventory, (int)selectedTypeId);
        if ((signed char)self->currentCaddieIndex == selectedCaddieIndex && self->selectedListIndex == 0) return 0;

        // Reuse the ABI word through a phase-specific reference. The selected
        // record is no longer needed after extracting its ID.
        void*& caddieTypeId = selectedProduct;
        caddieTypeId = (void*)((ShopProductRecord*)selectedProduct)->typeId;
        if (caddieTypeId != 0) {
            AsCPangya2Shop(self)->noCaddieSelected = 0;
            signed char previousCaddieIndex = self->currentCaddieIndex;
            int newCaddieIndex = ItemInventory_FindCaddieIndex(AsCSharedDoc(g_sharedDoc)->itemInventory, (int)caddieTypeId);
            self->currentCaddieIndex = (signed char)newCaddieIndex;
            void* caddieModel = self->caddieModelOperation;
            void** modelVtable = *(void***)caddieModel;
            ((void(*)(void*, void*))modelVtable[5])(caddieModel, caddieTypeId);
            // Phase 3: a changed caddie rebuilds its equipment tab, then reselects
            // the saved equipment ID. Product records and draw entries are borrowed.
            if (previousCaddieIndex != (signed char)newCaddieIndex) {
                CBuildProductCaddie_LoadIconGroups(self->caddieProductBuilder, newCaddieIndex);
                void* equipmentProductCount = CBuildProduct_GetProductCount(self->caddieProductBuilder, 1);
                CShopProductsTab_SetProductCount(((void**)self->productLists)[1], equipmentProductCount);
                void*& equipmentProducts = caddieTypeId;
                equipmentProducts = CBuildProduct_GetProductRecords(self->caddieProductBuilder, 1);
                void* equipmentDrawEntries = CBuildProduct_GetProductDrawEntries(self->caddieProductBuilder, 1);
                CShopProductsTab_SetProductArrays(((void**)self->productLists)[1], equipmentDrawEntries, equipmentProducts);
                void* equippedTypeId = ItemInventory_GetCaddieItemIdAt(AsCSharedDoc(g_sharedDoc)->itemInventory, (int)self->caddieItemIndex);
                CShopProductsTab_SelectProductByTypeId(((void**)self->productLists)[1], equippedTypeId);
            }
        }
        CPlayerStatus_RefreshEquipmentDisplay(self->playerStatus, self->categoryIndex);
        Sound_PlayByName(g_soundManager, (char*)shopStrings + kShopStrings_item_select_cloth, 0, 0, 0, 0, kFloat_0_5_shopSound, kFloat_200_Shop, kFloat_1_805E0918);
        return 1;
    }
    // Phase 2c: type ID zero is the explicit no-caddie selection.
    if (AsCPangya2Shop(self)->noCaddieSelected != 0) return 0;
    CShopProductsTab_SetProductCount(((void**)self->productLists)[1], 0);
    AsCPangya2Shop(self)->noCaddieSelected = 1;
    self->currentCaddieIndex = -1;
    self->caddieItemIndex = -1;
    CPlayerStatus_RefreshEquipmentDisplay(self->playerStatus, self->categoryIndex);
    Sound_PlayByName(g_soundManager, (char*)shopStrings + kShopStrings_item_select_cloth, 0, 0, 0, 0, kFloat_0_5_shopSound, kFloat_200_Shop, kFloat_1_805E0918);
    return 1;
}

int CPangya2Shop_PurchaseSelectedProduct(Obj* self);

// Preview the selected character, clothing, auxiliary attachment or hair style.
int CPangya2Shop_SelectCharaProduct(Obj* self) {
    Obj* shop = self;
    int selection;
    if (self->selectedListIndex == -1) {
        selection = 0;
    } else {
        void* p = ((void**)self->productLists)[self->selectedListIndex];
        int v = CShopProductsTab_GetSelectedProductIndex(p);
        int mode = self->categoryIndex;
        switch (mode) {
            case kShopCharacter: v = ShopProductTable_GetRecordAt(self->charaProductBuilder, self->selectedListIndex, v); break;
            case kShopItem: v = ShopProductTable_GetRecordAt(self->itemProductBuilder, self->selectedListIndex, v); break;
            case kShopCaddie: v = ShopProductTable_GetRecordAt(self->caddieProductBuilder, self->selectedListIndex, v); break;
            default: v = 0; break;
        }
        selection = v;
    }
    if (selection == 0) return 0;
    int itemId = (int)((ShopProductRecord*)selection)->typeId;
    int itemKind = (unsigned int)itemId >> kItemKindShift;
    if (itemKind == kItemCharacter) {
        int current = ItemInventory_FindCharacterIndex(AsCSharedDoc(g_sharedDoc)->itemInventory, itemId);
        if (self->currentCharaIndex == current) return 0;
        selection = (int)((ShopProductRecord*)selection)->typeId;
        if (selection != 0) {
            int id = ItemInventory_FindCharacterIndex(AsCSharedDoc(g_sharedDoc)->itemInventory, selection);
            self->currentCharaIndex = (signed char)id;
            void* obj = self->charaModelOperation;
            void** vt = *(void***)obj;
            ((void(*)(void*, int))vt[5])(obj, selection);
            CBuildProductChara_LoadProductTextures(self->charaProductBuilder, self->currentCharaIndex);
            int idx = 1;
            int ofs = 4;
            do {
                void* a = CBuildProduct_GetProductCount(self->charaProductBuilder, idx);
                CShopProductsTab_SetProductCount(((void**)self->productLists)[ofs / 4], a);
                void* len = CBuildProduct_GetProductRecords(self->charaProductBuilder, idx);
                void* data = CBuildProduct_GetProductDrawEntries(self->charaProductBuilder, idx);
                CShopProductsTab_SetProductArrays(((void**)self->productLists)[ofs / 4], data, len);
                if (idx == 0) {
                    int x = ItemInventory_GetCharacterIdAt(AsCSharedDoc(g_sharedDoc)->itemInventory, self->currentCharaIndex);
                    CShopProductsTab_SelectProductByTypeId(((void**)self->productLists)[0], (void*)x);
                } else {
                    selection = 0;
                    short* q = (short*)((char*)self + self->currentCharaIndex * 0x34 + 0x2e4);
                    for (; selection < kCharacterEquipmentSlots; ++selection, ++q) {
                        unsigned int x = ItemInventory_GetPartIdAt(AsCSharedDoc(g_sharedDoc)->itemInventory, q[1]);
                        if (CShopProductsTab_SelectProductByTypeId(((void**)self->productLists)[ofs / 4], (void*)x)) break;
                    }
                }
                ++idx; ofs += 4;
            } while (idx < 5);
        }
        Sound_PlayByName(g_soundManager, (char*)shopStrings + kShopStrings_item_select_cloth, 0, 0, 0, 0, kFloat_0_5_shopSound, kFloat_200_Shop, kFloat_1_805E0918);
    } else if (itemKind == kItemPart) {
        if (itemId != 0 && self->charaModelOperation != 0) {
            void* x = CModelOperationChara_ApplyPartChanges(self->charaModelOperation, itemId);
            CModelOperationChara_RequestModelUpdate(self->charaModelOperation, x);
        }
        Sound_PlayByName(g_soundManager, (char*)shopStrings + kShopStrings_item_select_cloth, 0, 0, 0, 0, kFloat_0_5_shopSound, kFloat_200_Shop, kFloat_1_805E0918);
    } else if (itemKind == kItemAuxPart) {
        CModelOperationChara_ToggleAttachment(self->charaModelOperation, 0, itemId);
        Sound_PlayByName(g_soundManager, (char*)shopStrings + kShopStrings_item_select_cloth, 0, 0, 0, 0, kFloat_0_5_shopSound, kFloat_200_Shop, kFloat_1_805E0918);
    } else if (itemKind == kItemHairStyle) {
        if (itemId != 0) {
            CModelOperationChara_SetHairStyle(self->charaModelOperation, (void*)itemId);
        }
        Sound_PlayByName(g_soundManager, (char*)shopStrings + kShopStrings_item_select_hair, 0, 0, 0, 0, kFloat_0_5_shopSound, kFloat_200_Shop, kFloat_1_805E0918);
    } else {
        return 0;
    }
    // Phase 4: write the preview equipment back and refresh category/list indicators.
    CModelOperationChara_WriteEquipmentToSave(self->charaModelOperation);
    if (itemKind != kItemHairStyle) CPlayerStatus_RefreshEquipmentDisplay(self->playerStatus, self->categoryIndex);
    return 1;
}

struct BitReference {
    unsigned int* word;
    unsigned int index;
};

struct RangeStringControl {
    void* vtable;
    int references;
    int capacity;
    char* data;
    // Adopt the range-error text in a shared character owner.
    RangeStringControl(char* p) {
        references = 1;
        capacity = 1;
        vtable = s_sharedCharDeleterVtable;
        data = p;
    }
};

struct RangeArrayOwner {
    char* data;
    char* allocator;
    // Free exception characters if ownership has not been transferred.
    ~RangeArrayOwner() { if (data) __dla__FPv(data); }
};

struct RangeErrorBase {
    void* vtable;
    char* message;
    RangeStringControl* control;
    // Destroy the exception base while retaining the enclosing storage.
    ~RangeErrorBase() { __dt__Q23std9exceptionFv(this, 0); }
};

namespace std {
struct out_of_range : RangeErrorBase {
    out_of_range();
    ~out_of_range();
};
}

// Set one ownership bit; reject indices outside the 32-bit save mask.
static inline void ApplyPurchaseBit(BitReference copy) {
    if (copy.index >= 32) {
        unsigned storage[3];
        std::out_of_range* error = (std::out_of_range*)storage;
        new (error) std::out_of_range();
        __throw((char*)shopStrings + kShopStrings_std_exception_std_logic_error_std_out_of_range, error, (void*)__dt__Q23std12out_of_rangeFv);
    }
    *copy.word |= 1U << copy.index;
}

// Pair an ownership word with the bit index to modify.
static inline BitReference MakeBitReference(unsigned int* word, unsigned int index) {
    BitReference result = {word, index};
    return result;
}

// Mark a caddie equipment inventory index as owned.
static inline void SetPurchaseBit(unsigned int* word, unsigned int index) {
    ApplyPurchaseBit(MakeBitReference(word, index));
}

// Construct the owning out-of-range exception used by purchase bit checks.
inline std::out_of_range::out_of_range() {
    vtable = __vt__Q23std11logic_error;
    char allocator = 0;
    unsigned long size = strlen(s_shopPurchaseBitRangeErrorText) + 1;
    char* data = (char*)__nwa__FUl(size * 2 - size);
    message = data;
    {
        RangeArrayOwner owner = {data, &allocator};
        control = new RangeStringControl(data);
        owner.data = 0;
    }
    strcpy(message, s_shopPurchaseBitRangeErrorText);
    vtable = __vt__Q23std12out_of_range;
}

// Compare the unsigned Pang balance against the price using the original borrow calculation.
static inline int PurchaseCanAfford(const int* funds, unsigned int cost) {
    unsigned int amount = *funds;
    unsigned int difference = amount - cost;
    cost = amount | ~cost;
    difference >>= 1;
    return (cost - difference) >> 31;
}

struct PurchaseNode {
    char padding[0x50];
    unsigned int item;
};

// Check whether this IFF type ID is already equipped in the shop save payload.
static inline int PurchaseEquipped(Obj* self, unsigned int item) {
    if (!item) return 0;
    unsigned int top = item >> kItemKindShift;
    switch (top) {
      case kItemCharacter:
        if (!item || top != kItemCharacter) return 0;
        return self->currentCharaIndex == ItemInventory_FindCharacterIndex(AsCSharedDoc(g_sharedDoc)->itemInventory, item);
      case kItemPart: {
        if (!item || top != kItemPart) return 0;
        char* q = (char*)self + self->currentCharaIndex * 0x34;
        for (int i = 0; i < kCharacterEquipmentSlots; ++i, q += 2) {
            short value = *(short*)(q + 0x2e6);
            if (value != -1 && item == ItemInventory_GetPartIdAt(AsCSharedDoc(g_sharedDoc)->itemInventory, value)) return 1;
        }
        return 0;
      }
      case kItemHairStyle: {
        if (!item || top != kItemHairStyle) return 0;
        int character = self->currentCharaIndex;
        int index = ItemInventory_FindHairStyleIndex(AsCSharedDoc(g_sharedDoc)->itemInventory, item);
        int difference = index - ((ShopSaveEquipment*)self)->characters[character].hairStyleIndex;
        return difference == 0;
      }
      case kItemBall:
        if (!item || top != kItemBall) return 0;
        return self->ballIndex == ItemInventory_FindBallIndex(AsCSharedDoc(g_sharedDoc)->itemInventory, (void*)item);
      case kItemClubSet:
        if (!item || top != kItemClubSet) return 0;
        return self->ballIndex == ItemInventory_FindClubSetIndex(AsCSharedDoc(g_sharedDoc)->itemInventory, (void*)item);
      case kItemCaddie:
        if (!item || top != kItemCaddie) return 0;
        return self->currentCaddieIndex == ((int(*)(void*, int))ItemInventory_FindCaddieIndex)(AsCSharedDoc(g_sharedDoc)->itemInventory, item);
      case kItemCaddieEquipment:
        if (!item || top != kItemCaddieEquipment) return 0;
        if (self->currentCaddieIndex == (int)((item >> 21) & 31) && self->caddieItemIndex == ItemInventory_FindCaddieItemIndex(AsCSharedDoc(g_sharedDoc)->itemInventory, (void*)item)) return 1;
        return 0;
      default:
        return 0;
    }
}

// Validate Pang, grant the selected product, charge its price and refresh the shop UI.
// Decision table by category (self->categoryIndex):
//   character : CPangya2Shop_PurchaseCharacterOrOutfitItem grants + charges; 0 -> fail
//   item      : CPangya2Shop_ApplyNonOutfitPurchase grants + charges; 0 -> fail
//   caddie    : here: caddie kind sets the caddie's owned flag, caddie-equipment kind sets
//               its owned bit; any other kind is not granted and not charged (returns 0);
//               on success the price is subtracted from pangBalance
//   other     : return 0, nothing changes
// After success (all paths): equip/refresh the category's selection (unless PurchaseEquipped
// handled it), play the shop_cart_buy sound, update the Pang text, product message and
// purchase button.
// Transaction order: resolve -> affordability gate -> category-specific grant/charge
// -> optional equip -> UI refresh. Failure returns 0; successful purchase returns 1.
// Character/item helpers own their charges. The caddie branch charges locally;
// its second affordability check and second price query are existing behavior.
int CPangya2Shop_PurchaseSelectedProduct(Obj* self) {
    unsigned int item;
    if (self->selectedListIndex == -1) return 0;
    void* selectedProduct;
    if (self->selectedListIndex == -1) {
        selectedProduct = 0;
    } else {
        void* productIndex = (void*)CShopProductsTab_GetSelectedProductIndex(((void**)self->productLists)[self->selectedListIndex]);
        void* result = 0;
        switch (self->categoryIndex) {
            case kShopCharacter: result = ((void*(*)(void*, int, void*))ShopProductTable_GetRecordAt)(self->charaProductBuilder, self->selectedListIndex, productIndex); break;
            case kShopItem: result = ((void*(*)(void*, int, void*))ShopProductTable_GetRecordAt)(self->itemProductBuilder, self->selectedListIndex, productIndex); break;
            case kShopCaddie: result = ((void*(*)(void*, int, void*))ShopProductTable_GetRecordAt)(self->caddieProductBuilder, self->selectedListIndex, productIndex); break;
        }
        selectedProduct = result;
    }
    if (selectedProduct == 0) return 0;

    // ShopProductTable_GetRecordAt product result: original item type ID +0x50.
    int pricePang = (int)ItemInventory_GetPricePang(AsCSharedDoc(g_sharedDoc)->itemInventory, *(void**)&((ShopProductRecord*)selectedProduct)->typeId);
    if (!PurchaseCanAfford(&(*(int*)&self->pangBalance), pricePang)) {
        self->clubDialogOpen = 1;
        CClubDialog_SetMode(self->clubDialog, kInsufficientPangDialog);
        return 0;
    }

    // Mutation boundary: everything above only queries data or opens an error
    // dialog. The two delegated helpers below can permanently grant AND charge;
    // there is no enclosing rollback if later UI/equip work fails.
    switch (self->categoryIndex) {
      case kShopCharacter: {
        int itemId = (int)((ShopProductRecord*)selectedProduct)->typeId;
        if (CPangya2Shop_PurchaseCharacterOrOutfitItem(self, itemId) == 0) return 0;
        break;
      }
      case kShopItem: {
        int itemId = (int)((ShopProductRecord*)selectedProduct)->typeId;
        if (CPangya2Shop_ApplyNonOutfitPurchase(self, itemId) == 0) return 0;
        break;
      }
      case kShopCaddie: {
        int caddieItemId = (int)((ShopProductRecord*)selectedProduct)->typeId;
        int pricePang = (int)ItemInventory_GetPricePang(AsCSharedDoc(g_sharedDoc)->itemInventory, (void*)caddieItemId);
        int success;
        if (!PurchaseCanAfford(&(*(int*)&self->pangBalance), pricePang)) {
            self->clubDialogOpen = 1;
            CClubDialog_SetMode(self->clubDialog, kInsufficientPangDialog);
            success = 0;
        } else {
                // Original 0x802C7308: unsupported products skip the charge.
                do {
                switch ((unsigned int)caddieItemId >> kItemKindShift) {
                  case kItemCaddie: {
                    int idx = ((int(*)(void*, int))ItemInventory_FindCaddieIndex)(AsCSharedDoc(g_sharedDoc)->itemInventory, caddieItemId);
                    unsigned char* ownershipFlags = &((ShopSaveEquipment*)self)->caddies[idx].flags;
                    *ownershipFlags |= kCaddieOwnedFlag; // First permanent inventory write on this path.
                    break;
                  }
                  case kItemCaddieEquipment: {
                    unsigned int idx = ItemInventory_FindCaddieItemIndex(AsCSharedDoc(g_sharedDoc)->itemInventory, (void*)caddieItemId);
                    SetPurchaseBit(&((ShopSaveEquipment*)self)->ownedCaddieEquipment, idx); // First permanent write.
                    break;
                  }
                  default:
                    success = 0;
                    continue;
                }
                // Ownership is already permanent. Requerying price and charging
                // follow that write; this is not an atomic inventory transaction.
                pricePang = (int)ItemInventory_GetPricePang(AsCSharedDoc(g_sharedDoc)->itemInventory, (void*)caddieItemId);
                (*(int*)&self->pangBalance) = (*(int*)&self->pangBalance) - pricePang;
                success = 1;
                } while (0);
        }
        if (!success) return 0;
        break;
      }
      default:
        return 0;
    }

    // Purchase succeeded: optionally equip the item before refreshing the UI.
    item = ((ShopProductRecord*)selectedProduct)->typeId;
    if (!PurchaseEquipped(self, item)) {
        if (self->selectedListIndex != -1) {
            int mode = self->categoryIndex;
            switch (mode) {
              case kShopCharacter: CPangya2Shop_SelectCharaProduct(self); CPangya2Shop_UpdateUnownedSelectedOutfitCategoryMarkers(self); break;
              case kShopItem: CPangya2Shop_SelectItemProduct(self); break;
              case kShopCaddie: CPangya2Shop_SelectCaddieProduct(self); CPangya2Shop_UpdateControlScriptSelection(self); break;
            }
            if (self->selectedListIndex != -1) CShopProductsTab_RefreshVisibleProducts(((void**)self->productLists)[self->selectedListIndex]);
        }
    }

    Sound_PlayByName(g_soundManager, (char*)shopStrings + kShopStrings_shop_cart_buy, 0, 0, 0, 0, kFloat_0_5_shopSound, kFloat_200_Shop, kFloat_1_805E0918);
    if (self->selectedListIndex != -1) CShopProductsTab_RefreshVisibleProducts(((void**)self->productLists)[self->selectedListIndex]);

    UITextTable_SetInteger(self->textTable, kPangText, (*(int*)&self->pangBalance));
    CClubHouseMsg_ResetTextState(self->productMessage);

    struct Frame {
        int itemId;
        char shortDescription[0xa];
        char description[0x28];
        char detail[0x28];
    } productInfo;
    productInfo.itemId = 0;
    Memory_Clear(productInfo.shortDescription, 0xa);
    Memory_Clear(productInfo.description, 0x28);
    Memory_Clear(productInfo.detail, 0x28);

    if (CPangya2Shop_GetSelectedProductInfo(self, &productInfo.itemId)) {
        void* r = UIGroup_GetOrCreateById(self->uiGroup, kProductMessageGroup);
        CClubHouseMsg_SetTextFromGroup(self->productMessage, productInfo.description, r, 0);
    }

    CPangya2Shop_RefreshSelectedProductValues(self);
    CPangya2Shop_UpdateSelectedPurchaseAmount(self);
    int (*refresh)(Obj*) = CPangya2Shop_UpdatePurchaseButton;
    refresh(self);
    return 1;
}

} // extern "C"

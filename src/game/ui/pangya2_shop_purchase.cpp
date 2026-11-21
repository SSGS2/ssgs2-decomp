#include "classes/CSharedDoc.h"
#define AsCSharedDoc(p) ((CSharedDoc*)(p))
#include "classes/CPangya2Shop.h"
#include "classes/ShopProductRecord.h"
#define AsCPangya2Shop(p) ((CPangya2Shop*)(p))

#include "game/vector.h"
// Translation unit 0x802C874C-0x802C9D94 (6 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

struct ShopStringPool { char clubhouse_zip[14]; char club_base_spd[14]; char club_base_b2d[14]; char club_setcos_info_b2d[21]; char mSG_CADDESC_UNSET[18]; char base_def[9]; char setcos_def[11]; char num_shop_00_gsr[16]; char num_status_01_gsr[18]; char ui_page_change[15]; char ui_cursol_move[15]; char ui_error[9]; char item_select_cloth[18]; char item_select_hair[17]; char item_select_club[17]; char item_select_aztec[18]; char std_exception_std_logic_error_std_out_of_range[55]; char shop_cart_buy[14]; char vector_length_error[20]; char std_exception_std_logic_error_std_length_error[55]; unsigned char reserved_184[4]; };

enum ShopPurchaseConstants {
    kItemKindShift = 26, kCharacterKind = 1, kPartKind = 2, kClubSetKind = 4,
    kBallKind = 5, kConsumableKind = 6, kSetKind = 9, kHairStyleKind = 15, kAuxPartKind = 28,
    kOwnedItemFlag = 0x40, kPartCount = 782, kHairStyleCount = 51,
    kOutfitPartCount = 24, kPurchasePreviewSlots = 9, kDialogInsufficientPang = 5,
    kCategoryCharacter = 0, kCategoryItem = 1, kCategoryCaddie = 2,
    kSubMenuSelect = 1, kSubMenuLeave = 2, kSubMenuApplySet = 3, kSubMenuUpgradeClub = 4,
    kClubResultDismiss = 1, kClubResultLeave = 2, kClubResultBuySelected = 3,
    kClubResultBuyOutfit = 4, kClubResultConfirmLeave = 5, kClubResultRestoreOutfit = 6,
    kDialogConfirmLeave = 4, kPurchaseResultCancel = 0, kPurchaseResultConfirm = 1
};
enum ShopUiElement { kOutfitSetHelpGroup = 0x42A };
enum ShopVectorLimits { kMaxShopVectorWords = 0x3FFFFFFF,
    kSmallShopVectorLimit = 0x15555555, kMediumShopVectorLimit = 0x2AAAAAAA };

// Per-character preview: saved inventory indices, not IFF type ids. A value of -1 is an empty slot.
struct ShopOutfitSelection {
    unsigned char ownershipFlags;
    unsigned char reserved;
    short partIndices[kOutfitPartCount];
    unsigned char hairSelection;
    signed char auxPartIndex;
};
typedef char ShopOutfitSelection_size[(sizeof(ShopOutfitSelection) == 0x34) ? 1 : -1];

extern "C" {

struct ObjC874C {
    char base[0x40];
    int pangBalance;
    char selections[0x10];
    unsigned int selectedPartBits[25]; // +0x54: checked preview parts
    char reservedB8[0xC];
    unsigned int ownedPartBits[25]; // +0xC4: purchased clothing
    char reserved128[4];
    unsigned int ownedHairBits[2]; // +0x12C
    short consumableCounts[13]; // +0x134
    unsigned char auxPartOwned[0x196]; // +0x14E: extent up to the character preview array
    ShopOutfitSelection outfits[8]; // +0x2E4
    char reserved484[0x40];
    unsigned char clubSetStatus[15][2]; // +0x4C4: ownership flag is in the first byte
    unsigned char ballStatus[30]; // +0x4E2
    char reserved500[0x398];
    void* itemCart; // +0x898
    char reserved89C[0x14];
    void* clubDialog;
    unsigned char clubDialogOpen;
    char reserved8B5[3];
    void* purchaseDialog;
    unsigned char purchaseDialogOpen;
};
#define SHOP_PURCHASE_OFF(field, offset) typedef char ShopPurchase_off_##field[(IFF_OFFSET(ObjC874C, field) == offset) ? 1 : -1]
SHOP_PURCHASE_OFF(pangBalance, 0x40);
SHOP_PURCHASE_OFF(selectedPartBits, 0x54);
SHOP_PURCHASE_OFF(ownedPartBits, 0xC4);
SHOP_PURCHASE_OFF(ownedHairBits, 0x12C);
SHOP_PURCHASE_OFF(consumableCounts, 0x134);
SHOP_PURCHASE_OFF(auxPartOwned, 0x14E);
SHOP_PURCHASE_OFF(outfits, 0x2E4);
SHOP_PURCHASE_OFF(clubSetStatus, 0x4C4);
SHOP_PURCHASE_OFF(ballStatus, 0x4E2);
SHOP_PURCHASE_OFF(itemCart, 0x898);
SHOP_PURCHASE_OFF(clubDialog, 0x8B0);
SHOP_PURCHASE_OFF(purchaseDialog, 0x8B8);
#undef SHOP_PURCHASE_OFF
extern void* g_sharedDoc; extern void* g_soundManager; extern const ShopStringPool shopStrings; extern const char bitsetSetOutOfRangeMessage[]; extern const char hairStyleBitsetSetOutOfRangeMessage[]; extern const char bitsetTestOutOfRangeMessage[];
extern char __vt__Q23std11logic_error[], __vt__Q23std12out_of_range[], __vt__Q23std12length_error[], s_sharedCharDeleterVtable[];
extern void __dt__Q23std12out_of_rangeFv(void*); extern void __dt__Q23std12length_errorFv(void*);
extern void* memcpy(void*, const void*, unsigned int); extern void* memset(void*, int, unsigned int);
extern unsigned int strlen(const char*);
extern char* strcpy(char*, const char*);
extern void* __nwa__FUl(unsigned int);
extern void* __nw__FUl(unsigned int);
extern void __dla__FPv(void*);
extern "C" void __dt__Q23std9exceptionFv(void*);
extern void __dt__Q23std12out_of_rangeFv(void*);
extern void __throw(const char*, void*, void*) __attribute__((noreturn));
extern float kFloat_1_805E0918, kFloat_0_5_shopSound, kFloat_200_Shop;
int ItemInventory_GetPricePang(void*, int); int ItemInventory_FindClubSetIndex(void*, int); int ItemInventory_FindBallIndex(void*, int); int ItemInventory_FindItemIndex(void*, int); int ItemInventory_FindPartIndex(void*, int);
int ItemInventory_FindCharacterIndex(void*, int); int ItemInventory_FindAuxPartIndex(void*, int); int ItemInventory_FindHairStyleIndex(void*, int);
void CClubDialog_SetMode(void*, int); void CItemCart_AddItem(void*, int); int CClubDialog_TryGetResult(void*, int*); void CPangya2Shop_PurchaseSelectedProduct(ObjC874C*); void CPangya2Shop_UpdateUnownedSelectedOutfitCategoryMarkers(ObjC874C*);
  int CPangya2Shop_CollectUnownedSelectedOutfitItems(ObjC874C*, void*); int CPangya2Shop_SumUnownedItemValues(ObjC874C*, void*); int CPangya2Shop_OpenOutfitPurchaseDialog(ObjC874C*); int CClubDialog_Update(void*);
int CClubSubMenu_IsSecondaryActionSelected(void*); void CClubHouseMsg_ResetTextState(void*); void* UIGroup_GetOrCreateById(void*, int); int CShopProductsTab_GetSelectedProductIndex(void*); void* ShopProductTable_GetRecordAt(void*, int, void*); unsigned int CBuildProductChara_FindSetContainingItem(void*, unsigned int); int ItemInventory_FindSetItemData(void*, unsigned int); void strncpy(void*, void*, int); void CClubHouseMsg_SetTextFromGroup(void*, void*, void*, int); void BpObject_SetEnabled(void*, int); int CClubSubMenu_GetSelectionResult(void*, int*); void CPangya2Shop_SelectCharaProduct(ObjC874C*); void CPangya2Shop_SelectItemProduct(ObjC874C*); void CPangya2Shop_SelectCaddieProduct(ObjC874C*); void CPangya2Shop_UpdateControlScriptSelection(ObjC874C*); void CPangya2Shop_ApplySelectedSet(ObjC874C*); void Memory_Clear(void*, int); int CPangya2Shop_GetSelectedProductInfo(ObjC874C*, int*); int CClubLvUpDialog_SetClubIndex(void*, int); void CClubLvUpDialog_RefreshStatsAndMessage(void*, int*); void CClubLvUpDialog_Show(void*); void* CShopProductsTab_RefreshVisibleProducts(void*); void* CBuildProduct_GetTextureEntryAt(void*, int, void*); void CClubLvUpDialog_SetClubIcon(void*, void*); void CClubSubMenu_Update(void*);
int CPurchaseDialog_TryCloseWithResult(void*, int*); void* WordVector_GetElementAddress(void*, unsigned int); void* SharedDoc_GetItemInventory(); int CPangya2Shop_IsItemOwned(ObjC874C*, void*); int CPangya2Shop_PurchaseCharacterOrOutfitItem(ObjC874C*, unsigned int); unsigned int Vector_GetCount(void*);
void Sound_PlayByName(void*, const char*, int, int, int, int, float, float, float); void CPangya2Shop_RefreshSelectedProductList(ObjC874C*); void CPangya2Shop_UpdatePurchaseButton(ObjC874C*); void CPangya2Shop_RefreshProductDetails(ObjC874C*); void CPangya2Shop_UpdateMoneyDisplay(ObjC874C*); int CPurchaseDialog_Update(void*); void CPangya2Shop_RestoreSavedOutfit(ObjC874C*);

} // extern "C"

struct BitAllocation {
    char* data;
    char* allocator;
    // Delete exception text until its control block has adopted the allocation.
    ~BitAllocation() { if (data) delete[] data; }
};
struct BitControl {
    void* vtable;
    int refs, owners;
    char* data;
    // Adopt exception text with its initial reference counts and character deleter.
    BitControl(char* p) { refs = 1; owners = 1; vtable = s_sharedCharDeleterVtable; data = p; }
};
namespace std {
struct exception {
    void* vtable;
};
struct logic_error : exception {
    char* data;
    BitControl* control;
    // Allocate/copy a NUL-terminated exception message under an unwind guard.
    logic_error(const char* message) {
        vtable = __vt__Q23std11logic_error;
        char allocator[4];
        allocator[0] = 0;
        unsigned int n = strlen(message) + 1;
        data = (char*)__nwa__FUl((n << 1) - n);
        {
            char* buffer = data;
            BitAllocation allocation = { buffer, allocator };
            control = new BitControl(buffer);
            allocation.data = 0;
        }
        strcpy(data, message);
    }
    ~logic_error();
};
struct out_of_range : logic_error {
    // Select the range-error runtime type after copying its message.
    out_of_range(const char* message) : logic_error(message) { vtable = __vt__Q23std12out_of_range; }
    ~out_of_range();
};
struct length_error : logic_error {
    // Construct the shop vector's length-error message and runtime type.
    length_error() : logic_error(shopStrings.vector_length_error) { vtable = __vt__Q23std12length_error; }
    ~length_error();
};
}

struct RangeError {
    void* table;
    char* text;
    BitControl* owner;
    // Release the range exception through the runtime base destructor.
    ~RangeError() { __dt__Q23std9exceptionFv(this); }
};
struct RangeGuard {
    char* text;
    char* allocator;
    // Free unadopted range-error message storage during unwind.
    ~RangeGuard() { if (text) __dla__FPv(text); }
};

struct BitRef30E {
    unsigned int* words;
    unsigned int index;
    // Borrow the clothing ownership bitset at a checked-later index.
    BitRef30E(unsigned int* p, unsigned int i) { words = p; index = i; }
    // Mark the clothing item owned, throwing when its index exceeds the bitset.
    void set() const {
        unsigned int i = index;
        if (i >= kPartCount) throw std::out_of_range(bitsetSetOutOfRangeMessage);
        unsigned int mask = 1u << (i & 31);
        words[i >> 5] |= mask;
    }
};
// Build a borrowed bit reference for clothing ownership.
inline BitRef30E ref30E(unsigned int* p, unsigned int i) { BitRef30E view(p, i); return view; }

struct BitRef33 {
    unsigned int* words;
    unsigned int index;
    // Borrow the hair ownership bitset at a checked-later index.
    BitRef33(unsigned int* p, unsigned int i) { words = p; index = i; }
    // Mark the hairstyle owned, throwing when its index exceeds the bitset.
    void set() const {
        unsigned int i = index;
        if (i >= kHairStyleCount) throw std::out_of_range(hairStyleBitsetSetOutOfRangeMessage);
        unsigned int mask = 1u << (i & 31);
        words[i >> 5] |= mask;
    }
};
// Build a borrowed bit reference for hairstyle ownership.
inline BitRef33 ref33(unsigned int* p, unsigned int i) { BitRef33 view(p, i); return view; }

#pragma does_not_return __throw

extern "C" {

#pragma opt_propagation off
// Check Pang, mark a club set or ball owned (or add a consumable), then charge its IFF price.
int CPangya2Shop_ApplyNonOutfitPurchase(ObjC874C* self, unsigned int typeId) {
    // Contract: typeId is an IFF ID, not an inventory index. Lookups must yield
    // valid indices. Return 1 commits ownership/count and a Pang deduction;
    // return 0 means insufficient balance (opens the error modal) or unknown kind.
    // No duplicate-ownership check is performed here. Price is queried twice.
    // Phase 1: reject insufficient balance before changing inventory.
    int pricePang = ItemInventory_GetPricePang(AsCSharedDoc(g_sharedDoc)->itemInventory, (int)typeId);
    // This unsigned identity implements signed balance < price, including
    // sentinel prices; equality permits purchase. Direct '<' and an extracted
    // inline helper were tested and changed code/relocations, so retain the form.
    unsigned halfDifference = ((unsigned)self->pangBalance - (unsigned)pricePang) >> 1;
    unsigned comparisonBits = (unsigned)self->pangBalance | ~(unsigned)pricePang;
    if (((comparisonBits - halfDifference) >> 31) == 0) {
        self->clubDialogOpen = 1;
        CClubDialog_SetMode(self->clubDialog, kDialogInsufficientPang);
        return 0;
    }
    // Phase 2: mutate the supported inventory category; unsupported kinds return
    // before charging. A cart/allocation exception is not rolled back here.
    switch (typeId >> kItemKindShift) {
    case kClubSetKind: {
        int i = ItemInventory_FindClubSetIndex(AsCSharedDoc(g_sharedDoc)->itemInventory, typeId);
        unsigned char* p = &self->clubSetStatus[i][0];
        *p |= kOwnedItemFlag;
        break;
    }
    case kBallKind: {
        int i = ItemInventory_FindBallIndex(AsCSharedDoc(g_sharedDoc)->itemInventory, typeId);
// The record holds 11 ball bytes; a larger index would write into the password/option bytes.
        unsigned char* p = &self->ballStatus[i];
        *p |= kOwnedItemFlag;
        break;
    }
    case kConsumableKind: {
        int i = ItemInventory_FindItemIndex(AsCSharedDoc(g_sharedDoc)->itemInventory, typeId);
        short* p = &self->consumableCounts[i];
        ++*p;
        CItemCart_AddItem(self->itemCart, typeId);
        break;
    }
    default: return 0;
    }
    // Phase 3: charge only after the category mutation completed successfully.
    self->pangBalance -= ((int(*)(void*, int))ItemInventory_GetPricePang)(AsCSharedDoc(g_sharedDoc)->itemInventory, typeId);
    return 1;
}
#pragma opt_propagation reset

struct DialogData {
    int value;
    char text[0xA];
    char item[0x28];
    char label[0x28];
};

// Dispatch preview, leave, outfit-set and club-upgrade actions, updating the corresponding dialogs.
int CPangya2Shop_ProcessClubSubMenu(ObjC874C* self) {
    // Phase 1: refresh contextual outfit-set help. The repeated category lookups
    // select different builder tables; returned records and UI groups are borrowed.
    // The secondary action requires a selected character product when help exists.
    // CPangya2Shop: category +0x840, selection +0x844, product lists +0x85c;
    // dialog/control pointers and state bytes occupy +0x86c..+0x8d4 (header checks each offset).
    CPangya2Shop* p = AsCPangya2Shop(self);
    // layout view of the same object
    int state;
// strncpy(name, ..., 0x28) does not terminate a full-length name, and the message lookup reads it as a C string.
    char name[0x28];
    DialogData data;
    if (CClubSubMenu_IsSecondaryActionSelected(p->clubSubMenu)) {
        CClubHouseMsg_ResetTextState(p->subMenuMessage);
        if (p->categoryIndex == kCategoryCharacter) {
            void* found = UIGroup_GetOrCreateById(p->scriptObject, kOutfitSetHelpGroup);
            if (found != 0) {
                void* obj = 0;
                int ok;
                if (p->selectedListIndex == -1) {
                    obj = 0;
                }
                else {
                    void* bff = (void*)CShopProductsTab_GetSelectedProductIndex(((void**)p->productLists)[p->selectedListIndex]);
                    obj = 0;
                    switch (p->categoryIndex) {
                        case kCategoryCharacter:
                        obj = ShopProductTable_GetRecordAt(p->charaProductBuilder, p->selectedListIndex, bff);
                        break;
                        case kCategoryItem:
                        obj = ShopProductTable_GetRecordAt(p->itemProductBuilder, p->selectedListIndex, bff);
                        break;
                        case kCategoryCaddie:
                        obj = ShopProductTable_GetRecordAt(p->caddieProductBuilder, p->selectedListIndex, bff);
                        break;
                    }
                }
                if ((((ShopProductRecord*)obj)->typeId >> kItemKindShift) != kPartKind) {
                    // selected product +0x50
                    ok = 0;
                }
                else {
                    unsigned int id = CBuildProductChara_FindSetContainingItem(p->charaProductBuilder, ((ShopProductRecord*)obj)->typeId);
                    if ((id >> kItemKindShift) != kSetKind) {
                        ok = 0;
                    }
                    else {
                        strncpy(name, (void*)ItemInventory_FindSetItemData(AsCSharedDoc(g_sharedDoc)->itemInventory, id), 0x28);
                        ok = 1;
                    }
                }
                if (ok != 0) {
                    CClubHouseMsg_SetTextFromGroup(p->subMenuMessage, name, found, 1);
                    BpObject_SetEnabled(p->scriptObject, 1);
                }
            }
        }
    }
    else {
        CClubHouseMsg_ResetTextState(p->subMenuMessage);
        BpObject_SetEnabled(p->scriptObject, 0);
    }
    // Phase 2: consume one submenu result. Upgrade opens its own modal; selection
    // updates preview/list state. A pending submenu is updated at the bottom.
    if (CClubSubMenu_GetSelectionResult(p->clubSubMenu, &state) != 0) {
        switch (state) {
            case 0:
            break;
            case kSubMenuSelect:
            if (p->selectedListIndex != -1) {
                switch (p->categoryIndex) {
                    case kCategoryCharacter:
                    CPangya2Shop_SelectCharaProduct(self);
                    CPangya2Shop_UpdateUnownedSelectedOutfitCategoryMarkers(self);
                    break;
                    case kCategoryItem:
                    CPangya2Shop_SelectItemProduct(self);
                    break;
                    case kCategoryCaddie:
                    CPangya2Shop_SelectCaddieProduct(self);
                    CPangya2Shop_UpdateControlScriptSelection(self);
                    break;
                }
                if (p->selectedListIndex != -1) {
                    CShopProductsTab_RefreshVisibleProducts(((void**)p->productLists)[p->selectedListIndex]);
                }
            }
            break;
            case kSubMenuLeave:
            CClubDialog_SetMode(p->clubDialog, 1);
            p->clubDialogOpen = 1;
            break;
            case kSubMenuApplySet:
            CPangya2Shop_ApplySelectedSet(self);
            Sound_PlayByName(g_soundManager, shopStrings.item_select_cloth, 0, 0, 0, 0, kFloat_0_5_shopSound, kFloat_200_Shop, kFloat_1_805E0918);
            break;
            case kSubMenuUpgradeClub:
            {
                data.value = 0;
                Memory_Clear(data.text, 0xA);
                Memory_Clear(data.item, 0x28);
                Memory_Clear(data.label, 0x28);
                if (CPangya2Shop_GetSelectedProductInfo(self, &data.value)) {
                    void* obj = 0;
                    if (p->selectedListIndex == -1) {
                        obj = 0;
                    }
                    else {
                        void* bff = (void*)CShopProductsTab_GetSelectedProductIndex(((void**)p->productLists)[p->selectedListIndex]);
                        obj = 0;
                        switch (p->categoryIndex) {
                            case kCategoryCharacter:
                            obj = ShopProductTable_GetRecordAt(p->charaProductBuilder, p->selectedListIndex, bff);
                            break;
                            case kCategoryItem:
                            obj = ShopProductTable_GetRecordAt(p->itemProductBuilder, p->selectedListIndex, bff);
                            break;
                            case kCategoryCaddie:
                            obj = ShopProductTable_GetRecordAt(p->caddieProductBuilder, p->selectedListIndex, bff);
                            break;
                        }
                    }
                    if (obj != 0) {
                        int v = ItemInventory_FindClubSetIndex(AsCSharedDoc(g_sharedDoc)->itemInventory, ((ShopProductRecord*)obj)->typeId);
                        CClubLvUpDialog_SetClubIndex(p->clubLvUpDialog, v);
                        CClubLvUpDialog_RefreshStatsAndMessage(p->clubLvUpDialog, &data.value);
                    }
                }
                CClubLvUpDialog_Show(p->clubLvUpDialog);
                CClubLvUpDialog_SetClubIcon(p->clubLvUpDialog, CBuildProduct_GetTextureEntryAt(p->itemProductBuilder, p->selectedListIndex, (void*)CShopProductsTab_GetSelectedProductIndex(((void**)p->productLists)[p->selectedListIndex])));
                p->clubLvUpDialogOpen = 1;
                break;
            }
        }
        // Phase 3: close contextual help after any completed submenu action.
        p->clubSubMenuOpen = 0;
        CClubHouseMsg_ResetTextState(p->subMenuMessage);
        BpObject_SetEnabled(p->scriptObject, 0);
        return 1;
    }
    CClubSubMenu_Update(p->clubSubMenu);
    return 1;
}

#pragma use_lmw_stmw on
// Check Pang, record character/clothing/hair/ring ownership, then subtract the IFF price.
int CPangya2Shop_PurchaseCharacterOrOutfitItem(ObjC874C* self, unsigned int typeId) {
    // Same check -> ownership mutation -> second price query/charge contract as
    // ApplyNonOutfitPurchase. Equality succeeds; unsupported kinds return zero.
    // The same unsigned identity below implements signed balance < price;
    // equality permits purchase. Keep the local snapshot for matching.
    int pricePang = ItemInventory_GetPricePang(AsCSharedDoc(g_sharedDoc)->itemInventory, typeId);
    unsigned int balanceSnapshot = (unsigned)self->pangBalance;
    unsigned halfDifference = (balanceSnapshot - (unsigned)pricePang) >> 1;
    unsigned comparisonBits = balanceSnapshot | ~(unsigned)pricePang;
    if (((comparisonBits - halfDifference) >> 31) == 0) {
        self->clubDialogOpen = 1;
        CClubDialog_SetMode(self->clubDialog, kDialogInsufficientPang);
        return 0;
    }
    switch (typeId >> kItemKindShift) {
    case kCharacterKind: {
        int i = ItemInventory_FindCharacterIndex(AsCSharedDoc(g_sharedDoc)->itemInventory, typeId);
        unsigned char* p = &self->outfits[i].ownershipFlags;
        *p |= kOwnedItemFlag;
        break;
    }
    case kPartKind: {
        unsigned int i = (unsigned int)ItemInventory_FindPartIndex(AsCSharedDoc(g_sharedDoc)->itemInventory, typeId);
        ref30E(self->ownedPartBits, i).set();
        break;
    }
    case kAuxPartKind: {
        int i = ItemInventory_FindAuxPartIndex(AsCSharedDoc(g_sharedDoc)->itemInventory, typeId);
        self->auxPartOwned[i] = 1;
        break;
    }
    case kHairStyleKind: {
        unsigned int i = (unsigned int)ItemInventory_FindHairStyleIndex(AsCSharedDoc(g_sharedDoc)->itemInventory, typeId);
        ref33(self->ownedHairBits, i).set();
        break;
    }
    default: return 0;
    }
    self->pangBalance -= ItemInventory_GetPricePang(AsCSharedDoc(g_sharedDoc)->itemInventory, typeId);
    return 1;
}
#pragma use_lmw_stmw off

// Handle leave, purchase, outfit purchase and restore-preview results from the club dialog.
int CPangya2Shop_ProcessClubDialogResult(ObjC874C* self) {
    int result;
    if (!CClubDialog_TryGetResult(self->clubDialog, &result)) {
        CClubDialog_Update(self->clubDialog);
        return 0;
    }
    switch (result) {
    case kClubResultDismiss:
        self->clubDialogOpen = 0;
        break;
    case kClubResultLeave:
        self->clubDialogOpen = 0;
        return 1;
    case kClubResultBuySelected:
        self->clubDialogOpen = 0;
        CPangya2Shop_PurchaseSelectedProduct(self);
        CPangya2Shop_UpdateUnownedSelectedOutfitCategoryMarkers(self);
        break;
    case kClubResultBuyOutfit: {
        IntVector outfitItems;
        CPangya2Shop_CollectUnownedSelectedOutfitItems(self, &outfitItems);
        if (self->pangBalance < CPangya2Shop_SumUnownedItemValues(self, &outfitItems))
            CClubDialog_SetMode(self->clubDialog, kDialogInsufficientPang);
        else {
            self->clubDialogOpen = 0;
            self->purchaseDialogOpen = 1;
            CPangya2Shop_OpenOutfitPurchaseDialog(self);
        }
        break;
    }
    case kClubResultConfirmLeave:
        CClubDialog_SetMode(self->clubDialog, kDialogConfirmLeave);
        break;
    case kClubResultRestoreOutfit:
        CPangya2Shop_RestoreSavedOutfit(self);
        self->clubDialogOpen = 0;
        break;
    }
    return 0;
}

// Buy each unowned outfit item in order; stop on failure and refresh shop displays on completion.
int CPangya2Shop_ProcessOutfitPurchaseResult(ObjC874C* self) {
    // This is a sequence of independent purchases, NOT an atomic outfit purchase.
    // On a later failure, earlier ownership changes/charges stay committed; the
    // purchase dialog is already closed and final display refreshes are skipped.
    int result;
    if (!CPurchaseDialog_TryCloseWithResult(self->purchaseDialog, &result)) {
        CPurchaseDialog_Update(self->purchaseDialog);
        return 1;
    }
    if (result == kPurchaseResultCancel) self->purchaseDialogOpen = 0;
    else if (result == kPurchaseResultConfirm) {
        self->purchaseDialogOpen = 0;
        IntVector outfitItems;
        CPangya2Shop_CollectUnownedSelectedOutfitItems(self, &outfitItems);
        // Buy in collection order, rechecking ownership and price for each item.
        for (unsigned int i = 0; i < Vector_GetCount(&outfitItems); ++i) {
            void* itemId = WordVector_GetElementAddress(&outfitItems, i);
            ItemInventory_FindPartIndex(SharedDoc_GetItemInventory(), (int)*(unsigned int*)itemId);
            itemId = WordVector_GetElementAddress(&outfitItems, i);
            if (!CPangya2Shop_IsItemOwned(self, *(void**)itemId)) {
                itemId = WordVector_GetElementAddress(&outfitItems, i);
                if (!CPangya2Shop_PurchaseCharacterOrOutfitItem(self, (unsigned int)*(void**)itemId)) {
                    return 0;
                }
            }
        }
        // Only full completion reaches success feedback and the money/list refresh.
        Sound_PlayByName(g_soundManager, shopStrings.shop_cart_buy, 0, 0, 0, 0, kFloat_0_5_shopSound, kFloat_200_Shop, kFloat_1_805E0918);
        CPangya2Shop_RefreshSelectedProductList(self);
        CPangya2Shop_UpdateUnownedSelectedOutfitCategoryMarkers(self);
        CPangya2Shop_UpdatePurchaseButton(self);
        CPangya2Shop_RefreshProductDetails(self);
        CPangya2Shop_UpdateMoneyDisplay(self);
    }
    return 1;
}

extern void* ItemInventory_GetPartIdAt(void*, int);
extern void* ItemInventory_GetAuxPartIdAt(void*, int);
extern void* appMallocDummy__FUl(unsigned int);
extern void appFreeUni__FPv(void*);
extern void* CBuildProductChara_FindResourceByTypeId(void*, void*);
extern void CPurchaseDialog_SetSlotDrawable(void*, int, int);
extern void CPurchaseDialog_SetPriceBreakdown(void*, int, int);
extern void CPurchaseDialog_Show(void*);
extern void* __nwa__FUl(unsigned int);

} // extern "C"
namespace std {
// Start an empty owning int vector; items are IFF IDs in the outfit collector.
inline vector<int, app::app_allocator<int> >::vector() : items(0), size(0), capacity(0) {}

}
extern "C" {
struct LocalIntVector_u802C874C {
    int* items; volatile unsigned int size; unsigned int capacity;
    // Initialize an empty IFF-ID collection with element-based size/capacity.
    LocalIntVector_u802C874C() : items(0), size(0), capacity(0) {}
    // Release the owned array; integer IDs themselves need no destruction.
    ~LocalIntVector_u802C874C() {
    if (items) {
        size = *(unsigned int*)&size - *(int*)&size;
        appFreeUni__FPv(items);
    }
}
};
typedef LocalIntVector_u802C874C ShopIntVector;
struct ShopVectorBuffer {
    int* data; volatile unsigned size; unsigned capacity; unsigned* owner; volatile unsigned offset;
    // Guard a growth allocation until it swaps with the destination vector.
    ShopVectorBuffer(unsigned* p) : data(0), size(0), capacity(0), owner(p), offset(0) {}
    // Release uncommitted or swapped-out storage with the original volatile reset.
    ~ShopVectorBuffer() { if (data) { volatile unsigned* count = &size; unsigned value = *count; *count = value - value; appFreeUni__FPv(data); } }
};
// Return the larger bounded growth request by reference.
inline const unsigned& ShopVectorMax(const unsigned& a, const unsigned& b) { return a < b ? b : a; }
// Throw the allocator/vector length error.
inline void ShopVectorThrow() { throw std::length_error(); }
// Throw the selected-clothing bitset's range error.
inline void ShopThrowOutOfRange() { throw std::out_of_range(bitsetTestOutOfRangeMessage); }

// Append an IFF ID, reserving/copying/swapping only when the array is full.
inline void ShopVectorPushBack(ShopIntVector* v, int value) {
    // In-place append publishes one new logical element before its store.
    unsigned size = v->size;
    if (size < v->capacity) {
        unsigned next = 1 + size;
        v->size = next;
        v->items[next - 1] = value;
    } else {
        // Reserve -> construct appended ID -> copy prefix -> commit ownership.
        if (kMaxShopVectorWords - v->capacity < 1) throw std::length_error();
        unsigned needed = (v->size + 1) - v->capacity;
        ShopVectorBuffer temp(&v->capacity);
        unsigned capacity = v->capacity;
        if (needed > kMaxShopVectorWords - capacity) throw std::length_error();
        if (capacity < kSmallShopVectorLimit) {
            capacity += ShopVectorMax((3 * (capacity + 1)) / 5, needed);
        } else if (capacity < kMediumShopVectorLimit) {
            capacity += ShopVectorMax((capacity + 1) / 2, needed);
        } else capacity = kMaxShopVectorWords;
        if (capacity > kMaxShopVectorWords) throw std::length_error();
        int* data = (int*)appMallocDummy__FUl(capacity * 4);
        unsigned oldSize = v->size;
        temp.offset = oldSize;
        data[oldSize + temp.size++] = value;
        unsigned count = ((char*)(v->items + v->size) - (char*)v->items) / 4;
        temp.capacity = capacity;
        temp.data = data;
        temp.offset = oldSize - count;
        memcpy(data + (oldSize - count), v->items, count * 4);
        memset(v->items, 0, count * 4);
        unsigned newSize = temp.size + count;
        int* oldData = v->items;
        unsigned oldCapacity = v->capacity;
        v->items = temp.data;
        v->size = newSize;
        v->capacity = temp.capacity;
        temp.data = oldData;
        temp.size = 0;
        temp.capacity = oldCapacity;
    }
}

struct ShopVectorBuffer2 {
    int* data; volatile unsigned size; unsigned capacity; unsigned* owner; volatile unsigned offset;
    // Guard auxiliary-item growth with its distinct unwind destructor.
    ShopVectorBuffer2(unsigned* p) : data(0), size(0), capacity(0), owner(p), offset(0) {}
    // Free the growth/old array using this specialization's direct size reset.
    ~ShopVectorBuffer2() { if (data) { size = 0; appFreeUni__FPv(data); } }
};
// Append the auxiliary IFF ID using the second container's unwind specialization.
// Deliberately parallel to ShopVectorPushBack: destructor symbol/reset differs,
// so merging these helpers can change exception relocations even for int payloads.
inline void ShopVectorPushBack2(ShopIntVector* v, int value) {
    unsigned size = v->size;
    if (size < v->capacity) {
        unsigned next = 1 + size;
        v->size = next;
        v->items[next - 1] = value;
    } else {
        // Same reserve -> append -> copy -> ownership swap; temp frees old storage.
        if (kMaxShopVectorWords - v->capacity < 1) throw std::length_error();
        unsigned needed = (v->size + 1) - v->capacity;
        ShopVectorBuffer2 temp(&v->capacity);
        unsigned capacity = v->capacity;
        if (needed > kMaxShopVectorWords - capacity) throw std::length_error();
        if (capacity < kSmallShopVectorLimit) {
            capacity += ShopVectorMax((3 * (capacity + 1)) / 5, needed);
        } else if (capacity < kMediumShopVectorLimit) {
            capacity += ShopVectorMax((capacity + 1) / 2, needed);
        } else capacity = kMaxShopVectorWords;
        if (capacity > kMaxShopVectorWords) throw std::length_error();
        int* data = (int*)appMallocDummy__FUl(capacity * 4);
        unsigned oldSize = v->size;
        temp.offset = oldSize;
        data[oldSize + temp.size++] = value;
        unsigned count = ((char*)(v->items + v->size) - (char*)v->items) / 4;
        temp.capacity = capacity;
        temp.data = data;
        temp.offset = oldSize - count;
        memcpy(data + (oldSize - count), v->items, count * 4);
        memset(v->items, 0, count * 4);
        unsigned newSize = temp.size + count;
        int* oldData = v->items;
        unsigned oldCapacity = v->capacity;
        v->items = temp.data;
        v->size = newSize;
        v->capacity = temp.capacity;
        temp.data = oldData;
        temp.size = 0;
        temp.capacity = oldCapacity;
    }
}

struct ShopBitReference { unsigned int* word; unsigned int index; };
// Test a checked-preview clothing bit (not ownership); retain the volatile proxy writes.
inline unsigned int ShopBitsetTest(void* base, unsigned int idx, volatile ShopBitReference* b) {
    volatile ShopBitReference a = { ((ObjC874C*)base)->selectedPartBits, idx };
    b->word = ((ObjC874C*)base)->selectedPartBits;
    b->index = idx;
    if (idx >= kPartCount) throw std::out_of_range(bitsetTestOutOfRangeMessage);
    return b->word[idx >> 5] & (1u << (idx & 31));
}

#pragma push
#pragma use_lmw_stmw on
#pragma opt_strength_reduction off
// Collect checked unowned clothing and the selected ring, show up to nine previews, and total their Pang prices.
int CPangya2Shop_OpenOutfitPurchaseDialog(ObjC874C* self) {
    // Phase 1: collect checked, unowned clothing IDs for the current character.
    // entries owns only the ID array; inventory records/resources remain borrowed.
    ShopIntVector entries;
    // Preserve the byte cursor: an eagerly adjusted typed pointer changes register scheduling.
    int baseOffset = AsCPangya2Shop(self)->currentCharaIndex * sizeof(ShopOutfitSelection);
    char* row = (char*)self + baseOffset;
    for (int i = 0; i < kOutfitPartCount; i++, row += sizeof(short)) {
        short index = *(short*)(row + IFF_OFFSET(ObjC874C, outfits) + IFF_OFFSET(ShopOutfitSelection, partIndices));
        if (index == -1) continue;
        volatile ShopBitReference bit;
        bool flag = ShopBitsetTest(self, (unsigned int)index, &bit) &&
            CPangya2Shop_IsItemOwned(self, ItemInventory_GetPartIdAt(AsCSharedDoc(g_sharedDoc)->itemInventory, index)) == 0;
        if (flag) {
            ShopVectorPushBack(&entries, (int)ItemInventory_GetPartIdAt(AsCSharedDoc(g_sharedDoc)->itemInventory, index));
        }
    }
    // Phase 2: append the selected unowned auxiliary item independently of clothing bits.
    int special = *(signed char*)((char*)self + IFF_OFFSET(ObjC874C, outfits) + IFF_OFFSET(ShopOutfitSelection, auxPartIndex) + baseOffset);
    if (special != -1) {
        if (CPangya2Shop_IsItemOwned(self, ItemInventory_GetAuxPartIdAt(AsCSharedDoc(g_sharedDoc)->itemInventory, special)) == 0) {
            ShopVectorPushBack2(&entries, (int)ItemInventory_GetAuxPartIdAt(AsCSharedDoc(g_sharedDoc)->itemInventory, special));
        }
    }
    if (entries.size == 0) return 0;
    // Phase 3: populate nine preview slots (missing IDs use zero), then total ALL
    // collected unowned IDs, including any beyond the nine displayed previews.
    for (int i = 0; i <= kPurchasePreviewSlots - 1; i++) {
        void* value = 0;
        value = (entries.size > (unsigned)i) ? (void*)entries.items[i] : value;
        void* table = AsCPangya2Shop(self)->charaProductBuilder;
        CPurchaseDialog_SetSlotDrawable(AsCPangya2Shop(self)->purchaseDialog, i, (int)CBuildProductChara_FindResourceByTypeId(table, value));
    }
    unsigned int i;
    int totalPang = 0;
    for (i = 0; i < entries.size; i++) {
        if (CPangya2Shop_IsItemOwned(self, (void*)entries.items[i]) == 0)
            totalPang += ItemInventory_GetPricePang(AsCSharedDoc(g_sharedDoc)->itemInventory, entries.items[i]);
    }
    CPurchaseDialog_SetPriceBreakdown(AsCPangya2Shop(self)->purchaseDialog, (int)AsCPangya2Shop(self)->pangBalance, totalPang);
    CPurchaseDialog_Show(AsCPangya2Shop(self)->purchaseDialog);
    return 1;
}
#pragma pop

} // extern "C"

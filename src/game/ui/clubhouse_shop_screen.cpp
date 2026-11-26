// Clubhouse shop: resource admission, equipment previews, modal input and drawing.
// Widescreen/clipping lookup: CPangya2Shop::Draw (0x802C5580) brackets the
// epilogue 0x802C586C. This source does not set a scissor rectangle directly.
// GXSetScissor hook; CLUB_OUT restores the engine rectangle only after an override.
// Preserve the whole draw scope, callback order and post-scope restoration in a
// later widescreen mod. Viewport, sprite placement, clipping and pointer hit tests
// must agree; moving only the preview does not change its input/layout rectangle.
// Model lookup: RefreshAllPanelsAndStartFade binds four ModelOperation objects to the
// embedded save snapshot; PollLoadComplete admits all loaded children; Update advances
// one selected preview.
// Caddies and Club Sets use the glossary's equipment names. Consumable lists
// display the cart and have no 3D preview. These functions borrow child pointers;
// temporary packed files/packTextureResource are the explicitly released owners.
// Frame rate lookup: Update forwards dt to previews/save menu, while
// updateFrameCount increments ONCE per admitted call and saturates at 0xA4CB62.
// Global fade blocks the whole update. Readiness waits must remain separate from
// interactive-rotation mode and from the first-active-modal dispatch priority.
// CPangya2Shop.h supplies the ABI; shopStrings/kShopControlGroupIds/
// kShopNumericTextEntries are shared resources. Required initialized self and
// children, category 0..2, selected list -1 or an existing slot 0..4; callbacks
// may change state between calls. Lifecycle access is serialized, without locks here.
// Preview routing matrix (pointers owned by the containing shop lifecycle):
// category 0 -> charaModelOperation; category 2 -> caddieModelOperation;
// category 1/list 0 -> primaryItemModelOperation (Club Set);
// category 1/list 1 -> secondaryItemModelOperation (Phoenix ball).
// Other equipment lists do not draw/update a model. noCaddieSelected suppresses
// caddie draw/rotation, but selected-preview update/readiness still call its object.
// Keep those admission differences: an empty caddie slot still has an initialized
// operation object. Product-builder slot 3 is a separate, no-delta callback.
// List count is CATEGORY-relative (RefreshProductLists, pangya2_shop_product_screen.cpp): character
// 0=character,1..4=outfit-part groups; equipment 0=Club Set,1=ball,2=consumable
// cart (only THREE lists; slots 3/4 have no equipment meaning); caddie 0=caddie,
// 1=caddie equipment (only TWO). Five is allocation capacity shared by categories,
// not five equipment tabs. Exact outfit group labels remain resource-defined.
#include "classes/SavePlayerRecord.h"
#include "classes/CSharedDoc.h"
#define AsCSharedDoc(p) ((CSharedDoc*)(p))
#include "game/vector.h"
#include "game/singletons.h"
#include "sdk_protos.h"
#include "classes/CPangya2Shop.h"
#include "classes/UITextTable.h"
#include "classes/ShopProductRecord.h"
#include "classes/CPackTextureResource.h"
#include "classes/CPackedFile.h"
#include "classes/CClubDialog.h"
#include "classes/CItemCart.h"
#include "classes/CShopMenu.h"
#include "classes/CShopSubMenuB.h"
#include "classes/CClubLvUpDialog.h"
#include "classes/CShopSubMenuC.h"
#include "classes/CClubSubMenu.h"
#include "classes/CItemInfo.h"
#include "classes/CPlayerStatus.h"
#include "classes/CShopProductsTab.h"
#include "classes/CProductsCategory.h"
#include "classes/CPurchaseDialog.h"
#include "classes/CShopLayoutState.h"
#include "classes/CShopSubMenu.h"
#include "classes/CB2dPlayerHandle.h"
#include "classes/CModelOperationCaddie.h"
#include "classes/CNowReloading.h"
#include "classes/CClubHouseMsg.h"
#include "classes/CBpCtrl.h"
#include "classes/CFontNumDraw.h"

#define AsCPangya2Shop(p) ((CPangya2Shop*)(p))
extern "C" {

int Layout_CollectActivated(void*, void*, int);
void* WordVector_GetElementAddress(void*, int);
int ShopPreview_PollReady(void*);
void* LayoutManager_GetInstance();
int Input_IsActionPressed(void*, int, int);
void BpObject_TryInvokeCurrentCallback(void*, int);
void BpObject_Draw(void*);
void Layout_DrawWithOpacity(void*, float);
void UITextTable_Draw(void*);
int ModelOperationBase_IsInteractiveRotationEnabled(void*);
int ModelOperationBase_Draw(void*);
void MPBase1_DrawIfInitialized(void*);
void* g_soundManager;
void Sound_PlayByName(void*, const char*, int, int, int, int, float, float, float);
int Layout_PollLoadComplete(void*);
void CBpCtrl_dtor(void*, int);
int ModelOperationBase_IsResourceLoaded(void*);
signed char s_bpCtrlInitialized;
char s_bpCtrl[];
char s_bpCtrlDtorRegistration_802C470C[]; // Destructor-registration storage for the shared BP controller.
signed char s_fadeOverlayInitialized;
signed char s_saveDataInitialized;
char s_fadeOverlay[];
char s_saveData[];
char s_fadeOverlayDtorRegistration[];
int FadeState_GetState(void*);
void FadeOverlay_Init(void*);
void ResourceState_Dtor(void*, int);
void FadeState_SetAlternateDrawEnabled(void*, int);
int ModelOperationBase_IsInteractiveRotationEnabled(void*);
void IFadeMenuBase_StartMenuFadeOut(void*);
void __dt__13CNowReloadingFv(void*, int);
void SaveData_Ctor(void*);
void SaveData_SetCurrentRecord(void*, void*);
void* UIGroup_GetOrCreateById(void*, int);
void UITextTable_SetInteger(void*, int, void*);
int MPBase1_IsComplete(void*);
void MPBase1_Process(void*, float);
void Layout_Update(void*);
int LoadingIndicator_IsAbsentOrState3(void*);
void Layout_SetModeByte(void*, int);
void BpObject_StartScriptByName(void*, const char*, int);
void UIGroup_SetEnabled(void*, int);
void BpObject_SetEnabled(void*, int);
void Layout_SetElementEnabled(void*, int, int);
void Layout_ReadElementRect(void*, int, int*);
void Layout_WriteElementRect(void*, int, int*);
void* ItemInventory_GetCharacterIdAt(void*, int);
void* ItemInventory_GetPartIdAt(void*, int);
void ModelOperationBase_CreatePositionTable(void*);
void ModelOperationBase_SetSaveRecord(void*, void*);
void ModelOperationBase_TryApplyArgument(void*, void*);
void* ItemInventory_GetCaddieItemIdAt(void*, int);
void* ItemInventory_GetCaddieIdAt(void*, int);
void* ItemInventory_GetClubSetIdAt(void*, int);
void* ItemInventory_GetBallIdAt(void*, int);
void UITextTable_SetEntryResourceName(void*, int, const char*);
void* ShopProductTable_GetRecordAt(void*, int, void*);
void CPangya2Shop_UpdatePurchaseButton(void*);
extern signed char s_nowReloadingInitialized;
extern char shopStrings[];
extern float kFloat_0_5_shopSound;
extern float kFloat_200_Shop;
extern float kFloat_1_805E0918;
extern char s_nowReloading;
extern void* g_sharedDoc;
extern char kShopControlGroupIds[];
extern char kShopNumericTextEntries[];
extern char s_nowReloadingDtorRegistration_802C470C[]; // Destructor-registration storage for the reloading indicator.
}
typedef IntVector ActivatedList;

enum ShopCategory {
    kCharacters = 0,
    kEquipment = 1,
    kCaddies = 2
};
enum ShopConstants {
    kProductListCount = 5,
    kPackedFileCount = 3,
    kPartSlotCount = 24,
    kClubSetList = 0,
    kBallList = 1,
    kConsumableList = 2,
    kPangBalanceText = 0x2B,
    kDescriptionGroup = 0x40E,
    kOpenClubDialogElement = 6,
    kOpenClubDialogAction = 0x25,
    kPreviousCategoryAction = 2,
    kNextCategoryAction = 3,
    kDiscardOutfitDialog = 2,
    kMaximumUpdateFrames = 0xA4CB62, // Saturation in update calls; not seconds or an animation duration.
    kControlGroupCount = 8,
    kFirstHiddenEntryElement = 7,
    kEndHiddenEntryElement = 14,
    kCategoryCount = 3,
    kCategoryRectSourceBase = 3,
    kNoSelectedProductList = -1,
    kPowerText = 0x2D,
    kControlText = 0x2E,
    kSpinText = 0x2F
};
enum ShopStringOffset {
    kBaseScript = 0x51,
    kOutfitScript = 0x5A,
    kPangNumberFont = 0x65,
    kStatusNumberFont = 0x75,
    kPageChangeSound = 0x87
};
// Model-operation object +0x0C dispatches (self,dt); it advances a preview, with
// dt in the caller's time units. Product-builder slot 3 has a DIFFERENT (self)
// signature. Neither is a Layout object merely because old names suggested it.
enum ShopPreviewDispatch {
    kPreviewUpdateSlot = 3
};
enum ShopProductBuilderDispatch {
    kProductBuilderVfunc3Slot = 3
};
typedef void (*ShopPreviewUpdateCallback)(void* modelOperation, float delta);
struct SelectedProductInfo {
    int pricePang;
    short power;
    short control;
    short spin;
    char reserved0A[4];
    char nameMessageId[40];
    char descriptionMessageId[40];
};
struct ShopSaveSnapshot {
    char base[0x18];
    SavePlayerRecord record;
};
struct ShopCharacterParts {
    short reserved;
    short partInventoryIndices[24];
    short reserved32;
};
struct ShopEquipmentView {
    char base[0x2E4];
    ShopCharacterParts characters[8];
};

typedef CPangya2Shop Controller;

struct ShopVirtual {
    virtual void destroy(int);
    virtual void update();
};

extern "C" {
#pragma use_lmw_stmw on
#pragma opt_lifetimes off
#pragma opt_strength_reduction off
#pragma opt_common_subs off
// Initialize shop lists, equipment previews, item descriptions, Pang display, and dialogs.
// Phase order matters: choose entry mode -> prepare loading indicator -> initialize
// category/list selection -> bind owned previews to the embedded save snapshot ->
// populate text/number resources -> reset modal layouts -> delegate fade-menu setup.
// Re-entry in initialized character-entry mode only delegates to fade setup.
// The embedded SavePlayerRecord must outlive every preview and SaveData use.
// Rectangle copying below is in layout X/Y/width/height, not world coordinates.
// Equipment admission map (snapshot ordinals, never packed item type IDs):
// currentCharaIndex must be 0..7 AND < inventory.characters.size. Besides the
// guarded ID getter, it directly indexes ShopEquipmentView::characters[8].
// Each of its 24 signed partInventoryIndices is -1 (empty) or a nonnegative
// ordinal < inventory.parts.size; GetPartIdAt maps -1/other invalid values to ID zero.
// clubSetIndex/ballIndex are nonnegative ordinals below clubSets.size/balls.size
// for an equipped preview. currentCaddieIndex is -1 or a nonnegative ordinal
// below caddies.size (all five equipped selection fields are signed bytes);
// caddieItemIndex is interpreted as SIGNED BYTE: -1 or 0..127, and any present
// item must be below caddieItems.size. Inventory getter bounds/zero returns are
// implemented in item_inventory_tables.cpp; they do not protect the direct character-array read.
// This routine checks none of these snapshot invariants and ignores the model
// TryApply/TryToggle results: invalid getter indices forward ID zero, with no
// local error publication or guaranteed preview replacement.
void CPangya2Shop::RefreshAllPanelsAndStartFade() {
    Controller* self = (Controller*)this;
    int partSlot;
    int listByteOffset;
    unsigned char* entryCursor;
    int elementIndex;
    int productListIndex;
    // Phase 1: establish entry/loading state and reset the control/script layout.
    ((CPangya2Shop*)self)->UpdateControlScriptSelection();
    unsigned char flag = AsCPangya2Shop(self)->characterEntryMode;
    if (flag && AsCPangya2Shop(self)->entryInitialized) {
        IFadeMenuBase_StartMenuFadeOut(self);
        return;
    }
    if (flag)
        AsCPangya2Shop(self)->entryInitialized = 1;
    if (!Sing_805DC610()) {
        if (!s_nowReloadingInitialized) {
            ((CNowReloading*)(&s_nowReloading))->Init();
            __register_global_object(&s_nowReloading, (void*)__dt__13CNowReloadingFv,
                                     s_nowReloadingDtorRegistration_802C470C);
            s_nowReloadingInitialized = 1;
        }
        Sing_805DC610() = &s_nowReloading;
    }
    entryCursor = (unsigned char*)Sing_805DC610();
    if (entryCursor && LoadingIndicator_IsAbsentOrState3(entryCursor))
        CB2dPlayerHandle::RequestPendingScript(entryCursor);
    Layout_SetModeByte(AsCPangya2Shop(self)->shopLayout, 0);
    void* p = AsCPangya2Shop(self)->packTextureResource;
    if (p) {
        if (p)
            ((ShopVirtual*)p)->destroy(1);
        AsCPangya2Shop(self)->packTextureResource = 0;
    }
    BpObject_StartScriptByName(AsCPangya2Shop(self)->uiGroup, shopStrings + kBaseScript, 1);
    entryCursor = (unsigned char*)kShopControlGroupIds;
    for (listByteOffset = 0; (unsigned)listByteOffset < kControlGroupCount; ++listByteOffset, entryCursor += 4)
        UIGroup_SetEnabled(UIGroup_GetOrCreateById(AsCPangya2Shop(self)->uiGroup, *(int*)entryCursor), 0);
    BpObject_StartScriptByName(AsCPangya2Shop(self)->scriptObject, shopStrings + kOutfitScript, 1);
    BpObject_SetEnabled(AsCPangya2Shop(self)->scriptObject, 0);
    for (elementIndex = kFirstHiddenEntryElement; elementIndex < kEndHiddenEntryElement; ++elementIndex)
        Layout_SetElementEnabled(AsCPangya2Shop(self)->shopLayout, elementIndex, 0);
    int elementRect[4] = {0, 0, 0, 0};
    for (elementIndex = 0; elementIndex < kCategoryCount; ++elementIndex) {
        Layout_ReadElementRect(AsCPangya2Shop(self)->shopLayout, elementIndex + kCategoryRectSourceBase, elementRect);
        Layout_WriteElementRect(AsCPangya2Shop(self)->shopLayout, elementIndex, elementRect);
    }
    ((CPangya2Shop*)self)->UpdateCategoryHighlights();
    // Phase 2: refresh product tabs and select the equipped character/part IDs.
    productListIndex = 0;
    listByteOffset = 0;
    for (; productListIndex < kProductListCount; ++productListIndex, listByteOffset += 4) {
        (*(ShopVirtual**)((char*)AsCPangya2Shop(self)->productLists + listByteOffset))->update();
        if (productListIndex == AsCPangya2Shop(self)->selectedListIndex)
            ((CShopProductsTab*)(*(void**)((char*)AsCPangya2Shop(self)->productLists + listByteOffset)))->SetEnabled(1);
        if (productListIndex >= 0 && productListIndex < kProductListCount)
            switch (productListIndex) {
            case 0: {
                void* x = ItemInventory_GetCharacterIdAt(AsCSharedDoc(g_sharedDoc)->itemInventory,
                                                         AsCPangya2Shop(self)->currentCharaIndex);
                ((CShopProductsTab*)(((void**)AsCPangya2Shop(self)->productLists)[0]))->SelectProductByTypeId(x);
                break;
            }
            case 1:
            case 2:
            case 3:
            case 4: {
                if (productListIndex < 1 || productListIndex >= 5)
                    break;
                signed char characterIndex = AsCPangya2Shop(self)->currentCharaIndex;
                // 0x802C4A10: +0x44
                entryCursor = (unsigned char*)&((ShopEquipmentView*)self)->characters[characterIndex];
                partSlot = 0;
                for (; partSlot < kPartSlotCount; ++partSlot, entryCursor += 2) {
                    void* x =
                        ItemInventory_GetPartIdAt(AsCSharedDoc(g_sharedDoc)->itemInventory, *(short*)(entryCursor + 2));
                    if (((CShopProductsTab*)(*(void**)(listByteOffset +
                                                       (*(unsigned int*)&AsCPangya2Shop(self)->productLists))))
                            ->SelectProductByTypeId(x))
                        break;
                }
                break;
            }
            }
    }
    int selected = AsCPangya2Shop(self)->selectedListIndex;
    if (selected != -1)
        ((CShopProductsTab*)(((void**)AsCPangya2Shop(self)->productLists)[selected]))->RefreshVisibleProducts();
    ((CProductsCategory*)(AsCPangya2Shop(self)->productsCategory))->InitLayout(AsCPangya2Shop(self)->categoryIndex, 1);
    // Phase 3: bind all four preview operations to the embedded save snapshot, then apply equipped IDs.
    ModelOperationBase_CreatePositionTable(AsCPangya2Shop(self)->charaModelOperation);
    ModelOperationBase_SetSaveRecord(AsCPangya2Shop(self)->charaModelOperation, &((ShopSaveSnapshot*)self)->record);
    ModelOperationBase_TryApplyArgument(AsCPangya2Shop(self)->charaModelOperation,
                                        ItemInventory_GetCharacterIdAt(AsCSharedDoc(g_sharedDoc)->itemInventory,
                                                                       AsCPangya2Shop(self)->currentCharaIndex));
    ModelOperationBase_CreatePositionTable(AsCPangya2Shop(self)->caddieModelOperation);
    ModelOperationBase_SetSaveRecord(AsCPangya2Shop(self)->caddieModelOperation, &((ShopSaveSnapshot*)self)->record);
    int caddieItemIndex = (signed char)*(unsigned char*)&AsCPangya2Shop(self)->caddieItemIndex;
    // Joint selection rule: item AND caddie present -> toggle the item; otherwise
    // apply the caddie. An item alone is ignored. For caddie==-1, mark no-caddie
    // and initialize with caddie ordinal zero (which must exist); draw/rotation
    // suppress that placeholder via noCaddieSelected. This path only SETS that
    // flag, never clears it: a visible selection needs an already-cleared flag
    // or InitializeProductSelection (pangya2_shop_screen.cpp), which updates both directions.
    if (caddieItemIndex != -1 && AsCPangya2Shop(self)->currentCaddieIndex != -1)
        ((CModelOperationCaddie*)AsCPangya2Shop(self)->caddieModelOperation)
            ->TryToggleItem(
                (int)ItemInventory_GetCaddieItemIdAt(AsCSharedDoc(g_sharedDoc)->itemInventory, caddieItemIndex));
    else {
        int caddieIndex = AsCPangya2Shop(self)->currentCaddieIndex;
        if (caddieIndex == -1) {
            AsCPangya2Shop(self)->noCaddieSelected = 1;
            caddieIndex = 0;
        }
        ModelOperationBase_TryApplyArgument(
            AsCPangya2Shop(self)->caddieModelOperation,
            ItemInventory_GetCaddieIdAt(AsCSharedDoc(g_sharedDoc)->itemInventory, caddieIndex));
    }
    ModelOperationBase_CreatePositionTable(AsCPangya2Shop(self)->primaryItemModelOperation);
    ModelOperationBase_SetSaveRecord(AsCPangya2Shop(self)->primaryItemModelOperation,
                                     &((ShopSaveSnapshot*)self)->record);
    ModelOperationBase_TryApplyArgument(
        AsCPangya2Shop(self)->primaryItemModelOperation,
        ItemInventory_GetClubSetIdAt(AsCSharedDoc(g_sharedDoc)->itemInventory, AsCPangya2Shop(self)->clubSetIndex));
    ModelOperationBase_CreatePositionTable(AsCPangya2Shop(self)->secondaryItemModelOperation);
    ModelOperationBase_SetSaveRecord(AsCPangya2Shop(self)->secondaryItemModelOperation,
                                     &((ShopSaveSnapshot*)self)->record);
    ModelOperationBase_TryApplyArgument(
        AsCPangya2Shop(self)->secondaryItemModelOperation,
        ItemInventory_GetBallIdAt(AsCSharedDoc(g_sharedDoc)->itemInventory, AsCPangya2Shop(self)->ballIndex));
    // Phase 4: populate status/cart, numeric labels and the selected product description.
    CPlayerStatus::InitEquipmentDisplay(AsCPangya2Shop(self)->playerStatus, AsCPangya2Shop(self)->categoryIndex);
    CItemCart::InitLayout(AsCPangya2Shop(self)->itemCart);
    ((CFontNumDraw*)(AsCPangya2Shop(self)->textTable))
        ->RegisterEntries(kShopNumericTextEntries, 5, AsCPangya2Shop(self)->uiGroup, shopStrings + kPangNumberFont);
    UITextTable_SetEntryResourceName(AsCPangya2Shop(self)->textTable, kPowerText, shopStrings + kStatusNumberFont);
    UITextTable_SetEntryResourceName(AsCPangya2Shop(self)->textTable, kControlText, shopStrings + kStatusNumberFont);
    UITextTable_SetEntryResourceName(AsCPangya2Shop(self)->textTable, kSpinText, shopStrings + kStatusNumberFont);
    AsUITextTable(AsCPangya2Shop(self)->textTable)->drawEnabled = 1;
    UITextTable_SetInteger(AsCPangya2Shop(self)->textTable, kPangBalanceText, AsCPangya2Shop(self)->pangBalance);
    ((CClubHouseMsg*)(AsCPangya2Shop(self)->productMessage))->CreateTextFromResource();
    ((CClubHouseMsg*)(AsCPangya2Shop(self)->subMenuMessage))->CreateTextFromResource();
    ((CClubHouseMsg*)(AsCPangya2Shop(self)->productMessage))->ResetTextState();
    SelectedProductInfo productInfo;
    productInfo.pricePang = 0;
    Memory_Clear(&productInfo.power, 10);
    Memory_Clear(productInfo.nameMessageId, 40);
    Memory_Clear(productInfo.descriptionMessageId, 40);
    if (((CPangya2Shop*)self)->GetSelectedProductInfo(&productInfo))
        ((CClubHouseMsg*)(AsCPangya2Shop(self)->productMessage))
            ->SetTextFromGroup(productInfo.nameMessageId,
                               (void*)UIGroup_GetOrCreateById(AsCPangya2Shop(self)->uiGroup, kDescriptionGroup), 0);
    ((CPangya2Shop*)self)->RefreshSelectedProductValues();
    ((CPangya2Shop*)self)->UpdateSelectedPurchaseAmount();
    if (AsCPangya2Shop(self)->categoryIndex == kEquipment &&
        AsCPangya2Shop(self)->selectedListIndex == kConsumableList) {
        ((CPlayerStatus*)(AsCPangya2Shop(self)->playerStatus))->SetEnabled(0);
        ((CShopSubMenuB*)(AsCPangya2Shop(self)->itemCart))->SetEnabled(1);
        void* result;
        if (AsCPangya2Shop(self)->selectedListIndex == -1)
            result = 0;
        else {
            void* item = ((CShopProductsTab*)((
                              (void**)AsCPangya2Shop(self)->productLists)[AsCPangya2Shop(self)->selectedListIndex]))
                             ->GetSelectedProductIndex();
            void* found = 0;
            switch (AsCPangya2Shop(self)->categoryIndex) {
            case kCharacters:
                found = ShopProductTable_GetRecordAt(AsCPangya2Shop(self)->charaProductBuilder,
                                                     AsCPangya2Shop(self)->selectedListIndex, item);
                break;
            case kEquipment:
                found = ShopProductTable_GetRecordAt(AsCPangya2Shop(self)->itemProductBuilder,
                                                     AsCPangya2Shop(self)->selectedListIndex, item);
                break;
            case kCaddies:
                found = ShopProductTable_GetRecordAt(AsCPangya2Shop(self)->caddieProductBuilder,
                                                     AsCPangya2Shop(self)->selectedListIndex, item);
                break;
            }
            result = found;
        }
        // Product-list result: +50 is a packed item TYPE ID. SetItemMessage
        // resolves it through ItemInventory_FindItemIndex (club_lvup_dialog_and_shop_submenus.cpp), then uses
        // that inventory index to select a description. The pointer-shaped
        // argument here is the ID word, not a caddie index or an owned pointer.
        if (result)
            ((CShopSubMenuB*)(AsCPangya2Shop(self)->itemCart))
                ->SetItemMessage((int)*(void**)&((ShopProductRecord*)result)->typeId);
    } else {
        ((CPlayerStatus*)(AsCPangya2Shop(self)->playerStatus))->SetEnabled(1);
        ((CShopSubMenuB*)(AsCPangya2Shop(self)->itemCart))->SetEnabled(0);
    }
    // Phase 5: reset modal layouts after product data is installed, then enter inherited fade setup.
    CPangya2Shop_UpdatePurchaseButton(self);
    ((CClubSubMenu*)(AsCPangya2Shop(self)->clubSubMenu))->ResetLayoutMode();
    CClubDialog::InitLayout(AsCPangya2Shop(self)->clubDialog);
    ((CPurchaseDialog*)(AsCPangya2Shop(self)->purchaseDialog))->InitLayout();
    ((CItemInfo*)(AsCPangya2Shop(self)->itemInfo))->InitializeDetailsUI();
    ((CClubLvUpDialog*)(AsCPangya2Shop(self)->clubLvUpDialog))->InitLayout();
    IFadeMenuBase_StartMenuFadeOut(self);
}

#pragma use_lmw_stmw reset
#pragma opt_lifetimes reset
#pragma opt_strength_reduction reset
#pragma opt_common_subs reset
// Wait for shop resources and child menus, then release the temporary packed-file handles.
// Returns 0 at the first pending child, retaining temporary handles for retry;
// returns 1 only after all checks and handle destruction. Packed files use a
// synchronous polling loop with no timeout. There is no fixed call-count delay.
int CPangya2Shop::PollLoadComplete() {
    Controller* self = (Controller*)this;
    // Phase 1: poll packed resources. Packed-file handles below are synchronously waited on.
    if (AsCPangya2Shop(self)->packTextureResource) {
        if (!((CPackTextureResource*)(AsCPangya2Shop(self)->packTextureResource))->IsLoadFinished())
            return 0;
    }
    unsigned off = 0;
    unsigned i = 0;
    for (; i < kPackedFileCount; i++, off += 4) {
        if (*(void**)((char*)AsCPangya2Shop(self)->packedFiles + off)) {
            for (;;) {
                if (((CPackedFile*)(*(void**)((char*)AsCPangya2Shop(self)->packedFiles + off)))->IsLoadFinished() ==
                    0) {
                    ((CPackedFile*)(*(void**)((char*)AsCPangya2Shop(self)->packedFiles + off)))->PollLoad();
                } else {
                    break;
                }
            }
        }
    }
    if (!Layout_PollLoadComplete(AsCPangya2Shop(self)->shopLayout))
        return 0;
    if (!Sing_805DC5F0()) {
        if (!s_bpCtrlInitialized) {
            char* p = s_bpCtrl;
            ((CBpCtrl*)p)->__ct();
            __register_global_object(p, (void*)CBpCtrl_dtor, s_bpCtrlDtorRegistration_802C470C);
            s_bpCtrlInitialized = 1;
        }
        Sing_805DC5F0() = s_bpCtrl;
    }
    if (!((CBpCtrl*)Sing_805DC5F0())->PollAllEntriesLoaded())
        return 0;
    if (!((CShopLayoutState*)(AsCPangya2Shop(self)->productsCategory))->IsLoaded())
        return 0;
    if (!ModelOperationBase_IsResourceLoaded(AsCPangya2Shop(self)->charaModelOperation))
        return 0;
    if (!ModelOperationBase_IsResourceLoaded(AsCPangya2Shop(self)->caddieModelOperation))
        return 0;
    if (!ModelOperationBase_IsResourceLoaded(AsCPangya2Shop(self)->primaryItemModelOperation))
        return 0;
    if (!ModelOperationBase_IsResourceLoaded(AsCPangya2Shop(self)->secondaryItemModelOperation))
        return 0;
    i = 0;
    off = 0;
    for (; (int)i < kProductListCount; i++, off += 4) {
        if (!((CShopProductsTab*)(*(void**)((char*)AsCPangya2Shop(self)->productLists + off)))->PollLoad())
            return 0;
    }
    if (!((CPlayerStatus*)(AsCPangya2Shop(self)->playerStatus))->PollResourcesLoaded())
        return 0;
    if (!((CShopSubMenuB*)(AsCPangya2Shop(self)->itemCart))->IsLoaded())
        return 0;
    if (!((CClubHouseMsg*)(AsCPangya2Shop(self)->productMessage))->IsResourceLoaded())
        return 0;
    if (!((CClubHouseMsg*)(AsCPangya2Shop(self)->subMenuMessage))->IsResourceLoaded())
        return 0;
    if (!((CPangya2Shop*)(AsCPangya2Shop(self)->clubSubMenu))->IsLayoutLoaded())
        return 0;
    if (!((CShopMenu*)(AsCPangya2Shop(self)->clubDialog))->IsLayoutLoaded())
        return 0;
    if (!((CShopSubMenu*)(AsCPangya2Shop(self)->purchaseDialog))->IsLoaded())
        return 0;
    if (!((CItemInfo*)(AsCPangya2Shop(self)->itemInfo))->IsLoaded())
        return 0;
    if (!((CShopSubMenuC*)(AsCPangya2Shop(self)->clubLvUpDialog))->IsLoaded())
        return 0;
    // Phase 2: all children are loaded; destroy temporary packed-file handles and clear slots.
    i = 0;
    off = 0;
    int zero = 0;
    for (; i < kPackedFileCount; i++, off += 4) {
        void* elem3 = *(void**)((char*)AsCPangya2Shop(self)->packedFiles + off);
        if (elem3) {
            if (elem3) {
                void** vt = *(void***)elem3;
                ((void (*)(void*, int))vt[2])(elem3, 1);
            }
            *(void**)((char*)AsCPangya2Shop(self)->packedFiles + off) = (void*)zero;
        }
    }
    return 1;
}

// Check the preview model selected by the character, equipment, or caddie category.
// Returns 1 if the selected preview is ready OR no preview applies (consumables,
// unselected/unknown categories). No model pointer ownership changes.
int CPangya2Shop_AreSelectedPreviewsReady(Controller* self) {
    if (self->categoryIndex == kCharacters && !ShopPreview_PollReady(self->charaModelOperation))
        return 0;
    if (self->categoryIndex == kCaddies && !ShopPreview_PollReady(self->caddieModelOperation))
        return 0;
    if (self->categoryIndex == kEquipment && self->selectedListIndex == kClubSetList &&
        !ShopPreview_PollReady(self->primaryItemModelOperation))
        return 0;
    if (self->categoryIndex == kEquipment && self->selectedListIndex == kBallList &&
        !ShopPreview_PollReady(self->secondaryItemModelOperation))
        return 0;
    return 1;
}

// Update the selected preview and active dialog, then process normal shop input.
// Frame priority (first matching branch consumes the frame): fade -> selected
// preview update -> interactive rotation -> item details -> purchase -> club
// dialog -> club submenu -> level-up -> save menu -> normal shop controls.
// Returns 1 only when club-dialog processing completes; 0 covers normal updates,
// modal waits and fade gates. It is NOT a success/failure or readiness result.
// The save snapshot is embedded in self; SaveData_SetCurrentRecord receives its
// address on club completion when outside character-entry mode.
// dt is simulated seconds, forwarded unchanged from actor processing; clock
// origin is CMainFrame::UpdateVirtualHooks (main_frame_letter_and_mouse_cursor.cpp, milliseconds->seconds).
// It must be finite/nonnegative by caller convention. The frame
// counter is independent of dt and freezes behind the global fade gate. A null
// saveMenu ends saveMenuActive but MPBase1_Process is still called; preserve the
// callee's nullable-pointer contract instead of removing that call.
int CPangya2Shop::Update(float dt) {
    Controller* self = (Controller*)this;
    if (!Sing_805DC608()) {
        if (!s_fadeOverlayInitialized) {
            FadeOverlay_Init(s_fadeOverlay);
            __register_global_object(s_fadeOverlay, (void*)ResourceState_Dtor, s_fadeOverlayDtorRegistration);
            s_fadeOverlayInitialized = 1;
        }
        Sing_805DC608() = s_fadeOverlay;
    }
    if (FadeState_GetState(Sing_805DC608()))
        return 0;
    // Embedded record storage is always present for a valid controller. This
    // address guard is retained solely to preserve the existing binary shape.
    if (&((ShopSaveSnapshot*)self)->record) {
        int n = AsCPangya2Shop(self)->updateFrameCount + 1;
        AsCPangya2Shop(self)->updateFrameCount = n < 0 ? 0 : (n > kMaximumUpdateFrames ? kMaximumUpdateFrames : n);
    }
    // Phase 1: advance exactly one selected model operation before testing modes.
    int category = AsCPangya2Shop(self)->categoryIndex;
    if (category == kCharacters)
        ((ShopPreviewUpdateCallback)(*(void***)AsCPangya2Shop(self)->charaModelOperation)[kPreviewUpdateSlot])(
            AsCPangya2Shop(self)->charaModelOperation, dt);
    else if (category == kCaddies)
        ((ShopPreviewUpdateCallback)(*(void***)AsCPangya2Shop(self)->caddieModelOperation)[kPreviewUpdateSlot])(
            AsCPangya2Shop(self)->caddieModelOperation, dt);
    else if (category == kEquipment) {
        int sel = AsCPangya2Shop(self)->selectedListIndex;
        if (sel == kClubSetList)
            ((ShopPreviewUpdateCallback)(*(void***)AsCPangya2Shop(self)
                                              ->primaryItemModelOperation)[kPreviewUpdateSlot])(
                AsCPangya2Shop(self)->primaryItemModelOperation, dt);
        else if (sel == kBallList)
            ((ShopPreviewUpdateCallback)(*(void***)AsCPangya2Shop(self)
                                              ->secondaryItemModelOperation)[kPreviewUpdateSlot])(
                AsCPangya2Shop(self)->secondaryItemModelOperation, dt);
    }
    // Interactive rotation is a mode flag, separate from asynchronous load readiness.
    category = AsCPangya2Shop(self)->categoryIndex;
    int interactiveRotationEnabled = 0;
    if (category == kCharacters)
        interactiveRotationEnabled =
            ModelOperationBase_IsInteractiveRotationEnabled(AsCPangya2Shop(self)->charaModelOperation);
    else if (category == kCaddies) {
        if (!AsCPangya2Shop(self)->noCaddieSelected)
            interactiveRotationEnabled =
                ModelOperationBase_IsInteractiveRotationEnabled(AsCPangya2Shop(self)->caddieModelOperation);
    } else if (category == kEquipment) {
        int sel = AsCPangya2Shop(self)->selectedListIndex;
        if (sel == kClubSetList)
            interactiveRotationEnabled =
                ModelOperationBase_IsInteractiveRotationEnabled(AsCPangya2Shop(self)->primaryItemModelOperation);
        else
            interactiveRotationEnabled =
                (sel == kBallList)
                    ? ModelOperationBase_IsInteractiveRotationEnabled(AsCPangya2Shop(self)->secondaryItemModelOperation)
                    : interactiveRotationEnabled;
    }
    if (interactiveRotationEnabled) {
        ((CPangya2Shop*)self)->HandleFixedCameraPresetInput();
        return 0;
    }
    // Phase 2: modal priority is explicit; even a modal closed this tick consumes it.
    if (AsCPangya2Shop(self)->itemInfoOpen) {
        if (((CItemInfo*)(AsCPangya2Shop(self)->itemInfo))->HandleDismissInput()) {
            AsCPangya2Shop(self)->itemInfoOpen = 0;
            ((CPangya2Shop*)self)->UpdateControlScriptSelection();
        } else
            ((CItemInfo*)(AsCPangya2Shop(self)->itemInfo))->UpdateLayoutAndMessage();
        return 0;
    }
    if (AsCPangya2Shop(self)->purchaseDialogOpen) {
        ((CPangya2Shop*)self)->ProcessOutfitPurchaseResult();
        return 0;
    }
    if (AsCPangya2Shop(self)->clubDialogOpen) {
        if (!((CPangya2Shop*)self)->ProcessClubDialogResult())
            return 0;
        if (!AsCPangya2Shop(self)->characterEntryMode) {
            if (!GetSaveDataSlot()) {
                if (!s_saveDataInitialized) {
                    SaveData_Ctor(s_saveData);
                    s_saveDataInitialized = 1;
                }
                GetSaveDataSlot() = s_saveData;
            }
            SaveData_SetCurrentRecord(GetSaveDataSlot(), &((ShopSaveSnapshot*)self)->record);
        }
        return 1;
    }
    if (AsCPangya2Shop(self)->clubSubMenuOpen) {
        ((CPangya2Shop*)self)->ProcessClubSubMenu();
        return 0;
    }
    if (AsCPangya2Shop(self)->clubLvUpDialogOpen) {
        if (((CClubLvUpDialog*)(AsCPangya2Shop(self)->clubLvUpDialog))->Close()) {
            CPlayerStatus::RefreshEquipmentDisplay(AsCPangya2Shop(self)->playerStatus,
                                                   AsCPangya2Shop(self)->categoryIndex);
            ((CClubHouseMsg*)(AsCPangya2Shop(self)->productMessage))->ResetTextState();
            SelectedProductInfo productInfo;
            productInfo.pricePang = 0;
            Memory_Clear(&productInfo.power, 10);
            Memory_Clear(productInfo.nameMessageId, 40);
            Memory_Clear(productInfo.descriptionMessageId, 40);
            if (((CPangya2Shop*)self)->GetSelectedProductInfo(&productInfo))
                ((CClubHouseMsg*)(AsCPangya2Shop(self)->productMessage))
                    ->SetTextFromGroup(productInfo.nameMessageId,
                                       (void*)UIGroup_GetOrCreateById(AsCPangya2Shop(self)->uiGroup, kDescriptionGroup),
                                       0);
            ((CPangya2Shop*)self)->RefreshSelectedProductValues();
            ((CPangya2Shop*)self)->UpdateSelectedPurchaseAmount();
            AsCPangya2Shop(self)->clubLvUpDialogOpen = 0;
            UITextTable_SetInteger(AsCPangya2Shop(self)->textTable, kPangBalanceText,
                                   AsCPangya2Shop(self)->pangBalance);
        } else
            ((CClubLvUpDialog*)(AsCPangya2Shop(self)->clubLvUpDialog))->Update();
        return 0;
    }
    if (AsCPangya2Shop(self)->saveMenuActive) {
        if (!AsCPangya2Shop(self)->saveMenu || MPBase1_IsComplete(AsCPangya2Shop(self)->saveMenu)) {
            AsCPangya2Shop(self)->saveMenuActive = 0;
            if (!Sing_805DC608()) {
                if (!s_fadeOverlayInitialized) {
                    FadeOverlay_Init(s_fadeOverlay);
                    __register_global_object(s_fadeOverlay, (void*)ResourceState_Dtor, s_fadeOverlayDtorRegistration);
                    s_fadeOverlayInitialized = 1;
                }
                Sing_805DC608() = s_fadeOverlay;
            }
            FadeState_SetAlternateDrawEnabled(Sing_805DC608(), 0);
        }
        MPBase1_Process(AsCPangya2Shop(self)->saveMenu, dt);
        return 0;
    }
    // Phase 3: no modal consumed the tick; update product builder, layouts, visible
    // status/cart, message reveal, and finally product selection in that order.
    category = AsCPangya2Shop(self)->categoryIndex;
    if (category == kCharacters)
        ((void (*)(void*))(*(void***)AsCPangya2Shop(self)->charaProductBuilder)[kProductBuilderVfunc3Slot])(
            AsCPangya2Shop(self)->charaProductBuilder);
    else if (category == kCaddies)
        ((void (*)(void*))(*(void***)AsCPangya2Shop(self)->caddieProductBuilder)[kProductBuilderVfunc3Slot])(
            AsCPangya2Shop(self)->caddieProductBuilder);
    else if (category == kEquipment)
        ((void (*)(void*))(*(void***)AsCPangya2Shop(self)->itemProductBuilder)[kProductBuilderVfunc3Slot])(
            AsCPangya2Shop(self)->itemProductBuilder);
    Layout_Update(AsCPangya2Shop(self)->shopLayout);
    ((CProductsCategory*)(AsCPangya2Shop(self)->productsCategory))->UpdateLayout();
    if (AsCPangya2Shop(self)->selectedListIndex != -1)
        ((CShopProductsTab*)(((void**)AsCPangya2Shop(self)->productLists)[AsCPangya2Shop(self)->selectedListIndex]))
            ->UpdateNavigationDelay();
    if (AsCPangya2Shop(self)->categoryIndex != kEquipment || AsCPangya2Shop(self)->selectedListIndex != kConsumableList)
        ((CPlayerStatus*)(AsCPangya2Shop(self)->playerStatus))->Update();
    else
        CItemCart::Update(AsCPangya2Shop(self)->itemCart);
    ((CClubHouseMsg*)(AsCPangya2Shop(self)->productMessage))->UpdateReveal();
    ((CClubHouseMsg*)(AsCPangya2Shop(self)->subMenuMessage))->UpdateReveal();
    ((CPangya2Shop*)self)->ProcessProductSelection();
    return 0;
}

// Open the club dialog from its button or logical action, warning about unowned outfit parts.
// Returns 1 only when opened, 0 if previews are pending or no trigger fired.
// Requires a nonempty activated list on CollectActivated success; only its first
// element is considered. Unowned parts select mode 2; otherwise mode 0.
// Readiness calls ShopPreview_PollReady -> CAsyncExhibition_PollReady, including
// the optional preloader's virtual readiness hook. Do not cache preview/layout
// borrows across that call; this function reloads self's fields afterwards.
// Collection/ownership checks have the synchronous lifetime contract documented
// at ProcessActivatedElement below; dialog mode-setting does not unload the shop.
int CPangya2Shop::TryOpenClubDialog() {
    Controller* self = (Controller*)this;
    int (*check)(Controller*) = CPangya2Shop_AreSelectedPreviewsReady;
    if (!check(self))
        return 0;
    ActivatedList list;
    // Original 0x802C5880: activate from the selected item or the input action.
    if ((Layout_CollectActivated(self->shopLayout, &list, 0) &&
         *(int*)WordVector_GetElementAddress(&list, 0) == kOpenClubDialogElement) ||
        Input_IsActionPressed(LayoutManager_GetInstance(), kOpenClubDialogAction, -1) != 0) {
        if (((CPangya2Shop*)self)->HasUnownedSelectedParts())
            ((CClubDialog*)(self->clubDialog))->SetMode(kDiscardOutfitDialog);
        else
            ((CClubDialog*)(self->clubDialog))->SetMode(0);
        self->clubDialogOpen = 1;
        return 1;
    }
    return 0;
}

// Apply the activated category unless an unowned outfit must be resolved first.
// Collected first element is a category ID (0 character, 1 equipment, 2 caddie).
// Returns 1 for a consumed activation, including a deferred change that opens the
// club dialog. In that case this helper does NOT commit categoryIndex.
// Borrow lifetime: Layout_CollectActivated (layout.cpp) queries input/hit tests
// and copies integer IDs into list; it does NOT call Layout_Update or BP callbacks.
// HasUnownedSelectedParts (pangya2_shop_item_selection.cpp) scans inventory into a temporary vector,
// with no preview/layout replacement. Thus no invalidating callback occurs
// between collecting the ID and reading it here. Layout_Update, preview virtual
// updates, and BpObject_TryInvokeCurrentCallback in the outer update/draw CAN run
// resource/script code; retain no borrowed preview/group pointer across them.
// Only self and the copied integer survive this helper. Shop lifecycle must stay
// serialized (no concurrent unload); no reference count or lock pins its children.
int CPangya2Shop::ProcessActivatedElement() {
    Controller* self = (Controller*)this;
    ActivatedList list;
    if (!Layout_CollectActivated(self->shopLayout, &list, 0))
        return 0;
    if (((CPangya2Shop*)self)->HasUnownedSelectedParts()) {
        self->clubDialogOpen = 1;
        return 1;
    }
    self->categoryIndex = *(int*)WordVector_GetElementAddress(&list, 0);
    self->selectedListIndex = -1;
    return 1;
}

// Cycle shop categories or handle category activation, confirming unowned outfits before switching.
// Returns 1 on consumed input/deferred confirmation; 0 on no action or unready
// preview. Previous/next actions only cycle when selectedListIndex==-1; activated
// elements are checked first. Sound/list rebuild occur only for a committed change.
int CPangya2Shop::HandleCategoryNavigation() {
    Controller* self = (Controller*)this;
    int (*check)(Controller*) = CPangya2Shop_AreSelectedPreviewsReady;
    if (!check(self))
        return 0;
    if (!((CPangya2Shop*)self)->ProcessActivatedElement()) {
        if (self->selectedListIndex != -1)
            return 0;
        if (Input_IsActionPressed(LayoutManager_GetInstance(), kPreviousCategoryAction, -1)) {
            if (((CPangya2Shop*)self)->HasUnownedSelectedParts())
                self->clubDialogOpen = 1;
            else if (self->categoryIndex >= 1)
                --self->categoryIndex;
            else
                self->categoryIndex = kCaddies;
        } else if (Input_IsActionPressed(LayoutManager_GetInstance(), kNextCategoryAction, -1)) {
            if (((CPangya2Shop*)self)->HasUnownedSelectedParts())
                self->clubDialogOpen = 1;
            else if (self->categoryIndex <= 1)
                ++self->categoryIndex;
            else
                self->categoryIndex = kCharacters;
        } else
            return 0;
    }
    if (self->clubDialogOpen) {
        ((CClubDialog*)(self->clubDialog))->SetMode(kDiscardOutfitDialog);
        return 1;
    }
    Sound_PlayByName(g_soundManager, shopStrings + kPageChangeSound, 0, 0, 0, 0, kFloat_0_5_shopSound, kFloat_200_Shop,
                     kFloat_1_805E0918);
    ((CPangya2Shop*)self)->RefreshProductLists();
    return 1;
}

// Query the selected preview now; callbacks between draw layers may change state.
// An absent caddie/unsupported equipment list leaves the mode disabled.
static inline int ShopSelectedPreviewRotationMode(Controller* self) {
    int r;
    r = 0;
    if (AsCPangya2Shop(self)->categoryIndex == kCharacters)
        r = ModelOperationBase_IsInteractiveRotationEnabled(AsCPangya2Shop(self)->charaModelOperation);
    else if (AsCPangya2Shop(self)->categoryIndex == kCaddies) {
        if (!AsCPangya2Shop(self)->noCaddieSelected)
            r = ModelOperationBase_IsInteractiveRotationEnabled(AsCPangya2Shop(self)->caddieModelOperation);
    } else if (AsCPangya2Shop(self)->categoryIndex == kEquipment) {
        if (AsCPangya2Shop(self)->selectedListIndex == kClubSetList)
            r = ModelOperationBase_IsInteractiveRotationEnabled(AsCPangya2Shop(self)->primaryItemModelOperation);
        else
            r = (AsCPangya2Shop(self)->selectedListIndex == kBallList)
                    ? ModelOperationBase_IsInteractiveRotationEnabled(AsCPangya2Shop(self)->secondaryItemModelOperation)
                    : r;
    }
    return r;
}

// Draw the currently selected borrowed preview; no-caddie and unsupported lists draw nothing.
static inline void ShopDrawSelectedPreview(Controller* self) {
    if (AsCPangya2Shop(self)->categoryIndex == kCharacters)
        ModelOperationBase_Draw(AsCPangya2Shop(self)->charaModelOperation);
    else if (AsCPangya2Shop(self)->categoryIndex == kCaddies) {
        if (!AsCPangya2Shop(self)->noCaddieSelected)
            ModelOperationBase_Draw(AsCPangya2Shop(self)->caddieModelOperation);
    } else if (AsCPangya2Shop(self)->categoryIndex == kEquipment) {
        if (AsCPangya2Shop(self)->selectedListIndex == kClubSetList)
            ModelOperationBase_Draw(AsCPangya2Shop(self)->primaryItemModelOperation);
        else if (AsCPangya2Shop(self)->selectedListIndex == kBallList)
            ModelOperationBase_Draw(AsCPangya2Shop(self)->secondaryItemModelOperation);
    }
}

// Draw shop controls, preview layers, status text, and every flagged modal dialog.
// Rotation mode draws the preview after messages; ordinary mode draws it before.
// Re-query the mode after callbacks, then draw dialogs in the explicit order below.
// restores cached GX scissor after the final initialized save-menu draw. Do not
// shrink the scope to just ModelOperationBase_Draw or cache rotation across Bp
// callbacks. Borrowed layouts/previews may alter render state; no ownership moves.
void CPangya2Shop::Draw() {
    Controller* self = (Controller*)this;
    // Phase 1: controls and selected product list precede the lower preview layer.
    int interactiveRotationEnabled;
    BpObject_TryInvokeCurrentCallback(AsCPangya2Shop(self)->uiGroup, 0);
    BpObject_Draw(AsCPangya2Shop(self)->uiGroup);
    Layout_DrawWithOpacity(AsCPangya2Shop(self)->shopLayout, kFloat_1_805E0918);
    ((CProductsCategory*)(AsCPangya2Shop(self)->productsCategory))->Draw();
    if (AsCPangya2Shop(self)->selectedListIndex != -1) {
        void* p = ((void**)AsCPangya2Shop(self)->productLists)[AsCPangya2Shop(self)->selectedListIndex];
        if (p)
            ((CShopProductsTab*)p)->Draw();
    }
    UITextTable_Draw(AsCPangya2Shop(self)->textTable);
    interactiveRotationEnabled = ShopSelectedPreviewRotationMode(self);
    // Noninteractive previews are drawn below the status/message layer.
    if (interactiveRotationEnabled == 0) {
        ShopDrawSelectedPreview(self);
    }
    // Phase 2: script/status/messages separate the ordinary and interactive previews.
    BpObject_TryInvokeCurrentCallback(AsCPangya2Shop(self)->scriptObject, 0);
    BpObject_Draw(AsCPangya2Shop(self)->scriptObject);
    if (AsCPangya2Shop(self)->categoryIndex == kEquipment && AsCPangya2Shop(self)->selectedListIndex == kConsumableList)
        ((CItemCart*)(AsCPangya2Shop(self)->itemCart))->Draw();
    else
        ((CPlayerStatus*)(AsCPangya2Shop(self)->playerStatus))->Draw();
    ((CClubHouseMsg*)(AsCPangya2Shop(self)->productMessage))->DrawText();
    ((CClubHouseMsg*)(AsCPangya2Shop(self)->subMenuMessage))->DrawText();
    interactiveRotationEnabled = ShopSelectedPreviewRotationMode(self);
    if (interactiveRotationEnabled != 0) {
        ShopDrawSelectedPreview(self);
    }
    // Phase 3: every flagged modal draws, unlike update's first-match dispatch.
    if (AsCPangya2Shop(self)->itemInfoOpen)
        ((CItemInfo*)(AsCPangya2Shop(self)->itemInfo))->Draw();
    if (AsCPangya2Shop(self)->purchaseDialogOpen)
        ((CPurchaseDialog*)(AsCPangya2Shop(self)->purchaseDialog))->Draw();
    if (AsCPangya2Shop(self)->clubSubMenuOpen)
        ((CClubSubMenu*)(AsCPangya2Shop(self)->clubSubMenu))->Draw();
    if (AsCPangya2Shop(self)->clubLvUpDialogOpen)
        ((CClubLvUpDialog*)(AsCPangya2Shop(self)->clubLvUpDialog))->Draw();
    if (AsCPangya2Shop(self)->clubDialogOpen)
        ((CClubDialog*)(AsCPangya2Shop(self)->clubDialog))->Draw();
    if (AsCPangya2Shop(self)->saveMenu)
        MPBase1_DrawIfInitialized(AsCPangya2Shop(self)->saveMenu);
}
}

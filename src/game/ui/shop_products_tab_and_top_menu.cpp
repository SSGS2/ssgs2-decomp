#include <decomp/static_init.h>
extern "C" char s_bpCtrlDtorRegistration_802C470C[];
extern "C" char s_fadeOverlayDtorRegistration[];
extern "C" char s_nowReloadingDtorRegistration_802C470C[];
// Byte offsets preserve the pooled-string storage and original relocation addends.
enum ShopStringsOffset {
    kShopStrings_club_base_spd = 0xE, // club_base.spd
    kShopStrings_club_base_b2d = 0x1C, // club_base.b2d
    kShopStrings_club_setcos_info_b2d = 0x2A, // club_setcos_info.b2d
};

// Byte offsets preserve the pooled-string storage and original relocation addends.
enum ShopProductsTabStringsOffset {
    kShopProductsTabStrings_club_iconbase_spd = 0x10, // club_iconbase.spd
    kShopProductsTabStrings_club_new_b2d = 0x22, // club_new.b2d
    kShopProductsTabStrings_new_anime = 0x2F, // new_anime
    kShopProductsTabStrings_num_shop_00_gsr = 0x39, // num_shop_00.gsr
    kShopProductsTabStrings_ui_error = 0x49, // ui_error
    kShopProductsTabStrings_ui_cursol_move2 = 0x52, // ui_cursol_move2
    kShopProductsTabStrings_std_exception_std_logic_error_std_out_of_range = 0x62, // !std::exception!!std::logic_error!!std::out_of_range!!
};

// Byte offsets preserve the pooled-string storage and original relocation addends.
enum TopMenuResourceStringsOffset {
    kTopMenuResourceStrings_in = 0x33, // in
    kTopMenuResourceStrings_R_MSG_info = 0x36, // R_MSG_info
    kTopMenuResourceStrings_out = 0x41, // out
};

enum ShopItemKind { kItemCharacter = 1, kItemPart = 2, kItemClubSet = 4, kItemBall = 5,
    kItemConsumable = 6, kItemCaddie = 7, kItemCaddieCostume = 8, kItemHairStyle = 15, kItemAuxPart = 28 };
enum ShopInventoryLimit { kCharacterCount = 8, kPartCount = 0x30E, kHairStyleCount = 0x33,
    kCaddieCount = 8, kCaddieCostumeCount = 32, kClubSetCount = 15, kBallCount = 11,
    kEquipmentPartSlotCount = 24, kVisibleProductSlots = 5, kNoQuantity = -1, kTypeIdKindShift = 26 };
// Main-menu protocol: element 1 -> submenu mode 0; 13 -> mode 2; 17 -> mode 3.
// Elements 5/9 publish result 0/11, set resultReady and request exit. The other
// element cases in HandleSelection keep state; result/submenu game labels have
// not been established from this unit. Every accepted activation starts "out".
enum TopMenuActivationElement {
    kTopMenuOpenSubmenu0 = 1, kTopMenuPublishResult0 = 5,
    kTopMenuPublishResult11 = 9, kTopMenuOpenSubmenu2 = 13,
    kTopMenuOpenSubmenu3 = 17
};
enum TopMenuProtocolCode {
    kTopMenuSubmenu0 = 0, kTopMenuSubmenu2 = 2, kTopMenuSubmenu3 = 3,
    kTopMenuResult0 = 0, kTopMenuResult11 = 11, kNoActivatedMenuElement = -1
};

enum ShopVisibleSlotOffset {
    kVisibleProductCenterSlot = 2, kFirstVisibleRelativeSlot = -2,
    kVisibleRelativeSlotEnd = 3
};

// secondaryLayout value-state encodes ownership/equipment; it is not a bit mask.
enum ShopProductIndicator { kProductUnavailable = 0, kProductEquippedOnly = 1,
    kProductOwned = 2, kProductOwnedAndEquipped = 3 };
enum ShopCategoryBinding { kShopListCount = 5, kShopListPitchPixels = 0x22,
    kShopCartItemCategory = 2, kShopCharacterBuilderMode = 5,
    kShopCaddieBuilderMode = 2, kShopItemBuilderMode = 3 };

enum ShopInputAction { kShopNavigateLeft = 2, kShopNavigateRight = 3 };

#include "iff/IffStruct.h"
#include "classes/SavePlayerRecord.h"

// Partial save-record view for the shop: new-item bits are separate from ownership.
// Character equipment rows are 0x34 bytes; caddie rows are 8 bytes, club rows 2 bytes.
struct ShopCharacterEquipment {
    unsigned char flags;
    char reserved;
    short partIndices[24];
    signed char hairStyleIndex;
    signed char auxPartIndex;
};
struct ShopCaddieOwnership { unsigned char flags; char reserved[7]; };
struct ShopClubSetOwnership { unsigned char flags; char reserved; };
// Status schema (all bitset indices are inventory indices, never full IFF IDs):
// category       persistent NEW      ownership in working playerRecord   equipped
// character      newCharacters       characters[i].flags bit6           characterIndex
// part           newParts[25]        ownedParts[25] bit i                selected row's 24 slots
// caddie         newCaddies          caddies[i].flags bit6               caddieIndex
// caddie costume newCaddieItems      ownedCaddieItems bit i               caddieItemIndex + owner
// club set       newClubSets         clubSets[i].flags bit6              clubSetIndex
// ball           newBalls            balls[i] bit6                       ballIndex
// hairstyle      none tested         ownedHairStyles[2] bit i            row.hairStyleIndex
// aux part       none tested         ownedAuxParts[i] != 0               row.auxPartIndex
// consumable     none tested         itemQuantities[i] > 0               8 equippedItemIndices
// NEW means persistent new bit set AND persistent ownership clear. Ownership and
// equipped tests use the tab's working record, which may differ during previews.
// Bit i lives in word[i>>5] at (1u<<(i&31)); row flag bit6 is isolated by shifts.
// Zero type ID represents no caddie. Kind8 with low13 bits zero denotes a bare
// caddie (owner index bits21..25), otherwise a costume. Status flags are cleared
// together by UpdateProductStatus; specialized helpers only set positive results.
struct ShopSaveFields {
    char prefix[0x28];
    unsigned int pangBalance; // Shop copies this balance into its working save at self+0x40.
    signed char characterIndex, clubSetIndex, ballIndex, caddieIndex, caddieItemIndex;
    signed char equippedItemIndices[8];
    char reserved039[0x73];
    unsigned int ownedParts[25];
    unsigned int ownedCaddieItems;
    unsigned int ownedHairStyles[2];
    short itemQuantities[13];
    signed char ownedAuxParts[10];
    unsigned int newCharacters;
    unsigned int newParts[25];
    char reserved1A8[0x18];
    unsigned int newClubSets, newBalls, newCaddies, newCaddieItems;
    char reserved1D0[0x8C];
    unsigned int recordBlock25C[28]; // Copied as words; meanings within this record block remain unresolved.
    ShopCharacterEquipment characters[8];
    ShopCaddieOwnership caddies[8];
    ShopClubSetOwnership clubSets[15];
    signed char balls[11];
    char reserved4D5[3];
    unsigned int recordBlock4D8[2];
    unsigned int recordBlock4E0[2];
    unsigned int practiceFlags;
    unsigned short recordBlock4EC[414];
};
#define AsShopSaveFields(p) ((ShopSaveFields*)(p))
#define SHOP_SAVE_OFF(f, o) typedef char ShopSave_off_##f[((int)&((ShopSaveFields*)0)->f == (o)) ? 1 : -1]
SHOP_SAVE_OFF(ownedParts, 0xAC);
SHOP_SAVE_OFF(ownedCaddieItems, 0x110);
SHOP_SAVE_OFF(ownedHairStyles, 0x114);
SHOP_SAVE_OFF(itemQuantities, 0x11C);
SHOP_SAVE_OFF(ownedAuxParts, 0x136);
SHOP_SAVE_OFF(newCharacters, 0x140);
SHOP_SAVE_OFF(newParts, 0x144);
SHOP_SAVE_OFF(newClubSets, 0x1C0);
SHOP_SAVE_OFF(newBalls, 0x1C4);
SHOP_SAVE_OFF(newCaddies, 0x1C8);
SHOP_SAVE_OFF(newCaddieItems, 0x1CC);
SHOP_SAVE_OFF(characters, 0x2CC);
SHOP_SAVE_OFF(caddies, 0x46C);
SHOP_SAVE_OFF(clubSets, 0x4AC);
SHOP_SAVE_OFF(balls, 0x4CA);
SHOP_SAVE_OFF(recordBlock25C, 0x25C);
SHOP_SAVE_OFF(practiceFlags, 0x4E8);
SHOP_SAVE_OFF(recordBlock4EC, 0x4EC);
#undef SHOP_SAVE_OFF
typedef char ShopSave_size[(sizeof(ShopSaveFields) == 0x828) ? 1 : -1];

struct ShopQuantityDrawFields { char prefix[0x28]; int enabled; };
#include "classes/CSharedDoc.h"
#define AsCSharedDoc(p) ((CSharedDoc*)(p))
#include "sdk_protos.h"
#include "game/types.h"
#include "game/singletons.h"
#include "classes/CPangya2Shop.h"
#include "classes/CPangya2TopMenu.h"
#include "classes/CShopProductsTab.h"

#define AsCShopProductsTab(p) ((CShopProductsTab*)(p))

#define AsCPangya2TopMenu(p) ((CPangya2TopMenu*)(p))

#define AsCPangya2Shop(p) ((CPangya2Shop*)(p))

struct Vtx802C { float x, y, z; unsigned char r, g, b, a; float u, v; };
struct Quad802C { Vtx802C q[4]; };

extern "C" {
void appFreeUni__FPv(void*);

// The raw vector's explicit normal cleanup releases this unwind guard first.
struct ActivatedUnwindCleanup {
    volatile unsigned int* words;
    bool active;
    // Guard an activated-element vector until explicit normal cleanup takes over.
    explicit ActivatedUnwindCleanup(volatile void* vector)
        : words((volatile unsigned int*)vector), active(true) {}
    // Disarm the unwind guard before normal cleanup.
    void release() { active = false; }
    // Free the activated-element allocation only while the guard remains armed.
    ~ActivatedUnwindCleanup() {
        if (active && words[0]) {
            words[1] = 0;
            appFreeUni__FPv((void*)words[0]);
        }
    }
};

void* Layout_Init(void*, int);
int CPangya2TopMenu_HandleSelection(char*);
void* CPangya2TopMenu_dtor(void*, int);
void Layout_Update(void*);
extern float kFloat_6_2831855;
extern float s_topMenuFullTurnAngle; // s_topMenuFullTurnAngle
int CShopProductsTab_UpdateProductStatus(void*, unsigned int);
void Layout_SetElementPlayerFlag(void*, int, int, int);
void Layout_SetElementValueState(void*, int, int);
void Layout_SetElementPlayerFlagsAndState(void*, int, int);
int Layout_PollLoadComplete(void*);
int CTopMenuBase_PollLoad(void*);
int CShopProductsTab_ClearProductFlag(void*, void*);
int CShopProductsTab_UpdateSlotIndicators(void*, int);
void __ct__7CBpCtrlFv(void*);
void CBpCtrl_dtor(void*);
int CBpCtrl_PollAllEntriesLoaded(void*);
extern signed char s_bpCtrlInitialized;
extern char s_bpCtrl[];
extern char s_bpCtrlDtorRegistration_ShopTab[]; // s_bpCtrlDtorRegistration_ShopTab
void Layout_DrawWithOpacity(void*, float);
void Engine_GetScissorRect(void*);
void Engine_SetScissorRect(void*);
void BpObject_TryInvokeCurrentCallback(void*, int);
void BpObject_Draw(void*);
void UITextTable_Draw(void*);
int CShopProductsTab_DrawProductQuads(const void*);
int CShopProductsTab_HandleNavigationInput(void*);
void BpObject_SetEnabled(void*, int);
int CTopMenuBase_GetFirstActivatedElement(void*);
void CTopMenuBase_EnablePreviousElement(void*, int);
void BpObject_StartScriptByName(void*, void*, int);
extern char topMenuResourceStrings[];
extern const float kFloat_1_ShopTab;
extern int shopProductScissorRect[]; // shopProductScissorRect
int CShopProductsTab_ApplyDrawOffsetY(void*);
void CFontNumDraw_RegisterEntries(void*, void*, int, void*, void*);
void MenuMessage_ResetTextState(void*);
void CTopMenuBase_SyncItemPosition(void*, int);
void* BpObject_GetOrCreateNamedGroup(void*, void*);
int UIGroup_GetTintColor(void*);
void MenuMessage_SetPosition(void*, void*);
void Layout_GetElementPosition(void*, int, void*);
void MenuMessage_SetColorMultiplier(void*, void*);
void Layout_Destroy(void*, int);
void CBpCtrl_DeleteEntryById(void*, int);
void UITextTable_ClearEntries(void*);
void PointerVector_InitReserved(void*);
void PointerArray_DeleteValue(void*, void*);
void __dl__FPv(void*);
void* __nw__FUl(unsigned long);
void Layout_LoadExtended(void*, int, void*);
void CBpCtrl_CreateEntry(void*, void*, void*, int);
void* IdMap_Lookup(void*, int);
void* CFontNumDraw_CreateInVector(void*);
extern char shopProductsTabStrings[];
extern char shopProductQuantityGroupNames[]; // shopProductQuantityGroupNames
extern char __vt__16CShopProductsTab[];
extern char s_fontNumDrawRegistry[];
extern char s_fontNumRegistryDtorRegistration_ShopTab[]; // s_fontNumRegistryDtorRegistration_ShopTab
void PointerVector_DestroyStorage(void*);
extern signed char s_pointerVectorInitialized;
extern int topMenuTexturedQuadTemplate[]; // topMenuTexturedQuadTemplate
extern int topMenuOverlayQuadTemplate[]; // topMenuOverlayQuadTemplate
extern signed char s_homeMenuInitialized;
extern double kUnsignedIntToDoubleBias_TopMenuDraw; // kUnsignedIntToDoubleBias_TopMenuDraw
extern float kFloat_1_TopMenuDraw; // kFloat_1_TopMenuDraw
void HomeButtonMenu_InitializeState(void*);
void HomeButtonResources_dtor(void);
void HomeMenu_OverrideBlockedState(void*, int);
void CTopMenuBase_ResetItemPlayerStates(void*);
void CTopMenuBase_HideItemGroup(void*, int);
void Layout_SetElementEnabled(void*, int, void*);
void CTopMenuBase_ReleaseTextureResource(void*);
int BpSprite_GetTextureDescription(void*);
void* GX_GetPrimitiveCount(int, int);
void Engine_SetFlexibleVertexFormat(int);
void DrawPrimitiveUP(int, void*, void*, int);
int Layout_ReadElementRect(void*, int, void*);
extern Quad802C shopProductQuadTemplate; // shopProductQuadTemplate
extern double kSignedIntToDoubleBias_ShopCarousel; // kSignedIntToDoubleBias_ShopCarousel
void Engine_SetTextureStageResource(int, int);
void Engine_SetTexturePalette(int, int);
void MenuMessage_DrawText(void*);
void MPBase1_DrawIfInitialized(void*);
void IFadeMenuBase_StartMenuFadeOut(void*);
extern char s_homeButtonResourceState[];
extern char s_homeMenuDtorRegistration_TopMenu[]; // s_homeMenuDtorRegistration_TopMenu
extern signed char s_bpCtrlHandleInitialized;
extern int s_bpCtrlHandle[2];
extern char s_bpCtrlHandleDtorRegistration_TopMenu[]; // s_bpCtrlHandleDtorRegistration_TopMenu
void CBpCtrlHandle_Dtor(void);
void CBpCtrlHandle_SetScriptIndex(void*, int);
int Layout_CollectActivated(void*, void*, int);
int CShopProductsTab_HandleNavigationActivation(void*);
void InputManager_Init(void*);
int Input_IsActionPressed(void*, int, int);
void appFreeUni__FPv(void*);
void Sound_PlayByName(void*, const char*, int, int, int, int, float, float, float);
int ItemInventory_FindHairStyleIndex(void*, unsigned int);
int ItemInventory_FindAuxPartIndex(void*, unsigned int);
int ItemInventory_FindItemIndex(void*, unsigned int);
void* __nwa__FUl(unsigned int);
void __dla__FPv(void*);
void __dt__Q23std12out_of_rangeFv(void*, int);
extern void* g_sharedDoc;
extern char __vt__Q23std11logic_error[];
extern char s_sharedCharDeleterVtable[];
extern char __vt__Q23std12out_of_range[];
extern char shopHairOwnershipBoundsError[]; // shopHairOwnershipBoundsError
int CShopProductsTab_UpdateCharacterStatus(void*, const char*);
int CShopProductsTab_UpdatePartStatus(void*, void*);
int CShopProductsTab_UpdateCaddieStatus(void*, void*);
int CShopProductsTab_UpdateCaddieItemStatus(void*, void*);
int CShopProductsTab_UpdateClubSetStatus(void*, void*);
int CShopProductsTab_UpdateBallStatus(void*, void*);
extern void* g_soundManager;
extern const float kFloat_0_5_ShopCarouselSound; // kFloat_0_5_ShopCarouselSound
extern const float kFloat_200_ShopCarouselSound; // kFloat_200_ShopCarouselSound
extern signed char s_inputManagerInitialized;
extern char s_gameInputManager[];
void InputManager_Dtor();
extern char s_inputManagerDtorRegistration_ShopTab[]; // s_inputManagerDtorRegistration_ShopTab
void SaveData_Ctor(void*);
void* SaveData_GetCurrentRecord(void*);
void SaveRecord_CopyCtor(void*, void*);
int ItemInventory_FindCharacterIndex(void*, void*);
int ItemInventory_FindPartIndex(void*, void*);
int ItemInventory_FindCaddieIndex(void*, void*);
int ItemInventory_FindClubSetIndex(void*, void*);
int ItemInventory_FindCaddieItemIndex(void*, void*);
extern char bitsetTestOutOfRangeMessage[];
extern signed char s_saveDataInitialized;
extern char s_saveData[];
extern char shopCharacterOrCaddieTestBoundsError[]; // shopCharacterOrCaddieTestBoundsError
extern char shopClubSetTestBoundsError[]; // shopClubSetTestBoundsError
extern char bitsetTestRangeError[];
extern "C" void __dt__Q23std9exceptionFv(void*, int);
extern char shopBallTestBoundsError[]; // shopBallTestBoundsError
extern int shopProductBadgeGroupIds[]; // shopProductBadgeGroupIds
extern float s_shopFullTurnAngle; // s_shopFullTurnAngle
int ItemInventory_FindBallIndex(void*, void*);
extern char shopCharacterOrCaddieClearBoundsError[]; // shopCharacterOrCaddieClearBoundsError
extern char bitset782ResetRangeErrorText[];
extern char shopCaddieCostumeClearBoundsError[]; // shopCaddieCostumeClearBoundsError
extern char shopClubSetClearBoundsError[]; // shopClubSetClearBoundsError
extern char shopBallClearBoundsError[]; // shopBallClearBoundsError
char* strcpy(char*, const char*);
int Layout_SetModeByte(void*, int);
void Layout_SetElementPosition(void*, int, void*);
void Layout_WriteElementRect(void*, int, void*);
void* UIGroup_GetOrCreateById(void*, int);
void UIGroup_SetEnabled(void*, int);
void UIGroup_GetPosition(void*, void*);
void UIGroup_SetPosition(void*, void*);
int NumericTextTable_ClearEntryText(void*, int);
void UITextTable_SetInteger(void*, int, int);
void MPBase1_Ctor(void*);
void SaveData_InitPlayerRecord(void*, void*);
void* CProductsCategory_Ctor(void*, int);
void __ct__20CModelOperationCharaFv(void*);
void CModelOperationCaddie_Ctor(void*);
void* CModelOperationItem_Ctor(void*, int);
void* __ct__18CBuildProductCharaFiUc(void*, int, int);
void* CBuildProductCaddie_Ctor(void*, int, int);
void* CBuildProductItem_Ctor(void*, int, int);
void __ct__13CClubHouseMsgFv(void*);
void CClubSubMenu_Ctor(void*);
void CClubDialog_Ctor(void*);
void CPurchaseDialog_Ctor(void*);
void CItemInfo_Ctor(void*);
void CNowReloading_Init(void*);
void Allocation_Dtor(void*);
int LoadingIndicator_IsAbsentOrState3(void*);
void LoadingIndicator_CreatePlayer(void*);
void* ResourceTable_FindEntryByName(void*, void*, void*);
void* CPackTextureResource_Init(void*, void*, int, void*);
void CModelOperationChara_OpenResource(void*);
void CModelOperationCaddie_OpenResource(void*);
void CModelOperationItem_OpenResource(void*);
void CBuildProduct_SelectSaveRecord(void*, void*);
void ShopItemList_BuildProductTextures(void*);
void CBuildProductChara_LoadProductTextures(void*, int);
void CBuildProductCaddie_LoadIconGroups(void*, int);
void* CBuildProduct_GetProductRecords(void*, int);
void* CBuildProduct_GetProductDrawEntries(void*, int);
void* CPlayerStatus_Ctor(void*, void*);
void* CBuildProduct_GetProductCount(void*, int);
void* CItemCart_Ctor(void*, void*, void*);
void CItemCart_SetItemIcons(void*, void*);
void CItemCart_SetItemRecords(void*, void*);
void* CClubLvUpDialog_Ctor(void*, void*);
void* CPackedFile_Create(void*);
extern signed char s_nowReloadingInitialized;
extern signed char s_resourceTableInitialized;
extern void* s_resourceTableStorage;
extern char shopStrings[];
extern void* kShopPreloadArchiveNames[];
// Three archive paths loaded into packedFiles during shop construction.
extern char __vt__12CPangya2Shop[];
void CNowReloading_dtor(void*);
extern char s_inputManagerDtorRegistration_80516C10[];
extern double kSignedIntToDoubleBias_ShopSetup; // kSignedIntToDoubleBias_ShopSetup
extern unsigned s_nowReloading;
void* memcpy(void*, const void*, unsigned long);
}

// Use the caller-provided placement address without allocating.
inline void* operator new(size_t, void* p) { return p; }

struct VObjBF3F0 {
    virtual void v0();
    virtual void v1();
    virtual void v2();
    virtual void v3();
    virtual int call();
};

struct ShopTabLayout {
    char data[0x5c];
    // Initialize the embedded layout for the requested mode.
    ShopTabLayout(int mode) { Layout_Init(this, mode); }
};

namespace std {
struct TargetMessageGuard {
    char* pointer;
    char* allocator;
    // Free exception text if its shared owner has not adopted it.
    ~TargetMessageGuard() { if (pointer) __dla__FPv(pointer); }
};
struct TargetMessageOwner {
    void* vtable;
    int references, owners;
    char* message;
    // Adopt exception text with initial shared ownership counts.
    TargetMessageOwner(char* p) {
        references = 1;
        owners = 1;
        vtable = s_sharedCharDeleterVtable;
        message = p;
    }
};
class logic_error {
public:
    void* vtable;
    char* message;
    TargetMessageOwner* owner;
    // Copy the error message into an owned shared exception representation.
    logic_error(const char* text) {
        vtable = __vt__Q23std11logic_error;
        struct Allocator { char value; char padding[3]; } allocator;
        allocator.value = 0;
        unsigned int len = strlen(text) + 1;
        char* p = (char*)__nwa__FUl(len * 2 - len);
        message = p;
        {
            TargetMessageGuard guard = { p, &allocator.value };
            owner = new TargetMessageOwner(p);
            guard.pointer = 0;
        }
        strcpy(message, text);
    }
    // Destroy the exception base without freeing this object.
    ~logic_error() { __dt__Q23std9exceptionFv(this, 0); }
};
class out_of_range : public logic_error {
public:
    // Select the out-of-range vtable after constructing the logic-error base.
    out_of_range(const char* text) : logic_error(text) { vtable = __vt__Q23std12out_of_range; }
    ~out_of_range();
};
}
struct OutOfRangeStorage { void* vtable; char* message; void* owner; };

struct TargetBitReference { unsigned int* word; unsigned int index; };
struct TargetSlotRecord { char padding[0x2c]; signed char selected; };
struct TargetSlotSelf { char padding[0x38]; TargetSlotRecord* currentRecord; unsigned char flag[3]; };

#pragma does_not_return __throw

extern "C" {

#pragma use_lmw_stmw on
#pragma opt_unroll_loops off
// Draw the background quads, then the active menu or its submenu.
void CPangya2TopMenu_Draw(char* self) {
    struct Blk96 { int v[24]; };
    void* b;
    void* a;
    unsigned int* dims = (unsigned int*)BpSprite_GetTextureDescription(AsCPangya2TopMenu(self)->menuTexture);
    ((VObjBF3F0*)AsCPangya2TopMenu(self)->menuTexture)->v3();
    int verts1[24];
    *(Blk96*)verts1 = *(Blk96*)(void*)topMenuTexturedQuadTemplate; // topMenuTexturedQuadTemplate
    *(float*)&verts1[6] = (float)dims[0];
    *(float*)&verts1[13] = (float)dims[1];
    *(float*)&verts1[18] = (float)dims[0];
    *(float*)&verts1[19] = (float)dims[1];
    a = GX_GetPrimitiveCount(4, 152);
    Engine_SetFlexibleVertexFormat(0x11f005);
    DrawPrimitiveUP(0x98, a, verts1, 0x18);
    Engine_SetTextureStageResource(0, 0);
    Engine_SetTexturePalette(0, 0);
    int verts2[24];
    *(Blk96*)verts2 = *(Blk96*)(void*)topMenuOverlayQuadTemplate; // topMenuOverlayQuadTemplate
    *(float*)&verts2[6] = (float)dims[0];
    *(float*)&verts2[13] = (float)dims[1];
    *(float*)&verts2[18] = (float)dims[0];
    *(float*)&verts2[19] = (float)dims[1];
    b = GX_GetPrimitiveCount(4, 152);
    Engine_SetFlexibleVertexFormat(0x11f005);
    DrawPrimitiveUP(0x98, b, verts2, 0x18);
    if (!AsCPangya2TopMenu(self)->subMenuActive || AsCPangya2TopMenu(self)->scriptRunning) {
        BpObject_Draw(AsCPangya2TopMenu(self)->bpObject);
        Layout_DrawWithOpacity(AsCPangya2TopMenu(self)->layout, kFloat_1_TopMenuDraw); // kFloat_1_TopMenuDraw
        MenuMessage_DrawText(AsCPangya2TopMenu(self)->message);
        if (AsCPangya2TopMenu(self)->optionDialog) MPBase1_DrawIfInitialized(AsCPangya2TopMenu(self)->optionDialog);
    } else if (AsCPangya2TopMenu(self)->subMenu) MPBase1_DrawIfInitialized(AsCPangya2TopMenu(self)->subMenu);
}

// Initialize menu scripts, item positions, message tint and the global script handle.
void CPangya2TopMenu_StartMenu(char* self) {
    if (((int&)Sing_805DC820()) == 0) {
        if (s_homeMenuInitialized == 0) {
            HomeButtonMenu_InitializeState(s_homeButtonResourceState);
            __register_global_object(s_homeButtonResourceState, (void*)HomeButtonResources_dtor, s_homeMenuDtorRegistration_TopMenu);
            s_homeMenuInitialized = 1;
        }
        ((int&)Sing_805DC820()) = (int)s_homeButtonResourceState;
    }
    HomeMenu_OverrideBlockedState((void*)((int&)Sing_805DC820()), 0);
    CTopMenuBase_ResetItemPlayerStates(self + 0x18);
    if (AsCPangya2TopMenu(self)->subMenuMode == -1) {
        BpObject_StartScriptByName(AsCPangya2TopMenu(self)->bpObject, topMenuResourceStrings + kTopMenuResourceStrings_in, 1);
        AsCPangya2TopMenu(self)->scriptRunning = 1;
    }
    int i = 0;
    do { CTopMenuBase_HideItemGroup(self + 0x18, i); CTopMenuBase_SyncItemPosition(self + 0x18, i); ++i; } while (i < 20);
    BpObject_TryInvokeCurrentCallback(AsCPangya2TopMenu(self)->bpObject, 0);
    int sc[2] = {0, 0};
    Layout_GetElementPosition(AsCPangya2TopMenu(self)->layout, 20, sc);
    MenuMessage_SetPosition(AsCPangya2TopMenu(self)->message, sc);
    void* group = BpObject_GetOrCreateNamedGroup(AsCPangya2TopMenu(self)->bpObject, topMenuResourceStrings + kTopMenuResourceStrings_R_MSG_info);
    if (group) {
        unsigned int x = UIGroup_GetTintColor(group);
        unsigned int val = ((x << 24) & 0xFF000000u) | ((x << 8) & 0x00FF0000u) | ((x >> 8) & 0x0000FF00u) | 0xFFu;
        MenuMessage_SetColorMultiplier(AsCPangya2TopMenu(self)->message, &val);
    }
    Layout_SetElementEnabled(AsCPangya2TopMenu(self)->layout, 20, 0);
    if (((int&)Sing_805DC5F8()) == 0) {
        if (s_bpCtrlHandleInitialized == 0) {
            s_bpCtrlHandle[0] = -1; s_bpCtrlHandle[1] = -1;
            __register_global_object(&s_bpCtrlHandle, (void*)CBpCtrlHandle_Dtor, s_bpCtrlHandleDtorRegistration_TopMenu); // s_bpCtrlHandleDtorRegistration_TopMenu
            s_bpCtrlHandleInitialized = 1;
        }
        ((int&)Sing_805DC5F8()) = (int)&s_bpCtrlHandle;
    }
    CBpCtrlHandle_SetScriptIndex((void*)((int&)Sing_805DC5F8()), 9);
    IFadeMenuBase_StartMenuFadeOut(self);
    CTopMenuBase_ReleaseTextureResource(self + 0x18);
}

// Report ready only after the base menu and all three menu layouts have loaded.
int CPangya2TopMenu_PollLoadComplete(void* self) {
    if (!CTopMenuBase_PollLoad((char*)&AsCPangya2TopMenu(self)->secondaryVtable)) {
        return 0;
    }
    void* layoutCursor = self;
    int layoutIndex = 0;
    while (layoutIndex < 3) {
        if (!Layout_PollLoadComplete(AsCPangya2TopMenu(layoutCursor)->firstMenuLayout)) {
            return 0;
        }
        layoutIndex = layoutIndex + 1;
        layoutCursor = (char*)layoutCursor + 4;
    }
    VObjBF3F0* obj = (VObjBF3F0*)AsCPangya2TopMenu(self)->menuTexture;
    int textureReady = obj->call();
    if (textureReady != 0) {
        return 1;
    }
    return 0;
}

// Advance the main layout and all three auxiliary menu layouts.
void CPangya2TopMenu_UpdateLayouts(void* p) {
    int i;
    void* q = p;
    Layout_Update(AsCPangya2TopMenu(q)->layout);
    i = 0;
    while (i < 3) {
        Layout_Update(AsCPangya2TopMenu(q)->firstMenuLayout);
        i = i + 1;
        q = (char*)q + 4;
    }
}

// Translate an activated menu entry into a result or submenu request and start the exit script.
int CPangya2TopMenu_HandleSelection(char* self) {
    int value = CTopMenuBase_GetFirstActivatedElement(self + 0x18);
    if (value == kNoActivatedMenuElement) return 0;
    switch (value) {
        case kTopMenuOpenSubmenu0: AsCPangya2TopMenu(self)->subMenuMode = kTopMenuSubmenu0; break;
        case kTopMenuPublishResult0: {
            AsCPangya2TopMenu(self)->result = kTopMenuResult0;
            AsCPangya2TopMenu(self)->resultReady = 1;
            break;
        }
        case kTopMenuPublishResult11: {
            AsCPangya2TopMenu(self)->result = kTopMenuResult11;
            AsCPangya2TopMenu(self)->resultReady = 1;
            break;
        }
        case kTopMenuOpenSubmenu2: AsCPangya2TopMenu(self)->subMenuMode = kTopMenuSubmenu2; break;
        case kTopMenuOpenSubmenu3: AsCPangya2TopMenu(self)->subMenuMode = kTopMenuSubmenu3; break;
        case 0: case 2: case 3: case 4: case 6: case 7: case 8: case 10: case 11: case 12: case 14: case 15: case 16: break;
    }
    if (AsCPangya2TopMenu(self)->resultReady == 0)
        AsCPangya2TopMenu(self)->subMenuActive = 1;
    else
        AsCPangya2TopMenu(self)->exitRequested = 1;
    CTopMenuBase_EnablePreviousElement(self + 0x18, value);
    BpObject_StartScriptByName(AsCPangya2TopMenu(self)->bpObject, topMenuResourceStrings + kTopMenuResourceStrings_out, 1);
    AsCPangya2TopMenu(self)->scriptRunning = 1;
    return 1;
}

// Restore the main menu, start its entrance script and reset item/message positions.
int CPangya2TopMenu_ActivateMenu(char* self) {
    BpObject_SetEnabled(AsCPangya2TopMenu(self)->bpObject, 1);
    BpObject_StartScriptByName(AsCPangya2TopMenu(self)->bpObject, topMenuResourceStrings + kTopMenuResourceStrings_in, 1);
    AsCPangya2TopMenu(self)->scriptRunning = 1;
    MenuMessage_ResetTextState(AsCPangya2TopMenu(self)->message);
    int itemIndex = 0;
    int layoutElementBase = 0;
    for (layoutElementBase = 0; layoutElementBase < 0x14; layoutElementBase++) {
        CTopMenuBase_SyncItemPosition(self + 0x18, layoutElementBase);
    }
    itemIndex = 0;
    layoutElementBase = 0;
    while (itemIndex < AsCPangya2TopMenu(self)->menuItemCount) {
        Layout_SetElementPlayerFlagsAndState(AsCPangya2TopMenu(self)->layout, layoutElementBase + 1, 0);
        Layout_SetElementPlayerFlagsAndState(AsCPangya2TopMenu(self)->layout, layoutElementBase, 1);
        Layout_SetElementValueState(AsCPangya2TopMenu(self)->layout, layoutElementBase, 0);
        layoutElementBase += 4;
        itemIndex += 1;
    }
    int sc[2];
    sc[0] = 0;
    sc[1] = 0;
    Layout_GetElementPosition(AsCPangya2TopMenu(self)->layout, 0x14, sc);
    MenuMessage_SetPosition(AsCPangya2TopMenu(self)->message, sc);
    void* obj20 = AsCPangya2TopMenu(self)->bpObject;
    void* group = BpObject_GetOrCreateNamedGroup(obj20, topMenuResourceStrings + kTopMenuResourceStrings_R_MSG_info);
    if (group != 0) {
        unsigned int x = (unsigned int)UIGroup_GetTintColor(group);
        unsigned int val = ((x << 24) & 0xFF000000u) | ((x << 8) & 0x00FF0000u) | ((x >> 8) & 0x0000FF00u) | 0xFFu;
        MenuMessage_SetColorMultiplier(AsCPangya2TopMenu(self)->message, &val);
    }
    return 1;
}

// Initialize the top-menu angle-wrap constant at startup.
void TopMenu_InitAngleWrapConstant(void) {
    s_topMenuFullTurnAngle = kFloat_6_2831855; // s_topMenuFullTurnAngle
}

// Adjust the secondary-base pointer before handling menu selection.
int CPangya2TopMenu_HandleSelectionThunk24(void* p) {
    return CPangya2TopMenu_HandleSelection((char*)p - 0x18);
}

// Adjust the secondary-base pointer before destroying the top menu.
void* CPangya2TopMenu_DtorThunk24(void* p, int x) {
    return CPangya2TopMenu_dtor((char*)p - 0x18, x);
}

#pragma opt_common_subs off
#pragma use_lmw_stmw off
// Create a five-slot product carousel and bind its player save record.
void* CShopProductsTab_Ctor(void* self, int productCount, const void* drawOffset, void* playerRecord) {
    void* newLayout;
    void* tertiaryLayout;
    void* record;
    record = playerRecord;
    AsCShopProductsTab(self)->vtbl = (void*)__vt__16CShopProductsTab;
    AsCShopProductsTab(self)->selectedProductIndex = 0;
    AsCShopProductsTab(self)->productCount = productCount;
    AsCShopProductsTab(self)->drawOffsetX = ((const int*)drawOffset)[0];
    AsCShopProductsTab(self)->drawOffsetY = ((const int*)drawOffset)[1];
    AsCShopProductsTab(self)->drawOffsetZ = ((const int*)drawOffset)[2];
    AsCShopProductsTab(self)->productDrawEntries = 0;
    newLayout = new ShopTabLayout(5);
    AsCShopProductsTab(self)->layout = newLayout;
    AsCShopProductsTab(self)->navigationDelayFrames = -1;
    AsCShopProductsTab(self)->productRecords = 0;
    newLayout = new ShopTabLayout(5);
    AsCShopProductsTab(self)->secondaryLayout = newLayout;
    tertiaryLayout = new ShopTabLayout(5);
    AsCShopProductsTab(self)->tertiaryLayout = tertiaryLayout;
    (*(int*)&AsCShopProductsTab(self)->scriptObject) = 0;
    AsCShopProductsTab(self)->scriptEntryId = -1;
    AsCShopProductsTab(self)->playerRecord = record;
    AsCShopProductsTab(self)->selectedProductNew = 0;
    AsCShopProductsTab(self)->selectedProductOwned = 0;
    AsCShopProductsTab(self)->selectedProductEquipped = 0;
    AsCShopProductsTab(self)->selectedProductQuantity = -1;
    (*(int*)&AsCShopProductsTab(self)->numberDraw) = 0;
    Layout_LoadExtended(AsCShopProductsTab(self)->layout, 0, shopProductsTabStrings);
    Layout_LoadExtended(AsCShopProductsTab(self)->secondaryLayout, 0, (char*)shopProductsTabStrings + kShopProductsTabStrings_club_iconbase_spd);
    if (((int&)Sing_805DC5F0()) == 0) {
        if (s_bpCtrlInitialized == 0) {
            __ct__7CBpCtrlFv(s_bpCtrl);
            __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, s_bpCtrlDtorRegistration_ShopTab); // s_bpCtrlDtorRegistration_ShopTab
            s_bpCtrlInitialized = 1;
        }
        ((int&)Sing_805DC5F0()) = (int)s_bpCtrl;
    }
    CBpCtrl_CreateEntry((void*)((int&)Sing_805DC5F0()), (char*)shopProductsTabStrings + kShopProductsTabStrings_club_new_b2d, &AsCShopProductsTab(self)->scriptEntryId, 0); // original +0x34
    if (((int&)Sing_805DC5F0()) == 0) {
        if (s_bpCtrlInitialized == 0) {
            __ct__7CBpCtrlFv(s_bpCtrl);
            __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, s_bpCtrlDtorRegistration_ShopTab); // s_bpCtrlDtorRegistration_ShopTab
            s_bpCtrlInitialized = 1;
        }
        ((int&)Sing_805DC5F0()) = (int)s_bpCtrl;
    }
    AsCShopProductsTab(self)->scriptObject = IdMap_Lookup((void*)((int&)Sing_805DC5F0()), AsCShopProductsTab(self)->scriptEntryId);
    if (((int&)Sing_805DC830()) == 0) {
        if (s_pointerVectorInitialized == 0) {
            PointerVector_InitReserved(s_fontNumDrawRegistry);
            __register_global_object(s_fontNumDrawRegistry, (void*)PointerVector_DestroyStorage, s_fontNumRegistryDtorRegistration_ShopTab); // s_fontNumRegistryDtorRegistration_ShopTab
            s_pointerVectorInitialized = 1;
        }
        ((int&)Sing_805DC830()) = (int)s_fontNumDrawRegistry;
    }
    (*(int*)&AsCShopProductsTab(self)->numberDraw) = (int)CFontNumDraw_CreateInVector((void*)((int&)Sing_805DC830()));
    return self;
}

#pragma opt_common_subs reset
#pragma use_lmw_stmw reset
// Release the carousel layouts, script entry and numeric-text registration.
int CShopProductsTab_dtor(void* self, int flag) {
    if (self != 0) {
        AsCShopProductsTab(self)->vtbl = (void*)__vt__16CShopProductsTab;
        if (AsCShopProductsTab(self)->layout != 0) {
            Layout_Destroy(AsCShopProductsTab(self)->layout, 1);
            AsCShopProductsTab(self)->layout = 0;
        }
        if (AsCShopProductsTab(self)->secondaryLayout != 0) {
            Layout_Destroy(AsCShopProductsTab(self)->secondaryLayout, 1);
            AsCShopProductsTab(self)->secondaryLayout = 0;
        }
        if (AsCShopProductsTab(self)->tertiaryLayout != 0) {
            Layout_Destroy(AsCShopProductsTab(self)->tertiaryLayout, 1);
            AsCShopProductsTab(self)->tertiaryLayout = 0;
        }
        if (AsCShopProductsTab(self)->scriptEntryId != -1) {
            if (((int&)Sing_805DC5F0()) == 0) {
                if (s_bpCtrlInitialized == 0) {
                    __ct__7CBpCtrlFv(s_bpCtrl);
                    __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, s_bpCtrlDtorRegistration_ShopTab); // s_bpCtrlDtorRegistration_ShopTab
                    s_bpCtrlInitialized = 1;
                }
                ((int&)Sing_805DC5F0()) = (int)s_bpCtrl;
            }
            CBpCtrl_DeleteEntryById((void*)((int&)Sing_805DC5F0()), AsCShopProductsTab(self)->scriptEntryId);
            AsCShopProductsTab(self)->scriptEntryId = -1;
        }
        UITextTable_ClearEntries(AsCShopProductsTab(self)->numberDraw);
        if (AsCShopProductsTab(self)->numberDraw != 0) {
                if (((int&)Sing_805DC830()) == 0) {
                    if (s_pointerVectorInitialized == 0) {
                        PointerVector_InitReserved(s_fontNumDrawRegistry);
                        __register_global_object(s_fontNumDrawRegistry, (void*)PointerVector_DestroyStorage, s_fontNumRegistryDtorRegistration_ShopTab); // s_fontNumRegistryDtorRegistration_ShopTab
                        s_pointerVectorInitialized = 1;
                    }
                    ((int&)Sing_805DC830()) = (int)s_fontNumDrawRegistry;
                }
                PointerArray_DeleteValue((void*)((int&)Sing_805DC830()), AsCShopProductsTab(self)->numberDraw);
            }
        if (flag > 0) {
            __dl__FPv(self);
        }
    }
    return (int)self;
}

// Start the new-item animation and initialize quantity text and navigation availability.
int CShopProductsTab_Reset(void* self) {
    void* obj30 = AsCShopProductsTab(self)->scriptObject;
    BpObject_StartScriptByName(obj30, shopProductsTabStrings + kShopProductsTabStrings_new_anime, 1);
    obj30 = AsCShopProductsTab(self)->scriptObject;
    BpObject_SetEnabled(obj30, 0);
    CShopProductsTab_ApplyDrawOffsetY(self);
    void* r3 = AsCShopProductsTab(self)->numberDraw;
    CFontNumDraw_RegisterEntries(r3, shopProductQuantityGroupNames, 5, AsCShopProductsTab(self)->scriptObject, shopProductsTabStrings + kShopProductsTabStrings_num_shop_00_gsr); // shopProductQuantityGroupNames
    int r4 = 0;
    r3 = AsCShopProductsTab(self)->numberDraw;
    ((ShopQuantityDrawFields*)r3)->enabled = r4;
    AsCShopProductsTab(self)->selectedProductIndex = r4;
    if (AsCShopProductsTab(self)->productDrawEntries != 0) {
        int r31 = 0;
        int r30 = 0;
        while (r31 < AsCShopProductsTab(self)->productCount) {
            ((VObjBF3F0*)((char*)(*(void**)&AsCShopProductsTab(self)->productDrawEntries) + r30))->call();
            r30 += 0x28;
            r31 += 1;
        }
    }
    int r0b = AsCShopProductsTab(self)->productCount;
    int r31 = 1;
    if (r0b <= 1) r31 = 0;
    void* layout = AsCShopProductsTab(self)->layout;
    Layout_SetElementPlayerFlag(layout, 0, r31, 5);
    layout = AsCShopProductsTab(self)->layout;
    Layout_SetElementPlayerFlag(layout, 1, r31, 5);
    return 1;
}

// Wait for both carousel layouts and all script resources to finish loading.
int CShopProductsTab_PollLoad(void* self) {
    if (!Layout_PollLoadComplete(AsCShopProductsTab(self)->layout)) return 0;
    if (!Layout_PollLoadComplete(AsCShopProductsTab(self)->secondaryLayout)) return 0;
    if (((int&)Sing_805DC5F0()) == 0) {
        if (s_bpCtrlInitialized == 0) {
            __ct__7CBpCtrlFv(s_bpCtrl);
            __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, s_bpCtrlDtorRegistration_ShopTab); // s_bpCtrlDtorRegistration_ShopTab
            s_bpCtrlInitialized = 1;
        }
        ((int&)Sing_805DC5F0()) = (int)s_bpCtrl;
    }
    int result = CBpCtrl_PollAllEntriesLoaded((void*)((int&)Sing_805DC5F0()));
    if (result == 0) return 0;
    return 1;
}

// Advance both carousel layouts by one update.
int CShopProductsTab_UpdateLayouts(void* p) {
    Layout_Update(AsCShopProductsTab(p)->layout);
    Layout_Update(AsCShopProductsTab(p)->secondaryLayout);
    return 1;
}

// Re-enable navigation when its click delay expires, then update both layouts.
int CShopProductsTab_UpdateNavigationDelay(void* p) {
    int r4 = AsCShopProductsTab(p)->navigationDelayFrames;
    if (r4 != -1) {
        int r0 = r4 - 1;
        AsCShopProductsTab(p)->navigationDelayFrames = r0;
        if (!(r4 > 0)) {
            int i = 0;
            while (i < 2) {
                Layout_SetElementValueState(AsCShopProductsTab(p)->layout, i, 0);
                Layout_SetElementPlayerFlag(AsCShopProductsTab(p)->layout, i, 1, 5);
                i = i + 1;
            }
            AsCShopProductsTab(p)->navigationDelayFrames = -1;
        }
    }
    Layout_Update(AsCShopProductsTab(p)->layout);
    Layout_Update(AsCShopProductsTab(p)->secondaryLayout);
    return 1;
}

// Draw clipped product icons and quantities, restore scissoring, then draw navigation.
int CShopProductsTab_Draw(void* self) {
    int other[4];
    int scissor[4];
    Engine_GetScissorRect(other);
    int* _p = shopProductScissorRect; // shopProductScissorRect
    scissor[0] = *_p++;
    scissor[1] = *_p++;
    scissor[2] = *_p++;
    scissor[3] = *_p++;
    Engine_SetScissorRect(scissor);
    Layout_DrawWithOpacity(AsCShopProductsTab(self)->secondaryLayout, kFloat_1_ShopTab);
    CShopProductsTab_DrawProductQuads(self);
    BpObject_TryInvokeCurrentCallback(AsCShopProductsTab(self)->scriptObject, 0);
    BpObject_Draw(AsCShopProductsTab(self)->scriptObject);
    UITextTable_Draw(AsCShopProductsTab(self)->numberDraw);
    Engine_SetScissorRect(other);
    Layout_DrawWithOpacity(AsCShopProductsTab(self)->layout, kFloat_1_ShopTab);
    return 1;
}

struct E94Ent_ { 
virtual void v0(); virtual void v1(); virtual void v2(); virtual void v3(); virtual void v4();
};
// Bind the borrowed draw/record arrays, reset selection and prepare each draw entry.
int CShopProductsTab_SetProductArrays(void* self, void* entries, void* arg) {
    (*(void**)&AsCShopProductsTab(self)->productDrawEntries) = entries;
    (*(void**)&AsCShopProductsTab(self)->productRecords) = arg;
    AsCShopProductsTab(self)->selectedProductIndex = 0;
    int r0;
    if (entries == 0) {
        r0 = 0;
    } else {
        int r31 = 0;
        int r30 = 0;
        while (r31 < AsCShopProductsTab(self)->productCount) {
            void* base = (*(void**)&AsCShopProductsTab(self)->productDrawEntries);
            ((E94Ent_*)((char*)base + r30))->v4();
            r30 += 0x28;
            r31++;
        }
        r0 = 1;
    }
    if (r0 == 0) return 0;
    return 1;
}

// Return the selected index through the existing pointer-sized ABI.
void* CShopProductsTab_GetSelectedProductIndex(void* p) {
    return *(void**)&AsCShopProductsTab(p)->selectedProductIndex;
}

// Accept a nonnegative product count and enable arrows only for multiple products.
int CShopProductsTab_SetProductCount(void* self, int x) {
    if (x < 0) {
        return 0;
    }
    AsCShopProductsTab(self)->productCount = x;
    int r31 = 1;
    if (x <= 1) r31 = 0;
    void* layout = AsCShopProductsTab(self)->layout;
    Layout_SetElementPlayerFlag(layout, 0, r31, 5);
    layout = AsCShopProductsTab(self)->layout;
    Layout_SetElementPlayerFlag(layout, 1, r31, 5);
    return 1;
}

// Handle navigation and refresh all five wrapped slots, clearing the selected new flag.
int CShopProductsTab_RefreshVisibleSlotIndicators(void* self) {
    int indexSum;
    int clearedStatus;
    int noQuantity;
    int visibleSlot;
    int productCount;
    int relativeSlot;
    int wrappedIndexOrAddress;
    int recordArrayAddress;
    int recordByteOffset;
    void* typeIdPayload;
    if (!CShopProductsTab_HandleNavigationInput(self)) return 0;
    // Display offsets [-2,+2] map to visible slots [0,4]. For a non-empty list,
    // (selected + relativeSlot + count) % count chooses the displayed product.
    // Keep signed division/remainder and the one-count bias. This code does not
    // normalize negative intermediates; count==1 at relativeSlot==-2 reaches -1.
    relativeSlot = kFirstVisibleRelativeSlot;
    clearedStatus = 0;
    noQuantity = kNoQuantity;
    for (; relativeSlot < kVisibleRelativeSlotEnd; relativeSlot++) {
        visibleSlot = relativeSlot + kVisibleProductCenterSlot;
        productCount = AsCShopProductsTab(self)->productCount;
        if (productCount != 0) {
            indexSum = AsCShopProductsTab(self)->selectedProductIndex;
            wrappedIndexOrAddress = relativeSlot + productCount;
            recordArrayAddress = AsCShopProductsTab(self)->productRecords;
            indexSum = indexSum + wrappedIndexOrAddress;
            wrappedIndexOrAddress = indexSum / productCount;
            wrappedIndexOrAddress = wrappedIndexOrAddress * productCount;
            wrappedIndexOrAddress = indexSum - wrappedIndexOrAddress;
            recordByteOffset = wrappedIndexOrAddress * sizeof(IFF_STRUCT::sBall);
            wrappedIndexOrAddress = recordArrayAddress + recordByteOffset;
            typeIdPayload = (void*)((IffItemCommon*)wrappedIndexOrAddress)->typeId;
            CShopProductsTab_UpdateProductStatus(self, (unsigned int)typeIdPayload);
            if (relativeSlot == 0) {
                recordArrayAddress = AsCShopProductsTab(self)->productRecords;
                wrappedIndexOrAddress = recordArrayAddress + recordByteOffset;
                typeIdPayload = (void*)((IffItemCommon*)wrappedIndexOrAddress)->typeId;
                CShopProductsTab_ClearProductFlag(self, typeIdPayload);
            }
        } else {
            AsCShopProductsTab(self)->selectedProductNew = (unsigned char)clearedStatus;
            AsCShopProductsTab(self)->selectedProductOwned = (unsigned char)clearedStatus;
            AsCShopProductsTab(self)->selectedProductEquipped = (unsigned char)clearedStatus;
            AsCShopProductsTab(self)->selectedProductQuantity = noQuantity;
        }
        // UpdateProductStatus sets shared status scratch; this call snapshots it into one slot.
        CShopProductsTab_UpdateSlotIndicators(self, visibleSlot);
    }
    return 1;
}

// Refresh ownership for the selected type ID; an empty list is treated as owned.
int CShopProductsTab_IsSelectedProductOwned(void* p) {
    int r0 = AsCShopProductsTab(p)->productCount;
    if (r0 == 0) {
        return 1;
    }
    r0 = AsCShopProductsTab(p)->selectedProductIndex;
    void* r4 = (*(void**)&AsCShopProductsTab(p)->productRecords);
    r4 = (char*)r4 + r0 * sizeof(IFF_STRUCT::sBall);
    r4 = (void*)((IffItemCommon*)r4)->typeId;
    CShopProductsTab_UpdateProductStatus(p, (unsigned int)r4);
    return AsCShopProductsTab(p)->selectedProductOwned;
}

// Enable carousel scripts and quantity drawing, then refresh visible product status.
int CShopProductsTab_SetEnabled(void* self, int arg) {
    int q;
    int zero;
    int minusOne;
    int index;
    int count;
    int value;
    int offset;
    int rem;
    int stride;
    void* typeId;
    void* base;
    BpObject_SetEnabled(AsCShopProductsTab(self)->scriptObject, arg);
    ((ShopQuantityDrawFields*)AsCShopProductsTab(self)->numberDraw)->enabled = arg;
    if (arg != 0) {
        offset = -2;
        zero = 0;
        minusOne = -1;
        for (; offset < 3; offset++) {
            index = offset + 2;
            count = AsCShopProductsTab(self)->productCount;
            if (count != 0) {
                value = (AsCShopProductsTab(self)->selectedProductIndex + offset + count);
                q = value / count;
                q = q * count;
                rem = value - q;
                stride = rem * sizeof(IFF_STRUCT::sBall);
                typeId = (void*)((IffItemCommon*)((char*)(*(void**)&AsCShopProductsTab(self)->productRecords) + stride))->typeId;
                CShopProductsTab_UpdateProductStatus(self, (unsigned int)typeId);
                if (offset == 0) {
                    base = (*(void**)&AsCShopProductsTab(self)->productRecords);
                    typeId = (void*)((IffItemCommon*)((char*)base + stride))->typeId;
                    CShopProductsTab_ClearProductFlag(self, typeId);
                }
            } else {
                AsCShopProductsTab(self)->selectedProductNew = (unsigned char)zero;
                AsCShopProductsTab(self)->selectedProductOwned = (unsigned char)zero;
                AsCShopProductsTab(self)->selectedProductEquipped = (unsigned char)zero;
                AsCShopProductsTab(self)->selectedProductQuantity = minusOne;
            }
            CShopProductsTab_UpdateSlotIndicators(self, index);
        }
    }
    return 1;
}

// Refresh the five wrapped product slots and acknowledge the selected product.
int CShopProductsTab_RefreshVisibleProducts(void* self) {
    int stride;
    int offset;
    int base;
    int zero;
    int minusOne;
    int count;
    int index;
    int r4;
    int r0;
    void* typeId;

    offset = -2;

    zero = 0;
    minusOne = -1;
    for (; offset < 3; offset++) {
        index = offset + 2;
        count = AsCShopProductsTab(self)->productCount;
        if (count != 0) {
            base = AsCShopProductsTab(self)->selectedProductIndex;
            r4 = offset + count;
            r0 = AsCShopProductsTab(self)->productRecords;
            base = base + r4;
            r4 = base / count;
            r4 = r4 * count;
            r4 = base - r4;
            stride = r4 * sizeof(IFF_STRUCT::sBall);
            r4 = r0 + stride;
            typeId = (void*)((IffItemCommon*)r4)->typeId;
            CShopProductsTab_UpdateProductStatus(self, (unsigned int)typeId);
            if (offset == 0) {
                r0 = AsCShopProductsTab(self)->productRecords;
                r4 = r0 + stride;
                typeId = (void*)((IffItemCommon*)r4)->typeId;
                CShopProductsTab_ClearProductFlag(self, typeId);
            }
        } else {
            AsCShopProductsTab(self)->selectedProductNew = (unsigned char)zero;
            AsCShopProductsTab(self)->selectedProductOwned = (unsigned char)zero;
            AsCShopProductsTab(self)->selectedProductEquipped = (unsigned char)zero;
            AsCShopProductsTab(self)->selectedProductQuantity = minusOne;
        }
        CShopProductsTab_UpdateSlotIndicators(self, index);
    }
    return 1;
}

// Select the record whose type ID equals the argument; the argument is encoded as a pointer.
// arg carries an unsigned 32-bit IFF value in pointer-width storage; never
// dereference/free it. This ABI convention also applies to every typeIdArg below.
// Actual productRecords/playerRecord pointers ARE borrowed addresses. Return 1
// only after selecting a match, otherwise 0 (including encoded zero).
int CShopProductsTab_SelectProductByTypeId(void* self, void* arg) {
    if (arg == 0) {
        return 0;
    }
    int count = AsCShopProductsTab(self)->productCount;
    int offset;
    int index;
    index = 0;
    offset = 0;
    int i;
    for (i = 0; i < count; i++) {
        void* p = (*(void**)&AsCShopProductsTab(self)->productRecords);
        p = (char*)p + offset;
        if (arg == (void*)((IffItemCommon*)p)->typeId) {
            AsCShopProductsTab(self)->selectedProductIndex = index;
            return 1;
        }
        offset += sizeof(IFF_STRUCT::sBall);
        index++;
    }
    return 0;
}

#pragma use_lmw_stmw on
// Draw five wrapped product icons, dimming the four unselected slots.
int CShopProductsTab_DrawProductQuads(const void* self) {
    if (AsCShopProductsTab(self)->productCount == 0) return 0;
    int offset;
    int i;
    int v[4];
    Quad802C verts;
    for (i = -2; i < 3; i++) {
        int count = AsCShopProductsTab(self)->productCount;
        int base = AsCShopProductsTab(self)->selectedProductIndex;
        void* obj = (*(void**)&AsCShopProductsTab(self)->productDrawEntries);
        offset = ((base + (i + count)) % count) * 0x28;
        if (((VObjBF3F0*)((char*)obj + offset))->call()) {
            int slot = i + 7;
            v[0]=0;
            void* layout = AsCShopProductsTab(self)->secondaryLayout;
            v[1]=0; v[2]=0; v[3]=0;
            Layout_ReadElementRect(layout, slot, v);
            int brightness = 0xFF;
            float f2 = (*(float*)&AsCShopProductsTab(self)->drawOffsetY);
            v[1] = (int)((float)v[1] + f2);
            v[3] = (int)((float)v[3] + f2);
            if (i != 0) brightness = 0x50;
            verts = shopProductQuadTemplate; // shopProductQuadTemplate
            verts.q[0].x = (float)v[0];
            verts.q[0].y = (float)v[1];
            verts.q[0].r = (unsigned char)brightness;
            verts.q[0].g = (unsigned char)brightness;
            verts.q[0].b = (unsigned char)brightness;
            verts.q[1].x = (float)v[2];
            verts.q[1].y = (float)v[1];
            verts.q[1].r = (unsigned char)brightness;
            verts.q[1].g = (unsigned char)brightness;
            verts.q[1].b = (unsigned char)brightness;
            verts.q[2].x = (float)v[0];
            verts.q[2].y = (float)v[3];
            verts.q[2].r = (unsigned char)brightness;
            verts.q[2].g = (unsigned char)brightness;
            verts.q[2].b = (unsigned char)brightness;
            verts.q[3].x = (float)v[2];
            verts.q[3].y = (float)v[3];
            verts.q[3].r = (unsigned char)brightness;
            verts.q[3].g = (unsigned char)brightness;
            verts.q[3].b = (unsigned char)brightness;
            Engine_SetFlexibleVertexFormat(0x11f005);
            void* obj2 = (*(void**)&AsCShopProductsTab(self)->productDrawEntries);
            ((VObjBF3F0*)((char*)obj2 + offset))->v3();
            void* primitiveCount = GX_GetPrimitiveCount(4, 0x98);
            DrawPrimitiveUP(0x98, primitiveCount, &verts, 0x18);
        }
    }
    return 1;
}

#pragma use_lmw_stmw reset
// Move left or right with wraparound; play an error when the list cannot scroll.
int CShopProductsTab_HandleNavigationInput(void* self) {
    if (CShopProductsTab_HandleNavigationActivation(self)) return 1;

    if (!Sing_805DC5C8()) {
        if (!s_inputManagerInitialized) {
            InputManager_Init(s_gameInputManager);
            __register_global_object(s_gameInputManager, (void*)InputManager_Dtor, s_inputManagerDtorRegistration_ShopTab); // s_inputManagerDtorRegistration_ShopTab
            s_inputManagerInitialized = 1;
        }
        Sing_805DC5C8() = s_gameInputManager;
    }
    int leftPressed = Input_IsActionPressed(Sing_805DC5C8(), kShopNavigateLeft, -1);
    if (leftPressed) {
        const unsigned int count = (*(unsigned int*)&AsCShopProductsTab(self)->productCount);
        if (count <= 1) {
            Sound_PlayByName(g_soundManager, shopProductsTabStrings + kShopProductsTabStrings_ui_error, 0, 0, 0, 0, kFloat_0_5_ShopCarouselSound, kFloat_200_ShopCarouselSound, kFloat_1_ShopTab);
            return 0;
        }
        int current = AsCShopProductsTab(self)->selectedProductIndex;
        if (current > 0) {
            AsCShopProductsTab(self)->selectedProductIndex = current - 1;
        } else {
            AsCShopProductsTab(self)->selectedProductIndex = (int)count - 1;
        }
    } else {
        if (!Sing_805DC5C8()) {
            if (!s_inputManagerInitialized) {
                InputManager_Init(s_gameInputManager);
                __register_global_object(s_gameInputManager, (void*)InputManager_Dtor, s_inputManagerDtorRegistration_ShopTab); // s_inputManagerDtorRegistration_ShopTab
                s_inputManagerInitialized = 1;
            }
            Sing_805DC5C8() = s_gameInputManager;
        }
        int rightPressed = Input_IsActionPressed(Sing_805DC5C8(), kShopNavigateRight, -1);
        if (rightPressed == 0) return 0;
        unsigned int productCount = (*(unsigned int*)&AsCShopProductsTab(self)->productCount);
        if (productCount <= 1) {
            Sound_PlayByName(g_soundManager, shopProductsTabStrings + kShopProductsTabStrings_ui_error, 0, 0, 0, 0, kFloat_0_5_ShopCarouselSound, kFloat_200_ShopCarouselSound, kFloat_1_ShopTab);
            return 0;
        }
        int current = AsCShopProductsTab(self)->selectedProductIndex;
        if (current < (int)productCount - 1) {
            AsCShopProductsTab(self)->selectedProductIndex = current + 1;
        } else {
            AsCShopProductsTab(self)->selectedProductIndex = 0;
        }
    }
    Sound_PlayByName(g_soundManager, shopProductsTabStrings + kShopProductsTabStrings_ui_cursol_move2, 0, 0, 0, 0, kFloat_0_5_ShopCarouselSound, kFloat_200_ShopCarouselSound, kFloat_1_ShopTab);
    return 1;
}

// Consume clicked navigation arrows, wrap the selection and start the click delay.
int CShopProductsTab_HandleNavigationActivation(void* self) {
    volatile int indices[3] = {0, 0, 0};
    ActivatedUnwindCleanup activatedCleanup(&indices);
    if (!Layout_CollectActivated(AsCShopProductsTab(self)->layout, (void*)indices, 0)) {
        void* p0 = (void*)indices[0];
        if (p0) {
            indices[1] -= indices[1];
            activatedCleanup.release(); appFreeUni__FPv(p0);
        }
        return 0;
    }
    unsigned int count = (*(unsigned int*)&AsCShopProductsTab(self)->productCount);
    if (count <= 1) {
        Sound_PlayByName(g_soundManager, shopProductsTabStrings + kShopProductsTabStrings_ui_error, 0, 0, 0, 0, kFloat_0_5_ShopCarouselSound, kFloat_200_ShopCarouselSound, kFloat_1_ShopTab);
        void* p1 = (void*)indices[0];
        if (p1) {
            indices[1] -= indices[1];
            activatedCleanup.release(); appFreeUni__FPv(p1);
        }
        return 0;
    }
    if (!*(int*)indices[0]) {
        int current = AsCShopProductsTab(self)->selectedProductIndex;
        if (current > 0) {
            AsCShopProductsTab(self)->selectedProductIndex = current - 1;
        } else {
            AsCShopProductsTab(self)->selectedProductIndex = (int)count - 1;
        }
    } else {
        int current = AsCShopProductsTab(self)->selectedProductIndex;
        if (current < (int)count - 1) {
            AsCShopProductsTab(self)->selectedProductIndex = current + 1;
        } else {
            AsCShopProductsTab(self)->selectedProductIndex = 0;
        }
    }
    int value = *(int*)indices[0];
    Layout_SetElementValueState(AsCShopProductsTab(self)->layout, value, 2);
    Layout_SetElementPlayerFlag(AsCShopProductsTab(self)->layout, value, 0, 5);
    AsCShopProductsTab(self)->navigationDelayFrames = 3;
    Sound_PlayByName(g_soundManager, shopProductsTabStrings + kShopProductsTabStrings_ui_cursol_move2, 0, 0, 0, 0, kFloat_0_5_ShopCarouselSound, kFloat_200_ShopCarouselSound, kFloat_1_ShopTab);
    void* p2 = (void*)indices[0];
    if (p2) {
        indices[1] -= indices[1];
        activatedCleanup.release(); appFreeUni__FPv(p2);
    }
    return 1;
}

#pragma push
#pragma use_lmw_stmw on
#pragma opt_common_subs off
// Decode the IFF type ID and refresh new, owned, equipped and quantity indicators.
int CShopProductsTab_UpdateProductStatus(void* self, unsigned int typeId) {
    struct BitReference { unsigned int* word; unsigned int index; };
    volatile BitReference ownershipBitRead, ownershipBitSetup;
    // Both aliases address the working hairstyle bitset. Volatile preserves the
    // original access schedule; neither is a copy of persistent NEW-item flags.
    unsigned char* tabBytes = (unsigned char*)self;
    AsCShopProductsTab(tabBytes)->selectedProductNew = 0;
    AsCShopProductsTab(tabBytes)->selectedProductOwned = 0;
    AsCShopProductsTab(tabBytes)->selectedProductEquipped = 0;
    AsCShopProductsTab(tabBytes)->selectedProductQuantity = -1;
    switch (typeId >> kTypeIdKindShift) {
    case kItemCharacter: CShopProductsTab_UpdateCharacterStatus(self, (const char*)typeId); break;
    case kItemPart: CShopProductsTab_UpdatePartStatus(self, (void*)typeId); break;
    case kItemHairStyle: {
        char* playerRecord = (*(char**)&AsCShopProductsTab(tabBytes)->playerRecord);
        unsigned int inventoryIndex = ItemInventory_FindHairStyleIndex(AsCSharedDoc(g_sharedDoc)->itemInventory, typeId);
        ownershipBitSetup.word = AsShopSaveFields(playerRecord)->ownedHairStyles; ownershipBitSetup.index = inventoryIndex;
        ownershipBitRead.word = AsShopSaveFields(playerRecord)->ownedHairStyles; ownershipBitRead.index = inventoryIndex;
        if (inventoryIndex >= kHairStyleCount) {
            OutOfRangeStorage storage;
            OutOfRangeStorage* ep = &storage;
            storage.vtable = __vt__Q23std11logic_error;
            struct AllocSlot { char value; char padding[3]; } allocator;
            allocator.value = 0;
            unsigned int len = strlen(shopHairOwnershipBoundsError) + 1; // shopHairOwnershipBoundsError
            char* p = (char*)__nwa__FUl(len * 2 - len);
            storage.message = p;
            {
                std::TargetMessageGuard guard = {p, &allocator.value};
                storage.owner = new std::TargetMessageOwner(p);
                guard.pointer = 0;
            }
            strcpy(storage.message, shopHairOwnershipBoundsError); // shopHairOwnershipBoundsError
            storage.vtable = __vt__Q23std12out_of_range;
            __throw(shopProductsTabStrings + kShopProductsTabStrings_std_exception_std_logic_error_std_out_of_range, ep, (void*)__dt__Q23std12out_of_rangeFv);
        }
        if (ownershipBitRead.word[inventoryIndex >> 5] & (1u << (inventoryIndex & 31))) AsCShopProductsTab(tabBytes)->selectedProductOwned = 1;
        if ((int)inventoryIndex == *(signed char*)((*(char**)&AsCShopProductsTab(tabBytes)->playerRecord) + AsSavePlayerRecord(playerRecord)->characterIndex * 0x34 + 0x2fe)) AsCShopProductsTab(tabBytes)->selectedProductEquipped = 1;
        break;
    }
    case kItemAuxPart: {
        char* playerRecord = (*(char**)&AsCShopProductsTab(tabBytes)->playerRecord);
        int inventoryIndex = ItemInventory_FindAuxPartIndex(AsCSharedDoc(g_sharedDoc)->itemInventory, typeId);
        if (AsShopSaveFields(playerRecord)->ownedAuxParts[inventoryIndex]) AsCShopProductsTab(tabBytes)->selectedProductOwned = 1;
        if (inventoryIndex == *(signed char*)((*(char**)&AsCShopProductsTab(tabBytes)->playerRecord) + AsSavePlayerRecord(playerRecord)->characterIndex * 0x34 + 0x2ff)) AsCShopProductsTab(tabBytes)->selectedProductEquipped = 1;
        break;
    }
    case kItemCaddie: CShopProductsTab_UpdateCaddieStatus(self, (void*)typeId); break;
    case kItemCaddieCostume: CShopProductsTab_UpdateCaddieItemStatus(self, (void*)typeId); break;
    case kItemClubSet: CShopProductsTab_UpdateClubSetStatus(self, (void*)typeId); break;
    case kItemBall: CShopProductsTab_UpdateBallStatus(self, (void*)typeId); break;
    case kItemConsumable: {
        char* playerRecord = (*(char**)&AsCShopProductsTab(tabBytes)->playerRecord);
        int inventoryIndex = ItemInventory_FindItemIndex(AsCSharedDoc(g_sharedDoc)->itemInventory, typeId);
        short* value = &AsShopSaveFields(playerRecord)->itemQuantities[inventoryIndex];
        if (*value > 0) AsCShopProductsTab(tabBytes)->selectedProductOwned = 1;
        AsCShopProductsTab(tabBytes)->selectedProductQuantity = *value;
        if (inventoryIndex == AsShopSaveFields(playerRecord)->equippedItemIndices[0]) AsCShopProductsTab(tabBytes)->selectedProductEquipped = 1;
        else if (inventoryIndex == AsShopSaveFields(playerRecord)->equippedItemIndices[1]) AsCShopProductsTab(tabBytes)->selectedProductEquipped = 1;
        else if (inventoryIndex == AsShopSaveFields(playerRecord)->equippedItemIndices[2]) AsCShopProductsTab(tabBytes)->selectedProductEquipped = 1;
        else if (inventoryIndex == AsShopSaveFields(playerRecord)->equippedItemIndices[3]) AsCShopProductsTab(tabBytes)->selectedProductEquipped = 1;
        else if (inventoryIndex == AsShopSaveFields(playerRecord)->equippedItemIndices[4]) AsCShopProductsTab(tabBytes)->selectedProductEquipped = 1;
        else if (inventoryIndex == AsShopSaveFields(playerRecord)->equippedItemIndices[5]) AsCShopProductsTab(tabBytes)->selectedProductEquipped = 1;
        else if (inventoryIndex == AsShopSaveFields(playerRecord)->equippedItemIndices[6]) AsCShopProductsTab(tabBytes)->selectedProductEquipped = 1;
        else if (inventoryIndex == AsShopSaveFields(playerRecord)->equippedItemIndices[7]) AsCShopProductsTab(tabBytes)->selectedProductEquipped = 1;
        break;
    }
    default:
        if (typeId == 0) {
            AsCShopProductsTab(tabBytes)->selectedProductOwned = 1;
            if (AsSavePlayerRecord(AsCShopProductsTab(tabBytes)->playerRecord)->caddieIndex == -1) AsCShopProductsTab(tabBytes)->selectedProductEquipped = 1;
        }
        break;
    }
    return 1;
}

#pragma opt_common_subs on
// Compare saved new-character bits with current ownership and equipped character.
int CShopProductsTab_UpdateCharacterStatus(void* self, const char* typeIdArg) {
    volatile TargetBitReference b, a;
    TargetSlotRecord* currentRecord;
    unsigned int inventoryIndex;
    TargetSlotSelf* s = (TargetSlotSelf*)self;
    currentRecord = s->currentRecord;
    if (!Sing_805DC5E0()) {
        if (!s_saveDataInitialized) {
            SaveData_Ctor(s_saveData);
            s_saveDataInitialized = 1;
        }
        Sing_805DC5E0() = s_saveData;
    }
    char savedRecord[0x828];
    SaveRecord_CopyCtor(savedRecord, SaveData_GetCurrentRecord(Sing_805DC5E0()));
    inventoryIndex = ItemInventory_FindCharacterIndex(AsCSharedDoc(g_sharedDoc)->itemInventory, (void*)typeIdArg);
    a.word = &AsShopSaveFields(savedRecord)->newCharacters; a.index = inventoryIndex;
    b.word = &AsShopSaveFields(savedRecord)->newCharacters; b.index = inventoryIndex;
    if ((unsigned int)inventoryIndex >= kCharacterCount) {
        OutOfRangeStorage storage;
        OutOfRangeStorage* ep = &storage;
        storage.vtable = __vt__Q23std11logic_error;
        struct AllocSlot { char value; char padding[3]; } allocator;
        allocator.value = 0;
        unsigned int len = strlen(shopCharacterOrCaddieTestBoundsError) + 1; // shopCharacterOrCaddieTestBoundsError
        char* p = (char*)__nwa__FUl(len * 2 - len);
        storage.message = p;
        {
            std::TargetMessageGuard guard = {p, &allocator.value};
            storage.owner = new std::TargetMessageOwner(p);
            guard.pointer = 0;
        }
        strcpy(storage.message, shopCharacterOrCaddieTestBoundsError); // shopCharacterOrCaddieTestBoundsError
        storage.vtable = __vt__Q23std12out_of_range;
        __throw(shopProductsTabStrings + kShopProductsTabStrings_std_exception_std_logic_error_std_out_of_range, ep, (void*)__dt__Q23std12out_of_rangeFv);
    }
    int statusFlags = 0;
    if (*b.word & (1u << inventoryIndex)) {
        struct Entry { unsigned char flags; char padding[0x33]; };
        unsigned char byte = ((ShopCharacterEquipment*)(savedRecord + IFF_OFFSET(ShopSaveFields, characters)))[inventoryIndex].flags;
        if (((int)((byte << 25) & 0xc0000000) >> 31) == 0) statusFlags = 1;
    }
    if (statusFlags) s->flag[0] = 1;
    unsigned char* arr = (unsigned char*)s->currentRecord;
    unsigned char byte = AsShopSaveFields(arr)->characters[inventoryIndex].flags;
    if (((int)((byte << 25) & 0xc0000000) >> 31) != 0)
        s->flag[1] = 1;
    if (currentRecord->selected == (int)inventoryIndex)
        s->flag[2] = 1;
    return 1;
}

#pragma opt_common_subs off

// Throw the part-ownership bitset bounds error with the original exception representation.
inline void TargetThrow0EA4() {
    OutOfRangeStorage storage;
    OutOfRangeStorage* ep = &storage;
    storage.vtable = __vt__Q23std11logic_error;
    struct AllocSlot { char value; char padding[3]; } allocator;
    allocator.value = 0;
    unsigned int len = strlen(bitsetTestOutOfRangeMessage) + 1;
    char* p = (char*)__nwa__FUl(len * 2 - len);
    storage.message = p;
    { std::TargetMessageGuard guard = {p, &allocator.value};
      storage.owner = new std::TargetMessageOwner(p);
      guard.pointer = 0; }
    strcpy(storage.message, bitsetTestOutOfRangeMessage);
    storage.vtable = __vt__Q23std12out_of_range;
    __throw(shopProductsTabStrings + kShopProductsTabStrings_std_exception_std_logic_error_std_out_of_range, ep, (void*)__dt__Q23std12out_of_rangeFv);
}
// Bounds-check a part index and read the corresponding ownership bit.
inline unsigned int TargetOwnedTestImpl(void* inner, unsigned int idx, volatile TargetBitReference* b) {
    volatile TargetBitReference a = { AsShopSaveFields(inner)->ownedParts, idx };
    b->word = AsShopSaveFields(inner)->ownedParts;
    b->index = idx;
    if (idx >= kPartCount) {
        TargetThrow0EA4();
    }
    return b->word[idx >> 5] & (1u << (idx & 31));
}
// Preserve the temporary ordering of the inlined bitset accessors.
// Preserve the original inline lifetime boundary around a part ownership test.
inline unsigned int TargetOwnedTestMiddle(void* inner, unsigned int idx, volatile TargetBitReference* b) {
    return TargetOwnedTestImpl(inner, idx, b);
}
// Read a part ownership bit through the original inline accessor nesting.
inline unsigned int TargetOwnedTest(void* inner, unsigned int idx, volatile TargetBitReference* b) {
    return TargetOwnedTestMiddle(inner, idx, b);
}
// Test new/owned part bits and scan all 24 equipped slots for the selected character.
int CShopProductsTab_UpdatePartStatus(void* self, void* typeIdArg) {
    volatile TargetBitReference b, d, f;
    unsigned int inventoryIndex;
    TargetSlotRecord* currentRecord;
    currentRecord = (*(TargetSlotRecord**)&AsCShopProductsTab(self)->playerRecord);
    if (!Sing_805DC5E0()) {
        if (!s_saveDataInitialized) {
            SaveData_Ctor(s_saveData);
            s_saveDataInitialized = 1;
        }
        Sing_805DC5E0() = s_saveData;
    }
    // Phase 1: NEW comes from the persisted save, not the outfit currently previewed.
    char savedRecord[0x828];
    SaveRecord_CopyCtor(savedRecord, SaveData_GetCurrentRecord(Sing_805DC5E0()));
    inventoryIndex = ItemInventory_FindPartIndex(AsCSharedDoc(g_sharedDoc)->itemInventory, typeIdArg);
    volatile TargetBitReference a = { AsShopSaveFields(savedRecord)->newParts, inventoryIndex };
    b.word = AsShopSaveFields(savedRecord)->newParts; b.index = inventoryIndex;
    if ((unsigned int)inventoryIndex >= kPartCount) {
        TargetThrow0EA4();
    }
    // A saved NEW bit counts only when the saved OWNED bit is clear.
    int statusFlags = (b.word[(unsigned int)inventoryIndex >> 5] & (1u << (inventoryIndex & 31))) != 0;
    if (statusFlags) {
        volatile TargetBitReference c = { AsShopSaveFields(savedRecord)->ownedParts, inventoryIndex };
        d.word = AsShopSaveFields(savedRecord)->ownedParts; d.index = inventoryIndex;
        if ((unsigned int)inventoryIndex >= kPartCount) {
            TargetThrow0EA4();
        }
        statusFlags = (d.word[(unsigned int)inventoryIndex >> 5] & (1u << (inventoryIndex & 31))) == 0;
    }
    if (statusFlags) AsCShopProductsTab(self)->selectedProductNew = 1;
    // Phase 2: OWNED/EQUIPPED come from the working record. The caller clears
    // these latches before dispatch; this category handler only sets true flags.
    if (TargetOwnedTest(currentRecord, inventoryIndex, &f))
        AsCShopProductsTab(self)->selectedProductOwned = 1;
    for (int j = 0; j < kEquipmentPartSlotCount; ++j) {
            signed char characterIndex = AsSavePlayerRecord(currentRecord)->characterIndex;
            void* workingSave = AsCShopProductsTab(self)->playerRecord;
            int characterPartRowOffset = characterIndex * 0x34;
            char* partSlotBytes = (char*)((int)workingSave + j * 2);
            short equippedPartInventoryIndex = *(short*)(characterPartRowOffset + partSlotBytes + 0x2ce);
            if ((int)inventoryIndex == equippedPartInventoryIndex) {
                AsCShopProductsTab(self)->selectedProductEquipped = 1;
                break;
            }
    }
    return 1;
}

#pragma opt_common_subs off
// Refresh new/owned/equipped caddie indicators from saved and current records.
int CShopProductsTab_UpdateCaddieStatus(void* self, void* typeIdArg) {
    volatile TargetBitReference b, a;
    struct Record { char padding[0x2f]; signed char selected; };
    struct Self { char padding[0x38]; Record* currentRecord; unsigned char statusFlags[3]; };
    Record* currentRecord;
    unsigned int inventoryIndex;
    Self* s = (Self*)self;
    currentRecord = s->currentRecord;
    if (!Sing_805DC5E0()) {
        if (!s_saveDataInitialized) {
            SaveData_Ctor(s_saveData);
            s_saveDataInitialized = 1;
        }
        Sing_805DC5E0() = s_saveData;
    }
    char savedRecord[0x828];
    SaveRecord_CopyCtor(savedRecord, SaveData_GetCurrentRecord(Sing_805DC5E0()));
    inventoryIndex = ItemInventory_FindCaddieIndex(AsCSharedDoc(g_sharedDoc)->itemInventory, typeIdArg);
    a.word = &AsShopSaveFields(savedRecord)->newCaddies; a.index = inventoryIndex;
    b.word = &AsShopSaveFields(savedRecord)->newCaddies; b.index = inventoryIndex;
    if ((unsigned int)inventoryIndex >= kCaddieCount) {
        OutOfRangeStorage storage;
        OutOfRangeStorage* ep = &storage;
        storage.vtable = __vt__Q23std11logic_error;
        struct AllocSlot { char value; char padding[3]; } allocator;
        allocator.value = 0;
        unsigned int len = strlen(shopCharacterOrCaddieTestBoundsError) + 1; // shopCharacterOrCaddieTestBoundsError
        char* p = (char*)__nwa__FUl(len * 2 - len);
        storage.message = p;
        {
            std::TargetMessageGuard guard = {p, &allocator.value};
            storage.owner = new std::TargetMessageOwner(p);
            guard.pointer = 0;
        }
        strcpy(storage.message, shopCharacterOrCaddieTestBoundsError); // shopCharacterOrCaddieTestBoundsError
        storage.vtable = __vt__Q23std12out_of_range;
        __throw(shopProductsTabStrings + kShopProductsTabStrings_std_exception_std_logic_error_std_out_of_range, ep, (void*)__dt__Q23std12out_of_rangeFv);
    }
    int statusFlags = 0;
    if (*b.word & (1u << inventoryIndex)) {
        struct Entry { unsigned char flags; char padding[7]; };
        unsigned char byte = ((ShopCaddieOwnership*)(savedRecord + IFF_OFFSET(ShopSaveFields, caddies)))[inventoryIndex].flags;
        if (((int)((byte << 25) & 0xc0000000) >> 31) == 0) statusFlags = 1;
    }
    if (statusFlags) s->statusFlags[0] = 1;
    unsigned char* current = (unsigned char*)s->currentRecord;
    unsigned char byte = AsShopSaveFields(current)->caddies[inventoryIndex].flags;
    if (((int)((byte << 25) & 0xc0000000) >> 31) != 0)
        s->statusFlags[1] = 1;
    if (currentRecord->selected == (int)inventoryIndex)
        s->statusFlags[2] = 1;
    return 1;
}

#pragma opt_common_subs off
// Preserve this expansion: a parameterized inline helper changes caller register allocation.
#define RANGE_THROW_802C1540(storage, text) do { \
    OutOfRangeStorage storage; \
    storage.vtable = __vt__Q23std11logic_error; \
    struct AllocSlot { char value; char padding[3]; } allocator; \
    allocator.value = 0; \
    OutOfRangeStorage* ep = &storage; \
    unsigned int len = strlen(text) + 1; \
    char* p = (char*)__nwa__FUl(len * 2 - len); \
    storage.message = p; \
    { std::TargetMessageGuard guard = { p, &allocator.value }; \
      storage.owner = new std::TargetMessageOwner(p); \
      guard.pointer = 0; } \
    strcpy(storage.message, text); \
    storage.vtable = __vt__Q23std12out_of_range; \
    __throw(shopProductsTabStrings + kShopProductsTabStrings_std_exception_std_logic_error_std_out_of_range, ep, (void*)__dt__Q23std12out_of_rangeFv); \
} while (0)
// Handle bare-caddie IDs separately from caddie costume ownership and equipment.
int CShopProductsTab_UpdateCaddieItemStatus(void* self, void* typeIdArg) {
    void* currentRecord = AsCShopProductsTab(self)->playerRecord;
    if (!Sing_805DC5E0()) {
        if (!s_saveDataInitialized) {
            SaveData_Ctor(s_saveData);
            s_saveDataInitialized = 1;
        }
        Sing_805DC5E0() = s_saveData;
    }
    char savedRecord[0x828];
    SaveRecord_CopyCtor(savedRecord, SaveData_GetCurrentRecord(Sing_805DC5E0()));
    struct BitReference { unsigned int* word; int index; };
    volatile BitReference b1, b2, b3, b4;
    if (((unsigned int)typeIdArg & 0x1fff) == 0) {
        unsigned int inventoryIndex = ((unsigned int)typeIdArg >> 21) & 31;
        volatile BitReference a1;
        a1.word = &AsShopSaveFields(savedRecord)->newCaddies; a1.index = inventoryIndex;
        b1.word = &AsShopSaveFields(savedRecord)->newCaddies; b1.index = inventoryIndex;
        if (inventoryIndex >= 8) {
            RANGE_THROW_802C1540(error, shopCharacterOrCaddieTestBoundsError); // shopCharacterOrCaddieTestBoundsError
        }
        int statusFlags = 0;
        if (*b1.word & (1u << inventoryIndex)) {
            struct Entry { unsigned char flags; char padding[7]; };
            unsigned char byte = ((ShopCaddieOwnership*)(savedRecord + IFF_OFFSET(ShopSaveFields, caddies)))[inventoryIndex].flags;
            if (((int)((byte << 25) & 0xc0000000) >> 31) == 0) statusFlags = 1;
        }
        if (statusFlags) AsCShopProductsTab(self)->selectedProductNew = 1;
        unsigned char* current = (*(unsigned char**)&AsCShopProductsTab(self)->playerRecord);
        unsigned char byte = AsShopSaveFields(current)->caddies[inventoryIndex].flags;
        if (((int)((byte << 25) & 0xc0000000) >> 31) != 0)
            AsCShopProductsTab(self)->selectedProductOwned = 1;
        if (AsSavePlayerRecord(currentRecord)->caddieIndex == (int)inventoryIndex &&
            AsShopSaveFields(currentRecord)->caddieItemIndex == -1)
            AsCShopProductsTab(self)->selectedProductEquipped = 1;
    } else {
        volatile BitReference a2, a3;
        unsigned int inventoryIndex = ItemInventory_FindCaddieItemIndex(AsCSharedDoc(g_sharedDoc)->itemInventory, typeIdArg);
        a2.word = &AsShopSaveFields(savedRecord)->newCaddieItems; a2.index = inventoryIndex;
        b2.word = &AsShopSaveFields(savedRecord)->newCaddieItems; b2.index = inventoryIndex;
        if (inventoryIndex >= 32) {
            RANGE_THROW_802C1540(error, bitsetTestRangeError);
        }
        int statusFlags = (*b2.word & (1u << inventoryIndex)) != 0;
        if (statusFlags) {
            a3.word = &AsShopSaveFields(savedRecord)->ownedCaddieItems; a3.index = inventoryIndex;
            b3.word = &AsShopSaveFields(savedRecord)->ownedCaddieItems; b3.index = inventoryIndex;
            if (inventoryIndex >= 32) {
                RANGE_THROW_802C1540(error2, bitsetTestRangeError);
            }
            statusFlags = (*b3.word & (1u << inventoryIndex)) == 0;
        }
        if (statusFlags) AsCShopProductsTab(self)->selectedProductNew = 1;
        volatile BitReference a4;
        a4.word = &AsShopSaveFields(currentRecord)->ownedCaddieItems; a4.index = inventoryIndex;
        b4.word = &AsShopSaveFields(currentRecord)->ownedCaddieItems; b4.index = inventoryIndex;
        if (inventoryIndex >= 32) {
            RANGE_THROW_802C1540(error3, bitsetTestRangeError);
        }
        if (*b4.word & (1u << inventoryIndex))
            AsCShopProductsTab(self)->selectedProductOwned = 1;
        if (AsSavePlayerRecord(currentRecord)->caddieIndex == (int)(((unsigned int)typeIdArg >> 21) & 31) &&
            AsShopSaveFields(currentRecord)->caddieItemIndex == (int)inventoryIndex)
            AsCShopProductsTab(self)->selectedProductEquipped = 1;
    }
    return 1;
}

#pragma opt_common_subs off
// Refresh new/owned/equipped club-set indicators with a 15-entry bounds check.
int CShopProductsTab_UpdateClubSetStatus(void* self, void* typeIdArg) {
    volatile TargetBitReference b, a;
    int inventoryIndex;
    struct Record { char padding[0x2d]; signed char selected; };
    struct Self { char padding[0x38]; Record* currentRecord; unsigned char statusFlags[3]; };
    Self* s = (Self*)self;
    Record* currentRecord = s->currentRecord;
    if (!Sing_805DC5E0()) {
        if (!s_saveDataInitialized) {
            SaveData_Ctor(s_saveData);
            s_saveDataInitialized = 1;
        }
        Sing_805DC5E0() = s_saveData;
    }
    char savedRecord[0x828];
    SaveRecord_CopyCtor(savedRecord, SaveData_GetCurrentRecord(Sing_805DC5E0()));
    inventoryIndex = ItemInventory_FindClubSetIndex(AsCSharedDoc(g_sharedDoc)->itemInventory, typeIdArg);
    a.word = &AsShopSaveFields(savedRecord)->newClubSets; a.index = inventoryIndex;
    b.word = &AsShopSaveFields(savedRecord)->newClubSets; b.index = inventoryIndex;
    if ((unsigned int)inventoryIndex >= kClubSetCount) {
        OutOfRangeStorage storage;
        OutOfRangeStorage* ep = &storage;
        storage.vtable = __vt__Q23std11logic_error;
        struct AllocSlot { char value; char padding[3]; } allocator;
        allocator.value = 0;
        unsigned int len = strlen(shopClubSetTestBoundsError) + 1; // shopClubSetTestBoundsError
        char* p = (char*)__nwa__FUl(len * 2 - len);
        storage.message = p;
        {
            std::TargetMessageGuard guard = {p, &allocator.value};
            storage.owner = new std::TargetMessageOwner(p);
            guard.pointer = 0;
        }
        strcpy(storage.message, shopClubSetTestBoundsError); // shopClubSetTestBoundsError
        storage.vtable = __vt__Q23std12out_of_range;
        __throw(shopProductsTabStrings + kShopProductsTabStrings_std_exception_std_logic_error_std_out_of_range, ep, (void*)__dt__Q23std12out_of_rangeFv);
    }
    int statusFlags = 0;
    if (*b.word & (1u << inventoryIndex)) {
        struct Entry { unsigned char flags; char padding; };
        unsigned char byte = ((ShopClubSetOwnership*)(savedRecord + IFF_OFFSET(ShopSaveFields, clubSets)))[inventoryIndex].flags;
        if (((int)((byte << 25) & 0xc0000000) >> 31) == 0) statusFlags = 1;
    }
    if (statusFlags) s->statusFlags[0] = 1;
    unsigned char* current = (unsigned char*)s->currentRecord;
    unsigned char byte = AsShopSaveFields(current)->clubSets[inventoryIndex].flags;
    if (((int)((byte << 25) & 0xc0000000) >> 31) != 0)
        s->statusFlags[1] = 1;
    if (currentRecord->selected == inventoryIndex) s->statusFlags[2] = 1;
    return 1;
}

#pragma pop
#pragma push
#pragma use_lmw_stmw on
// Refresh new/owned/equipped ball indicators with an 11-entry bounds check.
int CShopProductsTab_UpdateBallStatus(void* self, void* typeIdArg) {
    int inventoryIndex;
    register void* r31 = AsCShopProductsTab(self)->playerRecord;
    if (Sing_805DC5E0() == 0) {
        if (s_saveDataInitialized == 0) {
            SaveData_Ctor(s_saveData);
            s_saveDataInitialized = 1;
        }
        Sing_805DC5E0() = s_saveData;
    }
    void* r3 = SaveData_GetCurrentRecord(Sing_805DC5E0());
    char savedRecord[0x848];
    SaveRecord_CopyCtor(savedRecord, r3);

    struct Holder { char* p; char* statusFlags;
        // Guard the exception character allocation until ownership transfers.
        Holder(char* b, char* f):p(b),statusFlags(f){}
        // Free exception characters still held by this guard.
        ~Holder(){if(p)__dla__FPv(p);} };
    struct ErrBase {
        void* vt; char* savedRecord; void* ref;
        // Allocate and copy exception text, transferring it to a shared owner.
        ErrBase(const char* msg) {
            char done __attribute__((aligned(4)));
            vt = __vt__Q23std11logic_error;
            done = 0;
            unsigned n = strlen(msg) + 1;
            char* b = (char*)__nwa__FUl(n * 2 - n);
            savedRecord = b;
            {
                Holder h(b, &done);
                void* r = __nw__FUl(16);
                if (r) {
                    int table = (int)s_sharedCharDeleterVtable;
                    ((std::TargetMessageOwner*)r)->references=1;
                    ((std::TargetMessageOwner*)r)->owners=1;
                    ((std::TargetMessageOwner*)r)->vtable= (void*)table;
                    ((std::TargetMessageOwner*)r)->message= b;
                }
                ref = r;
                h.p = 0;
            }
            strcpy(savedRecord, msg);
        }
        // Retain the empty destructor boundary used by the exception ABI.
        ~ErrBase(){}
    };
    struct Err : ErrBase {
        // Throw the out-of-range exception with this constructed message owner.
        Err(const char* msg):ErrBase(msg){ vt = __vt__Q23std12out_of_range; __throw(shopProductsTabStrings + kShopProductsTabStrings_std_exception_std_logic_error_std_out_of_range, this, (void*)__dt__Q23std12out_of_rangeFv); }
        // Retain the empty derived exception destructor boundary.
        ~Err(){}
    };

    inventoryIndex = ItemInventory_FindBallIndex(AsCSharedDoc(g_sharedDoc)->itemInventory, typeIdArg);

    if ((unsigned int)inventoryIndex >= kBallCount) {
        Err e(shopBallTestBoundsError); // shopBallTestBoundsError
    }

    int sp1fc_val = *(int*)&AsShopSaveFields(savedRecord)->newBalls;
    int r5 = 0;
    if (sp1fc_val & (1 << inventoryIndex)) {
        signed char b = (signed char)savedRecord[IFF_OFFSET(ShopSaveFields, balls) + inventoryIndex];
        if ((((b & 0x60) << 25) >> 31) == 0) r5 = 1;
    }
    if (r5) AsCShopProductsTab(self)->selectedProductNew = 1;

    signed char b2 = AsShopSaveFields(AsCShopProductsTab(self)->playerRecord)->balls[inventoryIndex];
    if ((((b2 & 0x60) << 25) >> 31) != 0) AsCShopProductsTab(self)->selectedProductOwned = 1;

    signed char b3 = AsSavePlayerRecord(r31)->ballIndex;
    if (b3 == inventoryIndex) AsCShopProductsTab(self)->selectedProductEquipped = 1;

    return 1;
}

#pragma pop
// Display ownership/equipment state, the new badge and an optional item quantity.
int CShopProductsTab_UpdateSlotIndicators(void* self, int visibleSlot) {
    int indicatorState;
    if (AsCShopProductsTab(self)->productCount == 0) {
        indicatorState = kProductUnavailable;
    } else {
        if (AsCShopProductsTab(self)->selectedProductOwned != 0) {
            unsigned char equippedFlag = AsCShopProductsTab(self)->selectedProductEquipped;
            if (equippedFlag != 0) indicatorState = kProductOwnedAndEquipped;
            else indicatorState = kProductOwned;
        } else {
            int equippedFlag = AsCShopProductsTab(self)->selectedProductEquipped;
            indicatorState = (equippedFlag != 0) ? kProductEquippedOnly : kProductUnavailable;
        }
    }
    void* indicatorLayout = AsCShopProductsTab(self)->secondaryLayout;
    Layout_SetElementValueState(indicatorLayout, visibleSlot, (signed char)indicatorState);

    int showNewBadge;
    if (AsCShopProductsTab(self)->productCount == 0) showNewBadge = 0;
    else showNewBadge = AsCShopProductsTab(self)->selectedProductNew;

    void* scriptObject = AsCShopProductsTab(self)->scriptObject;
    int badgeGroupId = shopProductBadgeGroupIds[visibleSlot]; // shopProductBadgeGroupIds
    void* badgeGroup = UIGroup_GetOrCreateById(scriptObject, badgeGroupId);
    UIGroup_SetEnabled(badgeGroup, showNewBadge);

    int quantity = AsCShopProductsTab(self)->selectedProductQuantity;
    if (visibleSlot >= 0 && visibleSlot < kVisibleProductSlots) {
        if (quantity == kNoQuantity) {
            void* quantityText = AsCShopProductsTab(self)->numberDraw;
            NumericTextTable_ClearEntryText(quantityText, visibleSlot + 1);
        } else {
            void* quantityText = AsCShopProductsTab(self)->numberDraw;
            UITextTable_SetInteger(quantityText, visibleSlot + 1, quantity);
        }
    }
    return 1;
}

// Translate all carousel positions, hit rectangles and badge groups vertically.
int CShopProductsTab_ApplyDrawOffsetY(void* self) {
    double f31;
    unsigned int i;
    int idx;
    void* node;
    f31 = kSignedIntToDoubleBias_ShopCarousel; // kSignedIntToDoubleBias_ShopCarousel
    Layout_SetModeByte(AsCShopProductsTab(self)->layout, 0);
    Layout_SetModeByte(AsCShopProductsTab(self)->secondaryLayout, 0);
    Layout_SetModeByte(AsCShopProductsTab(self)->tertiaryLayout, 0);

    float fbuf[2] = { 0.0f, 0.0f };
    int ibuf[4] = { 0, 0, 0, 0 };

    idx = 0;
    while (idx < 2) {
        Layout_GetElementPosition(AsCShopProductsTab(self)->layout, idx, fbuf);
        fbuf[1] = fbuf[1] + (*(float*)&AsCShopProductsTab(self)->drawOffsetY);
        Layout_SetElementPosition(AsCShopProductsTab(self)->layout, idx, fbuf);
        Layout_ReadElementRect(AsCShopProductsTab(self)->layout, idx, ibuf);

        int r0 = ibuf[1];
        int r3 = ibuf[3];
        float d3 = (float)r3;
        d3 = d3 + (*(float*)&AsCShopProductsTab(self)->drawOffsetY);
        ibuf[3] = (int)d3;
        float d1 = (float)r0 + (*(float*)&AsCShopProductsTab(self)->drawOffsetY);
        ibuf[1] = (int)d1;

        Layout_WriteElementRect(AsCShopProductsTab(self)->layout, idx, ibuf);

        idx = idx + 1;
    }

    idx = 0;
    while (idx <= 4) {
        Layout_GetElementPosition(AsCShopProductsTab(self)->secondaryLayout, idx, fbuf);
        fbuf[1] = fbuf[1] + (*(float*)&AsCShopProductsTab(self)->drawOffsetY);
        Layout_SetElementPosition(AsCShopProductsTab(self)->secondaryLayout, idx, fbuf);
        Layout_ReadElementRect(AsCShopProductsTab(self)->secondaryLayout, idx, ibuf);

        int r0 = ibuf[1];
        int r3 = ibuf[3];
        float d3 = (float)r3;
        d3 = d3 + (*(float*)&AsCShopProductsTab(self)->drawOffsetY);
        ibuf[3] = (int)d3;
        float d1 = (float)r0 + (*(float*)&AsCShopProductsTab(self)->drawOffsetY);
        ibuf[1] = (int)d1;

        Layout_WriteElementRect(AsCShopProductsTab(self)->secondaryLayout, idx, ibuf);

        idx = idx + 1;
    }

    i = 0;
    while (i < 0xb) {
        node = UIGroup_GetOrCreateById(AsCShopProductsTab(self)->scriptObject, shopProductBadgeGroupIds[i]); // shopProductBadgeGroupIds
        UIGroup_GetPosition(node, fbuf);
        fbuf[1] = fbuf[1] + (*(float*)&AsCShopProductsTab(self)->drawOffsetY);
        UIGroup_SetPosition(node, fbuf);
        i = i + 1;
    }

    return 1;
}

#pragma opt_common_subs off
// Acknowledge a viewed product by clearing its category-specific new-item bit.
int CShopProductsTab_ClearProductFlag(void* self, void* typeIdArg) {
    struct Holder { char* word; char* flag;
        // Guard the exception character allocation until ownership transfers.
        Holder(char* b, char* f):word(b),flag(f){}
        // Free exception characters still held by this guard.
        ~Holder(){if(word)__dla__FPv(word);} };
    struct ErrBase {
        void* vt; char* buf; void* ref;
        // Allocate and copy exception text, transferring it to a shared owner.
        ErrBase(const char* msg) {
            char done __attribute__((aligned(4)));
            vt = __vt__Q23std11logic_error;
            done = 0;
            unsigned n = strlen(msg) + 1;
            char* b = (char*)__nwa__FUl(n * 2 - n);
            buf = b;
            {
                Holder h(b, &done);
                void* r = __nw__FUl(16);
                if (r) {
                    int table = (int)s_sharedCharDeleterVtable;
                    ((std::TargetMessageOwner*)r)->references=1;
                    ((std::TargetMessageOwner*)r)->owners=1;
                    ((std::TargetMessageOwner*)r)->vtable= (void*)table;
                    ((std::TargetMessageOwner*)r)->message= b;
                }
                ref = r;
                h.word = 0;
            }
            strcpy(buf, msg);
        }
        // Retain the empty destructor boundary used by the exception ABI.
        ~ErrBase(){}
    };
    struct Err : ErrBase {
        // Throw the out-of-range exception with this constructed message owner.
        Err(const char* msg):ErrBase(msg){ vt = __vt__Q23std12out_of_range; __throw(shopProductsTabStrings + kShopProductsTabStrings_std_exception_std_logic_error_std_out_of_range, this, (void*)__dt__Q23std12out_of_rangeFv); }
        // Retain the empty derived exception destructor boundary.
        ~Err(){}
    };

    unsigned itemKind = ((unsigned)typeIdArg) >> kTypeIdKindShift;
    void* playerRecord = AsCShopProductsTab(self)->playerRecord;
    switch (itemKind) {
    case kItemCharacter: {
        unsigned inventoryIndex = ItemInventory_FindCharacterIndex(AsCSharedDoc(g_sharedDoc)->itemInventory, typeIdArg);
        if (inventoryIndex >= 8) { Err e(shopCharacterOrCaddieClearBoundsError); }
        unsigned remainingBits = 1 << inventoryIndex;
        unsigned newItemBits = *&AsShopSaveFields(playerRecord)->newCharacters;
        remainingBits = newItemBits & ~remainingBits;
        *&AsShopSaveFields(playerRecord)->newCharacters = remainingBits;
        break;
    }
    case kItemPart: {
        unsigned inventoryIndex = ItemInventory_FindPartIndex(AsCSharedDoc(g_sharedDoc)->itemInventory, typeIdArg);
        if (inventoryIndex >= kPartCount) { Err e(bitset782ResetRangeErrorText); }
        int wordIndex = (int)(inventoryIndex >> 5);
        unsigned* word = AsShopSaveFields(playerRecord)->newParts + wordIndex;
        unsigned newItemBits = *word;
        unsigned itemMask = 1u << (inventoryIndex & 31);
        *word = newItemBits & ~itemMask;
        break;
    }
    case kItemCaddie: {
        unsigned inventoryIndex = ItemInventory_FindCaddieIndex(AsCSharedDoc(g_sharedDoc)->itemInventory, typeIdArg);
        if (inventoryIndex >= 8) { Err e(shopCharacterOrCaddieClearBoundsError); }
        unsigned remainingBits = 1 << inventoryIndex;
        unsigned newItemBits = *&AsShopSaveFields(playerRecord)->newCaddies;
        remainingBits = newItemBits & ~remainingBits;
        *&AsShopSaveFields(playerRecord)->newCaddies = remainingBits;
        break;
    }
    case kItemCaddieCostume: {
        unsigned inventoryIndex = ItemInventory_FindCaddieItemIndex(AsCSharedDoc(g_sharedDoc)->itemInventory, typeIdArg);
        if (inventoryIndex >= kCaddieCostumeCount) { Err e(shopCaddieCostumeClearBoundsError); }
        unsigned remainingBits = 1 << inventoryIndex;
        unsigned newItemBits = *&AsShopSaveFields(playerRecord)->newCaddieItems;
        remainingBits = newItemBits & ~remainingBits;
        *&AsShopSaveFields(playerRecord)->newCaddieItems = remainingBits;
        break;
    }
    case kItemClubSet: {
        unsigned inventoryIndex = ItemInventory_FindClubSetIndex(AsCSharedDoc(g_sharedDoc)->itemInventory, typeIdArg);
        if (inventoryIndex >= kClubSetCount) { Err e(shopClubSetClearBoundsError); }
        unsigned remainingBits = 1 << inventoryIndex;
        unsigned newItemBits = *&AsShopSaveFields(playerRecord)->newClubSets;
        remainingBits = newItemBits & ~remainingBits;
        *&AsShopSaveFields(playerRecord)->newClubSets = remainingBits;
        break;
    }
    case kItemBall: {
        unsigned inventoryIndex = ItemInventory_FindBallIndex(AsCSharedDoc(g_sharedDoc)->itemInventory, typeIdArg);
        if (inventoryIndex >= kBallCount) { Err e(shopBallClearBoundsError); }
        unsigned remainingBits = 1 << inventoryIndex;
        unsigned newItemBits = *&AsShopSaveFields(playerRecord)->newBalls;
        remainingBits = newItemBits & ~remainingBits;
        *&AsShopSaveFields(playerRecord)->newBalls = remainingBits;
        break;
    }
    case 0: case 3: case kItemConsumable:


        break;
    default:
        break;
    }
    return 1;
}

#pragma opt_common_subs reset
// Initialize the shop angle-wrap constant at startup.
void Shop_InitAngleWrapConstant() {
    s_shopFullTurnAngle = kFloat_6_2831855; // s_shopFullTurnAngle
}

#pragma opt_common_subs off
// Copy the player save into a shop working record and create product lists, previews and dialogs.
// Bind character products to the working snapshot and load its selected character icons.
static inline void BindShopCharacterProducts(void* self, void* workingRecord) {
    CBuildProduct_SelectSaveRecord(AsCPangya2Shop(self)->charaProductBuilder, workingRecord);
    CBuildProductChara_LoadProductTextures(AsCPangya2Shop(self)->charaProductBuilder, AsSavePlayerRecord(workingRecord)->characterIndex);
}

// Bind caddie products to the same snapshot and load its selected caddie icons.
static inline void BindShopCaddieProducts(void* self, void* workingRecord) {
    CBuildProduct_SelectSaveRecord(AsCPangya2Shop(self)->caddieProductBuilder, (char*)&AsCPangya2Shop(self)->saveRecordSnapshotStart);
    CBuildProductCaddie_LoadIconGroups(AsCPangya2Shop(self)->caddieProductBuilder, AsSavePlayerRecord(workingRecord)->caddieIndex);
}

// Bind the item builder to the working snapshot before building its texture lists.
static inline void BindShopItemProducts(void* self) {
    CBuildProduct_SelectSaveRecord(AsCPangya2Shop(self)->itemProductBuilder, (char*)&AsCPangya2Shop(self)->saveRecordSnapshotStart);
    ShopItemList_BuildProductTextures(AsCPangya2Shop(self)->itemProductBuilder);
}

// Open each preview operation's own resources before product lists borrow builder data.
static inline void OpenShopPreviewResources(void* self) {
    CModelOperationChara_OpenResource(AsCPangya2Shop(self)->charaModelOperation);
    CModelOperationCaddie_OpenResource(AsCPangya2Shop(self)->caddieModelOperation);
    CModelOperationItem_OpenResource(AsCPangya2Shop(self)->primaryItemModelOperation);
    CModelOperationItem_OpenResource(AsCPangya2Shop(self)->secondaryItemModelOperation);
}

// shopStorage is caller-owned construction storage; playerSaveRecord is borrowed
// only for copying. The embedded snapshot owns preview equipment/balance changes.
// Lists borrow builder records/draw entries and point at that snapshot; the shop
// owns builders, previews, layouts/dialogs and packed-file handles created here.
// Phase map: allocate members -> open UI/resources -> copy save -> bind five lists
// -> bind caddie/item/status/cart/dialog -> preload archives. Return shopStorage.
// Dependency map for category edits:
// character lists[0..4] <- charaProductBuilder <- copied characterIndex + workingRecord;
// caddie textures <- caddieProductBuilder <- copied caddieIndex + workingRecord;
// item textures/cart <- itemProductBuilder category 2 <- workingRecord;
// playerStatus and level-up dialog also borrow workingRecord. Model operations
// own their resources separately. Build/copy order is required before these bindings.
#pragma push
#pragma always_inline on
// Copy save blocks into the shop-owned snapshot, retaining copy widths and volatile tail stores.
// This copy retains no pointers into sourceRecord.
static inline void CopyShopWorkingSaveBlocks(void* self, void* sourceRecord) {
    struct Words151 { unsigned v[151]; };
    struct Words28 { unsigned v[28]; };
    struct Shorts208 { unsigned short v[208]; };
    (*(Words151*)&AsCPangya2Shop(self)->saveRecordSnapshotStart) = *(Words151*)sourceRecord;
    *(Words28*)((ShopSaveFields*)&AsCPangya2Shop(self)->saveRecordSnapshotStart)->recordBlock25C = *(Words28*)AsShopSaveFields(sourceRecord)->recordBlock25C;
    *(Shorts208*)((ShopSaveFields*)&AsCPangya2Shop(self)->saveRecordSnapshotStart)->characters = *(Shorts208*)AsShopSaveFields(sourceRecord)->characters;

    struct WordPair { unsigned v[2]; };
    // copy scalars
    *(WordPair*)&((ShopSaveFields*)&AsCPangya2Shop(self)->saveRecordSnapshotStart)->caddies[0] = *(WordPair*)&AsShopSaveFields(sourceRecord)->caddies[0];
    *(WordPair*)&((ShopSaveFields*)&AsCPangya2Shop(self)->saveRecordSnapshotStart)->caddies[1] = *(WordPair*)&AsShopSaveFields(sourceRecord)->caddies[1];
    *(WordPair*)&((ShopSaveFields*)&AsCPangya2Shop(self)->saveRecordSnapshotStart)->caddies[2] = *(WordPair*)&AsShopSaveFields(sourceRecord)->caddies[2];
    *(WordPair*)&((ShopSaveFields*)&AsCPangya2Shop(self)->saveRecordSnapshotStart)->caddies[3] = *(WordPair*)&AsShopSaveFields(sourceRecord)->caddies[3];
    *(WordPair*)&((ShopSaveFields*)&AsCPangya2Shop(self)->saveRecordSnapshotStart)->caddies[4] = *(WordPair*)&AsShopSaveFields(sourceRecord)->caddies[4];
    *(WordPair*)&((ShopSaveFields*)&AsCPangya2Shop(self)->saveRecordSnapshotStart)->caddies[5] = *(WordPair*)&AsShopSaveFields(sourceRecord)->caddies[5];
    *(WordPair*)&((ShopSaveFields*)&AsCPangya2Shop(self)->saveRecordSnapshotStart)->caddies[6] = *(WordPair*)&AsShopSaveFields(sourceRecord)->caddies[6];
    *(WordPair*)&((ShopSaveFields*)&AsCPangya2Shop(self)->saveRecordSnapshotStart)->caddies[7] = *(WordPair*)&AsShopSaveFields(sourceRecord)->caddies[7];
    struct Bytes24 { unsigned char v[24]; };
    *(Bytes24*)((ShopSaveFields*)&AsCPangya2Shop(self)->saveRecordSnapshotStart)->clubSets = *(Bytes24*)AsShopSaveFields(sourceRecord)->clubSets;
    AsCPangya2Shop(self)->clubSet12OwnershipFlags = (unsigned char)AsShopSaveFields(sourceRecord)->clubSets[12].flags;
    AsCPangya2Shop(self)->clubSet12Reserved = (unsigned char)AsShopSaveFields(sourceRecord)->clubSets[12].reserved;
    AsCPangya2Shop(self)->clubSet13OwnershipFlags = (unsigned char)AsShopSaveFields(sourceRecord)->clubSets[13].flags;
    AsCPangya2Shop(self)->clubSet13Reserved = (unsigned char)AsShopSaveFields(sourceRecord)->clubSets[13].reserved;
    AsCPangya2Shop(self)->clubSet14OwnershipFlags = (unsigned char)AsShopSaveFields(sourceRecord)->clubSets[14].flags;
    AsCPangya2Shop(self)->clubSet14Reserved = (unsigned char)AsShopSaveFields(sourceRecord)->clubSets[14].reserved;
    struct Bytes8 { unsigned char v[8]; };
    *(Bytes8*)((ShopSaveFields*)&AsCPangya2Shop(self)->saveRecordSnapshotStart)->balls = *(Bytes8*)AsShopSaveFields(sourceRecord)->balls;
    *(volatile unsigned char*)&((ShopSaveFields*)&AsCPangya2Shop(self)->saveRecordSnapshotStart)->balls[8] = *(volatile unsigned char*)&AsShopSaveFields(sourceRecord)->balls[8];
    *(volatile unsigned char*)&((ShopSaveFields*)&AsCPangya2Shop(self)->saveRecordSnapshotStart)->balls[9] = *(volatile unsigned char*)&AsShopSaveFields(sourceRecord)->balls[9];
    *(volatile unsigned char*)&((ShopSaveFields*)&AsCPangya2Shop(self)->saveRecordSnapshotStart)->balls[10] = *(volatile unsigned char*)&AsShopSaveFields(sourceRecord)->balls[10];
    *(WordPair*)((ShopSaveFields*)&AsCPangya2Shop(self)->saveRecordSnapshotStart)->recordBlock4D8 = *(WordPair*)AsShopSaveFields(sourceRecord)->recordBlock4D8;
    *(WordPair*)((ShopSaveFields*)&AsCPangya2Shop(self)->saveRecordSnapshotStart)->recordBlock4E0 = *(WordPair*)AsShopSaveFields(sourceRecord)->recordBlock4E0;
    AsCPangya2Shop(self)->practiceFlags = AsShopSaveFields(sourceRecord)->practiceFlags;

    struct Shorts414 { unsigned short v[414]; };
    *(Shorts414*)((ShopSaveFields*)&AsCPangya2Shop(self)->saveRecordSnapshotStart)->recordBlock4EC = *(Shorts414*)AsShopSaveFields(sourceRecord)->recordBlock4EC;

}
#pragma pop

// Construct the shop and bind previews, products, and dialogs to its save snapshot.
void* CPangya2Shop_Ctor(void* shopStorage, void* playerSaveRecord, int characterEntryMode) {
    struct ShopTabStorage { char data[0x48];
        // Construct a product tab that borrows its working save record.
        ShopTabStorage(void* pane, Vec3f pos, void* base) { CShopProductsTab_Ctor(this, (int)pane, &pos, base); }
    };
    struct ShopLayoutStorage { char data[0x5c];
        // Initialize the owned shop layout.
        ShopLayoutStorage(int a) { Layout_Init(this, a); }
    };
    struct ProductCategoryStorage { char data[0x24];
        // Initialize the category selector.
        ProductCategoryStorage(int a) { CProductsCategory_Ctor(this, a); }
    };
    struct CharacterPreviewStorage { char data[0x8c];
        // Initialize the character preview operation.
        CharacterPreviewStorage() { __ct__20CModelOperationCharaFv(this); }
    };
    struct CaddiePreviewStorage { char data[0x20];
        // Initialize the caddie preview operation.
        CaddiePreviewStorage() { CModelOperationCaddie_Ctor(this); }
    };
    struct ItemPreviewStorage { char data[0x20];
        // Initialize a club/ball preview operation for the supplied mode.
        ItemPreviewStorage(int a) { CModelOperationItem_Ctor(this, a); }
    };
    struct CharacterProductBuilderStorage { char data[0x2d88];
        // Initialize the character/outfit product builder.
        CharacterProductBuilderStorage(int a, int b) { __ct__18CBuildProductCharaFiUc(this, a, b); }
    };
    struct CaddieProductBuilderStorage { char data[0x2d74];
        // Initialize the caddie product builder.
        CaddieProductBuilderStorage(int a, int b) { CBuildProductCaddie_Ctor(this, a, b); }
    };
    struct ItemProductBuilderStorage { char data[0x2d74];
        // Initialize the item product builder.
        ItemProductBuilderStorage(int a, int b) { CBuildProductItem_Ctor(this, a, b); }
    };
    struct ClubHouseMessageStorage { char data[0x20];
        // Initialize an owned shop message panel.
        ClubHouseMessageStorage() { __ct__13CClubHouseMsgFv(this); }
    };
    struct ClubSubMenuStorage { char data[0x1c];
        // Initialize the owned club submenu.
        ClubSubMenuStorage() { CClubSubMenu_Ctor(this); }
    };
    struct ClubDialogStorage { char data[0x1c];
        // Initialize the owned general club dialog.
        ClubDialogStorage() { CClubDialog_Ctor(this); }
    };
    struct PurchaseDialogStorage { char data[0x44];
        // Initialize the owned purchase dialog.
        PurchaseDialogStorage() { CPurchaseDialog_Ctor(this); }
    };
    struct ItemInfoStorage { char data[0x1c];
        // Initialize the owned item-information panel.
        ItemInfoStorage() { CItemInfo_Ctor(this); }
    };
    struct PackTextureStorage { char data[0x20];
        // Initialize the owned packed texture-resource handle.
        PackTextureStorage(void* a, int b, void* c) { CPackTextureResource_Init(this, a, b, c); }
    };
    struct PlayerStatusStorage { char data[0xa8];
        // Initialize player status against the borrowed shop snapshot.
        PlayerStatusStorage(void* a) { CPlayerStatus_Ctor(this, a); }
    };
    struct ItemCartStorage { char data[0x34];
        // Initialize the cart against the borrowed snapshot and product count.
        ItemCartStorage(void* a, void* b) { CItemCart_Ctor(this, a, b); }
    };
    struct ClubLevelUpDialogStorage { char data[0x30];
        // Initialize the level-up dialog against the borrowed snapshot.
        ClubLevelUpDialogStorage(void* a) { CClubLvUpDialog_Ctor(this, a); }
    };
    void* self = shopStorage;
    void* sourceRecord = playerSaveRecord;
    int entryMode = characterEntryMode;
    char* destructorRegistrations = s_inputManagerDtorRegistration_80516C10;
    void* constructedMember;
    void* constructedMemberOrArchiveCursor;
    void* productRecords;
    int listOffsetY;
    int listPointerByteOffset;
    void* workingRecord;
    int listIndex;
    // Phase 1: establish base/snapshot storage and allocate owned UI/model members.
    MPBase1_Ctor(self);
    void* vtable = (void*)__vt__12CPangya2Shop;
    AsCPangya2Shop(self)->vtbl = vtable;
    SaveData_InitPlayerRecord((char*)&AsCPangya2Shop(self)->saveRecordSnapshotStart, vtable);

    // 0x840 = 0, 0x844 = 1
    AsCPangya2Shop(self)->categoryIndex = 0;
    AsCPangya2Shop(self)->selectedListIndex = 1;
    void* p = __nwa__FUl(0x14);
    AsCPangya2Shop(self)->productLists = p;
    (*(int*)&AsCPangya2Shop(self)->uiGroup) = 0;

    // new 0x5c -> Layout_Init(p,5)
    constructedMember = new ShopLayoutStorage(5);
    AsCPangya2Shop(self)->shopLayout = constructedMember;
    AsCPangya2Shop(self)->uiGroupEntryId = -1;
    (*(int*)&AsCPangya2Shop(self)->scriptObject) = 0;
    AsCPangya2Shop(self)->scriptEntryId = -1;

    constructedMember = new ProductCategoryStorage(5);
    AsCPangya2Shop(self)->productsCategory = constructedMember;

    constructedMember = new CharacterPreviewStorage();
    AsCPangya2Shop(self)->charaModelOperation = constructedMember;

    constructedMember = new CaddiePreviewStorage();
    AsCPangya2Shop(self)->caddieModelOperation = constructedMember;

    constructedMember = new ItemPreviewStorage(1);
    AsCPangya2Shop(self)->primaryItemModelOperation = constructedMember;

    constructedMember = new ItemPreviewStorage(0);
    AsCPangya2Shop(self)->secondaryItemModelOperation = constructedMember;

    constructedMember = new CharacterProductBuilderStorage(kShopCharacterBuilderMode, entryMode);
    AsCPangya2Shop(self)->charaProductBuilder = constructedMember;

    constructedMember = new CaddieProductBuilderStorage(kShopCaddieBuilderMode, entryMode);
    AsCPangya2Shop(self)->caddieProductBuilder = constructedMember;

    constructedMember = new ItemProductBuilderStorage(kShopItemBuilderMode, entryMode);
    AsCPangya2Shop(self)->itemProductBuilder = constructedMember;

    (*(int*)&AsCPangya2Shop(self)->playerStatus) = 0;
    (*(int*)&AsCPangya2Shop(self)->itemCart) = 0;
    (*(int*)&AsCPangya2Shop(self)->textTable) = 0;

    constructedMember = new ClubHouseMessageStorage();
    AsCPangya2Shop(self)->productMessage = constructedMember;

    constructedMember = new ClubHouseMessageStorage();
    AsCPangya2Shop(self)->subMenuMessage = constructedMember;

    constructedMember = new ClubSubMenuStorage();
    AsCPangya2Shop(self)->clubSubMenu = constructedMember;
    AsCPangya2Shop(self)->clubSubMenuOpen = 0;

    constructedMember = new ClubDialogStorage();
    AsCPangya2Shop(self)->clubDialog = constructedMember;
    AsCPangya2Shop(self)->clubDialogOpen = 0;

    constructedMember = new PurchaseDialogStorage();
    AsCPangya2Shop(self)->purchaseDialog = constructedMember;
    AsCPangya2Shop(self)->purchaseDialogOpen = 0;

    constructedMemberOrArchiveCursor = new ItemInfoStorage();
    AsCPangya2Shop(self)->itemInfo = constructedMemberOrArchiveCursor;
    AsCPangya2Shop(self)->itemInfoOpen = 0;
    AsCPangya2Shop(self)->noCaddieSelected = 0;
    (*(int*)&AsCPangya2Shop(self)->saveMenu) = 0;
    AsCPangya2Shop(self)->saveMenuActive = 0;
    AsCPangya2Shop(self)->characterEntryMode = (unsigned char)entryMode;
    AsCPangya2Shop(self)->entryInitialized = 0;
    (*(int*)&AsCPangya2Shop(self)->clubLvUpDialog) = 0;
    AsCPangya2Shop(self)->clubLvUpDialogOpen = 0;

    // Phase 2: prepare loading/texture resources and open the two UI script entries.
    void* r3 = __nwa__FUl(0xc);
    void* singleton610 = Sing_805DC610();
    AsCPangya2Shop(self)->packedFiles = r3;
    (*(int*)&AsCPangya2Shop(self)->packTextureResource) = 0;

    if (singleton610 == 0) {
        if (s_nowReloadingInitialized == 0) {
            CNowReloading_Init((void*)&s_nowReloading);
            __register_global_object((void*)&s_nowReloading, (void*)CNowReloading_dtor, (void*)((destructorRegistrations + 0x48)));
            s_nowReloadingInitialized = 1;
        }
        Sing_805DC610() = &s_nowReloading;
    }

    {
        listPointerByteOffset = (int)Sing_805DC610();
        if (listPointerByteOffset) {
            if (LoadingIndicator_IsAbsentOrState3((void*)listPointerByteOffset)) LoadingIndicator_CreatePlayer((void*)listPointerByteOffset);
        }
    }

    int tmp8 = 0;
    if (Sing_805DC620() == 0) {
        if (s_resourceTableInitialized == 0) {
            s_resourceTableStorage = 0;
            __register_global_object((void*)&s_resourceTableStorage, (void*)Allocation_Dtor, (void*)((destructorRegistrations + 0x3c)));
            s_resourceTableInitialized = 1;
        }
        Sing_805DC620() = &s_resourceTableStorage;
    }

    {
        listPointerByteOffset = (int)ResourceTable_FindEntryByName(Sing_805DC620(), shopStrings, &tmp8);
        constructedMemberOrArchiveCursor = new PackTextureStorage(shopStrings, tmp8, (void*)listPointerByteOffset);
        AsCPangya2Shop(self)->packTextureResource = constructedMemberOrArchiveCursor;
    }

    // Reset five unresolved shop words (+0x848..+0x858) to the same baseline
    // used by the later shop reset path in pangya2_shop_product_screen.cpp. Named-field searches find
    // only these writes; reads may use raw offsets elsewhere. Their ownership
    // and consumers are unresolved: keep all five zero stores together until
    // those consumers are identified, rather than assigning guessed UI meanings.
    AsCPangya2Shop(self)->unk_848 = 0;
    AsCPangya2Shop(self)->unk_84C = 0;
    AsCPangya2Shop(self)->unk_850 = 0;
    AsCPangya2Shop(self)->unk_854 = 0;
    AsCPangya2Shop(self)->unk_858 = 0;
    Layout_LoadExtended(AsCPangya2Shop(self)->shopLayout, 0, (void*)((char*)shopStrings + kShopStrings_club_base_spd));

    // Create the shared BP controller on first use.
    if (Sing_805DC5F0() == 0) {
        if (s_bpCtrlInitialized == 0) {
            __ct__7CBpCtrlFv((void*)s_bpCtrl);
            __register_global_object((void*)s_bpCtrl, (void*)CBpCtrl_dtor, (void*)((destructorRegistrations + 0xc)));
            s_bpCtrlInitialized = 1;
        }
        Sing_805DC5F0() = s_bpCtrl;
    }
    CBpCtrl_CreateEntry(Sing_805DC5F0(), (void*)((char*)shopStrings + kShopStrings_club_base_b2d), (char*)&AsCPangya2Shop(self)->uiGroupEntryId, 0);

    if (Sing_805DC5F0() == 0) {
        if (s_bpCtrlInitialized == 0) {
            __ct__7CBpCtrlFv((void*)s_bpCtrl);
            __register_global_object((void*)s_bpCtrl, (void*)CBpCtrl_dtor, (void*)((destructorRegistrations + 0xc)));
            s_bpCtrlInitialized = 1;
        }
        Sing_805DC5F0() = s_bpCtrl;
    }
    void* bp = IdMap_Lookup(Sing_805DC5F0(), AsCPangya2Shop(self)->uiGroupEntryId);
    void* singleton5F0 = Sing_805DC5F0();
    AsCPangya2Shop(self)->uiGroup = bp;

    if (singleton5F0 == 0) {
        if (s_bpCtrlInitialized == 0) {
            __ct__7CBpCtrlFv((void*)s_bpCtrl);
            __register_global_object((void*)s_bpCtrl, (void*)CBpCtrl_dtor, (void*)((destructorRegistrations + 0xc)));
            s_bpCtrlInitialized = 1;
        }
        Sing_805DC5F0() = s_bpCtrl;
    }
    CBpCtrl_CreateEntry(Sing_805DC5F0(), (void*)((char*)shopStrings + kShopStrings_club_setcos_info_b2d), (char*)&AsCPangya2Shop(self)->scriptEntryId, 0);

    if (Sing_805DC5F0() == 0) {
        if (s_bpCtrlInitialized == 0) {
            __ct__7CBpCtrlFv((void*)s_bpCtrl);
            __register_global_object((void*)s_bpCtrl, (void*)CBpCtrl_dtor, (void*)((destructorRegistrations + 0xc)));
            s_bpCtrlInitialized = 1;
        }
        Sing_805DC5F0() = s_bpCtrl;
    }
    (*(int*)&AsCPangya2Shop(self)->scriptObject) = (int)IdMap_Lookup(Sing_805DC5F0(), AsCPangya2Shop(self)->scriptEntryId);

    OpenShopPreviewResources(self);

    // Phase 3: copy the save by fixed ABI blocks. This is a working snapshot,
    // not a pointer alias to sourceRecord; flag/quantity writes stay inside the shop.
    CopyShopWorkingSaveBlocks(self, sourceRecord);

    // Phase 4: bind the copied save, load character products, create five tabs
    // separated vertically by 0x22 pixels; tabs borrow builder arrays and snapshot.
    workingRecord = (char*)&AsCPangya2Shop(self)->saveRecordSnapshotStart;
    BindShopCharacterProducts(self, workingRecord);
    listIndex = 0;
    listOffsetY = 0;
    listPointerByteOffset = 0;
    for (; listIndex < kShopListCount; listIndex++, listPointerByteOffset += 4, listOffsetY += kShopListPitchPixels) {
        Vec3f pos = { 0.0f, 0.0f, 0.0f };
        pos.y = (float)listOffsetY;
        constructedMemberOrArchiveCursor = new ShopTabStorage(CBuildProduct_GetProductCount(AsCPangya2Shop(self)->charaProductBuilder, listIndex), pos, workingRecord);
        *(void**)((char*)AsCPangya2Shop(self)->productLists + listPointerByteOffset) = constructedMemberOrArchiveCursor;
        productRecords = CBuildProduct_GetProductRecords(AsCPangya2Shop(self)->charaProductBuilder, listIndex);
        void* productDrawEntries = CBuildProduct_GetProductDrawEntries(AsCPangya2Shop(self)->charaProductBuilder, listIndex);
        CShopProductsTab_SetProductArrays(*(void**)((char*)AsCPangya2Shop(self)->productLists + listPointerByteOffset), productDrawEntries, productRecords);
    }

    // Phase 5: bind caddie/item builders and the status/cart/dialog to the same snapshot.
    BindShopCaddieProducts(self, workingRecord);
    BindShopItemProducts(self);

    constructedMemberOrArchiveCursor = new PlayerStatusStorage((char*)&AsCPangya2Shop(self)->saveRecordSnapshotStart);
    AsCPangya2Shop(self)->playerStatus = constructedMemberOrArchiveCursor;

    constructedMemberOrArchiveCursor = new ItemCartStorage((char*)&AsCPangya2Shop(self)->saveRecordSnapshotStart, CBuildProduct_GetProductCount(AsCPangya2Shop(self)->itemProductBuilder, 2));
    AsCPangya2Shop(self)->itemCart = constructedMemberOrArchiveCursor;
    CItemCart_SetItemIcons(AsCPangya2Shop(self)->itemCart, CBuildProduct_GetProductDrawEntries(AsCPangya2Shop(self)->itemProductBuilder, 2));
    CItemCart_SetItemRecords(AsCPangya2Shop(self)->itemCart, CBuildProduct_GetProductRecords(AsCPangya2Shop(self)->itemProductBuilder, 2));

    constructedMemberOrArchiveCursor = new ClubLevelUpDialogStorage((char*)&AsCPangya2Shop(self)->saveRecordSnapshotStart);
    void* singleton830 = Sing_805DC830();
    AsCPangya2Shop(self)->clubLvUpDialog = constructedMemberOrArchiveCursor;

    if (singleton830 == 0) {
        if (s_pointerVectorInitialized == 0) {
            PointerVector_InitReserved((void*)s_fontNumDrawRegistry);
            __register_global_object((void*)s_fontNumDrawRegistry, (void*)PointerVector_DestroyStorage, (void*)((destructorRegistrations + 0x18)));
            s_pointerVectorInitialized = 1;
        }
        Sing_805DC830() = s_fontNumDrawRegistry;
    }
    (*(int*)&AsCPangya2Shop(self)->textTable) = (int)CFontNumDraw_CreateInVector(Sing_805DC830());

    {
        // Phase 6: retain the three archive handles for subsequent loading/polling.
        constructedMemberOrArchiveCursor = kShopPreloadArchiveNames;
        unsigned int finalIndex = 0;
        int packedFileOffset = 0;
        for (; finalIndex < 3; constructedMemberOrArchiveCursor = (char*)constructedMemberOrArchiveCursor + 4) {
            void* packedFile = CPackedFile_Create(*(void**)constructedMemberOrArchiveCursor);
            *(void**)((char*)AsCPangya2Shop(self)->packedFiles + packedFileOffset) = packedFile; // stwx
            finalIndex++;
            packedFileOffset += 4;
        }
    }

    return self;
}

#undef RANGE_THROW_802C1540

} // extern "C"

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU804624E8", _ctors_u802BEFE0_0, TopMenu_InitAngleWrapConstant); // TopMenu_InitAngleWrapConstant
STATIC_INIT_ENTRY(".CRT$XCU804624E8", _ctors_u802BEFE0_1, Shop_InitAngleWrapConstant); // Shop_InitAngleWrapConstant
#pragma pop

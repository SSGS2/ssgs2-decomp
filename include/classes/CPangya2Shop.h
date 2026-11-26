#ifndef CLASSES_CPANGYA2SHOP_H
#define CLASSES_CPANGYA2SHOP_H

#include "game/types.h"

struct VectorU32;
struct EntryList;

class CPangya2Shop {
public:
    void* vtbl;                            // 0x0: original vtable pointer
    void* layout;                          // 0x4
    char pad_008[0x10];                    // 0x8
    void* saveRecordSnapshotStart;         // 0x18
    char pad_01C[0x1c];                    // 0x1c
    int updateFrameCount;                  // 0x38
    char pad_03C[0x4];                     // 0x3c
    void* pangBalance;                     // 0x40
    signed char currentCharaIndex;         // 0x44
    signed char clubSetIndex;              // 0x45
    signed char ballIndex;                 // 0x46
    signed char currentCaddieIndex;        // 0x47
    signed char caddieItemIndex;           // 0x48: CPangya2Shop_RefreshProductLists
    char pad_049[0x493];                   // 0x49
    unsigned char clubSet12OwnershipFlags; // 0x4dc
    unsigned char clubSet12Reserved;       // 0x4dd
    unsigned char clubSet13OwnershipFlags; // 0x4de
    unsigned char clubSet13Reserved;       // 0x4df
    unsigned char clubSet14OwnershipFlags; // 0x4e0
    unsigned char clubSet14Reserved;       // 0x4e1
    char pad_4E2[0x1e];                    // 0x4e2
    unsigned int practiceFlags;            // 0x500
    char pad_504[0x33c];                   // 0x504
    int categoryIndex;                     // 0x840
    int selectedListIndex;                 // 0x844
    int unk_848;                           // 0x848
    int unk_84C;                           // 0x84c
    int unk_850;                           // 0x850
    int unk_854;                           // 0x854
    int unk_858;                           // 0x858
    void* productLists;                    // 0x85c
    void* uiGroup;                         // 0x860
    void* shopLayout;                      // 0x864
    int uiGroupEntryId;                    // 0x868
    void* scriptObject;                    // 0x86c
    int scriptEntryId;                     // 0x870
    void* productsCategory;                // 0x874
    void* charaModelOperation;             // 0x878
    void* caddieModelOperation;            // 0x87c
    void* primaryItemModelOperation;       // 0x880
    void* secondaryItemModelOperation;     // 0x884
    void* charaProductBuilder;             // 0x888
    void* caddieProductBuilder;            // 0x88c
    void* itemProductBuilder;              // 0x890
    void* playerStatus;                    // 0x894
    void* itemCart;                        // 0x898
    void* textTable;                       // 0x89c
    void* productMessage;                  // 0x8a0
    void* subMenuMessage;                  // 0x8a4
    void* clubSubMenu;                     // 0x8a8
    unsigned char clubSubMenuOpen;         // 0x8ac
    char pad_8AD[0x3];                     // 0x8ad
    void* clubDialog;                      // 0x8b0
    unsigned char clubDialogOpen;          // 0x8b4
    char pad_8B5[0x3];                     // 0x8b5
    void* purchaseDialog;                  // 0x8b8
    unsigned char purchaseDialogOpen;      // 0x8bc
    char pad_8BD[0x3];                     // 0x8bd
    void* itemInfo;                        // 0x8c0
    unsigned char itemInfoOpen;            // 0x8c4
    unsigned char noCaddieSelected;        // 0x8c5
    char pad_8C6[0x2];                     // 0x8c6
    void* saveMenu;                        // 0x8c8
    unsigned char saveMenuActive;          // 0x8cc
    unsigned char characterEntryMode;      // 0x8cd
    unsigned char entryInitialized;        // 0x8ce
    char pad_8CF[0x1];                     // 0x8cf
    void* clubLvUpDialog;                  // 0x8d0
    unsigned char clubLvUpDialogOpen;      // 0x8d4
    char pad_8D5[0x3];                     // 0x8d5
    void* packedFiles;                     // 0x8d8
    void* packTextureResource;             // 0x8dc

    int IsLayoutLoaded();

    void RefreshAllPanelsAndStartFade();
    int PollLoadComplete();
    int Update(float dt);
    int TryOpenClubDialog();
    int ProcessActivatedElement();
    int HandleCategoryNavigation();
    void Draw();
    void* Destroy(int deleteStorage);
    void* Init(void* playerSaveRecord, int characterEntryMode);

    int HasUnownedSelectedParts();

    int HandleProductListNavigation();
    int UpdateCategoryHighlights();
    int HandleCategorySelection();
    int RefreshProductLists();
    int TryOpenCaddiePurchaseDialog();
    int OpenSelectedProductActions();
    int OpenItemPurchaseOrUpgrade();
    int ProcessProductSelection();
    int HandleProductActivation();

    int ApplyNonOutfitPurchase(unsigned int typeId);
    int ProcessClubSubMenu();
    int PurchaseCharacterOrOutfitItem(unsigned int typeId);
    int ProcessClubDialogResult();
    int OpenOutfitPurchaseDialog();
    int ProcessOutfitPurchaseResult();

    void UpdateLayoutsAndPollTexture();
    int SetControlsEnabled(void* enabled);
    static int CopyRecordIntoSnapshot(void* shop, const void* savedRecord);
    static int CopySnapshotToRecord(const void* shop, void* savedRecord);
    int InitializeProductSelection();
    int GetSelectedProductInfo(void* productInfo);

    int UpdateUnownedSelectedOutfitCategoryMarkers();
    int CollectUnownedSelectedOutfitItems(VectorU32* out);

    int UpdateMoneyDisplay();
    int RefreshProductDetails();
    int SelectItemProduct();
    int ApplySelectedSet();
    int SelectCaddieProduct();
    int SelectCharaProduct();
    int PurchaseSelectedProduct();

    int RestoreSavedOutfit();
    int UpdateControlScriptSelection();
    int ActivateModelCameraPreset();
    int IsItemSelected(unsigned int typeId);
    int IsItemOwned(unsigned int typeId);
    int SumUnownedItemValues(EntryList* list);
    int RefreshSelectedProductList();
    int GetSelectedProductRecord();
    int RefreshSelectedProductValues();
    int HandleFixedCameraPresetInput();
    int UpdateSelectedPurchaseAmount();
    int HandleProfileSaveInput();

    int OpenSelectedItemInfo();
};

#define CPANGYA2SHOP_OFF(f, o) typedef char CPangya2Shop_off_##f[((int)&((CPangya2Shop*)0)->f == (o)) ? 1 : -1]
CPANGYA2SHOP_OFF(layout, 0x4);
CPANGYA2SHOP_OFF(saveRecordSnapshotStart, 0x18);
CPANGYA2SHOP_OFF(updateFrameCount, 0x38);
CPANGYA2SHOP_OFF(pangBalance, 0x40);
CPANGYA2SHOP_OFF(currentCharaIndex, 0x44);
CPANGYA2SHOP_OFF(clubSetIndex, 0x45);
CPANGYA2SHOP_OFF(ballIndex, 0x46);
CPANGYA2SHOP_OFF(currentCaddieIndex, 0x47);
CPANGYA2SHOP_OFF(clubSet12OwnershipFlags, 0x4dc);
CPANGYA2SHOP_OFF(clubSet12Reserved, 0x4dd);
CPANGYA2SHOP_OFF(clubSet13OwnershipFlags, 0x4de);
CPANGYA2SHOP_OFF(clubSet13Reserved, 0x4df);
CPANGYA2SHOP_OFF(clubSet14OwnershipFlags, 0x4e0);
CPANGYA2SHOP_OFF(clubSet14Reserved, 0x4e1);
CPANGYA2SHOP_OFF(practiceFlags, 0x500);
CPANGYA2SHOP_OFF(categoryIndex, 0x840);
CPANGYA2SHOP_OFF(selectedListIndex, 0x844);
CPANGYA2SHOP_OFF(unk_848, 0x848);
CPANGYA2SHOP_OFF(unk_84C, 0x84c);
CPANGYA2SHOP_OFF(unk_850, 0x850);
CPANGYA2SHOP_OFF(unk_854, 0x854);
CPANGYA2SHOP_OFF(unk_858, 0x858);
CPANGYA2SHOP_OFF(productLists, 0x85c);
CPANGYA2SHOP_OFF(uiGroup, 0x860);
CPANGYA2SHOP_OFF(shopLayout, 0x864);
CPANGYA2SHOP_OFF(uiGroupEntryId, 0x868);
CPANGYA2SHOP_OFF(scriptObject, 0x86c);
CPANGYA2SHOP_OFF(scriptEntryId, 0x870);
CPANGYA2SHOP_OFF(productsCategory, 0x874);
CPANGYA2SHOP_OFF(charaModelOperation, 0x878);
CPANGYA2SHOP_OFF(caddieModelOperation, 0x87c);
CPANGYA2SHOP_OFF(primaryItemModelOperation, 0x880);
CPANGYA2SHOP_OFF(secondaryItemModelOperation, 0x884);
CPANGYA2SHOP_OFF(charaProductBuilder, 0x888);
CPANGYA2SHOP_OFF(caddieProductBuilder, 0x88c);
CPANGYA2SHOP_OFF(itemProductBuilder, 0x890);
CPANGYA2SHOP_OFF(playerStatus, 0x894);
CPANGYA2SHOP_OFF(itemCart, 0x898);
CPANGYA2SHOP_OFF(textTable, 0x89c);
CPANGYA2SHOP_OFF(productMessage, 0x8a0);
CPANGYA2SHOP_OFF(subMenuMessage, 0x8a4);
CPANGYA2SHOP_OFF(clubSubMenu, 0x8a8);
CPANGYA2SHOP_OFF(clubSubMenuOpen, 0x8ac);
CPANGYA2SHOP_OFF(clubDialog, 0x8b0);
CPANGYA2SHOP_OFF(clubDialogOpen, 0x8b4);
CPANGYA2SHOP_OFF(purchaseDialog, 0x8b8);
CPANGYA2SHOP_OFF(purchaseDialogOpen, 0x8bc);
CPANGYA2SHOP_OFF(itemInfo, 0x8c0);
CPANGYA2SHOP_OFF(itemInfoOpen, 0x8c4);
CPANGYA2SHOP_OFF(noCaddieSelected, 0x8c5);
CPANGYA2SHOP_OFF(saveMenu, 0x8c8);
CPANGYA2SHOP_OFF(saveMenuActive, 0x8cc);
CPANGYA2SHOP_OFF(characterEntryMode, 0x8cd);
CPANGYA2SHOP_OFF(entryInitialized, 0x8ce);
CPANGYA2SHOP_OFF(clubLvUpDialog, 0x8d0);
CPANGYA2SHOP_OFF(clubLvUpDialogOpen, 0x8d4);
CPANGYA2SHOP_OFF(packedFiles, 0x8d8);
CPANGYA2SHOP_OFF(packTextureResource, 0x8dc);
CPANGYA2SHOP_OFF(caddieItemIndex, 0x48);
#undef CPANGYA2SHOP_OFF

#endif // CLASSES_CPANGYA2SHOP_H

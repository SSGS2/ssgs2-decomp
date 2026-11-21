// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CPANGYA2SHOP_H
#define CLASSES_CPANGYA2SHOP_H

#include "game/types.h"

class CPangya2Shop {
public:
    void* vtbl;                         // 0x0: original vtable pointer
    void* layout;                        // 0x4: 2 uses in CItemInfo_IsLoaded, CPangya2Shop_IsLayoutLoaded
    char pad_008[0x10];                 // 0x8
    void* saveRecordSnapshotStart;                       // 0x18: 2 uses in CPangya2Shop_Ctor, CItemInfo_IsLoaded
    char pad_01C[0x1c];                 // 0x1c
    int updateFrameCount;                         // 0x38: 2 uses in CPangya2Shop_Update
    char pad_03C[0x4];                  // 0x3c
    void* pangBalance;                       // 0x40: 2 uses in CPangya2Shop_RefreshAllPanelsAndStartFade, CPangya2Shop_Update
    signed char currentCharaIndex;                 // 0x44: 2 uses in CPangya2Shop_RefreshAllPanelsAndStartFade
    signed char clubSetIndex;                 // 0x45: 1 use in CPangya2Shop_RefreshAllPanelsAndStartFade
    signed char ballIndex;                 // 0x46: 1 use in CPangya2Shop_RefreshAllPanelsAndStartFade
    signed char currentCaddieIndex;                 // 0x47: 1 use in CPangya2Shop_RefreshAllPanelsAndStartFade
    signed char caddieItemIndex; // 0x48: CPangya2Shop_RefreshProductLists
    char pad_049[0x493]; // 0x49
    unsigned char clubSet12OwnershipFlags;              // 0x4dc: 1 use in CPangya2Shop_Ctor
    unsigned char clubSet12Reserved;              // 0x4dd: 1 use in CPangya2Shop_Ctor
    unsigned char clubSet13OwnershipFlags;              // 0x4de: 1 use in CPangya2Shop_Ctor
    unsigned char clubSet13Reserved;              // 0x4df: 1 use in CPangya2Shop_Ctor
    unsigned char clubSet14OwnershipFlags;              // 0x4e0: 1 use in CPangya2Shop_Ctor
    unsigned char clubSet14Reserved;              // 0x4e1: 1 use in CPangya2Shop_Ctor
    char pad_4E2[0x1e];                 // 0x4e2
    unsigned int practiceFlags;               // 0x500: 1 use in CPangya2Shop_Ctor
    char pad_504[0x33c];                // 0x504
    int categoryIndex;                        // 0x840: 23 uses in CPangya2Shop_Draw, CPangya2Shop_Update, CPangya2Shop_RefreshAllPanelsAndStartFade...
    int selectedListIndex;                        // 0x844: 25 uses in CPangya2Shop_Draw, CPangya2Shop_RefreshAllPanelsAndStartFade, CPangya2Shop_Update...
    int unk_848;                        // 0x848: 1 use in CPangya2Shop_Ctor
    int unk_84C;                        // 0x84c: 1 use in CPangya2Shop_Ctor
    int unk_850;                        // 0x850: 1 use in CPangya2Shop_Ctor
    int unk_854;                        // 0x854: 1 use in CPangya2Shop_Ctor
    int unk_858;                        // 0x858: 1 use in CPangya2Shop_Ctor
    void* productLists;                      // 0x85c: 17 uses in CPangya2Shop_RefreshAllPanelsAndStartFade, CPangya2Shop_dtor, CPangya2Shop_Ctor...
    void* uiGroup;                      // 0x860: 9 uses in CPangya2Shop_RefreshAllPanelsAndStartFade, CPangya2Shop_Ctor, CPangya2Shop_Draw...
    void* shopLayout;                      // 0x864: 12 uses in CPangya2Shop_RefreshAllPanelsAndStartFade, CPangya2Shop_Ctor, CPangya2Shop_dtor...
    int uiGroupEntryId;                        // 0x868: 5 uses in CPangya2Shop_dtor, CPangya2Shop_Ctor
    void* scriptObject;                      // 0x86c: 6 uses in CPangya2Shop_Ctor, CPangya2Shop_RefreshAllPanelsAndStartFade, CPangya2Shop_Draw
    int scriptEntryId;                        // 0x870: 5 uses in CPangya2Shop_dtor, CPangya2Shop_Ctor
    void* productsCategory;                      // 0x874: 8 uses in CPangya2Shop_dtor, CPangya2Shop_Ctor, CPangya2Shop_UpdateLayoutsAndPollTexture...
    void* charaModelOperation;                      // 0x878: 15 uses in CPangya2Shop_Draw, CPangya2Shop_RefreshAllPanelsAndStartFade, CPangya2Shop_Update...
    void* caddieModelOperation;                      // 0x87c: 16 uses in CPangya2Shop_RefreshAllPanelsAndStartFade, CPangya2Shop_Draw, CPangya2Shop_Update...
    void* primaryItemModelOperation;                      // 0x880: 15 uses in CPangya2Shop_Draw, CPangya2Shop_RefreshAllPanelsAndStartFade, CPangya2Shop_Update...
    void* secondaryItemModelOperation;                      // 0x884: 15 uses in CPangya2Shop_Draw, CPangya2Shop_RefreshAllPanelsAndStartFade, CPangya2Shop_Update...
    void* charaProductBuilder;                      // 0x888: 11 uses in CPangya2Shop_Ctor, CPangya2Shop_dtor, CPangya2Shop_Update...
    void* caddieProductBuilder;                      // 0x88c: 8 uses in CPangya2Shop_Ctor, CPangya2Shop_dtor, CPangya2Shop_Update...
    void* itemProductBuilder;                      // 0x890: 11 uses in CPangya2Shop_Ctor, CPangya2Shop_dtor, CPangya2Shop_Update...
    void* playerStatus;                      // 0x894: 12 uses in CPangya2Shop_RefreshAllPanelsAndStartFade, CPangya2Shop_Ctor, CPangya2Shop_dtor...
    void* itemCart;                      // 0x898: 14 uses in CPangya2Shop_Ctor, CPangya2Shop_RefreshAllPanelsAndStartFade, CPangya2Shop_dtor...
    void* textTable;                      // 0x89c: 13 uses in CPangya2Shop_RefreshAllPanelsAndStartFade, CPangya2Shop_dtor, CPangya2Shop_Ctor...
    void* productMessage;                      // 0x8a0: 11 uses in CPangya2Shop_RefreshAllPanelsAndStartFade, CPangya2Shop_Update, CPangya2Shop_dtor...
    void* subMenuMessage;                      // 0x8a4: 7 uses in CPangya2Shop_dtor, CPangya2Shop_Ctor, CPangya2Shop_RefreshAllPanelsAndStartFade...
    void* clubSubMenu;                      // 0x8a8: 7 uses in CPangya2Shop_dtor, CPangya2Shop_Ctor, CPangya2Shop_UpdateLayoutsAndPollTexture...
    unsigned char clubSubMenuOpen;              // 0x8ac: 3 uses in CPangya2Shop_Ctor, CPangya2Shop_Update, CPangya2Shop_Draw
    char pad_8AD[0x3];                  // 0x8ad
    void* clubDialog;                      // 0x8b0: 7 uses in CPangya2Shop_dtor, CPangya2Shop_Ctor, CPangya2Shop_UpdateLayoutsAndPollTexture...
    unsigned char clubDialogOpen;              // 0x8b4: 3 uses in CPangya2Shop_Ctor, CPangya2Shop_Update, CPangya2Shop_Draw
    char pad_8B5[0x3];                  // 0x8b5
    void* purchaseDialog;                      // 0x8b8: 7 uses in CPangya2Shop_dtor, CPangya2Shop_Ctor, CPangya2Shop_UpdateLayoutsAndPollTexture...
    unsigned char purchaseDialogOpen;              // 0x8bc: 3 uses in CPangya2Shop_Ctor, CPangya2Shop_Update, CPangya2Shop_Draw
    char pad_8BD[0x3];                  // 0x8bd
    void* itemInfo;                      // 0x8c0: 9 uses in CPangya2Shop_dtor, CPangya2Shop_Update, CPangya2Shop_Ctor...
    unsigned char itemInfoOpen;              // 0x8c4: 4 uses in CPangya2Shop_Update, CPangya2Shop_Ctor, CPangya2Shop_Draw
    unsigned char noCaddieSelected;              // 0x8c5: 7 uses in CPangya2Shop_Draw, CPangya2Shop_Ctor, CPangya2Shop_RefreshAllPanelsAndStartFade...
    char pad_8C6[0x2];                  // 0x8c6
    void* saveMenu;                      // 0x8c8: 8 uses in CPangya2Shop_Update, CPangya2Shop_dtor, CPangya2Shop_Draw...
    unsigned char saveMenuActive;              // 0x8cc: 3 uses in CPangya2Shop_Update, CPangya2Shop_Ctor
    unsigned char characterEntryMode;              // 0x8cd: 3 uses in CPangya2Shop_Ctor, CPangya2Shop_RefreshAllPanelsAndStartFade, CPangya2Shop_Update
    unsigned char entryInitialized;              // 0x8ce: 3 uses in CPangya2Shop_RefreshAllPanelsAndStartFade, CPangya2Shop_Ctor
    char pad_8CF[0x1];                  // 0x8cf
    void* clubLvUpDialog;                      // 0x8d0: 10 uses in CPangya2Shop_Ctor, CPangya2Shop_dtor, CPangya2Shop_Update...
    unsigned char clubLvUpDialogOpen;              // 0x8d4: 4 uses in CPangya2Shop_Update, CPangya2Shop_Ctor, CPangya2Shop_Draw
    char pad_8D5[0x3];                  // 0x8d5
    void* packedFiles;                      // 0x8d8: 11 uses in CPangya2Shop_PollLoadComplete, CPangya2Shop_dtor, CPangya2Shop_Ctor
    void* packTextureResource;                      // 0x8dc: 9 uses in CPangya2Shop_Ctor, CPangya2Shop_dtor, CPangya2Shop_RefreshAllPanelsAndStartFade...

    // offsets whose address is passed on (sub-objects or arrays, not placed): 0x18, 0x44, 0x47, 0x48, 0x2e4, 0x868, 0x870
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
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

#endif  // CLASSES_CPANGYA2SHOP_H

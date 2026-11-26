#ifndef CLASSES_CPANGYA2TOPMENU_H
#define CLASSES_CPANGYA2TOPMENU_H

#include "game/types.h"

class CPangya2TopMenu {
public:
    void* vtbl;                       // 0x0: original vtable pointer
    char pad_004[0x14];               // 0x4
    void* secondaryVtable;            // 0x18
    void* layout;                     // 0x1c
    void* bpObject;                   // 0x20
    int result;                       // 0x24
    unsigned char resultReady;        // 0x28
    unsigned char exitRequested;      // 0x29
    char pad_02A[0x2];                // 0x2A
    int menuItemCount;                // 0x2C: menu-element loop bound; embedded CTopMenuBase::menuItemCount
    unsigned char scriptRunning;      // 0x30
    char pad_031[0x3];                // 0x31
    void* message;                    // 0x34
    void* optionDialog;               // 0x38
    unsigned char optionDialogActive; // 0x3c
    char pad_03D[0x13];               // 0x3d
    union {
        struct {
            void* firstMenuLayout;
            char pad_054[8];
        };
        void* menuLayouts[3]; // 0x50: CPangya2TopMenu_Ctor creates three Layout objects.
    };
    void* menuObjects[3]; // 0x5c: CPangya2TopMenu_Ctor stores IdMap_Lookup results.
    union {
        struct {
            int firstMenuEntryId;
            char pad_06C[8];
        };
        int menuEntryIds[3]; // 0x68: CPangya2TopMenu_Ctor creates entries; dtor deletes IDs.
    };
    unsigned char subMenuActive; // 0x74
    char pad_075[0x3];           // 0x75
    void* subMenu;               // 0x78
    int subMenuMode;             // 0x7c
    void* menuTexture;           // 0x80

    CPangya2TopMenu* Init(); // builds the bases, layouts and texture; returns this
    void Draw();
    void StartMenu();
    int PollLoadComplete();
    static void UpdateLayouts(void* p);
    int HandleSelection();
    int ActivateMenu();
    static int HandleSelectionThunk24(void* p);
    static void* DtorThunk24(void* p, int x);

    int TryGetResult(int* out);
    int RequestSubMenu(int subMenuMode);
    int UpdateMenuState(float dt);

    ~CPangya2TopMenu();
};

#define CPANGYA2TOPMENU_OFF(f, o) typedef char CPangya2TopMenu_off_##f[((int)&((CPangya2TopMenu*)0)->f == (o)) ? 1 : -1]
CPANGYA2TOPMENU_OFF(secondaryVtable, 0x18);
CPANGYA2TOPMENU_OFF(layout, 0x1c);
CPANGYA2TOPMENU_OFF(bpObject, 0x20);
CPANGYA2TOPMENU_OFF(result, 0x24);
CPANGYA2TOPMENU_OFF(resultReady, 0x28);
CPANGYA2TOPMENU_OFF(exitRequested, 0x29);
CPANGYA2TOPMENU_OFF(scriptRunning, 0x30);
CPANGYA2TOPMENU_OFF(message, 0x34);
CPANGYA2TOPMENU_OFF(optionDialog, 0x38);
CPANGYA2TOPMENU_OFF(optionDialogActive, 0x3c);
CPANGYA2TOPMENU_OFF(firstMenuLayout, 0x50);
CPANGYA2TOPMENU_OFF(firstMenuEntryId, 0x68);
CPANGYA2TOPMENU_OFF(subMenuActive, 0x74);
CPANGYA2TOPMENU_OFF(subMenu, 0x78);
CPANGYA2TOPMENU_OFF(subMenuMode, 0x7c);
CPANGYA2TOPMENU_OFF(menuTexture, 0x80);
CPANGYA2TOPMENU_OFF(menuItemCount, 0x2C);
CPANGYA2TOPMENU_OFF(menuLayouts, 0x50);
CPANGYA2TOPMENU_OFF(menuObjects, 0x5c);
CPANGYA2TOPMENU_OFF(menuEntryIds, 0x68);
#undef CPANGYA2TOPMENU_OFF

#endif // CLASSES_CPANGYA2TOPMENU_H

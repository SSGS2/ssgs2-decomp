#ifndef CLASSES_CTUTORIALILLUST2D_H
#define CLASSES_CTUTORIALILLUST2D_H

#include "game/types.h"

class CTutorialIllust2d {
public:
    void* vtbl;                       // 0x0: original vtable pointer
    char pad_004[0x58];               // 0x4
    int illustrationIndex;            // 0x5c
    int alternateIllustrationEnabled; // 0x60
    int visible;                      // 0x64
    int initialized;                  // 0x68
    int layoutDirty;                  // 0x6c
    void* primaryMessage;             // 0x70
    void* secondaryMessage;           // 0x74
    void* interfaceVtable;            // 0x78

    int Initialize();
    int SelectIllustrationPage(int state, void* arg);
    int UpdateLayoutAndCheckStages();
    int UpdatePageMessages();
    int Show();
    int HideAndResetPage();
    int vfunc_6();
    int vfunc_9();
    int vfunc_10();
    int RefreshPageElements();
    static int LoadLayout(void* a, int b);
    int Draw();

    ~CTutorialIllust2d();
    CTutorialIllust2d();
};

#define CTUTORIALILLUST2D_OFF(f, o) \
    typedef char CTutorialIllust2d_off_##f[((int)&((CTutorialIllust2d*)0)->f == (o)) ? 1 : -1]
CTUTORIALILLUST2D_OFF(illustrationIndex, 0x5c);
CTUTORIALILLUST2D_OFF(alternateIllustrationEnabled, 0x60);
CTUTORIALILLUST2D_OFF(visible, 0x64);
CTUTORIALILLUST2D_OFF(initialized, 0x68);
CTUTORIALILLUST2D_OFF(layoutDirty, 0x6c);
CTUTORIALILLUST2D_OFF(primaryMessage, 0x70);
CTUTORIALILLUST2D_OFF(secondaryMessage, 0x74);
CTUTORIALILLUST2D_OFF(interfaceVtable, 0x78);
#undef CTUTORIALILLUST2D_OFF

#endif // CLASSES_CTUTORIALILLUST2D_H

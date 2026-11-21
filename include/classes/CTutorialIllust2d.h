// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CTUTORIALILLUST2D_H
#define CLASSES_CTUTORIALILLUST2D_H

#include "game/types.h"

class CTutorialIllust2d {
public:
    void* vtbl;                         // 0x0: original vtable pointer
    char pad_004[0x58];                 // 0x4
    int illustrationIndex;                         // 0x5c: 8 uses in CTutorialIllust2d_SelectIllustrationPage, CTutorialIllust2d_RefreshPageElements, CTutorialIllust2d_UpdatePageMessages...
    int alternateIllustrationEnabled;                         // 0x60: 6 uses in CTutorialIllust2d_RefreshPageElements, CTutorialIllust2d_SelectIllustrationPage, CTutorialIllust2d_Ctor
    int visible;                         // 0x64: 5 uses in CTutorialIllust2d_UpdatePageMessages, CTutorialIllust2d_Show, CTutorialIllust2d_HideAndResetPage...
    int initialized;                         // 0x68: 3 uses in CTutorialIllust2d_Initialize, CTutorialIllust2d_RefreshPageElements, CTutorialIllust2d_Ctor
    int layoutDirty;                         // 0x6c: 7 uses in CTutorialIllust2d_RefreshPageElements, CTutorialIllust2d_Initialize, CTutorialIllust2d_SelectIllustrationPage...
    void* primaryMessage;                       // 0x70: 19 uses in CTutorialIllust2d_UpdatePageMessages, CTutorialIllust2d_dtor, CTutorialIllust2d_Initialize...
    void* secondaryMessage;                       // 0x74: 17 uses in CTutorialIllust2d_UpdatePageMessages, CTutorialIllust2d_dtor, CTutorialIllust2d_Initialize...
    void* interfaceVtable;                       // 0x78: 2 uses in CTutorialIllust2d_dtor, CTutorialIllust2d_Ctor
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define CTUTORIALILLUST2D_OFF(f, o) typedef char CTutorialIllust2d_off_##f[((int)&((CTutorialIllust2d*)0)->f == (o)) ? 1 : -1]
CTUTORIALILLUST2D_OFF(illustrationIndex, 0x5c);
CTUTORIALILLUST2D_OFF(alternateIllustrationEnabled, 0x60);
CTUTORIALILLUST2D_OFF(visible, 0x64);
CTUTORIALILLUST2D_OFF(initialized, 0x68);
CTUTORIALILLUST2D_OFF(layoutDirty, 0x6c);
CTUTORIALILLUST2D_OFF(primaryMessage, 0x70);
CTUTORIALILLUST2D_OFF(secondaryMessage, 0x74);
CTUTORIALILLUST2D_OFF(interfaceVtable, 0x78);
#undef CTUTORIALILLUST2D_OFF

#endif  // CLASSES_CTUTORIALILLUST2D_H

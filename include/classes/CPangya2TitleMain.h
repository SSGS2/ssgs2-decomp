// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CPANGYA2TITLEMAIN_H
#define CLASSES_CPANGYA2TITLEMAIN_H

#include "game/types.h"

class CPangya2TitleMain {
public:
    void* vtbl;                         // 0x0: original vtable pointer
    char pad_004[0x14];                 // 0x4
    void* interfaceVtable;                       // 0x18: 2 uses in CPangya2TitleMain_ctor, CPangya2TitleMain_dtor
    void* activeMenu;                       // 0x1c: 49 uses in CPangya2TitleMain_OnProcess, CPangya2TitleMain_HandleMsg, CPangya2TitleMain_OnDestroy...
    char pad_020[0x4];                  // 0x20
    int menuState;                         // 0x24: 18 uses in CPangya2TitleMain_OnProcess, CPangya2TitleMain_HandleMsg, CPangya2TitleMain_OnInit
    char pad_028[0x4];                  // 0x28
    int topMenuResult;                         // 0x2c: 3 uses in CPangya2TitleMain_OnProcess, CPangya2TitleMain_ctor
    void* textureResource;                       // 0x30: 9 uses in CPangya2TitleMain_OnDestroy, CPangya2TitleMain_OnLoad, CPangya2TitleMain_dtor...

    // offsets whose address is passed on (sub-objects or arrays, not placed): 0x2c
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define CPANGYA2TITLEMAIN_OFF(f, o) typedef char CPangya2TitleMain_off_##f[((int)&((CPangya2TitleMain*)0)->f == (o)) ? 1 : -1]
CPANGYA2TITLEMAIN_OFF(interfaceVtable, 0x18);
CPANGYA2TITLEMAIN_OFF(activeMenu, 0x1c);
CPANGYA2TITLEMAIN_OFF(menuState, 0x24);
CPANGYA2TITLEMAIN_OFF(topMenuResult, 0x2c);
CPANGYA2TITLEMAIN_OFF(textureResource, 0x30);
#undef CPANGYA2TITLEMAIN_OFF

#endif  // CLASSES_CPANGYA2TITLEMAIN_H

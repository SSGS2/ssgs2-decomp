// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CBALLWIND2D_H
#define CLASSES_CBALLWIND2D_H

#include "game/types.h"

class CBallWind2d {
public:
    void* vtbl;                         // 0x0: original vtable pointer
    char pad_004[0x58];                 // 0x4
    int selectedElementIndex;                         // 0x5c: 2 uses in CBallWind2d_Ctor, CBallWind2d_vfunc_5
    int selectionActive;                         // 0x60: 1 use in CBallWind2d_Ctor
    int selectionState;                         // 0x64: 1 use in CBallWind2d_Ctor
    int ballControlsVisible;                         // 0x68: 1 use in CBallWind2d_Ctor
    int element0Visible;                         // 0x6c: 1 use in CBallWind2d_Ctor
    int elements26And27Visible;                         // 0x70: 1 use in CBallWind2d_Ctor
    int element22Visible;                         // 0x74: 1 use in CBallWind2d_Ctor
    int element25Visible;                         // 0x78: 1 use in CBallWind2d_Ctor
    int elements25And26Bright;                         // 0x7c: 1 use in CBallWind2d_Ctor
    int element23Visible;                         // 0x80: 1 use in CBallWind2d_Ctor
    int element24Visible;                         // 0x84: 1 use in CBallWind2d_Ctor
    int element28Visible;                         // 0x88: 1 use in CBallWind2d_Ctor
    int element29Visible;                         // 0x8c: 1 use in CBallWind2d_Ctor
    int element3DrawnSeparately;                         // 0x90: 1 use in CBallWind2d_Ctor
    char pad_094[0x4];                  // 0x94
    float unk_98;                       // 0x98: 1 use in CBallWind2d_Ctor
    int unk_9C;                         // 0x9c: 1 use in CBallWind2d_Ctor
    int initialized;                         // 0xa0: 2 uses in CBallWind2d_Ctor, CBallWind2d_InitLayout
    void* interfaceVtable;                       // 0xa4: 1 use in CBallWind2d_Ctor
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define CBALLWIND2D_OFF(f, o) typedef char CBallWind2d_off_##f[((int)&((CBallWind2d*)0)->f == (o)) ? 1 : -1]
CBALLWIND2D_OFF(selectedElementIndex, 0x5c);
CBALLWIND2D_OFF(selectionActive, 0x60);
CBALLWIND2D_OFF(selectionState, 0x64);
CBALLWIND2D_OFF(ballControlsVisible, 0x68);
CBALLWIND2D_OFF(element0Visible, 0x6c);
CBALLWIND2D_OFF(elements26And27Visible, 0x70);
CBALLWIND2D_OFF(element22Visible, 0x74);
CBALLWIND2D_OFF(element25Visible, 0x78);
CBALLWIND2D_OFF(elements25And26Bright, 0x7c);
CBALLWIND2D_OFF(element23Visible, 0x80);
CBALLWIND2D_OFF(element24Visible, 0x84);
CBALLWIND2D_OFF(element28Visible, 0x88);
CBALLWIND2D_OFF(element29Visible, 0x8c);
CBALLWIND2D_OFF(element3DrawnSeparately, 0x90);
CBALLWIND2D_OFF(unk_98, 0x98);
CBALLWIND2D_OFF(unk_9C, 0x9c);
CBALLWIND2D_OFF(initialized, 0xa0);
CBALLWIND2D_OFF(interfaceVtable, 0xa4);
#undef CBALLWIND2D_OFF

#endif  // CLASSES_CBALLWIND2D_H

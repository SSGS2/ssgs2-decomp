// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CSTROKEPOINT2D_H
#define CLASSES_CSTROKEPOINT2D_H

#include "game/types.h"

class CStrokePoint2d {
public:
    void* vtbl;                         // 0x0: original vtable pointer
    char pad_004[0x78];                 // 0x4
    int useAlternateHelpText;                         // 0x7c: 3 uses in CStrokePoint2d_Ctor, CStrokePoint2d_SetPointerSelectionDisabled, CStrokePoint2d_UpdateHoveredHelpText
    char pad_080[0x30];                 // 0x80
    int selectionEnabled;              // 0xb0: 2 uses in CStrokePoint2d_Ctor, CStrokePoint2d_UpdateHoveredHelpText
    float opacity;                       // 0xb4: 4 uses in CStrokePoint2d_Draw, CStrokePoint2d_Ctor
    char pad_0B8[0x8];                  // 0xb8
    int selectionChanged;                         // 0xc0: 5 uses in CStrokePoint2d_Ctor, CStrokePoint2d_InitLayout, CStrokePoint2d_SetPointPosition...
    char pad_0C4[0x14];                 // 0xc4
    void* message;                     // 0xd8: 15 uses in CStrokePoint2d_dtor, CStrokePoint2d_UpdateHoveredHelpText, CStrokePoint2d_InitLayout...
    int selectedIndex;                         // 0xdc: 5 uses in CStrokePoint2d_UpdateHoveredHelpText, CStrokePoint2d_Ctor
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define CSTROKEPOINT2D_OFF(f, o) typedef char CStrokePoint2d_off_##f[((int)&((CStrokePoint2d*)0)->f == (o)) ? 1 : -1]
CSTROKEPOINT2D_OFF(useAlternateHelpText, 0x7c);
CSTROKEPOINT2D_OFF(selectionEnabled, 0xb0);
CSTROKEPOINT2D_OFF(opacity, 0xb4);
CSTROKEPOINT2D_OFF(selectionChanged, 0xc0);
CSTROKEPOINT2D_OFF(message, 0xd8);
CSTROKEPOINT2D_OFF(selectedIndex, 0xdc);
#undef CSTROKEPOINT2D_OFF

#endif  // CLASSES_CSTROKEPOINT2D_H

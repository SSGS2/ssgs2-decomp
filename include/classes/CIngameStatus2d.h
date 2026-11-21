// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CINGAMESTATUS2D_H
#define CLASSES_CINGAMESTATUS2D_H

#include "game/types.h"

class CIngameStatus2d {
public:
    void* vtbl;                         // 0x0: original vtable pointer
    char pad_004[0x60];                 // 0x4
    float gaugeFill;                       // 0x64: 1 use in CIngameStatus2d_UpdateIconLabelAndGauge
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define CINGAMESTATUS2D_OFF(f, o) typedef char CIngameStatus2d_off_##f[((int)&((CIngameStatus2d*)0)->f == (o)) ? 1 : -1]
CINGAMESTATUS2D_OFF(gaugeFill, 0x64);
#undef CINGAMESTATUS2D_OFF

#endif  // CLASSES_CINGAMESTATUS2D_H

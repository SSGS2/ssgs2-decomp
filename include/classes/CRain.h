// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CRAIN_H
#define CLASSES_CRAIN_H

#include "game/types.h"

class CRain {
public:
    void* vtbl;                         // 0x0: original vtable pointer
    char pad_004[0x1c];                 // 0x4
    void* vertexBuffer;                       // 0x20: 1 use in CRain_DrawLineSegments
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define CRAIN_OFF(f, o) typedef char CRain_off_##f[((int)&((CRain*)0)->f == (o)) ? 1 : -1]
CRAIN_OFF(vertexBuffer, 0x20);
#undef CRAIN_OFF

#endif  // CLASSES_CRAIN_H

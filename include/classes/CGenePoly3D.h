// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CGENEPOLY3D_H
#define CLASSES_CGENEPOLY3D_H

#include "game/types.h"

class CGenePoly3D {
public:
    void* vtbl;                         // 0x0: original vtable pointer
    char pad_004[0x4];                  // 0x4
    void* vertices;                        // 0x8: 1 use in CGenePoly3D_DrawStoredVertices
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define CGENEPOLY3D_OFF(f, o) typedef char CGenePoly3D_off_##f[((int)&((CGenePoly3D*)0)->f == (o)) ? 1 : -1]
CGENEPOLY3D_OFF(vertices, 0x8);
#undef CGENEPOLY3D_OFF

#endif  // CLASSES_CGENEPOLY3D_H

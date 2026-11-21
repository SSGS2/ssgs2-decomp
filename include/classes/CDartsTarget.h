// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CDARTSTARGET_H
#define CLASSES_CDARTSTARGET_H

#include "game/types.h"

class CDartsTarget {
public:
    void* vtbl;                         // 0x0: original vtable pointer
    void* primaryMeshHandle;                       // 0x4: OnPreLoadInit zeroes; CDartsTarget_dtor releases the pointed-to resource wrapper
    void* vertexStreamInfo;                        // 0x8: 1 use in CDartsTarget_Draw
    void* textureResource;                        // 0xc: 1 use in CDartsTarget_Draw
    int renderSuppressed;                         // 0x10: 1 use in CDartsTarget_Draw
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define CDARTSTARGET_OFF(f, o) typedef char CDartsTarget_off_##f[((int)&((CDartsTarget*)0)->f == (o)) ? 1 : -1]
CDARTSTARGET_OFF(vertexStreamInfo, 0x8);
CDARTSTARGET_OFF(primaryMeshHandle, 0x4);
CDARTSTARGET_OFF(textureResource, 0xc);
CDARTSTARGET_OFF(renderSuppressed, 0x10);
#undef CDARTSTARGET_OFF

#endif  // CLASSES_CDARTSTARGET_H

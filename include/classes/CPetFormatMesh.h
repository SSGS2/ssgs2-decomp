// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CPETFORMATMESH_H
#define CLASSES_CPETFORMATMESH_H

#include "game/types.h"

class CPetFormatMesh {
public:
    void* vtbl;                         // 0x0: original vtable pointer
    int chunkData;                          // 0x4: 1 use in CPetFormatMesh_Ctor
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define CPETFORMATMESH_OFF(f, o) typedef char CPetFormatMesh_off_##f[((int)&((CPetFormatMesh*)0)->f == (o)) ? 1 : -1]
CPETFORMATMESH_OFF(chunkData, 0x4);
#undef CPETFORMATMESH_OFF

#endif  // CLASSES_CPETFORMATMESH_H

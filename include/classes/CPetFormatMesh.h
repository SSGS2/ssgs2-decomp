#ifndef CLASSES_CPETFORMATMESH_H
#define CLASSES_CPETFORMATMESH_H

#include "game/types.h"

class CPetFormatMesh {
public:
    void* vtbl;    // 0x0: original vtable pointer
    int chunkData; // 0x4

    void* GetFirstDataWord();
    void* GetSecondaryData();

    CPetFormatMesh(int value);
    ~CPetFormatMesh();
};

#define CPETFORMATMESH_OFF(f, o) typedef char CPetFormatMesh_off_##f[((int)&((CPetFormatMesh*)0)->f == (o)) ? 1 : -1]
CPETFORMATMESH_OFF(chunkData, 0x4);
#undef CPETFORMATMESH_OFF

#endif // CLASSES_CPETFORMATMESH_H

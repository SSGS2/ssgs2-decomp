// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CPARTICLE_H
#define CLASSES_CPARTICLE_H

#include "game/types.h"

class CParticle {
public:
    void* vtbl;                         // 0x0: original vtable pointer
    char pad_004[0x4];                  // 0x4
    unsigned int placementStateFlags;                 // 0x8: 1 use in CCreature_InitRandomGridCell
    float boundsMinX;                        // 0xc: 3 uses in CCreature_InitRandomGridCell
    float boundsMinY;                       // 0x10: 3 uses in CCreature_InitRandomGridCell
    float boundsMinZ;                       // 0x14: 3 uses in CCreature_InitRandomGridCell
    float boundsMaxX;                       // 0x18: 2 uses in CCreature_InitRandomGridCell
    void* unk_01c;                       // 0x1c: bounds maximum Y in the grid view, owned object in the destructor view
    void* unk_020;                       // 0x20: bounds maximum Z in the grid view, owned object in the destructor view
    char pad_024[0x4];                  // 0x24
    float transform[12]; // 0x28: CCreature_InitRandomGridCell (0x80199AAC), Matrix_CopyAffine; translation at 0x4C
    char pad_058[0xc];                  // 0x58
    float gridPositionX;                       // 0x64: 2 uses in CCreature_InitRandomGridCell
    float gridPositionY;                       // 0x68: 1 use in CCreature_InitRandomGridCell
    float gridPositionZ;                       // 0x6c: 1 use in CCreature_InitRandomGridCell
    char pad_070[0x18];                 // 0x70
    float targetCellSize;                       // 0x88: 3 uses in CCreature_InitRandomGridCell
    int gridCountX;                         // 0x8c: 2 uses in CCreature_InitRandomGridCell
    int gridCountY;                         // 0x90: 2 uses in CCreature_InitRandomGridCell
    int gridCountZ;                         // 0x94: 2 uses in CCreature_InitRandomGridCell
    float cellSizeX;                       // 0x98: 2 uses in CCreature_InitRandomGridCell
    float cellSizeY;                       // 0x9c: 2 uses in CCreature_InitRandomGridCell
    float cellSizeZ;                       // 0xa0: 2 uses in CCreature_InitRandomGridCell
    int cellIndexCount;                         // 0xa4: 2 uses in CCreature_InitRandomGridCell
    void* cellIndices;                       // 0xa8: 2 uses in CCreature_InitRandomGridCell

    // offsets whose address is passed on (sub-objects or arrays, not placed): 0xc
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define CPARTICLE_OFF(f, o) typedef char CParticle_off_##f[((int)&((CParticle*)0)->f == (o)) ? 1 : -1]
CPARTICLE_OFF(placementStateFlags, 0x8);
CPARTICLE_OFF(boundsMinX, 0xc);
CPARTICLE_OFF(boundsMinY, 0x10);
CPARTICLE_OFF(boundsMinZ, 0x14);
CPARTICLE_OFF(boundsMaxX, 0x18);
CPARTICLE_OFF(unk_01c, 0x1c);
CPARTICLE_OFF(unk_020, 0x20);
CPARTICLE_OFF(transform, 0x28);
CPARTICLE_OFF(gridPositionX, 0x64);
CPARTICLE_OFF(gridPositionY, 0x68);
CPARTICLE_OFF(gridPositionZ, 0x6c);
CPARTICLE_OFF(targetCellSize, 0x88);
CPARTICLE_OFF(gridCountX, 0x8c);
CPARTICLE_OFF(gridCountY, 0x90);
CPARTICLE_OFF(gridCountZ, 0x94);
CPARTICLE_OFF(cellSizeX, 0x98);
CPARTICLE_OFF(cellSizeY, 0x9c);
CPARTICLE_OFF(cellSizeZ, 0xa0);
CPARTICLE_OFF(cellIndexCount, 0xa4);
CPARTICLE_OFF(cellIndices, 0xa8);
#undef CPARTICLE_OFF

#endif  // CLASSES_CPARTICLE_H

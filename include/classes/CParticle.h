#ifndef CLASSES_CPARTICLE_H
#define CLASSES_CPARTICLE_H

#include "game/types.h"

class CParticle {
public:
    void* vtbl;                       // 0x0: original vtable pointer
    char pad_004[0x4];                // 0x4
    unsigned int placementStateFlags; // 0x8
    float boundsMinX;                 // 0xc
    float boundsMinY;                 // 0x10
    float boundsMinZ;                 // 0x14
    float boundsMaxX;                 // 0x18
    void* unk_01c;                    // 0x1c: bounds maximum Y in the grid view, owned object in the destructor view
    void* unk_020;                    // 0x20: bounds maximum Z in the grid view, owned object in the destructor view
    char pad_024[0x4];                // 0x24
    float transform[12];  // 0x28: CCreature_InitRandomGridCell (0x80199AAC), Matrix_CopyAffine; translation at 0x4C
    char pad_058[0xc];    // 0x58
    float gridPositionX;  // 0x64
    float gridPositionY;  // 0x68
    float gridPositionZ;  // 0x6c
    char pad_070[0x18];   // 0x70
    float targetCellSize; // 0x88
    int gridCountX;       // 0x8c
    int gridCountY;       // 0x90
    int gridCountZ;       // 0x94
    float cellSizeX;      // 0x98
    float cellSizeY;      // 0x9c
    float cellSizeZ;      // 0xa0
    int cellIndexCount;   // 0xa4
    void* cellIndices;    // 0xa8
};

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

#endif // CLASSES_CPARTICLE_H

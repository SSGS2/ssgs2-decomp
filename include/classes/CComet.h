// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CCOMET_H
#define CLASSES_CCOMET_H

#include "game/types.h"

class CComet {
public:
    void* vtbl;                         // 0x0: original vtable pointer
    char pad_004[0x8];                  // 0x4
    int trailCapacity;                  // 0xc: 4 uses in CComet_CommitTrailEdge, CComet_DrawTrail
    int trailIndex;                     // 0x10: 8 uses in CComet_CommitTrailEdge, CComet_DrawTrail, CComet_StartTrail
    float sampleElapsed;                // 0x14: 6 uses in CComet_UpdateTrail, CComet_StartTrail
    float sampleInterval;               // 0x18: 2 uses in CComet_UpdateTrail
    unsigned char active;               // 0x1c: 4 uses in CComet_UpdateTrail, CComet_StartTrail, CComet_DrawTrail
    char pad_01D[0xf7];                 // 0x1d
    void* material;                     // 0x114: 11 uses in CComet_DrawTrail, CComet_dtor
    float edgeA[3];                    // 0x118: CComet_StartTrail: ball position + scaled perpendicular
    float edgeB[3];                    // 0x124: CComet_StartTrail: ball position - scaled perpendicular
    char pad_130[0x1c8];
    float trailLength;                  // 0x2f8: 11 uses in CComet_DrawTrail, CComet_CommitTrailEdge, CComet_UpdateTrail...
    char pad_2FC[0x4];                  // 0x2fc
    float brightness;                   // 0x300: 6 uses in CComet_UpdateTrail, CComet_StartTrail, CComet_DrawTrail
    void* bitmap;                       // 0x304: 3 uses in CComet_dtor, CComet_DrawTrail
    int hasPendingTrailEdge;                        // 0x308: 6 uses in CComet_DrawTrail, CComet_StartTrail, CComet_CommitTrailEdge...

    float trailEdgeA[3];               // 0x30c: CComet_UpdateTrail: ball position + scaled perpendicular
    float trailEdgeB[3];               // 0x318: CComet_UpdateTrail: ball position - scaled perpendicular

    // offsets whose address is passed on (sub-objects or arrays, not placed): 0x114, 0x118, 0x124
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define CCOMET_OFF(f, o) typedef char CComet_off_##f[((int)&((CComet*)0)->f == (o)) ? 1 : -1]
CCOMET_OFF(trailCapacity, 0xc);
CCOMET_OFF(trailIndex, 0x10);
CCOMET_OFF(sampleElapsed, 0x14);
CCOMET_OFF(sampleInterval, 0x18);
CCOMET_OFF(active, 0x1c);
CCOMET_OFF(material, 0x114);
CCOMET_OFF(trailLength, 0x2f8);
CCOMET_OFF(brightness, 0x300);
CCOMET_OFF(bitmap, 0x304);
CCOMET_OFF(hasPendingTrailEdge, 0x308);
CCOMET_OFF(edgeA, 0x118);
CCOMET_OFF(edgeB, 0x124);
CCOMET_OFF(trailEdgeA, 0x30C);
CCOMET_OFF(trailEdgeB, 0x318);
#undef CCOMET_OFF

#endif  // CLASSES_CCOMET_H

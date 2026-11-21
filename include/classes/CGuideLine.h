// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CGUIDELINE_H
#define CLASSES_CGUIDELINE_H

#include "game/types.h"

class CGuideLine {
public:
    void* vtbl;                         // 0x0: original vtable pointer
    char pad_004[0x1c];                 // 0x4
    int effectCount;                         // 0x20: 1 use in CGuideLine_OnPreLoadInit
    float opacity;                      // 0x24: 2 uses in CGuideLine_OnProcess
    char pad_028[0x8];
    unsigned char simulating; // 0x30: CGuideLine_SetEnabled 0x801782E8 sets 1; SimulateTrajectory 0x801788A4 clears
    char pad_031[0x27];
    void* effectSequence;                       // 0x58: 3 uses in CGuideLine_OnDestroy, CGuideLine_OnPreLoadInit
    char pad_05C[0xc58];                // 0x5c
    void* bitmap;                       // 0xcb4: 5 uses in CGuideLine_OnDestroy, CGuideLine_Ctor, CGuideLine_OnPreLoadInit
    char pad_CB8[0x10];                 // 0xcb8
    int vertexColor0;                   // 0xcc8: 1 use in CGuideLine_OnProcess
    char pad_CCC[0x34];                 // 0xccc
    int vertexColor1;                   // 0xd00: 1 use in CGuideLine_OnProcess
    char pad_D04[0x34];                 // 0xd04
    int vertexColor2;                   // 0xd38: 1 use in CGuideLine_OnProcess
    char pad_D3C[0x34];                 // 0xd3c
    int vertexColor3;                   // 0xd70: 1 use in CGuideLine_OnProcess

    // offsets whose address is passed on (sub-objects or arrays, not placed): 0x18, 0x28, 0xcb8, 0xcf0, 0xd28, 0xd60
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define CGUIDELINE_OFF(f, o) typedef char CGuideLine_off_##f[((int)&((CGuideLine*)0)->f == (o)) ? 1 : -1]
CGUIDELINE_OFF(effectCount, 0x20);
CGUIDELINE_OFF(opacity, 0x24);
CGUIDELINE_OFF(effectSequence, 0x58);
CGUIDELINE_OFF(bitmap, 0xcb4);
CGUIDELINE_OFF(vertexColor0, 0xcc8);
CGUIDELINE_OFF(vertexColor1, 0xd00);
CGUIDELINE_OFF(vertexColor2, 0xd38);
CGUIDELINE_OFF(vertexColor3, 0xd70);
CGUIDELINE_OFF(simulating, 0x30);
#undef CGUIDELINE_OFF

#endif  // CLASSES_CGUIDELINE_H

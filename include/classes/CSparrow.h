// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CSPARROW_H
#define CLASSES_CSPARROW_H

#include "game/types.h"

class CSparrow {
public:
    void* vtbl;                         // 0x0: original vtable pointer
    void* secondaryVtable;                        // 0x4: 1 use in CSparrow_Dtor
    char pad_008[0x1c];                 // 0x8
    void* model;                       // 0x24: 10 uses in CSparrow_Draw
    char pad_028[0x30];                 // 0x28
    float animationFrame;                       // 0x58: 1 use in CSparrow_Draw
    char pad_05C[0x51];                 // 0x5c
    unsigned char drawEnabled;               // 0xad: 1 use in CSparrow_Draw
    char pad_0AE[0x72];                 // 0xae
    int instanceCount;                        // 0x120: 1 use in CSparrow_Draw
    void* instanceMatrices;                      // 0x124: 10 uses in CSparrow_Draw, CSparrow_Dtor
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define CSPARROW_OFF(f, o) typedef char CSparrow_off_##f[((int)&((CSparrow*)0)->f == (o)) ? 1 : -1]
CSPARROW_OFF(secondaryVtable, 0x4);
CSPARROW_OFF(model, 0x24);
CSPARROW_OFF(animationFrame, 0x58);
CSPARROW_OFF(drawEnabled, 0xad);
CSPARROW_OFF(instanceCount, 0x120);
CSPARROW_OFF(instanceMatrices, 0x124);
#undef CSPARROW_OFF

#endif  // CLASSES_CSPARROW_H

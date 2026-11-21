// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CDISPOSABLE_H
#define CLASSES_CDISPOSABLE_H

#include "game/types.h"

class CDisposable {
public:
    void* vtbl;                         // 0x0: original vtable pointer
    void* secondaryVtable;                        // 0x4: 3 uses in CDisposable_Ctor, CDisposable_Dtor
    unsigned int flags;                 // 0x8: 4 uses in CCreature_Draw, CDisposable_Ctor
    char pad_00C[0x18];                 // 0xc
    void* puppet;                       // 0x24: 12 uses in CCreature_Draw, CDisposable_Ctor
    float matrixXX;                       // 0x28: 2 uses in CCreature_Draw
    char pad_02C[0xc];                  // 0x2c
    float matrixYY;                       // 0x38: 2 uses in CCreature_Draw
    char pad_03C[0xc];                  // 0x3c
    float matrixZZ;                       // 0x48: 2 uses in CCreature_Draw
    char pad_04C[0xc];                  // 0x4c
    float animationTime;                       // 0x58: 3 uses in CCreature_Draw, CDisposable_Ctor
    char pad_05C[0x4];                  // 0x5c
    int animationRange;                         // 0x60: 1 use in CDisposable_Ctor
    char pad_064[0x14];                 // 0x64
    float rotationZ;                       // 0x78: 4 uses in CCreature_Draw
    char pad_07C[0x8];                  // 0x7c
    float heading;                       // 0x84: 1 use in CDisposable_Ctor
    char pad_088[0x20];                 // 0x88
    int history;                         // 0xa8: 1 use in CDisposable_Ctor
    unsigned char opacity;               // 0xac: 1 use in CCreature_Draw
    unsigned char animationEnabled;               // 0xad: 1 use in CCreature_Draw
    char pad_0AE[0x2];                  // 0xae
    float ballSpawnOffsetDistance;                       // 0xb0: 1 use in CDisposable_Ctor
    int opacityKeyframeCount;                         // 0xb4: 1 use in CDisposable_Ctor
    void* opacityKeyframeNext;                       // 0xb8: 1 use in CDisposable_Ctor
    void* opacityKeyframePrevious;                       // 0xbc: 1 use in CDisposable_Ctor
    int targetPoints;                         // 0xc0: 1 use in CDisposable_Ctor
    int targetPointCount;                         // 0xc4: 1 use in CDisposable_Ctor
    int targetPointCapacity;                         // 0xc8: 1 use in CDisposable_Ctor
    char pad_0CC[0x4];                  // 0xcc
    int pathPoints;                         // 0xd0: 1 use in CDisposable_Ctor
    int pathPointCount;                         // 0xd4: 1 use in CDisposable_Ctor
    int pathPointCapacity;                         // 0xd8: 1 use in CDisposable_Ctor
    char pad_0DC[0x40];                 // 0xdc
    int sequenceHandleCount;                        // 0x11c: 4 uses in CDisposable_Ctor, CDisposable_Dtor
    unsigned int sequenceHandleNext;               // 0x120: 1 use in CDisposable_Ctor
    void* sequenceHandlePrevious;                      // 0x124: 4 uses in CDisposable_Dtor, CDisposable_Ctor
    int sprayHandleCount;                        // 0x128: 4 uses in CDisposable_Ctor, CDisposable_Dtor
    void* sprayHandleNext;                      // 0x12c: 1 use in CDisposable_Ctor
    void* sprayHandlePrevious;                      // 0x130: 5 uses in CDisposable_Dtor, CDisposable_Ctor

    // offsets whose address is passed on (sub-objects or arrays, not placed): 0xc, 0x18, 0x28, 0x40, 0x4c, 0x64, 0xb8, 0x11c, 0x120, 0x128, 0x12c
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define CDISPOSABLE_OFF(f, o) typedef char CDisposable_off_##f[((int)&((CDisposable*)0)->f == (o)) ? 1 : -1]
CDISPOSABLE_OFF(secondaryVtable, 0x4);
CDISPOSABLE_OFF(flags, 0x8);
CDISPOSABLE_OFF(puppet, 0x24);
CDISPOSABLE_OFF(matrixXX, 0x28);
CDISPOSABLE_OFF(matrixYY, 0x38);
CDISPOSABLE_OFF(matrixZZ, 0x48);
CDISPOSABLE_OFF(animationTime, 0x58);
CDISPOSABLE_OFF(animationRange, 0x60);
CDISPOSABLE_OFF(rotationZ, 0x78);
CDISPOSABLE_OFF(heading, 0x84);
CDISPOSABLE_OFF(history, 0xa8);
CDISPOSABLE_OFF(opacity, 0xac);
CDISPOSABLE_OFF(animationEnabled, 0xad);
CDISPOSABLE_OFF(ballSpawnOffsetDistance, 0xb0);
CDISPOSABLE_OFF(opacityKeyframeCount, 0xb4);
CDISPOSABLE_OFF(opacityKeyframeNext, 0xb8);
CDISPOSABLE_OFF(opacityKeyframePrevious, 0xbc);
CDISPOSABLE_OFF(targetPoints, 0xc0);
CDISPOSABLE_OFF(targetPointCount, 0xc4);
CDISPOSABLE_OFF(targetPointCapacity, 0xc8);
CDISPOSABLE_OFF(pathPoints, 0xd0);
CDISPOSABLE_OFF(pathPointCount, 0xd4);
CDISPOSABLE_OFF(pathPointCapacity, 0xd8);
CDISPOSABLE_OFF(sequenceHandleCount, 0x11c);
CDISPOSABLE_OFF(sequenceHandleNext, 0x120);
CDISPOSABLE_OFF(sequenceHandlePrevious, 0x124);
CDISPOSABLE_OFF(sprayHandleCount, 0x128);
CDISPOSABLE_OFF(sprayHandleNext, 0x12c);
CDISPOSABLE_OFF(sprayHandlePrevious, 0x130);
#undef CDISPOSABLE_OFF

#endif  // CLASSES_CDISPOSABLE_H

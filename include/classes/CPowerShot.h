// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CPOWERSHOT_H
#define CLASSES_CPOWERSHOT_H

#include "game/types.h"

class CPowerShot {
public:
    void* vtbl;                         // 0x0: original vtable pointer
    char pad_004[0x10];                 // 0x4
    int textureId;                         // 0x14: 3 uses in CPowerShot_BuildRingAndDraw, CPowerShot_Ctor
    unsigned char active;               // 0x18: 2 uses in CPowerShot_BuildRingAndDraw, CPowerShot_Ctor
    char pad_019[0x3];                  // 0x19
    void* ownedObject1;                       // 0x1c: 3 uses in CPowerShot_dtor, CPowerShot_Ctor
    void* ownedObject2;                       // 0x20: 3 uses in CPowerShot_dtor, CPowerShot_Ctor
    char pad_024[0xc];                  // 0x24
    void* vertex0;                       // 0x30: 1 use in CPowerShot_Ctor
    void* vertex1;                       // 0x34: 1 use in CPowerShot_Ctor
    void* vertex2;                       // 0x38: 1 use in CPowerShot_Ctor
    void* vertex3;                       // 0x3c: 1 use in CPowerShot_Ctor
    int vertexListTerminator;                         // 0x40: 1 use in CPowerShot_Ctor
    char pad_044[0xe0];                 // 0x44
    int alpha;                        // 0x124: 1 use in CPowerShot_BuildRingAndDraw
    float elapsedTime;                 // 0x128: 0x801ACFEC adds delta and tests lifetime; 0x801ACF90 clears it
    float radius;                      // 0x12c: 4 uses in CPowerShot_BuildRingAndDraw
    void* vertexBuffer;                      // 0x130: 7 uses in CPowerShot_BuildRingAndDraw, CPowerShot_dtor, CPowerShot_Ctor

    // offsets whose address is passed on (sub-objects or arrays, not placed): 0x8, 0x44, 0x7c, 0xb4, 0xec, 0x130
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define CPOWERSHOT_OFF(f, o) typedef char CPowerShot_off_##f[((int)&((CPowerShot*)0)->f == (o)) ? 1 : -1]
CPOWERSHOT_OFF(textureId, 0x14);
CPOWERSHOT_OFF(active, 0x18);
CPOWERSHOT_OFF(ownedObject1, 0x1c);
CPOWERSHOT_OFF(ownedObject2, 0x20);
CPOWERSHOT_OFF(vertex0, 0x30);
CPOWERSHOT_OFF(vertex1, 0x34);
CPOWERSHOT_OFF(vertex2, 0x38);
CPOWERSHOT_OFF(vertex3, 0x3c);
CPOWERSHOT_OFF(vertexListTerminator, 0x40);
CPOWERSHOT_OFF(alpha, 0x124);
CPOWERSHOT_OFF(elapsedTime, 0x128);
CPOWERSHOT_OFF(radius, 0x12c);
CPOWERSHOT_OFF(vertexBuffer, 0x130);
#undef CPOWERSHOT_OFF

#endif  // CLASSES_CPOWERSHOT_H

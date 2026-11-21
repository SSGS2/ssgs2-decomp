// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CSHOTPARTICLE_H
#define CLASSES_CSHOTPARTICLE_H

#include "game/types.h"

class CShotParticle {
public:
    void* vtbl;                         // 0x0: original vtable pointer
    int activeParticleCount;                          // 0x4: 2 uses in CShotParticle_Update
    char pad_008[0xc];                  // 0x8
    int bitmapId;                         // 0x14: 1 use in CShotParticle_Ctor
    unsigned char active;               // 0x18: 3 uses in CShotParticle_Update, CShotParticle_Ctor
    char pad_019[0x3];                  // 0x19
    void* ownedResource1;                       // 0x1c: 3 uses in CShotParticle_dtor, CShotParticle_Ctor
    void* ownedResource2;                       // 0x20: 3 uses in CShotParticle_dtor, CShotParticle_Ctor
    char pad_024[0xc];                  // 0x24
    void* vertex0;                       // 0x30: 1 use in CShotParticle_Ctor
    void* vertex1;                       // 0x34: 1 use in CShotParticle_Ctor
    void* vertex2;                       // 0x38: 1 use in CShotParticle_Ctor
    void* vertex3;                       // 0x3c: 1 use in CShotParticle_Ctor
    int unk_40;                         // 0x40: 1 use in CShotParticle_Ctor
    char pad_044[0xe0];                 // 0x44
    void* particles;                      // 0x124: 4 uses in CShotParticle_dtor, CShotParticle_Ctor, CShotParticle_Update
    int particleCapacity;                        // 0x128: 7 uses in CShotParticle_Ctor, CShotParticle_Update
    void* vertexBuffer;                      // 0x12c: 4 uses in CShotParticle_Ctor, CShotParticle_dtor
    void* indexBuffer;                      // 0x130: 4 uses in CShotParticle_Ctor, CShotParticle_dtor

    // offsets whose address is passed on (sub-objects or arrays, not placed): 0x44, 0x7c, 0xb4, 0xec, 0x12c, 0x130
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define CSHOTPARTICLE_OFF(f, o) typedef char CShotParticle_off_##f[((int)&((CShotParticle*)0)->f == (o)) ? 1 : -1]
CSHOTPARTICLE_OFF(activeParticleCount, 0x4);
CSHOTPARTICLE_OFF(bitmapId, 0x14);
CSHOTPARTICLE_OFF(active, 0x18);
CSHOTPARTICLE_OFF(ownedResource1, 0x1c);
CSHOTPARTICLE_OFF(ownedResource2, 0x20);
CSHOTPARTICLE_OFF(vertex0, 0x30);
CSHOTPARTICLE_OFF(vertex1, 0x34);
CSHOTPARTICLE_OFF(vertex2, 0x38);
CSHOTPARTICLE_OFF(vertex3, 0x3c);
CSHOTPARTICLE_OFF(unk_40, 0x40);
CSHOTPARTICLE_OFF(particles, 0x124);
CSHOTPARTICLE_OFF(particleCapacity, 0x128);
CSHOTPARTICLE_OFF(vertexBuffer, 0x12c);
CSHOTPARTICLE_OFF(indexBuffer, 0x130);
#undef CSHOTPARTICLE_OFF

#endif  // CLASSES_CSHOTPARTICLE_H

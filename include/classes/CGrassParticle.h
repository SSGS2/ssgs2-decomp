// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CGRASSPARTICLE_H
#define CLASSES_CGRASSPARTICLE_H

#include "game/types.h"

class CGrassParticle {
public:
    void* vtbl;                         // 0x0: original vtable pointer
    char pad_004[0x10];                 // 0x4
    unsigned int bitmapId;                // 0x14: 2 uses in CGrassParticle_Ctor, CGrassParticle_Begin
    unsigned char active;               // 0x18: 2 uses in CGrassParticle_UpdateBlades
    char pad_019[0x3];                  // 0x19
    void* sprite;                       // 0x1c: 5 uses in CGrassParticle_dtor, CGrassParticle_Ctor
    void* ownedAuxiliaryObject;                       // 0x20: 3 uses in CGrassParticle_dtor, CGrassParticle_Ctor
    char pad_024[0xc];                  // 0x24
    void* vertex0;                       // 0x30: 1 use in CGrassParticle_Ctor
    void* vertex1;                       // 0x34: 1 use in CGrassParticle_Ctor
    void* vertex2;                       // 0x38: 1 use in CGrassParticle_Ctor
    void* vertex3;                       // 0x3c: 1 use in CGrassParticle_Ctor
    unsigned int vertexListTerminator;                // 0x40: 1 use in CGrassParticle_Ctor
    char pad_044[0xe0];                 // 0x44
    void* particles;                      // 0x124: 4 uses in CGrassParticle_dtor, CGrassParticle_UpdateBlades, CGrassParticle_Ctor
    int particleCount;                        // 0x128: 3 uses in CGrassParticle_Ctor, CGrassParticle_UpdateBlades
    float initialVelocity[3];           // 0x12c: Ctor copies input vector; vfunc_3 randomizes particle velocity around it

    // offsets whose address is passed on (sub-objects or arrays, not placed): 0x44, 0x7c, 0xb4, 0xec, 0x12c
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define CGRASSPARTICLE_OFF(f, o) typedef char CGrassParticle_off_##f[((int)&((CGrassParticle*)0)->f == (o)) ? 1 : -1]
CGRASSPARTICLE_OFF(bitmapId, 0x14);
CGRASSPARTICLE_OFF(active, 0x18);
CGRASSPARTICLE_OFF(sprite, 0x1c);
CGRASSPARTICLE_OFF(ownedAuxiliaryObject, 0x20);
CGRASSPARTICLE_OFF(vertex0, 0x30);
CGRASSPARTICLE_OFF(vertex1, 0x34);
CGRASSPARTICLE_OFF(vertex2, 0x38);
CGRASSPARTICLE_OFF(vertex3, 0x3c);
CGRASSPARTICLE_OFF(vertexListTerminator, 0x40);
CGRASSPARTICLE_OFF(particles, 0x124);
CGRASSPARTICLE_OFF(particleCount, 0x128);
CGRASSPARTICLE_OFF(initialVelocity, 0x12c);
#undef CGRASSPARTICLE_OFF

#endif  // CLASSES_CGRASSPARTICLE_H

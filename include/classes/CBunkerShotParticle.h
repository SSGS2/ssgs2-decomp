// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CBUNKERSHOTPARTICLE_H
#define CLASSES_CBUNKERSHOTPARTICLE_H

#include "game/types.h"

class CBunkerShotParticle {
public:
    void* vtbl;                         // 0x0: original vtable pointer
    char pad_004[0x10];                 // 0x4
    unsigned int bitmapId;                // 0x14: 2 uses in CBunkerShotParticle_Begin, CBunkerShotParticle_Ctor
    unsigned char active;               // 0x18: 2 uses in CBunkerShotParticle_Begin, CBunkerShotParticle_Ctor
    char pad_019[0x3];                  // 0x19
    void* ownedAuxiliaryObject;                       // 0x1c: 3 uses in CBunkerShotParticle_dtor, CBunkerShotParticle_Ctor
    void* sprite;                       // 0x20: 9 uses in CBunkerShotParticle_dtor, CBunkerShotParticle_Begin, CBunkerShotParticle_Ctor
    char pad_024[0xc];                  // 0x24
    void* vertex0;                       // 0x30: 1 use in CBunkerShotParticle_Ctor
    void* vertex1;                       // 0x34: 1 use in CBunkerShotParticle_Ctor
    void* vertex2;                       // 0x38: 1 use in CBunkerShotParticle_Ctor
    void* vertex3;                       // 0x3c: 1 use in CBunkerShotParticle_Ctor
    int vertexListTerminator;                         // 0x40: 1 use in CBunkerShotParticle_Ctor
    char pad_044[0xe0];                 // 0x44
    void* spriteParticles;                      // 0x124: 5 uses in CBunkerShotParticle_dtor, CBunkerShotParticle_Ctor, CBunkerShotParticle_SeedSpritesAndStreaks
    void* streakParticles;                      // 0x128: 5 uses in CBunkerShotParticle_dtor, CBunkerShotParticle_Ctor, CBunkerShotParticle_SeedSpritesAndStreaks
    int spriteParticleCount;                        // 0x12c: 3 uses in CBunkerShotParticle_SeedSpritesAndStreaks, CBunkerShotParticle_dtor, CBunkerShotParticle_Ctor
    int streakParticleCount;                        // 0x130: 3 uses in CBunkerShotParticle_SeedSpritesAndStreaks, CBunkerShotParticle_dtor, CBunkerShotParticle_Ctor
    float fadeDuration;                      // 0x134: 1 use in CBunkerShotParticle_Begin
    float fadeRemaining;                      // 0x138: 3 uses in CBunkerShotParticle_Draw, CBunkerShotParticle_Begin
    unsigned int particleRgb;               // 0x13c: 1 use in CBunkerShotParticle_Ctor
    float streakLength;                      // 0x140: 1 use in CBunkerShotParticle_SeedSpritesAndStreaks
    void* vertexBuffer;                      // 0x144: 4 uses in CBunkerShotParticle_dtor, CBunkerShotParticle_Ctor
    void* indexBuffer;                      // 0x148: 5 uses in CBunkerShotParticle_dtor, CBunkerShotParticle_Ctor, CBunkerShotParticle_Draw

    // offsets whose address is passed on (sub-objects or arrays, not placed): 0x20, 0x44, 0x7c, 0xb4, 0xec, 0x144, 0x148
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define CBUNKERSHOTPARTICLE_OFF(f, o) typedef char CBunkerShotParticle_off_##f[((int)&((CBunkerShotParticle*)0)->f == (o)) ? 1 : -1]
CBUNKERSHOTPARTICLE_OFF(bitmapId, 0x14);
CBUNKERSHOTPARTICLE_OFF(active, 0x18);
CBUNKERSHOTPARTICLE_OFF(ownedAuxiliaryObject, 0x1c);
CBUNKERSHOTPARTICLE_OFF(sprite, 0x20);
CBUNKERSHOTPARTICLE_OFF(vertex0, 0x30);
CBUNKERSHOTPARTICLE_OFF(vertex1, 0x34);
CBUNKERSHOTPARTICLE_OFF(vertex2, 0x38);
CBUNKERSHOTPARTICLE_OFF(vertex3, 0x3c);
CBUNKERSHOTPARTICLE_OFF(vertexListTerminator, 0x40);
CBUNKERSHOTPARTICLE_OFF(spriteParticles, 0x124);
CBUNKERSHOTPARTICLE_OFF(streakParticles, 0x128);
CBUNKERSHOTPARTICLE_OFF(spriteParticleCount, 0x12c);
CBUNKERSHOTPARTICLE_OFF(streakParticleCount, 0x130);
CBUNKERSHOTPARTICLE_OFF(fadeDuration, 0x134);
CBUNKERSHOTPARTICLE_OFF(fadeRemaining, 0x138);
CBUNKERSHOTPARTICLE_OFF(particleRgb, 0x13c);
CBUNKERSHOTPARTICLE_OFF(streakLength, 0x140);
CBUNKERSHOTPARTICLE_OFF(vertexBuffer, 0x144);
CBUNKERSHOTPARTICLE_OFF(indexBuffer, 0x148);
#undef CBUNKERSHOTPARTICLE_OFF

#endif  // CLASSES_CBUNKERSHOTPARTICLE_H

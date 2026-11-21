// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CDUST_H
#define CLASSES_CDUST_H

#include "game/types.h"

class CDust {
public:
    void* vtbl;                         // 0x0: original vtable pointer
    char pad_004[0x10];                 // 0x4
    unsigned int bitmapId;                // 0x14: 2 uses in CDust_BeginEffect, CDust_Ctor
    unsigned char active;               // 0x18: 1 use in CDust_BeginEffect
    char pad_019[0x3];                  // 0x19
    void* ownedAuxiliaryObject;                       // 0x1c: 3 uses in CDust_dtor, CDust_Ctor
    void* sprite;                       // 0x20: 13 uses in CDust_dtor, CDust_BeginEffect, CDust_DrawParticles...
    char pad_024[0xc];                  // 0x24
    void* vertex0;                       // 0x30: 1 use in CDust_Ctor
    void* vertex1;                       // 0x34: 1 use in CDust_Ctor
    void* vertex2;                       // 0x38: 1 use in CDust_Ctor
    void* vertex3;                       // 0x3c: 1 use in CDust_Ctor
    unsigned int vertexListTerminator;                // 0x40: 1 use in CDust_Ctor
    char pad_044[0xe0];                 // 0x44
    void* particles;                      // 0x124: 8 uses in CDust_dtor, CDust_BeginEffect, CDust_Ctor...
    int particleCount;                        // 0x128: 2 uses in CDust_BeginEffect, CDust_Ctor
    float fadeDuration;                      // 0x12c: 1 use in CDust_BeginEffect
    float remainingLife;                      // 0x130: 3 uses in CDust_DrawParticles, CDust_BeginEffect
    float minParticleSize;                      // 0x134: 2 uses in CDust_BeginEffect, CDust_DrawParticles
    float maxParticleSize;                      // 0x138: 2 uses in CDust_BeginEffect, CDust_DrawParticles

    // offsets whose address is passed on (sub-objects or arrays, not placed): 0x44, 0x7c, 0xb4, 0xec
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define CDUST_OFF(f, o) typedef char CDust_off_##f[((int)&((CDust*)0)->f == (o)) ? 1 : -1]
CDUST_OFF(bitmapId, 0x14);
CDUST_OFF(active, 0x18);
CDUST_OFF(ownedAuxiliaryObject, 0x1c);
CDUST_OFF(sprite, 0x20);
CDUST_OFF(vertex0, 0x30);
CDUST_OFF(vertex1, 0x34);
CDUST_OFF(vertex2, 0x38);
CDUST_OFF(vertex3, 0x3c);
CDUST_OFF(vertexListTerminator, 0x40);
CDUST_OFF(particles, 0x124);
CDUST_OFF(particleCount, 0x128);
CDUST_OFF(fadeDuration, 0x12c);
CDUST_OFF(remainingLife, 0x130);
CDUST_OFF(minParticleSize, 0x134);
CDUST_OFF(maxParticleSize, 0x138);
#undef CDUST_OFF

#endif  // CLASSES_CDUST_H

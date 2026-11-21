// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CSTARDUST2_H
#define CLASSES_CSTARDUST2_H

#include "game/types.h"

class CStarDust2 {
public:
    void* vtbl;                         // 0x0: original vtable pointer
    char pad_004[0x14];                 // 0x4
    unsigned char active;               // 0x18: 6 uses in CStarDust2_UpdateGlowAndParticles, ParticleEffect_SetActive, CStarDust2_DrawGlowAndParticles...
    char pad_019[0x3];                  // 0x19
    void* sprite;                       // 0x1c: 20 uses in CStarDust2_DrawGlowAndParticles, CStarDust2_dtor, CStarDust2_BeginEffect...
    char pad_020[0x4];                  // 0x20
    float emissionElapsed;                       // 0x24: 5 uses in CStarDust2_UpdateGlowAndParticles, CStarDust2_BeginEffect
    float emissionInterval;                       // 0x28: 3 uses in CStarDust2_UpdateGlowAndParticles, CStarDust2_Ctor
    char pad_02C[0x4];                  // 0x2c
    void* vertex0;                       // 0x30: 1 use in CStarDust2_Ctor
    void* vertex1;                       // 0x34: 1 use in CStarDust2_Ctor
    void* vertex2;                       // 0x38: 1 use in CStarDust2_Ctor
    void* vertex3;                       // 0x3c: 1 use in CStarDust2_Ctor
    void* vertexListTerminator;                       // 0x40: 1 use in CStarDust2_Ctor
    char pad_044[0xe0];                 // 0x44
    void* particles;                      // 0x124: 7 uses in CStarDust2_dtor, CStarDust2_DrawGlowAndParticles, CStarDust2_EmitOrbitingParticles...
    int particleCount;                        // 0x128: 6 uses in CStarDust2_DrawGlowAndParticles, CStarDust2_EmitOrbitingParticles, CStarDust2_UpdateGlowAndParticles...
    int nextParticleIndex;                        // 0x12c: 3 uses in CStarDust2_EmitOrbitingParticles, CStarDust2_BeginEffect
    float spriteFrame;                      // 0x130: 4 uses in CStarDust2_UpdateGlowAndParticles, CStarDust2_DrawGlowAndParticles, CStarDust2_BeginEffect
    float opacity;                      // 0x134: 10 uses in CStarDust2_UpdateGlowAndParticles, CStarDust2_DrawGlowAndParticles, CStarDust2_BeginEffect

    // offsets whose address is passed on (sub-objects or arrays, not placed): 0x44, 0x7c, 0xb4, 0xec
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define CSTARDUST2_OFF(f, o) typedef char CStarDust2_off_##f[((int)&((CStarDust2*)0)->f == (o)) ? 1 : -1]
CSTARDUST2_OFF(active, 0x18);
CSTARDUST2_OFF(sprite, 0x1c);
CSTARDUST2_OFF(emissionElapsed, 0x24);
CSTARDUST2_OFF(emissionInterval, 0x28);
CSTARDUST2_OFF(vertex0, 0x30);
CSTARDUST2_OFF(vertex1, 0x34);
CSTARDUST2_OFF(vertex2, 0x38);
CSTARDUST2_OFF(vertex3, 0x3c);
CSTARDUST2_OFF(vertexListTerminator, 0x40);
CSTARDUST2_OFF(particles, 0x124);
CSTARDUST2_OFF(particleCount, 0x128);
CSTARDUST2_OFF(nextParticleIndex, 0x12c);
CSTARDUST2_OFF(spriteFrame, 0x130);
CSTARDUST2_OFF(opacity, 0x134);
#undef CSTARDUST2_OFF

#endif  // CLASSES_CSTARDUST2_H

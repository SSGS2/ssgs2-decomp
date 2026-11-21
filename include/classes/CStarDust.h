// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CSTARDUST_H
#define CLASSES_CSTARDUST_H

#include "game/types.h"

class CStarDust {
public:
    void* vtbl;                         // 0x0: original vtable pointer
    char pad_004[0x20];                 // 0x4
    float spawnElapsed;                       // 0x24: 1 use in CStarDust_StartAndResetParticles
    float spawnInterval;                       // 0x28: 1 use in CStarDust_Ctor
    char pad_02C[0x4];                  // 0x2c
    void* vertex0;                       // 0x30: 1 use in CStarDust_Ctor
    void* vertex1;                       // 0x34: 1 use in CStarDust_Ctor
    void* vertex2;                       // 0x38: 1 use in CStarDust_Ctor
    void* vertex3;                       // 0x3c: 1 use in CStarDust_Ctor
    void* vertexListTerminator;                       // 0x40: 1 use in CStarDust_Ctor
    char pad_044[0xe0];                 // 0x44
    void* particles;                      // 0x124: 7 uses in CStarDust_dtor, CStarDust_ResetParticle, CStarDust_DrawBallGlowAndParticles...
    unsigned int particleCount;               // 0x128: 5 uses in CStarDust_Ctor, CStarDust_DrawBallGlowAndParticles, CStarDust_dtor...
    int particleIndex;                        // 0x12c: 2 uses in CStarDust_ResetParticle, CStarDust_StartAndResetParticles
    char pad_130[0x4];                  // 0x130
    float alpha;                      // 0x134: 2 uses in CStarDust_DrawBallGlowAndParticles, CStarDust_StartAndResetParticles

    // offsets whose address is passed on (sub-objects or arrays, not placed): 0x44, 0x7c, 0xb4, 0xec
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define CSTARDUST_OFF(f, o) typedef char CStarDust_off_##f[((int)&((CStarDust*)0)->f == (o)) ? 1 : -1]
CSTARDUST_OFF(spawnElapsed, 0x24);
CSTARDUST_OFF(spawnInterval, 0x28);
CSTARDUST_OFF(vertex0, 0x30);
CSTARDUST_OFF(vertex1, 0x34);
CSTARDUST_OFF(vertex2, 0x38);
CSTARDUST_OFF(vertex3, 0x3c);
CSTARDUST_OFF(vertexListTerminator, 0x40);
CSTARDUST_OFF(particles, 0x124);
CSTARDUST_OFF(particleCount, 0x128);
CSTARDUST_OFF(particleIndex, 0x12c);
CSTARDUST_OFF(alpha, 0x134);
#undef CSTARDUST_OFF

#endif  // CLASSES_CSTARDUST_H

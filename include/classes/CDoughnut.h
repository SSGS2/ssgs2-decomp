// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CDOUGHNUT_H
#define CLASSES_CDOUGHNUT_H

#include "game/types.h"

class CDoughnut {
public:
    void* vtbl;                         // 0x0: original vtable pointer
    char pad_004[0x4];                  // 0x4
    void* bitmapResource;                        // 0x8: 3 uses in CDoughnut_dtor, CDoughnut_DrawParticleQuads
    int particleCount;                          // 0xc: 4 uses in CDoughnut_UpdateParticles, CDoughnut_DrawParticleQuads, CDoughnut_StartAndResetRingParticles
    int nextParticleIndex;                         // 0x10: 5 uses in CDoughnut_UpdateParticles, CDoughnut_StartAndResetRingParticles
    float spawnTimer;                       // 0x14: 4 uses in CDoughnut_UpdateParticles, CDoughnut_StartAndResetRingParticles
    float spawnInterval;                       // 0x18: 2 uses in CDoughnut_UpdateParticles
    unsigned char active;               // 0x1c: 4 uses in CDoughnut_UpdateParticles, CDoughnut_DrawParticleQuads, CDoughnut_StartAndResetRingParticles
    char pad_01D[0xf7];                 // 0x1d
    void* material;                      // 0x114: 7 uses in CDoughnut_DrawParticleQuads, CDoughnut_dtor, CDoughnut_StartAndResetRingParticles
    void* particles;                      // 0x118: 10 uses in CDoughnut_UpdateParticles, CDoughnut_dtor, CDoughnut_DrawParticleQuads...
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define CDOUGHNUT_OFF(f, o) typedef char CDoughnut_off_##f[((int)&((CDoughnut*)0)->f == (o)) ? 1 : -1]
CDOUGHNUT_OFF(bitmapResource, 0x8);
CDOUGHNUT_OFF(particleCount, 0xc);
CDOUGHNUT_OFF(nextParticleIndex, 0x10);
CDOUGHNUT_OFF(spawnTimer, 0x14);
CDOUGHNUT_OFF(spawnInterval, 0x18);
CDOUGHNUT_OFF(active, 0x1c);
CDOUGHNUT_OFF(material, 0x114);
CDOUGHNUT_OFF(particles, 0x118);
#undef CDOUGHNUT_OFF

#endif  // CLASSES_CDOUGHNUT_H

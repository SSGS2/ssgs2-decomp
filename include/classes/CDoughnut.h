// CDoughnut: the rings that spread out from the ball during a shot.
//
// A fixed set of particles is recycled; the newest ring is placed with the
// ball's position and velocity, and the rings shrink and fade as they age.
#ifndef CLASSES_CDOUGHNUT_H
#define CLASSES_CDOUGHNUT_H

#include "game/types.h"
#include "game/wvector.h"

// One ring: its size and opacity, and the world-space frame it was spawned in.
struct DoughnutParticle {
    float size;
    float opacity;
    unsigned char active;
    char pad_09[3];
    WVector right;
    WVector up;
    WVector forward;
    WVector position;
};

class CDoughnut {
public:
    ~CDoughnut();

    void StartAndResetRingParticles();
    void UpdateParticles(float delta);
    void DrawParticleQuads();

    void* vtbl;                  // 0x00
    char pad_004[0x4];           // 0x04
    void* bitmapResource;        // 0x08
    int particleCount;           // 0x0C
    int nextParticleIndex;       // 0x10
    float spawnTimer;            // 0x14
    float spawnInterval;         // 0x18
    unsigned char active;        // 0x1C
    char pad_01D[0xF7];          // 0x1D
    void* material;              // 0x114
    DoughnutParticle* particles; // 0x118
};

#define CDOUGHNUT_OFF(f, o) typedef char CDoughnut_off_##f[((int)&((CDoughnut*)0)->f == (o)) ? 1 : -1]
CDOUGHNUT_OFF(bitmapResource, 0x8);
CDOUGHNUT_OFF(particleCount, 0xC);
CDOUGHNUT_OFF(nextParticleIndex, 0x10);
CDOUGHNUT_OFF(spawnTimer, 0x14);
CDOUGHNUT_OFF(spawnInterval, 0x18);
CDOUGHNUT_OFF(active, 0x1C);
CDOUGHNUT_OFF(material, 0x114);
CDOUGHNUT_OFF(particles, 0x118);
#undef CDOUGHNUT_OFF
typedef char DoughnutParticle_size[(sizeof(DoughnutParticle) == 0x3C) ? 1 : -1];

#endif // CLASSES_CDOUGHNUT_H

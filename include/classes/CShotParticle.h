#ifndef CLASSES_CSHOTPARTICLE_H
#define CLASSES_CSHOTPARTICLE_H

#include "game/types.h"

class CShotParticle {
public:
    void* vtbl;              // 0x0: original vtable pointer
    int activeParticleCount; // 0x4
    char pad_008[0xc];       // 0x8
    int bitmapId;            // 0x14
    unsigned char active;    // 0x18
    char pad_019[0x3];       // 0x19
    void* ownedResource1;    // 0x1c
    void* ownedResource2;    // 0x20
    char pad_024[0xc];       // 0x24
    void* vertex0;           // 0x30
    void* vertex1;           // 0x34
    void* vertex2;           // 0x38
    void* vertex3;           // 0x3c
    int unk_40;              // 0x40
    char pad_044[0xe0];      // 0x44
    void* particles;         // 0x124
    int particleCapacity;    // 0x128
    void* vertexBuffer;      // 0x12c
    void* indexBuffer;       // 0x130

    void vfunc_5();
    void vfunc_6();
    void vfunc_7();

    // Shot trails: a set of streaks seeded from the ball's recent motion.
    void SpawnStreaks();
    void Update(float delta);
    void DrawStreaks();

    // Release the streak array and buffers. A delete flag above zero also frees
    // the object; either way the address is returned.
    void* Destroy(int flags);

    CShotParticle();
};

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
typedef char CShotParticle_size[(sizeof(CShotParticle) == 0x134) ? 1 : -1];

#endif // CLASSES_CSHOTPARTICLE_H

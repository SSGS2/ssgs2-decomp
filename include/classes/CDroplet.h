// CDroplet: the spray of water droplets a ball throws up out of a pond.
//
// The class has no virtual functions here: its vtable comes from the original
// data, so `vtbl` is written out as a member.
#ifndef CLASSES_CDROPLET_H
#define CLASSES_CDROPLET_H

#include "game/types.h"

class CDroplet {
public:
    ~CDroplet();

    // Launch the droplets with random radial velocities.
    void InitRandomVelocities();
    // Move the droplets under gravity and fade the spray.
    void UpdateAndFade(float delta);
    void DrawParticles();

    void* vtbl;                 // 0x0
    char pad_004[0x14];         // 0x4
    unsigned char active;       // 0x18
    char pad_019[3];            // 0x19
    void* ownedAuxiliaryObject; // 0x1c
    void* sprite;               // 0x20
    char pad_024[0x100];        // 0x24
    void* particles;            // 0x124
    int particleCount;          // 0x128
    float lifetime;             // 0x12c  seconds
    float alpha;                // 0x130  starts at 255 and fades to zero
};

#define CDROPLET_OFF(f, o) typedef char CDroplet_off_##f[((int)&((CDroplet*)0)->f == (o)) ? 1 : -1]
CDROPLET_OFF(active, 0x18);
CDROPLET_OFF(ownedAuxiliaryObject, 0x1c);
CDROPLET_OFF(sprite, 0x20);
CDROPLET_OFF(particles, 0x124);
CDROPLET_OFF(particleCount, 0x128);
CDROPLET_OFF(lifetime, 0x12c);
CDROPLET_OFF(alpha, 0x130);
#undef CDROPLET_OFF

#endif // CLASSES_CDROPLET_H

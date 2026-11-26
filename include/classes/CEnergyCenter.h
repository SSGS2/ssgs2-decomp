#ifndef CLASSES_CENERGYCENTER_H
#define CLASSES_CENERGYCENTER_H

#include "game/types.h"

class CEnergyCenter {
public:
    // Set up the effect with count particles, the center at src, and the rotation value in radians.
    void* Init(int count, void* src, float value);
    // Activate the effect, reset its spawn count, and recreate its sprite grid.
    void ResetEffect();
    // Emit inward-moving particles, accumulate center opacity as they arrive, then fade the center.
    void UpdateEmitAndParticles(float dt);
    // Draw the animated center sprite and every live inward-moving particle.
    void DrawCenterAndParticles();
    // Release owned particles, sprites, and buffers; a delete flag above zero also frees the object.
    void* Destroy(int flags);

    void* vtbl;                 // 0x0: original vtable pointer
    char pad_004[0x10];         // 0x4
    void* bitmapResource;       // 0x14
    unsigned char active;       // 0x18
    char pad_019[0x3];          // 0x19
    void* sprite;               // 0x1c
    void* auxiliarySprite;      // 0x20
    char pad_024[0xc];          // 0x24
    void* vertex0;              // 0x30
    void* vertex1;              // 0x34
    void* vertex2;              // 0x38
    void* vertex3;              // 0x3c
    void* vertexListTerminator; // 0x40
    char pad_044[0xe0];         // 0x44
    void* particles;            // 0x124
    float rotationAngle;        // 0x128
    int spawnedParticleCount;   // 0x12c
    int particleCapacity;       // 0x130
    char pad_134[0x4];          // 0x134
    float centerY;              // 0x138
    char pad_13C[0x4];          // 0x13c
    float centerOpacity;        // 0x140
    float centerFrame;          // 0x144
    float unk_148;              // 0x148
    float unk_14C;              // 0x14c
    void* golfBackgroundActor;  // 0x150
};

#define CENERGYCENTER_OFF(f, o) typedef char CEnergyCenter_off_##f[((int)&((CEnergyCenter*)0)->f == (o)) ? 1 : -1]
CENERGYCENTER_OFF(active, 0x18);
CENERGYCENTER_OFF(sprite, 0x1c);
CENERGYCENTER_OFF(vertex0, 0x30);
CENERGYCENTER_OFF(vertex1, 0x34);
CENERGYCENTER_OFF(vertex2, 0x38);
CENERGYCENTER_OFF(vertex3, 0x3c);
CENERGYCENTER_OFF(vertexListTerminator, 0x40);
CENERGYCENTER_OFF(particles, 0x124);
CENERGYCENTER_OFF(rotationAngle, 0x128);
CENERGYCENTER_OFF(spawnedParticleCount, 0x12c);
CENERGYCENTER_OFF(particleCapacity, 0x130);
CENERGYCENTER_OFF(centerY, 0x138);
CENERGYCENTER_OFF(centerOpacity, 0x140);
CENERGYCENTER_OFF(centerFrame, 0x144);
CENERGYCENTER_OFF(unk_148, 0x148);
CENERGYCENTER_OFF(unk_14C, 0x14c);
CENERGYCENTER_OFF(golfBackgroundActor, 0x150);
#undef CENERGYCENTER_OFF

#endif // CLASSES_CENERGYCENTER_H

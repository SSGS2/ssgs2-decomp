#ifndef CLASSES_CSTARDUST2_H
#define CLASSES_CSTARDUST2_H

#include "game/types.h"

class CStarDust2 {
public:
    // Set up the effect in storage the caller owns and allocate its orbit records; returns this.
    void* Init();
    // Draw the glow sprite and the live orbit particles.
    void DrawGlowAndParticles();
    // Emit four orbiting dust particles into the circular particle buffer.
    void EmitOrbitingParticles();
    // Fade the glow with the ball's descent, emit dust, and update the orbit particles.
    void UpdateGlowAndParticles(float delta);
    // Recreate the second dust sprite grid and reset its emission cursor and live flags.
    void BeginEffect();
    // Release owned particles, sprites, and buffers; a delete flag above zero also frees the object.
    void* Destroy(int flags);

    void* vtbl;                 // 0x0: original vtable pointer
    char pad_004[0x10];         // 0x4
    void* bitmapResource;       // 0x14
    unsigned char active;       // 0x18
    char pad_019[0x3];          // 0x19
    void* sprite;               // 0x1c
    void* auxiliarySprite;      // 0x20
    float emissionElapsed;      // 0x24
    float emissionInterval;     // 0x28
    char pad_02C[0x4];          // 0x2c
    void* vertex0;              // 0x30
    void* vertex1;              // 0x34
    void* vertex2;              // 0x38
    void* vertex3;              // 0x3c
    void* vertexListTerminator; // 0x40
    char pad_044[0xe0];         // 0x44
    void* particles;            // 0x124
    int particleCount;          // 0x128
    int nextParticleIndex;      // 0x12c
    float spriteFrame;          // 0x130
    float opacity;              // 0x134
};

#define CSTARDUST2_OFF(f, o) typedef char CStarDust2_off_##f[((int)&((CStarDust2*)0)->f == (o)) ? 1 : -1]
CSTARDUST2_OFF(bitmapResource, 0x14);
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

#endif // CLASSES_CSTARDUST2_H

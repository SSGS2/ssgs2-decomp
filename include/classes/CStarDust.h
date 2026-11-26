#ifndef CLASSES_CSTARDUST_H
#define CLASSES_CSTARDUST_H

#include "game/types.h"

class CStarDust {
public:
    // Initialize the next orbit particle from the ball and the camera basis.
    void ResetParticle();
    // Advance one orbit particle's angle, radius and fade.
    void UpdateParticle(int index, float dt);
    // Fade the dust with the ball's descent, restore the ball's opacity, and emit particles.
    void Update(float dt);
    // Recreate the dust sprite grid and reset the emission cursor, opacity, and live flags.
    void StartAndResetParticles();
    // Draw the ball glow and the active dust sprites.
    void DrawBallGlowAndParticles();
    // Release owned particles, sprites, and buffers; a delete flag above zero also frees the object.
    void* Destroy(int flags);
    // Allocate the orbit records and set the emission interval.
    void* Init();

    void* vtbl;                 // 0x0: original vtable pointer
    char pad_004[0x10];         // 0x4
    void* bitmapResource;       // 0x14
    unsigned char active;       // 0x18
    char pad_019[0x3];          // 0x19
    void* sprite;               // 0x1c
    void* auxiliarySprite;      // 0x20
    float spawnElapsed;         // 0x24
    float spawnInterval;        // 0x28
    char pad_02C[0x4];          // 0x2c
    void* vertex0;              // 0x30
    void* vertex1;              // 0x34
    void* vertex2;              // 0x38
    void* vertex3;              // 0x3c
    void* vertexListTerminator; // 0x40
    char pad_044[0xe0];         // 0x44
    void* particles;            // 0x124
    unsigned int particleCount; // 0x128
    int particleIndex;          // 0x12c
    float spriteFrame;          // 0x130
    float alpha;                // 0x134
};

#define CSTARDUST_OFF(f, o) typedef char CStarDust_off_##f[((int)&((CStarDust*)0)->f == (o)) ? 1 : -1]
CSTARDUST_OFF(bitmapResource, 0x14);
CSTARDUST_OFF(active, 0x18);
CSTARDUST_OFF(sprite, 0x1c);
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

#endif // CLASSES_CSTARDUST_H

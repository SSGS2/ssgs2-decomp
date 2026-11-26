// CDust: the puff of dust a ball kicks up when it lands.
//
// One billboard sprite is drawn once per particle, growing and fading over the
// effect's lifetime. The class is declared without virtual functions: its
// vtable comes from the original data, so `vtbl` is written out as a member.
#ifndef CLASSES_CDUST_H
#define CLASSES_CDUST_H

#include "game/types.h"
#include "game/wvector.h"

class CDust {
public:
    CDust(int count);
    ~CDust();

    // Restart the effect at the current ball.
    void BeginEffect();
    // Advance the normalized fade timer; the effect stops when it reaches zero.
    void UpdateFadeCountdown(float amount);
    // Draw the particles that are still alive.
    void DrawParticles();

    void* vtbl;                        // 0x0
    char pad_004[4];                   // 0x4
    WVector position;                  // 0x8: where the dust starts
    unsigned int bitmapId;             // 0x14
    unsigned char active;              // 0x18
    char pad_019[3];                   // 0x19
    void* ownedAuxiliaryObject;        // 0x1c
    void* sprite;                      // 0x20
    char pad_024[0xC];                 // 0x24
    void* vertex0;                     // 0x30
    void* vertex1;                     // 0x34
    void* vertex2;                     // 0x38
    void* vertex3;                     // 0x3c
    unsigned int vertexListTerminator; // 0x40
    char vertexBuffers[4][0x38];       // 0x44: the four quads the pointers above address
    void* particles;                   // 0x124
    int particleCount;                 // 0x128
    float fadeDuration;                // 0x12c
    float remainingLife;               // 0x130: 1 at the start, 0 when the effect ends
    float minParticleSize;             // 0x134
    float maxParticleSize;             // 0x138
};

#define CDUST_OFF(f, o) typedef char CDust_off_##f[((int)&((CDust*)0)->f == (o)) ? 1 : -1]
CDUST_OFF(position, 0x8);
CDUST_OFF(bitmapId, 0x14);
CDUST_OFF(active, 0x18);
CDUST_OFF(ownedAuxiliaryObject, 0x1c);
CDUST_OFF(sprite, 0x20);
CDUST_OFF(vertex0, 0x30);
CDUST_OFF(vertex1, 0x34);
CDUST_OFF(vertex2, 0x38);
CDUST_OFF(vertex3, 0x3c);
CDUST_OFF(vertexListTerminator, 0x40);
CDUST_OFF(vertexBuffers, 0x44);
CDUST_OFF(particles, 0x124);
CDUST_OFF(particleCount, 0x128);
CDUST_OFF(fadeDuration, 0x12c);
CDUST_OFF(remainingLife, 0x130);
CDUST_OFF(minParticleSize, 0x134);
CDUST_OFF(maxParticleSize, 0x138);
#undef CDUST_OFF
typedef char CDust_size[(sizeof(CDust) == 0x13C) ? 1 : -1];

#endif // CLASSES_CDUST_H

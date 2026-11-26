// CBunkerShotParticle: the sand thrown up by a shot out of a bunker.
//
// The effect keeps two particle arrays: rounded sprites and thin streaks, both
// seeded from the ball's recent motion. The class has no virtual functions here:
// its vtable comes from the original data, so `vtbl` is written out as a member.
#ifndef CLASSES_CBUNKERSHOTPARTICLE_H
#define CLASSES_CBUNKERSHOTPARTICLE_H

#include "game/types.h"

class CBunkerShotParticle {
public:
    CBunkerShotParticle(int spriteCount, int streakCount);
    ~CBunkerShotParticle();

    // Seed the sand sprites and streaks from the ball's recent motion.
    void SeedSpritesAndStreaks();
    // Restart the effect and play the sand sound.
    void Begin();
    // Move the sand sprites and streaks, apply gravity, and fade the effect.
    void UpdateParticles(float delta);
    // Draw the sand sprites and the live streaks.
    void Draw();

    void* vtbl;                  // 0x0
    char pad_004[0x10];          // 0x4
    unsigned int bitmapId;       // 0x14
    unsigned char active;        // 0x18
    char pad_019[3];             // 0x19
    void* ownedAuxiliaryObject;  // 0x1c
    void* sprite;                // 0x20
    char pad_024[0xC];           // 0x24
    void* vertex0;               // 0x30
    void* vertex1;               // 0x34
    void* vertex2;               // 0x38
    void* vertex3;               // 0x3c
    int vertexListTerminator;    // 0x40
    char vertexBuffers[4][0x38]; // 0x44: the four quads the pointers above address
    void* spriteParticles;       // 0x124
    void* streakParticles;       // 0x128
    int spriteParticleCount;     // 0x12c
    int streakParticleCount;     // 0x130
    float fadeDuration;          // 0x134
    float fadeRemaining;         // 0x138
    unsigned int particleRgb;    // 0x13c
    float streakLength;          // 0x140
    void* vertexBuffer;          // 0x144
    void* indexBuffer;           // 0x148
};

#define CBUNKERSHOTPARTICLE_OFF(f, o) \
    typedef char CBunkerShotParticle_off_##f[((int)&((CBunkerShotParticle*)0)->f == (o)) ? 1 : -1]
CBUNKERSHOTPARTICLE_OFF(bitmapId, 0x14);
CBUNKERSHOTPARTICLE_OFF(active, 0x18);
CBUNKERSHOTPARTICLE_OFF(ownedAuxiliaryObject, 0x1c);
CBUNKERSHOTPARTICLE_OFF(sprite, 0x20);
CBUNKERSHOTPARTICLE_OFF(vertex0, 0x30);
CBUNKERSHOTPARTICLE_OFF(vertex1, 0x34);
CBUNKERSHOTPARTICLE_OFF(vertex2, 0x38);
CBUNKERSHOTPARTICLE_OFF(vertex3, 0x3c);
CBUNKERSHOTPARTICLE_OFF(vertexListTerminator, 0x40);
CBUNKERSHOTPARTICLE_OFF(spriteParticles, 0x124);
CBUNKERSHOTPARTICLE_OFF(streakParticles, 0x128);
CBUNKERSHOTPARTICLE_OFF(spriteParticleCount, 0x12c);
CBUNKERSHOTPARTICLE_OFF(streakParticleCount, 0x130);
CBUNKERSHOTPARTICLE_OFF(fadeDuration, 0x134);
CBUNKERSHOTPARTICLE_OFF(fadeRemaining, 0x138);
CBUNKERSHOTPARTICLE_OFF(particleRgb, 0x13c);
CBUNKERSHOTPARTICLE_OFF(streakLength, 0x140);
CBUNKERSHOTPARTICLE_OFF(vertexBuffer, 0x144);
CBUNKERSHOTPARTICLE_OFF(indexBuffer, 0x148);
#undef CBUNKERSHOTPARTICLE_OFF
typedef char CBunkerShotParticle_size[(sizeof(CBunkerShotParticle) == 0x14C) ? 1 : -1];

#endif // CLASSES_CBUNKERSHOTPARTICLE_H

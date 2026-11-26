#ifndef CLASSES_OBJECT801AAED8_H
#define CLASSES_OBJECT801AAED8_H

#include "game/types.h"

class Object801AAED8 {
public:
    char pad_000[4];                // 0x0: unmodelled bytes
    float initialOpacity;           // 0x4: Particle_UpdateLifetimeSizeOpacityAndWind original object access
    float finalOpacity;             // 0x8: Particle_UpdateLifetimeSizeOpacityAndWind original object access
    float initialParticleSize;      // 0xc: Particle_UpdateLifetimeSizeOpacityAndWind original object access
    float finalParticleSize;        // 0x10: Particle_UpdateLifetimeSizeOpacityAndWind original object access
    char pad_014[0x18];             // 0x14
    float gravityScale;             // 0x2c: Particle_UpdateLifetimeSizeOpacityAndWind original object access
    char pad_030[0x4];              // 0x30
    float maxParticleLifetime;      // 0x34: ParticleEmitter_LoadDefinition original object access
    float spriteFrameInterval;      // 0x38: Particle_UpdateLifetimeSizeOpacityAndWind original object access
    float emissionInterval;         // 0x3c: ParticleEmitter_LoadDefinition original object access
    float bounceElasticity;         // 0x40: AdvanceParticle original object access
    float emissionTimeRemaining;    // 0x44: ParticleEmitter_UpdateAndEmit original object access
    float bouncePlaneY;             // 0x48: AdvanceParticle original object access
    char pad_04C[0x4];              // 0x4c
    unsigned char active;           // 0x50: ParticleEmitter_LoadDefinition original object access
    char pad_051[0x3];              // 0x51
    void* particleStorage;          // 0x54: AdvanceParticle original object access
    int activeParticleCount;        // 0x58: ParticleEmitter_LoadDefinition original object access
    void* sprite;                   // 0x5c: ParticleEmitter_LoadDefinition original object access
    char pad_060[0x68];             // 0x60
    float spriteSize;               // 0xc8: ParticleEmitter_LoadDefinition original object access
    char pad_0CC[0x24];             // 0xcc
    int particleCapacity;           // 0xf0: ParticleEmitter_LoadDefinition original object access
    int nextParticleIndex;          // 0xf4: ParticleEmitter_LoadDefinition original object access
    void* positionLink;             // 0xf8: ParticleEmitter_LoadDefinition original object access
    char pad_0FC[0xc];              // 0xfc
    float emissionTimer;            // 0x108: ParticleEmitter_LoadDefinition original object access
    int particlesPerEmission;       // 0x10c: ParticleEmitter_LoadDefinition original object access
    unsigned char drawingEnabled;   // 0x110: ParticleEmitter_LoadDefinition original object access
    char pad_111[0x3];              // 0x111
    int surfaceIndex;               // 0x114: ParticleEmitter_LoadDefinition original object access
    float surfaceOpacityMultiplier; // 0x118: ParticleEmitter_LoadDefinition original object access
};

#define OBJECT801AAED8_OFF(f, o) typedef char Object801AAED8_off_##f[((int)&((Object801AAED8*)0)->f == (o)) ? 1 : -1]
OBJECT801AAED8_OFF(initialOpacity, 0x4);
OBJECT801AAED8_OFF(finalOpacity, 0x8);
OBJECT801AAED8_OFF(initialParticleSize, 0xc);
OBJECT801AAED8_OFF(finalParticleSize, 0x10);
OBJECT801AAED8_OFF(gravityScale, 0x2c);
OBJECT801AAED8_OFF(maxParticleLifetime, 0x34);
OBJECT801AAED8_OFF(spriteFrameInterval, 0x38);
OBJECT801AAED8_OFF(emissionInterval, 0x3c);
OBJECT801AAED8_OFF(bounceElasticity, 0x40);
OBJECT801AAED8_OFF(emissionTimeRemaining, 0x44);
OBJECT801AAED8_OFF(bouncePlaneY, 0x48);
OBJECT801AAED8_OFF(active, 0x50);
OBJECT801AAED8_OFF(particleStorage, 0x54);
OBJECT801AAED8_OFF(activeParticleCount, 0x58);
OBJECT801AAED8_OFF(sprite, 0x5c);
OBJECT801AAED8_OFF(spriteSize, 0xc8);
OBJECT801AAED8_OFF(particleCapacity, 0xf0);
OBJECT801AAED8_OFF(nextParticleIndex, 0xf4);
OBJECT801AAED8_OFF(positionLink, 0xf8);
OBJECT801AAED8_OFF(emissionTimer, 0x108);
OBJECT801AAED8_OFF(particlesPerEmission, 0x10c);
OBJECT801AAED8_OFF(drawingEnabled, 0x110);
OBJECT801AAED8_OFF(surfaceIndex, 0x114);
OBJECT801AAED8_OFF(surfaceOpacityMultiplier, 0x118);
#undef OBJECT801AAED8_OFF

#endif // CLASSES_OBJECT801AAED8_H

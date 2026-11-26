// SnowParticle: owned by CSnow methods in snow_weather_particles.cpp.
// CSnow_RebuildParticleBuffers (0x801FA2D0) allocates 0x28-byte records; CSnow_UpdateParticles (0x801FA7F4)
// adds wind/fall to center, then adds a sinusoidal sway to produce position.
#ifndef CLASSES_SNOWPARTICLE_H
#define CLASSES_SNOWPARTICLE_H

struct SnowParticle {
    float position[3];  // 0x0: rendered position, CSnow_UpdateParticles final Vec3_Add
    float center[3];    // 0xC: wind/fall integration and bounds wrapping
    float directionSin; // 0x18: Math_SinApprox(random angle) in CSnow_RebuildParticleBuffers
    float directionCos; // 0x1C: Math_CosApprox(same angle)
    float swayPhase;    // 0x20: incremented by angular rate * elapsed time
    char unk_24;        // 0x24: initialized to 1; meaning not established
    char pad_025[3];
};

#define OFF(f, o) typedef char SnowParticle_off_##f[((int)&((SnowParticle*)0)->f == (o)) ? 1 : -1]
OFF(position, 0x0);
OFF(center, 0xC);
OFF(directionSin, 0x18);
OFF(directionCos, 0x1C);
OFF(swayPhase, 0x20);
OFF(unk_24, 0x24);
#undef OFF
typedef char SnowParticle_size[(sizeof(SnowParticle) == 0x28) ? 1 : -1];

#endif

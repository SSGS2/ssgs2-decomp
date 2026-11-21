// StarDust2Particle: scalar view of the 0x50-byte particles owned by CStarDust2 in course_ball_shot_effects.cpp.
// Layout-only view; no virtual declarations or generated RTTI.
#ifndef CLASSES_STARDUST2PARTICLE_H
#define CLASSES_STARDUST2PARTICLE_H

class StarDust2Particle {
public:
    unsigned char active; // 0x0: CStarDust2_EmitOrbitingParticles activates; CStarDust2_UpdateGlowAndParticles clears on expiry
    char pad_001[0x3];
    float radialScale; // 0x4: CStarDust2_UpdateGlowAndParticles scales the orbit vector by this value
    char pad_008[0x3C];
    float angle; // 0x44: CStarDust2_UpdateGlowAndParticles passes this value to sine and cosine
    float angularVelocity; // 0x48: CStarDust2_UpdateGlowAndParticles integrates into angle
    float opacity; // 0x4C: CStarDust2_DrawGlowAndParticles shifts this value into the alpha byte
};

#define OFF(f, o) typedef char StarDust2Particle_off_##f[((int)&((StarDust2Particle*)0)->f == (o)) ? 1 : -1]
OFF(active, 0x0);
OFF(radialScale, 0x4);
OFF(angle, 0x44);
OFF(angularVelocity, 0x48);
OFF(opacity, 0x4C);
#undef OFF

#endif

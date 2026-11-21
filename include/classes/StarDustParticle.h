// StarDustParticle: scalar view of the 0x50-byte particles owned by CStarDust in course_ball_shot_effects.cpp.
// Layout-only view; no virtual declarations or generated RTTI.
#ifndef CLASSES_STARDUSTPARTICLE_H
#define CLASSES_STARDUSTPARTICLE_H

class StarDustParticle {
public:
    unsigned char active; // 0x0: CStarDust_ResetParticle activates; CStarDust_DrawBallGlowAndParticles tests before drawing
    char pad_001[0x3];
    float radialScale; // 0x4: CStarDust_ResetParticle initializes from ball speed
    char pad_008[0x3C];
    float angle; // 0x44: CStarDust_ResetParticle initializes the orbit angle
    float angularVelocity; // 0x48: CStarDust_ResetParticle initializes the angular velocity
    float sizeFactor; // 0x4C: CStarDust_DrawBallGlowAndParticles multiplies the billboard scale by this value
};

#define OFF(f, o) typedef char StarDustParticle_off_##f[((int)&((StarDustParticle*)0)->f == (o)) ? 1 : -1]
OFF(active, 0x0);
OFF(radialScale, 0x4);
OFF(angle, 0x44);
OFF(angularVelocity, 0x48);
OFF(sizeFactor, 0x4C);
#undef OFF

#endif

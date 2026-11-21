// EnergyCenterParticle: scalar view of the 0x20-byte particles owned by CEnergyCenter in course_ball_shot_effects.cpp.
// Layout-only view; no virtual declarations or generated RTTI.
#ifndef CLASSES_ENERGYCENTERPARTICLE_H
#define CLASSES_ENERGYCENTERPARTICLE_H

class EnergyCenterParticle {
public:
    unsigned char active; // 0x0: CEnergyCenter_DrawCenterAndParticles tests before drawing
    char pad_001[0x1B];
    float opacity; // 0x1C: CEnergyCenter_DrawCenterAndParticles shifts this value into the alpha byte
};

#define OFF(f, o) typedef char EnergyCenterParticle_off_##f[((int)&((EnergyCenterParticle*)0)->f == (o)) ? 1 : -1]
OFF(active, 0x0);
OFF(opacity, 0x1C);
#undef OFF

#endif

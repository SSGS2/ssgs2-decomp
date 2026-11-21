// Shared layout view for ntPetal and ntSilicaParticle methods in petal_silica_particles.cpp.
#ifndef CLASSES_PARTICLEEMITTERSTATE801B7D5C_H
#define CLASSES_PARTICLEEMITTERSTATE801B7D5C_H
#include "game/types.h"
class PetalSilicaEmitterView {
public:
    void* vtbl;
    float gravityScale; // 0x4: vfunc_1 scales downward acceleration by this value.
    float displayScale; // 0x8: vfunc_2 multiplies this by kFloat_0_5_ParticleDisplayScale.
    float boundsMin[3]; // 0xc: vfunc_1 wraps particle coordinates between these bounds.
    float boundsMax[3]; // 0x18: vfunc_1 subtracts boundsMin to obtain wrapping extent.
    void* particles; // 0x24: vfunc_4 allocates count 0x38-byte entries; dtor frees them.
    int particleCount; // 0x28: loop bound in vfunc_1/2 and dtor.
};
#define PARTICLEEMITTERSTATE801B7D5C_OFF(f,o) typedef char ParticleEmitterState801B7D5C_off_##f[((int)&((PetalSilicaEmitterView*)0)->f == (o)) ? 1 : -1]
PARTICLEEMITTERSTATE801B7D5C_OFF(vtbl,0x0);
PARTICLEEMITTERSTATE801B7D5C_OFF(gravityScale,0x4);
PARTICLEEMITTERSTATE801B7D5C_OFF(displayScale,0x8);
PARTICLEEMITTERSTATE801B7D5C_OFF(boundsMin,0xc);
PARTICLEEMITTERSTATE801B7D5C_OFF(boundsMax,0x18);
PARTICLEEMITTERSTATE801B7D5C_OFF(particles,0x24);
PARTICLEEMITTERSTATE801B7D5C_OFF(particleCount,0x28);
#undef PARTICLEEMITTERSTATE801B7D5C_OFF
class PetalSilicaParticle {
public:
    float renderPosition[3]; // 0x0: vfunc_1 assigns position plus oscillation.
    float position[3]; // 0xc: vfunc_1 integrates wind and gravity.
    float unk_18;
    float oscillation[3]; // 0x1c: vfunc_1 computes sinusoidal displacement.
    float azimuth; // 0x28: vfunc_1 uses sin/cos of this angle.
    float phase; // 0x2c: vfunc_1 advances this angle each frame.
    unsigned char visible; // 0x30: vfunc_1 dot-product test; vfunc_2 gates display.
    char unk_31[3];
    void* object; // 0x34: vfunc_1 calls update; vfunc_2 displays; dtor destroys.
};
#define PARTICLEENTRY801B7D5C_OFF(f,o) typedef char ParticleEntry801B7D5C_off_##f[((int)&((PetalSilicaParticle*)0)->f == (o)) ? 1 : -1]
PARTICLEENTRY801B7D5C_OFF(renderPosition,0x0);
PARTICLEENTRY801B7D5C_OFF(position,0xc);
PARTICLEENTRY801B7D5C_OFF(unk_18,0x18);
PARTICLEENTRY801B7D5C_OFF(oscillation,0x1c);
PARTICLEENTRY801B7D5C_OFF(azimuth,0x28);
PARTICLEENTRY801B7D5C_OFF(phase,0x2c);
PARTICLEENTRY801B7D5C_OFF(visible,0x30);
PARTICLEENTRY801B7D5C_OFF(unk_31,0x31);
PARTICLEENTRY801B7D5C_OFF(object,0x34);
#undef PARTICLEENTRY801B7D5C_OFF
#endif

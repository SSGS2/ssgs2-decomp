// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CENERGYCENTER_H
#define CLASSES_CENERGYCENTER_H

#include "game/types.h"

class CEnergyCenter {
public:
    void* vtbl;                         // 0x0: original vtable pointer
    char pad_004[0x14];                 // 0x4
    unsigned char active;               // 0x18: 5 uses in CEnergyCenter_UpdateEmitAndParticles, CEnergyCenter_Ctor, CEnergyCenter_DrawCenterAndParticles...
    char pad_019[0x17];                 // 0x19
    void* vertex0;                       // 0x30: 1 use in CEnergyCenter_Ctor
    void* vertex1;                       // 0x34: 1 use in CEnergyCenter_Ctor
    void* vertex2;                       // 0x38: 1 use in CEnergyCenter_Ctor
    void* vertex3;                       // 0x3c: 1 use in CEnergyCenter_Ctor
    void* vertexListTerminator;                       // 0x40: 1 use in CEnergyCenter_Ctor
    char pad_044[0xe0];                 // 0x44
    void* particles;                      // 0x124: 6 uses in CEnergyCenter_UpdateEmitAndParticles, CEnergyCenter_dtor, CEnergyCenter_Ctor...
    float rotationAngle;                      // 0x128: 2 uses in CEnergyCenter_UpdateEmitAndParticles, CEnergyCenter_Ctor
    int spawnedParticleCount;                        // 0x12c: 6 uses in CEnergyCenter_UpdateEmitAndParticles, CEnergyCenter_DrawCenterAndParticles, CEnergyCenter_ResetEffect
    int particleCapacity;                        // 0x130: 6 uses in CEnergyCenter_UpdateEmitAndParticles, CEnergyCenter_Ctor, CEnergyCenter_dtor
    char pad_134[0x4];                  // 0x134
    float centerY;                      // 0x138: 1 use in CEnergyCenter_UpdateEmitAndParticles
    char pad_13C[0x4];                  // 0x13c
    float centerOpacity;                      // 0x140: 5 uses in CEnergyCenter_UpdateEmitAndParticles, CEnergyCenter_Ctor, CEnergyCenter_DrawCenterAndParticles
    float centerFrame;                      // 0x144: 3 uses in CEnergyCenter_UpdateEmitAndParticles, CEnergyCenter_Ctor, CEnergyCenter_DrawCenterAndParticles
    float unk_148;                      // 0x148: 4 uses in CEnergyCenter_UpdateEmitAndParticles, CEnergyCenter_Ctor
    float unk_14C;                      // 0x14c: 1 use in CEnergyCenter_Ctor
    void* golfBackgroundActor;                      // 0x150: 1 use in CEnergyCenter_Ctor

    // offsets whose address is passed on (sub-objects or arrays, not placed): 0x44, 0x7c, 0xb4, 0xec, 0x134
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define CENERGYCENTER_OFF(f, o) typedef char CEnergyCenter_off_##f[((int)&((CEnergyCenter*)0)->f == (o)) ? 1 : -1]
CENERGYCENTER_OFF(active, 0x18);
CENERGYCENTER_OFF(vertex0, 0x30);
CENERGYCENTER_OFF(vertex1, 0x34);
CENERGYCENTER_OFF(vertex2, 0x38);
CENERGYCENTER_OFF(vertex3, 0x3c);
CENERGYCENTER_OFF(vertexListTerminator, 0x40);
CENERGYCENTER_OFF(particles, 0x124);
CENERGYCENTER_OFF(rotationAngle, 0x128);
CENERGYCENTER_OFF(spawnedParticleCount, 0x12c);
CENERGYCENTER_OFF(particleCapacity, 0x130);
CENERGYCENTER_OFF(centerY, 0x138);
CENERGYCENTER_OFF(centerOpacity, 0x140);
CENERGYCENTER_OFF(centerFrame, 0x144);
CENERGYCENTER_OFF(unk_148, 0x148);
CENERGYCENTER_OFF(unk_14C, 0x14c);
CENERGYCENTER_OFF(golfBackgroundActor, 0x150);
#undef CENERGYCENTER_OFF

#endif  // CLASSES_CENERGYCENTER_H

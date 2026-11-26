// CBunkerParticle: the slow cloud of sand that hangs over a bunker.
//
// The class has no virtual functions here: its vtable comes from the original
// data, so `vtbl` is written out as a member.
#ifndef CLASSES_CBUNKERPARTICLE_H
#define CLASSES_CBUNKERPARTICLE_H

#include "game/types.h"

class CBunkerParticle {
public:
    CBunkerParticle();
    ~CBunkerParticle();

    // Restart the cloud and play the sand sound.
    void RestartEffect();
    // Expand and fade the cloud.
    void DecrementLifeFraction(float delta);
    void DrawSandCloud();

    void* vtbl;                  // 0x0
    char pad_004[4];             // 0x4
    Vec3f position;              // 0x8
    unsigned int bitmapId;       // 0x14
    unsigned char active;        // 0x18
    char pad_019[3];             // 0x19
    void* ownedAuxiliaryObject;  // 0x1c
    void* sprite;                // 0x20
    char pad_024[0xc];           // 0x24
    void* vertex0;               // 0x30
    void* vertex1;               // 0x34
    void* vertex2;               // 0x38
    void* vertex3;               // 0x3c
    int vertexListTerminator;    // 0x40
    char vertexBuffers[4][0x38]; // 0x44: the four quads the pointers above address
    int opacityByte;             // 0x124
    float lifeFraction;          // 0x128  1 at the start, counts down to 0
    char pad_12C[4];             // 0x12c
    float billboardSize;         // 0x130
    float lifeDuration;          // 0x134  seconds for the fraction to run out
};

#define CBUNKERPARTICLE_OFF(f, o) typedef char CBunkerParticle_off_##f[((int)&((CBunkerParticle*)0)->f == (o)) ? 1 : -1]
CBUNKERPARTICLE_OFF(bitmapId, 0x14);
CBUNKERPARTICLE_OFF(active, 0x18);
CBUNKERPARTICLE_OFF(ownedAuxiliaryObject, 0x1c);
CBUNKERPARTICLE_OFF(sprite, 0x20);
CBUNKERPARTICLE_OFF(vertex0, 0x30);
CBUNKERPARTICLE_OFF(vertexBuffers, 0x44);
CBUNKERPARTICLE_OFF(opacityByte, 0x124);
CBUNKERPARTICLE_OFF(lifeFraction, 0x128);
CBUNKERPARTICLE_OFF(billboardSize, 0x130);
CBUNKERPARTICLE_OFF(lifeDuration, 0x134);
#undef CBUNKERPARTICLE_OFF
typedef char CBunkerParticle_size[(sizeof(CBunkerParticle) == 0x138) ? 1 : -1];

#endif // CLASSES_CBUNKERPARTICLE_H

// CGrassParticle: blades of grass torn out of the turf behind a ball.
//
// Each particle is a rotating, fading sprite whose velocity starts near a
// caller-supplied vector. The class has no virtual functions here: its vtable
// comes from the original data, so `vtbl` is written out as a member.
#ifndef CLASSES_CGRASSPARTICLE_H
#define CLASSES_CGRASSPARTICLE_H

#include "game/types.h"

class CGrassParticle {
public:
    CGrassParticle(int count, const void* velocity);
    ~CGrassParticle();

    // Integrate the blades and retire the ones that hit ground or faded out.
    void UpdateBlades(float delta);
    // Draw every live blade.
    void DrawSprites();
    // Rebuild the sprite grid and place the blades at the ball.
    void Begin();

    void* vtbl;                        // 0x0
    char pad_004[0x10];                // 0x4
    unsigned int bitmapId;             // 0x14
    unsigned char active;              // 0x18
    char pad_019[3];                   // 0x19
    void* sprite;                      // 0x1c
    void* ownedAuxiliaryObject;        // 0x20
    char pad_024[0xC];                 // 0x24
    void* vertex0;                     // 0x30
    void* vertex1;                     // 0x34
    void* vertex2;                     // 0x38
    void* vertex3;                     // 0x3c
    unsigned int vertexListTerminator; // 0x40
    char vertexBuffers[4][0x38];       // 0x44: the four quads the pointers above address
    void* particles;                   // 0x124
    int particleCount;                 // 0x128
    float initialVelocity[3];          // 0x12c: the velocity the blades are spread around
};

#define CGRASSPARTICLE_OFF(f, o) typedef char CGrassParticle_off_##f[((int)&((CGrassParticle*)0)->f == (o)) ? 1 : -1]
CGRASSPARTICLE_OFF(bitmapId, 0x14);
CGRASSPARTICLE_OFF(active, 0x18);
CGRASSPARTICLE_OFF(sprite, 0x1c);
CGRASSPARTICLE_OFF(ownedAuxiliaryObject, 0x20);
CGRASSPARTICLE_OFF(vertex0, 0x30);
CGRASSPARTICLE_OFF(vertex1, 0x34);
CGRASSPARTICLE_OFF(vertex2, 0x38);
CGRASSPARTICLE_OFF(vertex3, 0x3c);
CGRASSPARTICLE_OFF(vertexListTerminator, 0x40);
CGRASSPARTICLE_OFF(vertexBuffers, 0x44);
CGRASSPARTICLE_OFF(particles, 0x124);
CGRASSPARTICLE_OFF(particleCount, 0x128);
CGRASSPARTICLE_OFF(initialVelocity, 0x12c);
#undef CGRASSPARTICLE_OFF
typedef char CGrassParticle_size[(sizeof(CGrassParticle) == 0x138) ? 1 : -1];

#endif // CLASSES_CGRASSPARTICLE_H

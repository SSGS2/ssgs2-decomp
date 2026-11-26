// CRipple: the expanding water rings and the droplet spray that go with them.
//
// The class has no virtual functions here: its vtable comes from the original
// data, so `vtbl` is written out as a member.
#ifndef CLASSES_CRIPPLE_H
#define CLASSES_CRIPPLE_H

#include "game/types.h"

class CRipple {
public:
    CRipple();

    // Release owned particles, sprites and buffers; a delete flag above zero also frees the object.
    void* Destroy(int deleting);
    // Reset the ripple delays and opacity, then restart the droplet child.
    void Begin();
    // Start delayed ripples, expand and fade them, and update the droplet child.
    void Update(float delta);
    // Draw the expanding ripple quads, then the droplet child.
    void Draw();

    void* vtbl;                  // 0x0: original vtable pointer
    int remainingRippleCount;    // 0x4
    float positionX;             // 0x8
    float positionY;             // 0xc
    float positionZ;             // 0x10
    int textureId;               // 0x14
    unsigned char active;        // 0x18
    char pad_019[0x3];           // 0x19
    void* ownedObject1;          // 0x1c
    void* ownedObject2;          // 0x20
    char pad_024[0xc];           // 0x24
    void* vertex0;               // 0x30
    void* vertex1;               // 0x34
    void* vertex2;               // 0x38
    void* vertex3;               // 0x3c
    int vertexListTerminator;    // 0x40
    char vertexBuffers[4][0x38]; // 0x44
    void* ripples;               // 0x124: 0x20-byte ripple records
    int rippleCount;             // 0x128
    void* dropletEffect;         // 0x12c
};

#define CRIPPLE_OFF(f, o) typedef char CRipple_off_##f[((int)&((CRipple*)0)->f == (o)) ? 1 : -1]
CRIPPLE_OFF(remainingRippleCount, 0x4);
CRIPPLE_OFF(positionX, 0x8);
CRIPPLE_OFF(positionZ, 0x10);
CRIPPLE_OFF(textureId, 0x14);
CRIPPLE_OFF(active, 0x18);
CRIPPLE_OFF(ownedObject1, 0x1c);
CRIPPLE_OFF(ownedObject2, 0x20);
CRIPPLE_OFF(vertex0, 0x30);
CRIPPLE_OFF(vertexListTerminator, 0x40);
CRIPPLE_OFF(ripples, 0x124);
CRIPPLE_OFF(rippleCount, 0x128);
CRIPPLE_OFF(dropletEffect, 0x12c);
#undef CRIPPLE_OFF

#endif // CLASSES_CRIPPLE_H

// CPowerShot: the ring that expands around the ball while a power shot charges.
//
// The ring is sixteen textured quads in one vertex buffer. The class is declared
// without virtual functions on purpose: its vtable still comes from the original
// game data, so `vtbl` is spelled out as a member.
#ifndef CLASSES_CPOWERSHOT_H
#define CLASSES_CPOWERSHOT_H

#include "game/types.h"

class CPowerShot {
public:
    // Lifetime. The ring starts at the ball, grows and fades over 0.8 seconds.
    void ResetRingAtBall();
    void GrowRingFadeAlpha(float delta);
    void BuildRingAndDraw();

    // Set up the ring buffer. Named Init rather than a constructor: the effect
    // pool allocates raw storage and calls it like a plain function.
    void* Init();

    // Release the owned buffer and sprites. A delete flag above zero also frees
    // the object; either way the address is returned.
    void* Destroy(int flags);

    void* vtbl;                  // 0x00
    char pad_004[4];             // 0x04
    Vec3f position;              // 0x08  ball position, raised when the ring starts
    int textureId;               // 0x14
    unsigned char active;        // 0x18
    char pad_019[3];             // 0x19
    void* ownedObject1;          // 0x1c
    void* ownedObject2;          // 0x20
    char pad_024[0xc];           // 0x24
    void* vertexPointers[4];     // 0x30
    int vertexListTerminator;    // 0x40
    char vertexStorage[4][0x38]; // 0x44  quad data the four pointers above address
    int alpha;                   // 0x124
    float elapsedTime;           // 0x128
    float radius;                // 0x12c
    void* vertexBuffer;          // 0x130
};

#define CPOWERSHOT_OFF(f, o) typedef char CPowerShot_off_##f[((int)&((CPowerShot*)0)->f == (o)) ? 1 : -1]
CPOWERSHOT_OFF(position, 0x08);
CPOWERSHOT_OFF(textureId, 0x14);
CPOWERSHOT_OFF(active, 0x18);
CPOWERSHOT_OFF(ownedObject1, 0x1c);
CPOWERSHOT_OFF(ownedObject2, 0x20);
CPOWERSHOT_OFF(vertexPointers, 0x30);
CPOWERSHOT_OFF(vertexListTerminator, 0x40);
CPOWERSHOT_OFF(vertexStorage, 0x44);
CPOWERSHOT_OFF(alpha, 0x124);
CPOWERSHOT_OFF(elapsedTime, 0x128);
CPOWERSHOT_OFF(radius, 0x12c);
CPOWERSHOT_OFF(vertexBuffer, 0x130);
#undef CPOWERSHOT_OFF
typedef char CPowerShot_size[(sizeof(CPowerShot) == 0x134) ? 1 : -1];

#endif // CLASSES_CPOWERSHOT_H

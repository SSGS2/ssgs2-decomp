// CFragment: a burst of small sprites launched from one point.
//
// Each fragment is a position and velocity record of 0x18 bytes. The class has no
// virtual functions here: its vtable comes from the original data, so `vtbl` is
// written out as a member.
#ifndef CLASSES_CFRAGMENT_H
#define CLASSES_CFRAGMENT_H

#include "game/types.h"
#include "game/wvector.h"

class CFragment {
public:
    CFragment();
    ~CFragment();

    // Launch the fragments from `position` around `launchAxis`.
    void LaunchFragments();
    // Move the fragments under gravity and fade the burst.
    void UpdateMotionAndFade(float delta);
    void DrawFragments();
    // Copy the launch axis used by subsequent fragment resets.
    void SetLaunchAxis(const WVector& source);

    void* vtbl;                  // 0x00
    char pad_004[4];             // 0x04
    Vec3f position;              // 0x08  launch origin, world space
    int bitmapId;                // 0x14
    unsigned char active;        // 0x18
    char pad_019[3];             // 0x19
    void* animatedSprite;        // 0x1c
    void* ownedSprite;           // 0x20
    char pad_024[0xc];           // 0x24
    void* vertex0;               // 0x30
    void* vertex1;               // 0x34
    void* vertex2;               // 0x38
    void* vertex3;               // 0x3c
    int vertexListTerminator;    // 0x40
    char vertexBuffers[4][0x38]; // 0x44: the four quads the pointers above address
    void* fragments;             // 0x124
    int fragmentCount;           // 0x128
    char pad_12C[8];             // 0x12c
    float alpha;                 // 0x134  fades to zero
    Vec3f launchAxis;            // 0x138
};

#define CFRAGMENT_OFF(f, o) typedef char CFragment_off_##f[((int)&((CFragment*)0)->f == (o)) ? 1 : -1]
CFRAGMENT_OFF(bitmapId, 0x14);
CFRAGMENT_OFF(active, 0x18);
CFRAGMENT_OFF(animatedSprite, 0x1c);
CFRAGMENT_OFF(ownedSprite, 0x20);
CFRAGMENT_OFF(vertex0, 0x30);
CFRAGMENT_OFF(vertex1, 0x34);
CFRAGMENT_OFF(vertex2, 0x38);
CFRAGMENT_OFF(vertex3, 0x3c);
CFRAGMENT_OFF(vertexListTerminator, 0x40);
CFRAGMENT_OFF(fragments, 0x124);
CFRAGMENT_OFF(fragmentCount, 0x128);
CFRAGMENT_OFF(alpha, 0x134);
CFRAGMENT_OFF(launchAxis, 0x138);
#undef CFRAGMENT_OFF
typedef char CFragment_size[(sizeof(CFragment) == 0x144) ? 1 : -1];

#endif // CLASSES_CFRAGMENT_H

// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CFRAGMENT_H
#define CLASSES_CFRAGMENT_H

#include "game/types.h"

class CFragment {
public:
    void* vtbl;                         // 0x0: original vtable pointer
    char pad_004[0x10];                 // 0x4
    int bitmapId;                         // 0x14: 2 uses in CFragment_Ctor, CFragment_LaunchFragments
    char active;                        // 0x18: 1 use in CFragment_Ctor
    char pad_019[0x3];                  // 0x19
    int sprite;                         // 0x1c: 4 uses in CFragment_DrawFragments, CFragment_Ctor
    void* unk_20;                       // 0x20: 1 use in CFragment_Ctor
    char pad_024[0xc];                  // 0x24
    void* vertex0;                       // 0x30: 1 use in CFragment_Ctor
    void* vertex1;                       // 0x34: 1 use in CFragment_Ctor
    void* vertex2;                       // 0x38: 1 use in CFragment_Ctor
    void* vertex3;                       // 0x3c: 1 use in CFragment_Ctor
    int vertexListTerminator;                         // 0x40: 1 use in CFragment_Ctor
    char pad_044[0xe0];                 // 0x44
    void* fragments;                      // 0x124: 4 uses in CFragment_Ctor, CFragment_dtor
    int fragmentCount;                        // 0x128: 1 use in CFragment_Ctor

    // offsets whose address is passed on (sub-objects or arrays, not placed): 0x44, 0x7c, 0xb4, 0xec, 0x138
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define CFRAGMENT_OFF(f, o) typedef char CFragment_off_##f[((int)&((CFragment*)0)->f == (o)) ? 1 : -1]
CFRAGMENT_OFF(bitmapId, 0x14);
CFRAGMENT_OFF(active, 0x18);
CFRAGMENT_OFF(sprite, 0x1c);
CFRAGMENT_OFF(unk_20, 0x20);
CFRAGMENT_OFF(vertex0, 0x30);
CFRAGMENT_OFF(vertex1, 0x34);
CFRAGMENT_OFF(vertex2, 0x38);
CFRAGMENT_OFF(vertex3, 0x3c);
CFRAGMENT_OFF(vertexListTerminator, 0x40);
CFRAGMENT_OFF(fragments, 0x124);
CFRAGMENT_OFF(fragmentCount, 0x128);
#undef CFRAGMENT_OFF

#endif  // CLASSES_CFRAGMENT_H

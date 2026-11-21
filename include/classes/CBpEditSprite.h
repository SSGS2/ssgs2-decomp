// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CBPEDITSPRITE_H
#define CLASSES_CBPEDITSPRITE_H

#include "game/types.h"

class CBpEditSprite {
public:
    void* vtbl;                         // 0x0: original vtable pointer
    char pad_004[0x4];                  // 0x4
    unsigned int selectedItemCount;                 // 0x8: 4 uses in CBpEditSprite_dtor, CBpEditSprite_ctor

    // offsets whose address is passed on (sub-objects or arrays, not placed): 0x4
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define CBPEDITSPRITE_OFF(f, o) typedef char CBpEditSprite_off_##f[((int)&((CBpEditSprite*)0)->f == (o)) ? 1 : -1]
CBPEDITSPRITE_OFF(selectedItemCount, 0x8);
#undef CBPEDITSPRITE_OFF

#endif  // CLASSES_CBPEDITSPRITE_H

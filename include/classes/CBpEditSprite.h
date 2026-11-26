#ifndef CLASSES_CBPEDITSPRITE_H
#define CLASSES_CBPEDITSPRITE_H

#include "game/types.h"

class CBpEditSprite {
public:
    void* vtbl;                     // 0x0: original vtable pointer
    char pad_004[0x4];              // 0x4
    unsigned int selectedItemCount; // 0x8

    ~CBpEditSprite();
    CBpEditSprite(void* value);
};

#define CBPEDITSPRITE_OFF(f, o) typedef char CBpEditSprite_off_##f[((int)&((CBpEditSprite*)0)->f == (o)) ? 1 : -1]
CBPEDITSPRITE_OFF(selectedItemCount, 0x8);
#undef CBPEDITSPRITE_OFF

#endif // CLASSES_CBPEDITSPRITE_H

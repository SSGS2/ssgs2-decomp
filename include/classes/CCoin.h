// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CCOIN_H
#define CLASSES_CCOIN_H

#include "game/types.h"

class CCoin {
public:
    void* vtbl;                         // 0x0: original vtable pointer
    char pad_004[0x10];                 // 0x4
    int bitmapId;                         // 0x14: 1 use in CCoin_Ctor
    unsigned char enabled;               // 0x18: 4 uses in CCoin_Ctor, CCoin_DrawCoinsAndSprites, CCoin_SetEnabled...
    char pad_019[0x3];                  // 0x19
    void* sprite;                       // 0x1c: 8 uses in CCoin_dtor, CCoin_DrawCoinsAndSprites, CCoin_Ctor
    void* sparkSprite;                       // 0x20: 3 uses in CCoin_dtor, CCoin_Ctor
    char pad_024[0xc];                  // 0x24
    void* vertex0;                       // 0x30: 1 use in CCoin_Ctor
    void* vertex1;                       // 0x34: 1 use in CCoin_Ctor
    void* vertex2;                       // 0x38: 1 use in CCoin_Ctor
    void* vertex3;                       // 0x3c: 1 use in CCoin_Ctor
    void* vertexListTerminator;                       // 0x40: 1 use in CCoin_Ctor
    char pad_044[0xe0];                 // 0x44
    void* coinEntries;                      // 0x124: 10 uses in CCoin_dtor, CCoin_InitializeLaunchAndSparks, CCoin_Ctor...
    void* spriteEntries;                      // 0x128: 6 uses in CCoin_dtor, CCoin_Ctor, CCoin_DrawCoinsAndSprites...
    int coinCount;                        // 0x12c: 11 uses in CCoin_InitializeLaunchAndSparks, CCoin_SetEnabled, CCoin_dtor...
    int spriteCount;                        // 0x130: 6 uses in CCoin_InitializeLaunchAndSparks, CCoin_Ctor, CCoin_DrawCoinsAndSprites...
    char pad_134[0xc];                  // 0x134
    float fadeDuration;                      // 0x140: 1 use in CCoin_UpdateCoinsAndSprites
    float opacity;                      // 0x144: 4 uses in CCoin_UpdateCoinsAndSprites
    int bounceSoundCount;                        // 0x148: 2 uses in CCoin_UpdateCoinsAndSprites

    // offsets whose address is passed on (sub-objects or arrays, not placed): 0x44, 0x7c, 0xb4, 0xec, 0x134
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define CCOIN_OFF(f, o) typedef char CCoin_off_##f[((int)&((CCoin*)0)->f == (o)) ? 1 : -1]
CCOIN_OFF(bitmapId, 0x14);
CCOIN_OFF(enabled, 0x18);
CCOIN_OFF(sprite, 0x1c);
CCOIN_OFF(sparkSprite, 0x20);
CCOIN_OFF(vertex0, 0x30);
CCOIN_OFF(vertex1, 0x34);
CCOIN_OFF(vertex2, 0x38);
CCOIN_OFF(vertex3, 0x3c);
CCOIN_OFF(vertexListTerminator, 0x40);
CCOIN_OFF(coinEntries, 0x124);
CCOIN_OFF(spriteEntries, 0x128);
CCOIN_OFF(coinCount, 0x12c);
CCOIN_OFF(spriteCount, 0x130);
CCOIN_OFF(fadeDuration, 0x140);
CCOIN_OFF(opacity, 0x144);
CCOIN_OFF(bounceSoundCount, 0x148);
#undef CCOIN_OFF

#endif  // CLASSES_CCOIN_H

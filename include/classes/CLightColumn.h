// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CLIGHTCOLUMN_H
#define CLASSES_CLIGHTCOLUMN_H

#include "game/types.h"

class CLightColumn {
public:
    void* vtbl;                         // 0x0: original vtable pointer
    char pad_004[0x10];                 // 0x4
    int unk_14;                         // 0x14: 1 use in CLightColumn_Ctor
    unsigned char initialized;               // 0x18: 3 uses in CLightColumn_Ctor, CLightColumn_LoadBitmaps, CLightColumn_UpdateWidthScrollAndFade
    char pad_019[0x3];                  // 0x19
    void* ownedObject1;                       // 0x1c: 3 uses in CLightColumn_dtor, CLightColumn_Ctor
    void* ownedObject2;                       // 0x20: 3 uses in CLightColumn_dtor, CLightColumn_Ctor
    char pad_024[0xc];                  // 0x24
    void* vertex0;                       // 0x30: 1 use in CLightColumn_Ctor
    void* vertex1;                       // 0x34: 1 use in CLightColumn_Ctor
    void* vertex2;                       // 0x38: 1 use in CLightColumn_Ctor
    void* vertex3;                       // 0x3c: 1 use in CLightColumn_Ctor
    int vertexListTerminator;                         // 0x40: 1 use in CLightColumn_Ctor
    char pad_044[0xe0];                 // 0x44
    int fadeMode;                        // 0x124: 4 uses in CLightColumn_UpdateWidthScrollAndFade, CLightColumn_LoadBitmaps
    float columnHeight;                // 0x128: 1 use in CLightColumn_LoadBitmaps
    float columnWidthScale;                      // 0x12c: 1 use in CLightColumn_UpdateWidthScrollAndFade
    unsigned char scrollingColumnAlpha;// 0x130: 6 uses in CLightColumn_UpdateWidthScrollAndFade, CLightColumn_LoadBitmaps
    unsigned char frameColumnAlpha;    // 0x131: 5 uses in CLightColumn_UpdateWidthScrollAndFade, CLightColumn_LoadBitmaps
    char pad_132[0x2];                  // 0x132
    void* frameBitmap0;                      // 0x134: 5 uses in CLightColumn_dtor, CLightColumn_Ctor, CLightColumn_LoadBitmaps
    int frameBitmap1;                        // 0x138: 1 use in CLightColumn_Ctor
    int frameBitmap2;                        // 0x13c: 1 use in CLightColumn_Ctor
    int frameBitmap3;                        // 0x140: 1 use in CLightColumn_Ctor
    int frameBitmap4;                        // 0x144: 1 use in CLightColumn_Ctor
    void* scrollingBitmap;                      // 0x148: 5 uses in CLightColumn_dtor, CLightColumn_Ctor, CLightColumn_LoadBitmaps
    float textureScrollV;              // 0x14c: 2 uses in CLightColumn_LoadBitmaps, CLightColumn_UpdateWidthScrollAndFade

    // offsets whose address is passed on (sub-objects or arrays, not placed): 0x44, 0x7c, 0xb4, 0xec
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define CLIGHTCOLUMN_OFF(f, o) typedef char CLightColumn_off_##f[((int)&((CLightColumn*)0)->f == (o)) ? 1 : -1]
CLIGHTCOLUMN_OFF(unk_14, 0x14);
CLIGHTCOLUMN_OFF(initialized, 0x18);
CLIGHTCOLUMN_OFF(ownedObject1, 0x1c);
CLIGHTCOLUMN_OFF(ownedObject2, 0x20);
CLIGHTCOLUMN_OFF(vertex0, 0x30);
CLIGHTCOLUMN_OFF(vertex1, 0x34);
CLIGHTCOLUMN_OFF(vertex2, 0x38);
CLIGHTCOLUMN_OFF(vertex3, 0x3c);
CLIGHTCOLUMN_OFF(vertexListTerminator, 0x40);
CLIGHTCOLUMN_OFF(fadeMode, 0x124);
CLIGHTCOLUMN_OFF(columnHeight, 0x128);
CLIGHTCOLUMN_OFF(columnWidthScale, 0x12c);
CLIGHTCOLUMN_OFF(scrollingColumnAlpha, 0x130);
CLIGHTCOLUMN_OFF(frameColumnAlpha, 0x131);
CLIGHTCOLUMN_OFF(frameBitmap0, 0x134);
CLIGHTCOLUMN_OFF(frameBitmap1, 0x138);
CLIGHTCOLUMN_OFF(frameBitmap2, 0x13c);
CLIGHTCOLUMN_OFF(frameBitmap3, 0x140);
CLIGHTCOLUMN_OFF(frameBitmap4, 0x144);
CLIGHTCOLUMN_OFF(scrollingBitmap, 0x148);
CLIGHTCOLUMN_OFF(textureScrollV, 0x14c);
#undef CLIGHTCOLUMN_OFF

#endif  // CLASSES_CLIGHTCOLUMN_H

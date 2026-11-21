// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CHEADICON_H
#define CLASSES_CHEADICON_H

#include "game/types.h"

class CHeadIcon {
public:
    void* vtbl;                         // 0x0: original vtable pointer
    char pad_004[0xc];                  // 0x4
    unsigned char drawingEnabled;               // 0x10: 1 use in CHeadIcon_ctor
    char pad_011[0x3];                  // 0x11
    unsigned short player0IconType;              // 0x14: 1 use in CHeadIcon_ctor
    char pad_016[0x2];                  // 0x16
    float player0IconElapsed;                       // 0x18: 1 use in CHeadIcon_ctor
    char pad_01C[0x24];                 // 0x1c
    int player0OverlayObject;                         // 0x40: 1 use in CHeadIcon_ctor
    unsigned short player1IconType;              // 0x44: 1 use in CHeadIcon_ctor
    char pad_046[0x2];                  // 0x46
    float player1IconElapsed;                       // 0x48: 1 use in CHeadIcon_ctor
    char pad_04C[0x24];                 // 0x4c
    int player1OverlayObject;                         // 0x70: 1 use in CHeadIcon_ctor
    unsigned short player2IconType;              // 0x74: 1 use in CHeadIcon_ctor
    char pad_076[0x2];                  // 0x76
    float player2IconElapsed;                       // 0x78: 1 use in CHeadIcon_ctor
    char pad_07C[0x24];                 // 0x7c
    int player2OverlayObject;                         // 0xa0: 1 use in CHeadIcon_ctor
    unsigned short player3IconType;              // 0xa4: 1 use in CHeadIcon_ctor
    char pad_0A6[0x2];                  // 0xa6
    float player3IconElapsed;                       // 0xa8: 1 use in CHeadIcon_ctor
    char pad_0AC[0x24];                 // 0xac
    int player3OverlayObject;                         // 0xd0: 1 use in CHeadIcon_ctor
    void* overlay;                       // 0xd4: 4 uses in CHeadIcon_dtor, CHeadIcon_ctor
    char pad_0D8[0x8];                  // 0xd8
    float iconUvWidth;                       // 0xe0: 1 use in CHeadIcon_ctor
    float iconUvHeight;                       // 0xe4: 1 use in CHeadIcon_ctor
    char pad_0E8[0x8];                  // 0xe8
    float iconWidth;                       // 0xf0: 1 use in CHeadIcon_ctor
    float iconHeight;                       // 0xf4: 1 use in CHeadIcon_ctor
    int overlayNodeCount;                         // 0xf8: 3 uses in CHeadIcon_ctor, CHeadIcon_dtor
    void* overlayTreeRoot;                       // 0xfc: 6 uses in CHeadIcon_ctor, CHeadIcon_dtor
    void* firstOverlayNode;                      // 0x100: 4 uses in CHeadIcon_ctor, CHeadIcon_dtor
    void* textures;                      // 0x104: 6 uses in CHeadIcon_ctor, CHeadIcon_dtor

    // offsets whose address is passed on (sub-objects or arrays, not placed): 0xf8, 0xfc
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define CHEADICON_OFF(f, o) typedef char CHeadIcon_off_##f[((int)&((CHeadIcon*)0)->f == (o)) ? 1 : -1]
CHEADICON_OFF(drawingEnabled, 0x10);
CHEADICON_OFF(player0IconType, 0x14);
CHEADICON_OFF(player0IconElapsed, 0x18);
CHEADICON_OFF(player0OverlayObject, 0x40);
CHEADICON_OFF(player1IconType, 0x44);
CHEADICON_OFF(player1IconElapsed, 0x48);
CHEADICON_OFF(player1OverlayObject, 0x70);
CHEADICON_OFF(player2IconType, 0x74);
CHEADICON_OFF(player2IconElapsed, 0x78);
CHEADICON_OFF(player2OverlayObject, 0xa0);
CHEADICON_OFF(player3IconType, 0xa4);
CHEADICON_OFF(player3IconElapsed, 0xa8);
CHEADICON_OFF(player3OverlayObject, 0xd0);
CHEADICON_OFF(overlay, 0xd4);
CHEADICON_OFF(iconUvWidth, 0xe0);
CHEADICON_OFF(iconUvHeight, 0xe4);
CHEADICON_OFF(iconWidth, 0xf0);
CHEADICON_OFF(iconHeight, 0xf4);
CHEADICON_OFF(overlayNodeCount, 0xf8);
CHEADICON_OFF(overlayTreeRoot, 0xfc);
CHEADICON_OFF(firstOverlayNode, 0x100);
CHEADICON_OFF(textures, 0x104);
#undef CHEADICON_OFF

#endif  // CLASSES_CHEADICON_H

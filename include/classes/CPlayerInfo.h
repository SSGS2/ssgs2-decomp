// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CPLAYERINFO_H
#define CLASSES_CPLAYERINFO_H

#include "game/types.h"

class CPlayerInfo {
public:
    void* vtbl;                         // 0x0: original vtable pointer
    char pad_004[0x14];                 // 0x4
    void* bpObject;                       // 0x18: 14 uses in CPlayerInfo_InitializePage, CPlayerInfo_Ctor
    int bpEntryId;                         // 0x1c: 2 uses in CPlayerInfo_Ctor
    void* layout;                       // 0x20: 63 uses in CPlayerInfo_InitializePage, CPlayerInfo_Ctor
    void* numberDraw;                       // 0x24: 4 uses in CPlayerInfo_InitializePage, CPlayerInfo_Ctor
    int textFactory;                         // 0x28: 1 use in CPlayerInfo_Ctor
    void* text;                       // 0x2c: 2 uses in CPlayerInfo_InitializePage
    int unk_30;                         // 0x30: 1 use in CPlayerInfo_Ctor
    int unk_34;                         // 0x34: 1 use in CPlayerInfo_Ctor
    int unk_38;                         // 0x38: 1 use in CPlayerInfo_Ctor
    int unk_3C;                         // 0x3c: 1 use in CPlayerInfo_Ctor
    int unk_40;                         // 0x40: 1 use in CPlayerInfo_Ctor
    int unk_44;                         // 0x44: 1 use in CPlayerInfo_Ctor
    int unk_48;                         // 0x48: 1 use in CPlayerInfo_Ctor
    char pad_04C[0xc];                  // 0x4c
    int levelIconIndex;                         // 0x58: 12 uses in CPlayerInfo_InitializePage
    int page;                         // 0x5c: 1 use in CPlayerInfo_InitializePage
    char pad_060[0x4];                  // 0x60
    int pageInitialized;                         // 0x64: 1 use in CPlayerInfo_InitializePage
    char pad_068[0xc];                  // 0x68
    void* courseTextures;                       // 0x74: 3 uses in CPlayerInfo_Ctor, CPlayerInfo_PollLoadComplete
    void* unavailableCourseTextures;                       // 0x78: 3 uses in CPlayerInfo_Ctor, CPlayerInfo_PollLoadComplete
    void* characterTextures;                       // 0x7c: 3 uses in CPlayerInfo_Ctor, CPlayerInfo_PollLoadComplete
    unsigned char nameStringFlags;               // 0x80: 4 uses in CPlayerInfo_InitializePage (CONFLICT sizes [1, 4])
    char pad_081[0x3];                  // 0x81
    unsigned int nameStringLength;                // 0x84: 2 uses in CPlayerInfo_InitializePage
    char pad_088[0xc];                  // 0x88
    void* padSwingCtrl;                       // 0x94: 1 use in CPlayerInfo_Ctor
    int playerRecord;                         // 0x98: 1 use in CPlayerInfo_Ctor
    void* texturePack;                       // 0x9c: 6 uses in CPlayerInfo_InitializePage, CPlayerInfo_Ctor
    int previousScriptIndex;                         // 0xa0: 2 uses in CPlayerInfo_Ctor, CPlayerInfo_InitializePage
    int pageEntryId;                         // 0xa4: 2 uses in CPlayerInfo_Ctor, CPlayerInfo_InitializePage

    // offsets whose address is passed on (sub-objects or arrays, not placed): 0xc, 0x18, 0x1c, 0x2c, 0x48, 0x80, 0x82, 0xa4
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define CPLAYERINFO_OFF(f, o) typedef char CPlayerInfo_off_##f[((int)&((CPlayerInfo*)0)->f == (o)) ? 1 : -1]
CPLAYERINFO_OFF(bpObject, 0x18);
CPLAYERINFO_OFF(bpEntryId, 0x1c);
CPLAYERINFO_OFF(layout, 0x20);
CPLAYERINFO_OFF(numberDraw, 0x24);
CPLAYERINFO_OFF(textFactory, 0x28);
CPLAYERINFO_OFF(text, 0x2c);
CPLAYERINFO_OFF(unk_30, 0x30);
CPLAYERINFO_OFF(unk_34, 0x34);
CPLAYERINFO_OFF(unk_38, 0x38);
CPLAYERINFO_OFF(unk_3C, 0x3c);
CPLAYERINFO_OFF(unk_40, 0x40);
CPLAYERINFO_OFF(unk_44, 0x44);
CPLAYERINFO_OFF(unk_48, 0x48);
CPLAYERINFO_OFF(levelIconIndex, 0x58);
CPLAYERINFO_OFF(page, 0x5c);
CPLAYERINFO_OFF(pageInitialized, 0x64);
CPLAYERINFO_OFF(courseTextures, 0x74);
CPLAYERINFO_OFF(unavailableCourseTextures, 0x78);
CPLAYERINFO_OFF(characterTextures, 0x7c);
CPLAYERINFO_OFF(nameStringFlags, 0x80);
CPLAYERINFO_OFF(nameStringLength, 0x84);
CPLAYERINFO_OFF(padSwingCtrl, 0x94);
CPLAYERINFO_OFF(playerRecord, 0x98);
CPLAYERINFO_OFF(texturePack, 0x9c);
CPLAYERINFO_OFF(previousScriptIndex, 0xa0);
CPLAYERINFO_OFF(pageEntryId, 0xa4);
#undef CPLAYERINFO_OFF

#endif  // CLASSES_CPLAYERINFO_H

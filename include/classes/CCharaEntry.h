// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CCHARAENTRY_H
#define CLASSES_CCHARAENTRY_H

#include "game/types.h"

class CCharaEntry {
public:
    void* vtbl;                         // 0x0: original vtable pointer
    char pad_004[0x28];                 // 0x4
    void* layout;                       // 0x2c: 4 uses in CCharaEntry_SetupEntryScreen, CCharaEntry_DrawEntryScreen
    void* statBarTransitions;                       // 0x30: 3 uses in CCharaEntry_SetupEntryScreen
    char pad_034[0x4];                  // 0x34
    int selectedPlayerIndex;            // 0x38: CCharaEntry_OpenPlayerShop stores index after opening that player's shop
    char pad_03C[0x8];                  // 0x3c
    void* entryScriptId;                       // 0x44: 2 uses in CCharaEntry_DrawEntryScreen
    void* noEntryScriptId;                       // 0x48: 2 uses in CCharaEntry_DrawEntryScreen
    void* characterStatsScriptId;                       // 0x4c: 2 uses in CCharaEntry_DrawEntryScreen
    char pad_050[0x10];                 // 0x50
    void* shop;                         // 0x60: CPangya2Shop created while shopOpen is set
    unsigned char shopOpen;             // 0x64: CCharaEntry_OpenPlayerShop guards reentry, then sets after shop creation/setup

    // offsets whose address is passed on (sub-objects or arrays, not placed): 0x2c
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define CCHARAENTRY_OFF(f, o) typedef char CCharaEntry_off_##f[((int)&((CCharaEntry*)0)->f == (o)) ? 1 : -1]
CCHARAENTRY_OFF(layout, 0x2c);
CCHARAENTRY_OFF(statBarTransitions, 0x30);
CCHARAENTRY_OFF(entryScriptId, 0x44);
CCHARAENTRY_OFF(noEntryScriptId, 0x48);
CCHARAENTRY_OFF(characterStatsScriptId, 0x4c);
CCHARAENTRY_OFF(shop, 0x60);
CCHARAENTRY_OFF(selectedPlayerIndex, 0x38);
CCHARAENTRY_OFF(shopOpen, 0x64);
#undef CCHARAENTRY_OFF

#endif  // CLASSES_CCHARAENTRY_H

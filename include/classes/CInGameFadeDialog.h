// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CINGAMEFADEDIALOG_H
#define CLASSES_CINGAMEFADEDIALOG_H

#include "game/types.h"

class CInGameFadeDialog {
public:
    void* vtbl;                         // 0x0: original vtable pointer
    char pad_004[0x20];                 // 0x4
    float fadeOpacity;                       // 0x24: 1 use in CInGameFadeDialog_Draw
    char pad_028[0x24];                 // 0x28
    void* bpEntryId;                       // 0x4c: 5 uses in CInGameFadeDialog_Draw, CInGameFadeDialog_StartScriptAndSetGroups, CInGameFadeDialog_CompleteClose
    unsigned int scriptNameStorageFlags;                // 0x50: 1 use in CInGameFadeDialog_StartScriptAndSetGroups
    char pad_054[0x4];                  // 0x54
    void* scriptNameHeap;                       // 0x58: 1 use in CInGameFadeDialog_StartScriptAndSetGroups
    void* groupVisibilityOverrides;                       // 0x5c: 2 uses in CInGameFadeDialog_StartScriptAndSetGroups
    int groupVisibilityOverrideCount;                         // 0x60: 1 use in CInGameFadeDialog_StartScriptAndSetGroups
    char pad_064[0x4];                  // 0x64
    unsigned int textGroupNameStorageFlags;                // 0x68: 1 use in CInGameFadeDialog_Draw
    char pad_06C[0x4];                  // 0x6c
    void* textGroupNameHeap;                       // 0x70: 1 use in CInGameFadeDialog_Draw
    int scriptStarted;                         // 0x74: 4 uses in CInGameFadeDialog_StartScriptAndSetGroups, CInGameFadeDialog_vfunc_14, CInGameFadeDialog_ResetScriptStarted

    // offsets whose address is passed on (sub-objects or arrays, not placed): 0x51, 0x69
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define CINGAMEFADEDIALOG_OFF(f, o) typedef char CInGameFadeDialog_off_##f[((int)&((CInGameFadeDialog*)0)->f == (o)) ? 1 : -1]
CINGAMEFADEDIALOG_OFF(fadeOpacity, 0x24);
CINGAMEFADEDIALOG_OFF(bpEntryId, 0x4c);
CINGAMEFADEDIALOG_OFF(scriptNameStorageFlags, 0x50);
CINGAMEFADEDIALOG_OFF(scriptNameHeap, 0x58);
CINGAMEFADEDIALOG_OFF(groupVisibilityOverrides, 0x5c);
CINGAMEFADEDIALOG_OFF(groupVisibilityOverrideCount, 0x60);
CINGAMEFADEDIALOG_OFF(textGroupNameStorageFlags, 0x68);
CINGAMEFADEDIALOG_OFF(textGroupNameHeap, 0x70);
CINGAMEFADEDIALOG_OFF(scriptStarted, 0x74);
#undef CINGAMEFADEDIALOG_OFF

#endif  // CLASSES_CINGAMEFADEDIALOG_H

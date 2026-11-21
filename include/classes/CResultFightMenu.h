// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CRESULTFIGHTMENU_H
#define CLASSES_CRESULTFIGHTMENU_H

#include "game/types.h"

class CResultFightMenu {
public:
    void* vtbl;                         // 0x0: original vtable pointer
    char pad_004[0x4];                  // 0x4
    int updateEnabled;                          // 0x8: 1 use in CResultFightMenu_UpdateAndTryAdvance
    char pad_00C[0xc];                  // 0xc
    void* resultMode;                       // 0x18: 1 use in CResultFightMenu_Ctor
    void* layout;                       // 0x1c: 9 uses in CResultFightMenu_dtor, CResultFightMenu_Ctor, CResultFightMenu_UpdateAndTryAdvance...
    int scriptEntryId;                         // 0x20: 5 uses in CResultFightMenu_dtor, CResultFightMenu_Ctor
    void* scriptObject;                       // 0x24: 9 uses in CResultFightMenu_Draw, CResultFightMenu_UpdateAndTryAdvance, CResultFightMenu_Ctor...
    void* numberDraw;                       // 0x28: 8 uses in CResultFightMenu_dtor, CResultFightMenu_Ctor, CResultFightMenu_InitPlayerAndScoreNumbers...
    int displayStarted;                         // 0x2c: 4 uses in CResultFightMenu_UpdateAndTryAdvance, CResultFightMenu_Ctor, CResultFightMenu_Draw
    int introScriptComplete;                         // 0x30: 3 uses in CResultFightMenu_UpdateAndTryAdvance, CResultFightMenu_Ctor
    float elapsedTime;                       // 0x34: 3 uses in CResultFightMenu_UpdateAndTryAdvance, CResultFightMenu_Ctor
    unsigned int entryCount;                // 0x38: 2 uses in CResultFightMenu_Ctor, CResultFightMenu_Draw
    char pad_03C[0xd0];                 // 0x3c
    int revealState;                        // 0x10c: 1 use in CResultFightMenu_Ctor
    int revealRowIndex;                        // 0x110: 1 use in CResultFightMenu_Ctor
    int scoreRevealStep;                        // 0x114: 1 use in CResultFightMenu_Ctor
    float revealElapsedTime;                      // 0x118: 1 use in CResultFightMenu_Ctor

    // offsets whose address is passed on (sub-objects or arrays, not placed): 0x20, 0x6c
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define CRESULTFIGHTMENU_OFF(f, o) typedef char CResultFightMenu_off_##f[((int)&((CResultFightMenu*)0)->f == (o)) ? 1 : -1]
CRESULTFIGHTMENU_OFF(updateEnabled, 0x8);
CRESULTFIGHTMENU_OFF(resultMode, 0x18);
CRESULTFIGHTMENU_OFF(layout, 0x1c);
CRESULTFIGHTMENU_OFF(scriptEntryId, 0x20);
CRESULTFIGHTMENU_OFF(scriptObject, 0x24);
CRESULTFIGHTMENU_OFF(numberDraw, 0x28);
CRESULTFIGHTMENU_OFF(displayStarted, 0x2c);
CRESULTFIGHTMENU_OFF(introScriptComplete, 0x30);
CRESULTFIGHTMENU_OFF(elapsedTime, 0x34);
CRESULTFIGHTMENU_OFF(entryCount, 0x38);
CRESULTFIGHTMENU_OFF(revealState, 0x10c);
CRESULTFIGHTMENU_OFF(revealRowIndex, 0x110);
CRESULTFIGHTMENU_OFF(scoreRevealStep, 0x114);
CRESULTFIGHTMENU_OFF(revealElapsedTime, 0x118);
#undef CRESULTFIGHTMENU_OFF

#endif  // CLASSES_CRESULTFIGHTMENU_H

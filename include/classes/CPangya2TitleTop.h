// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CPANGYA2TITLETOP_H
#define CLASSES_CPANGYA2TITLETOP_H

#include "game/types.h"

class CPangya2TitleTop {
public:
    void* vtbl;                         // 0x0: original vtable pointer
    char pad_004[0x14];                 // 0x4
    int selectionComplete;                         // 0x18: 2 uses in CPangya2TitleTop_UpdateTitleSequence, CPangya2TitleTop_Ctor
    int introStarted;                         // 0x1c: 5 uses in CPangya2TitleTop_UpdateTitleSequence, CPangya2TitleTop_Ctor
    int bpEntryId;                         // 0x20: 5 uses in CPangya2TitleTop_dtor, CPangya2TitleTop_Ctor
    char pad_024[0x4];                  // 0x24
    void* bpObject;                       // 0x28: 15 uses in CPangya2TitleTop_UpdateTitleSequence, CPangya2TitleTop_DrawBackgroundAndScene, CPangya2TitleTop_Ctor
    int idleUpdateCount;                         // 0x2c: 5 uses in CPangya2TitleTop_UpdateTitleSequence, CPangya2TitleTop_Ctor
    unsigned char singlePlayerSelected;               // 0x30: 1 use in CPangya2TitleTop_Ctor
    char pad_031[0x3];                  // 0x31
    void* layout;                         // 0x34: 13 uses in CPangya2TitleTop_StartMenuFadeOut, CPangya2TitleTop_dtor, CPangya2TitleTop_PollLoadAndUpdateLayout...
    int texturePack;                         // 0x38: 14 uses in CPangya2TitleTop_StartMenuFadeOut, CPangya2TitleTop_dtor, CPangya2TitleTop_PollLoadAndUpdateLayout...
    int texture;                         // 0x3c: 8 uses in CPangya2TitleTop_dtor, CPangya2TitleTop_Ctor, CPangya2TitleTop_PollLoadComplete...

    // offsets whose address is passed on (sub-objects or arrays, not placed): 0x20
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define CPANGYA2TITLETOP_OFF(f, o) typedef char CPangya2TitleTop_off_##f[((int)&((CPangya2TitleTop*)0)->f == (o)) ? 1 : -1]
CPANGYA2TITLETOP_OFF(selectionComplete, 0x18);
CPANGYA2TITLETOP_OFF(introStarted, 0x1c);
CPANGYA2TITLETOP_OFF(bpEntryId, 0x20);
CPANGYA2TITLETOP_OFF(bpObject, 0x28);
CPANGYA2TITLETOP_OFF(idleUpdateCount, 0x2c);
CPANGYA2TITLETOP_OFF(singlePlayerSelected, 0x30);
CPANGYA2TITLETOP_OFF(layout, 0x34);
CPANGYA2TITLETOP_OFF(texturePack, 0x38);
CPANGYA2TITLETOP_OFF(texture, 0x3c);
#undef CPANGYA2TITLETOP_OFF

#endif  // CLASSES_CPANGYA2TITLETOP_H

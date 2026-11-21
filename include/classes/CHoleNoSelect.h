// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CHOLENOSELECT_H
#define CLASSES_CHOLENOSELECT_H

#include "game/types.h"

class CHoleNoSelect {
public:
    void* vtbl;                         // 0x0: original vtable pointer
    void* layout;                        // 0x4: 4 uses in CHoleNoSelect_dtor, CHoleNoSelect_Ctor
    int scriptObject;                          // 0x8: 2 uses in CHoleNoSelect_Ctor
    int scriptEntryId;                          // 0xc: 4 uses in CHoleNoSelect_dtor, CHoleNoSelect_Ctor
    int selectionDelayFrames;                         // 0x10: 1 use in CHoleNoSelect_Ctor
    int selectedHoleIndex;                         // 0x14: 1 use in CHoleNoSelect_Ctor
    unsigned char backNineDisabled;               // 0x18: 1 use in CHoleNoSelect_Ctor
    unsigned char selectionComplete;               // 0x19: 1 use in CHoleNoSelect_Ctor
    char pad_01A[0x2];                  // 0x1a
    int previousScriptIndex;                         // 0x1c: 1 use in CHoleNoSelect_Ctor
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define CHOLENOSELECT_OFF(f, o) typedef char CHoleNoSelect_off_##f[((int)&((CHoleNoSelect*)0)->f == (o)) ? 1 : -1]
CHOLENOSELECT_OFF(layout, 0x4);
CHOLENOSELECT_OFF(scriptObject, 0x8);
CHOLENOSELECT_OFF(scriptEntryId, 0xc);
CHOLENOSELECT_OFF(selectionDelayFrames, 0x10);
CHOLENOSELECT_OFF(selectedHoleIndex, 0x14);
CHOLENOSELECT_OFF(backNineDisabled, 0x18);
CHOLENOSELECT_OFF(selectionComplete, 0x19);
CHOLENOSELECT_OFF(previousScriptIndex, 0x1c);
#undef CHOLENOSELECT_OFF

#endif  // CLASSES_CHOLENOSELECT_H

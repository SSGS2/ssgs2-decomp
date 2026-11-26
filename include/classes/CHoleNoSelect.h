#ifndef CLASSES_CHOLENOSELECT_H
#define CLASSES_CHOLENOSELECT_H

#include "game/types.h"

class CHoleNoSelect {
public:
    void* vtbl;                      // 0x0: original vtable pointer
    void* layout;                    // 0x4
    int scriptObject;                // 0x8
    int scriptEntryId;               // 0xc
    int selectionDelayFrames;        // 0x10
    int selectedHoleIndex;           // 0x14
    unsigned char backNineDisabled;  // 0x18
    unsigned char selectionComplete; // 0x19
    char pad_01A[0x2];               // 0x1a
    int previousScriptIndex;         // 0x1c

    CHoleNoSelect();
    ~CHoleNoSelect();

    int UpdateLayout();
    int SetEnabled(int enabled);
    int Draw();
    int ProcessInput();
    int Update();
    int GetSelectionResult(int* out);
    int SelectHole(int holeNumber);
    int IsLoaded();
    int InitLayout(signed char disableBackNine);
};

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

#endif // CLASSES_CHOLENOSELECT_H

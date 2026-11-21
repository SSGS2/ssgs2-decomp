// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_COPTIONDIALOG_H
#define CLASSES_COPTIONDIALOG_H

#include "game/types.h"

class COptionDialog {
public:
    void* vtbl;                         // 0x0: original vtable pointer
    char pad_004[0x14];                 // 0x4
    int bpEntryId;                         // 0x18: 7 uses in COptionDialog_dtor, COptionDialog_Ctor, COptionDialog_UpdateAndProcessInput...
    unsigned char controlModeOptionsVisible;               // 0x1c: 7 uses in COptionDialog_Ctor, COptionDialog_UpdateAndProcessInput, COptionDialog_InitializeFromSaveData
    char pad_01D[0x3];                  // 0x1d
    float inputDelay;                       // 0x20: 7 uses in COptionDialog_UpdateAndProcessInput, COptionDialog_Ctor
    char pad_024[0x4e0];                // 0x24
    int streamVolumePercent;                        // 0x504: 6 uses in COptionDialog_UpdateAndProcessInput
    int bufferVolumePercent;                        // 0x508: 6 uses in COptionDialog_UpdateAndProcessInput
    unsigned int optionFlags;               // 0x50c: 8 uses in COptionDialog_UpdateAndProcessInput
    char pad_510[0x33c];                // 0x510
    void* layout;                      // 0x84c: 30 uses in COptionDialog_UpdateAndProcessInput, COptionDialog_InitializeFromSaveData, COptionDialog_Ctor...
    void* saveMenu;                      // 0x850: 7 uses in COptionDialog_UpdateAndProcessInput, COptionDialog_Ctor, COptionDialog_Draw
    void* textureResource;                      // 0x854: 8 uses in COptionDialog_Ctor, COptionDialog_InitializeFromSaveData, COptionDialog_dtor...
    int previousScriptIndex;                        // 0x858: 3 uses in COptionDialog_Ctor, COptionDialog_InitializeFromSaveData, COptionDialog_dtor
    unsigned char closePending;              // 0x85c: 4 uses in COptionDialog_UpdateAndProcessInput, COptionDialog_Ctor, COptionDialog_InitializeFromSaveData

    // offsets whose address is passed on (sub-objects or arrays, not placed): 0x18, 0x24, 0x50c
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define COPTIONDIALOG_OFF(f, o) typedef char COptionDialog_off_##f[((int)&((COptionDialog*)0)->f == (o)) ? 1 : -1]
COPTIONDIALOG_OFF(bpEntryId, 0x18);
COPTIONDIALOG_OFF(controlModeOptionsVisible, 0x1c);
COPTIONDIALOG_OFF(inputDelay, 0x20);
COPTIONDIALOG_OFF(streamVolumePercent, 0x504);
COPTIONDIALOG_OFF(bufferVolumePercent, 0x508);
COPTIONDIALOG_OFF(optionFlags, 0x50c);
COPTIONDIALOG_OFF(layout, 0x84c);
COPTIONDIALOG_OFF(saveMenu, 0x850);
COPTIONDIALOG_OFF(textureResource, 0x854);
COPTIONDIALOG_OFF(previousScriptIndex, 0x858);
COPTIONDIALOG_OFF(closePending, 0x85c);
#undef COPTIONDIALOG_OFF

#endif  // CLASSES_COPTIONDIALOG_H

// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CSHADOWMANAGER_H
#define CLASSES_CSHADOWMANAGER_H

#include "game/types.h"

class CShadowManager {
public:
    void* vtbl;                         // 0x0: original vtable pointer
    char pad_004[0xc];                  // 0x4
    unsigned char selectionFlags;               // 0x10: 1 use in CShadowManager_Ctor
    char pad_011[0x3];                  // 0x11
    void* captureQueue;                       // 0x14: 10 uses in CShadowManager_Ctor, CShadowManager_dtor, fn_801EF4D8_v2...
    float minimumSelectionValue;                       // 0x18: 1 use in CShadowManager_Ctor
    unsigned int entryCount;                // 0x1c: 7 uses in CShadowManager_Ctor, fn_801EF4D8_v2, fn_801EF4D8_old...
    void* listSentinelNext;                       // 0x20: 1 use in CShadowManager_Ctor
    void* listSentinelPrevious;                       // 0x24: 10 uses in CShadowManager_Ctor, fn_801EF4D8_v2, CShadowManager_dtor...
    void* selection;                       // 0x28: 9 uses in CShadowManager_Ctor, CShadowManager_dtor, fn_801EF4D8_v2...
    int projectionDrawArgument;                         // 0x2c: 1 use in CShadowManager_Ctor

    // offsets whose address is passed on (sub-objects or arrays, not placed): 0x1c, 0x20
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define CSHADOWMANAGER_OFF(f, o) typedef char CShadowManager_off_##f[((int)&((CShadowManager*)0)->f == (o)) ? 1 : -1]
CSHADOWMANAGER_OFF(selectionFlags, 0x10);
CSHADOWMANAGER_OFF(captureQueue, 0x14);
CSHADOWMANAGER_OFF(minimumSelectionValue, 0x18);
CSHADOWMANAGER_OFF(entryCount, 0x1c);
CSHADOWMANAGER_OFF(listSentinelNext, 0x20);
CSHADOWMANAGER_OFF(listSentinelPrevious, 0x24);
CSHADOWMANAGER_OFF(selection, 0x28);
CSHADOWMANAGER_OFF(projectionDrawArgument, 0x2c);
#undef CSHADOWMANAGER_OFF

#endif  // CLASSES_CSHADOWMANAGER_H

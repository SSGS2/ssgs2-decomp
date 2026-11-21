// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CBPCTRLHANDLE_H
#define CLASSES_CBPCTRLHANDLE_H

#include "game/types.h"

class CBpCtrlHandle {
public:
    void* entryId;                         // 0x0: integer BP entry ID stored in a pointer-sized ABI word; -1 means none
    int scriptIndex;                          // 0x4: 5 uses in CBpCtrlHandle_SetScriptIndex, CBpCtrlHandle_ReleaseEntry
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define CBPCTRLHANDLE_OFF(f, o) typedef char CBpCtrlHandle_off_##f[((int)&((CBpCtrlHandle*)0)->f == (o)) ? 1 : -1]
CBPCTRLHANDLE_OFF(scriptIndex, 0x4);
#undef CBPCTRLHANDLE_OFF

#endif  // CLASSES_CBPCTRLHANDLE_H

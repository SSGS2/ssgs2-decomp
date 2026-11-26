#ifndef CLASSES_CBPCTRLHANDLE_H
#define CLASSES_CBPCTRLHANDLE_H

#include "game/types.h"

class CBpCtrlHandle {
public:
    // Select a caption script; an index outside the caption range releases the entry.
    void SetScriptIndex(int id);
    // Delete the caption BP entry and reset both indices.
    void ReleaseEntry();

    static int* GetInstance();

    void* entryId;   // 0x0: integer BP entry ID stored in a pointer-sized ABI word; -1 means none
    int scriptIndex; // 0x4
};

#define CBPCTRLHANDLE_OFF(f, o) typedef char CBpCtrlHandle_off_##f[((int)&((CBpCtrlHandle*)0)->f == (o)) ? 1 : -1]
CBPCTRLHANDLE_OFF(scriptIndex, 0x4);
#undef CBPCTRLHANDLE_OFF

#endif // CLASSES_CBPCTRLHANDLE_H

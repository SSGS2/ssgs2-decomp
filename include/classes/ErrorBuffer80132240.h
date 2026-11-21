// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_ERRORBUFFER80132240_H
#define CLASSES_ERRORBUFFER80132240_H

#include "game/types.h"

class ErrorBuffer80132240 {
public:
    void* unk_0;                        // 0x0: dtor_80132240 / dtor_801322A0: buffer passed to appFreeUni
    void* buffer;                       // 0x4: dtor_80132240 / dtor_801322A0: buffer passed to appFreeUni
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define ERRORBUFFER80132240_OFF(f, o) typedef char ErrorBuffer80132240_off_##f[((int)&((ErrorBuffer80132240*)0)->f == (o)) ? 1 : -1]
ERRORBUFFER80132240_OFF(buffer, 0x4);
ERRORBUFFER80132240_OFF(unk_0, 0x0);
#undef ERRORBUFFER80132240_OFF

#endif  // CLASSES_ERRORBUFFER80132240_H

// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CAMBIENTSOUND_H
#define CLASSES_CAMBIENTSOUND_H

#include "game/types.h"

class CAmbientSound {
public:
    void* vtbl;                         // 0x0: original vtable pointer
    int messageType;                          // 0x4: 1 use in CAmbientSound_HandleMsg
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define CAMBIENTSOUND_OFF(f, o) typedef char CAmbientSound_off_##f[((int)&((CAmbientSound*)0)->f == (o)) ? 1 : -1]
CAMBIENTSOUND_OFF(messageType, 0x4);
#undef CAMBIENTSOUND_OFF

#endif  // CLASSES_CAMBIENTSOUND_H

// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CPRELOADAVATARASYNC_H
#define CLASSES_CPRELOADAVATARASYNC_H

#include "game/types.h"

class CPreloadAvatarAsync {
public:
    void* vtbl;                         // 0x0: original vtable pointer
    char pad_004[0xc];                  // 0x4
    int loadComplete;                         // 0x10: 5 uses in CPreloadAvatarAsync_UpdateAndLatchComplete, CPreloadAvatarAsync_InitializeDependencies, CPreloadAvatarAsync_IsLoadComplete...
    void* partsPreload;                       // 0x14: 6 uses in CPreloadAvatarAsync_dtor, CPreloadAvatarAsync_UpdateAndLatchComplete, CPreloadAvatarAsync_ClearAndRestart
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define CPRELOADAVATARASYNC_OFF(f, o) typedef char CPreloadAvatarAsync_off_##f[((int)&((CPreloadAvatarAsync*)0)->f == (o)) ? 1 : -1]
CPRELOADAVATARASYNC_OFF(loadComplete, 0x10);
CPRELOADAVATARASYNC_OFF(partsPreload, 0x14);
#undef CPRELOADAVATARASYNC_OFF

#endif  // CLASSES_CPRELOADAVATARASYNC_H

// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CPRELOADPETASYNC_H
#define CLASSES_CPRELOADPETASYNC_H

#include "game/types.h"

class CPreloadPetAsync {
public:
    void* vtbl;                         // 0x0: original vtable pointer
    char preloadRequests;                         // 0x4: 1 use in CPreloadPetAsync_QueueModelAndShadowFromItemId
    char pad_005[0xb];                  // 0x5
    int loadState;                         // 0x10: 1 use in CPreloadPetAsync_QueueModelAndShadowFromItemId

    // offsets whose address is passed on (sub-objects or arrays, not placed): 0x4
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define CPRELOADPETASYNC_OFF(f, o) typedef char CPreloadPetAsync_off_##f[((int)&((CPreloadPetAsync*)0)->f == (o)) ? 1 : -1]
CPRELOADPETASYNC_OFF(preloadRequests, 0x4);
CPRELOADPETASYNC_OFF(loadState, 0x10);
#undef CPRELOADPETASYNC_OFF

#endif  // CLASSES_CPRELOADPETASYNC_H

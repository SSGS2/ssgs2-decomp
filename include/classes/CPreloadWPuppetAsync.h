// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CPRELOADWPUPPETASYNC_H
#define CLASSES_CPRELOADWPUPPETASYNC_H

#include "game/types.h"

class CPreloadWPuppetAsync {
public:
    CPreloadWPuppetAsync(); // Defined inline in warning_logo_screens.cpp.
    void* vtbl;                         // 0x0: original vtable pointer
    char pad_004[0xc];                  // 0x4
    void* sourceFile;                       // 0x10: 4 uses in CPreloadWPuppet_OpenSourceFile, CPreloadWPuppetAsync_UpdateLoadState
    int loadState;                         // 0x14: 4 uses in CPreloadWPuppetAsync_UpdateLoadState, CPreloadWPuppetAsync_IsResourcesReady
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define CPRELOADWPUPPETASYNC_OFF(f, o) typedef char CPreloadWPuppetAsync_off_##f[((int)&((CPreloadWPuppetAsync*)0)->f == (o)) ? 1 : -1]
CPRELOADWPUPPETASYNC_OFF(sourceFile, 0x10);
CPRELOADWPUPPETASYNC_OFF(loadState, 0x14);
#undef CPRELOADWPUPPETASYNC_OFF

#endif  // CLASSES_CPRELOADWPUPPETASYNC_H

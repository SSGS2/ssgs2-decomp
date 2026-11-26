#ifndef CLASSES_CPRELOADWPUPPETASYNC_H
#define CLASSES_CPRELOADWPUPPETASYNC_H

#include "game/types.h"

class CPreloadWPuppetAsync {
public:
    CPreloadWPuppetAsync(); // Defined inline in warning_logo_screens.cpp.
    void* vtbl;             // 0x0: original vtable pointer
    char pad_004[0xc];      // 0x4
    void* sourceFile;       // 0x10
    int loadState;          // 0x14

    int UpdateLoadState();
    void* CreateFileAsync(void* path);
    void* CreateSprAsync(void* path);
    void* CreateSeqPreload(void* path);
    void* CreateTextureAsync(void* path);
    int IsResourcesReady();
    static void* Create(const char* sourcePath);

    ~CPreloadWPuppetAsync();
};

#define CPRELOADWPUPPETASYNC_OFF(f, o) \
    typedef char CPreloadWPuppetAsync_off_##f[((int)&((CPreloadWPuppetAsync*)0)->f == (o)) ? 1 : -1]
CPRELOADWPUPPETASYNC_OFF(sourceFile, 0x10);
CPRELOADWPUPPETASYNC_OFF(loadState, 0x14);
#undef CPRELOADWPUPPETASYNC_OFF

#endif // CLASSES_CPRELOADWPUPPETASYNC_H

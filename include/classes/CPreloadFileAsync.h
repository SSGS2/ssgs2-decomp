#ifndef CLASSES_CPRELOADFILEASYNC_H
#define CLASSES_CPRELOADFILEASYNC_H

#include "game/types.h"

class CPreloadFileAsync {
public:
    static void* Create(void* arg);

    void* vtbl;         // 0x0: original vtable pointer
    char pad_004[0x10]; // 0x4
    int state;          // 0x14

    int IsComplete();
    static void* OpenResolvedPathAsync(void* unusedSelf, void* resolvedPath);
    int UpdatePreloadState();

    ~CPreloadFileAsync();
};

#define CPRELOADFILEASYNC_OFF(f, o) \
    typedef char CPreloadFileAsync_off_##f[((int)&((CPreloadFileAsync*)0)->f == (o)) ? 1 : -1]
CPRELOADFILEASYNC_OFF(state, 0x14);
#undef CPRELOADFILEASYNC_OFF

#endif // CLASSES_CPRELOADFILEASYNC_H

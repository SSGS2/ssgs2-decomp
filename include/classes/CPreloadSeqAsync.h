#ifndef CLASSES_CPRELOADSEQASYNC_H
#define CLASSES_CPRELOADSEQASYNC_H

#include "game/types.h"

struct Fn8027008CArg;
class CPreloadSeqAsync {
public:
    void* vtbl;         // 0x0: original vtable pointer
    char pad_004[0x10]; // 0x4
    int loadState;      // 0x14

    void* CreateFileAsync(void* arg);
    void* CreateSprAsync(void* arg);
    static int IsComplete(Fn8027008CArg* arg);
    static void* Create(void* arg);
    int UpdateLoadState();
    ~CPreloadSeqAsync();
};

#define CPRELOADSEQASYNC_OFF(f, o) \
    typedef char CPreloadSeqAsync_off_##f[((int)&((CPreloadSeqAsync*)0)->f == (o)) ? 1 : -1]
CPRELOADSEQASYNC_OFF(loadState, 0x14);
#undef CPRELOADSEQASYNC_OFF

#endif // CLASSES_CPRELOADSEQASYNC_H

#ifndef CLASSES_CPRELOADPARTSASYNC_H
#define CLASSES_CPRELOADPARTSASYNC_H

#include "game/types.h"

class CPreloadPartsAsync {
public:
    static void* Create(void* selection, void* alternateModelFlag);

    void* vtbl;        // 0x0: original vtable pointer
    char pad_004[0xc]; // 0x4
    int state;         // 0x10

    int QueueTexturesAndPuppets(void* source, int useAlt);
    int IsComplete();
    int UpdatePartLoaders();

    ~CPreloadPartsAsync();
};

#define CPRELOADPARTSASYNC_OFF(f, o) \
    typedef char CPreloadPartsAsync_off_##f[((int)&((CPreloadPartsAsync*)0)->f == (o)) ? 1 : -1]
CPRELOADPARTSASYNC_OFF(state, 0x10);
#undef CPRELOADPARTSASYNC_OFF

#endif // CLASSES_CPRELOADPARTSASYNC_H

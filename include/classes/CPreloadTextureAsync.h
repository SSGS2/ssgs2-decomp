#ifndef CLASSES_CPRELOADTEXTUREASYNC_H
#define CLASSES_CPRELOADTEXTUREASYNC_H

#include "game/types.h"

// The data layout of CPreloadTextureAsync has not been recovered yet. Until it is, its member functions reach the
// object's data through file-local layouts or raw offsets.
class CPreloadTextureAsync {
public:
    // Create a texture preloader that leaves resource readiness to later polls.
    static void* Create(void* arg);

    void* CreateFilePreload(void* arg);

    int vfunc_1();
    int IsResourceReady();

    ~CPreloadTextureAsync();
};

#endif // CLASSES_CPRELOADTEXTUREASYNC_H

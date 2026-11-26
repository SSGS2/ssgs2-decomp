#ifndef CLASSES_CPRELOADSPRASYNC_H
#define CLASSES_CPRELOADSPRASYNC_H

#include "game/types.h"

// The data layout of CPreloadSprAsync has not been recovered yet. Until it is, its member functions reach the
// object's data through file-local layouts or raw offsets.
struct Fn8027008CArg;
class CPreloadSprAsync {
public:
    void* CreateFileAsync(void* arg);
    void* CreateWPuppetAsync(void* arg);
    void* CreateTextureAsync(void* arg);
    static int IsComplete(Fn8027008CArg* arg);
    static void* Create(void* arg);
    int UpdateLoadState();
    ~CPreloadSprAsync();
};

#endif // CLASSES_CPRELOADSPRASYNC_H

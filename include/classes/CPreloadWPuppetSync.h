#ifndef CLASSES_CPRELOADWPUPPETSYNC_H
#define CLASSES_CPRELOADWPUPPETSYNC_H

#include "game/types.h"

// The data layout of CPreloadWPuppetSync has not been recovered yet. Until it is, its member functions reach the
// object's data through file-local layouts or raw offsets.
struct Fn80273814Alloc;
class CPreloadWPuppetSync {
public:
    static Fn80273814Alloc* Create(void* arg);
    int vfunc_1();
    static int IsResourcesReady();
    void* CreateFileSync(void* arg);
    void* CreateSprSync(void* arg);
    void* CreateSeqSync(void* arg);
    void* CreateTextureSync(void* arg);
    static int OpenFileAndLoadResources(void* arg, void* name);

    ~CPreloadWPuppetSync();
};

#endif // CLASSES_CPRELOADWPUPPETSYNC_H

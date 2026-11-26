#ifndef CLASSES_CPRELOADSPRSYNC_H
#define CLASSES_CPRELOADSPRSYNC_H

#include "game/types.h"

// The data layout of CPreloadSprSync has not been recovered yet. Until it is, its member functions reach the
// object's data through file-local layouts or raw offsets.
class CPreloadSprSync {
public:
    static int UpdateLoadState();
    int vfunc_2();
    static int InitSynchronously(void* a, void* name);

    // Factories for the sprite preloader and its script-requested children.
    static void* Create(const char* sourcePath);
    static void* CreateFileSync(int unusedReceiver, char* arg);
    static void* CreateWPuppetSync(int unusedReceiver, char* arg);
    static void* CreateTextureSync(int unusedReceiver, char* arg);

    ~CPreloadSprSync();
};

#endif // CLASSES_CPRELOADSPRSYNC_H

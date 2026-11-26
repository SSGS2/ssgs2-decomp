#ifndef CLASSES_CPRELOADTEXTURESYNC_H
#define CLASSES_CPRELOADTEXTURESYNC_H

#include "game/types.h"

// The data layout of CPreloadTextureSync has not been recovered yet. Until it is, its member functions reach the
// object's data through file-local layouts or raw offsets.
class CPreloadTextureSync {
public:
    // Create a synchronous preloader; returns 0 (and frees it) if its resource init fails.
    static void* Create(void* arg);
    // Destroy the preloader's texture state; deletes the storage when shouldDelete is positive.
    void* Destroy(int shouldDelete);

    int vfunc_1();
    int vfunc_2();
    int LoadSharedAndWaitReady(const char* name);
    void* CreateFileSync(void* arg);
};

#endif // CLASSES_CPRELOADTEXTURESYNC_H

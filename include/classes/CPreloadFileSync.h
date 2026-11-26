#ifndef CLASSES_CPRELOADFILESYNC_H
#define CLASSES_CPRELOADFILESYNC_H

#include "game/types.h"

// The data layout of CPreloadFileSync has not been recovered yet. Until it is, its member functions reach the
// object's data through file-local layouts or raw offsets.
class CPreloadFileSync {
public:
    static void* Create(void* arg);
    static int UpdatePreloadState();
    static int IsComplete();
    static void* OpenResolvedPathSync(void* unusedSelf, void* resolvedPath);

    ~CPreloadFileSync();
};

#endif // CLASSES_CPRELOADFILESYNC_H

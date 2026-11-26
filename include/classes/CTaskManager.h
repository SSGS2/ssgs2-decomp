#ifndef CLASSES_CTASKMANAGER_H
#define CLASSES_CTASKMANAGER_H

#include "game/types.h"

// The data layout of CTaskManager has not been recovered yet. Until it is, its member functions reach the
// object's data through file-local layouts or raw offsets.
struct CTaskManager {
public:
    CTaskManager();
    ~CTaskManager();
};

#endif // CLASSES_CTASKMANAGER_H

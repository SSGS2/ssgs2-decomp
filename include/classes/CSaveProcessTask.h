#ifndef CLASSES_CSAVEPROCESSTASK_H
#define CLASSES_CSAVEPROCESSTASK_H

#include "game/types.h"

// The data layout of CSaveProcessTask has not been recovered yet. Until it is, its member functions reach the
// object's data through file-local layouts or raw offsets.
class CSaveProcessTask {
public:
    static void* GetRTTI();

    CSaveProcessTask();
    ~CSaveProcessTask();
    void CreateActors();

    // Destructor entry for the secondary interface's vtable: its this pointer is
    // 8 bytes past the object's start.
    void DtorThunk8(int deleting);
};

#endif // CLASSES_CSAVEPROCESSTASK_H

#ifndef CLASSES_CTITLETASK_H
#define CLASSES_CTITLETASK_H

#include "game/types.h"

// The data layout of CTitleTask has not been recovered yet. Until it is, its member functions reach the
// object's data through file-local layouts or raw offsets.
class CTitleTask {
public:
    static void* GetRTTI();

    CTitleTask();
    static void NotifyActorsDestroy(void* a);
    static void StartActorLoading(void* a);
    ~CTitleTask();
    static void CreateActors(void* a);
    void vfunc_1_ThunkMinus8(int forwarded1);
};

#endif // CLASSES_CTITLETASK_H

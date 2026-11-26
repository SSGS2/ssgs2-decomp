#ifndef CLASSES_CPASSWORDTASK_H
#define CLASSES_CPASSWORDTASK_H

#include "game/types.h"

// The data layout of CPasswordTask has not been recovered yet. Until it is, its member functions reach the
// object's data through file-local layouts or raw offsets.
class CPasswordTask {
public:
    static void* GetRTTI();

    static void StartActorLoading(void* forwarded0);
    static void ProcessActors(void* forwarded0, float forwarded1);
    static void DisplayActorsByPriority(void* forwarded0);
    ~CPasswordTask();
    void CreateActors();
    // Adjust the task interface pointer back to the owning task for destruction.
    void DtorThunk8(int deleteFlag);
    CPasswordTask();
};

#endif // CLASSES_CPASSWORDTASK_H

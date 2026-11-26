#ifndef CLASSES_CSHOP2TASK_H
#define CLASSES_CSHOP2TASK_H

#include "game/types.h"

// The data layout of CShop2Task has not been recovered yet. Until it is, its member functions reach the
// object's data through file-local layouts or raw offsets.
class CShop2Task {
public:
    static void* GetRTTI();

    void StartActorLoading();
    void ProcessActors(float timeStep);
    void DisplayActorsByPriority();
    void CreateActors();
    CShop2Task();
    ~CShop2Task();
    void vfunc_1_Thunk8(int deleting); // entry for the base at +0x8
};

#endif // CLASSES_CSHOP2TASK_H

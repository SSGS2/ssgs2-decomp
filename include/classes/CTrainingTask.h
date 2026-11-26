#ifndef CLASSES_CTRAININGTASK_H
#define CLASSES_CTRAININGTASK_H

#include "game/types.h"

class CTrainingTask {
public:
    void* vtbl;            // 0x0: original vtable pointer
    void* secondaryVtable; // 0x4
    void* tertiaryVtable;  // 0x8

    static void* GetRTTI();
    void* DestroyThunkMinus8(int deleting);

    CTrainingTask();
    ~CTrainingTask();
    void CreateActors();
};

#define CTRAININGTASK_OFF(f, o) typedef char CTrainingTask_off_##f[((int)&((CTrainingTask*)0)->f == (o)) ? 1 : -1]
CTRAININGTASK_OFF(secondaryVtable, 0x4);
CTRAININGTASK_OFF(tertiaryVtable, 0x8);
#undef CTRAININGTASK_OFF

#endif // CLASSES_CTRAININGTASK_H

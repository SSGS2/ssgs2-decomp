#ifndef CLASSES_CTOURTASK_H
#define CLASSES_CTOURTASK_H

#include "game/types.h"

class CTourTask {
public:
    CTourTask();

    void* vtbl;            // 0x0: original vtable pointer
    void* secondaryVtable; // 0x4
    void* tertiaryVtable;  // 0x8

    // Destroys the task base; the thunk calls it through the subobject offset.
    void* Destroy(int deleting) __attribute__((noinline));
    // Adjusts the +8 interface pointer back to the task, then destroys it.
    void DtorThunk8(int deleting);

    static void* GetRTTI();

    static void CreateActors(void* task);
    static void StartActorLoading(void* forwarded0);
    static void ProcessActors(void* forwarded0, float forwarded1);
    static void DisplayActorsByPriority(void* forwarded0);
};

#define CTOURTASK_OFF(f, o) typedef char CTourTask_off_##f[((int)&((CTourTask*)0)->f == (o)) ? 1 : -1]
CTOURTASK_OFF(secondaryVtable, 0x4);
CTOURTASK_OFF(tertiaryVtable, 0x8);
#undef CTOURTASK_OFF

#endif // CLASSES_CTOURTASK_H

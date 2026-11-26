#ifndef CLASSES_CROUNDDOLTASK_H
#define CLASSES_CROUNDDOLTASK_H

#include "game/types.h"

class CRoundDolTask {
public:
    void* vtbl;                   // 0x0: original vtable pointer
    unsigned int secondaryVtable; // 0x4
    unsigned int tertiaryVtable;  // 0x8

    static void* GetRTTI();

    CRoundDolTask();
    void CreateActors();
    void DtorThunk8(int deleting);

    static void StartActorLoading(void* forwarded0);
    static void ProcessActors(void* forwarded0, float forwarded1);
    static void DisplayActorsByPriority(void* forwarded0);
    ~CRoundDolTask();
};

#define CROUNDDOLTASK_OFF(f, o) typedef char CRoundDolTask_off_##f[((int)&((CRoundDolTask*)0)->f == (o)) ? 1 : -1]
CROUNDDOLTASK_OFF(secondaryVtable, 0x4);
CROUNDDOLTASK_OFF(tertiaryVtable, 0x8);
#undef CROUNDDOLTASK_OFF

#endif // CLASSES_CROUNDDOLTASK_H

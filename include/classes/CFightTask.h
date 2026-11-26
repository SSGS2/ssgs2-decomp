// CFightTask: the versus-mode task.
//
// A task whose start holds three interface pointers: its own two vtables and
// the interface the task manager uses. The rest of its state belongs to CTask.
#ifndef CLASSES_CFIGHTTASK_H
#define CLASSES_CFIGHTTASK_H

#include "game/types.h"

class CFightTask {
public:
    CFightTask();
    ~CFightTask();

    static void* GetRTTI();
    static void* vfunc_1_ThunkMinus8(void* p, int deleting);

    void CreateMainActor();
    static void InitActors(void* forwarded0);
    static void PreLoadInitActors(void* forwarded0);
    static void ProcessActors(void* forwarded0, float forwarded1);
    static void DisplayActorsByPriority(void* forwarded0);
    static void NotifyActorsDestroy(void* forwarded0);

    void* vtbl;            // 0x0
    void* secondaryVtable; // 0x4
    void* tertiaryVtable;  // 0x8
    char pad_00C[0x39C];   // 0xc
};

#define CFIGHTTASK_OFF(f, o) typedef char CFightTask_off_##f[((int)&((CFightTask*)0)->f == (o)) ? 1 : -1]
CFIGHTTASK_OFF(secondaryVtable, 0x4);
CFIGHTTASK_OFF(tertiaryVtable, 0x8);
#undef CFIGHTTASK_OFF
typedef char CFightTask_size[(sizeof(CFightTask) == 0x3A8) ? 1 : -1];

#endif // CLASSES_CFIGHTTASK_H

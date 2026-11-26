#ifndef CLASSES_CTRAININGMAIN_H
#define CLASSES_CTRAININGMAIN_H

#include "game/types.h"

class CTrainingMain {
public:
    void* vtbl;              // 0x0: original vtable pointer
    char pad_004[0x14];      // 0x4
    void* taskMainVtable;    // 0x18
    void* courseSelect;      // 0x1c
    void* backgroundEntryId; // 0x20
    void* captionEntryId;    // 0x24
    int scriptsStarted;      // 0x28

    static void OnDestroy();
    void OnLoad();

    static void* GetRTTI();
    void OnDisplay();
    void OnProcess(float dt);

    // The task-main secondary interface forwards destruction to the owner.
    void* DestroyThunkMinus24(int deleting);

    CTrainingMain();
    ~CTrainingMain();
};

#define CTRAININGMAIN_OFF(f, o) typedef char CTrainingMain_off_##f[((int)&((CTrainingMain*)0)->f == (o)) ? 1 : -1]
CTRAININGMAIN_OFF(taskMainVtable, 0x18);
CTRAININGMAIN_OFF(courseSelect, 0x1c);
CTRAININGMAIN_OFF(backgroundEntryId, 0x20);
CTRAININGMAIN_OFF(captionEntryId, 0x24);
CTRAININGMAIN_OFF(scriptsStarted, 0x28);
#undef CTRAININGMAIN_OFF

#endif // CLASSES_CTRAININGMAIN_H

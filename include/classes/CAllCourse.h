#ifndef CLASSES_CALLCOURSE_H
#define CLASSES_CALLCOURSE_H

#include "game/types.h"

class CAllCourse {
public:
    void* vtbl;            // 0x0: original vtable pointer
    char pad_004[0xc];     // 0x4
    void* resourceManager; // 0x10

    int LoadRoundHoleRecords(int count);
};

#define CALLCOURSE_OFF(f, o) typedef char CAllCourse_off_##f[((int)&((CAllCourse*)0)->f == (o)) ? 1 : -1]
CALLCOURSE_OFF(resourceManager, 0x10);
#undef CALLCOURSE_OFF

#endif // CLASSES_CALLCOURSE_H

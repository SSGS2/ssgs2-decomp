#ifndef CLASSES_CNOWRELOADING_H
#define CLASSES_CNOWRELOADING_H

#include "game/types.h"

class CNowReloading {
public:
    void* vtbl;   // 0x0: original vtable pointer
    void* vtable; // 0x4

    static int IsPlayerAbsent(void* object);

    ~CNowReloading();
    void Init();
};

#define CNOWRELOADING_OFF(f, o) typedef char CNowReloading_off_##f[((int)&((CNowReloading*)0)->f == (o)) ? 1 : -1]
CNOWRELOADING_OFF(vtable, 0x4);
#undef CNOWRELOADING_OFF

#endif // CLASSES_CNOWRELOADING_H

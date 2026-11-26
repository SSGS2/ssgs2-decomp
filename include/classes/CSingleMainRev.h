#ifndef CLASSES_CSINGLEMAINREV_H
#define CLASSES_CSINGLEMAINREV_H

#include "game/types.h"

class CSingleMainRev {
public:
    void* vtbl;            // 0x0: original vtable pointer
    char pad_004[0x14];    // 0x4
    void* secondaryVtable; // 0x18
    int state;             // 0x1c
    void* childObject;     // 0x20

    static void* OnLoad();
    static void* GetRTTI();
    static void* OnDestroy();
    void OnDisplay();
    void HandleMsg(const void* event);
    void OnProcess(float dt);

    // The secondary interface forwards destruction to the owner.
    void* DestroyThunkMinus24(int deleting);

    CSingleMainRev();
    ~CSingleMainRev();
};

#define CSINGLEMAINREV_OFF(f, o) typedef char CSingleMainRev_off_##f[((int)&((CSingleMainRev*)0)->f == (o)) ? 1 : -1]
CSINGLEMAINREV_OFF(secondaryVtable, 0x18);
CSINGLEMAINREV_OFF(state, 0x1c);
CSINGLEMAINREV_OFF(childObject, 0x20);
#undef CSINGLEMAINREV_OFF

#endif // CLASSES_CSINGLEMAINREV_H

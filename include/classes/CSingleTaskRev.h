#ifndef CLASSES_CSINGLETASKREV_H
#define CLASSES_CSINGLETASKREV_H

#include "game/types.h"

class CSingleTaskRev {
public:
    void* vtbl;            // 0x0: original vtable pointer
    void* secondaryVtable; // 0x4
    void* tertiaryVtable;  // 0x8

    void vfunc_1_ThunkMinus8(int forwarded1);

    static void* GetRTTI();

    ~CSingleTaskRev();
    CSingleTaskRev();
    void CreateMainActor();
};

#define CSINGLETASKREV_OFF(f, o) typedef char CSingleTaskRev_off_##f[((int)&((CSingleTaskRev*)0)->f == (o)) ? 1 : -1]
CSINGLETASKREV_OFF(secondaryVtable, 0x4);
CSINGLETASKREV_OFF(tertiaryVtable, 0x8);
#undef CSINGLETASKREV_OFF

#endif // CLASSES_CSINGLETASKREV_H

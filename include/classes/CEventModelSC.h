#ifndef CLASSES_CEVENTMODELSC_H
#define CLASSES_CEVENTMODELSC_H

#include "game/types.h"

class CEventModelSC {
public:
    void* vtbl;         // 0x0: original vtable pointer
    unsigned int unk_4; // 0x4
    void* unk_8;        // 0x8
};

#define CEVENTMODELSC_OFF(f, o) typedef char CEventModelSC_off_##f[((int)&((CEventModelSC*)0)->f == (o)) ? 1 : -1]
CEVENTMODELSC_OFF(unk_4, 0x4);
CEVENTMODELSC_OFF(unk_8, 0x8);
#undef CEVENTMODELSC_OFF

#endif // CLASSES_CEVENTMODELSC_H

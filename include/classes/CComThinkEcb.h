#ifndef CLASSES_CCOMTHINKECB_H
#define CLASSES_CCOMTHINKECB_H

#include "game/types.h"

class CComThinkEcb {
public:
    void* vtbl;         // 0x0: original vtable pointer
    char pad_004[0x24]; // 0x4
    void* entryTable;   // 0x28
};

#define CCOMTHINKECB_OFF(f, o) typedef char CComThinkEcb_off_##f[((int)&((CComThinkEcb*)0)->f == (o)) ? 1 : -1]
CCOMTHINKECB_OFF(entryTable, 0x28);
#undef CCOMTHINKECB_OFF

#endif // CLASSES_CCOMTHINKECB_H

#ifndef CLASSES_CSERIFUECB_H
#define CLASSES_CSERIFUECB_H

#include "game/types.h"

class CSerifuEcb {
public:
    void* vtbl;              // 0x0: original vtable pointer
    void* table;             // 0x4
    void* groupedTextTables; // 0x8
    int formatType;          // 0xc
};

#define CSERIFUECB_OFF(f, o) typedef char CSerifuEcb_off_##f[((int)&((CSerifuEcb*)0)->f == (o)) ? 1 : -1]
CSERIFUECB_OFF(table, 0x4);
CSERIFUECB_OFF(groupedTextTables, 0x8);
CSERIFUECB_OFF(formatType, 0xc);
#undef CSERIFUECB_OFF

#endif // CLASSES_CSERIFUECB_H

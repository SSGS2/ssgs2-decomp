// CWindInfo: partial layout used by the methods that reference it.
// Non-virtual layout view; unknown meanings retain their offset names.
#ifndef CLASSES_CWINDINFO_H
#define CLASSES_CWINDINFO_H

#include "game/types.h"

class CWindInfo {
public:
    char pad_000[0x10]; // 0x0
    int state; // 0x10: CWindInfo_Ctor
    void* textBuffer; // 0x14: CWindInfo_Ctor
};

#define CWINDINFO_OFF(f, o) typedef char CWindInfo_off_##f[((int)&((CWindInfo*)0)->f == (o)) ? 1 : -1]
CWINDINFO_OFF(state, 0x10);
CWINDINFO_OFF(textBuffer, 0x14);
#undef CWINDINFO_OFF

#endif // CLASSES_CWINDINFO_H

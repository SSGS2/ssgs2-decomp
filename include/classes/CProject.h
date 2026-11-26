#ifndef CLASSES_CPROJECT_H
#define CLASSES_CPROJECT_H

#include "game/types.h"

class CProject {
public:
    void* vtbl;                       // 0x0: original vtable pointer
    char pad_004[0x28];               // 0x4
    unsigned char projectNameTagByte; // 0x2c

    CProject();
};

#define CPROJECT_OFF(f, o) typedef char CProject_off_##f[((int)&((CProject*)0)->f == (o)) ? 1 : -1]
CPROJECT_OFF(projectNameTagByte, 0x2c);
#undef CPROJECT_OFF

#endif // CLASSES_CPROJECT_H

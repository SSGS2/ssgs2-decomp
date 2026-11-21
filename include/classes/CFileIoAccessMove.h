// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CFILEIOACCESSMOVE_H
#define CLASSES_CFILEIOACCESSMOVE_H

#include "game/types.h"

class CFileIoAccessMove {
public:
    void* vtbl;                        // 0x0: 1 use in CFileIoAccessMove_Ctor
    char pad_004[0x8];                  // 0x4
    int state;                          // 0xc: 1 use in CFileIoAccessMove_Ctor
    unsigned int secondPathFlags;                // 0x10: 2 uses in CFileIoAccessMove_Ctor, CFileIoAccessMove_dtor
    int secondPathLength;                         // 0x14: 1 use in CFileIoAccessMove_Ctor
    int secondPathData;                         // 0x18: 2 uses in CFileIoAccessMove_Ctor, CFileIoAccessMove_dtor
    unsigned int firstPathFlags;                // 0x1c: 2 uses in CFileIoAccessMove_Ctor, CFileIoAccessMove_dtor
    int firstPathLength;                         // 0x20: 1 use in CFileIoAccessMove_Ctor
    int firstPathData;                         // 0x24: 2 uses in CFileIoAccessMove_Ctor, CFileIoAccessMove_dtor
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define CFILEIOACCESSMOVE_OFF(f, o) typedef char CFileIoAccessMove_off_##f[((int)&((CFileIoAccessMove*)0)->f == (o)) ? 1 : -1]
CFILEIOACCESSMOVE_OFF(state, 0xc);
CFILEIOACCESSMOVE_OFF(secondPathFlags, 0x10);
CFILEIOACCESSMOVE_OFF(secondPathLength, 0x14);
CFILEIOACCESSMOVE_OFF(secondPathData, 0x18);
CFILEIOACCESSMOVE_OFF(firstPathFlags, 0x1c);
CFILEIOACCESSMOVE_OFF(firstPathLength, 0x20);
CFILEIOACCESSMOVE_OFF(firstPathData, 0x24);
#undef CFILEIOACCESSMOVE_OFF

#endif  // CLASSES_CFILEIOACCESSMOVE_H

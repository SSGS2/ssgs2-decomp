// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CFILEIOACCESSCHECK_H
#define CLASSES_CFILEIOACCESSCHECK_H

#include "game/types.h"

class CFileIoAccessCheck {
public:
    void* vtbl;                         // 0x0: original vtable pointer
    char pad_004[0x8];                  // 0x4
    int checkState;                          // 0xc: 1 use in CFileIoAccessCheck_Ctor
    int checkItems;                         // 0x10: 3 uses in CFileIoAccessCheck_dtor, CFileIoAccessCheck_Ctor
    int checkItemCount;                         // 0x14: 1 use in CFileIoAccessCheck_Ctor
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define CFILEIOACCESSCHECK_OFF(f, o) typedef char CFileIoAccessCheck_off_##f[((int)&((CFileIoAccessCheck*)0)->f == (o)) ? 1 : -1]
CFILEIOACCESSCHECK_OFF(checkState, 0xc);
CFILEIOACCESSCHECK_OFF(checkItems, 0x10);
CFILEIOACCESSCHECK_OFF(checkItemCount, 0x14);
#undef CFILEIOACCESSCHECK_OFF

#endif  // CLASSES_CFILEIOACCESSCHECK_H

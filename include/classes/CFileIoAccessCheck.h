#ifndef CLASSES_CFILEIOACCESSCHECK_H
#define CLASSES_CFILEIOACCESSCHECK_H

#include "game/types.h"
#include "classes/CFileIoAccess.h"

class CFileIoAccessCheck : public CFileIoAccess {
public:
    int checkState;     // 0xc
    int checkItems;     // 0x10
    int checkItemCount; // 0x14

    CFileIoAccessCheck();
    ~CFileIoAccessCheck();

    int vfunc_2();
    static int IsFinished(void* a);
    int Update();
};

#define CFILEIOACCESSCHECK_OFF(f, o) \
    typedef char CFileIoAccessCheck_off_##f[((int)&((CFileIoAccessCheck*)0)->f == (o)) ? 1 : -1]
CFILEIOACCESSCHECK_OFF(checkState, 0xc);
CFILEIOACCESSCHECK_OFF(checkItems, 0x10);
CFILEIOACCESSCHECK_OFF(checkItemCount, 0x14);
#undef CFILEIOACCESSCHECK_OFF

#endif // CLASSES_CFILEIOACCESSCHECK_H

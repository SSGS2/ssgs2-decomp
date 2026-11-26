#ifndef CLASSES_CFILEIOACCESSREADFLASH_H
#define CLASSES_CFILEIOACCESSREADFLASH_H

#include "game/types.h"

// The data layout of CFileIoAccessReadFlash has not been recovered yet. Until it is, its member functions reach the
// object's data through file-local layouts or raw offsets.
class CFileIoAccessReadFlash {
public:
    static void* Create(void* a, void* b);
    static void* CreateFlash(void* a, void* b, void* c);

    ~CFileIoAccessReadFlash();
};

#endif // CLASSES_CFILEIOACCESSREADFLASH_H

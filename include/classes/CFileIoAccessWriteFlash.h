#ifndef CLASSES_CFILEIOACCESSWRITEFLASH_H
#define CLASSES_CFILEIOACCESSWRITEFLASH_H

#include "game/types.h"

// The data layout of CFileIoAccessWriteFlash has not been recovered yet. Until it is, its member functions reach the
// object's data through file-local layouts or raw offsets.
class CFileIoAccessWriteFlash {
public:
    static void* Create(void* a, void* b, void* c);
    static void* createFlash(void* a, void* b, void* c);

    ~CFileIoAccessWriteFlash();
};

#endif // CLASSES_CFILEIOACCESSWRITEFLASH_H

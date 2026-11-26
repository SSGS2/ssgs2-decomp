#ifndef CLASSES_CSAVEFORMFLASH_H
#define CLASSES_CSAVEFORMFLASH_H

#include "game/types.h"

// The data layout of CSaveFormFlash has not been recovered yet. Until it is, its member functions reach the
// object's data through file-local layouts or raw offsets.
class CSaveFormFlash {
public:
    // Build a flash save form with recordCount records; hasMd5 enables the MD5 trailer.
    static CSaveFormFlash* Create(int recordCount, int hasMd5);

    int ValidateOutfitRecords(int size, void* out);
    int CompareRecords(void* src, void* dst);

    ~CSaveFormFlash();
};

#endif // CLASSES_CSAVEFORMFLASH_H

#ifndef CLASSES_CSAVEFORMAPPLICATION_H
#define CLASSES_CSAVEFORMAPPLICATION_H

#include "game/types.h"

// The data layout of CSaveFormApplication has not been recovered yet. Until it is, its member functions reach the
// object's data through file-local layouts or raw offsets.
class CSaveFormApplication {
public:
    static void* Create(int recordCount, int hasMd5);
    static int CompareRecords(void* saveForm, void* leftRecord, void* rightRecord);
    static int ValidateOutfitRecord(void* saveForm, int bufferAddress, void* recordIndex);

    ~CSaveFormApplication();
};

#endif // CLASSES_CSAVEFORMAPPLICATION_H

#ifndef CLASSES_CSAVEFORM_H
#define CLASSES_CSAVEFORM_H

#include "game/types.h"

// The data layout of CSaveForm has not been recovered yet. Until it is, its member functions reach the
// object's data through file-local layouts or raw offsets.
class CSaveForm {
public:
    CSaveForm(int recordSizeArg, int recordCountArg, int hasMd5);

    void* Destroy(int deletingFlag);
};

#endif // CLASSES_CSAVEFORM_H

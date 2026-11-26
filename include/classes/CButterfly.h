#ifndef CLASSES_CBUTTERFLY_H
#define CLASSES_CBUTTERFLY_H

#include "game/types.h"

// The data layout of CButterfly has not been recovered yet. Until it is, its member functions reach the
// object's data through file-local layouts or raw offsets.
class CButterfly {
public:
    // Release CButterfly resources and optionally delete the object.
    void* Dtor(int doDelete);

    // Initialize the butterfly's creature state and its owned resources. Returns the butterfly.
    void* Init(void* arg);

    // Adjust the secondary-interface pointer back to its owner for destruction.
    void DtorThunk4(int deleteFlag);
};

#endif // CLASSES_CBUTTERFLY_H

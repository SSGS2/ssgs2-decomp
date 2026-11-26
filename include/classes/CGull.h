#ifndef CLASSES_CGULL_H
#define CLASSES_CGULL_H

#include "game/types.h"

// The data layout of CGull has not been recovered yet. Until it is, its member functions reach the
// object's data through file-local layouts or raw offsets.
class CGull {
public:
    // Release CGull resources and optionally delete the object.
    void* Dtor(int doDelete);

    // Initialize the gull's creature state and its owned resources. Returns the gull.
    void* Init(void* arg);

    // Adjust the secondary-interface pointer back to its owner for destruction.
    void DtorThunk4(int deleteFlag);
};

#endif // CLASSES_CGULL_H

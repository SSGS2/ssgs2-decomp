#ifndef CLASSES_CFIGHTER_H
#define CLASSES_CFIGHTER_H

#include "game/types.h"

// The data layout of CFighter has not been recovered yet. Until it is, its member functions reach the
// object's data through file-local layouts or raw offsets.
class CFighter {
public:
    // Follow or record waypoints, interpolate height, and bank into horizontal turns.
    void FlyAlongWaypointPath(float dt);

    // Adjust the secondary-interface pointer back to its owner for destruction.
    // Initialize state and owned resources (the original constructor; not a C++ constructor).
    void* Init(const char* arg);

    // Release resources and optionally delete the object.
    void* Dtor(int del);

    void DtorThunk4(int deleteFlag);
};

#endif // CLASSES_CFIGHTER_H

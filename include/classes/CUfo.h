#ifndef CLASSES_CUFO_H
#define CLASSES_CUFO_H

#include "game/types.h"

// The data layout of CUfo has not been recovered yet. Until it is, its member functions reach the
// object's data through file-local layouts or raw offsets.
class CUfo {
public:
    // Initialize CUfo state and owned resources (the original constructor; not a C++ constructor).
    void* Init(const char* arg);

    // Release resources and optionally delete the object.
    void* Dtor(int shouldDelete);

    // Advance UFO animation events, wrap the clip, and run its movement update.
    void UpdateAnimationAndMovement(float dt);

    // Fly along the waypoint path, steer and bank, then move the UFO.
    void FlyAlongWaypoints(float dt);

    // Adjust the secondary-interface pointer back to its owner for destruction.
    void DtorThunk4(int deleteFlag);
};

#endif // CLASSES_CUFO_H

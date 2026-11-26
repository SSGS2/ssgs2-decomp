#ifndef CLASSES_CCRAB_H
#define CLASSES_CCRAB_H

#include "game/types.h"

// The data layout of CCrab has not been recovered yet. Until it is, its member functions reach the
// object's data through file-local layouts or raw offsets.
class CCrab {
public:
    ~CCrab();

    // Place the crab on the ground at the ball-relative spawn point, if the player's eligibility allows it.
    void SpawnNearBall();

    // Initialize the crab's creature state and its owned resources. Returns the crab.
    void* Init(void* arg);

    // Adjust the secondary-interface pointer back to its owner for destruction.
    void DtorThunk4(int deleteFlag);
};

#endif // CLASSES_CCRAB_H

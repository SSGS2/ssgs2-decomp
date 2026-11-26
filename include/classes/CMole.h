#ifndef CLASSES_CMOLE_H
#define CLASSES_CMOLE_H

#include "game/types.h"

// The data layout of CMole has not been recovered yet. Until it is, its member functions reach the
// object's data through file-local layouts or raw offsets.
class CMole {
public:
    ~CMole();

    // Initialize the mole's creature state and its owned resources. Returns the mole.
    void* Init(void* arg);

    // Place the mole on the ground at the ball-relative spawn point, if the ground type allows it.
    void SpawnNearBall();

    // Adjust the secondary-interface pointer back to its owner for destruction.
    void DtorThunk4(int deleteFlag);
};

#endif // CLASSES_CMOLE_H

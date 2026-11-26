#ifndef CLASSES_CDOLPHIN_H
#define CLASSES_CDOLPHIN_H

#include "game/types.h"

// The data layout of CDolphin has not been recovered yet. Until it is, its member functions reach the
// object's data through file-local layouts or raw offsets.
class CDolphin {
public:
    ~CDolphin();

    // Place the dolphin on the ground near the ball once it has landed in lie category 1.
    void TrySpawnNearBall();

    // Initialize the dolphin's creature state and its owned resources. Returns the dolphin.
    void* Init(void* arg);

    // Adjust the secondary-interface pointer back to its owner for destruction.
    void DtorThunk4(int deleteFlag);
};

#endif // CLASSES_CDOLPHIN_H

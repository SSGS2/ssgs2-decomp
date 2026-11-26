#ifndef CLASSES_CHOLECUPINFO_H
#define CLASSES_CHOLECUPINFO_H

#include "game/types.h"

// The data layout of CHoleCupInfo has not been recovered yet. Until it is, its member functions reach the
// object's data through file-local layouts or raw offsets.
class CHoleCupInfo {
public:
    void GetScreenPosition(float* screenX, float* screenY);
    void SnapDistanceOverlay();
    void UpdateDistanceOverlay();
    void UpdateHeightDifference();
    void UpdateScreenPositionAndDraw();

    ~CHoleCupInfo();
};

#endif // CLASSES_CHOLECUPINFO_H

#ifndef CLASSES_CDISTTOHOLEINFO_H
#define CLASSES_CDISTTOHOLEINFO_H

#include "game/types.h"

// The data layout of CDistToHoleInfo has not been recovered yet. Until it is, its member functions reach the
// object's data through file-local layouts or raw offsets.
class CDistToHoleInfo {
public:
    CDistToHoleInfo();
    void SetScreenAnchor();
    void Draw();
    void BeginMoveToPin();
    void Reset();
    void UpdateAnimation(float deltaSeconds);

    ~CDistToHoleInfo();
};

#endif // CLASSES_CDISTTOHOLEINFO_H

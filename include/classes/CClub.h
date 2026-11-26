#ifndef CLASSES_CCLUB_H
#define CLASSES_CCLUB_H

#include "game/types.h"

// The data layout of CClub has not been recovered yet. Until it is, its member functions reach the
// object's data through file-local layouts or raw offsets.
class CClub {
public:
    void Init();

    void Process(float delta);
    void ClubProcess();
    void BarProcess(float delta);
    void AddClub(const char* name, unsigned int groupOrListCount, float distanceYards, float launchDegrees,
                 float launchSpeed, float selectionScale, float selectionBias);

    void SelectBestClubForHole();
    void SelectBestClub(void* targetPosition);
};

#endif // CLASSES_CCLUB_H

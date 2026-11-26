// CDriveInfo: the Driving Contest HUD panel.
//
// It keeps one label overlay per player and the animated display distance for
// the current player. The class is declared without virtual functions on
// purpose: its vtable is still taken from the original game data, so `vtbl` is
// spelled out as a member.
#ifndef CLASSES_CDRIVEINFO_H
#define CLASSES_CDRIVEINFO_H

#include "game/types.h"
#include "classes/CInfoBase.h"

class CDriveInfo : public CInfoBase {
public:
    CDriveInfo();
    ~CDriveInfo();

    void ResetState();
    void SnapshotStartingDistance();
    void UpdateDistanceAnimation(float delta);

    // Whether the panel is waiting for a new stroke.
    int vfunc_7();

    // Draw one distance row per player and highlight the current player.
    void DrawPlayerDistanceRows();

    float startingDistance;       // 0x18
    float previousStrokeDistance; // 0x1C
    float targetStrokeDistance;   // 0x20
    unsigned int opacity;         // 0x24
    int animationState;           // 0x28
    float elapsed;                // 0x2C
    unsigned int holeStrokeCount; // 0x30
    void** playerLabels;          // 0x34  one label overlay per player
};

#define CDRIVEINFO_OFF(f, o) typedef char CDriveInfo_off_##f[((int)&((CDriveInfo*)0)->f == (o)) ? 1 : -1]
CDRIVEINFO_OFF(startingDistance, 0x18);
CDRIVEINFO_OFF(previousStrokeDistance, 0x1C);
CDRIVEINFO_OFF(targetStrokeDistance, 0x20);
CDRIVEINFO_OFF(opacity, 0x24);
CDRIVEINFO_OFF(animationState, 0x28);
CDRIVEINFO_OFF(elapsed, 0x2C);
CDRIVEINFO_OFF(holeStrokeCount, 0x30);
CDRIVEINFO_OFF(playerLabels, 0x34);
#undef CDRIVEINFO_OFF
typedef char CDriveInfo_size[(sizeof(CDriveInfo) == 0x38) ? 1 : -1];

#endif // CLASSES_CDRIVEINFO_H

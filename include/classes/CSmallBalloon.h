// CSmallBalloon: the smallest party balloon. It holds two scale endpoints for
// its hit sequence and restores that sequence with a recovery sound.
//
// All three balloon sizes share one object layout, declared in
// BalloonState80069C.h; this class adds only the small balloon's behaviour.
#ifndef CLASSES_CSMALLBALLOON_H
#define CLASSES_CSMALLBALLOON_H

#include "game/types.h"
#include "classes/BalloonState80069C.h"

class CSmallBalloon : public BalloonState80069C {
public:
    // Construct in place from a borrowed spawn origin; returns the balloon.
    CSmallBalloon* Init(void* spawnPosition, int characterModel);
    ~CSmallBalloon();

    void RestoreScaleSequence();
    void StartExplosionEffect();

    // Small balloons have no extra per-frame update hook.
    void vfunc_3();
};

#define CSMALLBALLOON_OFF(f, o) typedef char CSmallBalloon_off_##f[((int)&((CSmallBalloon*)0)->f == (o)) ? 1 : -1]
CSMALLBALLOON_OFF(vtbl, 0x0);
CSMALLBALLOON_OFF(scaleValues, 0x24);
CSMALLBALLOON_OFF(scale, 0x70);
CSMALLBALLOON_OFF(position, 0x7C);
CSMALLBALLOON_OFF(effectPosition, 0xBC);
CSMALLBALLOON_OFF(sizeClass, 0xA0);
CSMALLBALLOON_OFF(model, 0xD0);
CSMALLBALLOON_OFF(baseScale, 0xD4);
#undef CSMALLBALLOON_OFF
typedef char CSmallBalloon_size[(sizeof(CSmallBalloon) == 0xF0) ? 1 : -1];

#endif // CLASSES_CSMALLBALLOON_H

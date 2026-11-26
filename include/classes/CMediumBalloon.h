// CMediumBalloon: the middle-sized party balloon. It holds three scale
// endpoints, chooses a small or medium explosion from its base radius, and
// restores its scale sequence when recovered.
//
// All three balloon sizes share one object layout, declared in
// BalloonState80069C.h; this class adds only the medium balloon's behaviour.
#ifndef CLASSES_CMEDIUMBALLOON_H
#define CLASSES_CMEDIUMBALLOON_H

#include "game/types.h"
#include "classes/BalloonState80069C.h"

class CMediumBalloon : public BalloonState80069C {
public:
    // Construct in place from a borrowed spawn origin; returns the balloon.
    CMediumBalloon* Init(void* spawnPosition, int characterModel);
    ~CMediumBalloon();

    void RestoreScaleSequence();
    void StartExplosionEffect();

    // Medium balloons have no extra per-frame update hook.
    void vfunc_3();
};

#define CMEDIUMBALLOON_OFF(f, o) typedef char CMediumBalloon_off_##f[((int)&((CMediumBalloon*)0)->f == (o)) ? 1 : -1]
CMEDIUMBALLOON_OFF(vtbl, 0x0);
CMEDIUMBALLOON_OFF(scaleValues, 0x24);
CMEDIUMBALLOON_OFF(scale, 0x70);
CMEDIUMBALLOON_OFF(position, 0x7C);
CMEDIUMBALLOON_OFF(sizeClass, 0xA0);
CMEDIUMBALLOON_OFF(effectPosition, 0xBC);
CMEDIUMBALLOON_OFF(model, 0xD0);
CMEDIUMBALLOON_OFF(baseScale, 0xD4);
#undef CMEDIUMBALLOON_OFF
typedef char CMediumBalloon_size[(sizeof(CMediumBalloon) == 0xF0) ? 1 : -1];

#endif // CLASSES_CMEDIUMBALLOON_H

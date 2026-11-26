// CBigBalloon: the largest party balloon. It holds four scale endpoints and
// restores its scale sequence when recovered.
//
// All three balloon sizes share one object layout, declared in
// BalloonState80069C.h; this class adds only the large balloon's behaviour.
#ifndef CLASSES_CBIGBALLOON_H
#define CLASSES_CBIGBALLOON_H

#include "game/types.h"
#include "classes/BalloonState80069C.h"

class CBigBalloon : public BalloonState80069C {
public:
    // Construct in place from a borrowed spawn origin; returns the balloon.
    CBigBalloon* Init(void* spawnPosition, int characterModel);
    ~CBigBalloon();

    void ResetScaleSequence();
    void StartExplosionEffect();

    // Large balloons have no extra per-frame update hook.
    void vfunc_3();
};

#define CBIGBALLOON_OFF(f, o) typedef char CBigBalloon_off_##f[((int)&((CBigBalloon*)0)->f == (o)) ? 1 : -1]
CBIGBALLOON_OFF(vtbl, 0x0);
CBIGBALLOON_OFF(scaleValues, 0x24);
CBIGBALLOON_OFF(scale, 0x70);
CBIGBALLOON_OFF(position, 0x7C);
CBIGBALLOON_OFF(sizeClass, 0xA0);
CBIGBALLOON_OFF(effectPosition, 0xBC);
CBIGBALLOON_OFF(model, 0xD0);
CBIGBALLOON_OFF(baseScale, 0xD4);
CBIGBALLOON_OFF(targetScale, 0xEC);
CBIGBALLOON_OFF(scaleDelta, 0xEE);
#undef CBIGBALLOON_OFF
typedef char CBigBalloon_size[(sizeof(CBigBalloon) == 0xF0) ? 1 : -1];

#endif // CLASSES_CBIGBALLOON_H

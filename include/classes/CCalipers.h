// CCalipers: the distance-measuring calipers on the shot gauge.
//
// The panel owns the two gauge overlays and keeps the pointer position the
// player locked in. It reads the pointer each frame and writes the selected
// gauge position into the golf document. The class is declared without virtual
// functions on purpose: its vtable is still taken from the original game data,
// so `vtbl` is spelled out as a member.
#ifndef CLASSES_CCALIPERS_H
#define CLASSES_CCALIPERS_H

#include "game/types.h"
#include "classes/CInfoBase.h"
#include "classes/HudTextPanel.h"

class CCalipers : public CInfoBase {
public:
    CCalipers();
    ~CCalipers();

    void UpdateGaugeMeasurement();
    void DrawDistanceGauge();
    void ResetMeasurement();
    unsigned int IsPreviewActive();

    // Load the panel's font through the HUD text panel vtable.
    void SetFont(const char* path, int a, int b) {
        ((HudTextPanelVTable*)this)->loadFont(path, a, b);
    }

    void* indicatorOverlay;       // 0x18  from the "indicator" resource
    void* calipersOverlay;        // 0x1C  from the "calipers" resource, drawn at the gauge
    float lockedGaugeX;           // 0x20
    unsigned int isLocked;        // 0x24
    unsigned int isPreviewActive; // 0x28
};

#define CCALIPERS_OFF(f, o) typedef char CCalipers_off_##f[((int)&((CCalipers*)0)->f == (o)) ? 1 : -1]
CCALIPERS_OFF(indicatorOverlay, 0x18);
CCALIPERS_OFF(calipersOverlay, 0x1C);
CCALIPERS_OFF(lockedGaugeX, 0x20);
CCALIPERS_OFF(isLocked, 0x24);
CCALIPERS_OFF(isPreviewActive, 0x28);
#undef CCALIPERS_OFF
typedef char CCalipers_size[(sizeof(CCalipers) == 0x2C) ? 1 : -1];

#endif // CLASSES_CCALIPERS_H

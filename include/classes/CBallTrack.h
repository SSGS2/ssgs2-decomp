// CBallTrack: the ribbon drawn through the ball's history samples.
//
// Each sample contributes a left and a right edge, pushed apart so that the
// ribbon faces the camera. The edge array and the bitmap are owned by the
// object; the shared trace opening (vtable and quad renderer) is spelled out
// here rather than inherited, because the vtable comes from the original data.
#ifndef CLASSES_CBALLTRACK_H
#define CLASSES_CBALLTRACK_H

#include "game/types.h"

class CBallTrack {
public:
    ~CBallTrack();

    void InitTrailPoints();
    void UpdateTrailAndFade(float delta);
    void Draw();
    void DefaultHook();

    void* vtbl;              // 0x00
    char pad_004[0x4];       // 0x04
    void* bitmapResourceId;  // 0x08
    char pad_00C[0x10];      // 0x0C
    unsigned char active;    // 0x1C
    char pad_01D[0xF7];      // 0x1D
    void* quadRenderer;      // 0x114
    void* trailPoints;       // 0x118  pairs of world-space edges, 0x18 bytes each
    int trailPointCount;     // 0x11C
    float opacity;           // 0x120
    float remainingLifetime; // 0x124
    float fadeDuration;      // 0x128
    float initialOpacity;    // 0x12C
    int historyIndex;        // 0x130
};

#define CBALLTRACK_OFF(f, o) typedef char CBallTrack_off_##f[((int)&((CBallTrack*)0)->f == (o)) ? 1 : -1]
CBALLTRACK_OFF(bitmapResourceId, 0x8);
CBALLTRACK_OFF(active, 0x1C);
CBALLTRACK_OFF(quadRenderer, 0x114);
CBALLTRACK_OFF(trailPoints, 0x118);
CBALLTRACK_OFF(trailPointCount, 0x11C);
CBALLTRACK_OFF(opacity, 0x120);
CBALLTRACK_OFF(remainingLifetime, 0x124);
CBALLTRACK_OFF(fadeDuration, 0x128);
CBALLTRACK_OFF(initialOpacity, 0x12C);
CBALLTRACK_OFF(historyIndex, 0x130);
#undef CBALLTRACK_OFF
typedef char CBallTrack_size[(sizeof(CBallTrack) == 0x134) ? 1 : -1];

#endif // CLASSES_CBALLTRACK_H

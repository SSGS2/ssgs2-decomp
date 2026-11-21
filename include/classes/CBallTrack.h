// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CBALLTRACK_H
#define CLASSES_CBALLTRACK_H

#include "game/types.h"

class CBallTrack {
public:
    void* vtbl;                         // 0x0: original vtable pointer
    char pad_004[0x4];                  // 0x4
    void* bitmapResourceId;                        // 0x8: 4 uses in CBallTrack_Destroy, CBallTrack_Draw
    char pad_00C[0x10];                 // 0xc
    unsigned char active;               // 0x1c: 4 uses in CBallTrack_UpdateTrailAndFade, CBallTrack_InitTrailPoints, CBallTrack_Draw
    char pad_01D[0xf7];                 // 0x1d
    void* quadRenderer;                      // 0x114: 9 uses in CBallTrack_Draw, CBallTrack_Destroy, CBallWindTrack_dtor...
    void* trailPoints;                      // 0x118: 15 uses in CBallTrack_InitTrailPoints, CBallTrack_UpdateTrailAndFade, CBallTrack_Destroy...
    int trailPointCount;                        // 0x11c: 9 uses in CBallTrack_UpdateTrailAndFade, CBallWindTrack_dtor, CBallTrack_InitTrailPoints...
    float opacity;                      // 0x120: 5 uses in CBallTrack_UpdateTrailAndFade, CBallTrack_InitTrailPoints, CBallTrack_Draw
    float remainingLifetime;                      // 0x124: 5 uses in CBallTrack_UpdateTrailAndFade, CBallWindTrack_dtor, CBallTrack_InitTrailPoints
    float fadeDuration;                      // 0x128: 3 uses in CBallTrack_InitTrailPoints, CBallTrack_UpdateTrailAndFade
    float initialOpacity;                      // 0x12c: 3 uses in CBallTrack_UpdateTrailAndFade, CBallTrack_InitTrailPoints
    int historyIndex;                        // 0x130: 7 uses in CBallTrack_UpdateTrailAndFade, CBallTrack_InitTrailPoints, CBallTrack_Draw
    char pad_134[0xfc];                 // 0x134
    void* unk_230;                      // 0x230: 2 uses in CBallWindTrack_dtor
    void* unk_234;                      // 0x234: 2 uses in CBallWindTrack_dtor

    // offsets whose address is passed on (sub-objects or arrays, not placed): 0x11c
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define CBALLTRACK_OFF(f, o) typedef char CBallTrack_off_##f[((int)&((CBallTrack*)0)->f == (o)) ? 1 : -1]
CBALLTRACK_OFF(bitmapResourceId, 0x8);
CBALLTRACK_OFF(active, 0x1c);
CBALLTRACK_OFF(quadRenderer, 0x114);
CBALLTRACK_OFF(trailPoints, 0x118);
CBALLTRACK_OFF(trailPointCount, 0x11c);
CBALLTRACK_OFF(opacity, 0x120);
CBALLTRACK_OFF(remainingLifetime, 0x124);
CBALLTRACK_OFF(fadeDuration, 0x128);
CBALLTRACK_OFF(initialOpacity, 0x12c);
CBALLTRACK_OFF(historyIndex, 0x130);
CBALLTRACK_OFF(unk_230, 0x230);
CBALLTRACK_OFF(unk_234, 0x234);
#undef CBALLTRACK_OFF

#endif  // CLASSES_CBALLTRACK_H

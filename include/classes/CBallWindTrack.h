// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CBALLWINDTRACK_H
#define CLASSES_CBALLWINDTRACK_H

#include "game/types.h"

class CBallWindTrack {
public:
    void* vtbl;                         // 0x0: original vtable pointer
    char pad_004[0x18];                 // 0x4
    unsigned char active;               // 0x1c: 2 uses in CBallWindTrack_UpdateCameraStreaks
    char pad_01D[0xfb];                 // 0x1d
    void* particles;                      // 0x118: 14 uses in CBallWindTrack_UpdateCameraStreaks
    void* ballTrackVtable;                      // 0x11c: 1 use in CBallWindTrack_dtor
    char pad_120[0x130];                // 0x120
    float opacityScale;                      // 0x250: 3 uses in CBallWindTrack_DrawRibbonAndStreaks, CBallWindTrack_UpdateCameraStreaks

    float previousCameraPosition[3];   // 0x254: CBallWindTrack_UpdateCameraStreaks copies s_cameraTransform + 0x24
    float cameraMotionVector[3];            // 0x260: camera displacement; normalized only when length >= 1 world unit

    // offsets whose address is passed on (sub-objects or arrays, not placed): 0x11c, 0x254, 0x260
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define CBALLWINDTRACK_OFF(f, o) typedef char CBallWindTrack_off_##f[((int)&((CBallWindTrack*)0)->f == (o)) ? 1 : -1]
CBALLWINDTRACK_OFF(active, 0x1c);
CBALLWINDTRACK_OFF(particles, 0x118);
CBALLWINDTRACK_OFF(ballTrackVtable, 0x11c);
CBALLWINDTRACK_OFF(opacityScale, 0x250);
CBALLWINDTRACK_OFF(previousCameraPosition, 0x254);
CBALLWINDTRACK_OFF(cameraMotionVector, 0x260);
#undef CBALLWINDTRACK_OFF

#endif  // CLASSES_CBALLWINDTRACK_H

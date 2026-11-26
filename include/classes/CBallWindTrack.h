// CBallWindTrack: the wind streaks that follow the ball, plus the ball ribbon
// they travel with.
//
// The streaks are a fixed array of 256 particles; the ribbon is a CBallTrack
// stored in place at 0x11C and uses the same edge and bitmap handling as the
// standalone ribbon.
#ifndef CLASSES_CBALLWINDTRACK_H
#define CLASSES_CBALLWINDTRACK_H

#include "game/types.h"
#include "game/wvector.h"

// One streak: a start point and an end point, with the opacity and length the
// streak was drawn with.
struct WindTrailParticle {
    WVector start;
    WVector end;
    int opacity;
    float length;
    unsigned char active;
    char pad_21[3];
};

// The ribbon's storage. CBallTrack is embedded here but held as raw bytes so
// that the enclosing destructor can release it in place, as the original does.
struct BallTrackStorage {
    char bytes[0x134];
};

class CBallWindTrack {
public:
    ~CBallWindTrack();

    void StartAndResetParticles();
    void UpdateCameraStreaks();
    void DrawRibbonAndStreaks();

    void* vtbl;                     // 0x00
    char pad_004[0x18];             // 0x04
    unsigned char active;           // 0x1C
    char pad_01D[0xF7];             // 0x1D
    void* quadRenderer;             // 0x114
    WindTrailParticle* particles;   // 0x118
    BallTrackStorage ballTrack;     // 0x11C  embedded ribbon, not a separate allocation
    float opacityScale;             // 0x250
    WVector previousCameraPosition; // 0x254
    WVector cameraMotionVector;     // 0x260  camera displacement for this update
};

#define CBALLWINDTRACK_OFF(f, o) typedef char CBallWindTrack_off_##f[((int)&((CBallWindTrack*)0)->f == (o)) ? 1 : -1]
CBALLWINDTRACK_OFF(active, 0x1C);
CBALLWINDTRACK_OFF(quadRenderer, 0x114);
CBALLWINDTRACK_OFF(particles, 0x118);
CBALLWINDTRACK_OFF(ballTrack, 0x11C);
CBALLWINDTRACK_OFF(opacityScale, 0x250);
CBALLWINDTRACK_OFF(previousCameraPosition, 0x254);
CBALLWINDTRACK_OFF(cameraMotionVector, 0x260);
#undef CBALLWINDTRACK_OFF
typedef char CBallWindTrack_size[(sizeof(CBallWindTrack) == 0x26C) ? 1 : -1];
typedef char WindTrailParticle_size[(sizeof(WindTrailParticle) == 0x24) ? 1 : -1];

#endif // CLASSES_CBALLWINDTRACK_H

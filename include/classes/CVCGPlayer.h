// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CVCGPLAYER_H
#define CLASSES_CVCGPLAYER_H

#include "game/types.h"

class CVCGPlayer {
public:
    void* vtbl;                         // 0x0: original vtable pointer
    char pad_004[0x1c];                 // 0x4
    int groupIndex;                     // 0x20: group selected for motion interpolation
    float playbackTime;                 // 0x24: advances until group end, wraps in loop mode
    int stopped;                       // 0x28: suppresses playback advancement when nonzero
    int loopMode;                       // 0x2c: CVCGPlayer_StartGroupPlayback stores mode; CVCGPlayer_AdvancePlayback resets time for mode 1 at the group limit
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define CVCGPLAYER_OFF(f, o) typedef char CVCGPlayer_off_##f[((int)&((CVCGPlayer*)0)->f == (o)) ? 1 : -1]
CVCGPLAYER_OFF(groupIndex, 0x20);
CVCGPLAYER_OFF(playbackTime, 0x24);
CVCGPLAYER_OFF(stopped, 0x28);
CVCGPLAYER_OFF(loopMode, 0x2c);
#undef CVCGPLAYER_OFF

#endif  // CLASSES_CVCGPLAYER_H

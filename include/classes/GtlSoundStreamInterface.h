// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_GTLSOUNDSTREAMINTERFACE_H
#define CLASSES_GTLSOUNDSTREAMINTERFACE_H

#include "game/types.h"

class GtlSoundStreamInterface {
public:
    char pad_000[0x4];                  // 0x0: uninterpreted prefix
    void* vtbl;                         // 0x4: gtl_ISoundStream_dtor (0x80403438)
    void* state;                        // 0x8: gtl_ISoundStream_UpdateParameters (0x804035E0), gtl_ISoundStream_UpdateModulation (0x804038C8), gtl_ISoundStream_dtor (0x80403438)
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define GTLSOUNDSTREAMINTERFACE_OFF(f, o) typedef char GtlSoundStreamInterface_off_##f[((int)&((GtlSoundStreamInterface*)0)->f == (o)) ? 1 : -1]
GTLSOUNDSTREAMINTERFACE_OFF(vtbl, 0x4);
GTLSOUNDSTREAMINTERFACE_OFF(state, 0x8);
#undef GTLSOUNDSTREAMINTERFACE_OFF

#endif  // CLASSES_GTLSOUNDSTREAMINTERFACE_H

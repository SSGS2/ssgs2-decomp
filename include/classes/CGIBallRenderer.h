// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CGIBALLRENDERER_H
#define CLASSES_CGIBALLRENDERER_H

#include "game/types.h"

class CGIBallRenderer {
public:
    void* vtbl;                         // 0x0: original vtable pointer
    char pad_004[0x18];                 // 0x4
    void* ballResource;                       // 0x1c: 4 uses in CGIBallRenderer_RenderPreview
    char pad_020[0x188];                // 0x20
    void* ringTextureResource;                      // 0x1a8: 2 uses in ScreenBallPreview_Ctor
    void* percentageTextureResource;                      // 0x1ac: 2 uses in ScreenBallPreview_Ctor

    // offsets whose address is passed on (sub-objects or arrays, not placed): 0x4, 0x20, 0x28, 0x34, 0x58, 0x64, 0xa4, 0xcc, 0x114, 0x15c, 0x1b0, 0x1b8, 0x1f8, 0x208, 0x5b0, 0x5e8, 0x620, 0x658
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define CGIBALLRENDERER_OFF(f, o) typedef char CGIBallRenderer_off_##f[((int)&((CGIBallRenderer*)0)->f == (o)) ? 1 : -1]
CGIBALLRENDERER_OFF(ballResource, 0x1c);
CGIBALLRENDERER_OFF(ringTextureResource, 0x1a8);
CGIBALLRENDERER_OFF(percentageTextureResource, 0x1ac);
#undef CGIBALLRENDERER_OFF

#endif  // CLASSES_CGIBALLRENDERER_H

#ifndef CLASSES_CGIBALLRENDERER_H
#define CLASSES_CGIBALLRENDERER_H

#include "game/types.h"

class CGIBallRenderer {
public:
    ~CGIBallRenderer();

    void RenderPreview(void* camera, void* matrix);

    void* vtbl;                      // 0x0: original vtable pointer
    char pad_004[0x18];              // 0x4
    void* ballResource;              // 0x1c
    char pad_020[0x188];             // 0x20
    void* ringTextureResource;       // 0x1a8
    void* percentageTextureResource; // 0x1ac
};

#define CGIBALLRENDERER_OFF(f, o) typedef char CGIBallRenderer_off_##f[((int)&((CGIBallRenderer*)0)->f == (o)) ? 1 : -1]
CGIBALLRENDERER_OFF(ballResource, 0x1c);
CGIBALLRENDERER_OFF(ringTextureResource, 0x1a8);
CGIBALLRENDERER_OFF(percentageTextureResource, 0x1ac);
#undef CGIBALLRENDERER_OFF

#endif // CLASSES_CGIBALLRENDERER_H

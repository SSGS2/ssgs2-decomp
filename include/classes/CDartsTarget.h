#ifndef CLASSES_CDARTSTARGET_H
#define CLASSES_CDARTSTARGET_H

#include "game/types.h"

// CDartsTarget: the darts target mesh, its vertex stream and its texture.
//
// CDartsSystem builds and releases the target and queues its preload. Draw
// submits the borrowed vertex stream and texture.
class CDartsTarget {
public:
    ~CDartsTarget();
    void Draw();

    void* vtbl;              // 0x0
    void* primaryMeshHandle; // 0x4: release in the destructor
    void* vertexStreamInfo;  // 0x8: DartsMeshHandle, released in the destructor
    void* textureResource;   // 0xc
    int renderSuppressed;    // 0x10
};

#define CDARTSTARGET_OFF(f, o) typedef char CDartsTarget_off_##f[((int)&((CDartsTarget*)0)->f == (o)) ? 1 : -1]
CDARTSTARGET_OFF(vertexStreamInfo, 0x8);
CDARTSTARGET_OFF(primaryMeshHandle, 0x4);
CDARTSTARGET_OFF(textureResource, 0xc);
CDARTSTARGET_OFF(renderSuppressed, 0x10);
#undef CDARTSTARGET_OFF
typedef char CDartsTarget_size[(sizeof(CDartsTarget) == 0x14) ? 1 : -1];

#endif // CLASSES_CDARTSTARGET_H

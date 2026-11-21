// PreviewResourcePair. Eight-byte resource pairs allocated by ScreenBallPreview_RebuildPlayerResources.
// Explicit layout view without inheritance or virtual methods.
#ifndef CLASSES_PREVIEWRESOURCEPAIR_H
#define CLASSES_PREVIEWRESOURCEPAIR_H
#include "iff/IffStruct.h"

class PreviewResourcePair {
public:
    IFF_STRUCT::sBall* ballRecord; // 0x0: ItemInventory_FindBallData result
    void* resource; // 0x4: ScreenBallPreview_RebuildPlayerResources loads by extension; ScreenBallPreview_dtor releases object
};

#define OFF(f, o) typedef char PreviewResourcePair_off_##f[((int)&((PreviewResourcePair*)0)->f == (o)) ? 1 : -1]
OFF(ballRecord, 0x0);
OFF(resource, 0x4);
#undef OFF
#define AsPreviewResourcePair(p) ((PreviewResourcePair*)(p))

#endif

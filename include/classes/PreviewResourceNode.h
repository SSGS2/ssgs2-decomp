// PreviewResourceNode. Resource-list nodes allocated as 0xC bytes by ScreenBallPreview_RebuildPlayerResources.
// Explicit layout view without inheritance or virtual methods.
#ifndef CLASSES_PREVIEWRESOURCENODE_H
#define CLASSES_PREVIEWRESOURCENODE_H

class PreviewResourceNode {
public:
    void* previous; // 0x0: ScreenBallPreview_RebuildPlayerResources, ScreenBallPreview_dtor doubly linked resource list
    void* next;     // 0x4: ScreenBallPreview_RebuildPlayerResources, ScreenBallPreview_dtor doubly linked resource list
    void* value;    // 0x8: ScreenBallPreview_RebuildPlayerResources, ScreenBallPreview_dtor doubly linked resource list
};

#define OFF(f, o) typedef char PreviewResourceNode_off_##f[((int)&((PreviewResourceNode*)0)->f == (o)) ? 1 : -1]
OFF(previous, 0x0);
OFF(next, 0x4);
OFF(value, 0x8);
#undef OFF
#define AsPreviewResourceNode(p) ((PreviewResourceNode*)(p))

#endif

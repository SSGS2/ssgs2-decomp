// TextureListNode: three-word intrusive list node used by texture methods in texture_capture_queue_and_mesh.cpp.
// Layout-only view; no virtual declarations or generated RTTI.
#ifndef CLASSES_TEXTURELISTNODE_H
#define CLASSES_TEXTURELISTNODE_H

class TextureListNode {
public:
    void* previous; // 0x0: TextureCaptureQueue_AddTexture stores the old previous link here
    void* next; // 0x4: TextureItemList_ClearAndRelease advances through this link
    void* value; // 0x8: TextureCaptureQueue_AddTexture placement-constructs the resource pointer here
};

#define OFF(f, o) typedef char TextureListNode_off_##f[((int)&((TextureListNode*)0)->f == (o)) ? 1 : -1]
OFF(previous, 0x0);
OFF(next, 0x4);
OFF(value, 0x8);
#undef OFF

#endif

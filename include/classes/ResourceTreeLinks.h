// ResourceTreeLinks: node linkage used by tree methods in camera_work_resource_trees.cpp; parent carries a low-bit tag.
// Layout-only view; no virtual declarations or generated RTTI.
#ifndef CLASSES_RESOURCETREELINKS_H
#define CLASSES_RESOURCETREELINKS_H

class ResourceTreeLinks {
public:
    void* left; // 0x0: CameraResourceGroupTree_FindInsertionPosition follows when the requested key is smaller
    void* right; // 0x4: CameraResourceGroupTree_FindInsertionPosition follows otherwise
    void* taggedParent; // 0x8: CameraOwnedAnimationIterator_PostIncrement and CameraOwnedGroupIterator_PostIncrement mask off bit 0 while climbing
};

#define OFF(f, o) typedef char ResourceTreeLinks_off_##f[((int)&((ResourceTreeLinks*)0)->f == (o)) ? 1 : -1]
OFF(left, 0x0);
OFF(right, 0x4);
OFF(taggedParent, 0x8);
#undef OFF

#endif

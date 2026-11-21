// ResourceLookupTree: tree subobject operated on by CameraResourceGroupTree_FindInsertionPosition, CameraResourceLookupTree_Clear and CameraResourceLookupTree_Assign.
// Layout-only view; no virtual declarations or generated RTTI.
#ifndef CLASSES_RESOURCELOOKUPTREE_H
#define CLASSES_RESOURCELOOKUPTREE_H

class ResourceLookupTree {
public:
    void* unk_0; // 0x0: CameraResourceLookupTree_Clear clears with root
    void* root; // 0x4: CameraResourceGroupTree_FindInsertionPosition begins its search here
    void* first; // 0x8: CameraResourceLookupTree_Clear resets to the +4 sentinel
};

#define OFF(f, o) typedef char ResourceLookupTree_off_##f[((int)&((ResourceLookupTree*)0)->f == (o)) ? 1 : -1]
OFF(unk_0, 0x0);
OFF(root, 0x4);
OFF(first, 0x8);
#undef OFF

#endif

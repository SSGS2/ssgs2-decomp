// ResourceStringNode: string-key node layout from ResourceGroups_FindNamedValue (inline bytes overlap the three string words).
// Layout-only view; no virtual declarations or generated RTTI.
#ifndef CLASSES_RESOURCESTRINGNODE_H
#define CLASSES_RESOURCESTRINGNODE_H

class ResourceStringNode {
public:
    void* left; // 0x0: ResourceGroups_FindNamedValue descends by string comparison
    void* right; // 0x4: ResourceGroups_FindNamedValue descends by string comparison
    void* taggedParent; // 0x8: CameraOwnedAnimationIterator_PostIncrement masks bit 0
    unsigned int stringFlags; // 0xC: ResourceGroups_FindNamedValue tests bit 31 for heap storage; low-address byte holds inline length
    unsigned int heapLength; // 0x10: ResourceGroups_FindNamedValue uses when stringFlags bit 31 is set
    const char* heapData; // 0x14: ResourceGroups_FindNamedValue compares; FREE_NODE releases for heap strings
    void* value; // 0x18: ResourceGroups_FindNamedValue returns after successful string lookup
};

#define OFF(f, o) typedef char ResourceStringNode_off_##f[((int)&((ResourceStringNode*)0)->f == (o)) ? 1 : -1]
OFF(left, 0x0);
OFF(right, 0x4);
OFF(taggedParent, 0x8);
OFF(stringFlags, 0xC);
OFF(heapLength, 0x10);
OFF(heapData, 0x14);
OFF(value, 0x18);
#undef OFF

#endif

// ResourceGroupNode: outer integer-key node from ResourceGroups_FindNamedValue.
// Layout-only view; no virtual declarations or generated RTTI.
#ifndef CLASSES_RESOURCEGROUPNODE_H
#define CLASSES_RESOURCEGROUPNODE_H

class ResourceGroupNode {
public:
    void* left;         // 0x0: ResourceGroups_FindNamedValue follows for smaller group keys
    void* right;        // 0x4: ResourceGroups_FindNamedValue follows for larger group keys
    void* taggedParent; // 0x8: tree iterator linkage
    unsigned int key;   // 0xC: ResourceGroups_FindNamedValue compares with its key argument
    char pad_010[0x4];
    void* stringRoot;  // 0x14: ResourceGroups_FindNamedValue begins the nested string search
    void* firstString; // 0x18: ResourceGroups_FindNamedValue uses when text is null
};

#define OFF(f, o) typedef char ResourceGroupNode_off_##f[((int)&((ResourceGroupNode*)0)->f == (o)) ? 1 : -1]
OFF(left, 0x0);
OFF(right, 0x4);
OFF(taggedParent, 0x8);
OFF(key, 0xC);
OFF(stringRoot, 0x14);
OFF(firstString, 0x18);
#undef OFF

#endif

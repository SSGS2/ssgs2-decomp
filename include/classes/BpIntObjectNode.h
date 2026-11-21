// Integer-keyed object-map node searched by BpIntObjectTree_FindInsertionPosition / CBpObject_FindTextureById in bp_object_sprite_resources.cpp.
// Payload is specific to this map; callback and string maps use different payloads.
#ifndef CLASSES_BPINTOBJECTNODE_H
#define CLASSES_BPINTOBJECTNODE_H
struct BpIntObjectNode {
    void* left;
    void* right;
    unsigned int parentAndColor;
    int key; // 0xc: CBpObject_FindTextureById compares to key to find a lower bound
    void* value; // 0x10: CBpObject_FindTextureById returns this for a matching node
};
#define OFF(f, o) typedef char BpIntObjectNode_off_##f[((int)&((BpIntObjectNode*)0)->f == (o)) ? 1 : -1]
OFF(left, 0);
OFF(right, 4);
OFF(parentAndColor, 8);
OFF(key, 0xc);
OFF(value, 0x10);
#undef OFF
#define AsBpIntObjectNode(p) ((BpIntObjectNode*)(p))
#endif

// FontCacheNode: offset-preserving view; no constructors, bases, or virtual methods.
// Evidence: WFixedFont_dtor (0x8021C59C) follows previous and rewires next/previous when recycling active nodes; object is virtually deleted.
#ifndef CLASSES_FONTCACHENODE_H
#define CLASSES_FONTCACHENODE_H

struct FontCacheNode {
    void* object; // 0x0
    char pad_004[0x8];
    void* previous; // 0xC
    void* next; // 0x10
};

#define OFF(f, o) typedef char FontCacheNode_off_##f[((int)&((FontCacheNode*)0)->f == (o)) ? 1 : -1]
OFF(object, 0x0);
OFF(previous, 0xC);
OFF(next, 0x10);
#undef OFF
typedef char FontCacheNode_size[(sizeof(FontCacheNode) == 0x14) ? 1 : -1];
#define AsFontCacheNode(p) ((FontCacheNode*)(p))

#endif // CLASSES_FONTCACHENODE_H

// BpTreeLinks: offset-preserving view; no constructors, bases, or virtual methods.
// Evidence: BpIntObjectTree_FindInsertionPosition / CBpObject::FindTextureById search left/right; CBpObject_ClearResources and BpObject_PollLoad mask parentAndColor with ~1u.
#ifndef CLASSES_BPTREELINKS_H
#define CLASSES_BPTREELINKS_H

struct BpTreeLinks {
    void* left;                  // 0x0
    void* right;                 // 0x4
    unsigned int parentAndColor; // 0x8
};

#define OFF(f, o) typedef char BpTreeLinks_off_##f[((int)&((BpTreeLinks*)0)->f == (o)) ? 1 : -1]
OFF(left, 0x0);
OFF(right, 0x4);
OFF(parentAndColor, 0x8);
#undef OFF
typedef char BpTreeLinks_size[(sizeof(BpTreeLinks) == 0xC) ? 1 : -1];
#define AsBpTreeLinks(p) ((BpTreeLinks*)(p))

#endif // CLASSES_BPTREELINKS_H

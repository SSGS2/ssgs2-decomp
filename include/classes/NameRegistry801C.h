#ifndef CLASSES_NAMEREGISTRY801C_H
#define CLASSES_NAMEREGISTRY801C_H

#include "game/types.h"

class NameRegistry801C {
public:
    void*
        first; // 0x0: CourseNameRegistry_Insert allocates 0x18-byte nodes; NameRegistry_FindByName hashes names and traverses buckets
    char pad_004[0x4]; // 0x4
    void*
        blocks; // 0x8: CourseNameRegistry_Insert allocates 0x18-byte nodes; NameRegistry_FindByName hashes names and traverses buckets
    void*
        freeNodes; // 0xc: CourseNameRegistry_Insert allocates 0x18-byte nodes; NameRegistry_FindByName hashes names and traverses buckets
    int nodesPerBlock; // 0x10: CourseNameRegistry_Insert allocates 0x18-byte nodes; NameRegistry_FindByName hashes names and traverses buckets
    unsigned int
        hashMask; // 0x14: CourseNameRegistry_Insert allocates 0x18-byte nodes; NameRegistry_FindByName hashes names and traverses buckets
    char pad_018[0x4]; // 0x18
    void*
        buckets; // 0x1c: CourseNameRegistry_Insert allocates 0x18-byte nodes; NameRegistry_FindByName hashes names and traverses buckets
    char lock
        [4]; // 0x20: CourseNameRegistry_Insert / NameRegistry_FindByName: address passed to WInstanceLock; only address anchor, size unknown
};

#define NAMEREGISTRY801C_OFF(f, o) \
    typedef char NameRegistry801C_off_##f[((int)&((NameRegistry801C*)0)->f == (o)) ? 1 : -1]
NAMEREGISTRY801C_OFF(blocks, 0x8);
NAMEREGISTRY801C_OFF(freeNodes, 0xc);
NAMEREGISTRY801C_OFF(nodesPerBlock, 0x10);
NAMEREGISTRY801C_OFF(hashMask, 0x14);
NAMEREGISTRY801C_OFF(buckets, 0x1c);
NAMEREGISTRY801C_OFF(lock, 0x20);
NAMEREGISTRY801C_OFF(first, 0x0);
#undef NAMEREGISTRY801C_OFF

#endif // CLASSES_NAMEREGISTRY801C_H

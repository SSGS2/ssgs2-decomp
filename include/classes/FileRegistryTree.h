// FileRegistryTree: explicit layout view; no inheritance, virtual methods, or RTTI.
// Field comments cite the original function addresses that establish their use.
#ifndef CLASSES_FILEREGISTRYTREE_H
#define CLASSES_FILEREGISTRYTREE_H

#include "game/types.h"

class FileRegistryTree {
public:
    unsigned int count; // 0x0: FileRegistry_InsertNode: size increment and maximum-size test
    unsigned int root;  // 0x4: FileRegistry_InsertNode: root argument to tree balancing
    void* first;        // 0x8: FileRegistry_InsertNode: first-node update
};

#define FILEREGISTRYTREE_OFF(f, o) \
    typedef char FileRegistryTree_off_##f[((int)&((FileRegistryTree*)0)->f == (o)) ? 1 : -1]
FILEREGISTRYTREE_OFF(count, 0x0);
FILEREGISTRYTREE_OFF(root, 0x4);
FILEREGISTRYTREE_OFF(first, 0x8);
#undef FILEREGISTRYTREE_OFF

#endif

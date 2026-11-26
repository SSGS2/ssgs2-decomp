#ifndef CLASSES_CEVENTEFFECT_H
#define CLASSES_CEVENTEFFECT_H

#include "game/types.h"

// The effect tree header: a root node map plus the first cached iterator.
class CEventEffect {
public:
    // Install the empty tree header.
    void* Init();
    // Delete every owned event effect, free the map nodes, and optionally release the object.
    void* Destroy(int dealloc);

    unsigned int count; // 0x0
    void* treeRoot;     // 0x4
    void* firstNode;    // 0x8
    void* effectVtable; // 0xc
};

#define CEVENTEFFECT_OFF(f, o) typedef char CEventEffect_off_##f[((int)&((CEventEffect*)0)->f == (o)) ? 1 : -1]
CEVENTEFFECT_OFF(count, 0x0);
CEVENTEFFECT_OFF(treeRoot, 0x4);
CEVENTEFFECT_OFF(firstNode, 0x8);
CEVENTEFFECT_OFF(effectVtable, 0xc);
#undef CEVENTEFFECT_OFF

#endif // CLASSES_CEVENTEFFECT_H

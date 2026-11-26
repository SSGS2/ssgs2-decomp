#ifndef CLASSES_CSOUNDSOURCELISTNODE_H
#define CLASSES_CSOUNDSOURCELISTNODE_H

#include "game/types.h"

class CSoundSourceListNode {
public:
    void* vtbl;                      // 0x0: original vtable pointer
    void* nodeVtable;                // 0x4
    unsigned int stringStorageFlags; // 0x8
    char pad_00C[0x4];               // 0xc
    void* stringHeap;                // 0x10
    void* soundBufferResource;       // 0x14
    void* soundBuffer;               // 0x18
    char pad_01C[0x3c];              // 0x1c
    void* unk_58;                    // 0x58

    ~CSoundSourceListNode();
};

#define CSOUNDSOURCELISTNODE_OFF(f, o) \
    typedef char CSoundSourceListNode_off_##f[((int)&((CSoundSourceListNode*)0)->f == (o)) ? 1 : -1]
CSOUNDSOURCELISTNODE_OFF(nodeVtable, 0x4);
CSOUNDSOURCELISTNODE_OFF(stringStorageFlags, 0x8);
CSOUNDSOURCELISTNODE_OFF(stringHeap, 0x10);
CSOUNDSOURCELISTNODE_OFF(soundBufferResource, 0x14);
CSOUNDSOURCELISTNODE_OFF(soundBuffer, 0x18);
CSOUNDSOURCELISTNODE_OFF(unk_58, 0x58);
#undef CSOUNDSOURCELISTNODE_OFF

#endif // CLASSES_CSOUNDSOURCELISTNODE_H

// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CSOUNDSOURCELISTNODE_H
#define CLASSES_CSOUNDSOURCELISTNODE_H

#include "game/types.h"

class CSoundSourceListNode {
public:
    void* vtbl;                         // 0x0: original vtable pointer
    void* nodeVtable;                        // 0x4: 1 use in CSoundSourceListNode_dtor
    unsigned int stringStorageFlags;                 // 0x8: 1 use in CSoundSourceListNode_dtor
    char pad_00C[0x4];                  // 0xc
    void* stringHeap;                       // 0x10: 1 use in CSoundSourceListNode_dtor
    void* soundBufferResource;                       // 0x14: 2 uses in CSoundSourceListNode_dtor
    void* soundBuffer;                       // 0x18: 2 uses in CSoundSourceListNode_dtor
    char pad_01C[0x3c];                 // 0x1c
    void* unk_58;                       // 0x58: 1 use in SoundManager_AcquireSoundBank

    // offsets whose address is passed on (sub-objects or arrays, not placed): 0x8
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define CSOUNDSOURCELISTNODE_OFF(f, o) typedef char CSoundSourceListNode_off_##f[((int)&((CSoundSourceListNode*)0)->f == (o)) ? 1 : -1]
CSOUNDSOURCELISTNODE_OFF(nodeVtable, 0x4);
CSOUNDSOURCELISTNODE_OFF(stringStorageFlags, 0x8);
CSOUNDSOURCELISTNODE_OFF(stringHeap, 0x10);
CSOUNDSOURCELISTNODE_OFF(soundBufferResource, 0x14);
CSOUNDSOURCELISTNODE_OFF(soundBuffer, 0x18);
CSOUNDSOURCELISTNODE_OFF(unk_58, 0x58);
#undef CSOUNDSOURCELISTNODE_OFF

#endif  // CLASSES_CSOUNDSOURCELISTNODE_H

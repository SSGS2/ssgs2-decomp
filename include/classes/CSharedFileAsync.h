// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CSHAREDFILEASYNC_H
#define CLASSES_CSHAREDFILEASYNC_H

#include "game/types.h"

class CSharedFileAsync {
public:
    void* vtbl;                         // 0x0: original vtable pointer
    void* readBuffer;                   // 0x4: 1 use in CSharedFileAsync_PollRead
    char pad_008[0x10];                 // 0x8
    int readFlags;                      // 0x18: 3 uses in CSharedFileAsync_PollRead
    void* fileAsync;                    // 0x1c: 8 uses in CSharedFileAsync_PollRead, CSharedFileAsync_dtor, CSharedFileAsync_GetSize
    unsigned int alignedReadSize;       // 0x20: 2 uses in CSharedFileAsync_PollRead

    // offsets whose address is passed on (sub-objects or arrays, not placed): 0x18, 0x24
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define CSHAREDFILEASYNC_OFF(f, o) typedef char CSharedFileAsync_off_##f[((int)&((CSharedFileAsync*)0)->f == (o)) ? 1 : -1]
CSHAREDFILEASYNC_OFF(readBuffer, 0x4);
CSHAREDFILEASYNC_OFF(readFlags, 0x18);
CSHAREDFILEASYNC_OFF(fileAsync, 0x1c);
CSHAREDFILEASYNC_OFF(alignedReadSize, 0x20);
#undef CSHAREDFILEASYNC_OFF

#endif  // CLASSES_CSHAREDFILEASYNC_H

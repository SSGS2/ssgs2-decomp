#ifndef CLASSES_CSHAREDFILEASYNC_H
#define CLASSES_CSHAREDFILEASYNC_H

#include "game/types.h"

class CSharedFileAsync {
public:
    void* vtbl;                   // 0x0: original vtable pointer
    void* readBuffer;             // 0x4
    char pad_008[0x10];           // 0x8
    int readFlags;                // 0x18
    void* fileAsync;              // 0x1c
    unsigned int alignedReadSize; // 0x20

    int GetSize();
    int PollRead();
    static void* Create(void* path);

    ~CSharedFileAsync();
};

#define CSHAREDFILEASYNC_OFF(f, o) \
    typedef char CSharedFileAsync_off_##f[((int)&((CSharedFileAsync*)0)->f == (o)) ? 1 : -1]
CSHAREDFILEASYNC_OFF(readBuffer, 0x4);
CSHAREDFILEASYNC_OFF(readFlags, 0x18);
CSHAREDFILEASYNC_OFF(fileAsync, 0x1c);
CSHAREDFILEASYNC_OFF(alignedReadSize, 0x20);
#undef CSHAREDFILEASYNC_OFF

#endif // CLASSES_CSHAREDFILEASYNC_H

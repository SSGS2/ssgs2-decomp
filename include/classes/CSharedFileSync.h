#ifndef CLASSES_CSHAREDFILESYNC_H
#define CLASSES_CSHAREDFILESYNC_H

#include "game/types.h"

class CSharedFileSync {
public:
    void* vtbl;         // 0x0: original vtable pointer
    void* buffer;       // 0x4
    char pad_008[0x10]; // 0x8
    int fileSize;       // 0x18

    int vfunc_1();
    static int vfunc_3(void* a);
    static void* Create(void* path);
    void* CreateFromBuffer(void* src, unsigned int n);
    int LoadFile(void* path);

    ~CSharedFileSync();
};

#define CSHAREDFILESYNC_OFF(f, o) typedef char CSharedFileSync_off_##f[((int)&((CSharedFileSync*)0)->f == (o)) ? 1 : -1]
CSHAREDFILESYNC_OFF(buffer, 0x4);
CSHAREDFILESYNC_OFF(fileSize, 0x18);
#undef CSHAREDFILESYNC_OFF

#endif // CLASSES_CSHAREDFILESYNC_H

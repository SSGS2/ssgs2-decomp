// FileRegistryString: explicit layout view; no inheritance, virtual methods, or RTTI.
// Field comments cite the original function addresses that establish their use.
#ifndef CLASSES_FILEREGISTRYSTRING_H
#define CLASSES_FILEREGISTRYSTRING_H

#include "game/types.h"

class FileRegistryString {
public:
    // Free the heap data of a registry key when its flags select heap storage; the allocator argument is unused.
    static void ReleaseStorage(int unusedAllocator, FileRegistryString* string);

    unsigned int flags;  // 0x0: ReleaseStorage: high bit means allocated data
    unsigned int length; // 0x4: FileIOString_CtorFromCString: string length initially cleared
    void* data;          // 0x8: ReleaseStorage: allocated data released
};

#define FILEREGISTRYSTRING_OFF(f, o) \
    typedef char FileRegistryString_off_##f[((int)&((FileRegistryString*)0)->f == (o)) ? 1 : -1]
FILEREGISTRYSTRING_OFF(flags, 0x0);
FILEREGISTRYSTRING_OFF(length, 0x4);
FILEREGISTRYSTRING_OFF(data, 0x8);
#undef FILEREGISTRYSTRING_OFF

#endif

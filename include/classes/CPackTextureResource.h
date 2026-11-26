#ifndef CLASSES_CPACKTEXTURERESOURCE_H
#define CLASSES_CPACKTEXTURERESOURCE_H

#include "game/types.h"

class CPackTextureResource {
public:
    void* vtbl;               // 0x0: original vtable pointer
    char pad_004[0x4];        // 0x4
    unsigned char pathFlags;  // 0x8
    unsigned char inlinePath; // 0x9
    char pad_00A[0x12];       // 0xa
    void* sharedFile;         // 0x1c

    int EntryBasenameMatches(unsigned int index, void* namePtr);
    int LoadArchiveEntries(void* file, void* section);
    int IsLoadFinished();
    int PollLoad();
    void* Init(const char* text, void** values, unsigned int count);

    ~CPackTextureResource();
};

#define CPACKTEXTURERESOURCE_OFF(f, o) \
    typedef char CPackTextureResource_off_##f[((int)&((CPackTextureResource*)0)->f == (o)) ? 1 : -1]
CPACKTEXTURERESOURCE_OFF(pathFlags, 0x8);
CPACKTEXTURERESOURCE_OFF(inlinePath, 0x9);
CPACKTEXTURERESOURCE_OFF(sharedFile, 0x1c);
#undef CPACKTEXTURERESOURCE_OFF

#endif // CLASSES_CPACKTEXTURERESOURCE_H

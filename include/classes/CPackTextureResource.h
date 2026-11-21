// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CPACKTEXTURERESOURCE_H
#define CLASSES_CPACKTEXTURERESOURCE_H

#include "game/types.h"

class CPackTextureResource {
public:
    void* vtbl;                         // 0x0: original vtable pointer
    char pad_004[0x4];                  // 0x4
    unsigned char pathFlags;                // 0x8: 3 uses in CPackTextureResource_PollLoad
    unsigned char inlinePath;                // 0x9: 2 uses in CPackTextureResource_dtor, CPackTextureResource_PollLoad
    char pad_00A[0x12];                 // 0xa
    void* sharedFile;                       // 0x1c: 6 uses in CPackTextureResource_PollLoad, CPackTextureResource_Init

    // offsets whose address is passed on (sub-objects or arrays, not placed): 0x8, 0x9
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define CPACKTEXTURERESOURCE_OFF(f, o) typedef char CPackTextureResource_off_##f[((int)&((CPackTextureResource*)0)->f == (o)) ? 1 : -1]
CPACKTEXTURERESOURCE_OFF(pathFlags, 0x8);
CPACKTEXTURERESOURCE_OFF(inlinePath, 0x9);
CPACKTEXTURERESOURCE_OFF(sharedFile, 0x1c);
#undef CPACKTEXTURERESOURCE_OFF

#endif  // CLASSES_CPACKTEXTURERESOURCE_H

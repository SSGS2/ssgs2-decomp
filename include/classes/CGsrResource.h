// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CGSRRESOURCE_H
#define CLASSES_CGSRRESOURCE_H

#include "game/types.h"

class CGsrResource {
public:
    void* imageData;                         // 0x0: owned image bytes; the actual vtable is at +0x38
    unsigned int bufferSize;                 // 0x4: 4 uses in CGsrResource_Ctor, CGsrResource_ctor
    void* primarySharedResource;                        // 0x8: 4 uses in CGsrResource_dtor, CGsrResource_Ctor, CGsrResource_ctor
    void* secondarySharedResource;                        // 0xc: 4 uses in CGsrResource_dtor, CGsrResource_Ctor, CGsrResource_ctor
    unsigned int texturePathStorageFlags;                // 0x10: 3 uses in CGsrResource_dtor, CGsrResource_Ctor, CGsrResource_ctor
    int texturePathLength;                         // 0x14: 2 uses in CGsrResource_Ctor, CGsrResource_ctor
    void* texturePathHeapData;                       // 0x18: 3 uses in CGsrResource_dtor, CGsrResource_Ctor, CGsrResource_ctor
    int textureInfo[7];                // 0x1c: CGsrResource_LoadSharedTexture copies seven descriptor words from ISharedResource_GetTextureDescription; Ctor passes to gtl_Image_GetPlatform7Description
    void* resourceVtable;                       // 0x38: 2 uses in CGsrResource_dtor, CGsrResource_Ctor

    // offsets whose address is passed on (sub-objects or arrays, not placed): 0x10, 0x1c, 0x38
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define CGSRRESOURCE_OFF(f, o) typedef char CGsrResource_off_##f[((int)&((CGsrResource*)0)->f == (o)) ? 1 : -1]
CGSRRESOURCE_OFF(bufferSize, 0x4);
CGSRRESOURCE_OFF(primarySharedResource, 0x8);
CGSRRESOURCE_OFF(secondarySharedResource, 0xc);
CGSRRESOURCE_OFF(texturePathStorageFlags, 0x10);
CGSRRESOURCE_OFF(texturePathLength, 0x14);
CGSRRESOURCE_OFF(texturePathHeapData, 0x18);
CGSRRESOURCE_OFF(textureInfo, 0x1c);
CGSRRESOURCE_OFF(resourceVtable, 0x38);
#undef CGSRRESOURCE_OFF

#endif  // CLASSES_CGSRRESOURCE_H

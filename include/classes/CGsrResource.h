#ifndef CLASSES_CGSRRESOURCE_H
#define CLASSES_CGSRRESOURCE_H

#include "game/types.h"

class CGsrResource {
public:
    void* imageData;                      // 0x0: owned image bytes; the actual vtable is at +0x38
    unsigned int bufferSize;              // 0x4
    void* primarySharedResource;          // 0x8
    void* secondarySharedResource;        // 0xc
    unsigned int texturePathStorageFlags; // 0x10
    int texturePathLength;                // 0x14
    void* texturePathHeapData;            // 0x18
    int textureInfo
        [7]; // 0x1c: CGsrResource_LoadSharedTexture copies seven descriptor words from ISharedResource_GetTextureDescription; Ctor passes to gtl_Image_GetPlatform7Description
    void* resourceVtable; // 0x38

    // Copy an encoded image into owned memory and query its platform descriptor.
    CGsrResource(const void* source, unsigned int size);
    // Start an empty resource and store the texture path.
    CGsrResource(const char* path);
    ~CGsrResource();

    void* LoadSharedTexture();
};

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

#endif // CLASSES_CGSRRESOURCE_H

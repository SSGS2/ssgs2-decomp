#ifndef CLASSES_CSHAREDTEXTURERESOURCE_H
#define CLASSES_CSHAREDTEXTURERESOURCE_H

#include "game/types.h"

// Only the vtable, mutex and palette fields are declared so far; the rest of the layout is still unmodelled.
// Other member functions reach their data through file-local layouts or raw offsets.
class CSharedTextureResource {
public:
    // Free the optional native palette before destroying the shared-resource base.
    ~CSharedTextureResource();

    int CreateTextureFromMemory(void* path, void* instance);

    void* vtbl;        // 0x00: __vt__22CSharedTextureResource
    char pad_04[0x2C]; // 0x04
    char mutex[0x18];  // 0x30: instance mutex
    char pad_48[0x1C]; // 0x48
    void* palette;     // 0x64: optional native palette object
};
typedef char CSharedTexture_off_palette[((int)&((CSharedTextureResource*)0)->palette == 0x64) ? 1 : -1];

#endif // CLASSES_CSHAREDTEXTURERESOURCE_H

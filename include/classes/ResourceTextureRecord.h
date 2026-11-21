// ResourceManager's texture record; allocated and populated by ResourceManager_GetOrLoadBitmapByPath (0x80062520).
// Layout only: no inheritance, constructors, or virtual methods.
#ifndef CLASSES_RESOURCETEXTURERECORD_H
#define CLASSES_RESOURCETEXTURERECORD_H
struct ResourceTextureRecord {
    int left;                   // 0x0: ResourceManager_GetBitmapWidth subtracts from right
    int top;                    // 0x4: ResourceManager_GetBitmapHeight subtracts from bottom
    int right;                  // 0x8: ResourceManager_GetOrLoadBitmapByPath copies descriptor width
    int bottom;                 // 0xc: ResourceManager_GetOrLoadBitmapByPath copies descriptor height
    int referenceCount;         // 0x10: ResourceManager_GetOrLoadBitmapByPath initializes to 1; cache hits increment
    void* resource;             // 0x14: create result, returned to caller
    unsigned int resourceBytes; // 0x18: area times bytes per pixel, added to manager total
};
#define OFF(f, o) typedef char ResourceTextureRecord_off_##f[((int)&((ResourceTextureRecord*)0)->f == (o)) ? 1 : -1]
OFF(left, 0x0);
OFF(top, 0x4);
OFF(right, 0x8);
OFF(bottom, 0xc);
OFF(referenceCount, 0x10);
OFF(resource, 0x14);
OFF(resourceBytes, 0x18);
#undef OFF
#endif

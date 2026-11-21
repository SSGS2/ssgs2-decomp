// Bitmap layout from Bitmap_ctor (0x80045FE4) and Bitmap_dtor (0x80046008).
// Layout view only; preserves the resource pointer before the vtable at +4.
#ifndef CLASSES_BITMAPLAYOUT_H
#define CLASSES_BITMAPLAYOUT_H
struct BitmapLayout {
    void* resource;          // 0x0: ctor argument; Bitmap_dtor deletes it
    void* vtable;            // 0x4: __vt__6Bitmap
    void* pixels;            // 0x8: Bitmap_dtor frees unless pixelsBorrowed is set
    char pad_00c[4];
    int unk_10;              // 0x10: Bitmap_ctor zeroes; meaning not established
    unsigned char pixelsBorrowed; // 0x14: suppresses appFreeUni(pixels)
};
#define OFF(f, o) typedef char BitmapLayout_off_##f[((int)&((BitmapLayout*)0)->f == (o)) ? 1 : -1]
OFF(resource, 0);
OFF(vtable, 4);
OFF(pixels, 8);
OFF(unk_10, 0x10);
OFF(pixelsBorrowed, 0x14);
#undef OFF
#endif

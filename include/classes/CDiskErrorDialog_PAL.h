#ifndef CLASSES_CDISKERRORDIALOG_PAL_H
#define CLASSES_CDISKERRORDIALOG_PAL_H

#include "game/types.h"

class CDiskErrorDialog_PAL {
public:
    // Runs the derived and base teardown hooks; frees the object when flag > 0. Returns this.
    void* Destroy(int flag);
    // Initialize the dialog in place; returns this.
    void* Init(void* archiveEntryIndex);

    void* vtbl;                   // 0x0: original vtable pointer
    char opacityTweenPrefix[0x4]; // 0x4
    float opacity;                // 0x8: multiplied into rendered vertex alpha
    char opacityTweenTail[0x14];  // 0xc
    float unk_20;                 // 0x20: CDiskErrorDialog_PAL_LoadImageTexture initializes; further meaning unproved
    void* archiveEntryIndex;      // 0x24
    int imageIndex;               // 0x28: selects one of three atlas rectangles
    int texture;                  // 0x2c: queried, bound and released as a texture resource
    float textureWidth;           // 0x30: divides horizontal atlas coordinates
    float textureHeight;          // 0x34: divides vertical atlas coordinates
};

#define CDISKERRORDIALOG_PAL_OFF(f, o) \
    typedef char CDiskErrorDialog_PAL_off_##f[((int)&((CDiskErrorDialog_PAL*)0)->f == (o)) ? 1 : -1]
CDISKERRORDIALOG_PAL_OFF(opacity, 0x8);
CDISKERRORDIALOG_PAL_OFF(unk_20, 0x20);
CDISKERRORDIALOG_PAL_OFF(archiveEntryIndex, 0x24);
CDISKERRORDIALOG_PAL_OFF(imageIndex, 0x28);
CDISKERRORDIALOG_PAL_OFF(texture, 0x2c);
CDISKERRORDIALOG_PAL_OFF(textureWidth, 0x30);
CDISKERRORDIALOG_PAL_OFF(textureHeight, 0x34);
#undef CDISKERRORDIALOG_PAL_OFF

#endif // CLASSES_CDISKERRORDIALOG_PAL_H

// CTimeOutInfo: the shot-timeout readout in the HUD.
//
// It owns a font resource and the fixed-size text buffer it formats.
#ifndef CLASSES_CTIMEOUTINFO_H
#define CLASSES_CTIMEOUTINFO_H

#include "game/types.h"

class CTimeOutInfo {
public:
    void vfunc_5();
    void UpdateFadeState(float delta);
    void SetTimeoutCountTextAndColor();

    // Release the font and the text buffer, then optionally delete the object.
    void* Destroy(int deleting);

    void* vtbl;         // 0x00
    void* fontResource; // 0x04
    char pad_008[4];    // 0x08
    unsigned int color; // 0x0C
    int state;          // 0x10
    void* textBuffer;   // 0x14
    int count;          // 0x18
    float value;        // 0x1C
    char pad_020[4];    // 0x20
};

#define CTIMEOUTINFO_OFF(f, o) typedef char CTimeOutInfo_off_##f[((int)&((CTimeOutInfo*)0)->f == (o)) ? 1 : -1]
CTIMEOUTINFO_OFF(fontResource, 0x04);
CTIMEOUTINFO_OFF(color, 0x0C);
CTIMEOUTINFO_OFF(state, 0x10);
CTIMEOUTINFO_OFF(textBuffer, 0x14);
CTIMEOUTINFO_OFF(count, 0x18);
CTIMEOUTINFO_OFF(value, 0x1C);
#undef CTIMEOUTINFO_OFF
typedef char CTimeOutInfo_size[(sizeof(CTimeOutInfo) == 0x24) ? 1 : -1];

#endif // CLASSES_CTIMEOUTINFO_H

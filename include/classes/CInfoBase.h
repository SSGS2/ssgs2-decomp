// CInfoBase: the text field under the HUD readouts. It holds the selected font, the
// text color and alignment, and the buffer the text is formatted into. The class is
// declared without virtual functions on purpose: its vtable is still taken from the
// original game data, so `vtbl` is spelled out as a member.
#ifndef CLASSES_CINFOBASE_H
#define CLASSES_CINFOBASE_H

#include "game/types.h"

struct CInfoBase {
    CInfoBase();
// The HUD readout file defines CINFOBASE_NO_DTOR: its classes write the base cleanup out in
// their own destructors, so the base destructor must not be declared there.
#ifndef CINFOBASE_NO_DTOR
    ~CInfoBase();
#endif

    // Register a named font configuration in the shared HUD font registry.
    void RegisterNamedInfo(const char* name, int cellWidth, int cellHeight, int imageCount, int atlasWidth,
                           int atlasHeight, const char* glyphOrder, int halfScale);

    void* vtbl;         // 0x00
    void* fontResource; // 0x04
    char pad_008[4];    // 0x08
    unsigned int color; // 0x0C
    int alignment;      // 0x10
    char* textBuffer;   // 0x14
};

#define CINFOBASE_OFFSET(field, offset) \
    typedef char CInfoBase_offset_##field[((int)&((CInfoBase*)0)->field == (offset)) ? 1 : -1]
CINFOBASE_OFFSET(fontResource, 0x04);
CINFOBASE_OFFSET(color, 0x0C);
CINFOBASE_OFFSET(alignment, 0x10);
CINFOBASE_OFFSET(textBuffer, 0x14);
#undef CINFOBASE_OFFSET
typedef char CInfoBase_size[(sizeof(CInfoBase) == 0x18) ? 1 : -1];

#endif // CLASSES_CINFOBASE_H

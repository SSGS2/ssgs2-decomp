// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CTOURWINDOW_H
#define CLASSES_CTOURWINDOW_H

#include "game/types.h"

class CTourWindow {
public:
    void* screenState;                         // 0x0: screen-state word (1 selecting, 2 closing)
    void* bpEntryId;                        // 0x4: 1 use in CTourWindow_IsLoadComplete
    void* layout;                        // 0x8: 2 uses in CTourWindow_IsLoadComplete
    signed char selectedCharacterIndex;          // 0xc: CTourWindow_UpdateSelectionAndStatBars stores param % 16 (first eight)
    signed char selectedCaddieIndex;         // 0xd: CTourWindow_UpdateSelectionAndStatBars stores param % 16 - 8
    signed char characterAvailable[8];       // 0xe: CTourWindow_InitializeCharacterSelection reads record bit 0x40
    signed char caddieAvailable[8];      // 0x16: CTourWindow_InitializeCharacterSelection reads record bit 0x40
    char pad_01E[2];
    void* playerRecord;                 // 0x20: CTourWindow_UpdateSelectionAndStatBars copies 0x828 bytes from here
    void* numberDraw;                   // 0x24: CTourWindow_Draw passes it to UITextTable_Draw
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define CTOURWINDOW_OFF(f, o) typedef char CTourWindow_off_##f[((int)&((CTourWindow*)0)->f == (o)) ? 1 : -1]
CTOURWINDOW_OFF(screenState, 0x0);
CTOURWINDOW_OFF(selectedCharacterIndex, 0xc);
CTOURWINDOW_OFF(selectedCaddieIndex, 0xd);
CTOURWINDOW_OFF(characterAvailable, 0xe);
CTOURWINDOW_OFF(caddieAvailable, 0x16);
CTOURWINDOW_OFF(playerRecord, 0x20);
CTOURWINDOW_OFF(numberDraw, 0x24);
CTOURWINDOW_OFF(bpEntryId, 0x4);
CTOURWINDOW_OFF(layout, 0x8);
#undef CTOURWINDOW_OFF

#endif  // CLASSES_CTOURWINDOW_H


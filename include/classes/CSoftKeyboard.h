#ifndef CLASSES_CSOFTKEYBOARD_H
#define CLASSES_CSOFTKEYBOARD_H

#include "game/types.h"

// One tab of the keyboard: its hit rectangle, its UTF-16 key table and its grid size.
struct KeyboardPage {
    int x0, y0, x1, y1;
    unsigned short* text;
    int columns, rows;
};

struct CodePair {
    unsigned short from, to;
};

// A modifier table: maps a code unit to the code unit it changes into.
struct CodeTable {
    unsigned short code, pad;
    int reserved04;
    CodePair* pairs;
    int reserved0C, reserved10, count;
};

// The on-screen text keyboard. Text is UTF-16, stored inline or on the heap depending
// on bit 31 of textStorageFlags.
class CSoftKeyboard {
public:
    int vfunc_4();
    int vfunc_3();
    int vfunc_5();

    unsigned int GetTextLength();
    int InitHoverOffsets();
    int UpdateInput();

    void* vtbl;               // 0x00
    float pointerX, pointerY; // 0x04
    union {
        struct {
            unsigned int textStorageFlags; // 0x0C
            unsigned int heapTextLength;   // 0x10
            unsigned short* heapText;      // 0x14
        };
        struct {
            unsigned short stringHeader;  // 0x0C
            unsigned short inlineText[5]; // 0x0E
        };
    };
    struct Position {
        int x, y;
    }* keyCoordinates;                       // 0x18
    int* characterPages;                     // 0x1C
    unsigned char* characterModifiers;       // 0x20
    unsigned int hoverState, hoverX, hoverY; // 0x24
    char reserved30[4];
    int cursorIndex, pageIndex, textLength;     // 0x34
    int gridX0, gridY0, gridX1, gridY1;         // 0x40
    int acceptX0, acceptY0, acceptX1, acceptY1; // 0x50
    int deleteX0, deleteY0, deleteX1, deleteY1; // 0x60
    char reserved70[0x10];
    KeyboardPage* pages; // 0x80
    char reserved84[8];
    CodeTable* modifierTables;       // 0x8C
    unsigned int modifierTableCount; // 0x90
    char reserved94[8];
    int maxCharacters;      // 0x9C
    int pageCount;          // 0xA0
    float offsetX, offsetY; // 0xA4
};

#define CSOFTKEYBOARD_OFFSET(field, offset) \
    typedef char CSoftKeyboard_offset_##field[((int)&((CSoftKeyboard*)0)->field == (offset)) ? 1 : -1]
CSOFTKEYBOARD_OFFSET(pointerX, 0x04);
CSOFTKEYBOARD_OFFSET(textStorageFlags, 0x0C);
CSOFTKEYBOARD_OFFSET(keyCoordinates, 0x18);
CSOFTKEYBOARD_OFFSET(hoverState, 0x24);
CSOFTKEYBOARD_OFFSET(cursorIndex, 0x34);
CSOFTKEYBOARD_OFFSET(gridX0, 0x40);
CSOFTKEYBOARD_OFFSET(pages, 0x80);
CSOFTKEYBOARD_OFFSET(maxCharacters, 0x9C);
CSOFTKEYBOARD_OFFSET(offsetX, 0xA4);
#undef CSOFTKEYBOARD_OFFSET

#endif // CLASSES_CSOFTKEYBOARD_H

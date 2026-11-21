// NoticeDisplay: explicit layout view; no inheritance, virtual methods, or RTTI.
// Display view manipulated by NoticeQueue_UpdateAndDraw/801865C8/8018666C; class name is descriptive.
#ifndef CLASSES_NOTICEDISPLAY_H
#define CLASSES_NOTICEDISPLAY_H

#include "game/types.h"

class NoticeDisplay {
public:
    char pad_000[0x4];
    unsigned int visible; // 0x4: NoticeQueue_UpdateAndDraw: gates WOverlay_DrawTransformedRect
    unsigned int dirty; // 0x8: NoticeQueue_UpdateAndDraw: set on content change, cleared after layout
    void* textRenderer; // 0xC: NoticeQueue_UpdateAndDraw: Text_SetContent/Text_DrawGlyphs receiver
    void* drawResource; // 0x10: NoticeQueue_UpdateAndDraw: WOverlay_DrawTransformedRect first argument
    unsigned int stringFlags; // 0x14: NoticeQueue_UpdateAndDraw: high bit selects heap string storage
    unsigned int stringLength; // 0x18: NoticeQueue_UpdateAndDraw: heap string length
    void* stringData; // 0x1C: NoticeQueue_UpdateAndDraw: heap string buffer
    float firstDrawTransform[4]; // 0x20: NoticeQueue_UpdateAndDraw: first WOverlay_DrawTransformedRect transform
    float secondDrawTransform[4]; // 0x30: NoticeQueue_UpdateAndDraw: second WOverlay_DrawTransformedRect transform
    float thirdDrawTransform[4]; // 0x40: NoticeQueue_UpdateAndDraw: third WOverlay_DrawTransformedRect transform
    float x; // 0x50: NoticeQueue_UpdateAndDraw: text position x plus margin
    float y; // 0x54: NoticeQueue_UpdateAndDraw: text position y plus margin
    char pad_058[0x4];
    float firstSectionHeight; // 0x5C: NoticeQueue_UpdateAndDraw: contributes to vertical bounds
    float secondSectionX; // 0x60: NoticeQueue_UpdateAndDraw: second WOverlay_DrawTransformedRect position starts here
    float secondSectionY; // 0x64: NoticeQueue_UpdateAndDraw: y + firstSectionHeight
    char pad_068[0x4];
    float secondSectionHeight; // 0x6C: NoticeQueue_UpdateAndDraw: measured text extent plus margin
    float thirdSectionX; // 0x70: NoticeQueue_UpdateAndDraw: third WOverlay_DrawTransformedRect position starts here
    float thirdSectionY; // 0x74: NoticeQueue_UpdateAndDraw: secondSectionY + secondSectionHeight
};

#define OFF(f, o) typedef char NoticeDisplay_off_##f[((int)&((NoticeDisplay*)0)->f == (o)) ? 1 : -1]
OFF(visible, 0x4);
OFF(dirty, 0x8);
OFF(textRenderer, 0xC);
OFF(drawResource, 0x10);
OFF(stringFlags, 0x14);
OFF(stringLength, 0x18);
OFF(stringData, 0x1C);
OFF(firstDrawTransform, 0x20);
OFF(secondDrawTransform, 0x30);
OFF(thirdDrawTransform, 0x40);
OFF(x, 0x50);
OFF(y, 0x54);
OFF(firstSectionHeight, 0x5C);
OFF(secondSectionX, 0x60);
OFF(secondSectionY, 0x64);
OFF(secondSectionHeight, 0x6C);
OFF(thirdSectionX, 0x70);
OFF(thirdSectionY, 0x74);
#undef OFF

#endif

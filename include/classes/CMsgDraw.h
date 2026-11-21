// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CMSGDRAW_H
#define CLASSES_CMSGDRAW_H

#include "game/types.h"

class CMsgDraw {
public:
    void* vtbl;                         // 0x0: original vtable pointer
    void* fontAtlas;                        // 0x4: 1 use in CMsgDraw_Ctor
    char pad_008[0x4];                  // 0x8
    int contentStorage;                          // 0xc: 1 use in CMsgDraw_Ctor
    int contentHeapLength;                         // 0x10: 1 use in CMsgDraw_Ctor
    int contentHeapData;                         // 0x14: 1 use in CMsgDraw_Ctor
    int linePointers;                         // 0x18: 1 use in CMsgDraw_Ctor
    int lineCount;                         // 0x1c: 1 use in CMsgDraw_Ctor
    int lineCapacity;                         // 0x20: 1 use in CMsgDraw_Ctor
    int lineIndex;                         // 0x24: 1 use in CMsgDraw_Ctor
    int parsedLineCount;                // 0x28: CMsgDraw_ParseFormattedText starts at 1 and increments on newline
    int unk_2C;                         // 0x2c: 1 use in CMsgDraw_Ctor
    float revealPosition;                       // 0x30: 1 use in CMsgDraw_Ctor
    float revealStep;                       // 0x34: 1 use in CMsgDraw_Ctor
    float originX;                       // 0x38: 1 use in CMsgDraw_Ctor
    float originY;                       // 0x3c: 1 use in CMsgDraw_Ctor
    float cursorX;                       // 0x40: 1 use in CMsgDraw_Ctor
    float cursorY;                       // 0x44: 1 use in CMsgDraw_Ctor
    float layoutWidth;                       // 0x48: 1 use in CMsgDraw_Ctor
    float layoutHeight;                       // 0x4c: 1 use in CMsgDraw_Ctor
    float glyphScaleX;                       // 0x50: 1 use in CMsgDraw_Ctor
    float glyphScaleY;                       // 0x54: 1 use in CMsgDraw_Ctor
    float textWidth;                       // 0x58: 1 use in CMsgDraw_Ctor
    float textHeight;                       // 0x5c: 1 use in CMsgDraw_Ctor
    int defaultColor;                         // 0x60: 1 use in CMsgDraw_Ctor
    int currentColor;                         // 0x64: 1 use in CMsgDraw_Ctor
    int colorMultiplier;                         // 0x68: 1 use in CMsgDraw_Ctor
    int revealDelayFrames;                         // 0x6c: 1 use in CMsgDraw_Ctor
    int revealDelayTagCount;                         // 0x70: 1 use in CMsgDraw_Ctor
    int revealDelayTreeRoot;                         // 0x74: 1 use in CMsgDraw_Ctor
    void* firstRevealDelayNode;                       // 0x78: 1 use in CMsgDraw_Ctor
    void* glyphs;                       // 0x7c: 2 uses in CMsgDraw_dtor, CMsgDraw_Ctor
    int glyphCount;                         // 0x80: 1 use in CMsgDraw_Ctor
    int soundTagCount;                         // 0x84: 1 use in CMsgDraw_Ctor
    int soundTagTreeRoot;                         // 0x88: 1 use in CMsgDraw_Ctor
    char pad_08C[0x4];                  // 0x8c
    void* firstSoundTagNode;                       // 0x90: 1 use in CMsgDraw_Ctor
    int drawSuppressed;                         // 0x94: 1 use in CMsgDraw_Ctor
    int nextLineY;                         // 0x98: 1 use in CMsgDraw_Ctor
    int revealSoundStorage;                         // 0x9c: 1 use in CMsgDraw_Ctor
    int revealSoundHeapLength;                         // 0xa0: 1 use in CMsgDraw_Ctor
    int revealSoundHeapData;                         // 0xa4: 1 use in CMsgDraw_Ctor
    int lastSoundGlyphCount;                         // 0xa8: 1 use in CMsgDraw_Ctor
    char pad_0AC[0x400];                // 0xac
    int defaultRevealSoundStorage;                        // 0x4ac: 1 use in CMsgDraw_Ctor
    int defaultRevealSoundHeapLength;                        // 0x4b0: 1 use in CMsgDraw_Ctor
    int defaultRevealSoundHeapData;                        // 0x4b4: 1 use in CMsgDraw_Ctor
    int drawEnabled;                        // 0x4b8: 1 use in CMsgDraw_Ctor

    // offsets whose address is passed on (sub-objects or arrays, not placed): 0xc, 0x18, 0x70, 0x74, 0x84, 0x88, 0x9c, 0x4ac
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define CMSGDRAW_OFF(f, o) typedef char CMsgDraw_off_##f[((int)&((CMsgDraw*)0)->f == (o)) ? 1 : -1]
CMSGDRAW_OFF(fontAtlas, 0x4);
CMSGDRAW_OFF(contentStorage, 0xc);
CMSGDRAW_OFF(contentHeapLength, 0x10);
CMSGDRAW_OFF(contentHeapData, 0x14);
CMSGDRAW_OFF(linePointers, 0x18);
CMSGDRAW_OFF(lineCount, 0x1c);
CMSGDRAW_OFF(lineCapacity, 0x20);
CMSGDRAW_OFF(lineIndex, 0x24);
CMSGDRAW_OFF(unk_2C, 0x2c);
CMSGDRAW_OFF(revealPosition, 0x30);
CMSGDRAW_OFF(revealStep, 0x34);
CMSGDRAW_OFF(originX, 0x38);
CMSGDRAW_OFF(originY, 0x3c);
CMSGDRAW_OFF(cursorX, 0x40);
CMSGDRAW_OFF(cursorY, 0x44);
CMSGDRAW_OFF(layoutWidth, 0x48);
CMSGDRAW_OFF(layoutHeight, 0x4c);
CMSGDRAW_OFF(glyphScaleX, 0x50);
CMSGDRAW_OFF(glyphScaleY, 0x54);
CMSGDRAW_OFF(textWidth, 0x58);
CMSGDRAW_OFF(textHeight, 0x5c);
CMSGDRAW_OFF(defaultColor, 0x60);
CMSGDRAW_OFF(currentColor, 0x64);
CMSGDRAW_OFF(colorMultiplier, 0x68);
CMSGDRAW_OFF(revealDelayFrames, 0x6c);
CMSGDRAW_OFF(revealDelayTagCount, 0x70);
CMSGDRAW_OFF(revealDelayTreeRoot, 0x74);
CMSGDRAW_OFF(firstRevealDelayNode, 0x78);
CMSGDRAW_OFF(glyphs, 0x7c);
CMSGDRAW_OFF(glyphCount, 0x80);
CMSGDRAW_OFF(soundTagCount, 0x84);
CMSGDRAW_OFF(soundTagTreeRoot, 0x88);
CMSGDRAW_OFF(firstSoundTagNode, 0x90);
CMSGDRAW_OFF(drawSuppressed, 0x94);
CMSGDRAW_OFF(parsedLineCount, 0x28);
CMSGDRAW_OFF(nextLineY, 0x98);
CMSGDRAW_OFF(revealSoundStorage, 0x9c);
CMSGDRAW_OFF(revealSoundHeapLength, 0xa0);
CMSGDRAW_OFF(revealSoundHeapData, 0xa4);
CMSGDRAW_OFF(lastSoundGlyphCount, 0xa8);
CMSGDRAW_OFF(defaultRevealSoundStorage, 0x4ac);
CMSGDRAW_OFF(defaultRevealSoundHeapLength, 0x4b0);
CMSGDRAW_OFF(defaultRevealSoundHeapData, 0x4b4);
CMSGDRAW_OFF(drawEnabled, 0x4b8);
#undef CMSGDRAW_OFF

#endif  // CLASSES_CMSGDRAW_H

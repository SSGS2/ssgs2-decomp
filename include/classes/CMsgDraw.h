#ifndef CLASSES_CMSGDRAW_H
#define CLASSES_CMSGDRAW_H

#include "game/types.h"

class CMsgDraw {
public:
    void* vtbl;                       // 0x0: original vtable pointer
    void* fontAtlas;                  // 0x4
    char pad_008[0x4];                // 0x8
    int contentStorage;               // 0xc
    int contentHeapLength;            // 0x10
    int contentHeapData;              // 0x14
    int linePointers;                 // 0x18
    int lineCount;                    // 0x1c
    int lineCapacity;                 // 0x20
    int lineIndex;                    // 0x24
    int parsedLineCount;              // 0x28: ParseFormattedText starts at 1 and increments on newline
    int unk_2C;                       // 0x2c
    float revealPosition;             // 0x30
    float revealStep;                 // 0x34
    float originX;                    // 0x38
    float originY;                    // 0x3c
    float cursorX;                    // 0x40
    float cursorY;                    // 0x44
    float layoutWidth;                // 0x48
    float layoutHeight;               // 0x4c
    float glyphScaleX;                // 0x50
    float glyphScaleY;                // 0x54
    float textWidth;                  // 0x58
    float textHeight;                 // 0x5c
    int defaultColor;                 // 0x60
    int currentColor;                 // 0x64
    int colorMultiplier;              // 0x68
    int revealDelayFrames;            // 0x6c
    int revealDelayTagCount;          // 0x70
    int revealDelayTreeRoot;          // 0x74
    void* firstRevealDelayNode;       // 0x78
    void* glyphs;                     // 0x7c
    int glyphCount;                   // 0x80
    int soundTagCount;                // 0x84
    int soundTagTreeRoot;             // 0x88
    char pad_08C[0x4];                // 0x8c
    void* firstSoundTagNode;          // 0x90
    int drawSuppressed;               // 0x94
    int nextLineY;                    // 0x98
    int revealSoundStorage;           // 0x9c
    int revealSoundHeapLength;        // 0xa0
    int revealSoundHeapData;          // 0xa4
    int lastSoundGlyphCount;          // 0xa8
    char pad_0AC[0x400];              // 0xac
    int defaultRevealSoundStorage;    // 0x4ac
    int defaultRevealSoundHeapLength; // 0x4b0
    int defaultRevealSoundHeapData;   // 0x4b4
    int drawEnabled;                  // 0x4b8

    static void* GetOrInsertFormattedTextEntry(void* tree, int* key);
    void SetAllGlyphColors(int rgba);
    int HasNoNextLine();
    int MeasureText(unsigned short* text, float* rect);
    bool GetTextExtent(float* extent);
    bool AppendGlyph(unsigned int code, float lineHeight);
    bool SetColor(int rgba);
    bool SetDrawEnabled(int enabled);
    int IsRevealComplete() const;
    int SetRevealStep(int restartReveal, float glyphsPerUpdate);
    int UpdateReveal();
    void SetDefaultRevealSound(const char* text);
    int ClearFormattingAndRevealAll();
    int ResetTextState();
    int NextLine();
    int PreviousLine();
    int ParseFormattedText(unsigned short* text);
    bool SetLayoutSize(float* size);
    bool SetGlyphScale(float* scale);
    static int* ParseNumericArgument(void* arg0, unsigned short* text, int* output);
    static int FindFormattingToken(unsigned short* first, unsigned short* excluded, unsigned short** out);
    static void EraseFormattingMapEntry(void** result, void* tree, void** nodePtr);

    ~CMsgDraw();
    CMsgDraw(void* fontAtlas);
};

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

#endif // CLASSES_CMSGDRAW_H

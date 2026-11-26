// Partial layout view owned by tour_hint_message_window.cpp; no constructors, vtable emission or RTTI.
#ifndef CLASSES_CTOURMESSAGE_H
#define CLASSES_CTOURMESSAGE_H
// Packed wide-string view: with flags >= 0 the UTF-16 text starts at +2, otherwise longText
// holds it. The text is NUL terminated.
struct TourMessageText {
    unsigned int flags;
    unsigned int length;
    void* longText;
};

struct CTourMessage {
    void* vtbl;         // 0x00
    int revealComplete; // 0x04
    char pad_08[8];     // 0x08
    int baseGlyphColor; // 0x10  RGB mask combined with the glyph opacity
    int inputMode;      // 0x14
    char pad_18[0x28];  // 0x18
    int fadeState;      // 0x40  4 decreases opacity, 5 increases it, 3 means fade complete

    void SetBaseGlyphColor(int value);
    void SetInputMode(int value);
    void ApplyGlyphColorMask(int value);
    int ProcessInput(int allowRevealSkip);
    void SetText(TourMessageText* text, void* style);

    CTourMessage();
};
#define OFF(f, o) typedef char CTourMessage_off_##f[((int)&((CTourMessage*)0)->f == (o)) ? 1 : -1]
OFF(revealComplete, 0x04);
OFF(baseGlyphColor, 0x10);
OFF(inputMode, 0x14);
OFF(fadeState, 0x40);
#undef OFF
#endif

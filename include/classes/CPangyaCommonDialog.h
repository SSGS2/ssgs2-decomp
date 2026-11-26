// CPangyaCommonDialog: the modal Yes/No/OK window.
//
// It owns a button layout, a background and a dialog Bp entry, the UTF-16 text
// it displays, a tag expander and the message renderer, and runs the fade that
// shows and hides the window. Button input is polled once per process call and
// produces an accepted/cancelled result.
//
// The class is declared without virtual functions on purpose: its vtable is
// still taken from the original game data, so `vtbl` is spelled out as a member.
#ifndef CLASSES_CPANGYACOMMONDIALOG_H
#define CLASSES_CPANGYACOMMONDIALOG_H

#include "game/types.h"

class CPangyaCommonDialog {
public:
    // Lifecycle. Construction blocks until the scripts and font are loaded.
    CPangyaCommonDialog(void* kind, int fadeDuration);
    ~CPangyaCommonDialog();

    // Construction-only: wait for the resources, build the text, then give the
    // global script handle back.
    void FinishLoading();

    // Per-frame work: advance the fade and input delay, then read the buttons.
    // Returns nonzero once a result has been produced.
    int UpdateFadeAndInput();
    void StartFade(int fadeIn, int durationFrames);

    // Draw the background, the window and the text.
    void Draw();

    // Placement, opacity, text and button layout.
    void SetPosition(int x, int y);
    void MoveCursorToButton(int index);
    void ApplyOpacity(int value);
    void SetWindowEnabled(int enabled);
    void SetText(void* taggedText);
    void ConfigureButtonLayout();

    void* vtbl;             // 0x00
    int kind;               // 0x04
    int result;             // 0x08  0 pending, 1 accepted, 2 cancelled
    int inputDelay;         // 0x0C  process calls left before input resumes
    void* layout;           // 0x10
    int backgroundEntryId;  // 0x14
    int dialogEntryId;      // 0x18
    void* message;          // 0x1C  owned message renderer
    void* tagExpander;      // 0x20  owned tag expander
    unsigned int textFlags; // 0x24  WString header; high bit means heap storage
    int textLength;         // 0x28
    void* textData;         // 0x2C
    int fading;             // 0x30
    int opacity;            // 0x34
    int opacityStep;        // 0x38
    int fadeDuration;       // 0x3C
    int savedScriptIndex;   // 0x40
};

#define CPANGYACOMMONDIALOG_OFF(f, o) \
    typedef char CPangyaCommonDialog_off_##f[((int)&((CPangyaCommonDialog*)0)->f == (o)) ? 1 : -1]
CPANGYACOMMONDIALOG_OFF(kind, 0x04);
CPANGYACOMMONDIALOG_OFF(result, 0x08);
CPANGYACOMMONDIALOG_OFF(layout, 0x10);
CPANGYACOMMONDIALOG_OFF(message, 0x1C);
CPANGYACOMMONDIALOG_OFF(textFlags, 0x24);
CPANGYACOMMONDIALOG_OFF(fading, 0x30);
CPANGYACOMMONDIALOG_OFF(savedScriptIndex, 0x40);
#undef CPANGYACOMMONDIALOG_OFF
typedef char CPangyaCommonDialog_size[(sizeof(CPangyaCommonDialog) == 0x44) ? 1 : -1];

#endif // CLASSES_CPANGYACOMMONDIALOG_H

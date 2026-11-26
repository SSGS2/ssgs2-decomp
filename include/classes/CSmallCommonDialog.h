#ifndef CLASSES_CSMALLCOMMONDIALOG_H
#define CLASSES_CSMALLCOMMONDIALOG_H

#include "game/types.h"

// A small message dialog with YES/NO or single OK buttons. It is built on a
// CInGameDialog base (the first 0x4C bytes of the object).
class CSmallCommonDialog {
public:
    CSmallCommonDialog(void* a, int extra);

    // Allocate a dialog and store it through out. Returns 1 when it was created.
    static int Create(void* a, void** out, int extra);
    static void* IsSharedResourcesLoaded();
    void* GetDialogResult();
    void SetDialogResult(int value);
    // Release the dialog's resources and restore the caption script; frees the object when deleting is positive.
    void* Destroy(int deleting);

    int UpdateDialog(float value);
    int InvokeResultCallback();
    void UpdateInputAndFeedback();
    void Draw();
    void LoadAndInitializeLayout();
    void ConfigureButtons();
    void SetExpandedWideText(int param);
    void SetCallback(unsigned int* source, unsigned int value);

    void* vtbl;                          // 0x0: original vtable pointer
    void* dialogVtable;                  // 0x4
    char pad_008[0x10];                  // 0x8
    int dialogState;                     // 0x18
    char pad_01C[0x8];                   // 0x1c
    float opacity;                       // 0x24
    char pad_028[0x24];                  // 0x28
    int singleButtonMode;                // 0x4c
    int result;                          // 0x50
    unsigned int callbackThisAdjustment; // 0x54
    unsigned int callbackVtableOffset;   // 0x58
    unsigned int callbackFunction;       // 0x5c
    unsigned int callbackTarget;         // 0x60
    void* layout;                        // 0x64
    void* textExpansionContext;          // 0x68
    int bpEntryId;                       // 0x6c
    int selectionDelayFrames;            // 0x70
    int savedScriptIndex;                // 0x74
};

#define CSMALLCOMMONDIALOG_OFF(f, o) \
    typedef char CSmallCommonDialog_off_##f[((int)&((CSmallCommonDialog*)0)->f == (o)) ? 1 : -1]
CSMALLCOMMONDIALOG_OFF(dialogVtable, 0x4);
CSMALLCOMMONDIALOG_OFF(dialogState, 0x18);
CSMALLCOMMONDIALOG_OFF(opacity, 0x24);
CSMALLCOMMONDIALOG_OFF(singleButtonMode, 0x4c);
CSMALLCOMMONDIALOG_OFF(result, 0x50);
CSMALLCOMMONDIALOG_OFF(callbackThisAdjustment, 0x54);
CSMALLCOMMONDIALOG_OFF(callbackVtableOffset, 0x58);
CSMALLCOMMONDIALOG_OFF(callbackFunction, 0x5c);
CSMALLCOMMONDIALOG_OFF(callbackTarget, 0x60);
CSMALLCOMMONDIALOG_OFF(layout, 0x64);
CSMALLCOMMONDIALOG_OFF(textExpansionContext, 0x68);
CSMALLCOMMONDIALOG_OFF(bpEntryId, 0x6c);
CSMALLCOMMONDIALOG_OFF(selectionDelayFrames, 0x70);
CSMALLCOMMONDIALOG_OFF(savedScriptIndex, 0x74);
#undef CSMALLCOMMONDIALOG_OFF

#endif // CLASSES_CSMALLCOMMONDIALOG_H

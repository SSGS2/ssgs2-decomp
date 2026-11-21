// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CSMALLCOMMONDIALOG_H
#define CLASSES_CSMALLCOMMONDIALOG_H

#include "game/types.h"

class CSmallCommonDialog {
public:
    void* vtbl;                         // 0x0: original vtable pointer
    void* dialogVtable;                        // 0x4: 1 use in CSmallCommonDialog_dtor
    char pad_008[0x10];                 // 0x8
    int dialogState;                         // 0x18: 2 uses in CSmallCommonDialog_UpdateInputAndFeedback, CSmallCommonDialog_UpdateDialog
    char pad_01C[0x8];                  // 0x1c
    float opacity;                       // 0x24: 3 uses in CSmallCommonDialog_Draw
    char pad_028[0x24];                 // 0x28
    int singleButtonMode;                         // 0x4c: 1 use in CSmallCommonDialog_ConfigureButtons
    int result;                         // 0x50: 5 uses in CSmallCommonDialog_UpdateInputAndFeedback, CSmallCommonDialog_vfunc_16, CSmallCommonDialog_vfunc_20
    unsigned int callbackThisAdjustment;                // 0x54: 1 use in CSmallCommonDialog_SetCallback
    unsigned int callbackVtableOffset;                // 0x58: 1 use in CSmallCommonDialog_SetCallback
    unsigned int callbackFunction;                // 0x5c: 1 use in CSmallCommonDialog_SetCallback
    unsigned int callbackTarget;                // 0x60: 1 use in CSmallCommonDialog_SetCallback
    void* layout;                       // 0x64: 20 uses in CSmallCommonDialog_ConfigureButtons, CSmallCommonDialog_UpdateInputAndFeedback, CSmallCommonDialog_LoadAndInitializeLayout...
    void* textExpansionContext;                       // 0x68: 3 uses in CSmallCommonDialog_dtor, CSmallCommonDialog_SetExpandedWideText
    int bpEntryId;                         // 0x6c: 10 uses in CSmallCommonDialog_dtor, CSmallCommonDialog_Draw, CSmallCommonDialog_LoadAndInitializeLayout
    int selectionDelayFrames;                         // 0x70: 3 uses in CSmallCommonDialog_UpdateInputAndFeedback
    int savedScriptIndex;                         // 0x74: 2 uses in CSmallCommonDialog_dtor, CSmallCommonDialog_LoadAndInitializeLayout

    // offsets whose address is passed on (sub-objects or arrays, not placed): 0x54, 0x68
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define CSMALLCOMMONDIALOG_OFF(f, o) typedef char CSmallCommonDialog_off_##f[((int)&((CSmallCommonDialog*)0)->f == (o)) ? 1 : -1]
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

#endif  // CLASSES_CSMALLCOMMONDIALOG_H

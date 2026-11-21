// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CINGAMETUTORIALDIALOG_H
#define CLASSES_CINGAMETUTORIALDIALOG_H

#include "game/types.h"

class CInGameTutorialDialog {
public:
    void* vtbl;                         // 0x0: original vtable pointer
    void* dialogVtable;                        // 0x4: 2 uses in CInGameTutorialDialog_Ctor
    float positionX;                        // 0x8: 2 uses in CInGameTutorialDialog_InitGroups, CInGameTutorialDialog_RefreshLayoutVisibility
    float positionY;                        // 0xc: 2 uses in CInGameTutorialDialog_InitGroups, CInGameTutorialDialog_RefreshLayoutVisibility
    float tutorialWidth;                       // 0x10: 1 use in CInGameTutorialDialog_Ctor
    float tutorialHeight;                       // 0x14: 1 use in CInGameTutorialDialog_Ctor
    int dialogState;                         // 0x18: 1 use in CInGameTutorialDialog_UpdateButtonSelection
    char pad_01C[0x8];                  // 0x1c
    float opacity;                       // 0x24: 2 uses in CInGameTutorialDialog_DrawDialogWithText
    char pad_028[0x24];                 // 0x28
    int tutorialIndex;                         // 0x4c: 11 uses in CInGameTutorialDialog_RefreshLayoutVisibility, CInGameTutorialDialog_Ctor, CInGameTutorialDialog_DrawDialogWithText
    int selectionResult;                         // 0x50: 2 uses in CInGameTutorialDialog_Ctor, CInGameTutorialDialog_UpdateButtonSelection
    int callbackThisAdjustment;                         // 0x54: 1 use in CInGameTutorialDialog_Ctor
    int callbackVtableOffset;                         // 0x58: 1 use in CInGameTutorialDialog_Ctor
    int callbackFunction;                         // 0x5c: 1 use in CInGameTutorialDialog_Ctor
    int callbackTarget;                         // 0x60: 1 use in CInGameTutorialDialog_Ctor
    int unk_64;                         // 0x64: 1 use in CInGameTutorialDialog_Ctor
    float cursorGroup0X;                       // 0x68: 1 use in CInGameTutorialDialog_InitGroups
    float cursorGroup0Y;                       // 0x6c: 1 use in CInGameTutorialDialog_InitGroups
    int cursorGroup0PlayerFlag;                         // 0x70: 2 uses in CInGameTutorialDialog_InitGroups, CInGameTutorialDialog_RefreshLayoutVisibility
    int cursorGroup0Enabled;                         // 0x74: 2 uses in CInGameTutorialDialog_InitGroups, CInGameTutorialDialog_RefreshLayoutVisibility
    float cursorGroup1X;                       // 0x78: 1 use in CInGameTutorialDialog_InitGroups
    float cursorGroup1Y;                       // 0x7c: 1 use in CInGameTutorialDialog_InitGroups
    int cursorGroup1PlayerFlag;                         // 0x80: 2 uses in CInGameTutorialDialog_InitGroups, CInGameTutorialDialog_RefreshLayoutVisibility
    int cursorGroup1Enabled;                         // 0x84: 2 uses in CInGameTutorialDialog_InitGroups, CInGameTutorialDialog_RefreshLayoutVisibility
    float elementGroup0X;                       // 0x88: 1 use in CInGameTutorialDialog_InitGroups
    float elementGroup0Y;                       // 0x8c: 1 use in CInGameTutorialDialog_InitGroups
    int elementGroup0PlayerFlag;                         // 0x90: 1 use in CInGameTutorialDialog_InitGroups
    int elementGroup0Enabled;                         // 0x94: 2 uses in CInGameTutorialDialog_InitGroups, CInGameTutorialDialog_RefreshLayoutVisibility
    float elementGroup1X;                       // 0x98: 1 use in CInGameTutorialDialog_InitGroups
    float elementGroup1Y;                       // 0x9c: 1 use in CInGameTutorialDialog_InitGroups
    int elementGroup1PlayerFlag;                         // 0xa0: 1 use in CInGameTutorialDialog_InitGroups
    int elementGroup1Enabled;                         // 0xa4: 1 use in CInGameTutorialDialog_InitGroups
    float elementGroup2X;                       // 0xa8: 1 use in CInGameTutorialDialog_InitGroups
    float elementGroup2Y;                       // 0xac: 1 use in CInGameTutorialDialog_InitGroups
    int elementGroup2PlayerFlag;                         // 0xb0: 1 use in CInGameTutorialDialog_InitGroups
    int elementGroup2Enabled;                         // 0xb4: 1 use in CInGameTutorialDialog_InitGroups
    float elementGroup3X;                       // 0xb8: 1 use in CInGameTutorialDialog_InitGroups
    float elementGroup3Y;                       // 0xbc: 1 use in CInGameTutorialDialog_InitGroups
    int elementGroup3PlayerFlag;                         // 0xc0: 1 use in CInGameTutorialDialog_InitGroups
    int elementGroup3Enabled;                         // 0xc4: 1 use in CInGameTutorialDialog_InitGroups
    float elementGroup4X;                       // 0xc8: 1 use in CInGameTutorialDialog_InitGroups
    float elementGroup4Y;                       // 0xcc: 1 use in CInGameTutorialDialog_InitGroups
    int elementGroup4PlayerFlag;                         // 0xd0: 1 use in CInGameTutorialDialog_InitGroups
    int elementGroup4Enabled;                         // 0xd4: 1 use in CInGameTutorialDialog_InitGroups

    // offsets whose address is passed on (sub-objects or arrays, not placed): 0x54, 0x68, 0x78, 0x88
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define CINGAMETUTORIALDIALOG_OFF(f, o) typedef char CInGameTutorialDialog_off_##f[((int)&((CInGameTutorialDialog*)0)->f == (o)) ? 1 : -1]
CINGAMETUTORIALDIALOG_OFF(dialogVtable, 0x4);
CINGAMETUTORIALDIALOG_OFF(positionX, 0x8);
CINGAMETUTORIALDIALOG_OFF(positionY, 0xc);
CINGAMETUTORIALDIALOG_OFF(tutorialWidth, 0x10);
CINGAMETUTORIALDIALOG_OFF(tutorialHeight, 0x14);
CINGAMETUTORIALDIALOG_OFF(dialogState, 0x18);
CINGAMETUTORIALDIALOG_OFF(opacity, 0x24);
CINGAMETUTORIALDIALOG_OFF(tutorialIndex, 0x4c);
CINGAMETUTORIALDIALOG_OFF(selectionResult, 0x50);
CINGAMETUTORIALDIALOG_OFF(callbackThisAdjustment, 0x54);
CINGAMETUTORIALDIALOG_OFF(callbackVtableOffset, 0x58);
CINGAMETUTORIALDIALOG_OFF(callbackFunction, 0x5c);
CINGAMETUTORIALDIALOG_OFF(callbackTarget, 0x60);
CINGAMETUTORIALDIALOG_OFF(unk_64, 0x64);
CINGAMETUTORIALDIALOG_OFF(cursorGroup0X, 0x68);
CINGAMETUTORIALDIALOG_OFF(cursorGroup0Y, 0x6c);
CINGAMETUTORIALDIALOG_OFF(cursorGroup0PlayerFlag, 0x70);
CINGAMETUTORIALDIALOG_OFF(cursorGroup0Enabled, 0x74);
CINGAMETUTORIALDIALOG_OFF(cursorGroup1X, 0x78);
CINGAMETUTORIALDIALOG_OFF(cursorGroup1Y, 0x7c);
CINGAMETUTORIALDIALOG_OFF(cursorGroup1PlayerFlag, 0x80);
CINGAMETUTORIALDIALOG_OFF(cursorGroup1Enabled, 0x84);
CINGAMETUTORIALDIALOG_OFF(elementGroup0X, 0x88);
CINGAMETUTORIALDIALOG_OFF(elementGroup0Y, 0x8c);
CINGAMETUTORIALDIALOG_OFF(elementGroup0PlayerFlag, 0x90);
CINGAMETUTORIALDIALOG_OFF(elementGroup0Enabled, 0x94);
CINGAMETUTORIALDIALOG_OFF(elementGroup1X, 0x98);
CINGAMETUTORIALDIALOG_OFF(elementGroup1Y, 0x9c);
CINGAMETUTORIALDIALOG_OFF(elementGroup1PlayerFlag, 0xa0);
CINGAMETUTORIALDIALOG_OFF(elementGroup1Enabled, 0xa4);
CINGAMETUTORIALDIALOG_OFF(elementGroup2X, 0xa8);
CINGAMETUTORIALDIALOG_OFF(elementGroup2Y, 0xac);
CINGAMETUTORIALDIALOG_OFF(elementGroup2PlayerFlag, 0xb0);
CINGAMETUTORIALDIALOG_OFF(elementGroup2Enabled, 0xb4);
CINGAMETUTORIALDIALOG_OFF(elementGroup3X, 0xb8);
CINGAMETUTORIALDIALOG_OFF(elementGroup3Y, 0xbc);
CINGAMETUTORIALDIALOG_OFF(elementGroup3PlayerFlag, 0xc0);
CINGAMETUTORIALDIALOG_OFF(elementGroup3Enabled, 0xc4);
CINGAMETUTORIALDIALOG_OFF(elementGroup4X, 0xc8);
CINGAMETUTORIALDIALOG_OFF(elementGroup4Y, 0xcc);
CINGAMETUTORIALDIALOG_OFF(elementGroup4PlayerFlag, 0xd0);
CINGAMETUTORIALDIALOG_OFF(elementGroup4Enabled, 0xd4);
#undef CINGAMETUTORIALDIALOG_OFF

#endif  // CLASSES_CINGAMETUTORIALDIALOG_H

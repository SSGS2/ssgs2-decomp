#ifndef CLASSES_CINGAMETUTORIALDIALOG_H
#define CLASSES_CINGAMETUTORIALDIALOG_H

#include "game/types.h"

struct TutorialDialogValueView;
struct TutorialDialogWordsView;
class CInGameTutorialDialog {
public:
    void* vtbl;                  // 0x0: original vtable pointer
    void* dialogVtable;          // 0x4
    float positionX;             // 0x8
    float positionY;             // 0xc
    float tutorialWidth;         // 0x10
    float tutorialHeight;        // 0x14
    int dialogState;             // 0x18
    char pad_01C[0x8];           // 0x1c
    float opacity;               // 0x24
    char pad_028[0x24];          // 0x28
    int tutorialIndex;           // 0x4c
    int selectionResult;         // 0x50
    int callbackThisAdjustment;  // 0x54
    int callbackVtableOffset;    // 0x58
    int callbackFunction;        // 0x5c
    int callbackTarget;          // 0x60
    int unk_64;                  // 0x64
    float cursorGroup0X;         // 0x68
    float cursorGroup0Y;         // 0x6c
    int cursorGroup0PlayerFlag;  // 0x70
    int cursorGroup0Enabled;     // 0x74
    float cursorGroup1X;         // 0x78
    float cursorGroup1Y;         // 0x7c
    int cursorGroup1PlayerFlag;  // 0x80
    int cursorGroup1Enabled;     // 0x84
    float elementGroup0X;        // 0x88
    float elementGroup0Y;        // 0x8c
    int elementGroup0PlayerFlag; // 0x90
    int elementGroup0Enabled;    // 0x94
    float elementGroup1X;        // 0x98
    float elementGroup1Y;        // 0x9c
    int elementGroup1PlayerFlag; // 0xa0
    int elementGroup1Enabled;    // 0xa4
    float elementGroup2X;        // 0xa8
    float elementGroup2Y;        // 0xac
    int elementGroup2PlayerFlag; // 0xb0
    int elementGroup2Enabled;    // 0xb4
    float elementGroup3X;        // 0xb8
    float elementGroup3Y;        // 0xbc
    int elementGroup3PlayerFlag; // 0xc0
    int elementGroup3Enabled;    // 0xc4
    float elementGroup4X;        // 0xc8
    float elementGroup4Y;        // 0xcc
    int elementGroup4PlayerFlag; // 0xd0
    int elementGroup4Enabled;    // 0xd4

    static void vfunc_16(TutorialDialogValueView* p, unsigned int value);
    static void SetCallback(TutorialDialogWordsView* p, unsigned int* values, unsigned int last);

    void InitGroups();
    void ResetAndApplyContent(void* arg1, void* arg2);
    void PositionPageElements(float* arg1, float* arg2);
    void PositionSecondaryElements(float* pos);
    void PositionBodyElements(float* pos);
    void* GetSharedLayoutReady();
    int InvokeResultCallback();
    int Create(void** out);
    void SetMovableGroupEnabled(int index, void* value);
    void SetCursorGroupEnabled(int index, void* value);
    void SetCursorGroupPlayerFlag(int index, void* value);
    void SetMovableGroupPosition(int index, float x, float y);
    void SetCursorGroupPosition(int index, float x, float y);
    void MoveCursorToGroup(int index);
    void RefreshLayoutVisibility();
    void UpdateButtonSelection();
    void DrawDialogWithText();
    static int IsEitherButtonActivated();
    void SetElementPositionAndRect(void* elementId, float* inputXY);

    CInGameTutorialDialog(void* arg1);
    ~CInGameTutorialDialog();
};

#define CINGAMETUTORIALDIALOG_OFF(f, o) \
    typedef char CInGameTutorialDialog_off_##f[((int)&((CInGameTutorialDialog*)0)->f == (o)) ? 1 : -1]
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

#endif // CLASSES_CINGAMETUTORIALDIALOG_H

#ifndef CLASSES_CTUTORIAL_H
#define CLASSES_CTUTORIAL_H

#include "game/types.h"

class CTutorial {
public:
    CTutorial();                        // Defined inline in mode_task_factory_creators.cpp.
    void* vtbl;                         // 0x0: original vtable pointer
    char pad_004[0x14];                 // 0x4
    void* secondaryVtable;              // 0x18
    int commandReadOffset;              // 0x1c
    int pendingCommandCount;            // 0x20
    void* commandBlocks;                // 0x24
    int firstCommandBlockIndex;         // 0x28
    int commandBlockCount;              // 0x2c
    unsigned int commandBlockCapacity;  // 0x30
    int stepCountdown;                  // 0x34
    int forcedActionIndex;              // 0x38
    int state;                          // 0x3c
    void* textMapCount;                 // 0x40
    void* textMapRoot;                  // 0x44
    char pad_048[0x4];                  // 0x48
    void* textMapLast;                  // 0x4c
    int aimAlignmentCount;              // 0x50
    float messagePosition[2];           // 0x54: CTutorial_HandleMsg (0x802083AC), message 2 copies a Vec2
    void* messageDialog;                // 0x5c
    void* modeMessageDialog;            // 0x60
    unsigned char firstStageCompletion; // 0x64
    char pad_065[0x3];                  // 0x65
    int completedStageCount;            // 0x68
    char stage0Completed;               // 0x6c
    char stage1Completed;               // 0x6d
    char stage2Completed;               // 0x6e
    char stage3Completed;               // 0x6f
    char stage4Completed;               // 0x70
    char stage5Completed;               // 0x71

    static void* GetRTTI();

    void HandleMsg(const void* event);
    void OnInit();
    void OnLoad();
    void OnDestroy();
    void OnProcess(float dt);

    // Lesson flow.
    void ResetLessonCommands(int stage);
    void BeginStageCompletion();

    static int PollSharedDialogLayout();

    ~CTutorial();

    // Destructor entry for the secondary interface's vtable: its this pointer is
    // 0x18 bytes past the object's start.
    void* DestroyThunkMinus24(int deleting);
};

#define CTUTORIAL_OFF(f, o) typedef char CTutorial_off_##f[((int)&((CTutorial*)0)->f == (o)) ? 1 : -1]
CTUTORIAL_OFF(secondaryVtable, 0x18);
CTUTORIAL_OFF(commandReadOffset, 0x1c);
CTUTORIAL_OFF(pendingCommandCount, 0x20);
CTUTORIAL_OFF(commandBlocks, 0x24);
CTUTORIAL_OFF(firstCommandBlockIndex, 0x28);
CTUTORIAL_OFF(commandBlockCount, 0x2c);
CTUTORIAL_OFF(commandBlockCapacity, 0x30);
CTUTORIAL_OFF(stepCountdown, 0x34);
CTUTORIAL_OFF(forcedActionIndex, 0x38);
CTUTORIAL_OFF(state, 0x3c);
CTUTORIAL_OFF(textMapCount, 0x40);
CTUTORIAL_OFF(textMapRoot, 0x44);
CTUTORIAL_OFF(textMapLast, 0x4c);
CTUTORIAL_OFF(aimAlignmentCount, 0x50);
CTUTORIAL_OFF(messagePosition, 0x54);
CTUTORIAL_OFF(messageDialog, 0x5c);
CTUTORIAL_OFF(modeMessageDialog, 0x60);
CTUTORIAL_OFF(firstStageCompletion, 0x64);
CTUTORIAL_OFF(completedStageCount, 0x68);
CTUTORIAL_OFF(stage0Completed, 0x6c);
CTUTORIAL_OFF(stage1Completed, 0x6d);
CTUTORIAL_OFF(stage2Completed, 0x6e);
CTUTORIAL_OFF(stage3Completed, 0x6f);
CTUTORIAL_OFF(stage4Completed, 0x70);
CTUTORIAL_OFF(stage5Completed, 0x71);
#undef CTUTORIAL_OFF

#endif // CLASSES_CTUTORIAL_H

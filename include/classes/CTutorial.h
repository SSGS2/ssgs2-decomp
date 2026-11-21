// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CTUTORIAL_H
#define CLASSES_CTUTORIAL_H

#include "game/types.h"

class CTutorial {
public:
    CTutorial(); // Defined inline in mode_task_factory_creators.cpp.
    void* vtbl;                         // 0x0: original vtable pointer
    char pad_004[0x14];                 // 0x4
    void* secondaryVtable;                       // 0x18: 1 use in CTutorial_dtor
    int commandReadOffset;                         // 0x1c: 4 uses in CTutorial_OnDestroy
    int pendingCommandCount;                         // 0x20: 4 uses in CTutorial_OnDestroy
    void* commandBlocks;                       // 0x24: 1 use in CTutorial_OnDestroy
    int firstCommandBlockIndex;                         // 0x28: 5 uses in CTutorial_OnDestroy
    int commandBlockCount;                         // 0x2c: 1 use in CTutorial_OnDestroy
    unsigned int commandBlockCapacity;                // 0x30: 1 use in CTutorial_OnDestroy
    int stepCountdown;                         // 0x34: 4 uses in CTutorial_OnProcess
    int forcedActionIndex;                         // 0x38: 2 uses in CTutorial_OnProcess
    int state;                         // 0x3c: 3 uses in CTutorial_HandleMsg, CTutorial_OnProcess
    void* textMapCount;                       // 0x40: 2 uses in CTutorial_OnLoad, CTutorial_OnDestroy
    void* textMapRoot;                       // 0x44: 3 uses in CTutorial_OnDestroy, CTutorial_dtor
    char pad_048[0x4];                  // 0x48
    void* textMapLast;                       // 0x4c: 1 use in CTutorial_OnDestroy
    int aimAlignmentCount;                         // 0x50: 1 use in CTutorial_OnProcess
    float messagePosition[2];           // 0x54: CTutorial_HandleMsg (0x802083AC), message 2 copies a Vec2
    void* messageDialog;                       // 0x5c: 11 uses in CTutorial_dtor, CTutorial_HandleMsg
    void* modeMessageDialog;                       // 0x60: 7 uses in CTutorial_dtor, CTutorial_OnInit
    char firstStageCompletion;                        // 0x64: 1 use in CTutorial_OnInit
    char pad_065[0x3];                  // 0x65
    int completedStageCount;                         // 0x68: 1 use in CTutorial_OnInit
    char stage0Completed;                        // 0x6c: 1 use in CTutorial_OnInit
    char stage1Completed;                        // 0x6d: 1 use in CTutorial_OnInit
    char stage2Completed;                        // 0x6e: 1 use in CTutorial_OnInit
    char stage3Completed;                        // 0x6f: 1 use in CTutorial_OnInit
    char stage4Completed;                        // 0x70: 1 use in CTutorial_OnInit
    char stage5Completed;                        // 0x71: 1 use in CTutorial_OnInit

    // offsets whose address is passed on (sub-objects or arrays, not placed): 0x18, 0x1c, 0x44, 0x54, 0x5c
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
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

#endif  // CLASSES_CTUTORIAL_H

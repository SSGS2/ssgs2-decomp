// Layout view for CComThinkCtrl_ValidateCommandResults/DBA8/DB44 in com_think_ctrl_conditions.cpp; class name unknown.
#ifndef CLASSES_COMMANDSTATE8022D190_H
#define CLASSES_COMMANDSTATE8022D190_H
#include "game/types.h"
class CommandState8022D190 {
public:
    char unk_0[0x24];
    int fileReady; // 0x24: 0x8022DB44 returns early when set, sets after nonzero load.
    int loadedBuffer; // 0x28: 0x8022DB44 stores GetLoadedBuffer result.
    int commandFlags[48]; // 0x2c: 0x8022D23C writes indexed flag after callback.
    int commandResults[48]; // 0xec: 0x8022D23C writes callback result; D190 scans for 2.
    int unk_1AC; // 0x1ac: CComThinkCtrl_EvaluateCheckpoint clears this word.
    int unk_1B0; // 0x1b0: CComThinkCtrl_EvaluateCheckpoint clears this word.
    int unk_1B4; // 0x1b4: CComThinkCtrl_EvaluateCheckpoint clears this word.
    int sharedFile; // 0x1b8: 0x8022DBA8 SharedFile_Create result; DB44 polls it.
    int shotFlags; // 0x1bc: CComThinkCtrl_EvaluateCheckpoint clears this callback out-parameter block.
    float shotCurve; // 0x1c0: CComThinkCtrl_EvaluateCheckpoint initializes to 0.0f.
    float shotSpin; // 0x1c4: CComThinkCtrl_EvaluateCheckpoint initializes to 0.0f.
    char powerShotLevel; // 0x1c8: CComThinkCtrl_EvaluateCheckpoint initializes to zero.
    char difficultyEnabled; // 0x1c9: CComThinkCtrl_EvaluateCheckpoint initializes to one.
    char unk_1CA[2];
    int clubIndex; // 0x1cc: CComThinkCtrl_EvaluateCheckpoint initializes to -1.
};
#define COMMANDSTATE8022D190_OFF(f,o) typedef char CommandState8022D190_off_##f[((int)&((CommandState8022D190*)0)->f == (o)) ? 1 : -1]
COMMANDSTATE8022D190_OFF(unk_0,0x0);
COMMANDSTATE8022D190_OFF(fileReady,0x24);
COMMANDSTATE8022D190_OFF(loadedBuffer,0x28);
COMMANDSTATE8022D190_OFF(commandFlags,0x2c);
COMMANDSTATE8022D190_OFF(commandResults,0xec);
COMMANDSTATE8022D190_OFF(unk_1AC,0x1ac);
COMMANDSTATE8022D190_OFF(unk_1B0,0x1b0);
COMMANDSTATE8022D190_OFF(unk_1B4,0x1b4);
COMMANDSTATE8022D190_OFF(sharedFile,0x1b8);
COMMANDSTATE8022D190_OFF(shotFlags,0x1bc);
COMMANDSTATE8022D190_OFF(shotCurve,0x1c0);
COMMANDSTATE8022D190_OFF(shotSpin,0x1c4);
COMMANDSTATE8022D190_OFF(powerShotLevel,0x1c8);
COMMANDSTATE8022D190_OFF(difficultyEnabled,0x1c9);
COMMANDSTATE8022D190_OFF(unk_1CA,0x1ca);
COMMANDSTATE8022D190_OFF(clubIndex,0x1cc);
#undef COMMANDSTATE8022D190_OFF
#endif

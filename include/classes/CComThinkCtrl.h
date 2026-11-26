// CComThinkCtrl: the computer player's rule-based decision list.
//
// It owns one tree of named conditions and one of named actions. A command file
// supplies the request scripts; each request walks the condition tree and runs
// the action callbacks whose names it matches.
#ifndef CLASSES_CCOMTHINKCTRL_H
#define CLASSES_CCOMTHINKCTRL_H

#include "game/types.h"

// Header of one integer-key command tree. The controller holds a condition tree
// at 0x04 and an action tree at 0x14, both in this form.
struct CommandStringTree {
    int count;
    void* root;
    int sentinelParent;
    void* first;
};

class CComThinkCtrl {
public:
    CComThinkCtrl();
    ~CComThinkCtrl();

    // Results of the last simulated shot. The boost, carry and area-stop
    // condition checks read them as zero (false) or nonzero (true).
    void SetSpecialLandingResult(int value);
    void SetFirstLandingWithinTargetResult(int value);
    void SetFinalWithinTargetResult(int value);

    // One of the 48 condition option results. The caller supplies the index.
    int GetOption(int optionIndex);

    static void* GetCommandState(void* controller);
    static int GetSpecialLandingResult(void* controller);
    static int GetFirstLandingWithinTargetResult(void* controller);
    static int GetFinalWithinTargetResult(void* controller);
    int FindNextEntryByName(void* entryName, int* cursor);

    // Command tables: the named conditions and actions that requests refer to.
    int RegisterCondition(int key, void* callbackWord, const char* text);
    int RegisterAction(int key, void* callbackWord, const char* text);
    static void ClearStringTree(CommandStringTree* tree);

    int ProcessRequest(void* request);
    int EvaluateCheckpoint(const char* input, void* output);
    int ValidateCommandResults();
    int RequestFileLoad(const char* fileName);
    int PollFileLoad();

    void* vtbl;                          // 0x00
    CommandStringTree conditionTree;     // 0x04
    CommandStringTree actionTree;        // 0x14
    int unk_24;                          // 0x24
    int unk_28;                          // 0x28
    char pad_02C[0xC0];                  // 0x2C
    int conditionOptions[48];            // 0xEC
    int specialLandingResult;            // 0x1AC
    int firstLandingWithinTargetResult;  // 0x1B0
    int finalPositionWithinTargetResult; // 0x1B4
    void* sharedFile;                    // 0x1B8
    char pad_1BC[0x14];                  // 0x1BC
};

#define CCOMTHINKCTRL_OFF(f, o) typedef char CComThinkCtrl_off_##f[((int)&((CComThinkCtrl*)0)->f == (o)) ? 1 : -1]
CCOMTHINKCTRL_OFF(conditionTree, 0x04);
CCOMTHINKCTRL_OFF(actionTree, 0x14);
CCOMTHINKCTRL_OFF(unk_24, 0x24);
CCOMTHINKCTRL_OFF(unk_28, 0x28);
CCOMTHINKCTRL_OFF(conditionOptions, 0xEC);
CCOMTHINKCTRL_OFF(specialLandingResult, 0x1AC);
CCOMTHINKCTRL_OFF(firstLandingWithinTargetResult, 0x1B0);
CCOMTHINKCTRL_OFF(finalPositionWithinTargetResult, 0x1B4);
CCOMTHINKCTRL_OFF(sharedFile, 0x1B8);
#undef CCOMTHINKCTRL_OFF
typedef char CommandStringTree_size[(sizeof(CommandStringTree) == 0x10) ? 1 : -1];
typedef char CComThinkCtrl_size[(sizeof(CComThinkCtrl) == 0x1D0) ? 1 : -1];

#endif // CLASSES_CCOMTHINKCTRL_H

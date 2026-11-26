// CGolfTask: the task that owns a golf round.
//
// It builds the round's actor table, follows the loading sequence, and releases
// the round's resources on teardown. Three interface vtables sit at the front;
// their slot layout comes from the original data.
#ifndef CLASSES_CGOLFTASK_H
#define CLASSES_CGOLFTASK_H

#include "game/types.h"

class CGolfTask {
public:
    void* vtable;            // 0x0
    void* secondaryVtable;   // 0x4
    void* tertiaryVtable;    // 0x8
    unsigned char unk_C;     // 0xc
    char pad_00D[0x39b];     // 0xd
    int treeNodeCount;       // 0x3a8
    void* treeRoot;          // 0x3ac
    void* treeSentinel;      // 0x3b0
    float delayTimer;        // 0x3b4
    int pauseScoreMenu;      // 0x3b8
    int messageActive;       // 0x3bc
    int loadingDraw;         // 0x3c0
    int loadingDrawFinished; // 0x3c4
    void* resources[6];      // 0x3c8  six resource owners with deleting virtual destructors
    int variantPreloadFile;  // 0x3e0
    int inGameTexturePack1;  // 0x3e4
    int inGameTexturePack2;  // 0x3e8
    int tutorialTexturePack; // 0x3ec

    void DrawLoadingScreen();
    void RequestRuleResetHoleBallWind();
    static void* GetRTTI();

    // Thunks for the secondary and tertiary task interfaces.
    void vfunc_14Thunk4(void* arg1, void* arg2);
    void* Vfunc1Thunk8(int forwarded1);

    void DisplayActorsByPriority();

    void BeginLoadingAndBuildPauseMenu();
    void LoadPreloadArchive();
    void InitActors();
    void LoadActorsCallback(int loadArgument1, int loadArgument2);
    void UpdatePauseMenuActorsAndScorecard(float delta);
    void ProcessLoadingDraw(float delta);
    int IsLoadingComplete();

    CGolfTask();
    ~CGolfTask();
    void CreateActors();
    static void RequestModeTransition();

    void NotifyActorsDestroy();
};

#define CGOLFTASK_OFF(f, o) typedef char CGolfTask_off_##f[((int)&((CGolfTask*)0)->f == (o)) ? 1 : -1]
CGOLFTASK_OFF(secondaryVtable, 0x4);
CGOLFTASK_OFF(tertiaryVtable, 0x8);
CGOLFTASK_OFF(unk_C, 0xc);
CGOLFTASK_OFF(treeNodeCount, 0x3a8);
CGOLFTASK_OFF(treeRoot, 0x3ac);
CGOLFTASK_OFF(treeSentinel, 0x3b0);
CGOLFTASK_OFF(delayTimer, 0x3b4);
CGOLFTASK_OFF(pauseScoreMenu, 0x3b8);
CGOLFTASK_OFF(messageActive, 0x3bc);
CGOLFTASK_OFF(loadingDraw, 0x3c0);
CGOLFTASK_OFF(loadingDrawFinished, 0x3c4);
CGOLFTASK_OFF(variantPreloadFile, 0x3e0);
CGOLFTASK_OFF(inGameTexturePack1, 0x3e4);
CGOLFTASK_OFF(inGameTexturePack2, 0x3e8);
CGOLFTASK_OFF(tutorialTexturePack, 0x3ec);
#undef CGOLFTASK_OFF
typedef char CGolfTask_size[(sizeof(CGolfTask) == 0x3F0) ? 1 : -1];

#endif // CLASSES_CGOLFTASK_H

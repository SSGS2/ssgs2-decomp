#ifndef CLASSES_CRESULTFIGHTMENU_H
#define CLASSES_CRESULTFIGHTMENU_H

#include "game/types.h"

class CResultFightMenu {
public:
    void* vtbl;              // 0x0: original vtable pointer
    char pad_004[0x4];       // 0x4
    int updateEnabled;       // 0x8
    char pad_00C[0xc];       // 0xc
    void* resultMode;        // 0x18
    void* layout;            // 0x1c
    int scriptEntryId;       // 0x20
    void* scriptObject;      // 0x24
    void* numberDraw;        // 0x28
    int displayStarted;      // 0x2c
    int introScriptComplete; // 0x30
    float elapsedTime;       // 0x34
    unsigned int entryCount; // 0x38
    char pad_03C[0xd0];      // 0x3c
    int revealState;         // 0x10c
    int revealRowIndex;      // 0x110
    int scoreRevealStep;     // 0x114
    float revealElapsedTime; // 0x118

    void DrawLayoutTextureQuad(int index);
    static int AdvanceRowRevealStages(void* arg);
    int UpdateRevealSequence(float dt);

    void InitPlayerAndScoreNumbers();
    void SetEntries(void* entries, unsigned int count);
    int PollLoadComplete();
    void Draw();
    void UpdateLayout();
    int UpdateAndTryAdvance(float delta);

    ~CResultFightMenu();
    CResultFightMenu(void* resultMode);
};

#define CRESULTFIGHTMENU_OFF(f, o) \
    typedef char CResultFightMenu_off_##f[((int)&((CResultFightMenu*)0)->f == (o)) ? 1 : -1]
CRESULTFIGHTMENU_OFF(updateEnabled, 0x8);
CRESULTFIGHTMENU_OFF(resultMode, 0x18);
CRESULTFIGHTMENU_OFF(layout, 0x1c);
CRESULTFIGHTMENU_OFF(scriptEntryId, 0x20);
CRESULTFIGHTMENU_OFF(scriptObject, 0x24);
CRESULTFIGHTMENU_OFF(numberDraw, 0x28);
CRESULTFIGHTMENU_OFF(displayStarted, 0x2c);
CRESULTFIGHTMENU_OFF(introScriptComplete, 0x30);
CRESULTFIGHTMENU_OFF(elapsedTime, 0x34);
CRESULTFIGHTMENU_OFF(entryCount, 0x38);
CRESULTFIGHTMENU_OFF(revealState, 0x10c);
CRESULTFIGHTMENU_OFF(revealRowIndex, 0x110);
CRESULTFIGHTMENU_OFF(scoreRevealStep, 0x114);
CRESULTFIGHTMENU_OFF(revealElapsedTime, 0x118);
#undef CRESULTFIGHTMENU_OFF

#endif // CLASSES_CRESULTFIGHTMENU_H

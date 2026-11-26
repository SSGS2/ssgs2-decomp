#ifndef CLASSES_CWIIPAUSESCORE_H
#define CLASSES_CWIIPAUSESCORE_H

#include "game/types.h"

class CWiiPauseScore {
public:
    void* vtbl;                       // 0x0: original vtable pointer
    char pad_004[0x14];               // 0x4
    int activationPending;            // 0x18
    int scorecardSuppressed;          // 0x1c
    int blackSpriteEntryId;           // 0x20
    void* blackSprite;                // 0x24
    int scoreEntryId;                 // 0x28
    void* scoreObject;                // 0x2c
    void* menuLayout;                 // 0x30
    void* optionsLayout;              // 0x34
    void* alternatePlayerLabelLayout; // 0x38
    void* playerLabelLayout;          // 0x3c
    void* tieMarkerLayout;            // 0x40
    void* lossMarkerLayout;           // 0x44
    void* textTable;                  // 0x48
    void* optionDialog;               // 0x4c
    void* confirmationDialog;         // 0x50
    int displayMode;                  // 0x54
    unsigned char playerCount;        // 0x58
    unsigned char holeCount;          // 0x59
    char pad_05A[0x2];                // 0x5a
    int postHoleMode;                 // 0x5c
    int interactionState;             // 0x60
    char pad_064[0x50];               // 0x64
    int graphOrigin0X;                // 0xb4
    int graphOrigin0Y;                // 0xb8
    int graphOrigin1X;                // 0xbc
    int graphOrigin1Y;                // 0xc0
    int graphOrigin2X;                // 0xc4
    int graphOrigin2Y;                // 0xc8
    int graphOrigin3X;                // 0xcc
    int graphOrigin3Y;                // 0xd0
    unsigned char displayedHoleCount; // 0xd4
    char pad_0D5[0x3];                // 0xd5
    void* holeInfo;                   // 0xd8
    int holeInfoCount;                // 0xdc
    char pad_0E0[0x4];                // 0xe0
    void* playerHoleScores;           // 0xe4
    int playerHoleScoreCount;         // 0xe8
    char pad_0EC[0x4];                // 0xec
    int menuButtonAvailable0;         // 0xf0
    char pad_0F4[0x24];               // 0xf4
    unsigned char selectedView;       // 0x118
    char pad_119[0x3];                // 0x119
    int showFrontNine;                // 0x11c
    char pad_120[0x24];               // 0x120
    int actionDelayPending;           // 0x144
    int activationDelayFrames;        // 0x148
    int selectedAction;               // 0x14c

    void DrawScoreGraph();
    void InitializeScoreScreen();
    int UpdateAndPollExit(float delta);
    int PollLoadComplete();
    void Draw();
    int ProcessOptionActions();
    int ProcessMenuActions();
    void UpdateLayouts();
    void RefreshScorecard();
    void RankPartyHoleResults(int* soleWinCounts, int* updateIndicators);
    void SetHoleInfo(int holeIndex, unsigned char holeNumber, int par);
    void SetPlayerHoleScore(int playerIndex, int holeIndex, int strokes, int competitionValue, int pang);
    void HideBpObjects();
    void HideInactiveScore();
    void SetMenuButtonAvailable(int index, int value);
    void SetMenuButton3Available(int value);

    CWiiPauseScore(int type, unsigned char playerCount, unsigned char holeCount, void* arg);

    ~CWiiPauseScore();
};

#define CWIIPAUSESCORE_OFF(f, o) typedef char CWiiPauseScore_off_##f[((int)&((CWiiPauseScore*)0)->f == (o)) ? 1 : -1]
CWIIPAUSESCORE_OFF(activationPending, 0x18);
CWIIPAUSESCORE_OFF(scorecardSuppressed, 0x1c);
CWIIPAUSESCORE_OFF(blackSpriteEntryId, 0x20);
CWIIPAUSESCORE_OFF(blackSprite, 0x24);
CWIIPAUSESCORE_OFF(scoreEntryId, 0x28);
CWIIPAUSESCORE_OFF(scoreObject, 0x2c);
CWIIPAUSESCORE_OFF(menuLayout, 0x30);
CWIIPAUSESCORE_OFF(optionsLayout, 0x34);
CWIIPAUSESCORE_OFF(alternatePlayerLabelLayout, 0x38);
CWIIPAUSESCORE_OFF(playerLabelLayout, 0x3c);
CWIIPAUSESCORE_OFF(tieMarkerLayout, 0x40);
CWIIPAUSESCORE_OFF(lossMarkerLayout, 0x44);
CWIIPAUSESCORE_OFF(textTable, 0x48);
CWIIPAUSESCORE_OFF(optionDialog, 0x4c);
CWIIPAUSESCORE_OFF(confirmationDialog, 0x50);
CWIIPAUSESCORE_OFF(displayMode, 0x54);
CWIIPAUSESCORE_OFF(playerCount, 0x58);
CWIIPAUSESCORE_OFF(holeCount, 0x59);
CWIIPAUSESCORE_OFF(postHoleMode, 0x5c);
CWIIPAUSESCORE_OFF(interactionState, 0x60);
CWIIPAUSESCORE_OFF(graphOrigin0X, 0xb4);
CWIIPAUSESCORE_OFF(graphOrigin0Y, 0xb8);
CWIIPAUSESCORE_OFF(graphOrigin1X, 0xbc);
CWIIPAUSESCORE_OFF(graphOrigin1Y, 0xc0);
CWIIPAUSESCORE_OFF(graphOrigin2X, 0xc4);
CWIIPAUSESCORE_OFF(graphOrigin2Y, 0xc8);
CWIIPAUSESCORE_OFF(graphOrigin3X, 0xcc);
CWIIPAUSESCORE_OFF(graphOrigin3Y, 0xd0);
CWIIPAUSESCORE_OFF(displayedHoleCount, 0xd4);
CWIIPAUSESCORE_OFF(holeInfo, 0xd8);
CWIIPAUSESCORE_OFF(holeInfoCount, 0xdc);
CWIIPAUSESCORE_OFF(playerHoleScores, 0xe4);
CWIIPAUSESCORE_OFF(playerHoleScoreCount, 0xe8);
CWIIPAUSESCORE_OFF(menuButtonAvailable0, 0xf0);
CWIIPAUSESCORE_OFF(selectedView, 0x118);
CWIIPAUSESCORE_OFF(showFrontNine, 0x11c);
CWIIPAUSESCORE_OFF(actionDelayPending, 0x144);
CWIIPAUSESCORE_OFF(activationDelayFrames, 0x148);
CWIIPAUSESCORE_OFF(selectedAction, 0x14c);
#undef CWIIPAUSESCORE_OFF

#endif // CLASSES_CWIIPAUSESCORE_H

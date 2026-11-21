// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CWIIPAUSESCORE_H
#define CLASSES_CWIIPAUSESCORE_H

#include "game/types.h"

class CWiiPauseScore {
public:
    void* vtbl;                         // 0x0: original vtable pointer
    char pad_004[0x14];                 // 0x4
    int activationPending;                         // 0x18: 3 uses in CWiiPauseScore_UpdateAndPollExit, CWiiPauseScore_InitializeScoreScreen
    int scorecardSuppressed;                         // 0x1c: 2 uses in CWiiPauseScore_InitializeScoreScreen, CWiiPauseScore_UpdateAndPollExit
    int blackSpriteEntryId;                         // 0x20: 3 uses in CWiiPauseScore_dtor
    void* blackSprite;                       // 0x24: 7 uses in CWiiPauseScore_Draw, CWiiPauseScore_InitializeScoreScreen, CWiiPauseScore_PollLoadComplete
    int scoreEntryId;                         // 0x28: 3 uses in CWiiPauseScore_dtor
    void* scoreObject;                       // 0x2c: 14 uses in CWiiPauseScore_InitializeScoreScreen, CWiiPauseScore_Draw, CWiiPauseScore_PollLoadComplete
    void* menuLayout;                       // 0x30: 44 uses in CWiiPauseScore_InitializeScoreScreen, CWiiPauseScore_UpdateAndPollExit, CWiiPauseScore_dtor...
    void* optionsLayout;                       // 0x34: 22 uses in CWiiPauseScore_InitializeScoreScreen, CWiiPauseScore_dtor, CWiiPauseScore_UpdateAndPollExit...
    void* alternatePlayerLabelLayout;                       // 0x38: 12 uses in CWiiPauseScore_InitializeScoreScreen, CWiiPauseScore_dtor, CWiiPauseScore_UpdateAndPollExit...
    void* playerLabelLayout;                       // 0x3c: 12 uses in CWiiPauseScore_InitializeScoreScreen, CWiiPauseScore_dtor, CWiiPauseScore_UpdateAndPollExit...
    void* tieMarkerLayout;                       // 0x40: 7 uses in CWiiPauseScore_dtor, CWiiPauseScore_InitializeScoreScreen, CWiiPauseScore_UpdateAndPollExit...
    void* lossMarkerLayout;                       // 0x44: 7 uses in CWiiPauseScore_dtor, CWiiPauseScore_InitializeScoreScreen, CWiiPauseScore_UpdateAndPollExit...
    void* textTable;                       // 0x48: 8 uses in CWiiPauseScore_InitializeScoreScreen, CWiiPauseScore_dtor, CWiiPauseScore_Draw...
    void* optionDialog;                       // 0x4c: 4 uses in CWiiPauseScore_UpdateAndPollExit, CWiiPauseScore_Draw
    void* confirmationDialog;                       // 0x50: 9 uses in CWiiPauseScore_UpdateAndPollExit, CWiiPauseScore_Draw
    int displayMode;                         // 0x54: 21 uses in CWiiPauseScore_InitializeScoreScreen, CWiiPauseScore_UpdateAndPollExit, CWiiPauseScore_PollLoadComplete...
    unsigned char playerCount;               // 0x58: 16 uses in CWiiPauseScore_InitializeScoreScreen
    unsigned char holeCount;               // 0x59: 2 uses in CWiiPauseScore_InitializeScoreScreen
    char pad_05A[0x2];                  // 0x5a
    int postHoleMode;                         // 0x5c: 8 uses in CWiiPauseScore_UpdateAndPollExit, CWiiPauseScore_InitializeScoreScreen
    int interactionState;                         // 0x60: 6 uses in CWiiPauseScore_UpdateAndPollExit, CWiiPauseScore_InitializeScoreScreen
    char pad_064[0x50];                 // 0x64
    int graphOrigin0X;                         // 0xb4: 4 uses in CWiiPauseScore_Ctor
    int graphOrigin0Y;                         // 0xb8: 4 uses in CWiiPauseScore_Ctor
    int graphOrigin1X;                         // 0xbc: 3 uses in CWiiPauseScore_Ctor
    int graphOrigin1Y;                         // 0xc0: 3 uses in CWiiPauseScore_Ctor
    int graphOrigin2X;                         // 0xc4: 2 uses in CWiiPauseScore_Ctor
    int graphOrigin2Y;                         // 0xc8: 2 uses in CWiiPauseScore_Ctor
    int graphOrigin3X;                         // 0xcc: 1 use in CWiiPauseScore_Ctor
    int graphOrigin3Y;                         // 0xd0: 1 use in CWiiPauseScore_Ctor
    unsigned char displayedHoleCount;               // 0xd4: 2 uses in CWiiPauseScore_InitializeScoreScreen
    char pad_0D5[0x3];                  // 0xd5
    void* holeInfo;                       // 0xd8: 1 use in CWiiPauseScore_dtor
    int holeInfoCount;                         // 0xdc: 5 uses in CWiiPauseScore_dtor, CWiiPauseScore_InitializeScoreScreen
    char pad_0E0[0x4];                  // 0xe0
    void* playerHoleScores;                       // 0xe4: 1 use in CWiiPauseScore_dtor
    int playerHoleScoreCount;                         // 0xe8: 4 uses in CWiiPauseScore_dtor
    char pad_0EC[0x4];                  // 0xec
    int menuButtonAvailable0;                         // 0xf0: 2 uses in CWiiPauseScore_InitializeScoreScreen
    char pad_0F4[0x24];                 // 0xf4
    unsigned char selectedView;              // 0x118: 2 uses in CWiiPauseScore_InitializeScoreScreen, CWiiPauseScore_Draw
    char pad_119[0x3];                  // 0x119
    int showFrontNine;                        // 0x11c: 2 uses in CWiiPauseScore_InitializeScoreScreen
    char pad_120[0x24];                 // 0x120
    int actionDelayPending;                        // 0x144: 2 uses in CWiiPauseScore_InitializeScoreScreen, CWiiPauseScore_UpdateAndPollExit
    int activationDelayFrames;                        // 0x148: 1 use in CWiiPauseScore_InitializeScoreScreen
    int selectedAction;                        // 0x14c: 3 uses in CWiiPauseScore_UpdateAndPollExit

    // offsets whose address is passed on (sub-objects or arrays, not placed): 0x14, 0x50, 0xd8, 0xe4, 0x124
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
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

#endif  // CLASSES_CWIIPAUSESCORE_H

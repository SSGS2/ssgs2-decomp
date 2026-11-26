// CCharaEntry: the four-player character entry screen.
//
// One instance exists while the screen is up. It owns the layout, the shop a
// player has opened, the per-player stat-bar transition records, and the flags
// that make Process wait until every player has confirmed.
//
// The class is declared without virtual functions on purpose: its vtable is
// still taken from the original game data, so `vtbl` is spelled out as a member.
#ifndef CLASSES_CCHARAENTRY_H
#define CLASSES_CCHARAENTRY_H

#include "game/types.h"

class CCharaEntry {
public:
    void* vtbl;                   // 0x00
    char pad_004[0x14];           // 0x04
    int ready[4];                 // 0x18  set when that player's entry is confirmed
    int result;                   // 0x28  Process result: 1 accepted, 0 cancelled
    void* layout;                 // 0x2c
    int* statBarTransitions;      // 0x30 four 3-word records; word 2 queues a shop choice
    int readyFrames;              // 0x34 updates all four players have been ready
    int selectedPlayerIndex;      // 0x38 player whose shop is open, -1 when none
    void* textureResource;        // 0x3c
    int shopState;                // 0x40 shop idle, waiting for load, or processing
    void* entryScriptId;          // 0x44
    void* noEntryScriptId;        // 0x48
    void* characterStatsScriptId; // 0x4c
    int started;                  // 0x50 first update after the fade has run
    void* selectionStorage;       // 0x54 selection vector: data, count, capacity
    int selectionCount;           // 0x58
    void* selectionCapacity;      // 0x5c
    void* shop;                   // 0x60
    unsigned char shopOpen;       // 0x64
    char pad_065[3];              // 0x65

    void UpdateResourceLoaders();
    int PollResourcesReady();
    void CyclePlayerEntryState(int idx);

    void RefreshPlayerRow(int index);
    void UpdateStatBars();
    void SetEnemyLevelSelection(int index, int selectedLevel);
    int IsSelectionConfirmed();
    void StartFadeInOrRequestTransition(int enabled);
    void DrawEntryScreen();

    void SetupEntryScreen();
    int Process(float elapsedSeconds);
    void OpenPlayerShop(int playerIndex, int inputPlayerIndexWord);

    CCharaEntry();
    ~CCharaEntry();
};

#define CCHARAENTRY_OFF(f, o) typedef char CCharaEntry_off_##f[((int)&((CCharaEntry*)0)->f == (o)) ? 1 : -1]
CCHARAENTRY_OFF(ready, 0x18);
CCHARAENTRY_OFF(result, 0x28);
CCHARAENTRY_OFF(layout, 0x2c);
CCHARAENTRY_OFF(statBarTransitions, 0x30);
CCHARAENTRY_OFF(readyFrames, 0x34);
CCHARAENTRY_OFF(selectedPlayerIndex, 0x38);
CCHARAENTRY_OFF(shopState, 0x40);
CCHARAENTRY_OFF(entryScriptId, 0x44);
CCHARAENTRY_OFF(noEntryScriptId, 0x48);
CCHARAENTRY_OFF(characterStatsScriptId, 0x4c);
CCHARAENTRY_OFF(started, 0x50);
CCHARAENTRY_OFF(shop, 0x60);
CCHARAENTRY_OFF(shopOpen, 0x64);
#undef CCHARAENTRY_OFF
typedef char CCharaEntry_size[(sizeof(CCharaEntry) == 0x68) ? 1 : -1];

#endif // CLASSES_CCHARAENTRY_H

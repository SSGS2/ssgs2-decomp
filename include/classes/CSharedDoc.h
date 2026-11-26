#ifndef CLASSES_CSHAREDDOC_H
#define CLASSES_CSHAREDDOC_H

#include "game/types.h"

class CSharedDoc {
public:
    void* vtbl;                               // 0x0: original vtable pointer
    char pad_004[0x3c];                       // 0x4
    short unk_40;                             // 0x40: GolfRule_UpdateRoundRewards
    char pad_042[0x722];                      // 0x42
    void* strokePlayName;                     // 0x764
    unsigned char unk_768;                    // 0x768
    unsigned char unk_769;                    // 0x769
    unsigned char unk_76A;                    // 0x76a
    char pad_76B[0x1];                        // 0x76b
    void* matchPlayName;                      // 0x76c
    unsigned char unk_770;                    // 0x770
    unsigned char unk_771;                    // 0x771
    unsigned char unk_772;                    // 0x772
    char pad_773[0x1];                        // 0x773
    void* caddiePracticeName;                 // 0x774
    unsigned char unk_778;                    // 0x778
    unsigned char unk_779;                    // 0x779
    unsigned char unk_77A;                    // 0x77a
    char pad_77B[0xd];                        // 0x77b
    void* firstOwnedTreeRoot;                 // 0x788
    char pad_78C[0xc];                        // 0x78c
    void* secondOwnedTreeRoot;                // 0x798
    char pad_79C[0x8];                        // 0x79c
    int unk_7A4;                              // 0x7a4
    char pad_7A8[0x4];                        // 0x7a8
    void* unk_7AC;                            // 0x7ac
    int unk_7B0;                              // 0x7b0
    char pad_7B4[0x4];                        // 0x7b4
    void* unk_7B8;                            // 0x7b8
    int unk_7BC;                              // 0x7bc
    char pad_7C0[0x4];                        // 0x7c0
    void* unk_7C4;                            // 0x7c4
    int unk_7C8;                              // 0x7c8
    char pad_7CC[0x4];                        // 0x7cc
    void* unk_7D0;                            // 0x7d0
    int unk_7D4;                              // 0x7d4
    char pad_7D8[0x4];                        // 0x7d8
    void* unk_7DC;                            // 0x7dc
    int unk_7E0;                              // 0x7e0
    char pad_7E4[0x4];                        // 0x7e4
    void* unk_7E8;                            // 0x7e8
    int unk_7EC;                              // 0x7ec
    char pad_7F0[0x4];                        // 0x7f0
    void* unk_7F4;                            // 0x7f4
    void* bonusItemIds;                       // 0x7f8
    unsigned int bonusItemCount;              // 0x7fc
    char pad_800[0x4];                        // 0x800
    unsigned char courseRecordUpdatesEnabled; // 0x804
    char pad_805[0x3];                        // 0x805
    unsigned int gameMode;                    // 0x808
    unsigned int unk_80C;                     // 0x80c
    unsigned char roundHoleOrder;             // 0x810
    char pad_811[0x3];                        // 0x811
    unsigned int unk_814;                     // 0x814
    char pad_818[0x84];                       // 0x818
    float pangRewardMultiplier;               // 0x89c
    unsigned int pangRewardPercent;           // 0x8a0
    char pad_8A4[0x18];                       // 0x8a4
    void* selectedCourseData;                 // 0x8bc: CGolfRule_OnInit, CGolfRule_PreloadShotAndPartySequences
    unsigned char
        golfRuleType; // 0x8c0: CGolfRule_ResetHoleBallAndWind, CGolfRule_OnReload, CGolfRule_ProcessDelayedStateTransition, fn_80168898_old
    char playNameBuffer[0x104];   // 0x8c1
    unsigned char roundHoleCount; // 0x9c5: CGolfRule_OnReload
    union {
        char pad_9C6[2]; // legacy byte-address spelling used by lighting consumers
        struct {
            unsigned char weatherType; // +9C6: music/weather selector; wider role unresolved
            char pad_9C7[1];
        };
    };
    unsigned int turnTimeLimitMs; // 0x9c8: CClock_OnProcess, CClock_HandleMsg
    int unk_9CC;                  // 0x9cc: CTourMain_PrepareSelectedEncounter
    char pad_9D0[0x34];           // 0x9d0
    char itemInventory
        [0xbc]; // 0xa04: CBuildProductChara_FindSetContainingItem, CPangya2Shop_RefreshProductLists: inventory lookup subobject; extent ends at existing 0xac0
    unsigned int randomizePlayerStats;           // 0xac0
    unsigned int unk_AC4;                        // 0xac4
    unsigned char unk_AC8;                       // 0xac8
    unsigned char unk_AC9;                       // 0xac9
    char pad_ACA[0x2];                           // 0xaca
    unsigned int unk_ACC;                        // 0xacc
    unsigned int unk_AD0;                        // 0xad0
    unsigned int unk_AD4;                        // 0xad4
    unsigned int partyGameMode;                  // 0xad8
    char partyTiers[0x14];                       // 0xadc
    unsigned int partyResultTier;                // 0xaf0
    char pad_AF4[0x8c];                          // 0xaf4
    unsigned int courseRecordIndex;              // 0xb80
    unsigned int enemyLevelIndex;                // 0xb84
    unsigned int partyCourseTableIndex;          // 0xb88
    unsigned int pendingCourseClearNotification; // 0xb8c
    unsigned int golfDoc;                        // 0xb90
    unsigned int menuTextureIndex;               // 0xb94

    void BindItemSelectionInventory();
    void ClearPlayerStateStorage();

    // Round setup setters. They write the shared document's current round state.
    static void SetCourseRecordUpdatesEnabled(unsigned char value);
    static void SetTurnTimeLimitMs(int value);
    static void SetRoundHoleOrder(unsigned char value);
    static void SetRoundHoleCount(unsigned char value);
    static void SetRoundOptionByte(unsigned char value);

    // Install the round defaults. Constructing the singleton base publishes
    // g_sharedDoc before the rest of the body runs.
    CSharedDoc();
    ~CSharedDoc();
};

#define CSHAREDDOC_OFF(f, o) typedef char CSharedDoc_off_##f[((int)&((CSharedDoc*)0)->f == (o)) ? 1 : -1]
CSHAREDDOC_OFF(strokePlayName, 0x764);
CSHAREDDOC_OFF(unk_768, 0x768);
CSHAREDDOC_OFF(unk_769, 0x769);
CSHAREDDOC_OFF(unk_76A, 0x76a);
CSHAREDDOC_OFF(matchPlayName, 0x76c);
CSHAREDDOC_OFF(unk_770, 0x770);
CSHAREDDOC_OFF(unk_771, 0x771);
CSHAREDDOC_OFF(unk_772, 0x772);
CSHAREDDOC_OFF(caddiePracticeName, 0x774);
CSHAREDDOC_OFF(unk_778, 0x778);
CSHAREDDOC_OFF(unk_779, 0x779);
CSHAREDDOC_OFF(unk_77A, 0x77a);
CSHAREDDOC_OFF(firstOwnedTreeRoot, 0x788);
CSHAREDDOC_OFF(secondOwnedTreeRoot, 0x798);
CSHAREDDOC_OFF(unk_7A4, 0x7a4);
CSHAREDDOC_OFF(unk_7AC, 0x7ac);
CSHAREDDOC_OFF(unk_7B0, 0x7b0);
CSHAREDDOC_OFF(unk_7B8, 0x7b8);
CSHAREDDOC_OFF(unk_7BC, 0x7bc);
CSHAREDDOC_OFF(unk_7C4, 0x7c4);
CSHAREDDOC_OFF(unk_7C8, 0x7c8);
CSHAREDDOC_OFF(unk_7D0, 0x7d0);
CSHAREDDOC_OFF(unk_7D4, 0x7d4);
CSHAREDDOC_OFF(unk_7DC, 0x7dc);
CSHAREDDOC_OFF(unk_7E0, 0x7e0);
CSHAREDDOC_OFF(unk_7E8, 0x7e8);
CSHAREDDOC_OFF(unk_7EC, 0x7ec);
CSHAREDDOC_OFF(unk_7F4, 0x7f4);
CSHAREDDOC_OFF(bonusItemIds, 0x7f8);
CSHAREDDOC_OFF(bonusItemCount, 0x7fc);
CSHAREDDOC_OFF(courseRecordUpdatesEnabled, 0x804);
CSHAREDDOC_OFF(gameMode, 0x808);
CSHAREDDOC_OFF(unk_80C, 0x80c);
CSHAREDDOC_OFF(roundHoleOrder, 0x810);
CSHAREDDOC_OFF(unk_814, 0x814);
CSHAREDDOC_OFF(pangRewardMultiplier, 0x89c);
CSHAREDDOC_OFF(pangRewardPercent, 0x8a0);
CSHAREDDOC_OFF(randomizePlayerStats, 0xac0);
CSHAREDDOC_OFF(unk_AC4, 0xac4);
CSHAREDDOC_OFF(unk_AC8, 0xac8);
CSHAREDDOC_OFF(unk_AC9, 0xac9);
CSHAREDDOC_OFF(unk_ACC, 0xacc);
CSHAREDDOC_OFF(unk_AD0, 0xad0);
CSHAREDDOC_OFF(unk_AD4, 0xad4);
CSHAREDDOC_OFF(partyGameMode, 0xad8);
CSHAREDDOC_OFF(partyResultTier, 0xaf0);
CSHAREDDOC_OFF(courseRecordIndex, 0xb80);
CSHAREDDOC_OFF(enemyLevelIndex, 0xb84);
CSHAREDDOC_OFF(partyCourseTableIndex, 0xb88);
CSHAREDDOC_OFF(pendingCourseClearNotification, 0xb8c);
CSHAREDDOC_OFF(golfDoc, 0xb90);
CSHAREDDOC_OFF(menuTextureIndex, 0xb94);
CSHAREDDOC_OFF(unk_40, 0x40);
CSHAREDDOC_OFF(selectedCourseData, 0x8bc);
CSHAREDDOC_OFF(golfRuleType, 0x8c0);
CSHAREDDOC_OFF(roundHoleCount, 0x9c5);
CSHAREDDOC_OFF(weatherType, 0x9C6);
CSHAREDDOC_OFF(turnTimeLimitMs, 0x9c8);
CSHAREDDOC_OFF(unk_9CC, 0x9cc);
CSHAREDDOC_OFF(itemInventory, 0xa04);
#undef CSHAREDDOC_OFF

typedef char CSharedDoc_size[(sizeof(CSharedDoc) == 0xb98) ? 1 : -1];

#endif // CLASSES_CSHAREDDOC_H

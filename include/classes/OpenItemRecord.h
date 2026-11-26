// OpenItemRecord: field view inferred from accesses in the cited functions.
// No virtual declarations or inheritance; this header emits no code or RTTI.
#ifndef CLASSES_OPENITEMRECORD_H
#define CLASSES_OPENITEMRECORD_H

class OpenItemRecord {
public:
    char pad_000[0x2];
    short connectionStyles[4]; // 0x02: left/right/up/down; -1 disables, 2 hides lit links
    char pad_00A[0xA];
    void* firstPrimaryStringOffset;    // 0x14: CTourMain_PrepareSelectedEncounter
    void* secondPrimaryStringOffset;   // 0x18: CTourMain_PrepareSelectedEncounter
    void* firstSecondaryStringOffset;  // 0x1c: CTourMain_PrepareSelectedEncounter
    void* secondSecondaryStringOffset; // 0x20: CTourMain_PrepareSelectedEncounter
    char pad_024[0x4];
    signed char unk_28; // 0x28: CTourMain_PrepareSelectedEncounter
    char pad_029[0x3];
    signed char previewModelIndex; // 0x2c: CTourMain_PollItemMapResources
    char pad_02D[0x2];
    signed char caddieIndex; // 0x2f: CTourMain_PollItemMapResources
    char pad_030[0xc];
    short golfRuleType; // 0x3c: CTourMain_PrepareSelectedEncounter
    short enemyRecord;  // 0x3e: CTourMain_PrepareSelectedEncounter
    char pad_040[0x2];
    unsigned char courseId;    // 0x42: CTourMain_PrepareSelectedEncounter
    unsigned char weatherCode; // 0x43: CTourMain_PrepareSelectedEncounter
    char pad_044[0x12];
    short firstCompletionTourCoins;           // 0x56: CTourMain_PrepareSelectedEncounter
    unsigned int firstCompletionPlayerReward; // 0x58: CTourMain_PrepareSelectedEncounter
    char pad_05C[0x20];
    short rewardClubSetIndex;   // 0x7c: OpenItemMenu_QueueSelectedItemRewards
    short rewardClubSetCount;   // 0x7e: OpenItemMenu_QueueSelectedItemRewards
    short rewardBallIndex;      // 0x80: OpenItemMenu_QueueSelectedItemRewards
    short rewardBallCount;      // 0x82: OpenItemMenu_QueueSelectedItemRewards
    short rewardCharacterIndex; // 0x84: OpenItemMenu_QueueSelectedItemRewards
    short rewardCharacterCount; // 0x86: OpenItemMenu_QueueSelectedItemRewards
};

#define OFF(f, o) typedef char OpenItemRecord_off_##f[((int)&((OpenItemRecord*)0)->f == (o)) ? 1 : -1]
OFF(connectionStyles, 0x2);
OFF(firstPrimaryStringOffset, 0x14);
OFF(secondPrimaryStringOffset, 0x18);
OFF(firstSecondaryStringOffset, 0x1c);
OFF(secondSecondaryStringOffset, 0x20);
OFF(unk_28, 0x28);
OFF(previewModelIndex, 0x2c);
OFF(caddieIndex, 0x2f);
OFF(golfRuleType, 0x3c);
OFF(enemyRecord, 0x3e);
OFF(courseId, 0x42);
OFF(weatherCode, 0x43);
OFF(firstCompletionTourCoins, 0x56);
OFF(firstCompletionPlayerReward, 0x58);
OFF(rewardClubSetIndex, 0x7c);
OFF(rewardClubSetCount, 0x7e);
OFF(rewardBallIndex, 0x80);
OFF(rewardBallCount, 0x82);
OFF(rewardCharacterIndex, 0x84);
OFF(rewardCharacterCount, 0x86);
#undef OFF

#endif

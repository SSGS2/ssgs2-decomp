// CPangInfo: the Pang reward presentation in the HUD.
//
// It counts the earned Pang and bonus Pang up, schedules one entry per reward
// category, and draws the totals and the reward banners.
//
// The class is not declared with virtual functions: its vtable comes from the
// original data, so `vtbl` is spelled out as a member.
#ifndef CLASSES_CPANGINFO_H
#define CLASSES_CPANGINFO_H

#include "game/types.h"
#include "classes/CInfoBase.h"

class CPangInfo : public CInfoBase {
public:
    CPangInfo();
    ~CPangInfo();

    void InitScreenPositions();
    void ResetFromCurrentPlayer();
    void UpdateShotRewardsAndBonuses(float dt);
    void CalculateShotRewards(int includeEndOfHoleFallback);
    void CalculateMatchPlayHoleRewards();
    void ResetPangAndBonusAnimation(int initialDisplayFlags, int initialEntryState);
    void AdvanceRewardDisplayTimers(float delta);
    void AnimateRewardEntries(float delta);
    void UpdateSettlementFlags();
    void UpdateCompletionDelay(float delta);
    void ScheduleBonusEntry(int category);
    void UpdateLongPuttBonus();
    void UpdateChipInBonus();
    void UpdateRecoveryBonus();
    void UpdateScoreBonusState();
    void UpdateConditionalBonusEntry(int index, int value);
    void UpdateObjectHitMultiplier();
    void UpdateBallBounceBonus();
    void Draw();
    void ResetBonusAnimations();
    void FinishDisplayedTotals();

    void* textRenderer; // 0x18: CPangInfo::Draw
    int displayMode;    // 0x1c
    unsigned int flags; // 0x20: CPangInfo::Draw, CPangInfo::CalculateShotRewards, CPangInfo::ResetPangAndBonusAnimation
    int countingSoundAlias; // 0x24: CPangInfo::ResetPangAndBonusAnimation, CPangInfo::AnimateRewardEntries
    int earnedPang; // 0x28: CPangInfo::UpdateShotRewardsAndBonuses, CPangInfo::CalculateShotRewards, CPangInfo::ResetPangAndBonusAnimation
    int earnedBonusPang; // 0x2c: CPangInfo::UpdateShotRewardsAndBonuses, CPangInfo::CalculateShotRewards, CPangInfo::ResetPangAndBonusAnimation
    unsigned int startingPang;      // 0x30: CPangInfo::ResetBonusAnimations
    unsigned int startingBonusPang; // 0x34: CPangInfo::ResetBonusAnimations
    int settledPang;                // 0x38: CPangInfo::AnimateRewardEntries
    int settledBonusPang;           // 0x3c: CPangInfo::AnimateRewardEntries
    int pangBeforeMultiplier;       // 0x40: CPangInfo::ResetPangAndBonusAnimation, CPangInfo::AnimateRewardEntries
    int bonusPangBeforeMultiplier;  // 0x44: CPangInfo::ResetPangAndBonusAnimation, CPangInfo::AnimateRewardEntries
    int pangAnimationState; // 0x48: CPangInfo::Draw, CPangInfo::ResetPangAndBonusAnimation, CPangInfo::AnimateRewardEntries
    int displayedPang;      // 0x4c: CPangInfo::Draw, CPangInfo::AnimateRewardEntries
    int pangEntryOrder;     // 0x50
    float pangAnimationTime;      // 0x54: CPangInfo::ResetPangAndBonusAnimation, CPangInfo::AnimateRewardEntries
    float pangBaselineY;          // 0x58: CPangInfo::Draw
    char pad_05C[0x4];            // 0x5c
    float pangUvX;                // 0x60
    float pangUvY;                // 0x64
    float pangUvWidth;            // 0x68
    float pangUvHeight;           // 0x6c
    unsigned char pangOpacity;    // 0x70: CPangInfo::Draw
    char pad_071[0x3];            // 0x71
    int bonusPangAnimationState;  // 0x74: CPangInfo::ResetPangAndBonusAnimation, CPangInfo::AnimateRewardEntries
    int displayedBonusPang;       // 0x78: CPangInfo::Draw, CPangInfo::AnimateRewardEntries
    char pad_07C[0x4];            // 0x7c
    float bonusPangAnimationTime; // 0x80: CPangInfo::ResetPangAndBonusAnimation, CPangInfo::AnimateRewardEntries
    char pad_084[0x1c];           // 0x84
    int bonusEntryState; // 0xa0: CPangInfo::UpdateShotRewardsAndBonuses, CPangInfo::Draw, CPangInfo::CalculateShotRewards
    int bonusEntryValue; // 0xa4: CPangInfo::Draw, CPangInfo::AnimateRewardEntries
    int bonusEntryOrder; // 0xa8: CPangInfo::AnimateRewardEntries, CPangInfo::ScheduleBonusEntry
    float
        bonusEntryAnimationTime; // 0xac: CPangInfo::UpdateShotRewardsAndBonuses, CPangInfo::Draw, CPangInfo::AnimateRewardEntries
    float bonusEntryY;           // 0xb0: CPangInfo::Draw, CPangInfo::AnimateRewardEntries
    float bonusEntryTargetY; // 0xb4: CPangInfo::AnimateRewardEntries
    char pad_0B8[0x10];      // 0xb8
    unsigned char
        bonusEntryOpacity; // 0xc8: CPangInfo::Draw, CPangInfo::AnimateRewardEntries, CPangInfo::ResetBonusAnimations
    char pad_0C9[0x3];     // 0xc9
    int longDriveMultiplierState;          // 0xcc: CPangInfo::CalculateShotRewards
    int longDriveMultiplier;               // 0xd0: CPangInfo::CalculateShotRewards
    char pad_0D4[0x24];                    // 0xd4
    int approachBonusState;                // 0xf8: CPangInfo::CalculateShotRewards
    char pad_0FC[0x28];                    // 0xfc
    int longPuttBonusState;                // 0x124: CPangInfo::CalculateShotRewards, CPangInfo::UpdateLongPuttBonus
    unsigned int longPuttBonusPang;        // 0x128: CPangInfo::UpdateLongPuttBonus
    char pad_12C[0x4];                     // 0x12c
    float longPuttBonusAnimationTime;      // 0x130: CPangInfo::UpdateLongPuttBonus
    char pad_134[0x1c];                    // 0x134
    int chipInBonusState;                  // 0x150: CPangInfo::CalculateShotRewards, CPangInfo::UpdateChipInBonus
    unsigned int chipInBonusPang;          // 0x154: CPangInfo::UpdateChipInBonus
    char pad_158[0x4];                     // 0x158
    float chipInBonusAnimationTime;        // 0x15c: CPangInfo::UpdateChipInBonus
    char pad_160[0x1c];                    // 0x160
    int pinContactBonusState;              // 0x17c: CPangInfo::CalculateShotRewards
    int pinContactBonusPang;               // 0x180: CPangInfo::CalculateShotRewards
    char pad_184[0x24];                    // 0x184
    int matchEarlyWinBonusState;           // 0x1a8: CPangInfo::CalculateShotRewards
    char pad_1AC[0x8];                     // 0x1ac
    float matchEarlyWinBonusAnimationTime; // 0x1b4: CPangInfo::UpdateShotRewardsAndBonuses
    char pad_1B8[0x1c];                    // 0x1b8
    int specialShotBonusState;             // 0x1d4: CPangInfo::CalculateShotRewards
    int specialShotBonusPang;              // 0x1d8: CPangInfo::CalculateShotRewards
    char pad_1DC[0x10];                    // 0x1dc
    float specialShotBonusUvX;             // 0x1ec: CPangInfo::CalculateShotRewards
    float specialShotBonusUvY;             // 0x1f0: CPangInfo::CalculateShotRewards
    char pad_1F4[0xc];                     // 0x1f4
    int scoreBonusState; // 0x200: CPangInfo::CalculateShotRewards, CPangInfo::UpdateScoreBonusState, CPangInfo::UpdateConditionalBonusEntry
    int scoreBonusPang;            // 0x204: CPangInfo::CalculateShotRewards, CPangInfo::AnimateRewardEntries
    int scoreBonusOrder;           // 0x208: CPangInfo::ScheduleBonusEntry
    float scoreBonusAnimationTime; // 0x20c: CPangInfo::UpdateScoreBonusState
    char pad_210[0xc];             // 0x210
    float scoreBonusUvY;           // 0x21c: CPangInfo::CalculateShotRewards
    char pad_220[0xc];             // 0x220
    int powerSpinMultiplierState;  // 0x22c: CPangInfo::CalculateShotRewards
    char pad_230[0x28];            // 0x230
    int powerCurveMultiplierState; // 0x258: CPangInfo::CalculateShotRewards
    char pad_25C[0x28];            // 0x25c
    int objectHitMultiplierState; // 0x284: CPangInfo::CalculateShotRewards, CPangInfo::ScheduleBonusEntry, CPangInfo::UpdateObjectHitMultiplier
    int objectHitMultiplier; // 0x288: CPangInfo::CalculateShotRewards, CPangInfo::UpdateObjectHitMultiplier, CPangInfo::ResetBonusAnimations
    int objectHitMultiplierOrder;            // 0x28c: CPangInfo::ScheduleBonusEntry
    float objectHitMultiplierAnimationTime;  // 0x290: CPangInfo::UpdateObjectHitMultiplier
    char pad_294[0x4];                       // 0x294
    float objectHitMultiplierTargetY;        // 0x298: CPangInfo::ScheduleBonusEntry
    char pad_29C[0x14];                      // 0x29c
    int holeInOneBonusState;                 // 0x2b0: CPangInfo::Draw, CPangInfo::CalculateShotRewards
    int holeInOneBonusPang;                  // 0x2b4: CPangInfo::CalculateShotRewards
    char pad_2B8[0x4];                       // 0x2b8
    float holeInOneBonusAnimationTime;       // 0x2bc: CPangInfo::UpdateShotRewardsAndBonuses
    char pad_2C0[0x1c];                      // 0x2c0
    int roundCompletionBonusState;           // 0x2dc: CPangInfo::CalculateShotRewards
    int roundCompletionBonusPang;            // 0x2e0: CPangInfo::CalculateShotRewards
    char pad_2E4[0x4];                       // 0x2e4
    float roundCompletionBonusAnimationTime; // 0x2e8: CPangInfo::UpdateShotRewardsAndBonuses
    char pad_2EC[0x1c];                      // 0x2ec
    int pangMultiplierState; // 0x308: CPangInfo::UpdateShotRewardsAndBonuses, CPangInfo::CalculateShotRewards, CPangInfo::AnimateRewardEntries
    int pangMultiplier;                // 0x30c: CPangInfo::CalculateShotRewards
    int pangMultiplierOrder;           // 0x310: CPangInfo::ScheduleBonusEntry
    float pangMultiplierAnimationTime; // 0x314: CPangInfo::UpdateShotRewardsAndBonuses
    char pad_318[0x4];                 // 0x318
    float pangMultiplierTargetY;       // 0x31c: CPangInfo::AnimateRewardEntries, CPangInfo::ScheduleBonusEntry
    char pad_320[0x14];                // 0x320
    int recoveryBonusState;            // 0x334: CPangInfo::CalculateShotRewards, CPangInfo::UpdateRecoveryBonus
    int recoveryBonusPang;             // 0x338: CPangInfo::UpdateRecoveryBonus
    char pad_33C[0x4];                 // 0x33c
    float recoveryBonusAnimationTime;  // 0x340: CPangInfo::UpdateRecoveryBonus
    char pad_344[0x1c];                // 0x344
    int ballBonusState; // 0x360: CPangInfo::Draw, CPangInfo::CalculateShotRewards, CPangInfo::AnimateRewardEntries
    int ballBonusPang;  // 0x364: CPangInfo::CalculateShotRewards
    char pad_368[0x4];  // 0x368
    float ballBonusAnimationTime; // 0x36c: CPangInfo::UpdateShotRewardsAndBonuses
    char pad_370[0x4];            // 0x370
    float ballBonusTargetY;       // 0x374: CPangInfo::AnimateRewardEntries
    char pad_378[0x14];           // 0x378
    int ballBounceBonusState; // 0x38c: CPangInfo::CalculateShotRewards, CPangInfo::AnimateRewardEntries, CPangInfo::UpdateBallBounceBonus
    int ballBounceBonusPang;            // 0x390: CPangInfo::CalculateShotRewards, CPangInfo::UpdateBallBounceBonus
    char pad_394[0x4];                  // 0x394
    float ballBounceBonusAnimationTime; // 0x398: CPangInfo::UpdateBallBounceBonus
    char pad_39C[0x4];                  // 0x39c
    float ballBounceBonusTargetY;       // 0x3a0: CPangInfo::AnimateRewardEntries
    char pad_3A4[0x14];                 // 0x3a4
    int powerShotBonusState;            // 0x3b8: CPangInfo::CalculateShotRewards, CPangInfo::AnimateRewardEntries
    int powerShotBonusPang;             // 0x3bc: CPangInfo::CalculateShotRewards
    char pad_3C0[0x4];                  // 0x3c0
    float powerShotBonusAnimationTime;  // 0x3c4: CPangInfo::UpdateShotRewardsAndBonuses
    char pad_3C8[0x4];                  // 0x3c8
    float powerShotBonusTargetY;        // 0x3cc: CPangInfo::AnimateRewardEntries
    char pad_3D0[0x18];                 // 0x3d0
    float holeDirectionY;               // 0x3e8: CPangInfo::CalculateShotRewards
    char pad_3EC[0x4];                  // 0x3ec
    int scoreRelativeToPar; // 0x3f0: CPangInfo::CalculateShotRewards, CPangInfo::UpdateConditionalBonusEntry, CPangInfo::UpdateObjectHitMultiplier
    int clubType;                  // 0x3f4: CPangInfo::CalculateShotRewards
    int shotGroundType;            // 0x3f8: CPangInfo::CalculateShotRewards
    float clubDistanceYards;       // 0x3fc: CPangInfo::CalculateShotRewards
    float holeDistanceYards;       // 0x400: CPangInfo::CalculateShotRewards
    float shotTravelDistanceYards; // 0x404: CPangInfo::UpdateLongPuttBonus, CPangInfo::UpdateChipInBonus
    float remainingDistanceYards;  // 0x408: CPangInfo::UpdateChipInBonus
    int scheduledBonusCount; // 0x40c: CPangInfo::ResetPangAndBonusAnimation, CPangInfo::ScheduleBonusEntry, CPangInfo::ResetBonusAnimations
    int processedTrajectoryFrame; // 0x410: CPangInfo::ResetPangAndBonusAnimation, CPangInfo::UpdateObjectHitMultiplier, CPangInfo::UpdateBallBounceBonus
    float bonusBannerStartX; // 0x414: CPangInfo::ScheduleBonusEntry
    float bonusBannerStartY; // 0x418: CPangInfo::ScheduleBonusEntry
    char pad_41C[0x20];      // 0x41c
    float
        holeInOneBannerElapsed; // 0x43c: CPangInfo::CPangInfo, CPangInfo::UpdateShotRewardsAndBonuses, CPangInfo::Draw
    float
        ballBonusBannerElapsed; // 0x440: CPangInfo::CPangInfo, CPangInfo::UpdateShotRewardsAndBonuses, CPangInfo::Draw
    char pad_444[0x8];          // 0x444
    float completionDelayElapsed; // 0x44c: CPangInfo::CPangInfo, CPangInfo::ResetPangAndBonusAnimation
    int unk_450;                  // 0x450: CPangInfo::Draw
    bool counting;                // 0x454
    char pad_455[3];              // 0x455
    int playerValues[4];          // 0x458
    int rewardedBounceCount; // 0x468: CPangInfo::ResetPangAndBonusAnimation, CPangInfo::UpdateBallBounceBonus, CPangInfo::ResetBonusAnimations
    int lastRewardedBounceFrame; // 0x46c: CPangInfo::ResetPangAndBonusAnimation, CPangInfo::UpdateBallBounceBonus, CPangInfo::ResetBonusAnimations
};

#define CPANGINFO_OFF(f, o) typedef char CPangInfo::off_##f[((int)&((CPangInfo*)0)->f == (o)) ? 1 : -1]
CPANGINFO_OFF(textRenderer, 0x18);
CPANGINFO_OFF(flags, 0x20);
CPANGINFO_OFF(countingSoundAlias, 0x24);
CPANGINFO_OFF(earnedPang, 0x28);
CPANGINFO_OFF(earnedBonusPang, 0x2c);
CPANGINFO_OFF(startingPang, 0x30);
CPANGINFO_OFF(startingBonusPang, 0x34);
CPANGINFO_OFF(settledPang, 0x38);
CPANGINFO_OFF(settledBonusPang, 0x3c);
CPANGINFO_OFF(pangBeforeMultiplier, 0x40);
CPANGINFO_OFF(bonusPangBeforeMultiplier, 0x44);
CPANGINFO_OFF(pangAnimationState, 0x48);
CPANGINFO_OFF(displayedPang, 0x4c);
CPANGINFO_OFF(pangAnimationTime, 0x54);
CPANGINFO_OFF(pangBaselineY, 0x58);
CPANGINFO_OFF(pangOpacity, 0x70);
CPANGINFO_OFF(bonusPangAnimationState, 0x74);
CPANGINFO_OFF(displayedBonusPang, 0x78);
CPANGINFO_OFF(bonusPangAnimationTime, 0x80);
CPANGINFO_OFF(bonusEntryState, 0xa0);
CPANGINFO_OFF(bonusEntryValue, 0xa4);
CPANGINFO_OFF(bonusEntryOrder, 0xa8);
CPANGINFO_OFF(bonusEntryAnimationTime, 0xac);
CPANGINFO_OFF(bonusEntryY, 0xb0);
CPANGINFO_OFF(bonusEntryTargetY, 0xb4);
CPANGINFO_OFF(bonusEntryOpacity, 0xc8);
CPANGINFO_OFF(longDriveMultiplierState, 0xcc);
CPANGINFO_OFF(longDriveMultiplier, 0xd0);
CPANGINFO_OFF(approachBonusState, 0xf8);
CPANGINFO_OFF(longPuttBonusState, 0x124);
CPANGINFO_OFF(longPuttBonusPang, 0x128);
CPANGINFO_OFF(longPuttBonusAnimationTime, 0x130);
CPANGINFO_OFF(chipInBonusState, 0x150);
CPANGINFO_OFF(chipInBonusPang, 0x154);
CPANGINFO_OFF(chipInBonusAnimationTime, 0x15c);
CPANGINFO_OFF(pinContactBonusState, 0x17c);
CPANGINFO_OFF(pinContactBonusPang, 0x180);
CPANGINFO_OFF(matchEarlyWinBonusState, 0x1a8);
CPANGINFO_OFF(matchEarlyWinBonusAnimationTime, 0x1b4);
CPANGINFO_OFF(specialShotBonusState, 0x1d4);
CPANGINFO_OFF(specialShotBonusPang, 0x1d8);
CPANGINFO_OFF(specialShotBonusUvX, 0x1ec);
CPANGINFO_OFF(specialShotBonusUvY, 0x1f0);
CPANGINFO_OFF(scoreBonusState, 0x200);
CPANGINFO_OFF(scoreBonusPang, 0x204);
CPANGINFO_OFF(scoreBonusOrder, 0x208);
CPANGINFO_OFF(scoreBonusAnimationTime, 0x20c);
CPANGINFO_OFF(scoreBonusUvY, 0x21c);
CPANGINFO_OFF(powerSpinMultiplierState, 0x22c);
CPANGINFO_OFF(powerCurveMultiplierState, 0x258);
CPANGINFO_OFF(objectHitMultiplierState, 0x284);
CPANGINFO_OFF(objectHitMultiplier, 0x288);
CPANGINFO_OFF(objectHitMultiplierOrder, 0x28c);
CPANGINFO_OFF(objectHitMultiplierAnimationTime, 0x290);
CPANGINFO_OFF(objectHitMultiplierTargetY, 0x298);
CPANGINFO_OFF(holeInOneBonusState, 0x2b0);
CPANGINFO_OFF(holeInOneBonusPang, 0x2b4);
CPANGINFO_OFF(holeInOneBonusAnimationTime, 0x2bc);
CPANGINFO_OFF(roundCompletionBonusState, 0x2dc);
CPANGINFO_OFF(roundCompletionBonusPang, 0x2e0);
CPANGINFO_OFF(roundCompletionBonusAnimationTime, 0x2e8);
CPANGINFO_OFF(pangMultiplierState, 0x308);
CPANGINFO_OFF(pangMultiplier, 0x30c);
CPANGINFO_OFF(pangMultiplierOrder, 0x310);
CPANGINFO_OFF(pangMultiplierAnimationTime, 0x314);
CPANGINFO_OFF(pangMultiplierTargetY, 0x31c);
CPANGINFO_OFF(recoveryBonusState, 0x334);
CPANGINFO_OFF(recoveryBonusPang, 0x338);
CPANGINFO_OFF(recoveryBonusAnimationTime, 0x340);
CPANGINFO_OFF(ballBonusState, 0x360);
CPANGINFO_OFF(ballBonusPang, 0x364);
CPANGINFO_OFF(ballBonusAnimationTime, 0x36c);
CPANGINFO_OFF(ballBonusTargetY, 0x374);
CPANGINFO_OFF(ballBounceBonusState, 0x38c);
CPANGINFO_OFF(ballBounceBonusPang, 0x390);
CPANGINFO_OFF(ballBounceBonusAnimationTime, 0x398);
CPANGINFO_OFF(ballBounceBonusTargetY, 0x3a0);
CPANGINFO_OFF(powerShotBonusState, 0x3b8);
CPANGINFO_OFF(powerShotBonusPang, 0x3bc);
CPANGINFO_OFF(powerShotBonusAnimationTime, 0x3c4);
CPANGINFO_OFF(powerShotBonusTargetY, 0x3cc);
CPANGINFO_OFF(holeDirectionY, 0x3e8);
CPANGINFO_OFF(scoreRelativeToPar, 0x3f0);
CPANGINFO_OFF(clubType, 0x3f4);
CPANGINFO_OFF(shotGroundType, 0x3f8);
CPANGINFO_OFF(clubDistanceYards, 0x3fc);
CPANGINFO_OFF(holeDistanceYards, 0x400);
CPANGINFO_OFF(shotTravelDistanceYards, 0x404);
CPANGINFO_OFF(remainingDistanceYards, 0x408);
CPANGINFO_OFF(scheduledBonusCount, 0x40c);
CPANGINFO_OFF(processedTrajectoryFrame, 0x410);
CPANGINFO_OFF(bonusBannerStartX, 0x414);
CPANGINFO_OFF(bonusBannerStartY, 0x418);
CPANGINFO_OFF(holeInOneBannerElapsed, 0x43c);
CPANGINFO_OFF(ballBonusBannerElapsed, 0x440);
CPANGINFO_OFF(completionDelayElapsed, 0x44c);
CPANGINFO_OFF(unk_450, 0x450);
CPANGINFO_OFF(rewardedBounceCount, 0x468);
CPANGINFO_OFF(lastRewardedBounceFrame, 0x46c);
#undef CPANGINFO_OFF
typedef char CPangInfo::size[(sizeof(CPangInfo) == 0x470) ? 1 : -1];

#endif // CLASSES_CPANGINFO_H

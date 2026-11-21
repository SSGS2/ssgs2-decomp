// CPangInfo: partial layout used by the methods that reference it.
// Non-virtual layout view; unknown meanings retain their offset names.
#ifndef CLASSES_CPANGINFO_H
#define CLASSES_CPANGINFO_H

#include "game/types.h"

class CPangInfo {
public:
    char pad_000[0xc]; // 0x0
    unsigned int textColor; // 0xc: CPangInfo_Draw
    char pad_010[0x4]; // 0x10
    void* textBuffer; // 0x14: CPangInfo_dtor, CPangInfo_Draw
    void* textRenderer; // 0x18: CPangInfo_Draw
    char pad_01C[0x4]; // 0x1c
    unsigned int flags; // 0x20: CPangInfo_Draw, CPangInfo_CalculateShotRewards, CPangInfo_ResetPangAndBonusAnimation
    int countingSoundAlias; // 0x24: CPangInfo_ResetPangAndBonusAnimation, CPangInfo_AnimateRewardEntries
    int earnedPang; // 0x28: CPangInfo_UpdateShotRewardsAndBonuses, CPangInfo_CalculateShotRewards, CPangInfo_ResetPangAndBonusAnimation
    int earnedBonusPang; // 0x2c: CPangInfo_UpdateShotRewardsAndBonuses, CPangInfo_CalculateShotRewards, CPangInfo_ResetPangAndBonusAnimation
    unsigned int startingPang; // 0x30: CPangInfo_ResetBonusAnimations
    unsigned int startingBonusPang; // 0x34: CPangInfo_ResetBonusAnimations
    int settledPang; // 0x38: CPangInfo_AnimateRewardEntries
    int settledBonusPang; // 0x3c: CPangInfo_AnimateRewardEntries
    int pangBeforeMultiplier; // 0x40: CPangInfo_ResetPangAndBonusAnimation, CPangInfo_AnimateRewardEntries
    int bonusPangBeforeMultiplier; // 0x44: CPangInfo_ResetPangAndBonusAnimation, CPangInfo_AnimateRewardEntries
    int pangAnimationState; // 0x48: CPangInfo_Draw, CPangInfo_ResetPangAndBonusAnimation, CPangInfo_AnimateRewardEntries
    int displayedPang; // 0x4c: CPangInfo_Draw, CPangInfo_AnimateRewardEntries
    char pad_050[0x4]; // 0x50
    float pangAnimationTime; // 0x54: CPangInfo_ResetPangAndBonusAnimation, CPangInfo_AnimateRewardEntries
    float pangBaselineY; // 0x58: CPangInfo_Draw
    char pad_05C[0x14]; // 0x5c
    unsigned char pangOpacity; // 0x70: CPangInfo_Draw
    char pad_071[0x3]; // 0x71
    int bonusPangAnimationState; // 0x74: CPangInfo_ResetPangAndBonusAnimation, CPangInfo_AnimateRewardEntries
    int displayedBonusPang; // 0x78: CPangInfo_Draw, CPangInfo_AnimateRewardEntries
    char pad_07C[0x4]; // 0x7c
    float bonusPangAnimationTime; // 0x80: CPangInfo_ResetPangAndBonusAnimation, CPangInfo_AnimateRewardEntries
    char pad_084[0x1c]; // 0x84
    int bonusEntryState; // 0xa0: CPangInfo_UpdateShotRewardsAndBonuses, CPangInfo_Draw, CPangInfo_CalculateShotRewards
    int bonusEntryValue; // 0xa4: CPangInfo_Draw, CPangInfo_AnimateRewardEntries
    int bonusEntryOrder; // 0xa8: CPangInfo_AnimateRewardEntries, CPangInfo_ScheduleBonusEntry
    float bonusEntryAnimationTime; // 0xac: CPangInfo_UpdateShotRewardsAndBonuses, CPangInfo_Draw, CPangInfo_AnimateRewardEntries
    float bonusEntryY; // 0xb0: CPangInfo_Draw, CPangInfo_AnimateRewardEntries
    float bonusEntryTargetY; // 0xb4: CPangInfo_AnimateRewardEntries
    char pad_0B8[0x10]; // 0xb8
    unsigned char bonusEntryOpacity; // 0xc8: CPangInfo_Draw, CPangInfo_AnimateRewardEntries, CPangInfo_ResetBonusAnimations
    char pad_0C9[0x3]; // 0xc9
    int longDriveMultiplierState; // 0xcc: CPangInfo_CalculateShotRewards
    int longDriveMultiplier; // 0xd0: CPangInfo_CalculateShotRewards
    char pad_0D4[0x24]; // 0xd4
    int approachBonusState; // 0xf8: CPangInfo_CalculateShotRewards
    char pad_0FC[0x28]; // 0xfc
    int longPuttBonusState; // 0x124: CPangInfo_CalculateShotRewards, CPangInfo_UpdateLongPuttBonus
    unsigned int longPuttBonusPang; // 0x128: CPangInfo_UpdateLongPuttBonus
    char pad_12C[0x4]; // 0x12c
    float longPuttBonusAnimationTime; // 0x130: CPangInfo_UpdateLongPuttBonus
    char pad_134[0x1c]; // 0x134
    int chipInBonusState; // 0x150: CPangInfo_CalculateShotRewards, CPangInfo_UpdateChipInBonus
    unsigned int chipInBonusPang; // 0x154: CPangInfo_UpdateChipInBonus
    char pad_158[0x4]; // 0x158
    float chipInBonusAnimationTime; // 0x15c: CPangInfo_UpdateChipInBonus
    char pad_160[0x1c]; // 0x160
    int pinContactBonusState; // 0x17c: CPangInfo_CalculateShotRewards
    int pinContactBonusPang; // 0x180: CPangInfo_CalculateShotRewards
    char pad_184[0x24]; // 0x184
    int matchEarlyWinBonusState; // 0x1a8: CPangInfo_CalculateShotRewards
    char pad_1AC[0x8]; // 0x1ac
    float matchEarlyWinBonusAnimationTime; // 0x1b4: CPangInfo_UpdateShotRewardsAndBonuses
    char pad_1B8[0x1c]; // 0x1b8
    int specialShotBonusState; // 0x1d4: CPangInfo_CalculateShotRewards
    int specialShotBonusPang; // 0x1d8: CPangInfo_CalculateShotRewards
    char pad_1DC[0x10]; // 0x1dc
    float specialShotBonusUvX; // 0x1ec: CPangInfo_CalculateShotRewards
    float specialShotBonusUvY; // 0x1f0: CPangInfo_CalculateShotRewards
    char pad_1F4[0xc]; // 0x1f4
    int scoreBonusState; // 0x200: CPangInfo_CalculateShotRewards, CPangInfo_UpdateScoreBonusState, CPangInfo_UpdateConditionalBonusEntry
    int scoreBonusPang; // 0x204: CPangInfo_CalculateShotRewards, CPangInfo_AnimateRewardEntries
    int scoreBonusOrder; // 0x208: CPangInfo_ScheduleBonusEntry
    float scoreBonusAnimationTime; // 0x20c: CPangInfo_UpdateScoreBonusState
    char pad_210[0xc]; // 0x210
    float scoreBonusUvY; // 0x21c: CPangInfo_CalculateShotRewards
    char pad_220[0xc]; // 0x220
    int powerSpinMultiplierState; // 0x22c: CPangInfo_CalculateShotRewards
    char pad_230[0x28]; // 0x230
    int powerCurveMultiplierState; // 0x258: CPangInfo_CalculateShotRewards
    char pad_25C[0x28]; // 0x25c
    int objectHitMultiplierState; // 0x284: CPangInfo_CalculateShotRewards, CPangInfo_ScheduleBonusEntry, CPangInfo_UpdateObjectHitMultiplier
    int objectHitMultiplier; // 0x288: CPangInfo_CalculateShotRewards, CPangInfo_UpdateObjectHitMultiplier, CPangInfo_ResetBonusAnimations
    int objectHitMultiplierOrder; // 0x28c: CPangInfo_ScheduleBonusEntry
    float objectHitMultiplierAnimationTime; // 0x290: CPangInfo_UpdateObjectHitMultiplier
    char pad_294[0x4]; // 0x294
    float objectHitMultiplierTargetY; // 0x298: CPangInfo_ScheduleBonusEntry
    char pad_29C[0x14]; // 0x29c
    int holeInOneBonusState; // 0x2b0: CPangInfo_Draw, CPangInfo_CalculateShotRewards
    int holeInOneBonusPang; // 0x2b4: CPangInfo_CalculateShotRewards
    char pad_2B8[0x4]; // 0x2b8
    float holeInOneBonusAnimationTime; // 0x2bc: CPangInfo_UpdateShotRewardsAndBonuses
    char pad_2C0[0x1c]; // 0x2c0
    int roundCompletionBonusState; // 0x2dc: CPangInfo_CalculateShotRewards
    int roundCompletionBonusPang; // 0x2e0: CPangInfo_CalculateShotRewards
    char pad_2E4[0x4]; // 0x2e4
    float roundCompletionBonusAnimationTime; // 0x2e8: CPangInfo_UpdateShotRewardsAndBonuses
    char pad_2EC[0x1c]; // 0x2ec
    int pangMultiplierState; // 0x308: CPangInfo_UpdateShotRewardsAndBonuses, CPangInfo_CalculateShotRewards, CPangInfo_AnimateRewardEntries
    int pangMultiplier; // 0x30c: CPangInfo_CalculateShotRewards
    int pangMultiplierOrder; // 0x310: CPangInfo_ScheduleBonusEntry
    float pangMultiplierAnimationTime; // 0x314: CPangInfo_UpdateShotRewardsAndBonuses
    char pad_318[0x4]; // 0x318
    float pangMultiplierTargetY; // 0x31c: CPangInfo_AnimateRewardEntries, CPangInfo_ScheduleBonusEntry
    char pad_320[0x14]; // 0x320
    int recoveryBonusState; // 0x334: CPangInfo_CalculateShotRewards, CPangInfo_UpdateRecoveryBonus
    int recoveryBonusPang; // 0x338: CPangInfo_UpdateRecoveryBonus
    char pad_33C[0x4]; // 0x33c
    float recoveryBonusAnimationTime; // 0x340: CPangInfo_UpdateRecoveryBonus
    char pad_344[0x1c]; // 0x344
    int ballBonusState; // 0x360: CPangInfo_Draw, CPangInfo_CalculateShotRewards, CPangInfo_AnimateRewardEntries
    int ballBonusPang; // 0x364: CPangInfo_CalculateShotRewards
    char pad_368[0x4]; // 0x368
    float ballBonusAnimationTime; // 0x36c: CPangInfo_UpdateShotRewardsAndBonuses
    char pad_370[0x4]; // 0x370
    float ballBonusTargetY; // 0x374: CPangInfo_AnimateRewardEntries
    char pad_378[0x14]; // 0x378
    int ballBounceBonusState; // 0x38c: CPangInfo_CalculateShotRewards, CPangInfo_AnimateRewardEntries, CPangInfo_UpdateBallBounceBonus
    int ballBounceBonusPang; // 0x390: CPangInfo_CalculateShotRewards, CPangInfo_UpdateBallBounceBonus
    char pad_394[0x4]; // 0x394
    float ballBounceBonusAnimationTime; // 0x398: CPangInfo_UpdateBallBounceBonus
    char pad_39C[0x4]; // 0x39c
    float ballBounceBonusTargetY; // 0x3a0: CPangInfo_AnimateRewardEntries
    char pad_3A4[0x14]; // 0x3a4
    int powerShotBonusState; // 0x3b8: CPangInfo_CalculateShotRewards, CPangInfo_AnimateRewardEntries
    int powerShotBonusPang; // 0x3bc: CPangInfo_CalculateShotRewards
    char pad_3C0[0x4]; // 0x3c0
    float powerShotBonusAnimationTime; // 0x3c4: CPangInfo_UpdateShotRewardsAndBonuses
    char pad_3C8[0x4]; // 0x3c8
    float powerShotBonusTargetY; // 0x3cc: CPangInfo_AnimateRewardEntries
    char pad_3D0[0x18]; // 0x3d0
    float holeDirectionY; // 0x3e8: CPangInfo_CalculateShotRewards
    char pad_3EC[0x4]; // 0x3ec
    int scoreRelativeToPar; // 0x3f0: CPangInfo_CalculateShotRewards, CPangInfo_UpdateConditionalBonusEntry, CPangInfo_UpdateObjectHitMultiplier
    int clubType; // 0x3f4: CPangInfo_CalculateShotRewards
    int shotGroundType; // 0x3f8: CPangInfo_CalculateShotRewards
    float clubDistanceYards; // 0x3fc: CPangInfo_CalculateShotRewards
    float holeDistanceYards; // 0x400: CPangInfo_CalculateShotRewards
    float shotTravelDistanceYards; // 0x404: CPangInfo_UpdateLongPuttBonus, CPangInfo_UpdateChipInBonus
    float remainingDistanceYards; // 0x408: CPangInfo_UpdateChipInBonus
    int scheduledBonusCount; // 0x40c: CPangInfo_ResetPangAndBonusAnimation, CPangInfo_ScheduleBonusEntry, CPangInfo_ResetBonusAnimations
    int processedTrajectoryFrame; // 0x410: CPangInfo_ResetPangAndBonusAnimation, CPangInfo_UpdateObjectHitMultiplier, CPangInfo_UpdateBallBounceBonus
    float bonusBannerStartX; // 0x414: CPangInfo_ScheduleBonusEntry
    float bonusBannerStartY; // 0x418: CPangInfo_ScheduleBonusEntry
    char pad_41C[0x20]; // 0x41c
    float holeInOneBannerElapsed; // 0x43c: CPangInfo_Ctor, CPangInfo_UpdateShotRewardsAndBonuses, CPangInfo_Draw
    float ballBonusBannerElapsed; // 0x440: CPangInfo_Ctor, CPangInfo_UpdateShotRewardsAndBonuses, CPangInfo_Draw
    char pad_444[0x8]; // 0x444
    float completionDelayElapsed; // 0x44c: CPangInfo_Ctor, CPangInfo_ResetPangAndBonusAnimation
    int unk_450; // 0x450: CPangInfo_Draw
    char pad_454[0x14]; // 0x454
    int rewardedBounceCount; // 0x468: CPangInfo_ResetPangAndBonusAnimation, CPangInfo_UpdateBallBounceBonus, CPangInfo_ResetBonusAnimations
    int lastRewardedBounceFrame; // 0x46c: CPangInfo_ResetPangAndBonusAnimation, CPangInfo_UpdateBallBounceBonus, CPangInfo_ResetBonusAnimations
};

#define CPANGINFO_OFF(f, o) typedef char CPangInfo_off_##f[((int)&((CPangInfo*)0)->f == (o)) ? 1 : -1]
CPANGINFO_OFF(textColor, 0xc);
CPANGINFO_OFF(textBuffer, 0x14);
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

#endif // CLASSES_CPANGINFO_H

// CBonusInfo: layout shared by its display and map methods.
#ifndef CLASSES_CBONUSINFO_H
#define CLASSES_CBONUSINFO_H

struct BonusMapNode;
class CBonusInfo {
public:
    char pad_000[4]; // 0x0
    void* unk_4; // 0x4: CBonusInfo_dtor
    char pad_008[0x8]; // 0x8
    int state; // 0x10: CBonusInfo_DrawRewardsAndItemIcons, CBonusInfo_Ctor
    void* text; // 0x14: CBonusInfo_dtor, CBonusInfo_DrawRewardsAndItemIcons, CBonusInfo_Ctor
    void* bonusOverlay; // 0x18: CBonusInfo_dtor, CBonusInfo_DrawRewardsAndItemIcons, CBonusInfo_Ctor
    void* extraOverlay; // 0x1c: CBonusInfo_dtor, CBonusInfo_DrawRewardsAndItemIcons, CBonusInfo_Ctor
    float elapsed; // 0x20: CBonusInfo_UpdatePangRewards, CBonusInfo_Ctor
    int earnedPang; // 0x24: CBonusInfo_UpdatePangRewards, CBonusInfo_DrawRewardsAndItemIcons, CBonusInfo_Ctor
    int earnedBonusPang; // 0x28: CBonusInfo_UpdatePangRewards, CBonusInfo_DrawRewardsAndItemIcons, CBonusInfo_Ctor
    int multiplierBonus; // 0x2c: CBonusInfo_UpdatePangRewards, CBonusInfo_DrawRewardsAndItemIcons, CBonusInfo_Ctor
    int totalReward; // 0x30: CBonusInfo_Ctor
    unsigned int mapSize; // 0x34: CBonusInfo_dtor, CBonusInfo_Ctor
    BonusMapNode* root; // 0x38: CBonusInfo_dtor
    BonusMapNode* end; // 0x3c: CBonusInfo_dtor, CBonusInfo_Ctor
};

#define CBONUSINFO_OFF(f, o) typedef char CBonusInfo_off_##f[((int)&((CBonusInfo*)0)->f == (o)) ? 1 : -1]
CBONUSINFO_OFF(unk_4, 0x4);
CBONUSINFO_OFF(state, 0x10);
CBONUSINFO_OFF(text, 0x14);
CBONUSINFO_OFF(bonusOverlay, 0x18);
CBONUSINFO_OFF(extraOverlay, 0x1c);
CBONUSINFO_OFF(elapsed, 0x20);
CBONUSINFO_OFF(earnedPang, 0x24);
CBONUSINFO_OFF(earnedBonusPang, 0x28);
CBONUSINFO_OFF(multiplierBonus, 0x2c);
CBONUSINFO_OFF(totalReward, 0x30);
CBONUSINFO_OFF(mapSize, 0x34);
CBONUSINFO_OFF(root, 0x38);
CBONUSINFO_OFF(end, 0x3c);
#undef CBONUSINFO_OFF

#endif // CLASSES_CBONUSINFO_H

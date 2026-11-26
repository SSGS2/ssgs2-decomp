// CBonusInfo: the bonus summary in the HUD.
//
// It draws the earned and bonus Pang with the item icons that produced them,
// and owns the map that caches one icon overlay per item id.
//
// The class is not declared with virtual functions: its vtable comes from the
// original data, so `vtbl` is spelled out as a member.
#ifndef CLASSES_CBONUSINFO_H
#define CLASSES_CBONUSINFO_H

#include "classes/CInfoBase.h"
#include "classes/HudTextPanel.h"

struct BonusMapNode;
class CBonusInfo : public CInfoBase {
public:
    CBonusInfo();
    ~CBonusInfo();

    void UpdatePangRewards(float delta);
    void DrawRewardsAndItemIcons();

    // Load the panel's font through the HUD text panel vtable.
    void SetFont(const char* path) {
        ((HudTextPanelVTable*)this)->loadFont(path, 0, 0);
    }

    void* bonusOverlay;   // 0x18
    void* extraOverlay;   // 0x1c
    float elapsed;        // 0x20
    int earnedPang;       // 0x24
    int earnedBonusPang;  // 0x28
    int multiplierBonus;  // 0x2c
    int totalReward;      // 0x30
    unsigned int mapSize; // 0x34
    BonusMapNode* root;   // 0x38
    BonusMapNode* end;    // 0x3c
};

#define CBONUSINFO_OFF(f, o) typedef char CBonusInfo_off_##f[((int)&((CBonusInfo*)0)->f == (o)) ? 1 : -1]
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
typedef char CBonusInfo_size[(sizeof(CBonusInfo) == 0x40) ? 1 : -1];

#endif // CLASSES_CBONUSINFO_H

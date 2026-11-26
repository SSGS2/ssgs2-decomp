// CClubRangeInfo: the club-range readout drawn in the HUD.
//
// It draws the horizontal distance of a point on the ground from the ball as a
// whole number of yards. The marker actor owns one and moves its world point to
// the aim marker each frame. The class is declared without virtual functions on
// purpose: its vtable is still taken from the original game data.
#ifndef CLASSES_CCLUBRANGEINFO_H
#define CLASSES_CCLUBRANGEINFO_H

#include "game/types.h"
#include "game/wvector.h"
#include "classes/CInfoBase.h"

class CClubRangeInfo : public CInfoBase {
public:
    CClubRangeInfo();
    ~CClubRangeInfo();

    void DrawRangeYardText();

    WVector rangePoint; // 0x18: world point the distance is measured to
};

#define CCLUBRANGEINFO_OFFSET(field, offset) \
    typedef char CClubRangeInfo_offset_##field[((int)&((CClubRangeInfo*)0)->field == (offset)) ? 1 : -1]
CCLUBRANGEINFO_OFFSET(rangePoint, 0x18);
#undef CCLUBRANGEINFO_OFFSET
typedef char CClubRangeInfo_size[(sizeof(CClubRangeInfo) == 0x24) ? 1 : -1];

#endif // CLASSES_CCLUBRANGEINFO_H

#ifndef CLASSES_CCLUBCUSTOMTBL_H
#define CLASSES_CCLUBCUSTOMTBL_H

#include "game/types.h"

// The club-customization table: the entry predicates and the upgrade lookup.
// The table data is reached through a borrowed handle rather than through
// members, so no layout is declared here.
class CClubCustomTbl {
public:
    // Entry predicates, all of which accept.
    int vfunc_1();
    int vfunc_2();
    int vfunc_3();
    int vfunc_4();
    int vfunc_5();
    int vfunc_6();

    // A club's upgrade record, or null.
    static void* FindUpgradeEntry(signed char clubIndex, signed char upgradeIndex);
};

#endif // CLASSES_CCLUBCUSTOMTBL_H

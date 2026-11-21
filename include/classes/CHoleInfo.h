// CHoleInfo: partial layout used by the methods that reference it.
// Non-virtual layout view; unknown meanings retain their offset names.
#ifndef CLASSES_CHOLEINFO_H
#define CLASSES_CHOLEINFO_H

#include "game/types.h"

class CHoleInfo {
public:
    char pad_000[0x10]; // 0x0
    int state; // 0x10: CHoleInfo_Ctor
    char pad_014[0x74]; // 0x14
    float courseOpacity; // 0x88: CHoleInfo_Ctor
    float playerOpacity; // 0x8c: CHoleInfo_Ctor
    float animationTime; // 0x90: CHoleInfo_Ctor
};

#define CHOLEINFO_OFF(f, o) typedef char CHoleInfo_off_##f[((int)&((CHoleInfo*)0)->f == (o)) ? 1 : -1]
CHOLEINFO_OFF(state, 0x10);
CHOLEINFO_OFF(courseOpacity, 0x88);
CHOLEINFO_OFF(playerOpacity, 0x8c);
CHOLEINFO_OFF(animationTime, 0x90);
#undef CHOLEINFO_OFF

#endif // CLASSES_CHOLEINFO_H

// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_RECORD801F31C0_H
#define CLASSES_RECORD801F31C0_H

#include "game/types.h"

class Record801F31C0 {
public:
    char courseId;                         // 0x0: RoundCourseSettings_Init original object access
    char pad_001[0x3];                  // 0x1
    int selectedCourseData;                          // 0x4: RoundCourseSettings_Init original object access
    char golfRuleType;                         // 0x8: RoundCourseSettings_Init original object access
    char pad_009[0x104];                // 0x9
    char roundHoleCount;                       // 0x10d: RoundCourseSettings_Init original object access
    char unk_10E;                       // 0x10e: RoundCourseSettings_Init original object access
    char pad_10F[0x1];                  // 0x10f
    int turnTimeLimitMs;                        // 0x110: RoundCourseSettings_Init original object access
    int unk_114;                        // 0x114: RoundCourseSettings_Init original object access
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define RECORD801F31C0_OFF(f, o) typedef char Record801F31C0_off_##f[((int)&((Record801F31C0*)0)->f == (o)) ? 1 : -1]
RECORD801F31C0_OFF(selectedCourseData, 0x4);
RECORD801F31C0_OFF(golfRuleType, 0x8);
RECORD801F31C0_OFF(roundHoleCount, 0x10d);
RECORD801F31C0_OFF(unk_10E, 0x10e);
RECORD801F31C0_OFF(turnTimeLimitMs, 0x110);
RECORD801F31C0_OFF(unk_114, 0x114);
RECORD801F31C0_OFF(courseId, 0x0);
#undef RECORD801F31C0_OFF

#endif  // CLASSES_RECORD801F31C0_H

// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CCAMERACONTROLLER_H
#define CLASSES_CCAMERACONTROLLER_H

#include "game/types.h"

class CCameraController {
public:
    unsigned int itemEntry;                 // 0x0: 1 use in CCameraController_Ctor
    unsigned int targetItemEntry;                 // 0x4: 1 use in CCameraController_Ctor
    float positionX;                        // 0x8: 1 use in CCameraController_Ctor
    float positionY;                        // 0xc: 1 use in CCameraController_Ctor
    float previousPositionX;                       // 0x10: 1 use in CCameraController_Ctor
    float previousPositionY;                       // 0x14: 1 use in CCameraController_Ctor
    unsigned int moving;                // 0x18: 1 use in CCameraController_Ctor
    void* modelPreview;                       // 0x1c: 2 uses in CCameraController_Ctor
    unsigned int modelIndex;                // 0x20: 1 use in CCameraController_Ctor
    float targetHeading;                       // 0x24: 1 use in CCameraController_Ctor
    float currentHeading;                       // 0x28: 1 use in CCameraController_Ctor
    float previousAnimationTime;                       // 0x2c: 1 use in CCameraController_Ctor
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define CCAMERACONTROLLER_OFF(f, o) typedef char CCameraController_off_##f[((int)&((CCameraController*)0)->f == (o)) ? 1 : -1]
CCAMERACONTROLLER_OFF(itemEntry, 0x0);
CCAMERACONTROLLER_OFF(targetItemEntry, 0x4);
CCAMERACONTROLLER_OFF(positionX, 0x8);
CCAMERACONTROLLER_OFF(positionY, 0xc);
CCAMERACONTROLLER_OFF(previousPositionX, 0x10);
CCAMERACONTROLLER_OFF(previousPositionY, 0x14);
CCAMERACONTROLLER_OFF(moving, 0x18);
CCAMERACONTROLLER_OFF(modelPreview, 0x1c);
CCAMERACONTROLLER_OFF(modelIndex, 0x20);
CCAMERACONTROLLER_OFF(targetHeading, 0x24);
CCAMERACONTROLLER_OFF(currentHeading, 0x28);
CCAMERACONTROLLER_OFF(previousAnimationTime, 0x2c);
#undef CCAMERACONTROLLER_OFF

#endif  // CLASSES_CCAMERACONTROLLER_H

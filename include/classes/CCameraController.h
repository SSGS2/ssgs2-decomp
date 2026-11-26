#ifndef CLASSES_CCAMERACONTROLLER_H
#define CLASSES_CCAMERACONTROLLER_H

#include "game/types.h"

class CCameraController {
public:
    CCameraController(int modelIndex);

    unsigned int itemEntry;       // 0x0
    unsigned int targetItemEntry; // 0x4
    float positionX;              // 0x8
    float positionY;              // 0xc
    float previousPositionX;      // 0x10
    float previousPositionY;      // 0x14
    unsigned int moving;          // 0x18
    void* modelPreview;           // 0x1c
    unsigned int modelIndex;      // 0x20
    float targetHeading;          // 0x24
    float currentHeading;         // 0x28
    float previousAnimationTime;  // 0x2c

    void SetTargetHeading(float heading);
    void* GetItemEntry();
    void SetAnimationByIndex(unsigned int animationIndex);
    void SetModelIndex(unsigned int modelIndex);
    void GetPosition(void* outPosition);
    void ResetPositionToItem();
    void SetItemEntry(void* itemEntry);
    int IsTourActionAnimation();
    void ResetPreview();
    int IsEitherActionAnimation();
    int TryMoveToItem(void* destinationItem);
    void UpdateHeading();
    void Update(float deltaTime, void* mapOrigin);
    void UpdatePosition(void* mapOrigin);
};

#define CCAMERACONTROLLER_OFF(f, o) \
    typedef char CCameraController_off_##f[((int)&((CCameraController*)0)->f == (o)) ? 1 : -1]
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

#endif // CLASSES_CCAMERACONTROLLER_H

// BalloonState80069C: shared state used by Balloon_InitState and balloon methods
// in party_balloon_system.cpp. This layout view emits no constructors, vtable or RTTI.
// 0x8006A294: scale/opacity steps, sine bob, Y rotation, translation and model update.
// 0x80069CA0: resource model, spawn origin and effect initialization.
#ifndef CLASSES_BALLOONSTATE80069C_H
#define CLASSES_BALLOONSTATE80069C_H

struct BalloonState80069C {
    void* vtbl;                 // 0x0
    char name[0x20];            // 0x4
    void* scaleValues;          // 0x24
    unsigned int scaleCount;    // 0x28
    unsigned int scaleCapacity; // 0x2C
    float transform[16];        // 0x30
    float scale;                // 0x70
    short opacity;              // 0x74
    char pad_076[0x2];
    float bobPhase;           // 0x78
    float position[3];        // 0x7C
    float heading;            // 0x88
    float opacityStep;        // 0x8C
    float scaleStep;          // 0x90
    float bobAmplitude;       // 0x94
    float bobPhaseStep;       // 0x98
    float headingStep;        // 0x9C
    int sizeClass;            // 0xA0  1 small, 2 medium, 3 large
    float attachmentScreenX;  // 0xA4
    float attachmentScreenY;  // 0xA8
    int attachmentTransform0; // 0xAC
    int attachmentTransform1; // 0xB0
    int attachmentTransform2; // 0xB4
    int attachmentTransform3; // 0xB8
    void* effectPosition;     // 0xBC
    int attachmentPosition1;  // 0xC0
    int attachmentPosition2;  // 0xC4
    int attachmentPosition3;  // 0xC8
    int forceSphereCollision; // 0xCC
    void* model;              // 0xD0
    unsigned short baseScale; // 0xD4
    short currentIndex;       // 0xD6
    int active;               // 0xD8
    int randomizedSound;      // 0xDC
    float origin[3];          // 0xE0
    short targetScale;        // 0xEC
    short scaleDelta;         // 0xEE
};

#define BALLOONSTATE_OFF(f, o) \
    typedef char BalloonState80069C_off_##f[((int)&((BalloonState80069C*)0)->f == (o)) ? 1 : -1]
BALLOONSTATE_OFF(vtbl, 0x0);
BALLOONSTATE_OFF(name, 0x4);
BALLOONSTATE_OFF(scaleValues, 0x24);
BALLOONSTATE_OFF(scaleCount, 0x28);
BALLOONSTATE_OFF(scaleCapacity, 0x2C);
BALLOONSTATE_OFF(transform, 0x30);
BALLOONSTATE_OFF(scale, 0x70);
BALLOONSTATE_OFF(opacity, 0x74);
BALLOONSTATE_OFF(bobPhase, 0x78);
BALLOONSTATE_OFF(position, 0x7C);
BALLOONSTATE_OFF(heading, 0x88);
BALLOONSTATE_OFF(opacityStep, 0x8C);
BALLOONSTATE_OFF(scaleStep, 0x90);
BALLOONSTATE_OFF(bobAmplitude, 0x94);
BALLOONSTATE_OFF(bobPhaseStep, 0x98);
BALLOONSTATE_OFF(headingStep, 0x9C);
BALLOONSTATE_OFF(sizeClass, 0xA0);
BALLOONSTATE_OFF(attachmentScreenX, 0xA4);
BALLOONSTATE_OFF(attachmentScreenY, 0xA8);
BALLOONSTATE_OFF(attachmentTransform0, 0xAC);
BALLOONSTATE_OFF(attachmentTransform1, 0xB0);
BALLOONSTATE_OFF(attachmentTransform2, 0xB4);
BALLOONSTATE_OFF(attachmentTransform3, 0xB8);
BALLOONSTATE_OFF(effectPosition, 0xBC);
BALLOONSTATE_OFF(attachmentPosition1, 0xC0);
BALLOONSTATE_OFF(attachmentPosition2, 0xC4);
BALLOONSTATE_OFF(attachmentPosition3, 0xC8);
BALLOONSTATE_OFF(forceSphereCollision, 0xCC);
BALLOONSTATE_OFF(model, 0xD0);
BALLOONSTATE_OFF(baseScale, 0xD4);
BALLOONSTATE_OFF(currentIndex, 0xD6);
BALLOONSTATE_OFF(active, 0xD8);
BALLOONSTATE_OFF(randomizedSound, 0xDC);
BALLOONSTATE_OFF(origin, 0xE0);
BALLOONSTATE_OFF(targetScale, 0xEC);
BALLOONSTATE_OFF(scaleDelta, 0xEE);
#undef BALLOONSTATE_OFF
#define AsBalloonState80069C(p) ((BalloonState80069C*)(p))

#endif

#ifndef CLASSES_CTRAJECTORY_H
#define CLASSES_CTRAJECTORY_H

#include "game/types.h"

// The actor that owns the shot's presentation effects: the comet trail, the
// ball ribbon, skid marks, wind streaks and the projected ground marker.
class CTrajectory {
public:
    static void* GetRTTI();
    static void* DrawThunk18(void* p);
    static void InitializeUnitConstants();

    void OnPreLoadInit();
    void OnLoad();
    void OnProcess(float delta);
    void OnDestroy();
    void HandleMsg(const void* msg);

    void Draw();

    ~CTrajectory();

    void* vtbl; // 0x0: original vtable pointer
    char pad_004[0x4];
    int unk_8; // 0x8: set to 1 when the actor is constructed
    char pad_00C[0xc];
    void* secondaryVtable; // 0x18: secondary message interface
    void* activeEffect0;   // 0x1c
    void* activeEffect1;   // 0x20
    void* activeEffect2;   // 0x24
    void* activeEffect3;   // 0x28
    void* activeEffect4;   // 0x2c
    void* comet;           // 0x30
    void* doughnut;        // 0x34
    void* puttLine;        // 0x38
    void* ballTrack;       // 0x3c
    void* skidMark;        // 0x40
    void* ballWindTrack;   // 0x44
    void* cobraSkidMark;   // 0x48
    void* groundMarker;    // 0x4c
};

#define CTRAJECTORY_OFF(f, o) typedef char CTrajectory_off_##f[((int)&((CTrajectory*)0)->f == (o)) ? 1 : -1]
CTRAJECTORY_OFF(activeEffect0, 0x1c);
CTRAJECTORY_OFF(activeEffect1, 0x20);
CTRAJECTORY_OFF(activeEffect2, 0x24);
CTRAJECTORY_OFF(activeEffect3, 0x28);
CTRAJECTORY_OFF(activeEffect4, 0x2c);
CTRAJECTORY_OFF(comet, 0x30);
CTRAJECTORY_OFF(doughnut, 0x34);
CTRAJECTORY_OFF(puttLine, 0x38);
CTRAJECTORY_OFF(ballTrack, 0x3c);
CTRAJECTORY_OFF(skidMark, 0x40);
CTRAJECTORY_OFF(ballWindTrack, 0x44);
CTRAJECTORY_OFF(cobraSkidMark, 0x48);
CTRAJECTORY_OFF(groundMarker, 0x4c);
CTRAJECTORY_OFF(unk_8, 0x8);
CTRAJECTORY_OFF(secondaryVtable, 0x18);
#undef CTRAJECTORY_OFF

#endif // CLASSES_CTRAJECTORY_H

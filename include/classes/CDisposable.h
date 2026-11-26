#ifndef CLASSES_CDISPOSABLE_H
#define CLASSES_CDISPOSABLE_H

#include "game/types.h"

class CDisposable {
public:
    void* vtbl;                      // 0x0: original vtable pointer
    void* secondaryVtable;           // 0x4
    unsigned int flags;              // 0x8
    char pad_00C[0x18];              // 0xc
    void* puppet;                    // 0x24
    float matrixXX;                  // 0x28
    char pad_02C[0xc];               // 0x2c
    float matrixYY;                  // 0x38
    char pad_03C[0xc];               // 0x3c
    float matrixZZ;                  // 0x48
    char pad_04C[0xc];               // 0x4c
    float animationTime;             // 0x58
    char pad_05C[0x4];               // 0x5c
    int animationRange;              // 0x60
    char pad_064[0x14];              // 0x64
    float rotationZ;                 // 0x78
    char pad_07C[0x8];               // 0x7c
    float heading;                   // 0x84
    char pad_088[0x20];              // 0x88
    int history;                     // 0xa8
    unsigned char opacity;           // 0xac
    unsigned char animationEnabled;  // 0xad
    char pad_0AE[0x2];               // 0xae
    float ballSpawnOffsetDistance;   // 0xb0
    int opacityKeyframeCount;        // 0xb4
    void* opacityKeyframeNext;       // 0xb8
    void* opacityKeyframePrevious;   // 0xbc
    int targetPoints;                // 0xc0
    int targetPointCount;            // 0xc4
    int targetPointCapacity;         // 0xc8
    char pad_0CC[0x4];               // 0xcc
    int pathPoints;                  // 0xd0
    int pathPointCount;              // 0xd4
    int pathPointCapacity;           // 0xd8
    char pad_0DC[0x40];              // 0xdc
    int sequenceHandleCount;         // 0x11c
    unsigned int sequenceHandleNext; // 0x120
    void* sequenceHandlePrevious;    // 0x124
    int sprayHandleCount;            // 0x128
    void* sprayHandleNext;           // 0x12c
    void* sprayHandlePrevious;       // 0x130

    void vfunc_27();

    int vfunc_35();
    void InitAtPathStart();
    void CreateAndStoreAttachedEffect(void* event);
    static void SetAnimationEnabled(void* ptr, int enable);

    void vfunc_24();
    void vfunc_23();
    void vfunc_22();
    void vfunc_21();
    void vfunc_20();
    void vfunc_10();
    void vfunc_18();
    void vfunc_17();
    void vfunc_16();
    void vfunc_15();
    void vfunc_14();
    void vfunc_11();
    void vfunc_9();
    void vfunc_8();
    void vfunc_7();
    void vfunc_6();

    CDisposable(void* definition);
    ~CDisposable();
    void DtorThunk4(int deleteFlag);
};

#define CDISPOSABLE_OFF(f, o) typedef char CDisposable_off_##f[((int)&((CDisposable*)0)->f == (o)) ? 1 : -1]
CDISPOSABLE_OFF(secondaryVtable, 0x4);
CDISPOSABLE_OFF(flags, 0x8);
CDISPOSABLE_OFF(puppet, 0x24);
CDISPOSABLE_OFF(matrixXX, 0x28);
CDISPOSABLE_OFF(matrixYY, 0x38);
CDISPOSABLE_OFF(matrixZZ, 0x48);
CDISPOSABLE_OFF(animationTime, 0x58);
CDISPOSABLE_OFF(animationRange, 0x60);
CDISPOSABLE_OFF(rotationZ, 0x78);
CDISPOSABLE_OFF(heading, 0x84);
CDISPOSABLE_OFF(history, 0xa8);
CDISPOSABLE_OFF(opacity, 0xac);
CDISPOSABLE_OFF(animationEnabled, 0xad);
CDISPOSABLE_OFF(ballSpawnOffsetDistance, 0xb0);
CDISPOSABLE_OFF(opacityKeyframeCount, 0xb4);
CDISPOSABLE_OFF(opacityKeyframeNext, 0xb8);
CDISPOSABLE_OFF(opacityKeyframePrevious, 0xbc);
CDISPOSABLE_OFF(targetPoints, 0xc0);
CDISPOSABLE_OFF(targetPointCount, 0xc4);
CDISPOSABLE_OFF(targetPointCapacity, 0xc8);
CDISPOSABLE_OFF(pathPoints, 0xd0);
CDISPOSABLE_OFF(pathPointCount, 0xd4);
CDISPOSABLE_OFF(pathPointCapacity, 0xd8);
CDISPOSABLE_OFF(sequenceHandleCount, 0x11c);
CDISPOSABLE_OFF(sequenceHandleNext, 0x120);
CDISPOSABLE_OFF(sequenceHandlePrevious, 0x124);
CDISPOSABLE_OFF(sprayHandleCount, 0x128);
CDISPOSABLE_OFF(sprayHandleNext, 0x12c);
CDISPOSABLE_OFF(sprayHandlePrevious, 0x130);
#undef CDISPOSABLE_OFF

#endif // CLASSES_CDISPOSABLE_H

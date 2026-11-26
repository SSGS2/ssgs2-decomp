#ifndef CLASSES_CFADE_H
#define CLASSES_CFADE_H

#include "game/types.h"

// A window fade: alpha moves by step toward fully transparent or fully opaque on
// each update, then the fade goes idle.
class CFade {
public:
    // Initialize a transparent, idle fade with a 16-alpha step.
    void Init();
    // Free this allocation only when size is positive; returns this.
    void* Dtor(int size);

    // Set the amount of alpha change per update.
    void SetStep(unsigned char value);
    // Move alpha toward its endpoint; report completion on the following idle update.
    // The caller passes a float that this does not read.
    int Update(float);
    // Fade the window out from fully opaque, or in from fully transparent.
    void StartFadeOut();
    void StartFadeIn();

    int direction;       // 0x0  0 idle, 1 decreasing alpha, 2 increasing alpha
    unsigned char alpha; // 0x4
    unsigned char step;  // 0x5
};

#define CFADE_OFF(f, o) typedef char CFade_off_##f[((int)&((CFade*)0)->f == (o)) ? 1 : -1]
CFADE_OFF(alpha, 0x4);
CFADE_OFF(step, 0x5);
#undef CFADE_OFF

#endif // CLASSES_CFADE_H

#ifndef CLASSES_CGALLERY_H
#define CLASSES_CGALLERY_H

#include "game/types.h"

// CGallery: a gallery object. Only the fields used by the gallery's countdown are
// named here; the rest of the layout is not recovered yet.
extern "C" {
void __ct__18NativeBase80201D18Fv(void*);
extern char __vt__8CGallery[];
extern float kFloat_0_Gallery;
}

class CGallery {
public:
    // Construct the base, install the gallery vtable and stop the countdown.
    CGallery() {
        __ct__18NativeBase80201D18Fv(this);
        float f0 = kFloat_0_Gallery;
        vtbl = __vt__8CGallery;
        remainingTime = f0;
        countdownEnabled = 0;
    }

    void HandleMsg(const unsigned int* event);
    static void* GetRTTI();

    void* image_vfunc_0(int flag);

    static int image_vfunc_1();
    static int image_vfunc_2();
    static int image_vfunc_3();
    static int image_vfunc_4();

    // Allocate a gallery object with a stopped, zero-valued countdown.
    static CGallery* Create();

    // Decrease the countdown only while its flag is enabled.
    void OnProcess(float delta);

    ~CGallery();

    void* vtbl; // 0x00
    char pad_04[0x14];
    float remainingTime;          // 0x18
    signed char countdownEnabled; // 0x1C
    char pad_1D[0x3C - 0x1D];
};

typedef char CGallery_size[(sizeof(CGallery) == 0x3C) ? 1 : -1];

#endif // CLASSES_CGALLERY_H

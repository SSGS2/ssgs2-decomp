#ifndef CLASSES_CCLOCK_H
#define CLASSES_CCLOCK_H

#include "game/types.h"

class CClock {
public:
    void* vtbl;                  // 0x0: original vtable pointer
    char pad_004[0x18];          // 0x4
    float countdownDisplayValue; // 0x1c
    unsigned int
        displayColor; // 0x20: RGBA color sent to IActor_DispatchMessage in CClock_OnProcess; initialized/reset in CClock_HandleMsg
    char pad_024[0x1];     // 0x24
    unsigned char running; // 0x25: countdown enabled by CClock_HandleMsg case 0, disabled by case 1 and expiry

    void OnProcess(float delta);
    void HandleMsg(const void* event);
    static void* GetRTTI();

    void OnInit();

    ~CClock();
};

#define CCLOCK_OFF(f, o) typedef char CClock_off_##f[((int)&((CClock*)0)->f == (o)) ? 1 : -1]
CCLOCK_OFF(countdownDisplayValue, 0x1c);
CCLOCK_OFF(displayColor, 0x20);
CCLOCK_OFF(running, 0x25);
#undef CCLOCK_OFF

#endif // CLASSES_CCLOCK_H

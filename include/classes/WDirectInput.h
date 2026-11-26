#ifndef CLASSES_WDIRECTINPUT_H
#define CLASSES_WDIRECTINPUT_H

#include "game/types.h"

class WDirectInput {
public:
    void* vtbl;               // 0x0: WDirectInput_Ctor (0x802A0144)
    unsigned char active;     // 0x4: WDirectInput_Ctor (0x802A0144), WInputDev_SetActive (0x802A02E4)
    unsigned char bUpdated;   // 0x5: WDirectInput_Ctor (0x802A0144), WInputDev_IsUpdated (0x802A02CC)
    char pad_006[0x2];        // 0x6
    int dwLastInputTime;      // 0x8: WInputDev_ResetInputTime (0x802A0294), WInputDev_GetLastInputTime (0x802A02C4)
    int deviceFactoryContext; // 0xc: WDirectInput_Ctor (0x802A0144), WDirectInput_MakeClone (0x802A01C4)
};

#define WDIRECTINPUT_OFF(f, o) typedef char WDirectInput_off_##f[((int)&((WDirectInput*)0)->f == (o)) ? 1 : -1]
WDIRECTINPUT_OFF(active, 0x4);
WDIRECTINPUT_OFF(bUpdated, 0x5);
WDIRECTINPUT_OFF(dwLastInputTime, 0x8);
WDIRECTINPUT_OFF(deviceFactoryContext, 0xc);
WDIRECTINPUT_OFF(vtbl, 0x0);
#undef WDIRECTINPUT_OFF

#endif // CLASSES_WDIRECTINPUT_H

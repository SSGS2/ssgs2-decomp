#ifndef CLASSES_CWAVELET_H
#define CLASSES_CWAVELET_H

#include "game/types.h"

class CWavelet {
public:
    void* vtbl;             // 0x0: original vtable pointer
    char pad_004[0xc];      // 0x4
    unsigned int texHandle; // 0x10
    float width;            // 0x14
    float timingScale;      // 0x18
    float deceleration;     // 0x1c
    float fadeInTime;       // 0x20
    float fadeOutTime;      // 0x24
    float delay;            // 0x28

    void ConfigureFromBeachParameterName(const char* name);

    void DrawLayers();
    void UpdateLayers(float dt);

    ~CWavelet();
};

#define CWAVELET_OFF(f, o) typedef char CWavelet_off_##f[((int)&((CWavelet*)0)->f == (o)) ? 1 : -1]
CWAVELET_OFF(texHandle, 0x10);
CWAVELET_OFF(width, 0x14);
CWAVELET_OFF(timingScale, 0x18);
CWAVELET_OFF(deceleration, 0x1c);
CWAVELET_OFF(fadeInTime, 0x20);
CWAVELET_OFF(fadeOutTime, 0x24);
CWAVELET_OFF(delay, 0x28);
#undef CWAVELET_OFF

#endif // CLASSES_CWAVELET_H

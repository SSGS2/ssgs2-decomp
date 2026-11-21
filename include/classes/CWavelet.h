// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CWAVELET_H
#define CLASSES_CWAVELET_H

#include "game/types.h"

class CWavelet {
public:
    void* vtbl;                         // 0x0: original vtable pointer
    char pad_004[0xc];                  // 0x4
    unsigned int texHandle;                // 0x10: 3 uses in CWavelet_DrawLayers, CWavelet_ConfigureFromBeachParameterName
    float width;                       // 0x14: 2 uses in CWavelet_ConfigureFromBeachParameterName
    float timingScale;                       // 0x18: 1 use in CWavelet_ConfigureFromBeachParameterName
    float deceleration;                       // 0x1c: 1 use in CWavelet_ConfigureFromBeachParameterName
    float fadeInTime;                       // 0x20: 1 use in CWavelet_ConfigureFromBeachParameterName
    float fadeOutTime;                       // 0x24: 1 use in CWavelet_ConfigureFromBeachParameterName
    float delay;                       // 0x28: 1 use in CWavelet_ConfigureFromBeachParameterName
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define CWAVELET_OFF(f, o) typedef char CWavelet_off_##f[((int)&((CWavelet*)0)->f == (o)) ? 1 : -1]
CWAVELET_OFF(texHandle, 0x10);
CWAVELET_OFF(width, 0x14);
CWAVELET_OFF(timingScale, 0x18);
CWAVELET_OFF(deceleration, 0x1c);
CWAVELET_OFF(fadeInTime, 0x20);
CWAVELET_OFF(fadeOutTime, 0x24);
CWAVELET_OFF(delay, 0x28);
#undef CWAVELET_OFF

#endif  // CLASSES_CWAVELET_H

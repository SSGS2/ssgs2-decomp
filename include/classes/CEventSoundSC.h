#ifndef CLASSES_CEVENTSOUNDSC_H
#define CLASSES_CEVENTSOUNDSC_H

#include "game/types.h"

class CEventSoundSC {
public:
    void* vtbl;                        // 0x0: original vtable pointer
    unsigned short startFrame;         // 0x4
    char pad_006[0x2];                 // 0x6
    signed char soundNameStorageFlags; // 0x8
    unsigned char soundNameFirstChar;  // 0x9
    char pad_00A[0x2];                 // 0xa
    unsigned int soundNameLength;      // 0xc
    void* soundNameData;               // 0x10
    unsigned char initialPitchPercent; // 0x14
    char pad_015[0x1];                 // 0x15
    short pitchDeltaPercent;           // 0x16
    unsigned int pitchDurationFrames;  // 0x18
};

#define CEVENTSOUNDSC_OFF(f, o) typedef char CEventSoundSC_off_##f[((int)&((CEventSoundSC*)0)->f == (o)) ? 1 : -1]
CEVENTSOUNDSC_OFF(startFrame, 0x4);
CEVENTSOUNDSC_OFF(soundNameStorageFlags, 0x8);
CEVENTSOUNDSC_OFF(soundNameFirstChar, 0x9);
CEVENTSOUNDSC_OFF(soundNameLength, 0xc);
CEVENTSOUNDSC_OFF(soundNameData, 0x10);
CEVENTSOUNDSC_OFF(initialPitchPercent, 0x14);
CEVENTSOUNDSC_OFF(pitchDeltaPercent, 0x16);
CEVENTSOUNDSC_OFF(pitchDurationFrames, 0x18);
#undef CEVENTSOUNDSC_OFF

#endif // CLASSES_CEVENTSOUNDSC_H

#ifndef CLASSES_CEVENTSOUNDSC_SND_SE_PITCH_H
#define CLASSES_CEVENTSOUNDSC_SND_SE_PITCH_H

#include "game/types.h"

class CEventSoundSC_SND_SE_PITCH {
public:
    void* vtbl;                         // 0x0: original vtable pointer
    char pad_004[0x4];                  // 0x4
    unsigned int soundNameStorageFlags; // 0x8
};

#define CEVENTSOUNDSC_SND_SE_PITCH_OFF(f, o) \
    typedef char CEventSoundSC_SND_SE_PITCH_off_##f[((int)&((CEventSoundSC_SND_SE_PITCH*)0)->f == (o)) ? 1 : -1]
CEVENTSOUNDSC_SND_SE_PITCH_OFF(soundNameStorageFlags, 0x8);
#undef CEVENTSOUNDSC_SND_SE_PITCH_OFF

#endif // CLASSES_CEVENTSOUNDSC_SND_SE_PITCH_H

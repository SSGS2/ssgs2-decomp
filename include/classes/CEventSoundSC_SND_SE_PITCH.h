// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CEVENTSOUNDSC_SND_SE_PITCH_H
#define CLASSES_CEVENTSOUNDSC_SND_SE_PITCH_H

#include "game/types.h"

class CEventSoundSC_SND_SE_PITCH {
public:
    void* vtbl;                         // 0x0: original vtable pointer
    char pad_004[0x4];                  // 0x4
    unsigned int soundNameStorageFlags;                 // 0x8: 3 uses in CEventSoundSC_SND_SE_PITCH_UpdateEffectPitch, EventSoundSePitch

    // offsets whose address is passed on (sub-objects or arrays, not placed): 0x9, 0xc
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define CEVENTSOUNDSC_SND_SE_PITCH_OFF(f, o) typedef char CEventSoundSC_SND_SE_PITCH_off_##f[((int)&((CEventSoundSC_SND_SE_PITCH*)0)->f == (o)) ? 1 : -1]
CEVENTSOUNDSC_SND_SE_PITCH_OFF(soundNameStorageFlags, 0x8);
#undef CEVENTSOUNDSC_SND_SE_PITCH_OFF

#endif  // CLASSES_CEVENTSOUNDSC_SND_SE_PITCH_H

// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CEVENTSOUNDSC_H
#define CLASSES_CEVENTSOUNDSC_H

#include "game/types.h"

class CEventSoundSC {
public:
    void* vtbl;                         // 0x0: original vtable pointer
    unsigned short startFrame;               // 0x4: 1 use in CEventSoundSC_SND_SE_PITCH_UpdateEffectPitch
    char pad_006[0x2];                  // 0x6
    signed char soundNameStorageFlags;                  // 0x8: 4 uses in CEventSoundSC_SND_SE_PITCH_UpdateEffectPitch, CEventSoundSC_SND_BGM_PLAY_RequestBgmChange (CONFLICT sizes [1, 4])
    unsigned char soundNameFirstChar;                // 0x9: 2 uses in CEventSoundSC_SND_SE_PITCH_UpdateEffectPitch, CEventSoundSC_SND_BGM_PLAY_RequestBgmChange
    char pad_00A[0x2];                  // 0xa
    unsigned int soundNameLength;                 // 0xc: 4 uses in CEventSoundSC_SND_SE_STOP_StopSoundAlias, CEventSoundSC_SND_SE_REQ_PlayNamedSoundOnce, CEventSoundSC_SND_SE_PLAY_TryPlaySoundEffect...
    void* soundNameData;                       // 0x10: 2 uses in CEventSoundSC_SND_SE_PITCH_UpdateEffectPitch
    unsigned char initialPitchPercent;               // 0x14: 2 uses in CEventSoundSC_SND_SE_PLAY_TryPlaySoundEffect, CEventSoundSC_SND_SE_PITCH_UpdateEffectPitch
    char pad_015[0x1];                  // 0x15
    short pitchDeltaPercent;                       // 0x16: 1 use in CEventSoundSC_SND_SE_PITCH_UpdateEffectPitch
    unsigned int pitchDurationFrames;                // 0x18: 3 uses in CEventSoundSC_SND_SE_PITCH_UpdateEffectPitch

    // offsets whose address is passed on (sub-objects or arrays, not placed): 0x9
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
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

#endif  // CLASSES_CEVENTSOUNDSC_H

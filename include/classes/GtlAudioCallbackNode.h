// GtlAudioCallbackNode: layout view; methods in gtl_audio_controller_effects.cpp (gtl_AudioController_RegisterCallback stores callback; gtl_AudioController_DispatchCallbacks calls it).
// No virtual methods, inheritance, or emitted runtime objects.
#ifndef CLASSES_AUDIOCALLBACK80404DCC_H
#define CLASSES_AUDIOCALLBACK80404DCC_H

class GtlAudioCallbackNode {
public:
    void* next;     // 0x0
    void* previous; // 0x4
    void* callback; // 0x8
};

#define OFF(f, o) typedef char AudioCallback80404DCC_off_##f[((int)&((GtlAudioCallbackNode*)0)->f == (o)) ? 1 : -1]
OFF(next, 0x0);
OFF(previous, 0x4);
OFF(callback, 0x8);
#undef OFF

#define AsAudioCallback80404DCC(p) ((GtlAudioCallbackNode*)(p))

#endif

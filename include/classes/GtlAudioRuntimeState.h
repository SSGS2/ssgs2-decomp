// Audio singleton g_gtlAudioManager, lazily initialized by methods in gtl_audio_spatial_and_runtime.cpp.
// Layout view only. Original symbols and initialization order remain unchanged.
#ifndef CLASSES_GTLAUDIORUNTIMESTATE_H
#define CLASSES_GTLAUDIORUNTIMESTATE_H
struct GtlAudioRuntimeState {
    char pad_0[4];
    void* (*allocate)(unsigned int); // 0x4: gtl_Audio_Alloc (0x803FE48C) calls this callback.
    void (*free)(void*);             // 0x8: gtl_Audio_Free calls this callback.
    char pad_C[0xC];
    unsigned int outputMode;         // 0x18: gtl_Audio_SetOutputMode maps 1..4 to controller modes 0..3.
    unsigned int cachedOutputOption; // 0x1C: GtlAudio_SetCachedOutputOption compares and stores its argument.
    float mainSend;                  // 0x20: GtlAudio_GetMainSend returns this float; meaning unknown.
    float listener[12];              // 0x24: gtl_Audio_GetListener returns this 12-word block.
};
#define OFF(f, o) typedef char GtlAudioRuntimeState_off_##f[((int)&((GtlAudioRuntimeState*)0)->f == (o)) ? 1 : -1]
OFF(allocate, 0x4);
OFF(free, 0x8);
OFF(outputMode, 0x18);
OFF(cachedOutputOption, 0x1C);
OFF(mainSend, 0x20);
OFF(listener, 0x24);
#undef OFF
#define AsGtlAudioRuntimeState(p) ((GtlAudioRuntimeState*)(p))
#endif

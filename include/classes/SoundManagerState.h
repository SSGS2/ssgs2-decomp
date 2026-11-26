// SoundManagerState. Receiver shared by the sound manager functions in sound_manager.cpp.
// Explicit layout view without inheritance or virtual methods.
#ifndef CLASSES_SOUNDMANAGERSTATE_H
#define CLASSES_SOUNDMANAGERSTATE_H

class SoundManagerState {
public:
    char pad_000[0x4];
    float
        volumeScale; // 0x4: SoundManager_ResetPlaybackState, SoundManager_ApplyBufferVolumes, SoundManager_SetBufferVolume, SoundManager_PlayVoiceByName, SoundManager_SetNamedVolumeScale
    char pad_008[0x18];
    float volume; // 0x20: SoundManager_ApplyStreamVolume scales channel volume before gtl_ISoundStream_SetVolume
    char pad_024[0x4];
    float musicVolumeTarget; // 0x28: SoundManager_GetVolumeByMode
    char pad_02C[0xC];
    float
        transitionDuration; // 0x38: SoundManager_PlayVoiceByName passes this duration to SoundChannelGroup_StopWithFade
    unsigned char
        enabled; // 0x3C: SoundManager_SetEnabled assigns; Sound_PlayByName and Sound_PlayByNameWithParameters return when zero
    char pad_03D[0x3];
    unsigned int
        outputMode; // 0x40: SoundManager_ResetPlaybackState, SoundManager_SetOutputMode, SoundManager_RequestBgmChange
    unsigned int
        playbackPauseDepth; // 0x44: SoundManager_ResetPlaybackState, SoundManager_PausePlayback, SoundManager_ResumePlayback
    int pauseDepth; // 0x48: SoundManager_PausePrimaryStreams increments; SoundManager_ResumePrimaryStreams decrements and resumes at zero
    float unk_4C; // 0x4C: SoundManager_ResetPlaybackState
    float unk_50; // 0x50: SoundManager_ResetPlaybackState
    float unk_54; // 0x54: SoundManager_ResetPlaybackState
    void*
        state; // 0x58: SoundManager_ResetPlaybackState initializes two channel groups and resource lists through this pointer
};

#define OFF(f, o) typedef char SoundManagerState_off_##f[((int)&((SoundManagerState*)0)->f == (o)) ? 1 : -1]
OFF(volumeScale, 0x4);
OFF(volume, 0x20);
OFF(musicVolumeTarget, 0x28);
OFF(transitionDuration, 0x38);
OFF(enabled, 0x3C);
OFF(outputMode, 0x40);
OFF(playbackPauseDepth, 0x44);
OFF(pauseDepth, 0x48);
OFF(unk_4C, 0x4C);
OFF(unk_50, 0x50);
OFF(unk_54, 0x54);
OFF(state, 0x58);
#undef OFF
#define AsSoundManagerState(p) ((SoundManagerState*)(p))

#endif

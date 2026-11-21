// SoundPlaybackState. 0x64-byte playback record; indexed group records begin four bytes after their group origin.
// Explicit layout view without inheritance or virtual methods.
#ifndef CLASSES_SOUNDPLAYBACKSTATE_H
#define CLASSES_SOUNDPLAYBACKSTATE_H

class SoundPlaybackState {
public:
    void* source; // 0x0: SoundStreamPlayback_Start invokes sound operations through this pointer
    float volume; // 0x4: SoundStreamPlayback_Start multiplies by baseVolume and fade value before gtl_ISoundStream_SetVolume
    float pitch; // 0x8: SoundStreamPlayback_Start multiplies by basePitch before gtl_ISoundStream_SetPitch
    int playState; // 0xC: SoundStreamPlayback_Start transitions stopping state 3 to playing state 2
    unsigned int nameFlags; // 0x10: SoundPlayback_StopWithOptionalFade, SoundPlayback_MatchesFileName
    unsigned int nameLength; // 0x14: SoundPlayback_StopWithOptionalFade, SoundPlayback_MatchesFileName
    void* nameData; // 0x18: SoundPlayback_MatchesFileName
    unsigned char loopEnabled; // 0x1C: SoundPlayback_StopWithOptionalFade, SoundStreamPlayback_Start
    unsigned char fadeInEnabled; // 0x1D: SoundPlayback_StopWithOptionalFade, SoundStreamPlayback_Start
    unsigned char useDolbyBank; // 0x1E: SoundPlayback_StopWithOptionalFade, SoundPlaybackState_BeginBankLoad
    char pad_01F[0x1];
    float transitionDuration; // 0x20: SoundPlayback_StopWithOptionalFade, SoundStreamPlayback_Start
    int fadeMode; // 0x24: SoundChannelGroup_StopWithFade, AAD2C_refresh, AAD2C_reset playback record
    float fadeValue; // 0x28: SoundStreamPlayback_Start
    float fadeStartValue; // 0x2C: SoundChannelGroup_StopWithFade, AAD2C_refresh, AAD2C_reset playback record
    float fadeTargetValue; // 0x30: SoundChannelGroup_StopWithFade, AAD2C_refresh, AAD2C_reset playback record
    float fadeDuration; // 0x34: SoundChannelGroup_StopWithFade, AAD2C_refresh, AAD2C_reset playback record
    float fadeElapsed; // 0x38: SoundChannelGroup_StopWithFade, AAD2C_refresh, AAD2C_reset playback record
    float fadeActiveBits; // 0x3C: SoundChannelGroup_StopWithFade, AAD2C_refresh, AAD2C_reset playback record
    unsigned int filePathFlags; // 0x40: SoundPlayback_StopWithOptionalFade
    unsigned int filePathLength; // 0x44: SoundPlayback_StopWithOptionalFade
    void* filePathData; // 0x48: SoundChannelGroup_StopWithFade, AAD2C_refresh, AAD2C_reset playback record
    unsigned int resourcePathFlags; // 0x4C: SoundPlayback_StopWithOptionalFade
    unsigned int resourcePathLength; // 0x50: SoundPlayback_StopWithOptionalFade
    void* resourcePathData; // 0x54: SoundChannelGroup_StopWithFade, AAD2C_refresh, AAD2C_reset playback record
    void* sharedFile; // 0x58: SoundPlayback_StopWithOptionalFade destroys with CSharedFileHandle_Destroy
    float baseVolume; // 0x5C: SoundStreamPlayback_Start multiplier passed to gtl_ISoundStream_SetVolume
    float basePitch; // 0x60: SoundStreamPlayback_Start multiplier passed to gtl_ISoundStream_SetPitch
};

#define OFF(f, o) typedef char SoundPlaybackState_off_##f[((int)&((SoundPlaybackState*)0)->f == (o)) ? 1 : -1]
OFF(source, 0x0);
OFF(volume, 0x4);
OFF(pitch, 0x8);
OFF(playState, 0xC);
OFF(nameFlags, 0x10);
OFF(nameLength, 0x14);
OFF(nameData, 0x18);
OFF(loopEnabled, 0x1C);
OFF(fadeInEnabled, 0x1D);
OFF(useDolbyBank, 0x1E);
OFF(transitionDuration, 0x20);
OFF(fadeMode, 0x24);
OFF(fadeValue, 0x28);
OFF(fadeStartValue, 0x2C);
OFF(fadeTargetValue, 0x30);
OFF(fadeDuration, 0x34);
OFF(fadeElapsed, 0x38);
OFF(fadeActiveBits, 0x3C);
OFF(filePathFlags, 0x40);
OFF(filePathLength, 0x44);
OFF(filePathData, 0x48);
OFF(resourcePathFlags, 0x4C);
OFF(resourcePathLength, 0x50);
OFF(resourcePathData, 0x54);
OFF(sharedFile, 0x58);
OFF(baseVolume, 0x5C);
OFF(basePitch, 0x60);
#undef OFF
#define AsSoundPlaybackState(p) ((SoundPlaybackState*)(p))

#endif

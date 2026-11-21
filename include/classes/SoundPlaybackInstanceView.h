// Sound-buffer entries allocated by ADD40Create / SoundManager_CreateSoundInstance in sound_manager.cpp.
// Layout view only; no constructors, virtual methods or runtime data.
#ifndef CLASSES_SOUNDBUFFERPLAYBACK802A8798_H
#define CLASSES_SOUNDBUFFERPLAYBACK802A8798_H

class SoundPlaybackInstanceView {
public:
    char pad_000[0x6C];
    unsigned char spatialized; // 0x6C: SoundManager_CreateSoundInstance enables for a position; SoundManager_UpdatePlayback transforms it
    unsigned char playbackStarted; // 0x6D: SoundManager_UpdatePlayback sets immediately after gtl_ISoundBuffer_Play
    unsigned char active; // 0x6E: SoundManager_CreateSoundInstance enables; Sound_StopByAlias clears
    char pad_06F[1];
    unsigned int playbackFlags; // 0x70: Sound_PlayByNameWithParameters tests bit 0 to choose Play mode 1 or 2
    char pad_074[0xC];
    void* soundBuffer; // 0x80: ADD40Create stores gtl_Audio_CreateSoundBuffer result; Pause/Stop/SetVolume receiver
    float volume; // 0x84: SoundManager_SetBufferVolume stores; ApplyBufferVolumes multiplies by baseVolume
    float baseVolume; // 0x88: SoundManager_ApplyBufferVolumes: factor * baseVolume * volume
    float basePitch; // 0x8C: SoundManager_SetNamedEffectPitch: basePitch * requested pitch
};

#define OFF(f, o) typedef char SoundBufferPlayback802A8798_off_##f[((int)&((SoundPlaybackInstanceView*)0)->f == (o)) ? 1 : -1]
OFF(spatialized, 0x6C);
OFF(playbackStarted, 0x6D);
OFF(active, 0x6E);
OFF(playbackFlags, 0x70);
OFF(soundBuffer, 0x80);
OFF(volume, 0x84);
OFF(baseVolume, 0x88);
OFF(basePitch, 0x8C);
#undef OFF
typedef char SoundBufferPlayback802A8798_size[(sizeof(SoundPlaybackInstanceView) == 0x90) ? 1 : -1];
#endif

// gtl::nw4r::detail::BasicPlayer: methods in gtl_voice_pool_basic_player_chorus.cpp.
// Layout-only view. ResetParameters (0x80409C18) supplies the defaults;
// indexed getters/setters at 0x80409C94..0x80409D04 establish the four arrays.
// Streaming voice updates establish the scalar controls and indexed output sends.
#ifndef CLASSES_GTLBASICPLAYER_H
#define CLASSES_GTLBASICPLAYER_H

class GtlBasicPlayer {
public:
    void* vtbl;
    int unk_4;
    float volumeScale;
    float pitchScale;
    float pan;
    float surroundPan;
    float panOffset;
    float surroundPanOffset;
    float lowPassFilterOffset;
    char pad_024[4];
    int outputLineFlags;
    float mainOutputVolume;
    float mainSend;
    float auxiliarySends[3];
    float remoteVolumes[4];
    float remoteSendOffsets[4];
    float additionalOutputMixControls[4];
    unsigned char biquadFilterSelector;
};

#define OFF(f, o) typedef char GtlBasicPlayer_off_##f[((int)&((GtlBasicPlayer*)0)->f == (o)) ? 1 : -1]
OFF(vtbl, 0x0);
OFF(unk_4, 0x4);
OFF(volumeScale, 0x8);
OFF(pitchScale, 0xC);
OFF(pan, 0x10);
OFF(surroundPan, 0x14);
OFF(panOffset, 0x18);
OFF(surroundPanOffset, 0x1C);
OFF(lowPassFilterOffset, 0x20);
OFF(outputLineFlags, 0x28);
OFF(mainOutputVolume, 0x2C);
OFF(mainSend, 0x30);
OFF(auxiliarySends, 0x34);
OFF(remoteVolumes, 0x40);
OFF(remoteSendOffsets, 0x50);
OFF(additionalOutputMixControls, 0x60);
OFF(biquadFilterSelector, 0x70);
#undef OFF

inline GtlBasicPlayer* AsGtlBasicPlayer(void* p) { return (GtlBasicPlayer*)p; }

#endif

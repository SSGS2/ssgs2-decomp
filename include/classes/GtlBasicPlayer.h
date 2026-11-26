// gtl::nw4r::detail::BasicPlayer: playback state for one basic player.
// Its methods are defined in gtl_voice_pool_basic_player_chorus.cpp.
// The voice and stream code in other files reads and writes the same fields.
#ifndef CLASSES_GTLBASICPLAYER_H
#define CLASSES_GTLBASICPLAYER_H

namespace gtl {
namespace nw4r {
namespace detail {

class BasicPlayer {
public:
    // Initialize the cached two-pi constant used by the player.
    static void InitTwoPi();
    // Indexed controls. Indices are not range-checked.
    // Auxiliary sends, index 0..2.
    void SetAuxiliarySend(int index, float value);
    float GetAuxiliarySend(int index);
    // Remote output controls, index 0..3.
    void SetRemoteVolume(int index, float value);
    float GetRemoteVolume(int index);
    void SetRemoteSendOffset(int index, float value);
    float GetRemoteSendOffset(int index);
    // Additional output-mix controls, index 0..3.
    void SetAdditionalOutputMixControl(int index, float value);
    float GetAdditionalOutputMixControl(int index);

    void* vtbl;                           // 0x00
    int unk_4;                            // 0x04
    float volumeScale;                    // 0x08
    float pitchScale;                     // 0x0C
    float pan;                            // 0x10
    float surroundPan;                    // 0x14
    float panOffset;                      // 0x18
    float surroundPanOffset;              // 0x1C
    float lowPassFilterOffset;            // 0x20
    char pad_024[4];                      // 0x24
    int outputLineFlags;                  // 0x28
    float mainOutputVolume;               // 0x2C
    float mainSend;                       // 0x30
    float auxiliarySends[3];              // 0x34
    float remoteVolumes[4];               // 0x40
    float remoteSendOffsets[4];           // 0x50
    float additionalOutputMixControls[4]; // 0x60
    unsigned char biquadFilterSelector;   // 0x70
};

} // namespace detail
} // namespace nw4r
} // namespace gtl

#define OFF(f, o) typedef char GtlBasicPlayer_off_##f[((int)&((gtl::nw4r::detail::BasicPlayer*)0)->f == (o)) ? 1 : -1]
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

inline gtl::nw4r::detail::BasicPlayer* AsGtlBasicPlayer(void* p) {
    return (gtl::nw4r::detail::BasicPlayer*)p;
}

#endif

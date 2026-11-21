// SoundResourceParameters8043: layout view owned by gtl_sound_buffer_stream_resources.cpp.
// Evidence: gtl_CGSPSoundProperty_ApplyToBuffer: +8 feeds volume/pan/pitch setters; +0x10 passed to gtl_ISoundBuffer_SetDistanceCurve. No virtual declarations or runtime objects.
#ifndef CLASSES_SOUNDRESOURCEPARAMETERS8043_H
#define CLASSES_SOUNDRESOURCEPARAMETERS8043_H

class SoundResourceParameters8043 {
public:
    char pad_0[0x8];
    volatile void* baseParameters; // 0x8; gtl_CGSPSoundProperty_ApplyToBuffer: +8 feeds volume/pan/pitch setters
    void* channelParameters; // 0xC; gtl_CGSPSoundProperty_ApplyToBuffer: +8 feeds volume/pan/pitch setters
    void* distanceCurve; // 0x10; gtl_CGSPSoundProperty_ApplyToBuffer: +8 feeds volume/pan/pitch setters
    void* envelopeParameters; // 0x14; gtl_CGSPSoundProperty_ApplyToBuffer: +8 feeds volume/pan/pitch setters
};

#define OFF(f, o) typedef char SoundResourceParameters8043_off_##f[((int)&((SoundResourceParameters8043*)0)->f == (o)) ? 1 : -1]
OFF(baseParameters, 0x8);
OFF(channelParameters, 0xC);
OFF(distanceCurve, 0x10);
OFF(envelopeParameters, 0x14);
#undef OFF

#endif

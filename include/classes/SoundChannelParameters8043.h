// SoundChannelParameters8043: layout view owned by gtl_sound_buffer_stream_resources.cpp.
// Evidence: gtl_CGSPSoundProperty_ApplyToBuffer: pairs passed to channel setters; exact meaning unknown. No virtual declarations or runtime objects.
#ifndef CLASSES_SOUNDCHANNELPARAMETERS8043_H
#define CLASSES_SOUNDCHANNELPARAMETERS8043_H

class SoundChannelParameters8043 {
public:
    char pad_0[0x30];
    float coneInnerAngle; // 0x30; gtl_CGSPSoundProperty_ApplyToBuffer: pairs passed to channel setters
    float coneOuterAngle; // 0x34; gtl_CGSPSoundProperty_ApplyToBuffer: pairs passed to channel setters
    float coneInnerGain; // 0x38; gtl_CGSPSoundProperty_ApplyToBuffer: pairs passed to channel setters
    float coneOuterGain; // 0x3C; gtl_CGSPSoundProperty_ApplyToBuffer: pairs passed to channel setters
    char pad_40[0x4];
    float distanceParameterA; // 0x44; gtl_CGSPSoundProperty_ApplyToBuffer: pairs passed to channel setters
    float distanceParameterB; // 0x48; gtl_CGSPSoundProperty_ApplyToBuffer: pairs passed to channel setters
    float minDistance; // 0x4C; gtl_CGSPSoundProperty_ApplyToBuffer: pairs passed to channel setters
    float maxDistance; // 0x50; gtl_CGSPSoundProperty_ApplyToBuffer: pairs passed to channel setters
};

#define OFF(f, o) typedef char SoundChannelParameters8043_off_##f[((int)&((SoundChannelParameters8043*)0)->f == (o)) ? 1 : -1]
OFF(coneInnerAngle, 0x30);
OFF(coneOuterAngle, 0x34);
OFF(coneInnerGain, 0x38);
OFF(coneOuterGain, 0x3C);
OFF(distanceParameterA, 0x44);
OFF(distanceParameterB, 0x48);
OFF(minDistance, 0x4C);
OFF(maxDistance, 0x50);
#undef OFF

#endif

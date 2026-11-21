// CSoundStreamSource view, owned by gtl_sound_stream_source_part1.cpp. gtl_CSoundStreamSource_Ctor (0x80404214) copies init fields/config and allocates callback. Unknown init/config meanings retain offset names.
#ifndef CLASSES_SOUNDSTREAMSOURCE_H
#define CLASSES_SOUNDSTREAMSOURCE_H

struct SoundStreamSource {
    unsigned int enabled;
    void* vtable;
    void* name;
    void* sourceData;
    void* sampleRate;
    unsigned short format;
    unsigned short channelCount;
    void* sampleCount;
    void* streamData;
    unsigned int config[7];
    unsigned short adpcmChannelParameters[48];
    void* callback;
};

#define OFF(f, o) typedef char SoundStreamSource_off_##f[((int)&((SoundStreamSource*)0)->f == (o)) ? 1 : -1]
OFF(enabled, 0x0);
OFF(vtable, 0x4);
OFF(name, 0x8);
OFF(sourceData, 0xc);
OFF(sampleRate, 0x10);
OFF(format, 0x14);
OFF(channelCount, 0x16);
OFF(sampleCount, 0x18);
OFF(streamData, 0x1c);
OFF(config, 0x20);
OFF(adpcmChannelParameters, 0x3c);
OFF(callback, 0x9c);
#undef OFF

#endif

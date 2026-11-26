// SoundPropertyWords8043: layout view owned by gtl_sound_buffer_stream_resources.cpp.
// Evidence: SoundProperties_Clear copies eleven words. No virtual declarations or runtime objects.
#ifndef CLASSES_SOUNDPROPERTYWORDS8043_H
#define CLASSES_SOUNDPROPERTYWORDS8043_H

class SoundPropertyWords8043 {
public:
    unsigned int volume;    // 0x0; SoundProperties_Clear copies eleven words
    unsigned int pitch;     // 0x4; SoundProperties_Clear copies eleven words
    unsigned int pan;       // 0x8; SoundProperties_Clear copies eleven words
    unsigned int unk_C;     // 0xC; SoundProperties_Clear copies eleven words
    unsigned int unk_10;    // 0x10; SoundProperties_Clear copies eleven words
    unsigned int unk_14;    // 0x14; SoundProperties_Clear copies eleven words
    unsigned int unk_18;    // 0x18; SoundProperties_Clear copies eleven words
    unsigned int unk_1C;    // 0x1C; SoundProperties_Clear copies eleven words
    unsigned int loopStart; // 0x20; SoundProperties_Clear copies eleven words
    unsigned int loopEnd;   // 0x24; SoundProperties_Clear copies eleven words
    unsigned int unk_28;    // 0x28; SoundProperties_Clear copies eleven words
};

#define OFF(f, o) typedef char SoundPropertyWords8043_off_##f[((int)&((SoundPropertyWords8043*)0)->f == (o)) ? 1 : -1]
OFF(volume, 0x0);
OFF(pitch, 0x4);
OFF(pan, 0x8);
OFF(unk_C, 0xC);
OFF(unk_10, 0x10);
OFF(unk_14, 0x14);
OFF(unk_18, 0x18);
OFF(unk_1C, 0x1C);
OFF(loopStart, 0x20);
OFF(loopEnd, 0x24);
OFF(unk_28, 0x28);
#undef OFF

#endif

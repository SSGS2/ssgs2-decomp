// SoundResource8043: layout view owned by gtl_sound_buffer_stream_resources.cpp.
// Evidence: gtl_CGSPSoundBufferResource_dtor; resource vfunc_4. No virtual declarations or runtime objects.
#ifndef CLASSES_SOUNDRESOURCE8043_H
#define CLASSES_SOUNDRESOURCE8043_H

class SoundResource8043 {
public:
    char pad_0[0x8];
    void* data; // 0x8; gtl_CGSPSoundBufferResource_dtor
};

#define OFF(f, o) typedef char SoundResource8043_off_##f[((int)&((SoundResource8043*)0)->f == (o)) ? 1 : -1]
OFF(data, 0x8);
#undef OFF

#endif

// Original addresses gtl_ISoundStream_Pause / gtl_ISoundStream_Resume: player pointer at +8.
#ifndef CLASSES_SOUNDSTREAMHANDLE804022BC_H
#define CLASSES_SOUNDSTREAMHANDLE804022BC_H

class SoundStreamHandle804022BC {
public:
    int refs; // 0x0: gtl_ISoundStream_Ctor initializes reference count to 1
    void* vtbl; // 0x4: gtl_ISoundStream_Ctor installs ISoundStream class table
    void* player; // 0x8: gtl_ISoundStream_Pause, gtl_ISoundStream_Resume load the stream player
};

#define SOUNDSTREAMHANDLE804022BC_OFF(f, o) typedef char SoundStreamHandle804022BC_off_##f[((int)&((SoundStreamHandle804022BC*)0)->f == (o)) ? 1 : -1]
SOUNDSTREAMHANDLE804022BC_OFF(refs, 0x0);
SOUNDSTREAMHANDLE804022BC_OFF(vtbl, 0x4);
SOUNDSTREAMHANDLE804022BC_OFF(player, 0x8);
#undef SOUNDSTREAMHANDLE804022BC_OFF

#endif

// SoundStreamSourceState: layout view; gtl_sound_stream_source_part4.cpp: gtl_CSoundStreamSource_GetStreamCallback returns +0x9C through out; precise pointee unknown
// No inheritance, virtual declarations, constructors, or emitted runtime objects.
#ifndef CLASSES_SOUNDSTREAMSOURCESTATE_H
#define CLASSES_SOUNDSTREAMSOURCESTATE_H

struct SoundStreamSourceState {
    char pad_000[0x9C];
    void* sourcePointer; // 0x9C
};

#define OFF(f, o) typedef char SoundStreamSourceState_off_##f[((int)&((SoundStreamSourceState*)0)->f == (o)) ? 1 : -1]
OFF(sourcePointer, 0x9C);
#undef OFF
#define AsSoundStreamSourceState(p) ((SoundStreamSourceState*)(p))

#endif

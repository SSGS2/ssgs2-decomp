// Layout views for gtl_ISoundBuffer_Ctor / gtl_ISoundBuffer_dtor in gtl_sound_buffer_and_effects.cpp.
// No inheritance, virtual declarations or generated code.
#ifndef CLASSES_GTLSOUNDBUFFER_H
#define CLASSES_GTLSOUNDBUFFER_H

struct SoundBufferData {
    unsigned int flags; // 0x00: constructor copies source flags and expands bit 0x10
    unsigned char playbackStorage[0x98];
    void* activeVoice; // 0x9c: destructor passes this to GtlVoice_Stop under interrupt lock
    void* refObject; // 0xa0: destructor decrements reference count, destroys at zero
    unsigned char sourceInfo[0x18];
    unsigned int entries[22]; // 0xbc: constructor copies s_soundBufferParameterDefaults
    char envelope[0x3C]; // 0x114: gtl_ISoundBuffer_Ctor calls gtl_AudioEnvelope_Initialize
    char lfo[0x28]; // 0x150: same constructor calls gtl_AudioLfo_Initialize
    char bufferScaleState[8]; // 0x178: same constructor calls gtl_AudioScale_Initialize
    float volumePitchAndPan[5]; // 0x180: initialized to 1.0f by that constructor
    int loopParameters[3]; // 0x194: initialized to zero
    float outputSendLevels[6]; // 0x1a0: initialized to 0.0f
};

struct SoundBufferOwner {
    int refs; // 0x00: gtl_ISoundBuffer_Ctor initializes to 1
    void* vtable; // 0x04: __vt__Q23gtl12ISoundBuffer
    SoundBufferData* data; // 0x08: constructor allocates 0x1b8; destructor frees
};

#define OFF(T, f, o) typedef char T##_off_##f[((int)&((T*)0)->f == (o)) ? 1 : -1]
OFF(SoundBufferData, flags, 0x00);
OFF(SoundBufferData, activeVoice, 0x9c);
OFF(SoundBufferData, refObject, 0xa0);
OFF(SoundBufferData, entries, 0xbc);
OFF(SoundBufferData, envelope, 0x114);
OFF(SoundBufferData, lfo, 0x150);
OFF(SoundBufferData, bufferScaleState, 0x178);
OFF(SoundBufferData, volumePitchAndPan, 0x180);
OFF(SoundBufferData, loopParameters, 0x194);
OFF(SoundBufferData, outputSendLevels, 0x1a0);
OFF(SoundBufferOwner, refs, 0x00);
OFF(SoundBufferOwner, vtable, 0x04);
OFF(SoundBufferOwner, data, 0x08);
#undef OFF

#endif

// gtl::ISoundBuffer: a sound buffer object. The object holds a reference count, a vtable word and a
// pointer to its state block (SoundBufferData). There are no virtual functions here: the vtable word is
// written by the constructor from original data.
#ifndef CLASSES_GTLSOUNDBUFFER_H
#define CLASSES_GTLSOUNDBUFFER_H

struct SoundBufferData {
    unsigned int flags; // 0x00: constructor copies source flags and expands bit 0x10
    unsigned char playbackStorage[0x98];
    void* activeVoice; // 0x9c: destructor passes this to GtlVoice_Stop under interrupt lock
    void* refObject;   // 0xa0: destructor decrements reference count, destroys at zero
    unsigned char sourceInfo[0x18];
    unsigned int entries[22];   // 0xbc: constructor copies s_soundBufferParameterDefaults
    char envelope[0x3C];        // 0x114: constructor calls gtl_AudioEnvelope_Initialize
    char lfo[0x28];             // 0x150: same constructor calls gtl_AudioLfo_Initialize
    char bufferScaleState[8];   // 0x178: same constructor calls gtl_AudioScale_Initialize
    float volumePitchAndPan[5]; // 0x180: initialized to 1.0f by that constructor
    int loopParameters[3];      // 0x194: initialized to zero
    float outputSendLevels[6];  // 0x1a0: initialized to 0.0f
};

struct SoundSourceReference;
struct SoundParameterEntry;

namespace gtl {

class ISoundBuffer {
public:
    // Construction and destruction.
    ISoundBuffer* Init(unsigned int* source);
    void* Destroy(int shouldDelete);

    // Playback control.
    int Play(int playbackFlags);
    int Stop(int mode);
    int Pause();
    int Resume();
    int GetStatusFlags(int* out);
    static void VoiceCallback(void* arg, int mode, ISoundBuffer* self);
    void UpdateModulation(int elapsedTicks);
    void UpdateParameters();

    // Parameter setters.
    int SetVolume(float volume);
    int SetPitch(float pitch);
    int SetPan(float pan);
    int SetEnvelope(int enabled);
    int SetSource(SoundSourceReference* next);
    int SetLoopRange(int loopStart, int loopEnd);
    int SetPosition(const void* source, int unused);
    int DefaultSuccess();
    int ApplyParameterEntries(SoundParameterEntry* entries, unsigned int count, void* unused);

    // Spatial controls.
    int SetConeAngles(int unused, float innerRadians, float outerRadians);
    int SetConeGains(int unused, float innerGain, float outerGain);
    int SetMinDistance(int unused, float value);
    int SetMaxDistance(int unused, float value);
    int SetDistanceCurve(int curveAddressBits, int unused);
    int SetDistanceParameterA(int unused, float value);
    int SetDistanceParameterB(int unused, float value);
    int SetSpatialMode(int value, int unused);

    // Accessors.
    void* GetPlaybackHeader();

    int refs;              // 0x00: Init sets it to 1
    void* vtable;          // 0x04: __vt__Q23gtl12ISoundBuffer
    SoundBufferData* data; // 0x08: Init allocates 0x1b8 bytes; Destroy frees them
};

} // namespace gtl

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
typedef char ISoundBuffer_off_refs[((int)&((gtl::ISoundBuffer*)0)->refs == 0x00) ? 1 : -1];
typedef char ISoundBuffer_off_vtable[((int)&((gtl::ISoundBuffer*)0)->vtable == 0x04) ? 1 : -1];
typedef char ISoundBuffer_off_data[((int)&((gtl::ISoundBuffer*)0)->data == 0x08) ? 1 : -1];
#undef OFF

#endif

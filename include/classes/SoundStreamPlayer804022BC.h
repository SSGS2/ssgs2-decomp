// Offsets preserved from gtl_ISoundStream_Play and helpers in gtl_sound_stream_player.cpp; no virtual declarations.
#ifndef CLASSES_SOUNDSTREAMPLAYER804022BC_H
#define CLASSES_SOUNDSTREAMPLAYER804022BC_H

class CSoundStreamPlayer {
public:
    unsigned int
        flags; // 0x0: gtl_ISoundStream_Play checks listener and mode bits; gtl_ISoundStream_Ctor copies source flags
    int startPending; // 0x4: gtl_ISoundStream_Play, original 0x80402984-0x804029FC: cleared after start returns 1
    char pad_008[0x541];
    unsigned char loopEnabled; // 0x549: gtl_ISoundStream_Play, original 0x804024B0-0x804024E8: loop setup
    char pad_54A[0x9E];
    void* stream; // 0x5E8: gtl_ISoundStream_Pause / gtl_ISoundStream_Resume: virtual playback resource
    void* source; // 0x5EC: gtl_ISoundStream_Play: source virtual calls attach/query output
    char pad_5F0[0x48];
    float innerConeAngle;     // 0x638: gtl::ISoundStream::SetConeAngles
    float outerConeAngle;     // 0x63C: gtl::ISoundStream::SetConeAngles
    float innerConeGain;      // 0x640: gtl::ISoundStream::SetConeGains
    float outerConeGain;      // 0x644: gtl::ISoundStream::SetConeGains
    int distanceCurve;        // 0x648: gtl::ISoundStream::SetDistanceCurve
    float distanceParameterA; // 0x64C: gtl::ISoundStream::SetDistanceParameterA
    float distanceParameterB; // 0x650: gtl::ISoundStream::SetDistanceParameterB
    float minDistance;        // 0x654: gtl::ISoundStream::SetMinDistance
    float maxDistance;        // 0x658: gtl::ISoundStream::SetMaxDistance
    int spatialMode;          // 0x65C: gtl_ISoundStream_Play compares with 2 before listener processing
    int fadeState;            // 0x660: gtl_ISoundStream_Stop sets 7 to stop; gtl_ISoundStream_Play, original 0x80402A34
    float fadeFactor;         // 0x664: Fade804022BC, original 0x80402A40-0x80402A74
    float fadeStartFactor;    // 0x668: gtl_ISoundStream_Stop copies current fade factor when stopping
    char pad_66C[0xC];
    int fadeRunning; // 0x678: Fade804022BC, original 0x80402B60-0x80402B88
    char pad_67C[0x24];
    float pitchScale; // 0x6A0: gtl_ISoundStream_Play multiplies pitch and listenerFactor by this scale
    char pad_6A4[0x20];
    float outputScale; // 0x6C4: gtl_ISoundStream_Play forwards to stream +0x20
    char pad_6C8[0x4];
    float volume; // 0x6CC: gtl_ISoundStream_Play multiplies listener volume and fade; gtl_ISoundStream_SetVolume setter
    float listenerVolume; // 0x6D0: gtl_ISoundStream_Play stores listener result[2]
    float pitch;          // 0x6D4: gtl_ISoundStream_SetPitch setter, gtl_ISoundStream_Play forwards to stream +0xC
    float listenerDopplerRatio; // 0x6D8: gtl_ISoundStream_Play stores listener result[3]
    float pan;                  // 0x6DC: gtl_ISoundStream_SetPan setter, gtl_ISoundStream_Play listener processing
    unsigned int
        loopRequested; // 0x6E0: gtl_ISoundStream_Play sets flags & 1; gtl_ISoundStream_GetStatusFlags reports 0x80000
    unsigned int loopStart; // 0x6E4: gtl_ISoundStream_Play, original 0x804024B0-0x804024E8
    unsigned int loopEnd;   // 0x6E8: gtl_ISoundStream_Play, original 0x804024B0-0x804024E8
    float mainSend;         // 0x6EC: OutputBits804022BC, original 0x8040264C-0x804026B0: bit 0
    float
        defaultSendParameter; // 0x6F0: gtl_ISoundStream_Play forwards to GtlBasicPlayer_SetAuxiliarySend / GtlBasicPlayer_SetAdditionalOutputMixControl
    float send1;              // 0x6F4: OutputBits804022BC: bit 1
    float send2;              // 0x6F8: OutputBits804022BC: bit 2
    float send3;              // 0x6FC: OutputBits804022BC: bit 3
    float send4;              // 0x700: OutputBits804022BC: bit 4
};

#define SOUNDSTREAMPLAYER804022BC_OFF(f, o) \
    typedef char SoundStreamPlayer804022BC_off_##f[((int)&((CSoundStreamPlayer*)0)->f == (o)) ? 1 : -1]
SOUNDSTREAMPLAYER804022BC_OFF(flags, 0x0);
SOUNDSTREAMPLAYER804022BC_OFF(startPending, 0x4);
SOUNDSTREAMPLAYER804022BC_OFF(loopEnabled, 0x549);
SOUNDSTREAMPLAYER804022BC_OFF(stream, 0x5E8);
SOUNDSTREAMPLAYER804022BC_OFF(source, 0x5EC);
SOUNDSTREAMPLAYER804022BC_OFF(innerConeAngle, 0x638);
SOUNDSTREAMPLAYER804022BC_OFF(maxDistance, 0x658);
SOUNDSTREAMPLAYER804022BC_OFF(spatialMode, 0x65C);
SOUNDSTREAMPLAYER804022BC_OFF(fadeState, 0x660);
SOUNDSTREAMPLAYER804022BC_OFF(fadeFactor, 0x664);
SOUNDSTREAMPLAYER804022BC_OFF(fadeStartFactor, 0x668);
SOUNDSTREAMPLAYER804022BC_OFF(fadeRunning, 0x678);
SOUNDSTREAMPLAYER804022BC_OFF(pitchScale, 0x6A0);
SOUNDSTREAMPLAYER804022BC_OFF(outputScale, 0x6C4);
SOUNDSTREAMPLAYER804022BC_OFF(volume, 0x6CC);
SOUNDSTREAMPLAYER804022BC_OFF(listenerVolume, 0x6D0);
SOUNDSTREAMPLAYER804022BC_OFF(pitch, 0x6D4);
SOUNDSTREAMPLAYER804022BC_OFF(listenerDopplerRatio, 0x6D8);
SOUNDSTREAMPLAYER804022BC_OFF(pan, 0x6DC);
SOUNDSTREAMPLAYER804022BC_OFF(loopRequested, 0x6E0);
SOUNDSTREAMPLAYER804022BC_OFF(loopStart, 0x6E4);
SOUNDSTREAMPLAYER804022BC_OFF(loopEnd, 0x6E8);
SOUNDSTREAMPLAYER804022BC_OFF(mainSend, 0x6EC);
SOUNDSTREAMPLAYER804022BC_OFF(defaultSendParameter, 0x6F0);
SOUNDSTREAMPLAYER804022BC_OFF(send1, 0x6F4);
SOUNDSTREAMPLAYER804022BC_OFF(send2, 0x6F8);
SOUNDSTREAMPLAYER804022BC_OFF(send3, 0x6FC);
SOUNDSTREAMPLAYER804022BC_OFF(send4, 0x700);
#undef SOUNDSTREAMPLAYER804022BC_OFF

#endif

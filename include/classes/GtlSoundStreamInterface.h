#ifndef CLASSES_GTLSOUNDSTREAMINTERFACE_H
#define CLASSES_GTLSOUNDSTREAMINTERFACE_H

#include "game/types.h"
#include "classes/SoundStreamPlayer804022BC.h"

struct SourceBase804022BC;

namespace gtl {

// A streamed sound. The state it points to is the player block, which owns the native
// playback object and every parameter of the stream.
class ISoundStream {
public:
    // Playback control. Play starts or restarts the stream; Stop fades or stops it.
    int Play(unsigned int flags);
    int Stop(int mode);
    int Pause();
    int Resume();
    int GetStatusFlags(unsigned int* result);
    int SetSource(const SourceBase804022BC* source);
    void* GetPlaybackHeader();

    // Setters for the stored parameters. Volume and pitch are ignored unless the source allows them.
    int SetVolume(float value);
    int SetPitch(float value);
    int SetPan(float value);
    void* SetEnvelope(void* settings);
    int SetLoopRange(int loopStart, int loopEnd);
    int SetConeAngles(int unused, float inner, float outer);
    int SetConeGains(int unused, float inner, float outer);
    int SetMinDistance(int unused, float value);
    int SetMaxDistance(int unused, float value);
    int SetDistanceCurve(int curve, int unused);
    int SetDistanceParameterA(int unused, float value);
    int SetDistanceParameterB(int unused, float value);

    // Per-frame updates: parameters are applied with interrupts disabled.
    void UpdateParameters();
    void UpdateModulation(void* elapsedTicks);

    // Construction and destruction on storage the caller owns.
    ISoundStream* Init(void* source);
    void* Destroy(int flags);

    int refs;                   // 0x0
    void* vtbl;                 // 0x4
    CSoundStreamPlayer* player; // 0x8
};

} // namespace gtl

#endif // CLASSES_GTLSOUNDSTREAMINTERFACE_H

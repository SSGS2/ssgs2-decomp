// gtl::CGSPSoundProperty: one decoded sound property. It holds a reference count, a vtable and four owned
// parameter blocks (base, spatial, distance curve, envelope). The block layouts are defined in
// src/engine/gtl/gtl_sound_buffer_stream_resources.cpp. No virtual functions are declared here; the vtable
// comes from the original data.
#ifndef CLASSES_GTL_CGSPSOUNDPROPERTY_H
#define CLASSES_GTL_CGSPSOUNDPROPERTY_H

struct BorrowedSoundPropertyChunks;

// Decoded runtime base properties (40 bytes), copied from the BSIC chunk.
struct RuntimeSoundBaseParameters {
    float volume;
    float pitch;
    float pan;
    char reserved0C[4];
    // Four encoded option bytes are widened to words; their value domains are unresolved.
    int options[4];
    int loopStart;
    int loopEnd;
};

// Decoded runtime spatial parameters; leading 0x30 bytes are outside this copy path.
struct RuntimeSoundSpatialParameters {
    char reserved00[0x30];
    float coneInnerAngle;
    float coneOuterAngle;
    float coneInnerGain;
    float coneOuterGain;
    unsigned int distanceCurve;
    float distanceParameterA;
    float distanceParameterB;
    float minDistance;
    float maxDistance;
    char reserved54[4]; // Spatial block allocation includes one unused trailing word.
};

// Runtime distance-curve header owning pointCount distance/gain pairs.
struct RuntimeSoundDistanceCurve {
    unsigned int curveKind;
    unsigned int pointCount;
    void* points;
};

// One curve key: scene-distance input and multiplicative gain output.
struct SoundDistanceGainKey {
    float distance;
    float gain;
};

// Layout consumed by gtl_AudioEnvelope_Initialize; time values remain in native ticks.
struct RuntimeSoundEnvelope {
    int configTag;
    int mode;
    int delayTicks;
    int attackTicks;
    int holdTicks;
    int decayTicks;
    int releaseTicks;
    int sustainUnits;
    int reservedConfig0;
    int reservedConfig1;
};

namespace gtl {

class CGSPSoundProperty {
public:
    // Decode the borrowed chunks into this property, which starts with one reference. Returns this.
    void* Init(BorrowedSoundPropertyChunks* src) __attribute__((noinline));
    // Free the owned parameter blocks. Frees this object when deleting is positive. Returns this.
    void* Destroy(int deleting);

    // Apply base, spatial and envelope parameters to the sound buffer dst. Returns 0 when dst is null, else 1.
    int ApplyToBuffer(void* dst);
    // Apply base, spatial and envelope parameters to the sound stream dst. Returns 0 when dst is null, else 1.
    int ApplyToStream(void* dst);

    int referenceCount;                               // 0x00
    void* vtable;                                     // 0x04
    RuntimeSoundBaseParameters* baseParameters;       // 0x08
    RuntimeSoundSpatialParameters* channelParameters; // 0x0C
    RuntimeSoundDistanceCurve* distanceCurve;         // 0x10
    RuntimeSoundEnvelope* envelope;                   // 0x14
};

} // namespace gtl

#define GTL_CGSPSOUNDPROPERTY_OFF(f, o) \
    typedef char gtl_CGSPSoundProperty_off_##f[((int)&((gtl::CGSPSoundProperty*)0)->f == (o)) ? 1 : -1]
GTL_CGSPSOUNDPROPERTY_OFF(vtable, 0x4);
GTL_CGSPSOUNDPROPERTY_OFF(baseParameters, 0x8);
GTL_CGSPSOUNDPROPERTY_OFF(envelope, 0x14);

#endif // CLASSES_GTL_CGSPSOUNDPROPERTY_H

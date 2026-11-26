// Spray-specific parser fields agree with CFxSpray_Ctor / CFxSpray_Reset.
#include "classes/CFxSpray.h"
#include "classes/CFxBox.h"
#include "classes/CSharedDoc.h"
// Borrow the spray view of the particle allocation without pointer adjustment.
inline CFxSpray* AsCFxSpray(void* object) {
    return (CFxSpray*)object;
}
#include "classes/CFxParticle.h"
// Borrow the common particle fields used for animation-key list ownership.
inline CFxParticle* AsCFxParticle(void* object) {
    return (CFxParticle*)object;
}

#include "sdk_protos.h"
#include "classes/CHashPath.h"
#pragma cpp_exceptions on

extern "C" {

// Tokenizer ABI: pos/end delimit input; buffer receives each parsed token.
struct ConfigText {
    int vtable;
    int pos;
    int end;
    int inputState;
    char* buffer;
    int bufferCapacity;
    ConfigText();
    ~ConfigText();
};
void cTokenV_Init(ConfigText*);
void cTokenV_SetInputBuffer(ConfigText*, const char*, int);
int cToken_ReadUntilDelimiter(ConfigText*, char*, const char*, int);
int String_TrimCharacterSet(char*, const char*);
int FxSprayScriptBuffer_EnsureTerminatorCapacity(ConfigText*);
void cToken_dtor(ConfigText*, short);
int CString_Compare(const char*, const char*);
int String_ParseDecimalInt(const char*);
double atof(const char*);
struct ConfigVector {
    float x, y, z;
    // Construct the three script components without scaling or conversion.
    ConfigVector(float a, float b, float c) : x(a), y(b), z(c) {}
};
void __as__7WVectorFRC7WVector(void*, const ConfigVector&);
void* appMallocDummy__FUl(unsigned int);
void appFreeUni__FPv(void*);
void ResourceManager_ReleaseId(void*, void*);
void* ResourceManager_LoadBitmap(void*, const char*, int, int, int);
extern CSharedDoc* g_sharedDoc;
// Script keywords and delimiters share one immutable string pool.
struct FxSprayScriptStringPool {
    char trimWhitespace[0x6];
    char scanLengthFormat[0x3];
    char assignmentDelimiter[0x2];
    char valueDelimiter[0x2];
    char trimValueCharacters[0x6];
    char source[0x7];
    char wordSeparators[0x3];
    char petSource[0x5];
    char petLighting[0x9];
    char billboard[0xa];
    char tail[0x5];
    char monolith[0x9];
    char cameraSide[0xb];
    char sphericalEmission[0x9];
    char emissionPosition[0x7];
    char regenerateAtSamePosition[0xf];
    char cameraOffset[0xa];
    char addGeneration[0xf];
    char lifetime[0x9];
    char volumeOpacity[0x10];
    char volumeCore[0x9];
    char initialVelocity[0xe];
    char addVelocity[0xd];
    char windScale[0x9];
    char externalWind[0x9];
    char externalOrientation[0x10];
    char integerFormat[0x3];
    char initialAngle[0xb];
    char initialRotation[0xe];
    char addRotation[0xd];
    char friction[0x9];
    char groundFriction[0xf];
    char gravity[0x8];
    char addGravityPoint[0x11];
    char groundCollision[0x7];
    char blend[0x6];
    char normalBlend[0x7];
    char addBlend[0x4];
    char multiplyBlend[0x9];
    char inverseMultiplyBlend[0xc];
    char lighting[0x9];
    char addFade[0x9];
    char initialSize[0xa];
    char floatFormat[0x3];
    char addSize[0x9];
    char initialFrame[0xb];
    char addFrame[0xa];
    char addQuake[0xa];
    char addFlash[0xa];
    char distanceLimit[0xb];
    char addBubble[0xb];
    char sound[0x6];
    char waveSuffix[0x5];
};
extern const FxSprayScriptStringPool fxSprayScriptStrings;
typedef char FxSprayStrings_source[((int)&((FxSprayScriptStringPool*)0)->source == 0x13) ? 1 : -1];
typedef char FxSprayStrings_sound[((int)&((FxSprayScriptStringPool*)0)->sound == 0x1D3) ? 1 : -1];
extern float kFloat_0_FxSpray;
extern float kFloat_1_FxSpray;
extern float kFloat_neg_1_805DE648;

struct ColorKey {
    float startLifetimePercent;
    unsigned char startAlpha, startRed, startGreen, startBlue;
    float endLifetimePercent;
    unsigned char endAlpha, endRed, endGreen, endBlue;
};

// Payload contracts checked against fx_spray_particles.cpp: EmitScheduledParticles,
// InitializeParticle and Update. Milliseconds are emitter time; lifetimePercent
// is particle age / initial lifetime * 100. These are different key domains.
// D = decimal int; F = atof float; S = ParseScaledFloat(owner, token).
// S suffix 'd' converts degrees to radians; 'f' multiplies by owner +4
// (nominal step milliseconds). Unsuffixed S is unchanged; int S truncates.
// Keep ConfigPayload type identities: exception relocations encode their names.
// addGeneration: S startMs, S endMs, D count, S spreadRadians.
struct ConfigPayload0 {
    int startMilliseconds, endMilliseconds, totalParticles;
    float spreadRadians; // Passed to InitializeParticle's random spread angle.
    int emittedParticles;
    float lastEmissionMilliseconds;
};
// addVelocity: S startMs, F speedPercent, S endMs, F speedPercent.
struct ConfigPayload1 {
    int startMilliseconds;
    float startSpeedPercent;
    int endMilliseconds;
    float endSpeedPercent;
};
// addRotation: F startAgePercent, S angularSpeedPercent, F endAgePercent, S percent.
// S accepts suffixes even though the evaluator uses these values as percentages.
// Equal endpoints apply base angular velocity directly (also true for size below).
struct ConfigPayload2 {
    float startLifetimePercent, startAngularSpeedPercent;
    float endLifetimePercent, endAngularSpeedPercent;
};
// addGravityPoint: F offset xyz, F innerStrength, F radius, F outerStrength.
// Offset is added to emitter world origin without a rotation. Within radius,
// acceleration magnitude = innerStrength / max(distanceSquared, 0.5); outside
// it is outerStrength. Both multiply the scaled update step. Game distance units.
struct ConfigPayload3 {
    float offsetX, offsetY, offsetZ;
    float innerStrength, radius, outerStrength;
    // Leave fields for the six directive arguments.
    ConfigPayload3() {}
    // Preserve original integer copies for position and float copies for force.
    ConfigPayload3(const ConfigPayload3& rhs) {
        ((unsigned int*)this)[0] = ((const unsigned int*)&rhs)[0];
        ((unsigned int*)this)[1] = ((const unsigned int*)&rhs)[1];
        ((unsigned int*)this)[2] = ((const unsigned int*)&rhs)[2];
        innerStrength = rhs.innerStrength;
        radius = rhs.radius;
        outerStrength = rhs.outerStrength;
    }
};
// addSize: %f startAgePercent, %f startSizePercent, %f endAgePercent, %f endSizePercent.
// Int-sized slots hold FLOAT BITS from sscanf; copying them never converts values.
// Unequal endpoints multiply initialSize by interpolated percent / 100; equal
// endpoints restore initialSize regardless of the requested endpoint percentage.
struct ConfigPayload4 {
    int startLifetimePercentBits, startSizePercentBits;
    int endLifetimePercentBits, endSizePercentBits;
};
// addFrame: %f startAgePercent, %f endAgePercent, D firstFrame, D lastFrame, S intervalMs.
// firstFrame==-1 samples rand()%lastFrame (exclusive); otherwise wraps sequential
// frames after lastFrame (inclusive). Lifetime window and interval must not be swapped.
struct ConfigPayload5 {
    int startLifetimePercentBits, endLifetimePercentBits;
    int firstFrame, lastFrame, intervalMilliseconds;
};
// addQuake: F center xyz, S startMs, F startAmplitude xyz, S endMs,
// F endAmplitude xyz, S sampleIntervalMs. Centers blend using per-axis
// amplitude weights; amplitudes scale random offsets (roughly -0.5..0.5).
struct ConfigPayload6 {
    float centerX, centerY, centerZ;
    int startMilliseconds;
    float startAmplitudeX, startAmplitudeY, startAmplitudeZ;
    int endMilliseconds;
    float endAmplitudeX, endAmplitudeY, endAmplitudeZ;
    int sampleIntervalMilliseconds;
    float lastSampleMilliseconds;
    // Leave fields for the twelve directive arguments and runtime zero.
    ConfigPayload6() {}
    // Copy the key with the original word-copy and final float-store order.
    ConfigPayload6(const ConfigPayload6& rhs) {
        ((unsigned int*)this)[0] = ((const unsigned int*)&rhs)[0];
        ((unsigned int*)this)[1] = ((const unsigned int*)&rhs)[1];
        ((unsigned int*)this)[2] = ((const unsigned int*)&rhs)[2];
        ((unsigned int*)this)[3] = ((const unsigned int*)&rhs)[3];
        ((unsigned int*)this)[4] = ((const unsigned int*)&rhs)[4];
        ((unsigned int*)this)[5] = ((const unsigned int*)&rhs)[5];
        ((unsigned int*)this)[6] = ((const unsigned int*)&rhs)[6];
        ((unsigned int*)this)[7] = ((const unsigned int*)&rhs)[7];
        ((unsigned int*)this)[8] = ((const unsigned int*)&rhs)[8];
        ((unsigned int*)this)[9] = ((const unsigned int*)&rhs)[9];
        ((unsigned int*)this)[10] = ((const unsigned int*)&rhs)[10];
        ((unsigned int*)this)[11] = ((const unsigned int*)&rhs)[11];
        lastSampleMilliseconds = rhs.lastSampleMilliseconds;
    }
};
// addFlash: S startMs, S peakMs, S endMs (absolute times, not phase durations).
// Opacity rises to 255 then falls to zero; multiple keys add with saturation.
struct ConfigPayload7 {
    int startMilliseconds, peakMilliseconds, endMilliseconds;
};
// addBubble: F amplitude0, F hertz0, F amplitude1, F hertz1, F amplitude2, F hertz2.
// Oscillation is integrated into a separate offset in the velocity-aligned basis:
// velocity, cross(velocity,side), side=cross(world Y,velocity), normalized.
// Frequencies multiply 2*pi*elapsedMilliseconds/1000 plus random per-particle phase.
// Each basis component then multiplies the historical 0.06*deltaMilliseconds
// motion step before integration; amplitude is not a direct position in meters.
struct ConfigPayload8 {
    float alongVelocityAmplitude, perpendicularAmplitude, sidewaysAmplitude;
    float alongVelocityHertz, perpendicularHertz, sidewaysHertz;
};
// Sound key storage contains a 260-byte name followed by one float parameter.
struct ConfigPayload9 {
    union {
        unsigned int words[66];
        struct {
            char resourceName[260];
            float parameter;
        } sound;
    };
};

// Directive names for local use; retain original type identities for unwind symbols.
typedef ConfigPayload0 GenerationKey;
typedef ConfigPayload1 VelocityKey;
typedef ConfigPayload2 RotationKey;
typedef ConfigPayload3 GravityPointKey;
typedef ConfigPayload4 SizeKey;
typedef ConfigPayload5 FrameKey;
typedef ConfigPayload6 QuakeKey;
typedef ConfigPayload7 FlashKey;
typedef ConfigPayload8 BubbleKey;
typedef ConfigPayload9 SoundKey;

// Initialize an owned tokenizer buffer.
inline ConfigText::ConfigText() {
    cTokenV_Init(this);
}
// Release tokenizer storage without deleting the stack object.
inline ConfigText::~ConfigText() {
    cToken_dtor(this, 0);
}

} // extern "C"

// Use caller-provided node storage for payload construction.
inline void* operator new(size_t, void* p) {
    return p;
}

struct ConfigAllocatorState {
    unsigned char state;
};
// Each key list is circular: the sentinel and all nodes start with the same two links.
struct ConfigNodeLinks {
    ConfigNodeLinks* previous;
    ConfigNodeLinks* next;
};

// Publish a fully constructed node at the list tail and increase its logical count.
// Caller disarms the allocation guard first; the circular list then owns the node.
inline void LinkConfigNode(void* sentinel, void* node, int& count) {
    ((ConfigNodeLinks*)sentinel)->previous->next = (ConfigNodeLinks*)node;
    ((ConfigNodeLinks*)node)->previous = ((ConfigNodeLinks*)sentinel)->previous;
    ((ConfigNodeLinks*)sentinel)->previous = (ConfigNodeLinks*)node;
    ((ConfigNodeLinks*)node)->next = (ConfigNodeLinks*)sentinel;
    ++count;
}

template <class T> struct ConfigNodeGuard {
    void* list;
    void* node;
    // Own the unlinked node until the append operation commits it.
    ConfigNodeGuard(void* l, void* n) : list(l), node(n) {}
    // Free an uncommitted node if payload construction throws.
    ~ConfigNodeGuard() {
        if (node)
            appFreeUni__FPv(node);
    }
};

// Physical payload copying is isolated from script decoding below. Word copies
// deliberately preserve float bits, load/store order and exception boundaries.
// Copy ConfigPayload0 in its required word/float order; storage is unlinked payload memory.
inline void CopyConfigPayload(void* storage, const ConfigPayload0& value) {
    unsigned int* dst = (unsigned int*)(storage);
    if (dst) {
        unsigned int first;
        unsigned int second;
        first = ((const unsigned int*)&value)[0];
        second = ((const unsigned int*)&value)[1];
        dst[0] = first;
        first = ((const unsigned int*)&value)[2];
        dst[1] = second;
        second = ((const unsigned int*)&value)[3];
        dst[2] = first;
        first = ((const unsigned int*)&value)[4];
        dst[3] = second;
        second = ((const unsigned int*)&value)[5];
        dst[4] = first;
        try {
            dst[5] = second;
        } catch (...) {
            throw;
        }
    }
}

// Copy ConfigPayload1 in its required word/float order; storage is unlinked payload memory.
inline void CopyConfigPayload(void* storage, const ConfigPayload1& value) {
    unsigned int* dst = (unsigned int*)(storage);
    if (dst) {
        unsigned int first;
        unsigned int second;
        first = ((const unsigned int*)&value)[0];
        second = ((const unsigned int*)&value)[1];
        dst[0] = first;
        first = ((const unsigned int*)&value)[2];
        dst[1] = second;
        second = ((const unsigned int*)&value)[3];
        dst[2] = first;
        try {
            dst[3] = second;
        } catch (...) {
            throw;
        }
    }
}

// Copy ConfigPayload2 in its required word/float order; storage is unlinked payload memory.
inline void CopyConfigPayload(void* storage, const ConfigPayload2& value) {
    unsigned int* dst = (unsigned int*)(storage);
    if (dst) {
        unsigned int first;
        unsigned int second;
        first = ((const unsigned int*)&value)[0];
        second = ((const unsigned int*)&value)[1];
        dst[0] = first;
        first = ((const unsigned int*)&value)[2];
        dst[1] = second;
        second = ((const unsigned int*)&value)[3];
        dst[2] = first;
        try {
            dst[3] = second;
        } catch (...) {
            throw;
        }
    }
}

// Copy ConfigPayload3 in its required word/float order; storage is unlinked payload memory.
inline void CopyConfigPayload(void* storage, const ConfigPayload3& value) {
    unsigned int* dst = (unsigned int*)(storage);
    if (dst) {
        ((unsigned int*)dst)[0] = ((const unsigned int*)&value)[0];
        ((unsigned int*)dst)[1] = ((const unsigned int*)&value)[1];
        ((unsigned int*)dst)[2] = ((const unsigned int*)&value)[2];
        ((float*)dst)[3] = ((const float*)&value)[3];
        ((float*)dst)[4] = ((const float*)&value)[4];
        float last = ((const float*)&value)[5];
        try {
            ((float*)dst)[5] = last;
        } catch (...) {
            throw;
        }
    }
}

// Copy ColorKey in its required word/float order; storage is unlinked payload memory.
inline void CopyConfigPayload(void* storage, const ColorKey& value) {
    unsigned int* dst = (unsigned int*)(storage);
    if (dst) {
        unsigned int first;
        unsigned int second;
        first = ((const unsigned int*)&value)[0];
        second = ((const unsigned int*)&value)[1];
        dst[0] = first;
        first = ((const unsigned int*)&value)[2];
        dst[1] = second;
        second = ((const unsigned int*)&value)[3];
        dst[2] = first;
        try {
            dst[3] = second;
        } catch (...) {
            throw;
        }
    }
}

// Copy ConfigPayload4 in its required word/float order; storage is unlinked payload memory.
inline void CopyConfigPayload(void* storage, const ConfigPayload4& value) {
    unsigned int* dst = (unsigned int*)(storage);
    if (dst) {
        unsigned int first = ((const unsigned int*)&value)[0];
        unsigned int second = ((const unsigned int*)&value)[1];
        try {
            dst[0] = first;
            dst[1] = second;
            unsigned int third = ((const unsigned int*)&value)[2];
            unsigned int fourth = ((const unsigned int*)&value)[3];
            dst[2] = third;
            dst[3] = fourth;
        } catch (...) {
            throw;
        }
    }
}

// Copy ConfigPayload5 in its required word/float order; storage is unlinked payload memory.
inline void CopyConfigPayload(void* storage, const ConfigPayload5& value) {
    unsigned int* dst = (unsigned int*)(storage);
    if (dst) {
        unsigned int first = ((const unsigned int*)&value)[0];
        unsigned int second = ((const unsigned int*)&value)[1];
        try {
            dst[0] = first;
            dst[1] = second;
            unsigned int third = ((const unsigned int*)&value)[2];
            unsigned int fourth = ((const unsigned int*)&value)[3];
            dst[2] = third;
            dst[3] = fourth;
            dst[4] = ((const unsigned int*)&value)[4];
        } catch (...) {
            throw;
        }
    }
}

// Copy ConfigPayload6 in its required word/float order; storage is unlinked payload memory.
inline void CopyConfigPayload(void* storage, const ConfigPayload6& value) {
    unsigned int* dst = (unsigned int*)(storage);
    if (dst) {
        ((unsigned int*)dst)[0] = ((const unsigned int*)&value)[0];
        ((unsigned int*)dst)[1] = ((const unsigned int*)&value)[1];
        ((unsigned int*)dst)[2] = ((const unsigned int*)&value)[2];
        ((unsigned int*)dst)[3] = ((const unsigned int*)&value)[3];
        ((unsigned int*)dst)[4] = ((const unsigned int*)&value)[4];
        ((unsigned int*)dst)[5] = ((const unsigned int*)&value)[5];
        ((unsigned int*)dst)[6] = ((const unsigned int*)&value)[6];
        ((unsigned int*)dst)[7] = ((const unsigned int*)&value)[7];
        ((unsigned int*)dst)[8] = ((const unsigned int*)&value)[8];
        ((unsigned int*)dst)[9] = ((const unsigned int*)&value)[9];
        ((unsigned int*)dst)[10] = ((const unsigned int*)&value)[10];
        ((unsigned int*)dst)[11] = ((const unsigned int*)&value)[11];
        float last = ((const float*)&value)[12];
        try {
            ((float*)dst)[12] = last;
        } catch (...) {
            throw;
        }
    }
}

// Copy ConfigPayload7 in its required word/float order; storage is unlinked payload memory.
inline void CopyConfigPayload(void* storage, const ConfigPayload7& value) {
    unsigned int* dst = (unsigned int*)(storage);
    if (dst) {
        unsigned int first;
        unsigned int second;
        first = ((const unsigned int*)&value)[0];
        second = ((const unsigned int*)&value)[1];
        dst[0] = first;
        first = ((const unsigned int*)&value)[2];
        dst[1] = second;
        try {
            dst[2] = first;
        } catch (...) {
            throw;
        }
    }
}

// Copy ConfigPayload8 in its required word/float order; storage is unlinked payload memory.
inline void CopyConfigPayload(void* storage, const ConfigPayload8& value) {
    unsigned int* dst = (unsigned int*)(storage);
    if (dst) {
        unsigned int first;
        unsigned int second;
        first = ((const unsigned int*)&value)[0];
        second = ((const unsigned int*)&value)[1];
        dst[0] = first;
        first = ((const unsigned int*)&value)[2];
        dst[1] = second;
        second = ((const unsigned int*)&value)[3];
        dst[2] = first;
        first = ((const unsigned int*)&value)[4];
        dst[3] = second;
        second = ((const unsigned int*)&value)[5];
        dst[4] = first;
        try {
            dst[5] = second;
        } catch (...) {
            throw;
        }
    }
}

// Copy ConfigPayload9 in its required word/float order; storage is unlinked payload memory.
inline void CopyConfigPayload(void* storage, const ConfigPayload9& value) {
    ConfigPayload9* dst = (ConfigPayload9*)(storage);
    if (dst) {
        try {
            *dst = value;
        } catch (...) {
            throw;
        }
    }
}

// Allocate, construct under a guard, then transfer ownership to the circular list.
// Copy overloads above describe representation; Decode* below describes semantics.
template <class T>
// Append one key while preserving unlinked-node cleanup if its copy throws.
inline void MakeConfigNode(void* sentinel, const T& value, int& count) {
    volatile ConfigAllocatorState allocatorState;
    allocatorState.state = 0;
    void* node = appMallocDummy__FUl(sizeof(T) + 8);
    void* volatile allocation = node;
    void* volatile construction = node;
    ConfigNodeGuard<T> guard(sentinel, node);
    CopyConfigPayload((ConfigNodeLinks*)node + 1, value);
    guard.node = 0;
    LinkConfigNode(sentinel, node, count);
}

extern "C" {

// Vector ranges occupy padding in the shared scalar-only CFxSpray view.
struct SprayScriptVectorFields {
    char beforeEmissionPosition[0x1A8];
    float minEmissionPosition[3], maxEmissionPosition[3];
    char beforeGravity[0x1E4 - 0x1C0];
    float gravity[3];
    char beforeVelocity[0x22C - 0x1F0];
    float minVelocity[3], maxVelocity[3];
    char beforeExternalWind[0x24C - 0x244];
    float externalWind[3];
    char beforeAngles[0x268 - 0x258];
    float minInitialAngle[3], maxInitialAngle[3];
    float minInitialRotation[3], maxInitialRotation[3];
};
typedef char SprayScriptVectorFields_size[(sizeof(SprayScriptVectorFields) == 0x298) ? 1 : -1];
enum SprayScriptConstants {
    kScriptValueCount = 16,
    kScriptTokenBytes = 0x104,
    kCircularEmissionScriptValue = 2,
    kScriptOpaqueAlpha = 255,
    kExternalAxisX = 1,
    kExternalAxisY = 2,
    kExternalAxisZ = 4,
    kBlendNormal = 0,
    kBlendAdd = 0x800000,
    kBlendMultiply = 0x1000000,
    kBlendInverseMultiply = 0x1800000
};

// Decode generation arguments in script order; storage/units are documented on the payload.
// values is borrowed; fill the caller's key only, without allocating or linking a node.
inline void DecodeGenerationKey(CFxSpray* self, char values[][kScriptTokenBytes], GenerationKey& generationKey) {
    generationKey.startMilliseconds = (int)((CFxSpray*)(self->effectOwner))->ParseScaledFloat(values[0]);
    generationKey.endMilliseconds = (int)((CFxSpray*)(self->effectOwner))->ParseScaledFloat(values[1]);
    generationKey.totalParticles = String_ParseDecimalInt(values[2]);
    generationKey.spreadRadians = ((CFxSpray*)(self->effectOwner))->ParseScaledFloat(values[3]);
    generationKey.emittedParticles = 0;
    generationKey.lastEmissionMilliseconds = kFloat_0_FxSpray;
}

// Decode velocity arguments in script order; storage/units are documented on the payload.
// values is borrowed; fill the caller's key only, without allocating or linking a node.
inline void DecodeVelocityKey(CFxSpray* self, char values[][kScriptTokenBytes], VelocityKey& velocityKey) {
    velocityKey.startMilliseconds = (int)((CFxSpray*)(self->effectOwner))->ParseScaledFloat(values[0]);
    velocityKey.startSpeedPercent = (float)atof(values[1]);
    velocityKey.endMilliseconds = (int)((CFxSpray*)(self->effectOwner))->ParseScaledFloat(values[2]);
    velocityKey.endSpeedPercent = (float)atof(values[3]);
}

// Decode rotation arguments in script order; storage/units are documented on the payload.
// values is borrowed; fill the caller's key only, without allocating or linking a node.
inline void DecodeRotationKey(CFxSpray* self, char values[][kScriptTokenBytes], RotationKey& rotationKey) {
    rotationKey.startLifetimePercent = (float)atof(values[0]);
    rotationKey.startAngularSpeedPercent = ((CFxSpray*)(self->effectOwner))->ParseScaledFloat(values[1]);
    rotationKey.endLifetimePercent = (float)atof(values[2]);
    rotationKey.endAngularSpeedPercent = ((CFxSpray*)(self->effectOwner))->ParseScaledFloat(values[3]);
}

// Decode gravitypoint arguments in script order; storage/units are documented on the payload.
// values is borrowed; fill the caller's key only, without allocating or linking a node.
inline void DecodeGravityPointKey(CFxSpray* self, char values[][kScriptTokenBytes], GravityPointKey& gravityPointKey) {
    gravityPointKey.offsetX = (float)atof(values[0]);
    gravityPointKey.offsetY = (float)atof(values[1]);
    gravityPointKey.offsetZ = (float)atof(values[2]);
    gravityPointKey.innerStrength = (float)atof(values[3]);
    gravityPointKey.radius = (float)atof(values[4]);
    gravityPointKey.outerStrength = (float)atof(values[5]);
}

// Decode size arguments in script order; storage/units are documented on the payload.
// values is borrowed; fill the caller's key only, without allocating or linking a node.
inline void DecodeSizeKey(CFxSpray* self, char values[][kScriptTokenBytes], SizeKey& sizeKey) {
    sscanf(values[0], fxSprayScriptStrings.floatFormat, &sizeKey.startLifetimePercentBits);
    sscanf(values[1], fxSprayScriptStrings.floatFormat, &sizeKey.startSizePercentBits);
    sscanf(values[2], fxSprayScriptStrings.floatFormat, &sizeKey.endLifetimePercentBits);
    sscanf(values[3], fxSprayScriptStrings.floatFormat, &sizeKey.endSizePercentBits);
}

// Decode frame arguments in script order; storage/units are documented on the payload.
// values is borrowed; fill the caller's key only, without allocating or linking a node.
inline void DecodeFrameKey(CFxSpray* self, char values[][kScriptTokenBytes], FrameKey& frameKey) {
    sscanf(values[0], fxSprayScriptStrings.floatFormat, &frameKey.startLifetimePercentBits);
    sscanf(values[1], fxSprayScriptStrings.floatFormat, &frameKey.endLifetimePercentBits);
    sscanf(values[2], fxSprayScriptStrings.integerFormat, &frameKey.firstFrame);
    sscanf(values[3], fxSprayScriptStrings.integerFormat, &frameKey.lastFrame);
    frameKey.intervalMilliseconds = (int)((CFxSpray*)(self->effectOwner))->ParseScaledFloat(values[4]);
}

// Decode quake arguments in script order; storage/units are documented on the payload.
// values is borrowed; fill the caller's key only, without allocating or linking a node.
inline void DecodeQuakeKey(CFxSpray* self, char values[][kScriptTokenBytes], QuakeKey& quakeKey) {
    quakeKey.centerX = (float)atof(values[0]);
    quakeKey.centerY = (float)atof(values[1]);
    quakeKey.centerZ = (float)atof(values[2]);
    quakeKey.startMilliseconds = (int)((CFxSpray*)(self->effectOwner))->ParseScaledFloat(values[3]);
    quakeKey.startAmplitudeX = (float)atof(values[4]);
    quakeKey.startAmplitudeY = (float)atof(values[5]);
    quakeKey.startAmplitudeZ = (float)atof(values[6]);
    quakeKey.endMilliseconds = (int)((CFxSpray*)(self->effectOwner))->ParseScaledFloat(values[7]);
    quakeKey.endAmplitudeX = (float)atof(values[8]);
    quakeKey.endAmplitudeY = (float)atof(values[9]);
    quakeKey.endAmplitudeZ = (float)atof(values[10]);
    quakeKey.sampleIntervalMilliseconds = (int)((CFxSpray*)(self->effectOwner))->ParseScaledFloat(values[11]);
    quakeKey.lastSampleMilliseconds = kFloat_0_FxSpray;
}

// Decode flash arguments in script order; storage/units are documented on the payload.
// values is borrowed; fill the caller's key only, without allocating or linking a node.
inline void DecodeFlashKey(CFxSpray* self, char values[][kScriptTokenBytes], FlashKey& flashKey) {
    flashKey.startMilliseconds = (int)((CFxSpray*)(self->effectOwner))->ParseScaledFloat(values[0]);
    flashKey.peakMilliseconds = (int)((CFxSpray*)(self->effectOwner))->ParseScaledFloat(values[1]);
    flashKey.endMilliseconds = (int)((CFxSpray*)(self->effectOwner))->ParseScaledFloat(values[2]);
}

// Decode bubble arguments in script order; storage/units are documented on the payload.
// values is borrowed; fill the caller's key only, without allocating or linking a node.
inline void DecodeBubbleKey(CFxSpray* self, char values[][kScriptTokenBytes], BubbleKey& bubbleKey) {
    bubbleKey.alongVelocityAmplitude = (float)atof(values[0]);
    bubbleKey.alongVelocityHertz = (float)atof(values[1]);
    bubbleKey.perpendicularAmplitude = (float)atof(values[2]);
    bubbleKey.perpendicularHertz = (float)atof(values[3]);
    bubbleKey.sidewaysAmplitude = (float)atof(values[4]);
    bubbleKey.sidewaysHertz = (float)atof(values[5]);
}

// Decode sound arguments in script order; storage/units are documented on the payload.
// values is borrowed; fill the caller's key only, without allocating or linking a node.
inline void DecodeSoundKey(CFxSpray* self, char values[][kScriptTokenBytes], SoundKey& soundKey) {
    soundKey.sound.parameter = kFloat_neg_1_805DE648;
    strcpy(soundKey.sound.resourceName, values[0]);
    char* suffix = strstr(((char*)soundKey.words), fxSprayScriptStrings.waveSuffix);
    if (suffix)
        *suffix = 0;
    sscanf(values[1], fxSprayScriptStrings.floatFormat, &soundKey.sound.parameter);
}

// Parse one effect-script directive, updating scalar ranges or appending an animation key.
// Grammar: key = value0, value1, ... (the actual delimiters/trim sets are in
// fxSprayScriptStrings). At most 16 values of 260 bytes are staged. Empty/missing
// values are passed to each conversion as empty strings; no arity validation is
// performed. Return 1 for handled directives, blank lines and leading ';'; return
// 0 for unknown keys, unsupported blends or failed resource operations.
// This is an incremental update, NOT a transaction: earlier field writes and
// resource releases remain on failure. Scalar keys overwrite, add* keys append
// owned list nodes. input is borrowed and no token points into it after return.
//
// Argument guide (zero based; D/F/S as defined beside the payloads):
// source: pet path, optional petLighting; otherwise sprite path,D,D,billboard tag.
// tail: D segments,S sample interval,D alpha/red/green/blue; optional bitmap path,
//       F width,monolith tag,cameraSide tag. SphericalEmission: D (2 also circular).
// emissionPosition: F min xyz, F max xy, max z is F or S in spherical mode.
// regenerateAtSamePosition:D; cameraOffset:F; lifetime:(int)S min,max.
// volumeOpacity:F clamped 0..1; volumeCore:F x,y clamped 0..1.
// initialVelocity:F min xyz,max xyz; windScale:F min,max; externalWind:F xyz.
// externalOrientation:D x,y,z enable bits; initialAngle/initialRotation:S min/max xyz.
// friction/groundFriction:F; gravity:F xyz; groundCollision:F offset,restitution,radius.
// blend:sprite mode,trail mode (normal/add/multiply/inverseMultiply tokens).
// lighting:D percent (legacy clamp: >255 becomes 100, negative becomes 0).
// addFade has TEN tokens: F startAgePercent,D alpha,D red,D green,D blue,
// F endAgePercent,D alpha,D red,D green,D blue (channels truncate to bytes).
// Example: addFade = 0,255,255,0,0,100,0,255,0,0
// fades red from opaque at birth to transparent at 100% of particle lifetime.
// initialSize:%f min,max,aspect.
// initialFrame:D min,max; distanceLimit:F min,max (nonnegative, max >= min).
// sound:path,optional %f parameter (defaults to -1; '.wav' suffix is removed).
// Remaining add* argument-to-storage maps are beside ConfigPayload0..8 above.
int CFxSpray::ParseScriptLine(const char* input) {
    char values[kScriptValueCount][kScriptTokenBytes];
    void* sentinel;
    volatile char frame_pad[0x3e0];
    (void)frame_pad;
    int first = (signed char)*input;
    if (first == 0 || first == ';')
        return 1;

    ConfigText source;
    // Phase 1: tokenize and trim into local buffers before mutating the effect.
    cTokenV_SetInputBuffer(&source, input, strlen(input));

    char key[kScriptTokenBytes];
    cToken_ReadUntilDelimiter(&source, key, fxSprayScriptStrings.assignmentDelimiter, 1);
    String_TrimCharacterSet(key, fxSprayScriptStrings.trimWhitespace);
    for (int i = 0; i < kScriptValueCount; ++i) {
        cToken_ReadUntilDelimiter(&source, values[i], fxSprayScriptStrings.valueDelimiter, 1);
        String_TrimCharacterSet(values[i], (fxSprayScriptStrings.trimValueCharacters));
    }
    // Phase 2: dispatch in script order. Resource/range directives overwrite
    // selected fields; each add* branch constructs a complete owned key node.
    if (!CString_Compare(key, fxSprayScriptStrings.source)) {
        ConfigText value;
        char* token;
        cTokenV_SetInputBuffer(&value, values[0], strlen(values[0]));
        const char* separators = fxSprayScriptStrings.wordSeparators;
        if (!FxSprayScriptBuffer_EnsureTerminatorCapacity(&value))
            token = 0;
        else {
            token = value.buffer;
            *token = 0;
            while (value.pos < value.end) {
                cToken_ReadUntilDelimiter(&value, token, separators, 2);
                if (*token)
                    break;
            }
        }
        if (!CString_Compare(token, fxSprayScriptStrings.petSource)) {
            char* name;
            const char* separators = fxSprayScriptStrings.wordSeparators;
            if (!FxSprayScriptBuffer_EnsureTerminatorCapacity(&value))
                name = 0;
            else {
                name = value.buffer;
                *name = 0;
                while (value.pos < value.end) {
                    cToken_ReadUntilDelimiter(&value, name, separators, 2);
                    if (*name)
                        break;
                }
            }
            void* manager = (void*)((CFxBox*)effectOwner)->resourceManager;
            if (!manager)
                return 0;
            if (!CString_Compare((const char*)((CHashPath*)manager)->FindPathForwarder(name), name))
                return 0;
            strcpy((char*)&petResourceName, values[0]);
            *(bool*)&petLightingEnabled = !CString_Compare(values[1], fxSprayScriptStrings.petLighting);
        } else {

            void* result = ((CFxBox*)(effectOwner))
                               ->GetOrLoadAnimatedSprite(values[0], (String_ParseDecimalInt(values[1])),
                                                         (String_ParseDecimalInt(values[2])));
            *(void**)&spriteResource = result;
            if (0 == result) {
                return 0;
            }
            billboardEnabled = !strlen(values[3]) || !CString_Compare(values[3], fxSprayScriptStrings.billboard);
        }

    } else if (!CString_Compare(key, fxSprayScriptStrings.tail)) {
        *(int*)&trailSegmentCount = String_ParseDecimalInt(values[0]);
        trailSampleInterval = ((CFxSpray*)(effectOwner))->ParseScaledFloat(values[1]);
        trailEndAlpha = String_ParseDecimalInt(values[2]);
        trailEndRed = String_ParseDecimalInt(values[3]);
        trailEndGreen = String_ParseDecimalInt(values[4]);
        trailEndBlue = String_ParseDecimalInt(values[5]);
        if (strlen(values[6])) {
            void* manager = (void*)((CFxBox*)effectOwner)->resourceManager;
            if (!manager) {
                return 0;
            }
            void** resource = (void**)((char*)&resourceId);
            if (*resource) {
                ResourceManager_ReleaseId(manager, *resource);
                *resource = 0;
            }
            *resource = ResourceManager_LoadBitmap((void*)((CFxBox*)effectOwner)->resourceManager, values[6], 0, 0, 0);
            if (!*resource) {
                return 0;
            }
            trailWidthScale = (float)atof(values[7]);
            *(bool*)&trailMonolithEnabled = !CString_Compare(values[8], fxSprayScriptStrings.monolith);
            *(bool*)&trailCameraSideEnabled = !CString_Compare(values[9], fxSprayScriptStrings.cameraSide);
        }
        // Emission geometry and generation: position components are effect-space values.
    } else if (!CString_Compare(key, fxSprayScriptStrings.sphericalEmission)) {
        int n = String_ParseDecimalInt(values[0]);
        sphericalEmission = 0 != n;
        if (n == kCircularEmissionScriptValue)
            circularEmission = 1;
    } else if (!CString_Compare(key, fxSprayScriptStrings.emissionPosition)) {
        __as__7WVectorFRC7WVector(((SprayScriptVectorFields*)this)->minEmissionPosition,
                                  ConfigVector((float)atof(values[0]), (float)atof(values[1]), (float)atof(values[2])));
        if (!sphericalEmission) {
            __as__7WVectorFRC7WVector(
                ((SprayScriptVectorFields*)this)->maxEmissionPosition,
                ConfigVector((float)atof(values[3]), (float)atof(values[4]), (float)atof(values[5])));
        } else {
            __as__7WVectorFRC7WVector(((SprayScriptVectorFields*)this)->maxEmissionPosition,
                                      ConfigVector((float)atof(values[3]), (float)atof(values[4]),
                                                   ((CFxSpray*)(effectOwner))->ParseScaledFloat(values[5])));
        }
    } else if (!CString_Compare(key, fxSprayScriptStrings.regenerateAtSamePosition)) {
        regenerateAtSamePosition = String_ParseDecimalInt(values[0]) != 0;
    } else if (!CString_Compare(key, fxSprayScriptStrings.cameraOffset)) {
        cameraOffset = (float)atof(values[0]);
    } else if (!CString_Compare(key, fxSprayScriptStrings.addGeneration)) {
        GenerationKey generationKey;
        DecodeGenerationKey(this, values, generationKey);
        sentinel = (char*)&AsCFxParticle(this)->generationKeyTail;
        MakeConfigNode(sentinel, generationKey, (*(int*)&AsCFxParticle(this)->generationKeyCount));
    } else if (!CString_Compare(key, fxSprayScriptStrings.lifetime)) {
        // Zero minimum lifetime selects the volume path: the first time-zero fade
        // key must have opaque endpoint alpha. Nonzero lifetime selects the other
        // path and resets volume-core/opacity overrides to unity. The rendering
        // reason for this distinction is outside this parser; order matters:
        // this visits the FIRST existing zero-time key, while addFade below fixes
        // each newly appended zero-time key. maxLifetime does not choose the path.
        *(int*)&minLifetime = (int)((CFxSpray*)(effectOwner))->ParseScaledFloat(values[0]);
        *(int*)&maxLifetime = (int)((CFxSpray*)(effectOwner))->ParseScaledFloat(values[1]);
        if (!*(int*)&minLifetime) {
            char* end = (char*)&AsCFxParticle(this)->fadeKeyTail;
            char* node = *(char**)(end + 4);
            while (end != node) {
                if (*(float*)(8 + node) == 0.0f) {
                    ((unsigned char*)node)[0x14] = kScriptOpaqueAlpha;
                    ((unsigned char*)node)[0xc] = kScriptOpaqueAlpha;
                    break;
                }
                node = *(char**)(node + 4);
            }
        } else {
            volumeCoreScaleY = 1.0f;
            volumeCoreScaleX = 1.0f;
            opacityMultiplier = 1.0f;
        }
    } else if (!CString_Compare(key, fxSprayScriptStrings.volumeOpacity)) {
        float v = (float)atof(values[0]);
        opacityMultiplier = kFloat_0_FxSpray > v ? kFloat_0_FxSpray : kFloat_1_FxSpray < v ? kFloat_1_FxSpray : v;
        if (*(int*)&minLifetime)
            opacityMultiplier = 1.0f;
    } else if (!CString_Compare(key, fxSprayScriptStrings.volumeCore)) {
        if (*(int*)&minLifetime) {
            volumeCoreScaleY = 1.0f;
            volumeCoreScaleX = 1.0f;
        } else {
            float coreScaleX = (float)atof(values[0]);
            volumeCoreScaleX = kFloat_0_FxSpray > coreScaleX   ? kFloat_0_FxSpray
                               : kFloat_1_FxSpray < coreScaleX ? kFloat_1_FxSpray
                                                               : coreScaleX;
            float coreScaleY = (float)atof(values[1]);
            volumeCoreScaleY = kFloat_0_FxSpray > coreScaleY   ? kFloat_0_FxSpray
                               : kFloat_1_FxSpray < coreScaleY ? kFloat_1_FxSpray
                                                               : coreScaleY;
        }
        // Motion keys: decode completely before publishing to their owning lists.
    } else if (!CString_Compare(key, fxSprayScriptStrings.initialVelocity)) {
        __as__7WVectorFRC7WVector(((SprayScriptVectorFields*)this)->minVelocity,
                                  ConfigVector((float)atof(values[0]), (float)atof(values[1]), (float)atof(values[2])));
        __as__7WVectorFRC7WVector(((SprayScriptVectorFields*)this)->maxVelocity,
                                  ConfigVector((float)atof(values[3]), (float)atof(values[4]), (float)atof(values[5])));
    } else if (!CString_Compare(key, fxSprayScriptStrings.addVelocity)) {
        VelocityKey velocityKey;
        DecodeVelocityKey(this, values, velocityKey);
        sentinel = (char*)&AsCFxParticle(this)->velocityKeyTail;
        MakeConfigNode(sentinel, velocityKey, (*(int*)&AsCFxParticle(this)->velocityKeyCount));
    } else if (!CString_Compare(key, fxSprayScriptStrings.windScale)) {
        minWindScale = (float)atof(values[0]);
        maxWindScale = (float)atof(values[1]);
    } else if (!CString_Compare(key, fxSprayScriptStrings.externalWind)) {
        __as__7WVectorFRC7WVector(((SprayScriptVectorFields*)this)->externalWind,
                                  ConfigVector((float)atof(values[0]), (float)atof(values[1]), (float)atof(values[2])));
    } else if (!CString_Compare(key, fxSprayScriptStrings.externalOrientation)) {
        unsigned int a = 0;
        int b = 0;
        int zEnabled = 0;
        sscanf(values[0], fxSprayScriptStrings.integerFormat, &a);
        sscanf(values[1], fxSprayScriptStrings.integerFormat, &b);
        sscanf(values[2], fxSprayScriptStrings.integerFormat, &zEnabled);
        int* flags = (int*)((char*)&externalOrientationAxisMask);
        *flags = 0;
        if (a)
            *flags |= kExternalAxisX;
        if (b)
            *flags |= kExternalAxisY;
        if (zEnabled)
            *flags |= kExternalAxisZ;
    } else if (!CString_Compare(key, fxSprayScriptStrings.initialAngle)) {
        ((SprayScriptVectorFields*)this)->minInitialAngle[0] = ((CFxSpray*)(effectOwner))->ParseScaledFloat(values[0]);
        ((SprayScriptVectorFields*)this)->minInitialAngle[1] = ((CFxSpray*)(effectOwner))->ParseScaledFloat(values[1]);
        ((SprayScriptVectorFields*)this)->minInitialAngle[2] = ((CFxSpray*)(effectOwner))->ParseScaledFloat(values[2]);
        ((SprayScriptVectorFields*)this)->maxInitialAngle[0] = ((CFxSpray*)(effectOwner))->ParseScaledFloat(values[3]);
        ((SprayScriptVectorFields*)this)->maxInitialAngle[1] = ((CFxSpray*)(effectOwner))->ParseScaledFloat(values[4]);
        ((SprayScriptVectorFields*)this)->maxInitialAngle[2] = ((CFxSpray*)(effectOwner))->ParseScaledFloat(values[5]);
    } else if (!CString_Compare(key, fxSprayScriptStrings.initialRotation)) {
        ((SprayScriptVectorFields*)this)->minInitialRotation[0] =
            ((CFxSpray*)(effectOwner))->ParseScaledFloat(values[0]);
        ((SprayScriptVectorFields*)this)->minInitialRotation[1] =
            ((CFxSpray*)(effectOwner))->ParseScaledFloat(values[1]);
        ((SprayScriptVectorFields*)this)->minInitialRotation[2] =
            ((CFxSpray*)(effectOwner))->ParseScaledFloat(values[2]);
        ((SprayScriptVectorFields*)this)->maxInitialRotation[0] =
            ((CFxSpray*)(effectOwner))->ParseScaledFloat(values[3]);
        ((SprayScriptVectorFields*)this)->maxInitialRotation[1] =
            ((CFxSpray*)(effectOwner))->ParseScaledFloat(values[4]);
        ((SprayScriptVectorFields*)this)->maxInitialRotation[2] =
            ((CFxSpray*)(effectOwner))->ParseScaledFloat(values[5]);
    } else if (!CString_Compare(key, fxSprayScriptStrings.addRotation)) {
        RotationKey rotationKey;
        DecodeRotationKey(this, values, rotationKey);
        sentinel = (char*)&AsCFxParticle(this)->rotationKeyTail;
        MakeConfigNode(sentinel, rotationKey, (*(int*)&AsCFxParticle(this)->rotationKeyCount));
    } else if (!CString_Compare(key, fxSprayScriptStrings.friction)) {
        friction = (float)atof(values[0]);
    } else if (!CString_Compare(key, fxSprayScriptStrings.groundFriction)) {
        groundFriction = (float)atof(values[0]);
    } else if (!CString_Compare(key, fxSprayScriptStrings.gravity)) {
        __as__7WVectorFRC7WVector(((SprayScriptVectorFields*)this)->gravity,
                                  ConfigVector((float)atof(values[0]), (float)atof(values[1]), (float)atof(values[2])));
    } else if (!CString_Compare(key, fxSprayScriptStrings.addGravityPoint)) {
        GravityPointKey gravityPointKey;
        DecodeGravityPointKey(this, values, gravityPointKey);
        sentinel = (char*)&AsCFxParticle(this)->gravityPointKeyTail;
        MakeConfigNode(sentinel, gravityPointKey, (*(int*)&AsCFxParticle(this)->gravityPointKeyCount));
    } else if (!CString_Compare(key, fxSprayScriptStrings.groundCollision)) {
        groundHeightOffset = (float)atof(values[0]);
        groundRestitution = (float)atof(values[1]);
        groundCollisionRadius = (float)atof(values[2]);
        // Appearance: blend writes are sequential; invalid trail mode leaves sprite mode changed.
    } else if (!CString_Compare(key, fxSprayScriptStrings.blend)) {
        int* firstMode = (int*)((char*)&spriteBlendMode);
        if (!CString_Compare(values[0], fxSprayScriptStrings.normalBlend))
            *firstMode = kBlendNormal;
        else if (!CString_Compare(values[0], fxSprayScriptStrings.addBlend))
            *firstMode = kBlendAdd;
        else if (!CString_Compare(values[0], fxSprayScriptStrings.multiplyBlend))
            *firstMode = kBlendMultiply;
        else if (!CString_Compare(values[0], fxSprayScriptStrings.inverseMultiplyBlend))
            *firstMode = kBlendInverseMultiply;
        else {
            return 0;
        }
        int* secondMode = (int*)((char*)&trailBlendMode);
        if (!CString_Compare(values[1], fxSprayScriptStrings.normalBlend))
            *secondMode = kBlendNormal;
        else if (!CString_Compare(values[1], fxSprayScriptStrings.addBlend))
            *secondMode = kBlendAdd;
        else if (!CString_Compare(values[1], fxSprayScriptStrings.multiplyBlend))
            *secondMode = kBlendMultiply;
        else if (!CString_Compare(values[1], fxSprayScriptStrings.inverseMultiplyBlend))
            *secondMode = kBlendInverseMultiply;
        else {
            return 0;
        }
    } else if (!CString_Compare(key, fxSprayScriptStrings.lighting)) {
        if (*(unsigned char*)&g_sharedDoc->pad_9C6[0] /* shared effect-lighting switch */) {
            *(int*)&colorModulationEnabled = 3;
            int n = String_ParseDecimalInt(values[0]);
            if (n > 255)
                n = 100;
            else if (0 > n)
                n = 0;
            int c = (255 * n) / 100;
            *(int*)&lightingColor = c | (c << 16) | (c << 8);
        } else
            *(int*)&lightingColor = 0xffffff;
    } else if (!CString_Compare(key, fxSprayScriptStrings.addFade)) {
        // Lifetime percentages, not emitter milliseconds. For zero minimum lifetime
        // and a zero start percent, both alpha endpoints are forced to 255.
        ColorKey data;
        data.startLifetimePercent = (float)atof(values[0]);
        data.startAlpha = String_ParseDecimalInt(values[1]);
        data.startRed = String_ParseDecimalInt(values[2]);
        data.startGreen = String_ParseDecimalInt(values[3]);
        data.startBlue = String_ParseDecimalInt(values[4]);
        data.endLifetimePercent = (float)atof(values[5]);
        data.endAlpha = String_ParseDecimalInt(values[6]);
        data.endRed = String_ParseDecimalInt(values[7]);
        data.endGreen = String_ParseDecimalInt(values[8]);
        data.endBlue = String_ParseDecimalInt(values[9]);
        if (!*(int*)&minLifetime) {
            if (kFloat_0_FxSpray == data.startLifetimePercent) {
                data.endAlpha = kScriptOpaqueAlpha;
                data.startAlpha = kScriptOpaqueAlpha;
            }
        }
        sentinel = (char*)&AsCFxParticle(this)->fadeKeyTail;
        MakeConfigNode(sentinel, data, (*(int*)&AsCFxParticle(this)->fadeKeyCount));
    } else if (!CString_Compare(key, fxSprayScriptStrings.initialSize)) {
        sscanf(values[0], fxSprayScriptStrings.floatFormat, (char*)&minInitialSize);
        sscanf(values[1], fxSprayScriptStrings.floatFormat, (char*)&maxInitialSize);
        sscanf(values[2], fxSprayScriptStrings.floatFormat, (char*)&spriteAspectRatio);
    } else if (!CString_Compare(key, fxSprayScriptStrings.addSize)) {
        SizeKey sizeKey;
        DecodeSizeKey(this, values, sizeKey);
        sentinel = (char*)&AsCFxParticle(this)->sizeKeyTail;
        MakeConfigNode(sentinel, sizeKey, (*(int*)&AsCFxParticle(this)->sizeKeyCount));
    } else if (!CString_Compare(key, fxSprayScriptStrings.initialFrame)) {
        sscanf(values[0], fxSprayScriptStrings.integerFormat, (char*)&minInitialFrame);
        sscanf(values[1], fxSprayScriptStrings.integerFormat, (char*)&maxInitialFrame);
    } else if (!CString_Compare(key, fxSprayScriptStrings.addFrame)) {
        FrameKey frameKey;
        DecodeFrameKey(this, values, frameKey);
        sentinel = (char*)&AsCFxParticle(this)->frameKeyTail;
        MakeConfigNode(sentinel, frameKey, (*(int*)&AsCFxParticle(this)->frameKeyCount));
        // Auxiliary keys: quake/flash/bubble/sound each append one independent node.
    } else if (!CString_Compare(key, fxSprayScriptStrings.addQuake)) {
        QuakeKey quakeKey;
        DecodeQuakeKey(this, values, quakeKey);
        void* sentinel = (char*)&AsCFxParticle(this)->quakeKeyTail;
        MakeConfigNode(sentinel, quakeKey, (*(int*)&AsCFxParticle(this)->quakeKeyCount));
    } else if (!CString_Compare(key, fxSprayScriptStrings.addFlash)) {
        FlashKey flashKey;
        DecodeFlashKey(this, values, flashKey);
        sentinel = (char*)&AsCFxParticle(this)->flashKeyTail;
        MakeConfigNode(sentinel, flashKey, (*(int*)&AsCFxParticle(this)->flashKeyCount));
    } else if (!CString_Compare(key, fxSprayScriptStrings.distanceLimit)) {
        float a = (float)atof(values[0]);
        minDrawDistance = 0.0f < a ? a : 0.0f;
        float b = (float)atof(values[1]);
        maxDrawDistance = 0.0f < b ? b : 0.0f;
        if (minDrawDistance > maxDrawDistance)
            maxDrawDistance = minDrawDistance;
    } else if (!CString_Compare(key, fxSprayScriptStrings.addBubble)) {
        BubbleKey bubbleKey;
        DecodeBubbleKey(this, values, bubbleKey);
        sentinel = (char*)&AsCFxParticle(this)->bubbleKeyTail;
        MakeConfigNode(sentinel, bubbleKey, (*(int*)&AsCFxParticle(this)->bubbleKeyCount));
    } else if (!CString_Compare(key, fxSprayScriptStrings.sound)) {
        SoundKey soundKey;
        DecodeSoundKey(this, values, soundKey);
        sentinel = (char*)&AsCFxParticle(this)->soundKeyTail;
        MakeConfigNode(sentinel, soundKey, (*(int*)&AsCFxParticle(this)->soundKeyCount));
    } else {

        return 0;
    }

    return 1;
}

} // extern "C"

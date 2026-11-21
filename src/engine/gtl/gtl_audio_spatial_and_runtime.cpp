#include <decomp/static_init.h>
#include "classes/GtlAudioRuntimeState.h"
#include "classes/GtlAudioCurve.h"
// Translation unit 0x803FDEC8-0x803FF5B8 (15 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

extern "C" {

extern "C" void gtl_Vec2_Copy(float*, const float*);
extern "C" void Vec2_CatmullRom(float*, const float*, const float*, const float*, const float*, float);

extern float kFloat_6_2831855;
extern float kFloat_1_GtlAudio;
extern float s_spatialAudioHelperFullTurnAngle;
extern float s_audioOmnidirectionalConeAngle;
extern int g_gtlAudioManager;
extern signed char s_audioStateInitialized;
extern unsigned int kAudioPropertyDefaults[];
extern unsigned int s_gtlAudioRuntime[];
extern unsigned char s_soundBufferParameterDefaults[];
void* memset(void*, int, unsigned int);
void* gtl_RemoteAudio_RequestEnableAll();
void* gtl_RemoteAudio_RequestDisableAll();
void* gtl_AudioController_GetInstance();
void gtl_AudioController_SetOutputMode(void*, int);
extern float kFloat_0_spatialAudio;
extern float kFloat_1_SpatialAudio;
extern float kDopplerListenerSupersonicRatio;
extern float s_float340_29_SpatialAudio;
extern float kFloat_neg1_SpatialAudio;
extern float kFloat_1_SpatialAudio;
extern float kFloat_0_5_SpatialAudio;
extern float kFloat_3_1415927;
extern float s_audioOmnidirectionalConeAngle;
extern "C" void Vec3_Scale(float*, const float*, float);
extern "C" void Vec3_Normalize(float*, const float*);
extern "C" float Vec3_Dot(const float*, const float*);
extern "C" double acos(double);
extern "C" void Vec3_Subtract(float*, const float*, const float*);
extern "C" float Vec3_Length(const float*);
extern "C" void Vec3_Add(float*, const float*, const float*);
extern "C" void MTXLookAt(float*, const float*, const float*, const float*);
extern "C" void Mtx44_TransformPointPerspective(void*, const void*, const float*);
extern "C" void Mtx44_TransformDirection(float*, const float*, const float*);
extern "C" int gtl_Audio_CalculatePanAndAttenuation(const float*, const float*, const float*, float*);

extern float s_spatialAudioDirectionAndDistance[4]; // [0..2] normalized direction, [3] distance (read by gtl_Audio_CalculatePanAndAttenuation as data[3])

struct SpatialAudioSource {
    // Position/velocity use the caller's common scene units; no conversion occurs.
    // forward/up are orientation vectors; cone angles are full angles in radians.
    float position[3];
    float velocity[3];
    float forward[3];
    float up[3];
    float innerConeAngle;
    float outerConeAngle;
    float innerConeGain;
    float outerConeGain;
    void* distanceCurve;
    // +0x44: zero bypasses ALL spatial processing; otherwise scales Doppler with +0x48.
    // Upstream SetDistanceParameterA stores this word; its physical meaning is unresolved.
    float dopplerAmountScale;
    float dopplerFactor;
    float referenceDistance;
    float maximumDistance;
    int coordinateMode;
};
// Borrowed emitter-layout view; an inline cast helper changed generated code.
#define AsSpatialSource(p) ((SpatialAudioSource*)(p))
enum AudioOutputMode { kAudioMono = 1, kAudioStereo = 2, kAudioSurround = 3, kAudioSurroundAlternate = 4 };
enum SpatialCoordinateMode { kSpatialRelative = 0, kSpatialDisabled = 2 };
enum AudioCurveMode { kCurveLinear = 1, kCurveCatmullRom = 2 };
enum SpatialParameterSlot { kSpatialPanX = 0, kSpatialPanDepth = 1,
    kSpatialGain = 2, kSpatialDopplerRatio = 3 };

// Compute pan, distance/cone gain and Doppler ratio, optionally culling beyond maximum distance.
// Borrowed inputs: listenerState = 12 floats [position xyz, velocity xyz, forward
// xyz, up xyz]; sourceState = the 22-word SpatialAudioSource layout above.
// Writable parameters[4] = [pan X, pan depth, distance*cone gain, Doppler ratio].
// Pan coordinates use the configured pan radius; gain and Doppler are multipliers.
// A NON-NULL cullAtMaximumDistance enables culling; its pointed-to int is never read.
// Return 1 includes disabled/zero-distance/culled cases; 0 means the pan helper failed.
int gtl_Audio_CalculateSpatialParameters(const float* listenerState, SpatialAudioSource* sourceState, float* parameters, int* cullAtMaximumDistance) {
    // Phase 1: neutral pan/unit gains, distance check and optional maximum-distance mute.
    parameters[1] = kFloat_0_spatialAudio;
    parameters[0] = kFloat_0_spatialAudio;
    parameters[2] = kFloat_1_SpatialAudio;
    parameters[3] = kFloat_1_SpatialAudio;
    if (sourceState->coordinateMode == kSpatialDisabled || sourceState->dopplerAmountScale == kFloat_0_spatialAudio)
        return 1;
    float difference[3];
    Vec3_Subtract(difference, (const float*)sourceState, listenerState);
    float distance = Vec3_Length(difference);
    s_spatialAudioDirectionAndDistance[3] = distance;
    if (distance <= kFloat_0_spatialAudio)
        return 1;
    if (cullAtMaximumDistance && distance >= sourceState->maximumDistance) {
        parameters[2] = kFloat_0_spatialAudio;
        return 1;
    }
    float listenerTarget[3];
    float transformedListener[12];
    float listenerView[16];
    float transformedSource[22];
    const float* listenerForPan = listenerState;
    const float* sourceForPan = (const float*)sourceState;
    if (sourceState->coordinateMode == kSpatialRelative) {
        // Phase 2: world -> listener view using position + forward as look-at target.
        // Transform source position, velocity and orientation, but listener velocity
        // only. The copied listener position is not used by the pan helper below.
        Vec3_Add(listenerTarget, listenerState, listenerState + 6);
        MTXLookAt(listenerView, listenerState, listenerTarget, listenerState + 9);
        for (int i = 0; i < 22; ++i)
            ((unsigned int*)transformedSource)[i] = ((unsigned int*)sourceState)[i];
        Mtx44_TransformPointPerspective(transformedSource, sourceState, listenerView);
        Mtx44_TransformDirection((float*)transformedSource + 6, (const float*)sourceState + 6, listenerView);
        Mtx44_TransformDirection((float*)transformedSource + 9, (const float*)sourceState + 9, listenerView);
        Mtx44_TransformDirection((float*)transformedSource + 3, (const float*)sourceState + 3, listenerView);
        for (int i = 0; i < 12; ++i)
            ((unsigned int*)transformedListener)[i] = ((unsigned int*)listenerState)[i];
        Mtx44_TransformDirection(transformedListener + 3, listenerState + 3, listenerView);
        listenerForPan = transformedListener;
        sourceForPan = transformedSource;
    }
    // Phase 3: normalize SOURCE POSITION in the pan frame, not the world-space
    // difference used for distance. In relative mode the view maps the listener to
    // the origin; the other mode uses source coordinates as supplied (origin-based).
    // Pan azimuth is atan2(x,-z): forward is -Z, horizontal right is +X, up is +Y.
    // The global scratch vector makes this routine non-reentrant.
    Vec3_Normalize(s_spatialAudioDirectionAndDistance, sourceForPan);
    if (!gtl_Audio_CalculatePanAndAttenuation(listenerForPan, sourceForPan, s_spatialAudioDirectionAndDistance, parameters))
        return 0;
    if (sourceState->dopplerFactor == kFloat_0_spatialAudio) {
        parameters[3] = kFloat_1_SpatialAudio;
        return 1;
    }
    float dopplerAmount = sourceState->dopplerAmountScale * sourceState->dopplerFactor;
    // Phase 4: project both velocities onto that same ray. The stored speed 340.29
    // is compared in the caller's velocity scale; this function imposes no time unit.
    float listenerVelocityAlongRay = Vec3_Dot(listenerForPan + 3, s_spatialAudioDirectionAndDistance);
    float listenerDopplerVelocity = dopplerAmount * listenerVelocityAlongRay;
    float sourceVelocityAlongRay = Vec3_Dot(sourceForPan + 3, s_spatialAudioDirectionAndDistance);
    float sourceDopplerVelocity = dopplerAmount * sourceVelocityAlongRay;
    if ((listenerDopplerVelocity < 0.0f ? -listenerDopplerVelocity : listenerDopplerVelocity) >= s_float340_29_SpatialAudio)
        parameters[3] = kDopplerListenerSupersonicRatio;
    else if ((sourceDopplerVelocity < 0.0f ? -sourceDopplerVelocity : sourceDopplerVelocity) >= s_float340_29_SpatialAudio)
        parameters[3] = kFloat_0_spatialAudio;
    else
        parameters[3] = (s_float340_29_SpatialAudio - sourceDopplerVelocity) / (s_float340_29_SpatialAudio - listenerDopplerVelocity);
    return 1;
}

// Initialize this spatial-audio helper's full-turn angle cache.
void InitSpatialAudioHelperFullTurnAngle() {
    s_spatialAudioHelperFullTurnAngle = kFloat_6_2831855;
}

// Set both default cone angles to a full turn, giving omnidirectional sound.
void SoundBuffer_InitializeConeAngleDefaults() {
    s_audioOmnidirectionalConeAngle = kFloat_6_2831855;
    AsSpatialSource(s_soundBufferParameterDefaults)->innerConeAngle = kFloat_6_2831855;
    AsSpatialSource(s_soundBufferParameterDefaults)->outerConeAngle = kFloat_6_2831855;
}

typedef GtlAudioCurvePoint CurvePoint;
typedef GtlAudioCurve Curve;
float gtl_AudioCurve_Evaluate(Curve*, float);
extern float s_spatialAudioPanRadius;
extern float kSurroundPanQuarterAngleDefault;
extern float kFloat_3_1415927;
extern float kFloat_1_5707964_tweenDartsAudio;
extern float kSmallPanSectorDivisor;
extern float kSurroundOuterSectorDivisor;
extern float kFloat_0_5_SpatialAudio;
extern signed char s_surroundPanQuarterAngleInitialized, s_spatialAudioSmallPanAngleInitialized, s_surroundPanOuterAngleInitialized;
extern float s_surroundPanQuarterAngle, s_spatialAudioSmallPanAngle, s_surroundPanOuterAngle;
extern int gtl_Audio_GetOutputMode();
extern double atan2(double, double);
extern float Math_CosApprox(float);
extern float gtl_Audio_CalculateConeGain(float*, const float*);

/* Piecewise-linear segment from (x0,y0) to (x1,y1) as the original computes it; equal ends give the midpoint. */
static inline float Seg803FEC2C(float a, float x0, float x1, float y0, float y1) {
    if (x0 == x1)
        return (y0 + y1) * 0.5f;
    return a * (y0 - y1) / (x0 - x1) + (x0 * y1 - x1 * y0) / (x0 - x1);
}

// Map horizontal direction to output-channel pan and evaluate distance and cone attenuation.
// Same output-slot contract as CalculateSpatialParameters; writes slots 0..2 only.
// sourceState must already be in the intended origin-based pan frame. directionAndDistance
// stores normalized source direction plus distance at [3]; unused listener input is retained by ABI.
int gtl_Audio_CalculatePanAndAttenuation(const float* unused, const float* sourceState, const float* directionAndDistance, float* parameters) {
    // Phase 1: clamp the XZ projection to the pan radius and derive azimuth/distance.
    if (directionAndDistance[3] <= kFloat_0_spatialAudio) return 0;
    float horizontalPosition[6] = { sourceState[0], 0.0f, sourceState[2], 0.0f, 0.0f, 0.0f };
    float horizontalDistance = Vec3_Length(horizontalPosition);
    if (horizontalDistance > s_spatialAudioPanRadius) {
        float radiusScale = s_spatialAudioPanRadius / horizontalDistance;
        horizontalPosition[0] *= radiusScale; horizontalPosition[2] *= radiusScale;
    }
    float clampedDistance = Vec3_Length(horizontalPosition);
    float panPosition[3];
    panPosition[0] = sourceState[0] * clampedDistance / directionAndDistance[3];
    panPosition[1] = kFloat_0_spatialAudio;
    panPosition[2] = sourceState[2] * clampedDistance / directionAndDistance[3];
    float azimuth = (float)atan2(panPosition[0], -panPosition[2]);
    float panDistance = Vec3_Length(panPosition);
    if (!s_surroundPanQuarterAngleInitialized) { s_surroundPanQuarterAngleInitialized = 1; s_surroundPanQuarterAngle = kSurroundPanQuarterAngleDefault; }
    if (!s_spatialAudioSmallPanAngleInitialized) { s_spatialAudioSmallPanAngleInitialized = 1; s_spatialAudioSmallPanAngle = kFloat_3_1415927 / kSmallPanSectorDivisor; }
    if (!s_surroundPanOuterAngleInitialized) { s_surroundPanOuterAngleInitialized = 1; s_surroundPanOuterAngle = kFloat_6_2831855 / kSurroundOuterSectorDivisor; }
    int mode = gtl_Audio_GetOutputMode();
    // Phase 2: walk the piecewise pan perimeter for stereo or surround. Mono/unknown
    // modes use centered pan; all modes still receive distance/cone attenuation.
    float panX, panDepth;
    if (mode == kAudioStereo) {
        float pi = kFloat_3_1415927, quarter = s_surroundPanQuarterAngle;
        if (azimuth < -pi + quarter)      { panX = Seg803FEC2C(azimuth, -pi, -pi + quarter, 0.0f, -1.0f); panDepth = 1.0f; }
        else if (azimuth < -quarter)      { panX = -1.0f; panDepth = Seg803FEC2C(azimuth, -pi + quarter, -quarter, 1.0f, -1.0f); }
        else if (azimuth < quarter)       { panX = Seg803FEC2C(azimuth, -quarter, quarter, -1.0f, 1.0f); panDepth = -1.0f; }
        else if (azimuth < pi - quarter)  { panX = 1.0f; panDepth = Seg803FEC2C(azimuth, quarter, pi - quarter, -1.0f, 1.0f); }
        else                        { panX = Seg803FEC2C(azimuth, pi - quarter, pi, 1.0f, 0.0f); panDepth = 1.0f; }
        parameters[0] = panX * s_spatialAudioPanRadius * panDistance;
        parameters[1] = panDepth * s_spatialAudioPanRadius * panDistance;
    } else if (mode == kAudioSurround || mode == kAudioSurroundAlternate) {
        float pi = kFloat_3_1415927, half = kFloat_1_5707964_tweenDartsAudio;
        float big = s_surroundPanOuterAngle, small = s_spatialAudioSmallPanAngle;
        if (azimuth < -big)        { panX = Seg803FEC2C(azimuth, -pi, -big, 0.0f, -1.0f); panDepth = 1.0f; }
        else if (azimuth < -half)  { panX = -1.0f; panDepth = Seg803FEC2C(azimuth, -big, -half, 1.0f, 0.0f); }
        else if (azimuth < -small) { panX = -1.0f; panDepth = Seg803FEC2C(azimuth, -half, -small, 0.0f, -1.0f); }
        else if (azimuth < small)  { panX = Seg803FEC2C(azimuth, -small, small, -1.0f, 1.0f); panDepth = -1.0f; }
        else if (azimuth < half)   { panX = 1.0f; panDepth = Seg803FEC2C(azimuth, small, half, -1.0f, 0.0f); }
        else if (azimuth < big)    { panX = 1.0f; panDepth = Seg803FEC2C(azimuth, half, big, 0.0f, 1.0f); }
        else                 { panX = Seg803FEC2C(azimuth, big, pi, 1.0f, 0.0f); panDepth = 1.0f; }
        float outerCosine = Math_CosApprox(s_surroundPanOuterAngle);
        float innerCosine = Math_CosApprox(s_spatialAudioSmallPanAngle);
        float centerWeight = (innerCosine + outerCosine) * kFloat_0_5_SpatialAudio;
        float outerCosineAgain = Math_CosApprox(s_surroundPanOuterAngle);
        parameters[0] = panX * s_spatialAudioPanRadius * panDistance;
        parameters[1] = panDepth * s_spatialAudioPanRadius * panDistance + (centerWeight / (centerWeight + -outerCosineAgain)) * (kFloat_1_SpatialAudio - panDistance);
    } else {
        parameters[0] = parameters[1] = kFloat_0_spatialAudio;
    }
    void* curve = AsSpatialSource(sourceState)->distanceCurve;
    // Phase 3: curve endpoint clamp/interpolation, or inverse-distance rolloff
    // between referenceDistance and maximumDistance; then multiply by cone gain.
    float distance = directionAndDistance[3], distanceGain;
    if (curve && ((Curve*)curve)->count && ((Curve*)curve)->mode) { // 0x803FEC2C.
        CurvePoint* points = ((Curve*)curve)->points;
        unsigned int pointCount = ((Curve*)curve)->count;
        if (distance <= points[0].x) distanceGain=points[0].y;
        else if (distance >= points[pointCount-1].x) distanceGain=points[pointCount-1].y;
        else distanceGain=gtl_AudioCurve_Evaluate((Curve*)curve,distance);
    } else if (distance <= AsSpatialSource(sourceState)->referenceDistance) distanceGain=1.0f;
    else { if (distance > AsSpatialSource(sourceState)->maximumDistance) distance=AsSpatialSource(sourceState)->maximumDistance; distanceGain=AsSpatialSource(sourceState)->referenceDistance/distance; }
    parameters[2] = distanceGain * ((float (*)(const float*, const float*))gtl_Audio_CalculateConeGain)(sourceState, directionAndDistance);
    return 1;
}

// Clamp to curve endpoints, then interpolate the containing linear or Catmull-Rom segment.
float gtl_AudioCurve_Evaluate(Curve* object, float t) {
    unsigned int count = object->count;
    CurvePoint* points = object->points;
    if (count == 1)
        return points[0].y;
    unsigned int last = count - 1;
    if (t <= points[0].x)
        return points[0].y;
    if (points[last].x <= t)
        return points[last].y;
    unsigned int index = 0;
    CurvePoint* cursor = points;
    for (unsigned int i = 0; i < last; ++i, ++cursor) {
        if (cursor->x <= t && t < cursor[1].x) {
            index = i;
            break;
        }
    }
    if (object->mode == kCurveLinear) {
        float x0 = points[index].x;
        float y0 = points[index].y;
        float x1 = points[index + 1].x;
        float y1 = points[index + 1].y;
        return y0 + (t - x0) * (y1 - y0) / (x1 - x0);
    }
    if (object->mode == kCurveCatmullRom) {
        float p0[2], p1[2], p2[2], p3[2], result[2];
        CurvePoint* current = points + index;
        p1[0] = current[0].x; p1[1] = current[0].y;
        p2[0] = current[1].x; p2[1] = current[1].y;
        if (index != 0) {
            p0[0] = current[-1].x; p0[1] = current[-1].y;
        } else {
            gtl_Vec2_Copy(p0, p1);
        }
        if (index + 2 <= last) {
            p3[0] = current[2].x; p3[1] = current[2].y;
        } else {
            gtl_Vec2_Copy(p3, p1);
        }
        float u = (t - current[0].x) / (current[1].x - current[0].x);
        Vec2_CatmullRom(result, p0, p1, p2, p3, u);
        return result[1];
    }
    return kFloat_0_spatialAudio;
}

// Ensure the process-lifetime runtime is initialized, then publish its singleton pointer.
// Defaults: callbacks/bookkeeping zeroed, stereo mode (2), main-send 1,
// twelve listener words copied from kAudioPropertyDefaults, tail words zero.
// Accessors below return borrowed state or forward to the configured callbacks.
static inline void EnsureAudioRuntime() {
    if (!g_gtlAudioManager) {
        if (!s_audioStateInitialized) {
            unsigned int* source = kAudioPropertyDefaults;
            unsigned int a = source[0];
            unsigned int b = source[1];
            unsigned int c = source[2];
            unsigned int d = source[3];
            unsigned int e = source[4];
            unsigned int f = source[5];
            unsigned int g = source[6];
            unsigned int h = source[7];
            unsigned int i = source[8];
            unsigned int j = source[9];
            unsigned int k = source[10];
            unsigned int l = source[11];
            unsigned int* destination = s_gtlAudioRuntime;
            destination[0] = 0;
            destination[6] = 2;
            destination[7] = 0;
            *(float*)(destination + 8) = kFloat_1_GtlAudio;
            destination[9] = a;
            destination[10] = b;
            destination[11] = c;
            destination[12] = d;
            destination[13] = e;
            destination[14] = f;
            destination[15] = g;
            destination[16] = h;
            destination[17] = i;
            destination[18] = j;
            destination[19] = k;
            destination[20] = l;
            destination[21] = 0;
            destination[22] = 0;
            destination[23] = 0;
            memset(destination + 1, 0, 0x14);
            s_audioStateInitialized = 1;
        }
        g_gtlAudioManager = (int)s_gtlAudioRuntime;
    }
}

// Initialize the audio singleton if needed, then request all remote speakers enabled.
void* RemoteAudio_RequestEnableAllLazy() {
    EnsureAudioRuntime();
    return gtl_RemoteAudio_RequestEnableAll();
}

// Initialize the audio singleton if needed, then request all remote speakers disabled.
void* gtl_RemoteAudio_EnsureInitializedAndDisableAll() {
    EnsureAudioRuntime();
    return gtl_RemoteAudio_RequestDisableAll();
}

// Return the shared listener transform after lazy audio initialization.
void* gtl_Audio_GetListener() {
    EnsureAudioRuntime();
    return AsGtlAudioRuntimeState(g_gtlAudioManager)->listener; // gtl_Audio_GetListener, +0x24.
}

// Return the cached main-send scalar after lazy audio initialization.
float GtlAudio_GetMainSend() {
    EnsureAudioRuntime();
    return AsGtlAudioRuntimeState(g_gtlAudioManager)->mainSend; // 0x803FE284.
}

// Return the cached speaker output mode after lazy audio initialization.
int gtl_Audio_GetOutputMode() {
    EnsureAudioRuntime();
    return (int)AsGtlAudioRuntimeState(g_gtlAudioManager)->outputMode;
}

// Returns the allocator callback's result (bctrl at 0x803FE584, r3 kept to the blr); callers test it
// (e.g. 0x803FA77C).
void* gtl_Audio_Alloc(unsigned int size) {
    EnsureAudioRuntime();
    return AsGtlAudioRuntimeState(g_gtlAudioManager)->allocate(size);
}

// Free through the configured audio allocator after ensuring singleton initialization.
void gtl_Audio_Free(void* object) {
    EnsureAudioRuntime();
    AsGtlAudioRuntimeState(g_gtlAudioManager)->free(object);
}

// Cache the secondary mode word; this entry does not reconfigure the audio controller.
int GtlAudio_SetCachedOutputOption(int value) {
    EnsureAudioRuntime();
    GtlAudioRuntimeState* state = AsGtlAudioRuntimeState(g_gtlAudioManager);
    if ((int)state->cachedOutputOption == value)
        return 1;
    state->cachedOutputOption = value;
    return 1;
}

// Validate the public speaker mode, reconfigure the controller, and cache the accepted mode.
int gtl_Audio_SetOutputMode(int value) {
    EnsureAudioRuntime();
    GtlAudioRuntimeState* state = AsGtlAudioRuntimeState(g_gtlAudioManager);
    if ((int)state->outputMode == value)
        return 1;
    switch (value) {
    case kAudioMono: gtl_AudioController_SetOutputMode(gtl_AudioController_GetInstance(), 0); break;
    case kAudioStereo: gtl_AudioController_SetOutputMode(gtl_AudioController_GetInstance(), 1); break;
    case kAudioSurround: gtl_AudioController_SetOutputMode(gtl_AudioController_GetInstance(), 2); break;
    case kAudioSurroundAlternate: gtl_AudioController_SetOutputMode(gtl_AudioController_GetInstance(), 3); break;
    default: return 0;
    }
    state->outputMode = value;
    return 1;
}

// Interpolate gain between the source's inner and outer directional cone angles.
float gtl_Audio_CalculateConeGain(float* object, const float* data) {
    if (AsSpatialSource(object)->innerConeAngle == s_audioOmnidirectionalConeAngle)
        return AsSpatialSource(object)->innerConeGain;
    if (AsSpatialSource(object)->outerConeAngle == kFloat_0_spatialAudio)
        return AsSpatialSource(object)->outerConeGain;
    float direction[3];
    float normal[3];
    Vec3_Scale(direction, data, kFloat_neg1_SpatialAudio);
    Vec3_Normalize(normal, AsSpatialSource(object)->forward);
    float dot = Vec3_Dot(direction, normal);
    float angle;
    if (dot >= kFloat_1_SpatialAudio)
        angle = kFloat_0_spatialAudio;
    else if (dot <= kFloat_neg1_SpatialAudio)
        angle = kFloat_3_1415927;
    else
        angle = (float)acos(dot);
    float firstHalf = AsSpatialSource(object)->innerConeAngle * kFloat_0_5_SpatialAudio;
    float second = AsSpatialSource(object)->outerConeAngle;
    if (angle <= firstHalf)
        return AsSpatialSource(object)->innerConeGain;
    if (second * kFloat_0_5_SpatialAudio <= angle)
        return AsSpatialSource(object)->outerConeGain;
    return AsSpatialSource(object)->innerConeGain + (AsSpatialSource(object)->outerConeGain - AsSpatialSource(object)->innerConeGain) * (angle - firstHalf) / (second * kFloat_0_5_SpatialAudio - firstHalf);
}

} // extern "C"

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU80462688", _ctors_u803FDEC8_0, InitSpatialAudioHelperFullTurnAngle);
STATIC_INIT_ENTRY(".CRT$XCU80462688", _ctors_u803FDEC8_1, SoundBuffer_InitializeConeAngleDefaults);
#pragma pop

// Course wind: speed and direction, the Silvia Cannon that replaces the wind on
// some holes, and the wind the HUD arrow shows.
//
// The rest of CWind (construction, the singleton, and the local wind regions) is
// in wind_regions_and_vectors.cpp.
#include <decomp/local_class.h>
#include "classes/CSceneManager.h"
#include "fp_fused.h"
#include "game/golfball.h"
#include "classes/CourseData801C.h"
#include "classes/CourseBounds801C.h"
#include "classes/CWind.h"
#include "classes/CQuadTree.h"
#include "classes/CSharedDoc.h"
#include "classes/CGolfDoc.h"
#include "game/wvector.h"
#include "classes/CFxSequence.h"
#include "classes/CFx.h"

// Fields of other classes that this file reads. They belong in those classes'
// headers once those exist.
struct PartyItemSlots {
    char pad_00[0x28];
    float forcedWindSpeed; // 0x28
    char pad_2C[0x24];
    void* unitWindOverride;       // 0x50
    void* forcedWindSpeedEnabled; // 0x54
};
struct GolfPlayerEntry {
    char pad_00[0xA0];
    unsigned int windItemId; // 0xA0
};
struct GolfDocWindReference {
    char pad_000[0x33C];
    WVector direction; // 0x33C
};
struct SharedHoleWindSettings {
    char pad_000[0x850];
    unsigned int perHole[18]; // 0x850
};
struct FxAttachment {
    char pad_00[8];
    void* rotation; // 0x08
    char pad_0C[0xC];
    void* offset; // 0x18
    char pad_1C[0xC];
};
// A 3x4 transform: three axes, then a position.
struct AffineMatrix {
    WVector right;
    WVector up;
    WVector forward;
    WVector position;
};
// A vector copied as three raw words, as the original does in a few places.
struct VectorWords {
    unsigned int x, y, z;
};
// The scene's weather effect. Only the virtual call made here is named.
struct WeatherEffect {
    virtual void Slot0() DECOMP_LOCAL_VIRTUAL;
    virtual void Slot1() DECOMP_LOCAL_VIRTUAL;
    virtual void Slot2() DECOMP_LOCAL_VIRTUAL;
    virtual void OnWindChanged() DECOMP_LOCAL_VIRTUAL; // name inferred from the one call below
};

enum {
    kWindItemSlot = 4,
    kRandomWindByte = 255, // passed for strength or direction: pick one at random
    kRandomSpeedCount = 9,
    kTutorialWindEndStage = 6,
    kMinimumCannonSpeed = 6,
    kCannonSpeedMask = 3,
    kUseDocumentWindReference = 0x200, // CGolfDoc::gameStateFlags
    kWindBoundsType = 4,
    kCourseBoundsSize = 0x5C,
    kCourseBoundsCommandOffset = 0x44
};

// Offsets into courseEnvironmentStrings. The text is in the game's data; the
// sound names are Japanese and translated here.
enum {
    kCannonFireSound = 0x00,    // "laser fire"
    kCannonEndSound = 0x0D,     // "laser fire end"
    kCannonChargeSound = 0x1C,  // "energy builds up"
    kCannonSequenceName = 0x2D, // "SilviaCannon.seq"
    kCannonGatherSound = 0x3E,  // "energy gathers"
    kCannonBoundsName = 0x4D,   // "silviacannon"
    kFxCommand = 0x5A,          // "*fx"
    kFxCommandFormat = 0x5E     // "%s %f %f %f"
};

extern "C" {

extern const char courseEnvironmentStrings[];
extern const char s_sceneLighting[];
extern const char s_cameraTransform[];
extern void* g_fx;
extern void* g_sceneManager;
extern void* g_sharedDoc;
extern void* g_soundManager;

// CWind::GetInstance's storage. LoadCourseRegions contains an inlined copy of it.
extern signed char s_windInitialized;
extern const char s_windInstance[];
extern const char s_windDtorRegistration[];

// Float constants. Their values are still linked from the original data
// section, so they are referenced by symbol and given readable names below.
extern const float kFloat_0_SceneEffects;
extern const float kFloat_1_SceneEffects;
extern float kFloat_0_5_SceneEffects;
extern float kFloat_9_SceneEffects;
extern float kFloat_200_SceneEffects;
extern float kFloat_99999_SceneEffects;
extern float kFloat_neg1_SceneWind;
extern float kFloat_0_1_Trajectory;
extern float kFloat_2_Trajectory;
extern float kFloat_0_47123894_WindPhase;
extern float kFloat_3_1415927_Wind;
extern float kNegativePi_Wind;
extern float kFloat_0_017453292_Wind;
extern const float kFloat_6_2831855_wind;
extern const float kFloat_255_WindAndTrajectory;
extern const float kFloat_0_34906584_Wind;
extern const float kWindDirectionByteScale;
extern float kAdditiveWindSpeedThreshold;
extern float s_windIndicatorSmoothingFactor;
// 400.0f and 1500.0f, copied as raw words into a sound's distance range.
extern unsigned int kFloat_400_SceneEffectsSound;
extern unsigned int kFloatBits_1500_SceneEffects;

extern double floor(double);
extern int rand(void);
extern void __register_global_object(void*, void*, void*);

extern void FxAttachment_Init(void*);
extern void Sound_PlayByName(void*, const char*, void*, int, const char*, unsigned int*, float, float, float);
extern void Matrix_SetAxisRotation(void*, float, int);
extern void Matrix_SetIdentityAffine(void*);
extern void Matrix_AppendEulerRotation(void*, void*);
extern void Vec3_Zero(void*);
extern float Vector_SignedAngleXZ(const void*, const void*);
extern int CourseData_FindBoundsByNameSubstring(void*, const char*);
extern void CourseData_UpdateWindActors(void*);
extern void String_ScanTokensAfterSubstring(void*, const char*, const char*, ...);
extern unsigned int ItemInventory_GetItemIdAt(void*, int);
extern void* GolfPlayer_GetEntryByIndex(void*, unsigned char);
extern void* PartyItemSlots_GetInstance(void);
extern void* CQuadTree_GetInstance(void);
extern void* GolfBall__Fv(void);
extern int GolfTask_NoOpStatus(void);
extern "C" void* __dt__5CWindFv(void*, int);
// GetWindVectorAtPoint called as a C function, writing its result straight into the caller's buffer.
extern void GetWindVectorAtPoint__5CWindFRC7WVectorf(void* result, const void* self, const void* point,
                                                     float maxBaseMagnitude);

} // extern "C"

#define kZero kFloat_0_SceneEffects
#define kOne kFloat_1_SceneEffects
#define kHalf kFloat_0_5_SceneEffects
#define kMaxWindSpeed kFloat_9_SceneEffects
#define kTwoPi kFloat_6_2831855_wind
#define kPi kFloat_3_1415927_Wind
#define kNegativePi kNegativePi_Wind
#define kOneDegree kFloat_0_017453292_Wind
#define kWholeCourseRadius kFloat_99999_SceneEffects
#define kCannonSoundVolume kFloat_0_5_SceneEffects
#define kCannonSoundDistance kFloat_200_SceneEffects

static inline CSharedDoc* SharedDoc() {
    return (CSharedDoc*)g_sharedDoc;
}

static inline CGolfDoc* GolfDoc() {
    return (CGolfDoc*)SharedDoc()->golfDoc;
}

// Switch between the cannon's wind and the normal wind, and play the firing sound.
void CWind::SetOverrideEnabled(unsigned int enable) {
    if (overrideActive == enable) {
        return;
    }
    overrideActive = enable;
    if (enable & 0xFF) {
        VectorWords wind = *(VectorWords*)&overrideVector;
        ((CFx*)g_fx)->SetWindVector(&wind);
        if (courseEnvironmentStrings + kCannonFireSound) {
            unsigned int range[2] = {kFloat_400_SceneEffectsSound, kFloatBits_1500_SceneEffects};
            Sound_PlayByName(g_soundManager, courseEnvironmentStrings + kCannonFireSound, &effectPosition, 0,
                             courseEnvironmentStrings + kCannonFireSound, range, kCannonSoundVolume,
                             kCannonSoundDistance, kOne);
        }
    } else {
        VectorWords wind = *(VectorWords*)&windVector;
        ((CFx*)g_fx)->SetWindVector(&wind);
    }
}

// Play the sound that ends the cannon's blast, once per effect.
// SetEffectEnabled clears the flag when it creates a new effect.
void CWind::PlayCannonEndSoundOnce() {
    if (!cannonEndSoundPlayed) {
        const char* strings = courseEnvironmentStrings;
        cannonEndSoundPlayed = 1;
        if (strings + kCannonEndSound) {
            unsigned int range[2];
            range[0] = kFloat_400_SceneEffectsSound;
            range[1] = kFloatBits_1500_SceneEffects;
            Sound_PlayByName(g_soundManager, strings + kCannonEndSound, &effectPosition, 0, strings + kCannonEndSound,
                             range, kCannonSoundVolume, kCannonSoundDistance, kOne);
        }
    }
}

// Play the cannon's charging sound, once per effect.
void CWind::PlayCannonChargeSoundOnce() {
    if (!cannonChargeSoundPlayed) {
        const char* strings = courseEnvironmentStrings;
        cannonChargeSoundPlayed = 1;
        if (strings + kCannonChargeSound) {
            unsigned int range[2];
            range[0] = kFloat_400_SceneEffectsSound;
            range[1] = kFloatBits_1500_SceneEffects;
            Sound_PlayByName(g_soundManager, strings + kCannonChargeSound, &effectPosition, 0,
                             strings + kCannonChargeSound, range, kCannonSoundVolume, kCannonSoundDistance, kOne);
        }
    }
}

// Create or remove the cannon's visual effect.
void CWind::SetEffectEnabled(int enable) {
    if (enable == 0) {
        void* effect = attachedEffect;
        if (effect != 0) {
            ((CFx*)g_fx)->RemoveSequence((unsigned int)(effect), 0);
            attachedEffect = 0;
        }
    } else if (attachedEffect == 0) {
        FxAttachment attachment;
        FxAttachment_Init(&attachment);
        attachment.rotation = effectRotation;
        attachment.offset = &effectOffset;
        void* effect = ((CFx*)g_fx)
                           ->CreateAttachedEffect((Vec2_8010*)(&attachment),
                                                  courseEnvironmentStrings + kCannonSequenceName, 0, 0, 0);
        attachedEffect = effect;
        if (effect) {
            ((CFxSequence*)effect)->InvokeSprayLifecycleHooks(s_sceneLighting);
        }
        if (courseEnvironmentStrings + kCannonGatherSound) {
            unsigned int range[2] = {kFloat_400_SceneEffectsSound, kFloatBits_1500_SceneEffects};
            Sound_PlayByName(g_soundManager, courseEnvironmentStrings + kCannonGatherSound, &effectPosition, 0,
                             courseEnvironmentStrings + kCannonGatherSound, range, kCannonSoundVolume,
                             kCannonSoundDistance, kOne);
        }
        cannonEndSoundPlayed = 0;
        cannonChargeSoundPlayed = 0;
    }
}

// Find the hole's "silviacannon" bounds and set up the cannon from them.
//
// The bounds carry a command of the form "*fx <effect> <x> <y> <z>", where the
// three numbers are rotation angles. The cannon blows along the Y angle, and its
// speed is 6 to 9 depending on the hole's wind setting. A hole without the
// bounds gets a negative speed, which callers read as "no cannon".
#pragma push
#pragma opt_common_subs off
void CWind::InitOverrideFromCourse() {
    CGolfDoc* golfDoc = GolfDoc();
    const char* strings = courseEnvironmentStrings;
    int bounds = CourseData_FindBoundsByNameSubstring(golfDoc->courseData, strings + kCannonBoundsName);
    if (bounds) {
        float angles[3];
        char effectName[0x40];

        int command = bounds + 1;
        String_ScanTokensAfterSubstring(&command, strings + kFxCommand, strings + kFxCommandFormat, effectName, angles,
                                        angles + 1, angles + 2);
        float direction = angles[1] * kFloat_neg1_SceneWind;
        angles[1] = direction;
        overrideDirection = direction;
        unsigned char holeIndex = GolfDoc()->currentHoleIndex;
        unsigned int speedVariant = ((SharedHoleWindSettings*)g_sharedDoc)->perHole[holeIndex] & kCannonSpeedMask;

        overrideSpeed = (float)(kMinimumCannonSpeed + speedVariant);
        Matrix_SetIdentityAffine(effectRotation);
        Matrix_AppendEulerRotation(effectRotation, angles);
        effectPosition =
            (*(WVector*)&((CourseBounds801C*)bounds)->minX + *(WVector*)&((CourseBounds801C*)bounds)->maxX) * kHalf;
        Vec3_Zero(&effectOffset);
        AffineMatrix rotation;
        Matrix_SetAxisRotation(&rotation, angles[1], 1);
        overrideVector = overrideSpeed * rotation.right;
    } else {
        overrideSpeed = kFloat_neg1_SceneWind;
    }
    overrideActive = 0;
    void* effect = attachedEffect;
    if (effect) {
        ((CFx*)g_fx)->RemoveSequence((unsigned int)(effect), 0);
        attachedEffect = 0;
    }
}
#pragma pop

// Set the global wind from two bytes.
//
// With resetWind, the bytes are a speed step (0 to 8, giving speed 1 to 9) and a
// direction in 1/255ths of a turn. Without it they are changes relative to the
// current wind. kRandomWindByte in either picks a random value.
void CWind::SetWindParams(unsigned char strengthByte, unsigned char directionByte, int unused, int resetWind) {
    float twoPi;
    float newDirection;
    float directionStep;
    float angleScale;
    float byteScale;
    if (strengthByte == kRandomWindByte) {
        strengthByte = (unsigned char)(rand() % kRandomSpeedCount);
    }
    if (directionByte == kRandomWindByte) {
        directionByte = (unsigned char)(rand() % kRandomWindByte);
    }
    if (GolfDoc()->tutorialStage < kTutorialWindEndStage) {
        strengthByte = 0;
    }
    if (resetWind) {
        CQuadTree* quadTree = (CQuadTree*)CQuadTree_GetInstance();
        float newSpeed = kOne + (float)strengthByte;
        float initialDirection = (kTwoPi * (float)directionByte) / kFloat_255_WindAndTrajectory;
        quadTree->effectRandomState = directionByte;
        indicatorPhase = kZero;
        modelSetup.indicatorDirection = kZero;
        directionAngle = initialDirection;
        speed = newSpeed;
    } else {
        float clampedSpeed = kOne;
        float adjustedSpeed = speed + (float)(((strengthByte >> 1) & 0x7F) - 2);
        speed = adjustedSpeed;
        if (adjustedSpeed < clampedSpeed) {
            clampedSpeed = kOne;
        } else {
            if (adjustedSpeed > kMaxWindSpeed) {
                clampedSpeed = kMaxWindSpeed;
            } else {
                clampedSpeed = adjustedSpeed;
            }
        }
        directionStep = (float)(directionByte - 0x80);
        byteScale = kWindDirectionByteScale;
        angleScale = kFloat_0_34906584_Wind; // 20 degrees
        angleScale *= directionStep;
        newDirection = FP_FMADDS(angleScale, byteScale, directionAngle);
        twoPi = kTwoPi;
        speed = clampedSpeed;
        directionAngle = newDirection;
        if (newDirection > twoPi) {
            directionAngle = newDirection - twoPi;
        } else if (newDirection < kZero) {
            directionAngle = newDirection + twoPi;
        }
        void* courseData = GolfDoc()->courseData;
        if (courseData != 0) {
            CourseData_UpdateWindActors(courseData);
        }
    }
    AffineMatrix rotation;
    Matrix_SetAxisRotation(&rotation, directionAngle, 1);
    WVector wind = speed * rotation.forward;
    windVector = wind;
}

// Per-frame update for the HUD wind arrow.
//
// Eases the additive wind toward its target, samples the wind at the ball (or at
// the camera while the ball's first event index is negative), advances the
// arrow's animation faster in stronger wind, and turns the arrow toward the
// wind's direction relative to the view by at most pi radians per second.
void CWind::UpdateIndicator(float deltaSeconds) {
    float delta = deltaSeconds;
    float smoothing = s_windIndicatorSmoothingFactor;
    WVector windAtCamera;
    WVector windAtBall;
    WVector difference = additiveWindTarget - additiveWind;
    WVector step = smoothing * difference;
    additiveWind += step;
    CGolfBall* ball = (CGolfBall*)GolfBall__Fv();
    if (ball->m_eventPreDataIndex[0] < 0) {
        GetWindVectorAtPoint__5CWindFRC7WVectorf(&windAtCamera, this, &((AffineMatrix*)s_cameraTransform)->position,
                                                 kWholeCourseRadius);
        indicatorWind = windAtCamera;
    } else {
        ball = (CGolfBall*)GolfBall__Fv();
        GetWindVectorAtPoint__5CWindFRC7WVectorf(&windAtBall, this, ball, kWholeCourseRadius);
        indicatorWind = windAtBall;
    }
    float windSpeed = indicatorWind.Magnitude();
    if (!(windSpeed < kFloat_0_1_Trajectory)) {
        windSpeed = indicatorWind.Magnitude();
        float twoPi = kTwoPi;
        float nextPhase = indicatorPhase + delta * (kFloat_0_47123894_WindPhase *
                                                    (kFloat_2_Trajectory * windSpeed + kFloat_neg1_SceneWind));
        indicatorPhase = nextPhase;
        if (nextPhase > twoPi) {
            indicatorPhase = nextPhase - twoPi;
        }
        CGolfDoc* golfDoc = GolfDoc();
        unsigned int flags = golfDoc->gameStateFlags;
        float target;
        if (flags & kUseDocumentWindReference) {
            target = Vector_SignedAngleXZ(&indicatorWind, &((GolfDocWindReference*)golfDoc)->direction);
        } else {
            target = Vector_SignedAngleXZ(&indicatorWind, &((AffineMatrix*)s_cameraTransform)->forward);
        }
        float turn = target - modelSetup.indicatorDirection;
        float magnitude;
        if (turn > kZero) {
            magnitude = turn;
        } else {
            magnitude = -turn;
        }
        if (magnitude > kTwoPi) {
            turn = turn - kTwoPi * (float)floor(turn / kTwoPi);
        }
        if (turn > kPi) {
            turn -= kTwoPi;
        } else if (turn < kNegativePi) {
            turn += kTwoPi;
        }
        magnitude = turn > kZero ? turn : -turn;
        if (magnitude > kOneDegree) {
            float lowest = kNegativePi * delta;
            float limited;
            if (turn < lowest) {
                limited = lowest;
            } else {
                float highest = kPi * delta;
                if (!(turn > highest)) {
                    limited = turn;
                } else {
                    limited = highest;
                }
            }
            float next = modelSetup.indicatorDirection + limited;
            magnitude = next > kZero ? next : -next;
            if (magnitude > kTwoPi) {
                next = next - kTwoPi * (float)floor(next / kTwoPi);
            }
            if (next > kPi) {
                next -= kTwoPi;
            } else if (next < kNegativePi) {
                next += kTwoPi;
            }
            modelSetup.indicatorDirection = next;
        }
    }
}

// Give every local region its wind for the current global speed. A region's
// strength either scales the global speed or is added to it.
inline void CWind::RescaleRegions() {
    WindRegion* region = regions;
    while (region != regions + regionCount) {
        region->wind.Normalize();
        region->wind *=
            region->strengthMode == WindRegion::kScaleGlobalSpeed ? speed * region->strength : speed + region->strength;
        ++region;
    }
}

// Set the global wind from a vector.
void CWind::SetWindVector(const WVector& wind) {
    WVector zAxis;
    AffineMatrix rotation;
    zAxis.x = kZero;
    zAxis.y = kZero;
    zAxis.z = kOne;
    float angle = Vector_SignedAngleXZ(&wind, &zAxis);
    directionAngle = angle;
    float magnitude = wind.Magnitude();
    speed = magnitude;
    Matrix_SetAxisRotation(&rotation, directionAngle, 1);
    windVector = speed * rotation.forward;
    RescaleRegions();
    WeatherEffect* weather = (WeatherEffect*)((CSceneManager*)g_sceneManager)->weatherEffect;
    if (weather) {
        weather->OnWindChanged();
    }
}

// The wind that acts on the ball: the global wind or the cannon's, plus the
// additive wind. Party items can force its speed or reduce it to a unit vector.
//
// This is CWind::GetWindVector(), which returns a WVector by value. It is
// defined here with the result pointer written out because the original copies
// the result as three raw words, and the member form does not compile to that.
extern "C" void GetWindVector__5CWindFv(VectorWords* result, CWind* self) {
    if (!self->enabled) {
        float zero = kZero;
        float* components = (float*)result;
        components[0] = zero;
        components[1] = zero;
        components[2] = zero;
        return;
    }
    PartyItemSlots* items = (PartyItemSlots*)PartyItemSlots_GetInstance();
    WVector wind;
    if (self->overrideActive) {
        wind = self->overrideVector;
    } else {
        if (items->forcedWindSpeedEnabled) {
            wind = self->windVector;
            wind.Normalize();
            wind *= items->forcedWindSpeed;
        } else {
            wind = self->windVector;
        }
    }
    WVector combined = wind + self->additiveWind;
    wind = combined;
    CGolfDoc* golfDoc = GolfDoc();
    void* inventory = SharedDoc()->itemInventory;
    GolfPlayerEntry* player = (GolfPlayerEntry*)GolfPlayer_GetEntryByIndex(golfDoc, golfDoc->currentPlayer);
    unsigned int windItemId = ItemInventory_GetItemIdAt(inventory, kWindItemSlot);
    if (player->windItemId == windItemId || items->unitWindOverride) {
        wind.Normalize();
    }
    result->x = *(unsigned int*)&wind.x;
    result->y = *(unsigned int*)&wind.y;
    result->z = *(unsigned int*)&wind.z;
}

// The wind speed as one number. Capped at 9 when additive wind is in effect.
#pragma push
#pragma opt_common_subs off
float CWind::GetWindSpeed() const {
    if (!enabled) {
        return kZero;
    }
    PartyItemSlots* items = (PartyItemSlots*)PartyItemSlots_GetInstance();
    CGolfDoc* golfDoc = GolfDoc();
    void* inventory = SharedDoc()->itemInventory;
    unsigned char currentPlayer = golfDoc->currentPlayer;
    GolfPlayerEntry* player = (GolfPlayerEntry*)GolfPlayer_GetEntryByIndex(golfDoc, currentPlayer);
    unsigned int windItemId = ItemInventory_GetItemIdAt(inventory, kWindItemSlot);
    if (player->windItemId == windItemId || items->unitWindOverride) {
        return kOne;
    }
    if (overrideActive) {
        return overrideSpeed;
    }
    float additiveSpeed = additiveWind.Magnitude();
    if (additiveSpeed >= kAdditiveWindSpeedThreshold) {
        WVector wind;

        wind = windVector;
        WVector combined = wind + additiveWind;
        wind = combined;
        float combinedSpeed = wind.Magnitude();
        float capped = combinedSpeed;
        capped = (capped > kMaxWindSpeed) ? kMaxWindSpeed : capped;
        return capped;
    }
    return speed;
}
#pragma pop

// The cannon's direction while it fires, otherwise the global wind's.
float CWind::GetDirectionAngle() {
    if (overrideActive) {
        return overrideDirection;
    }
    return directionAngle;
}

// Change the global wind speed, keeping its direction.
void CWind::SetStrength(float newSpeed) {
    speed = newSpeed;
    AffineMatrix rotation;
    Matrix_SetAxisRotation(&rotation, directionAngle, 1);
    windVector = speed * rotation.forward;
    RescaleRegions();
}

// Start a hole: pick a random global wind and rebuild the region list from the
// course's wind bounds.
void CWind::LoadCourseRegions() {
    if (!GolfTask_NoOpStatus()) {
        // CWind::GetInstance(), inlined: the singleton is built on first use.
        if (!s_windInitialized) {
            ((CWind*)((void*)s_windInstance))->__ct();
            __register_global_object((void*)s_windInstance, (void*)__dt__5CWindFv, (void*)s_windDtorRegistration);
            s_windInitialized = 1;
        }
        ((CWind*)s_windInstance)->SetWindParams(kRandomWindByte, kRandomWindByte, 0, 1);
    }
    // Empty the region list. The count is read back through a volatile pointer
    // because the original stores it to the stack and reloads it here.
    int count = regionCount;
    int offset, index;
    index = 0;
    offset = 0;
    regionCount = count - *(volatile int*)&count;
    for (; index < ((CourseData801C*)GolfDoc()->courseData)->boundsCount; offset += kCourseBoundsSize, ++index) {
        char* bounds = (char*)((CourseData801C*)GolfDoc()->courseData)->bounds + offset;
        if ((unsigned char)*bounds == kWindBoundsType) {
            AddRegionFromCommand(bounds + kCourseBoundsCommandOffset, bounds + 1);
        }
    }
}

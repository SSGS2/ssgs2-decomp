#include <decomp/local_class.h>
#include "classes/CSceneManager.h"
#include "fp_fused.h"
#include "game/golfball.h"
#include "classes/CourseData801C.h"
#include "classes/CourseBounds801C.h"
#include "classes/CWind.h"
#define AsCWind(p) ((CWind*)(p))
#include "classes/CQuadTree.h"
#define AsCQuadTree(p) ((CQuadTree*)(p))
#include "classes/CSharedDoc.h"
#define AsCSharedDoc(p) ((CSharedDoc*)(p))
#include "classes/CGolfDoc.h"
#define AsCGolfDoc(p) ((CGolfDoc*)(p))
#include "game/wvector.h"
// Translation unit 0x80212950-0x80213768 (13 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

// Narrow views for fields absent from the shared class headers.
struct WindAssistState {
    char reserved0[0x28];
    float forcedSpeed;
    char reserved2C[0x24];
    void* unitWindOverride;
    void* forcedSpeedEnabled;
};
struct WindPlayerItemState {
    char reserved[0xA0];
    unsigned int windItemId;
};
struct SharedHoleWindSettings {
    char reserved[0x850];
    unsigned int perHole[18];
};
struct WindRegion {
    char boundsAndFlags[0x1C];
    WVector wind;
    unsigned char strengthMode;
    char reserved29[3];
    float strength;
    char reserved30[0xC];
};
struct WindVectorWords {
    unsigned int x, y, z;
};
struct DocumentWindReference {
    char reserved[0x33C];
    float direction[3];
};
struct CameraAffineAxes {
    float right[3], up[3], forward[3], position[3];
};
typedef char WindRegion_size_check[(sizeof(WindRegion) == 0x3C) ? 1 : -1];
typedef char WindAssist_speed_flag_check[((int)&((WindAssistState*)0)->forcedSpeedEnabled == 0x54) ? 1 : -1];
enum WindRegionLayout {
    kRegionWindOffset = (int)&((WindRegion*)0)->wind,
    kDocumentWindDirectionOffset = (int)&((DocumentWindReference*)0)->direction,
};
enum WindRules {
    kWindItemSlot = 4,
    kRelativeRegionStrength = 1,
    kRandomWindByte = 255,
    kRandomSpeedCount = 9,
    kTutorialWindEndStage = 6,
    kMinimumCannonSpeed = 6,
    kCannonSpeedMask = 3,
    kUseDocumentWindReference = 0x200,
    kWindBoundsType = 4,
};
enum WindStringOffset {
    kCannonEndSound = 0xD,
    kCannonChargeSound = 0x1C,
    kCannonSequence = 0x2D,
    kCannonGatherSound = 0x3E,
    kCannonBoundsName = 0x4D,
    kFxCommand = 0x5A,
    kFxCommandFormat = 0x5E,
};

extern "C" {

struct WindOverrideView {
    char baseStorage[0x2C];
    float directionAngle;
    char indicatorStorage[0x88];
    float overrideDirection;
    unsigned char overrideEnabled;
    char effectBasisStorage[0xE4 - 0xBD];
    char effectPositionAnchor[1];
};

extern const char courseEnvironmentStrings[];
extern unsigned int kFloat_400_SceneEffectsSound;
extern unsigned int kFloatBits_1500_SceneEffects;
extern void* g_soundManager;
extern float kFloat_0_5_SceneEffects;
extern float kFloat_200_SceneEffects;
extern const float kFloat_1_SceneEffects;
extern const float kFloat_0_SceneEffects;
extern float kFloat_neg1_SceneWind;
extern const float kFloat_6_2831855_wind, kFloat_255_WindAndTrajectory, kFloat_0_34906584_Wind, kWindDirectionByteScale;
extern float kAdditiveWindSpeedThreshold;
extern float s_windIndicatorSmoothingFactor;
extern float kFloat_9_SceneEffects;
extern void* g_sceneManager;
extern void* g_sharedDoc;
extern const char s_sceneLighting[];
extern const char s_cameraTransform[];
extern void* g_fx;
struct WindIndicatorView {
    char p0[4];
    char targetVector[12];
    char smoothedVector[12];
    char sampledWind[20];
    float phase;
    char p34[0x4c];
    float displayDirection;
};
extern float kFloat_99999_SceneEffects, kFloat_0_1_Trajectory, kFloat_0_47123894_WindPhase, kFloat_2_Trajectory, kFloat_3_1415927_Wind, kNegativePi_Wind, kFloat_0_017453292_Wind;
extern double floor(double);
extern void CFx_RemoveSequence(void*, void*, int);
extern void FxAttachment_Init(void*);
extern void* CFx_CreateAttachedEffect(void*, void*, const char*, int, int, int);
extern void CFxSequence_InvokeSprayLifecycleHooks(void*, const char*);
extern unsigned int ItemInventory_GetItemIdAt(void*, int);
extern void* GolfPlayer_GetEntryByIndex(void*, unsigned char);
extern void* PartyItemSlots_GetInstance(void);
extern float Vector_SignedAngleXZ(void*, void*);
extern float Magnitude__7WVectorCFv(void*);
extern void __ml__FfRC7WVector(void*, void*, float);
extern void __pl__FRC7WVectorRC7WVector(void*, void*, void*);
extern void Sound_PlayByName(void*, const char*, void*, int, const char*, unsigned int*, float, float, float);
extern void Matrix_SetAxisRotation(void*, float, int);
extern void __ml__FfRC7WVector(void*, void*, float);
extern void __as__7WVectorFRC7WVector(void*, void*);
extern void __apl__7WVectorFRC7WVector(void*, void*);
extern void __mi__FRC7WVectorRC7WVector(void*, void*, void*);
extern void Normalize__7WVectorFv(void*);
extern void __amu__7WVectorFf(void*, float);
extern void Matrix_SetIdentityAffine(void*);
extern void Matrix_AppendEulerRotation(void*, void*);
extern void __ml__FRC7WVectorf(void*, void*, float);
extern void Vec3_Zero(void*);
extern int CourseData_FindBoundsByNameSubstring(void*, const char*);
extern void String_ScanTokensAfterSubstring(void*, const char*, const char*, ...);
extern void CFx_SetWindVector(void*, void*);
extern int GolfTask_NoOpStatus(void);
extern void CWind_Ctor(void*);
extern void CWind_dtor(void);
extern void __register_global_object(void*, void*, void*);
extern void CWind_SetWindParams(void*, unsigned char, unsigned char, int, int);
extern void CWind_AddRegionFromCommand(void*, void*, void*);
extern int rand(void);
extern void* CQuadTree_GetInstance(void);
extern void CourseData_UpdateWindActors(void*);
extern float kFloat_99999_SceneEffects, kFloat_0_1_Trajectory, kFloat_0_47123894_WindPhase, kFloat_2_Trajectory, kFloat_3_1415927_Wind, kNegativePi_Wind, kFloat_0_017453292_Wind;
extern double floor(double);
extern void* GolfBall__Fv(void);
extern void CWind_GetWindVectorAtPoint(void*, void*, void*, float);
extern void* g_fx;
extern void* g_sharedDoc;
extern signed char s_windInitialized;
extern const char s_windInstance[];
extern const char s_windDtorRegistration[];

struct WindRegionOwner {
    char pad0[0x28]; float speed; float directionAngle;
    char pad30[4]; char pad34[0x5c]; void* entries; int count;
};

struct WindParametersView {
    char pad00[0x28];
    float speed;
    float directionAngle;
    char pad30[0x84];
    float overrideSpeed;
    float overrideDirection;
    unsigned char overrideEnabled;
    char padBD[3];
    char matrixC0[0x3C];
    char padFC[4];
};

// Create or remove the Silvia Cannon effect and reset its one-shot sound guards.
void CWind_SetEffectEnabled(WindParametersView* self, int enable) {
    if (enable == 0) {
        void* old = AsCWind(self)->attachedEffect;
        if (old != 0) {
            CFx_RemoveSequence(g_fx, old, 0);
            AsCWind(self)->attachedEffect = 0;
        }
    } else if (AsCWind(self)->attachedEffect == 0) {
        char args[0x28];
        FxAttachment_Init(args);
        *(void**)(args + 8) = ((char*)AsCWind(self)->effectMatrix);
        *(void**)(args + 0x18) = ((char*)AsCWind(self)->effectOffset);
        void* result = CFx_CreateAttachedEffect(g_fx, args, courseEnvironmentStrings + kCannonSequence, 0, 0, 0);
        AsCWind(self)->attachedEffect = result;
        if (result) CFxSequence_InvokeSprayLifecycleHooks(result, s_sceneLighting);
        if (courseEnvironmentStrings + kCannonGatherSound) {
            unsigned int xy[2] = {kFloat_400_SceneEffectsSound, kFloatBits_1500_SceneEffects};
            Sound_PlayByName(g_soundManager, courseEnvironmentStrings + kCannonGatherSound, ((char*)AsCWind(self)->effectPosition), 0, courseEnvironmentStrings + kCannonGatherSound, xy,
                        kFloat_0_5_SceneEffects, kFloat_200_SceneEffects, kFloat_1_SceneEffects);
        }
        AsCWind(self)->cannonEndSoundPlayed = 0;
        AsCWind(self)->cannonChargeSoundPlayed = 0;
    }
}

// Combine base or cannon wind with additive wind and apply item/assist normalization.
void CWind_GetWindVector(float* out, WindParametersView* self) {
    if (!AsCWind(self)->enabled) {
        float zero = kFloat_0_SceneEffects;
        out[0] = zero;
        out[1] = zero;
        out[2] = zero;
        return;
    }
    void* assistState = PartyItemSlots_GetInstance();
    char baseWindStorage[12];
    if (AsCWind(self)->overrideActive) {
        __as__7WVectorFRC7WVector(baseWindStorage, ((char*)AsCWind(self)->overrideVector));
    } else {
        if (((WindAssistState*)assistState)->forcedSpeedEnabled) {
            __as__7WVectorFRC7WVector(baseWindStorage, ((char*)AsCWind(self)->windVector));
            Normalize__7WVectorFv(baseWindStorage);
            __amu__7WVectorFf(baseWindStorage, ((WindAssistState*)assistState)->forcedSpeed);
        } else __as__7WVectorFRC7WVector(baseWindStorage, ((char*)AsCWind(self)->windVector));
    }
    char combinedWindStorage[12];
    __pl__FRC7WVectorRC7WVector(combinedWindStorage, baseWindStorage, ((char*)&AsCWind(self)->additiveWindXBits));
    __as__7WVectorFRC7WVector(baseWindStorage, combinedWindStorage);
    void* golfDocument = (*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc);
    void* itemInventory = ((char*)&AsCSharedDoc(g_sharedDoc)->itemInventory);
    void* playerState = GolfPlayer_GetEntryByIndex(golfDocument, AsCGolfDoc(golfDocument)->currentPlayer);
    unsigned int windItemId = ItemInventory_GetItemIdAt(itemInventory, kWindItemSlot);
    if (((WindPlayerItemState*)playerState)->windItemId == windItemId || ((WindAssistState*)assistState)->unitWindOverride) Normalize__7WVectorFv(baseWindStorage);
    *(unsigned int*)out = *(unsigned int*)baseWindStorage;
    *(unsigned int*)((char*)out + 4) = *(unsigned int*)(baseWindStorage + 4);
    *(unsigned int*)((char*)out + 8) = *(unsigned int*)(baseWindStorage + 8);
}

// Fx command supplies an effect-name token and three rotation components.
// Heading is negated for the wind convention; matrix calls consume radians.
// Bounds min/max and their midpoint are world XYZ. Vector helper scratch is
// raw 12-byte storage so copies retain the engine vector ABI and instruction order.
// Read the cannon region command, position its effect, and calculate override speed and direction.
#pragma push
#pragma opt_common_subs off
void CWind_InitOverrideFromCourse(WindParametersView* self) {
    void* game = (*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc);
    const char* base = courseEnvironmentStrings;
    int boundsAddress = CourseData_FindBoundsByNameSubstring(AsCGolfDoc(game)->courseData, base + kCannonBoundsName);
    if (boundsAddress) { float angles[3];
        char parsedEffectName[0x40];

        int commandAddress = boundsAddress + 1; String_ScanTokensAfterSubstring(&commandAddress, base + kFxCommand, base + kFxCommandFormat, parsedEffectName, angles, angles + 1, angles + 2);
        float negatedHeading = (angles[1] * kFloat_neg1_SceneWind);
        angles[1] = negatedHeading;
        self->overrideDirection = negatedHeading;
        void* data = (*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc);
        unsigned char holeIndex = AsCGolfDoc(data)->currentHoleIndex;
        unsigned int speedVariant = ((SharedHoleWindSettings*)g_sharedDoc)->perHole[holeIndex] & kCannonSpeedMask;

        AsCWind(self)->overrideSpeed = (float)(kMinimumCannonSpeed + speedVariant);
        Matrix_SetIdentityAffine(((char*)AsCWind(self)->effectMatrix));
        Matrix_AppendEulerRotation(((char*)AsCWind(self)->effectMatrix), angles);
        char boundsCenter[12];
        char boundsCornerSum[12]; __pl__FRC7WVectorRC7WVector(boundsCornerSum, ((char*)&AsCourseBounds801C(boundsAddress)->minX), ((char*)&AsCourseBounds801C(boundsAddress)->maxX));
        __ml__FRC7WVectorf(boundsCenter, boundsCornerSum, kFloat_0_5_SceneEffects);
        char overrideVectorStorage[12];
        __as__7WVectorFRC7WVector(((char*)AsCWind(self)->effectPosition), boundsCenter);
         Vec3_Zero(((char*)AsCWind(self)->effectOffset));
        char headingRotationMatrix[0x30]; Matrix_SetAxisRotation(headingRotationMatrix, angles[1], 1);
        __ml__FfRC7WVector(overrideVectorStorage, headingRotationMatrix, AsCWind(self)->overrideSpeed);
        __as__7WVectorFRC7WVector(((char*)AsCWind(self)->overrideVector), overrideVectorStorage);
    } else AsCWind(self)->overrideSpeed = kFloat_neg1_SceneWind;
    AsCWind(self)->overrideActive = 0;
    void* old = AsCWind(self)->attachedEffect;
    if (old) {
        CFx_RemoveSequence(g_fx, old, 0);
        AsCWind(self)->attachedEffect = 0;
    }
}
#pragma pop

// Derive speed and heading from the supplied vector and rescale every local wind region.
void CWind_SetWindVector(WindRegionOwner* self, void* source) {
    // The two adjacent 24-byte arrays are one affine rotation matrix.
    // Its forward basis occupies the second half; scaling it reconstructs wind XYZ.
    struct Locals { char y[12]; float vec[3]; char x[24]; char z[24]; } buf;
    buf.vec[0] = kFloat_0_SceneEffects;
    buf.vec[1] = kFloat_0_SceneEffects;
    buf.vec[2] = kFloat_1_SceneEffects;
    float a = Vector_SignedAngleXZ(source, buf.vec);
    self->directionAngle = a;
    float b = (*(WVector*)source).Magnitude();
    self->speed = b;
    Matrix_SetAxisRotation(buf.x, self->directionAngle, 1);
    __ml__FfRC7WVector(buf.y, buf.z, self->speed);
    *(WVector*)(((char*)AsCWind(self)->windVector)) = *(WVector*)buf.y;
    char* p = (char*)self->entries;
    while (p != (char*)self->entries + self->count * sizeof(WindRegion)) {
        (*(WVector*)(p + kRegionWindOffset)).Normalize();
        *(WVector*)(p + kRegionWindOffset) *= ((WindRegion*)p)->strengthMode == kRelativeRegionStrength
            ? self->speed * ((WindRegion*)p)->strength
            : self->speed + ((WindRegion*)p)->strength;
        p += sizeof(WindRegion);
    }
    struct VirtualObject {
        virtual void v0() DECOMP_LOCAL_VIRTUAL; virtual void v4() DECOMP_LOCAL_VIRTUAL; virtual void v8() DECOMP_LOCAL_VIRTUAL;
        virtual void v14() DECOMP_LOCAL_VIRTUAL;
    };
    VirtualObject* obj = ((VirtualObject*)((CSceneManager*)g_sceneManager)->weatherEffect);
    if (obj) {
        obj->v14();
    }
}

// Return disabled, item-adjusted, cannon, or capped combined wind strength.
#pragma push
#pragma opt_common_subs off
float CWind_GetWindSpeed(const CWind* wind) {
    const WindParametersView* self = (const WindParametersView*)wind;
    if (!wind->enabled) return kFloat_0_SceneEffects;
    void* manager = PartyItemSlots_GetInstance();
    void* game = (*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc);
    void* base = ((char*)&AsCSharedDoc(g_sharedDoc)->itemInventory);
    unsigned char mode = AsCGolfDoc(game)->currentPlayer;
    void* state = GolfPlayer_GetEntryByIndex(game, mode);
    unsigned int resource = ItemInventory_GetItemIdAt(base, kWindItemSlot);
    if (((WindPlayerItemState*)state)->windItemId == resource || ((WindAssistState*)manager)->unitWindOverride) return kFloat_1_SceneEffects;
    if (AsCWind(self)->overrideActive) return AsCWind(self)->overrideSpeed;
    float speed = ((const WVector*)&wind->additiveWindXBits)->Magnitude();
    if (speed >= kAdditiveWindSpeedThreshold) {
        WVector a;

        a = *(WVector*)(((char*)AsCWind(self)->windVector));
        WVector b = a + *(const WVector*)&wind->additiveWindXBits;
        a = b;
        float d = a.Magnitude();
        float capped = d;
        capped = (capped > kFloat_9_SceneEffects) ? kFloat_9_SceneEffects : capped;
        return capped;
    }
    return self->speed;
}
#pragma pop

// Update the global wind strength and rescale every local wind region.
void CWind_SetStrength(WindRegionOwner* self, float speed) {
    self->speed = speed;
    char buf[0x40];
    Matrix_SetAxisRotation(buf + 0x10, self->directionAngle, 1);
    __ml__FfRC7WVector(buf, buf + 0x28, self->speed);
    *(WVector*)(((char*)AsCWind(self)->windVector)) = *(WVector*)buf;
    char* p = (char*)self->entries;
    while (p != (char*)self->entries + self->count * sizeof(WindRegion)) {
        (*(WVector*)(p + kRegionWindOffset)).Normalize();
        *(WVector*)(p + kRegionWindOffset) *= ((WindRegion*)p)->strengthMode == kRelativeRegionStrength
            ? self->speed * ((WindRegion*)p)->strength
            : self->speed + ((WindRegion*)p)->strength;
        p += sizeof(WindRegion);
    }
}

// Switch the effect system to cannon or ordinary wind and play the cannon firing sound.
void CWind_SetOverrideEnabled(WindOverrideView* self, unsigned int enable) {
    if (self->overrideEnabled == enable) return;
    self->overrideEnabled = enable;
    struct VectorCopy { unsigned int x, y, z; };
    if (enable & 0xFF) {
        VectorCopy a = *(VectorCopy*)(((char*)AsCWind(self)->overrideVector));
        CFx_SetWindVector(g_fx, &a);
        if (courseEnvironmentStrings) {
            unsigned int args[2] = {kFloat_400_SceneEffectsSound, kFloatBits_1500_SceneEffects};
            Sound_PlayByName(g_soundManager, courseEnvironmentStrings, ((char*)AsCWind(self)->effectPosition), 0,
                courseEnvironmentStrings, args, kFloat_0_5_SceneEffects, kFloat_200_SceneEffects, kFloat_1_SceneEffects);
        }
    } else {
        VectorCopy a = *(VectorCopy*)(((char*)AsCWind(self)->windVector));
        CFx_SetWindVector(g_fx, &a);
    }
}

// Reset wind-region storage and load type-4 bounds from the current course.
void CWind_LoadCourseRegions(WindRegionOwner* self) {
    if (!GolfTask_NoOpStatus()) {
        if (!s_windInitialized) {
            CWind_Ctor((void*)s_windInstance);
            __register_global_object((void*)s_windInstance, (void*)CWind_dtor, (void*)s_windDtorRegistration);
            s_windInitialized = 1;
        }
        CWind_SetWindParams((void*)s_windInstance, 255, 255, 0, 1);
    }
    int c = self->count;
    int offset, index;
    index = 0;
    offset = 0;
    self->count = c - *(volatile int*)&c;
    for (; index < AsCourseData801C(AsCGolfDoc((*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc))->courseData)->boundsCount; offset += 0x5c, ++index) {
        char* item = ((char*)AsCourseData801C(AsCGolfDoc((*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc))->courseData)->bounds) + offset;
        if ((unsigned char)*item == kWindBoundsType) CWind_AddRegionFromCommand(self, item + 0x44, item + 1);
    }
}

// Return the cannon heading while override is active, otherwise the ordinary wind heading.
float CWind_GetDirectionAngle(WindOverrideView* self) {
    if (self->overrideEnabled) {
        return self->overrideDirection;
    }
    return self->directionAngle;
}

struct WindStrengthView {
    char pad00[0x28];
    float speed;
    float directionAngle;
    float phase;
    char pad34[0x4C];
    float displayDirection;
};

// Initialize or perturb wind speed and direction, then rebuild the wind vector.
void CWind_SetWindParams(void* self, unsigned char strengthByte, unsigned char directionByte, int unused, int resetWind) {
    float twopi;
    float adjustedDirection;
    float direction;
    float angleScale;
    float byteScale;
    WindStrengthView* s = (WindStrengthView*)self;
    if (strengthByte == kRandomWindByte) strengthByte = (unsigned char)(rand() % kRandomSpeedCount);
    if (directionByte == kRandomWindByte) directionByte = (unsigned char)(rand() % kRandomWindByte);
    if (AsCGolfDoc((*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc))->tutorialStage < kTutorialWindEndStage) strengthByte = 0;
    if (resetWind) {
        void* game = CQuadTree_GetInstance();
        float speed = kFloat_1_SceneEffects + (float)strengthByte;
        float initialDirection = (kFloat_6_2831855_wind * (float)directionByte) / kFloat_255_WindAndTrajectory;
        AsCQuadTree(game)->effectRandomState = directionByte;
        s->phase = kFloat_0_SceneEffects;
        s->displayDirection = kFloat_0_SceneEffects;
        s->directionAngle = initialDirection;
        s->speed = speed;
    } else {
        float clampedSpeed = kFloat_1_SceneEffects;
        float adjustedSpeed = s->speed + (float)(((strengthByte >> 1) & 0x7f) - 2);
        s->speed = adjustedSpeed;
        if (adjustedSpeed < clampedSpeed) {
            clampedSpeed = kFloat_1_SceneEffects;
        } else {
            if (adjustedSpeed > kFloat_9_SceneEffects) {
                clampedSpeed = kFloat_9_SceneEffects;
            } else {
                clampedSpeed = adjustedSpeed;
            }
        }
        direction = (float)(directionByte - 0x80);
        byteScale = kWindDirectionByteScale;
        angleScale = kFloat_0_34906584_Wind;
        angleScale *= direction;
        adjustedDirection = FP_FMADDS(angleScale, byteScale, s->directionAngle);
        twopi = kFloat_6_2831855_wind;
        s->speed = clampedSpeed;
        s->directionAngle = adjustedDirection;
        if (adjustedDirection > twopi) {
            s->directionAngle = adjustedDirection - twopi;
        } else if (adjustedDirection < kFloat_0_SceneEffects) {
            s->directionAngle = adjustedDirection + twopi;
        }
        void* effects = AsCGolfDoc((*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc))->courseData;
        if (effects != 0) {
            CourseData_UpdateWindActors(effects);
        }
    }
    float matrix[12];
    char bufB[16];
    Matrix_SetAxisRotation(matrix, s->directionAngle, 1);
    __ml__FfRC7WVector(bufB, matrix + 6, s->speed);
    *(WVector*)(((char*)AsCWind(s)->windVector)) = *(WVector*)bufB;
}

// Smooth the indicator vector, sample local wind, and ease its phase and heading toward the reference.
void WindIndicator_UpdateDirectionAndPhase(WindIndicatorView* s, float amount) {
    void* self = s;
    float saved = amount;
    float scale = s_windIndicatorSmoothingFactor;
    char cameraWind[12], ballWind[12], tmp[12], out[12];
    __mi__FRC7WVectorRC7WVector(tmp, s->targetVector, s->smoothedVector);
    __ml__FfRC7WVector(out, tmp, scale);
    __apl__7WVectorFRC7WVector(s->smoothedVector, out);
    void* state = GolfBall__Fv();
    if (((CGolfBall*)state)->m_eventPreDataIndex[0] < 0) {

        CWind_GetWindVectorAtPoint(cameraWind, self, (void*)((CameraAffineAxes*)s_cameraTransform)->position, kFloat_99999_SceneEffects);
        __as__7WVectorFRC7WVector(s->sampledWind, cameraWind);
    } else {
        state = GolfBall__Fv();

        CWind_GetWindVectorAtPoint(ballWind, self, state, kFloat_99999_SceneEffects);
        __as__7WVectorFRC7WVector(s->sampledWind, ballWind);
    }
    float windSpeed = Magnitude__7WVectorCFv(s->sampledWind);
    if (!(windSpeed < kFloat_0_1_Trajectory)) {
        windSpeed = Magnitude__7WVectorCFv(s->sampledWind);
        float twopi = kFloat_6_2831855_wind;
        float nextPhase = s->phase + saved * (kFloat_0_47123894_WindPhase * (kFloat_2_Trajectory * windSpeed + kFloat_neg1_SceneWind));
        s->phase = nextPhase;
        if (nextPhase > twopi) s->phase = nextPhase - twopi;
        void* game = (*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc);
        void* manager = (char*)game;
        unsigned int flags = (*(unsigned int*)&AsCGolfDoc(manager)->gameStateFlags);
        float reference;
        if (flags & kUseDocumentWindReference) reference = Vector_SignedAngleXZ(s->sampledWind, (char*)manager + kDocumentWindDirectionOffset);
        else reference = Vector_SignedAngleXZ(s->sampledWind, (void*)((CameraAffineAxes*)s_cameraTransform)->forward);
        float diff = reference - s->displayDirection;
        float absoluteAngle;
        if (diff > kFloat_0_SceneEffects) absoluteAngle = diff;
        else absoluteAngle = -diff;
        if (absoluteAngle > kFloat_6_2831855_wind) {
            diff = diff - kFloat_6_2831855_wind * (float)floor(diff / kFloat_6_2831855_wind);
        }
        if (diff > kFloat_3_1415927_Wind) diff -= kFloat_6_2831855_wind;
        else if (diff < kNegativePi_Wind) diff += kFloat_6_2831855_wind;
        absoluteAngle = diff > kFloat_0_SceneEffects ? diff : -diff;
        if (absoluteAngle > kFloat_0_017453292_Wind) {
            float lo = kNegativePi_Wind * saved;
            float step;
            if (diff < lo) step = lo;
            else {
                float hi = kFloat_3_1415927_Wind * saved;
                if (!(diff > hi)) step = diff;
                else step = hi;
            }
            float next = s->displayDirection + step;
            absoluteAngle = next > kFloat_0_SceneEffects ? next : -next;
            if (absoluteAngle > kFloat_6_2831855_wind) {
                next = next - kFloat_6_2831855_wind * (float)floor(next / kFloat_6_2831855_wind);
            }
            if (next > kFloat_3_1415927_Wind) next -= kFloat_6_2831855_wind;
            else if (next < kNegativePi_Wind) next += kFloat_6_2831855_wind;
            s->displayDirection = next;
        }
    }
}

// The two sound guards are independent; no required call order is established.
// This guard is reset by CWind_SetEffectEnabled when a new effect is created.
// Play the cannon end sound once per enabled effect cycle.
void CWind_PlayCannonEndSoundOnce(WindOverrideView* self) {
    if (!AsCWind(self)->cannonEndSoundPlayed) {
        const char* base = courseEnvironmentStrings;
        AsCWind(self)->cannonEndSoundPlayed = 1;
        if (base + kCannonEndSound) {
            unsigned int args[2];
            args[0] = kFloat_400_SceneEffectsSound;
            args[1] = kFloatBits_1500_SceneEffects;
            Sound_PlayByName(g_soundManager, base + kCannonEndSound, ((char*)AsCWind(self)->effectPosition), 0, base + kCannonEndSound,
                        args, kFloat_0_5_SceneEffects, kFloat_200_SceneEffects, kFloat_1_SceneEffects);
        }
    }
}

// Independently guarded charge sound; reset when a new cannon effect is created.
// Play the cannon charge sound once per enabled effect cycle.
void CWind_PlayCannonChargeSoundOnce(WindOverrideView* self) {
    if (!AsCWind(self)->cannonChargeSoundPlayed) {
        const char* base = courseEnvironmentStrings;
        AsCWind(self)->cannonChargeSoundPlayed = 1;
        if (base + kCannonChargeSound) {
            unsigned int args[2];
            args[0] = kFloat_400_SceneEffectsSound;
            args[1] = kFloatBits_1500_SceneEffects;
            Sound_PlayByName(g_soundManager, base + kCannonChargeSound, ((char*)AsCWind(self)->effectPosition), 0, base + kCannonChargeSound,
                        args, kFloat_0_5_SceneEffects, kFloat_200_SceneEffects, kFloat_1_SceneEffects);
        }
    }
}

} // extern "C"

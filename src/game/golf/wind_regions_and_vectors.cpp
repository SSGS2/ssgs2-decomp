// The second half of CWind: queries over the local wind regions (is a point in
// one, what is the wind there), parsing a region from a course command, and
// drawing the HUD wind arrow. Construction and the singleton are in
// wind_indicator_wavelet_cloud_overlay.cpp; the rest of the class is in
// wind_system.cpp.
#include <decomp/static_init.h>
#include <decomp/offset_of.h>
#include <decomp/local_class.h>
#include "classes/CWind.h"
#include "iff/IffStruct.h"
#include "classes/CGolfDoc.h"
#include "classes/WView.h"
#include "classes/CSharedDoc.h"

#include "sdk_protos.h"
#include "game/wvector.h"

// Return caller-provided storage for copy construction; no allocation occurs.
inline void* operator new(size_t, void* p) {
    return p;
}

// Fields of other classes that this file reads. They belong in those classes'
// headers once those exist.
struct ShotStatOverrides {
    char pad_00[0x28];
    float windMagnitude; // 0x28
    char pad_2C[0x24];
    int normalizeWind;         // 0x50
    int overrideWindMagnitude; // 0x54
};
struct RoundPlayerStats {
    char pad_00[0xA0];
    unsigned int equippedItemId; // 0xA0
};

extern "C" {

extern const float kFloat_6_2831855;
extern float s_windAngularPeriod;

extern void* g_sharedDoc;
extern WView* g_view;
extern const float kFloat_0_SceneEffects;
extern const float kFloat_180_ScoreCard;
extern const float kFloat_0_017453292_Wind;
extern const float kFloat_1_SceneEffects;
extern const float kFloat_neg0_5235988_Trajectory;
extern const float kFloat_neg170_WindAndTrajectory;
extern const float kFloat_640_TrajectoryViewport;
extern const float kFloat_0_5_SceneEffects;
extern const float kFloat_0_1_Trajectory;
extern const float kFloat_2_Trajectory;
extern const float kWindIndicatorFovAspectScale;
extern const float kWindIndicatorFieldOfView;
extern const float kFloat_neg1_5707964_SceneEffects;
extern const float kWindIndicatorVerticalDirectionThreshold;
extern const float kWindIndicatorPositiveVerticalPitch;

// The render state lists the arrow is drawn with: nine render states, six
// texture-stage states and five sampler parameters, two words each (the lists
// are copied to the stack and installed).
struct RenderStateList {
    int data[18];
};
struct TextureStateList {
    int data[12];
};
struct SamplerStateList {
    int data[10];
};
struct IndicatorRenderStates {
    RenderStateList render;
    TextureStateList texture;
    SamplerStateList sampler;
};
extern IndicatorRenderStates s_windIndicatorRenderStateTables;
extern char s_cameraTransform[];
extern const char courseEnvironmentStrings[];

extern char __vt__Q23std11logic_error[];
extern char s_sharedCharDeleterVtable[];
extern char __vt__Q23std12length_error[];
// Separate byte symbols supply the parser's initial eight-byte course-variant token.
extern unsigned char kWindDefaultCourseVariantByte0, kWindDefaultCourseVariantByte1, kWindDefaultCourseVariantByte2,
    kWindDefaultCourseVariantByte3, kWindDefaultCourseVariantByte4, kWindDefaultCourseVariantByte5,
    kWindDefaultCourseVariantByte6, kWindDefaultCourseVariantByte7;

void appFreeUni__FPv(void*);
void __dl__FPv(void*);

void* PartyItemSlots_GetInstance(void);
void* GolfPlayer_GetEntryByIndex(void*, unsigned char);
unsigned int ItemInventory_GetItemIdAt(void*, int);

void Matrix_CopyAffine(void*, const void*);
void Matrix_ConcatAffine(void*, const void*, const void*);
void Matrix_SetAxisRotation(void*, float, signed char);
void Vec3_Zero(void*);
void AABB_Copy(void*, void*, unsigned int);
int AABB_Overlaps(void*, void*);
void* __nwa__FUl(unsigned int);
void __dla__FPv(void*);
void* appMallocDummy__FUl(unsigned int);
int CString_Compare(const char*, const char*);
void __dt__Q23std12length_errorFv(void*, int);
void* __dt__Q23std9exceptionFv(void*, int);
void __ml__FfRC7WVector(void*, const void*, float);
void __pl__FRC7WVectorRC7WVector(void*, const void*, const void*);
void __mi__FRC7WVectorRC7WVector(void*, const void*, const void*);

void Engine_GetViewport(void*);
void Engine_SetViewport(void*);
void Engine_SetRenderStateList(void*, int);
void Engine_SetTextureStageStateList(int, void*, int);
void Engine_SetTextureStageParameterList(int, void*, int);
void Engine_GetLightEnabled(int, void*);
void Engine_SetLightEnabled(int, void*);
void Engine_SetLight(int, void*);
void SetTransform(int, void*);
void gtl_GenericRenderer_SetProjectionMatrix(void*);
void gtl_GenericRenderer_SetViewMatrix(void*);
void Camera_SetFov(void*, float);
void Camera_BuildFrustumPlanes(void*);
void WView_SetViewMatrix(void*, void*);
void Camera_Apply(void*, float, float);
void* Camera_GetRenderState(void*);
void* WPuppet_GetSkinMatrices(void*);
void Model_SetMatrixRecursive(void*, int, void*);
void WPuppet_UpdateBoneWorldMatrices(void*, int, int, float);

} // extern "C"

// Offsets into courseEnvironmentStrings, the text is in the game's data.
enum {
    kWindRegionFormat = 0x6A,  // "%f %f %f %s"
    kNormalVariant = 0x76,     // "normal"
    kHardVariant = 0x7D,       // "hard"
    kEasyVariant = 0x82,       // "easy"
    kVectorLengthError = 0x87, // "vector length error"
    kLengthErrorType = 0x9B    // "!std::exception!!std::logic_error!!std::length_error!!"
};

enum {
    kSteepTiltState = 1 << 9, // CGolfDoc::gameStateFlags: the arrow camera tilts further
    kWindYawAxis = 1,
    kWindPitchAxis = 0,
    kCourseVariantMask = 0x0F
};
enum {
    kWindRegionBytes = 0x3C,
    kWindRegionMaxCount = 0x04444444,
    kWindRegionThreeFifthsLimit = 0x016c16c1,
    kWindRegionHalfLimit = 0x02d82d82
};

// Initialize the wind module's full-turn constant.
extern "C" void CWind_InitializeAngularConstant(void) {
    s_windAngularPeriod = kFloat_6_2831855;
}

// A buffer's pointer and element count.
struct CountedBuffer {
    void* data;
    int count;
};

// Empty a buffer, free its storage, and free the buffer object itself if asked.
extern "C" void* dtor_80214C24(void* self, int flags) {
    if (self) {
        CountedBuffer* buffer = (CountedBuffer*)self;
        if (*(void**)self) {
            CountedBuffer* buffer = (CountedBuffer*)self;
            buffer->count -= ((char*)buffer + buffer->count) - (char*)buffer;
            appFreeUni__FPv(buffer->data);
        }
        if (flags > 0) {
            __dl__FPv(self);
        }
    }
    return self;
}

// Empty a buffer, free its storage, and free the buffer object itself if asked.
extern "C" void* dtor_80214C94(void* self, int flags) {
    if (self) {
        CountedBuffer* buffer = (CountedBuffer*)self;
        buffer->count = 0;
        if (self) {
            if (buffer->data) {
                buffer->count = 0;
                appFreeUni__FPv(buffer->data);
            }
        }
        if (flags > 0) {
            __dl__FPv(self);
        }
    }
    return self;
}

// Whether the point lies within a region's inclusive bounds.
static inline int InBounds(const WindRegion* region, const WVector& point) {
    int inside = 0;
    if (region->minimum.x <= point.x && point.x <= region->maximum.x && region->minimum.y <= point.y &&
        point.y <= region->maximum.y && region->minimum.z <= point.z && point.z <= region->maximum.z)
        inside = 1;
    return inside;
}

// Whether the point lies inside any wind region.
bool CWind::ContainsPoint(const WVector& point) {
    WindRegion* region = regions;
    for (; region != regions + regionCount; region++) {
        if (InBounds(region, point))
            return true;
    }
    return false;
}

// The wind at a point. The base wind is the cannon's if it is firing, else the
// global wind (rescaled to the party item's strength when one forces it), plus
// the additive wind, and its length is capped at maxBaseMagnitude. A region
// then replaces it: the first region in list order that contains the point wins.
// If that region's neighbour also contains the point, the two winds are
// blended, each weighted by 1 - 2 * (distance from the point to the box centre)
// / (the box's diagonal). Finally a party item or stat override may normalize
// the result.
WVector CWind::GetWindVectorAtPoint(const WVector& point, float maxBaseMagnitude) {
    // The intermediate vectors share one frame, and the operators are called by their
    // symbols so that each result is written straight into its slot: assigning an
    // operator's result would go through a temporary.
    struct Frame {
        WVector basePlusAdditive, weightedNeighbor, weightedRegion, blendedWind;
        WVector neighborBoundsSum;
        WVector neighborCenter;
        WVector regionBoundsSum;
        WVector regionCenter;
        WVector neighborDiagonal, regionDiagonal, neighborCenterDelta, regionCenterDelta, selectedWind;
    } fr;
    WVector result;
    if (enabled == 0) {
        float zero = kFloat_0_SceneEffects;
        result.x = zero;
        result.y = zero;
        result.z = zero;
        return result;
    }
    ShotStatOverrides* stats = (ShotStatOverrides*)PartyItemSlots_GetInstance();
    if (overrideActive != 0) {
        fr.selectedWind = overrideVector;
    } else {
        if (stats->overrideWindMagnitude != 0) {
            fr.selectedWind = windVector;
            fr.selectedWind.Normalize();
            fr.selectedWind *= stats->windMagnitude;
        } else {
            fr.selectedWind = windVector;
        }
    }
    __pl__FRC7WVectorRC7WVector(&fr.basePlusAdditive, &fr.selectedWind, &additiveWind);
    fr.selectedWind = fr.basePlusAdditive;
    float len = fr.selectedWind.Magnitude();
    if (len > maxBaseMagnitude) {
        fr.selectedWind *= maxBaseMagnitude / len;
    }
    // The region search steps through the list by byte offset, and the neighbour is
    // addressed the same way; the compiler allocates registers differently for
    // typed pointer arithmetic.
    char* end;
    unsigned int inside;
    char* entries = (char*)regions;
    end = entries + regionCount * kWindRegionBytes;
    for (char* p = entries; end != p; p += kWindRegionBytes) {
        WindRegion* region = (WindRegion*)p;
        inside = 0;
        if (region->minimum.x <= point.x && point.x <= region->maximum.x && region->minimum.y <= point.y &&
            point.y <= region->maximum.y && region->minimum.z <= point.z && point.z <= region->maximum.z)
            inside = 1;
        if (inside) {
            int idx = region->adjacentRegion;
            char* ref;
            if (idx < 0 ||
                (ref = (char*)(idx * kWindRegionBytes), ref = (char*)((unsigned int)entries + (unsigned int)ref),
                 !InBounds((WindRegion*)ref, point))) {
                fr.selectedWind = region->wind;
            } else {
                WindRegion* neighbor;
                __pl__FRC7WVectorRC7WVector(&fr.regionBoundsSum, &region->minimum, &region->maximum);
                __ml__FfRC7WVector(&fr.regionCenter, &fr.regionBoundsSum, kFloat_0_5_SceneEffects);
                __mi__FRC7WVectorRC7WVector(&fr.regionCenterDelta, &fr.regionCenter, &point);
                ref = (char*)regions + region->adjacentRegion * kWindRegionBytes;
                neighbor = (WindRegion*)ref;
                __pl__FRC7WVectorRC7WVector(&fr.neighborBoundsSum, &neighbor->minimum, &neighbor->maximum);
                __ml__FfRC7WVector(&fr.neighborCenter, &fr.neighborBoundsSum, kFloat_0_5_SceneEffects);
                __mi__FRC7WVectorRC7WVector(&fr.neighborCenterDelta, &fr.neighborCenter, &point);
                __mi__FRC7WVectorRC7WVector(&fr.regionDiagonal, &region->maximum, &region->minimum);
                ref = (char*)regions + region->adjacentRegion * kWindRegionBytes;
                neighbor = (WindRegion*)ref;
                __mi__FRC7WVectorRC7WVector(&fr.neighborDiagonal, &neighbor->maximum, &neighbor->minimum);
                float regionDiagonalLength = fr.regionDiagonal.Magnitude();
                float regionRadialFraction = fr.regionCenterDelta.Magnitude();
                regionRadialFraction = (kFloat_2_Trajectory * regionRadialFraction) / regionDiagonalLength;
                float regionWeight = kFloat_1_SceneEffects - regionRadialFraction;
                float neighborDiagonalLength = fr.neighborDiagonal.Magnitude();
                float neighborRadialFraction = fr.neighborCenterDelta.Magnitude();
                neighborRadialFraction = kFloat_2_Trajectory * neighborRadialFraction / neighborDiagonalLength;
                float neighborWeight = kFloat_1_SceneEffects - neighborRadialFraction;

                ref = (char*)regions + region->adjacentRegion * kWindRegionBytes;
                neighbor = (WindRegion*)ref;
                __ml__FfRC7WVector(&fr.weightedNeighbor, &neighbor->wind,
                                   neighborWeight / (regionWeight + neighborWeight));
                __ml__FfRC7WVector(&fr.weightedRegion, &region->wind, regionWeight / (regionWeight + neighborWeight));
                __pl__FRC7WVectorRC7WVector(&fr.blendedWind, &fr.weightedRegion, &fr.weightedNeighbor);
                fr.selectedWind = fr.blendedWind;
            }
            break;
        }
    }
    {
        CSharedDoc* shared = (CSharedDoc*)g_sharedDoc;
        void* golfDoc = (void*)shared->golfDoc;
        unsigned char playerCount = ((CGolfDoc*)golfDoc)->playerCount;
        char* inventory = shared->itemInventory;
        if (playerCount) {
            unsigned char idx = ((CGolfDoc*)golfDoc)->currentPlayer;
            void* currentPlayer = GolfPlayer_GetEntryByIndex(golfDoc, idx);
            if (((RoundPlayerStats*)currentPlayer)->equippedItemId == (ItemInventory_GetItemIdAt(inventory, 4)) ||
                stats->normalizeWind != 0) {
                fr.selectedWind.Normalize();
            }
        }
    }
    // The result is copied out as raw words, not component by component.
    struct Words2 {
        int data[2];
    };
    *(Words2*)&result = *(Words2*)&fr.selectedWind;
    *(int*)&result.z = *(int*)&fr.selectedWind.z;
    return result;
}

// The arrow's model, called through its vtable. Only the slots used here are named.
struct WindModel {
    virtual void v0() DECOMP_LOCAL_VIRTUAL;
    virtual void v1() DECOMP_LOCAL_VIRTUAL;
    virtual void v2() DECOMP_LOCAL_VIRTUAL;
    virtual void setup(void*, int, int) DECOMP_LOCAL_VIRTUAL;
    virtual void v4() DECOMP_LOCAL_VIRTUAL;
    virtual void v5() DECOMP_LOCAL_VIRTUAL;
    virtual void v6() DECOMP_LOCAL_VIRTUAL;
    virtual void v7() DECOMP_LOCAL_VIRTUAL;
    virtual void draw(void*, int, int, int, int, void*, int, float) DECOMP_LOCAL_VIRTUAL;
};
struct CameraState {
    char pad_00[0x20];
    float view[16];       // 0x20
    float projection[16]; // 0x60
};

// Draw the wind arrow in its own viewport, restoring the camera, viewport and
// light enables afterwards. Nothing is drawn while the wind is negligible. The
// arrow's camera tilts forward, further when the game-state flag asks for it,
// and its distance scales with the main viewport's width. The arrow turns to
// the wind's direction, or is pitched straight up or down when the wind is
// nearly vertical. Light 7 and the render, texture and sampler states are left
// as set here, so the caller must set up its next pass.
void CWind::DrawDirectionModel(void* arrowModel) {
    struct Frame {
        float scaledCameraOffset[4];
        int savedLightEnable[8];
        unsigned int savedViewport[6];
        float verticalYaw[12];
        float verticalPitch[12];
        float verticalRoll[12];
        float verticalRollPitch[12];
        float verticalOrientation[12];
        float horizontalYaw[12], horizontalRoll[12], horizontalOrientation[12], normalCameraTilt[12],
            steepCameraTilt[12];
        SamplerStateList samplerStates;
        TextureStateList textureStates;
        float modelToWorld[12];
        RenderStateList renderStates;
    } fr;
    IndicatorRenderStates* data = &s_windIndicatorRenderStateTables;
    float verticalPitchRadians;
    void* arg = arrowModel;
    // A second pointer to this object keeps the compiled register use.
    CWind* self = this;
    // Both readiness guards share the exit that restores nothing.
    do {
        if (0 == arg)
            break;
        {
            if (self->indicatorWind.Magnitude() < kFloat_0_1_Trajectory)
                break;
        }
        {
            int i;
            // Keep the main camera's field of view, then install the tilted arrow camera.
            void* camera = g_view;
            float mainCameraFov =
                ((WView*)camera)->fieldOfView /
                (kWindIndicatorFovAspectScale * (((WView*)camera)->viewportWidth / ((WView*)camera)->viewportHeight));
            Camera_SetFov(camera, kWindIndicatorFieldOfView);

            void* golfDoc = (*(void**)&((CSharedDoc*)g_sharedDoc)->golfDoc);

            if (((*(unsigned int*)&((CGolfDoc*)golfDoc)->gameStateFlags)) & kSteepTiltState) {
                Matrix_SetAxisRotation(fr.steepCameraTilt, kFloat_neg1_5707964_SceneEffects, 0);
                Matrix_CopyAffine(self->indicatorViewRotation, fr.steepCameraTilt);
            } else {
                Matrix_SetAxisRotation(fr.normalCameraTilt, kFloat_neg0_5235988_Trajectory, 0);
                Matrix_CopyAffine(self->indicatorViewRotation, fr.normalCameraTilt);
            }

            void* mainView = g_view;
            float viewportScale = kFloat_640_TrajectoryViewport;
            float viewportWidth = ((WView*)mainView)->viewportWidth;
            viewportScale = viewportWidth / viewportScale;
            float cameraDistance = (kFloat_neg170_WindAndTrajectory * viewportScale);
            __ml__FfRC7WVector(fr.scaledCameraOffset, &self->indicatorViewRotation[6], cameraDistance);
            self->indicatorViewPosition = *(WVector*)fr.scaledCameraOffset;

            WView_SetViewMatrix(g_view, self->indicatorViewRotation);
            Camera_BuildFrustumPlanes(g_view);

            // Install the arrow's viewport. The orientation is roll * yaw, with a
            // pitch in between when the wind is nearly vertical.
            Engine_GetViewport(fr.savedViewport);
            Engine_SetViewport(&self->indicatorViewport);

            // Read the sign bit of wind.y to compute its absolute magnitude.
            float absoluteWindY;
            unsigned int windYBits = (*(unsigned int*)&self->indicatorWind.y);
            if (!(windYBits & 0x80000000)) {
                absoluteWindY = self->indicatorWind.y;
            } else {

                absoluteWindY = -(self->indicatorWind.y);
            }
            unsigned int useHorizontalOrientation = (absoluteWindY < kWindIndicatorVerticalDirectionThreshold);
            if (useHorizontalOrientation) {
                Matrix_SetAxisRotation(fr.horizontalYaw, self->modelSetup.indicatorDirection, 1);
                Matrix_SetAxisRotation(fr.horizontalRoll, self->indicatorPhase, 2);
                Matrix_ConcatAffine(fr.horizontalOrientation, fr.horizontalRoll, fr.horizontalYaw);
                Matrix_CopyAffine(fr.modelToWorld, fr.horizontalOrientation);
            } else {
                float windY = self->indicatorWind.y;
                if (windY > kFloat_0_SceneEffects)
                    verticalPitchRadians = kWindIndicatorPositiveVerticalPitch;
                else
                    verticalPitchRadians = kFloat_neg1_5707964_SceneEffects;
                Matrix_SetAxisRotation(fr.verticalYaw, self->modelSetup.indicatorDirection, 1);
                Matrix_SetAxisRotation(fr.verticalPitch, verticalPitchRadians, 0);
                Matrix_SetAxisRotation(fr.verticalRoll, self->indicatorPhase, 2);
                Matrix_ConcatAffine(fr.verticalRollPitch, fr.verticalRoll, fr.verticalPitch);
                Matrix_ConcatAffine(fr.verticalOrientation, fr.verticalRollPitch, fr.verticalYaw);
                Matrix_CopyAffine(fr.modelToWorld, fr.verticalOrientation);
            }

            Model_SetMatrixRecursive(arg, 0, fr.modelToWorld);
            WPuppet_UpdateBoneWorldMatrices(arg, 0, 0, kFloat_1_SceneEffects);
            ((WindModel*)arg)->setup(&self->modelSetup, 0, 0);

            TextureStateList* texturePreset = &data->texture;
            SamplerStateList* samplerPreset = &data->sampler;
            fr.renderStates = data->render;
            fr.textureStates = *texturePreset;
            fr.samplerStates = *samplerPreset;
            Engine_SetRenderStateList(&fr.renderStates, 9);
            Engine_SetTextureStageStateList(0, &fr.textureStates, 6);
            Engine_SetTextureStageParameterList(0, &fr.samplerStates, 5);

            Camera_Apply(g_view, kFloat_0_SceneEffects, kFloat_0_SceneEffects);
            CameraState* cameraMatrices = (CameraState*)Camera_GetRenderState(g_view);
            SetTransform(1, cameraMatrices->projection);
            gtl_GenericRenderer_SetProjectionMatrix(cameraMatrices->projection);
            SetTransform(0, cameraMatrices->view);
            gtl_GenericRenderer_SetViewMatrix(cameraMatrices->view);

            // Switch every light off, then light the arrow with light 7.
            char* savedLightSlot = (char*)fr.savedLightEnable;
            for (i = 0; i < 8; i++) {
                Engine_GetLightEnabled(i, savedLightSlot);
                Engine_SetLightEnabled(i, 0);
                savedLightSlot = (savedLightSlot + 4);
            }
            Engine_SetLight(7, &self->indicatorLight);
            Engine_SetLightEnabled(7, (void*)1);

            void* skinMatrices = WPuppet_GetSkinMatrices(arg);
            ((WindModel*)arg)->draw(g_view, 1, 1, 0, 0, skinMatrices, 0, kFloat_1_SceneEffects);

            // Restore light enablement before restoring viewport and main camera.
            char* restoreLightSlot = (char*)fr.savedLightEnable;
            for (i = 0; 8 > i; i++) {

                Engine_SetLightEnabled(i, (void*)(*(int*)restoreLightSlot));
                restoreLightSlot += 4;
            }

            // Restore the viewport and the main camera.
            Engine_SetViewport(fr.savedViewport);
            WView_SetViewMatrix(g_view, s_cameraTransform);
            Camera_SetFov(g_view, mainCameraFov);
            Camera_BuildFrustumPlanes(g_view);
        }
    } while (0);
    return;
}

struct RegionWordPair {
    int x, y;
    // Leave two raw component words for the parser to populate.
    RegionWordPair() {}
    // Copy the first two vector components without floating-point conversion.
    RegionWordPair(const RegionWordPair& other) {
        struct Raw {
            int words[2];
        };
        *(Raw*)this = *(const Raw*)&other;
    }
    // Copy two component words and return the destination.
    RegionWordPair& operator=(const RegionWordPair& other) {
        struct Raw {
            int words[2];
        };
        *(Raw*)this = *(const Raw*)&other;
        return *this;
    }
};
struct RegionWordVector {
    RegionWordPair xy;
    int z;
    // Leave the three raw component words uninitialized.
    RegionWordVector() {}
    // Copy all three component words as stored.
    RegionWordVector(const RegionWordVector& src) : xy(src.xy), z(src.z) {}
};
struct RegionCopy {
    int index;
    RegionWordVector minimum, maximum, velocity;
    unsigned char strengthMode;
    float strength, yawDegrees, pitchDegrees;
    int adjacentRegion;
    // Copy the whole record word by word.
    RegionCopy(const RegionCopy& src) {
        try {
            index = src.index;
            minimum.xy = src.minimum.xy;
            minimum.z = src.minimum.z;
            maximum.xy = src.maximum.xy;
            maximum.z = src.maximum.z;
            velocity.xy = src.velocity.xy;
            velocity.z = src.velocity.z;
            strengthMode = src.strengthMode;
            strength = src.strength;
            yawDegrees = src.yawDegrees;
            pitchDegrees = src.pitchDegrees;
            adjacentRegion = src.adjacentRegion;
        } catch (...) {
            throw;
        }
    }
};

struct WindRotationScratch {
    float yaw[12], pitch[12], combined[12];
};
// The parser's stack frame, accessed by byte offset. Only candidate becomes a
// region; courseVariant filters it, the rotations and velocities are temporaries,
// and the other fields track the region list growing and the cleanup if that
// throws.
struct WindParseFrame {
    char exceptionFlags[0x40];
    unsigned int growthSmall, growthLarge, extraCapacity;
    char *oldEnd, *oldBegin, *oldEndCopy, *oldBeginCopy;
    char reserved5C[0x1C];
    char courseVariant[8];
    char exceptionObjects[0x24];
    float defaultVelocity[3], replaceVelocity[3], subtractVelocity[3], addVelocity[3], multiplyVelocity[3];
    char reservedE0[0x48];
    void* replacement;
    unsigned int constructedCount, replacementCapacity;
    void* capacityOwner;
    unsigned int oldRemaining;
    char reserved13C[4];
    WindRotationScratch defaultRotation, replaceRotation, subtractRotation, addRotation, multiplyRotation;
    WindRegion candidate;
    char reserved44C[8];
};
typedef char WindRegion_size[sizeof(WindRegion) == 0x3C ? 1 : -1];
typedef char WindParseFrame_size[sizeof(WindParseFrame) == 0x454 ? 1 : -1];
typedef char WindParseFrame_candidate[(int)DECOMP_OFFSETOF(WindParseFrame, candidate) == 0x410 ? 1 : -1];

// Offsets into the parser's frame. A rotation's DirectionRow is combined[6..8],
// the direction the region's wind points along. The frame is read through byte
// offsets because declaring typed locals changes the compiler's scheduling.
enum WindCommandOffsets {
    kCommandMinimum = (int)DECOMP_OFFSETOF(WindParseFrame, candidate.minimum),
    kCommandMaximum = (int)DECOMP_OFFSETOF(WindParseFrame, candidate.maximum),
    kCommandRegion = (int)DECOMP_OFFSETOF(WindParseFrame, candidate),
    kCommandNeighborIndex = (int)DECOMP_OFFSETOF(WindParseFrame, candidate.adjacentRegion),
    kCommandYawDegrees = (int)DECOMP_OFFSETOF(WindParseFrame, candidate.yawDegrees),
    kCommandPitchDegrees = (int)DECOMP_OFFSETOF(WindParseFrame, candidate.pitchDegrees),
    kCommandStrength = (int)DECOMP_OFFSETOF(WindParseFrame, candidate.strength),
    kCourseVariant = (int)DECOMP_OFFSETOF(WindParseFrame, courseVariant),
    kCommandKind = (int)DECOMP_OFFSETOF(WindParseFrame, candidate.strengthMode),
    kMultiplyRotationYaw = (int)DECOMP_OFFSETOF(WindParseFrame, multiplyRotation.yaw),
    kMultiplyRotationPitch = (int)DECOMP_OFFSETOF(WindParseFrame, multiplyRotation.pitch),
    kMultiplyRotationCombined = (int)DECOMP_OFFSETOF(WindParseFrame, multiplyRotation.combined),
    kMultiplyVelocity = (int)DECOMP_OFFSETOF(WindParseFrame, multiplyVelocity),
    kMultiplyRotationCombinedDirectionRow = (int)DECOMP_OFFSETOF(WindParseFrame, multiplyRotation.combined[6]),
    kCommandVelocity = (int)DECOMP_OFFSETOF(WindParseFrame, candidate.wind),
    kAddRotationYaw = (int)DECOMP_OFFSETOF(WindParseFrame, addRotation.yaw),
    kAddRotationPitch = (int)DECOMP_OFFSETOF(WindParseFrame, addRotation.pitch),
    kAddRotationCombined = (int)DECOMP_OFFSETOF(WindParseFrame, addRotation.combined),
    kAddVelocity = (int)DECOMP_OFFSETOF(WindParseFrame, addVelocity),
    kAddRotationCombinedDirectionRow = (int)DECOMP_OFFSETOF(WindParseFrame, addRotation.combined[6]),
    kSubtractRotationYaw = (int)DECOMP_OFFSETOF(WindParseFrame, subtractRotation.yaw),
    kSubtractRotationPitch = (int)DECOMP_OFFSETOF(WindParseFrame, subtractRotation.pitch),
    kSubtractRotationCombined = (int)DECOMP_OFFSETOF(WindParseFrame, subtractRotation.combined),
    kSubtractVelocity = (int)DECOMP_OFFSETOF(WindParseFrame, subtractVelocity),
    kSubtractRotationCombinedDirectionRow = (int)DECOMP_OFFSETOF(WindParseFrame, subtractRotation.combined[6]),
    kReplaceRotationYaw = (int)DECOMP_OFFSETOF(WindParseFrame, replaceRotation.yaw),
    kReplaceRotationPitch = (int)DECOMP_OFFSETOF(WindParseFrame, replaceRotation.pitch),
    kReplaceRotationCombined = (int)DECOMP_OFFSETOF(WindParseFrame, replaceRotation.combined),
    kReplaceVelocity = (int)DECOMP_OFFSETOF(WindParseFrame, replaceVelocity),
    kReplaceRotationCombinedDirectionRow = (int)DECOMP_OFFSETOF(WindParseFrame, replaceRotation.combined[6]),
    kDefaultRotationYaw = (int)DECOMP_OFFSETOF(WindParseFrame, defaultRotation.yaw),
    kDefaultRotationPitch = (int)DECOMP_OFFSETOF(WindParseFrame, defaultRotation.pitch),
    kDefaultRotationCombined = (int)DECOMP_OFFSETOF(WindParseFrame, defaultRotation.combined),
    kDefaultVelocity = (int)DECOMP_OFFSETOF(WindParseFrame, defaultVelocity),
    kDefaultRotationCombinedDirectionRow = (int)DECOMP_OFFSETOF(WindParseFrame, defaultRotation.combined[6]),
    kOldBegin = (int)DECOMP_OFFSETOF(WindParseFrame, oldBegin),
    kOldBeginCopy = (int)DECOMP_OFFSETOF(WindParseFrame, oldBeginCopy),
    kOldEnd = (int)DECOMP_OFFSETOF(WindParseFrame, oldEnd),
    kOldEndCopy = (int)DECOMP_OFFSETOF(WindParseFrame, oldEndCopy),
    kExtraCapacity = (int)DECOMP_OFFSETOF(WindParseFrame, extraCapacity),
    kReplacement = (int)DECOMP_OFFSETOF(WindParseFrame, replacement),
    kConstructedCount = (int)DECOMP_OFFSETOF(WindParseFrame, constructedCount),
    kReplacementCapacity = (int)DECOMP_OFFSETOF(WindParseFrame, replacementCapacity),
    kCapacityOwner = (int)DECOMP_OFFSETOF(WindParseFrame, capacityOwner),
    kOldRemaining = (int)DECOMP_OFFSETOF(WindParseFrame, oldRemaining),
    kGrowthSmall = (int)DECOMP_OFFSETOF(WindParseFrame, growthSmall),
    kGrowthLarge = (int)DECOMP_OFFSETOF(WindParseFrame, growthLarge),
};

// Copy-construct one region record in destination storage.
static inline void CopyRegion(RegionCopy* dst, const RegionCopy* src) {
    new (dst) RegionCopy(*src);
}

struct SharedText {
    void* vtable;
    int strong, weak;
    char* text;
    // Take ownership of exception-message bytes in a shared deleter record.
    SharedText(char* p) {
        strong = 1;
        weak = 1;
        vtable = s_sharedCharDeleterVtable;
        text = p;
    }
};

struct TextGuard {
    char* text;
    char* flag;
    // Guard exception-message bytes until a shared owner is installed.
    TextGuard(char* p, char* f) : text(p), flag(f) {}
    // Delete message bytes if ownership has not transferred.
    ~TextGuard() {
        if (text)
            __dla__FPv(text);
    }
};

struct ErrorConstruction {
    void* object;
    // Guard an incompletely constructed exception base.
    ErrorConstruction(void* p) : object(p) {}
    // Destroy the exception base if construction unwinds.
    ~ErrorConstruction() {
        if (object)
            __dt__Q23std9exceptionFv(object, -1);
    }
};

// Build and throw the vector-length exception in the parser's frame.
static inline void ThrowLengthError(char* frame, unsigned int objectOffset, unsigned int flagOffset) {
    char* error = frame + objectOffset;
    const char* message = courseEnvironmentStrings + kVectorLengthError;
    *(void**)error = __vt__Q23std11logic_error;
    frame[flagOffset] = 0;
    unsigned int size = strlen(message) + 1;
    char* text = (char*)__nwa__FUl((size << 1) - size);
    *(char**)(error + 4) = text;
    {
        ErrorConstruction base(error);
        {
            TextGuard guard(text, frame + flagOffset);
            SharedText* shared = new SharedText(text);
            *(SharedText**)(error + 8) = shared;
            guard.text = 0;
        }
        strcpy(*(char**)(error + 4), message);
        *(void**)error = __vt__Q23std12length_error;
        base.object = 0;
    }
    __throw(courseEnvironmentStrings + kLengthErrorType, error, (void*)__dt__Q23std12length_errorFv);
}

// Convert the borrowed degree value to radians and build an axis rotation.
static inline void SetRotationFromDegrees(void* matrix, const char& axis, const float& degrees,
                                          float factor = kFloat_0_017453292_Wind) {
    Matrix_SetAxisRotation(matrix, factor * (float)degrees, (signed char)axis);
}

// Build pitch*yaw into a branch's own matrices; candidate angles are in degrees.
// Offsets select scratch only. The resulting direction is combined matrix[6..8].
static inline void BuildWindRegionRotation(char* frame, int yawOffset, int pitchOffset, int combinedOffset) {
    char yawAxis = kWindYawAxis;
    SetRotationFromDegrees(frame + yawOffset, yawAxis, *(float*)(frame + kCommandYawDegrees));
    char pitchAxis = kWindPitchAxis;
    SetRotationFromDegrees(frame + pitchOffset, pitchAxis, *(float*)(frame + kCommandPitchDegrees));
    Matrix_ConcatAffine(frame + combinedOffset, frame + pitchOffset, frame + yawOffset);
}

// Adjust the parsed yaw then build pitch*yaw in one branch's scratch matrices.
// Keep the assignment inside the rotation argument: it rounds/stores before use.
static inline void BuildAdjustedWindRegionRotation(char* frame, int yawOffset, int pitchOffset, int combinedOffset) {
    char yawAxis = kWindYawAxis;
    SetRotationFromDegrees(
        frame + yawOffset, yawAxis,
        (*(float*)(frame + kCommandYawDegrees) = kFloat_180_ScoreCard - *(float*)(frame + kCommandYawDegrees)));
    char pitchAxis = kWindPitchAxis;
    SetRotationFromDegrees(frame + pitchOffset, pitchAxis, *(float*)(frame + kCommandPitchDegrees));
    Matrix_ConcatAffine(frame + combinedOffset, frame + pitchOffset, frame + yawOffset);
}

// Add a region from one of the course's wind commands: its bounds come from the
// course box, its wind from the text. The text is an optional prefix followed by
// "strength yaw pitch [variant]", with the angles in degrees (default 0). With no
// prefix or '+' the region adds its strength to the global speed, '*' multiplies
// by it, '-' subtracts from it (not below zero) and '=' replaces it. The wind
// points along the yaw (taken as 180 - yaw) and pitch. A variant token limits the
// region to some course variants. The new region is linked to the last older region
// it overlaps, and that region to it, and the list grows if it is full.
void CWind::AddRegionFromCommand(void* bounds, const char* command) {
    // Byte views are intentional: direct scratch members change CodeWarrior output.
    char frameStore[sizeof(WindParseFrame)];
    char* frame = frameStore;
    // Start the candidate region: bounds from the course, no neighbour, angles 0.
    Vec3_Zero(frame + kCommandMinimum);
    Vec3_Zero(frame + kCommandMaximum);
    int count = regionCount;
    *(int*)(frame + kCommandRegion) = count;
    *(int*)(frame + kCommandNeighborIndex) = -1;
    AABB_Copy(frame + kCommandMinimum, bounds, count);
    float z = kFloat_0_SceneEffects;
    *(float*)(frame + kCommandYawDegrees) = z;
    *(float*)(frame + kCommandPitchDegrees) = z;
    ((WindParseFrame*)frame)->courseVariant[0] = kWindDefaultCourseVariantByte0;
    ((WindParseFrame*)frame)->courseVariant[1] = kWindDefaultCourseVariantByte1;
    ((WindParseFrame*)frame)->courseVariant[2] = kWindDefaultCourseVariantByte2;
    ((WindParseFrame*)frame)->courseVariant[3] = kWindDefaultCourseVariantByte3;
    ((WindParseFrame*)frame)->courseVariant[4] = kWindDefaultCourseVariantByte4;
    ((WindParseFrame*)frame)->courseVariant[5] = kWindDefaultCourseVariantByte5;
    ((WindParseFrame*)frame)->courseVariant[6] = kWindDefaultCourseVariantByte6;
    ((WindParseFrame*)frame)->courseVariant[7] = kWindDefaultCourseVariantByte7;
    // Each prefix has its own rotation scratch, and a velocity from the global speed.
    switch (*(const volatile signed char*)command) {
    case '*': {
        sscanf(command + 1, courseEnvironmentStrings + kWindRegionFormat, frame + kCommandStrength,
               frame + kCommandYawDegrees, frame + kCommandPitchDegrees, frame + kCourseVariant);
        char kind = WindRegion::kScaleGlobalSpeed;
        *(char*)(frame + kCommandKind) = kind;
        BuildAdjustedWindRegionRotation(frame, kMultiplyRotationYaw, kMultiplyRotationPitch, kMultiplyRotationCombined);
        __ml__FfRC7WVector(frame + kMultiplyVelocity, frame + kMultiplyRotationCombinedDirectionRow,
                           *(float*)(frame + kCommandStrength) * speed);
        *(WVector*)(frame + kCommandVelocity) = *(WVector*)(frame + kMultiplyVelocity);
        break;
    }
    case '+': {
        sscanf(command + 1, courseEnvironmentStrings + kWindRegionFormat, frame + kCommandStrength,
               frame + kCommandYawDegrees, frame + kCommandPitchDegrees, frame + kCourseVariant);
        char kind = WindRegion::kAddToGlobalSpeed;
        *(char*)(frame + kCommandKind) = kind;
        BuildAdjustedWindRegionRotation(frame, kAddRotationYaw, kAddRotationPitch, kAddRotationCombined);
        __ml__FfRC7WVector(frame + kAddVelocity, frame + kAddRotationCombinedDirectionRow,
                           *(float*)(frame + kCommandStrength) + speed);
        *(WVector*)(frame + kCommandVelocity) = *(WVector*)(frame + kAddVelocity);
        break;
    }
    case '-': {
        sscanf(command + 1, courseEnvironmentStrings + kWindRegionFormat, frame + kCommandStrength,
               frame + kCommandYawDegrees, frame + kCommandPitchDegrees, frame + kCourseVariant);
        char kind = WindRegion::kSubtractFromGlobalSpeed;
        *(char*)(frame + kCommandKind) = kind;
        float t = kFloat_180_ScoreCard - *(float*)(frame + kCommandYawDegrees);
        *(float*)(frame + kCommandYawDegrees) = t;
        float remainingStrength = -*(float*)(frame + kCommandStrength) + speed;
        remainingStrength = kFloat_0_SceneEffects < remainingStrength ? remainingStrength : kFloat_0_SceneEffects;
        BuildWindRegionRotation(frame, kSubtractRotationYaw, kSubtractRotationPitch, kSubtractRotationCombined);
        __ml__FfRC7WVector(frame + kSubtractVelocity, frame + kSubtractRotationCombinedDirectionRow, remainingStrength);
        *(WVector*)(frame + kCommandVelocity) = *(WVector*)(frame + kSubtractVelocity);
        break;
    }
    case '=': {
        sscanf(command + 1, courseEnvironmentStrings + kWindRegionFormat, frame + kCommandStrength,
               frame + kCommandYawDegrees, frame + kCommandPitchDegrees, frame + kCourseVariant);
        char kind = WindRegion::kReplaceGlobalSpeed;
        *(char*)(frame + kCommandKind) = kind;
        BuildAdjustedWindRegionRotation(frame, kReplaceRotationYaw, kReplaceRotationPitch, kReplaceRotationCombined);
        __ml__FfRC7WVector(frame + kReplaceVelocity, frame + kReplaceRotationCombinedDirectionRow,
                           *(float*)(frame + kCommandStrength));
        *(WVector*)(frame + kCommandVelocity) = *(WVector*)(frame + kReplaceVelocity);
        break;
    }
    default: {
        sscanf(command, courseEnvironmentStrings + kWindRegionFormat, frame + kCommandStrength,
               frame + kCommandYawDegrees, frame + kCommandPitchDegrees, frame + kCourseVariant);
        char kind = WindRegion::kAddToGlobalSpeed;
        *(char*)(frame + kCommandKind) = kind;
        BuildAdjustedWindRegionRotation(frame, kDefaultRotationYaw, kDefaultRotationPitch, kDefaultRotationCombined);
        __ml__FfRC7WVector(frame + kDefaultVelocity, frame + kDefaultRotationCombinedDirectionRow,
                           *(float*)(frame + kCommandStrength) + speed);
        *(WVector*)(frame + kCommandVelocity) = *(WVector*)(frame + kDefaultVelocity);
        break;
    }
    }

    // Filter by course variant before touching the list: variant 0 accepts only
    // "normal", 1 rejects "hard", 2 rejects "easy", others accept everything.
    void* sharedDoc = g_sharedDoc;
    void* selectedCourse = ((CSharedDoc*)sharedDoc)->selectedCourseData;
    unsigned char variant;
    if (!selectedCourse) {
        variant = 0;
    } else {
        variant = ((IFF_STRUCT::sCourse*)selectedCourse)->courseFlags;
        variant = variant & kCourseVariantMask;
    }
    switch ((int)variant) {
    case 0:
        if (CString_Compare(frame + kCourseVariant, courseEnvironmentStrings + kNormalVariant) != 0)
            return;
        break;
    case 1:
        if (CString_Compare(frame + kCourseVariant, courseEnvironmentStrings + kHardVariant) == 0)
            return;
        break;
    case 2:
        if (CString_Compare(frame + kCourseVariant, courseEnvironmentStrings + kEasyVariant) == 0)
            return;
        break;
    }

    // Link every overlapping region to the candidate. The candidate keeps the last
    // overlap's index.
    char* p = (char*)regions;
    char* endp;
    char* cur = p;
    *(char**)(frame + kOldBegin) = cur;
    *(char**)(frame + kOldBeginCopy) = cur;
    unsigned int oldCount;
    // The bounds are reloaded before each overlap check.
    while ((oldCount = (unsigned int)regionCount, endp = (char*)regions + oldCount * kWindRegionBytes,
            *(char**)(frame + kOldEnd) = endp, *(char**)(frame + kOldEndCopy) = endp, cur != endp)) {
        if (AABB_Overlaps(frame + kCommandMinimum, cur + 0x04) != 0) {
            *(int*)(frame + kCommandNeighborIndex) = *(int*)cur;
            *(int*)(cur + 0x38) = *(int*)(frame + kCommandRegion);
        }
        cur += kWindRegionBytes;
    }
    unsigned int cap = regionCapacity;
    // With spare capacity, appending only copies the candidate and bumps the count.
    frame[0x3c] = 0;
    if (oldCount < cap) {
        char* slot = endp;
        CopyRegion((RegionCopy*)slot, (RegionCopy*)(frame + kCommandRegion));
        ++regionCount;
        return;
    }

    // No spare slot: allocate a bigger list and copy the regions across. Their
    // indices do not change, so the links stay valid. The small byte stores into
    // the frame mark which parts are built, for the cleanup if something throws.
    *(unsigned char*)(frame + 0x38) = 0;
    *(unsigned char*)(frame + 0x10) = 0;
    if (kWindRegionMaxCount - cap < 1u) {
        frame[8] = 0;
        ThrowLengthError(frame, 0x80, 0x0c);
    }
    unsigned int needed = (unsigned int)regionCount + 1 - regionCapacity;
    *(unsigned int*)(frame + kExtraCapacity) = needed;
    *(void**)(frame + kReplacement) = 0;
    unsigned int oldCap = regionCapacity;
    *(unsigned int*)(frame + kConstructedCount) = 0;
    *(unsigned int*)(frame + kReplacementCapacity) = 0;
    *(void**)(frame + kCapacityOwner) = &regionCapacity;
    *(unsigned int*)(frame + kOldRemaining) = 0;
    unsigned int maxMinus = kWindRegionMaxCount - oldCap;
    if (needed > maxMinus) {
        frame[0x34] = 0;
        ThrowLengthError(frame, 0x98, 0x30);
    }
    unsigned int newCap;
    if (oldCap < kWindRegionThreeFifthsLimit) {
        *(unsigned int*)(frame + kGrowthSmall) = ((oldCap + 1) * 3) / 5;
        unsigned int* growth = (unsigned int*)(frame + kGrowthSmall);
        if (*growth < *(unsigned int*)(frame + kExtraCapacity))
            growth = (unsigned int*)(frame + kExtraCapacity);
        newCap = oldCap + *growth;
    } else if ((unsigned int)oldCap < kWindRegionHalfLimit) {
        *(unsigned int*)(frame + kGrowthLarge) = (oldCap + 1) / 2;
        unsigned int* growth = (unsigned int*)(frame + kGrowthLarge);
        if (*growth < *(unsigned int*)(frame + kExtraCapacity))
            growth = (unsigned int*)(frame + kExtraCapacity);
        newCap = oldCap + *growth;
    } else {
        newCap = kWindRegionMaxCount;
    }
    frame[0x24] = 0;
    if (newCap > kWindRegionMaxCount) {
        frame[0x2c] = 0;
        ThrowLengthError(frame, 0x8c, 0x28);
    }
    void* newArr = appMallocDummy__FUl(newCap * kWindRegionBytes);
    *(void**)(frame + kReplacement) = newArr;
    *(int*)(frame + kReplacementCapacity) = newCap;
    *(unsigned int*)(frame + kOldRemaining) = (unsigned int)regionCount;
    *(unsigned char*)(frame + 0x20) = 0;

    // Construct the new region first, after the existing ones.
    char* slot = (char*)newArr + (unsigned int)regionCount * kWindRegionBytes +
                 *(unsigned int*)(frame + kConstructedCount) * kWindRegionBytes;
    unsigned int insertCount;
    insertCount = 1;
    // Fill the new slots.
    while (insertCount != 0) {
        CopyRegion((RegionCopy*)slot, (RegionCopy*)(frame + kCommandRegion));
        --insertCount;
        slot += kWindRegionBytes;
        ++*(unsigned int*)(frame + kConstructedCount);
    }

    // Copy the existing regions across, last first.
    char* oldPtr = (char*)regions;
    char* oldEnd = oldPtr + (unsigned int)regionCount * kWindRegionBytes;
    char* newPtr = (char*)newArr;
    char* dst = newPtr + *(unsigned int*)(frame + kOldRemaining) * kWindRegionBytes;
    char* src = oldEnd;
    frame[0x1c] = 0;
    // Copy the existing elements backwards.
    while (src > oldPtr) {
        src -= kWindRegionBytes;
        dst -= kWindRegionBytes;
        CopyRegion((RegionCopy*)dst, (RegionCopy*)src);
        --*(unsigned int*)(frame + kOldRemaining);
        *(int*)(frame + kConstructedCount) = *(int*)(frame + kConstructedCount) + 1;
    }

    // Install the new list and free the old one.
    char* oldArr = (char*)regions;
    void* replacement = *(void**)(frame + kReplacement);
    unsigned int replacementCount = *(unsigned int*)(frame + kConstructedCount);
    unsigned char zero = 0;
    regionCount = 0;
    unsigned int oldCapacity = regionCapacity;
    unsigned int replacementCapacity = *(unsigned int*)(frame + kReplacementCapacity);
    *(void**)(frame + kReplacement) = (void*)oldArr;
    *(unsigned char*)(frame + 0x18) = zero;
    regionCapacity = replacementCapacity;
    *(unsigned int*)(frame + kReplacementCapacity) = oldCapacity;
    regions = (WindRegion*)replacement;
    regionCount = replacementCount;
    *(int*)(frame + kConstructedCount) = 0;
    if (oldArr != 0) {
        *(unsigned char*)(frame + 0x14) = zero;
        *(int*)(frame + kConstructedCount) = 0;
        appFreeUni__FPv(oldArr);
    }
}

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU804621F4", _ctors_u80213768_0, CWind_InitializeAngularConstant);
#pragma pop

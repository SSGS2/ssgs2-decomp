#include <decomp/static_init.h>
#include <decomp/offset_of.h>
#include "classes/CWind.h"
#include "iff/IffStruct.h"
#include "classes/WindStateView.h"
#include "classes/CGolfDoc.h"
#include "classes/WView.h"
#define AsWView(p) ((WView*)(p))
#include "classes/CSharedDoc.h"
#define AsCSharedDoc(p) ((CSharedDoc*)(p))
// Translation unit 0x80213768-0x80214D14 (7 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

#include "sdk_protos.h"
#include "game/wvector.h"

// Return caller-provided storage for copy construction; no allocation occurs.
inline void* operator new(size_t, void* p) { return p; }

extern "C" {

// PartyItemSlots_GetInstance supplies optional wind and spin/curve stat overrides.
struct ShotStatOverrides {
    char reserved0[0x28]; float windMagnitude;
    char reserved2C[8]; float spinRange; float curveRange;
    char reserved3C[0x14]; int normalizeWind; int overrideWindMagnitude; int overrideSpinCurve;
};
struct RoundPlayerStats {
    char reserved0[0xA0]; unsigned int equippedItemId;
    char reservedA4[0x40]; int spin; int curve;
};


extern "C" const float kFloat_6_2831855;
extern "C" float s_windAngularPeriod;

extern void* g_sharedDoc;
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

extern void* g_view;
struct Block72 { int data[18]; };
struct Block48 { int data[12]; };
struct Block40 { int data[10]; };
struct RenderData { Block72 a; Block48 b; Block40 c; };
extern RenderData s_windIndicatorRenderStateTables;
extern char s_cameraTransform[];
extern const char courseEnvironmentStrings[];
struct CourseEnvironmentStringPool {
    char reserved0[0x6A];
    char windRegionFormat[0xC]; // +0x6A
    char normalVariant[0x7]; // +0x76
    char hardVariant[0x5]; // +0x7D
    char easyVariant[0x5]; // +0x82
    char vectorLengthError[0x14]; // +0x87
    char lengthErrorType[0x37]; // +0x9B
};

extern char __vt__Q23std11logic_error[];
extern char s_sharedCharDeleterVtable[];
extern char __vt__Q23std12length_error[];
// Separate byte symbols supply the parser's initial eight-byte course-variant token.
extern unsigned char kWindDefaultCourseVariantByte0, kWindDefaultCourseVariantByte1, kWindDefaultCourseVariantByte2, kWindDefaultCourseVariantByte3, kWindDefaultCourseVariantByte4, kWindDefaultCourseVariantByte5, kWindDefaultCourseVariantByte6, kWindDefaultCourseVariantByte7;

void appFreeUni__FPv(void*);
void __dl__FPv(void*);

void* PartyItemSlots_GetInstance(void);
void* GolfPlayer_GetEntryByIndex(void*, unsigned char);
unsigned int ItemInventory_GetItemIdAt(void*, int);

void __as__7WVectorFRC7WVector(void*, const void*);
void Matrix_CopyAffine(void*, const void*);
void Normalize__7WVectorFv(void*);
void __amu__7WVectorFf(void*, float);
void __pl__FRC7WVectorRC7WVector(void*, const void*, const void*);
void __mi__FRC7WVectorRC7WVector(void*, const void*, const void*);
void __ml__FfRC7WVector(void*, const void*, float);
float Magnitude__7WVectorCFv(const void*);
void Matrix_ConcatAffine(void*, const void*, const void*);
void Matrix_SetAxisRotation(void*, float, signed char);
void Vec3_Zero(void*);
void AABB_Copy(void*, void*, unsigned int);
int AABB_Overlaps(void*, void*);
void* __nwa__FUl(unsigned int);
void* __nw__FUl(unsigned int);
void __dla__FPv(void*);
void* appMallocDummy__FUl(unsigned int);
void appFreeUni__FPv(void*);
int CString_Compare(const char*, const char*);
void __dt__Q23std12length_errorFv(void*, int);
extern "C" void* __dt__Q23std9exceptionFv(void*, int);

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

// Region payload (0x3C bytes): +00 index; +04/+10 inclusive world-space min/max;
// +1C precomputed wind XYZ; +28 operation byte; +2C strength; +30/+34 yaw/pitch
// in degrees; +38 adjacent-region index (-1 if none). Vector owns records by value.
// Operation affects velocity at parse time; queries blend the stored XYZ directly.
// Game-state bit 9 selects the steeper indicator camera tilt; gameplay name unresolved.
enum { kWindIndicatorSteepTiltState = 1 << 9, kWindYawAxis = 1, kWindPitchAxis = 0, kCourseVariantMask = 0x0F };
enum WindRegionKind { kWindAdd = 0, kWindMultiply = 1, kWindSubtract = 2, kWindReplace = 3 };
enum { kWindRegionBytes = 0x3C, kWindRegionMaxCount = 0x04444444, kWindRegionThreeFifthsLimit = 0x016c16c1, kWindRegionHalfLimit = 0x02d82d82 };

// Runtime by-value wind payload; bounds/velocity use world XYZ, angles use degrees.
struct WindCommandRegion {
    int index; float minimum[3], maximum[3], velocity[3];
    char kind; char reserved29[3]; float strength, yawDegrees, pitchDegrees; int adjacentRegion;
};

struct ObjC24 {
    void* data;
    int count;
};

struct ObjC94 {
    void* data;
    int count;
};

// Initialize the wind module's full-turn constant.
void CWind_InitializeAngularConstant(void) {
    s_windAngularPeriod = kFloat_6_2831855;
}

// Free the wind-region vector and optionally its owner.
void* dtor_80214C24(void* self, int flags) {
    if (self) {
        ObjC24* o = (ObjC24*)self;
        if (*(void**)self) {
            ObjC24* o = (ObjC24*)self;
            o->count -= ((char*)o + o->count) - (char*)o;
            appFreeUni__FPv(o->data);
        }
        if (flags > 0) {
            __dl__FPv(self);
        }
    }
    return self;
}

// Clear and free vector storage and optionally its owner.
void* dtor_80214C94(void* self, int flags) {
    if (self) {
        ObjC94* o = (ObjC94*)self;
        o->count = 0;
        if (self) {
            if (o->data) {
                o->count = 0;
                appFreeUni__FPv(o->data);
            }
        }
        if (flags > 0) {
            __dl__FPv(self);
        }
    }
    return self;
}

// Return whether the point lies inside any inclusive wind-region bounds.
int CWind_ContainsPointInRegion(void* self, const float* pt) {
    struct Box { int h; float mn[3]; float mx[3]; char rest[0x20]; };
    struct Holder { char pad[0x90]; Box* items; int count; };
    Holder* h = (Holder*)self;
    // Test a borrowed XYZ point against the inclusive bounds stored in this record.
    struct In {
    // Return one when the borrowed world-space point lies within all three bound intervals.
    static inline int test(Box* p, const float* pt) {
        int res = 0;
        if (p->mn[0] <= pt[0] && pt[0] <= p->mx[0] &&
            p->mn[1] <= pt[1] && pt[1] <= p->mx[1] &&
            p->mn[2] <= pt[2] && pt[2] <= p->mx[2]) res = 1;
        return res;
    } };
    Box* p = h->items;
    for (; p != h->items + h->count; p++) {
        int r = In::test(p, pt);
        if (r != 0) return 1;
    }
    return 0;
}

// Test a region's inclusive minimum/maximum bounds.
static inline int inBox(char* p, const float* pt) {
    WindCommandRegion* region = (WindCommandRegion*)p;
    int res = 0;
    if (region->minimum[0] <= pt[0] && pt[0] <= region->maximum[0] &&
        region->minimum[1] <= pt[1] && pt[1] <= region->maximum[1] &&
        region->minimum[2] <= pt[2] && pt[2] <= region->maximum[2]) res = 1;
    return res;
}

// Select base/override wind, cap its magnitude and blend overlapping linked regions.
// point/out each address three floats in world axes; inputs are borrowed, output
// is copied. maxBaseMagnitude caps only base+additive wind, BEFORE a region may replace it.
// First containing region in vector order wins. If its linked neighbor also contains
// the point, weight each wind by 1 - 2*distance(point,boxCenter)/boxDiagonalLength
// and divide by the sum. This is radial center weighting, not per-axis trilinear
// interpolation. Nonzero diagonals and a nonzero weight sum are caller/data assumptions.
// The final item/stat override may normalize the result AFTER region selection.
void CWind_GetWindVectorAtPoint(float* out, void* obj, const float* point, float maxBaseMagnitude) {
    struct Frame {
        float basePlusAdditive[3], weightedNeighbor[3], weightedRegion[3], blendedWind[3];
        float neighborBoundsSum[3]; float neighborCenter[3]; float regionBoundsSum[3]; float regionCenter[3];
        float neighborDiagonal[3], regionDiagonal[3], neighborCenterDelta[3], regionCenterDelta[3], selectedWind[3];
    } fr;
    // Phase 1: choose base/stat wind and cap its magnitude before applying regions.
    if (((WindStateView*)obj)->windEnabled == 0) {
        float z = kFloat_0_SceneEffects;
        out[0] = z;

        out[1] = z; out[2] = z;
        return;
    }
    void* state = PartyItemSlots_GetInstance();
    if (((WindStateView*)obj)->useOverrideDirection != 0) {
        *(WVector*)fr.selectedWind = (*(WVector*)&((WindStateView*)obj)->overrideDirection[0]);
    } else {
        if (((ShotStatOverrides*)state)->overrideWindMagnitude != 0) {
            *(WVector*)fr.selectedWind = (*(WVector*)&((WindStateView*)obj)->direction[0]);
            (*(WVector*)fr.selectedWind).Normalize();
            *(WVector*)fr.selectedWind *= ((ShotStatOverrides*)state)->windMagnitude;
        } else {
            *(WVector*)fr.selectedWind = (*(WVector*)&((WindStateView*)obj)->direction[0]);
        }
    }
    __pl__FRC7WVectorRC7WVector(fr.basePlusAdditive, fr.selectedWind, &((CWind*)obj)->additiveWindXBits);
    *(WVector*)fr.selectedWind = *(WVector*)fr.basePlusAdditive;
    float len = (*(WVector*)fr.selectedWind).Magnitude();
    if (len > maxBaseMagnitude) {
        *(WVector*)fr.selectedWind *= maxBaseMagnitude / len;
    }
    char* end;
    unsigned int result;
    // Phase 2: first containing region wins; blend only its stored adjacent region.
    char* entries = (char*)((WindStateView*)obj)->boxes;

    end = entries + (((WindStateView*)obj)->boxCount) * kWindRegionBytes;
    for (char* p = entries; end != p; p += kWindRegionBytes) {
        WindCommandRegion* region = (WindCommandRegion*)p;
        result = 0;
        if (region->minimum[0] <= point[0] && point[0] <= region->maximum[0] &&
            region->minimum[1] <= point[1] && point[1] <= region->maximum[1] &&
            region->minimum[2] <= point[2] && point[2] <= region->maximum[2]) result = 1;
        if (result) {
            // The selected region stores at most one neighbor; other overlaps are ignored.
            int idx = ((WindRegionLinkView*)p)->adjacentRegion /* 0x38 */;
            // Original wind-region selection: evaluate the adjacent pointer before its box test.
            char* ref;
            if (idx < 0 || (ref = (char*)(idx * kWindRegionBytes),
                            ref = (char*)(((unsigned int)entries) + (unsigned int)ref),
                            !inBox(ref, point))) {
                *(WVector*)fr.selectedWind = *(WVector*)(p + 0x1c);
            } else {
                // Compute center distances and full diagonal lengths for both containing boxes.
                __pl__FRC7WVectorRC7WVector(fr.regionBoundsSum, (float*)(p + 0x04), (float*)(p + 0x10));
                __ml__FfRC7WVector(fr.regionCenter, fr.regionBoundsSum, kFloat_0_5_SceneEffects);
                __mi__FRC7WVectorRC7WVector(fr.regionCenterDelta, fr.regionCenter, point);
                ref = (char*)((WindStateView*)obj)->boxes + ((WindRegionLinkView*)p)->adjacentRegion /* 0x38 */ * kWindRegionBytes;
                __pl__FRC7WVectorRC7WVector(fr.neighborBoundsSum, (float*)(ref + 0x04), (float*)(ref + 0x10));
                __ml__FfRC7WVector(fr.neighborCenter, fr.neighborBoundsSum, kFloat_0_5_SceneEffects);
                __mi__FRC7WVectorRC7WVector(fr.neighborCenterDelta, fr.neighborCenter, point);
                __mi__FRC7WVectorRC7WVector(fr.regionDiagonal, (float*)(p + 0x10), (float*)(p + 0x04));
                ref = (char*)((WindStateView*)obj)->boxes + ((WindRegionLinkView*)p)->adjacentRegion /* 0x38 */ * kWindRegionBytes;
                __mi__FRC7WVectorRC7WVector(fr.neighborDiagonal, (float*)(ref + 0x10), (float*)(ref + 0x04));
                float regionDiagonalLength = (*(WVector*)fr.regionDiagonal).Magnitude();
                float regionRadialFraction = (*(WVector*)fr.regionCenterDelta).Magnitude();
                regionRadialFraction = (kFloat_2_Trajectory * regionRadialFraction) / regionDiagonalLength;
                float regionWeight = kFloat_1_SceneEffects - regionRadialFraction;
                float neighborDiagonalLength = (*(WVector*)fr.neighborDiagonal).Magnitude();
                float neighborRadialFraction = (*(WVector*)fr.neighborCenterDelta).Magnitude();
                neighborRadialFraction = kFloat_2_Trajectory * neighborRadialFraction / neighborDiagonalLength;
                float neighborWeight = kFloat_1_SceneEffects - neighborRadialFraction;

                ref = (char*)((WindStateView*)obj)->boxes + ((WindRegionLinkView*)p)->adjacentRegion /* 0x38 */ * kWindRegionBytes;
                __ml__FfRC7WVector(fr.weightedNeighbor, ref + 0x1c, (neighborWeight / (regionWeight + neighborWeight)));

                __ml__FfRC7WVector(fr.weightedRegion, p + 0x1c, (regionWeight / (regionWeight + neighborWeight)));
                __pl__FRC7WVectorRC7WVector(fr.blendedWind, fr.weightedRegion, fr.weightedNeighbor);
                *(WVector*)fr.selectedWind = *(WVector*)fr.blendedWind;
            }
            break;
        }
    }
    {
        // Phase 3: optional unit-length wind, then copy out without retaining pointers.
        struct GSub { char pad[0xa04]; char sub[0x18c]; void* cur; };
        GSub* g = (GSub*)g_sharedDoc;
        void* golfDoc = g->cur;
        unsigned char playerCount = ((CGolfDoc*)golfDoc)->playerCount;
        char* inventory = g->sub;
        if (playerCount) {
            unsigned char idx = ((CGolfDoc*)golfDoc)->currentPlayer;
            void* currentPlayer = GolfPlayer_GetEntryByIndex(golfDoc, idx);
            if (((RoundPlayerStats*)currentPlayer)->equippedItemId == (ItemInventory_GetItemIdAt(inventory, 4)) || ((ShotStatOverrides*)state)->normalizeWind != 0) {
                (*(WVector*)fr.selectedWind).Normalize();
            }
        }
    }
    struct Copy2 { int data[2]; };
    *(Copy2*)out = *(Copy2*)fr.selectedWind;
    *(int*)&out[2] = *(int*)&fr.selectedWind[2];
    return;
}

struct WindModel {
    virtual void v0();
    virtual void v1();
    virtual void v2();
    virtual void setup(void*, int, int);
    virtual void v4();
    virtual void v5();
    virtual void v6();
    virtual void v7();
    virtual void draw(void*, int, int, int, int, void*, int, float);
};
struct CameraState { char pad[0x20]; float view[16]; float projection[16]; };

// Draw the wind indicator in its own viewport, restoring camera and light state afterward.
// Borrowed argIn is a ready model; negligible wind draws nothing. The indicator
// uses a private viewport and world-axis wind orientation, with distance scaled
// by main viewportWidth/640. It restores viewport, eight light-enable values,
// global camera transform/FOV/frustum. Light 7 contents, installed render/texture
// states, model matrices and renderer transforms are NOT fully saved/restored;
// callers must establish their next pass state. Matrix concat order is shown below.
void CWind_DrawDirectionModel(void* selfIn, void* argIn) {
    struct Frame {
        float scaledCameraOffset[4]; int savedLightEnable[8]; unsigned int savedViewport[6];
        float verticalYaw[12]; float verticalPitch[12]; float verticalRoll[12]; float verticalRollPitch[12]; float verticalOrientation[12];
        float horizontalYaw[12],horizontalRoll[12], horizontalOrientation[12], normalCameraTilt[12], steepCameraTilt[12];
        Block40 samplerStates; Block48 textureStates; float modelToWorld[12]; Block72 renderStates;
    } fr;
    // Affine matrices use the engine layout: +0x18 is the forward XYZ basis;
    // +0x24 is translation. Model orientation and private camera tilt are separate.
    RenderData* data = &s_windIndicatorRenderStateTables;
    float verticalPitchRadians;
    void* arg = argIn;
    char* self = (char*)selfIn;
    // Original 0x80214790: share the render exit after the two readiness guards.
    do {
        if (0 == arg) break;
        {

            if ((((*(WVector*)&((WindStateView*)self)->sampledWindVector[0])).Magnitude()) < kFloat_0_1_Trajectory) break;
        }
        { int i;
            // Phase 1: retain main-camera FOV, then install the tilted indicator camera.
            void* camera = g_view;
            float mainCameraFov = AsWView(camera)->fieldOfView /
                (kWindIndicatorFovAspectScale * (AsWView(camera)->viewportWidth / AsWView(camera)->viewportHeight));
            Camera_SetFov(camera, kWindIndicatorFieldOfView);

            void* golfDoc = (*(void**)&((CSharedDoc*)g_sharedDoc)->golfDoc);

            if (((*(unsigned int*)&((CGolfDoc*)golfDoc)->gameStateFlags)) & kWindIndicatorSteepTiltState) {
                Matrix_SetAxisRotation(fr.steepCameraTilt, kFloat_neg1_5707964_SceneEffects, 0);
                Matrix_CopyAffine(self + 0x34, fr.steepCameraTilt);
            } else {
                Matrix_SetAxisRotation(fr.normalCameraTilt, kFloat_neg0_5235988_Trajectory, 0);
                Matrix_CopyAffine(self + 0x34, fr.normalCameraTilt);
            }

            void* mainView = g_view;
            float viewportScale = kFloat_640_TrajectoryViewport;
            float viewportWidth = AsWView(mainView)->viewportWidth;
            viewportScale = viewportWidth / viewportScale;
            float cameraDistance = (kFloat_neg170_WindAndTrajectory * viewportScale);
            __ml__FfRC7WVector(fr.scaledCameraOffset, self + 0x4c, cameraDistance);
            (*(WVector*)&((WindStateView*)self)->viewTranslation[0]) = *(WVector*)fr.scaledCameraOffset;

            WView_SetViewMatrix(g_view, self + 0x34);
            Camera_BuildFrustumPlanes(g_view);

            // Phase 2: install the private indicator viewport; orientation is built
            // as Z*Y normally, Z*X*Y for near-vertical wind.
            Engine_GetViewport(fr.savedViewport);
            Engine_SetViewport(self + 0x100);

            // Read the sign bit of wind.y to compute its absolute magnitude.
            float absoluteWindY;
            unsigned int windYBits = (*(unsigned int*)&((WindStateView*)self)->sampledWindVector[1]);
            if (!(windYBits & 0x80000000)) {
                absoluteWindY = ((WindStateView*)self)->sampledWindVector[1];
            } else {

                absoluteWindY = -(((WindStateView*)self)->sampledWindVector[1]);
            }
            unsigned int useHorizontalOrientation = (absoluteWindY < kWindIndicatorVerticalDirectionThreshold);
            if (useHorizontalOrientation) {
                Matrix_SetAxisRotation(fr.horizontalYaw, ((WindStateView*)self)->rotationY, 1);
                Matrix_SetAxisRotation(fr.horizontalRoll, ((WindStateView*)self)->rotationZ, 2);
                Matrix_ConcatAffine(fr.horizontalOrientation, fr.horizontalRoll, fr.horizontalYaw);
                Matrix_CopyAffine(fr.modelToWorld, fr.horizontalOrientation);
            } else {
                float windY = ((WindStateView*)self)->sampledWindVector[1];
                if (windY > kFloat_0_SceneEffects) verticalPitchRadians = kWindIndicatorPositiveVerticalPitch; else verticalPitchRadians = kFloat_neg1_5707964_SceneEffects;
                Matrix_SetAxisRotation(fr.verticalYaw, ((WindStateView*)self)->rotationY, 1);
                Matrix_SetAxisRotation(fr.verticalPitch, verticalPitchRadians, 0);
                Matrix_SetAxisRotation(fr.verticalRoll, ((WindStateView*)self)->rotationZ, 2);
                Matrix_ConcatAffine(fr.verticalRollPitch, fr.verticalRoll, fr.verticalPitch);
                Matrix_ConcatAffine(fr.verticalOrientation, fr.verticalRollPitch, fr.verticalYaw);
                Matrix_CopyAffine(fr.modelToWorld, fr.verticalOrientation);
            }

            Model_SetMatrixRecursive(arg, 0, fr.modelToWorld);
            WPuppet_UpdateBoneWorldMatrices(arg, 0, 0, kFloat_1_SceneEffects);
            ((WindModel*)arg)->setup(self + 0x64, 0, 0);

            Block48* texturePreset = &data->b;
            Block40* samplerPreset = &data->c;
            fr.renderStates = data->a;
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

            // Phase 3: temporarily disable all eight lights and use light 7 to draw.
            char* savedLightSlot = (char*)fr.savedLightEnable;
            for (i = 0; i < 8; i++) {
                Engine_GetLightEnabled(i, savedLightSlot);
                Engine_SetLightEnabled(i, 0);
                savedLightSlot = (savedLightSlot + 4);
            }
            Engine_SetLight(7, self + 0x118);
            Engine_SetLightEnabled(7, (void*)1);

            void* skinMatrices = WPuppet_GetSkinMatrices(arg);
            ((WindModel*)arg)->draw(g_view, 1, 1, 0, 0, skinMatrices, 0, kFloat_1_SceneEffects);

            // Restore light enablement before restoring viewport and main camera.
            char* restoreLightSlot = (char*)fr.savedLightEnable;
            for (i = 0; 8 > i; i++) {

                Engine_SetLightEnabled(i, (void*)(*(int*)restoreLightSlot));
                restoreLightSlot += 4;
            }

            // Phase 4: restore the saved enable bits/viewport and main camera transform.
            Engine_SetViewport(fr.savedViewport);
            WView_SetViewMatrix(g_view, s_cameraTransform);
            Camera_SetFov(g_view, mainCameraFov);
            Camera_BuildFrustumPlanes(g_view);
        }
    } while (0);
    return;
}

struct EntryPair13768 {
    int x, y;
    // Leave two raw component words for the parser to populate.
    EntryPair13768() {}
    // Copy the first two vector components without floating-point conversion.
    EntryPair13768(const EntryPair13768& other) {
        struct Raw { int words[2]; };
        *(Raw*)this = *(const Raw*)&other;
    }
    // Copy two component words and return the destination.
    EntryPair13768& operator=(const EntryPair13768& other) {
        struct Raw { int words[2]; };
        *(Raw*)this = *(const Raw*)&other;
        return *this;
    }
};
struct EntryVector {
    EntryPair13768 xy; int z;
    // Leave the three raw component words uninitialized.
    EntryVector() {}
    // Copy all three component words as stored.
    EntryVector(const EntryVector& src) : xy(src.xy), z(src.z) {}
};
struct Entry13768 {
    int index;
    EntryVector minimum, maximum, velocity;
    unsigned char kind;
    float strength, yawDegrees, pitchDegrees;
    int adjacentRegion;
    // Copy-construct the complete by-value wind record with original EH scope.
    Entry13768(const Entry13768& src) {
        try {
            index = src.index;
            minimum.xy = src.minimum.xy; minimum.z = src.minimum.z;
            maximum.xy = src.maximum.xy; maximum.z = src.maximum.z;
            velocity.xy = src.velocity.xy; velocity.z = src.velocity.z;
            kind = src.kind;
            strength = src.strength;
            yawDegrees = src.yawDegrees;
            pitchDegrees = src.pitchDegrees;
            adjacentRegion = src.adjacentRegion;
        } catch (...) { throw; }
    }
};

struct WindRotationScratch { float yaw[12], pitch[12], combined[12]; };
// Compiler-frame map, NOT saved/runtime region data. Only candidate becomes a
// vector element; courseVariant gates parsing, rotations/vectors are temporary,
// and the low bytes/old-allocation fields implement vector growth and EH cleanup.
// The parser borrows typed subviews below while retaining its original byte frame.
struct WindParseFrameLayout_u80213768 {
    char exceptionFlags[0x40];
    unsigned int growthSmall, growthLarge, extraCapacity;
    char *oldEnd, *oldBegin, *oldEndCopy, *oldBeginCopy;
    char reserved5C[0x1C]; char courseVariant[8]; char exceptionObjects[0x24];
    float defaultVelocity[3], replaceVelocity[3], subtractVelocity[3], addVelocity[3], multiplyVelocity[3];
    char reservedE0[0x48]; void* replacement;
    unsigned int constructedCount, replacementCapacity; void* capacityOwner; unsigned int oldRemaining;
    char reserved13C[4];
    WindRotationScratch defaultRotation, replaceRotation, subtractRotation, addRotation, multiplyRotation;
    WindCommandRegion candidate; char reserved44C[8];
};
typedef char WindCommandRegion_size[sizeof(WindCommandRegion) == 0x3C ? 1 : -1];
typedef char WindParseFrameLayout_u80213768_size[sizeof(WindParseFrameLayout_u80213768) == 0x454 ? 1 : -1];
typedef char WindParseFrameLayout_u80213768_candidate[(int)DECOMP_OFFSETOF(WindParseFrameLayout_u80213768, candidate) == 0x410 ? 1 : -1];

// Compile-time addresses into the byte scratch frame. Rotation DirectionRow means
// combined[6..8], the XYZ direction used by the parser.
// Keep byte access here: constructing typed scratch objects changes MWCC scheduling.
enum WindCommandOffsets {
    kCommandMinimum = (int)DECOMP_OFFSETOF(WindParseFrameLayout_u80213768, candidate.minimum),
    kCommandMaximum = (int)DECOMP_OFFSETOF(WindParseFrameLayout_u80213768, candidate.maximum),
    kCommandRegion = (int)DECOMP_OFFSETOF(WindParseFrameLayout_u80213768, candidate),
    kCommandNeighborIndex = (int)DECOMP_OFFSETOF(WindParseFrameLayout_u80213768, candidate.adjacentRegion),
    kCommandYawDegrees = (int)DECOMP_OFFSETOF(WindParseFrameLayout_u80213768, candidate.yawDegrees),
    kCommandPitchDegrees = (int)DECOMP_OFFSETOF(WindParseFrameLayout_u80213768, candidate.pitchDegrees),
    kCommandStrength = (int)DECOMP_OFFSETOF(WindParseFrameLayout_u80213768, candidate.strength),
    kCourseVariant = (int)DECOMP_OFFSETOF(WindParseFrameLayout_u80213768, courseVariant),
    kCommandKind = (int)DECOMP_OFFSETOF(WindParseFrameLayout_u80213768, candidate.kind),
    kMultiplyRotationYaw = (int)DECOMP_OFFSETOF(WindParseFrameLayout_u80213768, multiplyRotation.yaw),
    kMultiplyRotationPitch = (int)DECOMP_OFFSETOF(WindParseFrameLayout_u80213768, multiplyRotation.pitch),
    kMultiplyRotationCombined = (int)DECOMP_OFFSETOF(WindParseFrameLayout_u80213768, multiplyRotation.combined),
    kMultiplyVelocity = (int)DECOMP_OFFSETOF(WindParseFrameLayout_u80213768, multiplyVelocity),
    kMultiplyRotationCombinedDirectionRow = (int)DECOMP_OFFSETOF(WindParseFrameLayout_u80213768, multiplyRotation.combined[6]),
    kCommandVelocity = (int)DECOMP_OFFSETOF(WindParseFrameLayout_u80213768, candidate.velocity),
    kAddRotationYaw = (int)DECOMP_OFFSETOF(WindParseFrameLayout_u80213768, addRotation.yaw),
    kAddRotationPitch = (int)DECOMP_OFFSETOF(WindParseFrameLayout_u80213768, addRotation.pitch),
    kAddRotationCombined = (int)DECOMP_OFFSETOF(WindParseFrameLayout_u80213768, addRotation.combined),
    kAddVelocity = (int)DECOMP_OFFSETOF(WindParseFrameLayout_u80213768, addVelocity),
    kAddRotationCombinedDirectionRow = (int)DECOMP_OFFSETOF(WindParseFrameLayout_u80213768, addRotation.combined[6]),
    kSubtractRotationYaw = (int)DECOMP_OFFSETOF(WindParseFrameLayout_u80213768, subtractRotation.yaw),
    kSubtractRotationPitch = (int)DECOMP_OFFSETOF(WindParseFrameLayout_u80213768, subtractRotation.pitch),
    kSubtractRotationCombined = (int)DECOMP_OFFSETOF(WindParseFrameLayout_u80213768, subtractRotation.combined),
    kSubtractVelocity = (int)DECOMP_OFFSETOF(WindParseFrameLayout_u80213768, subtractVelocity),
    kSubtractRotationCombinedDirectionRow = (int)DECOMP_OFFSETOF(WindParseFrameLayout_u80213768, subtractRotation.combined[6]),
    kReplaceRotationYaw = (int)DECOMP_OFFSETOF(WindParseFrameLayout_u80213768, replaceRotation.yaw),
    kReplaceRotationPitch = (int)DECOMP_OFFSETOF(WindParseFrameLayout_u80213768, replaceRotation.pitch),
    kReplaceRotationCombined = (int)DECOMP_OFFSETOF(WindParseFrameLayout_u80213768, replaceRotation.combined),
    kReplaceVelocity = (int)DECOMP_OFFSETOF(WindParseFrameLayout_u80213768, replaceVelocity),
    kReplaceRotationCombinedDirectionRow = (int)DECOMP_OFFSETOF(WindParseFrameLayout_u80213768, replaceRotation.combined[6]),
    kDefaultRotationYaw = (int)DECOMP_OFFSETOF(WindParseFrameLayout_u80213768, defaultRotation.yaw),
    kDefaultRotationPitch = (int)DECOMP_OFFSETOF(WindParseFrameLayout_u80213768, defaultRotation.pitch),
    kDefaultRotationCombined = (int)DECOMP_OFFSETOF(WindParseFrameLayout_u80213768, defaultRotation.combined),
    kDefaultVelocity = (int)DECOMP_OFFSETOF(WindParseFrameLayout_u80213768, defaultVelocity),
    kDefaultRotationCombinedDirectionRow = (int)DECOMP_OFFSETOF(WindParseFrameLayout_u80213768, defaultRotation.combined[6]),
    kOldBegin = (int)DECOMP_OFFSETOF(WindParseFrameLayout_u80213768, oldBegin),
    kOldBeginCopy = (int)DECOMP_OFFSETOF(WindParseFrameLayout_u80213768, oldBeginCopy),
    kOldEnd = (int)DECOMP_OFFSETOF(WindParseFrameLayout_u80213768, oldEnd),
    kOldEndCopy = (int)DECOMP_OFFSETOF(WindParseFrameLayout_u80213768, oldEndCopy),
    kExtraCapacity = (int)DECOMP_OFFSETOF(WindParseFrameLayout_u80213768, extraCapacity),
    kReplacement = (int)DECOMP_OFFSETOF(WindParseFrameLayout_u80213768, replacement),
    kConstructedCount = (int)DECOMP_OFFSETOF(WindParseFrameLayout_u80213768, constructedCount),
    kReplacementCapacity = (int)DECOMP_OFFSETOF(WindParseFrameLayout_u80213768, replacementCapacity),
    kCapacityOwner = (int)DECOMP_OFFSETOF(WindParseFrameLayout_u80213768, capacityOwner),
    kOldRemaining = (int)DECOMP_OFFSETOF(WindParseFrameLayout_u80213768, oldRemaining),
    kGrowthSmall = (int)DECOMP_OFFSETOF(WindParseFrameLayout_u80213768, growthSmall),
    kGrowthLarge = (int)DECOMP_OFFSETOF(WindParseFrameLayout_u80213768, growthLarge),
};

// Copy-construct one wind-region record in destination storage.
static inline void copyEntry13768(Entry13768* dst, const Entry13768* src) {
    new(dst) Entry13768(*src);
}

struct SharedText13768 {
    void* vtable;
    int strong, weak;
    char* text;
    // Take ownership of exception-message bytes in a shared deleter record.
    SharedText13768(char* p) {
        strong = 1; weak = 1;
        vtable = s_sharedCharDeleterVtable;
        text = p;
    }
};

struct TextGuard13768 {
    char* text;
    char* flag;
    // Guard exception-message bytes until a shared owner is installed.
    TextGuard13768(char* p, char* f) : text(p), flag(f) {}
    // Delete message bytes if ownership has not transferred.
    ~TextGuard13768() { if (text) __dla__FPv(text); }
};

struct ErrorConstruction13768 {
    void* object;
    // Guard an incompletely constructed exception base.
    ErrorConstruction13768(void* p) : object(p) {}
    // Destroy the exception base if construction unwinds.
    ~ErrorConstruction13768() { if (object) __dt__Q23std9exceptionFv(object, -1); }
};

// Build and throw the vector-length exception with its original unwind storage.
static inline void lengthError13768(char* frame, unsigned int objectOffset,
                                    unsigned int flagOffset) {
    char* error = frame + objectOffset;
    const char* message = ((CourseEnvironmentStringPool*)courseEnvironmentStrings)->vectorLengthError;
    *(void**)error = __vt__Q23std11logic_error;
    frame[flagOffset] = 0;
    unsigned int size = strlen(message) + 1;
    char* text = (char*)__nwa__FUl((size << 1) - size);
    *(char**)(error + 4) = text;
    {
        ErrorConstruction13768 base(error);
        {
            TextGuard13768 guard(text, frame + flagOffset);
            SharedText13768* shared = new SharedText13768(text);
            *(SharedText13768**)(error + 8) = shared;
            guard.text = 0;
        }
        strcpy(*(char**)(error + 4), message);
        *(void**)error = __vt__Q23std12length_error;
        base.object = 0;
    }
    __throw(((CourseEnvironmentStringPool*)courseEnvironmentStrings)->lengthErrorType, error, (void*)__dt__Q23std12length_errorFv);
}

// Convert the borrowed degree value to radians and build an axis rotation.
static inline void rotate13768(void* matrix, const char& axis, const float& degrees,
                                float factor = kFloat_0_017453292_Wind) {
    Matrix_SetAxisRotation(matrix, factor * (float)degrees, (signed char)axis);
}

// Build pitch*yaw into a branch's own matrices; candidate angles are in degrees.
// Offsets select scratch only. The resulting direction is combined matrix[6..8].
static inline void BuildWindRegionRotation(char* frame, int yawOffset, int pitchOffset,
                                           int combinedOffset) {
    char yawAxis = kWindYawAxis;
    rotate13768(frame + yawOffset, yawAxis, *(float*)(frame + kCommandYawDegrees));
    char pitchAxis = kWindPitchAxis;
    rotate13768(frame + pitchOffset, pitchAxis, *(float*)(frame + kCommandPitchDegrees));
    Matrix_ConcatAffine(frame + combinedOffset, frame + pitchOffset, frame + yawOffset);
}

// Adjust the parsed yaw then build pitch*yaw in one branch's scratch matrices.
// Keep the assignment inside the rotation argument: it rounds/stores before use.
static inline void BuildAdjustedWindRegionRotation(char* frame, int yawOffset,
                                                   int pitchOffset, int combinedOffset) {
    char yawAxis = kWindYawAxis;
    rotate13768(frame + yawOffset, yawAxis, (*(float*)(frame + kCommandYawDegrees) = kFloat_180_ScoreCard - *(float*)(frame + kCommandYawDegrees)));
    char pitchAxis = kWindPitchAxis;
    rotate13768(frame + pitchOffset, pitchAxis, *(float*)(frame + kCommandPitchDegrees));
    Matrix_ConcatAffine(frame + combinedOffset, frame + pitchOffset, frame + yawOffset);
}

// Parse wind strength/direction and course variant, link overlapping bounds and append the region.
// Borrowed arg supplies world-space AABB data; str is a NUL-terminated command.
// Optional leading *, +, -, = modifies base strength (multiply/add/clamped subtract/
// replace); no prefix means add. Payload fields are strength, yaw degrees, pitch
// degrees, optional course-variant token (pooled sscanf "%f %f %f %s").
// Conversion count is unchecked: strength must parse, angles default to zero,
// variant uses its eight-byte default. Caller must bound the token to that storage.
// Yaw becomes 180-yaw, rotations concatenate pitch then yaw, and matrix[6..8]
// supplies the direction multiplied by computed strength. No input pointers retained.
void CWind_AddRegionFromCommand(void* self, void* arg, const char* str) {
    // Byte views are intentional: direct scratch members change CodeWarrior output.
    char frameStore[sizeof(WindParseFrameLayout_u80213768)];
    char* frame = frameStore;
    // Phase 1: stage bounds/defaults; every prefix builds the same candidate layout.
    Vec3_Zero(frame + kCommandMinimum);
    Vec3_Zero(frame + kCommandMaximum);
    int count = ((WindStateView*)self)->boxCount /* 0x94 */;
    *(int*)(frame + kCommandRegion) = count;
    *(int*)(frame + kCommandNeighborIndex) = -1;
    AABB_Copy(frame + kCommandMinimum, arg, count);
    float z = kFloat_0_SceneEffects;
    *(float*)(frame + kCommandYawDegrees) = z;
    *(float*)(frame + kCommandPitchDegrees) = z;
    ((WindParseFrameLayout_u80213768*)frame)->courseVariant[0] = kWindDefaultCourseVariantByte0;
    ((WindParseFrameLayout_u80213768*)frame)->courseVariant[1] = kWindDefaultCourseVariantByte1;
    ((WindParseFrameLayout_u80213768*)frame)->courseVariant[2] = kWindDefaultCourseVariantByte2;
    ((WindParseFrameLayout_u80213768*)frame)->courseVariant[3] = kWindDefaultCourseVariantByte3;
    ((WindParseFrameLayout_u80213768*)frame)->courseVariant[4] = kWindDefaultCourseVariantByte4;
    ((WindParseFrameLayout_u80213768*)frame)->courseVariant[5] = kWindDefaultCourseVariantByte5;
    ((WindParseFrameLayout_u80213768*)frame)->courseVariant[6] = kWindDefaultCourseVariantByte6;
    ((WindParseFrameLayout_u80213768*)frame)->courseVariant[7] = kWindDefaultCourseVariantByte7;
    // Policy (B=base strength, S=parsed strength): *=S*B, +=S+B, -=max(B-S,0),
    // ==S; no prefix also uses S+B. Each branch keeps its own rotation scratch.
    switch (*(const volatile signed char*)str) {
    case '*': {
        sscanf(str + 1, ((CourseEnvironmentStringPool*)courseEnvironmentStrings)->windRegionFormat, frame + kCommandStrength, frame + kCommandYawDegrees, frame + kCommandPitchDegrees, frame + kCourseVariant);
        char kind = kWindMultiply;
        *(char*)(frame + kCommandKind) = kind;
        BuildAdjustedWindRegionRotation(frame, kMultiplyRotationYaw, kMultiplyRotationPitch, kMultiplyRotationCombined);
        __ml__FfRC7WVector(frame + kMultiplyVelocity, frame + kMultiplyRotationCombinedDirectionRow, *(float*)(frame + kCommandStrength) * ((WindStateView*)self)->windStrength);
        *(WVector*)(frame + kCommandVelocity) = *(WVector*)(frame + kMultiplyVelocity);
        break; }
    case '+': {
        sscanf(str + 1, ((CourseEnvironmentStringPool*)courseEnvironmentStrings)->windRegionFormat, frame + kCommandStrength, frame + kCommandYawDegrees, frame + kCommandPitchDegrees, frame + kCourseVariant);
        char kind = kWindAdd;
        *(char*)(frame + kCommandKind) = kind;
        BuildAdjustedWindRegionRotation(frame, kAddRotationYaw, kAddRotationPitch, kAddRotationCombined);
        __ml__FfRC7WVector(frame + kAddVelocity, frame + kAddRotationCombinedDirectionRow, *(float*)(frame + kCommandStrength) + ((WindStateView*)self)->windStrength);
        *(WVector*)(frame + kCommandVelocity) = *(WVector*)(frame + kAddVelocity);
        break; }
    case '-': {
        sscanf(str + 1, ((CourseEnvironmentStringPool*)courseEnvironmentStrings)->windRegionFormat, frame + kCommandStrength, frame + kCommandYawDegrees, frame + kCommandPitchDegrees, frame + kCourseVariant);
        char kind = kWindSubtract;
        *(char*)(frame + kCommandKind) = kind;
        float t = kFloat_180_ScoreCard - *(float*)(frame + kCommandYawDegrees);
        *(float*)(frame + kCommandYawDegrees) = t;
        float remainingStrength = -*(float*)(frame + kCommandStrength) + ((WindStateView*)self)->windStrength;
        remainingStrength = kFloat_0_SceneEffects < remainingStrength ? remainingStrength : kFloat_0_SceneEffects;
        BuildWindRegionRotation(frame, kSubtractRotationYaw, kSubtractRotationPitch, kSubtractRotationCombined);
        __ml__FfRC7WVector(frame + kSubtractVelocity, frame + kSubtractRotationCombinedDirectionRow, remainingStrength);
        *(WVector*)(frame + kCommandVelocity) = *(WVector*)(frame + kSubtractVelocity);
        break; }
    case '=': {
        sscanf(str + 1, ((CourseEnvironmentStringPool*)courseEnvironmentStrings)->windRegionFormat, frame + kCommandStrength, frame + kCommandYawDegrees, frame + kCommandPitchDegrees, frame + kCourseVariant);
        char kind = kWindReplace;
        *(char*)(frame + kCommandKind) = kind;
        BuildAdjustedWindRegionRotation(frame, kReplaceRotationYaw, kReplaceRotationPitch, kReplaceRotationCombined);
        __ml__FfRC7WVector(frame + kReplaceVelocity, frame + kReplaceRotationCombinedDirectionRow, *(float*)(frame + kCommandStrength));
        *(WVector*)(frame + kCommandVelocity) = *(WVector*)(frame + kReplaceVelocity);
        break; }
    default: {
        sscanf(str, ((CourseEnvironmentStringPool*)courseEnvironmentStrings)->windRegionFormat, frame + kCommandStrength, frame + kCommandYawDegrees, frame + kCommandPitchDegrees, frame + kCourseVariant);
        char kind = kWindAdd;
        *(char*)(frame + kCommandKind) = kind;
        BuildAdjustedWindRegionRotation(frame, kDefaultRotationYaw, kDefaultRotationPitch, kDefaultRotationCombined);
        __ml__FfRC7WVector(frame + kDefaultVelocity, frame + kDefaultRotationCombinedDirectionRow, *(float*)(frame + kCommandStrength) + ((WindStateView*)self)->windStrength);
        *(WVector*)(frame + kCommandVelocity) = *(WVector*)(frame + kDefaultVelocity);
        break; }
    }

    // Phase 2: filter before mutating links/storage. Variant 0 accepts only normal;
    // 1 rejects hard; 2 rejects easy; other low-nibble values are unfiltered.
    void* g = g_sharedDoc;
    void* r3 = AsCSharedDoc(g)->selectedCourseData;
    unsigned char b;
    if (!r3) {
        b = 0;
    } else {
        b = ((IFF_STRUCT::sCourse*)r3)->courseFlags;
        b = b & kCourseVariantMask;
    }
    switch ((int)b) {
    case 0:
        if (CString_Compare(frame + kCourseVariant, ((CourseEnvironmentStringPool*)courseEnvironmentStrings)->normalVariant) != 0) return;
        break;
    case 1:
        if (CString_Compare(frame + kCourseVariant, ((CourseEnvironmentStringPool*)courseEnvironmentStrings)->hardVariant) == 0) return;
        break;
    case 2:
        if (CString_Compare(frame + kCourseVariant, ((CourseEnvironmentStringPool*)courseEnvironmentStrings)->easyVariant) == 0) return;
        break;
    }

    // Phase 3: link every overlapping old record to this candidate. The candidate
    // keeps the LAST overlap index, while every old overlap is updated to its index.
    // These links are mutated before allocation and are not rolled back on failure.
    char* p = (char*)(*(void**)&((WindStateView*)self)->boxes);
    char* endp;
    char* cur = p;
    *(char**)(frame + kOldBegin) = cur;
    *(char**)(frame + kOldBeginCopy) = cur;
    unsigned int oldCount;
    // Original region insertion: reload the bounds before each overlap check.
    while ((oldCount = (*(unsigned int*)&((WindStateView*)self)->boxCount),
            endp = (char*)(*(void**)&((WindStateView*)self)->boxes) + oldCount * kWindRegionBytes,
            *(char**)(frame + kOldEnd) = endp,
            *(char**)(frame + kOldEndCopy) = endp,
            cur != endp)) {
            if (AABB_Overlaps(frame + kCommandMinimum, cur + 0x04) != 0) {
                *(int*)(frame + kCommandNeighborIndex) = *(int*)cur;
                *(int*)(cur + 0x38) = *(int*)(frame + kCommandRegion);
            }
            cur += kWindRegionBytes;
    }
    unsigned int cap = (*(unsigned int*)&((WindStateView*)self)->boxCapacity);
    // A spare-capacity append commits only boxCount after copying the candidate.
    frame[0x3c] = 0;
    if (oldCount < cap) {
        char* slot = endp;
        copyEntry13768((Entry13768*)slot, (Entry13768*)(frame + kCommandRegion));
        ++(*(unsigned int*)&((WindStateView*)self)->boxCount);
        return;
    }

    // Phase 4: reserve a larger by-value vector when no spare slot exists.
    // Existing indices remain stable, so links survive moving the records.
    // Low-frame bytes are allocator/exception construction markers, not region
    // active flags. The append count and prefix-copy remainder are separate:
    // constructedCount grows; oldRemaining shrinks to zero before commit.
    *(unsigned char*)(frame + 0x38) = 0;
    *(unsigned char*)(frame + 0x10) = 0;
    if (kWindRegionMaxCount - cap < 1u) {
        frame[8] = 0;
        lengthError13768(frame, 0x80, 0x0c);
    }
    unsigned int needed = (*(unsigned int*)&((WindStateView*)self)->boxCount) + 1 - (*(unsigned int*)&((WindStateView*)self)->boxCapacity);
    *(unsigned int*)(frame + kExtraCapacity) = needed;
    *(void**)(frame + kReplacement) = 0;
    unsigned int oldCap = (*(unsigned int*)&((WindStateView*)self)->boxCapacity);
    *(unsigned int*)(frame + kConstructedCount) = 0;
    *(unsigned int*)(frame + kReplacementCapacity) = 0;
    *(void**)(frame + kCapacityOwner) = &((WindStateView*)self)->boxCapacity;
    *(unsigned int*)(frame + kOldRemaining) = 0;
    unsigned int maxMinus = kWindRegionMaxCount - oldCap;
    if (needed > maxMinus) {
        frame[0x34] = 0;
        lengthError13768(frame, 0x98, 0x30);
    }
    unsigned int newCap;
    if (oldCap < kWindRegionThreeFifthsLimit) {
        *(unsigned int*)(frame + kGrowthSmall) = ((oldCap + 1) * 3) / 5;
        unsigned int* growth = (unsigned int*)(frame + kGrowthSmall);
        if (*growth < *(unsigned int*)(frame + kExtraCapacity)) growth = (unsigned int*)(frame + kExtraCapacity);
        newCap = oldCap + *growth;
    } else if ((unsigned int)oldCap < kWindRegionHalfLimit) {
        *(unsigned int*)(frame + kGrowthLarge) = (oldCap + 1) / 2;
        unsigned int* growth = (unsigned int*)(frame + kGrowthLarge);
        if (*growth < *(unsigned int*)(frame + kExtraCapacity)) growth = (unsigned int*)(frame + kExtraCapacity);
        newCap = oldCap + *growth;
    } else {
        newCap = kWindRegionMaxCount;
    }
    frame[0x24] = 0;
    if (newCap > kWindRegionMaxCount) {
        frame[0x2c] = 0;
        lengthError13768(frame, 0x8c, 0x28);
    }
    void* newArr = appMallocDummy__FUl(newCap * kWindRegionBytes);
    *(void**)(frame + kReplacement) = newArr;
    *(int*)(frame + kReplacementCapacity) = newCap;
    *(unsigned int*)(frame + kOldRemaining) = (*(unsigned int*)&((WindStateView*)self)->boxCount);
    *(unsigned char*)(frame + 0x20) = 0;

    // Construct the appended candidate first, at oldCount.
    char* slot = (char*)newArr + (*(unsigned int*)&((WindStateView*)self)->boxCount) * kWindRegionBytes
                 + *(unsigned int*)(frame + kConstructedCount) * kWindRegionBytes;
    unsigned int insertCount;
    insertCount = 1;
    // Original 0x80213768 region insertion: fill the requested new slots.
    while (insertCount != 0) {
        copyEntry13768((Entry13768*)slot, (Entry13768*)(frame + kCommandRegion));
        --insertCount;
        slot += kWindRegionBytes;
        ++*(unsigned int*)(frame + kConstructedCount);
    }


    // Copy-construct prior records backwards into the unchanged logical indices.
    char* oldPtr = (char*)(*(void**)&((WindStateView*)self)->boxes);
    char* oldEnd = oldPtr + (*(unsigned int*)&((WindStateView*)self)->boxCount) * kWindRegionBytes;
    char* newPtr = (char*)newArr;
    char* dst = newPtr + *(unsigned int*)(frame + kOldRemaining) * kWindRegionBytes;
    char* src = oldEnd;
    frame[0x1c] = 0;
    // Original region insertion: copy existing elements backwards.
    while (src > oldPtr) {
        src -= kWindRegionBytes;
        dst -= kWindRegionBytes;
        copyEntry13768((Entry13768*)dst, (Entry13768*)src);
        --*(unsigned int*)(frame + kOldRemaining);
        *(int*)(frame + kConstructedCount) = *(int*)(frame + kConstructedCount) + 1;
    }


    // Commit replacement pointer/count/capacity, then free the old raw allocation.
    char* oldArr = (char*)(*(void**)&((WindStateView*)self)->boxes);
    void* replacement = *(void**)(frame + kReplacement);
    unsigned int replacementCount = *(unsigned int*)(frame + kConstructedCount);
    unsigned char zero = 0;
    (*(unsigned int*)&((WindStateView*)self)->boxCount) = 0;
    unsigned int oldCapacity = (*(unsigned int*)&((WindStateView*)self)->boxCapacity);
    unsigned int replacementCapacity = *(unsigned int*)(frame + kReplacementCapacity);
    *(void**)(frame + kReplacement) = (void*)oldArr;
    *(unsigned char*)(frame + 0x18) = zero;
    (*(unsigned int*)&((WindStateView*)self)->boxCapacity) = replacementCapacity;
    *(unsigned int*)(frame + kReplacementCapacity) = oldCapacity;
    (*(void**)&((WindStateView*)self)->boxes) = replacement;
    (*(unsigned int*)&((WindStateView*)self)->boxCount) = replacementCount;
    *(int*)(frame + kConstructedCount) = 0;
    if (oldArr != 0) {
        *(unsigned char*)(frame + 0x14) = zero;
        *(int*)(frame + kConstructedCount) = 0;
        appFreeUni__FPv(oldArr);
    }
}

} // extern "C"

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU804621F4", _ctors_u80213768_0, CWind_InitializeAngularConstant);
#pragma pop

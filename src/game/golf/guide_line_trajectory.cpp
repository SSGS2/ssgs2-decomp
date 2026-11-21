#include <decomp/static_init.h>
#include <decomp/offset_of.h>
#include "game/golfball.h"
#include "classes/WView.h"
#include "classes/CGuideLine.h"
#define AsCGuideLine(p) ((CGuideLine*)(p))
#include "classes/CSharedDoc.h"
#define AsCSharedDoc(p) ((CSharedDoc*)(p))
#include "classes/CGolfDoc.h"
#define AsCGolfDoc(p) ((CGolfDoc*)(p))
// Translation unit 0x80178178-0x80179180 (14 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

#include "game/singletons.h"
#include "classes/CScreen.h"

#define AsCScreen(p) ((CScreen*)(p))
extern "C" {

// Ground-ribbon records occupy 40 bytes; their centers follow the two edges.
struct GuideSegmentView { float left[3], right[3], center[3]; void* effect; };
struct GuideTrackView { char common[0x34]; GuideSegmentView segments[80]; };
struct GuideFxView {
    char common[0x80]; unsigned char active; char padding081[3];
    float position[3]; char padding090[0x14]; float startDelay;
};
struct GuideBallSampleView { char common[0x70]; int ballState; };
struct GuidePlayerPositionView { char common[0x1C]; float position[3]; };
struct GuideHoleView { char common[0xC]; float position[3]; char padding018[8]; };
struct GuideDocVectorView { char common[0x33C]; float initialSegmentDirection[3]; };
struct GuideTrackStrings { char common[0x18]; char yawActor[7]; char groundSequence[15]; };
enum {
    kGuideSegmentLimit = 80, kGuideSegmentStride = sizeof(GuideSegmentView),
    kGuideTutorialStage = 6, kGuideGroundCategory = 2, kGuideBallSimulating = 3,
    kGuideMsgUpdate = 0, kGuideMsgSetEnabled = 1, kGuideMsgDisable = 2,
    kPlayerQueryYawMessage = 3, kRollingStartSampleSlot = 8,
    kGuideLeftOffset = (int)DECOMP_OFFSETOF(GuideTrackView, segments[0].left),
    kGuideRightOffset = (int)DECOMP_OFFSETOF(GuideTrackView, segments[0].right),
    kGuideCenterOffset = (int)DECOMP_OFFSETOF(GuideTrackView, segments[0].center),
    kGuideEffectOffset = (int)DECOMP_OFFSETOF(GuideTrackView, segments[0].effect),
    kGuideLeftHeightOffset = (int)DECOMP_OFFSETOF(GuideTrackView, segments[0].left[1]),
    kGuideRightHeightOffset = (int)DECOMP_OFFSETOF(GuideTrackView, segments[0].right[1]),
    kScreenNameBucketCount = 32,
    kScreenNameMutexOffset = (int)DECOMP_OFFSETOF(CScreen, pad_084),
    kScreenFirstScriptOffset = (int)DECOMP_OFFSETOF(CScreen, ballWindScript),
    kScreenLastScriptOffset = (int)DECOMP_OFFSETOF(CScreen, featherBadgeScript)
};


extern char s_guideLineTypeInfo;
extern char s_screenRttiDescriptor;
extern float kFloat_6_2831855;
extern float s_guideLineFullTurnAngle;
extern float kFloat_0_05_GuideLine;
extern float kFloat_5_GuideLine;
extern float kFloat_3_2_GuideLine, kFloat_1_6_GuideLine, kFloat_0_033333335_GuideLine;
extern float kFloat_1_PvsEffects;
extern float kFloat_0_805DECF4;
extern float kFloat_50_GuideLineTrack;
extern float kFloat_1_AuxMenu, kFloat_0_5_AuxMenu, kFloat_neg0_20293_Screen, kFloat_0_457741_Screen, kFloat_neg0_865617_ScreenLighting;
extern char __vt__7CScreen[];
extern signed char s_pointerVectorInitialized;
extern char s_fontNumDrawRegistry[], s_fontNumDrawRegistryDtorRegistration[];
extern char guideLineTrackStrings[0x100];
extern void* g_fx;
extern void* g_sharedDoc;
extern void* g_view;
void CGuideLine_Draw(void*);
void CFx_RemoveSequence(void*, void*, int);
void CGuideLine_SimulateTrajectory(void*);
void CGuideLine_AppendGroundSegment(void*, void*);
void OSLockMutex(void*);
void OSUnlockMutex(void*);
void __dt__18NativeBase80201D18Fv(void*, int);
void appFreeUni__FPv(void*);
void __dl__FPv(void*);
void Matrix_TransformPoint(void*, void*, void*);
void WView_ProjectViewPoint(void*, void*, void*, int);
void* GolfBall__Fv();
void* CGolfBall_NewPreData(void*);
void CGolfBall_Reset(void*, void*);
void* GolfPlayer_GetEntryByIndex(void*, unsigned char);
float Magnitude__7WVectorCFv(void*);
void* InstanceTable_Get();
float GolfParameter_ResolveFloat(void*, int);
float ShotParameters_GetBaseLaunchSpeed(void*);
float Math_SquareRootApprox(float);
void IActor_DispatchMessage(void*, void*, const char*, int, float*, int, int, int);
void Matrix_SetAxisRotation(void*, float, int);
void* __ml__FfRC7WVector(void*, void*, float);
float __ml__FRC7WVectorRC7WVector(void*, void*);
void* CQuadTree_StepBallSimulation(void*, float);
void* __as__7WVectorFRC7WVector(void*, void*);
void* __pl__FRC7WVectorRC7WVector(void*, void*, void*);
void* __mi__FRC7WVectorRC7WVector(void*, void*, void*);
void* Vector_CrossProduct(void*, void*, void*);
void* Normalize__7WVectorFv(void*);
void* __amu__7WVectorFf(void*, float);
void* CQuadTree_GetInstance();
void* GetGroundPoint__9CQuadTreeF7WVectorbPib(void*, void*, void*, int, int, int);
void* CFx_CreateSequence(void*, void*, int);
void __ct__18NativeBase80201D18Fv(void*);
void IActor_AddSkipFlags(void*, int);
void OSInitMutex(void*);
void OSLockMutex(void*);
void OSUnlockMutex(void*);
void* appMallocUni__FUl(unsigned int);
void* __nw__FUl(unsigned int);
void Layout_Init(void*, int);
void CStrokePoint2d_Ctor(void*);
void CMapCamera2d_Ctor(void*);
void CBallWind2d_Ctor(void*);
void CGolfBase2d_Ctor(void*);
void CSwingResult2d_Ctor(void*);
void PointerVector_InitReserved(void*);
void __register_global_object(void*, void*, void*);
void* CFontNumDraw_CreateInVector(void*);
void Memory_Clear(void*, int);
void Vec3_Normalize(void*, void*);
struct S80178F28;
void* PointerVector_DestroyStorage(S80178F28*, int);

// Return the guide-line runtime type descriptor.
char* CGuideLine_GetRTTI() {
    return &s_guideLineTypeInfo;
}

struct WInstanceLock {
    void* lock;
    // Acquire one level of the name-table mutex for this guard's lifetime.
    WInstanceLock(void* l) { lock = l; OSLockMutex(l); }
    // Release exactly the mutex level acquired by this guard.
    ~WInstanceLock() { OSUnlockMutex(lock); }
};
struct Obj5 { char pad[0x5c]; Obj5(int); };
struct ObjE4 { char pad[0xe4]; ObjE4(); };
struct ObjA0 { char pad[0xa0]; ObjA0(); };
struct ObjA8 { char pad[0xa8]; ObjA8(); };
struct Obj168 { char pad[0x168]; Obj168(); };
struct Obj78 { char pad[0x78]; Obj78(); };
// Construct a layout child using the caller's layout mode.
Obj5::Obj5(int x) { Layout_Init(this, x); }
// Construct the stroke-point child in its allocated storage.
ObjE4::ObjE4() { CStrokePoint2d_Ctor(this); }
// Construct the map-camera child in its allocated storage.
ObjA0::ObjA0() { CMapCamera2d_Ctor(this); }
// Construct the ball/wind child in its allocated storage.
ObjA8::ObjA8() { CBallWind2d_Ctor(this); }
// Construct the shot-gauge child in its allocated storage.
Obj168::Obj168() { CGolfBase2d_Ctor(this); }
// Construct the swing-result child in its allocated storage.
Obj78::Obj78() { CSwingResult2d_Ctor(this); }

struct BigCtor { char pad[0x358]; };
struct S80178FB4;
void* NameTable_dtor(S80178FB4*, int);
void destroy__9CTaskMainFi(void*, int);
// Create HUD child displays, initialize the name table, and set default directional lighting.
void CScreen_Ctor(BigCtor* self) {
    // Phase 1: base construction and mutex-protected name buckets. tableReady
    // tracks which cleanup is valid if a later child allocation throws.
    __ct__18NativeBase80201D18Fv(self);
    bool tableReady = false;
    try {

    AsCScreen(self)->taskMainVtable = (char*)&__vt__7CScreen + 0x3c;
    (*(int*)&AsCScreen(self)->windIndicatorResource) = (0);
    *(void**)self = &__vt__7CScreen;
    OSInitMutex((char*)self + kScreenNameMutexOffset);
    (*(int*)&AsCScreen(self)->nameHashBuckets) = 0;
    OSLockMutex((char*)self + kScreenNameMutexOffset);
    (*(int*)&AsCScreen(self)->nameHashHead) = 0;
    (*(int*)&AsCScreen(self)->nameHashFreeList) = 0;
    AsCScreen(self)->unk_6C = 0;
    AsCScreen(self)->nameHashInitialBucketCount = kScreenNameBucketCount;
    (*(int*)&AsCScreen(self)->nameHashIterator) = 0;
    AsCScreen(self)->nameHashBucketCount = kScreenNameBucketCount;
    if (AsCScreen(self)->nameHashInitialBucketCount > 0) {
        try {
            AsCScreen(self)->nameHashBuckets = appMallocUni__FUl(0x80);
        } catch (...) {
            OSUnlockMutex((char*)self + kScreenNameMutexOffset);
            throw;
        }
        AsCScreen(self)->nameHashMask = AsCScreen(self)->nameHashBucketCount - 1;
        int i = 0;
        while (i < AsCScreen(self)->nameHashBucketCount) {
            ((int*)AsCScreen(self)->nameHashBuckets)[i] = 0;
                int savedI = i;
                int savedValue = savedI + 1;
                i = savedValue;
        }
    } else AsCScreen(self)->nameHashBuckets = 0;
    OSUnlockMutex((char*)self + kScreenNameMutexOffset);
    tableReady = true;
    // Phase 2: clear optional children and allocate the six core HUD children.
    AsCScreen(self)->chatMessageFontResource=0; AsCScreen(self)->hudFlags=0;
    (*(int*)&AsCScreen(self)->layout)=0; (*(int*)&AsCScreen(self)->strokePointDisplay)=0; (*(int*)&AsCScreen(self)->mapCameraDisplay)=0;
    (*(int*)&AsCScreen(self)->ballWindDisplay)=0; (*(int*)&AsCScreen(self)->golfBaseDisplay)=0; (*(int*)&AsCScreen(self)->tutorialIllustration)=0; (*(int*)&AsCScreen(self)->textTable)=0;
    (*(int*)&AsCScreen(self)->holeInfoSet)=0; (*(int*)&AsCScreen(self)->ballPreview)=0; (*(int*)&AsCScreen(self)->windInfo)=0; (*(int*)&AsCScreen(self)->remainingTimeInfo)=0;
    (*(int*)&AsCScreen(self)->ingameStatus)=0; (*(int*)&AsCScreen(self)->auxiliaryMenu)=0; (*(int*)&AsCScreen(self)->accuracyInfo)=0; (*(int*)&AsCScreen(self)->pangInfo)=0;
    (*(int*)&AsCScreen(self)->powerInfo)=0; (*(int*)&AsCScreen(self)->flyInfo)=0; (*(int*)&AsCScreen(self)->modeInfo)=0; (*(int*)&AsCScreen(self)->modeLabelRenderer)=0;
    (*(int*)&AsCScreen(self)->statusTextRenderer)=0; (*(int*)&AsCScreen(self)->calipers)=0; (*(int*)&AsCScreen(self)->skinsInfo)=0; (*(int*)&AsCScreen(self)->nearestPinInfo)=0;
    AsCScreen(self)->fxInterpreterKey=-1; AsCScreen(self)->unk_8=4;
    IActor_AddSkipFlags(self, 0x400);
    (*(Obj5**)&AsCScreen(self)->layout) = new Obj5(5);
    (*(ObjE4**)&AsCScreen(self)->strokePointDisplay) = new ObjE4();
    (*(ObjA0**)&AsCScreen(self)->mapCameraDisplay) = new ObjA0();
    (*(ObjA8**)&AsCScreen(self)->ballWindDisplay) = new ObjA8();
    (*(Obj168**)&AsCScreen(self)->golfBaseDisplay) = new Obj168();
    (*(Obj78**)&AsCScreen(self)->swingResultDisplay) = new Obj78();
    void* manager=g_sharedDoc;
    if ((*(int*)&AsCSharedDoc(manager)->partyGameMode)==3) {
        if (0 == Sing_805DC830()) {
            if (0 == s_pointerVectorInitialized) {
                PointerVector_InitReserved(&s_fontNumDrawRegistry);
                __register_global_object(&s_fontNumDrawRegistry,(void*)PointerVector_DestroyStorage,&s_fontNumDrawRegistryDtorRegistration);
                s_pointerVectorInitialized=1;
            }
            Sing_805DC830()=&s_fontNumDrawRegistry;
        }
        AsCScreen(self)->textTable=CFontNumDraw_CreateInVector(Sing_805DC830());
    }
    AsCScreen(self)->ballWindScriptId=-1; AsCScreen(self)->partyGolfBaseScriptId=-1; AsCScreen(self)->timebombScriptId=-1; AsCScreen(self)->pululuJarashiScriptId=-1; AsCScreen(self)->featherBadgeScriptId=-1;
    for(int off=kScreenFirstScriptOffset;off<=kScreenLastScriptOffset;off+=4) *(int*)((char*)self+off)=0;
    AsCScreen(self)->hudInitialized=0; AsCScreen(self)->shotOptionEnabled=0;
    // Phase 3: shared lighting defaults; direction is normalized in scene space.
    Memory_Clear(((char*)&AsCScreen(self)->lightType),0x68);
    float z = kFloat_neg0_865617_ScreenLighting;
    float y = kFloat_0_457741_Screen;
    float x = kFloat_neg0_20293_Screen;
    float half = kFloat_0_5_AuxMenu;
    float one = kFloat_1_AuxMenu;
    float savedOne = one;
    AsCScreen(self)->lightType=3;
    AsCScreen(self)->lightDiffuseRed=savedOne;
    AsCScreen(self)->lightDiffuseGreen=one;
    AsCScreen(self)->lightDiffuseBlue=one;
    float savedOne2 = one;
    AsCScreen(self)->lightSpecularRed=savedOne2;
    AsCScreen(self)->lightSpecularGreen=one;
    float savedHalf = half;
    AsCScreen(self)->lightSpecularBlue=one;
    AsCScreen(self)->lightAmbientRed=savedHalf;
    float savedHalf2 = half;
    AsCScreen(self)->lightAmbientGreen=savedHalf2;
    AsCScreen(self)->lightAmbientBlue=half;
    AsCScreen(self)->lightDirectionX=x;
    float savedY = y;
    AsCScreen(self)->lightDirectionY=savedY;
    AsCScreen(self)->lightDirectionZ=z;
    Vec3_Normalize(((char*)&AsCScreen(self)->lightDirectionX),((char*)&AsCScreen(self)->lightDirectionX));
    AsCScreen(self)->meterPresentation=0; AsCScreen(self)->clubChangeDisplayState=0; AsCScreen(self)->displayedClubIndex=-1; AsCScreen(self)->hudEnabled=1;
    } catch (...) {
        if (tableReady) NameTable_dtor((S80178FB4*)(((char*)&AsCScreen(self)->nameHashHead)), -1);
        destroy__9CTaskMainFi(self, -1);
        throw;
    }
}

// Adjust the secondary-base pointer before calling the guide-line drawing hook.
void CGuideLine_vfunc_1Thunk18(void* self) {
    CGuideLine_Draw((char*)self - 0x18);
}

// Return the screen runtime type descriptor.
char* CScreen_GetRTTI() {
    return &s_screenRttiDescriptor;
}

// Initialize the shared full-turn angle during static initialization.
void GuideLine_InitFullTurnAngle() {
    s_guideLineFullTurnAngle = kFloat_6_2831855;
}

// Destroy the guide-line base and optionally free the object.
void* CGuideLine_dtor(void* self, int shouldDelete) {
    if (self) {
        __dt__18NativeBase80201D18Fv(self, 0);
        if (shouldDelete > 0)
            __dl__FPv(self);
    }
    return self;
}

struct S80178F28 {
    void* ptr;
    volatile int count;
};

// Free vector backing storage, reset its volatile size, and optionally delete the vector.
void* PointerVector_DestroyStorage(S80178F28* self, int shouldDelete) {
    if (self) {
        void* ptr = self->ptr;
        if (ptr) {
            int count = self->count;
            self->count = count - count;
            appFreeUni__FPv(ptr);
        }
        if (shouldDelete > 0)
            __dl__FPv(self);
    }
    return self;
}

struct S80178260 {
    char padding000[0x1C];
    unsigned char drawingSuppressed;
    unsigned char stopRequested;
    char padding01E[2];
    int count;
    char padding024[4];
    unsigned char enabled;
    char padding029[3];
    float value;
    char padding030[0x28];
    void* ptr;
};

// Remove old segment effects, then rebuild the ground trajectory when enabled.
void CGuideLine_SetEnabled(S80178260* self, unsigned char enabled) {
    if (self->enabled) {
        S80178260* item = self;
        int i = 0;
        while (i < self->count) {
            CFx_RemoveSequence(g_fx, item->ptr, 0);
            item->ptr = 0;
            item = (S80178260*)((char*)item + kGuideSegmentStride);
            ++i;
        }
        self->count = 0;
    }
    self->enabled = enabled;
    if (enabled) {
        self->drawingSuppressed = 0;
        self->stopRequested = 0;
        AsCGuideLine(self)->simulating = 1;
        CGuideLine_SimulateTrajectory(self);
        self->value = kFloat_5_GuideLine + kFloat_0_05_GuideLine * (float)self->count;
    }
}

// Build a ground ribbon by running the shared ball through a temporary rolling simulation.
// Requires category-2 ground; other categories leave both ball and ribbon alone.
// This is destructive scratch simulation, not a save/restore of CGolfBall:
// - Only initial XYZ is copied before Reset; mass, radius, random key and fields
//   outside Reset's write set survive (see CGolfBall_Reset in golf_ball_predata_and_gallery.cpp).
// - Reset clears velocity, rotation, lie/spin/curve, flags, sample/event/contact
//   state, hole-in value and overflow storage. Those prior values are NOT saved.
// - Simulation fills pre-data and the ribbon. The final Reset clears its samples
//   again, places the ball at ribbon center[0], restores ground category 2 and
//   clears the guide's simulating flag. Ribbon effects remain owned by the guide.
void CGuideLine_SimulateTrajectory(void* arg) {
    S80178260* self = (S80178260*)arg;
    if (((CGolfBall*)GolfBall__Fv())->m_groundType == kGuideGroundCategory) {
        struct Words { unsigned int x; int y; int z; };
        float horizontalZero;
        void* scene = GolfBall__Fv();
        float yawMatrix[12], ballToLastCenter[3], ballToNextCenter[3], holeDirection[4], segmentStep[3];
        // Phase 1: reset at the current world position and derive horizontal
        // player-to-hole distance. Y is up; distance/spacing use the XZ plane.
        Words initial = *(Words*)scene;
        CGolfBall_Reset(GolfBall__Fv(), &initial);
        void* manager = g_sharedDoc;
        void* golfDoc = ((void*)AsCSharedDoc(manager)->golfDoc);
        void* playerEntry = GolfPlayer_GetEntryByIndex(golfDoc, AsCGolfDoc(golfDoc)->currentPlayer);
        void* holeGolfDoc = ((void*)AsCSharedDoc(g_sharedDoc)->golfDoc);

        void* table = AsCGolfDoc(holeGolfDoc)->holeRecords;
        unsigned int idx = AsCGolfDoc(holeGolfDoc)->currentHoleIndex; unsigned int* entry = (unsigned int*)((GuideHoleView*)table)[idx].position;
        Words entryVec = *(Words*)entry;
        __mi__FRC7WVectorRC7WVector(holeDirection, &entryVec, ((GuidePlayerPositionView*)playerEntry)->position);
        float launchVelocity[3], initialSegmentDirection[3], lastCenterToBall[3], nextCenterDelta[3], nextCenter[3], lastCenterDelta[3], candidateDelta[3], nextCandidateCenter[3], previousCenterDelta[3];
        holeDirection[1] = kFloat_0_805DECF4;
        float factor = kFloat_3_2_GuideLine * GolfParameter_ResolveFloat(InstanceTable_Get(), 0);
        float speed = Magnitude__7WVectorCFv(holeDirection) / factor;
        float scale = ShotParameters_GetBaseLaunchSpeed(InstanceTable_Get());
        speed = Math_SquareRootApprox(speed);
        float savedScale = speed * scale;
        speed = savedScale;
        float yawRadians;
        // Player message 3 writes yaw (radians) through payload0; consumed now,
        // so the stack output is borrowed only for this synchronous dispatch.
        IActor_DispatchMessage(self, self, (char*)&((GuideTrackStrings*)guideLineTrackStrings)->yawActor, kPlayerQueryYawMessage, &yawRadians, 0, 0, 0);
        float savedParams = yawRadians;
        Matrix_SetAxisRotation(yawMatrix, savedParams, 1);
        __ml__FfRC7WVector(launchVelocity, yawMatrix + 6, speed);
        __as__7WVectorFRC7WVector(&((CGolfBall*)GolfBall__Fv())->m_velocity, launchVelocity);
        // Slot 8 is the rolling-start sample index: collision code records it
        // when entering state 3; zero marks this preview as rolling from sample 0.
        ((CGolfBall*)GolfBall__Fv())->m_state=kGuideBallSimulating; ((CGolfBall*)GolfBall__Fv())->m_eventPreDataIndex[kRollingStartSampleSlot]=0;
        float unitValue = *(volatile float*)&kFloat_1_PvsEffects;
        float pointScale = *(volatile float*)&kFloat_1_6_GuideLine;
        AsCGuideLine(self)->opacity=unitValue;
        __ml__FfRC7WVector(initialSegmentDirection, ((GuideDocVectorView*)AsCSharedDoc(g_sharedDoc)->golfDoc)->initialSegmentDirection, pointScale);
        CGuideLine_AppendGroundSegment((void*)self, initialSegmentDirection);
        horizontalZero=kFloat_0_805DECF4;
        // Phase 2: step physics by 1/30 second, then catch the ribbon up to the ball.
        // segmentStep is 1.6 world units along horizontal (ball-lastCenter).
        while (self->count < kGuideSegmentLimit && ((CGolfBall*)GolfBall__Fv())->m_state == kGuideBallSimulating) {
            void* current = CGolfBall_NewPreData(GolfBall__Fv());
            CQuadTree_StepBallSimulation(CQuadTree_GetInstance(), kFloat_0_033333335_GuideLine);
            __as__7WVectorFRC7WVector(current, GolfBall__Fv());
            ((GuideBallSampleView*)current)->ballState = ((CGolfBall*)GolfBall__Fv())->m_state;
            __mi__FRC7WVectorRC7WVector(lastCenterToBall,GolfBall__Fv(),(char*)self + (self->count-1) * kGuideSegmentStride + kGuideCenterOffset);
            __as__7WVectorFRC7WVector(segmentStep,lastCenterToBall); segmentStep[1] = horizontalZero;
            Normalize__7WVectorFv(segmentStep); __amu__7WVectorFf(segmentStep,kFloat_1_6_GuideLine);
            void* last = GolfBall__Fv();
            __pl__FRC7WVectorRC7WVector(nextCenter,(char*)self + (self->count-1) * kGuideSegmentStride + kGuideCenterOffset,segmentStep);
            __mi__FRC7WVectorRC7WVector(nextCenterDelta,nextCenter,last);
            __as__7WVectorFRC7WVector(ballToNextCenter,nextCenterDelta);
            float savedProgress = horizontalZero;
            ballToNextCenter[1] = savedProgress;
            __mi__FRC7WVectorRC7WVector(lastCenterDelta,(char*)self + (self->count-1) * kGuideSegmentStride + kGuideCenterOffset,GolfBall__Fv());
            __as__7WVectorFRC7WVector(ballToLastCenter,lastCenterDelta);
            float savedProgress2 = horizontalZero;
            ballToLastCenter[1] = savedProgress2;
            // L = last center, N = L+step, B = simulated ball (all projected XZ).
            // (N-B).(L-B)>0 means B is outside the diameter circle of LN.
            // Along the forward ray this means B passed N: emit another center.
            // Keep the exact dot test; it is not a distance>=1.6 comparison
            // when the ball has moved sideways or a candidate was ground-clamped.
            while (self->count < kGuideSegmentLimit && __ml__FRC7WVectorRC7WVector(ballToNextCenter,ballToLastCenter) > horizontalZero) {
                CGuideLine_AppendGroundSegment(self,segmentStep);
                void* last = GolfBall__Fv();
                int savedCount = self->count;
                __pl__FRC7WVectorRC7WVector(nextCandidateCenter,(char*)self + (savedCount-1) * kGuideSegmentStride + kGuideCenterOffset,segmentStep);
                __mi__FRC7WVectorRC7WVector(candidateDelta,nextCandidateCenter,last);
                __as__7WVectorFRC7WVector(ballToNextCenter,candidateDelta);
                float savedProgress3 = horizontalZero;
                ballToNextCenter[1] = savedProgress3;
                __mi__FRC7WVectorRC7WVector(previousCenterDelta,(char*)self + (self->count-1) * kGuideSegmentStride + kGuideCenterOffset,GolfBall__Fv());
                __as__7WVectorFRC7WVector(ballToLastCenter,previousCenterDelta); ballToLastCenter[1] = horizontalZero;
            }
        }
        // Phase 3: reset at the first center (not the simulated endpoint).
        Words end = *(Words*)((char*)self+kGuideCenterOffset);
        CGolfBall_Reset(GolfBall__Fv(),&end); ((CGolfBall*)GolfBall__Fv())->m_groundType=kGuideGroundCategory; AsCGuideLine(self)->simulating=0;
    }
}

struct S80178FB4Node { char padding000[0x0C]; S80178FB4Node* next; S80178FB4Node* prev; };
enum { kNameTableMutexOffset = 0x20 };
struct S80178FB4 {
    char padding000[8];
    S80178FB4Node* head;
    char padding00C[0x0C];
    char padding018[4];
    void* allocation;
};

#pragma push
#pragma use_lmw_stmw on
// Unlink one name-table node under its mutex and return the previous node or null.
inline S80178FB4Node* remove80178FB4(S80178FB4* self, S80178FB4Node* node) {
    void* lock = self + 1;
    OSLockMutex(lock);
    node->prev->next = node->next;
    node->next->prev = node->prev;
    S80178FB4Node* next = node->prev;
    if (next == node) next = 0;
    OSUnlockMutex(lock);
    return next;
}
// Remove and free all name-table nodes and buckets while holding the table mutex.
void* NameTable_dtor(S80178FB4* self, int shouldDelete) {
    if (self) {
        {
        WInstanceLock guard((char*)self + kNameTableMutexOffset);
        if (self->head) {
        S80178FB4Node* node;
        while ((node = self->head) != 0) {
            self->head = remove80178FB4(self, node);
            appFreeUni__FPv(node);
        }
        }
        if (self->allocation) {
            appFreeUni__FPv(self->allocation);
            self->allocation = 0;
        }
        }
        if (shouldDelete > 0) __dl__FPv(self);
    }
    return self;
}
#pragma pop

struct S80178178 : S80178260 {};
// Handle enable, disable and stop requests for the ground guide line.
void CGuideLine_HandleMsg(S80178178* self, const int* arg) {
    switch (arg[1]) {
    case kGuideMsgDisable:
        CGuideLine_SetEnabled(self, 0);
        break;
    case kGuideMsgSetEnabled:
        CGuideLine_SetEnabled(self, arg[2] != 0);
        break;
    case kGuideMsgUpdate:
        if (self->enabled) {
            if (!self->drawingSuppressed) {
                S80178260* item = self;
                int i = 0;
                while (i < self->count) {
                    CFx_RemoveSequence(g_fx, item->ptr, 0);
                    item->ptr = 0;
                    item = (S80178260*)((char*)item + kGuideSegmentStride);
                    ++i;
                }
                self->count = 0;
            }
            self->stopRequested = 1;
        }
        break;
    }
}

struct S8017908CNode { char padding000[0x0C]; S8017908CNode* next; S8017908CNode* prev; };
struct S8017908C {
    char padding000[0x64];
    char padding064[8];
    S8017908CNode* head;
    char padding070[0x10];
    void* allocation;
    char mutex[0x10];
};

#pragma push
#pragma use_lmw_stmw on
// Destroy the screen name table under its mutex, then its base and optional allocation.
void* CScreen_dtor(S8017908C* self, int shouldDelete) {
    if (self) {
        if (((char*)&AsCScreen(self)->nameHashHead)) {
        WInstanceLock guard(self->mutex);
        if (self->head) {
        S8017908CNode* next;
        void* lock = (char*)self + kScreenNameMutexOffset;
        S8017908CNode* node;
        while ((node = self->head) != 0) {
            OSLockMutex(lock);
            node->prev->next = node->next;
            node->next->prev = node->prev;
            next = node->prev;
            if (next == node) next = 0;
            OSUnlockMutex(lock);
            self->head = next;
            appFreeUni__FPv(node);
        }
        }
        if (self->allocation) {
            appFreeUni__FPv(self->allocation);
            self->allocation = 0;
        }
        }
        if (self) {
            __dt__18NativeBase80201D18Fv(self, 0);
        }
        if (shouldDelete > 0) __dl__FPv(self);
    }
    return self;
}
#pragma pop

struct S80178338Entry { char padding000[0x34]; void* first; void* second; char padding03C[4]; };
struct S80178338 {
    void render();
    char padding000[0x1C]; unsigned char state; char stopRequested[3];
    int count;
    char padding024[0xCB4 - 0x24];
    int flags;
    char projectionScratch[0xCD0 - 0xCB8];
    float alpha;
    float time;
    char vertexScratch[0xC0];
    char paddingD98[0x10];
};

struct Render78338 {
    virtual void slot0();
    virtual void slot1();
    virtual void slot2();
    virtual void slot3();
    virtual void draw(void*, int, int, int);
};
struct Query78338 {
    virtual void slot0(); virtual void slot1(); virtual void slot2();
    virtual void slot3(); virtual void slot4(); virtual void slot5();
    virtual void slot6(); virtual void slot7(); virtual void slot8();
    virtual void slot9(); virtual void slot10(); virtual void slot11();
    virtual int query();
};
// Project both edges of each segment and submit one textured ground-ribbon quad.
inline void S80178338::render() {
    // Each quad joins centers i and i+1: [left_i,right_i,left_next,right_next].
    // Edges are world points -> inputTransform view space -> projected vertices.
    // U selects the edge (0/1); V advances 0.1 per center along the ribbon.
    char* dst;
    char* src;
    int low;
    int high;
    int i;
    int j;
    void* item;
    char temp[0xC];
    float f1;
    if (AsCGolfDoc(((void*)AsCSharedDoc(g_sharedDoc)->golfDoc))->tutorialStage >= kGuideTutorialStage &&
        !((Query78338*)this)->query() &&
        AsCGolfDoc(((void*)AsCSharedDoc(g_sharedDoc)->golfDoc))->tutorialStage >= kGuideTutorialStage &&
        ((CGolfBall*)GolfBall__Fv())->m_groundType == kGuideGroundCategory &&
        !state && ((S80178260*)this)->enabled) {
    i = 0;
    while (i < count - 1) {
        src = (char*)this;
        dst = src + 0xCB8;
        j = 0;
        while (j < 4) {
            low = j & 1;
            if (low) {
                high = j & 2;
                item = (char*)this + (i + ((j >> 1) & 1)) * 0x28 + 0x40;
            } else {
                high = j & 2;
                item = (char*)this + (i + ((j >> 1) & 1)) * 0x28 + 0x34;
            }
            Matrix_TransformPoint(temp, item, (char*)((WView*)g_view)->inputTransform); // Original +0x14 in CGuideLine drawing.
            WView_ProjectViewPoint((char*)g_view, dst, temp, 0);
            *(float*)(src + 0xCD0) = low ? kFloat_1_PvsEffects : kFloat_0_805DECF4;
            f1 = high ? kFloat_1_PvsEffects : kFloat_0_805DECF4;
            *(float*)(src + 0xCD4) = 0.1f * ((float)i + f1);
            ++j;
            src += 0x38;
            dst += 0x38;
        }
        ((Render78338*)g_view)->draw(src + 0xCB8, 0x10, 0x20B00000 | (flags & 0x7FF), 0);
        ++i;
    }
    }
    return;
}

// Project and draw the enabled ground-guide ribbon in supported tutorial states.
void CGuideLine_Draw(void* arg) {
    ((S80178338*)arg)->render();
}

struct S801788D0 { char padding000[0x20]; int count; char padding024[0x0C]; unsigned char enabled; };
struct Arr3f { float v[3]; };
// Append a ground-clamped center and two ribbon edges, then attach a delayed segment effect.
void CGuideLine_AppendGroundSegment(void* arg0, void* segmentStep) {
    // segmentStep is a borrowed world-space displacement. First center copies
    // ball XYZ; later centers query ground and add the ball radius along +Y.
    // Edges = center +/- normalize(step cross up)*radius, then subtract radius
    // from edge Y so the ribbon rests on the ground rather than at ball height.
    S801788D0* self = (S801788D0*)arg0;
    unsigned int halfWidthBits[3];
    float groundCenter[3];
    float groundPoint[3];
    unsigned int queryPositionBits[3];
    float nextCenter[3];
    float radiusOffset[3];
    float lateralDirection[3];
    float upVector[3];
    float leftEdge[3];
    float rightEdge[3];
    if (self->count < kGuideSegmentLimit) {
        void* scene;
        if (self->count == 0) {
            void* a = GolfBall__Fv();
            __as__7WVectorFRC7WVector((char*)self + self->count * kGuideSegmentStride + kGuideCenterOffset, a);
        } else {
            scene = GolfBall__Fv();
            radiusOffset[1] = ((CGolfBall*)scene)->m_radius;
            radiusOffset[0] = kFloat_0_805DECF4;
            radiusOffset[2] = kFloat_0_805DECF4;
            __pl__FRC7WVectorRC7WVector(nextCenter, (char*)self + (self->count - 1) * kGuideSegmentStride + kGuideCenterOffset, segmentStep);
            queryPositionBits[0] = ((unsigned int*)nextCenter)[0];
            queryPositionBits[1] = ((unsigned int*)nextCenter)[1];
            queryPositionBits[2] = ((unsigned int*)nextCenter)[2];
            void* quadTree = CQuadTree_GetInstance();
            GetGroundPoint__9CQuadTreeF7WVectorbPib(groundPoint, quadTree, queryPositionBits, 0, 0, 0);
            __pl__FRC7WVectorRC7WVector(groundCenter, groundPoint, radiusOffset);
            __as__7WVectorFRC7WVector((char*)self + self->count * kGuideSegmentStride + kGuideCenterOffset, groundCenter);
        }
        upVector[0] = kFloat_0_805DECF4;
        upVector[1] = kFloat_1_PvsEffects;
        upVector[2] = kFloat_0_805DECF4;
        Vector_CrossProduct(lateralDirection, segmentStep, upVector);
        void* ptr = Normalize__7WVectorFv(lateralDirection);
        *(Arr3f*)halfWidthBits = *(Arr3f*)ptr;
        float scale = ((CGolfBall*)GolfBall__Fv())->m_radius;
        __amu__7WVectorFf((float*)halfWidthBits, scale);
        __pl__FRC7WVectorRC7WVector(leftEdge, (char*)self + self->count * kGuideSegmentStride + kGuideCenterOffset, halfWidthBits);
        __as__7WVectorFRC7WVector((char*)self + self->count * kGuideSegmentStride + kGuideLeftOffset, leftEdge);
        __mi__FRC7WVectorRC7WVector(rightEdge, (char*)self + self->count * kGuideSegmentStride + kGuideCenterOffset, halfWidthBits);
        __as__7WVectorFRC7WVector((char*)self + self->count * kGuideSegmentStride + kGuideRightOffset, rightEdge);
        scene = GolfBall__Fv();
        float edgeHeight = *(float*)((char*)self + self->count * kGuideSegmentStride + kGuideLeftHeightOffset);
        float ballRadius = ((CGolfBall*)scene)->m_radius;
        *(float*)((char*)self + self->count * kGuideSegmentStride + kGuideLeftHeightOffset) = edgeHeight - ballRadius;
        scene = GolfBall__Fv();
        edgeHeight = *(float*)((char*)self + self->count * kGuideSegmentStride + kGuideRightHeightOffset);
        ballRadius = ((CGolfBall*)scene)->m_radius;
        *(float*)((char*)self + self->count * kGuideSegmentStride + kGuideRightHeightOffset) = edgeHeight - ballRadius;
        *(void**)((char*)self + self->count * kGuideSegmentStride + kGuideEffectOffset) = CFx_CreateSequence(g_fx, (char*)&((GuideTrackStrings*)guideLineTrackStrings)->groundSequence, 1);
        __as__7WVectorFRC7WVector((char*)*(void**)((char*)self + self->count * kGuideSegmentStride + kGuideEffectOffset) + 0x84, (char*)self + self->count * kGuideSegmentStride + kGuideCenterOffset);
        if (self->enabled) {
            void* effect = *(void**)((char*)self + self->count * kGuideSegmentStride + kGuideEffectOffset);
            ((GuideFxView*)effect)->startDelay = kFloat_50_GuideLineTrack * (float)self->count;
            if (((GuideFxView*)effect)->startDelay > 0.0f && ((GuideFxView*)effect)->active)
                ((GuideFxView*)effect)->active = 0;
        }
        ++self->count;
    }
}

} // extern "C"

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU80462140", _ctors_u80178178_0, GuideLine_InitFullTurnAngle);
#pragma pop

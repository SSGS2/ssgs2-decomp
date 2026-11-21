enum TomahawkState { kTomahawkWaitingForMotion = 0, kTomahawkRising = 1, kTomahawkFalling = 2, kTomahawkImpact = 3 };
enum SpikeState { kSpikeLaunch = 0, kSpikeWaitingForAscent = 1, kSpikeWaitingForBeam = 2, kSpikeBeamActive = 3, kSpikeImpact = 4 };
enum { kSpikeBeamDelayFrames = 60, kTrajectoryImpactMessage = 9, kBallSpikeFlightFlag = 0x40 };
// Ball event entries hold PreData sample indices, not positions or event IDs.
// Slot 0 is the current sample; slot 2 gates Spike's ascent/beam timing; positive
// slots 11/14 terminate Tomahawk/Spike. Exact event names for 2/11/14 are unresolved.
enum ShotEffectEventSlot {
    kCurrentShotSample = 0, kSpikeTimingSample = 2,
    kTomahawkEndSample = 11, kSpikeEndSample = 14
};
// Surface names below describe the resource family selected HERE, not a complete
// global ground-type taxonomy. Codes 3/6 select sand, 5/7 path, 8 water; impact
// additionally handles 0 tee, 4 grass, 10 default-sand and 12 water-bunker.
enum ShotEffectSurface {
    kEffectTee = 0, kEffectSand3 = 3, kEffectGrass4 = 4,
    kEffectPath5 = 5, kEffectSand6 = 6, kEffectPath7 = 7,
    kEffectWater8 = 8, kEffectDefaultSand10 = 10, kEffectWaterBunker12 = 12
};
enum EffectTreeTag { kTreeParentTag = 1 };

// Address-preserving view of the existing resource string pool.
struct ShotEffectResourceStrings {
    char spr[4]; // +0x0
    char GroundRun_seq[14]; // +0x4
    char WaterRun_seq[13]; // +0x12
    char Spike_C_seq[12]; // +0x1F
    char Spike_ball_seq[15]; // +0x2B
    char Twist_seq[10]; // +0x3A
    char party_bln_bom03_seq[20]; // +0x44
    char Cobra_BG_seq[13]; // +0x58
    char Cobra_BW_seq[13]; // +0x65
    char Cobra_BR_seq[13]; // +0x72
    char Cobra_C_seq[12]; // +0x7F
    char Cobra_A_seq[12]; // +0x8B
    char Tomahawk_A_seq[15]; // +0x97
    char Tomahawk_B_seq[15]; // +0xA6
    char Tomahawk_C_seq[15]; // +0xB5
    char Tomahawk_D_seq[15]; // +0xC4
    char tomahawkImpactSound[18]; // +0xD3
    char trajectoryActor[11]; // +0xE5
    char powerSpinSandFormat[19]; // +0xF0
    char powerSpinPathFormat[19]; // +0x103
    char powerSpinWaterFormat[19]; // +0x116
    char powerSpinGrassFormat[19]; // +0x129
    char powerSpinFallback[21]; // +0x13C
    char impactDefaultSand[22]; // +0x151
    char impactSandFormat[20]; // +0x167
    char impactPath[17]; // +0x17B
    char impactGrassFormat[20]; // +0x18C
    char impactTee[17]; // +0x1A0
    char impactWater[17]; // +0x1B1
    char impactWaterBunkerFormat[21]; // +0x1C2
    char Spike_A_seq[12]; // +0x1D7
    char spike_shot_01_seq[18]; // +0x1E3
    char spike_ball_duration[20]; // +0x1F5
    char Spike_shot_seq[15]; // +0x209
    char spike_beam[11]; // +0x218
    char woodwave2_seq[14]; // +0x223
    char powerShotLoadError[31]; // +0x231
    char errorTitle[6]; // +0x250
    char boostergate[12]; // +0x256
    char treeLengthError[26]; // +0x262
    char lengthErrorTypeName[55]; // +0x27C
    char padding[5]; // +0x2B3
};
#include "game/golfball.h"
#include <decomp/record_sizes.h>
#include "iff/IffStruct.h"
#include "classes/CSharedDoc.h"
#define AsCSharedDoc(p) ((CSharedDoc*)(p))
#include "game/types.h"
#include "game/wvector.h"
#include "sdk_protos.h"
#include "classes/CFxSpike.h"
#include "classes/CFxSequence.h"
#include "classes/CFxTomahawk.h"
#include "classes/CFxShotImpact.h"
#include "classes/CFxBoosterWarp.h"
#include "classes/CFxPowerSpin.h"
#include "classes/CFxPowerShot2.h"

#define AsCFxPowerShot2(p) ((CFxPowerShot2*)(p))

#define AsCFxPowerSpin(p) ((CFxPowerSpin*)(p))

#define AsCFxBoosterWarp(p) ((CFxBoosterWarp*)(p))

#define AsCFxShotImpact(p) ((CFxShotImpact*)(p))

#define AsCFxTomahawk(p) ((CFxTomahawk*)(p))

#define AsCFxSpike(p) ((CFxSpike*)(p))
// Translation unit 0x80105A48-0x80107180 (14 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

#pragma use_lmw_stmw on

// Construct a tree payload in its already allocated node.
inline void* operator new(size_t, void* p) { return p; }

extern "C" {

void __dl__FPv(void*);

extern char __vt__14CFxBoosterWarp[0x10];
extern char __vt__8CFxSpike[];
extern char __vt__13CFxShotImpact[];
extern char __vt__12CFxPowerSpin[];
extern ShotEffectResourceStrings shotEffectResourceStrings;
struct EffectCameraTransform { float right[3], up[3], forward[3], position[3]; };
extern EffectCameraTransform s_cameraTransform;
extern char __vt__Q23std11logic_error[];
extern char s_sharedCharDeleterVtable[];
extern char __vt__Q23std12length_error[];
extern CSharedDoc* g_sharedDoc;
extern int s_fxInstanceSerial;
extern float kPowerShot2HeightOffset;
extern float kFloat_0_5_FxShot;
extern float kFloat_0_shotEffects;
extern float kFloat_Neg1_0;
extern float kFloat_6_2831855_ShotEffects_805DE5CC;
extern float kFloat_10000_ShotEffects;
extern float kFloat_1_FxShot;
extern float kTomahawkDescentVelocityThreshold;
extern float kFloat_200_FxShot;
extern void* g_legacyWindowContext;
extern void* g_soundManager;
extern void* g_taskManager;

void* CFxBox_RemoveSequence(void*, void*, int);
void* CFxSequence_Create(void*, const char*, int, int, int);
void* CFxBox_RegisterSequence(void*, void*);
void* GetGroundPoint__9CQuadTreeF7WVectorbPib(void*, void*, void*, int, int, int);
void Sound_PlayByName(void*, const char*, int, int, int, int, float, float, float);
void* TaskActorTable_FindByName(void*, const char*, int*);
void* Object_GetChild(void*);
void* PreData__9CGolfBallFi(void*, int);
void* Normalize__7WVectorFv(void*);
// These signatures match the call ABI used below: float dot, double acos.
float Vec3_DotProduct(void*, void*);
double acos(double);
void Matrix_SetNegativeAxisAngleRotation(void*, void*, float);
void Matrix_TransformPoint(void*, void*, void*);
void* appMallocDummy__FUl(unsigned int);
void RBTree_RebalanceAfterInsert(void*, void*);
void appFreeUni__FPv(void*);
void* __nw__FUl(unsigned int);
void* __nwa__FUl(unsigned int);
void __dla__FPv(void*);
void __dt__Q23std12length_errorFv(void*);

void* CQuadTree_GetInstance(void*);
unsigned char GetGroundType__9CQuadTreeF7WVector(void*, void*);
void __mi__FRC7WVectorRC7WVector(void*, const char*, const char*);
void* __ml__FRC7WVectorf(void*, void*, float);
void __as__7WVectorFRC7WVector(void*, void*);
float Magnitude__7WVectorCFv(void*);
void __as__7WVectorFRC7WVector(void*, void*);
void MessageBox_Report(void*, const char*, const char*, int);
void __mi__FRC7WVectorRC7WVector(void*, const char*, const char*);
void Sound_PlayByName(void*, const char*, int, int, int, int, float, float, float);

// Position components are copied as integer words to preserve their float bit patterns.
struct EffectBallPositionWords { void* x; void* y; void* z; };

// Shared destructor view: the two child slots begin at +0x10.
struct ObjectWithChildren {
    void* vtable;
    void* instanceIdBits;
    void* owner;
    void* golfBall;
    void* child0;
    void* child1;
};

typedef Vec3f Vec3;

// Advance Tomahawk launch, ascent, descent, and impact effects and optionally notify the trajectory actor.
// Return 1 to keep updating, 0 after impact or sequence creation failure. Ball
// positions, velocity and terrain query results use world space; owner/ball are borrowed.
int CFxTomahawk_AdvancePhase(void* self) {
    CFxTomahawk* effect = (CFxTomahawk*)self; 
    Vec3 bounds;

    void* b = ((EffectBallPositionWords*)effect->ball)->y;
    void* a = ((EffectBallPositionWords*)effect->ball)->x;
    void* c = ((EffectBallPositionWords*)effect->ball)->z;
    void* vals[3] = { a, b, c };

    volatile void* vals2[3] = { a, b, c };
    if (0 != vals2[0]) { (void)vals2[1]; }
    void* manager = CQuadTree_GetInstance(c);
    GetGroundPoint__9CQuadTreeF7WVectorbPib(&bounds, manager, vals, 1, 0, 1);
    volatile float frameY;
    frameY = bounds.y;
    int limit = ((CGolfBall*)effect->ball)->m_eventPreDataIndex[kTomahawkEndSample];
    (void)frameY;
    if (limit > 0 && ((CGolfBall*)effect->ball)->m_eventPreDataIndex[kCurrentShotSample] >= limit) effect->state = kTomahawkImpact;
    switch (effect->state) {
    case kTomahawkWaitingForMotion: {
        if ((((CGolfBall*)effect->ball)->m_velocity).Magnitude() > kFloat_1_FxShot) {
            effect->state = kTomahawkRising;
            void* child = CFxSequence_Create(effect->sequenceOwner, shotEffectResourceStrings.Tomahawk_A_seq, 1, 0, 0);
            if (!child) return 0;
            CFxBox_RegisterSequence(effect->sequenceOwner, child);
            ((CFxSequence*)child)->worldPosition = ((CGolfBall*)effect->ball)->m_pos;
            child = CFxSequence_Create(effect->sequenceOwner, shotEffectResourceStrings.Tomahawk_B_seq, 0, 0, 0);
            effect->sequence = child;
            if (!child) return 0;
            CFxBox_RegisterSequence(effect->sequenceOwner, child);
        }
        break;
    }
    case kTomahawkRising: {
        if (((CGolfBall*)effect->ball)->m_velocity.y < kTomahawkDescentVelocityThreshold) {
            effect->state = kTomahawkFalling;
            void* child = effect->sequence;
            if (child) {
                CFxBox_RemoveSequence(effect->sequenceOwner, child, 0);
                child = effect->sequence;
                if (child) ((void (**)(void*, int))(*(void**)child))[2](child, 1);
                effect->sequence = 0;
            }
            child = CFxSequence_Create(effect->sequenceOwner, shotEffectResourceStrings.Tomahawk_C_seq, 0, 0, 0);
            effect->sequence = child;
            if (!child) return 0;
            CFxBox_RegisterSequence(effect->sequenceOwner, child);
        }
        break;
    }
    case kTomahawkImpact: {
        void* child = effect->sequence;
        if (child) {
            CFxBox_RemoveSequence(effect->sequenceOwner, child, 0);
            child = effect->sequence;
            if (child) ((void (**)(void*, int))(*(void**)child))[2](child, 1);
            effect->sequence = 0;
        }
        child = CFxSequence_Create(effect->sequenceOwner, shotEffectResourceStrings.Tomahawk_D_seq, 1, 0, 0);
        if (child == 0) return 0;
        CFxBox_RegisterSequence(effect->sequenceOwner, child);
        ((CFxSequence*)child)->worldPosition = ((CGolfBall*)effect->ball)->m_pos;
        Sound_PlayByName(g_soundManager, shotEffectResourceStrings.tomahawkImpactSound, 0, 0, 0, 0, kFloat_0_5_FxShot, kFloat_200_FxShot, kFloat_1_FxShot);
        if (effect->notifyTrajectoryOnImpact) { 
            void* mgr = g_taskManager;
            void* arg = mgr ? Object_GetChild(mgr) : 0;

            int p[8]; p[0] = 0; p[1] = kTrajectoryImpactMessage; p[2] = 0; p[3] = 0; p[4] = 0; p[5] = 0; p[6] = 0; p[7] = 0;
            void* obj = TaskActorTable_FindByName(arg, shotEffectResourceStrings.trajectoryActor, 0);
            if (obj) ((void (**)(void*, int*))(*(void**)obj))[11](obj, p);
        }
        return 0;
    }
    default:
        break;
    }
    {
        void* child = effect->sequence;
        if (child) ((CFxSequence*)child)->worldPosition = ((CGolfBall*)effect->ball)->m_pos;
    }
    return 1;
}

#pragma opt_common_subs off
#pragma opt_propagation off
#pragma opt_lifetimes off
struct Vt11 { 
virtual void _0(); virtual void _1(); virtual void _2(); virtual void _3(); virtual void _4(); virtual void _5(); virtual void _6(); virtual void _7(); virtual void _8(); virtual void _9(); virtual void _10(); virtual void m(int*);
};
// Advance Spike launch and beam timing, follow the ball, and create the final impact effect.
// Return 1 while active; 0 on impact, missing flight flag, or a checked creation
// failure. Delay 60 is in trajectory sample/frame indices, not elapsed seconds.
int CFxSpike_UpdateAndAdvanceState(void* self) {
    CFxSpike* effect = (CFxSpike*)self;
    void* values[3];
    values[2] = ((EffectBallPositionWords*)effect->ball)->z;
    values[0] = ((EffectBallPositionWords*)effect->ball)->x;
    values[1] = ((EffectBallPositionWords*)effect->ball)->y;
    Vec3 bounds;
    volatile float frameY;
    GetGroundPoint__9CQuadTreeF7WVectorbPib(&bounds, CQuadTree_GetInstance(values[2]), values, 1, 0, 1);
    frameY = bounds.y;
    (void)frameY;
    if (0 < ((CGolfBall*)effect->ball)->m_eventPreDataIndex[kSpikeEndSample] && ((CGolfBall*)effect->ball)->m_eventPreDataIndex[kCurrentShotSample] >= ((CGolfBall*)effect->ball)->m_eventPreDataIndex[kSpikeEndSample]) effect->state = kSpikeImpact;

    switch (effect->state) {
    case kSpikeLaunch: {
        CFxBox_RegisterSequence(effect->effectManager, effect->initialSequence);
        ((CFxSequence*)effect->initialSequence)->worldPosition = ((CGolfBall*)effect->ball)->m_pos;
        void* child = CFxSequence_Create(effect->effectManager, shotEffectResourceStrings.Spike_A_seq, 1, 0, 0);
        if (!child) return 0;
        CFxBox_RegisterSequence(effect->effectManager, child);
        ((CFxSequence*)child)->worldPosition = ((CGolfBall*)effect->ball)->m_pos;
        effect->state = kSpikeWaitingForAscent;
        break;
    }
    case kSpikeWaitingForAscent: {
        if (((CGolfBall*)effect->ball)->m_eventPreDataIndex[kSpikeTimingSample] < ((CGolfBall*)effect->ball)->m_eventPreDataIndex[kCurrentShotSample]) {
            void* child = CFxSequence_Create(effect->effectManager, shotEffectResourceStrings.spike_shot_01_seq, 1, 0, 0);
            if (!child) return 0;
            CFxBox_RegisterSequence(effect->effectManager, child);
            ((CFxSequence*)child)->worldPosition = ((CGolfBall*)effect->ball)->m_pos;
            Sound_PlayByName(g_soundManager, shotEffectResourceStrings.spike_ball_duration, 0, 0, 0, 0, kFloat_0_5_FxShot, kFloat_200_FxShot, kFloat_1_FxShot);
            effect->state = kSpikeWaitingForBeam;
        } else {
            ((CFxSequence*)effect->initialSequence)->worldPosition = ((CGolfBall*)effect->ball)->m_pos;
        }
        if (!((*(unsigned int*)&((CGolfBall*)effect->ball)->m_flags) & kBallSpikeFlightFlag)) return 0;
        break;
    }
    case kSpikeWaitingForBeam: {
        if (((CGolfBall*)effect->ball)->m_eventPreDataIndex[kSpikeTimingSample] + kSpikeBeamDelayFrames < ((CGolfBall*)effect->ball)->m_eventPreDataIndex[kCurrentShotSample]) {
            void* child = CFxSequence_Create(effect->effectManager, shotEffectResourceStrings.Spike_shot_seq, 1, 0, 0);
            CFxBox_RegisterSequence(effect->effectManager, child);
            ((CFxSequence*)child)->worldPosition = ((CGolfBall*)effect->ball)->m_pos;
            CFxBox_RegisterSequence(effect->effectManager, effect->delayedSequence);
            ((CFxSequence*)effect->delayedSequence)->worldPosition = ((CGolfBall*)effect->ball)->m_pos;
            void* obj = effect->initialSequence;
            if ((*(unsigned char*)&((CFxSequence*)obj)->playbackEnabled)) (*(unsigned char*)&((CFxSequence*)obj)->playbackEnabled) = 0;
            Sound_PlayByName(g_soundManager, shotEffectResourceStrings.spike_beam, 0, 0, 0, 0, kFloat_0_5_FxShot, kFloat_200_FxShot, kFloat_1_FxShot);
            effect->state = kSpikeBeamActive;
        }
        break;
    }
    case kSpikeBeamActive:
        ((CFxSequence*)effect->delayedSequence)->worldPosition = ((CGolfBall*)effect->ball)->m_pos;
        break;
    case kSpikeImpact: {
        void* child = CFxSequence_Create(effect->effectManager, shotEffectResourceStrings.Tomahawk_D_seq, 1, 0, 0);
        if (0 == child) return 0;
        CFxBox_RegisterSequence(effect->effectManager, child);
        ((CFxSequence*)child)->worldPosition = ((CGolfBall*)effect->ball)->m_pos;
        void* obj = effect->delayedSequence;
        if ((*(unsigned char*)&((CFxSequence*)obj)->playbackEnabled)) (*(unsigned char*)&((CFxSequence*)obj)->playbackEnabled) = 0;
        Sound_PlayByName(g_soundManager, shotEffectResourceStrings.tomahawkImpactSound, 0, 0, 0, 0, kFloat_0_5_FxShot, kFloat_200_FxShot, kFloat_1_FxShot);
        if (effect->notifyTrajectoryOnImpact) {
            void* mgr = g_taskManager;
            void* arg = mgr ? Object_GetChild(mgr) : 0;
            int p[8];
            p[0] = 0; p[1] = kTrajectoryImpactMessage; p[2] = 0; p[3] = 0; p[4] = 0; p[5] = 0; p[6] = 0; p[7] = 0;
            void* obj2 = TaskActorTable_FindByName(arg, shotEffectResourceStrings.trajectoryActor, 0);
            if (obj2) {
                void** vt = *(void***)obj2;
                ((void(*)(void*, int*))vt[11])(obj2, p);
            }
        }
        return 0;
    }
    default:
        break;
    }
    return 1;
}
#pragma opt_common_subs reset
#pragma opt_propagation reset
#pragma opt_lifetimes reset
struct TreeNode { TreeNode* left; TreeNode* right; TreeNode* parentColor; unsigned int key; };
struct TreeHead { TreeNode* root; };

// Choose a course/surface-specific Power Spin sequence, register it, and assign an instance serial.
void* CFxPowerSpin_Ctor(void* self, void* owner, void* input) { AsCFxPowerSpin(self)->owner = owner; AsCFxPowerSpin(self)->vtable = __vt__12CFxPowerSpin;  AsCFxPowerSpin(self)->golfBall = input;  char text[32]; 
    AsCFxPowerSpin(self)->particlesOriented = 0;

    struct Values { void* p[3]; } values = *(Values*)input;
    void* manager = CQuadTree_GetInstance(values.p[0]);

    switch ((GetGroundType__9CQuadTreeF7WVector(manager, &values))) {
    case kEffectSand3: case kEffectSand6:
        sprintf(text, shotEffectResourceStrings.powerSpinSandFormat,
            AsCSharedDoc(g_sharedDoc)->selectedCourseData ? ((IFF_STRUCT::sCourse*)AsCSharedDoc(g_sharedDoc)->selectedCourseData)->mapName : 0); break;
    case kEffectPath5: case kEffectPath7:
        sprintf(text, shotEffectResourceStrings.powerSpinPathFormat,
            AsCSharedDoc(g_sharedDoc)->selectedCourseData ? ((IFF_STRUCT::sCourse*)AsCSharedDoc(g_sharedDoc)->selectedCourseData)->mapName : 0); break;
    case kEffectWater8:
        sprintf(text, shotEffectResourceStrings.powerSpinWaterFormat,
            AsCSharedDoc(g_sharedDoc)->selectedCourseData ? ((IFF_STRUCT::sCourse*)AsCSharedDoc(g_sharedDoc)->selectedCourseData)->mapName : 0); break;
    default:
        sprintf(text, shotEffectResourceStrings.powerSpinGrassFormat,
            AsCSharedDoc(g_sharedDoc)->selectedCourseData ? ((IFF_STRUCT::sCourse*)AsCSharedDoc(g_sharedDoc)->selectedCourseData)->mapName : 0); break;
    }
    void* child = CFxSequence_Create(AsCFxPowerSpin(self)->owner, text, 1, 0, 0);
    AsCFxPowerSpin(self)->sequence = child;
    if (!child) AsCFxPowerSpin(self)->sequence = CFxSequence_Create(AsCFxPowerSpin(self)->owner, shotEffectResourceStrings.powerSpinFallback, 1, 0, 0);
    if (AsCFxPowerSpin(self)->sequence) CFxBox_RegisterSequence(AsCFxPowerSpin(self)->owner, AsCFxPowerSpin(self)->sequence);
    int nextInstanceId = s_fxInstanceSerial + 1;
    AsCFxPowerSpin(self)->instanceId = s_fxInstanceSerial;
    s_fxInstanceSerial = nextInstanceId;
    return self;
}

struct ShotTrajectorySample { Vec3 position; Vec3 velocity; };

struct SpinParticle {
    char pad0[0xB8]; Vec3 position;
    char padC4[0x168]; Vec3 direction; Vec3 velocity;
};
struct SpinNode {
    SpinNode* left;
    SpinNode* right;
    unsigned int parentColor;
    unsigned int key;
    SpinParticle* item;
};
// Tree invariants: sentinel.left is the search root; sentinel.right is the first
// node for traversal; &sentinel marks end. Null child links end a descent. Parent
// addresses mask out bit 0 before dereference. Manager membership is keyed by the
// sequence pointer's 32-bit value; a missing key means the sequence has expired.
struct SpinSequence {
    char pad0[0x20]; unsigned int count;
    SpinNode sentinel;
};
struct SpinOwner { char pad0[0x164]; SpinNode sentinel; };
struct SpinBall { char pad0[0x48]; unsigned int flags; char pad4C[0x1C]; int frame; };
struct SpinControl {
    void* vtable; int serial; SpinOwner* owner; SpinBall* ball;
    SpinSequence* sequence; unsigned char orientationApplied;
};
// SpinControl.orientationApplied aliases the controllers' one-time orientation byte. It is
// set only after a nonempty particle set is rotated; empty sets retry later.
// Power Spin subsequently follows ball position, while ShotImpact only rotates.
struct SpinKey { unsigned int value; };

// Orient spin particles once along the shot direction, then keep their positions attached to the ball.
int CFxPowerSpin_OrientAndAttachParticles(SpinControl* self) {
    // Phase 1: confirm manager membership before dereferencing the sequence.
    SpinSequence* data;
    SpinNode* cur;
    SpinNode* end;
    SpinOwner* manager = self->owner;
    SpinKey key = (*(SpinKey*)&self->sequence);
    SpinNode* node = manager->sentinel.left;
    SpinNode* found = &manager->sentinel;
    while (node) {
        if (node->key >= key.value) { found = node; node = node->left; }
        else node = node->right;
    }
    if (found == &manager->sentinel || key.value < found->key) found = &manager->sentinel;
    if (found == &manager->sentinel) return 0;
    data = self->sequence;
    if (!self->orientationApplied) {
        // Phase 2: derive horizontal world-space yaw from initial/current samples,
        // rotating local -Z onto the shot direction about world +Y (radians).
        if (!data->count) return 1;
        SpinBall* context = self->ball;
        cur = (SpinNode*)PreData__9CGolfBallFi(context, 0);
        Vec3* v2 = (Vec3*)PreData__9CGolfBallFi(context, context->frame);
        Vec3 p;
        __mi__FRC7WVectorRC7WVector(&p, (char*)v2, (char*)cur);
        p.y = kFloat_0_shotEffects;
        Vec3 transformed;
        void* normalized;
        if (self->ball->flags & 1) {
            void* rotated = Normalize__7WVectorFv(&p);
            __ml__FRC7WVectorf(&transformed, rotated, kFloat_Neg1_0);
            normalized = &transformed;
        } else {
            normalized = Normalize__7WVectorFv(&p);
        }
        __as__7WVectorFRC7WVector(&p, normalized);
        Vec3 dir;
        dir.x = kFloat_0_shotEffects;
        dir.y = kFloat_0_shotEffects;
        dir.z = kFloat_Neg1_0;
        float dot = Vec3_DotProduct(&p, &dir);
        float a = (float)acos(dot);
        if (p.x < kFloat_0_shotEffects) a = kFloat_6_2831855_ShotEffects_805DE5CC - a;
        Vec3 scale;
        scale.x = kFloat_0_shotEffects;
        scale.y = kFloat_1_FxShot;
        scale.z = kFloat_0_shotEffects;
        char matrix[36];
        Matrix_SetNegativeAxisAngleRotation(matrix, &scale, a);
        // Rotate velocity and direction once; tree traversal never owns particles.
        cur = data->sentinel.right;
        end = &data->sentinel;
        while (cur != end) {
            SpinParticle* item = cur->item;
            char t1[12], t2[12];
            Matrix_TransformPoint(t1, &item->velocity, matrix);
            __as__7WVectorFRC7WVector(&item->velocity, t1);
            Matrix_TransformPoint(t2, &item->direction, matrix);
            __as__7WVectorFRC7WVector(&item->direction, t2);
            SpinNode* r = cur->right;
            if (r) {
                while (r->left) r = r->left;
                cur = r;
            } else {
                SpinNode* p;
                while (cur != (p = (SpinNode*)(cur->parentColor & ~kTreeParentTag))->left) {
                    cur = p;
                }
                cur = p;
            }
        }
        self->orientationApplied = 1;
    } else {
        // Phase 3 on later updates: attach every existing particle to ball position.
        cur = data->sentinel.right;
        end = &data->sentinel;
        while (cur != end) {
            __as__7WVectorFRC7WVector(&cur->item->position, self->ball);
            SpinNode* r = cur->right;
            if (r) {
                while (r->left) r = r->left;
                cur = r;
            } else {
                SpinNode* p;
                while (cur != (p = (SpinNode*)(cur->parentColor & ~kTreeParentTag))->left) {
                    cur = p;
                }
                cur = p;
            }
        }
    }
    return 1;
}


struct SearchObject { char pad[8]; void* manager; void* context; unsigned int key; unsigned char unk_14; };

struct InsertHead { unsigned int count; void* sentinelParent; void* sentinelChild; };
// Copy the final payload word while retaining the original exception boundary.
inline void InsertCopyLast(unsigned int* destination, unsigned int value) {
    try { *destination = value; } catch (...) { throw; }
}
// The 44-byte tree payload has no established semantic layout; retain its original copy widths.
struct InsertInput {
    unsigned int word; float x, y; unsigned int fields[8];
    // Preserve ordered payload loads/stores and the final-word exception boundary.
    InsertInput(const InsertInput& other) {
        unsigned int f0;
        volatile InsertInput* destination = this;
        const volatile InsertInput* source = &other;
        unsigned int wordValue = source->word;
        float fx = source->x;
        destination->word = wordValue;
        float fy = source->y;
        destination->x = fx;
        f0 = source->fields[0];
        destination->y = fy;
        unsigned int f1 = source->fields[1];
        destination->fields[0] = f0;
        unsigned int f2 = source->fields[2];
        destination->fields[1] = f1;
        unsigned int f3 = source->fields[3];
        destination->fields[2] = f2;
        unsigned int f4 = source->fields[4];
        destination->fields[3] = f3;
        unsigned int f5 = source->fields[5];
        destination->fields[4] = f4;
        unsigned int f6 = source->fields[6];
        destination->fields[5] = f5;
        unsigned int f7 = source->fields[7];
        destination->fields[6] = f6;
        InsertCopyLast(&fields[7], f7);
    }
};
struct InsertNode { InsertNode* left; InsertNode* right; InsertNode* parent; InsertInput input; };
struct InsertParent {
    InsertNode* value;
    // Construct the parent subobject with the original exception boundary.
    InsertParent(InsertNode* parent) {
        try { value = parent; } catch (...) { throw; }
    }
};

struct InsertBufferGuard {
    char* buffer;
    unsigned char* allocator;
    // Delete the exception message unless its control block took ownership.
    ~InsertBufferGuard() { if (buffer) __dla__FPv(buffer); }
};
struct InsertAllocator { unsigned char value; char padding[3]; };
struct InsertErrorBase {
    void* vtable;
    char* text;
    void* reference;
    static void initialize(void*& vtable, char*& text, void*& reference,
        const char* message, InsertAllocator* original, InsertAllocator* copy);
};
struct InsertErrorReference {
    void* vtable;
    int strong;
    int weak;
    char* buffer;
    // Own the copied exception message with one strong and one weak reference.
    InsertErrorReference(char* p) {
        strong = 1;
        weak = 1;
        vtable = &s_sharedCharDeleterVtable;
        buffer = p;
    }
};
// Allocate and copy an exception message, guarded until its reference block owns it.
inline void InsertErrorBase::initialize(void*& vtable, char*& text, void*& reference,
    const char* message, InsertAllocator* original, InsertAllocator* copy) {
    vtable = &__vt__Q23std11logic_error;
    *(volatile unsigned char*)&original->value = 0;
    copy->value = 0;
    unsigned int length = strlen(message) + 1;
    char* buffer = (char*)__nwa__FUl(length * 2 - length);
    text = buffer;
    {
        InsertBufferGuard guard;
        guard.buffer = buffer;
        guard.allocator = &copy->value;
        reference = new InsertErrorReference(buffer);
        guard.buffer = 0;
    }
    strcpy(text, message);
}
struct InsertAllocationGuard {
    void* allocator;
    void* volatile node;
    // Free a tree node if payload construction exits through an exception.
    ~InsertAllocationGuard() { void* p = node; if (p) appFreeUni__FPv(p); }
};

// Allocate and copy one effect-tree payload, link its node, and rebalance the red-black tree.
InsertNode* RBTree_InsertNode_Large(InsertHead* self, InsertNode* slot, int left, int setRoot, InsertInput* input) {
    // Phase 1: capacity check and guarded allocation. input is borrowed; the new
    // node owns its copied payload. slot is an existing parent (possibly header).
    InsertNode* volatile constructed;
    InsertNode* volatile allocated;
    volatile InsertAllocator allocator;
    InsertAllocator originalAllocator;
    InsertAllocator copyAllocator;
    if (self->count == 0xFFFFFFFFU) {
        const char* message = shotEffectResourceStrings.treeLengthError;
        InsertErrorBase error;
        InsertErrorBase* errorPointer = &error;
        InsertErrorBase::initialize(errorPointer->vtable, error.text, error.reference,
            message, &originalAllocator, &copyAllocator);
        errorPointer->vtable = &__vt__Q23std12length_error;
        __throw(shotEffectResourceStrings.lengthErrorTypeName, errorPointer, (void*)__dt__Q23std12length_errorFv);
    }
    allocator.value = 0;
    InsertNode* const node = (InsertNode*)appMallocDummy__FUl(0x38);
    allocated = node;
    constructed = node;
    InsertAllocationGuard guard;
    guard.allocator = &self->sentinelParent;
    guard.node = node;
    new (&node->input) InsertInput(*input);
    // Phase 2: payload is constructed; transfer the node out of the unwind guard,
    // link under slot, update count, then rebalance without moving payload bytes.
    node->right = 0;
    guard.node = 0;
    node->left = 0;
    new (&node->parent) InsertParent(slot);
    if (left) slot->left = node;
    else slot->right = node;
    unsigned int count = self->count;
    self->count = count + 1;
    RBTree_RebalanceAfterInsert(node, self->sentinelParent);
    if (setRoot) self->sentinelChild = node;
    return node;
}

// Orient all impact particles once to the initial horizontal shot direction.
int CFxShotImpact_OrientParticlesToShotDirection(SpinControl* self) {
    // Membership and orientation-byte contracts are shared with Power Spin above.
    // Return 0 when the manager no longer contains the sequence, else 1.
    SpinNode* cur;
    SpinSequence* data;
    SpinNode* end;
    SpinOwner* manager = self->owner;
    SpinKey key = (*(SpinKey*)&self->sequence);
    SpinNode* node = manager->sentinel.left;
    SpinNode* found = &manager->sentinel;
    while (node) {
        if (node->key >= key.value) { found = node; node = node->left; }
        else node = node->right;
    }
    if (found == &manager->sentinel || key.value < found->key) found = &manager->sentinel;
    if (found == &manager->sentinel) return 0;
    data = self->sequence;
    if (!self->orientationApplied) {
    if (!data->count) return 1;
    Vec3* source = &((ShotTrajectorySample*)PreData__9CGolfBallFi(self->ball, 0))->velocity;
    Vec3 pos = *source;
    pos.y = kFloat_0_shotEffects;
    (*(WVector*)&pos).Normalize();
    Vec3 dir;
    dir.x = kFloat_0_shotEffects;
    dir.y = kFloat_0_shotEffects;
    dir.z = kFloat_Neg1_0;
    float dot = Vec3_DotProduct(&pos, &dir);
    float angle = (float)acos(dot);
    if (pos.x < kFloat_0_shotEffects) angle = kFloat_6_2831855_ShotEffects_805DE5CC - angle;
    double matrix[6];
    Vec3 scale;
    scale.x = kFloat_0_shotEffects;
    scale.y = kFloat_1_FxShot;
    scale.z = kFloat_0_shotEffects;
    Matrix_SetNegativeAxisAngleRotation(matrix, &scale, angle);
    cur = data->sentinel.right;
    end = &data->sentinel;
    while (cur != end) {
        SpinParticle* item = cur->item;
        WVector tmp1;
        WVector tmp2;
        Matrix_TransformPoint(&tmp1, &item->velocity, matrix);
        *(WVector*)(&item->velocity) = tmp1;
        Matrix_TransformPoint(&tmp2, &item->direction, matrix);
        *(WVector*)(&item->direction) = tmp2;
        SpinNode* right = cur->right;
        if (right) {
            while (right->left) right = right->left;
            cur = right;
        } else {
            SpinNode* p;
            while (cur != (p = (SpinNode*)(cur->parentColor & ~kTreeParentTag))->left) {
                cur = p;
            }
            cur = p;
        }
    }
    self->orientationApplied = 1;
    }
    return 1;
}

#pragma opt_common_subs off
// Choose and register an impact sequence from the ground type and course map name.
void* CFxShotImpact_Ctor(void* self, int arg1, void* input) {
    (*(int*)&AsCFxShotImpact(self)->sequenceOwner) = arg1;
    AsCFxShotImpact(self)->vtbl = (void*)__vt__13CFxShotImpact;
    AsCFxShotImpact(self)->ball = input;
    AsCFxShotImpact(self)->particlesAlignedToShot = 0;
    struct Values { void* p[3]; } values = *(Values*)input;
    unsigned char kind = GetGroundType__9CQuadTreeF7WVector(CQuadTree_GetInstance(values.p[0]), &values);
    char text[32];
    switch (kind) {
    case kEffectDefaultSand10: {
        void* filename = AsCSharedDoc(g_sharedDoc)->selectedCourseData;
        sprintf(text, shotEffectResourceStrings.impactDefaultSand, filename ? ((IFF_STRUCT::sCourse*)filename)->mapName : 0);
        AsCFxShotImpact(self)->sequence = CFxSequence_Create(AsCFxShotImpact(self)->sequenceOwner, text, 1, 0, 0);
        break;
    }
    case kEffectSand3: {
        void* filename = AsCSharedDoc(g_sharedDoc)->selectedCourseData;
        sprintf(text, shotEffectResourceStrings.impactSandFormat, filename ? ((IFF_STRUCT::sCourse*)filename)->mapName : 0);
        AsCFxShotImpact(self)->sequence = CFxSequence_Create(AsCFxShotImpact(self)->sequenceOwner, text, 1, 0, 0);
        break;
    }
    case kEffectPath5:
    case kEffectPath7:
        AsCFxShotImpact(self)->sequence = CFxSequence_Create(AsCFxShotImpact(self)->sequenceOwner, shotEffectResourceStrings.impactPath, 1, 0, 0);
        break;
    case kEffectGrass4: {
        void* filename = AsCSharedDoc(g_sharedDoc)->selectedCourseData;
        sprintf(text, shotEffectResourceStrings.impactGrassFormat, filename ? ((IFF_STRUCT::sCourse*)filename)->mapName : 0);
        AsCFxShotImpact(self)->sequence = CFxSequence_Create(AsCFxShotImpact(self)->sequenceOwner, text, 1, 0, 0);
        break;
    }
    case kEffectTee:
        AsCFxShotImpact(self)->sequence = CFxSequence_Create(AsCFxShotImpact(self)->sequenceOwner, shotEffectResourceStrings.impactTee, 1, 0, 0);
        break;
    case kEffectWater8:
        AsCFxShotImpact(self)->sequence = CFxSequence_Create(AsCFxShotImpact(self)->sequenceOwner, shotEffectResourceStrings.impactWater, 1, 0, 0);
        break;
    case kEffectWaterBunker12: {
        void* filename = AsCSharedDoc(g_sharedDoc)->selectedCourseData;
        sprintf(text, shotEffectResourceStrings.impactWaterBunkerFormat, filename ? ((IFF_STRUCT::sCourse*)filename)->mapName : 0);
        AsCFxShotImpact(self)->sequence = CFxSequence_Create(AsCFxShotImpact(self)->sequenceOwner, text, 1, 0, 0);
        break;
    }
    default:
        AsCFxShotImpact(self)->sequence = 0;
        break;
    }
    if (AsCFxShotImpact(self)->sequence) {
        CFxBox_RegisterSequence(AsCFxShotImpact(self)->sequenceOwner, AsCFxShotImpact(self)->sequence);
        void* destination = &((CFxSequence*)AsCFxShotImpact(self)->sequence)->worldPosition;
        void* pos = PreData__9CGolfBallFi(AsCFxShotImpact(self)->ball, 0);
        *(WVector*)destination = *(WVector*)pos;
    }
    int serial = s_fxInstanceSerial;
    AsCFxShotImpact(self)->serialNumber = serial;
    s_fxInstanceSerial = serial + 1;
    return self;
}
#pragma opt_common_subs reset
#pragma opt_propagation reset

#pragma opt_propagation off
// Start the warp sound and launch effects, then follow the ball until it slows down.
int CFxBoosterWarp_Update(void* self) {
    if (AsCFxBoosterWarp(self)->initialSetupComplete) {
        float t = (((CGolfBall*)AsCFxBoosterWarp(self)->ball)->m_velocity).Magnitude();
        if (t < kFloat_0_5_FxShot) {
            if ((*(unsigned char*)&((CFxSequence*)AsCFxBoosterWarp(self)->ballFollowSequence)->playbackEnabled)) (*(unsigned char*)&((CFxSequence*)AsCFxBoosterWarp(self)->ballFollowSequence)->playbackEnabled) = 0;
            if ((*(unsigned char*)&((CFxSequence*)AsCFxBoosterWarp(self)->launchSequence)->playbackEnabled)) (*(unsigned char*)&((CFxSequence*)AsCFxBoosterWarp(self)->launchSequence)->playbackEnabled) = 0;
        } else ((CFxSequence*)AsCFxBoosterWarp(self)->ballFollowSequence)->worldPosition = ((CGolfBall*)AsCFxBoosterWarp(self)->ball)->m_pos;
    } else {
        ((CFxSequence*)AsCFxBoosterWarp(self)->ballFollowSequence)->worldPosition = ((CGolfBall*)AsCFxBoosterWarp(self)->ball)->m_pos;
        if ((*(unsigned char*)&((CFxSequence*)AsCFxBoosterWarp(self)->launchSequence)->playbackEnabled) != 1) (*(unsigned char*)&((CFxSequence*)AsCFxBoosterWarp(self)->launchSequence)->playbackEnabled) = 1;
        ((CFxSequence*)AsCFxBoosterWarp(self)->launchSequence)->worldPosition = ((CGolfBall*)AsCFxBoosterWarp(self)->ball)->m_pos;
        if ((*(unsigned char*)&((CFxSequence*)AsCFxBoosterWarp(self)->launchSequence)->playbackEnabled) != 1) (*(unsigned char*)&((CFxSequence*)AsCFxBoosterWarp(self)->launchSequence)->playbackEnabled) = 1;
        char text[8];
        void* name = shotEffectResourceStrings.boostergate;
        __mi__FRC7WVectorRC7WVector((void*)text, (const char*)AsCFxBoosterWarp(self)->ball, (char*)s_cameraTransform.position);
        ((void (*)(void*, const char*, void*, int, int, int, float, float, float))Sound_PlayByName)(g_soundManager, (char*)name, (void*)text, 5, 0, 0, kFloat_0_shotEffects, kFloat_10000_ShotEffects, kFloat_1_FxShot);
        AsCFxBoosterWarp(self)->initialSetupComplete = 1;
    }
    return 1;
}
#pragma opt_propagation reset

class Dt0 { public: 
virtual void v08(int);
};
// Unregister and destroy both warp sequences, clear their slots, and optionally free the controller.
void* CFxBoosterWarp_dtor(ObjectWithChildren* self, int shouldDelete) {
    if (self != 0) {
        self->vtable = (void*)__vt__14CFxBoosterWarp;
        for (unsigned int i = 0; i < 2; ++i) {
            void* child = (&self->child0)[i];
            if (child != 0) {
                CFxBox_RemoveSequence(self->owner, child, 0);
                child = (&self->child0)[i];
                if (child != 0) ((Dt0*)child)->v08(1);
                (&self->child0)[i] = 0;
            }
        }
        if (shouldDelete > 0) __dl__FPv(self);
    }
    return self;
}

// Unregister and destroy the Spike sequences and optionally free the controller.
void* CFxSpike_dtor(ObjectWithChildren* self, int shouldDelete) {
    if (self != 0) {
        self->vtable = (void*)__vt__8CFxSpike;
        for (unsigned int i = 0; i < 2; ++i) {
            void* child = (&self->child0)[i];
            if (child != 0) {
                CFxBox_RemoveSequence(self->owner, child, 0);
                child = (&self->child0)[i];
                if (child != 0) ((Dt0*)child)->v08(1);
            }
        }
        if (shouldDelete > 0) __dl__FPv(self);
    }
    return self;
}

// Attempt the wood-wave effect once at the launch position; latch the attempt even on failure.
int CFxPowerShot2_TryCreateWoodWaveSequence(void* self) {
    unsigned char* bytes = (unsigned char*)self;
    if (AsCFxPowerShot2(bytes)->sequenceCreationAttempted) return 1;
    const ShotEffectResourceStrings& resources = shotEffectResourceStrings;
    void* child = CFxSequence_Create(AsCFxPowerShot2(bytes)->sequenceOwner, resources.woodwave2_seq, 1, 0, 0);
    if (child != 0) {
        CFxBox_RegisterSequence(AsCFxPowerShot2(bytes)->sequenceOwner, child);
        Vec3* source = (Vec3*)PreData__9CGolfBallFi(AsCFxPowerShot2(bytes)->golfBall, 0);
        Vec3 pos = *source;
        pos.y += kPowerShot2HeightOffset;
        ((CFxSequence*)child)->worldPosition = *(WVector*)&pos;
    } else {
        MessageBox_Report(g_legacyWindowContext, resources.powerShotLoadError, resources.errorTitle, 0);
    }
    AsCFxPowerShot2(bytes)->sequenceCreationAttempted = 1;
    return 1;
}

// Optionally free a Power Spin controller; the sequence is owned by the effect manager.
void* CFxPowerSpin_vfunc_0(void* ptr, int shouldDelete) {
    if (ptr != 0 && shouldDelete > 0) {
        __dl__FPv(ptr);
    }
    return ptr;
}

// Optionally free an impact controller; the sequence is owned by the effect manager.
void* CFxShotImpact_dtor(void* ptr, int shouldDelete) {
    if (ptr != 0 && shouldDelete > 0) {
        __dl__FPv(ptr);
    }
    return ptr;
}

// Optionally free a power-shot controller; the sequence is owned by the effect manager.
void* CFxPowerShot2_dtor(void* ptr, int shouldDelete) {
    if (ptr != 0 && shouldDelete > 0) {
        __dl__FPv(ptr);
    }
    return ptr;
}

} // extern "C"

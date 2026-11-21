// Tour board entry trail/twinkle and crash burst: 2D particles, texture binding,
// timer expiry and owned pointer-vector growth. Caller: OpenItemMenu (tour_character_map_board.cpp).
// Mod lookup (frame rate and texture):
// - Path/emission: TourParticleEmitter.trailDurationFrames (default 40 calls),
//   trailParticlesPerFrame (default 1) and trailSpread (default 20 UI units)
//   are initialized by TourParticleEffects_Init in tour_particle_effects.cpp/_data.cpp.
// - Atlas cycling: Particle2D_TickCyclingTrailTimer advances 1 cell per update,
//   masked by kTrailFrameMask=15 (16 cells); it ignores delta for frame selection.
//   Trail path fraction=frame/duration; final spawn is (duration-1)/duration,
//   never the endpoint. Starting again resets emission, not existing particles.
// - Lifetime: _data.cpp defines trail base 1.7 plus .5*rand/32768 seconds,
//   burst base 1.2 plus .3*rand/32768. Timers expire only if already <0 on entry;
//   crossing zero during subtraction survives until the NEXT update call.
// - Burst emission is 16 particles in one call; return 1 on the following call
//   only when disabling the emitter, not when particles have finished fading.
// - Appearance: same data companion defines trail size 36, atlas 32-pixel cells
//   in 128x128 (4x4), burst size 100, minimum dimension 8, rise distance 24.
//   Quad positions are UI coordinates (X right/Y down), minus borrowed origin;
//   normalized UVs are distinct from pixels. No aspect/viewport/scissor setter.
// - Textures: TourLoadEffect_InitTextures uses kTourTwinkleTexturePath and
//   kTourSmokeTexturePath; the emitter owns the CBpTex array and every particle.
//   tour_particle_effects.cpp's destructor releases them. Growth/erase moves pointer slots only.
// Preserve RNG call order, delta versus call clocks, pre-subtraction expiry,
// vector erase iteration, the eighth-root/square expansion and render-state lists.
// All required pointers/iterators borrow valid constructed storage. Allocator
// failure is not tolerated before SetPosition; length overflow throws. Caller
// drives update/draw sequentially; no synchronization or saved render-state stack.
#include "sdk_protos.h"

// Field view of the tour-load effect; each particle vector is {data, size, capacity}.
struct TourParticleEmitter {
    void* textures;
    char reserved04[4];
    float trailSpread;                  // 0x08: random spread perpendicular to the path
    int trailDurationFrames;            // 0x0C
    unsigned char trailParticlesPerFrame; // 0x10
    char reserved11[7];
    unsigned int trailParticles[3];     // 0x18
    float trailStart[2];                // 0x24
    float trailEnd[2];                  // 0x2C
    int trailFrame;                     // 0x34
    int trailEnabled;                   // 0x38
    char reserved3C[4];
    unsigned int burstParticles[3];     // 0x40
    float burstOrigin[2];               // 0x4C
    int burstFrame;                     // 0x54
    int burstEnabled;                   // 0x58
};
typedef char TourParticleEmitter_size[(sizeof(TourParticleEmitter) == 0x5C) ? 1 : -1];
// Borrow the complete 0x5C-byte emitter view; no ownership or offset adjustment.
// Keep this single-expression cast: the inline-helper trial changed .text in the
// whole-unit before/after check by altering CodeWarrior's operand scheduling.
#define AsTourParticleEmitter(p) ((TourParticleEmitter*)(p))
// Keep byte-pointer address expressions where CodeWarrior otherwise caches subobject addresses.
#define EMITTER_OFFSET(field) ((int)&((TourParticleEmitter*)0)->field)
struct ParticlePositionView { char timer[12]; float position[2]; };
enum ParticleEffectConstants {
    kTrailFrameMask = 15, kTrailAtlasFrameStep = 1, // cells per update, no delta
    kBurstParticleCount = 16, kBurstEmissionCalls = 1,
    kTrailParticleRecordBytes = 0x18, kBurstParticleRecordBytes = 0x24,
    kParticleVertexBytes = 0x18, kParticleTextureRecordBytes = 0x28,
    kParticleTextureArrayBytes = 0x60, // 2*0x28 plus 0x10-byte runtime cookie
    kParticleRenderStateCount = 10, kParticleTextureStateCount = 6,
    kParticleOpaqueChannel = 255,
    kAtlasColumnMask = 3, kAtlasRowShift = 2, kAtlasCellPixels = 32,
    kEffectTextureCount = 2, kQuadVertexCount = 4,
    kTriangleStrip = 0x98, kParticleVertexFormat = 0x11F005,
    kMaxWordCapacity = 0x3FFFFFFF,
    kSmallWordGrowthLimit = 0x15555555, kMediumWordGrowthLimit = 0x2AAAAAAA
};
enum TourEffectStringOffsets {
    kCrashBoardSound = 15, kVectorLengthError = 0x20, kLengthErrorType = 0x34
};

extern "C" {
struct Particle; struct Timer; struct GrowVector; struct WordVector;
int rand();
extern const float kFloat_32768_particle2d;
extern const float kFloat_0_5_GeneralEffects,kFloat_1_ResultEffects,kFloat_10_TourParticles; // 10.0: horizontal jitter offset / color count
void* __nw__FUl(unsigned long);
Particle* BurstParticle2D_InitColoredParticle(Particle*); Particle* Particle2D_InitTrailParticle(Particle*);
void BurstParticle2D_SetPosition(char* self, const void*); void TrailParticle2D_SetPosition(char* self, const void*);
void BurstParticleVector_Append(GrowVector*,const unsigned int*);
void TrailParticleVector_Append(GrowVector*,const volatile unsigned int*);
float Math_Atan2Float(double, double);
unsigned int WordVectorIterator_Get(const unsigned int*), TrailParticleVector_End(const unsigned int*),BurstParticleVector_Begin(const unsigned int*),BurstParticleVector_End(const unsigned int*),TrailIterator_GetAddress(const unsigned int*),BurstIterator_GetAddress(const unsigned int*);
void TrailIterator_Copy(unsigned int*,const unsigned int*),BurstIterator_Copy(unsigned int*,const unsigned int*),TrailIterator_Assign(unsigned int*,const unsigned int*),BurstIterator_Assign(unsigned int*,const unsigned int*);
bool TrailIterator_NotEqual(const unsigned int*,const unsigned int*),BurstIterator_NotEqual(const unsigned int*,const unsigned int*);
bool Particle2D_TickCyclingTrailTimer(Timer*,float),Particle2D_TickResetBurstTimer(Timer*,float);
void* Particle_Dtor(void*,int); void* Object_DeleteDtor(void*,int);
int* TrailParticleVector_Erase(WordVector*,int* const volatile*),*BurstParticleVector_Erase(WordVector*,int* const volatile*);
void TrailIterator_Increment(unsigned int*),BurstIterator_Increment(unsigned int*);
}
extern "C" {
extern const float kBurstParticleHorizontalJitterRange, kBurstParticleVerticalJitterRange, kBurstParticleVerticalJitterCenter, kBurstParticleInitialRise; // 0x163C: 21.0: burst horizontal jitter range; 0x1648: 26.0: burst vertical jitter range; 0x164C: 12.5: burst vertical jitter centering; 0x1650: 12.0: burst initial upward offset
struct EmitVec2 { float x,y; };
struct EmitVec4 { float x,y,z,w; };
struct EmitMatrix { float m[16]; };
extern const EmitVec4 tourParticleConstantData; // Initial homogeneous offset before trail jitter and rotation.
void gtl_Vec2_Subtract(EmitVec2*,const void*,const void*);
void Vec2_Scale(EmitVec2*,const EmitVec2*,float);
void Mtx44_SetRotationZ(EmitMatrix*,float);
void Mtx44_TransformVec4(EmitVec4*,const EmitVec4*,const EmitMatrix*);

// Emit trail/burst particles, tick both particle lists and delete expired entries; report burst emission completion.
// Phases: (1) burst: on its first frame spawn kBurstParticleCount particles with random jitter
// around burstOrigin, afterwards switch the burst off and return 1; (2) trail: for
// trailDurationFrames frames spawn trailParticlesPerFrame particles along the start->end path
// (position = trailStart + rotated random sideways offset + path * frame/duration);
// (3) walk both particle vectors, tick each timer and, when it reports expiry, delete the
// particle and erase its slot (the iterator is re-assigned from the erase result).
// Both lists store particle pointers as unsigned ints (32-bit addresses), hence the
// integer iterators and casts; ownership: the emitter owns every pointer in the lists.
// elapsedSeconds advances lifetimes only; counts/path/atlas use update calls.
// Positive trailDurationFrames is required while emitting (division); zero or
// negative duration disables emission before division for a nonnegative frame.
// The returned flag describes burst emission only. Use IsTrailFinished/
// IsBurstFinished for list drainage. RNG and both vectors can change on every call.
// If vector growth throws, this routine supplies no rollback of already spawned
// particles/emitter counters. Preserve the order burst -> trail -> trail/burst ticks.
int Particle2D_UpdateTrailAndBurst(char* self,float elapsedSeconds) {
    int burstEmissionFinished=0;
    // Phase 1: spawn the one-frame burst in screen coordinates (jitter is pixels).
    if (AsTourParticleEmitter(self)->burstEnabled) {
        if (AsTourParticleEmitter(self)->burstFrame<kBurstEmissionCalls) {
            Particle* particle;
            for(int i=0;i<kBurstParticleCount;++i) {
                EmitVec2 spawnPosition;
                spawnPosition.x=AsTourParticleEmitter(self)->burstOrigin[0]+(kBurstParticleHorizontalJitterRange*rand()/kFloat_32768_particle2d-kFloat_10_TourParticles);
                spawnPosition.y=AsTourParticleEmitter(self)->burstOrigin[1]+(kBurstParticleVerticalJitterRange*rand()/kFloat_32768_particle2d-kBurstParticleVerticalJitterCenter)-kBurstParticleInitialRise;
                particle=(Particle*)__nw__FUl(kBurstParticleRecordBytes);
                if(particle) BurstParticle2D_InitColoredParticle(particle);
                unsigned int particleAddress=(unsigned int)particle;
                ((void(*)(void*,void*))BurstParticle2D_SetPosition)(particle,&spawnPosition);
                BurstParticleVector_Append((GrowVector*)(self + EMITTER_OFFSET(burstParticles)),&particleAddress);
            }
            ++AsTourParticleEmitter(self)->burstFrame;
        }
        else {
            AsTourParticleEmitter(self)->burstEnabled=0;
            burstEmissionFinished=1;
        }
    }
    // Phase 2: place trail particles along the frame-based path; timers use elapsed seconds.
    if (AsTourParticleEmitter(self)->trailEnabled) {
        if (AsTourParticleEmitter(self)->trailFrame<AsTourParticleEmitter(self)->trailDurationFrames) {
            EmitVec4 rotated;
            EmitVec4 offset=tourParticleConstantData;
            EmitVec2 pathDirection;
            gtl_Vec2_Subtract(&pathDirection,(self + EMITTER_OFFSET(trailEnd)),(self + EMITTER_OFFSET(trailStart)));
            EmitMatrix matrix;
            Mtx44_SetRotationZ(&matrix,-((float(*)(double,double))Math_Atan2Float)(pathDirection.x,pathDirection.y));
            EmitVec2 pathStep;
            Vec2_Scale(&pathStep,&pathDirection,(float)AsTourParticleEmitter(self)->trailFrame/(float)AsTourParticleEmitter(self)->trailDurationFrames);
            for(int i=0;i<AsTourParticleEmitter(self)->trailParticlesPerFrame;++i) {
                offset.x=(kFloat_1_ResultEffects+AsTourParticleEmitter(self)->trailSpread)*rand()/kFloat_32768_particle2d-AsTourParticleEmitter(self)->trailSpread*kFloat_0_5_GeneralEffects;
                Mtx44_TransformVec4(&rotated,&offset,&matrix);
                EmitVec2 spawnPosition;
                spawnPosition.x=AsTourParticleEmitter(self)->trailStart[0]+(rotated.x+pathStep.x);
                spawnPosition.y=AsTourParticleEmitter(self)->trailStart[1]+(rotated.y+pathStep.y);
                Particle* particle=(Particle*)__nw__FUl(kTrailParticleRecordBytes);
                if(particle) Particle2D_InitTrailParticle(particle);
                unsigned int particleAddress=(unsigned int)particle;
                ((void(*)(void*,void*))TrailParticle2D_SetPosition)(particle,&spawnPosition);
                TrailParticleVector_Append((GrowVector*)(self + EMITTER_OFFSET(trailParticles)),&particleAddress);
            }
            ++AsTourParticleEmitter(self)->trailFrame;
        }
        else AsTourParticleEmitter(self)->trailEnabled=0;
    }
    // Phase 3: tick owned trails; erase returns the shifted slot, so do not increment after erasing.
    unsigned int trailIterator, burstIterator;
    unsigned int trailBegin=WordVectorIterator_Get((unsigned int*)(self + EMITTER_OFFSET(trailParticles)));
    TrailIterator_Copy(&trailIterator,&trailBegin);
    unsigned int trailAfterErase, trailErasePosition;
    unsigned int trailEnd;
    while (trailEnd=TrailParticleVector_End((unsigned int*)(self + EMITTER_OFFSET(trailParticles))), TrailIterator_NotEqual(&trailIterator,&trailEnd)) {
        if(Particle2D_TickCyclingTrailTimer(*(Timer**)TrailIterator_GetAddress(&trailIterator),elapsedSeconds)) {
            Particle_Dtor(*(void**)TrailIterator_GetAddress(&trailIterator),1);
            trailErasePosition=trailIterator;
            trailAfterErase=(unsigned int)TrailParticleVector_Erase((WordVector*)(self + EMITTER_OFFSET(trailParticles)),(int**)&trailErasePosition);
            TrailIterator_Assign(&trailIterator,&trailAfterErase);
        }
        else TrailIterator_Increment(&trailIterator);
    }
    // Phase 4: apply the same erase discipline to burst particles.
    unsigned int burstBegin=BurstParticleVector_Begin((unsigned int*)(self + EMITTER_OFFSET(burstParticles)));
    BurstIterator_Copy(&burstIterator,&burstBegin);
    unsigned int burstAfterErase, burstErasePosition;
    unsigned int burstEnd;
    while (burstEnd=BurstParticleVector_End((unsigned int*)(self + EMITTER_OFFSET(burstParticles))), BurstIterator_NotEqual(&burstIterator,&burstEnd)) {
        if(Particle2D_TickResetBurstTimer(*(Timer**)BurstIterator_GetAddress(&burstIterator),elapsedSeconds)) {
            Object_DeleteDtor(*(void**)BurstIterator_GetAddress(&burstIterator),1);
            burstErasePosition=burstIterator;
            burstAfterErase=(unsigned int)BurstParticleVector_Erase((WordVector*)(self + EMITTER_OFFSET(burstParticles)),(int**)&burstErasePosition);
            BurstIterator_Assign(&burstIterator,&burstAfterErase);
        }
        else BurstIterator_Increment(&burstIterator);
    }
    return burstEmissionFinished;
}
}

// Translation unit 0x80333FFC-0x80335C0C (38 functions; boundaries inferred by decomp-toolkit from extab).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

extern "C" {

void gtl_Vec2_Copy(void*, const void*);
double atan2(double, double);
extern const float kFloat_0_805E1620;
void __dl__FPv(void*);

// Optionally release the trail particle allocation; its timer and position need no cleanup.
void* Particle_Dtor(void* self, int deleting) {
    if (self && deleting > 0) __dl__FPv(self);
    return self;
}

// Optionally release the burst particle allocation; it owns no secondary resources.
void* Object_DeleteDtor(void* self, int deleting) {
    if (self && deleting > 0) __dl__FPv(self);
    return self;
}

// Report that the trail has neither live particles nor an enabled emitter.
bool TourLoadEffect_IsTrailFinished(const int* self) {
    return AsTourParticleEmitter(self)->trailParticles[1] == 0 && AsTourParticleEmitter(self)->trailEnabled == 0;
}

// Report that the burst has neither live particles nor an enabled emitter.
bool TourLoadEffect_IsBurstFinished(const int* self) {
    return AsTourParticleEmitter(self)->burstParticles[1] == 0 && AsTourParticleEmitter(self)->burstEnabled == 0;
}

struct Timer {
    float lifetime;
    float remaining;
    unsigned int state;
};

// Cycle one trail atlas cell per call and subtract delta seconds; return preexisting expiry.
// Required self is the shared 12-byte timer prefix. remaining<0 returns true
// without mutation; remaining==0 advances/wraps the cell and subtracts delta.
// Return false does not promise remaining>=0. Keep that one-update expiry delay.
bool Particle2D_TickCyclingTrailTimer(Timer* self, float delta) {
    if (self->remaining < kFloat_0_805E1620) return true;
    self->state += kTrailAtlasFrameStep;
    self->remaining -= delta;
    self->state &= kTrailFrameMask;
    return false;
}

// Subtract delta seconds and reset the burst frame; return preexisting expiry.
// Required self is a 12-byte timer prefix; remaining<0 returns true untouched.
// Zero is still live. Otherwise state becomes zero and remaining may cross below
// zero while returning false; the next update deletes it. No allocation occurs.
bool Particle2D_TickResetBurstTimer(Timer* self, float delta) {
    if (self->remaining < kFloat_0_805E1620) return true;
    self->remaining -= delta;
    self->state = 0;
    return false;
}

// Compare two burst-vector iterator addresses for inequality.
bool BurstIterator_NotEqual(const unsigned int* self, const unsigned int* other) {
    return *self != *other;
}

// Compare two trail-vector iterator addresses for inequality.
bool TrailIterator_NotEqual(const unsigned int* self, const unsigned int* other) {
    return *self != *other;
}

// Return atan2 rounded to float, preserving the double-precision call.
float Math_Atan2Float(double value, double other) {
    return atan2(value, other);
}

// Copy the two-dimensional burst spawn origin.
void Particle2D_SetBurstOrigin(char* self, const void* position) {
    gtl_Vec2_Copy(AsTourParticleEmitter(self)->burstOrigin, position);
}

// Copy a trail particle position after its 12-byte timer prefix.
void TrailParticle2D_SetPosition(char* self, const void* position) {
    gtl_Vec2_Copy(((ParticlePositionView*)self)->position, position);
}

// Copy a burst particle position after its 12-byte timer prefix.
void BurstParticle2D_SetPosition(char* self, const void* position) {
    gtl_Vec2_Copy(((ParticlePositionView*)self)->position, position);
}

// Read the trail iterator address; the caller dereferences the particle pointer stored there.
unsigned int TrailIterator_GetAddress(const unsigned int* self) {
    return *self;
}

// Read the burst iterator address; the caller dereferences the particle pointer stored there.
unsigned int BurstIterator_GetAddress(const unsigned int* self) {
    return *self;
}

// Advance a trail-vector iterator by one four-byte pointer slot.
void TrailIterator_Increment(unsigned int* self) {
    *self += 4;
}

// Advance a burst-vector iterator by one four-byte pointer slot.
void BurstIterator_Increment(unsigned int* self) {
    *self += 4;
}

// Return the first slot address of a word vector.
unsigned int WordVectorIterator_Get(const unsigned int* self) {
    return *self;
}

// Copy a trail-vector iterator address.
void TrailIterator_Copy(unsigned int* self, const unsigned int* other) {
    *self = *other;
}

// Assign the trail-vector iterator returned after erasing an entry.
void TrailIterator_Assign(unsigned int* self, const unsigned int* other) {
    *self = *other;
}

// Return the address one past the last trail-vector pointer slot.
unsigned int TrailParticleVector_End(const unsigned int* self) {
    return self[0] + self[1] * 4;
}

// Return the first slot address of the burst vector.
unsigned int BurstParticleVector_Begin(const unsigned int* self) {
    return *self;
}

// Copy a burst-vector iterator address.
void BurstIterator_Copy(unsigned int* self, const unsigned int* other) {
    *self = *other;
}

// Assign the burst-vector iterator returned after erasing an entry.
void BurstIterator_Assign(unsigned int* self, const unsigned int* other) {
    *self = *other;
}

// Return the address one past the last burst-vector pointer slot.
unsigned int BurstParticleVector_End(const unsigned int* self) {
    return self[0] + self[1] * 4;
}

extern const float kFloat_0_5_GeneralEffects;
extern const float kFloat_200_ResultEffects;
extern const float kFloat_1_ResultEffects;
extern char tourLoadEffectStrings[];
extern void* g_soundManager;
void Sound_PlayByName(void*, const char*, int, int, int, int, float, float, float);

// Set the start and end points used to interpolate trail particle spawns.
void Particle2D_SetTrailEndpoints(char* self, void* startPosition, void* endPosition) {
    ((void (*)(void*, void*))gtl_Vec2_Copy)(AsTourParticleEmitter(self)->trailStart, startPosition);
    ((void (*)(void*, void*))gtl_Vec2_Copy)(AsTourParticleEmitter(self)->trailEnd, endPosition);
}

// Play the opening sound and restart call-based trail emission at frame zero.
// Required constructed emitter retains both live vectors; no particles are removed.
// Sound uses shared .5/200/1 parameters; their API units are not inferred here.
void TourLoadEffect_StartOpen(int* self) {
    Sound_PlayByName(g_soundManager, tourLoadEffectStrings, 0, 0, 0, 0, kFloat_0_5_GeneralEffects, kFloat_200_ResultEffects, kFloat_1_ResultEffects);
    AsTourParticleEmitter(self)->trailFrame = 0;
    AsTourParticleEmitter(self)->trailEnabled = 1;
}

// Play the crash-board sound and restart one-call burst emission at frame zero.
// Required constructed emitter retains existing particles; the next update appends
// another 16 bursts. IsBurstFinished also waits for the retained list to drain.
void TourLoadEffect_StartBurst(int* self) {
    Sound_PlayByName(g_soundManager, tourLoadEffectStrings + kCrashBoardSound, 0, 0, 0, 0, kFloat_0_5_GeneralEffects, kFloat_200_ResultEffects, kFloat_1_ResultEffects);
    AsTourParticleEmitter(self)->burstFrame = 0;
    AsTourParticleEmitter(self)->burstEnabled = 1;
}

struct WordVector {
    int* data;
    int size;
};

// Erase a trail-vector slot by shifting later pointers left; return the erased position.
int* TrailParticleVector_Erase(WordVector* self, int* const volatile* iterator) {
    int* position = *iterator;
    int* next = position + 1;
    int count = (self->data + self->size - position) - 1;
    memmove(position, next, count * 4);
    --self->size;
    return position;
}

// Erase a burst-vector slot by shifting later pointers left; return the erased position.
int* BurstParticleVector_Erase(WordVector* self, int* const volatile* iterator) {
    int* position = *iterator;
    int* next = position + 1;
    int count = (self->data + self->size - position) - 1;
    memmove(position, next, count * 4);
    --self->size;
    return position;
}

struct CheckObject {
    virtual void method0();
    virtual void method4();
    virtual void method8();
    virtual void methodC();
    virtual int check();
    char padding[0x24];
};

// Report ready only when both tour-effect textures pass their readiness check.
int TourLoadEffect_AreTexturesReady(CheckObject** self) {
    for (int i = 0; i < 2; ++i) {
        if (!(*self)[i].check()) return 0;
    }
    return 1;
}

} // extern "C"

extern "C" {
int rand();
extern const float kTrailParticleLifetimeBase; // 1.7: trail lifetime base, seconds
extern const float kTrailParticleQuadSize, kTrailParticleAtlasFrameCount; // 0x1674: 36.0: trail quad size; 0x1678: 16.0: random atlas-frame count
// Two allocations share this view, not the same extent: a trail is exactly the
// 0x18-byte prefix {life,remaining,state,x,y,size}; only bursts own the trailing
// widthScale/heightScale/RGBA words, extending to 0x24 bytes. Trail paths must
// never read/write those trailing members. Timer aliases the first 12 bytes.
struct Particle { float life, remaining; int state; float x,y,size,widthScale,heightScale; unsigned char r,g,b,a; };
// Initialize a trail particle with a random atlas frame, 1.7-2.2 second lifetime and size 36.
Particle* Particle2D_InitTrailParticle(Particle* self) {
    float half, base, size;
    half = *(const volatile float*)&kFloat_0_5_GeneralEffects;
    base = kTrailParticleLifetimeBase;
    size = *(const volatile float*)&kTrailParticleQuadSize;

    self->state = Runtime_ConvertDoubleToUnsigned(kTrailParticleAtlasFrameCount * rand() / kFloat_32768_particle2d);
    int random = rand();
    float life = base + half * random / kFloat_32768_particle2d;
    float zero = kFloat_0_805E1620;
    self->x = zero;
    self->y = zero;
    self->size = size;
    self->remaining = self->life = life;
    return self;
}
int GX_GetPrimitiveCount(int,int);
float Math_SquareRootApprox(float);
void Engine_SetFlexibleVertexFormat(int);
void DrawPrimitiveUP(int,int,void*,int);
extern float kFloat_255_TourParticles, kBurstParticleMinimumDimension, kBurstParticleRiseDistance; // 0x1694: 255.0: full-opacity alpha; 0x1690: 8.0: minimum rendered burst dimension; 0x1698: 24.0: total burst upward drift
struct Vertex { float x,y,z; unsigned char r,g,b,a; float u,v; };
// Take three successive square roots for the original burst expansion curve.
// The caller squares this eighth-root progress, producing a quarter-power size curve.
static inline float BurstExpansionProgressRoot(float progress) {
    return Math_SquareRootApprox(Math_SquareRootApprox(Math_SquareRootApprox(progress)));
}
// Draw a burst quad that expands, rises and fades as its remaining lifetime decreases.
// With progress p = 1 - remaining/life (0 at spawn, 1 at expiry):
//   scale  = size * (p^(1/8))^2 = size * p^(1/4)      (three nested square roots, then squared)
//   width  = widthScale  * scale + 8.0   height = heightScale * scale + 8.0   (8.0 = minimum dimension)
//   alpha  = 255 * remaining/life                      (linear fade out)
//   center = (x - origin.x, y - origin.y), y offset by -24.0 * p over the lifetime (drift)
// Corners (triangle strip): v0 top-left, v1 top-right, v2 bottom-left, v3 bottom-right;
// all four share the particle's RGB and the computed alpha. Not drawn once remaining < 0.
void Particle_DrawExpandingFadingQuad(Particle* self, float* origin) {
    if (!(self->remaining < kFloat_0_805E1620)) {
        int count = GX_GetPrimitiveCount(kQuadVertexCount,kTriangleStrip);
        Vertex v[4];
        v[0].u = v[2].u = kFloat_0_805E1620;
        v[1].u = v[3].u = kFloat_1_ResultEffects;
        v[0].v = v[1].v = kFloat_0_805E1620;
        v[2].v = v[3].v = kFloat_1_ResultEffects;
        float eighthRootProgress = BurstExpansionProgressRoot(kFloat_1_ResultEffects - self->remaining/self->life);
        float scale = self->size * (eighthRootProgress*eighthRootProgress);
        float width = self->widthScale * scale + kBurstParticleMinimumDimension;
        float height = self->heightScale * scale + kBurstParticleMinimumDimension;
        int alpha = kFloat_255_TourParticles * (self->remaining / self->life);
        Engine_SetFlexibleVertexFormat(kParticleVertexFormat);
        float x, bottom, drop, half, y, ratio, left, right, top;
        ratio = kFloat_1_ResultEffects - self->remaining/self->life;
        y = self->y - origin[1];
        half = kFloat_0_5_GeneralEffects;
        __typeof__(self->x) savedX = self->x;
        x = savedX - origin[0];
        drop = kBurstParticleRiseDistance;
        bottom = y + height * half;
        bottom -= drop * ratio;
        right = x + width * half;
        left = x - width * half;
        top = (y - height * half) - drop * ratio;
        v[2].z = v[3].z = kFloat_0_805E1620;
        v[3].x = right;
        v[2].x = left;
        v[0].x = left;
        v[1].x = right;
        v[0].y = v[1].y = top;
        v[2].y = v[3].y = bottom;
        v[0].z = v[1].z = kFloat_0_805E1620;
        unsigned char r, g, b;
        b=self->b;
        g=self->g;
        r=self->r;
        v[0].r=r;
        v[0].g=g;
        v[0].b=b;
        v[1].r=r;
        v[1].g=g;
        v[1].b=b;
        v[2].r=r;
        v[2].g=g;
        v[2].b=b;
        v[3].r=r;
        v[3].g=g;
        v[3].b=b;
        v[3].a=v[2].a=v[1].a=v[0].a=alpha;
        DrawPrimitiveUP(kTriangleStrip,count,v,0x18);
    }
}
}

extern "C" {
void* __nwa__FUl(unsigned long);
void __destroy_new_array(void*,void*);
void CBpTex_Init();
void CBpTex_dtor();
extern unsigned int kTourTwinkleTexturePath, kTourSmokeTexturePath; // Texture name words for the two effect textures.
struct InitObject {
    virtual void method0();
    virtual void init(unsigned int);
    char data[0x24];
};
void __dla__FPv(void*);
struct InitAllocation {
    char bytes[0x60];
    // Allocate the texture array including the runtime array header.
    static void* operator new(size_t n) { return __nwa__FUl(n); }
    // Release storage when array construction fails.
    static void operator delete(void* p) { __dla__FPv(p); }
    // Construct the two effect-texture objects in the owned array.
    InitAllocation() { __construct_new_array(this,(void*)CBpTex_Init,(void*)CBpTex_dtor,0x28,2); }
};
// Destroy the previous owned CBpTex array, construct two replacements and set paths.
// self points to the emitter's texture-array field and is required; *self may be
// null. Replacing invalidates prior texture-object aliases. Array cookie is 0x10,
// element stride 0x28; init(path) is virtual slot 1, consuming a borrowed name.
// Allocation failure is unchecked before the init loop. Readiness is separate
// (TourLoadEffect_AreTexturesReady); this function does not imply loaded textures.
void TourLoadEffect_InitTextures(InitObject** self) {
    unsigned int names[2] = {kTourTwinkleTexturePath,kTourSmokeTexturePath};
    if (*self) { __destroy_new_array(*self,(void*)CBpTex_dtor); *self=0; }
    InitObject* objects = (InitObject*)__nwa__FUl(kParticleTextureArrayBytes);
    if (objects)
        objects = (InitObject*)__construct_new_array(objects,(void*)CBpTex_Init,(void*)CBpTex_dtor,kParticleTextureRecordBytes,kEffectTextureCount);
    *self=objects;
    unsigned int* name=names;
    int offset;
    int i=0;
    offset=0;
    for (;i<kEffectTextureCount;++i,++name,offset+=kParticleTextureRecordBytes) { ((InitObject*)((char*)*self+offset))->init(*name); }
}
}

extern "C" {
struct ColorTable { unsigned char c[10][3]; };
extern const ColorTable tourBurstParticleColors;
extern const float kBurstParticleLifetimeRange, kBurstParticleLifetimeBase, kBurstParticleQuadBaseSize, kFloat_0_8_TourParticles, kBurstParticleVerticalStretchRange, kFloat_10_TourParticles; // 0x167C: 0.3: burst lifetime random range, seconds; 0x1680: 1.2: burst lifetime base, seconds; 0x1684: 100.0: burst base quad size; 0x1688: 0.8: burst stretch range/base; 0x168C: 0.7: burst vertical stretch random range; 0x1644: 10.0: horizontal jitter offset / color count
// Initialize a colored burst particle with random lifetime and horizontal/vertical stretch.
Particle* BurstParticle2D_InitColoredParticle(Particle* self) {
    const ColorTable colors = tourBurstParticleColors;
    float range = kBurstParticleLifetimeRange;
    float base = kBurstParticleLifetimeBase;
    float size = kBurstParticleQuadBaseSize;
    self->state = 0;
    int random = rand();
    self->size = size;
    self->remaining = self->life = base + range * random / kFloat_32768_particle2d;
    self->widthScale = kFloat_0_5_GeneralEffects + kFloat_0_8_TourParticles * rand() / *(const float*)&kFloat_32768_particle2d;
    self->heightScale = kFloat_0_8_TourParticles + kBurstParticleVerticalStretchRange * rand() / *(const float*)&kFloat_32768_particle2d;
    signed char index = kFloat_10_TourParticles * rand() / kFloat_32768_particle2d;
    self->x = kFloat_0_805E1620;
    self->y = kFloat_0_805E1620;
    self->r = colors.c[index][0];
    self->g = colors.c[index][1];
    self->b = colors.c[index][2];
    return self;
}
}

extern "C" {
struct RenderStateBlock; struct RenderFormatBlock;
extern const float kFloat_0_0078125_IconAtlas;
void Render_SetColoredQuadState();
void Engine_SetTextureStageStateList(int, void*, int);
void Engine_SetRenderStateList(void*, int);
struct RenderStateBlock { unsigned int words[20]; };
struct RenderFormatBlock { unsigned int words[12]; };
struct RenderData { char pad[0x38]; RenderStateBlock restore; RenderStateBlock setup; RenderFormatBlock format; };
extern const RenderData tourParticleRenderData;
struct ParticleTexture {
    virtual void method0();
    virtual void method4();
    virtual void method8();
    virtual void bind();
    char padding[0x24];
};
struct ParticleSystem {
    ParticleTexture* texture;
    char pad04[0x14];
    Particle** first;
    unsigned int count;
    char pad20[0x20];
    Particle** second;
    unsigned int secondCount;
};
// Draw live shrinking atlas trails, then expanding colored burst quads.
// Required self has two initialized textures and valid pointer vectors; origin
// borrows two floats in the same UI coordinate space. Skips remaining<0 particles.
// Installs setup then fixed restore lists from tourParticleRenderData: these are
// engine defaults, not a saved snapshot of the caller's current state. Texture 1
// remains bound after burst drawing. No timer/particle ownership changes.
void TourTwinkle_DrawParticles(ParticleSystem* self, float* origin) {
    const char* base = (const char*)&tourParticleRenderData;
    RenderStateBlock restore = *(const RenderStateBlock*)(((char*)(base + (int)&((RenderData*)0)->restore)));
    RenderStateBlock setup = *(const RenderStateBlock*)(((char*)(base + (int)&((RenderData*)0)->setup)));
    Vertex v[4];
    RenderFormatBlock format = *(const RenderFormatBlock*)(((char*)(base + (int)&((RenderData*)0)->format)));
    Render_SetColoredQuadState();
    Engine_SetTextureStageStateList(0, &format, kParticleTextureStateCount);
    Engine_SetRenderStateList(&setup, kParticleRenderStateCount);
    self->texture->bind();
    Particle** it = self->first;
    for (; it != self->first + self->count; ++it) {
        Particle* p = *it;
        if (p->remaining < kFloat_0_805E1620) continue;
        int count = GX_GetPrimitiveCount(kQuadVertexCount, kTriangleStrip);
        v[0].u = v[2].u = kFloat_0_0078125_IconAtlas * (unsigned int)((p->state & kAtlasColumnMask) * kAtlasCellPixels);
        v[1].u = v[3].u = kFloat_0_0078125_IconAtlas * (unsigned int)((p->state & kAtlasColumnMask) * kAtlasCellPixels + kAtlasCellPixels);
        v[0].v = v[1].v = kFloat_0_0078125_IconAtlas * (((unsigned int)p->state >> kAtlasRowShift) * kAtlasCellPixels);
        v[2].v = v[3].v = kFloat_0_0078125_IconAtlas * (((unsigned int)p->state >> kAtlasRowShift) * kAtlasCellPixels + kAtlasCellPixels);
        float scale = p->size * (p->remaining / p->life);
        Engine_SetFlexibleVertexFormat(kParticleVertexFormat);
        float x = p->x - origin[0];
        float y = p->y - origin[1];
        float bottom = y + scale * kFloat_0_5_GeneralEffects;
        float right = x + scale * kFloat_0_5_GeneralEffects;
        v[1].z = v[2].z = v[3].z = kFloat_0_805E1620;
        float left = x - scale * kFloat_0_5_GeneralEffects;
        float top = y - scale * kFloat_0_5_GeneralEffects;
        v[3].y = bottom;
        v[3].x = right;
        v[0].x = v[2].x = left;
        v[1].x = right;
        v[0].y = v[1].y = top;
        v[2].y = bottom;
        v[0].z = kFloat_0_805E1620;
        v[0].r=kParticleOpaqueChannel; v[0].g=kParticleOpaqueChannel; v[0].b=kParticleOpaqueChannel;
        v[1].r=kParticleOpaqueChannel; v[1].g=kParticleOpaqueChannel; v[1].b=kParticleOpaqueChannel;
        v[2].r=kParticleOpaqueChannel; v[2].g=kParticleOpaqueChannel; v[2].b=kParticleOpaqueChannel;
        v[3].r=kParticleOpaqueChannel; v[3].g=kParticleOpaqueChannel; v[3].b=kParticleOpaqueChannel;
        v[3].a=v[2].a=v[1].a=v[0].a=kParticleOpaqueChannel;
        DrawPrimitiveUP(kTriangleStrip,count,v,kParticleVertexBytes);
    }
    Engine_SetRenderStateList(&restore, kParticleRenderStateCount);
    self->texture[1].bind();
    for (Particle** it = self->second; it != self->second + self->secondCount; ++it)
        Particle_DrawExpandingFadingQuad(*it, origin);
}
}

extern "C" {
void* __nw__FUl(unsigned long);
size_t strlen(const char*);
char* strcpy(char*,const char*);
void* memcpy(void*,const void*,unsigned long);
void* memset(void*,int,unsigned long);
void* appMallocDummy__FUl(unsigned long);
void appFreeUni__FPv(void*);
void __dt__Q23std12length_errorFv();
extern char __vt__Q23std11logic_error[],s_sharedCharDeleterVtable[],__vt__Q23std12length_error[];
void __dla__FPv(void*);
struct VectorGuard_u80333FFC {
    char* text;
    char* flag;
    // If still armed, call the original array-delete ABI with the GUARD address,
    // not text. The source retains this unusual operand exactly; no ordinary
    // delete[]-text ownership guarantee is inferred for this unwind-only path.
    // Successful transfer clears text so this branch is skipped.
    ~VectorGuard_u80333FFC() { if(text) __dla__FPv(this); }
};
struct VectorError { void* table; char* text; void* owner; };
struct VectorOwner { void* table; int a,b; void* data; };
// Construct and throw the original vector length error while guarding the allocated message.
inline void vectorLengthError() {
    int flag; *(char*)&flag=0;
    VectorError e;
    const char* msg=tourLoadEffectStrings+kVectorLengthError;
    e.table=__vt__Q23std11logic_error;
    unsigned long n=strlen(msg)+1;
    e.text=(char*)__nwa__FUl((n*2)-n);
    char* allocated=e.text;
    { VectorGuard_u80333FFC guard; guard.text=allocated; guard.flag=(char*)&flag;
    VectorOwner* owner=(VectorOwner*)__nw__FUl(16);
    if(owner) { owner->a=1; owner->b=1; owner->table=s_sharedCharDeleterVtable; owner->data=allocated; }
    e.owner=owner; guard.text=0; }
    strcpy(e.text,msg);
    e.table=__vt__Q23std12length_error;
    __throw(tourLoadEffectStrings+kLengthErrorType,&e,(void*)__dt__Q23std12length_errorFv);
}
struct GrowVector { unsigned int* data; unsigned int size,capacity; };
struct VectorBuffer {
    unsigned int* data;
    unsigned int size, capacity;
    unsigned int* allocator;
    unsigned int offset;
    // Guard a replacement pointer array until its exchange with the live vector.
    VectorBuffer(unsigned int* a) : data(0),size(0),capacity(0),allocator(a),offset(0) {}
    // Free the allocation left in the guard after commit or exceptional exit.
    ~VectorBuffer() { if(data) { size=0; appFreeUni__FPv(data); } }
};
// Append a burst particle pointer, preserving the guarded growth and old-buffer cleanup order.
// Same algorithm as TrailParticleVector_Append (a std::vector<unsigned> push_back for pointer
// values): if size < capacity store in place; else compute new capacity (growth ~ x1.6 below
// kSmallWordGrowthLimit, x1.5 below kMediumWordGrowthLimit, else max; at least +1), allocate,
// copy the old words, swap buffers and let VectorBuffer's destructor free the old block.
// Only this function's element is read plain; the trail variant reads its source volatile.
// value is a borrowed one-word pointer representation and must remain readable
// through growth (it must not alias overwritten/retired slots). Appending transfers
// particle lifetime responsibility to the emitter; growth owns only its new slot
// buffer. If allocation/length throws, caller handles the not-yet-appended particle.
void BurstParticleVector_Append(GrowVector* self,const unsigned int* value) {
    unsigned int oldCapacityLimit=self->capacity;
    unsigned int size=self->size;
    // Phase 1: fast append into an existing free pointer slot.
    if(size<oldCapacityLimit) {
        unsigned int next=1+size;
        unsigned int item=*value;
        self->size=next;
        self->data[next-1]=item;
        return;
    }
    // Phase 2: choose capacity in 32-bit pointer words and guard the replacement allocation.
    if(kMaxWordCapacity-oldCapacityLimit<1) vectorLengthError();
    VectorBuffer growthBuffer(&self->capacity);
    unsigned int additionalSlots=self->size+1-self->capacity;
    unsigned int capacity=self->capacity;
    if(additionalSlots>kMaxWordCapacity-capacity) vectorLengthError();
    if(capacity<kSmallWordGrowthLimit) {
        unsigned int growth=((capacity+1)*3)/5;
        capacity+=*(growth<additionalSlots?&additionalSlots:&growth);
    }
    else if(capacity<kMediumWordGrowthLimit) {
        unsigned int growth=(capacity+1)/2;
        capacity+=*((growth<additionalSlots)?&additionalSlots:&growth);
    }
    else capacity=kMaxWordCapacity;
    if(capacity>kMaxWordCapacity) vectorLengthError();
    // Phase 3: append first, then transfer existing pointer words and clear the old slots.
    unsigned int* data=(unsigned int*)appMallocDummy__FUl(capacity*4);
    unsigned int oldSize=self->size;
    *(volatile unsigned int*)&growthBuffer.offset=oldSize;
    (data+oldSize)[growthBuffer.size]=*value;
    growthBuffer.data=data;
    growthBuffer.capacity=capacity;
    growthBuffer.size++;
    unsigned int* oldData=self->data;
    unsigned int oldWordCount=((int)(self->data+self->size)-(int)oldData)/4;
    growthBuffer.offset=oldSize-oldWordCount;
    memcpy((data+growthBuffer.offset),oldData,oldWordCount*4);
    memset(oldData,0,4*oldWordCount);
    *(volatile unsigned int*)&growthBuffer.size=growthBuffer.size+oldWordCount;
    // Phase 4: publish the replacement; its guard now frees the old allocation.
    __typeof__(growthBuffer.data) savedData = growthBuffer.data;
    *(volatile unsigned int*)&self->size=0;
    unsigned int* oldAllocation=self->data;
    self->data=savedData;
    __typeof__(self->capacity) savedCapacityCopy = self->capacity;
    growthBuffer.data=oldAllocation;
    self->capacity=growthBuffer.capacity;
    self->size=growthBuffer.size;
    growthBuffer.capacity=(savedCapacityCopy);
    growthBuffer.size=0;
}
// Append a trail particle pointer from a volatile source, preserving guarded buffer growth.
// See BurstParticleVector_Append for the shared growth/ownership description.
void TrailParticleVector_Append(GrowVector* self,const volatile unsigned int* value) {
    __typeof__(self->capacity) savedCapacity = self->capacity;
    unsigned int size=self->size;
    unsigned int oldCapacityLimit=savedCapacity;
    // Phase 1: fast append into an existing free pointer slot.
    if(size<oldCapacityLimit) {
        unsigned int next=1+size;
        unsigned int item=*value;
        self->size=next;
        self->data[next-1]=item;
        return;
    }
    // Phase 2: choose capacity in 32-bit pointer words and guard the replacement allocation.
    if(kMaxWordCapacity-oldCapacityLimit<1) vectorLengthError();
    VectorBuffer growthBuffer(&self->capacity);
    unsigned int additionalSlots=self->size+1-self->capacity;
    unsigned int capacity=*(volatile unsigned int*)&self->capacity;
    if(additionalSlots>kMaxWordCapacity-capacity) vectorLengthError();
    if(capacity<kSmallWordGrowthLimit) {
        unsigned int growth=((1+capacity)*3)/5;
        capacity+=*(growth<additionalSlots?&additionalSlots:&growth);
    }
    else if(capacity<kMediumWordGrowthLimit) {
        unsigned int growth=(capacity+1)/2;
        capacity+=*((growth<additionalSlots)?&additionalSlots:&growth);
    }
    else capacity=kMaxWordCapacity;
    if(capacity>kMaxWordCapacity) vectorLengthError();
    // Phase 3: append first, then transfer existing pointer words and clear the old slots.
    unsigned int* data=(unsigned int*)appMallocDummy__FUl(4*capacity);
    unsigned int oldSize=self->size;
    growthBuffer.data=data;
    (data+oldSize)[growthBuffer.size]=*value;
    *(volatile unsigned int*)&growthBuffer.offset=oldSize;
    growthBuffer.capacity=capacity;
    growthBuffer.size += 1;
    unsigned int* oldData=self->data;
    unsigned int oldWordCount=((int)(self->data+self->size)-(int)oldData)/4;
    growthBuffer.offset=oldSize-oldWordCount;
    memcpy(growthBuffer.offset+data,oldData,oldWordCount*4);
    memset(oldData,0,4*oldWordCount);
    // Phase 4: publish the replacement; its guard now frees the old allocation.
    unsigned int* oldAllocation=self->data;
    *(volatile unsigned int*)&growthBuffer.size=oldWordCount+growthBuffer.size;
    unsigned int* replacement=growthBuffer.data;
    *(volatile unsigned int*)&self->size=0;
    unsigned int oldCapacity=self->capacity;
    self->capacity=growthBuffer.capacity;
    growthBuffer.data=oldAllocation;
    growthBuffer.capacity=oldCapacity;
    self->size=growthBuffer.size;
    self->data=replacement;
    growthBuffer.size=0;
}
}


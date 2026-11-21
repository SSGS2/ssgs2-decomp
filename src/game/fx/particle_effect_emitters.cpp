#include <decomp/static_init.h>
#include "classes/CSharedDoc.h"
#include "classes/CGolfDoc.h"
#include "classes/Object801AAED8.h"
#include "sdk_protos.h"
#include "game/types.h"
#include "game/wvector.h"
#include "classes/CStarDust2.h"
#include "classes/CParticle.h"
#include "classes/CShotParticle.h"
#include "classes/CParticleEffect.h"
#include "classes/W3dAniSpr.h"
#include "classes/CLightColumn.h"





extern "C" void* __nw__FUl(unsigned int);
extern "C" void __dl__FPv(void*);

// Initialize the SDK mutex at the supplied in-object address.
inline void InitRecursiveMutex(void* p) { OSInitMutex((OSMutex*)p); }
// Acquire the supplied SDK mutex; pool helpers may acquire it recursively.
inline void LockMutex(void* p) { OSLockMutex((OSMutex*)p); }
// Release one acquisition of the supplied SDK mutex.
inline void UnlockMutex(void* p) { OSUnlockMutex((OSMutex*)p); }

// Allocate object storage through the game allocator.
inline void* operator new(size_t size) { return __nw__FUl(size); }
void operator delete(void*) throw();

// Byte-derived member addresses retain CodeWarrior's aliasing/scheduling at resource boundaries.
#define FIELD_OFFSET(Type, member) ((int)&((Type*)0)->member)

extern "C" {

void* appMallocUni__FUl(unsigned int);
void appFreeUni__FPv(void*);
void ResourceManager_ReleaseId(void*, void*);
void __dt__13WInstanceLockFv(void*);
void ParticleBitmapRegistry_AddNamedBitmap(void*, const char*, const char*);
int strcmp(const char*, const char*);
enum {
    kEffectSlotCapacity = 20,
    kPoolAllocationBatchSize = 16,
    kInitialBitmapBucketCount = 8,
    kRandomSampleModulo = 1023,
    kNameHashFirstByteMask = 0x2A,
    kNameHashLastByteMask = 0x15,
    kNameHashLengthShift = 6,
    kParticleRecordBytes = 0x3C,
    kShotTrailParticleBytes = 0x28,
    kShotTrailCapacity = 100,
    kShotTrailVertexPairBytes = 0x20,
    kShotTrailVertexFormat = 0xF001,
    kShotTrailIndexFormat = 3,
    kBoxEmissionShape = 0,
    kSphereEmissionShape = 1,
    kBallEffectPartyMode = 3,
    kBallVisibilityItemType = 3
};

struct ParticleEffectStrings {
    char taskName[11];
    char powerShot[15];
    char grass[3];
    char bunker[9];
    char dust[3];
    char fragment[5];
    char ripple[5];
    char effectFileFormat[7];
    char coinSound[15];
    char holeCup[13];
    char ballObject[7];
    char shockwaveBitmap[14];
    char holeCupBitmap[20];
    char dustBitmap[10];
    char grassBitmap[11];
    char bunkerBitmap[12];
    char fragmentBitmap[20];
    char deferredEffect[5];
    char rippleBitmap[12];
    char actorTypeName[16];
    char padding[3];
};
typedef char ParticleStringPoolSize[(sizeof(ParticleEffectStrings) == 0xD8) ? 1 : -1];
typedef char ParticleTypeStringOffset[(FIELD_OFFSET(ParticleEffectStrings, actorTypeName) == 0xC5) ? 1 : -1];
extern const ParticleEffectStrings particleEffectStrings;
extern void* g_resourceManager;
void* ResourceManager_LoadBitmap(void*, const char*, int, int, int);

// Named pools maintain three circular rings: active nodes, free nodes, and
// allocated blocks. The first node of each allocation is a block header; only
// the remaining batchSize-1 nodes enter the free ring. Nodes borrow name bytes
// and hash buckets chain through hashNext. Pool destruction frees blocks/buckets;
// the containing effect/bitmap owner destroys payloads before pool cleanup.
// Locks nest (the same mutex is acquired by callers and list/hash helpers).
// allocationBatchSize must be >= 2; enabled hash bucket counts must be powers of
// two because lookup uses hash & (count-1). These helpers do not validate either.
struct NamedPoolState {
    int activeHead;
    int activeCursor;
    int allocationHead;
    int freeHead;
    int allocationBatchSize;
    int bucketMask;
    int bucketCount;
    int* buckets;
    OSMutex mutex;
};

class InstanceLock_u801A97D0 {
public:
    void* mutex;
    // Leave lock storage uninitialized for the original default-construction path.
    InstanceLock_u801A97D0() {}
    // Acquire the supplied mutex.
    InstanceLock_u801A97D0(void* m) : mutex(m) { LockMutex(m); }
    // Release the mutex acquisition owned by this lock guard.
    ~InstanceLock_u801A97D0() { UnlockMutex(mutex); }
};

struct ParticleBitmapRegistryState {
    int slots[kEffectSlotCapacity];
    int lastSlotIndex;
    int usedSlotCount;
    int persistentSlotCount;
    NamedPoolState namedPoolStorage;
};

NamedPoolState* NamedParticlePool_Ctor(NamedPoolState*, int, int);

struct PoolAllocationNode {
    char pad00[0xC];
    PoolAllocationNode* prev;
    PoolAllocationNode* next;
};

struct NamedPoolCleanupState {
    char pad00[0x8];
    PoolAllocationNode* allocationHead;
    char pad0C[0x10];
    int* buckets;
    OSMutex mutex;
};

struct ParticleEffectItem {
    virtual void destroy(int);
    virtual void update(float);
    virtual void reserved();
    virtual void begin();
    char reserved04[0x14];
    unsigned char active;
};

struct ParticleEffectPoolState {
    void* slots[kEffectSlotCapacity];
    int lastSlotIndex;
    int usedSlotCount;
    int persistentSlotCount;
};

struct ParticleEffectInsertState {
    ParticleEffectItem* slots[kEffectSlotCapacity];
    int lastSlotIndex;
    int usedSlotCount;
    int persistentSlotCount;
    char namedPoolStorage[0x14];
    unsigned int bucketMask;
    int bucketCount;
    void** buckets;
    OSMutex mutex;
};

struct EffectSpriteBinding { char precedingBitmap[0x14]; int bitmapId; };

struct BitmapLookupNode {
    void* item;
    const char* name;
    char reserved08[0x0C];
    BitmapLookupNode* hashNext;
    char reserved18[8];
    int value;
};

void ParticleEffectPool_AddAndStart(ParticleEffectInsertState* self, ParticleEffectItem* effect, const char* name, int persistent);

struct ParticleEffectDestroyState {
    void* slots[kEffectSlotCapacity];
    char slotCounters[0x0C];
    void* activeHead; // 0x5C
    void* activeCursor; // 0x60
    void* allocationHead; // 0x64
    char poolParameters[0x10];
    void* buckets; // 0x78
    OSMutex mutex; // 0x7C
};

// Separate allocations: payload is a 32-byte path plus resource handle;
// each ring/hash link is 24 bytes and only borrows its payload pointer.
struct BitmapPayload { char path[0x20]; void* resource; };
typedef char BitmapPayload_size[(sizeof(BitmapPayload) == 0x24) ? 1 : -1];
struct BitmapPoolNode {
    void* data;
    int nameAddress;
    unsigned char flag;
    char pad09[3];
    BitmapPoolNode* prev;
    BitmapPoolNode* next;
    unsigned int hashNextAddress;
};
typedef char BitmapPoolNode_size[(sizeof(BitmapPoolNode) == 0x18) ? 1 : -1];

struct BitmapRegistryPoolState {
    char pad00[0x5C];
    BitmapPoolNode* activeHead; // 0x5C
    BitmapPoolNode* activeCursor; // 0x60
    BitmapPoolNode* allocationHead; // 0x64
    BitmapPoolNode* freeHead; // 0x68
    int allocationBatchSize; // 0x6C
    unsigned int bucketMask; // 0x70
    char pad74[4];
    unsigned int* buckets; // 0x78
    OSMutex mutex;
};

typedef Vec3f Vec3;
typedef Arr3 Vec3i;

struct NamedPoolNode {
    void* item;          // 0x00
    const char* name;    // 0x04
    unsigned char flag;  // 0x08
    char pad09[3];
    NamedPoolNode* prev;         // 0x0C
    NamedPoolNode* next;         // 0x10
    NamedPoolNode* hashNext;     // 0x14
};

struct ParticleEffectState {
    char pad00[0x1C];
    NamedPoolNode* activeHead;          // 0x1C
    NamedPoolNode* activeCursor;       // 0x20
    NamedPoolNode* allocationHead;         // 0x24
    NamedPoolNode* freeHead;     // 0x28
    int allocationBatchSize;           // 0x2C
    unsigned int bucketMask;   // 0x30
    char pad34[4];
    NamedPoolNode** buckets;     // 0x38
    OSMutex mutex;      // 0x3C
    void* effectManager;        // 0x54
    Vec3 emissionPosition;          // 0x58
    float bouncePlaneY;           // 0x64
    float emissionDurationOverride;           // 0x68
    char pad6C[0x38];
    void* lightColumn;      // 0xA4
};

extern void CPowerShot_Ctor(void*);
extern void* CShotParticle_Ctor(void*);
extern void CGrassParticle_Ctor(void*, int, int);
extern void CBunkerShotParticle_Ctor(void*, int, int);
extern void CBunkerParticle_Ctor(void*);
extern void CDust_Ctor(void*, int);
extern void CFragment_Ctor(void*);
extern void CRipple_Ctor(void*);
extern void CCoin_Ctor(void*, const char*, Vec3*);
extern void* GolfBall__Fv(void);
extern void* PreData__9CGolfBallFi(void*, int);
extern void __as__7WVectorFRC7WVector(void*, const void*);
extern void __mi__FRC7WVectorRC7WVector(void*, const void*, const void*);
extern void __ml__FRC7WVectorf(void*, const void*, float);
extern void* String_FormatTemporary(const char*, ...);
extern int CQuadTree_GetInstance(void*);
extern void GetGroundPoint__9CQuadTreeF7WVectorbPib(void*, int, Vec3*, int, int, int);
extern void* ParticleEmitter_LoadDefinition(void*, const char*);
extern CSharedDoc* g_sharedDoc;
extern void* g_soundManager;
extern void* g_sceneManager;
extern void Sound_PlayByName(void*, const char*, int, int, int, int, float, float, float);
extern void CEnergyCenter_Ctor(void*, int, Vec3*, float);
extern void CStarDust_Ctor(void*);
extern void CStarDust2_Ctor(void*);
extern void* PartyItemSlots_GetInstance(void);
extern int PartyItemSlots_GetItemType(void*, int);
extern void* NamedEffectList_FindByName(void*, const char*);
extern void WPuppet_SetOpacityRecursive(void*, int);

// Hash an effect name's endpoint bytes and length under the pool mutex.
static inline unsigned int NamedEmitter_Hash(ParticleEffectState* t, const char* s) {
    void* m = &t->mutex;
    LockMutex(m);
    int len = strlen(s);
    unsigned int h = (((unsigned char)s[0] & kNameHashFirstByteMask) | ((unsigned char)s[len > 0 ? len - 1 : 0] & kNameHashLastByteMask) | (len << kNameHashLengthShift)) & t->bucketMask;
    UnlockMutex(m);
    return h;
}

// Insert a node before the circular-list head, creating a singleton list if necessary.
static inline NamedPoolNode* NamedEmitter_LinkFront(ParticleEffectState* t, NamedPoolNode* n, NamedPoolNode* first) {
    void* m = &t->mutex;
    LockMutex(m);
    if (!first) {
        n->prev = n;
        n->next = n;
        UnlockMutex(m);
        first = n;
    } else {
        n->next = first;
        n->prev = first->prev;
        first->prev = n;
        n->prev->next = n;
        UnlockMutex(m);
    }
    return first;
}

// Insert a node and publish the active-list head under the mutex.
static inline void NamedEmitter_LinkCursor(ParticleEffectState* t, NamedPoolNode* first, NamedPoolNode*& n) {
    void* m = &t->mutex;
    LockMutex(m);
    if (!first) {
        n->prev = n;
        n->next = n;
        UnlockMutex(m);
    } else {
        n->next = first;
        n->prev = first->prev;
        first->prev = n;
        n->prev->next = n;
        UnlockMutex(m);
        n = first;
    }
    t->activeHead = n;
}

// Detach a circular-list node and return the next head, or null for the last node.
static inline NamedPoolNode* NamedEmitter_Unlink(ParticleEffectState* t, NamedPoolNode* n) {
    void* m = &t->mutex;
    LockMutex(m);
    n->next->prev = n->prev;
    n->prev->next = n->next;
    NamedPoolNode* next = n->next;
    if (next == n) next = 0;
    UnlockMutex(m);
    return next;
}

// Prepend a named node to its hash bucket under the pool mutex.
static inline void NamedEmitter_BucketAdd(ParticleEffectState* t, NamedPoolNode* n, unsigned int h) {
    void* m = &t->mutex;
    LockMutex(m);
    n->hashNext = t->buckets[h];
    t->buckets[h] = n;
    UnlockMutex(m);
}

// Borrow the next emitter payload (not its pool node), advancing one circular pass.
static inline void* NamedEmitter_NextActive(ParticleEffectState* t) {
    void* m = &t->mutex;
    void* r;
    LockMutex(m);
    NamedPoolNode* c = t->activeCursor;
    if (c) {
        NamedPoolNode* next = c->next;
        r = c->item;
        if (next == t->activeHead) next = 0;
        t->activeCursor = next;
        UnlockMutex(m);
    } else {
        UnlockMutex(m);
        r = 0;
    }
    return r;
}

// Borrow an emitter payload by name under the lock; null means no registered name.
// (Same body as NamedEmitter_FindByName below; the two copies are separate inlines in the original and are used by different cases.)
static inline void* NamedEmitter_FindByName2(ParticleEffectState* t, const char* name) {
    InstanceLock_u801A97D0 lock(&t->mutex);
    if (name == 0 || t->buckets == 0) return 0;
    unsigned int h = NamedEmitter_Hash(t, name);
    NamedPoolNode* node = t->buckets[h];
    while (node) {
        if (strcmp((const char*)node->name, name) == 0) return node->item;
        node = node->hashNext;
    }
    return 0;
}

// Borrow an emitter payload by name under the lock; null means no registered name.
static inline void* NamedEmitter_FindByName(ParticleEffectState* t, const char* name) {
    InstanceLock_u801A97D0 lock(&t->mutex);
    if (name == 0 || t->buckets == 0) return 0;
    unsigned int h = NamedEmitter_Hash(t, name);
    NamedPoolNode* node = t->buckets[h];
    while (node) {
        if (strcmp((const char*)node->name, name) == 0) return node->item;
        node = node->hashNext;
    }
    return 0;
}

struct PEffShot { char d[0x134];
    // Construct the effect payload through its original constructor; storage is owned by the slot pool.
    PEffShot() { CPowerShot_Ctor(this); } };
struct PEffShot2 { char d[0x134];
    // Construct the effect payload through its original constructor; storage is owned by the slot pool.
    PEffShot2() { CShotParticle_Ctor(this); } };
struct PEffGrass { char d[0x138];
    // Construct the effect payload through its original constructor; storage is owned by the slot pool.
    PEffGrass(int a, int b) { CGrassParticle_Ctor(this, a, b); } };
struct PEffBunkerShot { char d[0x14C];
    // Construct the effect payload through its original constructor; storage is owned by the slot pool.
    PEffBunkerShot(int a, int b) { CBunkerShotParticle_Ctor(this, a, b); } };
struct PEffBunker { char d[0x138];
    // Construct the effect payload through its original constructor; storage is owned by the slot pool.
    PEffBunker() { CBunkerParticle_Ctor(this); } };
struct PEffDust { char d[0x13C];
    // Construct the effect payload through its original constructor; storage is owned by the slot pool.
    PEffDust(int a) { CDust_Ctor(this, a); } };
struct PEffFragment { char d[0x144];
    // Construct the effect payload through its original constructor; storage is owned by the slot pool.
    PEffFragment() { CFragment_Ctor(this); } };
struct PEffRipple { char d[0x130];
    // Construct the effect payload through its original constructor; storage is owned by the slot pool.
    PEffRipple() { CRipple_Ctor(this); } };
struct PEffRecv { char d[0x120];
    // Load the named emitter definition into this owned effect payload.
    PEffRecv(const char* n) { ParticleEmitter_LoadDefinition(this, n); } };
struct PEffLin { char d[0x14C];
    // Construct the effect payload through its original constructor; storage is owned by the slot pool.
    PEffLin(const char* n, Vec3* v) { CCoin_Ctor(this, n, v); } };
struct PEffCol { char d[0x154];
    // Construct the effect payload through its original constructor; storage is owned by the slot pool.
    PEffCol(int n, Vec3* v, float f) { CEnergyCenter_Ctor(this, n, v, f); } };
struct PEffA { char d[0x138];
    // Construct the effect payload through its original constructor; storage is owned by the slot pool.
    PEffA() { CStarDust_Ctor(this); } };
struct PEffB2 { char d[0x138];
    // Construct the effect payload through its original constructor; storage is owned by the slot pool.
    PEffB2() { CStarDust2_Ctor(this); } };

extern "C" void* __nwa__FUl(unsigned int);
extern "C" void EmitterParticle_DefaultCtor();
struct Elem_u801A97D0 {
    char pad[0x3c];
    // Invoke the empty particle-element constructor; spawn fills particle state later.
    Elem_u801A97D0() { EmitterParticle_DefaultCtor(); }
    // Allocate particle-array storage through the game array allocator.
    static void* operator new[](size_t sz) { return __nwa__FUl(sz); }
};

void VertexBuffer_Create(unsigned int, int, int, int, void**, int);
void Resource_GetOffsetPtr(void*, int, int, void**, int);
void GraphicsBuffer_Flush(void*);
void IndexBuffer_Create(unsigned int, int, int, int, void**, int, int);
void IndexBuffer_GetOffsetPtr(void*, int, int, void**, int);
void IndexBuffer_Flush(void*);
extern const char __vt__13CShotParticle[];

float Magnitude__7WVectorCFv(void*);
void Particle_UpdateLifetimeSizeOpacityAndWind(void*, int, float);
struct Emitter;
void ParticleEmitter_InitializeNextParticle(Emitter*);
extern const char s_cameraTransform[];
extern const float kParticleEmissionDistanceScale; // 160: distance at which the emission-distance ratio reaches one
extern const float kFloat_1_particles;
extern const float kFloat_0_805DF3CC;
extern const float kFloat_NegOne_ParticleEmitter; // -1: bounce/spin sign
extern const float kFloat_Pi_ParticleEmitter; // pi: random emission angle scale
extern const float kFloat_Two_ParticleEmitter; // 2: full-turn random angle multiplier
extern const float kParticleEmissionRadiusEpsilon; // 0.00001: zero-radius tolerance
extern const float kParticleGravityAcceleration; // 34.2952957: particle gravity constant
extern const float kParticleOpacityByteScale; // 255: surface opacity normalization
extern void* g_particleCourseEffects; // Cached course-effects owner; +8 supplies surface properties.
void* CWind_GetInstance();
void CWind_GetWindVectorAtPoint(void*, void*, void*, float);
float CourseSurfaceProperties_GetParticleWindScale(void*, int);
void __ml__FfRC7WVector(float*, const float*, float);
void __apl__7WVectorFRC7WVector(float*, const float*);
int CourseSurfaceProperties_GetParticleOpacity(void*, int);

extern char s_particleEffectTypeInfo;

void CParticleEffect_Draw(void*);

void IntPair_Set(void*, const char*, void*);
void* ActorFactoryRegistry_GetInstance();
void CParticleEffect_Create();
void StringMap_SetByName(void*, void (*)(), const char*);
void* TaskRegistry_GetInstance();
void TaskRegistry_AppendFactory(void*, const char*);
extern float g_particleEffectStaticTwoPi;
extern const float kFloat_6_2831855;
extern char s_actorTypeInfo;
extern const char s_particleEffectActorFactoryName[];
extern const char s_particleEffectTaskFactoryName[];

struct Link { char payload[0xC]; Link* prev; Link* next; };
void __dt__18NativeBase80201D18Fv(void*, int);
extern char s_particleVtable[64];

int rand();
void __amu__7WVectorFf(void*, float);
void Matrix_SetAxisRotation(void*, float, int);
void Matrix_TransformPoint(void*, const void*, const void*);
void Vec3_DivideScalar(float*, float, float*);
extern const float kParticleRandomSampleScale; // 1/1024: original random-sample scale
// Particle state contract: position/velocity, emitter position, bounce plane,
// camera distance and wind samples share game world coordinates (+Y vertical).
// Bounds add directly to world position; the box velocity is transformed by the
// emitter matrix. Sphere velocity is radial and does not use that matrix here.
// dt, lifetimes and emission/frame intervals use the caller's same time scale;
// the absolute time unit is not established here. Axis-matrix angles are radians;
// particle spin uses the definition's angular unit, which this unit does not convert.
// Size/opacity interpolate linearly over initialLifetime; surface opacity is a
// separate byte/255 multiplier. The overwrite ring advances modulo capacity;
// activeParticleCount counts emits minus expirations, not a fresh population scan.
// Particle records are 60 bytes; vectors retain their original float-array representation.
struct Particle {
    float position[3];
    float velocity[3];
    float opacity;
    unsigned char active;
    float rotation;
    float angularVelocity;
    float size;
    int spriteFrame;
    float frameTimeRemaining;
    float initialLifetime;
    float lifetimeRemaining;
};
struct EmissionBounds { float minimum[3]; float maximum[3]; };
// Complete emitter view for definition loading and spawning; shared fields use Object801AAED8.
struct Emitter {
    int emissionShape;
    float initialOpacity, finalOpacity, initialSize, finalSize;
    float minimumVerticalSpeed, maximumVerticalSpeed;
    float minimumHorizontalSpeed, maximumHorizontalSpeed;
    float minimumAngularSpeed, maximumAngularSpeed;
    float gravityScale;
    float minimumLifetime, maximumLifetime, spriteFrameInterval;
    float emissionInterval, bounceElasticity, emissionTimeRemaining, bouncePlaneY;
    int definitionFlags;
    unsigned char active;
    char reserved51[3];
    Particle* particles;
    int activeParticleCount;
    W3dAniSpr* sprite;
    float transform[12];
    float rotation[3];
    float acceleration[3];
    EmissionBounds bounds;
    float emissionRadius;
    unsigned char randomizeSpriteFrame;
    char reservedC5[3];
    float spriteSize;
    int initialSpriteFrame;
    char spriteName[0x20];
    int particleCapacity;
    int nextParticleIndex;
    void* positionLink;
    float position[3];
    float emissionTimer;
    int particlesPerEmission;
    unsigned char drawingEnabled;
    char reserved111[3];
    int surfaceIndex;
    float surfaceOpacityMultiplier;
};
typedef char ParticleRecordSize[(sizeof(Particle) == 0x3C) ? 1 : -1];
#define EMITTER_OFFSET(field, offset) typedef char EmitterOffset_##field[((int)&((Emitter*)0)->field == offset) ? 1 : -1]
EMITTER_OFFSET(particles, 0x54);
EMITTER_OFFSET(sprite, 0x5C);
EMITTER_OFFSET(bounds, 0xA8);
EMITTER_OFFSET(spriteName, 0xD0);
EMITTER_OFFSET(position, 0xFC);
EMITTER_OFFSET(surfaceOpacityMultiplier, 0x118);
#undef EMITTER_OFFSET

// Return the original integer random sample as a float, in the range 0 through 1022.
static inline float rndr() {
    int r = rand() % kRandomSampleModulo;
    return (float)r;
}
// Scale the original modulo-1023 sample into the emitter random fraction.
static inline float rndf() {
    int r = rand() % kRandomSampleModulo;
    return (float)r * kParticleRandomSampleScale;
}
// Interpolate a random value between the supplied bounds, preserving multiply/add order.
static inline float RandomRange(const float& minimum, const float& maximum) {
    float range = (maximum - minimum) * rndr();
    return range * kParticleRandomSampleScale + minimum;
}
// Cap the distance ratio at one without clamping negative values.
static inline float CapRatioAtOne(float ratio) {
    return kFloat_1_particles < ratio ? kFloat_1_particles : ratio;
}
// Truncate elapsed time to a count of complete emission intervals.
static inline int WholePeriods(float elapsed, float duration) {
    return (int)(elapsed / duration);
}


void Vec3_Zero(void*);
void* g_taskManager;
void* Resource_OpenResolvedPath(void*, const char*, int, int, int, int);
void Resource_Release(void*);
void Matrix_SetIdentityAffine(void*);
void* W3dAniSpr_Create(void*, float, float, void*, int);
void* Object_GetChild(void*);
void ResourceHandle_dtor(void*, int);
struct ResGuard {
    void** slot;
    // Borrow the address of a local resource slot for scope-exit cleanup.
    ResGuard(void** s) { slot = s; }
    // Release the resource still in the borrowed local slot and clear the slot.
    ~ResGuard() { if (*slot) { Resource_Release(*slot); *slot = 0; } }
};
struct Resource {
    virtual void a0();
    virtual void a1();
    virtual void read(void*, int);
};

// Read one definition field through the resource interface; arrays include their full extent.
inline void ReadEmitterField(Resource* resource, void* field, int size) {
    resource->read(field, size);
}

// Find a bitmap registry payload through the named-pool buckets.
static inline void* FindManagerItem(ParticleEffectInsertState* self, const char* name) {
    void* mutex = (char*)self + FIELD_OFFSET(ParticleEffectInsertState, mutex);
    LockMutex(mutex);
    if (name == 0 || self->buckets == 0) {
        UnlockMutex(mutex);
        return 0;
    }
    void* mutex2 = (char*)self + FIELD_OFFSET(ParticleEffectInsertState, mutex);
    LockMutex(mutex2);
    unsigned int len = strlen(name);
    unsigned int lastIdx = (len - 1) & ((int)((-(int)len) & ~((int)len)) >> 31);
    unsigned int pos = (((unsigned char)name[0]) & kNameHashFirstByteMask) | (((unsigned char)name[lastIdx]) & kNameHashLastByteMask) | (len << kNameHashLengthShift);
    pos &= self->bucketMask;
    UnlockMutex(mutex2);
    BitmapLookupNode* node = (BitmapLookupNode*)self->buckets[pos];
    while (node) {
        if (strcmp(node->name, name) == 0) {
            void* found = node->item;
            UnlockMutex(mutex);
            return found;
        }
        node = node->hashNext;
    }
    UnlockMutex(mutex);
    return 0;
}

// Integrate velocity, reflecting vertical velocity if the particle crosses its bounce plane.
static inline void AdvanceParticle(char* p, unsigned int particleByteOffset, float dt) {
    void* base;
    Particle* particle;
    // Bounce only on a crossing from above the plane; this is not penetration repair.
    if (((Object801AAED8*)p)->bounceElasticity > kFloat_0_805DF3CC) {
        base = ((Object801AAED8*)p)->particleStorage;
        particle = (Particle*)((char*)base + particleByteOffset);
        float sampledHeight = particle->position[1];
        float particleSize = particle->size;
        float bouncePlaneY = ((Object801AAED8*)p)->bouncePlaneY;
        float bottomHeight = sampledHeight - particleSize;
        if (bottomHeight > bouncePlaneY) {

            float previousHeight = sampledHeight;
            float verticalSpeed = particle->velocity[1]; float projectedBottom = dt * verticalSpeed + previousHeight;
            float projectedHeight = projectedBottom;
            projectedBottom = projectedHeight - particleSize;
            if (projectedBottom < bouncePlaneY) { float verticalStep = dt * verticalSpeed;  float bounceDisplacement[3];

                float crossingVelocity[3]; __ml__FfRC7WVector(crossingVelocity, particle->velocity, (bottomHeight / verticalStep));

                __ml__FRC7WVectorf(bounceDisplacement, crossingVelocity, dt);
                base = ((Object801AAED8*)p)->particleStorage;
                __apl__7WVectorFRC7WVector((float*)((char*)base + particleByteOffset), bounceDisplacement);
                base = ((Object801AAED8*)p)->particleStorage;
                unsigned int particleOffset = particleByteOffset;
                particle = (Particle*)((char*)base + particleOffset);
                float negativeOne = kFloat_NegOne_ParticleEmitter;
                float elasticityScratch = ((Object801AAED8*)p)->bounceElasticity;
                float verticalSpeedScratch = particle->velocity[1];
                float elasticity = elasticityScratch;
                float bounceScale = negativeOne * elasticity;
                elasticityScratch = bounceScale;
                float velocity = elasticityScratch * verticalSpeedScratch;
                float reflectedVelocity = velocity;
                verticalSpeedScratch = reflectedVelocity;
                particle->velocity[1] = verticalSpeedScratch;
                return;
            }
        }
    }
    {
        base = ((Object801AAED8*)p)->particleStorage;
        particle = (Particle*)((char*)base + particleByteOffset);
        float displacement[3];
        __ml__FRC7WVectorf(displacement, particle->velocity, dt);
        base = ((Object801AAED8*)p)->particleStorage;
        __apl__7WVectorFRC7WVector((float*)((char*)base + particleByteOffset), displacement);
        particle = (Particle*)((char*)base + particleByteOffset);
    }
}

// Message payload map (request borrows all pointer payloads for this call):
// grass: first/second = constructor integers; bunker/fragments: first = ball
// sample index; start named: first = retained name, second = NamedEmissionParameters*;
// stop named: first = name, null stops all; coin: first = constructor name,
// second 0 selects cup, nonzero ball; energy: first selects cup/ball likewise,
// second integer is scaled by 0.1 for its constructor. Light-column and other
// fixed-effect messages ignore the payload words. Named-start stores its name
// pointer in a pool node, so that name must outlive the registered emitter.
// Virtual slots of an effect object called through raw tables in this unit (slot 2: deleting destructor taking
// a delete flag; slot 6: switches the effect on (1) or off (0), the flag is the only argument).
enum { kEffectVSlotDelete = 2, kEffectVSlotSetActive = 6, kEffectVSlotStop = 6 };
enum ParticleEffectMessage {
    kSpawnPowerShot = 0, kSpawnShotTrail = 2, kSpawnGrass = 3, kSpawnBunkerShot = 4,
    kSpawnBunkerAtBallSample = 5, kStartNamedEmitter = 6, kSpawnDust = 7,
    kSpawnFragments = 8, kStopNamedEmitter = 9, kSpawnRipple = 10,
    kClearTransientEffects = 11, kLightColumnFadeIn = 12, kLightColumnFadeOut = 13,
    kStartLightColumn = 14, kStopLightColumn = 15, kSpawnCoin = 17,
    kSpawnEnergyColumn = 18, kReservedEffect = 19, kSpawnStarDust = 20, kSpawnStarDust2 = 21
};
union EffectMessageWord { int integer; void* pointer; const char* name; };
struct EffectMessage { unsigned int header; unsigned int type; EffectMessageWord first; EffectMessageWord second; };
struct NamedEmissionParameters { Vec3 position; float bouncePlaneY; float duration; };
struct BallEffectState { char precedingRadius[0xC0]; float radius; };
struct BallEffectSample { char precedingNormal[0x64]; Vec3 surfaceNormal; };
struct PositionedParticleEffect { char precedingPosition[8]; Vec3 position; };
struct HoleEffectRecord { char precedingPosition[0xC]; Vec3 position; char followingPosition[8]; };
struct CameraParticleView { char precedingPosition[0x24]; WVector position; };
struct CourseEffectState { char precedingSurface[8]; void* surfaceProperties; };
struct CourseEffectOwner { char precedingCourse[0x4C]; void* courseEffects; };

// Functions in original address order.

// Role aliases select constructor variants without changing their ABI symbols.
typedef PEffShot PowerShotEffectStorage;
typedef PEffShot2 ShotTrailEffectStorage;
typedef PEffGrass GrassEffectStorage;
typedef PEffBunkerShot BunkerShotEffectStorage;
typedef PEffBunker BunkerEffectStorage;
typedef PEffDust DustEffectStorage;
typedef PEffFragment FragmentEffectStorage;
typedef PEffRipple RippleEffectStorage;
typedef PEffRecv NamedEmitterStorage;
typedef PEffLin CoinEffectStorage;
typedef PEffCol EnergyColumnEffectStorage;
typedef PEffA StarDustEffectStorage;
typedef PEffB2 SecondStarDustEffectStorage;

// Dispatch shot/landing effects, named emitters, light-column fades, and party item visuals.
// Payload contract is selected by request->type; first/second are a tagged
// union, not interchangeable words. Spawned objects transfer to effectManager.
// Named starts retain a borrowed name pointer until registry destruction;
// second.pointer parameters are borrowed only during the call. Named stops
// require an existing name, or null to stop all active emitters.
// Ball samples, hole positions and emitted effect positions use world XYZ.
void CParticleEffect_HandleMsg(ParticleEffectState* self, EffectMessage* request) {
    Vec3 fragmentPosition, surfaceNormal, ripplePosition, coinPosition, energyPosition, dustPosition, ballRadiusOffset, fragmentPositionScratch, surfaceRadiusOffset, ballPosition, holeCoinPosition, holeEnergyPosition;

    switch (request->type) {
    case kSpawnPowerShot: {
        PowerShotEffectStorage* p = new PowerShotEffectStorage();
        ParticleEffectPool_AddAndStart((ParticleEffectInsertState*)self->effectManager, (ParticleEffectItem*)p, particleEffectStrings.powerShot, 0);
        break;
    }
    case kSpawnShotTrail: {
        ShotTrailEffectStorage* p = new ShotTrailEffectStorage();
        ParticleEffectPool_AddAndStart((ParticleEffectInsertState*)self->effectManager, (ParticleEffectItem*)p, 0, 0);
        break;
    }
    case kSpawnGrass: {
        GrassEffectStorage* p = new GrassEffectStorage(request->first.integer, request->second.integer);
        ParticleEffectPool_AddAndStart((ParticleEffectInsertState*)self->effectManager, (ParticleEffectItem*)p, particleEffectStrings.grass, 0);
        break;
    }
    case kSpawnBunkerShot: {
        BunkerShotEffectStorage* p = new BunkerShotEffectStorage(4, 100);
        ParticleEffectPool_AddAndStart((ParticleEffectInsertState*)self->effectManager, (ParticleEffectItem*)p, particleEffectStrings.bunker, 0);
        break;
    }
    case kSpawnBunkerAtBallSample: {
        BunkerEffectStorage* p;
        p = new BunkerEffectStorage();
        __as__7WVectorFRC7WVector(&((PositionedParticleEffect*)p)->position, PreData__9CGolfBallFi(GolfBall__Fv(), request->first.integer));
        ParticleEffectPool_AddAndStart((ParticleEffectInsertState*)self->effectManager, (ParticleEffectItem*)p, particleEffectStrings.bunker, 0);
        break;
    }
    case kSpawnDust: {
        DustEffectStorage* p;
        p = new DustEffectStorage(1);
        float f = ((BallEffectState*)GolfBall__Fv())->radius;
        ballRadiusOffset.y = f;
        ballRadiusOffset.x = 0.0f;
        ballRadiusOffset.z = 0.0f;
        __mi__FRC7WVectorRC7WVector(&dustPosition, GolfBall__Fv(), &ballRadiusOffset);
        __as__7WVectorFRC7WVector(&((PositionedParticleEffect*)p)->position, &dustPosition);
        ParticleEffectPool_AddAndStart((ParticleEffectInsertState*)self->effectManager, (ParticleEffectItem*)p, particleEffectStrings.dust, 0);
        break;
    }
    case kSpawnFragments: {
        __as__7WVectorFRC7WVector(&surfaceNormal, &((BallEffectSample*)PreData__9CGolfBallFi(GolfBall__Fv(), request->first.integer))->surfaceNormal);
        float ballRadius = ((BallEffectState*)GolfBall__Fv())->radius;
        __ml__FRC7WVectorf(&surfaceRadiusOffset, &surfaceNormal, ballRadius);
        __mi__FRC7WVectorRC7WVector(&fragmentPositionScratch, PreData__9CGolfBallFi(GolfBall__Fv(), request->first.integer), &surfaceRadiusOffset);
        __as__7WVectorFRC7WVector(&fragmentPosition, &fragmentPositionScratch);
        FragmentEffectStorage* p;
        p = new FragmentEffectStorage();
        ((void (*)(void*, Vec3*))(*(void***)p)[7])(p, &surfaceNormal);
        __as__7WVectorFRC7WVector(&((PositionedParticleEffect*)p)->position, &fragmentPosition);
        ParticleEffectPool_AddAndStart((ParticleEffectInsertState*)self->effectManager, (ParticleEffectItem*)p, particleEffectStrings.fragment, 0);
        break;
    }
    case kSpawnRipple: {
        void* p = GolfBall__Fv();
        *(Vec3i*)&ballPosition = *(Vec3i*)p;
        GetGroundPoint__9CQuadTreeF7WVectorbPib(&ripplePosition, CQuadTree_GetInstance(p), &ballPosition, 1, 0, 1);
        float dy = ((BallEffectState*)GolfBall__Fv())->radius;
        ripplePosition.y = ripplePosition.y + dy;
        RippleEffectStorage* r;
        r = new RippleEffectStorage();
        __as__7WVectorFRC7WVector(&((PositionedParticleEffect*)r)->position, &ripplePosition);
        ParticleEffectPool_AddAndStart((ParticleEffectInsertState*)self->effectManager, (ParticleEffectItem*)r, particleEffectStrings.ripple, 0);
        break;
    }
    case kStartNamedEmitter: {
        // Phase 1: reuse a named emitter or allocate its payload and a pooled index node.
        void* found = NamedEmitter_FindByName2(self, request->first.name);
        if (found == 0) {
            void* item = new NamedEmitterStorage((const char*)String_FormatTemporary(particleEffectStrings.effectFileFormat, request->first.pointer));
            const char* name2 = request->first.name;
            {
                InstanceLock_u801A97D0 lockA(&self->mutex);
                NamedPoolNode* tail;
                {
                InstanceLock_u801A97D0 lockB(&self->mutex);
                if (self->freeHead == 0) {
                    NamedPoolNode* pool = (NamedPoolNode*)appMallocUni__FUl(self->allocationBatchSize * 0x18);
                    self->allocationHead = NamedEmitter_LinkFront(self, pool, self->allocationHead);
                    if (self->allocationBatchSize > 1) {
                        NamedPoolNode* cur = pool + 1;
                        for (int i = 1; i < self->allocationBatchSize; ++i) {
                            self->freeHead = NamedEmitter_LinkFront(self, cur, self->freeHead);
                            cur = cur + 1;
                        }
                    }
                }
                tail = self->freeHead;
                self->freeHead = NamedEmitter_Unlink(self, tail);
                }
                tail->item = item;
                tail->flag = 0;
                tail->name = name2;
                if (name2) {
                    unsigned int h2 = NamedEmitter_Hash(self, name2);
                    NamedEmitter_BucketAdd(self, tail, h2);
                }
                NamedEmitter_LinkCursor(self, self->activeHead, tail);
            }
            found = item;
        }
        {
            // Phase 2: start at the requested world position/plane; a positive duration overrides the file.
            NamedEmissionParameters* parameters = (NamedEmissionParameters*)request->second.pointer;
            __as__7WVectorFRC7WVector(&self->emissionPosition, &parameters->position);
            self->bouncePlaneY = parameters->bouncePlaneY;
            self->emissionDurationOverride = parameters->duration;
            ((Object801AAED8*)found)->active = 1;
            __as__7WVectorFRC7WVector(((Emitter*)found)->position, &self->emissionPosition);
            ((Object801AAED8*)found)->bouncePlaneY = self->bouncePlaneY;
            float f2 = ((CParticleEffect*)self)->emissionDurationOverride;
            if (f2 > 0.0f) ((Object801AAED8*)found)->emissionTimeRemaining = f2;
        }
        break;
    }
    // Stop changes payload activity; it does not unlink or free the pool entries.
    case kStopNamedEmitter: {
        const char* nm = request->first.name;
        if (nm != 0) {
            void* node = NamedEmitter_FindByName(self, nm);
            ((Object801AAED8*)node)->active = 0;
        } else {
            void* found;
            {
                InstanceLock_u801A97D0 lock(&self->mutex);
                self->activeCursor = self->activeHead;
                found = NamedEmitter_NextActive(self);
            }
            while (found) {
                ((Object801AAED8*)found)->active = 0;
                found = NamedEmitter_NextActive(self);
            }
        }
        break;
    }
    case kClearTransientEffects: {
        void* data = self->effectManager;
        int i = ((ParticleEffectPoolState*)data)->persistentSlotCount;
        void** cur = (void**)((char*)data + i * 4);
        while (i <= ((ParticleEffectPoolState*)data)->lastSlotIndex) {
            if (*cur) {
                ((void (*)(void*, int))(*(void***)*cur)[kEffectVSlotStop])(*cur, 0);
                if (*cur) ((void (*)(void*, int))(*(void***)*cur)[kEffectVSlotDelete])(*cur, 1);
                *cur = 0;
            }
            cur = (void**)((char*)cur + 4);
            ++i;
        }
        ((ParticleEffectPoolState*)data)->usedSlotCount = ((ParticleEffectPoolState*)data)->persistentSlotCount;
        break;
    }
    case kLightColumnFadeIn: {
        void* p = self->lightColumn;
        if (p) ((CLightColumn*)p)->fadeMode = 1;
        break;
    }
    case kLightColumnFadeOut: {
        void* p = self->lightColumn;
        if (p) ((CLightColumn*)p)->fadeMode = 2;
        break;
    }
    case kStartLightColumn: {
        void* p = self->lightColumn;
        if (p) ((void (*)(void*, int))(*(void***)p)[kEffectVSlotSetActive])(p, 1);
        break;
    }
    case kStopLightColumn: {
        void* p = self->lightColumn;
        if (p) ((void (*)(void*, int))(*(void***)p)[kEffectVSlotSetActive])(p, 0);
        break;
    }
    // Position-selecting variants: zero selects the hole record, nonzero the ball.
    case kSpawnCoin: {
        Vec3* src;
        if (request->second.integer == 0) {
            void* g = (*(void**)&((CSharedDoc*)g_sharedDoc)->golfDoc);
            unsigned char idx = ((CGolfDoc*)g)->currentHoleIndex; // 0x3B6
            HoleEffectRecord* hole = (HoleEffectRecord*)((CGolfDoc*)g)->holeRecords + idx; // 0x2BC
            *(Vec3i*)&holeCoinPosition = *(Vec3i*)&hole->position;
            src = &holeCoinPosition;
        } else {
            src = (Vec3*)GolfBall__Fv();
        }
        *(Vec3i*)&coinPosition = *(Vec3i*)src;
        CoinEffectStorage* p = new CoinEffectStorage(request->first.name, &coinPosition);
        Sound_PlayByName(g_soundManager, particleEffectStrings.coinSound, 0, 0, 0, 0, 0.5f, 200.0f, 1.0f);
        ParticleEffectPool_AddAndStart((ParticleEffectInsertState*)self->effectManager, (ParticleEffectItem*)p, particleEffectStrings.holeCup, 0);
        break;
    }
    case kSpawnEnergyColumn: {
        Vec3* src;
        if (request->first.integer == 0) {
            void* g = (*(void**)&((CSharedDoc*)g_sharedDoc)->golfDoc);
            unsigned char idx = ((CGolfDoc*)g)->currentHoleIndex; // 0x3B6
            HoleEffectRecord* hole = (HoleEffectRecord*)((CGolfDoc*)g)->holeRecords + idx; // 0x2BC
            *(Vec3i*)&holeEnergyPosition = *(Vec3i*)&hole->position;
            src = &holeEnergyPosition;
        } else {
            src = (Vec3*)GolfBall__Fv();
        }
        *(Vec3i*)&energyPosition = *(Vec3i*)src;
        EnergyColumnEffectStorage* p = new EnergyColumnEffectStorage(0x1E, &energyPosition, 0.1f * (float)request->second.integer);
        ParticleEffectPool_AddAndStart((ParticleEffectInsertState*)self->effectManager, (ParticleEffectItem*)p, particleEffectStrings.holeCup, 0);
        break;
    }
    case kReservedEffect:
        break;
    // Star-dust variants also adjust the scene ball's visibility before pool transfer.
    case kSpawnStarDust: {
        CSharedDoc* g = g_sharedDoc;
        if (*(int*)&((CSharedDoc*)g)->partyGameMode == kBallEffectPartyMode &&
            (PartyItemSlots_GetItemType(PartyItemSlots_GetInstance(), ((CGolfDoc*)(*(void**)&((CSharedDoc*)g)->golfDoc))->currentPlayer) == kBallVisibilityItemType)) {
            WPuppet_SetOpacityRecursive(NamedEffectList_FindByName(g_sceneManager, particleEffectStrings.ballObject), 0xFF);
        } else {
            WPuppet_SetOpacityRecursive(NamedEffectList_FindByName(g_sceneManager, particleEffectStrings.ballObject), 0);
        }
        StarDustEffectStorage* p = new StarDustEffectStorage();
        ParticleEffectPool_AddAndStart((ParticleEffectInsertState*)self->effectManager, (ParticleEffectItem*)p, particleEffectStrings.holeCup, 0);
        break;
    }
    case kSpawnStarDust2: {
        CSharedDoc* g = g_sharedDoc;
        if (*(int*)&((CSharedDoc*)g)->partyGameMode == kBallEffectPartyMode &&
            (PartyItemSlots_GetItemType(PartyItemSlots_GetInstance(), ((CGolfDoc*)(*(void**)&((CSharedDoc*)g)->golfDoc))->currentPlayer) == kBallVisibilityItemType)) {
            WPuppet_SetOpacityRecursive(NamedEffectList_FindByName(g_sceneManager, particleEffectStrings.ballObject), 0xFF);
        } else {
            WPuppet_SetOpacityRecursive(NamedEffectList_FindByName(g_sceneManager, particleEffectStrings.ballObject), 0);
        }
        SecondStarDustEffectStorage* p = new SecondStarDustEffectStorage();
        ParticleEffectPool_AddAndStart((ParticleEffectInsertState*)self->effectManager, (ParticleEffectItem*)p, particleEffectStrings.holeCup, 0);
        break;
    }
    default:
        break;
    }
}

// Shot particles require no action for this lifecycle hook.
void CShotParticle_vfunc_5() {}

// Set the common shot-effect active byte.
void ParticleEffect_SetActive(void* self, unsigned char value) {
    ((CStarDust2*)self)->active = value;
}

// Initialize the bitmap pool and register the default shot-effect sprite resources.
ParticleBitmapRegistryState* ParticleBitmapRegistry_Ctor(ParticleBitmapRegistryState* self) {
    NamedParticlePool_Ctor(&self->namedPoolStorage, kPoolAllocationBatchSize, kInitialBitmapBucketCount);
    self->usedSlotCount = 0;
    self->persistentSlotCount = 0;
    self->lastSlotIndex = -1;
    ParticleBitmapRegistry_AddNamedBitmap(self, particleEffectStrings.powerShot, particleEffectStrings.shockwaveBitmap);
    ParticleBitmapRegistry_AddNamedBitmap(self, particleEffectStrings.holeCup, particleEffectStrings.holeCupBitmap);
    ParticleBitmapRegistry_AddNamedBitmap(self, particleEffectStrings.dust, particleEffectStrings.dustBitmap);
    ParticleBitmapRegistry_AddNamedBitmap(self, particleEffectStrings.grass, particleEffectStrings.grassBitmap);
    ParticleBitmapRegistry_AddNamedBitmap(self, particleEffectStrings.bunker, particleEffectStrings.bunkerBitmap);
    ParticleBitmapRegistry_AddNamedBitmap(self, particleEffectStrings.fragment, particleEffectStrings.fragmentBitmap);
    ParticleBitmapRegistry_AddNamedBitmap(self, particleEffectStrings.deferredEffect, particleEffectStrings.dustBitmap);
    ParticleBitmapRegistry_AddNamedBitmap(self, particleEffectStrings.ripple, particleEffectStrings.rippleBitmap);
    for (int i = 0; i < kEffectSlotCapacity; ++i) self->slots[i] = 0;
    return self;
}

#pragma optimization_level 4
#pragma use_lmw_stmw on
// Initialize a mutex-protected named pool and allocate its hash buckets.
NamedPoolState* NamedParticlePool_Ctor(NamedPoolState* self, int allocationBatchSize, int count) {
    volatile int* array;
    int n;
    int i;
    unsigned int offset;
    volatile NamedPoolState* vs;
    self->buckets = 0;
    InitRecursiveMutex((char*)self + FIELD_OFFSET(NamedPoolState, mutex));
    vs = self;
    InstanceLock_u801A97D0 lock((char*)vs + FIELD_OFFSET(NamedPoolState, mutex));
    vs->activeHead = 0;
    vs->freeHead = 0;
    vs->allocationHead = 0;
    vs->allocationBatchSize = allocationBatchSize;
    vs->activeCursor = 0;
    vs->bucketCount = count;
    if (count > 0) {
        n = vs->bucketCount;
        vs->buckets = (int*)appMallocUni__FUl((unsigned int)(count * 4));
        i = 0;
        offset = 0;
        vs->bucketMask = n - 1;
        while (i < vs->bucketCount) {
            array = vs->buckets;
            ++i;
            *(int*)((char*)array + offset) = 0;
            offset += 4;
        }
    } else {
        vs->buckets = 0;
    }
    return (NamedPoolState*)vs;
}
#pragma optimization_level reset
#pragma use_lmw_stmw reset

// Free every named-pool allocation block and its bucket array, then optionally delete the pool.
NamedPoolCleanupState* NamedPoolCleanupState_dtor(NamedPoolCleanupState* self, int doDelete) {
    if (self) {
        LockMutex((char*)self + FIELD_OFFSET(NamedPoolState, mutex));
        while (self->allocationHead) {
            LockMutex((char*)self + FIELD_OFFSET(NamedPoolState, mutex));
            PoolAllocationNode* node = self->allocationHead;
            node->prev->next = node->next;
            node->next->prev = node->prev;
            self->allocationHead = node->next == node ? 0 : node->next;
            UnlockMutex((char*)self + FIELD_OFFSET(NamedPoolState, mutex));
            appFreeUni__FPv(node);
        }
        if (self->buckets) {
            appFreeUni__FPv(self->buckets);
            self->buckets = 0;
        }
        UnlockMutex((char*)self + FIELD_OFFSET(NamedPoolState, mutex));
        if (doDelete > 0)
            __dl__FPv(self);
    }
    return self;
}

#pragma use_lmw_stmw on
#pragma opt_common_subs off
// arg is the retained lookup key; name is the bitmap resource path copied into
// its payload's 32-byte path field (plus the resource handle). The key bytes
// must outlive the node; the resource path must fit, including NUL.
// Allocation/resource failure is not guarded on every path in this original routine.
// Load a bitmap, insert its named payload into the hash table, and activate its pool node.
void ParticleBitmapRegistry_AddNamedBitmap(void* self_, const char* arg, const char* name) {
    BitmapRegistryPoolState* self = (BitmapRegistryPoolState*)self_;
    void* mutex;
    // Phase 1: allocate the bitmap payload; pool nodes below only refer to this object.
    BitmapPayload* bitmap = (BitmapPayload*)__nw__FUl(sizeof(BitmapPayload));
    BitmapPoolNode* first;
    BitmapPoolNode* block;
    BitmapPoolNode* cur;
    BitmapPoolNode* prev;
    unsigned int hash;
    if (bitmap) bitmap->resource = 0;
    bitmap->resource = (void*)ResourceManager_LoadBitmap(g_resourceManager, name, 0, 0, 0);
    strcpy(bitmap->path, name);
    {
        InstanceLock_u801A97D0 lockA((char*)self + FIELD_OFFSET(ParticleEffectInsertState, mutex));
        {
            InstanceLock_u801A97D0 lockB((char*)self + FIELD_OFFSET(ParticleEffectInsertState, mutex));
            /* The original (0x801AA730 +0x5C..+0x150) uses this+0x7C as the mutex on every path. */
            mutex = (char*)self + FIELD_OFFSET(ParticleEffectInsertState, mutex);
            if (!self->freeHead) {
                /* New block of count 0x18-byte entries; nothing is stored at +0x60 (+0x8C..+0x94). */
                block = (BitmapPoolNode*)appMallocUni__FUl(self->allocationBatchSize * 0x18);
                first = self->allocationHead;
                LockMutex(mutex);
                if (!first) {
                    block->prev = block;
                    block->next = block;
                    UnlockMutex(mutex);
                    first = block;
                } else {
                    /* insert before head: new->next = head; new->prev = head->prev; head->prev = new;
                       new->prev->next = new (+0xC0..+0xD0) */
                    block->next = first;
                    block->prev = first->prev;
                    first->prev = block;
                    block->prev->next = block;
                    UnlockMutex(mutex);
                }
                self->allocationHead = first;
                cur = (BitmapPoolNode*)((NamedPoolNode*)block + 1);
                for (int i = 1; i < self->allocationBatchSize; ++i) {
                    first = self->freeHead;
                    LockMutex(mutex);
                    if (!first) {
                        cur->prev = cur;
                        cur->next = cur;
                        UnlockMutex(mutex);
                        first = cur;
                    } else {
                        cur->next = first;
                        cur->prev = first->prev;
                        first->prev = cur;
                        cur->prev->next = cur;
                        UnlockMutex(mutex);
                    }
                    self->freeHead = first;
                    cur = (BitmapPoolNode*)((NamedPoolNode*)cur + 1);
                }
            }
            /* pop the free-list head; the new head is head->next (+0x188..+0x1A4) */
            first = self->freeHead;
            LockMutex(mutex);
            first->next->prev = first->prev;
            first->prev->next = first->next;
            cur = first->next;
            if (cur == first) cur = 0;
            UnlockMutex(mutex);
            self->freeHead = cur;
        }
        // Phase 2: bind payload/key, hash the key, then publish the active-ring node.
        first->data = bitmap;
        first->flag = 0;
        first->nameAddress = (int)arg;
        if (arg) {
            LockMutex(mutex);
            int len = strlen(arg);
            int idx = len ? len - 1 : 0;
            hash = (((unsigned char)arg[0] & kNameHashFirstByteMask) | ((unsigned char)arg[idx] & kNameHashLastByteMask) | (len << kNameHashLengthShift)) & self->bucketMask;
            UnlockMutex(mutex);
            LockMutex(mutex);
            first->hashNextAddress = self->buckets[hash];
            self->buckets[hash] = (unsigned int)first;
            UnlockMutex(mutex);
        }
        /* link the popped entry (not a block) into the active list at +0x5C (+0x254..+0x2A0) */
        prev = self->activeHead;
        LockMutex(mutex);
        if (!prev) {
            first->prev = first;
            first->next = first;
            UnlockMutex(mutex);
            prev = first;
        } else {
            first->next = prev;
            first->prev = prev->prev;
            prev->prev = first;
            first->prev->next = first;
            UnlockMutex(mutex);
        }
        self->activeHead = prev;
    }
}
#pragma opt_common_subs reset
#pragma use_lmw_stmw reset

#pragma use_lmw_stmw on
// Delete effect slots, release named bitmaps, and free registry nodes and buckets.
ParticleEffectDestroyState* ParticleEffectPool_Destroy(ParticleEffectDestroyState* self, int doDelete) {
    if (self) {
        // Phase 1: destroy effects before releasing the bitmaps their sprites may reference.
        void** cur = (void**)self;
        int i;
        for (i = 0; i < kEffectSlotCapacity; ++i) {
            void* slot = *cur;
            if (slot) ((void (*)(void*, int))(*(void***)slot)[kEffectVSlotDelete])(slot, 1);
            *cur = 0;
            ++cur;
        }
        {
            // Phase 2: release each active bitmap resource ID and delete its separate payload.
            void* lockA = (char*)self + FIELD_OFFSET(ParticleEffectInsertState, mutex);
            LockMutex(lockA);
            void* lockB = (char*)self + FIELD_OFFSET(ParticleEffectInsertState, mutex);
            self->activeCursor = self->activeHead;
            LockMutex(lockB);
            void* node;
            if (self->activeCursor) {
                void* nx = ((NamedPoolNode*)self->activeCursor)->next;
                node = *(void**)self->activeCursor;
                self->activeCursor = nx == self->activeHead ? 0 : nx;
                UnlockMutex(lockB);
            } else {
                UnlockMutex(lockB);
                node = 0;
            }
            UnlockMutex(lockA);
            if (node) {
                void* mutex = (char*)self + FIELD_OFFSET(ParticleEffectInsertState, mutex);
                do {
                    if (((BitmapPayload*)node)->resource) {
                        ResourceManager_ReleaseId(g_resourceManager, ((BitmapPayload*)node)->resource);
                        ((BitmapPayload*)node)->resource = 0;
                    }
                    __dl__FPv(node);
                    LockMutex(mutex);
                    void* next;
                    if (self->activeCursor) {
                        void* nx = ((NamedPoolNode*)self->activeCursor)->next;
                        next = *(void**)self->activeCursor;
                        self->activeCursor = nx == self->activeHead ? 0 : nx;
                        UnlockMutex(mutex);
                    } else {
                        UnlockMutex(mutex);
                        next = 0;
                    }
                    node = next;
                } while (node);
            }
        }
        // Phase 3: free node allocation blocks and bucket storage after payload cleanup.
        if (&self->activeHead) {
            void* lockC = (char*)self + FIELD_OFFSET(ParticleEffectInsertState, mutex);
            LockMutex(lockC);
            if (self->allocationHead) {
                void* mutex2 = (char*)self + FIELD_OFFSET(ParticleEffectInsertState, mutex);
                do {
                    LockMutex(mutex2);
                    void* nd = self->allocationHead;
                    void* nx = ((Link*)nd)->next;
                    ((Link*)nd)->prev->next = (Link*)nx;
                    ((Link*)nx)->prev = ((Link*)nd)->prev;
                    if (nx == nd) nx = 0;
                    UnlockMutex(mutex2);
                    self->allocationHead = nx;
                    appFreeUni__FPv(nd);
                } while (self->allocationHead);
            }
            if (self->buckets) {
                appFreeUni__FPv(self->buckets);
                self->buckets = 0;
            }
            UnlockMutex(lockC);
        }
        if (doDelete > 0) __dl__FPv(self);
    }
    return self;
}
#pragma use_lmw_stmw reset

// Update each effect and delete inactive transient slots, keeping the persistent prefix.
void ParticleEffectPool_UpdateAndReap(ParticleEffectPoolState* self, float delta) {
    // Slot range is inclusive of lastSlotIndex; holes remain available for reuse.
    int zero = 0;
    int i = 0;
    ParticleEffectItem** cur = (ParticleEffectItem**)self->slots;
    while (i <= self->lastSlotIndex) {
        if (*cur) {
            (*cur)->update(delta);
            if (i >= self->persistentSlotCount && !(*cur)->active) {
                --self->usedSlotCount;
                if (*cur)
                    (*cur)->destroy(1);
                *cur = (ParticleEffectItem*)zero;
                if (i == self->lastSlotIndex) {
                    while (self->lastSlotIndex > 0 && !self->slots[self->lastSlotIndex])
                        --self->lastSlotIndex;
                }
            }
        }
        ++cur;
        ++i;
    }
}

// Takes ownership of effect even when full (destroys it on rejection). effect
// must be nonnull on the success path; name must resolve if supplied. Persistent
// entries must be inserted before transient entries: cleanup protects an index
// prefix, not per-slot flags. Bitmap lookup returns a borrowed registry payload.
// Insert an effect into the first free slot, bind its named bitmap, and start it.
void ParticleEffectPool_AddAndStart(ParticleEffectInsertState* self, ParticleEffectItem* effect, const char* name, int persistent) {
    int index;
    int i;
    void* mutex;
    if (self->usedSlotCount == kEffectSlotCapacity) {
        if (effect)
            effect->destroy(1);
        return;
    }
    {
        ParticleEffectItem** cur = (ParticleEffectItem**)self;
        index = 0;
        for (;;) {
            if (*cur == 0) break;
            ++cur;
            ++index;
        }
    }
    i = index;
    if (i < self->lastSlotIndex) i = self->lastSlotIndex;
    self->lastSlotIndex = i;
    ++self->usedSlotCount;
    if (persistent)
        ++self->persistentSlotCount;
    self->slots[index] = effect;
    if (name) {
        void* found = FindManagerItem(self, name);
        int val = ((BitmapLookupNode*)found)->value;
        ((EffectSpriteBinding*)self->slots[index])->bitmapId = val;
    }
    self->slots[index]->begin();
}

// Returns self even if opening the definition fails; this is not a success flag.
// Definition order/field widths are a file format. Positive emissionInterval,
// nonzero capacity/lifetime and a loaded sprite with frames are assumed later.
// Read an emitter definition, allocate its particles and sprite, and initialize emission state.
void* ParticleEmitter_LoadDefinition(void* self, const char* name) {
    char* p = (char*)self;
    ((Object801AAED8*)p)->sprite = 0;
    Vec3_Zero(p + FIELD_OFFSET(Emitter, bounds.minimum));
    Vec3_Zero(p + FIELD_OFFSET(Emitter, bounds.maximum));
    void* resource = Resource_OpenResolvedPath(g_resourceManager, name, -1, 0, 0, 0);
    ResGuard guard(&resource);
    if (!resource) return p;
    // Phase 1: consume the fixed binary field sequence; do not reorder by struct layout.
    ReadEmitterField((Resource*)resource, p + FIELD_OFFSET(Emitter, initialOpacity), sizeof(((Emitter*)0)->initialOpacity));
    ReadEmitterField((Resource*)resource, p + FIELD_OFFSET(Emitter, finalOpacity), sizeof(((Emitter*)0)->finalOpacity));
    ReadEmitterField((Resource*)resource, p + FIELD_OFFSET(Emitter, initialSize), sizeof(((Emitter*)0)->initialSize));
    ReadEmitterField((Resource*)resource, p + FIELD_OFFSET(Emitter, finalSize), sizeof(((Emitter*)0)->finalSize));
    ReadEmitterField((Resource*)resource, p + FIELD_OFFSET(Emitter, minimumVerticalSpeed), sizeof(((Emitter*)0)->minimumVerticalSpeed));
    ReadEmitterField((Resource*)resource, p + FIELD_OFFSET(Emitter, maximumVerticalSpeed), sizeof(((Emitter*)0)->maximumVerticalSpeed));
    ReadEmitterField((Resource*)resource, p + FIELD_OFFSET(Emitter, minimumAngularSpeed), sizeof(((Emitter*)0)->minimumAngularSpeed));
    ReadEmitterField((Resource*)resource, p + FIELD_OFFSET(Emitter, maximumAngularSpeed), sizeof(((Emitter*)0)->maximumAngularSpeed));
    ReadEmitterField((Resource*)resource, p + FIELD_OFFSET(Emitter, minimumLifetime), sizeof(((Emitter*)0)->minimumLifetime));
    ReadEmitterField((Resource*)resource, p + FIELD_OFFSET(Emitter, maximumLifetime), sizeof(((Emitter*)0)->maximumLifetime));
    ReadEmitterField((Resource*)resource, p + FIELD_OFFSET(Emitter, gravityScale), sizeof(((Emitter*)0)->gravityScale));
    ReadEmitterField((Resource*)resource, p + FIELD_OFFSET(Emitter, emissionInterval), sizeof(((Emitter*)0)->emissionInterval));
    ReadEmitterField((Resource*)resource, p + FIELD_OFFSET(Emitter, spriteFrameInterval), sizeof(((Emitter*)0)->spriteFrameInterval));
    ReadEmitterField((Resource*)resource, p + FIELD_OFFSET(Emitter, emissionTimeRemaining), sizeof(((Emitter*)0)->emissionTimeRemaining));
    ReadEmitterField((Resource*)resource, p + FIELD_OFFSET(Emitter, bounceElasticity), sizeof(((Emitter*)0)->bounceElasticity));
    ReadEmitterField((Resource*)resource, p + FIELD_OFFSET(Emitter, acceleration), sizeof(((Emitter*)0)->acceleration));
    ReadEmitterField((Resource*)resource, p + FIELD_OFFSET(Emitter, definitionFlags), sizeof(((Emitter*)0)->definitionFlags));
    ReadEmitterField((Resource*)resource, p + FIELD_OFFSET(Emitter, particlesPerEmission), sizeof(((Emitter*)0)->particlesPerEmission));
    ReadEmitterField((Resource*)resource, p + FIELD_OFFSET(Emitter, emissionShape), sizeof(((Emitter*)0)->emissionShape));
    ReadEmitterField((Resource*)resource, p + FIELD_OFFSET(Emitter, minimumHorizontalSpeed), sizeof(((Emitter*)0)->minimumHorizontalSpeed));
    ReadEmitterField((Resource*)resource, p + FIELD_OFFSET(Emitter, maximumHorizontalSpeed), sizeof(((Emitter*)0)->maximumHorizontalSpeed));
    ReadEmitterField((Resource*)resource, p + FIELD_OFFSET(Emitter, bounds), sizeof(((Emitter*)0)->bounds));
    ReadEmitterField((Resource*)resource, p + FIELD_OFFSET(Emitter, emissionRadius), sizeof(((Emitter*)0)->emissionRadius));
    ReadEmitterField((Resource*)resource, p + FIELD_OFFSET(Emitter, spriteSize), sizeof(((Emitter*)0)->spriteSize));
    ReadEmitterField((Resource*)resource, p + FIELD_OFFSET(Emitter, initialSpriteFrame), sizeof(((Emitter*)0)->initialSpriteFrame));
    ReadEmitterField((Resource*)resource, p + FIELD_OFFSET(Emitter, randomizeSpriteFrame), sizeof(((Emitter*)0)->randomizeSpriteFrame));
    ReadEmitterField((Resource*)resource, p + FIELD_OFFSET(Emitter, spriteName), sizeof(((Emitter*)0)->spriteName));

    // Phase 2: release the definition; derive ring capacity from lifetime/interval.
    if (resource) { Resource_Release(resource); resource = 0; }
    float lifetimeIntervals = (1.0f / ((Object801AAED8*)p)->emissionInterval) * ((Object801AAED8*)p)->maxParticleLifetime;
    ((Object801AAED8*)p)->particleCapacity = ((Object801AAED8*)p)->particlesPerEmission * (int)lifetimeIntervals;
    Elem_u801A97D0* arr = new Elem_u801A97D0[((Object801AAED8*)p)->particleCapacity];
    ((Object801AAED8*)p)->particleStorage=arr;
    for(int i=0;i<((Object801AAED8*)p)->particleCapacity;i++) *((unsigned char*)((Object801AAED8*)p)->particleStorage+i*0x3c+0x1c)=0;
    // Phase 3: create the sprite and publish initial emission, surface and position state.
    Matrix_SetIdentityAffine(p + FIELD_OFFSET(Emitter, transform));
    Vec3_Zero(p + FIELD_OFFSET(Emitter, rotation));
    void* obj=W3dAniSpr_Create(g_resourceManager,((Object801AAED8*)p)->spriteSize,((Object801AAED8*)p)->spriteSize,p + FIELD_OFFSET(Emitter, spriteName),0);
    ((Object801AAED8*)p)->sprite=obj;
    ((Object801AAED8*)p)->emissionTimer=0;
    ((Object801AAED8*)p)->nextParticleIndex=0; ((Object801AAED8*)p)->activeParticleCount=0;
    ((Object801AAED8*)p)->active=1; ((Object801AAED8*)p)->drawingEnabled=1;
    ((Object801AAED8*)p)->surfaceIndex=0xffff; ((Object801AAED8*)p)->surfaceOpacityMultiplier=1.0f;
    if (!g_particleCourseEffects) { void* g=g_taskManager ? Object_GetChild(g_taskManager) : 0; g_particleCourseEffects=g ? ((CourseEffectOwner*)g)->courseEffects : 0; }
    ((Object801AAED8*)p)->positionLink=p + FIELD_OFFSET(Emitter, position);
    return p;
}

// Default particle-array element constructor; initialization occurs when a particle is emitted.
void EmitterParticle_DefaultCtor() {}

// Advance live particles and emit elapsed batches, including updates for missed intervals.
void ParticleEmitter_UpdateAndEmit(void* self, float dt) {
    char* p = (char*)self;
    if (!((Object801AAED8*)p)->active) return;
    float cameraDistance = (((CameraParticleView*)s_cameraTransform)->position - *(const WVector*)(p + FIELD_OFFSET(Emitter, position))).Magnitude();
    float ratio = CapRatioAtOne(cameraDistance / kParticleEmissionDistanceScale);
    int offset;
    int index;
    float emissionInterval = ((Object801AAED8*)p)->emissionInterval;
    index = 0;
    offset = 0;
    // Phase 1: advance all live particles before accounting for newly elapsed emissions.
    while (index < ((Object801AAED8*)p)->particleCapacity) {
        if (*(unsigned char*)((*(char**)&((Object801AAED8*)p)->particleStorage) + offset + FIELD_OFFSET(Particle, active)))
            Particle_UpdateLifetimeSizeOpacityAndWind(self, index, dt);
        offset += kParticleRecordBytes;
        index++;
    }
    // Phase 2: consume complete emission intervals and age missed batches to this update.
    ((Object801AAED8*)p)->emissionTimer = ((Object801AAED8*)p)->emissionTimer + dt;
    if (((Object801AAED8*)p)->emissionTimeRemaining > kFloat_0_805DF3CC) ((Object801AAED8*)p)->emissionTimeRemaining -= dt;
    if (((Object801AAED8*)p)->emissionTimer > emissionInterval) {
        if (((Object801AAED8*)p)->emissionTimeRemaining >= kFloat_0_805DF3CC) {
            int elapsedBatches = dt > emissionInterval ? WholePeriods(((Object801AAED8*)p)->emissionTimer, emissionInterval) : 1;
            ((Object801AAED8*)p)->emissionTimer -= emissionInterval * elapsedBatches;
            while (0 < elapsedBatches) {
                int emittedInBatch = 0;
                while (emittedInBatch < ((Object801AAED8*)p)->particlesPerEmission) {
                    ParticleEmitter_InitializeNextParticle((Emitter*)self);
                    if (dt > emissionInterval) {

                        int emittedIndex = ((Object801AAED8*)p)->nextParticleIndex;
                        Particle_UpdateLifetimeSizeOpacityAndWind(self, emittedIndex, emissionInterval * (float)elapsedBatches + ((Object801AAED8*)p)->emissionTimer);
                    }
                    ++emittedInBatch;
                    int currentRingIndex = ((Object801AAED8*)p)->nextParticleIndex;

                    int ringCapacity = ((Object801AAED8*)p)->particleCapacity;
                    int next = currentRingIndex + 1; int wrapCount = next / ringCapacity;
                    ((Object801AAED8*)p)->nextParticleIndex = next - wrapCount * ringCapacity;
                    ++((Object801AAED8*)p)->activeParticleCount;
                }
                elapsedBatches -= 1;
            }
        }
    }
    // Phase 3: cache the current surface multiplier independently of per-particle opacity.
    int surfaceOpacityByte = CourseSurfaceProperties_GetParticleOpacity(((CourseEffectState*)g_particleCourseEffects)->surfaceProperties, ((Object801AAED8*)p)->surfaceIndex);
    ((Object801AAED8*)p)->surfaceOpacityMultiplier = (float)surfaceOpacityByte / kParticleOpacityByteScale;
}

// Initialize the next particle with randomized life, spin, sprite frame, position, and velocity.
void ParticleEmitter_InitializeNextParticle(Emitter* p) {
    float t4[3];
    p->particles[p->nextParticleIndex].active = 1;
    p->particles[p->nextParticleIndex].opacity = p->initialOpacity;
    p->particles[p->nextParticleIndex].size = p->initialSize;
    p->particles[p->nextParticleIndex].initialLifetime = (p->maximumLifetime - p->minimumLifetime) * rndf() + p->minimumLifetime;
    p->particles[p->nextParticleIndex].lifetimeRemaining = p->particles[p->nextParticleIndex].initialLifetime;
    p->particles[p->nextParticleIndex].frameTimeRemaining = p->spriteFrameInterval;
    unsigned int sel;
    if (p->randomizeSpriteFrame) { int n = p->sprite->frameCount; sel = rand() % n; }
    else sel = p->initialSpriteFrame;
    p->particles[p->nextParticleIndex].spriteFrame = sel;

    p->particles[p->nextParticleIndex].angularVelocity = ((rand() % 2) ? kFloat_1_particles : kFloat_NegOne_ParticleEmitter) * RandomRange(p->minimumAngularSpeed, p->maximumAngularSpeed);
    p->particles[p->nextParticleIndex].rotation = kFloat_0_805DF3CC;
    __as__7WVectorFRC7WVector(p->particles[p->nextParticleIndex].position, p->position);
    // Select world-position offsets and initial velocity using the configured shape.
    switch (p->emissionShape) {
    case kBoxEmissionShape:
        p->particles[p->nextParticleIndex].position[0] += (p->bounds.maximum[0] - p->bounds.minimum[0]) * rndf() + p->bounds.minimum[0];
        p->particles[p->nextParticleIndex].position[1] += (p->bounds.maximum[1] - p->bounds.minimum[1]) * rndf() + p->bounds.minimum[1];
        p->particles[p->nextParticleIndex].position[2] += (p->bounds.maximum[2] - p->bounds.minimum[2]) * rndf() + p->bounds.minimum[2];
        {
            float rot[12], res[3], vec[3], axis[3], res2[3];
            axis[0] = kFloat_0_805DF3CC; axis[1] = kFloat_0_805DF3CC; axis[2] = kFloat_1_particles;
            Matrix_SetAxisRotation(rot, kFloat_Pi_ParticleEmitter * (kFloat_Two_ParticleEmitter * rndf()), 1);
            __ml__FfRC7WVector(vec, axis, RandomRange(p->minimumHorizontalSpeed, p->maximumHorizontalSpeed));
            Matrix_TransformPoint(res, vec, rot);
            __as__7WVectorFRC7WVector(p->particles[p->nextParticleIndex].velocity, res);
            p->particles[p->nextParticleIndex].velocity[1] = RandomRange(p->minimumVerticalSpeed, p->maximumVerticalSpeed);
            Matrix_TransformPoint(res2, p->particles[p->nextParticleIndex].velocity, p->transform);
            __as__7WVectorFRC7WVector(p->particles[p->nextParticleIndex].velocity, res2);
        }
        break;
    case kSphereEmissionShape: {
        float v = (*(unsigned int*)&p->emissionRadius & 0x80000000) ? -p->emissionRadius : p->emissionRadius;
        bool radiusIsNearlyZero = v < kParticleEmissionRadiusEpsilon;
        if (radiusIsNearlyZero) {
            float r2[12]; float r1[12]; float t2[3]; float t1[3]; float axis[3];
            axis[0] = kFloat_0_805DF3CC; axis[1] = kFloat_0_805DF3CC; axis[2] = kFloat_1_particles;
            Matrix_SetAxisRotation(r1, kFloat_Pi_ParticleEmitter * (kFloat_Two_ParticleEmitter * rndf()), 1);
            Matrix_SetAxisRotation(r2, kFloat_Pi_ParticleEmitter * (kFloat_Two_ParticleEmitter * rndf()), 0);
            Matrix_TransformPoint(t1, axis, r2);
            Matrix_TransformPoint(t2, t1, r1);
            __as__7WVectorFRC7WVector(p->particles[p->nextParticleIndex].velocity, t2);
        } else {
            float r2[12], r1[12], t3[3], t2[3], t1[3], axis[3], t5[3];
            axis[0] = kFloat_0_805DF3CC; axis[1] = kFloat_0_805DF3CC; axis[2] = kFloat_1_particles;
            Matrix_SetAxisRotation(r1, kFloat_Pi_ParticleEmitter * (kFloat_Two_ParticleEmitter * rndf()), 1);
            Matrix_SetAxisRotation(r2, kFloat_Pi_ParticleEmitter * (kFloat_Two_ParticleEmitter * rndf()), 0);
            __ml__FfRC7WVector(t1, axis, p->emissionRadius);
            Matrix_TransformPoint(t2, t1, r2);
            Matrix_TransformPoint(t3, t2, r1);
            __apl__7WVectorFRC7WVector(p->particles[p->nextParticleIndex].position, t3);
            __mi__FRC7WVectorRC7WVector(t4, (const char*)p->particles[p->nextParticleIndex].position, p->position);
            float magnitude = Magnitude__7WVectorCFv(t4);
            Vec3_DivideScalar(t5, magnitude, t4);
            __as__7WVectorFRC7WVector(p->particles[p->nextParticleIndex].velocity, t5);
        }
        __amu__7WVectorFf(p->particles[p->nextParticleIndex].velocity, RandomRange(p->minimumVerticalSpeed, p->maximumVerticalSpeed));
        break;
    }
    }
}

// index addresses the emitter-owned fixed particle ring; dt uses definition time units.
// position is world XYZ; velocity/acceleration are world units per time/time squared.
// Expiration is tested before visual interpolation; a newly negative size/opacity
// therefore expires on a later update. Spin retains the definition's angular units.
// Age a particle, interpolate size/opacity, apply wind/gravity/bounce, and step its sprite frame.
void Particle_UpdateLifetimeSizeOpacityAndWind(void* self, int index, float dt) {
    char* p = (char*)self;
    unsigned int particleByteOffset = index * kParticleRecordBytes;
    void* base;
    Particle* particle = (Particle*)((char*)((Object801AAED8*)p)->particleStorage + particleByteOffset);
    float lifetimeBeforeUpdate = particle->lifetimeRemaining;
    particle->lifetimeRemaining = lifetimeBeforeUpdate - dt;
    if (((Object801AAED8*)p)->spriteFrameInterval > kFloat_0_805DF3CC) {
        particle = (Particle*)((char*)((Object801AAED8*)p)->particleStorage + particleByteOffset);
        particle->frameTimeRemaining = particle->frameTimeRemaining - dt;
    }
    particle = (Particle*)((char*)((Object801AAED8*)p)->particleStorage + particleByteOffset);
    float remainingLifetime = particle->lifetimeRemaining;
    if (remainingLifetime < kFloat_0_805DF3CC || particle->opacity < kFloat_0_805DF3CC || particle->size < kFloat_0_805DF3CC) {
        particle->active = 0;
        int activeCount = ((Object801AAED8*)p)->activeParticleCount;
        ((Object801AAED8*)p)->activeParticleCount = activeCount - 1;
        return;
    }

    // Surviving particle: interpolate visual properties, then integrate forces.
    float oldSize = particle->size;
    float sizeDelta = (((Object801AAED8*)p)->finalParticleSize) - ((Object801AAED8*)p)->initialParticleSize;
    float sizeStep = particle->initialLifetime; sizeStep = sizeDelta / sizeStep;
    float previousSize = oldSize;
    sizeStep = dt * sizeStep + previousSize;
    particle->size = sizeStep;

    particle = (Particle*)((char*)((Object801AAED8*)p)->particleStorage + particleByteOffset);

    float rotation = particle->rotation;
    rotation = dt * (particle->angularVelocity) + rotation;
    particle->rotation = rotation;

    particle = (Particle*)((char*)((Object801AAED8*)p)->particleStorage + particleByteOffset);
    float opacityDelta = ((Object801AAED8*)p)->finalOpacity - ((Object801AAED8*)p)->initialOpacity;
    float opacityStep = particle->initialLifetime;
    opacityDelta = opacityDelta / opacityStep;
    opacityStep = dt * opacityDelta + particle->opacity;
    particle->opacity = opacityStep;

    // Wind and configured acceleration add dt-scaled velocity; gravity subtracts on Y.
    float windVelocity[3];
    void* wind = CWind_GetInstance();
    CWind_GetWindVectorAtPoint(windVelocity, wind, (char*)((Object801AAED8*)p)->particleStorage + particleByteOffset, 99999.0f);

    void* courseEffects = g_particleCourseEffects;
    void* surfaceProperties = ((CourseEffectState*)courseEffects)->surfaceProperties;

    float scaledWind[3];
    __ml__FfRC7WVector(scaledWind, windVelocity, (CourseSurfaceProperties_GetParticleWindScale(surfaceProperties, (((Object801AAED8*)p)->surfaceIndex))));

    float windStep[3];
    __ml__FRC7WVectorf(windStep, scaledWind, dt);

    base = ((Object801AAED8*)p)->particleStorage;
    __apl__7WVectorFRC7WVector((float*)((char*)base + particleByteOffset + FIELD_OFFSET(Particle, velocity)), windStep);

    float accelerationStep[3];
    __ml__FRC7WVectorf(accelerationStep, ((Emitter*)p)->acceleration, dt);

    base = ((Object801AAED8*)p)->particleStorage;
    __apl__7WVectorFRC7WVector((float*)((char*)base + particleByteOffset + FIELD_OFFSET(Particle, velocity)), accelerationStep);

    base = ((Object801AAED8*)p)->particleStorage;
    float gravityConstant = kParticleGravityAcceleration;
    particle = (Particle*)((char*)base + particleByteOffset);

    float verticalSpeed = particle->velocity[1];
    float gravityScale = ((Object801AAED8*)p)->gravityScale;
    float gravity = gravityScale * gravityConstant;
    gravityConstant = gravity;
    verticalSpeed = verticalSpeed - gravityConstant * dt;
    particle->velocity[1] = verticalSpeed;

    // Move with the updated velocity, resolving the original one-crossing bounce rule.
    AdvanceParticle(p, particleByteOffset, dt);
    base = ((Object801AAED8*)p)->particleStorage;
    particle = (Particle*)((char*)base + particleByteOffset);

    // Advance at most one sprite frame per call; excess frame time is discarded.
    if ((particle->frameTimeRemaining) < kFloat_0_805DF3CC) {
        particle->frameTimeRemaining = ((Object801AAED8*)p)->spriteFrameInterval;
        base = ((Object801AAED8*)p)->particleStorage;
        particle = (Particle*)((char*)base + particleByteOffset);
        int nextFrame = particle->spriteFrame + 1;

        particle->spriteFrame = nextFrame % ((W3dAniSpr*)((Object801AAED8*)p)->sprite)->frameCount;
    }
}

// Return the registered particle-effect type record.
void* CParticleEffect_GetRTTI() { return &s_particleEffectTypeInfo; }

// Free both named-pool allocation lists and buckets before destroying the actor base.
void* CParticleEffect_dtor(void* self, int deleting) {
    if (self) {
        char* p = (char*)self;
        if (&((CParticleEffect*)p)->secondListHead) {
            InstanceLock_u801A97D0 g(p + FIELD_OFFSET(CParticleEffect, pad_08C));
            while (*(void**)(p + FIELD_OFFSET(CParticleEffect, secondAllocationHead))) {
                void* list = p + FIELD_OFFSET(CParticleEffect, pad_08C);
                Link* item = *(Link**)(p + FIELD_OFFSET(CParticleEffect, secondAllocationHead));
                LockMutex(list);
                item->next->prev = item->prev;
                item->prev->next = item->next;
                Link* successor = item->next;
                bool last = item == successor;
                Link* next = last ? 0 : successor;
                UnlockMutex(list);
                (*(Link**)&((CParticleEffect*)p)->secondAllocationHead) = next;
                appFreeUni__FPv(item);
            }
            if (((CParticleEffect*)p)->secondSlotBuckets) { appFreeUni__FPv(((CParticleEffect*)p)->secondSlotBuckets); ((CParticleEffect*)p)->secondSlotBuckets = 0; }
        }
        if (&((CParticleEffect*)p)->activeHead) {
            InstanceLock_u801A97D0 g(p + FIELD_OFFSET(CParticleEffect, pad_03C));
            while (*(void**)(p + FIELD_OFFSET(CParticleEffect, allocationHead))) {
                Link* item = *(Link**)(p + FIELD_OFFSET(CParticleEffect, allocationHead));
                void* list = p + FIELD_OFFSET(CParticleEffect, pad_03C);
                LockMutex(list);
                item->next->prev = item->prev;
                item->prev->next = item->next;
                Link* next = item->next == item ? 0 : item->next;
                UnlockMutex(list);
                (*(Link**)&((CParticleEffect*)p)->allocationHead) = next;
                appFreeUni__FPv(item);
            }
            if (((CParticleEffect*)p)->slotBuckets) { appFreeUni__FPv(((CParticleEffect*)p)->slotBuckets); ((CParticleEffect*)p)->slotBuckets = 0; }
        }
        __dt__18NativeBase80201D18Fv(self, 0);
        if (0 < deleting) __dl__FPv(self);
    }
    return self;
}

// Register the particle-effect actor factory and task type during static initialization.
void CParticleEffect_RegisterType() {
    g_particleEffectStaticTwoPi = kFloat_6_2831855;
    IntPair_Set(&s_particleEffectTypeInfo, particleEffectStrings.actorTypeName, &s_actorTypeInfo);
    void* result = ActorFactoryRegistry_GetInstance();
    StringMap_SetByName(result, CParticleEffect_Create, s_particleEffectActorFactoryName);
    result = TaskRegistry_GetInstance();
    TaskRegistry_AppendFactory(result, s_particleEffectTaskFactoryName);
}

// Adjust the secondary task-interface pointer to the actor before drawing.
void CParticleEffect_DrawThunk18(void* self) {
    CParticleEffect_Draw((char*)self - 0x18);
}

// Delete both owned particle resources, then optionally free the particle object.
void* CParticleBase_dtor(void* self, int deleting) {
    if (self) {
        *(void**)self = (void*)s_particleVtable;
        if (((CParticle*)self)->unk_020) {
            if (((CParticle*)self)->unk_020) {
                void* obj = ((CParticle*)self)->unk_020;
                void** vtable = *(void***)obj;
                void (*fn)(void*, int) = (void (*)(void*, int))vtable[2];
                fn(obj, 1);
            }
            ((CParticle*)self)->unk_020 = 0;
        }
        if (((CParticle*)self)->unk_01c) {
            if (((CParticle*)self)->unk_01c) {
                void* obj = ((CParticle*)self)->unk_01c;
                void** vtable = *(void***)obj;
                void (*fn)(void*, int) = (void (*)(void*, int))vtable[2];
                fn(obj, 1);
            }
            ((CParticle*)self)->unk_01c = 0;
        }
        if (deleting > 0) __dl__FPv(self);
    }
    return self;
}

// Allocate shot-trail particles and initialize the paired-vertex and sequential-index buffers.
void* CShotParticle_Ctor(void* self) {
    int zero = 0;
    char* p = (char*)self;
    ((CShotParticle*)p)->bitmapId = zero;
    ((CShotParticle*)p)->active = zero;
    (*(int*)&((CParticle*)p)->unk_01c) = zero;
    (*(int*)&((CParticle*)p)->unk_020) = zero;
    ((CShotParticle*)p)->vertex0 = &((CShotParticle*)p)->pad_044[0];
    ((CShotParticle*)p)->vertex1 = &((CShotParticle*)p)->pad_044[0x38];
    ((CShotParticle*)p)->vertex2 = &((CShotParticle*)p)->pad_044[0xA8];
    ((CShotParticle*)p)->vertex3 = &((CShotParticle*)p)->pad_044[0x70];
    ((CShotParticle*)p)->unk_40 = zero;
    *(const char**)p = __vt__13CShotParticle;
    ((CShotParticle*)p)->particleCapacity = kShotTrailCapacity;
    ((CShotParticle*)p)->particles = __nwa__FUl(((CShotParticle*)p)->particleCapacity * kShotTrailParticleBytes);
    VertexBuffer_Create(((CShotParticle*)p)->particleCapacity << 5, 0, kShotTrailVertexFormat, 0, (void**)(p + FIELD_OFFSET(CShotParticle, vertexBuffer)), 0);
    void* vertices;
    Resource_GetOffsetPtr(((CShotParticle*)p)->vertexBuffer, 0, 0, &vertices, 0);
    int vertexByteOffset = zero;
    int i = zero;
    int fullColorChannel = 0xFF;
    while (i < ((CShotParticle*)p)->particleCapacity) {
        ((unsigned char*)vertices)[vertexByteOffset + 0x0F] = zero;
        ((unsigned char*)vertices)[vertexByteOffset + 0x0E] = fullColorChannel;
        ((unsigned char*)vertices)[vertexByteOffset + 0x0D] = fullColorChannel;
        ((unsigned char*)vertices)[vertexByteOffset + 0x0C] = fullColorChannel;
        ((unsigned char*)vertices)[vertexByteOffset + 0x1F] = fullColorChannel;
        ((unsigned char*)vertices)[vertexByteOffset + 0x1E] = fullColorChannel;
        ((unsigned char*)vertices)[vertexByteOffset + 0x1D] = fullColorChannel;
        vertexByteOffset += kShotTrailVertexPairBytes;
        ((unsigned char*)vertices)[vertexByteOffset - 4] = fullColorChannel;
        ++i;
    }
    GraphicsBuffer_Flush(((CShotParticle*)p)->vertexBuffer);
    IndexBuffer_Create(((CShotParticle*)p)->particleCapacity << 2, 0, kShotTrailIndexFormat, 0, (void**)(p + FIELD_OFFSET(CShotParticle, indexBuffer)), 0, 0);
    void* indices;
    IndexBuffer_GetOffsetPtr(((CShotParticle*)p)->indexBuffer, 0, 0, &indices, 0);
    for (int i = 0; i < ((CShotParticle*)p)->particleCapacity * 2; ++i) ((unsigned short*)indices)[i] = i;
    IndexBuffer_Flush(((CShotParticle*)p)->indexBuffer);
    return self;
}

} // extern "C"



/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU80462174", _ctors_u801A97D0_0, CParticleEffect_RegisterType);
#pragma pop

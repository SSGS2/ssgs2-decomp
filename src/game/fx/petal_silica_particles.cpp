// Petal and silica particle effects: the drifting petals and silica dust, and the pet body
// objects that each emitter owns.
// Particles move with wind and gravity, wrap around the camera bounds, and are drawn with an
// optional full-screen dim overlay.

#include <decomp/static_init.h>
#include <decomp/local_class.h>
#include <decomp/offset_of.h>
#include "MSL/vector.h"
// Containers whose deleting destructors this unit keeps out of line under plain names. The element
// types are placeholders: the destructor depends only on whether elements are scalars or records.
struct MSLRecord {
    unsigned int words[1];
};
typedef std::vector<MSLRecord, app::app_allocator<MSLRecord> > MSLRecordVector;
#include "classes/WView.h"
// Borrow the current view layout; this performs no construction or ownership transfer.
static inline WView* AsWView(void* view) {
    return (WView*)view;
}
#define AsParticleEntry801B7D5C(p) ((PetalSilicaParticle*)(p))
#define AsParticleEmitterState801B7D5C(p) ((PetalSilicaEmitterView*)(p))
#include "classes/PetalSilicaEmitterView.h"
#include "game/types.h"
#include "game/wvector.h"
#include "sdk_protos.h"
#include "classes/CPetBody.h"
#include "classes/WPuppet.h"
#include "classes/CSharedDoc.h"
#include "classes/CameraTransform800D.h"
#include "classes/CWind.h"
#include "classes/CRain.h"

// Borrow the pet-body view; an inline replacement changes generated code with these pragmas.
#define AsCPetBody(p) ((CPetBody*)(p))

extern "C" {

extern const float kFloat_6_2831855;
extern float s_petParticleTwoPiCache;
extern const float kFloat_1_PetEffects;
extern const float kFloat_0_7_PetEffects;
extern float kFloat_Neg112_PetParticleExtent;
extern const float kFloat_0_3_PetEffects;
extern const float kFloat_112_PetParticleExtent;
extern const float kFloat_0_PetParticles;
extern const float kFloat_6_2831855_ParticleAngle;
extern float kFloat_3_1415927_PetEffects;
extern const float kFloat_neg1_PetEffects;
extern const float kFloat_2_PetParticleInitialUpdateLimit;
extern const float kFloat_34_295296_ParticleEmitter;
extern const float kFloat_1_5707964_PetEffects;
extern const float kFloat_99999_ParticleWind;
extern const float kFloat_4_ParticleOscillation;
extern const float kFloat_0_5_ParticleDisplayScale;
extern void* g_sharedDoc;
extern void* g_view;
void Vec3_Zero(void*);
void __ml__FfRC7WVector(void*, void*, float);
void __as__7WVectorFRC7WVector(void*, void*);
void Sprite_DrawColoredRect(void*, void*, int, unsigned int, float);
void appFreeUni__FPv(void*);
void __dl__FPv(void*);
void __dla__FPv(void*);
extern char __vt__7ntPetal[0x1C];
extern char __vt__16ntSilicaParticle[0x1C];
extern char __vt__8CPetBody[0x78];
extern const char petBodyStrings[];
extern const float kFloat_1_petBody;
extern const float kFloat_0_petBody;
extern void* g_resourceManager;
int appMallocUni__FUl(int);
void String_ReplaceRange(void*, int, unsigned int, const char*, const char*, const char*);
void Matrix_SetIdentityAffine(void*);
void Vec2_Zero(void*);
int String_ScanTokensAfterSubstring(const char*, const char*, ...);
void* ResourceManager_LoadByExtension(void*, const char*, int, int, int);
void Matrix_SetBasisScale(void*, void*);
void WPuppet_GetPivot(void*, void*, int);
void __apl__7WVectorFRC7WVector(void*, void*);
float __ml__FRC7WVectorRC7WVector(void*, void*);
void __mi__FRC7WVectorRC7WVector(void*, void*, void*);
void __ml__FRC7WVectorf(void*, void*, float);
void __as__7WVectorFRC7WVector(void*, void*);
void __pl__FRC7WVectorRC7WVector(void*, void*, void*);
extern "C" void GetWindVectorAtPoint__5CWindFRC7WVectorf(void* result, const void* self, const void* point,
                                                         float maxBaseMagnitude);
float Math_SinApprox(float);
float Math_CosApprox(float);
double ceil(double);
extern char s_cameraTransform[];
void __amu__7WVectorFf(void*, float);
void __ami__7WVectorFRC7WVector(void*, void*);
void* __nwa__FUl(unsigned int);
float Random_RangeFloat(int, float, float);
void __pl__FRC7WVectorRC7WVector(void*, void*, void*);
void* __nw__FUl(unsigned int);

void PetalEntry_ConstructionHook();
void SilicaEntry_ConstructionHook();
extern "C" void PlayAnimationByName__8CPetBodyFfPvii(void*, void*, int, int, float);
extern char s_cameraTransform[];
extern const char petFlowerPowderFilenameFormats[];

enum {
    kPetDrawVtableWord = 28,      // RTTI/null headers + 26: CPetBody_Draw.
    kPetInitialMovementState = 2, // Protocol value only; full state meaning is unresolved.
    kPetInitialLightingMode = 2,  // SetupLighting selects its mode-2 branch.
    kPetInstanceWordDCInitialValue = 8,
    kPetInitialWord12C = 0xFFFFFF,
    kParticleArrayCookieBytes = 0x10,
    kParticleAxisCount = 3,
    kParticleEntryBytes = sizeof(PetalSilicaParticle),
    kParticleDimOverlay = 0x4c3c3c3c,
    kEmitterBoundsMinOffset = (int)DECOMP_OFFSETOF(PetalSilicaEmitterView, boundsMin),
    kEmitterBoundsMaxOffset = (int)DECOMP_OFFSETOF(PetalSilicaEmitterView, boundsMax),
    kCameraPositionOffset = (int)DECOMP_OFFSETOF(CameraTransform800D, position),
    kCameraForwardOffset = (int)DECOMP_OFFSETOF(CameraTransform800D, basis[6])
};
// Particle storage: 0x38-byte entries follow a 0x10-byte new[] cookie. Each entry owns
// its CPetBody; emitter destruction deletes bodies before freeing the array allocation.
// position is a world-space base, oscillation is an additive world-space displacement,
// and renderPosition = position + oscillation. Angles azimuth/phase are in radians.
// Update delta uses the caller's time step; no seconds-to-milliseconds conversion occurs here.
struct ParticleAnimationRange {
    char reserved[0x18];
    void* name;
};
struct PetBodyStringPool {
    char petToken[5], stringFormat[3], shadowToken[8], integerFormat[3];
    char emptyString[1]; // The padding byte at +0x13 is used as an empty name.
    char headBone[5];
};
struct Entry801B {
    unsigned char pad[0x34];
    void* object;
};

struct Owner801B {
    void* vtable;
    unsigned char pad04[0x20];
    Entry801B* entries;
    int count;
};

typedef Vec3f Vec3_801B;
struct Vec3i_801B {
    int x, y, z;
};
struct Vec4_801B {
    float x, y, z, w;
};
struct Moving801B {
    void* vtable;
    float value;
    float displayScale;
    Vec3_801B a;
    Vec3_801B b;
    void* entries;
};

struct Deleting801B {
    virtual void release(int);
};

struct PetalSlot801B {
    virtual void v0();
    virtual void v1();
    virtual void v2();
    virtual void v3();
    virtual void v4();
    virtual void v5();
    virtual void v6();
    virtual void v7();
    virtual void v8();
    virtual void v9();
    virtual void v10();
    virtual void v11();
    virtual void v12();
    virtual void v13();
    virtual void v14();
    virtual void v15();
    virtual void v16();
    virtual void v17();
    virtual void v18();
    virtual void v19();
    virtual void v20();
    virtual void v21();
    virtual void v22();
    virtual void v23();
    virtual void v24();
    virtual void v25();
    virtual void v26();
    virtual void update(float);
};

// Initialize the silica emitter, its camera-relative bounds, and its particle storage.
// arg is forwarded to virtual slot 4 as its count argument; this C ABI models it as void*.
void* ntSilicaParticle_Ctor(void* ptr, void* arg) {
    float __f0 = kFloat_1_PetEffects;
    char* self = (char*)ptr;
    AsParticleEmitterState801B7D5C(self)->gravityScale = __f0;
    *(void**)self = &__vt__16ntSilicaParticle;
    Vec3_Zero(self + kEmitterBoundsMinOffset);
    Vec3_Zero(self + kEmitterBoundsMaxOffset);
    AsParticleEmitterState801B7D5C(self)->particles = 0;
    struct Pair {
        float x, y, z;
    } dst1, src1, dst2, src2;
    src1.x = kFloat_1_PetEffects;
    src1.y = kFloat_0_7_PetEffects;
    src1.z = kFloat_1_PetEffects;
    __ml__FfRC7WVector(&dst1, &src1, kFloat_Neg112_PetParticleExtent);
    *(WVector*)AsParticleEmitterState801B7D5C(self)->boundsMin = *(WVector*)&dst1;
    src2.x = kFloat_1_PetEffects;
    src2.y = kFloat_0_3_PetEffects;
    src2.z = kFloat_1_PetEffects;
    __ml__FfRC7WVector(&dst2, &src2, kFloat_112_PetParticleExtent);
    *(WVector*)AsParticleEmitterState801B7D5C(self)->boundsMax = *(WVector*)&dst2;
    struct Vtbl6_801B {
        virtual void a0() DECOMP_LOCAL_VIRTUAL;
        virtual void a1() DECOMP_LOCAL_VIRTUAL;
        virtual void a2() DECOMP_LOCAL_VIRTUAL;
        virtual void a3() DECOMP_LOCAL_VIRTUAL;
        virtual void a6(void*, float) DECOMP_LOCAL_VIRTUAL;
    };
    ((Vtbl6_801B*)ptr)->a6(arg, kFloat_1_PetEffects);
    return ptr;
}

// Draw visible petal objects and the optional full-screen dim overlay.
void ntPetal_DrawPetals(void* ptr) {
    Owner801B* self = (Owner801B*)ptr;
    Vec4_801B overlayRect;
    volatile float _pad[3];
    volatile float scale = kFloat_0_5_ParticleDisplayScale * AsParticleEmitterState801B7D5C(ptr)->displayScale;
    int offset = 0;
    int index = 0;
    while (index < self->count) {
        if (AsParticleEntry801B7D5C((char*)self->entries + offset)->visible) {
            ((CPetBody*)(AsParticleEntry801B7D5C((char*)self->entries + offset)->object))
                ->SetPosition((char*)self->entries + offset);
            ((CPetBody*)(AsParticleEntry801B7D5C((char*)self->entries + offset)->object))->SetupLighting();
            void* o = AsParticleEntry801B7D5C((char*)self->entries + offset)->object;
            ((void (**)(void*, int, int))(*(void**)o))[kPetDrawVtableWord](o, 1, 1);
        }
        offset += kParticleEntryBytes;
        ++index;
    }
    if (*(unsigned char*)&((CSharedDoc*)g_sharedDoc)->pad_9C6[0]) {
        void* p = (void*)g_view;
        float viewportHeight = AsWView(p)->viewportHeight;
        float viewportWidth = AsWView(p)->viewportWidth;
        overlayRect.x = kFloat_0_PetParticles;
        overlayRect.y = kFloat_0_PetParticles;
        overlayRect.z = viewportWidth;
        overlayRect.w = viewportHeight;
        Sprite_DrawColoredRect(p, &overlayRect, 0, kParticleDimOverlay,
                               kFloat_0_PetParticles); // original f1 = kFloat_0_PetParticles (0x801B8494 / 0x801B8CF0)
    }
}

// Draw visible silica objects and the optional full-screen dim overlay.
void ntSilicaParticle_DrawParticlesAndOverlay(void* ptr) {
    Owner801B* self = (Owner801B*)ptr;
    Vec4_801B overlayRect;
    volatile float _pad[3];
    volatile float scale = kFloat_0_5_ParticleDisplayScale * AsParticleEmitterState801B7D5C(ptr)->displayScale;
    int offset = 0;
    int index = 0;
    while (index < self->count) {
        if (AsParticleEntry801B7D5C((char*)self->entries + offset)->visible) {
            ((CPetBody*)(AsParticleEntry801B7D5C((char*)self->entries + offset)->object))
                ->SetPosition((char*)self->entries + offset);
            ((CPetBody*)(AsParticleEntry801B7D5C((char*)self->entries + offset)->object))->SetupLighting();
            // ABI slots 0..25 precede CPetBody_Draw. This call draws; it does not tick time.
            struct PetDrawSlots801B {
                virtual void a0() DECOMP_LOCAL_VIRTUAL;
                virtual void a1() DECOMP_LOCAL_VIRTUAL;
                virtual void a2() DECOMP_LOCAL_VIRTUAL;
                virtual void a3() DECOMP_LOCAL_VIRTUAL;
                virtual void a4() DECOMP_LOCAL_VIRTUAL;
                virtual void a5() DECOMP_LOCAL_VIRTUAL;
                virtual void a6() DECOMP_LOCAL_VIRTUAL;
                virtual void a7() DECOMP_LOCAL_VIRTUAL;
                virtual void a8() DECOMP_LOCAL_VIRTUAL;
                virtual void a9() DECOMP_LOCAL_VIRTUAL;
                virtual void a10() DECOMP_LOCAL_VIRTUAL;
                virtual void a11() DECOMP_LOCAL_VIRTUAL;
                virtual void a12() DECOMP_LOCAL_VIRTUAL;
                virtual void a13() DECOMP_LOCAL_VIRTUAL;
                virtual void a14() DECOMP_LOCAL_VIRTUAL;
                virtual void a15() DECOMP_LOCAL_VIRTUAL;
                virtual void a16() DECOMP_LOCAL_VIRTUAL;
                virtual void a17() DECOMP_LOCAL_VIRTUAL;
                virtual void a18() DECOMP_LOCAL_VIRTUAL;
                virtual void a19() DECOMP_LOCAL_VIRTUAL;
                virtual void a20() DECOMP_LOCAL_VIRTUAL;
                virtual void a21() DECOMP_LOCAL_VIRTUAL;
                virtual void a22() DECOMP_LOCAL_VIRTUAL;
                virtual void a23() DECOMP_LOCAL_VIRTUAL;
                virtual void a24() DECOMP_LOCAL_VIRTUAL;
                virtual void a25() DECOMP_LOCAL_VIRTUAL;
                virtual void update(int, int) DECOMP_LOCAL_VIRTUAL;
                // Likely signature: draw(useViewDraw, drawEnabled).
            };
            ((PetDrawSlots801B*)AsParticleEntry801B7D5C((char*)self->entries + offset)->object)->update(1, 1);
        }
        offset += kParticleEntryBytes;
        ++index;
    }
    if (*(unsigned char*)&((CSharedDoc*)g_sharedDoc)->pad_9C6[0]) {
        void* p = (void*)g_view;
        float viewportHeight = AsWView(p)->viewportHeight;
        float viewportWidth = AsWView(p)->viewportWidth;
        overlayRect.x = kFloat_0_PetParticles;
        overlayRect.y = kFloat_0_PetParticles;
        overlayRect.z = viewportWidth;
        overlayRect.w = viewportHeight;
        Sprite_DrawColoredRect(p, &overlayRect, 0, kParticleDimOverlay,
                               kFloat_0_PetParticles); // original f1 = kFloat_0_PetParticles (0x801B8494 / 0x801B8CF0)
    }
}

#pragma push
#pragma use_lmw_stmw on
// Destroy particle objects and their array, then optionally delete the petal emitter.
void* ntPetal_dtor(void* ptr, int flag) {
    if (ptr != 0) {
        char* self = (char*)ptr;
        *(void**)self = &__vt__7ntPetal;
        int particleIndex = 0;
        int entryByteOffset = 0;
        void* zero = 0;
        while (particleIndex < AsParticleEmitterState801B7D5C(self)->particleCount) {
            if (AsParticleEntry801B7D5C((*(char**)&AsParticleEmitterState801B7D5C(self)->particles) + entryByteOffset)
                    ->object != 0) {
                if (AsParticleEntry801B7D5C((*(char**)&AsParticleEmitterState801B7D5C(self)->particles) +
                                            entryByteOffset)
                        ->object != 0) {
                    void* o = AsParticleEntry801B7D5C((*(char**)&AsParticleEmitterState801B7D5C(self)->particles) +
                                                      entryByteOffset)
                                  ->object;
                    ((Deleting801B*)o)->release(1);
                }
                AsParticleEntry801B7D5C((*(char**)&AsParticleEmitterState801B7D5C(self)->particles) + entryByteOffset)
                    ->object = zero;
            }
            entryByteOffset += kParticleEntryBytes;
            ++particleIndex;
        }
        if (AsParticleEmitterState801B7D5C(self)->particles != 0) {
            if (AsParticleEmitterState801B7D5C(self)->particles != 0) {
                __dla__FPv((*(char**)&AsParticleEmitterState801B7D5C(self)->particles) - 0x10);
            }
            AsParticleEmitterState801B7D5C(self)->particles = 0;
        }
        if (flag > 0)
            __dl__FPv(self);
    }
    return ptr;
}

// Destroy particle objects and their array, then optionally delete the silica emitter.
void* ntSilicaParticle_dtor(void* ptr, int flag) {
    if (ptr != 0) {
        char* self = (char*)ptr;
        *(void**)self = &__vt__16ntSilicaParticle;
        int particleIndex = 0;
        int entryByteOffset = 0;
        void* zero = 0;
        while (particleIndex < AsParticleEmitterState801B7D5C(self)->particleCount) {
            if (AsParticleEntry801B7D5C((*(char**)&AsParticleEmitterState801B7D5C(self)->particles) + entryByteOffset)
                    ->object != 0) {
                if (AsParticleEntry801B7D5C((*(char**)&AsParticleEmitterState801B7D5C(self)->particles) +
                                            entryByteOffset)
                        ->object != 0) {
                    void* o = AsParticleEntry801B7D5C((*(char**)&AsParticleEmitterState801B7D5C(self)->particles) +
                                                      entryByteOffset)
                                  ->object;
                    ((Deleting801B*)o)->release(1);
                }
                AsParticleEntry801B7D5C((*(char**)&AsParticleEmitterState801B7D5C(self)->particles) + entryByteOffset)
                    ->object = zero;
            }
            entryByteOffset += kParticleEntryBytes;
            ++particleIndex;
        }
        if (AsParticleEmitterState801B7D5C(self)->particles != 0) {
            if (AsParticleEmitterState801B7D5C(self)->particles != 0) {
                __dla__FPv((*(char**)&AsParticleEmitterState801B7D5C(self)->particles) - 0x10);
            }
            AsParticleEmitterState801B7D5C(self)->particles = 0;
        }
        if (flag > 0)
            __dl__FPv(self);
    }
    return ptr;
}
#pragma pop

struct WInstanceLock {
    void* lock;
    // Acquire one recursive mutex level for the instance-list operation.
    WInstanceLock(void* l) {
        lock = l;
        OSLockMutex((OSMutex*)l);
    }
    // Release the guard's mutex level on normal exit or unwinding.
    ~WInstanceLock() {
        OSUnlockMutex((OSMutex*)lock);
    }
};

struct LNode801B {
    char nodePayload[0xC];
    LNode801B* volatile next;
    LNode801B* volatile prev;
};

struct LOwner801B {
    char ownerPrefix[8];
    LNode801B* head;
    char ownerStateBeforeBuffer[0x10];
    void* buf;
    char lock[4];
};

#pragma push
#pragma use_lmw_stmw on
// Unlink instance nodes under the mutex, release storage, and optionally delete the owner.
LOwner801B* dtor_801B8F98(LOwner801B* self, int flag) {
    if (self) {
        {
            WInstanceLock g(&self->lock);
            if (self->head) {
                LNode801B* nx;
                void* lk = &self->lock;
                LNode801B* n;
                while ((n = self->head) != 0) {
                    OSLockMutex((OSMutex*)lk);
                    n->prev->next = n->next;
                    n->next->prev = n->prev;
                    nx = n->prev;
                    if (nx == n)
                        nx = 0;
                    OSUnlockMutex((OSMutex*)lk);
                    self->head = nx;
                    appFreeUni__FPv(n);
                }
            }
            if (self->buf) {
                appFreeUni__FPv(self->buf);
                self->buf = 0;
            }
        }
        if (flag > 0)
            __dl__FPv(self);
    }
    return self;
}
#pragma pop

// Both initializers below use the shared ParticleEmitterState/ParticleEntry views.
// Each 0x38-byte entry owns one model body; positions/displacements use game world
// units, angles use radians, and displayScale is a dimensionless model multiplier.
// No metre conversion or real-time unit for the animation step is established here.

#pragma push
#pragma opt_loop_invariants off // Preserve the original pre-loop loads.
#pragma opt_rotateloops off     // Keep the condition at the bottom, matching the original.
// Recreate petal entries with randomized positions, phases, models and animation offsets.
// count must be nonnegative and displayScale sets the model display multiplier. Existing arrays are released
// directly here; this is an initialization path, not an object-preserving resize operation.
void ntPetal_RecreateParticleEntries(PetalSilicaEmitterView* self, int count, float displayScale) {
    void* entries0 = self->particles;
    self->particleCount = count;
    if (entries0 != 0) {
        if (entries0 != 0) {
            __dla__FPv((char*)entries0 - kParticleArrayCookieBytes);
        }
        self->particles = 0;
    }
    // Phase 1: allocate entries (including the array cookie); construction hooks are empty.
    int allocationCount = self->particleCount;
    int arrayConstructed = 0;
    void* block = __nwa__FUl(allocationCount * kParticleEntryBytes + kParticleArrayCookieBytes);
    if (block != 0) {
        arrayConstructed = 1;
        block =
            __construct_new_array(block, (void*)PetalEntry_ConstructionHook, 0, kParticleEntryBytes, allocationCount);
    }
    self->particles = (PetalSilicaParticle*)block;
    arrayConstructed = 0;
    float scale = kFloat_112_PetParticleExtent;
    self->displayScale = displayScale;
    int entryByteOffset = 0;
    int index = 0;
    int one = 1;
    // Phase 2: seed world positions around the current camera, then create one model per entry.
    while (index < self->particleCount) {
        float azimuth = Random_RangeFloat(-1, kFloat_0_PetParticles, kFloat_6_2831855_ParticleAngle);
        float phaseLimitRadians = kFloat_3_1415927_PetEffects;
        ((PetalSilicaParticle*)((char*)self->particles + entryByteOffset))->azimuth = azimuth;
        ((PetalSilicaParticle*)((char*)self->particles + entryByteOffset))->phase =
            Random_RangeFloat(-1, kFloat_0_PetParticles, phaseLimitRadians);
        {
            const float* cameraTransform = (const float*)s_cameraTransform;
            int axisByteOffset = 0;
            int axisIndex = 0;
            for (; axisIndex < kParticleAxisCount; axisByteOffset += 4, ++axisIndex) {
                float random = Random_RangeFloat(-1, kFloat_neg1_PetEffects, kFloat_1_PetEffects);
                AsParticleEntry801B7D5C((char*)self->particles + (axisByteOffset + entryByteOffset))->position[0] =
                    scale * random + cameraTransform[9 + axisIndex];
            }
        }
        Vec3_801B matrix;
        char* entryBytes = (char*)self->particles + entryByteOffset;
        __pl__FRC7WVectorRC7WVector(&matrix, entryBytes + 0xc, entryBytes + 0x1c);
        *(WVector*)((char*)self->particles + entryByteOffset) = *(WVector*)&matrix;
        int modelRandomBits = rand();
        unsigned int randomSign = (unsigned int)modelRandomBits >> 31;
        int modelIndex = (((modelRandomBits & 1) ^ randomSign) - randomSign) + 1;
        unsigned int formatted[0x40];
        sprintf((char*)formatted, petFlowerPowderFilenameFormats, modelIndex);
        void* object = __nw__FUl(0x1b4);
        void* firstAnimationRange = 0;
        if (object != 0) {
            {
                try {
                    ((CPetBody*)object)->__ct((char*)formatted, 0);
                } catch (...) {
                    __dl__FPv(object);
                    throw;
                }
            }
        }
        ((PetalSilicaParticle*)((char*)self->particles + entryByteOffset))->object = object;
        void* resource =
            ((CPetBody*)(((PetalSilicaParticle*)((char*)self->particles + entryByteOffset))->object))->GetPuppet();
        firstAnimationRange = ((WPuppet*)resource)->animationRanges;
        if (firstAnimationRange != 0) {
            firstAnimationRange = *(void**)firstAnimationRange;
            firstAnimationRange = *(void**)firstAnimationRange;
        }
        if (firstAnimationRange != 0)
            ((void (*)(void*, void*, int, int, float))PlayAnimationByName__8CPetBodyFfPvii)(
                ((PetalSilicaParticle*)((char*)self->particles + entryByteOffset))->object,
                ((ParticleAnimationRange*)firstAnimationRange)->name, 0, 0, kFloat_0_PetParticles);
        float initialAnimationDelta =
            Random_RangeFloat(-1, kFloat_0_PetParticles, kFloat_2_PetParticleInitialUpdateLimit);
        ((void (**)(void*, float))(
            *(void**)((PetalSilicaParticle*)((char*)self->particles + entryByteOffset))->object))[29](
            ((PetalSilicaParticle*)((char*)self->particles + entryByteOffset))->object, initialAnimationDelta);
        AsCPetBody(((PetalSilicaParticle*)((char*)self->particles + entryByteOffset))->object)
            ->defaultRenderParametersEnabled = (unsigned char)one;
        ++index;
        entryByteOffset += kParticleEntryBytes;
    }
}

#pragma pop
struct CPetBodyStorage {
    char data[0x1b4];
    // Construct one particle's owned pet body from its model command.
    CPetBodyStorage(const char* name, int mode) {
        ((CPetBody*)this)->__ct(name, mode);
    }
};
struct Array8684 {
    char data[0x38];
    // Invoke the empty entry construction hook used by the array runtime.
    Array8684() {
        SilicaEntry_ConstructionHook();
    }
    // Allocate cookie-backed particle array storage through the game allocator.
    static void* operator new[](size_t size) {
        return __nwa__FUl(size);
    }
};
struct Virtual8684 {
    virtual void a0();
    virtual void a1();
    virtual void a2();
    virtual void a3();
    virtual void a4();
    virtual void a5();
    virtual void a6();
    virtual void a7();
    virtual void a8();
    virtual void a9();
    virtual void a10();
    virtual void a11();
    virtual void a12();
    virtual void a13();
    virtual void a14();
    virtual void a15();
    virtual void a16();
    virtual void a17();
    virtual void a18();
    virtual void a19();
    virtual void a20();
    virtual void a21();
    virtual void a22();
    virtual void a23();
    virtual void a24();
    virtual void a25();
    virtual void a26();
    virtual void update(float);
};

#pragma push
#pragma opt_loop_invariants off // Preserve the original pre-loop loads.
#pragma opt_rotateloops off     // Keep the condition at the bottom, matching the original.
// Recreate silica entries with randomized positions, phases, models and animation offsets.
// Same count/display-scale and initialization-only contract as the petal path above.
void ntSilicaParticle_InitializeParticles(void* ptr, int count, float displayScale) {
    PetalSilicaEmitterView* self = (PetalSilicaEmitterView*)ptr;
    int entryByteOffset;

    self->particleCount = count;
    if (self->particles) {
        if (self->particles)
            __dla__FPv((char*)self->particles - kParticleArrayCookieBytes);
        self->particles = 0;
    }
    int allocationCount = self->particleCount;
    // Allocate cookie-backed entries, then seed each body and its animation independently.
    self->particles = (PetalSilicaParticle*)new Array8684[allocationCount];
    AsParticleEmitterState801B7D5C(ptr)->displayScale = displayScale;

    int index = 0;
    entryByteOffset = 0;
    while (index < self->particleCount) {
        // Seed azimuth in [0, 2*pi], phase in [0, pi]; positions are camera-relative world XYZ.
        float azimuth = Random_RangeFloat(-1, kFloat_0_PetParticles, kFloat_6_2831855_ParticleAngle);
        int axisByteOffset;

        float pi = kFloat_3_1415927_PetEffects;

        AsParticleEntry801B7D5C((char*)self->particles + entryByteOffset)->azimuth = azimuth;

        const float* cameraTransform = (const float*)s_cameraTransform;

        AsParticleEntry801B7D5C((char*)self->particles + entryByteOffset)->phase =
            (Random_RangeFloat(-1, kFloat_0_PetParticles, pi));

        int axisIndex = 0;
        axisByteOffset = 0;
        for (; axisIndex < kParticleAxisCount; ++axisIndex, axisByteOffset += 4) {
            float random = Random_RangeFloat(-1, kFloat_neg1_PetEffects, kFloat_1_PetEffects);
            // Retain these coordinate-copy lifetimes: collapsing them reorders instructions.
            PetalSilicaParticle* entriesForCoordinate = (PetalSilicaParticle*)self->particles;
            int entryByteOffsetSnapshot = entryByteOffset;
            int coordinateByteOffset = axisByteOffset + entryByteOffsetSnapshot;
            float scale = kFloat_112_PetParticleExtent;
            float positionScale = scale;
            float unitOffset = random;
            AsParticleEntry801B7D5C((char*)entriesForCoordinate + coordinateByteOffset)->position[0] =
                positionScale * unitOffset + cameraTransform[9 + axisIndex];
        }
        // Publish base + sway, choose model 1 or 2, then start its first animation range.
        struct {
            Vec3_801B matrix;
            char formatted[0x100];
        } locals;

        char* entry = (char*)self->particles + entryByteOffset;
        __pl__FRC7WVectorRC7WVector(&locals.matrix, entry + 0x0c, entry + 0x1c);

        *(WVector*)((char*)self->particles + entryByteOffset) = *(WVector*)(&locals.matrix);
        const char* format = petFlowerPowderFilenameFormats + 0x18;
        int randomModelBits = rand();

        int modelParity = 1 & randomModelBits;
        unsigned int randomSign = (unsigned int)randomModelBits >> 31;

        int modelIndexMinusOne = modelParity ^ randomSign;
        modelIndexMinusOne = modelIndexMinusOne - randomSign;

        sprintf(locals.formatted, format, (1 + modelIndexMinusOne));
        void* object = new CPetBodyStorage(locals.formatted, 0);

        void* initialRange = 0;
        AsParticleEntry801B7D5C((char*)self->particles + entryByteOffset)->object = object;

        void* resource =
            ((CPetBody*)(AsParticleEntry801B7D5C((char*)self->particles + entryByteOffset)->object))->GetPuppet();
        void* list = ((WPuppet*)resource)->animationRanges;
        if (list) {
            initialRange = **(void***)list;
        }
        if (initialRange)
            ((void (*)(void*, void*, int, int, float))PlayAnimationByName__8CPetBodyFfPvii)(
                AsParticleEntry801B7D5C((char*)self->particles + entryByteOffset)->object,
                ((ParticleAnimationRange*)initialRange)->name, 0, 0, kFloat_0_PetParticles);

        float initialAnimationDelta =
            Random_RangeFloat(-1, kFloat_0_PetParticles, kFloat_2_PetParticleInitialUpdateLimit);

        ((Virtual8684*)AsParticleEntry801B7D5C((char*)self->particles + entryByteOffset)->object)
            ->update(initialAnimationDelta);

        AsCPetBody(AsParticleEntry801B7D5C((char*)self->particles + entryByteOffset)->object)
            ->defaultRenderParametersEnabled = 1;
        entryByteOffset += kParticleEntryBytes;

        index = (index + 1);
    }
}

#pragma pop
// Byte cursors preserve the entry/coordinate addition order used by the original compiler.
inline char* ParticleEntryBytes(void* emitter, int byteOffset) {
    return (*(char**)&AsParticleEmitterState801B7D5C(emitter)->particles) + byteOffset;
}
// Borrow one base-position coordinate; both offsets are byte offsets, not element indices.
inline float& ParticlePositionComponent(void* emitter, int entryByteOffset, int axisByteOffset) {
    // Select position[X/Y/Z] with the existing byte cursor; indexing by axis/4
    // changes the emitted address arithmetic. The layout supplies position's +0C.
    return AsParticleEntry801B7D5C((*(char**)&AsParticleEmitterState801B7D5C(emitter)->particles) +
                                   (axisByteOffset + entryByteOffset))
        ->position[0];
}

#pragma opt_common_subs off
// Apply wind, gravity and oscillation; wrap particles around the camera bounds and update visibility.
void ntPetal_UpdateParticles(unsigned char* self, float delta) {
    // Phase 1: translate the emitter-relative box into world space around the camera.
    unsigned char* cameraTransform = (unsigned char*)s_cameraTransform;
    struct Bounds {
        Vec3i_801B a, b;
        // Copy min/max coordinate bits before translating both bounds to world space.
        Bounds(unsigned char* object)
            : a(*(Vec3i_801B*)AsParticleEmitterState801B7D5C(object)->boundsMin),
              b(*(Vec3i_801B*)AsParticleEmitterState801B7D5C(object)->boundsMax) {}
    } bounds(self);
    *(WVector*)&bounds.a += *(WVector*)(cameraTransform + kCameraPositionOffset);
    *(WVector*)&bounds.b += *(WVector*)(cameraTransform + kCameraPositionOffset);
    Vec3_801B boxExtents;
    __mi__FRC7WVectorRC7WVector(&boxExtents, AsParticleEmitterState801B7D5C(self)->boundsMax,
                                AsParticleEmitterState801B7D5C(self)->boundsMin);
    float gravityAcceleration = kFloat_34_295296_ParticleEmitter;
    float phaseRate = kFloat_1_5707964_PetEffects;
    int offset = 0;
    int index = 0;
    float z = kFloat_0_PetParticles;
    while (index < AsParticleEmitterState801B7D5C(self)->particleCount) {
        Vec3_801B windVector;
        GetWindVectorAtPoint__5CWindFRC7WVectorf(&windVector, CWind::GetInstance(),
                                                 ParticleEntryBytes(self, offset) + 0x0c, kFloat_99999_ParticleWind);
        Vec3_801B q;
        __ml__FRC7WVectorf(&q, &windVector, delta);
        *(WVector*)AsParticleEntry801B7D5C(ParticleEntryBytes(self, offset))->position += *(WVector*)&q;
        AsParticleEntry801B7D5C(ParticleEntryBytes(self, offset))->position[1] -=
            gravityAcceleration * AsParticleEmitterState801B7D5C(self)->gravityScale * delta;
        AsParticleEntry801B7D5C(ParticleEntryBytes(self, offset))->phase += phaseRate * delta;
        float sy = Math_SinApprox(AsParticleEntry801B7D5C(ParticleEntryBytes(self, offset))->azimuth);
        float sx = Math_CosApprox(AsParticleEntry801B7D5C(ParticleEntryBytes(self, offset))->azimuth);
        Vec3_801B azimuthDirection = {sx, z, sy};
        float swayFactor = Math_SinApprox(AsParticleEntry801B7D5C(ParticleEntryBytes(self, offset))->phase);
        Vec3_801B m;
        __ml__FfRC7WVector(&m, &azimuthDirection, kFloat_4_ParticleOscillation);
        Vec3_801B n;
        __ml__FRC7WVectorf(&n, &m, swayFactor);
        *(WVector*)AsParticleEntry801B7D5C(ParticleEntryBytes(self, offset))->oscillation = *(WVector*)&n;
        // Phase 2: wrap by whole box extents, allowing travel across multiple widths per tick.
        // Each extent must be positive. pa[0]/pa[3] are the adjacent min/max vectors.
        int k = 0;
        float* pa = (float*)&bounds;
        float* pb = (float*)&boxExtents;
        for (int i = 0; i < 3; ++i, k += 4, ++pa, ++pb) {
            if (ParticlePositionComponent(self, offset, k) > pa[3]) {
                double x = ceil((ParticlePositionComponent(self, offset, k) - pa[3]) / *pb);
                ParticlePositionComponent(self, offset, k) -= *pb * (float)x;
            } else if (ParticlePositionComponent(self, offset, k) < pa[0]) {
                double x = ceil((pa[0] - ParticlePositionComponent(self, offset, k)) / *pb);
                ParticlePositionComponent(self, offset, k) += *pb * (float)x;
            }
        }
        // Phase 3: add sway and test only the camera-forward half-space, not the full frustum.
        Vec3_801B tmp;
        char* e = ParticleEntryBytes(self, offset);
        __pl__FRC7WVectorRC7WVector(&tmp, e + 0x0c, e + 0x1c);
        *(WVector*)ParticleEntryBytes(self, offset) = *(WVector*)&tmp;
        Vec3_801B tmp2;
        __mi__FRC7WVectorRC7WVector(&tmp2, ParticleEntryBytes(self, offset), cameraTransform + kCameraPositionOffset);
        float result = (*(WVector*)&tmp2 * *(WVector*)(cameraTransform + kCameraForwardOffset));
        AsParticleEntry801B7D5C((*(char**)&AsParticleEmitterState801B7D5C(self)->particles) + offset)->visible =
            result > z;
        ((PetalSlot801B*)AsParticleEntry801B7D5C(ParticleEntryBytes(self, offset))->object)->update(delta);
        offset += kParticleEntryBytes;
        ++index;
    }
}

#pragma opt_common_subs reset

#pragma opt_common_subs off
#pragma opt_propagation off
// Advance silica motion, wrap coordinates into camera-relative bounds, and update each pet object.
void ntSilicaParticle_UpdateParticles(unsigned char* self, float delta_arg) {
    // Translate bounds to world space; integrate wind/downward drift and sinusoidal sway.
    unsigned char* cameraTransform = (unsigned char*)s_cameraTransform;
    Vec3i_801B worldBoundsMax;
    Vec3i_801B worldBoundsMin;
    *(struct Arr3*)&worldBoundsMin = *(struct Arr3*)AsParticleEmitterState801B7D5C(self)->boundsMin;
    *(struct Arr3*)&worldBoundsMax = *(struct Arr3*)AsParticleEmitterState801B7D5C(self)->boundsMax;
    __apl__7WVectorFRC7WVector(&worldBoundsMin, cameraTransform + kCameraPositionOffset);
    __apl__7WVectorFRC7WVector(&worldBoundsMax, cameraTransform + kCameraPositionOffset);
    float boxExtents[3];
    __mi__FRC7WVectorRC7WVector(boxExtents, AsParticleEmitterState801B7D5C(self)->boundsMax,
                                AsParticleEmitterState801B7D5C(self)->boundsMin);
    float phaseRate;
    float gravityAcceleration = kFloat_34_295296_ParticleEmitter;
    float delta = delta_arg;
    float sy;
    int offset;
    int index = 0;
    phaseRate = kFloat_1_5707964_PetEffects;
    offset = 0;
    float z = kFloat_0_PetParticles;
    while (index < AsParticleEmitterState801B7D5C(self)->particleCount) {
        CWind* wind = CWind::GetInstance();
        Vec3_801B q;
        Vec3_801B windVector;
        Vec3_801B n;
        Vec3_801B m;
        Vec3_801B azimuthDirection;
        GetWindVectorAtPoint__5CWindFRC7WVectorf(&windVector, wind, ParticleEntryBytes(self, offset) + 0x0c,
                                                 kFloat_99999_ParticleWind);
        __ml__FRC7WVectorf(&q, &windVector, delta);
        __apl__7WVectorFRC7WVector(ParticleEntryBytes(self, offset) + 0x0c, &q);
        float gravity = gravityAcceleration * AsParticleEmitterState801B7D5C(self)->gravityScale;
        AsParticleEntry801B7D5C(ParticleEntryBytes(self, offset))->position[1] -= gravity * delta;
        AsParticleEntry801B7D5C(ParticleEntryBytes(self, offset))->phase += phaseRate * delta;
        sy = Math_SinApprox(AsParticleEntry801B7D5C(ParticleEntryBytes(self, offset))->azimuth);
        float sx = Math_CosApprox(AsParticleEntry801B7D5C(ParticleEntryBytes(self, offset))->azimuth);
        azimuthDirection.x = sx;
        azimuthDirection.y = z;
        azimuthDirection.z = sy;
        float swayFactor = Math_SinApprox(AsParticleEntry801B7D5C(ParticleEntryBytes(self, offset))->phase);
        __ml__FfRC7WVector(&m, &azimuthDirection, kFloat_4_ParticleOscillation);
        __ml__FRC7WVectorf(&n, &m, swayFactor);
        __as__7WVectorFRC7WVector(ParticleEntryBytes(self, offset) + 0x1c, &n);
        // Wrap the base position by complete positive extents, then publish base + sway.
        int k;
        float* pa = (float*)&worldBoundsMin;
        float* pb = boxExtents;
        int i = 0;
        k = 0;
        for (; i < 3; ++i, k += 4, ++pa, ++pb) {
            if (ParticlePositionComponent(self, offset, k) > pa[3]) {
                double x = ceil((ParticlePositionComponent(self, offset, k) - pa[3]) / *pb);
                ParticlePositionComponent(self, offset, k) -= *pb * (float)x;
            } else if (ParticlePositionComponent(self, offset, k) < pa[0]) {
                double x = ceil((pa[0] - ParticlePositionComponent(self, offset, k)) / *pb);
                ParticlePositionComponent(self, offset, k) += *pb * (float)x;
            }
        }
        Vec3_801B tmp;
        char* e = ParticleEntryBytes(self, offset);
        __pl__FRC7WVectorRC7WVector(&tmp, e + 0x0c, e + 0x1c);
        __as__7WVectorFRC7WVector(ParticleEntryBytes(self, offset), &tmp);
        Vec3_801B tmp2;
        __mi__FRC7WVectorRC7WVector(&tmp2, ParticleEntryBytes(self, offset), cameraTransform + kCameraPositionOffset);
        // Positive dot(renderPosition - cameraPosition, cameraForward) means in front.
        float result = __ml__FRC7WVectorRC7WVector(&tmp2, cameraTransform + kCameraForwardOffset);
        AsParticleEntry801B7D5C((*(char**)&AsParticleEmitterState801B7D5C(self)->particles) + offset)->visible =
            result > z;
        ((PetalSlot801B*)AsParticleEntry801B7D5C(ParticleEntryBytes(self, offset))->object)->update(delta);
        offset += kParticleEntryBytes;
        ++index;
    }
}

#pragma opt_propagation reset
#pragma opt_common_subs reset

// Empty construction hook used for petal array entries.
void PetalEntry_ConstructionHook() {}
// Empty construction hook used for silica array entries.
void SilicaEntry_ConstructionHook() {}
// Unused silica particle virtual hook.
void ntSilicaParticle_vfunc_3() {}
// Unused rain configuration virtual hook in this unit.
void CRain::vfunc_4() {}
// Initialize this particle helper's two-pi cache.
void PetParticles_InitAngleCache() {
    s_petParticleTwoPiCache = kFloat_6_2831855;
}

// Release the owned buffer, clear its count and optionally delete the wrapper.
void* dtor_801B9070(MSLRecordVector* self, int flag) {
    return ::std::__vector_deleting_dtor(self, flag);
}

#pragma push
#pragma opt_common_subs off
#pragma opt_propagation off
#pragma opt_lifetimes off
#pragma opt_strength_reduction off
#pragma use_lmw_stmw on
// Reset pet playback, movement, matrices, lighting and look-at state without loading a model.
// Initialization-only: this clears resource pointers without releasing their former objects.
void CPetBody::InitDefaults(void* ptr) {
    unsigned char* self = (unsigned char*)ptr;
    int two = kPetInitialMovementState;
    int zero = 0;
    float f0 = kFloat_1_petBody;
    float f1 = kFloat_0_petBody;
    char* name = (char*)((const PetBodyStringPool*)petBodyStrings)->emptyString;
    char buf[8];
    unsigned int w = (*(unsigned int*)&AsCPetBody(self)->nameStringFlags);
    (void)buf;
    (*(int*)&AsCPetBody(self)->currentAnimation) = zero;
    (*(int*)&AsCPetBody(self)->puppet) = zero;
    (*(int*)&AsCPetBody(self)->followTarget) = zero;
    AsCPetBody(self)->animationTime = f1;
    AsCPetBody(self)->uniformScale = f0;
    AsCPetBody(self)->rootMotionScale = f0;
    AsCPetBody(self)->animationSpeed = f0;
    (*(int*)&AsCPetBody(self)->movementState) = two;
    AsCPetBody(self)->flightMovementEnabled = (unsigned char)zero;
    AsCPetBody(self)->queuedAnimationName[0] = (unsigned char)zero;
    unsigned int r30;
    switch (w >> 31) {
    case 0:
        r30 = (*(unsigned char*)&AsCPetBody(self)->nameStringFlags) & 0x7f;
        break;
    default:
        r30 = (*(unsigned int*)&AsCPetBody(self)->nameStringLength);
        break;
    }
    buf[0] = buf[4];
    String_ReplaceRange(&AsCPetBody(self)->nameStringFlags, 0, r30, name, name + strlen(name), buf);
    Vec3_Zero(&AsCPetBody(self)->rotationX);
    Vec3_Zero(&AsCPetBody(self)->positionX);
    Vec3_Zero(AsCPetBody(self)->movementDirection);
    Matrix_SetIdentityAffine(AsCPetBody(self)->baseMatrixBasis);
    Matrix_SetIdentityAffine(AsCPetBody(self)->matrixBasis);
    AsCPetBody(self)->renderParametersEnabled = 1;
    AsCPetBody(self)->defaultRenderParametersEnabled = 1;
    AsCPetBody(self)->lightingMode = kPetInitialLightingMode;
    Vec2_Zero(&AsCPetBody(self)->lookAtPitch);
    Vec2_Zero(&AsCPetBody(self)->targetPitch);
    AsCPetBody(self)->animationFinished = 1;
    AsCPetBody(self)->resourcePending = 0;
    AsCPetBody(self)->unk_1B0 = 0; // Reset here and after model load; broader role unresolved.
    AsCPetBody(self)->unk_1A8 = 0; // One-byte state; no meaning inferred from zero alone.
}
#pragma pop

// Initialize pet storage, parse model/shadow tokens, and prepare the optional puppet.
// arg is a borrowed token string used only during construction; mode is forwarded to the
// resource loader. The new pet retains the returned puppet and owns its instance containers.
CPetBody::CPetBody(const char* modelCommand, int mode) {
    // Phase 1: initialize the owned instance table under its mutex, then empty attachments.
    char* self = (char*)this;
    int zero = 0;
    (*(int*)&AsCPetBody(self)->flags) = zero;
    *(void**)self = __vt__8CPetBody;
    AsCPetBody(self)->instanceHashBuckets = 0;
    OSInitMutex((OSMutex*)(self + 0xEC));
    char* lock = self + 0xEC;
    OSLockMutex((OSMutex*)lock);
    AsCPetBody(self)->instanceHead = 0;
    AsCPetBody(self)->instanceFreeNodeHead = 0;
    AsCPetBody(self)->instanceNodeAllocationHead = 0;
    AsCPetBody(self)->unk_DC = kPetInstanceWordDCInitialValue; // Instance state word; exact policy unresolved.
    AsCPetBody(self)->instanceIterator = 0;
    AsCPetBody(self)->instanceHashBucketCount = zero;
    if (AsCPetBody(self)->instanceHashBucketCount > 0) {
        (*(void**)&AsCPetBody(self)->instanceHashBuckets) = (void*)appMallocUni__FUl(0);
        AsCPetBody(self)->instanceHashMask = AsCPetBody(self)->instanceHashBucketCount - 1;
        int bucketIndex = 0;
        int bucketByteOffset = 0;
        while (bucketIndex < AsCPetBody(self)->instanceHashBucketCount) {
            *(int*)((*(char**)&AsCPetBody(self)->instanceHashBuckets) + bucketByteOffset) = zero;
            bucketIndex++;
            bucketByteOffset += 4;
        }
    } else {
        AsCPetBody(self)->instanceHashBuckets = 0;
    }
    OSUnlockMutex((OSMutex*)lock);
    float zeroLightingDirection = kFloat_0_petBody;
    (*(int*)&AsCPetBody(self)->renderParameters) = zero;
    AsCPetBody(self)->renderParameterCount = zero;
    AsCPetBody(self)->unk_10C = zero; // Adjacent to render parameters; capacity is unconfirmed.
    AsCPetBody(self)->lightingMode = zero;
    AsCPetBody(self)->lightColor = zero;
    AsCPetBody(self)->ambientLightColor = zero;
    char* attachmentSentinel = (char*)&AsCPetBody(self)->attachmentTail;
    AsCPetBody(self)->unk_12C = kPetInitialWord12C; // RGB-shaped initial word; consumer unresolved.
    AsCPetBody(self)->lightDirectionZ = zeroLightingDirection;
    AsCPetBody(self)->lightDirectionY = zeroLightingDirection;
    AsCPetBody(self)->lightDirectionX = zeroLightingDirection;
    AsCPetBody(self)->nameStringFlags = zero;
    AsCPetBody(self)->nameStringLength = zero;
    AsCPetBody(self)->nameStringData = zero;
    (*(int*)&AsCPetBody(self)->attachmentCount) = zero;
    AsCPetBody(self)->attachmentHead = attachmentSentinel;
    AsCPetBody(self)->attachmentTail = attachmentSentinel;
    AsCPetBody(self)->unk_1A8 = zero;
    // Phase 2: initialize playback defaults, then parse the optional model token.
    CPetBody::InitDefaults(self);
    int parsedShadowMode;
    // ScanTokens receives the address of a borrowed string-pointer word.
    int commandStringWord = (int)modelCommand;
    parsedShadowMode = zero;
    char path[0x100];
    if (String_ScanTokensAfterSubstring((const char*)&commandStringWord, petBodyStrings,
                                        ((const PetBodyStringPool*)petBodyStrings)->stringFormat, path) != 0) {
        void* loadedPuppet = ResourceManager_LoadByExtension(g_resourceManager, path, 0, 1, mode);
        float unitScale = kFloat_1_petBody;
        AsCPetBody(self)->puppet = loadedPuppet;
        AsCPetBody(self)->unk_1B0 = zero;
        AsCPetBody(self)->uniformScale = unitScale;
        // Phase 2b: scale the puppet basis, then subtract its bone-zero offset from pet position.
        struct WVector boneAdjustment;
        struct Arr3 zeroAdjustment;
        float basisScale[3];
        basisScale[0] = unitScale;
        basisScale[1] = unitScale;
        basisScale[2] = unitScale;
        Matrix_SetBasisScale(AsCPetBody(self)->matrixBasis, basisScale);
        if (AsCPetBody(self)->puppet != 0) {
            Matrix_SetBasisScale(((WPuppet*)AsCPetBody(self)->puppet)->worldMatrix, basisScale);
        }
        float z = kFloat_0_petBody;
        zeroAdjustment.v[0] = z;
        zeroAdjustment.v[1] = z;
        zeroAdjustment.v[2] = z;
        *(struct Arr3*)&boneAdjustment = *(struct Arr3*)&zeroAdjustment;
        void* puppet = AsCPetBody(self)->puppet;
        if (puppet != 0) {
            WVector scaledBoneOffset;
            WPuppet_GetPivot(&scaledBoneOffset, puppet, 0);
            scaledBoneOffset += boneAdjustment;
            scaledBoneOffset *= AsCPetBody(self)->uniformScale;
            (*(WVector*)&AsCPetBody(self)->positionX) -= scaledBoneOffset;
        }
    }
    // Phase 3: independently parse the optional integer shadow mode.
    if (String_ScanTokensAfterSubstring(
            (const char*)&commandStringWord, ((const PetBodyStringPool*)petBodyStrings)->shadowToken,
            ((const PetBodyStringPool*)petBodyStrings)->integerFormat, (const char*)&parsedShadowMode) != 0) {
        AsCPetBody(self)->shadowMode = parsedShadowMode;
    }
}

} // extern "C"

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU80462184", _ctors_u801B7D5C_0, PetParticles_InitAngleCache);
#pragma pop

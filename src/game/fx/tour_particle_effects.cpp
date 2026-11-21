#include <decomp/static_init.h>
// Translation unit 0x80333CE8-0x80333FFC (6 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

// Same trail/burst layout used by the emitter methods in tour_load_effect_particles.cpp.
struct ParticlePointerVector {
    void** data;                        // owning particle pointer slots
    int count;
    int capacity;
};
struct TourParticleEmitter {
    void* textures;
    int textureStorageState;
    float trailSpread;
    int trailDurationFrames;
    unsigned char trailParticlesPerFrame;
    char reserved11[7];
    ParticlePointerVector trailParticles;
    float trailStart[2];
    float trailEnd[2];
    int trailFrame;
    int trailEnabled;
    char reserved3C[4];
    ParticlePointerVector burstParticles;
    float burstOrigin[2];
    int burstFrame;
    int burstEnabled;
};
enum { kDefaultTrailDurationFrames = 40, kParticlePointerBytes = sizeof(void*) };
typedef char TourParticleEmitter_size[(sizeof(TourParticleEmitter) == 0x5c) ? 1 : -1];

extern "C" {

void appFreeUni__FPv(void*);
void __dl__FPv(void*);
void* memmove(void*, const void*, unsigned int);
void __destroy_new_array(void*, void*);
void CBpTex_dtor();

// Release a trail-particle pointer vector buffer; individual particles are released by the emitter.
void* dtor_80333CE8(void* self, int shouldDelete) {
    if (self != 0) {
        ((ParticlePointerVector*)self)->count = 0;
        if (self != 0) {
            if (((ParticlePointerVector*)self)->data != 0) {
                ((ParticlePointerVector*)self)->count = 0;
                appFreeUni__FPv(((ParticlePointerVector*)self)->data);
            }
        }
        if (shouldDelete > 0)
            __dl__FPv(self);
    }
    return self;
}

// Release a burst-particle pointer vector buffer; individual particles are released by the emitter.
void* dtor_80333D5C(void* self, int shouldDelete) {
    if (self != 0) {
        ((ParticlePointerVector*)self)->count = 0;
        if (self != 0) {
            if (((ParticlePointerVector*)self)->data != 0) {
                ((ParticlePointerVector*)self)->count = 0;
                appFreeUni__FPv(((ParticlePointerVector*)self)->data);
            }
        }
        if (shouldDelete > 0)
            __dl__FPv(self);
    }
    return self;
}

// Destroy the texture array, delete both particle lists, and release their backing buffers.
void* TourParticleEffects_Dtor(void* self, int shouldDelete) {
    if (self != 0) {
        TourParticleEmitter* emitter = (TourParticleEmitter*)self;
        void* textureArray = emitter->textures;
        if (textureArray != 0) {
            __destroy_new_array(textureArray, (void*)CBpTex_dtor);
            emitter->textures = 0;
        }
        void** firstSlot = emitter->trailParticles.data;
        while (firstSlot != emitter->trailParticles.data + emitter->trailParticles.count) {
            if (*firstSlot != 0)
                __dl__FPv(*firstSlot);
            unsigned int count = *(unsigned int*)&emitter->trailParticles.count;
            unsigned int remainingSlots = (unsigned int)((emitter->trailParticles.data + count - firstSlot)) - 1;
            memmove(firstSlot, firstSlot + 1, remainingSlots * kParticlePointerBytes);
            --emitter->trailParticles.count;
        }
        firstSlot = emitter->burstParticles.data;
        while (firstSlot != emitter->burstParticles.count + emitter->burstParticles.data) {
            if (*firstSlot != 0)
                __dl__FPv(*firstSlot);
            unsigned int count = *(unsigned int*)&emitter->burstParticles.count;
            unsigned int remainingSlots = (unsigned int)((emitter->burstParticles.data + count - firstSlot)) - 1;
            memmove(firstSlot, firstSlot + 1, remainingSlots * kParticlePointerBytes);
            --emitter->burstParticles.count;
        }
        // These nested guards retain the original inlined container destructor branches.
        // Collapsing them removes 48 bytes from this unit with the matching compiler.
        if (&emitter->burstParticles != 0) {
            if (&emitter->burstParticles != 0) {
                if (&emitter->burstParticles != 0) {
                    if (&emitter->burstParticles != 0) {
                        if (&emitter->burstParticles != 0) {
                            if (emitter->burstParticles.data != 0) {
                                int oldCount = *(volatile int*)&emitter->burstParticles.count;
                                *(volatile int*)&emitter->burstParticles.count = oldCount - *(volatile int*)&oldCount;
                                appFreeUni__FPv(emitter->burstParticles.data);
                            }
                        }
                    }
                }
            }
        }
        if (&emitter->trailParticles != 0) {
            if (&emitter->trailParticles != 0) {
                if (&emitter->trailParticles != 0) {
                    if (&emitter->trailParticles != 0) {
                        if (&emitter->trailParticles != 0) {
                            if (emitter->trailParticles.data != 0) {
                                int oldCount = *(volatile int*)&emitter->trailParticles.count;
                                *(volatile int*)&emitter->trailParticles.count = oldCount - *(volatile int*)&oldCount;
                                appFreeUni__FPv(emitter->trailParticles.data);
                            }
                        }
                    }
                }
            }
        }
        if (shouldDelete > 0)
            __dl__FPv(self);
    }
    return self;
}

extern const float kFloat_6_2831855;
extern const float kFloat_0_805E1620;
extern const float kFloat_20_TourTrailSpread;
extern float s_tourParticleFullTurnAngle;

// Empty lifecycle hook retained for its original entry point.
void TourParticleEffects_EmptyHook() {}

// Initialize the tour particle full-turn angle cache.
void TourParticleEffects_InitFullTurnAngle() { s_tourParticleFullTurnAngle = kFloat_6_2831855; }

// Initialize empty trail/burst emitters with the default spread, duration, and emission count.
void TourParticleEffects_Init(void* self) {
    TourParticleEmitter* emitter = (TourParticleEmitter*)self;
    const float defaultTrailSpread = kFloat_20_TourTrailSpread;
    const float zero = kFloat_0_805E1620;
    emitter->trailParticles.data = 0;
    emitter->trailParticles.capacity = 0;
    emitter->burstParticles.data = 0;
    emitter->burstParticles.capacity = 0;
    *(int*)&emitter->textures = 0;
    emitter->textureStorageState = 0;
    emitter->trailParticles.count = 0;
    emitter->burstParticles.count = 0;
    emitter->trailStart[0] = zero;
    emitter->trailStart[1] = zero;
    emitter->trailEnd[0] = zero;
    emitter->trailEnd[1] = zero;
    emitter->burstOrigin[0] = zero;
    emitter->burstOrigin[1] = zero;
    emitter->trailSpread = defaultTrailSpread;
    emitter->trailDurationFrames = kDefaultTrailDurationFrames;
    emitter->trailFrame = 0;
    emitter->burstFrame = 0;
    emitter->trailParticlesPerFrame = 1;
    emitter->trailEnabled = 0;
    emitter->burstEnabled = 0;
}

} // extern "C"

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU80462640", _ctors_u80333CE8_0, TourParticleEffects_InitFullTurnAngle);
#pragma pop

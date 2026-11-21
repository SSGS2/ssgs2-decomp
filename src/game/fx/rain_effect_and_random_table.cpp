#include <decomp/static_init.h>
#include "classes/WView.h"
#define AsWView(p) ((WView*)(p))
#include "game/types.h"
#include "sdk_protos.h"
#include "classes/CRain.h"

#include "classes/RandomTable.h"
#include <decomp/cw_bitfields.h>
#define AsCRain(p) ((CRain*)(p))
// Translation unit 0x801DB98C-0x801DD078 (19 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

static const int kRainDropCount = 500;
static const int kMtStateWords = 624;
static const int kMtUnseededIndex = 625;
static const unsigned int kMtDefaultSeed = 0x1105;
// MT19937 recurrence: M=397, N-M=227; the last word wraps to word zero.
enum { kMtTwistOffset = 397, kMtWrapOffset = 227, kMtLastWord = 623 };
// Each drop is two XYZ/RGBA vertices (16 bytes each); the overlay is four vertices.
enum { kRainVertexBytes = 16, kRainSegmentBytes = 32, kRainOverlayVertices = 4 };

// Coordinates below are in game world units; the camera stores three basis
// vectors followed by its world translation. Rain direction also stores line length.
// timeStep is forwarded from the effect update; its conversion to seconds is unresolved.
// Deleting destructors release owned allocations; deleting > 0 also frees self.

extern "C" {

extern const float kFloat_6_2831855;
extern float s_quadTreeCollisionFullTurnAngle;
extern float s_initializedTwoPi_801DC648;
extern float s_randomHouseHelperFullTurnAngle;
extern const float kFloat_0_34906584_QuadTreeCollision, kFloat_0_7853982_QuadTreeCollision, kFloat_1_QuadTree;
extern float kFloat_0_Rain, kFloat_neg20_Rain, kFloat_neg0_Rain, kFloat_15_Rain;
extern float s_quadTreeCollisionState[4];
extern float s_quadTreeUnitScaleVector[3];

extern void BaseObject_dtor(void*, int);
extern void BaseObject_ctor(void*);
extern void appFreeUni__FPv(void*);
extern void* __nwa__FUl(unsigned int);
extern void __dl__FPv(void*);
extern void __dla__FPv(void*);
extern float Math_CosApprox(float);
extern float Math_SinApprox(float);
extern void Vec3_Normalize(float*, float*);
extern void Vec3_Scale(float*, float*, float);
extern void* CWind_GetInstance(void*);
extern void CWind_GetWindVector(float*, void*);
extern int g_randomHouse;
extern char s_randomHouseBaseVtable[16];
extern char __vt__12CRandomHouse[16];
extern char __vt__5CRain[16];
extern unsigned int kRainRenderStateDefaults[24];
extern unsigned int kRainTextureStageStateDefaults[12];
extern unsigned int kRainTextureStageParameterDefaults[10];
extern char kIdentityMatrix4x4[64];
extern char s_cameraTransform[48];
extern int g_view;
extern float kFloat_160_Rain, kFloat_320_Rain, kFloat_neg1_Rain;
extern float kFloat_1_Rain;
extern unsigned int mt19937TwistXorTable[2];
extern void CubeResourceStorage_dtor(void*, int);
extern void Vec3_Scale(float*, float*, float);
extern void Vec3_Add(float*, float*, float*);
extern void Vector_CopyComponents(float*, float*);
extern void Engine_SetRenderStateList(void*, int);
extern void Engine_SetTextureStageStateList(int, void*, int);
extern void Engine_SetTextureStageParameterList(int, void*, int);
extern void Engine_SetType7Resource(int);
extern void Engine_SetTextureStageResource(int, int);
extern void SetTransform(int, void*);
extern void Engine_SetFlexibleVertexFormat(int);
extern void Engine_SetVertexStream(int, void*, int, int);
extern void DrawPrimitiveFromBoundBuffers(int, int, int);
extern void Matrix_ExpandAffineTo4x4(void*, void*);
extern void Resource_GetOffsetPtr(void*, int, int*, int, int*);
extern void GraphicsBuffer_Flush(void*);
extern void VertexBuffer_Create(int, int, int, int, void**, int);
extern void Resource_GetOffsetPtr(void*, int, int*, int, int*);
extern float Random_RangeFloat(int, float, float);

struct DtorObject {
    void* resource;
    volatile int state;
};

struct OrderedDtorObject {
    void* volatile resource;
    volatile int state;
};

// Clear the random-house singleton, destroy its base, and optionally free the object.
void* __dt__15RandomHouseBaseFv(void* self, int deleting) {
    if (self != 0) {
        g_randomHouse = 0;
        BaseObject_dtor(self, 0);
        if (deleting > 0)
            __dl__FPv(self);
    }
    return self;
}

// Reset the buffer count and free its storage, then optionally delete the buffer owner.
void* dtor_801DB98C(DtorObject* self, int deleting) {
    if (self != 0) {
        if (self->resource != 0) {
            int state = self->state;
            self->state = state - state;
            appFreeUni__FPv(self->resource);
        }
        if (deleting > 0)
            __dl__FPv(self);
    }
    return self;
}

// Reset the buffer count and free its storage, then optionally delete the buffer owner.
void* dtor_801DB9FC(DtorObject* self, int deleting) {
    if (self != 0) {
        if (self->resource != 0) {
            int state = self->state;
            self->state = state - state;
            appFreeUni__FPv(self->resource);
        }
        if (deleting > 0)
            __dl__FPv(self);
    }
    return self;
}

// Clear the volatile count, release storage, and optionally delete the owner.
void* dtor_801DBA6C(OrderedDtorObject* self, int deleting) {
    if (self != 0) {
        self->state = 0;
        if (self != 0) {
            void* resource = self->resource;
            if (resource != 0) {
                self->state = 0;
                appFreeUni__FPv(resource);
            }
        }
        if (deleting > 0)
            __dl__FPv(self);
    }
    return self;
}

// Clear the volatile count, release storage, and optionally delete the owner.
void* dtor_801DBAE0(OrderedDtorObject* self, int deleting) {
    if (self != 0) {
        self->state = 0;
        if (self != 0) {
            void* resource = self->resource;
            if (resource != 0) {
                self->state = 0;
                appFreeUni__FPv(resource);
            }
        }
        if (deleting > 0)
            __dl__FPv(self);
    }
    return self;
}

struct RandomHouseBase {
    char storage[0x10];
    // Construct the base and publish this object as the random-house singleton.
    RandomHouseBase() {
        BaseObject_ctor(this);
        *(void**)this = s_randomHouseBaseVtable;
        g_randomHouse = (int)this;
    }
    ~RandomHouseBase();
};
struct CRandomHouse : RandomHouseBase {
    char rest[0xC];
    CRandomHouse();
};
// Allocate the fixed 624-word state; the first draw performs lazy seeding.
CRandomHouse::CRandomHouse() {
    void* self = this;
    void* second_vtable = __vt__12CRandomHouse;
    *(void**)self = second_vtable;
    AsRandomTable(self)->data = 0;
    AsRandomTable(self)->nextWordIndex = kMtUnseededIndex;
    AsRandomTable(self)->seed = kMtDefaultSeed;
    AsRandomTable(self)->data = (unsigned int*)__nwa__FUl(kMtStateWords * sizeof(unsigned int));
}

// Initialize the rain helper's full-turn angle cache.
void InitRainHelperFullTurnAngle() {
    s_initializedTwoPi_801DC648 = kFloat_6_2831855;
}

// Cache collision-angle trigonometry and reset the quadtree unit scale.
void QuadTree_InitializeCollisionConstants() {
    s_quadTreeCollisionFullTurnAngle = kFloat_6_2831855;
    float r0 = Math_CosApprox(kFloat_0_34906584_QuadTreeCollision);
    float* p = s_quadTreeCollisionState;
    p[0] = r0;
    p[1] = Math_CosApprox(kFloat_0_7853982_QuadTreeCollision);
    p[2] = Math_SinApprox(kFloat_0_34906584_QuadTreeCollision);
    ((int*)p)[3] = 0;
    s_quadTreeUnitScaleVector[0] = kFloat_1_QuadTree;
    s_quadTreeUnitScaleVector[1] = kFloat_1_QuadTree;
    s_quadTreeUnitScaleVector[2] = kFloat_1_QuadTree;
}

struct RefCount_801DBF84 {
    CW_BITS2(
        unsigned int refs : 16,
        unsigned int other : 16)
};

// Release the rain and overlay vertex buffers, then optionally delete the effect.
void* CRain_dtor(void* self, int deleting) {
    if (self != 0) {
        void** p = (void**)self;
        RefCount_801DBF84* first = (RefCount_801DBF84*)p[8];
        *(void**)self = __vt__5CRain;
        if (first != 0) {
            first->refs--;
            if (first->refs == 0)
                CubeResourceStorage_dtor(first, 1);
            p[8] = 0;
        }
        RefCount_801DBF84* second = (RefCount_801DBF84*)p[9];
        if (second != 0) {
            second->refs--;
            if (second->refs == 0)
                CubeResourceStorage_dtor(second, 1);
            p[9] = 0;
        }
        if (deleting > 0)
            __dl__FPv(self);
    }
    return self;
}

// Compute the axis-aligned rain volume from the current camera transform.
void Rain_ComputeCameraVolumeBounds(void* /*unused*/, float* min, float* max) {
    // Build eight camera-relative corners: +/-160 on the first two bases,
    // with a second plane 320 units along the third basis.
    float corners[24];
    float scale = kFloat_neg1_Rain;
    float* rowDst = corners;
    float* rowSrc = rowDst;
    for (int i = 0; i < 4; ++i) {
        float y = kFloat_160_Rain;
        float x = y;
        if (i & 1) x *= scale;
        if (i & 2) y *= scale;
        float a[3], b[3];
        Vec3_Scale(a, (float*)s_cameraTransform, x);
        Vec3_Scale(b, (float*)(s_cameraTransform + 12), y);
        Vec3_Add(rowDst, a, b);
        Vec3_Scale(b, (float*)(s_cameraTransform + 24), kFloat_320_Rain);
        Vec3_Add(&corners[i * 3 + 12], rowSrc, b);
        rowSrc += 3;
        rowDst += 3;
    }
    // Reduce the corners to an AABB before adding the camera's world translation.
    Vector_CopyComponents(min, corners);
    Vector_CopyComponents(max, corners);
    float* p = corners + 3;
    for (int k = 0; k < 7; ++k) {
        if (min[0] > p[0]) min[0] = p[0];
        if (min[1] > p[1]) min[1] = p[1];
        if (min[2] > p[2]) min[2] = p[2];
        if (max[0] < p[0]) max[0] = p[0];
        if (max[1] < p[1]) max[1] = p[1];
        if (max[2] < p[2]) max[2] = p[2];
        p += 3;
    }
    Vec3_Add(min, min, (float*)(s_cameraTransform + 36));
    Vec3_Add(max, max, (float*)(s_cameraTransform + 36));
}

#pragma push
#pragma use_lmw_stmw on
class RenderDevice801DC28C {
public:
    virtual void slot00();
    virtual void slot01();
    virtual void slot02();
    virtual void slot03();
    virtual void slot04();
    virtual void slot05();
    virtual void slot06();
    virtual void slot07();
    virtual void slot08();
    virtual void slot09();
    virtual void slot10();
    virtual void slot11();
    virtual void slot12();
    virtual void slot13();
    virtual void slot14();
    virtual void slot15();
    virtual void slot16();
    virtual void slot17();
    virtual void slot18();
    virtual void slot19();
    virtual void slot20();
    virtual void slot21();
    virtual void slot22();
    virtual void state64(int);
    virtual void state68(int);
};
// Bind rain render state and vertex buffers, then draw the 500 rain segments.
void CRain_DrawLineSegments(void* self) {
    // Install the fixed rain material, then the view transform and XYZ/RGBA stream.
    struct Defaults { unsigned int words[24]; };
    struct StateA { unsigned int words[12]; };
    struct StateB { unsigned int words[10]; };
    Defaults defaults = *(Defaults*)kRainRenderStateDefaults;
    float transform[16];
    StateB stateB;
    StateA stateA;
    stateA = *(StateA*)kRainTextureStageStateDefaults;
    stateB = *(StateB*)kRainTextureStageParameterDefaults;
    Engine_SetRenderStateList(&defaults, 12);
    Engine_SetTextureStageStateList(0, &stateA, 6);
    Engine_SetTextureStageParameterList(0, &stateB, 5);
    RenderDevice801DC28C* device = (RenderDevice801DC28C*)AsWView(g_view)->renderTarget;
    if (device != 0) {
        device->state64(0x20200000);
        device->state68(0x00800000);
    }
    SetTransform(2, kIdentityMatrix4x4);
    SetTransform(11, kIdentityMatrix4x4);
    Engine_SetType7Resource(0);
    Engine_SetTextureStageResource(0, 0);
    Matrix_ExpandAffineTo4x4(((char*)&AsWView(g_view)->inputTransform), transform);
    SetTransform(0, transform);
    Engine_SetFlexibleVertexFormat(0xF001);
    Engine_SetVertexStream(0, AsCRain(self)->vertexBuffer, 0, kRainVertexBytes);
    DrawPrimitiveFromBoundBuffers(0xA8, 0, kRainDropCount);
}
#pragma pop

// RandomTable +0x10 owns 624 words; +0x14 (nextWordIndex) is the NEXT
// WORD INDEX, not allocation size. 624 requests a twist; 625 requests lazy seeding.
// +0x18 is the saved seed. Seeding and drawing mutate this state without a lock.

// Generate the next tempered MT19937 word, refreshing the 624-word state when exhausted.
unsigned int MT19937_NextU32(RandomTable* self) {
    if (self->nextWordIndex >= kMtStateWords) {
        if (self->nextWordIndex == kMtUnseededIndex) {
            // Legacy seeding joins two high halfwords from successive LCG steps.
            unsigned int seed = self->seed;
            for (int i = 0; i < kMtStateWords; ++i) {
                self->data[i] = seed & 0xFFFF0000;
                seed = seed * 0x10DCD + 1;
                self->data[i] |= (seed & 0xFFFF0000) >> 16;
                seed = seed * 0x10DCD + 1;
            }
            self->nextWordIndex = kMtStateWords;
        }
        int i;
        unsigned int y;
        // Twist in place: first the nonwrapping range, then the wrapped range.
        for (i = 0; i < kMtWrapOffset; ++i) {
            y = (self->data[i] & 0x80000000) | (self->data[i + 1] & 0x7FFFFFFF);
            self->data[i] = self->data[i + kMtTwistOffset] ^ (y >> 1) ^ mt19937TwistXorTable[y & 1];
        }
        for (; i < kMtLastWord; ++i) {
            y = (self->data[i] & 0x80000000) | (self->data[i + 1] & 0x7FFFFFFF);
            self->data[i] = self->data[i - kMtWrapOffset] ^ (y >> 1) ^ mt19937TwistXorTable[y & 1];
        }
        y = (self->data[623] & 0x80000000) | (self->data[0] & 0x7FFFFFFF);
        self->data[623] = self->data[396] ^ (y >> 1) ^ mt19937TwistXorTable[y & 1];
        self->nextWordIndex = 0;
    }
    // Temper one word and advance the cursor; the stored recurrence stays untempered.
    unsigned int value = self->data[self->nextWordIndex++];
    value ^= value >> 11;
    value ^= (value << 7) & 0x9D2C5680;
    value ^= (value << 15) & 0xEFC60000;
    value ^= value >> 18;
    return value;
}

// Fill MT19937 state with the legacy 69069 generator; seed zero reuses the saved seed.
void MT19937_Seed(RandomTable* self, unsigned int seed) {
    if (seed == 0) seed = self->seed;
    else self->seed = seed;
    for (int i = 0; i < kMtStateWords; ++i) {
        self->data[i] = seed & 0xFFFF0000;
        seed = seed * 69069 + 1;
        self->data[i] |= (seed & 0xFFFF0000) >> 16;
        seed = seed * 69069 + 1;
    }
    self->nextWordIndex = kMtStateWords;
}

typedef Vec3f Vec3;
struct Bounds { Vec3 min, max; };

class RainVirtual {
public:
    virtual void slot0();
    virtual void slot1();
    virtual void slot2();
    virtual void slot3();
};
struct Obj801DC054 { void* vt; char pad[0x10]; float dropDirection[3]; void* particles; };
#pragma opt_propagation off
// Move rain line segments through the camera volume, wrapping each coordinate at its bounds.
void CRain_UpdateDropVertices(Obj801DC054* object, float timeStep) {
    // Refresh wind direction and camera bounds before borrowing writable buffer data.
    int firstVertexByteOffset;
    int dropIndex;
    Obj801DC054* self;
    int secondVertexByteOffset;
    self = object;
    ((RainVirtual*)self)->slot3();
    float minBound[3];
    float maxBound[3];
    Rain_ComputeCameraVolumeBounds(self, minBound, maxBound);

    float stepVector[3];
    float step = (kFloat_15_Rain * timeStep); Vec3_Scale(stepVector, self->dropDirection, step);
    float* vertices;
    Resource_GetOffsetPtr(self->particles, 0, 0, (int)&vertices, 0);
    float wrapWidth = kFloat_320_Rain;
    dropIndex = 0;
    firstVertexByteOffset = 0;
    secondVertexByteOffset = kRainVertexBytes;
    // Advance/wrap the first endpoint independently on XYZ. The second endpoint
    // is rebuilt from it, so the segment keeps its length across a wrap.
    for (; dropIndex < kRainDropCount; ++dropIndex) {
        float* firstVertexPosition = (float*)((char*)vertices + firstVertexByteOffset);
        Vec3_Add(firstVertexPosition, firstVertexPosition, stepVector);
        float coordinate = ((float*)((char*)vertices + firstVertexByteOffset))[0];
        if (coordinate > maxBound[0]) {
            float wrapCount = ceil((coordinate - maxBound[0]) / wrapWidth);
            ((float*)((char*)vertices + firstVertexByteOffset))[0] -= wrapWidth * wrapCount;
        } else if (coordinate < minBound[0]) {
            float wrapCount = ceil((maxBound[0] - coordinate) / wrapWidth);
            ((float*)((char*)vertices + firstVertexByteOffset))[0] += wrapWidth * wrapCount;
        }
        coordinate = ((float*)((char*)vertices + firstVertexByteOffset))[1];
        if (coordinate > maxBound[1]) {
            float wrapCount = ceil((coordinate - maxBound[1]) / wrapWidth);
            ((float*)((char*)vertices + firstVertexByteOffset))[1] -= wrapWidth * wrapCount;
        } else if (coordinate < minBound[1]) {
            float wrapCount = ceil((maxBound[1] - coordinate) / wrapWidth);
            ((float*)((char*)vertices + firstVertexByteOffset))[1] += wrapWidth * wrapCount;
        }
        coordinate = ((float*)((char*)vertices + firstVertexByteOffset))[2];
        if (coordinate > maxBound[2]) {
            float wrapCount = ceil((coordinate - maxBound[2]) / wrapWidth);
            ((float*)((char*)vertices + firstVertexByteOffset))[2] -= wrapWidth * wrapCount;
        } else if (coordinate < minBound[2]) {
            float wrapCount = ceil((maxBound[2] - coordinate) / wrapWidth);
            ((float*)((char*)vertices + firstVertexByteOffset))[2] += wrapWidth * wrapCount;
        }
        float* secondVertexPosition = (float*)((char*)vertices + secondVertexByteOffset);
        Vec3_Add(secondVertexPosition, (float*)((char*)vertices + firstVertexByteOffset), self->dropDirection);
        secondVertexByteOffset += kRainSegmentBytes;
        firstVertexByteOffset += kRainSegmentBytes;
    }
    GraphicsBuffer_Flush(self->particles);
}
#pragma opt_propagation reset

// Combine downward rain motion with wind, normalize it, and scale the drop direction.
void CRain_UpdateDropDirection(float* self) {
    float wind[3];
    CWind_GetWindVector(wind, CWind_GetInstance(self));
    float downwardZ;
    float downwardY;
    float downwardX;
    downwardX = kFloat_0_Rain;
    downwardY = kFloat_neg20_Rain;
    downwardZ = kFloat_neg0_Rain;
    self[5] = downwardX;
    self[6] = downwardY;
    self[7] = downwardZ;
    self[5] = downwardX + wind[0];
    self[6] = downwardY + wind[1];
    self[7] = downwardZ + wind[2];
    Vec3_Normalize(self + 5, self + 5);
    Vec3_Scale(self + 5, self + 5, kFloat_15_Rain);
}

struct Particle2D { float x, y, z; unsigned char pad[8]; unsigned char a, b, c, d; };
struct ParticleSystem;
struct ParticleVtable { void (*slots[6])(ParticleSystem*); };
struct ParticleVertex { float x, y, z; unsigned char r, g, b, a; };
struct ParticlePair { ParticleVertex first, second; };
class ParticleVirtual {
public:
    virtual void slot0();
    virtual void slot1();
    virtual void slot2();
    virtual void slot3();
};
struct ParticleSystem { ParticleVtable* vtable; float scale; char pad[0xC]; float direction[3]; void* particles; void* small; };
struct ParticleRefs { CW_BITS2( unsigned refs:16, unsigned other:16) };

// Build 500 translucent rain segments and the four-vertex screen overlay.
ParticleSystem* CRain_ctor(ParticleSystem* self) {
    // Initialize ownership slots and derive the world-space line direction.
    self->scale = kFloat_1_Rain;
    self->vtable = (ParticleVtable*)__vt__5CRain;
    self->particles = 0;
    self->small = 0;
    ((ParticleVirtual*)self)->slot3();

    if (self->particles != 0) {
        ParticleRefs* count = (ParticleRefs*)self->particles;
        count->refs--;
        if (count->refs == 0) CubeResourceStorage_dtor(count, 1);
        self->particles = 0;
    }
    // Allocate 500 line pairs, scatter their starts around the camera, and fade
    // alpha from 0 at the first endpoint to 0x50 at the second.
    VertexBuffer_Create(kRainDropCount * kRainSegmentBytes, 0, 0xF001, 0, &self->particles, 0);
    ParticleVertex* vertices;
    Resource_GetOffsetPtr(self->particles, 0, 0, (int)&vertices, 0);
    register int first = 0;
    for (int i = 0; i < kRainDropCount; ++i, first += 2) {
        int second = i * 2 + 1;
        vertices[first].x = ((float*)s_cameraTransform)[9] + Random_RangeFloat(-1, -160.0f, 160.0f);
        vertices[first].y = ((float*)s_cameraTransform)[10] + Random_RangeFloat(-1, -160.0f, 160.0f);
        vertices[first].z = ((float*)s_cameraTransform)[11] + Random_RangeFloat(-1, -160.0f, 160.0f);
        Vec3_Add(&vertices[second].x, &vertices[first].x, self->direction);
        vertices[first].a = 0;
        vertices[first + 1].a = 0x50;
        vertices[first + 1].b = 0xE0;
        vertices[first + 1].g = 0xE0;
        vertices[first + 1].r = 0xE0;
        vertices[first].b = 0xE0;
        vertices[first].g = 0xE0;
        vertices[first].r = 0xE0;
    }
    GraphicsBuffer_Flush(self->particles);
    // Build a separate screen-space overlay in viewport pixels (z=0.05).
    if (self->small != 0) {
        ParticleRefs* count = (ParticleRefs*)self->small;
        count->refs--;
        if (count->refs == 0) CubeResourceStorage_dtor(count, 1);
        self->small = 0;
    }
    VertexBuffer_Create(kRainOverlayVertices * kRainVertexBytes, 0, 0xF005, 0, &self->small, 0);
    ParticleVertex* smallVertices;
    Resource_GetOffsetPtr(self->small, 0, 0, (int)&smallVertices, 0);
    for (int i = 0; i < kRainOverlayVertices; ++i) {
        float x, y;
        float width = ((float*)(((char*)&AsWView(g_view)->viewportWidth)))[0];
        x = (float)(i & 1);
        width = x * width;
        y = (float)(i >> 1);
        smallVertices[i].x = width;
        float height = ((float*)(((char*)&AsWView(g_view)->viewportHeight)))[0];
        smallVertices[i].y = y * height;
        smallVertices[i].z = 0.05f;
        smallVertices[i].a = 0x60;
        smallVertices[i].r = 0x30;
        smallVertices[i].g = 0x30;
        smallVertices[i].b = 0x30;
    }
    GraphicsBuffer_Flush(self->small);
    return self;
}

// Release the MT19937 state array and clear the singleton before base destruction.
void* CRandomHouse_dtor(void* self, int deleting) {
    if (self != 0) {
        void** p = (void**)self;
        void* resource = p[4];
        *p = __vt__12CRandomHouse;
        if (resource != 0) {
            __dla__FPv(resource);
            p[4] = 0;
        }
        if (self != 0) {
            g_randomHouse = 0;
            BaseObject_dtor(self, 0);
        }
        if (deleting > 0)
            __dl__FPv(self);
    }
    return self;
}

// Initialize this random-house helper's full-turn angle cache.
void InitRandomHouseHelperFullTurnAngle() {
    s_randomHouseHelperFullTurnAngle = kFloat_6_2831855;
}

// Disabled computer-shot logging entry point; accepts arguments without output.
void ComShot_Log(int arg0, ...) {
}

} // extern "C"

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU80462198", _ctors_u801DB98C_0, QuadTree_InitializeCollisionConstants);
STATIC_INIT_ENTRY(".CRT$XCU80462198", _ctors_u801DB98C_1, InitRainHelperFullTurnAngle);
STATIC_INIT_ENTRY(".CRT$XCU80462198", _ctors_u801DB98C_2, InitRandomHouseHelperFullTurnAngle);
#pragma pop

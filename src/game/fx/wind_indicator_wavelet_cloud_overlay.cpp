// Course wind and overlays: the wind singleton CWind with its local wind regions, the ground
// wavelet strips (CWavelet) that the wind indicator draws, and the translucent cloud tint (CCloud).

#include <decomp/static_init.h>
#include <decomp/ct_member.h>
// Course environment helpers: wind indicator initialization, ground wavelet
// animation/geometry, and a full-viewport cloud tint. These are three separate
// subsystems; none of their presentation constants changes physics wind directly.
// Modding lookup:
// - Wind HUD placement: CWind_Ctor indicatorViewport {514,354,130,126} pixels,
//   normalized depth [0,1], -pi/6 camera tilt, -170*(viewportWidth/640) distance.
//   CScreen (ingame_hud_screen.cpp) loads/releases the model and CWind::DrawDirectionModel
//   (wind_regions_and_vectors.cpp) consumes viewport/light/basis; keep its restore policy in mind.
// - Wind strength/direction for shots: CWind::SetWindParams (wind_system.cpp),
//   CWind::GetWindVectorAtPoint and region parser (wind_regions_and_vectors.cpp), CWind.h. Regions
//   are course world-space boxes, not indicator viewport or cloud settings.
// - Wavelet width/pace/texture: CWavelet_ConfigureFromBeachParameterName (beach_wavelet_ground_strip.cpp) reads the numeric
//   name suffix into CGolfDoc::beachParameterTreeCount's tree. The record holds
//   bitmapName, width and timingScale; configure them together. Update here is
//   CWavelet::UpdateLayers; draw CWavelet::DrawLayers consumes the resulting scale/alpha.
//   CGolfBg_OnLoad (golf_background_load.cpp) populates the keyed tree from
//   BackgroundText.waveParametersFormat with the selected course's mapName;
//   edit that course's text record, not the wind-region table, to tune a strip.
//   CSceneManager::CreateCourseObjects (course_scene_render_and_geometry.cpp), kEntryTextureStrip, constructs a
//   GeneTex3DAlloc and registers it in the scene's generated-texture pool +0x80.
//   GroundStripOwner_Init/destruction (beach_wavelet_ground_strip.cpp) owns point/layer/mesh arrays.
// - Cloud darkness: CCloud::DrawOverlay's 0x60303030 color and current g_view
//   viewport; the class's virtual function in slot 1 is empty. It is a screen tint, not weather physics.
// Constants: the data section (alpha, displacement, light), the data section
// (wave timing fractions), shared scene-effect pool and course environment data.
// Preserve shared-pool roles and floating evaluation order when adding settings.
#include "MSL/vector.h"
// Containers whose deleting destructors this unit keeps out of line under plain names. The element
// types are placeholders: the destructor depends only on whether elements are scalars or records.
struct MSLRecord {
    unsigned int words[1];
};
typedef std::vector<MSLRecord, app::app_allocator<MSLRecord> > MSLRecordVector;
#include "classes/WView.h"
#define AsWView(p) ((WView*)(p))
namespace exception_runtime {
extern "C" {
void __throw(const char*, void*, void*) __attribute__((noreturn));
void __dt__Q23std12length_errorFv(void*, int);
extern const char taskSequenceStrings[];
}
}

#include "game/types.h"
#include "game/wvector.h"

#include "sdk_protos.h"
#include "classes/CWavelet.h"
#include "classes/CWind.h"
#include "classes/CCloud.h"

#define AsCWavelet(p) ((CWavelet*)(p))

// Construct in caller-provided storage without allocating.
inline void* operator new(size_t, void* p) {
    return p;
}

extern "C" {

extern float kFloat_6_2831855;
extern float s_windFullTurnRadians;
extern float s_cloudFullTurnRadians;
extern unsigned char s_windInitialized;
extern void* g_view;
extern unsigned char s_windInstance[];
extern unsigned char s_windDtorRegistration[];
extern const float kFloat_0_CloudOverlay;
extern const float kFloat_4_Wavelet;
extern const float kFloat_0_1_Wavelet;
extern const float kFloat_0_5_Wavelet;
extern const float kFloat_1_SceneEffects_805DFDC0;
extern const float kFloat_0_805DFDA4;
extern const float kFloat_255_Wavelet;
extern const float kFloat_0_SceneEffects;
extern const float kFloat_1_SceneEffects;
extern const float kFloat_9_SceneEffects;
extern const float kFloat_neg1_SceneWind;
extern const float kFloat_neg0_5235988_Trajectory;
extern const float kFloat_neg170_WindAndTrajectory;
extern const float kFloat_640_TrajectoryViewport;
extern const float kFloat_0_5_SceneEffects;
extern const float kFloat_0_005122_SceneEffects;
extern const float kFloat_neg0_858835_Wind;
extern const float kFloat_0_512227_SceneEffects;
extern unsigned char __vt__5CWind[];
// CWind's deleting destructor, registered with the singleton.
extern "C" void* __dt__5CWindFv(CWind*, int);
// Cloud overlay color is an immediate value, not a data address.
struct Config72 {
    unsigned int words[18];
};
struct Config48 {
    unsigned int words[12];
};
struct Config40 {
    unsigned int words[10];
};
struct WaveletConfig {
    Config72 a;
    Config48 b;
    Config40 c;
};
extern const WaveletConfig waveletRenderDefaults; // Wavelet render, texture-stage, and sampler defaults.
extern unsigned char kIdentityMatrix4x4[];

extern void __dl__FPv(void*);
extern void appFreeUni__FPv(void*);
extern void Sprite_DrawColoredRect(void*, float*, int, unsigned int, float);
extern void Matrix_SetAxisRotation(void*, float, signed char);
extern void Matrix_CopyAffine(void*, void*);
extern void __ml__FfRC7WVector(void*, float*, float);
extern void __as__7WVectorFRC7WVector(void*, void*);
extern void* Normalize__7WVectorFv(float*);
extern void Vec3_Normalize(void*, void*);
extern void Engine_SetRenderStateList(void*, int);
extern void Engine_SetTextureStageStateList(int, void*, int);
extern void Engine_SetTextureStageParameterList(int, void*, int);
extern void Matrix_ExpandAffineTo4x4(void*, void*);
extern void SetTransform(int, void*);
extern void Engine_SetType7Resource(int);
extern void Engine_SetFlexibleVertexFormat(int);
extern void Engine_SetTextureStageResource(int, void*);
extern void Engine_SetTexturePalette(int, void*);
extern void Resource_GetOffsetPtr(void*, int, int, void*, int);
extern void GraphicsBuffer_Flush(void*);
extern void Engine_SetVertexStream(int, void*, int, int);
extern void DrawPrimitiveFromBoundBuffers(int, int, int);
extern void __mi__FRC7WVectorRC7WVector(void*, void*, void*);
extern void* CQuadTree_GetInstance();
extern void GetGroundPoint__9CQuadTreeF7WVectorbPib(void*, void*, void*, int, int, int);
extern void Vector_CopyComponents(void*, void*);
extern void* appMallocDummy__FUl(int);

struct DtorObject {
    void* resource;
    volatile int count;
};

struct Particle {
    float x;
    float delay;
    float y;
    float life;
    float vx;
    int state;
};

struct ParticleSystem {
    char pad00[0x1C];
    float speed;
    float maxLife0;
    float maxLife1;
    float rate;
    float spawnDelay;
    Particle* particles;
    unsigned int count;
};

struct MatrixLike {
    char bytes[0x40];
};
typedef Vec3f VectorLike;
struct ParticleEntry {
    unsigned int words[10];
    unsigned char flag;
    char pad[3];
    float a, b, c;
    unsigned int tail;
};
struct WindTriple {
    unsigned int x, y, z;
    // Copy three raw words, retaining the original volatile read ordering.
    WindTriple& operator=(const WindTriple& r) {
        x = r.x;
        unsigned int ry = *(const volatile unsigned int*)&r.y;
        unsigned int rz = *(const volatile unsigned int*)&r.z;
        y = ry;
        z = rz;
        return *this;
    }
};
struct WindEntry {
    WindTriple a, b, c;
    unsigned int d;
    unsigned char flag;
    char pad[3];
    float x, y, z;
    unsigned int tail;
    // Copy a wind record with the original exception boundary.
    WindEntry(const WindEntry& r) {
        try {
            copy(r);
        } catch (...) {
            throw;
        }
    }
    // Copy all record words, flag and trailing scalar fields without interpretation.
    void copy(const WindEntry& r) {
        a = r.a;
        b = r.b;
        c = r.c;
        d = r.d;
        flag = r.flag;
        x = r.x;
        y = r.y;
        z = r.z;
        tail = r.tail;
    }
};
// Complete semantic overlay for the SAME 0x3C-byte record copied by WindEntry.
// CWind's +90/+94/+98 vector owns these values (no per-record heap pointers).
// CWind::GetWindVectorAtPoint / region parser (wind_regions_and_vectors.cpp) establish this schema:
// +00 index, +04 min XYZ, +10 max XYZ, +1C precomputed wind XYZ, +28 operation
// byte (0 add/1 multiply/2 subtract/3 replace), +2C strength, +30 yaw degrees,
// +34 pitch degrees, +38 adjacent region index (-1 none). Bounds inclusive;
// neighbor must be a valid vector index. Queries blend stored wind, not angles.
// WindTriple a/b/c cross these semantic field boundaries to retain the original
// word-copy schedule; x/y/z after flag are strength/yaw/pitch, not a position.
struct WindRegionRecord {
    int index;
    VectorLike minimum, maximum, velocity;
    unsigned char operation;
    char reserved29[3];
    float strength, yawDegrees, pitchDegrees;
    int adjacentRegion;
};
typedef char WindRegionRecord_size[sizeof(WindRegionRecord) == sizeof(WindEntry) ? 1 : -1];
struct WindAllocator {
    signed char value;
    char pad[3];
};
// Materialize the original allocator tag; volatile writes preserve compiled code.
static inline void windAllocatorSet(int v) {
    volatile WindAllocator allocator;
    allocator.value = v;
}
// Preserve one level of the wind allocator's inline initialization chain.
static inline void windAllocatorInit(int v) {
    windAllocatorSet(v);
}
// Forward the allocator tag through the original third inline layer.
static inline void windAllocatorInit3(int v) {
    windAllocatorInit(v);
}
// Forward the allocator tag through the original fourth inline layer.
static inline void windAllocatorInit4(int v) {
    windAllocatorInit3(v);
}
struct WindStorage {
    WindEntry* entries;
    unsigned int count;
    unsigned int capacity;
    // Start an empty temporary owner for replacement wind-record storage.
    WindStorage() : entries(0), count(0), capacity(0) {}
    // Reduce the constructed-record count without freeing the allocation.
    void remove(unsigned int n) {
        count -= n;
    }
    // Clear the logical count and free only the record array.
    ~WindStorage() {
        if (entries) {
            int n = count;
            volatile WindAllocator allocator;
            allocator.value = 0;
            remove(n);
            appFreeUni__FPv(entries);
        }
    }
};
struct ParticleList {
    unsigned char pad[8];
    unsigned char a;
    char p0[3];
    unsigned char b;
    char p1[3];
    unsigned int count;
    unsigned int capacity;
    ParticleEntry* entries;
};

enum WindConfigurationDefaults {
    kWindIndicatorLightType = 3
};

enum WaveletState {
    kWaveletFadeIn = 0,
    kWaveletFadeOut = 1,
    kWaveletDormant = 2
};
// One branch per layer per update; comparisons are deliberately strict:
// state        per-tick work                    transition / side effect
// FadeIn  (0)  alpha += 255*dt/fadeInTime        alpha > 255 -> FadeOut;
//                                                clamp to 255; arm NEXT delay
// FadeOut (1)  alpha -= 255*dt/fadeOutTime       alpha < 0 -> Dormant
// Dormant (2)  subtract dt only if delay > 0    delay < 0 -> FadeIn;
//                                                zero scale/alpha/elapsed,
//                                                speed = 4*deceleration
// Both visible states first reduce widthVelocity by deceleration*dt, then
// integrate scale using that new velocity. elapsed increases even when dormant.
// State 1 does not clamp negative alpha on exit; state 2 resets it on restart.
enum WaveletMeshConstants {
    kWaveletVertexFormat = 0x11F001,
    kWaveletTriangleStrip = 0x98,
    kWaveletVertexBytes = 0x18,
    kWaveletLayerBytes = 0x18,
    kGroundStripPointBytes = 0x24
};
// Each layer owns a separate mesh. scale multiplies the point displacement;
// alpha is in [0,255] while visible, not [0,1]. Timing fields share dt's units.
// Fade-in completion arms the NEXT layer's delay; dormant restarts only when a
// positive delay crosses strictly below zero (landing at zero does not restart).
struct WaveletLayer {
    float scale;
    float delayRemaining;
    float alpha;
    float elapsed;
    float widthVelocity;
    int state;
};

struct TargetRecord {
    float edges[2][3];
    float displacement[3];
};
struct TargetPoint {
    unsigned int x, y, z;
};
struct TargetOutput {
    float position[3];
    unsigned char color[4];
    char textureCoordinates[8];
};
struct WaveletManager {
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
    virtual void slot23();
    virtual void slot24();
    virtual void slot25();
    virtual void* texture(unsigned int);
    virtual void* textureState(unsigned int);
};
struct TargetObject {
    char pad00[4];
    TargetRecord* records;
    unsigned int count;
    char pad0c[0x20];
    WaveletLayer* layers;
    unsigned int total;
    char pad34[4];
    void** meshes;
};

struct PtrVector {
    void** begin;
    unsigned int size;
    unsigned int capacity;
};
extern void* __nwa__FUl(unsigned int);
extern void* appMallocDummy__FUl(int);
extern void* __nw__FUl(unsigned int);
extern void __dla__FPv(void*);
extern void __dt__Q23std12length_errorFv(void*);
extern unsigned char __vt__Q23std11logic_error[];
extern unsigned char s_sharedCharDeleterVtable[];
extern unsigned char __vt__Q23std12length_error[];
extern char taskSequenceStrings[];
extern void* memcpy(void*, const void*, unsigned int);
extern void* memset(void*, int, unsigned int);

struct RecordTriple {
    unsigned int x, y, z;
};
// A point record is 0x24 (36) bytes.
// The three triples are bit-preserving copies of the two edges and displacement.
struct GroundStripPointRecord {
    RecordTriple a, b, c;
    // Copy three triples while preserving the original exception boundary.
    GroundStripPointRecord(const GroundStripPointRecord& r) {
        try {
            *this = r;
        } catch (...) {
            throw;
        }
    }
};
struct RecordVector {
    GroundStripPointRecord* begin;
    unsigned int size;
    unsigned int capacity;
};

struct RecordAllocator {
    signed char value;
};
// Allocator forwarding preserves stack-byte placement in Append; no layer changes policy.
// Materialize the point-vector allocator's zero tag.
inline void recordAllocatorSet() {
    volatile RecordAllocator a;
    a.value = 0;
}
// Preserve the second allocator initialization layer.
inline void recordAllocator2() {
    recordAllocatorSet();
}
// Preserve the third allocator initialization layer.
inline void recordAllocator3() {
    recordAllocator2();
}
// Preserve the fourth allocator initialization layer.
inline void recordAllocator4() {
    recordAllocator3();
}
// Preserve the fifth allocator initialization layer.
inline void recordAllocator5() {
    recordAllocator4();
}
// Preserve the sixth allocator initialization layer.
inline void recordAllocator6() {
    recordAllocator5();
}
// Preserve the seventh allocator initialization layer.
inline void recordAllocator7() {
    recordAllocator6();
}
struct MessageGuard {
    char* ptr;
    RecordAllocator* allocator;
    // Guard a message array until its shared control block is constructed.
    MessageGuard(char* p, RecordAllocator* a) : ptr(p), allocator(a) {}
    // Delete the array unless the caller has transferred it by clearing ptr.
    ~MessageGuard() {
        if (ptr)
            __dla__FPv(ptr);
    }
};
struct MessageOwner {
    void* vtable;
    unsigned int refs, refs2;
    char* text;
    // Own a message array with initial strong/weak counts of one.
    MessageOwner(char* p) {
        refs = 1;
        refs2 = 1;
        vtable = s_sharedCharDeleterVtable;
        text = p;
    }
};
extern "C" void __dt__Q23std9exceptionFv(void*, int);
struct ErrorBase_u80211250 {
    void* vtable;
    char* text;
    MessageOwner* owner;
    // Duplicate the error message into reference-counted array storage.
    ErrorBase_u80211250(const char* input) {
        const char* what = input;
        vtable = __vt__Q23std11logic_error;
        volatile RecordAllocator allocator0, allocator1;
        allocator0.value = 0;
        allocator1.value = 0;
        unsigned int n = strlen(what) + 1;
        char* buffer = (char*)__nwa__FUl(n * 2 - n);
        text = buffer;
        {
            MessageGuard guard(buffer, (RecordAllocator*)&allocator1);
            owner = new MessageOwner(buffer);
            guard.ptr = 0;
        }
        strcpy(text, what);
    }
    // Destroy the std::exception base without freeing the stack exception.
    ~ErrorBase_u80211250() {
        __dt__Q23std9exceptionFv(this, -1);
    }
    // Build the pointer-vector variant using its original allocator-tag expression.
    ErrorBase_u80211250(const char* input, int) {
        const char* const& what = input;
        vtable = __vt__Q23std11logic_error;
        RecordAllocator allocator;
        allocator.value = 0;
        unsigned int n = strlen(what) + 1;
        char* buffer = (char*)__nwa__FUl(n * 2 - n);
        text = buffer;
        {
            MessageGuard guard(buffer, &allocator);
            owner = new MessageOwner(buffer);
            guard.ptr = 0;
        }
        strcpy(text, what);
    }
};
}
namespace std {
struct length_error : ErrorBase_u80211250 {
    // Construct the point-vector length error and install its derived table.
    length_error(const char* what) : ErrorBase_u80211250(what) {
        vtable = __vt__Q23std12length_error;
    }
    // Construct the pointer-vector length error and install its derived table.
    length_error(const char* what, int tag) : ErrorBase_u80211250(what, tag) {
        vtable = __vt__Q23std12length_error;
    }
    ~length_error();
    DECOMP_CT_MEMBER(length_error)
};
}
extern "C" {
// Construct and throw the point-vector length exception using the MSL runtime ABI.
inline void recordLengthError() {
    {
        union {
            double alignment;
            unsigned char bytes[sizeof(std::length_error)];
        } storage;
        std::length_error* error = (std::length_error*)storage.bytes;
        error->__ct(taskSequenceStrings);
        exception_runtime::__throw(exception_runtime::taskSequenceStrings + 0x14, error,
                                   (void*)exception_runtime::__dt__Q23std12length_errorFv);
    }
}
// Forward the point-vector overflow exception through its second inline layer.
inline void recordLengthError2() {
    recordLengthError();
}
// Forward the point-vector overflow exception through its third inline layer.
inline void recordLengthError3() {
    recordLengthError2();
}
// Forward the point-vector overflow exception through its fourth inline layer.
inline void recordLengthError4() {
    recordLengthError3();
}
struct RecordStorage {
    GroundStripPointRecord* begin;
    unsigned int size, capacity;
    unsigned int* allocator;
    unsigned int position;
    // Start an empty guarded allocation for point-vector growth.
    RecordStorage(unsigned int* a) : begin(0), size(0), capacity(0), allocator(a), position(0) {}
    // Free the temporary/retired record array without freeing any external payload.
    ~RecordStorage() {
        size = 0;
        if (begin) {
            volatile RecordAllocator a;
            a.value = 0;
            size = size - size;
            appFreeUni__FPv(begin);
        }
    }
};
// Select the larger growth amount by reference.
inline const unsigned int& recordMax(const unsigned int& a, const unsigned int& b) {
    return a < b ? b : a;
}

#pragma push
#pragma use_lmw_stmw on
// Append a ground-strip point record with exception-safe vector growth.
void GroundStripPointVector_Append(RecordVector* self, const GroundStripPointRecord& value) {
    // value is borrowed and may alias an old element. Construct the new tail
    // before copying old records backward; commit only after every copy succeeds.
    unsigned int grown0, grown1, extra;
    unsigned int oldCapacity, capacity;
    recordAllocator7();
    unsigned int size = self->size;
    unsigned int cap = self->capacity;
    if (size < cap) {
        GroundStripPointRecord* dst = self->begin + size;
        new (dst) GroundStripPointRecord(value);
        self->size = self->size + 1;
        return;
    }
    // Phase 1: choose capacity and guard the replacement storage.
    volatile RecordAllocator allocator1;
    recordAllocator6();
    allocator1.value = 0;
    const unsigned int maxSize = 0x071c71c7;
    if (1 > maxSize - self->capacity)
        recordLengthError();
    RecordStorage temp(&self->capacity);
    extra = self->size + 1 - self->capacity;
    oldCapacity = self->capacity;
    if (extra > maxSize - oldCapacity)
        recordLengthError3();
    if (oldCapacity < 0x025ed097) {
        grown0 = (oldCapacity + 1) * 3 / 5;
        capacity = oldCapacity + recordMax(grown0, extra);
    } else if (oldCapacity < 0x04bda12e) {
        grown1 = (oldCapacity + 1) / 2;
        capacity = oldCapacity + recordMax(grown1, extra);
    } else
        capacity = maxSize;
    recordAllocator4();
    if (capacity > maxSize)
        recordLengthError2();
    temp.begin = (GroundStripPointRecord*)appMallocDummy__FUl(capacity * 0x24);
    temp.capacity = capacity;
    temp.position = self->size;
    GroundStripPointRecord* dst = temp.begin + temp.position + temp.size;
    recordAllocator3();
    for (unsigned int n = 1; n != 0; --n, ++dst) {
        new (dst) GroundStripPointRecord(value);
        ++temp.size;
    }
    // Phase 2: the appended value exists; copy old records backward into the prefix.
    GroundStripPointRecord* src = self->begin + self->size;
    GroundStripPointRecord* first = self->begin;
    GroundStripPointRecord* dst2 = temp.begin + temp.position;
    recordAllocator2();
    while (src > first) {
        --dst2;
        --src;
        new (dst2) GroundStripPointRecord(*src);
        --temp.position;
        ++temp.size;
    }
    // Phase 3: swap allocation ownership; temp frees the retired buffer on exit.
    self->size = 0;
    recordAllocatorSet();
    unsigned int oldCapacity2 = self->capacity;
    self->capacity = temp.capacity;
    temp.capacity = oldCapacity2;
    GroundStripPointRecord* old = self->begin;
    self->begin = temp.begin;
    temp.begin = old;
    unsigned int oldSize = self->size;
    self->size = temp.size;
    temp.size = oldSize;
}

#pragma pop
struct PtrStorage {
    void** begin;
    unsigned int size, capacity;
    unsigned int* allocator;
    unsigned int position;
    // Start an empty guarded pointer-array allocation.
    PtrStorage(unsigned int* a) : begin(0), size(0), capacity(0), allocator(a), position(0) {}
    // Free pointer storage only; pointed-to graphics buffers are untouched.
    ~PtrStorage() {
        size = 0;
        if (begin) {
            size = 0;
            appFreeUni__FPv(begin);
        }
    }
};
// Construct and throw the buffer-pointer vector's length exception.
inline void ptrLengthError() {
    {
        union {
            double alignment;
            unsigned char bytes[sizeof(std::length_error)];
        } storage;
        std::length_error* error = (std::length_error*)storage.bytes;
        error->__ct(taskSequenceStrings, 0);
        exception_runtime::__throw(exception_runtime::taskSequenceStrings + 0x14, error,
                                   (void*)exception_runtime::__dt__Q23std12length_errorFv);
    }
}
// Preserve the second pointer-vector throw wrapper.
inline void ptrLengthError2() {
    ptrLengthError();
}
// Preserve the third pointer-vector throw wrapper.
inline void ptrLengthError3() {
    ptrLengthError2();
}
// Preserve the fourth pointer-vector throw wrapper.
inline void ptrLengthError4() {
    ptrLengthError3();
}
#pragma push
#pragma use_lmw_stmw on
// Append a graphics-buffer pointer; growth transfers array storage without retaining/releasing buffers.
void GroundStripBufferVector_Append(PtrVector* self, void* const& value) {
    unsigned int size = self->size;
    unsigned int cap = self->capacity;
    if (size < cap) {
        self->size = size + 1;
        self->begin[self->size - 1] = value;
        return;
    }

    // Phase 1: reserve a guarded replacement array large enough for the new tail.
    const unsigned int max = 0x3fffffff;
    if (1 > max - self->capacity)
        ptrLengthError4();
    PtrStorage temp(&self->capacity);
    unsigned int extra = self->size + 1 - self->capacity;
    unsigned int oldCapacity = self->capacity;
    if (extra > max - oldCapacity)
        ptrLengthError2();
    unsigned int newCap;
    unsigned int grown1, grown0;
    if (oldCapacity < 0x15555555) {
        grown0 = (oldCapacity + 1) * 3 / 5;
        newCap = oldCapacity + recordMax(grown0, extra);
    } else if (oldCapacity < 0x2aaaaaaa) {
        grown1 = (oldCapacity + 1) / 2;
        newCap = oldCapacity + recordMax(grown1, extra);
    } else
        newCap = max;
    if (newCap > max)
        ptrLengthError3();

    void** fresh = (void**)appMallocDummy__FUl(newCap * 4);
    temp.position = self->size;
    (fresh + temp.position)[temp.size] = value;
    temp.begin = fresh;
    temp.capacity = newCap;
    ++temp.size;
    // Phase 2: copy pointer words into the prefix and clear the old slots.
    int count;
    void** first = self->begin;
    count = (int)((char*)(first + self->size) - (char*)first) / 4;
    temp.position -= count;
    unsigned int bytes = count * 4;
    memcpy(temp.begin + temp.position, first, bytes);
    memset(first, 0, bytes);
    temp.size += count;
    self->size = 0;
    // Phase 3: install the replacement; temp frees only the retired pointer array.
    unsigned int oldCap = self->capacity;
    self->capacity = temp.capacity;
    temp.capacity = oldCap;
    void** old = self->begin;
    self->begin = temp.begin;
    temp.begin = old;
    unsigned int oldSize = self->size;
    self->size = temp.size;
    temp.size = oldSize;
}
#pragma pop

// Draw each active wavelet layer as a ground-projected strip with tapered edge alpha.
void CWavelet::DrawLayers() {
    TargetObject* self = (TargetObject*)this;
    // Point edges/displacement are in course world space. Each layer displaces
    // isEndPoint[edgeIndex] by -factor*scale*displacement, projects onto terrain, then emits
    // two vertices per point. End points have zero alpha; interior alpha is texture byte.
    // The byte-sized loop counters require counts representable in that range.
    // Phase 1: install shared view/identity transforms, texture and render defaults.
    int isEndPoint;
    int pointByteOffset, firstVertexIndex, layerByteOffset, meshByteOffset;
    unsigned char pointIndex, edgeIndex, layerIndex;
    unsigned int config = (unsigned int)&waveletRenderDefaults;
    Config72 cfg = *(Config72*)config;
    Config48 cfg2 = ((const WaveletConfig*)config)->b; // +0x48: second render-state block
    Config40 cfg3 = ((const WaveletConfig*)config)->c; // +0x78: third render-state block
    Engine_SetRenderStateList(&cfg, 9);
    Engine_SetTextureStageStateList(0, &cfg2, 6);
    Engine_SetTextureStageParameterList(0, &cfg3, 5);
    float matrix[16];
    Matrix_ExpandAffineTo4x4(((char*)&AsWView(g_view)->inputTransform), matrix);
    SetTransform(0, matrix);
    Engine_SetType7Resource(0);
    SetTransform(2, kIdentityMatrix4x4);
    SetTransform(11, kIdentityMatrix4x4);
    Engine_SetFlexibleVertexFormat(kWaveletVertexFormat);
    WaveletManager* manager = (WaveletManager*)AsWView(g_view)->renderTarget; // WView +0xa4
    void* texture = manager->texture(AsCWavelet(self)->texHandle);
    Engine_SetTextureStageResource(0, texture);
    manager = (WaveletManager*)AsWView(g_view)->renderTarget;
    void* textureState = manager->textureState(AsCWavelet(self)->texHandle);
    Engine_SetTexturePalette(0, textureState);
    // Phase 2: rewrite each visible layer's dynamic strip, flush, and draw it.
    for (layerIndex = 0; layerIndex < self->total; ++layerIndex) {
        layerByteOffset = layerIndex * kWaveletLayerBytes;
        int mode = ((WaveletLayer*)((char*)self->layers + layerByteOffset))->state;
        if (mode == kWaveletDormant)
            continue;
        TargetOutput* vertices;
        meshByteOffset = layerIndex * 4;
        Resource_GetOffsetPtr(*(void**)((char*)self->meshes + meshByteOffset), 0, 0, &vertices, 0);
        for (pointIndex = 0; pointIndex < self->count; ++pointIndex) {
            isEndPoint = 0;
            if ((unsigned char)pointIndex == 0 || (unsigned char)pointIndex == self->count - 1)
                isEndPoint = 1;
            pointByteOffset = pointIndex * kGroundStripPointBytes;
            firstVertexIndex = pointIndex * 2;
            for (edgeIndex = 0; edgeIndex < 2; ++edgeIndex) {
                TargetRecord* point = (TargetRecord*)((char*)self->records + pointByteOffset);
                WaveletLayer* layers = self->layers;
                float factor = (unsigned char)edgeIndex
                                   ? kFloat_0_1_Wavelet
                                   : (isEndPoint ? kFloat_0_5_Wavelet : kFloat_1_SceneEffects_805DFDC0);
                TargetPoint groundPoint, offsetPosition, scaledDisplacement, groundProjection, groundQueryPosition;
                __ml__FfRC7WVector(&scaledDisplacement, point->displacement,
                                   factor * ((WaveletLayer*)((char*)layers + layerByteOffset))->scale);
                __mi__FRC7WVectorRC7WVector(&offsetPosition, point->edges[(unsigned char)edgeIndex],
                                            &scaledDisplacement);
                *(WVector*)&groundPoint = *(WVector*)&offsetPosition;
                groundQueryPosition = groundPoint;
                void* quadTree = CQuadTree_GetInstance();
                GetGroundPoint__9CQuadTreeF7WVectorbPib(&groundProjection, quadTree, &groundQueryPosition, 1, 0, 1);
                *(WVector*)&groundPoint = *(WVector*)&groundProjection;
                Vector_CopyComponents(&vertices[((unsigned char)edgeIndex + firstVertexIndex)], &groundPoint);
                if (isEndPoint)
                    vertices[firstVertexIndex + (unsigned char)edgeIndex].color[3] = 0;
                else
                    vertices[firstVertexIndex + (unsigned char)edgeIndex].color[3] =
                        (unsigned char)((WaveletLayer*)(layerByteOffset + (unsigned int)self->layers))->alpha;
            }
        }
        GraphicsBuffer_Flush(*(void**)((char*)self->meshes + meshByteOffset));
        Engine_SetVertexStream(0, *(void**)((char*)self->meshes + meshByteOffset), 0, kWaveletVertexBytes);
        DrawPrimitiveFromBoundBuffers(kWaveletTriangleStrip, 0, self->count * 2 - 2);
    }
}

// Set up the wind state, the indicator's tilted viewport transform and light,
// and an empty region list.
CWind::CWind() {
    // Phase 1: initialize wind state and the -30 degree viewport basis.
    int zero;
    int onePast24BitWord;
    float negativeOne;
    float nineFloat;
    float oneFloat;
    float zeroFloat;
    float tiltRadians;
    float lightDirectionZ;
    float lightDirectionY;
    float lightDirectionX;
    float ambientComponent;
    float diffuseComponent;
    zeroFloat = kFloat_0_SceneEffects;
    oneFloat = kFloat_1_SceneEffects;
    nineFloat = kFloat_9_SceneEffects;
    negativeOne = kFloat_neg1_SceneWind;
    tiltRadians = kFloat_neg0_5235988_Trajectory;
    zero = 0;
    onePast24BitWord = 0x01000000;
    vtbl = __vt__5CWind;
    modelSetup.word = zero;
    modelSetup.wordA = zero;
    modelSetup.wordB = zero;
    modelSetup.wordC = onePast24BitWord - 1;
    modelSetup.direction.z = zeroFloat;
    modelSetup.direction.y = zeroFloat;
    modelSetup.direction.x = zeroFloat;
    regions = 0;
    regionCount = zero;
    regionCapacity = zero;
    enabled = 1;
    modelSetup.scalarA = oneFloat;
    modelSetup.scalarB = nineFloat;
    speed = zeroFloat;
    directionAngle = zeroFloat;
    overrideSpeed = negativeOne;
    overrideDirection = zeroFloat;
    overrideActive = 0;
    char initialTiltMatrix[0x30];
    Matrix_SetAxisRotation(&initialTiltMatrix, tiltRadians, (signed char)zero);
    Matrix_CopyAffine(indicatorViewRotation, &initialTiltMatrix);
    float viewportWidth = AsWView(g_view)->viewportWidth;
    char translationTiltMatrix[0x30];
    Matrix_SetAxisRotation(&translationTiltMatrix, kFloat_neg0_5235988_Trajectory, (signed char)zero);
    VectorLike viewportTranslation;
    __ml__FfRC7WVector(&viewportTranslation, (float*)(translationTiltMatrix + 0x18),
                       kFloat_neg170_WindAndTrajectory * (viewportWidth / kFloat_640_TrajectoryViewport));
    indicatorViewPosition = *(WVector*)&viewportTranslation;
    modelSetup.word = 2;
    VectorLike axis;
    float axisZ;
    float axisY;
    float axisX;
    axisX = kFloat_0_SceneEffects;
    modelSetup.wordB = onePast24BitWord - 1;
    axisY = kFloat_neg1_SceneWind;
    modelSetup.wordA = onePast24BitWord - 1;
    axisZ = kFloat_1_SceneEffects;
    axis.x = axisX;
    axis.y = axisY;
    axis.z = axisZ;
    modelSetup.direction = (*(WVector*)&axis).Normalize();
    float maximumDepth;
    float minimumDepth;
    minimumDepth = kFloat_0_SceneEffects;
    maximumDepth = kFloat_1_SceneEffects;
    attachedEffect = 0;
    cannonEndSoundPlayed = zero;
    cannonChargeSoundPlayed = zero;
    indicatorViewport.x = 514;
    indicatorViewport.y = 354;
    indicatorViewport.width = 130;
    indicatorViewport.height = 126;
    indicatorViewport.minimumDepth = minimumDepth;
    indicatorViewport.maximumDepth = maximumDepth;
    Memory_Clear(&indicatorLight, sizeof(indicatorLight));
    diffuseComponent = kFloat_1_SceneEffects;
    ambientComponent = kFloat_0_5_SceneEffects;
    lightDirectionX = kFloat_0_005122_SceneEffects;
    lightDirectionY = kFloat_neg0_858835_Wind;
    lightDirectionZ = kFloat_0_512227_SceneEffects;
    // Light colors are float components; Memory_Clear supplied the zero alpha/specular.
    indicatorLight.type = kWindIndicatorLightType;
    indicatorLight.diffuse[0] = diffuseComponent;
    indicatorLight.diffuse[1] = diffuseComponent;
    indicatorLight.diffuse[2] = diffuseComponent;
    indicatorLight.ambient[0] = ambientComponent;
    indicatorLight.ambient[1] = ambientComponent;
    indicatorLight.ambient[2] = ambientComponent;
    indicatorLight.direction.x = lightDirectionX;
    indicatorLight.direction.y = lightDirectionY;
    indicatorLight.direction.z = lightDirectionZ;
    Vec3_Normalize(&indicatorLight.direction, &indicatorLight.direction);
    // Phase 2: reserve eight 0x3C-byte wind records, copy existing records, and
    // swap buffers. The temporary frees the retired array after the swap.
    if (8 > regionCapacity) {
        WindStorage temp;
        unsigned int start;
        windAllocatorInit4(zero);
        WindEntry* allocation = (WindEntry*)appMallocDummy__FUl(0x1e0);
        unsigned int count = regionCount;
        start = temp.count;
        WindEntry* src = (WindEntry*)regions;
        WindEntry* dst;
        WindEntry* endSrc;
        temp.capacity = 8;
        temp.entries = allocation;
        endSrc = count + src;
        dst = start + temp.entries;
        windAllocatorInit3(zero);
        for (; src < endSrc; ++src, ++dst) {
            new (dst) WindEntry(*src);
            temp.count = (temp.count + 1);
        }
        regionCount = 0;
        unsigned int n;
        unsigned int previousCapacity;
        previousCapacity = regionCapacity;
        windAllocatorInit(zero);
        regionCapacity = temp.capacity;
        temp.capacity = previousCapacity;
        WindEntry* old = (WindEntry*)regions;
        regions = (WindRegion*)temp.entries;
        temp.entries = old;
        n = regionCount;
        regionCount = temp.count;
        temp.count = n;
    }
}

typedef WaveletLayer Rec1C68; // Legacy type identity; update code uses the documented layer layout.
struct WaveletView {
    char baseWavelet[0x2C];
    WaveletLayer* records;
    unsigned int count;
};

// Borrow one layer from its byte displacement without changing the mesh/layer ownership.
static inline WaveletLayer* WaveletLayerAtOffset(WaveletLayer* layers, int byteOffset) {
    return (WaveletLayer*)((char*)layers + byteOffset);
}

// Borrow the scale at a layer displacement; reference assignment preserves the indexed float store.
static inline float& WaveletScaleAtOffset(WaveletLayer* layers, int byteOffset) {
    return ((float*)((char*)layers + byteOffset))[0];
}

// Advance wavelet layer timing, state, width, and fade alpha by the elapsed time.
// Tuning/clock contract: dt, elapsed, fades, delay and timingScale use simulated
// seconds; this entry applies no conversion or local frame multiplier.
// CWavelet_ConfigureFromBeachParameterName builds
// fadeInTime=0.080000006*timingScale, fadeOutTime=0.72*timingScale,
// delay=0.2*timingScale and deceleration=width/(4*fadeIn-.5*fadeIn*fadeIn).
// width/scale use course world units; velocity world units/second; alpha uses
// byte-alpha units. elapsed increments even while dormant.
// Increasing timingScale lengthens fades/delay, but ALSO changes deceleration:
// it is not merely a playback-speed divisor. Keep all four derived fields together.
// Require finite dt>=0, positive fade durations and a nonzero deceleration
// denominator. No validation/clamping/substepping occurs. Larger dt can overshoot
// width/alpha and only one branch runs per layer, so equal total time split into
// different update steps need not yield equal geometry. Baseline constants are
// in the data section; max alpha=255 in this unit's data, edge factors .1/.5/1 in draw.
// Clock/dispatch evidence: CMainFrame::UpdateVirtualHooks (main_frame_letter_and_mouse_cursor.cpp) sends
// seconds to CProjectG_Update (vtable in phase1_class_tables); that function
// (project_g_app_and_devices.cpp) multiplies by project timeScale, including zero on pause, then
// CTask_UpdateCurrentTaskWithFadeGates (ctask_transition_command_queue.cpp) forwards it to the task.
// CGolfTask_OnProcess (golf_task_loading_and_pause.cpp) -> CTask_ProcessActors (task_manager_and_actors.cpp) ->
// DrawScene_UpdateRegisteredObjects (draw_scene_named_effect_list.cpp) forwards the same float to the
// generated-texture pool's second virtual slot. GroundStripOwner_Init installs
// s_waveletVtable_804B1B48 on those objects; its table bytes remain external to
// src/include. Registry callbacks borrow objects: the owner must keep each strip
// alive during update; the registry mutex protects traversal, not its lifetime.
void CWavelet::UpdateLayers(float dt) {
    WaveletView* self = (WaveletView*)this;
    // stateLayer/currentLayer/fadingLayer refer to the SAME current layer. Reloads
    // after each store preserve the original aliasing and floating-point evaluation order.
    // Requires nonzero fade durations and count <= 255 (the index is narrowed
    // to a byte). The next-layer modulo uses count, so an active layer also
    // requires a nonempty array. Each layer executes one state branch per call;
    // a newly armed next layer may consume its delay later in this same loop.
    float zero;
    int layerIndex;
    float restartVelocityFactor;
    int fadeInState;
    float maxAlpha;
    int dormantState;
    int fadeOutState;
    int layerByteOffset;
    WaveletLayer* layers;
    unsigned int index;
    WaveletLayer* stateLayer;
    float changeRate;
    WaveletLayer* currentLayer;
    float updatedValue;
    unsigned int nextIndex;
    unsigned int layerCount;
    WaveletLayer* layerBase;
    unsigned int quotient;
    float restartDelay;
    unsigned int wrappedCount;
    unsigned int wrappedIndex;
    WaveletLayer* nextLayer;

    restartVelocityFactor = kFloat_4_Wavelet;
    layerIndex = 0;
    zero = kFloat_0_805DFDA4;
    fadeInState = kWaveletFadeIn;
    maxAlpha = kFloat_255_Wavelet;
    dormantState = kWaveletDormant;
    fadeOutState = kWaveletFadeOut;
    for (; (unsigned char)layerIndex < self->count; ++layerIndex) {
        index = (unsigned char)layerIndex;
        layers = self->records;
        layerByteOffset = index * kWaveletLayerBytes;
        currentLayer = WaveletLayerAtOffset(layers, layerByteOffset);
        updatedValue = currentLayer->elapsed;
        updatedValue = updatedValue + dt;
        currentLayer->elapsed = updatedValue;
        layers = self->records;
        stateLayer = WaveletLayerAtOffset(layers, layerByteOffset);
        switch (stateLayer->state) {
        case kWaveletFadeIn: {
            // Raise alpha and integrate the decelerating width; on strict
            // alpha overflow, clamp and arm the next layer in the cycle.
            changeRate = AsCWavelet(self)->fadeInTime;
            updatedValue = stateLayer->alpha;
            changeRate = maxAlpha / changeRate;
            updatedValue = changeRate * dt + updatedValue;
            stateLayer->alpha = updatedValue;
            layers = self->records;
            currentLayer = WaveletLayerAtOffset(layers, layerByteOffset);
            changeRate = AsCWavelet(self)->deceleration;
            updatedValue = currentLayer->widthVelocity;
            updatedValue = updatedValue - changeRate * dt;
            currentLayer->widthVelocity = updatedValue;
            layers = self->records;
            currentLayer = WaveletLayerAtOffset(layers, layerByteOffset);
            changeRate = currentLayer->widthVelocity;
            updatedValue = currentLayer->scale;
            updatedValue = dt * changeRate + updatedValue;
            currentLayer->scale = updatedValue;
            layers = self->records;
            currentLayer = WaveletLayerAtOffset(layers, layerByteOffset);
            updatedValue = currentLayer->alpha;
            if (!(updatedValue > maxAlpha))
                break;
            currentLayer->alpha = maxAlpha;
            nextIndex = index + 1;
            layers = self->records;
            WaveletLayer* fadingLayer = WaveletLayerAtOffset(layers, layerByteOffset);
            fadingLayer->state = fadeOutState;
            layerCount = self->count;
            layerBase = self->records;
            quotient = nextIndex / layerCount;
            restartDelay = AsCWavelet(self)->delay;
            wrappedCount = quotient * layerCount;
            wrappedIndex = nextIndex - wrappedCount;
            nextLayer = WaveletLayerAtOffset(layerBase, wrappedIndex * kWaveletLayerBytes);
            nextLayer->delayRemaining = restartDelay;
            break;
        }
        case kWaveletFadeOut: {
            // Lower alpha with the same width integration; only a strictly
            // negative alpha makes this layer dormant.
            changeRate = AsCWavelet(self)->fadeOutTime;
            updatedValue = stateLayer->alpha;
            changeRate = maxAlpha / changeRate;
            updatedValue = updatedValue - changeRate * dt;
            stateLayer->alpha = updatedValue;
            layers = self->records;
            currentLayer = WaveletLayerAtOffset(layers, layerByteOffset);
            changeRate = AsCWavelet(self)->deceleration;
            updatedValue = currentLayer->widthVelocity;
            updatedValue = updatedValue - changeRate * dt;
            currentLayer->widthVelocity = updatedValue;
            layers = self->records;
            currentLayer = WaveletLayerAtOffset(layers, layerByteOffset);
            changeRate = currentLayer->widthVelocity;
            updatedValue = currentLayer->scale;
            updatedValue = dt * changeRate + updatedValue;
            currentLayer->scale = updatedValue;
            layers = self->records;
            WaveletLayer* fadingLayer = WaveletLayerAtOffset(layers, layerByteOffset);
            updatedValue = fadingLayer->alpha;
            if (!(updatedValue < zero))
                break;
            fadingLayer->state = dormantState;
            break;
        }
        case kWaveletDormant: {
            // A positive armed delay must cross below zero to restart.
            // Reset alpha/elapsed/scale and set width speed to 4*deceleration.
            updatedValue = stateLayer->delayRemaining;
            if (!(updatedValue > zero))
                break;
            updatedValue = updatedValue - dt;
            stateLayer->delayRemaining = updatedValue;
            layers = self->records;
            currentLayer = WaveletLayerAtOffset(layers, layerByteOffset);
            updatedValue = currentLayer->delayRemaining;
            if (!(updatedValue < zero))
                break;
            currentLayer->state = fadeInState;
            layers = self->records;
            currentLayer = WaveletLayerAtOffset(layers, layerByteOffset);
            currentLayer->alpha = zero;
            layers = self->records;
            currentLayer = WaveletLayerAtOffset(layers, layerByteOffset);
            currentLayer->elapsed = zero;
            layers = self->records;
            currentLayer = layers;
            WaveletScaleAtOffset(currentLayer, layerByteOffset) = zero;
            layers = self->records;
            currentLayer = WaveletLayerAtOffset(layers, layerByteOffset);
            updatedValue = AsCWavelet(self)->deceleration;
            updatedValue = restartVelocityFactor * updatedValue;
            currentLayer->widthVelocity = updatedValue;
        }
        }
    }
}

// Clear and release the owned counted buffer, optionally freeing the wrapper.
void* dtor_8021222C(DtorObject* self, int deleteFlag) {
    if (self != 0) {
        if (self->resource != 0) {
            int count = self->count;
            int* q = &count;
            self->count = count - *(volatile int*)q;
            appFreeUni__FPv(self->resource);
        }
        if (deleteFlag > 0) {
            __dl__FPv(self);
        }
    }
    return self;
}

// Clear and release the owned counted buffer, optionally freeing the wrapper.
void* dtor_8021229C(DtorObject* self, int deleteFlag) {
    if (self != 0) {
        if (self->resource != 0) {
            int count = self->count;
            int* q = &count;
            self->count = count - *(volatile int*)q;
            appFreeUni__FPv(self->resource);
        }
        if (deleteFlag > 0) {
            __dl__FPv(self);
        }
    }
    return self;
}

// Clear and release the owned counted buffer, optionally freeing the wrapper.
void* dtor_8021230C(DtorObject* self, int deleteFlag) {
    if (self != 0) {
        self->count = 0;
        if (self != 0 && self->resource != 0) {
            self->count = 0;
            appFreeUni__FPv(self->resource);
        }
        if (deleteFlag > 0) {
            __dl__FPv(self);
        }
    }
    return self;
}

// Clear and release the owned counted buffer, optionally freeing the wrapper.
void* dtor_80212380(DtorObject* self, int deleteFlag) {
    if (self != 0) {
        self->count = 0;
        if (self != 0 && self->resource != 0) {
            self->count = 0;
            appFreeUni__FPv(self->resource);
        }
        if (deleteFlag > 0) {
            __dl__FPv(self);
        }
    }
    return self;
}

// Release the wind region list. The repeated checks are the original's nested
// array-destruction test; the delete-if-requested wrapper around this body is
// compiler-generated.
CWind::~CWind() {
    if (&regions != 0) {
        if (&regions != 0) {
            if (&regions != 0) {
                if (&regions != 0) {
                    if (regions != 0) {
                        int count = regionCount;
                        regionCount = count - *(volatile int*)&count;
                        appFreeUni__FPv(regions);
                    }
                }
            }
        }
    }
}

// Release a counted heap buffer and optionally free its wrapper.
void* dtor_80212588(MSLRecordVector* self, int deleteFlag) {
    return ::std::__vector_deleting_dtor(self, deleteFlag);
}

// Initialize this unit's writable two-pi cache.
void Wind_InitTwoPiCache() {
    s_windFullTurnRadians = kFloat_6_2831855;
}

// Empty cloud update hook.
void CCloud::vfunc_1() {}

// The class owns nothing, so only the compiler-generated deleting destructor
// remains.
CCloud::~CCloud() {}

// Draw the translucent dark cloud overlay across the viewport.
void CCloud::DrawOverlay() {
    float pos[4];
    float f0;

    enum {
        kCloudOverlayArgb = 0x60303030
    };
    const unsigned int color = kCloudOverlayArgb;
    void* obj = g_view;
    const double zero = kFloat_0_CloudOverlay;
    double f2;
    f0 = AsWView(obj)->viewportHeight;
    f2 = AsWView(obj)->viewportWidth;
    pos[0] = zero;

    pos[1] = zero;
    pos[2] = f2;
    pos[3] = f0;
    Sprite_DrawColoredRect(obj, pos, 0, color, (float)zero);
}

// Initialize the companion scene-effect two-pi cache.
void Cloud_InitTwoPiCache() {
    s_cloudFullTurnRadians = kFloat_6_2831855;
}

} // extern "C"

// Construct and register the wind singleton on first access.
CWind* CWind::GetInstance() {
    if ((signed char)s_windInitialized == 0) {
        // The singleton lives in raw storage, so its constructor is called explicitly.
        ((CWind*)s_windInstance)->__ct();
        __register_global_object(s_windInstance, (void*)__dt__5CWindFv, s_windDtorRegistration);
        s_windInitialized = 1;
    }
    return (CWind*)s_windInstance;
}

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU804621EC", _ctors_u80211250_0, Wind_InitTwoPiCache);
STATIC_INIT_ENTRY(".CRT$XCU804621EC", _ctors_u80211250_1, Cloud_InitTwoPiCache);
#pragma pop

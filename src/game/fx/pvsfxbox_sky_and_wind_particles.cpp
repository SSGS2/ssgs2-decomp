// Effect box (CPvsFxBox): course sky layers and their resource loading, wind particles, sprays
// and sequences from the course script, and the drawing of enabled effects.
// Also holds the player-rank band lookup, the named-rectangle overlay text cache, a token
// scanner, and the snow effect's construction and cleanup.

#include <decomp/static_init.h>
// Course sky, weather variants, wind particles and effect rendering. The merged
// unit also holds the player-rank lookup, the named-rectangle overlay text cache,
// a token scanner and the snow effect's construction and cleanup.
//
// The sky storage at CGolfDoc::courseEffectBox holds the frame time queued for
// the next update, the sky-loaded latch, four sky layer records, the wind
// particle slots and the effect owners. Particle motion is world-space, the sky
// geometry is camera-relative, and effect scripts advance in milliseconds.
extern "C" int s_defaultSkyTextureRepeats[];
struct SkyLayerStringPool {
    char variantSeparator[2];       // 0
    char cloudVariant[6];           // 2
    char rainVariant[5];            // 8
    char farSkyFormat[11];          // 13
    char fallbackFarSkyFormat[13];  // 24
    char nearSkyFormat[13];         // 37
    char fallbackNearSkyFormat[15]; // 50
    char upperSkyFormat[10];        // 65
    char lowerSkyFormat[10];        // 75
    char cloudBitmapSuffix[11];     // 85
    char sprayExtension[4];         // 96
    char sequenceExtension[4];      // 100
    char reserveLengthError[35];    // 104
    char lengthErrorTypeName[55];   // 139
    char outOfRange[27];            // 194
    char outOfRangeTypeName[55];    // 221
    char stringLengthError[27];     // 276
};

#include "iff/IffStruct.h"
#include "classes/WView.h"
// WView layout: 0x80065F84; render-target uses: 0x801F9048.
#include "classes/CGolfDoc.h"
#include "classes/CSharedDoc.h"
#define AsCGolfDoc(p) ((CGolfDoc*)(p))
#define AsCSharedDoc(p) ((CSharedDoc*)(p))
#include "sdk_protos.h"
#include "game/wvector.h"
#include "game/types.h"
#include "classes/CPvsFxBox.h"
#include "classes/CFxBox.h"
#include "classes/CFxSequence.h"
#include <decomp/cw_bitfields.h>
#include "classes/CWind.h"
#include "classes/CFxSpray.h"
#include "classes/CSnow.h"
extern char kIdentityMatrix4x4[];

// Generated sky geometry and its retained graphics buffers extend beyond the shared class view.
struct SkyBufferView {
    char prefix[0x9a0];
    unsigned char vertexCount;
    char countPadding[3];
    void* geometryVtable;
    void* generatedVertices;
    void* farVertices;
    void* nearVertices;
    void* farIndices;
    void* nearIndices;
    int farTexture, nearTexture, upperTexture, lowerTexture;
};
enum SkyLayerIndex {
    kFarRingLayer = 0,
    kNearRingLayer = 1,
    kUpperCapLayer = 2,
    kLowerCapLayer = 3,
    kSkyLayerCount = 4
};
// The same prefix repeats for both ring and cap entries; the remaining words
// belong to layer-specific geometry. This is an entry, never a CPvsFxBox.
struct SkyLayerView {
    char prefix[8];
    void* sourceVertices;
    unsigned int resourceFlags;
    char remainingGeometry[0x50];
};
// The first two words have two meanings: the effect-box wrapper stores a vtable
// pointer and a second vtable word there, while this storage holds the clock for
// the next update and the sky-loaded latch. CPvsFxBox spells both out as unions.
// The clock is written once per frame and cleared by the update that consumes it;
// a second writer before that update replaces it instead of adding to it. No
// mutex protects the storage. The wrapper is a separate allocation, so its
// constructor is not the Init operation below.
typedef char SkyLayerStrideCheck[sizeof(SkyLayerView) == 0x60 ? 1 : -1];

// Sky layers 0/1/2/3 are the far ring, near ring, upper and lower caps. Each
// layer has source vertices at +8 and flags at +0xC; the low 11 flag bits are
// the resource ID. The generated buffers at 0x9AC.. are retained references
// whose first word keeps the reference count in its high 16 bits. The source
// vector arrays are owned new[] allocations.
// World Y is vertical. Sky source geometry is camera-relative; drawing adds the
// camera translation and skyVerticalOffset. Projection and view transforms stay
// separate.
struct EffectSprayAttachmentView {
    char prefix[8];
    void* attachment;
    float attachmentParameters[2];
    unsigned int attachmentTransform[4];
    unsigned int targetTransform[4];
    unsigned char useCameraAttachment;
};
struct EffectSequenceAttachmentView {
    char prefix[0x14];
    void* attachment;
    float attachmentParameters[2];
};
struct EffectUpdateView {
    void* vtable;
    float updateStep;
};
struct CameraMatricesView {
    char prefix[0x20];
    float view[16];
    float projection[16];
};
struct SharedWeatherView {
    char prefix[0x9c6];
    unsigned char weatherVariant;
};

// Spray vectors are embedded in a gap of the shared CFxSpray layout.
struct CourseSprayPositionView {
    char prefix[0xB8];
    WVector worldPosition;
    char positionPadding[0xC];
    WVector cameraOffset;
};
struct CameraPositionView {
    char prefix[0x24];
    WVector position;
};
struct SkyTranslationView {
    char prefix[0x988];
    WVector translation;
};
typedef char CourseSprayCameraOffsetCheck[(int)&((CourseSprayPositionView*)0)->cameraOffset == 0xD0 ? 1 : -1];

struct PvsEffectEnableFlags {
    int drawSkyCaps, drawCourseSprays, drawSceneSprays, drawNearSky, drawParticles;
    int drawCourseSprayPass, resetCourseView;
    int updateCourseSprays, updateSceneSprays, updateAuxiliary;
};

// One wind-particle slot: dimensionless speed jitter, world-unit billboard size/
// position, opacity in 0..255, activity, atlas row 0..3 and horizontal UV flip.
struct WindParticleSlot {
    float speedJitter, size;
    Vec3f position;
    float opacity;
    unsigned char active;
    char activePadding[3];
    int atlasRow;
    unsigned char flipU;
    char flipPadding[3];
};
struct WindParticleAccess {
    char prefix[0x19c];
    WindParticleSlot particle;
};
typedef char WindParticleSlotSizeCheck[sizeof(WindParticleSlot) == 0x24 ? 1 : -1];
enum {
    kWindParticlePositionOffset = (int)&((WindParticleAccess*)0)->particle.position
};

enum SkyConstants {
    kSkyRingPointCount = 33,
    kSkyCapCornerCount = 4,
    kSkyVertexFormatXYZ_RGBA_UV = 0x11f001,
    kSkyIndexFormat = 3,
    kFarSkyVertexBytes = 0xc60,
    kFarSkyIndexBytes = 0x18c,
    kNearSkyBufferBytes = 0x948,
    kSkyTriangleStrip = 0x98,
    kParticleQuads = 0x80,
    kParticleBatchQuads = 32,
    kParticleVertexBytes = 0x18,
    kTransformView = 0,
    kTransformProjection = 1,
    kTransformWorld = 2,
    kTransformTexture0 = 11,
    kGeometryResetVirtualSlot = 3,
    kViewResetVirtualSlot = 4,
    kSkyRingSegments = 32,
    kMaxWindParticles = 50,
    kResourceIdMask = 0x7ff,
    kSkyLayerRenderFlags = 0x22300000,
    kSkyCapRenderFlags = 0x20300000,
    kWeatherClear = 0,
    kWeatherCloud = 1,
    kWeatherRain = 2,
    kTextHorizontalAlignmentMask = 6,
    kTextAlignCenter = 2,
    kTextAlignRight = 4,
    kTextVerticalAlignmentMask = 0x60,
    kTextAlignMiddle = 0x20,
    kTextAlignBottom = 0x40,
    kGameStateSkipCourseEffects = 0x200,
    kGameStateSkipParticleDraw = 0x200000
};
// IDs belong to the engine cache; values below are verified by gx_render_state_and_texture_upload.cpp.
enum CourseEffectRenderState {
    kEffectFogEnabled = 1,
    kEffectDepthCompare = 3,
    kEffectDepthWrite = 5,
    kEffectSourceBlend = 13,
    kEffectDestinationBlend = 14,
    kEffectFogEnd = 24
};
enum CourseEffectRenderValue {
    kEffectCompareAlways = 7,
    kEffectBlendSourceAlpha = 4,
    kEffectBlendInverseSourceAlpha = 5,
    kEffectSamplerWrapS = 0,
    kEffectSamplerWrapT = 1,
    kEffectSamplerRepeat = 1
};
typedef char SkyBufferOffsetCheck[(int)&((SkyBufferView*)0)->farVertices == 0x9ac ? 1 : -1];

struct WInstanceLock {
    void* value;
    // Lock the object's recursive OS mutex, including when already held by this thread.
    WInstanceLock(void* p) : value(p) {
        OSLockMutex((OSMutex*)p);
    }
    // Unlock on normal return or exception unwinding.
    ~WInstanceLock() {
        OSUnlockMutex((OSMutex*)value);
    }
};

// Names shared by the CPvsFxBox methods converted below. The namespace wrappers
// keep the original unit's conflicting prototypes apart; these declarations are
// the versions the member functions use.
extern "C" {

extern void* g_resourceManager;
extern void* g_sharedDoc;
extern void* g_view;
extern char s_cameraTransform[];
extern const char __vt__9CPvsFxBox[];
extern char __vt__5CSnow[];
extern unsigned int __vt__10CGeneTex3D[9];
extern const unsigned char courseEffectRenderDefaults[];
extern PvsEffectEnableFlags s_pvsEffectEnableFlags;
extern const SkyLayerStringPool skyLayerResourceStrings;
extern const float kFloat_0_CourseRender;
extern const float kFloat_0_1_CourseRender;
extern const float kFloat_0_25_CourseRender;
extern const float kFloat_0_03125_CourseRender;
extern const float kFloat_0_5_Effects;
extern const float kFloat_1_CourseRender;
extern const float kFloat_neg1_CourseRender;
extern const float kFloat_1500_Sky;
extern const float kFloat_235_61946_CourseRender;
extern const float kFloat_1884_9557_CourseRender;
extern const float kFloat_1178_0973_CourseRender;
extern const float kFloat_33_333332_CourseEffects;
extern const float kFloat_50_ShotEffect;
extern const float kFloat_1_CourseBox;
extern const float kPi;
extern const float kTwoPi_CourseRender;
extern const float kNearSkyRadius;
extern const float kFarSkyOpaqueBandV;
extern const float kNearSkyRingDropSecond;
extern const float kNearSkyRingDropFirst;
extern const float kNearSkyOpaqueBandV;
extern const float kNearSkyRingRise;

void ResourceManager_ReleaseId(void*, void*);
void PvsFxAuxiliaryResources_Dtor(void*, int);
void PvsFxAuxiliaryResources_Ctor(void*);
void PvsFxAuxiliaryResources_LoadCourseParameters(void*, void*);
void PvsFxAuxiliaryResources_AdvancePhase(void*, float);
void CubeResourceStorage_dtor(void*, int);
void Object_DeleteVirtual(void*, void*);
void __dla__FPv(void*);
void __dl__FPv(void*);
void* __nw__FUl(unsigned int);
void* __nwa__FUl(unsigned int);
void appFreeUni__FPv(void*);
unsigned long strlen(const char*);
char* strrchr(const char*, int);
void* String_FormatTemporary(const char*, ...);
void String_ReserveCapacity(void*, unsigned int);
void String_ReplaceRange(void*, unsigned int, unsigned int, const char*, const char*, const void*);
int ResourceManager_LoadBitmap(void*, const void*, int, int, int);
void* memcpy(void*, const void*, unsigned int);
void* Normalize__7WVectorFv(void*);
void* __ml__FRC7WVectorf(float*, void*, float);
float Math_CosApprox(float);
float Math_SinApprox(float);
void __ml__FfRC7WVector(void*, void*, float);
void __as__7WVectorFRC7WVector(void*, void*);
unsigned int Color_ScaleRGB(void*, float);
void VertexBuffer_Create(int, int, int, int, void**, int);
void IndexBuffer_Create(int, int, int, int, void**, int, int);
void Resource_GetOffsetPtr(void*, int, int, void*, int);
void IndexBuffer_GetOffsetPtr(void*, int, int, int*, int);
void GraphicsBuffer_Flush(void*);
void IndexBuffer_Flush(void*);
void Vector_CopyComponents(void*, void*);
unsigned char SharedDoc_GetCourseId();
void* ItemInventory_FindCourseData(void*, int);
int String_CompareCString(const char*, const char*);
void* CFxSequence_Create(void*, const char*, int, int, int);
void FxAttachment_Init(float*);
float Random_RangeFloat(int, float, float);
int rand();
void Matrix_SetAxisRotation(void*, float, signed char);
void* Matrix_TransformPoint(void*, void*, void*);
void __apl__7WVectorFRC7WVector(void*, void*);
void __mi__FRC7WVectorRC7WVector(void*, void*, void*);
void __pl__FRC7WVectorRC7WVector(void*, void*, void*);
float Magnitude__7WVectorCFv(void*);
float Math_SquareRootApprox(float);
void GetWindVector__5CWindFv(void* result, const void* self);
void Engine_SetRenderStateList(void*, int);
void Engine_SetTextureStageStateList(int, void*, int);
void Engine_SetTextureStageParameterList(int, void*, int);
void Engine_SetCachedRenderState(int, int);
void Engine_SetType7Resource(int);
void Camera_Apply(void*, float, float);
void* Camera_GetRenderState(void*);
void SetTransform(int, void*);
void Engine_SetTextureStageParameter(int, int, int);
void Engine_GetRenderState(int, int*);
void Engine_SetTextureStageResource(int, int);
void Engine_SetTexturePalette(int, int);
void* W3dAniSpr_Create(void*, float, float, void*, void*);
void* W3dAniSpr_GetFrameDescription(void*, int);
void __dt__13GeneTexBufferFv(void*, int);
void dtor_801B7D1C(void*, int);

} // extern "C"

struct RefHdr {
    CW_BITS2(unsigned int cnt : 16, unsigned int rest : 16)
};

// Release one graphics-resource reference and delete the resource at zero references.
static inline void ReleaseRef(void** slot) {
    RefHdr* p = (RefHdr*)*slot;
    if (p) {
        p->cnt = p->cnt - 1;
        if (p->cnt == 0)
            CubeResourceStorage_dtor(p, 1);
        *slot = 0;
    }
}

// ---- merged from pvsfxbox_sky_and_wind_particles.cpp ----
namespace ns0 {
struct NamedRectangle {
    float x, y, width, height;
};

struct F5CEntry {
    void* data;
    char* name;
    unsigned char flag;
    unsigned char pad[3];
    F5CEntry* next;
    F5CEntry* prev;
    void* hashNext;
};
// Overlay-owner layout for the named-rectangle pool (all pointers are Wii words).
struct NamedRectangleCacheView {
    void* overlay;
    float pixelWidth, pixelHeight;
    unsigned int color;
    unsigned int unknown10;
    float rotation;
    F5CEntry* activeTail;
    F5CEntry* iterationCursor;
    F5CEntry* allocationBlocks;
    F5CEntry* freeTail;
    unsigned int nodesPerBlock, bucketMask;
    int bucketCount;
    F5CEntry** buckets;
    char mutex[0x18];
};
typedef char NamedRectangleCacheMutexOffset[(int)&((NamedRectangleCacheView*)0)->mutex == 0x38 ? 1 : -1];

extern "C" {

extern const float kFloat_6_2831855;
extern float s_courseEffectsTwoPi;
extern unsigned char* g_sharedDoc;
extern unsigned int s_playerRankBandThresholds[3];
extern unsigned int __vt__10CGeneTex3D[9];
extern unsigned int s_playerRankThresholdRecords[64];
extern const float kFloat_0_CourseRender;
extern unsigned int g_resourceManager;
extern const float kFloat_0_TextDraw;
extern const float kFloat_0_5_SpriteEffects;
extern const float kNamedTextRotationEpsilon;
extern unsigned int g_view;

void OSLockMutex(void*);
void OSUnlockMutex(void*);
char* strcpy(char*, const char*);
unsigned int strcmp(const char*, const char*) throw();
void WOverlay_DrawTransformedRect(void*, unsigned int, void*, float, void*, unsigned int, unsigned int, unsigned int);
void Sprite_DrawRotatedQuadWithViewportRect(void*, unsigned int, void*, float, float, float, void*, unsigned int,
                                            unsigned int, unsigned int);

void OSInitMutex(void*);
void* appMallocUni__FUl(unsigned int);
void* WOverlay_Create(void*, void*, int);
int WOverlay_GetWidth(void*);
int WOverlay_GetHeight(void*);

// Find the first covering threshold among 56 entries; equality stays in that
// tier and values above all entries clamp to 55. Each iteration scans eight
// records; every successful comparison assigns result before leaving the loop.
// Table entries are five words each; p[4] is the threshold. The legacy extern
// extent [64] above is not the traversed extent: this walk requires 56*5 words.
int PlayerRank_FindTierForValue(unsigned int value) {
    unsigned int* p = s_playerRankThresholdRecords;
    int idx = 0;
    int result = 0;
    for (int outer = 0; outer < 7; ++outer) {
        if (value <= p[4]) {
            result = idx;
            break;
        }
        ++idx;
        if (value <= p[9]) {
            result = idx;
            break;
        }
        ++idx;
        if (value <= p[14]) {
            result = idx;
            break;
        }
        ++idx;
        if (value <= p[19]) {
            result = idx;
            break;
        }
        ++idx;
        if (value <= p[24]) {
            result = idx;
            break;
        }
        ++idx;
        if (value <= p[29]) {
            result = idx;
            break;
        }
        ++idx;
        if (value <= p[34]) {
            result = idx;
            break;
        }
        ++idx;
        if (value <= p[39]) {
            result = idx;
            break;
        }
        p += 40;
        ++idx;
    }
    // Original 0x801F5570: clamp after either a match or exhaustion.
done: // Keep the label to preserve the compiler's later local-type symbol numbering.
    if (idx >= 56)
        result = 55;
    return result;
}

} // extern "C"
} // namespace ns0

// The sky storage's generated-vertex owner. Its vtable pointer follows the byte
// member, so the two are spelled out rather than declared virtual.
class F61Inst {
public:
    unsigned char b;
    virtual void V0();
    virtual void V1();
    virtual unsigned char Get(int count);
};

// Release sky textures, generated geometry, effect managers, and retained
// graphics buffers.
void CPvsFxBox::ReleaseResources() {
    if (skyLoaded != 0) {
        // Phase 1: release each loaded layer ID and its owned source geometry.
        unsigned char* entry = (unsigned char*)this;
        for (int i = 0; i < 4; ++i) {
            unsigned int flags = *reinterpret_cast<unsigned int*>(entry + 0xC);
            if ((flags & 0x7FF) != 0) {
                ResourceManager_ReleaseId(g_resourceManager, (void*)(flags & 0x7FF));
                void* p = *reinterpret_cast<void**>(entry + 8);
                if (p != 0) {
                    // The original tests the pointer three times; the repeated
                    // tests are part of the matched code shape.
                    if (p != 0) {
                        if (p != 0) {
                            __dla__FPv((unsigned char*)p - 0x10);
                        }
                    }
                    *reinterpret_cast<void**>(entry + 8) = 0;
                }
                *reinterpret_cast<void**>(entry + 0x10) = 0;
                *reinterpret_cast<unsigned int*>(entry + 0xC) = 0;
            }
            entry += 0x60;
        }
        // Phase 2: particle bitmap and effect-manager owners.
        void* manager = g_resourceManager;
        if (manager != 0) {
            void* resource = particleBitmap;
            if (resource != 0) {
                ResourceManager_ReleaseId(manager, resource);
                particleBitmap = 0;
            }
        }
        particleCapacity = 0;
        activeParticleCount = 0;
        void* p = auxiliaryEffect;
        if (p != 0) {
            PvsFxAuxiliaryResources_Dtor(p, 1);
            auxiliaryEffect = 0;
        }
        p = courseEffects;
        if (p != 0) {
            // The original tests the pointer twice; both tests are part of the shape.
            if (p != 0) {
                ((void (*)(void*, int))((void**)*(void***)p)[2])(p, 1);
            }
            courseEffects = 0;
        }
        p = sceneEffects;
        if (p != 0) {
            if (p != 0) {
                ((void (*)(void*, int))((void**)*(void***)p)[2])(p, 1);
            }
            sceneEffects = 0;
        }
        // Phase 3: drop retained graphics-buffer references, then clear the loaded flag.
        ReleaseRef(&skyBuffers.farVertices);
        ReleaseRef(&skyBuffers.farIndices);
        ReleaseRef(&skyBuffers.nearVertices);
        ReleaseRef(&skyBuffers.nearIndices);
        skyLoaded = 0;
    }
}

// Initialize the course-environment storage: clear the queued time and the
// loaded latch, reset the effect and particle settings, and set up the sky
// geometry buffers.
CPvsFxBox* CPvsFxBox::Init() {
    F61Inst* inst = (F61Inst*)&geometry;
    const float zero = 0.0f;
    const float w = 1120.0f;
    const float d = 416.0f;

    const float s = 65.0f;
    auxiliaryEffect = 0;
    particleColor = 0x00FFFFFF;
    courseEffects = 0;
    sceneEffects = 0;
    inst->b = 0;
    geometry.vtbl = __vt__10CGeneTex3D;
    geometry.vertices = 0;
    skyBuffers.farVertices = 0;
    skyBuffers.nearVertices = 0;
    skyBuffers.farIndices = 0;
    skyBuffers.nearIndices = 0;
    particleSpawnRadius = w;
    particleCameraFadeDistance = 1600.0f;
    particleSpawnHeight = d;
    particleBitmap = 0;
    particleCapacity = 0;
    activeParticleCount = 0;
    spawnTimer = zero;
    unk_8B0 = s;
    skyVerticalOffset = zero;
    skyAngularPhaseScale = zero;
    skyTextureRepeatCount = 2;
    nearSkyTextureRepeatCount = 2;
    skyLoaded = 0;
    farSkySourceVertices = 0;
    nearSkySourceVertices = 0;
    upperSkySourceVertices = 0;
    lowerSkySourceVertices = 0;
    geometry.vertexCount = inst->Get(4); // allocate four generated vertices
    pendingSeconds = 0.0f;
    return this;
}

namespace ns0 {
extern "C" {

void* dtor_801F583C(void*, int);

// Create an overlay and initialize its mutex-protected named-rectangle cache.
void* OverlayPlayer_Init(void* self, void* arg) {
    unsigned char* obj = (unsigned char*)self;
    *reinterpret_cast<unsigned int*>(obj) = 0;
    *reinterpret_cast<unsigned int*>(obj + 0x34) = 0;
    OSInitMutex(obj + 0x38);
    {
        WInstanceLock lock(obj + 0x38);
        *reinterpret_cast<unsigned int*>(obj + 0x18) = 0;
        *reinterpret_cast<unsigned int*>(obj + 0x24) = 0;
        *reinterpret_cast<unsigned int*>(obj + 0x20) = 0;
        *reinterpret_cast<int*>(obj + 0x28) = 8;
        *reinterpret_cast<unsigned int*>(obj + 0x1c) = 0;
        *reinterpret_cast<int*>(obj + 0x30) = 8;
        if (*reinterpret_cast<int*>(obj + 0x28) > 0) {
            *reinterpret_cast<void**>(obj + 0x34) = appMallocUni__FUl(0x20);
            *reinterpret_cast<int*>(obj + 0x2c) = *reinterpret_cast<int*>(obj + 0x30) - 1;
            int i = 0;
            unsigned int off = 0;
            for (; i < *reinterpret_cast<int*>(obj + 0x30); ++i, off += 4)
                *(unsigned int*)((unsigned char*)*reinterpret_cast<void**>(obj + 0x34) + off) = 0;
        } else {
            *reinterpret_cast<void**>(obj + 0x34) = 0;
        }
    }
    try {
        void* instance = WOverlay_Create((void*)g_resourceManager, arg, 0);
        *reinterpret_cast<void**>(obj) = instance;
        if (instance) {
            *reinterpret_cast<float*>(obj + 0x4) = (float)WOverlay_GetWidth(instance);
            *reinterpret_cast<float*>(obj + 0x8) = (float)WOverlay_GetHeight(*reinterpret_cast<void**>(obj));
        }
        float zero = kFloat_0_TextDraw;
        *reinterpret_cast<int*>(obj + 0xc) = -1;
        *reinterpret_cast<float*>(obj + 0x14) = zero;
        return self;
    } catch (...) {
        dtor_801F583C(obj + 0x18, -1);
        throw;
    }
}

// Classify a value into one of four bands using three ascending thresholds.
int PlayerRank_FindThresholdBand(unsigned int value) {
    unsigned int* p = s_playerRankBandThresholds;
    int result = 0;
    if (value > *p++) {
        result = 1;
        if (value > *p++) {
            result = 2;
            if (value > *p)
                result = 3;
        }
    }
    return result;
}

} // extern "C"
} // namespace ns0

// Release the sky storage and its generated vertices, then free the storage
// itself when a positive delete flag is passed. This is the storage's cleanup,
// not the effect-box wrapper's destructor.
CPvsFxBox* CPvsFxBox::Destroy(int deallocate) {
    if (this != 0) {
        ReleaseResources();
        if ((char*)this + 0x9A0 != 0) {
            void* data = geometry.vertices;
            geometry.vtbl = __vt__10CGeneTex3D;
            if (data != 0) {
                __dla__FPv(data);
                geometry.vertices = 0;
            }
        }
        if (deallocate > 0)
            __dl__FPv(this);
    }
    return this;
}

namespace ns0 {
extern "C" {

#pragma push
#pragma use_lmw_stmw on
// Destroy the cache subobject, releasing allocated node blocks and the bucket table.
void* dtor_801F583C(void* self, int deallocate) {
    if (self != 0) {
        unsigned char* obj = (unsigned char*)self;
        {
            WInstanceLock lock(obj + 0x20);
            if (*reinterpret_cast<void**>(obj + 8) != 0) {
                struct Node {
                    char pad[0xC];
                    Node* prev;
                    Node* next;
                };
                Node* head;
                void* mutex = obj + 0x20;
                Node* node;
                while ((node = *reinterpret_cast<Node**>(obj + 8)) != 0) {
                    OSLockMutex(mutex);
                    node->next->prev = node->prev;
                    node->prev->next = node->next;
                    head = node->next;
                    if (head == node)
                        head = 0;
                    OSUnlockMutex(mutex);
                    *reinterpret_cast<void**>(obj + 8) = head;
                    appFreeUni__FPv(node);
                }
            }
            void* p = *reinterpret_cast<void**>(obj + 0x1C);
            if (p != 0) {
                appFreeUni__FPv(p);
                *reinterpret_cast<void**>(obj + 0x1C) = 0;
            }
        }
        if (deallocate > 0)
            __dl__FPv(self);
    }
    return self;
}
#pragma pop

// Return bit zero of the selected player record, or zero when no shared document exists.
int SharedDoc_IsPlayerFlag0Set(int index) {
    unsigned char* base = g_sharedDoc;
    if (base == 0)
        return 0;
    return *reinterpret_cast<unsigned int*>(base + index * 0x1D4 + 0x38) & 1;
}

// Hash a rectangle name from its length and endpoint bytes under the cache mutex.
static inline unsigned int NamedRectangle_HashBucket(unsigned char* obj, const char* text) {
    unsigned char* m = obj + 0x38;
    OSLockMutex(m);
    int len = strlen(text);
    unsigned int h =
        (len << 6) | (((unsigned char)text[0] & 0x2A) | ((unsigned char)text[len > 0 ? len - 1 : 0] & 0x15));
    unsigned int idx = h & *reinterpret_cast<unsigned int*>(obj + 0x2c);
    OSUnlockMutex(m);
    return idx;
}

// Find a name along one hash-bucket chain.
static inline unsigned char* NamedRectangle_FindNode(unsigned char* n, const char* text) {
    for (; n != 0; n = *reinterpret_cast<unsigned char**>(n + 0x14)) {
        if (strcmp(*(const char**)(n + 4), text) == 0)
            return n;
    }
    return 0;
}

// Look up a named rectangle payload under the cache mutex.
static inline void* NamedRectangle_Find(unsigned char* obj, const char* text) {
    WInstanceLock lock(obj + 0x38);
    if (text == 0 || *reinterpret_cast<void**>(obj + 0x34) == 0)
        return 0;
    unsigned int idx = NamedRectangle_HashBucket(obj, text);
    for (unsigned char* n = (unsigned char*)((void**)*reinterpret_cast<void**>(obj + 0x34))[idx]; n != 0;) {
        if (strcmp(*(const char**)(n + 4), text) == 0)
            return *reinterpret_cast<void**>(n);
        n = *reinterpret_cast<unsigned char**>(n + 0x14);
    }
    return 0;
}

// Draw a cached rectangle with horizontal/vertical alignment, scale, color, and optional rotation.
// x/y and scaled dimensions use viewport units; the cached rectangle uses overlay
// UV fractions. Each alignment mask must select one of its three listed values:
// the combined center|right or middle|bottom encodings leave xy uninitialized.
void NamedText_DrawAlignedScaled(void* self, const char* text, unsigned int flags, float x, float y, float scale) {
    unsigned char* obj = (unsigned char*)self;
    if ((*reinterpret_cast<int*>(obj + 0xc) >> 24) == 0 || text == 0)
        return;
    unsigned char* found = (unsigned char*)NamedRectangle_Find(obj, text);
    if (found == 0)
        return;
    float size[2];
    float xy[2];
    size[0] = ((NamedRectangle*)found)->width * (scale * *reinterpret_cast<float*>(obj + 0x4));
    switch ((int)(flags & kTextHorizontalAlignmentMask)) {
    case 0:
        xy[0] = x;
        break;
    case kTextAlignRight:
        xy[0] = x - size[0];
        break;
    case kTextAlignCenter:
        xy[0] = x - kFloat_0_5_SpriteEffects * size[0];
        break;
    }
    size[1] = ((NamedRectangle*)found)->height * (scale * *reinterpret_cast<float*>(obj + 0x8));
    switch ((int)(flags & kTextVerticalAlignmentMask)) {
    case 0:
        xy[1] = y;
        break;
    case kTextAlignBottom:
        xy[1] = y - size[1];
        break;
    case kTextAlignMiddle:
        xy[1] = y - kFloat_0_5_SpriteEffects * size[1];
        break;
    }
    // The original passes &xy and relies on size[] lying right behind it on the stack (a 4-float rectangle).
    int small = ((*reinterpret_cast<unsigned int*>(obj + 0x14) & 0x80000000)
                     ? -*reinterpret_cast<float*>(obj + 0x14)
                     : *reinterpret_cast<float*>(obj + 0x14)) < kNamedTextRotationEpsilon;
    if (small)
        ((void (*)(void*, unsigned int, void*, void*, unsigned int, unsigned int, float,
                   unsigned int))WOverlay_DrawTransformedRect)(
            *(void**)obj, g_view, found, xy, 0, *reinterpret_cast<unsigned int*>(obj + 0xc), kFloat_0_TextDraw, 0);
    else
        Sprite_DrawRotatedQuadWithViewportRect(*(void**)obj, g_view, found, xy[0] + kFloat_0_5_SpriteEffects * size[0],
                                               xy[1] + kFloat_0_5_SpriteEffects * size[1],
                                               *reinterpret_cast<float*>(obj + 0x14), xy, 0,
                                               *reinterpret_cast<unsigned int*>(obj + 0xc), 0);
}

// Test for game mode six with party submode zero.
int SharedDoc_IsMode6PartyMode0() {
    int result = 0;
    unsigned char* base = g_sharedDoc;
    if (AsCSharedDoc(base)->gameMode == 6) {
        if ((*(int*)&((CSharedDoc*)base)->partyGameMode) == 0)
            result = 1;
    }
    return result;
}

// Initialize this unit's stored full-turn angle constant.
void CourseEffects_InitTwoPi() {
    s_courseEffectsTwoPi = kFloat_6_2831855;
}

// Named-rectangle cache map relative to the overlay owner:
// +0 overlay resource; +4/+8 pixel dimensions; +0xC color; +0x14 rotation;
// +0x18 active circular list; +0x1C iteration cursor; +0x20 allocation-block list;
// +0x24 free-node list; +0x28 nodes/block; +0x2C bucket mask; +0x30 bucket count;
// +0x34 bucket array; +0x38 mutex. Nodes have payload/name/ownership byte at
// +0/+4/+8, circular links +0xC/+0x10 and hash-next +0x14 (sizeof=0x18).
// Each allocated block reserves its first node as a block-list header. Remaining
// nodes circulate between free and active lists; bucket chains borrow those nodes.
// Rectangle payloads are owned new allocations. A nonzero flag copies the name;
// zero borrows it. Cleanup shown here frees payloads/blocks/buckets, but does not
// separately free copied names: do not claim complete string cleanup from this code.

// Normalize a rectangle to overlay dimensions and register its payload and optional owned name.
// rect={pixel X,Y,width,height}; dimensions must be nonzero. name must be nonnull
// when flag!=0; with flag==0 it must outlive the cache entry. No duplicate removal.
void Register_NormalizedNamedRectangle(void* self, const char* name, const float* rect, unsigned char flag) {
    unsigned char* obj = (unsigned char*)self;
    float* normalizedRectangle = (float*)__nw__FUl(sizeof(NamedRectangle));
    void* rectanglePayload = normalizedRectangle;
    normalizedRectangle[0] = rect[0] / ((NamedRectangleCacheView*)obj)->pixelWidth;
    normalizedRectangle[1] = rect[1] / ((NamedRectangleCacheView*)obj)->pixelHeight;
    normalizedRectangle[2] = rect[2] / ((NamedRectangleCacheView*)obj)->pixelWidth;
    normalizedRectangle[3] = rect[3] / ((NamedRectangleCacheView*)obj)->pixelHeight;
    {
        WInstanceLock lock1(obj + 0x38);
        // Phase 1: reserve a free pooled node, allocating one node block if needed.
        F5CEntry* entry;
        {
            WInstanceLock lock2(obj + 0x38);
            if ((*(int*)&((NamedRectangleCacheView*)obj)->freeTail) == 0) {
                unsigned int nodesPerBlock = ((NamedRectangleCacheView*)obj)->nodesPerBlock;
                F5CEntry* nodes = (F5CEntry*)appMallocUni__FUl(nodesPerBlock * sizeof(F5CEntry));
                void* poolMutex = obj + (int)&((NamedRectangleCacheView*)0)->mutex;
                F5CEntry* first = nodes;
                F5CEntry* allocationTail = ((NamedRectangleCacheView*)obj)->allocationBlocks;
                OSLockMutex(poolMutex);
                if (allocationTail == 0) {
                    first->next = first;
                    first->prev = first;
                    OSUnlockMutex(poolMutex);
                    allocationTail = first;
                } else {
                    first->prev = allocationTail;
                    first->next = allocationTail->next;
                    allocationTail->next = first;
                    first->next->prev = first;
                    OSUnlockMutex(poolMutex);
                }
                ((NamedRectangleCacheView*)obj)->allocationBlocks = allocationTail;
                F5CEntry* freeNodeCursor = nodes;
                int freeNodeIndex = 1;
                if (freeNodeIndex < (*(int*)&((NamedRectangleCacheView*)obj)->nodesPerBlock)) {
                    ++freeNodeCursor;
                    for (; freeNodeIndex < (*(int*)&((NamedRectangleCacheView*)obj)->nodesPerBlock); ++freeNodeIndex) {
                        F5CEntry* freeTail = ((NamedRectangleCacheView*)obj)->freeTail;
                        OSLockMutex(poolMutex);
                        if (freeTail == 0) {
                            freeNodeCursor->next = freeNodeCursor;
                            freeNodeCursor->prev = freeNodeCursor;
                            OSUnlockMutex(poolMutex);
                            freeTail = freeNodeCursor;
                        } else {
                            freeNodeCursor->prev = freeTail;
                            freeNodeCursor->next = freeTail->next;
                            freeTail->next = freeNodeCursor;
                            freeNodeCursor->next->prev = freeNodeCursor;
                            OSUnlockMutex(poolMutex);
                        }
                        ((NamedRectangleCacheView*)obj)->freeTail = freeTail;
                        ++freeNodeCursor;
                    }
                }
            }
            entry = ((NamedRectangleCacheView*)obj)->freeTail;
            void* poolMutex = obj + (int)&((NamedRectangleCacheView*)0)->mutex;
            OSLockMutex(poolMutex);
            entry->prev->next = entry->next;
            entry->next->prev = entry->prev;
            F5CEntry* remainingFreeTail = entry->prev;
            if (remainingFreeTail == entry)
                remainingFreeTail = 0;
            OSUnlockMutex(poolMutex);
            ((NamedRectangleCacheView*)obj)->freeTail = remainingFreeTail;
        }
        // Phase 2: bind the normalized rectangle payload and copied/borrowed name.
        entry->data = rectanglePayload;
        entry->flag = flag;
        if (flag) {
            unsigned int n = strlen(name) + 1;
            entry->name = (char*)appMallocUni__FUl(n);
            strcpy(entry->name, name);
        } else {
            entry->name = (char*)name;
        }
        // Phase 3: link into the hash bucket, then publish on the active circular list.
        if (name != 0) {
            void* poolMutex = obj + (int)&((NamedRectangleCacheView*)0)->mutex;
            OSLockMutex(poolMutex);
            unsigned int nameLength = strlen(name);
            unsigned int bucketIndex =
                ((nameLength << 6) | (((unsigned char)name[0] & 0x2A) |
                                      ((unsigned char)name[(nameLength - 1) & -(nameLength != 0)] & 0x15))) &
                ((NamedRectangleCacheView*)obj)->bucketMask;
            OSUnlockMutex(poolMutex);
            OSLockMutex(poolMutex);
            entry->hashNext = (((NamedRectangleCacheView*)obj)->buckets)[bucketIndex];
            (((NamedRectangleCacheView*)obj)->buckets)[bucketIndex] = entry;
            OSUnlockMutex(poolMutex);
        }
        F5CEntry* activeTail = ((NamedRectangleCacheView*)obj)->activeTail;
        void* poolMutex = obj + (int)&((NamedRectangleCacheView*)0)->mutex;
        OSLockMutex(poolMutex);
        if (activeTail == 0) {
            entry->next = entry;
            entry->prev = entry;
            OSUnlockMutex(poolMutex);
        } else {
            entry->prev = activeTail;
            entry->next = activeTail->next;
            activeTail->next = entry;
            entry->next->prev = entry;
            OSUnlockMutex(poolMutex);
            entry = activeTail;
        }
        ((NamedRectangleCacheView*)obj)->activeTail = entry;
    }
}

#pragma push
#pragma use_lmw_stmw on
// Return the next active payload while advancing the cache iterator under its mutex.
inline void* NamedRectangle_AdvanceIterator(unsigned char* obj) {
    void* mutex = obj + 0x38;
    OSLockMutex(mutex);
    F5CEntry* node = *(F5CEntry**)(obj + 0x1c);
    if (node != 0) {
        F5CEntry* next = node->prev;
        void* result = node->data;
        if (next == *(F5CEntry**)(obj + 0x18))
            next = 0;
        *(F5CEntry**)(obj + 0x1c) = next;
        OSUnlockMutex(mutex);
        return result;
    }
    OSUnlockMutex(mutex);
    return 0;
}
// Unlink a node under the mutex; return its predecessor or null for a singleton.
inline F5CEntry* NamedRectangle_UnlinkNode(F5CEntry* node, void* mutex) {
    OSLockMutex(mutex);
    node->prev->next = node->next;
    node->next->prev = node->prev;
    F5CEntry* next = node->prev;
    if (next == node)
        next = 0;
    OSUnlockMutex(mutex);
    return next;
}
// Insert after tail under the mutex; return the existing tail or a new singleton tail.
inline F5CEntry* NamedRectangle_LinkNode(F5CEntry* node, F5CEntry* tail, void* mutex) {
    OSLockMutex(mutex);
    if (tail == 0) {
        node->next = node;
        node->prev = node;
        OSUnlockMutex(mutex);
        return node;
    }
    node->prev = tail;
    node->next = tail->next;
    tail->next = node;
    node->next->prev = node;
    OSUnlockMutex(mutex);
    return tail;
}
// Release the overlay, rectangle payloads, pooled nodes, and hash buckets.
void* OverlayPlayer_Destroy(void* self, int deallocate) {
    // Phase 1: release the overlay resource and every active rectangle payload.
    if (self != 0) {
        unsigned char* obj = (unsigned char*)self;
        void* global = (void*)g_resourceManager;
        if (global != 0 && *(void**)obj != 0) {
            Object_DeleteVirtual(global, *(void**)obj);
            *(void**)obj = 0;
        }
        void* entry;
        {
            WInstanceLock lock(obj + 0x38);
            *(void**)(obj + 0x1c) = *(void**)(obj + 0x18);
            entry = NamedRectangle_AdvanceIterator(obj);
        }
        while (entry != 0) {
            if (entry != 0)
                __dl__FPv(entry);
            entry = NamedRectangle_AdvanceIterator(obj);
        }
        {
            // Phase 2: splice active nodes back to the free list and clear bucket links.
            WInstanceLock lock(obj + 0x38);
            if (*(F5CEntry**)(obj + 0x18) != 0 && *(F5CEntry**)(obj + 0x24) != 0) {
                (*(F5CEntry**)(obj + 0x18))->next->prev = (*(F5CEntry**)(obj + 0x24))->prev;
                (*(F5CEntry**)(obj + 0x24))->prev->next = (*(F5CEntry**)(obj + 0x18))->next;
                (*(F5CEntry**)(obj + 0x24))->prev = *(F5CEntry**)(obj + 0x18);
                (*(F5CEntry**)(obj + 0x18))->next = *(F5CEntry**)(obj + 0x24);
                *(void**)(obj + 0x18) = 0;
            } else if (*(F5CEntry**)(obj + 0x18) != 0) {
                void* mutex = obj + 0x38;
                F5CEntry* node;
                while ((node = *(F5CEntry**)(obj + 0x18)) != 0) {
                    F5CEntry* newHead = NamedRectangle_UnlinkNode(node, mutex);
                    *(F5CEntry**)(obj + 0x24) =
                        NamedRectangle_LinkNode(*(F5CEntry**)(obj + 0x18), *(F5CEntry**)(obj + 0x24), mutex);
                    *(F5CEntry**)(obj + 0x18) = newHead;
                }
            }
            for (int i = 0; i < *(int*)(obj + 0x30); ++i)
                ((void**)*(void**)(obj + 0x34))[i] = 0;
        }
        // Phase 3: free allocation blocks and the bucket array, then optionally self.
        if (obj + 0x18 != 0) {
            WInstanceLock lock(obj + 0x38);
            if (*(void**)(obj + 0x20) != 0) {
                void* mutex = obj + 0x38;
                F5CEntry* node;
                while ((node = *(F5CEntry**)(obj + 0x20)) != 0) {
                    F5CEntry* newHead = NamedRectangle_UnlinkNode(node, mutex);
                    *(F5CEntry**)(obj + 0x20) = newHead;
                    appFreeUni__FPv(node);
                }
            }
            void* table = *(void**)(obj + 0x34);
            if (table != 0) {
                appFreeUni__FPv(table);
                *(void**)(obj + 0x34) = 0;
            }
        }
        if (deallocate > 0)
            __dl__FPv(self);
    }
    return self;
}
#pragma pop

} // extern "C"

} // namespace ns0

// ---- merged from pvsfxbox_sky_and_wind_particles.cpp ----

struct TmpString {
    unsigned int flags;
    unsigned int len;
    char* ptr;
};

struct V3 {
    float x, y, z;
    // Preserve uninitialized vector storage; callers fill components explicitly.
    V3() {}
    static void* operator new[](size_t);
};
// Allocate the source-geometry vector array through the original new[] entry point.
inline void* V3::operator new[](size_t size) {
    return __nwa__FUl(size);
}

struct FlagSource {
    virtual void m0();
    virtual void m1();
    virtual void m2();
    virtual void m3();
    virtual void m4();
    virtual void m5();
    virtual void m6();
    virtual void get(int, int, int*);
};

struct Vtx {
    float pos[3];
    unsigned char r, g, b, a;
    float u, v;
};

// Append one suffix to the small-string resource path while preserving allocator temporaries.
static inline void StrAppendImpl(TmpString& s, const char* p, int& a0, int& a1) {
    const char* start = p;
    unsigned int n = !(s.flags >> 31) ? (*(unsigned char*)&s.flags & 0x7f) : s.len;
    *(char*)&a1 = *(char*)&a0;
    unsigned int len = strlen(start);
    String_ReplaceRange(&s, n, 0, start, start + len, (const char*)&a1);
}

// Return inline or heap character storage from the resource-path string.
static inline const char* StrCStr(TmpString& s) {
    return !(s.flags >> 31) ? (char*)&s.flags + 1 : s.ptr;
}

// Return positive one for a set corner bit and negative one otherwise.
static inline float Sign(int c) {
    return c ? kFloat_1_CourseRender : kFloat_neg1_CourseRender;
}

struct V3i {
    int x, y, z;
};

// Copy a three-component vector through the original word view.
static inline void CopyV3(V3& d, const void* src) {
    *(V3i*)&d = *(const V3i*)src;
}

// Resolve ring fallbacks and cap textures; successful ring fallbacks force one repeat.
// effectBox and resource manager are required; borrowed stem lives through calls.
// Publish each successful retained ID in both its draw cache and layer flags;
// on miss clear ONLY layer flags, leaving that draw-cache word unchanged. Ring
// fallback failure leaves caller's repeat count unchanged; caps have no fallback.
// Partial success is supported: geometry later skips layer IDs that remain zero.
// ReleaseResources owns ResourceManager_ReleaseId of each live low-11-bit ID.
static inline void LoadSkyLayerTextures(CPvsFxBox* box, TmpString& resourceStem, int& farRepeatCount,
                                        int& nearRepeatCount) {
    // Ring textures request flag 0x80000000; caps use zero. Only a successful
    // fallback forces repeat=1. Path formats are supplied by skyLayerResourceStrings.
    int textureId = ResourceManager_LoadBitmap(
        g_resourceManager, String_FormatTemporary(skyLayerResourceStrings.farSkyFormat, StrCStr(resourceStem)),
        0x80000000, 0, 0);
    if (!textureId) {
        textureId = ResourceManager_LoadBitmap(
            g_resourceManager,
            String_FormatTemporary(skyLayerResourceStrings.fallbackFarSkyFormat, StrCStr(resourceStem)), 0x80000000, 0,
            0);
        if (textureId)
            farRepeatCount = 1;
    }
    if (textureId) {
        box->skyBuffers.farTexture = (void*)textureId;
        box->farSkyResourceFlags = (textureId & kResourceIdMask) | kSkyLayerRenderFlags;
    } else {
        box->farSkyResourceFlags = 0;
    }

    textureId = ResourceManager_LoadBitmap(
        g_resourceManager, String_FormatTemporary(skyLayerResourceStrings.nearSkyFormat, StrCStr(resourceStem)),
        0x80000000, 0, 0);
    if (!textureId) {
        textureId = ResourceManager_LoadBitmap(
            g_resourceManager,
            String_FormatTemporary(skyLayerResourceStrings.fallbackNearSkyFormat, StrCStr(resourceStem)), 0x80000000, 0,
            0);
        if (textureId)
            nearRepeatCount = 1;
    }
    if (textureId) {
        box->skyBuffers.nearTexture = (void*)textureId;
        box->nearSkyResourceFlags = (textureId & kResourceIdMask) | kSkyLayerRenderFlags;
    } else {
        box->nearSkyResourceFlags = 0;
    }

    textureId = ResourceManager_LoadBitmap(
        g_resourceManager, String_FormatTemporary(skyLayerResourceStrings.upperSkyFormat, StrCStr(resourceStem)), 0, 0,
        0);
    if (textureId) {
        box->skyBuffers.upperTexture = (void*)textureId;
        box->upperSkyResourceFlags = (textureId & kResourceIdMask) | kSkyCapRenderFlags;
    } else {
        box->upperSkyResourceFlags = 0;
    }

    textureId = ResourceManager_LoadBitmap(
        g_resourceManager, String_FormatTemporary(skyLayerResourceStrings.lowerSkyFormat, StrCStr(resourceStem)), 0, 0,
        0);
    if (textureId) {
        box->skyBuffers.lowerTexture = (void*)textureId;
        box->lowerSkyResourceFlags = (textureId & kResourceIdMask) | kSkyCapRenderFlags;
    } else {
        box->lowerSkyResourceFlags = 0;
    }
}

// Load weather-specific sky textures and construct camera-relative ring/cap geometry.
// text is a borrowed resource-name stem. Clear uses it directly; weather variants
// append a separator and cloud/rain suffix. Far/near fallbacks force one repeat.
// Caller-provided scale is stored as skyAngularPhaseScale and also offsets ring angular phase;
// actual ring radii use the separate fixed world-unit constants below.
// Requires initialized environment storage, live resource/shared-doc globals and
// NUL-terminated text. Release old loaded state first, then set loaded=1 even if
// every texture misses. No status/output argument or rollback; void return does
// not promise complete sky resources. Allocation/creation errors are delegated
// to callees and may leave partial owners; ReleaseResources is the cleanup path.
// Ring/cap source arrays use new[] and graphics buffers hold packed references;
// do not free either with ResourceManager_ReleaseId. Empty layers are skipped.
// Repeat counts come from s_defaultSkyTextureRepeats or successful fallback=1;
// stored values clamp to >=1, but phase division uses the raw counts. Those raw
// counts must therefore be nonzero; changing only the stored clamp is insufficient.
void CPvsFxBox::LoadSkyResources(const char* text, float scale) {
    // Phase scale is dimensionless; ring radii below are separate world-unit inputs.
    float angularPhaseScale = scale;
    TmpString resourceStem;
    V3 farRingPoint, nearRingPoint, scaledRingPoint, unitRingPoint;
    float phaseTurnsPerRing[2];
    unsigned int color;
    int geometryPathFlag;
    Vtx* farVertices;
    unsigned short* farIndices;
    Vtx* nearVertices;
    unsigned short* nearIndices;
    // Empty allocator-tag bytes preserve the native string ABI, not resource state.
    int stemAllocatorSource;
    int stemAllocatorArgument;
    int cloudSeparatorAllocatorSource;
    int cloudSeparatorAllocatorArgument;
    int cloudSuffixAllocatorSource;
    int cloudSuffixAllocatorArgument;
    int rainSeparatorAllocatorSource;
    int rainSeparatorAllocatorArgument;
    int rainSuffixAllocatorSource;
    int rainSuffixAllocatorArgument;
    int farRepeatCount, nearRepeatCount;
    unsigned int len;
    // Phase 1: discard old resources, select weather suffix and resolve four textures.
    if (skyLoaded)
        ReleaseResources();
    skyLoaded = 1;
    int* ptab = s_defaultSkyTextureRepeats;
    farRepeatCount = *ptab++;
    nearRepeatCount = *ptab;

    resourceStem.flags = 0;
    resourceStem.len = 0;
    resourceStem.ptr = 0;
    len = strlen(text);
    String_ReserveCapacity(&resourceStem, len);
    *(char*)&stemAllocatorArgument = *(char*)&stemAllocatorSource;
    String_ReplaceRange(&resourceStem, 0, 0, text, text + len, (const char*)&stemAllocatorArgument);

    switch (((SharedWeatherView*)g_sharedDoc)->weatherVariant) {
    case kWeatherCloud: // Append the cloudy-weather resource suffix.
        StrAppendImpl(resourceStem, skyLayerResourceStrings.variantSeparator, cloudSeparatorAllocatorSource,
                      cloudSeparatorAllocatorArgument);
        StrAppendImpl(resourceStem, skyLayerResourceStrings.cloudVariant, cloudSuffixAllocatorSource,
                      cloudSuffixAllocatorArgument);
        break;
    case kWeatherRain: // Append the rainy-weather resource suffix.
        StrAppendImpl(resourceStem, skyLayerResourceStrings.variantSeparator, rainSeparatorAllocatorSource,
                      rainSeparatorAllocatorArgument);
        StrAppendImpl(resourceStem, skyLayerResourceStrings.rainVariant, rainSuffixAllocatorSource,
                      rainSuffixAllocatorArgument);
        break;
    }

    LoadSkyLayerTextures(this, resourceStem, farRepeatCount, nearRepeatCount);

    {
        (*(FlagSource**)g_resourceManager)->get(0x1f, 2, &geometryPathFlag);
    }
    skyAngularPhaseScale = angularPhaseScale;
    int clampedFarRepeatCount;
    if (farRepeatCount >= 1)
        clampedFarRepeatCount = farRepeatCount;
    else
        clampedFarRepeatCount = 1;
    skyTextureRepeatCount = clampedFarRepeatCount;
    nearSkyTextureRepeatCount = nearRepeatCount >= 1 ? nearRepeatCount : 1;
    phaseTurnsPerRing[0] = angularPhaseScale / (float)farRepeatCount;
    phaseTurnsPerRing[1] = angularPhaseScale / (float)nearRepeatCount;

    // Phase 2: this capability path builds 33 seam-closed ring points or four cap
    // corners. Ring angle = 2*pi*(segment/32 - scale/repeats), in radians.
    if (geometryPathFlag == 0) {
        int layerIndex = kFarRingLayer;
        char* layerAddress = (char*)this;
        float* ringPhaseCursor = phaseTurnsPerRing;
        // The entry cursor visits four layers; the phase cursor supplies values
        // only to the first two. Caps never dereference it. Keep the byte walk
        // to preserve the original scheduling of entry writes and vector calls.
        for (; layerIndex < kSkyLayerCount; layerIndex++, layerAddress += sizeof(SkyLayerView), ringPhaseCursor++) {
            if (!(((SkyLayerView*)layerAddress)->resourceFlags & kResourceIdMask))
                continue;
            switch (layerIndex) {
            case kFarRingLayer:
            case kNearRingLayer: {
                V3* arr = new V3[kSkyRingPointCount];
                ((SkyLayerView*)layerAddress)->sourceVertices = arr;
                float ringRadius = (layerIndex == kFarRingLayer) ? kFloat_1500_Sky : kNearSkyRadius;
                for (int i = 0; i <= kSkyRingSegments; i++) {
                    float cosAngle = Math_CosApprox(kTwoPi_CourseRender *
                                                    ((float)i * kFloat_0_03125_CourseRender - *ringPhaseCursor));
                    float sinAngle = Math_SinApprox(kTwoPi_CourseRender *
                                                    ((float)i * kFloat_0_03125_CourseRender - *ringPhaseCursor));
                    unitRingPoint.x = cosAngle;
                    unitRingPoint.y = kFloat_0_CourseRender;
                    unitRingPoint.z = sinAngle;
                    __ml__FfRC7WVector(&scaledRingPoint, &unitRingPoint, ringRadius);
                    __as__7WVectorFRC7WVector((char*)((SkyLayerView*)layerAddress)->sourceVertices + i * 12,
                                              &scaledRingPoint);
                }
                break;
            }
            case kUpperCapLayer:
            case kLowerCapLayer: {
                V3* arr = new V3[kSkyCapCornerCount];
                ((SkyLayerView*)layerAddress)->sourceVertices = arr;
                for (int m = 0; m < 4; m++) {
                    ((V3*)((SkyLayerView*)layerAddress)->sourceVertices)[m].x = kFloat_1500_Sky * Sign(m & 1);
                    ((V3*)((SkyLayerView*)layerAddress)->sourceVertices)[m].z = kFloat_1500_Sky * Sign(m & 2);
                    ((V3*)((SkyLayerView*)layerAddress)->sourceVertices)[m].y =
                        kFloat_0_5_Effects *
                        (kPi * (kFloat_0_25_CourseRender * (kFloat_1500_Sky * Sign(layerIndex == kUpperCapLayer))));
                }
                break;
            }
            }
        }
        // Phase 3: expand far ring into four height rows, transparent at top/bottom;
        // three index bands join adjacent rows. Near ring uses three fully opaque rows.
        color = Color_ScaleRGB((void*)particleColor, AsCGolfDoc((*(int*)&((CSharedDoc*)g_sharedDoc)->golfDoc))->bright);

        if (farSkySourceVertices != 0) {
            ReleaseRef(&skyBuffers.farVertices);
            VertexBuffer_Create(kFarSkyVertexBytes, 0, kSkyVertexFormatXYZ_RGBA_UV, 0, (void**)&skyBuffers.farVertices,
                                0);
            {
                Resource_GetOffsetPtr(skyBuffers.farVertices, 0, 0, (int*)&farVertices, 0);
                unsigned int red = (color >> 16) & 0xff;
                unsigned int green = (color >> 8) & 0xff;
                unsigned int blue = color & 0xff;
                for (int j = 0; j <= kSkyRingSegments; j++) {
                    CopyV3(farRingPoint, (char*)farSkySourceVertices + j * 0xc);
                    for (int i = 0; i < 4; i++) {
                        if (i == 0) {
                            farRingPoint.y += kFloat_1178_0973_CourseRender;
                            farVertices[j * 4 + 0].v = kFloat_0_CourseRender;
                            farVertices[j * 4 + 0].a = 0;
                        } else if (i == 1) {
                            farRingPoint.y -= kFloat_235_61946_CourseRender;
                            farVertices[j * 4 + 1].v = kFloat_0_1_CourseRender;
                            farVertices[j * 4 + 1].a = 0xff;
                        } else if (i == 2) {
                            farRingPoint.y -= kFloat_1884_9557_CourseRender;
                            farVertices[j * 4 + 2].v = kFarSkyOpaqueBandV;
                            farVertices[j * 4 + 2].a = 0xff;
                        } else {
                            farRingPoint.y -= kFloat_235_61946_CourseRender;
                            farVertices[j * 4 + 3].v = kFloat_1_CourseRender;
                            farVertices[j * 4 + 3].a = 0;
                        }
                        Vector_CopyComponents(&farVertices[j * 4 + i], &farRingPoint);
                        farVertices[j * 4 + i].u = (float)(j * skyTextureRepeatCount) * kFloat_0_03125_CourseRender;
                        farVertices[j * 4 + i].r = red;
                        farVertices[j * 4 + i].g = green;
                        farVertices[j * 4 + i].b = blue;
                    }
                }
                GraphicsBuffer_Flush(skyBuffers.farVertices);
            }

            // Join each adjacent pair of the four far height rows as a separate strip.
            int n = 0;
            ReleaseRef(&skyBuffers.farIndices);
            IndexBuffer_Create(kFarSkyIndexBytes, 0, kSkyIndexFormat, 0, (void**)&skyBuffers.farIndices, 0, 0);
            {
                IndexBuffer_GetOffsetPtr(skyBuffers.farIndices, 0, 0, (int*)&farIndices, 0);
                for (int m = 0; m <= kSkyRingSegments; m++) {
                    farIndices[n] = (unsigned short)(4 * m);
                    farIndices[n + 1] = 4 * m + 1;
                    n += 2;
                }
                for (int m = 0; m <= kSkyRingSegments; m++) {
                    farIndices[n] = (unsigned short)(4 * m + 1);
                    farIndices[n + 1] = (unsigned short)(4 * m + 2);
                    n += 2;
                }
                for (int m = 0; m <= kSkyRingSegments; m++) {
                    farIndices[n] = (unsigned short)(4 * m + 2);
                    farIndices[n + 1] = (unsigned short)(4 * m + 3);
                    n += 2;
                }
                IndexBuffer_Flush(skyBuffers.farIndices);
            }
        }

        // Phase 4: build the near-strip vertices and indices. Preserve the original
        // far repeat-count use for U and the final flush of the far index buffer, even in this path.
        if (nearSkySourceVertices != 0) {
            ReleaseRef(&skyBuffers.nearVertices);
            VertexBuffer_Create(kNearSkyBufferBytes, 0, kSkyVertexFormatXYZ_RGBA_UV, 0,
                                (void**)&skyBuffers.nearVertices, 0);
            {
                Resource_GetOffsetPtr(skyBuffers.nearVertices, 0, 0, (int*)&nearVertices, 0);
                unsigned int red = (color >> 16) & 0xff;
                unsigned int green = (color >> 8) & 0xff;
                unsigned int blue = color & 0xff;
                for (int j = 0; j <= kSkyRingSegments; j++) {
                    CopyV3(nearRingPoint, (char*)nearSkySourceVertices + j * 0xc);
                    for (int i = 0; i < 3; i++) {
                        if (i == 0) {
                            nearRingPoint.y += kNearSkyRingRise;
                            nearVertices[j * 3 + 0].v = kFloat_0_1_CourseRender;
                        } else if (i == 1) {
                            nearRingPoint.y -= kNearSkyRingDropFirst;
                            nearVertices[j * 3 + 1].v = kNearSkyOpaqueBandV;
                        } else {
                            nearRingPoint.y -= kNearSkyRingDropSecond;
                            nearVertices[j * 3 + 2].v = kFloat_1_CourseRender;
                        }
                        Vector_CopyComponents(&nearVertices[j * 3 + i], &nearRingPoint);
                        nearVertices[j * 3 + i].u =
                            (float)((0x20 - j) * skyTextureRepeatCount) * kFloat_0_03125_CourseRender;
                        nearVertices[j * 3 + i].a = 0xff;
                        nearVertices[j * 3 + i].r = red;
                        nearVertices[j * 3 + i].g = green;
                        nearVertices[j * 3 + i].b = blue;
                    }
                }
                GraphicsBuffer_Flush(skyBuffers.nearVertices);
            }

            // Join the three near height rows in two strips; allocation size is retained.
            int n = 0;
            ReleaseRef(&skyBuffers.nearIndices);
            IndexBuffer_Create(kNearSkyBufferBytes, 0, kSkyIndexFormat, 0, (void**)&skyBuffers.nearIndices, 0, 0);
            {
                IndexBuffer_GetOffsetPtr(skyBuffers.nearIndices, 0, 0, (int*)&nearIndices, 0);
                for (int m = 0; m <= kSkyRingSegments; m++) {
                    nearIndices[n] = 3 * m;
                    nearIndices[n + 1] = 3 * m + 1;
                    n += 2;
                }
                for (int m = 0; m <= kSkyRingSegments; m++) {
                    nearIndices[n] = 3 * m + 1;
                    nearIndices[n + 1] = 3 * m + 2;
                    n += 2;
                }
                IndexBuffer_Flush(skyBuffers.farIndices);
            }
        }
    }
    if (resourceStem.flags >> 31)
        appFreeUni__FPv(resourceStem.ptr);
}

// Release the variant particle bitmap and clear particle counts.
void CPvsFxBox::ReleaseParticleResource() {
    void* resource;
    if (g_resourceManager != 0) {
        resource = particleBitmap;
        if (resource != 0) {
            ResourceManager_ReleaseId(g_resourceManager, resource);
            particleBitmap = 0;
        }
    }
    particleCapacity = 0;
    activeParticleCount = 0;
}

// ---- merged from pvsfxbox_sky_and_wind_particles.cpp ----

struct MWStringStorageView {
    unsigned char data[12];
};

// Views of the course data that the auxiliary effect reads. They describe the
// course resource records, not fields of the effect box.
struct CourseEntryRecord {
    unsigned char type;
    unsigned char padding[0x23];
    unsigned char effectParameters[0x18];
};

struct Resource801F86DC {
    unsigned char prefix[0x78];
    int count;
    unsigned char paddingBeforeEntries[0x24];
    CourseEntryRecord* entries;
};

struct GolfDocView {
    unsigned char prefix[8];
    Resource801F86DC* resource;
};

struct SharedDocView {
    unsigned char prefix[0xB90];
    GolfDocView* manager;
};

struct Object801F86DC {
    unsigned char prefix[0x4C];
    // Construct an owned auxiliary course-effect resource wrapper.
    Object801F86DC() {
        PvsFxAuxiliaryResources_Ctor(this);
    }
};

// Empty effect hook retained by the original interface.
extern "C" void CourseEffects_EmptyHook() {}

// Construction view of CPvsFxBox; canonical members: classes/CPvsFxBox.h.
// Constructor preserves integer stores and attachment-sentinel addresses.
struct CPvsFxBoxConstructorView {
    void* vtable;
    unsigned char baseState[0x378];
    int sprayAttachmentCount;
    void* sprayAttachmentRoot;
    void* sprayAttachmentFirst;
    int sequenceAttachmentCount;
    void* sequenceAttachmentRoot;
    void* sequenceAttachmentFirst;
    // Preserve byte-addressed stores and compiler-generated cleanup symbol names;
    // introducing an inline address helper renumbered a later local destructor.
#define ATTACHMENT_ADDRESS(field) ((char*)this + (int)&((CPvsFxBoxConstructorView*)0)->field)
    // Construct an effect box and two empty attachment trees with root sentinels.
    CPvsFxBoxConstructorView() {
        ((CFxBox*)this)->__ct();
        *(void**)this = (void*)__vt__9CPvsFxBox;
        *(int*)ATTACHMENT_ADDRESS(sprayAttachmentCount) = 0;
        *(void**)ATTACHMENT_ADDRESS(sprayAttachmentRoot) = 0;
        *(void**)ATTACHMENT_ADDRESS(sprayAttachmentFirst) = ATTACHMENT_ADDRESS(sprayAttachmentRoot);
        *(int*)ATTACHMENT_ADDRESS(sequenceAttachmentCount) = 0;
        *(void**)ATTACHMENT_ADDRESS(sequenceAttachmentRoot) = 0;
        *(void**)ATTACHMENT_ADDRESS(sequenceAttachmentFirst) = ATTACHMENT_ADDRESS(sequenceAttachmentRoot);
    }
#undef ATTACHMENT_ADDRESS
};

struct FxBoxView {
    virtual ~FxBoxView();
};

// Load the course sky script and attach its spray or expanded sequence to the camera.
void CPvsFxBox::ReloadCourseEffects() {
    // Phase 1: remove the old manager; only clear-weather courses supply this sky script.
    char* record;
    char* dot;
    void* mgr;
    FxBoxView* old = (*(FxBoxView**)&courseEffects);
    if (old != 0) {
        delete old;
        courseEffects = 0;
    }
    if (((SharedWeatherView*)g_sharedDoc)->weatherVariant != 0)
        return;
    unsigned char flag = 0;
    if (SharedDoc_GetCourseId() > 0x7f && SharedDoc_GetCourseId() != 0xfd)
        flag = 1;
    int index;
    if (flag != 0) {
        index = SharedDoc_GetCourseId() - 0x80;
    } else {
        index = SharedDoc_GetCourseId();
    }
    record =
        (char*)ItemInventory_FindCourseData(((char*)&AsCSharedDoc(g_sharedDoc)->itemInventory), index | 0x28000000);
    if (record == 0)
        return;
    if (record[IFF_OFFSET(IFF_STRUCT::sCourse, skyEffect)] == 0)
        return;
    if ((int)strlen(record + IFF_OFFSET(IFF_STRUCT::sCourse, skyEffect)) < 5)
        return;
    dot = strrchr(record + IFF_OFFSET(IFF_STRUCT::sCourse, skyEffect), 0x2E);
    if (dot == 0)
        return;
    if (strlen(dot) < 4)
        return;
    mgr = courseEffects;
    if (mgr == 0) {
        mgr = new CPvsFxBoxConstructorView();
        courseEffects = mgr;
        ((CFxBox*)mgr)->SetResourceManager((void*)g_resourceManager);
        ((CFxBox*)courseEffects)->SetCameraTransform(s_cameraTransform);
        *(float*)((char*)&((EffectUpdateView*)courseEffects)->updateStep) = kFloat_33_333332_CourseEffects;
    }
    // Phase 2: register a spray directly, or expand one sequence update into sprays.
    // The local descriptor lives only during this call. The stores below copy its
    // parameters and transform/point pointers into each spray before registration.
    if (String_CompareCString(dot + 1, skyLayerResourceStrings.sprayExtension) == 0) {
        void* effect = CFxSpray::CreateFromScript(
            (CFxBox*)(courseEffects), (const char*)(record + IFF_OFFSET(IFF_STRUCT::sCourse, skyEffect)), 0, 0);
        if (effect == 0)
            return;
        float pos[10];
        unsigned int* zero = (unsigned int*)(pos + 2);
        unsigned int* tgt = (unsigned int*)(pos + 6);
        FxAttachment_Init(pos);
        zero[0] = 0;
        tgt[0] = (unsigned int)((void*)(((WView*)g_view)->viewMatrix + 9));
        (((EffectSprayAttachmentView*)effect)->attachment) = pos;
        (((EffectSprayAttachmentView*)effect)->attachmentParameters[0]) = pos[0];
        (((EffectSprayAttachmentView*)effect)->attachmentParameters[1]) = pos[1];
        (((EffectSprayAttachmentView*)effect)->useCameraAttachment) = 1;
        memcpy(((EffectSprayAttachmentView*)effect)->attachmentTransform, zero, 0x10);
        memcpy(((EffectSprayAttachmentView*)effect)->targetTransform, tgt, 0x10);
        ((CFxBox*)(courseEffects))->RegisterSpray(((CFxSpray*)(effect)));
    } else if (String_CompareCString(dot + 1, skyLayerResourceStrings.sequenceExtension) == 0) {
        void* effect = CFxSequence_Create(courseEffects, record + IFF_OFFSET(IFF_STRUCT::sCourse, skyEffect), 1, 0, 0);
        if (effect == 0)
            return;
        float pos[10];
        unsigned int* zero = (unsigned int*)(pos + 2);
        unsigned int* tgt = (unsigned int*)(pos + 6);
        FxAttachment_Init(pos);
        zero[0] = 0;
        tgt[0] = (unsigned int)((void*)(((WView*)g_view)->viewMatrix + 9));
        (((EffectSequenceAttachmentView*)effect)->attachment) = pos;
        (((EffectSequenceAttachmentView*)effect)->attachmentParameters[0]) = pos[0];
        (((EffectSequenceAttachmentView*)effect)->attachmentParameters[1]) = pos[1];
        ((CFxBox*)(courseEffects))->RegisterSequence(((CFxSequence*)(effect)));
        ((CFxBox*)courseEffects)->UpdateEffects(*(float*)((char*)&((EffectUpdateView*)courseEffects)->updateStep));
        unsigned char* node = (unsigned char*)((CFxSequence*)effect)->firstSprayNode;
        unsigned char* header = (unsigned char*)&((CFxSequence*)effect)->sprayTreeRoot;
        while (header != node) {
            void* elem = *(void**)(node + 0x10);
            (((EffectSprayAttachmentView*)elem)->attachment) = pos;
            (((EffectSprayAttachmentView*)elem)->attachmentParameters[0]) = pos[0];
            (((EffectSprayAttachmentView*)elem)->attachmentParameters[1]) = pos[1];
            (((EffectSprayAttachmentView*)elem)->useCameraAttachment) = 1;
            memcpy(((EffectSprayAttachmentView*)elem)->attachmentTransform, zero, 0x10);
            memcpy((char*)((EffectSprayAttachmentView*)elem)->targetTransform, tgt, 0x10);
            ((CFxBox*)(courseEffects))->RegisterSpray(((CFxSpray*)(elem)));
            if (*(void**)(node + 0x04)) {
                unsigned char* r3 = *(unsigned char**)(node + 0x04);
                while (*(void**)r3)
                    r3 = *(unsigned char**)r3;
                node = r3;
            } else {
                unsigned char* r3 = (unsigned char*)(*(unsigned int*)(node + 0x08) & ~1u);
                while (node != *(unsigned char**)r3) {
                    node = r3;
                    r3 = (unsigned char*)(*(unsigned int*)(node + 0x08) & ~1u);
                }
                node = r3;
            }
        }
        ((CFxBox*)(courseEffects))->RemoveSequence(((CFxSequence*)(effect)), 0);
    }
}

// Initialize at most fifty wind particles, excluding the four course IDs without this effect.
// count is clamped above 50 only (negative values are not corrected). value is the
// spawn interval in seconds; this does not reset activeParticleCount before seeding.
void CPvsFxBox::InitWindParticles(int count, float value) {
    unsigned char courseAllowsParticles = 1;
    int courseId = SharedDoc_GetCourseId();
    if (courseId == 5 || courseId == 7 || courseId == 8 || courseId == 13)
        courseAllowsParticles = 0;
    if (courseAllowsParticles == 0)
        return;
    if (count > kMaxWindParticles)
        count = kMaxWindParticles;
    spawnInterval = value;
    unsigned char* particleBase = (unsigned char*)this;
    float zero = 0.0f;
    unsigned char* particlePosition = (unsigned char*)&((WindParticleAccess*)this)->particle.position;
    particleCapacity = count;
    int particleIndex = 0;
    float baseParticleSize = 256.0f;
    unsigned char inactive = 0;
    unsigned char active = 1;
    for (; particleIndex < count; ++particleIndex) {
        float random = Random_RangeFloat(-1, 1.0f, 2.0f);
        ((WindParticleAccess*)particleBase)->particle.size = baseParticleSize * random;
        int r = rand();
        ((WindParticleAccess*)particleBase)->particle.atlasRow = r % 4;
        float x = Random_RangeFloat(-1, -0.1f, 0.1f);
        ((WindParticleAccess*)particleBase)->particle.speedJitter = x;
        r = rand();
        ((WindParticleAccess*)particleBase)->particle.flipU = (r % 2 != 0);
        r = rand();
        if (r % 2 == 0) {
            float randomZ = Random_RangeFloat(-1, -1.0f, 1.0f);
            float randomX = Random_RangeFloat(-1, -1.0f, 1.0f);
            float out[3];
            float vec[3] = {randomX, zero, randomZ};
            __ml__FfRC7WVector(out, vec, particleSpawnRadius);
            __as__7WVectorFRC7WVector(particlePosition, out);
            ((WindParticleAccess*)particleBase)->particle.position.y = particleSpawnHeight;
            ((WindParticleAccess*)particleBase)->particle.active = active;
            ++activeParticleCount;
        } else {
            ((WindParticleAccess*)particleBase)->particle.active = inactive;
        }
        particleBase += 0x24;
        particlePosition += 0x24;
    }
}

struct Ch801F7B00 {
    unsigned char out;
    unsigned char outputPadding[3];
    unsigned char in;
    unsigned char inputPadding[3];
};

// Replace the particle bitmap using borrowed NUL-terminated stem text and live
// g_sharedDoc/g_resourceManager. Weather 1 appends separator+cloud, 2 adds
// separator+rain, all other values add neither; all append cloudBitmapSuffix.
// Release the previous ID when manager+old ID exist, clear both particle counts,
// then publish the returned ID even when zero. A miss leaves the previous bitmap
// gone and particles disabled, with no fallback/status/rollback. New ownership
// belongs to this environment; ReleaseParticleResource/ReleaseResources releases
// it. Path allocation exceptions also leave the old resource/counts discarded;
// the local cleanup guard frees the built heap string after construction.
void CPvsFxBox::LoadVariantBitmap(const char* text) {
    if (g_resourceManager && particleBitmap) {
        ResourceManager_ReleaseId(g_resourceManager, particleBitmap);
        particleBitmap = 0;
    }
    particleCapacity = 0;
    activeParticleCount = 0;
    MWStringStorageView str = {};
    unsigned int length = strlen(text);
    String_ReserveCapacity(&str, length);
    Ch801F7B00 c0;
    c0.out = c0.in;
    String_ReplaceRange(&str, 0, 0, text, text + length, &c0.out);
    struct StringCleanup {
        void* object;
        // Release heap-backed resource-path text on normal return and unwind.
        ~StringCleanup() {
            unsigned int* words = (unsigned int*)object;
            if (words[0] >> 31)
                appFreeUni__FPv(*(char**)(words + 2));
        }
    } cleanup = {&str};
    unsigned int index;
    switch ((int)((SharedWeatherView*)g_sharedDoc)->weatherVariant) {
    case 1: {
        const char* s0 = skyLayerResourceStrings.variantSeparator;
        index = ((*(unsigned int*)str.data >> 31) == 0) ? (str.data[0] & 0x7f) : *(unsigned int*)(str.data + 4);
        Ch801F7B00 c1;
        c1.out = c1.in;
        String_ReplaceRange(&str, index, 0, s0, s0 + strlen(s0), &c1.out);
        const char* s1 = skyLayerResourceStrings.cloudVariant;
        index = ((*(unsigned int*)str.data >> 31) == 0) ? (str.data[0] & 0x7f) : *(unsigned int*)(str.data + 4);
        Ch801F7B00 c2;
        c2.out = c2.in;
        String_ReplaceRange(&str, index, 0, s1, s1 + strlen(s1), &c2.out);
        break;
    }
    case 2: {
        const char* s0 = skyLayerResourceStrings.variantSeparator;
        index = ((*(unsigned int*)str.data >> 31) == 0) ? (str.data[0] & 0x7f) : *(unsigned int*)(str.data + 4);
        Ch801F7B00 c1;
        c1.out = c1.in;
        String_ReplaceRange(&str, index, 0, s0, s0 + strlen(s0), &c1.out);
        const char* s1 = skyLayerResourceStrings.rainVariant;
        index = ((*(unsigned int*)str.data >> 31) == 0) ? (str.data[0] & 0x7f) : *(unsigned int*)(str.data + 4);
        Ch801F7B00 c2;
        c2.out = c2.in;
        String_ReplaceRange(&str, index, 0, s1, s1 + strlen(s1), &c2.out);
        break;
    }
    }
    const char* tail = skyLayerResourceStrings.cloudBitmapSuffix;
    index = ((*(unsigned int*)str.data >> 31) == 0) ? (str.data[0] & 0x7f) : *(unsigned int*)(str.data + 4);
    Ch801F7B00 c3;
    c3.out = c3.in;
    String_ReplaceRange(&str, index, 0, tail, tail + strlen(tail), &c3.out);
    const char* result =
        ((*(unsigned int*)str.data >> 31) == 0) ? (const char*)str.data + 1 : *(const char**)(str.data + 8);
    particleBitmap = (void*)ResourceManager_LoadBitmap(g_resourceManager, result, 0, 0, 0);
}

// Overwrite the queued frame time without accumulating it. A zero suppresses
// the next update.
void CPvsFxBox::SetPendingDelta(float deltaSeconds) {
    pendingSeconds = deltaSeconds;
}

// Borrow the course entry's script name from its inline/heap small-string storage.
static inline const char* SceneEntryScriptName(unsigned char* entry) {
    return ((*(unsigned int*)(entry + 0x30) >> 31) == 0) ? (const char*)(entry + 0x31) : *(const char**)(entry + 0x38);
}

struct Fx801F8324 {
    float pos[2];
    unsigned int zero[4];
    unsigned int tgt[4];
};

#pragma opt_lifetimes off
// Create camera-relative sprays from type-three course entries, expanding sequence scripts when needed.
void CPvsFxBox::RebuildSceneEffects() {
    // Phase 1: replace the scene manager and scan type-three entries in clear weather.
    FxBoxView* old = (*(FxBoxView**)&sceneEffects);
    if (old != 0) {
        delete old;
        sceneEffects = 0;
    }
    unsigned char* root = (unsigned char*)g_sharedDoc;
    if (((SharedWeatherView*)root)->weatherVariant != 0)
        return;
    float f31 = kFloat_33_333332_CourseEffects;
    unsigned char* resource = *(unsigned char**)(&((CGolfDoc*)((CSharedDoc*)root)->golfDoc)->courseData);
    unsigned char* entry = *(unsigned char**)(resource + 0xA0);
    int i = 0;
    int count = *(int*)(resource + 0x78);
    for (; count > i; entry += 0x3C, ++i) {
        if (entry[0] != 3)
            continue;
        void* effect;
        char* dot = strrchr((char*)SceneEntryScriptName(entry), '.');
        if (dot == 0)
            continue;
        if (strlen(dot) < 4)
            continue;
        if (sceneEffects == 0) {
            sceneEffects = new CPvsFxBoxConstructorView();
            ((CFxBox*)sceneEffects)->SetResourceManager((void*)g_resourceManager);
            ((CFxBox*)sceneEffects)->SetCameraTransform(s_cameraTransform);
            *(float*)((char*)&((EffectUpdateView*)sceneEffects)->updateStep) = f31;
        }
        if (String_CompareCString(dot + 1, skyLayerResourceStrings.sprayExtension) == 0) {
            // Direct spray: normalize the entry's direction and use a 50-world-unit offset.
            effect =
                CFxSpray::CreateFromScript((CFxBox*)(sceneEffects), (const char*)(SceneEntryScriptName(entry)), 0, 0);
            if (effect == 0)
                continue;
            Fx801F8324 fx;
            float mat[3];
            FxAttachment_Init(fx.pos);
            fx.zero[0] = 0;
            fx.tgt[0] = (unsigned int)((void*)(((WView*)g_view)->viewMatrix + 9));
            ((EffectSprayAttachmentView*)effect)->attachment = fx.pos;
            (((EffectSprayAttachmentView*)effect)->attachmentParameters[0]) = fx.pos[0];
            (((EffectSprayAttachmentView*)effect)->attachmentParameters[1]) = fx.pos[1];
            __ml__FRC7WVectorf(mat, Normalize__7WVectorFv(entry + 0x24), kFloat_50_ShotEffect);
            __as__7WVectorFRC7WVector(&((CourseSprayPositionView*)effect)->cameraOffset, mat);
            memcpy(((EffectSprayAttachmentView*)effect)->attachmentTransform, fx.zero, 0x10);
            memcpy((char*)((EffectSprayAttachmentView*)effect)->targetTransform, fx.tgt, 0x10);
            ((CFxBox*)(sceneEffects))->RegisterSpray(((CFxSpray*)(effect)));
        } else if (String_CompareCString(dot + 1, skyLayerResourceStrings.sequenceExtension) == 0) {
            // Sequence: expand once, add each spray's initial world offset, then detach
            // the wrapper. The resulting sprays remain registered with sceneEffects.
            effect = CFxSequence_Create(sceneEffects, SceneEntryScriptName(entry), 1, 0, 0);
            if (effect == 0)
                continue;
            Fx801F8324 fx;
            FxAttachment_Init(fx.pos);
            fx.zero[0] = 0;
            fx.tgt[0] = (unsigned int)((void*)(((WView*)g_view)->viewMatrix + 9));
            (((EffectSequenceAttachmentView*)effect)->attachment) = fx.pos;
            (((EffectSequenceAttachmentView*)effect)->attachmentParameters[0]) = fx.pos[0];
            (((EffectSequenceAttachmentView*)effect)->attachmentParameters[1]) = fx.pos[1];
            ((CFxBox*)(sceneEffects))->RegisterSequence(((CFxSequence*)(effect)));
            ((CFxBox*)sceneEffects)->UpdateEffects(*(float*)((char*)&((EffectUpdateView*)sceneEffects)->updateStep));
            unsigned char* node = (unsigned char*)((CFxSequence*)effect)->firstSprayNode;
            unsigned char* header = (unsigned char*)&((CFxSequence*)effect)->sprayTreeRoot;
            while (node != header) {
                void* elem = *(void**)(node + 0x10);
                float mat2[3];
                float mat[3];
                (((EffectSprayAttachmentView*)elem)->attachment) = fx.pos;
                (((EffectSprayAttachmentView*)elem)->attachmentParameters[0]) = fx.pos[0];
                (((EffectSprayAttachmentView*)elem)->attachmentParameters[1]) = fx.pos[1];
                __ml__FRC7WVectorf(mat, Normalize__7WVectorFv(entry + 0x24), kFloat_50_ShotEffect);
                __pl__FRC7WVectorRC7WVector(mat2, mat, &((CourseSprayPositionView*)elem)->worldPosition);
                __as__7WVectorFRC7WVector(&((CourseSprayPositionView*)elem)->cameraOffset, mat2);
                memcpy(((EffectSprayAttachmentView*)elem)->attachmentTransform, fx.zero, 0x10);
                memcpy((char*)((EffectSprayAttachmentView*)elem)->targetTransform, fx.tgt, 0x10);
                ((CFxBox*)(sceneEffects))->RegisterSpray(((CFxSpray*)(elem)));
                if (*(void**)(node + 0x04)) {
                    unsigned char* r3 = *(unsigned char**)(node + 0x04);
                    while (*(void**)r3)
                        r3 = *(unsigned char**)r3;
                    node = r3;
                } else {
                    unsigned char* r3;
                    while (node != *(unsigned char**)(r3 = (unsigned char*)(*(unsigned int*)(node + 0x08) & ~1u))) {
                        node = r3;
                    }
                    node = r3;
                }
            }
            ((CFxBox*)(sceneEffects))->RemoveSequence(((CFxSequence*)(effect)), 0);
        }
    }
}
#pragma opt_lifetimes reset

// Create the auxiliary effect from the first type-two course entry.
void CPvsFxBox::RebuildAuxiliaryEffect() {
    if (auxiliaryEffect != 0) {
        PvsFxAuxiliaryResources_Dtor(auxiliaryEffect, 1);
        auxiliaryEffect = 0;
    }

    void* object = 0;
    CourseEntryRecord* entry = ((SharedDocView*)g_sharedDoc)->manager->resource->entries;
    for (int i = 0; i < ((SharedDocView*)g_sharedDoc)->manager->resource->count; ++i) {
        if (entry != 0 && entry->type == 2) {
            if (auxiliaryEffect == 0) {
                Object801F86DC* object2 = new Object801F86DC();
                object = object2;
                auxiliaryEffect = object;
            }
            PvsFxAuxiliaryResources_LoadCourseParameters(auxiliaryEffect, entry->effectParameters);
            break;
        }
        ++entry;
    }
}

extern "C" void GetWindVector__5CWindFv(void* result, const void* self);

// Spawn and move wind particles, fade them by distance, update enabled effects,
// and consume the queued frame time. At most one particle spawns per update;
// excess timer time is retained. Particle positions here are world-space vectors.
void CPvsFxBox::UpdateWindParticles() {
    if (pendingSeconds != 0.0f) {
        float dt = pendingSeconds;
        // Phase 1: consume one spawn interval and activate the first free slot.
        float timer = spawnTimer + dt;
        float interval = spawnInterval;
        spawnTimer = timer;
        if (timer > interval) {
            spawnTimer = timer - interval;
            int count = particleCapacity;
            if (activeParticleCount < count) {
                for (int i = 0; i < count; ++i) {
                    // Keep an object-relative cursor; the typed view locates its particle record.
                    unsigned char* slotCursor = (unsigned char*)this + i * (int)sizeof(WindParticleSlot);
                    if (!((WindParticleAccess*)slotCursor)->particle.active) {
                        float x = Random_RangeFloat(-1, kFloat_neg1_CourseRender, kFloat_1_CourseRender);
                        float pos[3] = {x, 0.0f, kFloat_neg1_CourseRender};
                        float windAngle = CWind::GetInstance()->GetDirectionAngle();
                        float mtx[12];
                        Matrix_SetAxisRotation(mtx, windAngle, 1);
                        float a[3], b[3];
                        __ml__FfRC7WVector(a, pos, particleSpawnRadius);
                        Matrix_TransformPoint(b, a, mtx);
                        __as__7WVectorFRC7WVector(slotCursor + kWindParticlePositionOffset, b);
                        ((WindParticleAccess*)slotCursor)->particle.position.y = particleSpawnHeight;
                        ((WindParticleAccess*)slotCursor)->particle.active = 1;
                        ((WindParticleAccess*)slotCursor)->particle.flipU = rand() % 2 != 0;
                        ++activeParticleCount;
                        break;
                    }
                }
            }
        }
        // Phase 2: integrate wind displacement, fade by horizontal camera distance,
        // then fade near the spawn-region edge and retire particles outside its square diagonal.
        for (int i = 0; i < particleCapacity; ++i) {
            unsigned char* slotCursor = (unsigned char*)this + i * (int)sizeof(WindParticleSlot);
            if (((WindParticleAccess*)slotCursor)->particle.active) {
                char windVector[12];
                GetWindVector__5CWindFv(windVector, CWind::GetInstance());
                float windSpeedScale = (1.0f + ((WindParticleAccess*)slotCursor)->particle.speedJitter) * 3.0f;
                float scaled[3], moved[3];
                __ml__FfRC7WVector(scaled, (float*)windVector, windSpeedScale);
                __ml__FRC7WVectorf(moved, scaled, pendingSeconds);
                __apl__7WVectorFRC7WVector(slotCursor + kWindParticlePositionOffset, moved);
                char diff[16];
                __mi__FRC7WVectorRC7WVector(diff, slotCursor + kWindParticlePositionOffset,
                                            &((CameraPositionView*)s_cameraTransform)->position);
                *(float*)(diff + 4) = 0.0f;
                float d = Magnitude__7WVectorCFv(diff);
                float t = 1.0f - d / particleCameraFadeDistance;
                t = (0.0f < t) ? t : 0.0f;
                ((WindParticleAccess*)slotCursor)->particle.opacity = t > 0.5f ? 255.0f : 510.0f * t;
                __as__7WVectorFRC7WVector(diff, (float*)(slotCursor + kWindParticlePositionOffset));
                *(float*)(diff + 4) = 0.0f;
                d = Magnitude__7WVectorCFv(diff);
                t = 1.0f - d / particleSpawnRadius;
                t = (0.0f < t) ? t : 0.0f;
                if (t < 0.1f)
                    ((WindParticleAccess*)slotCursor)->particle.opacity *= 10.0f * t;
                if (Magnitude__7WVectorCFv(diff) > particleSpawnRadius * Math_SquareRootApprox(2.0f)) {
                    ((WindParticleAccess*)slotCursor)->particle.active = 0;
                    --activeParticleCount;
                }
            }
        }
        // Phase 3: effect-box scripts use milliseconds; auxiliary particles use seconds.
        // Consuming the delta prevents a second update when drawing repeats this frame.
        if (courseEffects && s_pvsEffectEnableFlags.updateCourseSprays)
            ((CFxBox*)courseEffects)->UpdateEffects(1000.0f * pendingSeconds);
        if (sceneEffects && s_pvsEffectEnableFlags.updateSceneSprays)
            ((CFxBox*)sceneEffects)->UpdateEffects(1000.0f * pendingSeconds);
        if (auxiliaryEffect && s_pvsEffectEnableFlags.updateAuxiliary)
            PvsFxAuxiliaryResources_AdvancePhase(auxiliaryEffect, pendingSeconds);
        pendingSeconds = 0.0f;
    }
}

void PvsFxAuxiliaryResources_Draw(void*, void*);

// Apply course-effect render state and draw enabled sky layers, sprays, auxiliary effects, and particles.
// Only fog-end state (24) is saved/restored by this entry. Other preset states,
// transforms and format bindings are left installed; individual draw passes reset
// view state where shown. This is not a full renderer-state save/restore scope.
void CPvsFxBox::DrawEnabledEffects(void* camera) {
    int buf[30];
    int savedFogEndBits;

    // Phase 1: consume pending updates, establish effect render states and camera transforms.

    int* a = (int*)(courseEffectRenderDefaults + 0x78);

    UpdateWindParticles();

    memcpy(buf, courseEffectRenderDefaults, 0x78);
    int s4 = a[4];
    int s0 = a[0];
    int* b = (int*)(courseEffectRenderDefaults + 0xA8);
    int s6 = a[6];
    int s5 = a[5];
    unsigned int s10 = a[10];
    int s9 = a[9];
    int s8 = a[8];
    int s14 = b[2];
    int s13 = b[1];
    int s12 = b[0];
    int s15 = b[3];
    unsigned int s21 = b[9];
    int s18 = b[6];
    int s20 = b[8];

    unsigned int states[22];
    states[0] = s0;
    states[1] = (a[1]);
    states[2] = (a[2]);
    int s3 = a[3];
    states[3] = s3;
    states[4] = s4;
    states[5] = s5;
    states[6] = s6;
    states[7] = (a[7]);
    states[8] = s8;
    states[9] = s9;
    states[10] = s10;
    states[11] = (a[11]);
    states[12] = s12;
    states[13] = s13;
    states[14] = s14;
    states[15] = s15;
    states[16] = (b[4]);
    states[17] = (b[5]);
    states[18] = s18;
    states[19] = (b[7]);
    states[20] = s20;
    states[21] = s21;
    // Defaults are ID/value pairs: 15 render pairs, 6 stage pairs, 5 parameter pairs.
    // The scalar copies above preserve the original table-load scheduling.
    Engine_SetRenderStateList(buf, 15);
    Engine_SetTextureStageStateList(0, states, 6);
    Engine_SetTextureStageParameterList(0, (states + 12), 5);
    // gx_render_state_and_texture_upload.cpp: source alpha / inverse source alpha, disable depth writes,
    // compare always. These affect the pass independently of sky geometry.
    Engine_SetCachedRenderState(kEffectSourceBlend, kEffectBlendSourceAlpha);
    Engine_SetCachedRenderState(kEffectDestinationBlend, kEffectBlendInverseSourceAlpha);
    Engine_SetCachedRenderState(kEffectDepthWrite, 0);
    Engine_SetCachedRenderState(kEffectDepthCompare, kEffectCompareAlways);
    Camera_Apply(camera, kFloat_0_CourseRender, kFloat_0_CourseRender);
    void* view = Camera_GetRenderState((void*)g_view);
    SetTransform(kTransformView, ((CameraMatricesView*)view)->view);
    SetTransform(kTransformProjection, ((CameraMatricesView*)view)->projection);
    Engine_SetType7Resource(0);
    void** vt = *(void***)&geometry.vtbl;
    ((void (*)(void*))vt[kGeometryResetVirtualSlot])(&geometry.vertexCount);
    SetTransform(kTransformWorld, (void*)((char*)&kIdentityMatrix4x4));
    SetTransform(kTransformTexture0, (void*)((char*)&kIdentityMatrix4x4));
    Engine_SetTextureStageParameter(0, kEffectSamplerWrapS, kEffectSamplerRepeat);
    Engine_SetTextureStageParameter(0, kEffectSamplerWrapT, kEffectSamplerRepeat);
    Engine_SetCachedRenderState(kEffectFogEnabled, 1);
    // State 24 is saved, doubled for the effect pass, and restored at the end.
    Engine_GetRenderState(kEffectFogEnd, &savedFogEndBits);
    Engine_SetCachedRenderState(kEffectFogEnd, savedFogEndBits << 1);
    // The shift operates on the RAW float bit pattern, not fogEnd*2. Preserve
    // this distinction when modifying the fog policy; the final restore is exact.
    skyTranslation = *(WVector*)((((WView*)camera)->viewMatrix + 9));
    skyTranslation.y += skyVerticalOffset;
    // Phase 2: sky follows camera translation; these gameStateFlags comparisons are
    // exact whole-word comparisons, not bit tests, despite the flag-style constants.
    if (s_pvsEffectEnableFlags.drawSkyCaps)
        DrawSkyCaps(camera);
    unsigned char* root = (unsigned char*)g_sharedDoc;
    if (AsCGolfDoc((*(void**)&((CSharedDoc*)root)->golfDoc))->gameStateFlags != kGameStateSkipCourseEffects) {
        DrawFarSkyStrip(camera);
        // Effect collection traversal and optional rendering hooks.
        if (courseEffects) {
            if (s_pvsEffectEnableFlags.drawCourseSprays) {
                void* manager = courseEffects;
                unsigned char* node = (unsigned char*)((CFxBox*)manager)->sprayTreeFirst;
                while (node != (unsigned char*)&((CFxBox*)manager)->sprayTreeRoot) {
                    ((CourseSprayPositionView*)*(void**)(node + 0x10))->worldPosition =
                        *(WVector*)((((WView*)g_view)->viewMatrix + 9));
                    if (*(void**)(node + 0x04)) {
                        unsigned char* r3 = *(unsigned char**)(node + 0x04);
                        while (*(void**)r3)
                            r3 = *(unsigned char**)r3;
                        node = r3;
                    } else {
                        while (1) {
                            unsigned char* r3 = (unsigned char*)(*(unsigned int*)(node + 0x08) & ~1u);
                            if (node == *(unsigned char**)r3)
                                break;
                            node = r3;
                        }
                        node = (unsigned char*)(*(unsigned int*)(node + 0x08) & ~1u);
                    }
                }
                if (s_pvsEffectEnableFlags.drawCourseSprayPass)
                    ((CFxBox*)manager)->DrawSprays((void*)g_view);
                if (s_pvsEffectEnableFlags.resetCourseView) {
                    void* obj = (void*)g_view;
                    ((void (*)(void*, int))(*(void***)obj)[kViewResetVirtualSlot])(obj, 0);
                }
            }
        }
        // Phase 3: scene sprays use camera position + stored offset; course sprays
        // above use camera position alone. Both become world positions before draw.
        if (sceneEffects) {
            if (s_pvsEffectEnableFlags.drawSceneSprays) {
                void* manager = sceneEffects;
                unsigned char* node = (unsigned char*)((CFxBox*)manager)->sprayTreeFirst;
                while (node != (unsigned char*)&((CFxBox*)manager)->sprayTreeRoot) {
                    unsigned char buf[0x0C];
                    void* obj = *(void**)(node + 0x10);

                    __pl__FRC7WVectorRC7WVector(buf, (void*)(((WView*)g_view)->viewMatrix + 9),
                                                &((CourseSprayPositionView*)obj)->cameraOffset);
                    ((CourseSprayPositionView*)obj)->worldPosition = *(WVector*)((float*)buf);
                    if (*(void**)(node + 0x04)) {
                        unsigned char* r3 = *(unsigned char**)(node + 0x04);
                        while (*(void**)r3)
                            r3 = *(unsigned char**)r3;
                        node = r3;
                    } else {
                        while (1) {
                            unsigned char* r3 = (unsigned char*)(*(unsigned int*)(node + 0x08) & ~1u);
                            if (node == *(unsigned char**)r3)
                                break;
                            node = r3;
                        }
                        node = (unsigned char*)(*(unsigned int*)(node + 0x08) & ~1u);
                    }
                }
                ((CFxBox*)manager)->DrawSprays((void*)g_view);
                void* obj = (void*)g_view;
                ((void (*)(void*, int))(*(void***)obj)[kViewResetVirtualSlot])(obj, 0);
            }
        }
        if (auxiliaryEffect && !((SharedWeatherView*)root)->weatherVariant)
            PvsFxAuxiliaryResources_Draw(auxiliaryEffect, camera);
        if (s_pvsEffectEnableFlags.drawNearSky)
            DrawNearSkyStrip(camera);
        if (AsCGolfDoc((*(void**)&((CSharedDoc*)root)->golfDoc))->gameStateFlags != kGameStateSkipParticleDraw &&
            s_pvsEffectEnableFlags.drawParticles)
            DrawParticles(camera);
    }
    // Phase 4: restore only the fog-end word borrowed at entry.
    Engine_SetCachedRenderState(kEffectFogEnd, savedFogEndBits);
}

// Write per-vertex color/XYZ/UV data and submit immediate sky-cap strips.
struct SkyGeometryRenderer {
    unsigned char count; // 0x9a0
    unsigned char countPadding[3];
    // PowerPC passes a 12-byte struct by reference.
    virtual void f00();
    virtual void f04();
    virtual void f08();
    virtual void f14(int, int, int, int, int);
    virtual void f18(int, Arr3);
    virtual void f1c(int, float, float);
    virtual void f20(int, int);
};

struct RenderObject {
    virtual void v00();
    virtual void v04();
    virtual void v08();
    virtual void v0c();
    virtual void v10();
    virtual void v14();
    virtual void v18();
    virtual void v1c();
    virtual void v20();
    virtual void v24();
    virtual void v28();
    virtual void v2c();
    virtual void v30();
    virtual void v34();
    virtual void v38();
    virtual void v3c();
    virtual void v40();
    virtual void v44();
    virtual void v48();
    virtual void v4c();
    virtual void v50();
    virtual void v54();
    virtual void v58();
    virtual void v5c();
    virtual void v60();
    virtual void v64();
    virtual int v70(void*);
    virtual int v74(void*);
};

// ---- merged from pvsfxbox_sky_and_wind_particles.cpp ----

extern "C" {

extern float s_skyRenderTwoPi;
extern float s_courseTokenScanTwoPi;
extern const float kFloat_6_2831855;

// Initialize the rendering helper's stored full-turn angle constant.
void SkyRender_InitTwoPi() {
    s_skyRenderTwoPi = kFloat_6_2831855;
}

// Initialize the scan helper's stored full-turn angle constant.
void CourseTokenScan_InitTwoPi() {
    s_courseTokenScanTwoPi = kFloat_6_2831855;
}

extern const char s_courseTokenHexFormat[];
extern char s_courseTokenDelimiters[];
extern char* strcpy(char*, const char*);
extern double atof(const char*);
extern int String_ParseDecimalInt(const char*);
extern void* __va_arg(void*, int);

// Find a prefix and parse following delimited tokens into the supported numeric/string outputs.
// source is a holder for a borrowed NUL-terminated string; it is not advanced.
// %f/%g take float*, %d/%u int*, %x unsigned*, %s char* (uppercase also accepted).
// Caller must supply a token for every specifier, limit each token to 255 bytes,
// and provide enough string-output space; there are no length/end checks.
// Return 1 means prefix found/scan attempted, not a
// conversion count. Unsupported specifiers consume a token without an output arg.
int String_ScanTokensAfterSubstring(void** source, const char* input, const char* format, ...) {
    if (!*source)
        return 0;
    char* parsed =
        (char*)strstr((const char*)*source, input); // original 0x801F9E8C lwz r3,0x0(r3): the string, not the holder
    if (!parsed)
        return 0;

    char* text = parsed + strlen(input);
    char temp[0x100];
    struct {
        unsigned int a;
        void* b;
        void* c;
    } args; // va_list {gpr, fpr, reserved, overflow area, register save area}
    __va_setup(
        &args); // same compiler intrinsic as Stream_ScanFormat; &source is the spill slot, not the varargs save area

    while (*format) {
        if (*format++ != '%')
            continue;
        while (*text == ' ' || *text == '\t')
            ++text;
        // original 0x801F9F1C-0x801F9F74: the byte is compared unsigned, text moves past the terminating byte too,
        // and the format letter is range-checked as a signed int
        unsigned n = 0;
        char* out = temp;
        for (;;) {
            unsigned char c = *(unsigned char*)text++;
            *out = c;
            if (c <= ' ')
                break;
            if (strchr(s_courseTokenDelimiters, (signed char)*out))
                break;
            ++out;
            ++n;
        }
        temp[n] = 0;
        signed char spec = *format++;
        if ((unsigned int)(spec - 'D') > 0x34)
            continue;
        switch (spec) {
        case 'F':
        case 'G':
        case 'f':
        case 'g': {
            float value = (float)atof(temp);
            float* dst = *(float**)__va_arg(&args, 1);
            memcpy(dst, &value, 4);
            break;
        }
        case 'D':
        case 'U':
        case 'd':
        case 'u': {
            int value = String_ParseDecimalInt(temp);
            int* dst = *(int**)__va_arg(&args, 1);
            memcpy(dst, &value, 4);
            break;
        }
        case 'X':
        case 'x': {
            unsigned* dst = (unsigned*)__va_arg(&args, 1);
            sscanf(temp, s_courseTokenHexFormat, *dst);
            break;
        }
        case 'S':
        case 's': {
            char* dst = *(char**)__va_arg(&args, 1);
            strcpy(dst, temp);
            break;
        }
        }
    }
    return 1;
}

} // extern "C"

// Draw the upper and lower sky caps using generated geometry tinted by course brightness.
// A corner's world position is the camera-following sky translation plus its
// local cap corner. U comes from the corner's low bit; V is zero for all corners.
void CPvsFxBox::DrawSkyCaps(void* camera) {
    int color =
        Color_ScaleRGB((void*)particleColor, AsCGolfDoc((*(void**)&((CSharedDoc*)g_sharedDoc)->golfDoc))->bright);
    if (upperSkySourceVertices != 0) {
        RenderObject* obj = (RenderObject*)((WView*)camera)->renderTarget;
        Engine_SetTextureStageResource(0, obj->v70(skyBuffers.upperTexture));
        obj = (RenderObject*)((WView*)camera)->renderTarget;
        Engine_SetTexturePalette(0, obj->v74(skyBuffers.upperTexture));
        unsigned char a = color;
        unsigned char b = color >> 8;
        unsigned char c = color >> 16;
        for (int i = 0; i < 4; ++i) {
            Arr3 tmp;
            WVector pos = skyTranslation + *(WVector*)((char*)upperSkySourceVertices + i * 12);
            *(Arr3*)&tmp = *(Arr3*)&pos;
            if (((SkyGeometryRenderer*)&geometry)->count > (unsigned char)i) {
                ((SkyGeometryRenderer*)&geometry)->f18((unsigned char)i, tmp);
            }
            if (((SkyGeometryRenderer*)&geometry)->count > (unsigned char)i)
                ((SkyGeometryRenderer*)&geometry)->f14((unsigned char)i, 0xff, c, b, a);
            if (((SkyGeometryRenderer*)&geometry)->count > (unsigned char)i)
                ((SkyGeometryRenderer*)&geometry)->f1c((unsigned char)i, (float)(i & 1), kFloat_0_CourseRender);
        }
        ((SkyGeometryRenderer*)&geometry)->f20(kSkyTriangleStrip, 2);
    }
    if (lowerSkySourceVertices != 0) {
        RenderObject* obj = (RenderObject*)((WView*)camera)->renderTarget;
        Engine_SetTextureStageResource(0, obj->v70(skyBuffers.lowerTexture));
        obj = (RenderObject*)((WView*)camera)->renderTarget;
        Engine_SetTexturePalette(0, obj->v74(skyBuffers.lowerTexture));
        unsigned char a = color;
        unsigned char b = color >> 8;
        unsigned char c = color >> 16;
        for (int i = 0; i < 4; ++i) {
            Arr3 tmp;
            WVector pos = skyTranslation + *(WVector*)((char*)lowerSkySourceVertices + i * 12);
            *(Arr3*)&tmp = *(Arr3*)&pos;
            if (((SkyGeometryRenderer*)&geometry)->count > (unsigned char)i) {
                ((SkyGeometryRenderer*)&geometry)->f18((unsigned char)i, tmp);
            }
            if (((SkyGeometryRenderer*)&geometry)->count > (unsigned char)i)
                ((SkyGeometryRenderer*)&geometry)->f14((unsigned char)i, 0xff, c, b, a);
            if (((SkyGeometryRenderer*)&geometry)->count > (unsigned char)i)
                ((SkyGeometryRenderer*)&geometry)->f1c((unsigned char)i, (float)(i & 1), kFloat_0_CourseRender);
        }
        ((SkyGeometryRenderer*)&geometry)->f20(kSkyTriangleStrip, 2);
    }
}

// Create a snow effect from animated-sprite UV bounds and initialize its rendering context.
CSnow* CSnow::Init(int count, const char* resourceName, int context, float scale) {
    float topval = kFloat_1_CourseBox;
    fallSpeedMultiplier = topval;
    vtbl = (void**)__vt__5CSnow;
    particles = 0;
    animatedSprite = 0;
    vertexBuffer = 0;
    indexBuffer = 0;
    auxiliaryBuffer = 0;
    geometry.vertexCount = 0;
    geometry.vtbl = __vt__10CGeneTex3D;
    geometry.vertices = 0;
    try {
        animatedSprite = ((void* (*)(void*, float, float, const char*, int))W3dAniSpr_Create)(g_resourceManager, scale,
                                                                                              scale, resourceName, 0);
        void* config = W3dAniSpr_GetFrameDescription(animatedSprite, 0);
        float unused = scale;
        spriteTexture = *(void**)config;
        float f3 = *(float*)((char*)config + 4);
        uMin = f3;
        float f2 = *(float*)((char*)config + 8);
        vMin = f2;
        float f0 = *(float*)((char*)config + 12);
        uMax = f0;
        f0 = f0 - f3;
        float f3b = *(float*)((char*)config + 16);
        f2 = f3b - f2;
        vMax = f3b;
        uSpan = f0;
        vSpan = f2;
        effectContext = (void*)context;
        ((void (*)(CSnow*, int, float))((void**)vtbl)[6])(this, count, kFloat_1_CourseBox);
        (void)unused;
        return this;
    } catch (...) {
        __dt__13GeneTexBufferFv(&geometry, -1);
        dtor_801B7D1C(this, -1);
        throw;
    }
}

// Release the sprite, particle storage and graphics buffers.
CSnow::~CSnow() {
    vtbl = __vt__5CSnow;
    ReleaseRef(&vertexBuffer);
    ReleaseRef(&indexBuffer);
    ReleaseRef(&auxiliaryBuffer);
    if (particles) {
        __dla__FPv(particles);
        particles = 0;
    }
    if (g_resourceManager && animatedSprite) {
        Object_DeleteVirtual(g_resourceManager, animatedSprite);
        animatedSprite = 0;
    }
    if (&geometry.vertexCount) {
        geometry.vtbl = __vt__10CGeneTex3D;
        if (geometry.vertices) {
            __dla__FPv(geometry.vertices);
            geometry.vertices = 0;
        }
    }
}

extern "C" {

typedef Vec3f Vec3;
struct MatrixThing {
    char data[0x18];
};
extern void Vec3_Add(void*, void*, void*);
extern void Engine_SetFlexibleVertexFormat(int);
extern void Engine_SetVertexStream(int, void*, int, int);
extern void Engine_SetIndexBuffer(void*, int);
extern void Engine_DrawIndexedPrimitive(int, int, int, int, int, int);
extern void DrawPrimitiveUP(int, int, void*, int);
extern void Matrix_TransformDirection(void*, void*, void*);
extern float kParticleBillboardBottom;
extern float kParticleBillboardLeft;

struct Words24 {
    unsigned w[24];
};
struct Words12 {
    unsigned w[12];
};
struct Words10 {
    unsigned w[10];
};
// The course-effect render defaults: a setup list, then two more lists that
// the particle pass copies.
struct RenderDefaultsView {
    char pad_00[0xE0];
    Words24 setup; // 0xE0
    Words12 data1; // 0x140
    Words10 data2; // 0x170
};
} // extern "C"

// Batch active wind particles into camera-facing textured quads, flushing every 32 quads.
// 0xC00 bytes = 32 quads * 4 vertices * 0x18-byte XYZ/RGBA/UV records.
// Rotate each local XY corner by the camera basis (direction only), multiply by
// particle size, then add the world center. UV V selects one of four atlas rows;
// flipU reverses U without changing geometry. Alpha is the update-computed 0..255 value.
// The camera argument is not read: the view comes from g_view and s_cameraTransform.
void CPvsFxBox::DrawParticles(void* camera) {
    // Phase 1: bind camera transforms, render state and the particle atlas.
    RenderDefaultsView* g = (RenderDefaultsView*)courseEffectRenderDefaults;
    char* state = (*(char**)&((CSharedDoc*)g_sharedDoc)->golfDoc);
    if ((unsigned)AsCGolfDoc(state)->gameStateFlags == kGameStateSkipCourseEffects)
        return;

    unsigned char vertices[0xc00];
    Words24 setup = *(Words24*)(&g->setup);
    Words12 data1 = *(Words12*)(&g->data1);
    Words10 data2 = *(Words10*)(&g->data2);
    Engine_SetRenderStateList(&setup, 12);
    Engine_SetTextureStageStateList(0, &data1, 6);
    Engine_SetTextureStageParameterList(0, &data2, 5);
    void* renderer = Camera_GetRenderState(g_view);
    SetTransform(kTransformProjection, ((CameraMatricesView*)renderer)->projection);
    SetTransform(kTransformView, ((CameraMatricesView*)renderer)->view);
    Engine_SetType7Resource(0);
    SetTransform(kTransformWorld, kIdentityMatrix4x4);
    SetTransform(kTransformTexture0, kIdentityMatrix4x4);
    RenderObject* obj = ((RenderObject*)((WView*)g_view)->renderTarget);
    void* item = particleBitmap;
    int x = obj->v70(item);
    Engine_SetTextureStageResource(0, x);
    obj = ((RenderObject*)((WView*)g_view)->renderTarget);
    item = particleBitmap;
    int y = obj->v74(item);
    Engine_SetTexturePalette(0, y);
    Engine_SetFlexibleVertexFormat((0x12 << 16) - 0xfff);

    // Vertex byte layout: XYZ +00/+04/+08, RGBA +0C..0F, UV +10/+14.
    // Sixteen explicit stores per iteration cover four quads (0x180 bytes);
    // preserve the expanded stores to retain instruction scheduling.
    // Phase 2: prefill RGB for the entire reusable batch; each quad sets alpha below.
    unsigned color =
        Color_ScaleRGB((void*)particleColor, AsCGolfDoc((*(char**)&((CSharedDoc*)g_sharedDoc)->golfDoc))->bright);
    unsigned char* bytes = vertices;
    unsigned char red = color >> 16, green = color >> 8, blue = color;
    for (int i = 0; i < 8; ++i, bytes += 0x180) {
        bytes[0xc] = red;
        bytes[0xd] = green;
        bytes[0xe] = blue;
        bytes[0x24] = red;
        bytes[0x25] = green;
        bytes[0x26] = blue;
        bytes[0x3c] = red;
        bytes[0x3d] = green;
        bytes[0x3e] = blue;
        bytes[0x54] = red;
        bytes[0x55] = green;
        bytes[0x56] = blue;
        bytes[0x6c] = red;
        bytes[0x6d] = green;
        bytes[0x6e] = blue;
        bytes[0x84] = red;
        bytes[0x85] = green;
        bytes[0x86] = blue;
        bytes[0x9c] = red;
        bytes[0x9d] = green;
        bytes[0x9e] = blue;
        bytes[0xb4] = red;
        bytes[0xb5] = green;
        bytes[0xb6] = blue;
        bytes[0xcc] = red;
        bytes[0xcd] = green;
        bytes[0xce] = blue;
        bytes[0xe4] = red;
        bytes[0xe5] = green;
        bytes[0xe6] = blue;
        bytes[0xfc] = red;
        bytes[0xfd] = green;
        bytes[0xfe] = blue;
        bytes[0x114] = red;
        bytes[0x115] = green;
        bytes[0x116] = blue;
        bytes[0x12c] = red;
        bytes[0x12d] = green;
        bytes[0x12e] = blue;
        bytes[0x144] = red;
        bytes[0x145] = green;
        bytes[0x146] = blue;
        bytes[0x15c] = red;
        bytes[0x15d] = green;
        bytes[0x15e] = blue;
        bytes[0x174] = red;
        bytes[0x175] = green;
        bytes[0x176] = blue;
    }
    // The 0.25 value serves both local top Y and one atlas-row UV height.
    float zero, billboardBottom, quarterHeightAndUvStep;
    billboardBottom = kParticleBillboardBottom;
    quarterHeightAndUvStep = kFloat_0_25_CourseRender;
    zero = kFloat_0_CourseRender;
    char* itemBase = (char*)this;
    char* worldCenter = (char*)&((WindParticleAccess*)this)->particle.position;
    unsigned char* v;
    int pending = 0;
    // Phase 3: build world-space corners and emit full batches, then the remainder.
    for (int n = 0; n < particleCapacity; itemBase += 0x24, worldCenter += 0x24, ++n) {
        if (!((WindParticleAccess*)itemBase)->particle.active)
            continue;
        v = (unsigned char*)((Vtx*)vertices + pending * 4);
        for (int j = 0; j < 4; ++j, v += 0x18) {
            Vec3 pos;
            Vec3 out;
            Vec3 scaled;
            Vec3 rotated;
            pos.x = (j & 2) ? kFloat_0_5_Effects : kParticleBillboardLeft;
            if ((unsigned)(j - 1) <= 1)
                pos.y = quarterHeightAndUvStep;
            else
                pos.y = billboardBottom;
            pos.z = zero;
            Matrix_TransformDirection(&rotated, &pos, s_cameraTransform);
            __ml__FfRC7WVector(&scaled, &rotated, ((WindParticleAccess*)itemBase)->particle.size);
            __pl__FRC7WVectorRC7WVector(&out, worldCenter, &scaled);
            *(float*)(v + 0) = out.x;
            *(float*)(v + 4) = out.y;
            *(float*)(v + 8) = out.z;
            if (((WindParticleAccess*)itemBase)->particle.flipU)
                *(float*)(v + 16) = 1.0f - (float)(j >> 1);
            else
                *(float*)(v + 16) = (float)(j >> 1);
            if ((unsigned)(j - 1) <= 1)
                *(float*)(v + 20) = quarterHeightAndUvStep * (float)((WindParticleAccess*)itemBase)->particle.atlasRow;
            else
                *(float*)(v + 20) =
                    quarterHeightAndUvStep * (float)(((WindParticleAccess*)itemBase)->particle.atlasRow + 1);
            v[15] = (unsigned char)((WindParticleAccess*)itemBase)->particle.opacity;
        }
        if (++pending == kParticleBatchQuads) {
            DrawPrimitiveUP(kParticleQuads, kParticleBatchQuads, vertices, kParticleVertexBytes);
            pending = 0;
        }
    }
    if (pending)
        DrawPrimitiveUP(kParticleQuads, pending, vertices, kParticleVertexBytes);
}

// Draw the near-sky pass by updating its destination buffer from far-ring source points.
// The near source and near vertex buffer gate drawing, but the points come from the
// far-ring source, so changing only the near source does not move the strip.
void CPvsFxBox::DrawNearSkyStrip(void* camera) {
    // Phase 1: load the authored state lists and camera view/projection transforms.
    int* src = (int*)courseEffectRenderDefaults;
    unsigned int work[52];
    if (!nearSkySourceVertices || !skyBuffers.nearVertices)
        return;

    for (int i = 0; i < 30; ++i)
        work[i + 22] = src[i];

    work[0] = src[30];
    work[1] = src[31];
    work[2] = src[32];
    work[3] = src[33];
    work[4] = src[34];
    work[5] = src[35];
    work[6] = src[36];
    work[7] = src[37];
    work[8] = src[38];
    work[9] = src[39];
    work[10] = src[40];
    work[11] = src[41];
    work[12] = src[42];
    work[13] = src[43];
    work[14] = src[44];
    work[15] = src[45];
    work[16] = src[46];
    work[17] = src[47];
    work[18] = src[48];
    work[19] = src[49];
    work[20] = src[50];
    work[21] = src[51];

    Engine_SetRenderStateList(work + 22, 15);
    Engine_SetTextureStageStateList(0, work, 6);
    Engine_SetTextureStageParameterList(0, work + 12, 5);
    Engine_SetCachedRenderState(0xd, 4);
    Engine_SetCachedRenderState(0xe, 5);
    void* matrices = Camera_GetRenderState(g_view);
    SetTransform(kTransformView, ((CameraMatricesView*)matrices)->view);
    SetTransform(kTransformProjection, ((CameraMatricesView*)matrices)->projection);
    Engine_SetCachedRenderState(5, 0);
    SetTransform(kTransformWorld, kIdentityMatrix4x4);
    SetTransform(kTransformTexture0, kIdentityMatrix4x4);
    Engine_SetTextureStageParameter(0, 0, 1);
    Engine_SetTextureStageParameter(0, 1, 1);
    Engine_SetCachedRenderState(1, 1);

    // Phase 2: reconstruct three height rows, then translate into world space.
    void* nearPassVertexData;
    Resource_GetOffsetPtr(skyBuffers.nearVertices, 0, 0, &nearPassVertexData, 0);
    float finalRowDrop = kNearSkyRingDropSecond;
    float middleRowDrop = kNearSkyRingDropFirst;
    float topRowRise = kNearSkyRingRise;
    int sourcePointByteOffset = 0;
    int destinationVertexBase = 0;
    for (int i = 0; i <= 0x20; ++i) {
        Vec3 v;
        Vector_CopyComponents(&v, (char*)farSkySourceVertices + sourcePointByteOffset);
        for (int j = 0; j < 3; ++j) {
            if (j == 0)
                v.y += topRowRise;
            else if (j == 1)
                v.y -= middleRowDrop;
            else
                v.y -= finalRowDrop;
            Vec3_Add((char*)nearPassVertexData + (j + destinationVertexBase) * 0x18, &v, &skyTranslation);
        }
        sourcePointByteOffset += 12;
        destinationVertexBase += 3;
    }
    // Phase 3: flush updated positions, bind retained buffers/texture, then draw.
    GraphicsBuffer_Flush(skyBuffers.nearVertices);
    Engine_SetFlexibleVertexFormat((0x12 << 16) - 0xfff);
    Engine_SetCachedRenderState(2, 0);
    RenderObject* obj = (RenderObject*)((WView*)camera)->renderTarget;
    Engine_SetTextureStageResource(0, obj->v70(skyBuffers.nearTexture));
    obj = (RenderObject*)((WView*)camera)->renderTarget;
    Engine_SetTexturePalette(0, obj->v74(skyBuffers.nearTexture));
    Engine_SetVertexStream(0, skyBuffers.nearVertices, 0, 0x18);
    Engine_SetIndexBuffer(skyBuffers.nearIndices, 0);
    Engine_DrawIndexedPrimitive(0x98, 0, 0, 0x84, 0, 0x82);
}

// Update and draw the four-row far-sky strip with the current camera translation.
void CPvsFxBox::DrawFarSkyStrip(void* camera) {
    if (farSkySourceVertices && skyBuffers.farVertices) {
        void* temp;
        Resource_GetOffsetPtr(skyBuffers.farVertices, 0, 0, &temp, 0);
        float step0 = kFloat_235_61946_CourseRender;
        float step1 = kFloat_1884_9557_CourseRender;
        float step2 = kFloat_1178_0973_CourseRender;
        for (int i = 0; i <= 0x20; ++i) {
            Vec3 v;
            Vector_CopyComponents(&v, (char*)farSkySourceVertices + i * 12);
            for (int j = 0; j < 4; ++j) {
                if (j == 0)
                    v.y += step2;
                else if (j == 1)
                    v.y -= step0;
                else if (j == 2)
                    v.y -= step1;
                else
                    v.y -= step0;
                Vec3_Add((char*)temp + (j + i * 4) * 0x18, &v, &skyTranslation);
            }
        }
        GraphicsBuffer_Flush(skyBuffers.farVertices);
        Engine_SetFlexibleVertexFormat((0x12 << 16) - 0xfff);
        Engine_SetCachedRenderState(2, 0);
        RenderObject* obj = (RenderObject*)((WView*)camera)->renderTarget;
        Engine_SetTextureStageResource(0, obj->v70(skyBuffers.farTexture));
        obj = (RenderObject*)((WView*)camera)->renderTarget;
        Engine_SetTexturePalette(0, obj->v74(skyBuffers.farTexture));
        Engine_SetVertexStream(0, skyBuffers.farVertices, 0, 0x18);
        Engine_SetIndexBuffer(skyBuffers.farIndices, 0);
        Engine_DrawIndexedPrimitive(0x98, 0, 0, 0xc6, 0, 0xc4);
        Engine_SetCachedRenderState(1, 0);
    }
}

extern "C" {
void CourseEffects_InitTwoPi();
void SkyRender_InitTwoPi();
void CourseTokenScan_InitTwoPi();
}
/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU804621C0", _ctors_u801F5570_0, CourseEffects_InitTwoPi);
STATIC_INIT_ENTRY(".CRT$XCU804621C0", _ctors_u801F5570_1, SkyRender_InitTwoPi);
STATIC_INIT_ENTRY(".CRT$XCU804621C0", _ctors_u801F5570_2, CourseTokenScan_InitTwoPi);
#pragma pop

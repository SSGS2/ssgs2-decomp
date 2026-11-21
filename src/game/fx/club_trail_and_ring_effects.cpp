#include <decomp/static_init.h>
// Club models, club trails and rotating textured rings; also shared RNG/string
// utilities. Model lookup: ClubModel_CreateForItem, ClubModelSet, IFF sClubSet.
// Trail orientation: named camera/target bones and angle marker in clubAndPlayerStrings.
// Ring size/opacity: CGeneTex3D_CreateRingVertexBuffers and kRing* constants;
// rotation rate: RingEffect_UpdateRotation; texture/world placement: RingEffect_Draw.
// Constants are externally stored; Random_RangeFloat is shared with snow and other
// effects, so changing its sampling also changes those consumers. Matrix API and
// row-vector convention are implemented in gtl_vector_matrix_math.cpp, PSMTX44Concat.
#include "iff/IffStruct.h"
#include "fp_fused.h"
#include "classes/GeneTexBufferLayout.h"
#include "classes/CSharedDoc.h"
#define AsCSharedDoc(p) ((CSharedDoc*)(p))
#include "game/types.h"
#include "classes/WView.h"
// Translation unit 0x800EE8AC-0x800EFEAC (30 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

#include "sdk_protos.h"
#include <decomp/cw_bitfields.h>
typedef char* va_list;

// These effects use a compact layout distinct from the particle class in CGeneTex3D.h.
struct RingEffectLayout {
    void* texture;
    float angle;
    unsigned char enabled;
    char alignment[3];
    void* innerVertices;
    void* outerVertices;
    float worldMatrix[16], innerRotation[16], outerRotation[16];
    GeneTexBufferLayout primitive;
};
struct CameraRenderMatrices { char prefix[0x20]; float view[16], projection[16]; };
struct ClubTrailSlot { int mode; float direction[3]; int startColor, middleColor, endColor; };
struct ClubModelSet {
    IFF_STRUCT::sClubSet* item;
    void* models[4];
    void* secondaryModels[4];
    float clubFrames[4][12];
    ClubTrailSlot trails[4];
};
struct ClubCacheNode { ClubCacheNode* previous; ClubCacheNode* next; ClubModelSet* value; };
struct ClubModelCache { char prefix[0x7C]; int count; ClubCacheNode sentinel; };
struct ClubBoneData { char prefix[0x18]; float position[3]; char reserved24[4]; int parserSeed; };
struct ClubBoneEntry { void* name; ClubBoneData* data; };
struct ClubModelStrings {
    char prefix[0x62];
    char modelFileFormat[7], cameraBone[7], targetBone[7], angleMarker[7], floatFormat[3];
};
enum {
    kClubKindCount = 4,
    kClubCacheSentinelOffset = (int)&((ClubModelCache*)0)->sentinel,
    kColorWhite = 0xFFFFFF,
    kColorMiddleGray = 0x808080,
    kRingSegments = 16,
    kRingVertexFormat = 0x11F001,
    kRingPrimitive = 0x98,
    kFormatBufferCount = 64,
    kFormatBufferBytes = 0x240,
    kFormatStorageBytes = kFormatBufferCount * kFormatBufferBytes
};
typedef char RingEffectLayout_size[(sizeof(RingEffectLayout) == 0xE0) ? 1 : -1];
typedef char ClubModelSet_size[(sizeof(ClubModelSet) == 0x154) ? 1 : -1];

struct RefObj { CW_BITS2( unsigned int count : 16, unsigned int other : 16) };
struct DrawWords30 { unsigned int v[30]; };
struct DrawWords12 { unsigned int v[12]; };
struct DrawWords10 { unsigned int v[10]; };
struct DrawStateData { DrawWords30 state; DrawWords12 tev; DrawWords10 params; };
struct DrawPrimitiveBase { unsigned char flag; };
struct DrawPrimitive : DrawPrimitiveBase {
    virtual void unused();
    virtual void apply();
};
struct DrawDevice {
    virtual void v0(); virtual void v1(); virtual void v2(); virtual void v3();
    virtual void v4(); virtual void v5(); virtual void v6(); virtual void v7();
    virtual void v8(); virtual void v9(); virtual void v10(); virtual void v11();
    virtual void v12(); virtual void v13(); virtual void v14(); virtual void v15();
    virtual void v16(); virtual void v17(); virtual void v18(); virtual void v19();
    virtual void v20(); virtual void v21(); virtual void v22(); virtual void v23();
    virtual void v24(); virtual void v25();
    virtual void* texture(void*);
    virtual void* texturePalette(void*);
};
struct Vertex3 {
    float x, y, z;          // 0x00
    unsigned char r, g, b, a; // 0x0c
    float u, v;             // 0x10, 0x14
};                          // size 0x18
#define VERTEX3_OFF(f, o) typedef char Vertex3_off_##f[((int)&((Vertex3*)0)->f == (o)) ? 1 : -1]
VERTEX3_OFF(x, 0x0);
VERTEX3_OFF(y, 0x4);
VERTEX3_OFF(z, 0x8);
VERTEX3_OFF(r, 0xC);
VERTEX3_OFF(g, 0xD);
VERTEX3_OFF(b, 0xE);
VERTEX3_OFF(a, 0xF);
VERTEX3_OFF(u, 0x10);
VERTEX3_OFF(v, 0x14);
#undef VERTEX3_OFF
typedef char Vertex3_size[(sizeof(Vertex3) == 0x18) ? 1 : -1];
enum { kRingBufferBytes = (kRingSegments + 1) * 2 * sizeof(Vertex3),
       kRingVertexBytes = sizeof(Vertex3), kInnerRingAlpha = 0x50,
       kOuterRingAlpha = 0x32, kOpaqueColorChannel = 0xFF, kRandomSampleModulus = 1023,
       kRingTriangleCount = kRingSegments * 2 };
extern "C" {

// kTwoPi_DefaultState = 6.2831854820251465
extern float kFloat_6_2831855;
// s_clubEffectsFloatState1
extern float s_clubEffectsFloatState1;
// s_clubEffectsFloatState2
extern float s_clubEffectsFloatState2;
// kRandomFractionScale1024 = 0.0009765625
extern float kRandomFractionScale1024;
void Engine_SetFlexibleVertexFormat(int);
void Mtx44_SetTranslation(void*, float, float, float);
void DrawPrimitiveUP(int, int, void*, int);
void Vec3_CopyToOutput(void*, void*);
void* __nwa__FUl(int);
void* memset(void*, int, unsigned int);
// s_temporaryFormatArena
extern int s_temporaryFormatArena;
void __dl__FPv(void*);
void __dla__FPv(void*);
void Mtx44_SetRotationY(void*, float);
void* ResourceManager_LoadBitmap(void*, void*, int, int, int, void*);
void CGeneTex3D_CreateRingVertexBuffers(void*);
extern void* g_resourceManager;
extern unsigned char __vt__10CGeneTex3D[];
// kRingAngularSpeedRadiansPerSecond = 2.5132741928100586
extern float kRingAngularSpeedRadiansPerSecond;
// kRingFullTurnRadians = 6.2831854820251465
extern const float kRingFullTurnRadians;
// kRgbMaximumComponent = 255
extern float kRgbMaximumComponent;
// s_temporaryFormatSlot
extern unsigned char s_temporaryFormatSlot;
void String_VFormatBoundedCopy(void*, int, const char*, void*);
void CubeResourceStorage_dtor(void*, int);
void ResourceManager_ReleaseId(void*, void*);
void Engine_SetRenderStateList(void*, int);
void Engine_SetTextureStageStateList(int, void*, int);
void Engine_SetTextureStageParameterList(int, void*, int);
void Engine_SetCachedRenderState(int, int);
void Camera_Apply(void*, float, float);
void* Camera_GetRenderState(void*);
void SetTransform(int, void*);
void Engine_SetType7Resource(int);
void Engine_SetFlexibleVertexFormat(int);
void Engine_SetTextureStageResource(int, void*);
void Engine_SetTexturePalette(int, void*);
void PSMTX44Concat(void*, void*, void*);
void Engine_SetVertexStream(int, void*, int, int);
void DrawPrimitiveFromBoundBuffers(int, int, int);
void VertexBuffer_Create(int, int, int, int, void*, int);
void Resource_GetOffsetPtr(void*, int, int, unsigned char**, int);
void GraphicsBuffer_Flush(void*);
float Math_CosApprox(float);
float Math_SinApprox(float);
void Vec3_Scale(void*, float*, float);
void Vec3_Add(void*, void*, void*);
// ringStripOffset
extern Vec3f ringStripOffset;
// kRingInitialAngle = 0; kRingSegmentFraction = 0.0625; kRingUvHeight = 1; kRingInnerRadius = 1.9600000381469727; kRingVerticalOffset = 0.30000001192092896; kRingOuterRadius = 2
extern const float kRingInitialAngle, kRingSegmentFraction, kRingUvHeight, kRingInnerRadius, kRingVerticalOffset, kRingOuterRadius;
extern const float kFloat_0_Club;
// kGolfDegreesToRadians = 0.01745329238474369
extern float kGolfDegreesToRadians, kFloat_10_GolfHud;
extern CSharedDoc* g_sharedDoc;
extern void* g_resourceManager;
void* ItemInventory_FindClubSetData(void*, void*);
void* ItemInventory_FindClubData(void*, void*);
void* __nw__FUl(unsigned int);
void* ResourceManager_LoadByExtension(void*, const char*, int, int, int);
void WPuppet_UpdateBoundsFromResourceBox(void*, int, int);
void* WPuppet_FindNamedBoneEntry(void*, const char*);
int String_ScanTokensAfterSubstring(char*, const char*, ...);
void* __as__7WVectorFRC7WVector(void*, void*);
void* __mi__FRC7WVectorRC7WVector(void*, void*, void*);
void* Normalize__7WVectorFv(void*);
void* Vector_CrossProduct(void*, void*, void*);
void* __ml__FfRC7WVector(void*, void*, float);
void* __pl__FRC7WVectorRC7WVector(void*, void*, void*);
void* Matrix_SetIdentityAffine(void*);
void* Vec3_NegateInPlace(void*);
void* appMallocDummy__FUl(int);
void* memset(void*, int, unsigned int);
float Math_SinApprox(float);
float Math_CosApprox(float);
extern unsigned char clubAndPlayerStrings[];
// ringEffectRenderState
extern const DrawStateData ringEffectRenderState;
extern Vec3f ringStripOffset;
char* String_FormatTemporary(const char*, ...);
void appFreeUni__FPv(void*);
}

// Placement construction uses the caller-provided cache-entry storage.
inline void* operator new(size_t, void* p) { return p; }
struct EEListGuard {
    void* head;
    void* node;
    // Own an unlinked cache-node allocation until insertion commits.
    EEListGuard(void* h, void* n) : head(h), node(n) {}
    // Free only the uncommitted node, never its model-set payload.
    ~EEListGuard() { if (node) appFreeUni__FPv(node); }
};

extern "C" {

// Find or load a four-club model set, derive its bone-based trail frames, and append it to the item cache.
// Required self is ClubModelCache; key is an IFF type-id carried as an ABI pointer
// word, not an address to dereference. The IFF set/club records remain borrowed
// from g_sharedDoc. A hit returns the same borrowed entry; a missing set returns 0.
// A miss allocates Data, then loads primary/secondary models for each of four clubs,
// then publishes one cache node. The cache owns the published entry; callers must
// not delete it or outlive cache teardown. This routine performs no eviction.
// Failure is NOT transactional: Data allocation is placement-constructed without
// a null check; primary-load failure skips that club's frame/secondary setup but
// still permits caching the partial set. Data only zeroes the primary handles;
// skipped secondary handles/frames must not be assumed initialized. A secondary
// load is passed unchecked to WPuppet_UpdateBoundsFromResourceBox. Missing bone pairs use an
// identity frame. Missing individual club records are dereferenced, not reported.
// EEListGuard only frees an uncommitted node; it does not roll back Data or loaded
// models if an allocation/load throws. Preserve this existing failure behavior;
// adding rollback or a new model release policy is a separate functional change.
void* ClubModel_CreateForItem(void* self, void* key) {
    float boneDirection[3];
    struct Slot {
        int mode; float directionX, directionY, directionZ;
        int startColor, middleColor, endColor;
        // Initialize the trail slot with no mode, zero direction and a white end color.
        Slot() {
            mode = 0; startColor = 0; middleColor = 0; endColor = kColorWhite;
            directionZ = kFloat_0_Club; directionY = kFloat_0_Club; directionX = kFloat_0_Club;
        }
    };
    struct Data {
        IFF_STRUCT::sClubSet* clubRecord;
        void* models[4];
        void* secondaryModels[4];
        float clubFrames[4][12];
        Slot slots[4];
        // Clear the primary handle array; remaining members are filled by the loader.
        Data() { clubRecord = 0; memset(models, 0, sizeof(models)); }
    };
    float sideDirection[3]; float angleDirection[3]; float upDirection[3];
    float trailOffset[3]; float trailEndpoint[3]; float scaledUp[3]; float fallbackEndpoint[3]; float fallbackUp[3];
    struct Parse { int type; float value; } angleParameter;
    unsigned char volatile flag;
    void* volatile savedKey;
    void* volatile searchKey;
    void* volatile found;
    void* volatile foundCopy;
    void* volatile node1;
    void* volatile nodeCopy;
    void* targetBoneOrClub;
    void* cameraBone;
    const ClubModelStrings* strings;
    char* modelCursor;
    char* trailCursor;
    char* frameOrigin;
    char* frameForward;
    char* frameSide;
    char* frameUp;
    char* trailDirection;
    char* clubRecord;
    char* result;
    void** head;
    int clubKind;
    char* cacheBytes;
    char* cacheSentinel;
    void* cacheNode;
    savedKey = key;
    cacheBytes = (char*)self;
    searchKey = key;
    cacheNode = ((ClubModelCache*)cacheBytes)->sentinel.next;
    cacheSentinel = (char*)&((ClubModelCache*)cacheBytes)->sentinel;
    // Original unit 0x800EE8AC: find the cached club set.
    void* cachedSet;
    void* cachedItem;
    while ((unsigned int)cacheNode != (unsigned int)cacheSentinel &&
        (cachedSet = ((ClubCacheNode*)cacheNode)->value, cachedItem = *(void**)cachedSet,
        (void*)((IffItemCommon*)cachedItem)->typeId != searchKey)) {
        cacheNode = ((ClubCacheNode*)cacheNode)->next;
    }
    found = cacheNode;
    foundCopy = cacheNode;
    if ((unsigned int)cacheNode != (unsigned int)(cacheBytes + kClubCacheSentinelOffset))
        result = (char*)((ClubCacheNode*)cacheNode)->value;
    else
        result = 0;
    if (result)
        return result;

    // Phase 2: cache miss. Resolve the set record and allocate cache-owned storage.
    // Volatile aliases above preserve compiler lifetimes; they are not extra owners.
    clubRecord = (char*)ItemInventory_FindClubSetData((char*)&AsCSharedDoc(g_sharedDoc)->itemInventory, key);
    if (clubRecord) {
        result = (char*)__nw__FUl(sizeof(Data));
        new (result) Data;
        ((ClubModelSet*)result)->item = (IFF_STRUCT::sClubSet*)clubRecord;

        // Phase 3: four parallel cursors visit model handles, 3x4 frames and trails.
        // Models are loaded twice; each handle belongs to the new cache entry.
        modelCursor = result;
        trailCursor = result;
        frameOrigin = (char*)&((ClubModelSet*)result)->clubFrames[0][9];
        frameForward = (char*)&((ClubModelSet*)result)->clubFrames[0][6];
        frameSide = (char*)&((ClubModelSet*)result)->clubFrames[0][0];
        frameUp = (char*)&((ClubModelSet*)result)->clubFrames[0][3];
        trailDirection = (char*)((ClubModelSet*)result)->trails[0].direction;
        clubKind = 0;
        while (kClubKindCount > clubKind)
        {
            targetBoneOrClub = ItemInventory_FindClubData((char*)&AsCSharedDoc(g_sharedDoc)->itemInventory, (void*)((IFF_STRUCT::sClubSet*)clubRecord)->clubIds[0]);
            strings = (const ClubModelStrings*)clubAndPlayerStrings;
            char* name = (char*)String_FormatTemporary(strings->modelFileFormat, ((IFF_STRUCT::sClub*)targetBoneOrClub)->model);
            cameraBone = 0; // 0x800EEBF0: Supply zero flags to the first resource load.
            void* a = ResourceManager_LoadByExtension(g_resourceManager, name, (int)cameraBone, 1, (int)cameraBone);
            cameraBone = 0;
            ((ClubModelSet*)modelCursor)->models[0] = a;
            if (a) {
                WPuppet_UpdateBoundsFromResourceBox(a, 0, 1);
                name = (char*)String_FormatTemporary(strings->modelFileFormat, ((IFF_STRUCT::sClub*)targetBoneOrClub)->model);
                void* b = ResourceManager_LoadByExtension(g_resourceManager, name, (int)cameraBone, 1, (int)cameraBone);
                ((ClubModelSet*)modelCursor)->secondaryModels[0] = b;
                WPuppet_UpdateBoundsFromResourceBox(b, 0, 1);
                cameraBone = WPuppet_FindNamedBoneEntry(((ClubModelSet*)modelCursor)->models[0], strings->cameraBone);
                targetBoneOrClub = WPuppet_FindNamedBoneEntry(((ClubModelSet*)modelCursor)->models[0], strings->targetBone);
                ((ClubModelSet*)trailCursor)->trails[0].startColor = kColorWhite;
                ((ClubModelSet*)trailCursor)->trails[0].middleColor = kColorMiddleGray;
                ((ClubModelSet*)trailCursor)->trails[0].endColor = kColorWhite;
                ((ClubModelSet*)trailCursor)->trails[0].mode = 2;
                // Frame: origin=camera bone; forward=unit(target-camera),
                // side=unit(angleDirection cross forward), up=forward cross side.
                // Parsed angle is degrees; frame components use model bone coordinates.
                if (cameraBone != 0 && targetBoneOrClub != 0) {
                    angleParameter.value = kFloat_0_Club;
                    char* cdata = (char*)((ClubBoneEntry*)cameraBone)->data;
                    angleParameter.type = ((ClubBoneData*)cdata)->parserSeed;
                    String_ScanTokensAfterSubstring((char*)&angleParameter.type, strings->angleMarker, strings->floatFormat, &angleParameter.value, cdata);
                    __as__7WVectorFRC7WVector(frameOrigin, ((ClubBoneEntry*)cameraBone)->data->position);
                    __mi__FRC7WVectorRC7WVector(boneDirection, ((ClubBoneEntry*)targetBoneOrClub)->data->position, ((ClubBoneEntry*)cameraBone)->data->position);
                    __as__7WVectorFRC7WVector(frameForward, Normalize__7WVectorFv(boneDirection));
                    float negativeSine = -Math_SinApprox(kGolfDegreesToRadians * angleParameter.value);
                    angleDirection[1] = Math_CosApprox(kGolfDegreesToRadians * angleParameter.value);
                    angleDirection[0] = kFloat_0_Club;
                    angleDirection[2] = negativeSine;
                    Vector_CrossProduct(sideDirection, angleDirection, frameForward);
                    __as__7WVectorFRC7WVector(frameSide, Normalize__7WVectorFv(sideDirection));
                    Vector_CrossProduct(upDirection, frameForward, frameSide);
                    __as__7WVectorFRC7WVector(frameUp, upDirection);
                    __ml__FfRC7WVector(scaledUp, frameUp, kFloat_10_GolfHud);
                    __pl__FRC7WVectorRC7WVector(trailEndpoint, frameOrigin, scaledUp);
                    __mi__FRC7WVectorRC7WVector(trailOffset, trailEndpoint, ((ClubBoneEntry*)targetBoneOrClub)->data->position);
                    __as__7WVectorFRC7WVector(trailDirection, trailOffset);
                } else {
                    Matrix_SetIdentityAffine(frameSide);
                    __ml__FfRC7WVector(fallbackUp, frameUp, kFloat_10_GolfHud);
                    __pl__FRC7WVectorRC7WVector(fallbackEndpoint, frameOrigin, fallbackUp);
                    __as__7WVectorFRC7WVector(trailDirection, fallbackEndpoint);
                }
                Vec3_NegateInPlace(trailDirection);
                Normalize__7WVectorFv(trailDirection);
            }
            clubRecord += 4;
            modelCursor = modelCursor + 4;
            trailCursor += 0x1C;
            frameOrigin += 0x30;
            frameForward += 0x30;
            frameSide += 0x30;
            frameUp += 0x30;
            trailDirection += 0x1C;
            clubKind++;
        }

        // Phase 4: splice a cache node only after model/frame setup; the guard owns
        // the node allocation until payload construction succeeds. Return is borrowed.
        head = (void**)(cacheBytes + kClubCacheSentinelOffset);
        flag = 0;
        char* node2 = (char*)appMallocDummy__FUl(12);
        node1 = node2;
        nodeCopy = node2;
        EEListGuard guard(head, node2);
        if (8 + node2 != 0) try {
            ((ClubCacheNode*)node2)->value = (ClubModelSet*)result;
        } catch (...) {
            throw;
        }
        guard.node = 0;
        ((ClubCacheNode*)*head)->next = (ClubCacheNode*)node2;
        *(void**)node2 = *(void**)head;
        *(void**)head = node2;
        ((ClubCacheNode*)node2)->next = (ClubCacheNode*)head;
        ((ClubModelCache*)cacheBytes)->count = ((ClubModelCache*)cacheBytes)->count + 1;
    }
    return result;
}

// Build two translucent 16-segment ring strips and replace their previous reference-counted vertex buffers.
void CGeneTex3D_CreateRingVertexBuffers(void* self) {
    RingEffectLayout* object = (RingEffectLayout*)self;
    object->enabled = 0;
    object->angle = kRingInitialAngle;
    Vec3f state = ringStripOffset;
    void* p = object->innerVertices;
    if (p != 0) {
        if (--((RefObj*)p)->count == 0)
            CubeResourceStorage_dtor(p, 1);
        object->innerVertices = 0;
    }
    p = object->outerVertices;
    if (p != 0) {
        if (--((RefObj*)p)->count == 0)
            CubeResourceStorage_dtor(p, 1);
        object->outerVertices = 0;
    }
    // Recreate both buffers, then fill local-space triangle strips. Each angular
    // sample has two vertices: radial point and radial point + ringStripOffset.
    // Only the first vertex receives the vertical offset. i=16 closes the seam.
    VertexBuffer_Create(kRingBufferBytes, 0, kRingVertexFormat, 0, &object->innerVertices, 0);
    VertexBuffer_Create(kRingBufferBytes, 0, kRingVertexFormat, 0, &object->outerVertices, 0);
    unsigned char* vertex;
    Resource_GetOffsetPtr(object->innerVertices, 0, 0, &vertex, 0);
    {
    short j;
    int i;
    unsigned int n;
    for (i = 0, n = 0; i <= kRingSegments; i++, n += 2) {
        for (j = 0; j < 2; ++j) {
            ((Vertex3*)vertex)[j + i * 2].u = (float)i * kRingSegmentFraction;
            ((Vertex3*)vertex)[j + i * 2].v = kRingUvHeight - (float)j;
            ((Vertex3*)vertex)[j + i * 2].a = kInnerRingAlpha;
            ((Vertex3*)vertex)[j + i * 2].b = kOpaqueColorChannel;
            ((Vertex3*)vertex)[j + i * 2].g = kOpaqueColorChannel;
            ((Vertex3*)vertex)[j + i * 2].r = kOpaqueColorChannel;
        }
        float phase = kRingFullTurnRadians * (float)i;
        float angle = phase * kRingSegmentFraction;
        float vec[3] = { 0.0f, 0.0f, 0.0f };
        vec[0] = Math_CosApprox(angle);
        vec[2] = Math_SinApprox(angle);
        Vec3_Scale(vertex + n * kRingVertexBytes, vec, kRingInnerRadius);
        Vec3_Add(vertex + (i * 2 + 1) * kRingVertexBytes, vertex + n * kRingVertexBytes, &state);
        ((Vertex3*)(vertex + n * kRingVertexBytes))->y += kRingVerticalOffset;
    }
    }
    GraphicsBuffer_Flush(object->innerVertices);
    Resource_GetOffsetPtr(object->outerVertices, 0, 0, &vertex, 0);
    {
    short j;
    int i;
    unsigned int n;
    for (i = 0, n = 0; i <= kRingSegments; i++, n += 2) {
        for (j = 0; j < 2; ++j) {
            ((Vertex3*)vertex)[j + i * 2].u = (float)i * kRingSegmentFraction;
            ((Vertex3*)vertex)[j + i * 2].v = kRingUvHeight - (float)j;
            ((Vertex3*)vertex)[j + i * 2].a = kOuterRingAlpha;
            ((Vertex3*)vertex)[j + i * 2].b = kOpaqueColorChannel;
            ((Vertex3*)vertex)[j + i * 2].g = kOpaqueColorChannel;
            ((Vertex3*)vertex)[j + i * 2].r = kOpaqueColorChannel;
        }
        float phase = kRingFullTurnRadians * (float)i;
        float angle = phase * kRingSegmentFraction;
        float vec[3] = { 0.0f, 0.0f, 0.0f };
        vec[0] = Math_CosApprox(angle);
        vec[2] = Math_SinApprox(angle);
        Vec3_Scale(vertex + n * kRingVertexBytes, vec, kRingOuterRadius);
        Vec3_Add(vertex + (i * 2 + 1) * kRingVertexBytes, vertex + n * kRingVertexBytes, &state);
        ((Vertex3*)(vertex + n * kRingVertexBytes))->y += kRingVerticalOffset;
    }
    }
    GraphicsBuffer_Flush(object->outerVertices);
}
extern const DrawStateData ringEffectRenderState;
extern unsigned char kIdentityMatrix4x4[];
extern WView* g_view;

// Skip leading spaces/tabs, copy through the next space, and return the next source position (null at initial end).
// Requires non-null buffers and enough destination capacity for token plus NUL.
// Tabs delimit only the leading skip; inside the copied token only space ends it.
char* String_ParseSpaceDelimitedToken(signed char* source, char* destination) {
    while ((*source == ' ' || *source == '\t') && *source != 0)
        ++source;
    if (*source == 0) {
        source = 0;
        *destination = 0;
        return (char*)source;
    }
    while (*source != 0 && *source != ' ')
        *destination++ = *source++;
    *destination = 0;
    return (char*)source;
}

// Skip leading whitespace and copy a quoted token; non-quote input produces an empty token and advances once.
// source must be non-null despite the initial conditional whitespace scan;
// destination must fit the token plus NUL. Backslash escapes are not interpreted.
char* String_ParseQuotedToken(signed char* source, char* destination) {
    if (source != 0) {
        while (*source != 0 && (*source == ' ' || *source == '\t'))
            ++source;
    }
    if (*source == 0) {
        *destination = 0;
        return (char*)source;
    }
    if (*source != '"') {
        int z = 0;
        ++source;
        *destination = (char)z;
        return (char*)source;
    }
    ++source;
    while (*source != 0 && *source != '"')
        *destination++ = *source++;
    if (*source == 0) {
        *destination = 0;
        return (char*)source;
    }
    int z = 0;
    ++source;
    *destination = (char)z;
    return (char*)source;
}

// Sample minimum + (sample % 1023)/1024 * (maximum-minimum); negative requests rand().
// For nonnegative samples the fraction is 0..1022/1024, so maximum is unreachable
// and only bin 1023 of the 1024 fractional bins is excluded; bin 1022 is reachable.
// Keep this original biased distribution.
// No endpoint reordering occurs when maximum < minimum.
float Random_RangeFloat(int randomSample, float minimum, float maximum) {
    if (randomSample < 0)
        randomSample = rand();
    int wrapped = randomSample % kRandomSampleModulus;
    return FP_FMADDS((float)wrapped * kRandomFractionScale1024, (maximum - minimum), minimum);
}

// Replace the primitive vertex array with zeroed storage for the requested vertex count.
int CGeneTex3D_AllocVertexBuffer(void* self, int count) {
    if (((GeneTexBufferLayout*)self)->vertices != 0) {
        __dla__FPv(((GeneTexBufferLayout*)self)->vertices);
        ((GeneTexBufferLayout*)self)->vertices = 0;
    }
    if (count != 0) {
        ((GeneTexBufferLayout*)self)->vertices = __nwa__FUl(count * sizeof(Vertex3));
        memset(((GeneTexBufferLayout*)self)->vertices, 0, (unsigned int)count * (unsigned int)sizeof(Vertex3));
    }
    return count & -(((GeneTexBufferLayout*)self)->vertices != 0);
}

// Advance an enabled ring effect angle and rotate its two rings in opposite directions.
void RingEffect_UpdateRotation(void* self, float deltaSeconds) {
    float fullTurn;
    float previousAngle;
    float angularSpeed;
    float angle;
    if (((RingEffectLayout*)self)->enabled != 0) {
        fullTurn = kRingFullTurnRadians;
        angularSpeed = kRingAngularSpeedRadiansPerSecond;
        previousAngle = ((RingEffectLayout*)self)->angle;

        angle = angularSpeed * deltaSeconds + previousAngle;
        ((RingEffectLayout*)self)->angle = angle;
        if (angle > fullTurn)
            ((RingEffectLayout*)self)->angle = angle - fullTurn;
        angle = ((RingEffectLayout*)self)->angle;
        Mtx44_SetRotationY(((RingEffectLayout*)self)->innerRotation, angle);
        angle = ((RingEffectLayout*)self)->angle;
        Mtx44_SetRotationY(((RingEffectLayout*)self)->outerRotation, -angle);
    }
}

// Restore the primitive vtable, free its CPU vertex array, and optionally delete the buffer object.
void* __dt__13GeneTexBufferFv(void* self, int deleting) {
    if (self != 0) {
        void* data = ((GeneTexBufferLayout*)self)->vertices;
        ((GeneTexBufferLayout*)self)->vtable = __vt__10CGeneTex3D;
        if (data != 0) {
            __dla__FPv(data);
            ((GeneTexBufferLayout*)self)->vertices = 0;
        }
        if (deleting > 0)
            __dl__FPv(self);
    }
    return self;
}

// Optionally free the object; this wrapper has no owned resources to release.
void* dtor_800EF304(void* self, int deleting) {
    if (self != 0 && deleting > 0)
        __dl__FPv(self);
    return self;
}

// Return the cached club-model set for an IFF item key, or null if the circular-list search misses.
void* ClubModel_FindCachedForItem(void* self, void* key) {
    void* sentinel = &((ClubModelCache*)self)->sentinel;
    void* node = ((ClubModelCache*)self)->sentinel.next;
    // Original 0x800EEED8: stop at the sentinel or the matching key.
    void* value;
    void* object;
    while (node != sentinel &&
        (value = ((ClubCacheNode*)node)->value, object = *(void**)value,
        (void*)((IffItemCommon*)object)->typeId != key)) {
        node = ((ClubCacheNode*)node)->next;
    }
    if ((unsigned int)node != (unsigned int)self + 0x80) return ((ClubCacheNode*)node)->value;
    return 0;
}

} // extern C
struct GeneTexBuffer {
    char storage[12];
    ~GeneTexBuffer();
};
struct CGeneTex3D {
    char prefix[0xd4];
    GeneTexBuffer buffer;
    CGeneTex3D(void*);
};
// Load the ring texture, initialize the embedded primitive, and create both ring vertex buffers.
CGeneTex3D::CGeneTex3D(void* arg) {
    void* self = this;
    ((RingEffectLayout*)self)->texture = 0;
    ((RingEffectLayout*)self)->innerVertices = 0;
    ((RingEffectLayout*)self)->outerVertices = 0;
    ((DrawPrimitive*)&((RingEffectLayout*)self)->primitive)->flag = 0;
    ((RingEffectLayout*)self)->primitive.vtable = __vt__10CGeneTex3D;
    ((RingEffectLayout*)self)->primitive.vertices = 0;
    ((RingEffectLayout*)self)->texture = ResourceManager_LoadBitmap(g_resourceManager, arg, 0, 0, 0, __vt__10CGeneTex3D);
    CGeneTex3D_CreateRingVertexBuffers(self);
}
extern "C" {

// Set texture UV coordinates on one CPU-side primitive vertex.
void CGeneTex3D_SetTexCoord(void* self, int index, float x, float y) {
    int byteOffset = index * sizeof(Vertex3);
    // CGeneTex3D_SetTexCoord: texture coordinates +0x10/+0x14 in 0x18-byte Vertex3.
    ((Vertex3*)((char*)((GeneTexBufferLayout*)self)->vertices + byteOffset))->u = x;
    ((Vertex3*)((char*)((GeneTexBufferLayout*)self)->vertices + byteOffset))->v = y;
}

// Allocate the shared 64-slot temporary formatting arena on first use.
// Original pointer-returning ABI supplies no C return statement here. Call only
// for its allocation side effect; a defined return pointer is not established.
void* String_InitTemporaryFormatBuffers() {
    if (s_temporaryFormatArena == 0)
        s_temporaryFormatArena = (int)__nwa__FUl(kFormatStorageBytes);
// The original returns no value: its callers never read one.
}

// Set one primitive vertex color from arguments in alpha, red, green, blue order.
void CGeneTex3D_SetVertexColorARGB(void* self, int index, unsigned char alpha, unsigned char red, unsigned char green, unsigned char blue) {
    ((Vertex3*)((GeneTexBufferLayout*)self)->vertices)[index].a = alpha;
    ((Vertex3*)((GeneTexBufferLayout*)self)->vertices)[index].r = red;
    ((Vertex3*)((GeneTexBufferLayout*)self)->vertices)[index].g = green;
    ((Vertex3*)((GeneTexBufferLayout*)self)->vertices)[index].b = blue;
}

// Initialize this unit's second shared floating-point state slot from the common default.
void ClubEffects_InitFloatState1() { s_clubEffectsFloatState1 = kFloat_6_2831855; }

// Scale packed RGB components, clamp each to the stored maximum, and return packed RGB without alpha.
// Only the upper bound is clamped. Negative factors are truncated and masked
// back to bytes rather than clamped to black; preserve this caller-visible behavior.
unsigned int Color_ScaleRGB(unsigned int color, float factor) {

    unsigned int redByte = (color >> 16) & 0xff;

    float redScaled = factor * (float)redByte;
    float redClamp = kRgbMaximumComponent;
    redClamp = redScaled > redClamp ? redClamp : factor * (float)redByte;
    unsigned int greenByte = (color >> 8) & 0xff;

    unsigned int redRes = (int)redClamp;

    float greenScaled = factor * (float)greenByte;
    float greenClamp = kRgbMaximumComponent;
    greenClamp = greenScaled > greenClamp ? greenClamp : factor * (float)greenByte;
    unsigned int greenRes = (int)greenClamp;

    float blueClamp = kRgbMaximumComponent;

    unsigned int blueByte = color & 0xff;
    float blueScaled = factor * (float)blueByte;
    blueClamp = blueScaled > blueClamp ? blueClamp : factor * (float)blueByte;

    return ((redRes & 0xff) << 16) | (((int)blueClamp) & 0xff) | ((greenRes & 0xff) << 8);
}

// Release both reference-counted ring buffers, the texture resource, and the embedded primitive vertex storage.
void* CGeneTex3D_Dtor(void* self, int deleting) {
    if (self != 0) {
        RingEffectLayout* ring = (RingEffectLayout*)self;
        void* a = ring->innerVertices;
        if (a) {
            if (--((RefObj*)a)->count == 0) CubeResourceStorage_dtor(a, 1);
            ring->innerVertices = 0;
        }
        a = ring->outerVertices;
        if (a) {
            if (--((RefObj*)a)->count == 0) CubeResourceStorage_dtor(a, 1);
            ring->outerVertices = 0;
        }
        if (ring->texture) { ResourceManager_ReleaseId(g_resourceManager, ring->texture); ring->texture = 0; }
        if (&((RingEffectLayout*)self)->primitive) {
            ((RingEffectLayout*)self)->primitive.vtable = __vt__10CGeneTex3D;
            a = ((RingEffectLayout*)self)->primitive.vertices;
            if (a) { __dla__FPv(a); ((RingEffectLayout*)self)->primitive.vertices = 0; }
        }
        if (deleting > 0) __dl__FPv(self);
    }
    return self;
}

// Format into the next 576-byte temporary slot; the 64-slot arena wraps and overwrites older strings.
char* String_FormatTemporary(const char* format, ...) {
    if (s_temporaryFormatArena == 0)
        s_temporaryFormatArena = (int)__nwa__FUl(kFormatStorageBytes);
    int fields[3];
    fields[0] = 0x01000000;
    fields[1] = (int)fields + 0x20;
    fields[2] = (int)((char*)fields - 0x60);
    String_VFormatBoundedCopy((char*)s_temporaryFormatArena + (s_temporaryFormatSlot & (kFormatBufferCount - 1)) * kFormatBufferBytes, kFormatBufferBytes, format, fields);
    int buffer = s_temporaryFormatArena;
    unsigned char idx = s_temporaryFormatSlot;
    s_temporaryFormatSlot = (unsigned char)(idx + 1);
    return (char*)buffer + (idx & (kFormatBufferCount - 1)) * kFormatBufferBytes;
}
#pragma opt_common_subs off
// Apply the camera and ring render state, bind the texture, and draw the two oppositely rotated ring strips.
void RingEffect_Draw(void* self) {
    const DrawStateData* data = &ringEffectRenderState;
    if (((RingEffectLayout*)self)->enabled != 0) {
        // Phase 1: apply the stored render/TEV state and the active camera matrices.
        DrawWords30 state = data->state;
        unsigned char matrix[0x40];
        DrawWords10 params;
        DrawWords12 tev;
        tev = data->tev;
        params = data->params;
        Engine_SetRenderStateList(&state, 0xf);
        Engine_SetTextureStageStateList(0, &tev, 6);
        Engine_SetTextureStageParameterList(0, &params, 5);
        Engine_SetCachedRenderState(0xd, 4);
        Engine_SetCachedRenderState(0xe, 1);
        Camera_Apply(g_view, 0.0f, 0.0f);
        void* camera = Camera_GetRenderState(g_view);
        SetTransform(0, ((CameraRenderMatrices*)camera)->view);
        SetTransform(1, ((CameraRenderMatrices*)camera)->projection);
        Engine_SetType7Resource(0);
        ((DrawPrimitive*)(&((RingEffectLayout*)self)->primitive))->apply();
        Engine_SetCachedRenderState(5, 0);
        SetTransform(2, kIdentityMatrix4x4);
        Engine_SetFlexibleVertexFormat(kRingVertexFormat);
        // Phase 2: resolve the borrowed texture-stage resources for this render target.
        void* a = ((DrawDevice*)g_view->renderTarget)->texture(((RingEffectLayout*)self)->texture);
        Engine_SetTextureStageResource(0, a);
        a = ((DrawDevice*)g_view->renderTarget)->texturePalette(((RingEffectLayout*)self)->texture);
        Engine_SetTexturePalette(0, a);
        // Phase 3: submit inner and outer strips using opposite rotations.
        // PSMTX44Concat(dst, first, second), implemented in gtl_vector_matrix_math.cpp, writes
        // scratch = rotation * worldMatrix. Matrices are row-major, row-vector:
        // p' = p * rotation * worldMatrix (translation at float indices 12..14).
        // SetTransform(0xB) consumes that scratch matrix for each strip; neither
        // persistent input is changed. Keep the two products in submission order.
        PSMTX44Concat(matrix, ((RingEffectLayout*)self)->innerRotation, ((RingEffectLayout*)self)->worldMatrix);
        SetTransform(0xb, matrix);
        Engine_SetVertexStream(0, ((RingEffectLayout*)self)->innerVertices, 0, kRingVertexBytes);
        DrawPrimitiveFromBoundBuffers(kRingPrimitive, 0, kRingTriangleCount);
        PSMTX44Concat(matrix, ((RingEffectLayout*)self)->outerRotation, ((RingEffectLayout*)self)->worldMatrix);
        SetTransform(0xb, matrix);
        Engine_SetVertexStream(0, ((RingEffectLayout*)self)->outerVertices, 0, kRingVertexBytes);
        DrawPrimitiveFromBoundBuffers(kRingPrimitive, 0, kRingTriangleCount);
    }
}
#pragma opt_common_subs reset
// Select the textured position/color/UV vertex format for primitive drawing.
void CGeneTex3D_SelectVertexFormat() { Engine_SetFlexibleVertexFormat(0x11F001); }
// Initialize this unit's third shared floating-point state slot from the common default.
void ClubEffects_InitFloatState2() { s_clubEffectsFloatState2 = kFloat_6_2831855; }

// Set the ring effect world-matrix translation from an XYZ vector.
void CGeneTex3D_SetTranslation(void* self, float* value) {
    Mtx44_SetTranslation(((RingEffectLayout*)self)->worldMatrix, value[0], value[1], value[2]);
}

// Draw the CPU-side vertex array with the caller's primitive kind and count.
void CGeneTex3D_DrawVertices(void* self, int primitive, int count) {
    DrawPrimitiveUP(primitive, count, ((GeneTexBufferLayout*)self)->vertices, sizeof(Vertex3));
}

// Copy one primitive vertex XYZ position into the caller's output vector.
void CGeneTex3D_SetVertexPosition(void* self, int index, void* destination) {
    Vec3_CopyToOutput(destination, &((Vertex3*)((GeneTexBufferLayout*)self)->vertices)[index]);
}

// Default primitive hook; performs no work.
void CGenericPrimitive_vfunc_6() {}
// Default primitive hook; performs no work.
void CGenericPrimitive_vfunc_5() {}
// Default primitive hook; performs no work.
void CGenericPrimitive_vfunc_4() {}
// Default primitive hook; performs no work.
void CGenericPrimitive_vfunc_3() {}
// Default primitive allocation hook; reports no vertices.
int CGenericPrimitive_vfunc_2() { return 0; }

extern float kFloat_6_2831855;
// s_clubEffectsFloatState0
extern float s_clubEffectsFloatState0;
// Initialize this unit's first shared floating-point state slot from the common default.
void ClubEffects_InitFloatState0() { s_clubEffectsFloatState0 = kFloat_6_2831855; }

} // extern "C"

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU804620F0", _ctors_u800EE8AC_0, ClubEffects_InitFloatState0);
STATIC_INIT_ENTRY(".CRT$XCU804620F0", _ctors_u800EE8AC_1, ClubEffects_InitFloatState1);
STATIC_INIT_ENTRY(".CRT$XCU804620F0", _ctors_u800EE8AC_2, ClubEffects_InitFloatState2);
#pragma pop

#include "classes/WView.h"
#define AsWView(p) ((WView*)(p))
#include "classes/CSharedDoc.h"
#define AsCSharedDoc(p) ((CSharedDoc*)(p))
#include "game/types.h"
#include "sdk_protos.h"
#include "game/exception.h"
#include "classes/NtRiverLavaLayout.h"
#include "iff/IffStruct.h"
#include "classes/CRiver.h"
#include <decomp/cw_bitfields.h>

#define AsCRiver(p) ((CRiver*)(p))

// Additional owned buffers and the inline material, beyond the common lava view.
struct LavaOwnedState {
    char reserved[0x4C];
    void* auxiliaryData;
    int auxiliarySize;
    int auxiliaryCapacity;
    unsigned char materialFlags;
    char alignment[3];
    void* materialVtable;
    void* materialData;
};
#define LavaOwned(p) ((LavaOwnedState*)(p))
// Translation unit 0x801E356C-0x801E4C08 (6 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

typedef Vec3f Vec3;
// Packed texture and exception strings; member sizes include NUL.
// Use field-derived byte offsets because direct member pointers change address hoisting.
struct RiverTextureNames {
    char separator[2];
    char layerIdFormat[3];
    char courseTextureFormat[17];
    char upstream[3];
    char downstream[5];
    char pinkTextureFormat[19];
    char windTextureFormat[19];
    char vectorLengthError[20];
    char lengthErrorType[55];
    char lavaTextureFormat[20];
    char alignment[1];
};
enum RiverStringOffset {
    kRiverName_separator = (int)&((RiverTextureNames*)0)->separator,
    kRiverName_layerIdFormat = (int)&((RiverTextureNames*)0)->layerIdFormat,
    kRiverName_courseTextureFormat = (int)&((RiverTextureNames*)0)->courseTextureFormat,
    kRiverName_upstream = (int)&((RiverTextureNames*)0)->upstream,
    kRiverName_downstream = (int)&((RiverTextureNames*)0)->downstream,
    kRiverName_pinkTextureFormat = (int)&((RiverTextureNames*)0)->pinkTextureFormat,
    kRiverName_windTextureFormat = (int)&((RiverTextureNames*)0)->windTextureFormat,
    kRiverName_vectorLengthError = (int)&((RiverTextureNames*)0)->vectorLengthError,
    kRiverName_lengthErrorType = (int)&((RiverTextureNames*)0)->lengthErrorType,
    kRiverName_lavaTextureFormat = (int)&((RiverTextureNames*)0)->lavaTextureFormat,
};

// Camera_GetRenderState supplies two consecutive 4x4 matrices at these offsets.
struct RiverCameraState {
    char reserved[0x20];
    float view[16];
    float projection[16];
};

// Construct a path node directly in its caller-provided allocation.
inline void* operator new(size_t, void* p) { return p; }
struct NodeExtra { float value[2]; };
// Two bank positions and cumulative V coordinates, one pair per river section.
struct PathNode {
    Vec3 pos[2]; float vCoordinates[2];
    // Leave bank positions and V coordinates for the section builder to initialize.
    PathNode() {}
    // Copy one bank pair and its V coordinates with original exception boundaries.
    PathNode(const PathNode& p) {
        try {
        *(Arr3*)&pos[0] = *(const Arr3*)&p.pos[0];
        *(Arr3*)&pos[1] = *(const Arr3*)&p.pos[1];
        *(NodeExtra*)vCoordinates = *(const NodeExtra*)p.vCoordinates;
        } catch (...) { throw; }
    }
};
struct Box3 { Vec3 min, max; };
struct NodeArray { PathNode* begin; unsigned int size; unsigned int cap; };
struct RiverTextureLayer {
    float vOffset;
    float vScrollSpeed;
    void* bitmap;
    float vScale;
    float uExtent;
    float heightOffset;
    unsigned char enabled;
    char pad[3];
};
struct RiverMeshState {
    void* vtable;
    float halfWidth;
    NodeArray nodes;
    RiverTextureLayer layers[2];
    char materialStorage[0x18];
    void* vertexBuffers[2];
};
struct RiverCenterline { Vec3* points; unsigned int count; };
struct RenderStates { unsigned int words[30]; };
struct RenderStages { unsigned int words[12]; };
struct RenderSamplers { unsigned int words[10]; };
struct RenderConstants { RenderStates states; RenderStages stages; RenderSamplers samplers; };

// These are engine wrapper IDs, not GX register numbers. Only transform roles
// established by the supplied matrices are named; state/parameter meanings remain
// unresolved rather than borrowing similarly numbered constants from another API.
enum RiverDrawConstants {
    kRiverViewTransform = 0, kRiverProjectionTransform = 1,
    kRiverIdentityTransform2 = 2, kRiverIdentityTransform11 = 11,
    kRiverStatePairCount = 15, kRiverStagePairCount = 6, kRiverSamplerPairCount = 5,
    kRiverVertexBytes = 0x18, kRiverSectionVertexBytes = 0x30,
    kRiverVertexFormat = 0x11F001, kRiverTriangleStrip = 0x98
};

extern "C" {
extern void* g_resourceManager;
extern const char __vt__10CGeneTex3D[];
extern char __vt__12ntRiver_Lava[];
extern float kFloat_0_River;
extern float kFloat_1_River;
extern float kFloat_3_2_river;
extern float kFloat_1_6_River;
extern float kFloat_neg1_River;
extern float kFloat_2_24_River;
extern float kFloat_3_River;
extern float kFloat_10_56_River;
extern RenderConstants riverRenderStateDefaults;
extern void* g_view;
extern const unsigned int kIdentityMatrix4x4[];
extern CSharedDoc* g_sharedDoc;
extern const char riverTextureStrings[];
extern char __vt__Q23std11logic_error[], s_sharedCharDeleterVtable[], __vt__Q23std12length_error[];
float Random_RangeFloat(int, float, float);
char* String_FormatTemporary(const char*, ...);
void* ResourceManager_LoadBitmap(void*, char*, int, int, int);
int SharedDoc_GetCourseId();
void ResourceManager_ReleaseId(void*, void*);
void appFreeUni__FPv(void*);
void __dla__FPv(void*);
void __dl__FPv(void*);
void CubeResourceStorage_dtor(void*, int);
void* __nwa__FUl(int);
void Engine_SetRenderStateList(void*, int);
void Engine_SetTextureStageStateList(int, void*, int);
void Engine_SetTextureStageParameterList(int, void*, int);
void* Camera_GetRenderState(void*);
void SetTransform(int, const void*);
void Engine_SetType7Resource(int);
void Engine_SetTextureStageState(int, int, int);
void Engine_SetTextureStageParameter(int, int, int);
void Engine_SetCachedRenderState(int, int);
void Resource_GetOffsetPtr(void*, int, int, void**, int);
void GraphicsBuffer_Flush(void*);
void Engine_SetTextureStageResource(int, int);
void Engine_SetTexturePalette(int, int);
void Engine_SetVertexStream(int, void*, int, int);
void DrawPrimitiveFromBoundBuffers(int, int, int);
void Vec3_Zero(Vec3*);
void __mi__FRC7WVectorRC7WVector(Vec3*, Vec3*, Vec3*);
void __as__7WVectorFRC7WVector(Vec3*, Vec3*);
void Vector_CrossProduct(Vec3*, Vec3*, Vec3*);
Vec3* Normalize__7WVectorFv(Vec3*);
void __ml__FfRC7WVector(Vec3*, Vec3*, float);
void __pl__FRC7WVectorRC7WVector(Vec3*, Vec3*, Vec3*);
void __amu__7WVectorFf(Vec3*, float);
void __ml__FRC7WVectorf(Vec3*, Vec3*, float);
float WVectorLen__FRC7WVector(Vec3*);
void* CQuadTree_GetInstance();
float CQuadTree_GetLowestGround(void*, const Vec3&);
// A by-value Box3 argument is a pointer to a copy in CodeWarrior's ABI.
void GroundRegionList_AppendBounds(void*, Box3, int);
#define GRL_BOX_ARG(b) (b)
void* appMallocDummy__FUl(unsigned int);
void VertexBuffer_Create(int, int, unsigned int, int, void**, int);
void Vector_CopyComponents(void*, void*);
void CRiver_LoadTextureLayers(void*, char*);
} // extern "C"

struct NodeAllocation {
    char* pointer; char* allocator;
    // Guard allocated diagnostic text until its control block takes ownership.
    NodeAllocation(char* p, char* a) : pointer(p), allocator(a) {}
    // Free diagnostic text if ownership transfer did not finish.
    ~NodeAllocation() { if (pointer) __dla__FPv(pointer); }
};
struct NodeControlBase {
    void* table; unsigned strong, weak;
    // Initialize one strong and weak reference for diagnostic text ownership.
    NodeControlBase() { strong = 1; weak = 1; table = s_sharedCharDeleterVtable; }
};
struct NodeControl : NodeControlBase {
    char* pointer;
    // Store the diagnostic allocation in its shared control block.
    NodeControl(char* p) { pointer = p; }
};
namespace std {
struct logic_error : exception {
    // Install the local exception ABI vtable.
    void SetTable(const void* vt) { *(const void**)this = vt; }
    char* text; void* control;
    // Copy the river vector diagnostic into reference-counted string storage.
    logic_error() {
        SetTable(__vt__Q23std11logic_error);
        const char* message = (riverTextureStrings + kRiverName_vectorLengthError);
        struct Allocator { char value; char pad[3]; } allocator;
        allocator.value = 0;
        unsigned n = strlen(message) + 1;
        text = new char[2 * n - n];
        {
            char* p = text;
            NodeAllocation allocation(p, &allocator.value);
            control = new NodeControl(p);
            allocation.pointer = 0;
        }
        strcpy(text, message);
    }

    ~logic_error();
};
struct length_error : logic_error {
    // Specialize the constructed logic error as a length error.
    length_error() : logic_error() {
        SetTable(__vt__Q23std12length_error);
    }

    ~length_error();
};
}
struct NodeBuf3 {
    PathNode* data; unsigned int size; unsigned int cap;
    // Initialize an empty guarded section buffer for initial reserve.
    NodeBuf3() : data(0), size(0), cap(0) {}
    // Free the section allocation currently owned by this reserve buffer.
    ~NodeBuf3() { if (data) { size = size - size; appFreeUni__FPv(data); } }
};
struct NodeBuf5 {
    PathNode* data; unsigned int size; unsigned int cap; unsigned int* owner; unsigned int offset;
    // Initialize a split buffer for suffix construction and prefix relocation.
    NodeBuf5(unsigned int* p) : data(0), size(0), cap(0), owner(p), offset(0) {}
    // Free the allocation left in the split buffer after unwind or commit.
    ~NodeBuf5() { if (data) { size = size - size; appFreeUni__FPv(data); } }
};
// Return the larger section-vector growth bound by reference.
inline const unsigned int& NodeMax(const unsigned int& a, const unsigned int& b) { return a < b ? b : a; }
struct RC801E3830 { CW_BITS2( unsigned hi : 16, unsigned lo : 16) };
// Byte cursors preserve the original scheduling of the two 24-byte vertices per section.
inline unsigned char* RiverVertexBytes(void* vertices, unsigned int vertexOffset) {
    return (unsigned char*)vertices + vertexOffset;
}

extern "C" {
// Build terrain-following bank sections, register river bounds and allocate two textured vertex strips.
// centerlineArg borrows a RiverCenterline with count >= 2 and valid points.
// Coordinates are terrain/world space, Y up. Every segment must have a nonzero
// XZ projection for normalization. The bank-width search samples only the positive
// lateral ray at point[1]: terrain MUST eventually exceed the center height plus
// the larger layer offset along that ray. There is no bound/timeout or fallback.
// self owns the appended nodes and replacement vertex buffers; the centerline and
// name are borrowed. Repeated calls append to existing nodes (not cleared here).
void CRiver_BuildTerrainStrips(void* self_, void* centerlineArg, char* name) {
    RiverMeshState* self = (RiverMeshState*)self_;
    RiverCenterline* centerline = (RiverCenterline*)centerlineArg;
    CRiver_LoadTextureLayers(self, name);

    Box3 box;
    Vec3_Zero(&box.min);
    Vec3_Zero(&box.max);

    // Phase 1: use the first flattened segment crossed with +Y to get a unit
    // lateral direction. bankOffset later becomes this direction times halfWidth.
    Vec3 firstSegmentTangent;
    __mi__FRC7WVectorRC7WVector(&firstSegmentTangent, centerline->points + 1, centerline->points);
    // Reused for the flattened tangent, then the lateral direction and bank offset.
    Vec3 bankOffset;
    __as__7WVectorFRC7WVector(&bankOffset, &firstSegmentTangent);
    bankOffset.y = kFloat_0_River;
    Vec3 up;
    up.x = kFloat_0_River;
    up.y = kFloat_1_River;
    up.z = kFloat_0_River;
    Vec3 unitLateralScratch;
    Vector_CrossProduct(&unitLateralScratch, &bankOffset, &up);
    __as__7WVectorFRC7WVector(&bankOffset, Normalize__7WVectorFv(&unitLateralScratch));

    float bankHeight = self->layers[1].heightOffset;
    float firstLayerHeight = self->layers[0].heightOffset;
    self->halfWidth = kFloat_0_River;
    bankHeight = (firstLayerHeight < bankHeight) ? bankHeight : firstLayerHeight;
    Vec3 heightPoint = centerline->points[1];
    bankHeight = CQuadTree_GetLowestGround(CQuadTree_GetInstance(), heightPoint) + bankHeight;

    // Phase 2: grow in 3.2 world-unit steps until one bank sample clears both
    // layers, then add one safety step. bankSampleWorld is the sampled world
    // position; bankStepWorld is its lateral displacement from centerline->points[1].
    Vec3 bankSampleWorld;
    float height;
    do {
        Vec3 bankStepWorld;
        Vec3* start = centerline->points + 1;
        self->halfWidth = self->halfWidth + kFloat_3_2_river;
        __ml__FfRC7WVector(&bankStepWorld, &bankOffset, self->halfWidth);
        __pl__FRC7WVectorRC7WVector(&bankSampleWorld, start, &bankStepWorld);
        Vec3 heightPoint = bankSampleWorld;
        height = CQuadTree_GetLowestGround(CQuadTree_GetInstance(), heightPoint);
    } while (!(height > bankHeight));
    self->halfWidth = self->halfWidth + kFloat_3_2_river;

    // Phase 3: reserve 50 sections, preserving old nodes and transferring ownership.
    if (self->nodes.cap < 0x32) {
        NodeBuf3 tmp;
        tmp.data = (PathNode*)appMallocDummy__FUl(0x640);
        tmp.cap = 0x32;
        PathNode* end = self->nodes.begin + self->nodes.size;
        PathNode* dst = tmp.data + tmp.size;
        for (PathNode* p = self->nodes.begin; p < end; ++p, ++dst) {
            new (dst) PathNode(*p);
            tmp.size = tmp.size + 1;
        }
        PathNode* oldBegin = self->nodes.begin;
        self->nodes.size = 0;
        PathNode* newBegin = tmp.data;
        unsigned int oldCap = self->nodes.cap;
        unsigned int newSize = tmp.size;
        tmp.data = oldBegin;
        self->nodes.cap = tmp.cap;
        tmp.cap = oldCap;
        self->nodes.begin = newBegin;
        self->nodes.size = newSize;
        tmp.size = 0;
    }

    // Phase 4: sample each segment at max(1, ceil(abs(delta ground Y)/1.6))
    // intervals. segmentFraction is a unitless fraction excluding the endpoint.
    // sampleWorldPosition is interpolated world position; sampleGroundY/lastGroundY
    // hold terrain Y (not elapsed time). Accepted samples create banks at
    // sampleWorldPosition +/- bankOffset. Each bank's cumulative
    // V increases by layer[i].vScale * world distance / halfWidth. Adjacent bank
    // pairs register a terrain box; node growth below preserves prior sections.
    unsigned int outputIndex = 0;
    for (unsigned int seg = 0; seg < centerline->count - 1; ++seg) {
        unsigned int nextOffset = (seg + 1) * 0xc;
        Vec3 sv;
        __mi__FRC7WVectorRC7WVector(&sv, (Vec3*)((char*)centerline->points + nextOffset), centerline->points + seg);
        __as__7WVectorFRC7WVector(&bankOffset, &sv);
        bankOffset.y = kFloat_0_River;
        Vec3 up2;
        up2.x = kFloat_0_River;
        up2.y = kFloat_1_River;
        up2.z = kFloat_0_River;
        Vec3 sp;
        Vector_CrossProduct(&sp, &bankOffset, &up2);
        __as__7WVectorFRC7WVector(&bankOffset, Normalize__7WVectorFv(&sp));
        __amu__7WVectorFf(&bankOffset, self->halfWidth);

        Vec3* segmentPoints = centerline->points + seg;
        Vec3 p0 = segmentPoints[0];
        Vec3 p1 = segmentPoints[1];
        float startGroundY = CQuadTree_GetLowestGround(CQuadTree_GetInstance(), p0);
        float endGroundY = CQuadTree_GetLowestGround(CQuadTree_GetInstance(), p1);
        float delta = endGroundY - startGroundY;
        delta = (delta > kFloat_0_River) ? delta : -delta;
        float sampleIntervals = (float)ceil(delta / kFloat_1_6_River);
        sampleIntervals = (kFloat_1_River < sampleIntervals) ? sampleIntervals : kFloat_1_River;

        Vec3 sampleWorldPosition;
        float lastGroundY = kFloat_0_River;
        float segmentFraction;
        segmentFraction = lastGroundY;
        PathNode node;
        while (segmentFraction < kFloat_1_River) {
            Vec3 a1, a2, ip;
            Vec3* points = centerline->points;
            __ml__FfRC7WVector(&a1, (Vec3*)((char*)points + nextOffset), segmentFraction);
            __ml__FfRC7WVector(&a2, (Vec3*)((char*)points + seg * 0xc), kFloat_1_River - segmentFraction);
            __pl__FRC7WVectorRC7WVector(&ip, &a2, &a1);
            __as__7WVectorFRC7WVector(&sampleWorldPosition, &ip);
            Vec3 cpCopy = sampleWorldPosition;
            float sampleGroundY = CQuadTree_GetLowestGround(CQuadTree_GetInstance(), cpCopy);
            float previousGroundY = lastGroundY;
            lastGroundY = sampleGroundY;
            if (!(sampleGroundY > kFloat_1_6_River + previousGroundY)) {
                for (unsigned int i = 0; i < 2; ++i) {
                    float sgn = (i & 1) ? kFloat_1_River : kFloat_neg1_River;
                    Vec3 off, np;
                    __ml__FRC7WVectorf(&off, &bankOffset, sgn);
                    __pl__FRC7WVectorRC7WVector(&np, &sampleWorldPosition, &off);
                    __as__7WVectorFRC7WVector(&node.pos[i], &np);
                    node.pos[i].y = sampleGroundY + self->layers[0].heightOffset;
                    if (outputIndex == 0) {
                        node.vCoordinates[i] = kFloat_0_River;
                    } else {
                        PathNode* prev = self->nodes.begin + (outputIndex - 1);
                        Vec3 d;
                        __mi__FRC7WVectorRC7WVector(&d, &node.pos[i], &prev->pos[i]);
                        float len = WVectorLen__FRC7WVector(&d);
                        node.vCoordinates[i] = prev->vCoordinates[i] + (self->layers[i].vScale * len) / self->halfWidth;
                    }
                }

                if (outputIndex != 0) {
                    float fv = self->nodes.begin[outputIndex - 1].pos[1].x;
                    float fw = self->nodes.begin[outputIndex - 1].pos[0].x;
                    fv = (fw < fv) ? fv : fw;
                    float fu = node.pos[1].x;
                    float ft = node.pos[0].x;
                    fu = (ft < fu) ? fu : ft;
                    fv = (fu < fv) ? fv : fu;
                    box.max.x = fv;

                    float fy = node.pos[0].y;
                    float fp = self->nodes.begin[outputIndex - 1].pos[0].y;
                    fp = (fy > fp) ? fp : fy;
                    box.max.y = fp;

                    fv = self->nodes.begin[outputIndex - 1].pos[1].z;
                    fw = self->nodes.begin[outputIndex - 1].pos[0].z;
                    fv = (fw < fv) ? fv : fw;
                    fu = node.pos[1].z;
                    ft = node.pos[0].z;
                    fu = (ft < fu) ? fu : ft;
                    fv = (fu < fv) ? fv : fu;
                    box.max.z = fv;

                    fv = self->nodes.begin[outputIndex - 1].pos[1].x;
                    fw = self->nodes.begin[outputIndex - 1].pos[0].x;
                    fv = (fw > fv) ? fv : fw;
                    fu = node.pos[1].x;
                    ft = node.pos[0].x;
                    fu = (ft > fu) ? fu : ft;
                    fv = (fu > fv) ? fv : fu;
                    box.min.x = fv;

                    float dy = self->layers[1].heightOffset;
                    float dz = self->layers[0].heightOffset;
                    dy = (dz < dy) ? dy : dz;
                    box.min.y = box.max.y - dy;

                    fv = self->nodes.begin[outputIndex - 1].pos[1].z;
                    fw = self->nodes.begin[outputIndex - 1].pos[0].z;
                    fv = (fw > fv) ? fv : fw;
                    fu = node.pos[1].z;
                    ft = node.pos[0].z;
                    fu = (ft > fu) ? fu : ft;
                    fv = (fu > fv) ? fv : fu;
                    box.min.z = fv;

                    GroundRegionList_AppendBounds(CQuadTree_GetInstance(), GRL_BOX_ARG(box), 2);
                }

                // Append the completed section; growth builds the new suffix,
                // copies the old prefix backward, then swaps the guarded buffers.
                unsigned int count = self->nodes.size;
                if (count < self->nodes.cap) {
                    PathNode* dst = self->nodes.begin + count;
                    new (dst) PathNode(node);
                    self->nodes.size = self->nodes.size + 1;
                } else {
                    if (0x7FFFFFFu - self->nodes.cap < 1) throw std::length_error();
                    unsigned int needed = self->nodes.size + 1 - self->nodes.cap;
                    NodeBuf5 buf(&self->nodes.cap);
                    unsigned int capacity = self->nodes.cap;
                    if (needed > 0x7FFFFFFu - capacity) throw std::length_error();
                    unsigned int newCap;
                    if (capacity < 0x2AAAAAAu) {
                        unsigned int grow = (3 * (capacity + 1)) / 5;
                        newCap = capacity + NodeMax(grow, needed);
                    } else if (capacity < 0x5555554u) {
                        unsigned int grow = (capacity + 1) / 2;
                        newCap = capacity + NodeMax(grow, needed);
                    } else {
                        newCap = 0x7FFFFFFu;
                    }
                    if (newCap > 0x7FFFFFFu) throw std::length_error();
                    buf.data = (PathNode*)appMallocDummy__FUl(newCap << 5);
                    unsigned int oldSize = self->nodes.size;
                    buf.cap = newCap;
                    buf.offset = oldSize;
                    PathNode* dst = buf.data + oldSize + buf.size;
                    for (unsigned int n = 1; n != 0;) {
                        new (dst) PathNode(node);
                        buf.size = buf.size + 1;
                        --n;
                        ++dst;
                    }
                    PathNode* base = self->nodes.begin;
                    PathNode* src = base + self->nodes.size;
                    PathNode* d2 = buf.data + buf.offset;
                    while (src > base) {
                        --d2;
                        --src;
                        new (d2) PathNode(*src);
                        buf.offset = buf.offset - 1;
                        buf.size = buf.size + 1;
                    }
                    PathNode* oldBegin = self->nodes.begin;
                    PathNode* newBegin = buf.data;
                    unsigned int oldCap = self->nodes.cap;
                    unsigned int newSize = buf.size;
                    self->nodes.size = 0;
                    buf.data = oldBegin;
                    self->nodes.cap = buf.cap;
                    buf.cap = oldCap;
                    self->nodes.begin = newBegin;
                    self->nodes.size = newSize;
                    buf.size = 0;
                }
                outputIndex = outputIndex + 1;
            }
            segmentFraction = segmentFraction + kFloat_1_River / sampleIntervals;
        }
    }

    // Phase 5: create two 24-byte vertices per section for each enabled layer:
    // XYZ at +0, RGBA at +0xC, UV at +0x10. U spans layer.uExtent to 0;
    // V is filled at draw time. End-section alpha is zero to soften strip ends.
    unsigned int total = outputIndex * kRiverSectionVertexBytes;
    RiverMeshState* emitter = self;
    char* layer = (char*)self;
    void** ep = self->vertexBuffers;
    for (unsigned int i = 0; i < 2; ++i, emitter = (RiverMeshState*)((char*)emitter + 4), layer += 0x1c, ++ep) {
        if (emitter->vertexBuffers[0] != 0) {
            RC801E3830* rc = (RC801E3830*)emitter->vertexBuffers[0];
            rc->hi--;
            if (rc->hi == 0) CubeResourceStorage_dtor(rc, 1);
            emitter->vertexBuffers[0] = 0;
        }
        if (AsCRiver(layer)->layerEnabled) {
            void* vertices;
            VertexBuffer_Create((int)total, 0, kRiverVertexFormat, 0, ep, 0);
            Resource_GetOffsetPtr(emitter->vertexBuffers[0], 0, 0, &vertices, 0);
            unsigned int nodeOffset = 0;
            unsigned int firstVertex = 0;
            unsigned int secondVertex = 1;
            for (unsigned int j = 0; j < outputIndex; ++j, nodeOffset += 0x20, firstVertex += 2, secondVertex += 2) {
                unsigned int vertexOffset = firstVertex * 0x18;
                Vector_CopyComponents(RiverVertexBytes(vertices, vertexOffset), (char*)self->nodes.begin + nodeOffset);
                Vector_CopyComponents((char*)vertices + secondVertex * 0x18, (char*)self->nodes.begin + nodeOffset + 0xc);
                *(float*)(RiverVertexBytes(vertices, vertexOffset) + 0x10) = AsCRiver(layer)->layerUExtent;
                *(float*)(RiverVertexBytes(vertices, vertexOffset) + 0x28) = kFloat_0_River;
                unsigned char on = 0xff;
                if (j == 0 || j == outputIndex - 1) {
                    RiverVertexBytes(vertices, vertexOffset)[0x27] = 0;
                    RiverVertexBytes(vertices, vertexOffset)[0x0f] = 0;
                } else {
                    RiverVertexBytes(vertices, vertexOffset)[0x27] = on;
                    RiverVertexBytes(vertices, vertexOffset)[0x0f] = on;
                }
                RiverVertexBytes(vertices, vertexOffset)[0x0e] = on;
                RiverVertexBytes(vertices, vertexOffset)[0x0d] = on;
                RiverVertexBytes(vertices, vertexOffset)[0x0c] = on;
                RiverVertexBytes(vertices, vertexOffset)[0x26] = on;
                RiverVertexBytes(vertices, vertexOffset)[0x25] = on;
                RiverVertexBytes(vertices, vertexOffset)[0x24] = on;
            }
            GraphicsBuffer_Flush(emitter->vertexBuffers[0]);
        }
    }
}

// Parse optional layer IDs and load course-specific, pink or default wind river textures.
// name is borrowed and must contain the expected underscore separators when the
// course-specific pattern matches; this routine does not validate strchr results.
void CRiver_LoadTextureLayers(void* self, char* name) {
    int ids[2];
    ids[0] = -1;
    ids[1] = -1;
    if (strstr(name, riverTextureStrings)) {
        char* pos = strchr(name, '_');
        if ((sscanf(pos + 1, (riverTextureStrings + kRiverName_layerIdFormat), &ids[0])))
            AsCRiver(self)->layerEnabled = 1;
        else
            AsCRiver(self)->layerEnabled = 0;
        pos = strrchr(name, '_');
        if (sscanf(pos + 1, (riverTextureStrings + kRiverName_layerIdFormat), &ids[1]) && ids[1] != ids[0])
            AsCRiver(self)->secondLayerEnabled = 1;
        else
            AsCRiver(self)->secondLayerEnabled = 0;
        float one = kFloat_1_River;
        char* item = (char*)self;
        for (int i = 0; i < 2; ++i, item += 0x1c) {
            if (AsCRiver(item)->layerEnabled) {
                AsCRiver(item)->layerVOffset = Random_RangeFloat(-1, kFloat_0_River, kFloat_1_River);
                AsCRiver(item)->layerVScrollSpeed = one;
                AsCRiver(item)->layerVScale = one;
                AsCRiver(item)->layerUExtent = one;
                AsCRiver(item)->layerHeightOffset = ids[i] ? kFloat_3_2_river : kFloat_2_24_River;
                void* mgr = AsCSharedDoc(g_sharedDoc)->selectedCourseData;
                char* arg = mgr ? ((IFF_STRUCT::sCourse*)mgr)->mapName : 0;
                const char* suffix = (riverTextureStrings + kRiverName_downstream);
                suffix = (i == 0) ? (riverTextureStrings + kRiverName_upstream) : suffix;
                char* str = String_FormatTemporary((riverTextureStrings + kRiverName_courseTextureFormat), arg, suffix);
                AsCRiver(item)->layerBitmap = ResourceManager_LoadBitmap(g_resourceManager, str, 0, 0, 0);
            }
        }
    } else {
        float ten = kFloat_10_56_River;
        float three_two = kFloat_3_2_river;
        char* item = (char*)self;
        for (int i = 0; i < 2; ++i, item += 0x1c) {
            AsCRiver(item)->layerVOffset = Random_RangeFloat(-1, kFloat_0_River, kFloat_1_River);
            float one = kFloat_1_River;
            AsCRiver(item)->layerVScrollSpeed = Random_RangeFloat(-1, one, kFloat_3_River);
            AsCRiver(item)->layerVScale = one;
            AsCRiver(item)->layerUExtent = one;

            if ((unsigned char)(SharedDoc_GetCourseId()) == 11) {
                AsCRiver(item)->layerHeightOffset = ten;
                const char* suffix = (riverTextureStrings + kRiverName_downstream);
                suffix = (i == 0) ? (riverTextureStrings + kRiverName_upstream) : suffix;
                char* str = String_FormatTemporary((riverTextureStrings + kRiverName_pinkTextureFormat), suffix);
                AsCRiver(item)->layerBitmap = ResourceManager_LoadBitmap(g_resourceManager, str, 0, 0, 0);
            } else {
                AsCRiver(item)->layerHeightOffset = three_two;
                const char* suffix = (riverTextureStrings + kRiverName_downstream);
                suffix = (i == 0) ? (riverTextureStrings + kRiverName_upstream) : suffix;
                char* str = String_FormatTemporary((riverTextureStrings + kRiverName_windTextureFormat), suffix);
                AsCRiver(item)->layerBitmap = ResourceManager_LoadBitmap(g_resourceManager, str, 0, 0, 0);
            }
            AsCRiver(item)->layerEnabled = 1;
        }
    }
}

// Scroll the V coordinate of each enabled texture layer by elapsed time.
void CRiver_ScrollTextureLayers(void* self, float deltaSeconds) {
    unsigned char layerEnabled = AsCRiver(self)->layerEnabled;
    if (layerEnabled != 0) {
        float vOffset;
        float scrollSpeed;
        scrollSpeed = AsCRiver(self)->layerVScrollSpeed;
        vOffset = AsCRiver(self)->layerVOffset;
        vOffset = vOffset - deltaSeconds * scrollSpeed;
        AsCRiver(self)->layerVOffset = vOffset;
    }
    layerEnabled = AsCRiver(self)->secondLayerEnabled;
    if (layerEnabled == 0) return;
    float vOffset;
    float scrollSpeed;
    scrollSpeed = AsCRiver(self)->secondLayerVScrollSpeed;
    vOffset = AsCRiver(self)->secondLayerVOffset;
    vOffset = vOffset - deltaSeconds * scrollSpeed;
    AsCRiver(self)->secondLayerVOffset = vOffset;
}

struct RiverUpdater {
    unsigned int value;
    virtual void reserved();
    virtual void update();
};
struct RiverTextures {
    virtual void v00(); virtual void v01(); virtual void v02(); virtual void v03();
    virtual void v04(); virtual void v05(); virtual void v06(); virtual void v07();
    virtual void v08(); virtual void v09(); virtual void v10(); virtual void v11();
    virtual void v12(); virtual void v13(); virtual void v14(); virtual void v15();
    virtual void v16(); virtual void v17(); virtual void v18(); virtual void v19();
    virtual void v20(); virtual void v21(); virtual void v22(); virtual void v23();
    virtual void v24(); virtual void v25();
    virtual int texture(void*);
    virtual int sampler(void*);
};
#pragma push
#pragma opt_propagation off
#pragma opt_common_subs off
// Apply river render state, update scrolling V coordinates and draw each enabled texture layer.
void CRiver_DrawTextureLayers(void* self) {
    // Phase 1: restore the engine's default state/stage/sampler key-value lists.
    RenderConstants* constants = &riverRenderStateDefaults;
    RenderStates* states = &constants->states;
    RenderStages* stages = &constants->stages;
    RenderSamplers* samplers = &constants->samplers;
    RenderStates copy = *states;
    struct { RenderStages args; RenderSamplers samplers; } config;
    config.args = *stages;
    config.samplers = *samplers;
    Engine_SetRenderStateList(&copy, kRiverStatePairCount);
    Engine_SetTextureStageStateList(0, &config.args, kRiverStagePairCount);
    Engine_SetTextureStageParameterList(0, &config.samplers, kRiverSamplerPairCount);

    void* dev = Camera_GetRenderState(g_view);
    // Phase 2: terrain-space vertices use camera view/projection directly. Slots
    // 2/11 receive identity; their exact engine roles are not established here.
    SetTransform(kRiverViewTransform, ((RiverCameraState*)dev)->view);
    SetTransform(kRiverProjectionTransform, ((RiverCameraState*)dev)->projection);
    Engine_SetType7Resource(0);
    ((RiverUpdater*)&LavaOwned(self)->materialFlags)->update();
    SetTransform(kRiverIdentityTransform2, kIdentityMatrix4x4);
    SetTransform(kRiverIdentityTransform11, kIdentityMatrix4x4);
    Engine_SetTextureStageState(0, 0, 0);
    Engine_SetTextureStageState(0, 3, 0);
    Engine_SetTextureStageParameter(0, 0, 1);
    Engine_SetTextureStageParameter(0, 1, 1);
    Engine_SetCachedRenderState(13, 4);
    Engine_SetCachedRenderState(14, 1);
    Engine_SetCachedRenderState(5, 0);
    Engine_SetCachedRenderState(3, 3);

    // Phase 3: for each layer, add its scroll offset to the section V selected
    // by layer index and write that same V to both bank vertices. Bind texture
    // and sampler, flush CPU writes, and draw the resulting triangle strip.
    // nodeCount must describe at least two sections for the unsigned count below.
    char* item = (char*)self;
    char* emitter = (char*)self;
    for (int i = 0; i < 2; ++i, emitter += 4, item += 0x1c) {
        if (AsCRiver(item)->layerEnabled) {
            void* buffer;
            Resource_GetOffsetPtr(AsCRiver(emitter)->vertexBuffer, 0, 0, &buffer, 0);
            unsigned int sourceOffset = 0;
            unsigned int destOffset = 0;
            for (unsigned int n = 0; n < AsCRiver(self)->nodeCount; ++n, sourceOffset += 0x20, destOffset += 0x30) {
                char* base = i * 4 + (*(char**)&AsCRiver(self)->nodes);
                float value = AsCRiver(item)->layerVOffset + *(float*)(sourceOffset + base + 0x18);
                *(float*)((char*)buffer + destOffset + 0x2c) = value;
                *(float*)((char*)buffer + destOffset + 0x14) = value;
            }
            GraphicsBuffer_Flush(AsCRiver(emitter)->vertexBuffer);
            void* p = AsCRiver(item)->layerBitmap;
            int a = (((RiverTextures*)AsWView(g_view)->renderTarget))->texture(p);
            Engine_SetTextureStageResource(0, a);
            int b = (((RiverTextures*)AsWView(g_view)->renderTarget))->sampler(p);
            Engine_SetTexturePalette(0, b);
            Engine_SetVertexStream(0, AsCRiver(emitter)->vertexBuffer, 0, kRiverVertexBytes);
            DrawPrimitiveFromBoundBuffers(kRiverTriangleStrip, 0, AsCRiver(self)->nodeCount * 2 - 2);
        }
    }
}
#pragma pop

#pragma push
#pragma scheduling off
#pragma opt_propagation off
#pragma opt_common_subs off
// Initialize lava texture layers, strip/auxiliary storage and the embedded material.
void ntRiver_Lava_Ctor(void* self) {
    void* baseMaterialVtable = (void*)__vt__10CGeneTex3D;
    void* lavaVtable = (void*)__vt__12ntRiver_Lava;
    int zeroWord = 0;
    float zeroFloat = kFloat_0_River;
    *(void**)self = lavaVtable;
    (*(int*)&AsNtRiverLava(self)->stripPoints) = zeroWord;
    (*(int*)&AsNtRiverLava(self)->stripCount) = zeroWord;
    (*(int*)&AsNtRiverLava(self)->stripCapacity) = zeroWord;
    (*(int*)&LavaOwned(self)->auxiliaryData) = zeroWord;
    LavaOwned(self)->auxiliarySize = zeroWord;
    LavaOwned(self)->auxiliaryCapacity = zeroWord;
    LavaOwned(self)->materialFlags = (unsigned char)zeroWord;
    LavaOwned(self)->materialVtable = baseMaterialVtable;
    (*(int*)&LavaOwned(self)->materialData) = zeroWord;
    AsNtRiverLava(self)->layers[0].vOffset = zeroFloat;
    AsNtRiverLava(self)->layers[0].vScrollSpeed = zeroFloat;
    (*(int*)&AsNtRiverLava(self)->layers[0].bitmap) = zeroWord;
    AsNtRiverLava(self)->layers[0].vScale = zeroFloat;
    AsNtRiverLava(self)->layers[0].uExtent = zeroFloat;
    AsNtRiverLava(self)->layers[0].heightOffset = zeroFloat;
    AsNtRiverLava(self)->layers[0].enabled = (unsigned char)zeroWord;
    (*(int*)&AsNtRiverLava(self)->vertexBuffers[0]) = zeroWord;
    AsNtRiverLava(self)->layers[1].vOffset = zeroFloat;
    AsNtRiverLava(self)->layers[1].vScrollSpeed = zeroFloat;
    (*(int*)&AsNtRiverLava(self)->layers[1].bitmap) = zeroWord;
    AsNtRiverLava(self)->layers[1].vScale = zeroFloat;
    AsNtRiverLava(self)->layers[1].uExtent = zeroFloat;
    AsNtRiverLava(self)->layers[1].heightOffset = zeroFloat;
    AsNtRiverLava(self)->layers[1].enabled = (unsigned char)zeroWord;
    (*(int*)&AsNtRiverLava(self)->vertexBuffers[1]) = zeroWord;
}
#pragma pop

#pragma push
#pragma use_lmw_stmw on
#pragma opt_propagation off
#pragma opt_common_subs off
struct RC801E4AB0 { CW_BITS2( unsigned hi : 16, unsigned lo : 16) };
struct Vec801E4AB0 { void* data; int size; };
struct Obj801E4AB0 { char pad[0x40]; Vec801E4AB0 a; char pad2[4]; Vec801E4AB0 b; };
// Release both textures and vertex buffers, material data and owned arrays, then optionally delete.
void* ntRiver_Lava_dtor(void* self, int deleting) {
    if (self != 0) {
        void* layerCursor = self;
        *(void**)self = (void*)__vt__12ntRiver_Lava;
        void* bufferCursor = self;
        for (int iter = 0; iter < 2; iter++) {
            void* resourceManager = g_resourceManager;
            if (resourceManager != 0) {
                void* ptr = AsNtRiverLava(layerCursor)->layers[0].bitmap;
                if (0 != ptr) {
                    ResourceManager_ReleaseId(resourceManager, ptr);
                    AsNtRiverLava(layerCursor)->layers[0].bitmap = 0;
                }
            }
            RC801E4AB0* bufferRefCount = (*(RC801E4AB0**)&AsNtRiverLava(bufferCursor)->vertexBuffers[0]);
            if (bufferRefCount != 0) {
                bufferRefCount->hi--;
                if (bufferRefCount->hi == 0) {
                    CubeResourceStorage_dtor(bufferRefCount, 1);
                }
                (*(void**)&AsNtRiverLava(bufferCursor)->vertexBuffers[0]) = 0;
            }
            layerCursor = ((char*)&AsNtRiverLava(layerCursor)->layers[0].heightOffset);
            bufferCursor = (char*)bufferCursor + 4;
        }
        if (((char*)&LavaOwned(self)->materialFlags) != 0) {
            void* materialData = LavaOwned(self)->materialData;
            LavaOwned(self)->materialVtable = (void*)__vt__10CGeneTex3D;
            if (0 != materialData) {
                __dla__FPv(materialData);
                LavaOwned(self)->materialData = 0;
            }
        }
        if (&((Obj801E4AB0*)self)->b != 0) {
            if (&((Obj801E4AB0*)self)->b != 0) {
                if (&((Obj801E4AB0*)self)->b != 0) {
                    if (&((Obj801E4AB0*)self)->b != 0) {
                        void* auxiliaryData = ((Obj801E4AB0*)self)->b.data;
                        if (auxiliaryData != 0) {
                            int n = ((Obj801E4AB0*)self)->b.size;
                            int m = n;
                            ((Obj801E4AB0*)self)->b.size = n - m;
                            appFreeUni__FPv(auxiliaryData);
                        }
                    }
                }
            }
        }
        if (&((Obj801E4AB0*)self)->a != 0) {
            if (&((Obj801E4AB0*)self)->a != 0) {
                if (&((Obj801E4AB0*)self)->a != 0) {
                    if (&((Obj801E4AB0*)self)->a != 0) {
                        void* stripData = ((Obj801E4AB0*)self)->a.data;
                        if (stripData != 0) {
                            int n = ((Obj801E4AB0*)self)->a.size;
                            int m = n;
                            ((Obj801E4AB0*)self)->a.size = n - m;
                            appFreeUni__FPv(stripData);
                        }
                    }
                }
            }
        }
        if (deleting > 0) {
            __dl__FPv(self);
        }
    }
    return self;
}
#pragma pop

} // extern "C"

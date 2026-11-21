#include <decomp/static_init.h>
#include <decomp/offset_of.h>
typedef unsigned long new_size_t;
#include "classes/NtRiverLavaLayout.h"
#include "classes/WView.h"
#include "game/types.h"
// Translation unit 0x801E4C08-0x801E6024 (9 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

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
    kRiverName_separator = (int)DECOMP_OFFSETOF(RiverTextureNames, separator),
    kRiverName_layerIdFormat = (int)DECOMP_OFFSETOF(RiverTextureNames, layerIdFormat),
    kRiverName_courseTextureFormat = (int)DECOMP_OFFSETOF(RiverTextureNames, courseTextureFormat),
    kRiverName_upstream = (int)DECOMP_OFFSETOF(RiverTextureNames, upstream),
    kRiverName_downstream = (int)DECOMP_OFFSETOF(RiverTextureNames, downstream),
    kRiverName_pinkTextureFormat = (int)DECOMP_OFFSETOF(RiverTextureNames, pinkTextureFormat),
    kRiverName_windTextureFormat = (int)DECOMP_OFFSETOF(RiverTextureNames, windTextureFormat),
    kRiverName_vectorLengthError = (int)DECOMP_OFFSETOF(RiverTextureNames, vectorLengthError),
    kRiverName_lengthErrorType = (int)DECOMP_OFFSETOF(RiverTextureNames, lengthErrorType),
    kRiverName_lavaTextureFormat = (int)DECOMP_OFFSETOF(RiverTextureNames, lavaTextureFormat),
};

// Camera_GetRenderState supplies two consecutive 4x4 matrices at these offsets.
struct RiverCameraState {
    char reserved[0x20];
    float view[16];
    float projection[16];
};

enum { kRiverLayerCount = 2, kMaxPathPoints = 256, kInitialStripCapacity = 50, kRiverVertexFormat = 0x11f001, kTriangleStrip = 0x98 };

// Construct in caller-owned storage without allocating.
inline void* operator new(new_size_t, void* p) { return p; }
extern "C" {

extern void Vec3_Zero(void*);
extern void __mi__FRC7WVectorRC7WVector(void*, void*, void*);
extern void __as__7WVectorFRC7WVector(void*, void*);
extern void Vector_CrossProduct(void*, void*, void*);
extern void* Normalize__7WVectorFv(void*);
extern void __ml__FfRC7WVector(void*, void*, float);
extern void __ml__FRC7WVectorf(void*, void*, float);
extern void __pl__FRC7WVectorRC7WVector(void*, void*, void*);
extern float WVectorLen__FRC7WVector(void*);
extern void __amu__7WVectorFf(void*, float);
extern void* appMallocDummy__FUl(unsigned int);
extern void* CQuadTree_GetInstance();
extern float CQuadTree_GetLowestGround(void*, void*);
extern void GroundRegionList_AppendBounds(void*, void*, int);
extern void appFreeUni__FPv(void*);
extern int Resource_GetOffsetPtr(void*, int, int, void*, int);
extern int GraphicsBuffer_Flush(void*);
extern void VertexBuffer_Create(int, int, int, int, int, int);
extern void Vector_CopyComponents(void*, void*);
extern void CubeResourceStorage_dtor(void*, short);
extern void* __nwa__FUl(unsigned int);
extern void* __nw__FUl(unsigned int);
extern void __dla__FPv(void*);
extern unsigned int strlen(const char*);
extern char* strcpy(char*, const char*);
extern char riverTextureStrings[], __vt__Q23std11logic_error[], s_sharedCharDeleterVtable[], __vt__Q23std12length_error[];
extern void __dt__Q23std12length_errorFv();
extern void __throw(void*, void*, void*);
extern void __dl__FPv(void*);
extern float kFloat_6_4_River, kFloat_160_River, kFloat_neg1_River, kFloat_3_2_river;
extern float kFloat_0_River, kFloat_1_River;

typedef Vec3f Vec3;
void RiverLava_InitializeTextureLayers(NtRiverLavaLayout*, void*);
struct Pair3 { Vec3 a, b; };
struct FloatPair { float a, b; };
struct Word3 { unsigned int a, b, c; };
struct Word2 { unsigned int a, b; };
struct Vertex { float v[12]; };
struct Slice { void* data; unsigned int count; };
struct StripPoint {
    // World-space banks at -side/+side and two accumulated texture-V coordinates.
    // uv[j] is advanced along bank j using layer j's vScale; the draw loop
    // gives BOTH vertices of layer i the value uv[i], plus its scroll offset.
    Vec3 p[2]; float uv[2];
    // Scalar fields are filled by the builder before append.
    StripPoint() {}
    // Copy bank positions and V words without floating-point conversion.
    StripPoint(const StripPoint& other) {
        *(Word3*)&p[0] = *(const Word3*)&other.p[0];
        *(Word3*)&p[1] = *(const Word3*)&other.p[1];
        *(Word2*)uv = *(const Word2*)other.uv;
    }
};
struct LavaMaterialView { char reserved[0x58]; unsigned char material; };
struct RenderVertex { Vec3 p; unsigned char color[4]; float u, v; };
// Preserve the original byte-cursor induction variables; typed array indexing changes scheduling.
enum {
    kLayerStride = sizeof(NtRiverLavaLayer),
    kMaterialOffset = 0x58,
    kMaterialVtableOffset = (int)DECOMP_OFFSETOF(NtRiverLavaLayout, materialVtable),
    kVertexBuffersOffset = (int)DECOMP_OFFSETOF(NtRiverLavaLayout, vertexBuffers),
    kLayerVOffset = (int)DECOMP_OFFSETOF(NtRiverLavaLayout, layers[0].vOffset),
    kLayerVScaleOffset = (int)DECOMP_OFFSETOF(NtRiverLavaLayout, layers[0].vScale),
    kLayerBitmapOffset = (int)DECOMP_OFFSETOF(NtRiverLavaLayout, layers[0].bitmap),
    kLayerEnabledOffset = (int)DECOMP_OFFSETOF(NtRiverLavaLayout, layers[0].enabled),
    kStripVOffset = (int)DECOMP_OFFSETOF(StripPoint, uv),
    kFirstVertexVOffset = (int)DECOMP_OFFSETOF(RenderVertex, v),
    kSecondVertexVOffset = sizeof(RenderVertex) + kFirstVertexVOffset
};
struct LengthError { void* table; char* text; void* ref; };
// Allocate the shared-message exception and transfer it to the exception runtime.
inline void strip_length_error() {
    LengthError error;
    error.table = __vt__Q23std11logic_error;
    const char* message = (riverTextureStrings + kRiverName_vectorLengthError);
    unsigned int length = strlen(message) + 1;
    error.text = (char*)__nwa__FUl(length * 2 - length);
    unsigned int* ref;
    try {
        ref = (unsigned int*)__nw__FUl(16);
    } catch (...) {
        __dla__FPv(error.text);
        throw;
    }
    if (ref) {
        ref[1] = 1; ref[2] = 1; ref[0] = (unsigned int)s_sharedCharDeleterVtable; ref[3] = (unsigned int)error.text;
    }
    error.ref = ref;
    strcpy(error.text, message);
    error.table = __vt__Q23std12length_error;
    __throw((void*)(riverTextureStrings + kRiverName_lengthErrorType), &error, (void*)__dt__Q23std12length_errorFv);
}
struct StripBuffer {
    StripPoint* data; unsigned int size, capacity;
    // Start an empty temporary owner for strip storage.
    StripBuffer() : data(0), size(0), capacity(0) {}
    // Release whichever allocation remains in the temporary after a swap or unwind.
    ~StripBuffer() { if (data) { size = size - *(volatile unsigned int*)&size; appFreeUni__FPv(data); } }
};
struct StripGrowth {
    StripPoint* data; unsigned int size, capacity; unsigned int* owner; unsigned int offset;
    // Track a replacement allocation until its constructed range is committed.
    StripGrowth(unsigned int* p) : data(0), size(0), capacity(0), owner(p), offset(0) {}
    // Free the uncommitted allocation (or old storage after the swap).
    ~StripGrowth() { size = 0; if (data) { size = 0; appFreeUni__FPv(data); } }
};

// Legacy parameter shape is retained solely for CodeWarrior alias analysis; access actual data through AsNtRiverLava.
struct LavaStripBuilderAliasView {
    char pad00[0x1c]; float limit; char pad20[0x1c]; float max;
    char pad3c[4]; unsigned int used; unsigned int capacity; unsigned int limit2;
    void* storage; unsigned int end; unsigned int count; char pad54[0x10];
    void* handles[2]; char pad6c[0x14]; unsigned int active;
};

// Find a width for each centerline segment, build bank bounds and allocate the lava vertex strips.
// Byte traversal below intentionally retains the original induction variables and alias view.
// slices borrows world-space Vec3 centers; require 2 <= count <= 257 and
// nonzero horizontal segment directions. No checks are added here. count-1
// samples are generated, at most 256 (the final center supplies the final direction only).
// Ground heights, 6.4 width steps, 3.2 bank margin and 160 search limit use
// the SAME world units as the centers; no meters conversion is established.
// The builder appends to existing stripPoints: caller must supply the intended
// initial count (normally zero). It replaces owned GPU buffers for both layers.
void ntRiver_Lava_RebuildStripGeometry(LavaStripBuilderAliasView* self, Slice* slices, void* forwardedArg) {
    RiverLava_InitializeTextureLayers((NtRiverLavaLayout*)self, forwardedArg);
    Vec3 bounds[2];
    Vec3_Zero(&bounds[0]); Vec3_Zero(&bounds[1]);
    // Phase 1: probe one bank along normalize(horizontal direction cross world-up).
    // Stop above terrain+max(layer heights), or beyond the search limit.
    float halfWidths[kMaxPathPoints] = {};
    for (unsigned int i = 0; i < slices->count - 1; ++i) {
        Vec3 input[2];
        __mi__FRC7WVectorRC7WVector(&input[0], (char*)slices->data + (i + 1) * 12, (char*)slices->data + i * 12);
        Vec3 transform; __as__7WVectorFRC7WVector(&transform, &input[0]);
        transform.y = kFloat_0_River;
        Vec3 origin; origin.x = kFloat_0_River; origin.y = kFloat_1_River; origin.z = kFloat_0_River;
        Vec3 temp; Vector_CrossProduct(&temp, &transform, &origin);
        __as__7WVectorFRC7WVector(&transform, Normalize__7WVectorFv(&temp));
        float heightThreshold = AsNtRiverLava(self)->layers[0].heightOffset < AsNtRiverLava(self)->layers[1].heightOffset ? AsNtRiverLava(self)->layers[1].heightOffset : AsNtRiverLava(self)->layers[0].heightOffset;
        Vec3 key = *((Vec3*)((char*)slices->data + i * 12));
        float groundHeight = CQuadTree_GetLowestGround(CQuadTree_GetInstance(), &key);
        heightThreshold = groundHeight + heightThreshold;
        for (;;) {
            halfWidths[i] += kFloat_6_4_River;
            Vec3 scaled; __ml__FfRC7WVector(&scaled, &transform, halfWidths[i]);
            Vec3 v; __pl__FRC7WVectorRC7WVector(&v, (char*)slices->data + i * 12, &scaled);
            Vec3 copy = v;
            float d = CQuadTree_GetLowestGround(CQuadTree_GetInstance(), &copy);
            if (d > heightThreshold) { halfWidths[i] += kFloat_3_2_river; break; }
            else if (halfWidths[i] > kFloat_160_River) break;
        }
    }
    // Phase 2: reserve CPU strip storage; swap transfers ownership only after copying.
    StripPoint*& data = *(StripPoint**)&AsNtRiverLava(self)->stripPoints;
    unsigned int& size = AsNtRiverLava(self)->stripCount;
    unsigned int& capacity = AsNtRiverLava(self)->stripCapacity;
    if (capacity < kInitialStripCapacity) {
        StripBuffer temp;
        temp.data = (StripPoint*)appMallocDummy__FUl(kInitialStripCapacity * sizeof(StripPoint));
        temp.capacity = kInitialStripCapacity;
        StripPoint* end = data + size;
        StripPoint* dst = temp.data + temp.size;
        for (StripPoint* src = data; src < end; ++src, ++dst) {
            new(dst) StripPoint(*src);
            ++temp.size;
        }
        size = 0;
        StripPoint* old = data; data = temp.data; temp.data = old;
        unsigned int cap = capacity; capacity = temp.capacity; temp.capacity = cap;
        unsigned int used = size; size = temp.size; temp.size = used;
    }
    // Phase 3: construct banks around each center, accumulate V, and register
    // bounds between successive sections before appending. Bounds[1].y uses
    // the LOWER adjacent surface height; bounds[0].y subtracts the larger layer height offset.
    unsigned int count = 0;
    for (unsigned int i = 0; i < slices->count - 1; ++i) {
        Vec3 delta; __mi__FRC7WVectorRC7WVector(&delta, (Vec3*)slices->data + i + 1, (Vec3*)slices->data + i);
        Vec3 side; __as__7WVectorFRC7WVector(&side, &delta);
        side.y = kFloat_0_River;
        Vec3 up; up.x = kFloat_0_River; up.y = kFloat_1_River; up.z = kFloat_0_River;
        Vec3 cross; Vector_CrossProduct(&cross, &side, &up);
        __as__7WVectorFRC7WVector(&side, Normalize__7WVectorFv(&cross));
        __amu__7WVectorFf(&side, halfWidths[i]);
        Vec3 center; __as__7WVectorFRC7WVector(&center, (Vec3*)slices->data + i);
        Vec3 copy = center;
        float height = CQuadTree_GetLowestGround(CQuadTree_GetInstance(), &copy);
        StripPoint point;
        for (unsigned int j = 0; j < kRiverLayerCount; ++j) {
            Vec3 offset; __ml__FRC7WVectorf(&offset, &side, (j & 1) ? kFloat_1_River : kFloat_neg1_River);
            Vec3 sum; __pl__FRC7WVectorRC7WVector(&sum, &center, &offset);
            __as__7WVectorFRC7WVector(&point.p[j], &sum);
            point.p[j].y = height + AsNtRiverLava(self)->layers[0].heightOffset;
            if (count == 0) point.uv[j] = kFloat_0_River;
            else {
                StripPoint* previous = data + count - 1;
                Vec3 difference; __mi__FRC7WVectorRC7WVector(&difference, &point.p[j], &previous->p[j]);
                float length = WVectorLen__FRC7WVector(&difference);
                point.uv[j] = previous->uv[j] + (*(float*)((char*)self + j * kLayerStride + kLayerVScaleOffset) * length) / halfWidths[i];
            }
        }
        if (count) {
            StripPoint* previous = data + count - 1;
            float a = previous->p[0].x < previous->p[1].x ? previous->p[1].x : previous->p[0].x;
            float b = point.p[0].x < point.p[1].x ? point.p[1].x : point.p[0].x;
            bounds[1].x = b < a ? a : b;
            bounds[1].y = point.p[0].y > previous->p[0].y ? previous->p[0].y : point.p[0].y;
            a = previous->p[0].z < previous->p[1].z ? previous->p[1].z : previous->p[0].z;
            b = point.p[0].z < point.p[1].z ? point.p[1].z : point.p[0].z;
            bounds[1].z = b < a ? a : b;
            a = previous->p[0].x > previous->p[1].x ? previous->p[1].x : previous->p[0].x;
            b = point.p[0].x > point.p[1].x ? point.p[1].x : point.p[0].x;
            bounds[0].x = b > a ? a : b;
            float heightLimit = AsNtRiverLava(self)->layers[0].heightOffset < AsNtRiverLava(self)->layers[1].heightOffset ? AsNtRiverLava(self)->layers[1].heightOffset : AsNtRiverLava(self)->layers[0].heightOffset;
            bounds[0].y = bounds[1].y - heightLimit;
            a = previous->p[0].z > previous->p[1].z ? previous->p[1].z : previous->p[0].z;
            b = point.p[0].z > point.p[1].z ? point.p[1].z : point.p[0].z;
            bounds[0].z = b > a ? a : b;
            Pair3 box = *(Pair3*)bounds;
            GroundRegionList_AppendBounds(CQuadTree_GetInstance(), &box, 2);
        }
        // Append with spare capacity, or build the new tail and copy the old prefix
        // backwards before swapping. The temporary then owns only the old allocation.
        if (size < capacity) {
            StripPoint* dst = data + size;
            new(dst) StripPoint(point);
            ++size;
        } else {
            if (0x7ffffff - capacity < 1) strip_length_error();
            unsigned int needed = size + 1 - capacity;
            StripGrowth temp(&capacity);
            unsigned int oldCap = capacity;
            if (needed > 0x7ffffff - oldCap) strip_length_error();
            unsigned int next;
            if (oldCap < 0x2aaaaaa) {
                unsigned int extra = (oldCap + 1) * 3 / 5;
                next = oldCap + *(extra < needed ? &needed : &extra);
            } else if (oldCap < 0x5555554) {
                unsigned int extra = (oldCap + 1) / 2;
                next = oldCap + *(extra < needed ? &needed : &extra);
            } else next = 0x7ffffff;
            if (next > 0x7ffffff) strip_length_error();
            temp.data = (StripPoint*)appMallocDummy__FUl(next * sizeof(StripPoint));
            temp.capacity = next; temp.offset = size;
            StripPoint* dst = temp.data + size + temp.size;
            int remaining = 1;
            while (remaining) {
                new(dst) StripPoint(point);
                ++temp.size; --remaining; ++dst;
            }
            StripPoint* src = data + size;
            dst = temp.data + temp.offset;
            while (src > data) {
                --src; --dst;
                new(dst) StripPoint(*src);
                --temp.offset; ++temp.size;
            }
            size = 0;
            StripPoint* old = data; data = temp.data; temp.data = old;
            unsigned int cap = capacity; capacity = temp.capacity; temp.capacity = cap;
            size = temp.size; temp.size = 0;
        }
        ++count;
    }
    // Phase 4: release previous GPU handles (reference count in high 16 bits),
    // allocate two 24-byte vertices/section, set white RGB and transparent end caps.
    // V is written on each draw, so the initial upload here only sets position/U/color.
    for (unsigned int j = 0; j < kRiverLayerCount; ++j) {
        void** handle = (void**)((char*)self + kVertexBuffersOffset + j * 4);
        if (*handle) {
            unsigned int* ref = (unsigned int*)*handle;
            *ref = (*ref & 0xffff) | (((*ref >> 16) - 1) << 16);
            if ((*ref >> 16) == 0) CubeResourceStorage_dtor(*handle, 1);
            *handle = 0;
        }
        char* settings = (char*)self + j * kLayerStride;
        if (AsNtRiverLava(settings)->layers[0].enabled) {
            VertexBuffer_Create(count * 0x30, 0, kRiverVertexFormat, 0, (int)handle, 0);
            RenderVertex* buffer;
            Resource_GetOffsetPtr(*handle, 0, 0, &buffer, 0);
            for (unsigned int k = 0; k < count; ++k) {
                Vector_CopyComponents(buffer + k * 2, &data[k].p[0]);
                Vector_CopyComponents(buffer + k * 2 + 1, &data[k].p[1]);
                buffer[k * 2].u = AsNtRiverLava(settings)->layers[0].uExtent;
                buffer[k * 2 + 1].u = kFloat_0_River;
                if (k == 0 || k == count - 1) {
                    buffer[k * 2 + 1].color[3] = 0;
                    buffer[k * 2].color[3] = 0;
                } else {
                    buffer[k * 2 + 1].color[3] = 255;
                    buffer[k * 2].color[3] = 255;
                }
                buffer[k * 2].color[2] = 255;
                buffer[k * 2].color[1] = 255;
                buffer[k * 2].color[0] = 255;
                buffer[k * 2 + 1].color[2] = 255;
                buffer[k * 2 + 1].color[1] = 255;
                buffer[k * 2 + 1].color[0] = 255;
            }
            GraphicsBuffer_Flush(*handle);
        }
    }
}

extern const float kFloat_6_2831855;
extern float s_riverLavaAngleCache;
extern const float kFloat_0_5_River;
extern float kFloat_1_River;
extern float kFloat_0_River;
extern const float kFloat_0_08_River;
extern const float kFloat_0_04_RiverLava;
extern const float kFloat_5_1200004_RiverLayers;
extern float kFloat_3_2_river;
class ResourceManager;
extern ResourceManager* g_resourceManager;
extern WView* g_view;
extern char riverTextureStrings[];
extern const char riverRenderStateDefaults[];
extern char kIdentityMatrix4x4[];

extern float Random_RangeFloat(int, float, float);
extern const char* String_FormatTemporary(const char*, ...);
extern void* ResourceManager_LoadBitmap(ResourceManager*, const char*, int, int, int);
extern int Engine_SetRenderStateList(void*, int);
extern int Engine_SetTextureStageStateList(int, unsigned int*, int);
extern int Engine_SetTextureStageParameterList(int, unsigned int*, int);
extern int Camera_GetRenderState(void*);
extern int SetTransform(int, void*);
extern int Engine_SetType7Resource(int);
extern int Engine_SetTextureStageState(int, int, int);
extern int Engine_SetTextureStageParameter(int, int, int);
extern int Engine_SetCachedRenderState(int, int);
extern int Resource_GetOffsetPtr(void*, int, int, void*, int);
extern int GraphicsBuffer_Flush(void*);
extern int Engine_SetTextureStageResource(int, int);
extern int Engine_SetTexturePalette(int, int);
extern int Engine_SetVertexStream(int, void*, int, int);
extern int DrawPrimitiveFromBoundBuffers(int, int, int);


// Retain this legacy parameter shape to preserve register allocation; its members are not accessed.
struct LavaRendererAliasView {
    char reserved_00[0x20];
    unsigned char active0;
    char reserved_21[0x23];
    void* reserved_44;
    void* reserved_48;
    void* reserved_4C;
    void* reserved_50;
    void* reserved_54;
    void* reserved_58;
    void* reserved_5C;
    unsigned int count;
    void* data0;
    void* data1;
    char reserved_6C[0x14];
    unsigned int active1;
};

// Update scrolling texture coordinates and draw each enabled lava layer with its texture and material.
// Require built strips and a current camera/render target. Engine state IDs below
// are engine indices, not GX enum values; their complete mapping is unresolved.
// This routine installs its preset and leaves that render state active for the caller.
void ntRiver_Lava_Draw(LavaRendererAliasView* self) {
    // Phase 1: copy render/texture presets, bind camera view+projection and the
    // embedded material callback (+0x58 receiver). Transform slots 2/11 use identity.
    unsigned int buf[0x34];
    unsigned int* src = (unsigned int*)riverRenderStateDefaults;
    for (int i = 0; i < 0x1e; ++i) buf[i + 0x16] = src[i];
    for (int i = 0; i < 0x16; ++i) buf[i] = src[i + 0x1e];
    Engine_SetRenderStateList(buf + 0x16, 15);
    Engine_SetTextureStageStateList(0, buf, 6);
    Engine_SetTextureStageParameterList(0, buf + 0x0c, 5);
    int device = Camera_GetRenderState((void*)g_view);
    SetTransform(0, ((RiverCameraState*)device)->view);
    SetTransform(1, ((RiverCameraState*)device)->projection);
    Engine_SetType7Resource(0);
    ((void (*)(void*))(*(void***)((char*)self + kMaterialVtableOffset))[0x0c / 4])((char*)self + kMaterialOffset);
    SetTransform(2, kIdentityMatrix4x4);
    SetTransform(11, kIdentityMatrix4x4);
    Engine_SetTextureStageState(0, 0, 0);
    Engine_SetTextureStageState(0, 3, 0);
    Engine_SetTextureStageParameter(0, 0, 1);
    Engine_SetTextureStageParameter(0, 1, 1);
    Engine_SetCachedRenderState(13, 4);
    Engine_SetCachedRenderState(14, 1);
    Engine_SetCachedRenderState(5, 0);
    Engine_SetCachedRenderState(3, 3);
    // Phase 2: for each enabled layer, update and flush its own GPU buffer.
    // Two 24-byte vertices per section; both receive the same layer V coordinate.
    for (int i = 0; i < kRiverLayerCount; ++i) {
        if (!*((unsigned char*)self + i * kLayerStride + kLayerEnabledOffset)) continue;
        void* buffer = 0;
        Resource_GetOffsetPtr(*((void**)((char*)self + kVertexBuffersOffset + i * 4)), 0, 0, &buffer, 0);
        for (unsigned int j = 0; j < AsNtRiverLava(self)->stripCount; ++j) {
            char* stripBytes = (char*)AsNtRiverLava(self)->stripPoints;
            float textureV = *((float*)((char*)self + i * kLayerStride + kLayerVOffset)) + *((float*)(stripBytes + j * sizeof(StripPoint) + kStripVOffset + i * 4));
            *((float*)((char*)buffer + j * (sizeof(RenderVertex) * 2) + kSecondVertexVOffset)) = textureV;
            *((float*)((char*)buffer + j * (sizeof(RenderVertex) * 2) + kFirstVertexVOffset)) = textureV;
        }
        GraphicsBuffer_Flush(*((void**)((char*)self + kVertexBuffersOffset + i * 4)));
        // Resolve stage resource and texture through distinct render-target slots
        // +0x70/+0x74 for this layer's bitmap, then bind matching vertex storage.
        {
            void* base = (void*)g_view;
            void* target = ((WView*)base)->renderTarget; // original active WView +0xA4
            int bitmapHandle = *((int*)((char*)self + i * kLayerStride + kLayerBitmapOffset));
            void* r = ((void*(*)(void*, int))((*(void***)target)[0x70 / 4]))(target, bitmapHandle);
            Engine_SetTextureStageResource(0, (int)r);
        }
        {
            void* base = (void*)g_view;
            void* target = ((WView*)base)->renderTarget; // original active WView +0xA4
            int bitmapHandle = *((int*)((char*)self + i * kLayerStride + kLayerBitmapOffset));
            void* r = ((void*(*)(void*, int))((*(void***)target)[0x74 / 4]))(target, bitmapHandle);
            Engine_SetTexturePalette(0, (int)r);
        }
        Engine_SetVertexStream(0, *((void**)((char*)self + kVertexBuffersOffset + i * 4)), 0, sizeof(RenderVertex));
        DrawPrimitiveFromBoundBuffers(kTriangleStrip, 0, AsNtRiverLava(self)->stripCount * 2 - 2);
    }
}

// Initialize this unit's cached full-circle angle.
void RiverLava_InitializeAngleCache() {
    s_riverLavaAngleCache = kFloat_6_2831855;
}


// Scroll each enabled lava layer by its own V speed and elapsed time.
// delta uses the caller's update-time unit; vScrollSpeed is texture-V per that unit.
void ntRiver_Lava_ScrollTextureLayers(NtRiverLavaLayout* self, float delta) {
    if (self->layers[0].enabled) {
        self->layers[0].vOffset -= delta * self->layers[0].vScrollSpeed;
    }
    if (self->layers[1].enabled) {
        self->layers[1].vOffset -= delta * self->layers[1].vScrollSpeed;
    }
}


// Enable two lava layers, randomize their V offsets and load upstream/downstream textures.
void RiverLava_InitializeTextureLayers(NtRiverLavaLayout* self, void* unused) {
    float defaultVScale = kFloat_0_5_River;
    float defaultUExtent = kFloat_1_River;
    NtRiverLavaLayout* cursor = self;
    self->layers[0].enabled = 1;
    self->layers[1].enabled = 1;
    for (int i = 0; i < kRiverLayerCount; ++i) {
        cursor->layers[0].vOffset = Random_RangeFloat(-1, kFloat_0_River, kFloat_1_River);
        cursor->layers[0].vScrollSpeed = i == 0 ? kFloat_0_08_River : kFloat_0_04_RiverLava;
        cursor->layers[0].vScale = defaultVScale;
        cursor->layers[0].uExtent = defaultUExtent;
        cursor->layers[0].heightOffset = i == 0 ? kFloat_5_1200004_RiverLayers : kFloat_3_2_river;
        const char* name = String_FormatTemporary((riverTextureStrings + kRiverName_lavaTextureFormat), i == 0 ? (riverTextureStrings + kRiverName_upstream) : (riverTextureStrings + kRiverName_downstream));
        cursor->layers[0].bitmap = ResourceManager_LoadBitmap(g_resourceManager, name, 0, 0, 0);
        cursor = (NtRiverLavaLayout*)((char*)cursor + sizeof(NtRiverLavaLayer));
    }
}

extern void appFreeUni__FPv(void*);
extern void __dl__FPv(void*);

struct RiverAllocation {
    void* resource;
    int size;
};

// Clear and release an owned river array, preserving the original volatile size-reset sequence.
void* dtor_801E5E50(RiverAllocation* self, int shouldDelete) {
    if (self != 0) {
        if (self->resource != 0) {
            int oldValue = *(volatile int*)&self->size;
            self->size = oldValue - *(volatile int*)&oldValue;
            appFreeUni__FPv(self->resource);
        }
        if (shouldDelete > 0) {
            __dl__FPv(self);
        }
    }
    return self;
}

// Clear and release an owned river array, preserving the original volatile size-reset sequence.
void* RiverAllocation_Release(RiverAllocation* self, int shouldDelete) {
    if (self != 0) {
        if (self->resource != 0) {
            int oldValue = *(volatile int*)&self->size;
            self->size = oldValue - *(volatile int*)&oldValue;
            appFreeUni__FPv(self->resource);
        }
        if (shouldDelete > 0) {
            __dl__FPv(self);
        }
    }
    return self;
}

// Reset and release an owned river array and optionally free its wrapper.
void* dtor_801E5F30(RiverAllocation* self, int shouldDelete) {
    if (self != 0) {
        self->size = 0;
        if (self != 0 && self->resource != 0) {
            self->size = 0;
            appFreeUni__FPv(self->resource);
        }
        if (shouldDelete > 0) {
            __dl__FPv(self);
        }
    }
    return self;
}

// Reset and release an owned river array and optionally free its wrapper.
void* dtor_801E5FA4(RiverAllocation* self, int shouldDelete) {
    if (self != 0) {
        self->size = 0;
        if (self != 0 && self->resource != 0) {
            self->size = 0;
            appFreeUni__FPv(self->resource);
        }
        if (shouldDelete > 0) {
            __dl__FPv(self);
        }
    }
    return self;
}

} // extern "C"

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU804621A8", _ctors_u801E4C08_0, RiverLava_InitializeAngleCache);
#pragma pop

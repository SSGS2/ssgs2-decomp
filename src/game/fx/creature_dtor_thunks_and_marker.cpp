#include <decomp/static_init.h>
// Prefix returned by Camera_GetRenderState; matrices are passed directly to SetTransform.
struct MarkerCameraMatrices {
    char cameraParameters[0x20];
    float view[16];
    float projection[16];
};
#include "classes/CMarker.h"
enum CreatureLayout { kCreatureSecondaryInterfaceOffset = 4 };
#include "classes/WView.h"
// Translation unit 0x801A7580-0x801A8C20 (30 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

#include "sdk_protos.h"
#include "game/wvector.h"

extern "C" {
void CCreature_dtor(void* self, int deleteFlag);
void CGull_Dtor(void*, int);
void CButterfly_Dtor(void*, int);
void CDuck_Dtor(void*, int);
void CMole_Dtor(void*, int);
void CDolphin_Dtor(void*, int);
void CCrab_Dtor(void*, int);
void CSparrow_Dtor(void*, int);
void CSanta_Dtor(void*, int);
void CFighter_Dtor(void*, int);
void CUfo_Dtor(void*, int);
void ntPenguin_Dtor(void*, int);
void ntPenguin_Sailor_Dtor(void*, int);
void ntPenguin_Duck_Dtor(void*, int);
void ntNpc_Dragon_Dtor(void*, int);
void ntNpc_BlackSparrow_Dtor(void* self, int deleteFlag);
void ntNpc_PolaShip_Dtor(void* self, int deleteFlag);
void ntNpc_PolaPenguin1_Dtor(void*, int);
void ntNpc_PolaPenguin2_Dtor(void*, int);
void CDisposable_Dtor(void*, int);
void CCreature_ParseBoneReferences(void*, int, int);
void CCreature_PlaySoundFromParsedToken(void*, int);
void appFreeUni__FPv(void* self);
void CubeResourceStorage_dtor(void* self, int flags);
void __dl__FPv(void* self);
void String_ReplaceRange(void* a, int b, unsigned int c, const char* d, const char* e, const void* f);
unsigned char SharedDoc_GetCourseId(void);
void* W3dSpr_Create(void* a, const char* b, int c);
void W3dSpr_SetRect(void* a, float b, float c, float d, float e);
void VertexBuffer_Create(unsigned int a, int b, unsigned int c, int d, void* e, int f);
void Resource_GetOffsetPtr(void* a, int b, int c, void* d, int e);
void GraphicsBuffer_Flush(void* a);
void IndexBuffer_Create(unsigned int a, int b, unsigned int c, int d, void* e, int f, int g);
void IndexBuffer_GetOffsetPtr(void* a, int b, int c, void* d, int e);
void IndexBuffer_Flush(void* a);
void Engine_GetRenderState(int a, void* b);
void Engine_SetRenderStateList(void* a, int b);
void Engine_SetTextureStageStateList(int a, void* b, int c);
void Engine_SetTextureStageParameterList(int a, void* b, int c);
void* Camera_GetRenderState(void* a);
void* memcpy(void* a, const void* b, unsigned int c);
void SetTransform(int a, void* b);
void Resource_GetOffsetPtr(void* a, int b, int c, void* d, int e);
void GraphicsBuffer_Flush(void* a);
void Matrix_ExpandAffineTo4x4(void* a, void* b);
void Engine_SetType7Resource(int a);
void Engine_SetFlexibleVertexFormat(unsigned int a);
void Engine_SetVertexStream(int a, void* b, int c, int d);
void Engine_SetIndexBuffer(void* a, int b);
void* W3dSpr_GetBitmap(void* a, void* b);
void Engine_SetTextureStageResource(int a, void* b);
void Engine_SetTexturePalette(int a, void* b);
void Engine_DrawIndexedPrimitive(int a, int b, int c, int d, int e, int f);
void Engine_SetCachedRenderState(int a, int b);
extern char* g_view;
struct Words80 { unsigned int words[20]; };
struct Words32 { unsigned int words[8]; };
struct Words40 { unsigned int words[10]; };
struct RenderData83 { Words80 renderStates; Words32 textureStageStates; Words40 textureStageParameters; };
extern unsigned char kMarkerRenderStateDefaults[0x98];
struct Manager83 {
    virtual void pad00(); virtual void pad01(); virtual void pad02();
    virtual void pad03(); virtual void pad04(); virtual void pad05();
    virtual void pad06(); virtual void pad07(); virtual void pad08();
    virtual void pad09(); virtual void pad10(); virtual void pad11();
    virtual void pad12(); virtual void pad13(); virtual void pad14();
    virtual void pad15(); virtual void pad16(); virtual void pad17();
    virtual void pad18(); virtual void pad19(); virtual void pad20();
    virtual void pad21(); virtual void pad22(); virtual void pad23();
    virtual void pad24(); virtual void pad25();
    virtual void* a(void*);
    virtual void* b(void*);
};
extern float kIdentityMatrix4x4[];
extern float kFloat_0_Marker;
extern float kFloat_1_3dSprite;
extern float kFloat_2_Marker_805DF3A4;
extern float kFloat_0_5_3dSprite;
extern const float kFloat_Neg0_5;
void* g_resourceManager;
extern char counterResourceStrings[];
struct CounterResourceStrings {
    char vectorLengthError[20];
    char lengthErrorType[55];
    char course9Marker[17]; // ob_counter03.gsr, course 9
    char course12Marker[17]; // ob_counter04.gsr, course 12
    char defaultMarker[17]; // ob_counter01.gsr
};
enum { kCounter03CourseId = 9, kCounter04CourseId = 12,
       kMarkerVertexFormat = 0x110001, kMarkerTriangleStrip = 0x98 };
// CMarker word map (counts/capacities are elements, not bytes):
// 0 vtable; 1 sourcePositions; 2 pointCount; 3 pointCapacity;
// 4 sourceVertices; 5 vertexCount; 6 vertexCapacity;
// 7 vertexBuffer; 8 indexBuffer; 9 sprite. CPU arrays are owned allocations;
// GPU buffers are retained references; sprite uses its virtual deleting dtor.
enum MarkerStorageWord {
    kMarkerVtableWord = 0, kMarkerPointsWord = 1, kMarkerPointCountWord = 2,
    kMarkerPointCapacityWord = 3, kMarkerVerticesWord = 4,
    kMarkerVertexCountWord = 5, kMarkerVertexCapacityWord = 6,
    kMarkerVertexBufferWord = 7, kMarkerIndexBufferWord = 8, kMarkerSpriteWord = 9
};
// Record sizes and growth thresholds are element-count limits, not world units.
enum MarkerVectorLimits { kPointBytes = 12, kMarkerVertexBytes = 20,
    kPointMaximum = 0x15555555U, kPointGrowthThreeFifthsLimit = 0x071C71C7U,
    kPointGrowthHalfLimit = 0x0E38E38EU, kVertexMaximum = 0x0CCCCCCCU,
    kVertexGrowthThreeFifthsLimit = 0x04444444U, kVertexGrowthHalfLimit = 0x08888888U };
// Query (0x1F,2) gates sprite creation: only status zero creates resources.
// The manager property domain is unresolved; do not infer a visibility bit.
enum MarkerManagerQuery { kMarkerResourceQueryCode = 0x1F, kMarkerResourceQueryArgument = 2 };
enum MarkerBufferReference { kBufferReferenceShift = 16, kBufferFlagMask = 0xFFFF };
// Buffer word 0 packs a reference count in bits 31..16 and unrelated flags in
// bits 15..0. Releasing decrements only the high half, destroys at zero, and
// clears the owner's handle. Do not decrement the packed word as a whole.
extern unsigned int __vt__7CMarker[4];
extern float kFloat_6_2831855;
extern float s_twoPi_MarkerRendering;

// Adjust the secondary-interface pointer back to its owner before destruction.
void CCreature_DtorThunk4(void* self, int deleteFlag) { CCreature_dtor((char*)self - kCreatureSecondaryInterfaceOffset, deleteFlag); }
// Adjust the secondary-interface pointer back to its owner before destruction.
void CGull_DtorThunk4(void* self, int forwarded1) { CGull_Dtor((char*)self - kCreatureSecondaryInterfaceOffset, forwarded1); }
// Adjust the secondary-interface pointer back to its owner before destruction.
void CButterfly_DtorThunk4(void* self, int forwarded1) { CButterfly_Dtor((char*)self - kCreatureSecondaryInterfaceOffset, forwarded1); }
// Adjust the secondary-interface pointer back to its owner before destruction.
void CDuck_DtorThunk4(void* self, int forwarded1) { CDuck_Dtor((char*)self - kCreatureSecondaryInterfaceOffset, forwarded1); }
// Adjust the secondary-interface pointer back to its owner before destruction.
void CMole_DtorThunk4(void* self, int forwarded1) { CMole_Dtor((char*)self - kCreatureSecondaryInterfaceOffset, forwarded1); }
// Adjust the secondary-interface pointer back to its owner before destruction.
void CDolphin_DtorThunk4(void* self, int forwarded1) { CDolphin_Dtor((char*)self - kCreatureSecondaryInterfaceOffset, forwarded1); }
// Adjust the secondary-interface pointer back to its owner before destruction.
void CCrab_DtorThunk4(void* self, int forwarded1) { CCrab_Dtor((char*)self - kCreatureSecondaryInterfaceOffset, forwarded1); }
// Adjust the secondary-interface pointer back to its owner before destruction.
void CSparrow_DtorThunk4(void* self, int forwarded1) { CSparrow_Dtor((char*)self - kCreatureSecondaryInterfaceOffset, forwarded1); }
// Adjust the secondary-interface pointer back to its owner before destruction.
void CSanta_DtorThunk4(void* self, int forwarded1) { CSanta_Dtor((char*)self - kCreatureSecondaryInterfaceOffset, forwarded1); }
// Adjust the secondary-interface pointer back to its owner before destruction.
void CFighter_DtorThunk4(void* self, int forwarded1) { CFighter_Dtor((char*)self - kCreatureSecondaryInterfaceOffset, forwarded1); }
// Adjust the secondary-interface pointer back to its owner before destruction.
void CUfo_DtorThunk4(void* self, int forwarded1) { CUfo_Dtor((char*)self - kCreatureSecondaryInterfaceOffset, forwarded1); }
// Adjust the secondary-interface pointer back to its owner before destruction.
void ntPenguin_DtorThunk4(void* self, int forwarded1) { ntPenguin_Dtor((char*)self - kCreatureSecondaryInterfaceOffset, forwarded1); }
// Adjust the secondary-interface pointer back to its owner before destruction.
void ntPenguin_Sailor_DtorThunk4(void* self, int forwarded1) { ntPenguin_Sailor_Dtor((char*)self - kCreatureSecondaryInterfaceOffset, forwarded1); }
// Adjust the secondary-interface pointer back to its owner before destruction.
void ntPenguin_Duck_DtorThunk4(void* self, int forwarded1) { ntPenguin_Duck_Dtor((char*)self - kCreatureSecondaryInterfaceOffset, forwarded1); }
// Adjust the secondary-interface pointer back to its owner before destruction.
void ntNpc_Dragon_DtorThunk4(void* self, int forwarded1) { ntNpc_Dragon_Dtor((char*)self - kCreatureSecondaryInterfaceOffset, forwarded1); }
// Adjust the secondary-interface pointer back to its owner before destruction.
void ntNpc_BlackSparrow_DtorThunk4(void* self, int deleteFlag) { ntNpc_BlackSparrow_Dtor((char*)self - kCreatureSecondaryInterfaceOffset, deleteFlag); }
// Adjust the secondary-interface pointer back to its owner before destruction.
void ntNpc_PolaShip_DtorThunk4(void* self, int deleteFlag) { ntNpc_PolaShip_Dtor((char*)self - kCreatureSecondaryInterfaceOffset, deleteFlag); }
// Adjust the secondary-interface pointer back to its owner before destruction.
void ntNpc_PolaPenguin1_DtorThunk4(void* self, int forwarded1) { ntNpc_PolaPenguin1_Dtor((char*)self - kCreatureSecondaryInterfaceOffset, forwarded1); }
// Adjust the secondary-interface pointer back to its owner before destruction.
void ntNpc_PolaPenguin2_DtorThunk4(void* self, int forwarded1) { ntNpc_PolaPenguin2_Dtor((char*)self - kCreatureSecondaryInterfaceOffset, forwarded1); }
// Adjust the secondary-interface pointer back to its owner before destruction.
void CDisposable_DtorThunk4(void* self, int forwarded1) { CDisposable_Dtor((char*)self - kCreatureSecondaryInterfaceOffset, forwarded1); }
// Adjust the secondary-interface pointer back to its owner before forwarding the method.
void CCreature_vfunc_36Thunk4(void* self, int forwarded1, int forwarded2) { CCreature_ParseBoneReferences((char*)self - kCreatureSecondaryInterfaceOffset, forwarded1, forwarded2); }
// Adjust the secondary-interface pointer back to its owner before forwarding the method.
void CCreature_vfunc_37Thunk4(void* self, int forwarded1) { CCreature_PlaySoundFromParsedToken((char*)self - kCreatureSecondaryInterfaceOffset, forwarded1); }
// Install the marker vtable and clear point/vertex arrays and render resources.
void CMarker_Ctor(void* self) {
    unsigned int* p = (unsigned int*)self;
    int zero = 0;
    p[kMarkerPointsWord] = zero;
    p[kMarkerVtableWord] = (unsigned int)&__vt__7CMarker;
    p[kMarkerPointCountWord] = zero;
    p[kMarkerPointCapacityWord] = zero;
    p[kMarkerVerticesWord] = zero;
    p[kMarkerVertexCountWord] = zero;
    p[kMarkerVertexCapacityWord] = zero;
    p[kMarkerVertexBufferWord] = zero;
    p[kMarkerIndexBufferWord] = zero;
    p[kMarkerSpriteWord] = zero;
}
// Clear and free a marker vector's backing allocation, then optionally delete the vector object.
void* dtor_801A8BA0(void* self, int flags) {
    if (self != 0) {
        ((unsigned int*)self)[1] = 0;
        if (self != 0 && *(void**)self != 0) {
            ((unsigned int*)self)[1] = 0;
            appFreeUni__FPv(*(void**)self);
        }
        if (flags > 0) __dl__FPv(self);
    }
    return self;
}
// Clear generated counts and release vertex/index buffer references and the sprite.
void CMarker_ReleaseRenderResources(void* self) {
    unsigned int* p = (unsigned int*)self;
    unsigned int t2 = p[kMarkerPointCountWord];
    void* a = AsCMarker(self)->vertexBuffer;
    unsigned int t5 = p[kMarkerVertexCountWord];
    p[kMarkerPointCountWord] = t2 - t2;
    p[kMarkerVertexCountWord] = t5 - t5;
    if (a) {
        unsigned int v = *(unsigned int*)a;
        unsigned int hi = (v >> kBufferReferenceShift) - 1;
        v = (v & kBufferFlagMask) | (hi << kBufferReferenceShift);
        *(unsigned int*)a = v;
        if ((v >> 16) == 0) CubeResourceStorage_dtor(a, 1);
        p[kMarkerVertexBufferWord] = 0;
    }
    void* b = AsCMarker(self)->indexBuffer;
    if (b) {
        unsigned int v = *(unsigned int*)b;
        unsigned int hi = (v >> 16) - 1;
        v = (v & 0xFFFF) | (hi << 16);
        *(unsigned int*)b = v;
        if ((v >> 16) == 0) CubeResourceStorage_dtor(b, 1);
        p[kMarkerIndexBufferWord] = 0;
    }
    void* c = AsCMarker(self)->sprite;
    if (c) {
        if (c) {
            void** vtable = *(void***)c;
            ((void (*)(void*, int))vtable[2])(c, 1);
        }
        p[kMarkerSpriteWord] = 0;
    }
}
// Release render resources and both CPU-side arrays, then optionally delete the marker.
// Repeated subobject guards retain the original inlined vector-destruction branches.
void* CMarker_dtor(void* self, int flags) {
    // Phase 1: invalidate counts, release both packed-reference GPU buffers,
    // then delete the sprite. CPU allocations remain owned until phase 2.
    if (self) {
        unsigned int* p = (unsigned int*)self;
        volatile unsigned int* vp = (volatile unsigned int*)self;
        unsigned int t2 = vp[2];
        unsigned int vt = (unsigned int)&__vt__7CMarker;
        unsigned int t5 = vp[5];
        void* c = (void*)vp[7];
        vp[2] = t2 - t2;
        vp[5] = t5 - t5;
        vp[0] = vt;
        if (c) {
            unsigned int v = *(unsigned int*)c;
                unsigned int hi = (v >> 16) - 1;
                v = (hi << 16) | (v & 0xFFFF);
                *(unsigned int*)c = v;
                if ((v >> 16) == 0) CubeResourceStorage_dtor(c, 1);
                p[7] = 0;
        }
        {
            void* d = (void*)p[8];
            if (d) {
                unsigned int v = *(unsigned int*)d;
                unsigned int hi = (v >> 16) - 1;
                v = (hi << 16) | (v & 0xFFFF);
                *(unsigned int*)d = v;
                if ((v >> 16) == 0) CubeResourceStorage_dtor(d, 1);
                p[8] = 0;
            }
        }
        {
            void* e = (void*)p[9];
            if (e) {
                if (e) {
                    void** vtab = *(void***)e;
                    ((void(*)(void*, int))vtab[2])(e, 1);
                }
                p[9] = 0;
            }
        }
        // Phase 2: destroy the two vector subobjects, vertices before positions.
        // Nested address guards are inlined destructor scaffolding, not separate
        // optional resources; only p[4]/p[1] test actual backing allocations.
        if ((void*)&AsCMarker(self)->sourceVertices) {
            if ((void*)&AsCMarker(self)->sourceVertices) {
                if ((void*)&AsCMarker(self)->sourceVertices) {
                    if ((void*)&AsCMarker(self)->sourceVertices) {
                        if (p[4]) {
                            volatile unsigned int* vq = (volatile unsigned int*)self;
                            unsigned int u = vq[5];
                            vq[5] = u - u;
                            appFreeUni__FPv((void*)p[4]);
                        }
                    }
                }
            }
        }
        if ((void*)&AsCMarker(self)->sourcePositions) {
            if ((void*)&AsCMarker(self)->sourcePositions) {
                if ((void*)&AsCMarker(self)->sourcePositions) {
                    if ((void*)&AsCMarker(self)->sourcePositions) {
                        if (p[1]) {
                            volatile unsigned int* vq = (volatile unsigned int*)self;
                            unsigned int u = vq[2];
                            vq[2] = u - u;
                            appFreeUni__FPv((void*)p[1]);
                        }
                    }
                }
            }
        }
        if (flags > 0) __dl__FPv(self);
    }
    return self;
}

void* CQuadTree_GetInstance(unsigned int);
void GetGroundPoint__9CQuadTreeF7WVectorbPib(void*, void*, void*, int, int*, int);
void __as__7WVectorFRC7WVector(void*, void*);
void* __nwa__FUl(unsigned int);
void* appMallocDummy__FUl(unsigned int);
void* __nw__FUl(unsigned long);
void __dla__FPv(void*);
extern "C" void __dt__Q23std9exceptionFv(void*, int);
extern char __vt__Q23std11logic_error[];
extern char __vt__Q23std12length_error[];
extern char s_sharedCharDeleterVtable[];
}

// Construct an element in already allocated vector storage.
inline void* operator new(size_t, void* p) { return p; }
// Placement construction cleanup does not own the surrounding allocation.
inline void operator delete(void*, void*) {}
union Point78 {
    struct { float x, y, z; };
    struct { unsigned int a, b, c; } bits;
    // Leave words for the subsequent terrain-position copy.
    Point78() {}
};
union Vertex78 {
    struct { float x, y, z, u, v; };
    struct { unsigned int a, b, c, d, e; } bits;
    // Leave words for the subsequent position/UV assignments.
    Vertex78() {}
    // Copy all five words, preserving the exact position and UV bit patterns.
    Vertex78& operator=(const Vertex78& other) { bits = other.bits; return *this; }
};
struct Input78 { unsigned int a, b, c; };
struct Inputs78 { Input78* data; unsigned int size; };
struct Object78 {
    void* vtable;
    Point78* points;
    unsigned int pointSize, pointCapacity;
    Vertex78* vertices;
    unsigned int vertexSize, vertexCapacity;
};
struct Buffer78 {
    char* data;
    void* allocator;
    // Delete an exception-message allocation unless ownership was transferred.
    ~Buffer78() { if (data) __dla__FPv(data); }
};
struct Shared78 {
    void* vtable;
    int strong, weak;
    char* data;
    // Own the message with its initial strong/weak reference pair.
    Shared78(char* allocation) {
        strong = 1;
        weak = 1;
        vtable = s_sharedCharDeleterVtable;
        data = allocation;
    }
};
struct Alloc78 {};
// Return the larger capacity increment by reference, preserving its original load.
inline const unsigned int& max78(const unsigned int& a, const unsigned int& b) {
    return a < b ? b : a;
}
// Remove a count of trivially destructible elements without releasing storage.
inline void clear78(unsigned int& size, unsigned int count) {
    Alloc78 allocator = Alloc78();
    size -= count;
}
namespace std {
struct exception {
    void* vtable;
    char* message;
    void* owner;
    // Copy the exception text and transfer it from a guard to shared ownership.
    exception(const char* const& text) {
        vtable = __vt__Q23std11logic_error;
        Alloc78 allocator0 = Alloc78();
        Alloc78 allocator = Alloc78();
        unsigned int length = strlen(text) + 1;
        message = (char*)__nwa__FUl(length * 2 - length);
        {
            Buffer78 buffer;
            char* allocation = message;
            buffer.data = allocation;
            buffer.allocator = &allocator;
            Shared78* control = new Shared78(allocation);
            owner = control;
            buffer.data = 0;
        }
        strcpy(message, text);
    }
    ~exception();
};
struct logic_error : public exception {
    // Construct the logic-error message through the shared exception base.
    logic_error(const char* text) : exception(text) {}

    ~logic_error();
};
class length_error : public logic_error {
public:
    // Construct the vector length error and install its derived vtable.
    length_error(const char* text) : logic_error(text) { vtable = __vt__Q23std12length_error; }
    ~length_error();
};
}
struct PointTemp78 {
    Point78* data;
    unsigned int size, capacity;
    unsigned int* allocator;
    unsigned int offset;
    // Dispose staged point storage, or old vector storage after a successful swap.
    ~PointTemp78() {
        if (data) {
            clear78(size, size);
            appFreeUni__FPv(data);
        }
    }
};
struct VertexTemp78 {
    Vertex78* data;
    unsigned int size, capacity;
    unsigned int* allocator;
    unsigned int offset;
    // Dispose staged vertex storage, or old vector storage after a successful swap.
    ~VertexTemp78() {
        if (data) {
            clear78(size, size);
            appFreeUni__FPv(data);
        }
    }
};

// Each expansion is a separate inlined construction of std::length_error.
#define LENGTH_ERROR78() throw std::length_error(counterResourceStrings)

// Project input world positions onto terrain and append one center/four XYZUV vertices.
// input is borrowed; self owns both vectors. Terrain status <0 skips a point.
// Reserve/copy/swap blocks below only provide storage: quad geometry is set
// in Phase 2. Each temporary owns new storage until commit, then old storage.
// Capacity exceptions preserve existing allocations, but point append occurs
// before vertex append: callers must not assume both vectors roll back together.
extern "C" void BuildGroundProjectedPointQuads(Object78* self, Inputs78* input) {
    // Each accepted terrain query contributes one world-space XYZ and four
    // XYZUV vertices. Rejected points contribute neither. Vector growth uses
    // element counts; allocations multiply by 12 (point) or 20 (vertex) bytes.
    unsigned int i = 0;
    unsigned int byteOffset = 0;
    for (; i < input->size; byteOffset += 12, ++i) {
        Input78* entry = (Input78*)((char*)input->data + byteOffset);
        Point78 point;
        Point78 intermediate;
        Input78 source = *entry;
        int status;
        GetGroundPoint__9CQuadTreeF7WVectorbPib(&intermediate, CQuadTree_GetInstance(source.a), &source, 1, &status, 0);
        *(WVector*)&point = *(WVector*)&intermediate;
        if (status < 0) continue;

        // Phase 1: append the projected point, staging old storage if growth is needed.
        Alloc78 pointAllocator = Alloc78();
        if (self->pointSize < self->pointCapacity) {
            Point78* dest = self->points + self->pointSize;
            new (dest) Point78(point);
            ++self->pointSize;
        } else {
            Alloc78 growthAllocator = Alloc78();
            Alloc78 limitAllocator = Alloc78();
            if (kPointMaximum - self->pointCapacity < 1U) LENGTH_ERROR78();
            unsigned int needed = self->pointSize + 1 - self->pointCapacity;
            PointTemp78 temp;
            temp.data = 0;
            unsigned int newCapacity;
            unsigned int capacity = self->pointCapacity;
            temp.size = 0;
            temp.capacity = 0;
            temp.allocator = &self->pointCapacity;
            temp.offset = 0;
            if (needed > kPointMaximum - capacity) LENGTH_ERROR78();
            if (capacity < kPointGrowthThreeFifthsLimit) {
                unsigned int growth = (3 * (capacity + 1)) / 5;
                newCapacity = capacity + max78(growth, needed);
            } else if (capacity < kPointGrowthHalfLimit) {
                unsigned int growth = (capacity + 1) >> 1;
                newCapacity = capacity + max78(growth, needed);
            } else {
                newCapacity = kPointMaximum;
            }
            Alloc78 allocationAllocator = Alloc78();
            if (newCapacity > kPointMaximum) LENGTH_ERROR78();
            // Reserve: allocate replacement storage, then construct the new tail.
            Point78* allocation = (Point78*)appMallocDummy__FUl(newCapacity * kPointBytes);
            unsigned int oldSize = self->pointSize;
            unsigned int constructed = temp.size;
            temp.data = allocation;
            temp.capacity = newCapacity;
            temp.offset = oldSize;
            Point78* dest = allocation + oldSize + constructed;
            Alloc78 insertAllocator = Alloc78();
            unsigned int count = 1;
            while (count != 0) {
                new (dest) Point78(point);
                --count;
                ++dest;
                ++temp.size;
            }
            // Copy: relocate old elements backwards into the reserved prefix.
            Point78* begin = self->points;
            Alloc78 copyAllocator = Alloc78();
            Point78* end = begin + self->pointSize;
            dest = temp.data + temp.offset;
            while (end > begin) {
                --dest;
                --end;
                new (dest) Point78(*end);
                --temp.offset;
                ++temp.size;
            }
            // Commit: exchange ownership; temp frees the superseded allocation.
            Point78* oldData = self->points;
            Point78* newData = temp.data;
            unsigned int newSize = temp.size;
            self->pointSize = 0;
            unsigned int oldCapacity = self->pointCapacity;
            Alloc78 exchangeAllocator = Alloc78();
            temp.data = oldData;
            self->pointCapacity = temp.capacity;
            temp.capacity = oldCapacity;
            self->points = newData;
            self->pointSize = newSize;
            temp.size = 0;
        }

        // Phase 2: form a one-unit-wide, two-unit-tall quad around that point;
        // UVs span [0,1], with V flipped. Draw later adjusts X/Z to face the view.
        Vertex78 corners[4];
        int corner = 0;
        Vertex78* vertex = corners;
        for (; corner < 4; ++vertex, ++corner) {
            int column = corner & 1;
            int row = corner >> 1;
            float left = point.x - kFloat_0_5_3dSprite;
            vertex->x = left + (float)column;
            vertex->y = kFloat_2_Marker_805DF3A4 * (float)row + point.y;
            vertex->z = point.z;
            vertex->u = (float)column;
            vertex->v = kFloat_1_3dSprite - (float)row;
            Alloc78 vertexAllocator = Alloc78();
            if (self->vertexSize < self->vertexCapacity) {
                unsigned int size = self->vertexSize + 1;
                self->vertexSize = size;
                self->vertices[size - 1] = *vertex;
            } else {
                Alloc78 growthAllocator = Alloc78();
                Alloc78 limitAllocator = Alloc78();
                if (kVertexMaximum - self->vertexCapacity < 1U) LENGTH_ERROR78();
                unsigned int needed = self->vertexSize + 1 - self->vertexCapacity;
                VertexTemp78 temp;
                temp.data = 0;
                unsigned int newCapacity;
                unsigned int capacity = self->vertexCapacity;
                temp.size = 0;
                temp.capacity = 0;
                temp.allocator = &self->vertexCapacity;
                temp.offset = 0;
                if (needed > kVertexMaximum - capacity) LENGTH_ERROR78();
                if (capacity < kVertexGrowthThreeFifthsLimit) {
                    unsigned int growth = 3 * (capacity + 1) / 5;
                    newCapacity = capacity + max78(growth, needed);
                } else if (capacity < kVertexGrowthHalfLimit) {
                    unsigned int growth = (capacity + 1) >> 1;
                    newCapacity = capacity + max78(growth, needed);
                } else {
                    newCapacity = kVertexMaximum;
                }
                Alloc78 allocationAllocator = Alloc78();
                if (newCapacity > kVertexMaximum) LENGTH_ERROR78();
                // Reserve/copy/commit mirrors the center vector above, in 20-byte elements.
                Vertex78* allocation = (Vertex78*)appMallocDummy__FUl(newCapacity * kMarkerVertexBytes);
                unsigned int oldSize = self->vertexSize;
                temp.data = allocation;
                Vertex78* dest = allocation + oldSize;
                dest[temp.size] = *vertex;
                temp.capacity = newCapacity;
                temp.offset = oldSize;
                Alloc78 insertAllocator = Alloc78();
                ++temp.size;
                Vertex78* begin = self->vertices;
                Alloc78 copyAllocator = Alloc78();
                Vertex78* end = begin + self->vertexSize;
                while (end > begin) {
                    --dest;
                    --end;
                    new (dest) Vertex78(*end);
                    --temp.offset;
                    ++temp.size;
                }
                Vertex78* oldData = self->vertices;
                Vertex78* newData = temp.data;
                unsigned int newSize = temp.size;
                self->vertexSize = 0;
                unsigned int oldCapacity = self->vertexCapacity;
                Alloc78 exchangeAllocator = Alloc78();
                temp.data = oldData;
                self->vertexCapacity = temp.capacity;
                temp.capacity = oldCapacity;
                self->vertices = newData;
                self->vertexSize = newSize;
                temp.size = 0;
            }
        }
    }
}
#undef LENGTH_ERROR78

struct Manager88 {
    virtual void pad0(); virtual void pad1(); virtual void pad2();
    virtual void pad3(); virtual void pad4(); virtual void pad5();
    virtual void pad6();
    virtual void query(int, int, int*);
};
struct String88 {
    unsigned int flags, size;
    char* data;
    // Initialize an empty inline string.
    String88() { flags = 0; size = 0; data = 0; }
    // Free only heap-backed string storage (flag bit 31).
    ~String88() { if (flags >> 31) appFreeUni__FPv(data); }
    // Decode the heap length or the inline first-byte length.
    unsigned int length() const {
        unsigned int result;
        if (flags >> 31) result = size;
        else result = *(const unsigned char*)&flags & 0x7f;
        return result;
    }
    // Borrow NUL-terminated string bytes until this string changes or is destroyed.
    const char* c_str() const {
        const char* result;
        if (flags >> 31) result = data;
        else result = (const char*)&flags + 1;
        return result;
    }
};
struct Alloc88 { char value; };

extern "C" {

// Choose the course's marker sprite, upload vertices, and build a strip with degenerate joins.
void CMarker_CreateRenderResources(void* self) {
    // Phase 1: select a course-specific sprite name; no resources for zero points.
    // obj uses the MarkerStorageWord map above; vertexBytes = vertexCount * 20.
    unsigned int* obj = (unsigned int*)self;
    if (!obj[2]) return;
    int status;
    (*(Manager88**)g_resourceManager)->query(kMarkerResourceQueryCode, kMarkerResourceQueryArgument, &status);
    String88 name;
    if (SharedDoc_GetCourseId() == kCounter03CourseId) {
        const char* label = ((CounterResourceStrings*)counterResourceStrings)->course9Marker;
        unsigned int count;
        if ((name.flags >> 31) == 0) count = *(unsigned char*)&name.flags & 0x7f;
        else count = name.size;
        Alloc88 allocator;
        Alloc88 copy = allocator;
        unsigned int len = strlen(label);
        String_ReplaceRange(&name, 0, count, label, label + len, &copy);
    } else {
        if (SharedDoc_GetCourseId() == kCounter04CourseId) {
            unsigned int count;
            const char* label = ((CounterResourceStrings*)counterResourceStrings)->course12Marker;
            if ((name.flags >> 31) == 0) count = *(unsigned char*)&name.flags & 0x7f;
            else count = name.size;
            Alloc88 allocator;
            Alloc88 copy = allocator;
            unsigned int len = strlen(label);
            String_ReplaceRange(&name, 0, count, label, label + len, &copy);
        } else {
            unsigned int count;
            const char* label = ((CounterResourceStrings*)counterResourceStrings)->defaultMarker;
            if ((name.flags >> 31) == 0) count = *(unsigned char*)&name.flags & 0x7f;
            else count = name.size;
            Alloc88 allocator;
            Alloc88 copy = allocator;
            unsigned int len = strlen(label);
            String_ReplaceRange(&name, 0, count, label, label + len, &copy);
        }
    }
    if (status == 0) {
        void* manager = g_resourceManager;
        const char* text;
        if ((name.flags >> 31) == 0) text = (const char*)&name.flags + 1;
        else text = name.data;
        void* alloc = W3dSpr_Create(manager, text, 0);
        obj[9] = (unsigned int)alloc;
        W3dSpr_SetRect(alloc, kFloat_1_3dSprite, kFloat_2_Marker_805DF3A4, kFloat_0_5_3dSprite, kFloat_0_Marker);
        unsigned int vertexBytes = obj[kMarkerVertexCountWord] * kMarkerVertexBytes;
        VertexBuffer_Create(vertexBytes, 0, kMarkerVertexFormat, 0, &obj[7], 0);
        void* verts;
        Resource_GetOffsetPtr((void*)obj[7], 0, 0, &verts, 0);
        memcpy(verts, (void*)obj[4], vertexBytes);
        GraphicsBuffer_Flush((void*)obj[7]);
        // Phase 2: upload four vertices per point and 16-bit strip indices.
        // Four indices draw each quad; two duplicate endpoints join successive quads
        // without drawing a connecting triangle: 6*pointCount-2 indices total.
        unsigned int indexBytes = (obj[2] * 6 - 2) << 1;
        IndexBuffer_Create(indexBytes, 0, 3, 0, &obj[8], 0, 0);
        void* indexData;
        IndexBuffer_GetOffsetPtr((void*)obj[8], 0, 0, &indexData, 0);
        unsigned int firstVertexIndex, indexByteOffset, secondVertexIndex, thirdVertexIndex, fourthVertexIndex, nextQuadFirstIndex, pointIndex;
        pointIndex = 0;
        firstVertexIndex = 0;
        indexByteOffset = 0;
        for (; pointIndex < obj[2]; ) {
            secondVertexIndex = firstVertexIndex + 1;
            thirdVertexIndex = secondVertexIndex + 1;
            fourthVertexIndex = thirdVertexIndex + 1;
            nextQuadFirstIndex = fourthVertexIndex + 1;
            *(unsigned short*)((char*)indexData + indexByteOffset) = firstVertexIndex;
            *(unsigned short*)((char*)indexData + indexByteOffset + 2) = secondVertexIndex;
            *(unsigned short*)((char*)indexData + indexByteOffset + 4) = thirdVertexIndex;
            *(unsigned short*)((char*)indexData + indexByteOffset + 6) = fourthVertexIndex;
            if (pointIndex < obj[2] - 1) {
                *(unsigned short*)((char*)indexData + indexByteOffset + 8) = fourthVertexIndex;
                *(unsigned short*)((char*)indexData + indexByteOffset + 10) = nextQuadFirstIndex;
            }
            firstVertexIndex += 4;
            indexByteOffset += 12;
            pointIndex += 1;
        }
        IndexBuffer_Flush((void*)obj[8]);
    }
}
// Face marker quads toward the camera, draw, and restore cached render-state 1.
// Other transforms, streams, texture bindings and stage state stay installed.
void CMarker_Draw(void* self) {
    // Phase 1: save render-state 1, install the fixed state tables and select projection.
    unsigned char* data = kMarkerRenderStateDefaults;
    unsigned char* obj = (unsigned char*)self;
    if (AsCMarker(self)->pointCount && AsCMarker(self)->sprite) {
        int savedRenderState1;
        Engine_GetRenderState(1, &savedRenderState1);
        Words80 renderStates = ((RenderData83*)data)->renderStates;
        Words32 textureStageStates = ((RenderData83*)data)->textureStageStates;
        Words40 textureStageParameters = ((RenderData83*)data)->textureStageParameters;
        Engine_SetRenderStateList(&renderStates, 10);
        Engine_SetTextureStageStateList(0, &textureStageStates, 4);
        Engine_SetTextureStageParameterList(0, &textureStageParameters, 5);
        void* cameraState=Camera_GetRenderState(g_view);
        if ((((MarkerCameraMatrices*)cameraState)->projection[14] >= kFloat_0_Marker && ((MarkerCameraMatrices*)cameraState)->projection[10]>kFloat_1_3dSprite) ||
            (((MarkerCameraMatrices*)cameraState)->projection[14]<=kFloat_0_Marker && ((MarkerCameraMatrices*)cameraState)->projection[10]<kFloat_0_Marker))
            SetTransform(1, ((MarkerCameraMatrices*)cameraState)->projection);
        void* vertexData;
        Resource_GetOffsetPtr(AsCMarker(obj)->vertexBuffer,0,0,&vertexData,0);
        int zero = 0, one = 1;
        unsigned int i = 0;
        unsigned int quadFloatOffset = 0, centerFloatOffset = 0;
        // Phase 2: rebuild camera-facing horizontal offsets; leave height and UV untouched.
        // Reconstruct world X/Z from each source center and viewMatrix[0]/[6];
        // Y and UV remain those uploaded at creation. Strides are three source
        // floats and twenty destination floats (four XYZUV vertices) per marker.
        // Vertex78 gives the XYZUV layout: quad X offsets 0/5/10/15 and Z offsets
        // 2/7/12/17. The two left corners share X/Z, as do the two right corners.
        // No world-unit-to-pixel conversion occurs in this loop.
        for (;i<AsCMarker(obj)->pointCount;i++) {
            float x0 = kFloat_Neg0_5 + (float)zero;
            ((float*)vertexData)[quadFloatOffset] = x0 * ((WView*)g_view)->viewMatrix[0] + ((float*)AsCMarker(obj)->sourcePositions)[centerFloatOffset];
            float y0 = kFloat_Neg0_5 + (float)zero;
            ((float*)vertexData)[quadFloatOffset+2] = -(y0 * ((WView*)g_view)->viewMatrix[6] - ((float*)AsCMarker(obj)->sourcePositions)[centerFloatOffset+2]);
            ((float*)vertexData)[quadFloatOffset+5] = ((WView*)g_view)->viewMatrix[0] * (kFloat_Neg0_5 + (float)one) + ((float*)AsCMarker(obj)->sourcePositions)[centerFloatOffset];
            ((float*)vertexData)[quadFloatOffset+7] = -(((WView*)g_view)->viewMatrix[6] * (kFloat_Neg0_5 + (float)one) - ((float*)AsCMarker(obj)->sourcePositions)[centerFloatOffset+2]);
            ((float*)vertexData)[quadFloatOffset+10] = ((WView*)g_view)->viewMatrix[0] * (kFloat_Neg0_5 + (float)zero) + ((float*)AsCMarker(obj)->sourcePositions)[centerFloatOffset];
            ((float*)vertexData)[quadFloatOffset+12] = -(((WView*)g_view)->viewMatrix[6] * (kFloat_Neg0_5 + (float)zero) - ((float*)AsCMarker(obj)->sourcePositions)[centerFloatOffset+2]);
            ((float*)vertexData)[quadFloatOffset+15] = ((WView*)g_view)->viewMatrix[0] * (kFloat_Neg0_5 + (float)one) + ((float*)AsCMarker(obj)->sourcePositions)[centerFloatOffset];
            ((float*)vertexData)[quadFloatOffset+17] = -(((WView*)g_view)->viewMatrix[6] * (kFloat_Neg0_5 + (float)one) - ((float*)AsCMarker(obj)->sourcePositions)[centerFloatOffset+2]);
            centerFloatOffset += 3;
            quadFloatOffset += 20;
        }
        // Phase 3: publish CPU edits, bind transforms/buffers/textures, and draw.
        GraphicsBuffer_Flush(AsCMarker(obj)->vertexBuffer);
        float matrix[16];
        Matrix_ExpandAffineTo4x4(((WView*)g_view)->inputTransform, matrix);
        SetTransform(0,matrix);
        Engine_SetType7Resource(0);
        SetTransform(2,kIdentityMatrix4x4);
        SetTransform(11,kIdentityMatrix4x4);
        Engine_SetFlexibleVertexFormat(kMarkerVertexFormat);
        Engine_SetVertexStream(0,AsCMarker(obj)->vertexBuffer,0,kMarkerVertexBytes);
        Engine_SetIndexBuffer(AsCMarker(obj)->indexBuffer,0);
        Manager83* renderTarget=(Manager83*)((WView*)g_view)->renderTarget;
        Engine_SetTextureStageResource(0, renderTarget->a(W3dSpr_GetBitmap(AsCMarker(obj)->sprite,g_view)));
        renderTarget=(Manager83*)((WView*)g_view)->renderTarget;
        Engine_SetTexturePalette(0, renderTarget->b(W3dSpr_GetBitmap(AsCMarker(obj)->sprite,g_view)));
        unsigned int stripIndexCount=AsCMarker(obj)->pointCount*6;
        Engine_DrawIndexedPrimitive(kMarkerTriangleStrip,0,0,stripIndexCount-2,0,stripIndexCount-4); // original +8
        Engine_SetCachedRenderState(1,savedRenderState1);
    }
}
// Initialize the marker-rendering helper's two-pi cache.
void Marker_InitTwoPiCache(void) { s_twoPi_MarkerRendering = kFloat_6_2831855; }
} // extern "C"

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU80462170", _ctors_u801A7580_0, Marker_InitTwoPiCache);
#pragma pop

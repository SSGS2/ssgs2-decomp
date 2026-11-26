// Beach wavelets and ground strips: the wave strip geometry and its point arrays, the wavelet
// setup from a beach parameter name, and the strip parameter lists that CBeach fills in.

#include "game/types.h"
#include "game/wvector.h"

#include "classes/CSharedDoc.h"
#include "sdk_protos.h"
#include "classes/CWavelet.h"
#include "classes/CGolfDoc.h"
#include "classes/CBeach.h"
#include "classes/CTutorial.h"

// Borrow the wavelet layout view; callers retain ownership of the enclosing object.
static inline CWavelet* AsCWavelet(void* p) {
    return (CWavelet*)p;
}

// Construct a record in existing vector storage; no allocation occurs.
inline void* operator new(size_t, void* p) {
    return p;
}
// Placement-delete is a no-op because the vector owns the enclosing allocation.
inline void operator delete(void*, void*) {}

extern "C" {

// Preserve the strip-point constructor hook; vectors are filled by the geometry builder.
void GroundStripPoint_DefaultConstructNoOp(void*) {}

struct Entry8020 {
    Vec3f center;
    Vec3f outerEdge;
    Vec3f widthDirection;
};
// One row uses world-space center/outerEdge positions and a horizontal unit
// width direction in `widthDirection`: normalize(world-up cross horizontal path tangent).
// It lies across the strip in XZ; it is not a terrain surface normal.
// Arrays own their storage; entries returned below are borrowed, index unchecked.

// Return the indexed center/edge/widthDirection record from the strip-point array.
extern "C" Entry8020* EntryArray_GetEntry(Entry8020** entries, int index) {
    return *entries + index;
}

extern const float kFloat_16_GeneTex3D;
extern char s_waveletVtable_804B1B48[0x100];
extern char __vt__10CGeneTex3D[0x100];
extern void* g_resourceManager;
extern void CubeResourceStorage_dtor(void*, int);
extern void ResourceManager_ReleaseId(void*, void*);
extern void __dla__FPv(void*);
extern void* __nw__FUl(unsigned int);
extern void __dl__FPv(void*);
extern void appFreeUni__FPv(void*);

enum PackedResourceConstants {
    kPackedResourceCountShift = 16,
    kPackedResourceMetadataMask = 0xFFFF
};

struct GroundStripStorage {
    void* vtable;
    void* stripRows;
    int stripRowCount;
    unsigned int stripRowCapacity;
    void* bitmap;
    float width;
    unsigned char pad18[0x14];
    void* stripParameters;
    int stripCount;
    int stripCapacity;
    void** vertexBuffers;
    int vertexBufferCount;
    int vertexBufferCapacity;
    unsigned char enabled;
    unsigned char pad45[3];
    void* textureGeneratorVtable;
    void* generatedTextureData;
};

// Initialize empty strip storage and the embedded texture generator.
void GroundStripOwner_Init(GroundStripStorage* self) {
    int zero = 0;
    self->stripRows = 0;
    self->vtable = &s_waveletVtable_804B1B48;
    self->stripRowCount = 0;
    self->stripRowCapacity = 0;
    self->bitmap = 0;
    self->width = kFloat_16_GeneTex3D;
    self->stripParameters = 0;
    self->stripCount = 0;
    self->stripCapacity = 0;
    self->vertexBuffers = 0;
    self->vertexBufferCount = 0;
    self->vertexBufferCapacity = zero;
    self->enabled = 0;
    self->textureGeneratorVtable = &__vt__10CGeneTex3D;
    self->generatedTextureData = 0;
}

// Release referenced buffers, the bitmap, texture data and all owned strip arrays.
CWavelet::~CWavelet() {
    GroundStripStorage* self = (GroundStripStorage*)this;
    unsigned int zero = 0;
    self->vtable = &s_waveletVtable_804B1B48;
    self->stripRowCount = zero;
    self->stripCount = zero;
    for (unsigned int i = zero; i < (unsigned int)self->vertexBufferCount; ++i) {
        void* item = self->vertexBuffers[i];
        if (item) {
            unsigned int* ref = (unsigned int*)item;
            unsigned int value = *ref;
            value = (value & kPackedResourceMetadataMask) |
                    (((value >> kPackedResourceCountShift) - 1) << kPackedResourceCountShift);
            *ref = value;
            if ((value >> kPackedResourceCountShift) == zero)
                CubeResourceStorage_dtor(item, 1);
            self->vertexBuffers[i] = (void*)zero;
        }
    }
    self->vertexBufferCount = zero;
    if (g_resourceManager && self->bitmap) {
        ResourceManager_ReleaseId(g_resourceManager, self->bitmap);
        self->bitmap = (void*)zero;
    }
    self->textureGeneratorVtable = &__vt__10CGeneTex3D;
    if (self->generatedTextureData) {
        __dla__FPv(self->generatedTextureData);
        self->generatedTextureData = (void*)zero;
    }
    if (self->vertexBuffers) {
        self->vertexBufferCount = zero;
        appFreeUni__FPv(self->vertexBuffers);
    }
    if (self->stripParameters) {
        self->stripCount = zero;
        appFreeUni__FPv(self->stripParameters);
    }
    if (self->stripRows) {
        self->stripRowCount = zero;
        appFreeUni__FPv(self->stripRows);
    }
}

extern char s_tutorialRttiDescriptor;

// Return the tutorial class RTTI descriptor.
void* CTutorial::GetRTTI() {
    return &s_tutorialRttiDescriptor;
}

struct StripPointArrayHeader {
    Entry8020* data;
    int size;
};

// Clear the strip-point array's logical size without releasing its storage.
void GroundStripPointVector_Clear(int* array) {
    // Keep the original range-clear expression: plain assignment changes its load/store sequence.
    ((StripPointArrayHeader*)array)->size -= ((char*)array + ((StripPointArrayHeader*)array)->size) - (char*)array;
}

extern int Vec3Vector_GetPointCount(void*);
extern void* Vec3Vector_GetPointAt(void*, int);
extern void* CQuadTree_GetInstance(void*);
extern void GetGroundPoint__9CQuadTreeF7WVectorbPib(void*, void*, void*, int, int, int);
extern void WVector_DefaultConstructNoOp(void*);
extern void __as__7WVectorFRC7WVector(void*, void*);
extern void __mi__FRC7WVectorRC7WVector(void*, void*, void*);
extern void* Vec3_Set(void*, float, float, float);
extern void Vector_CrossProduct(void*, void*, void*);
extern void* Normalize__7WVectorFv(void*);
extern float WVectorLen__FRC7WVector(void*);
extern void __ml__FfRC7WVector(void*, void*, float);
extern void __pl__FRC7WVectorRC7WVector(void*, void*, void*);
extern void VertexBuffer_Create(int, int, int, int, void*, int);
extern void Resource_GetOffsetPtr(void*, int, int, void*, int);
extern void GraphicsBuffer_Flush(void*);
extern void GroundStripBufferVector_Append(void*, void*);
extern int SharedDoc_GetCourseId();
extern void* GolfDoc_GetCurrent();
extern const float kFloat_0_805DFDA4, kFloat_1_SceneEffects_805DFDC0, kFloat_5_SceneEffects;
extern const float kFloat_0_5_Wavelet;
extern void GroundStripPointVector_Append(void*, void*);

typedef Vec3f Vec3_80210;
struct GroundStripVertex {
    Vec3f position;
    unsigned char red, green, blue, alpha;
    float u, v;
};
typedef char StripVertex_size_check[sizeof(GroundStripVertex) == 0x18 ? 1 : -1];
enum StripGeometryConstants {
    kBytesPerStripPoint = 2 * sizeof(GroundStripVertex),
    kStripVertexFormat = (0x12 << 16) - 0xFFF,
    kOpaqueColorChannel = 0xFF,
    kCourseId0 = 0,
    kCourseId1 = 1,
    kCourseId6 = 6, // Numeric IDs: course names unverified.
    kHole3 = 2,
    kHole9 = 8,
    kHole11 = 10,
    kHole18 = 17
};
struct StripPointRecord {
    unsigned int words[9];
};
struct Container80211018 {
    StripPointRecord* data;
    unsigned int size, capacity, spare;
};
// Grow the point-record array, preserving its elements and freeing the previous allocation.
void GroundStripPointVector_Reserve(Container80211018*, unsigned int);
struct Node80210 {
    unsigned char pad[0x30];
};
struct ObjList80210 {
    Node80210* data;
    unsigned int size, capacity, spare;
};
struct Target80210 {
    virtual void slot08();
    virtual void slot0C();
    virtual void slot10();
    virtual void slot14(int);
    virtual void slot18(int);
};

#pragma dont_inline on
#pragma opt_common_subs off
#pragma opt_loop_invariants off
// Build paired strip vertices, repeating UVs, and the per-hole row-height tolerance.
// pathPoints is borrowed and must contain at least two points; width must be nonzero.
// parameterNameAddress is forwarded to virtual slot 0x14; bufferCount to slot 0x18 and the
// allocation loop. All centers use terrain Y sampled at path point 1, even on slopes.
// Each buffer gets UV/RGB setup here; XYZ, alpha, and the first center's RGB are
// not written here. Later geometry updates must populate them before rendering.
void GroundStrip_BuildGeometry(void* strip, void* pathPoints, int parameterNameAddress, int bufferCount) {
    struct StripRow {
        Vec3_80210 center, outerEdge, widthDirection;
    } row;
    Vec3_80210 temporaryPoint, groundPoint;
    void* vertexBuffer;
    char* volatile vertexStream;
    // Phase 1: configure strip parameters and reserve the owned row array.
    ((Target80210*)strip)->slot14(parameterNameAddress);
    WVector_DefaultConstructNoOp(&temporaryPoint);
    GroundStripPointVector_Clear((int*)&((GroundStripStorage*)strip)->stripRows);
    GroundStripPointVector_Reserve((Container80211018*)&((GroundStripStorage*)strip)->stripRows,
                                   (unsigned int)Vec3Vector_GetPointCount(pathPoints));
    struct Words {
        unsigned int xBits, yBits, zBits;
    };
    void* src = Vec3Vector_GetPointAt(pathPoints, 1);
    Words params = *(Words*)src;
    Vec3_80210 direction, perpendicular, up, outerPoint, widthOffset, segment;
    GetGroundPoint__9CQuadTreeF7WVectorbPib(&groundPoint, CQuadTree_GetInstance(src), &params, 1, 0, 1);
    float groundHeightOrOuterV = groundPoint.y;
    float zeroHeight = kFloat_0_805DFDA4;
    void* previousCenter;
    char* vertex;
    int vertexByteOffset;
    int bufferIndex;
    unsigned int pointIndex;
    unsigned int rowIndex;
    void* currentPoint;
    // Phase 2: flatten centers to sampled Y and offset each edge by width.
    // Ground height is sampled once, not per vertex; width is a world-space distance.
    // The final row copies its predecessor's width direction.
    for (rowIndex = 0; rowIndex < (unsigned int)Vec3Vector_GetPointCount(pathPoints); ++rowIndex) {
        GroundStripPoint_DefaultConstructNoOp(&row);
        *(WVector*)(&row.center) = *(WVector*)(Vec3Vector_GetPointAt(pathPoints, rowIndex));
        row.center.y = groundHeightOrOuterV;
        if (rowIndex == (unsigned int)Vec3Vector_GetPointCount(pathPoints) - 1) {
            *(WVector*)(&row.widthDirection) =
                *(WVector*)&EntryArray_GetEntry((Entry8020**)&((GroundStripStorage*)strip)->stripRows, rowIndex - 1)
                     ->widthDirection;
        } else {
            currentPoint = Vec3Vector_GetPointAt(pathPoints, rowIndex);
            __mi__FRC7WVectorRC7WVector(&direction, Vec3Vector_GetPointAt(pathPoints, rowIndex + 1), currentPoint);
            *(WVector*)(&row.widthDirection) = *(WVector*)&direction;
            row.widthDirection.y = zeroHeight;
            Vector_CrossProduct(&perpendicular, Vec3_Set(&up, zeroHeight, kFloat_1_SceneEffects_805DFDC0, zeroHeight),
                                &row.widthDirection);
            *(WVector*)(&row.widthDirection) = (*(WVector*)&perpendicular).Normalize();
        }
        __ml__FfRC7WVector(&widthOffset, &row.widthDirection, ((GroundStripStorage*)strip)->width);
        __pl__FRC7WVectorRC7WVector(&outerPoint, &row.center, &widthOffset);
        *(WVector*)(&row.outerEdge) = *(WVector*)&outerPoint;
        GroundStripPointVector_Append(&((GroundStripStorage*)strip)->stripRows, &row); // original +0x4
    }
    // Phase 3: allocate referenced graphics buffers. Each path point contributes
    // center then outer-edge vertices; U accumulates distance/width and V is 0/1.
    ((Target80210*)strip)->slot18(bufferCount);
    // Reuse the former ground-height temporary for the outer-edge V coordinate.
    groundHeightOrOuterV = kFloat_1_SceneEffects_805DFDC0;
    float zeroUv = kFloat_0_805DFDA4;
    for (bufferIndex = 0; bufferIndex < bufferCount; ++bufferIndex) {
        VertexBuffer_Create(Vec3Vector_GetPointCount(pathPoints) * kBytesPerStripPoint, 0, kStripVertexFormat, 0,
                            &vertexBuffer, 0);
        Resource_GetOffsetPtr(vertexBuffer, 0, 0, (void*)&vertexStream, 0);
        ((GroundStripVertex*)vertexStream)->u = zeroUv;
        ((GroundStripVertex*)vertexStream)->v = zeroUv;
        pointIndex = 1;
        vertexByteOffset = kBytesPerStripPoint;
        for (; pointIndex < (unsigned int)Vec3Vector_GetPointCount(pathPoints);
             ++pointIndex, vertexByteOffset += kBytesPerStripPoint) {
            vertex = vertexStream + vertexByteOffset;
            previousCenter = EntryArray_GetEntry((Entry8020**)&((GroundStripStorage*)strip)->stripRows, pointIndex - 1);
            __mi__FRC7WVectorRC7WVector(
                &segment, EntryArray_GetEntry((Entry8020**)&((GroundStripStorage*)strip)->stripRows, pointIndex),
                previousCenter);
            ((GroundStripVertex*)vertex)->u = ((GroundStripVertex*)vertex - 2)->u +
                                              WVectorLen(*(WVector*)&segment) / ((GroundStripStorage*)strip)->width;
            ((GroundStripVertex*)(vertexStream + vertexByteOffset))->v = zeroUv;
            ((GroundStripVertex*)(vertexStream + vertexByteOffset))->blue = kOpaqueColorChannel;
            ((GroundStripVertex*)(vertexStream + vertexByteOffset))->green = kOpaqueColorChannel;
            ((GroundStripVertex*)(vertexStream + vertexByteOffset))->red = kOpaqueColorChannel;
        }
        pointIndex = 0;
        vertexByteOffset = sizeof(GroundStripVertex);
        for (; pointIndex < (unsigned int)Vec3Vector_GetPointCount(pathPoints);
             ++pointIndex, vertexByteOffset += kBytesPerStripPoint) {
            vertex = vertexStream + vertexByteOffset;
            ((GroundStripVertex*)vertex)->u = ((GroundStripVertex*)vertex - 1)->u;
            ((GroundStripVertex*)(vertexStream + vertexByteOffset))->v = groundHeightOrOuterV;
            ((GroundStripVertex*)(vertexStream + vertexByteOffset))->blue = kOpaqueColorChannel;
            ((GroundStripVertex*)(vertexStream + vertexByteOffset))->green = kOpaqueColorChannel;
            ((GroundStripVertex*)(vertexStream + vertexByteOffset))->red = kOpaqueColorChannel;
        }
        GraphicsBuffer_Flush(vertexBuffer);
        GroundStripBufferVector_Append(&((GroundStripStorage*)strip)->vertexBuffers, &vertexBuffer);
    }
    // Course-specific exceptions use zero-based hole numbers; preserve repeated document queries.
    if (((unsigned char)SharedDoc_GetCourseId() == kCourseId0 &&
         (unsigned char)((CGolfDoc*)GolfDoc_GetCurrent())->GetCurrentHoleNumber() == kHole18) ||
        ((unsigned char)SharedDoc_GetCourseId() == kCourseId1 &&
         (unsigned char)((CGolfDoc*)GolfDoc_GetCurrent())->GetCurrentHoleNumber() == kHole18) ||
        ((unsigned char)SharedDoc_GetCourseId() == kCourseId6 &&
         (unsigned char)((CGolfDoc*)GolfDoc_GetCurrent())->GetCurrentHoleNumber() == kHole9))
        ((CBeach*)strip)->maxRowHeightDelta = kFloat_5_SceneEffects;
    else if (((unsigned char)SharedDoc_GetCourseId() == kCourseId0 &&
              (unsigned char)((CGolfDoc*)GolfDoc_GetCurrent())->GetCurrentHoleNumber() == kHole3) ||
             ((unsigned char)SharedDoc_GetCourseId() == kCourseId1 &&
              (unsigned char)((CGolfDoc*)GolfDoc_GetCurrent())->GetCurrentHoleNumber() == kHole3) ||
             ((unsigned char)SharedDoc_GetCourseId() == kCourseId6 &&
              (unsigned char)((CGolfDoc*)GolfDoc_GetCurrent())->GetCurrentHoleNumber() == kHole11))
        ((CBeach*)strip)->maxRowHeightDelta = kFloat_1_SceneEffects_805DFDC0;
    else
        ((CBeach*)strip)->maxRowHeightDelta = kFloat_0_5_Wavelet;
}
#pragma opt_common_subs reset
#pragma opt_loop_invariants reset
#pragma dont_inline reset

extern char __vt__Q23std11logic_error[0x18];
extern char s_sharedCharDeleterVtable[0x18];
extern char __vt__Q23std12length_error[0x18];
struct TaskSequenceStringPool {
    char vectorLengthError[0x14];
    char lengthErrorType[0x37];
    char integerFormat[3];
    char treeLengthError[0x1A];
};
extern TaskSequenceStringPool taskSequenceStrings;
extern void* __nwa__FUl(unsigned int);
extern void* appMallocDummy__FUl(unsigned int);
extern void __dt__Q23std12length_errorFv(void*);

extern char* strchr(const char*, int);
extern int sscanf(const char*, const char*, ...);
struct BeachParameterRecordView {
    int id;
    char bitmapName[0x20];
    float width;
    float timingScale;
};
struct BeachParameterNode {
    unsigned int links[3];
    BeachParameterRecordView parameter;
};
// Opaque tree-owner storage borrowed from CGolfDoc; no node layout is implied.
struct BeachParameterTreeStorageView {
    unsigned char pad[0x40];
};
extern void* g_sharedDoc;
extern void* BeachParameterTree_FindInsertionPosition(BeachParameterTreeStorageView*, int*, BeachParameterRecordView**,
                                                      unsigned char*, unsigned char*);
extern BeachParameterRecordView* Payload44Tree_InsertAndRebalance(BeachParameterTreeStorageView*, void*, unsigned char,
                                                                  unsigned char, void*);
extern void* ResourceManager_LoadBitmap(void*, void*, int, int, int);
extern const float kFloat_0_080000006_Wavelet, kFloat_0_72_Wavelet, kFloat_0_5_Wavelet, kFloat_0_2_Wavelet,
    kFloat_4_Wavelet;

struct StripParameterWordCopy {
    unsigned int words[6];
};
struct StripParameterLeadingChannels {
    float x, y, z;
    // Initialize the record's leading three scalar channels in source order.
    void initialize(float newX, float newY, float newZ) {
        x = newX;
        y = newY;
        z = newZ;
    }
    // Copy the leading channels without changing their float representation.
    void assign(const StripParameterLeadingChannels& other) {
        x = other.x;
        y = other.y;
        z = other.z;
    }
};
struct StripParameterAllocatorTag {
    volatile unsigned char value;
    // Preserve the original observable allocator-tag initialization.
    StripParameterAllocatorTag() : value(0) {}
};
// Strip defaults are five scalar channels plus a mode, not a geometric XYZW vector.
// Channels 0..3 default to zero; channel 4 defaults to 38.4. Their physical roles
// are not established here. Keep argument order when changing strip defaults.
struct StripParameterRecord {
    float x, y, z, w, scale; // +0,+4,+8,+12,+16: scalar channels 0..4; roles unresolved.
    unsigned int mode;       // +20: first strip 0, appended default 2.
    // Leave fields untouched for placement construction followed by explicit assignment.
    StripParameterRecord() {}
    // Copy all six words as a payload; these records contain no owning pointers.
    StripParameterRecord(const StripParameterRecord& other) {
        *(StripParameterWordCopy*)this = *(const StripParameterWordCopy*)&other;
    }
    // Construct the five scalar channels and mode in their stored argument order.
    StripParameterRecord(float a, float b, float c, float d, float e, unsigned int f)
        : x(a), y(b), z(c), w(d), scale(e), mode(f) {}
    // Write the five scalar channels and mode without allocating storage.
    void initialize(float newX, float newY, float newZ, float newW, float newScale, unsigned int newMode) {
        ((StripParameterLeadingChannels*)this)->initialize(newX, newY, newZ);
        w = newW;
        scale = newScale;
        mode = newMode;
    }
    // Assign each scalar channel and mode from another nonowning record.
    void assign(const StripParameterRecord& other) {
        ((StripParameterLeadingChannels*)this)->assign(*(const StripParameterLeadingChannels*)&other);
        w = other.w;
        scale = other.scale;
        mode = other.mode;
    }
};
extern const float kFloat_38_4_ShotDirection;
struct StripParameterVectorView {
    unsigned char pad[0x2C];
    StripParameterRecord* data;
    unsigned int size, capacity;
};
extern // Reserve strip-parameter storage and append defaults, marking the first strip as the start.

    struct StripExceptionMessageGuard {
    void* data;
    StripParameterAllocatorTag* allocator;
    // Guard a new exception-message allocation while constructing its shared owner.
    StripExceptionMessageGuard(void* p, StripParameterAllocatorTag* a) : data(p), allocator(a) {}
    // Free the message unless successful construction transferred ownership.
    ~StripExceptionMessageGuard() {
        if (data)
            __dla__FPv(data);
    }
};
struct StripExceptionMessageOwner {
    void* vtable;
    unsigned int strong, weak;
    void* data;
    // Retain a newly allocated message with one strong/weak reference.
    StripExceptionMessageOwner(void* p) {
        strong = 1;
        weak = 1;
        vtable = s_sharedCharDeleterVtable;
        data = p;
    }
};
extern "C" void __dt__Q23std9exceptionFv(void*, int);
} // extern "C"

namespace std {
struct exception {
    void* vtable;
    // Install the reconstructed exception-base dispatch table.
    exception() {
        vtable = __vt__Q23std11logic_error;
    }
    ~exception();
};
}
struct StripExceptionMessage {
    char* text;
    StripExceptionMessageOwner* shared;
    // Copy borrowed exception text into a new shared allocation with unwind cleanup.
    StripExceptionMessage(const char* const& message) {
        StripParameterAllocatorTag allocator;
        unsigned int n = strlen(message) + 1;
        char* allocation = (char*)__nwa__FUl(n * 2 - n);
        text = allocation;
        {
            StripExceptionMessageGuard buffer(allocation, &allocator);
            shared = new StripExceptionMessageOwner(allocation);
            buffer.data = 0;
        }
        strcpy(text, message);
    }
};
namespace std {
struct logic_error : exception {
    StripExceptionMessage message;
    // Construct the shared message as part of the exception base.
    logic_error(const char* const& text) : message(text) {}

    ~logic_error();
};
struct length_error : logic_error {
    // Build the logic-error message and select length_error dispatch.
    length_error(const char* const& message) : logic_error(message) {
        vtable = __vt__Q23std12length_error;
    }
    ~length_error();
};
}

extern "C" {
// Throw the vector length error with the original shared-string ownership guards.
inline void lengthError8021011C() {
    StripParameterAllocatorTag allocator;
    throw std::length_error(taskSequenceStrings.vectorLengthError);
}
enum StripParameterLimits {
    kStripParameterMaxCount = 0x0AAAAAAA,
    kStripThreeFifthsGrowthLimit = 0x38E38E3,
    kStripHalfGrowthLimit = 0x71C71C6,
    kMaxStripPointCount = 0x71C71C7,
    kFirstStripMode = 0,
    kDefaultStripMode = 2
};

struct StripParameterReserveGuard {
    StripParameterRecord* data;
    unsigned int size, capacity;
    // Start an empty guard for replacement record storage.
    StripParameterReserveGuard() : data(0), size(0), capacity(0) {}
    // Allocate requested record capacity after the element-byte overflow check.
    void allocate(unsigned int requested) {
        StripParameterAllocatorTag allocator;
        if (requested > kStripParameterMaxCount)
            lengthError8021011C(); // UINT_MAX / sizeof(StripParameterRecord).
        data = (StripParameterRecord*)appMallocDummy__FUl(requested * sizeof(StripParameterRecord));
    }
    // Release whichever record allocation remains with the guard after the swap.
    ~StripParameterReserveGuard() {
        if (data) {
            StripParameterAllocatorTag allocator;
            // Empty-element range destruction still contributes pointer arithmetic to this ABI.
            size -= ((char*)data + size) - (char*)data;
            appFreeUni__FPv(data);
        }
    }
};
struct StripParameterInsertionGuard {
    StripParameterRecord* data;
    unsigned int size, capacity;
    unsigned int* allocator;
    unsigned int remaining;
    // Start an empty insertion guard borrowing the vector's allocator word.
    StripParameterInsertionGuard(unsigned int* a) : data(0), size(0), capacity(0), allocator(a), remaining(0) {}
    // Release moved-out old storage or an abandoned replacement allocation.
    ~StripParameterInsertionGuard() {
        if (data) {
            StripParameterAllocatorTag allocator;
            // Empty-element range destruction still contributes pointer arithmetic to this ABI.
            size -= ((char*)data + size) - (char*)data;
            appFreeUni__FPv(data);
        }
    }
};

// Exchange vector storage while preserving the reserve path's volatile size reload.
inline void swapStorage8021011C(StripParameterRecord*& currentData, unsigned int& as, unsigned int& ac,
                                StripParameterRecord*& replacementData, unsigned int& bs, unsigned int& bc,
                                bool reload = false) {
    unsigned int oldCap;
    unsigned int oldSize;
    StripParameterRecord* replacement;
    StripParameterRecord* old = currentData;
    replacement = replacementData;
    oldCap = ac;
    unsigned int cap = bc;
    oldSize = reload ? *(volatile unsigned int*)&as : as;
    unsigned int size = bs;
    replacementData = old;
    StripParameterAllocatorTag allocator;
    ac = cap;
    bc = oldCap;
    currentData = replacement;
    as = size;
    bs = oldSize;
}

// Select the larger growth request without extending either argument's lifetime.
inline const unsigned int& max8021011C(const unsigned int& a, const unsigned int& b) {
    return a < b ? b : a;
}

// Choose the original capacity growth regime without overflowing element storage.
inline unsigned int capacity8021011C(const unsigned int& oldCap, const unsigned int& need) {
    if (oldCap < kStripThreeFifthsGrowthLimit) {
        unsigned int growth = (oldCap + 1) * 3 / 5;
        return oldCap + max8021011C(growth, need);
    }
    if (oldCap < kStripHalfGrowthLimit) {
        unsigned int half = (oldCap + 1) >> 1;
        return oldCap + max8021011C(half, need);
    }
    return kStripParameterMaxCount;
}

// Reserve capacity, append `requested` default records, then mark element zero as first.
// This is not reserve-only: existing size increases by requested. Element zero
// must exist even when requested == 0; use on an empty vector requires requested > 0.
void CBeach::AppendDefaultStripParameters(unsigned int requested) {
    StripParameterVectorView* self = (StripParameterVectorView*)this;
    unsigned int cap;
    unsigned int oldCap;
    int i;
    // Phase 1: move existing records into storage of at least requested capacity.
    if (requested > self->capacity) {
        StripParameterReserveGuard reserve;
        reserve.allocate(requested);
        StripParameterAllocatorTag copyAllocator;
        StripParameterRecord* src = self->data;
        StripParameterRecord* end = src + self->size;
        reserve.capacity = requested;
        StripParameterRecord* dst = reserve.data + reserve.size;
        while (src < end) {
            new (dst) StripParameterRecord(*src);
            ++src;
            ++dst;
            ++reserve.size;
        }
        *(volatile unsigned int*)&self->size = 0;
        swapStorage8021011C(self->data, self->size, self->capacity, reserve.data, reserve.size, reserve.capacity, true);
    }
    // Phase 2: append defaults; insertion may grow again when old size is nonzero.
    // Per-append ownership: construct suffix -> copy old records backward -> swap;
    // insertion guard then releases the retired allocation, without owning pointees.
    for (i = 0; i < (int)requested; ++i) {
        unsigned int need;
        unsigned int currentSize = self->size;
        StripParameterRecord value;
        value.z = kFloat_0_805DFDA4;
        value.scale = kFloat_38_4_ShotDirection;
        value.x = kFloat_0_805DFDA4;
        value.mode = kDefaultStripMode;
        value.y = kFloat_0_805DFDA4;
        value.w = kFloat_0_805DFDA4;
        StripParameterAllocatorTag allocator;
        if (currentSize < self->capacity) {
            ++self->size;
            self->data[self->size - 1].initialize(value.x, value.y, value.z, value.w, value.scale, value.mode);
        } else {
            StripParameterAllocatorTag growthAllocator;
            StripParameterAllocatorTag limitAllocator;
            if (kStripParameterMaxCount - self->capacity < 1u)
                lengthError8021011C();
            need = self->size + 1 - self->capacity;
            StripParameterInsertionGuard insert(&self->capacity);
            oldCap = self->capacity;
            if (need > kStripParameterMaxCount - oldCap)
                lengthError8021011C();
            cap = capacity8021011C(oldCap, need);
            StripParameterAllocatorTag reserveAllocator;
            if (cap > kStripParameterMaxCount)
                lengthError8021011C();
            StripParameterRecord* data = (StripParameterRecord*)appMallocDummy__FUl(cap * sizeof(StripParameterRecord));
            unsigned int newSize;
            StripParameterRecord* rec;
            unsigned int oldSize = self->size;
            StripParameterRecord* dst = data + oldSize;
            newSize = insert.size + 1;
            rec = dst + insert.size;
            rec->assign(value);
            StripParameterRecord* begin = self->data;
            StripParameterRecord* src = begin + self->size;
            insert.data = data;
            insert.capacity = cap;
            insert.remaining = oldSize;
            StripParameterAllocatorTag rangeAllocator;
            insert.size = newSize;
            StripParameterAllocatorTag copyAllocator;
            while (src > begin) {
                --dst;
                --src;
                new (dst) StripParameterRecord(*src);
                --insert.remaining;
                ++insert.size;
            }
            self->size -= self->size;
            swapStorage8021011C(self->data, self->size, self->capacity, insert.data, insert.size, insert.capacity);
        }
    }
    // This marks element zero of the full vector, including pre-existing records.
    self->data->mode = kFirstStripMode;
}

// Resolve the numeric suffix to beach parameters and configure wave texture/timing.
void CWavelet::ConfigureFromBeachParameterName(const char* name) {
    // name is borrowed. Missing '_' or an unparsed suffix leaves current settings
    // untouched; a missing numeric key is inserted with zero-valued parameters.
    // timingScale is multiplied by 0.08/0.72/0.2 for timing segments; no reciprocal is taken.
    char* underscore = strchr(name, '_');
    // Repeated lookup calls preserve the original insertion and allocation behavior.
    // The document's embedded tree stores id -> bitmap name, wave width, and timing scale.
    if (strchr(name, '_')) {
        int parameterId;
        unsigned char insertionFlag;

        if (sscanf(strchr(name, '_') + 1, taskSequenceStrings.integerFormat, &parameterId) == 0)
            return;
        BeachParameterTreeStorageView* parameterTree =
            (BeachParameterTreeStorageView*)&((CGolfDoc*)((CSharedDoc*)g_sharedDoc)->golfDoc)->beachParameterTreeCount;
        // FindInsertionPosition writes a NODE address through this legacy payload-
        // typed output. Until +sizeof(links) is applied below it is not a payload.
        // After each branch, parameter points to the embedded ConfigItem payload.
        BeachParameterRecordView* parameter = 0;

        unsigned char comparisonFlag;
        int lookupId = parameterId;
        void* insertionPosition = BeachParameterTree_FindInsertionPosition(parameterTree, &lookupId, &parameter,
                                                                           &insertionFlag, &comparisonFlag);
        if (parameter == 0 || ((BeachParameterNode*)parameter)->parameter.id < parameterId) {
            volatile unsigned int constructionScratch[10] = {};
            unsigned int defaultRecord[11] = {};

            defaultRecord[0] = parameterId;
            parameter = &((BeachParameterNode*)Payload44Tree_InsertAndRebalance(
                              parameterTree, insertionPosition, insertionFlag, comparisonFlag, defaultRecord))
                             ->parameter;
        } else {
            parameter = &((BeachParameterNode*)parameter)->parameter;
        }
        texHandle = (unsigned int)ResourceManager_LoadBitmap(g_resourceManager, parameter->bitmapName, 0, 0, 0);
        // Re-enter the tree: the previous call may have inserted/allocated a node.
        lookupId = parameterId;
        parameterTree =
            (BeachParameterTreeStorageView*)&((CGolfDoc*)((CSharedDoc*)g_sharedDoc)->golfDoc)->beachParameterTreeCount;
        insertionPosition = BeachParameterTree_FindInsertionPosition(parameterTree, &lookupId, &parameter,
                                                                     &insertionFlag, &comparisonFlag);
        if (parameter == 0 || ((BeachParameterNode*)parameter)->parameter.id < parameterId) {
            volatile unsigned int constructionScratch[10] = {};
            unsigned int defaultRecord[11] = {};
            defaultRecord[0] = parameterId;
            parameter = &((BeachParameterNode*)Payload44Tree_InsertAndRebalance(
                              parameterTree, insertionPosition, insertionFlag, comparisonFlag, defaultRecord))
                             ->parameter;
        } else {
            parameter = &((BeachParameterNode*)parameter)->parameter;
        }
        width = parameter->width;
        parameter = 0;
        // Re-enter the tree: the previous call may have inserted/allocated a node.
        lookupId = parameterId;
        parameterTree =
            (BeachParameterTreeStorageView*)&((CGolfDoc*)((CSharedDoc*)g_sharedDoc)->golfDoc)->beachParameterTreeCount;
        insertionPosition = BeachParameterTree_FindInsertionPosition(parameterTree, &lookupId, &parameter,
                                                                     &insertionFlag, &comparisonFlag);
        if (!parameter || parameterId > ((BeachParameterNode*)parameter)->parameter.id) {
            volatile unsigned int constructionScratch[10] = {};
            unsigned int defaultRecord[11] = {};

            defaultRecord[0] = parameterId;
            parameter = &((BeachParameterNode*)Payload44Tree_InsertAndRebalance(
                              parameterTree, insertionPosition, insertionFlag, comparisonFlag, defaultRecord))
                             ->parameter;
        } else {
            parameter = &((BeachParameterNode*)parameter)->parameter;
        }
        float timingScale = parameter->timingScale;

        this->timingScale = timingScale;

        float fadeIn = kFloat_0_080000006_Wavelet * timingScale;
        fadeInTime = fadeIn;
        __typeof__(timingScale * kFloat_0_72_Wavelet) fadeOut = kFloat_0_72_Wavelet * timingScale;
        fadeOutTime = fadeOut;
        delay = (kFloat_0_2_Wavelet * timingScale);
        deceleration = width / (kFloat_4_Wavelet * fadeIn - (kFloat_0_5_Wavelet * fadeIn) * fadeIn);
    }
}

struct Alloc80211018 {
    void* vtable;
    unsigned int a, b;
    void* data;
};

// Grow the point-record array while preserving existing rows and releasing old storage.
void GroundStripPointVector_Reserve(Container80211018* self, unsigned int requested) {
    if (requested <= self->capacity)
        return;
    if (requested > kMaxStripPointCount) {
        lengthError8021011C();
    }
    StripPointRecord* newData = (StripPointRecord*)appMallocDummy__FUl(requested * sizeof(StripPointRecord));
    StripPointRecord* oldData = self->data;
    unsigned int oldSize = self->size;
    unsigned int i = 0;
    StripPointRecord* src = oldData;
    StripPointRecord* dst = newData;
    StripPointRecord* end = oldData + oldSize;
    while (src < end) {
        if (dst)
            *dst = *src;
        ++src;
        ++dst;
        ++i;
    }
    self->size = 0;
    self->capacity = requested;
    self->data = newData;
    self->size = i;
    if (oldData)
        appFreeUni__FPv(oldData);
}

} // extern "C"

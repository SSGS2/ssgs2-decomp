#include <decomp/static_init.h>
typedef unsigned long new_size_t;
#pragma push
#include "classes/CSharedDoc.h"
#include "classes/GtlRecordDeque.h"
#define AsCSharedDoc(p) ((CSharedDoc*)(p))
#include "game/types.h"
#include "iff/IffStruct.h"
#include "classes/CGeneTex3D.h"
#include <decomp/cw_bitfields.h>

#define AsCGeneTex3D(p) ((CGeneTex3D*)(p))
// Translation unit 0x801C6B14-0x801C8124 (17 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

// Byte positions in the external packed string pool; preserve its original storage and encoding.
// The geometry owns one generated vertex array and a vector of retained layer buffers.
struct CausticBufferStorage {
    unsigned char vertexCount;
    char alignment[3];
    const char* vtable;
    void* vertices;
    void* layerBuffers;
    unsigned int layerBufferCount;
    unsigned int layerBufferCapacity;
};
// Owner view: 0x74 bytes of unrelated geometry state, then the caustic vertex/layer buffer at +0x74.
struct CausticBufferOwner {
    char geometryState[0x74];
    CausticBufferStorage buffer;
};
typedef char CausticBufferOwner_vertices_offset[(int)&((CausticBufferOwner*)0)->buffer.vertices == 0x7C ? 1 : -1];
enum ResourceStringOffsets1 {
    kCausticBufferVectorOffset = ((int)&((CausticBufferOwner*)0)->buffer.layerBuffers),
    kWaterCaustic_lengthErrorType = 0x44, // "!std::exception!!std::logic_error!!std::length_error!!"
    kWaterCaustic_vectorLengthError = 0x30, // "vector length error"
    kWaterCaustic_iceCourseAlias = 5, // "icee"
    kWaterCaustic_courseTextureFormat = 0xa, // "[caustic_%s%02d.gsr"
    kWaterCaustic_fallbackTextureFormat = 0x1e, // "[caustic_%02d.gsr"
};

// Byte positions in the external packed string pool; preserve its original storage and encoding.
enum ResourceStringOffsets2 {
    kCourseModelResource_rankNameFormat = 0x259, // "%s_rank"
    kCourseModelResource_holeNameFormat = 0x261, // "%s_%02d"
    kCourseModelResource_variantSeparator = 0x269, // "_"
    kCourseModelResource_listArchiveSuffix = 0x26b, // ".list.zip"
    kCourseModelResource_rankMarker = 0x1CF, // "rank"
};

extern "C" {

extern float kFloat_6_2831855;
extern float s_geneTexFullTurnRadians;
extern void* g_resourceManager;
extern const char __vt__10CGeneTex3D[];
extern const char courseModelResourceStrings[];
extern void* s_courseModelVariantLabels[];
extern void* g_sharedDoc;
extern const char waterCausticStrings[];
extern unsigned int s_sceneLighting[];
int String_CompareCString(const char*, const char*);
void* String_FormatTemporary(const char*, ...);
int sprintf(char*, const char*, ...);
int strcmp(void*, const char*);
void appFreeUni__FPv(void*);
void __dla__FPv(void*);
void __dl__FPv(void*);
void Vec3_Zero(void*);
void* __nwa__FUl(unsigned int);
void* ResourceManager_LoadBitmap(void*, const char*, int, int, int);
void* String_ReplaceRange(void*, int, unsigned int, const char*, const char*, char*);
void OSInitMutex(void*);
void PSMTX44Identity(void*);
unsigned int strlen(const char*);
void* memset(void*, int, unsigned int);
void* __construct_new_array(void*, void*, void*, unsigned int, unsigned int);
void* appMallocDummy__FUl(unsigned int);
void* memcpy(void*, const void*, unsigned int);
void VertexBuffer_Create(int, int, int, int, void*, int);
void* CausticLayer_Dtor(void*, int);
int CourseModel_LoadPendingFile(void*);
int CourseModel_BuildListArchiveName(void*, void*, int, unsigned int);
unsigned int String_FindSubstring(void*, const char*, int, unsigned int);
void* CPackedFile_Create(void*);
int CPackedFile_PollLoad(void*);
void OSLockMutex(void*);
void OSUnlockMutex(void*);
void* CourseModel_LoadFromFile(void*, const char*);
void CubeResourceStorage_dtor(void*, int);
void __destroy_new_array(void*, void*);

// The point-geometry prefix is bounds data, unlike the generic class header's vtable view.
struct GeneTexPointInput { void* points; int count; };
struct GeneTexPointBounds {
    Vec3f minimum;
    Vec3f maximum;
    Vec3f origin;
};
struct Point3New801C {
    float x, y, z;
    // Reserve point storage without initializing its coordinates.
    Point3New801C() {}
};
struct CausticDrawRecord { char prefix[0x10]; unsigned int tint; char suffix[0x24]; };
// Layer arrays are owned; texture is a resource-manager ID released by the layer
// destructor. drawRecordPointers has pointCount entries plus a null terminator.
enum CausticGeometryConstants {
    kCausticLightingSlot = 6, // Packed RGB source combined with caller alpha.
    kCausticDrawRecordBytes = sizeof(CausticDrawRecord), // 0x38, tint at +0x10.
    kCausticVertexBytes = 0x18,
    kCausticVertexFormat = 0x11F001 // Engine format for the 24-byte vertex; bit meanings unresolved.
};
struct CausticLayer {
    float scrollX, scrollY;
    unsigned int tint;
    void* texcoords;
    void* drawRecordPointers;
    void* drawRecords;
    void* texture;
};

struct NarrowStringStorage {
    union { unsigned int flags; unsigned char firstByte; };
    unsigned int length;
    char* data;
};

struct CourseArchiveSelection {
    char prefix[0x9C6];
    unsigned char variantIndex;
    char paddingToCourseName[0x1D];
    char courseName[0x20];
};
struct CourseHoleSelection { char prefix[0x3B7]; unsigned char holeNumber; };
struct CourseModelLoadState {
    char paddingToFilename[0x20];
    char filename[0x4C48];
    char mutex[0x18];
    NarrowStringStorage pendingFilename;
    void* loadedModel;
};
typedef char CourseModelMutexOffset[((int)&((CourseModelLoadState*)0)->mutex == 0x4C68) ? 1 : -1];
typedef char CausticLayerSize[(sizeof(CausticLayer) == 0x1C) ? 1 : -1];

void CausticLayer_InitPointers(void*);
void ResourceManager_ReleaseId(void*, void*);
unsigned int SharedDoc_GetCourseId();
struct Pair801C { unsigned int a, b; };
Pair801C CursorVec2_Scale(void*, float);
void Vec2_AddAssign(void*, void*);

void __ml__FfRC7WVector(void*, void*, float);
void __as__7WVectorFRC7WVector(void*, void*);
void __mi__FRC7WVectorRC7WVector(void*, void*, void*);
void AABB_Copy(void*, void*);
float Math_SinApprox(float);
float Math_CosApprox(float);
void CGeneTex3D_AddCausticLayer(void*, int, float, float, float);
extern float kFloat_1_GeneTex3D;

extern float kFloat_0_7_GeneTex3D, kFloat_0_5_Collision;

extern char __vt__Q23std11logic_error[];
extern char s_sharedCharDeleterVtable[];
extern char __vt__Q23std12length_error[];
void* __nw__FUl(unsigned int size);
void __throw(const char* typeinfo, void* object, void* dtor);
void __dt__Q23std12length_errorFv();
char* strcpy(char* dst, const char* src);

struct Point801C {
    float x, y;
    // Reserve UV storage; AddCausticLayer fills both coordinates.
    Point801C() {}
};

} // extern "C"
// Allocate array storage through the game's array allocator.
inline void* operator new[](new_size_t size) { return __nwa__FUl((unsigned int)size); }
extern "C" {

struct Vec801C {
    void** data;
    unsigned int size;
    unsigned int capacity;
};

struct VecTemp801C {
    void** data;
    unsigned int size;
    unsigned int capacity;
    unsigned int* owner;
    unsigned int offset;
    // Track buffer growth and the constructed range starting at offset.
    VecTemp801C(unsigned int* p) : data(0), size(0), capacity(0), owner(p), offset(0) {}
    // Release the allocation left with this temporary after transfer or failure.
    ~VecTemp801C() {
        if (data) {
            size = 0;
            appFreeUni__FPv(data);
        }
    }
};

struct VecGuard801C {
    char* pointer;
    void* allocator;
    // Guard exception-message bytes until their shared control block exists.
    VecGuard801C(char* p, void* a) : pointer(p), allocator(a) {}
    // Free untransferred message bytes during construction failure.
    ~VecGuard801C() {
        if (pointer) __dla__FPv(pointer);
    }
};

struct VecControl801C {
    void* table;
    int refs;
    int owners;
    char* text;
    // Own the message allocation with one strong and one control-block reference.
    VecControl801C(char* p) {
        refs = 1;
        owners = 1;
        table = s_sharedCharDeleterVtable;
        text = p;
    }
};

struct VecErrorBase801C {
    void* table;
    char* text;
    VecControl801C* ref;
    // Copy the error message and transfer it to a shared-message control block.
    VecErrorBase801C(const char* msg) {
        const char* m = msg;
        table = __vt__Q23std11logic_error;
        struct Alloc { char value; } allocator;
        allocator.value = 0;
        unsigned int n = (unsigned int)strlen(m) + 1;
        char* p = (char*)__nwa__FUl(n * 2 - n);
        text = p;
        {
            VecGuard801C guard(p, &allocator);
            ref = new VecControl801C(p);
            guard.pointer = 0;
        }
        strcpy(text, m);
    }
};

struct VecError801C : VecErrorBase801C {
    // Finish the vector length_error interface after constructing its message.
    VecError801C(const char* msg) : VecErrorBase801C(msg) { table = __vt__Q23std12length_error; }
};

// Throw a length_error with shared-message ownership and the original runtime ABI.
inline void VecFail801C(const char* msg) {
    VecError801C error(msg);
    VecError801C* ep = &error;
    __throw(waterCausticStrings + kWaterCaustic_lengthErrorType, ep, (void*)__dt__Q23std12length_errorFv);
}

// Reject growth beyond the maximum count of four-byte vector entries.
inline void VecCheckLen801C(unsigned int need, unsigned int cap) {
    if (need > 0x3FFFFFFFu - cap) VecFail801C(waterCausticStrings + kWaterCaustic_vectorLengthError);
}

// Select the greater growth increment without copying the argument slot.
inline const unsigned int& VecMax801C(const unsigned int& a, const unsigned int& b) {
    return a < b ? b : a;
}

// Publish the new vector; leave old allocation in the temporary for destruction.
inline void VecSwap801C(Vec801C* v, VecTemp801C& t) {
    void** p = v->data;
    void** q = t.data;
    t.data = p;
    v->size = 0;
    unsigned int c = v->capacity;
    v->capacity = t.capacity;
    t.capacity = c;
    v->data = q;
    v->size = t.size;
    t.size = 0;
}

// Allocate a scrolling caustic texture layer and per-point draw records, then append its vertex buffer.
void CGeneTex3D_AddCausticLayer(void* self, int alpha, float scrollX, float scrollY, float textureScale);

// Test the inline-or-heap pending course-model filename for emptiness.
int CourseModel_IsPendingFilenameEmpty(void* self);

// Replace the pending course filename, initialize its mutex and load the corresponding model.
int CourseModel_SetFileAndLoad(void* self, const char* text);

// Layout used by the vector destructors dtor_801C73B0 and dtor_801C709C: {data pointer, element count}.
struct DataCountView { void* data; int count; };

// Destructor of a small owner whose buffer pointer is at +4: free the buffer, then optionally delete the owner.
void* dtor_801C703C(void* self, int deleting);

// Destructor of a {data, count} vector: zero the count, free the data, then optionally delete the vector.
// (The five identical null tests below are the compiler-matching expansion of nested member destructors;
// in plain C this is: if (self) { if (data) { count = 0; free(data); } if (deleting > 0) delete self; }.)
void* dtor_801C73B0(void* self, int deleting);


// Copy surface points, compute bounds and relative points, and create three scrolling caustic layers.
void CGeneTex3D_InitializePointGeometry(void* self, void* pointInput, int renderMode);

// Clear a vector's size and free its data, then optionally delete the vector.
void* dtor_801C709C(void* self, int deleting);

// Initialize this geometry unit's full-turn angle constant.
void GeneTexPointGeometry_InitFullTurnConstant();

// Initialize point geometry, three empty caustic layers, buffer storage and the texture transform.
void* CGeneTex3D_Ctor(void* self);

// Initialize the four owned pointers in a caustic layer to null.
void CausticLayer_InitPointers(void* self);

// Free caustic coordinate/draw arrays, release its texture and optionally delete the layer.
void* CausticLayer_Dtor(void* self, int deleting);

struct DequeElement28 {
    virtual ~DequeElement28();
    char data[28];
};

// Plain C: destroy every live element of a deque (data, size, capacity, allocator, firstIndex) of 28-byte
// polymorphic elements and free its storage; the second element loop runs on an already-cleared size and is a duplicate from inlining.
// Destroy the temporary deque's live elements and storage, then optionally delete it.
void* GtlRecordDeque_dtor(void* self, int deleting);

// Advance each layer's texture coordinates by scroll velocity, time step and course-specific scale.
void CGeneTex3D_AdvanceLayerTexcoords(void* self, float deltaSeconds);

// Compare the model's current filename with the requested filename.
int CourseModel_MatchesFilename(void* self, const char* name);

// Build the localized-rank or hole-specific list archive name and append the selected variant.
int CourseModel_BuildListArchiveName(void* self, void* string, int useRankVariant, unsigned int unused);

struct Handle801C {
    virtual ~Handle801C();
};

// Load the companion list archive and pending model under the model mutex, then clear the pending filename.
int CourseModel_LoadPendingFile(void* self);

struct Ref7430 { CW_BITS2( unsigned int count : 16, unsigned int flags : 16) };
struct Vector7430 {
    Ref7430** data;
    int size;
    int capacity;
    // Clear the element count while retaining handle storage (byte arithmetic is ABI-shaped).
    void clear() {
        char* end = (char*)data + size;
        char* start = (char*)data;
        size -= end - start;
    }
};
struct Self7430 {
    char pad[0x80];
    Vector7430 vec;
};
// Release the existing layer-buffer reference, replacing its slot only if nonnull.
inline void Release7430(Self7430* self, int index, Ref7430* replacement) {
    Ref7430* ref = self->vec.data[index];
    if (ref) {
        if (--ref->count == 0) CubeResourceStorage_dtor(ref, 1);
        self->vec.data[index] = replacement;
    }
}

#pragma use_lmw_stmw on
// Release layer textures, draw arrays, point arrays and reference-counted vertex buffers.
void* GeneTexPointGeometry_Dtor(void* self, int deleting);
#pragma use_lmw_stmw reset

} // extern "C"

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU8046218C", _ctors_u801C6B14_0, GeneTexPointGeometry_InitFullTurnConstant);
#pragma pop

#pragma pop

// Definitions follow the original text-address order.
extern "C" {

#pragma push
// Build an optional archive, wait under the filename mutex, load the model and clear the pending name.
int CourseModel_LoadPendingFile(void* self) {
    volatile void* _vs = self;
    (void)&_vs;
    // Phase 1: build the optional archive name and start its asynchronous load.
    unsigned int local[3] = {0, 0, 0};
    struct StringCleanup {
        unsigned int* words;
        // Release this temporary archive name only if it owns heap text.
        ~StringCleanup() {
            if (words[0] >> 31) appFreeUni__FPv((void*)words[2]);
        }
    } cleanup = {local};
    const char* name = courseModelResourceStrings + kCourseModelResource_rankMarker;
    void* object = 0;
    unsigned int length = strlen(name);
    unsigned int result = String_FindSubstring((char*)&((CourseModelLoadState*)_vs)->pendingFilename, name, 0, length);
    int found = CourseModel_BuildListArchiveName(self, local, result != -1, result);
    if (found) {
        object = CPackedFile_Create((local[0] >> 31) == 0 ? (void*)((char*)local + 1) : (void*)local[2]);
    }
    // Phase 2: hold the filename/model mutex through polling, model load and filename reset.
    OSLockMutex((char*)&((CourseModelLoadState*)self)->mutex);
    while (object && !CPackedFile_PollLoad(object)) {}
    // The high bit chooses heap storage; otherwise characters start after the flag byte.
    const char* text = (*(unsigned int*)((char*)&((CourseModelLoadState*)self)->pendingFilename) >> 31) == 0
        ? (const char*)(((char*)&((CourseModelLoadState*)self)->pendingFilename + 1)) : *(const char**)((char*)&((CourseModelLoadState*)self)->pendingFilename.data);
    *(void**)((char*)&((CourseModelLoadState*)self)->loadedModel) = CourseModel_LoadFromFile(self, text);
    // Phase 3: release the packed-file handle, then clear text while retaining string capacity.
    if (object) delete (Handle801C*)object;
    if ((*(unsigned int*)((char*)&((CourseModelLoadState*)self)->pendingFilename) >> 31) == 0) {
        unsigned int flagByte = *(unsigned char*)((char*)&((CourseModelLoadState*)self)->pendingFilename);
        *(unsigned char*)(((char*)&((CourseModelLoadState*)self)->pendingFilename + 1)) = 0;
        *(unsigned char*)((char*)&((CourseModelLoadState*)self)->pendingFilename) = (flagByte >> 7) << 7;
    } else {
        char* p = *(char**)((char*)&((CourseModelLoadState*)self)->pendingFilename.data);
        *p = 0;
        *(unsigned int*)((char*)&((CourseModelLoadState*)self)->pendingFilename.length) = 0;
    }
    OSUnlockMutex((char*)&((CourseModelLoadState*)_vs)->mutex);
    return 0;
}
#pragma pop

// Literal-only constants avoid extra MWCC constant symbols and preserve relocation indices.
#define kBoundsSeedMagnitude (10000.0f)
#define kPointLowering (1.0f)
#define kDefaultScrollRadius (0.02f)
#define kBaseTextureScale (1.3f)
#define kCourse13TextureMultiplier (0.25f)
#define kScrollAngleRadians (0.52359879f)
#define kFirstLayerScale (200.0f)
#define kSecondLayerScale (100.0f)
#define kThirdLayerScale (150.0f)

enum CausticLayerPolicy {
    kCausticLayerCapacity = 3, kCausticFullAlpha = 255,
    kCausticSpeedCourse12 = 12, kCausticScaleCourse13 = 13, kCausticSpecialRenderMode = 2
};

// Borrow a layer by its ABI byte stride; the geometry owns the layer array.
static inline CausticLayer* CausticLayerAt(void* self, unsigned int layerByteOffset) {
    return (CausticLayer*)((*(char**)&AsCGeneTex3D(self)->causticLayers) + layerByteOffset);
}

// Borrow that layer's UV array; offsets within it count bytes, two floats per point.
static inline char* CausticTexcoordsAt(void* self, unsigned int layerByteOffset) {
    return (char*)((CausticLayer*)(layerByteOffset + (*(unsigned int*)&AsCGeneTex3D(self)->causticLayers)))->texcoords;
}

// Decode length in the 12-byte MW string: high bit selects heap; inline length is seven bits.
static inline unsigned int CourseFilenameLength(void* storage, unsigned int flags) {
    if ((flags >> 31) == 0) return ((NarrowStringStorage*)storage)->firstByte & 0x7F;
    return ((NarrowStringStorage*)storage)->length;
}

// Assign a decoded length into caller scratch, preserving its lifetime across append operations.
static inline void ReadCourseFilenameLength(void* storage, unsigned int flags, unsigned int& length) {
    if ((flags >> 31) == 0) length = ((NarrowStringStorage*)storage)->firstByte & 0x7F;
    else length = ((NarrowStringStorage*)storage)->length;
}

#pragma push
// Replace the pending filename, initialize load state and synchronously run the pending-file loader.
int CourseModel_SetFileAndLoad(void* self, const char* text) {
    unsigned int flag = *(unsigned int*)((char*)&((CourseModelLoadState*)self)->pendingFilename);
    unsigned int previousFilenameLength;
    previousFilenameLength = CourseFilenameLength(&((CourseModelLoadState*)self)->pendingFilename, flag);
    struct CharSlot { char c; char pad[3]; };
    volatile CharSlot _u0;
    char _e0;
    _e0 = _u0.c;
    unsigned int length = strlen(text);
    volatile void* _vs = self;
    (void)&_vs;
    String_ReplaceRange((char*)&((CourseModelLoadState*)_vs)->pendingFilename, 0, previousFilenameLength, text, text + length, &_e0);
    *(unsigned int*)((char*)&((CourseModelLoadState*)self)->loadedModel) = 0;
    OSInitMutex((char*)&((CourseModelLoadState*)self)->mutex);
    CourseModel_LoadPendingFile(self);
    return 1;
}
#pragma pop

#pragma push
// Return 1 when the inline-or-heap pending filename has zero characters, otherwise 0.
int CourseModel_IsPendingFilenameEmpty(void* self) {
    unsigned int flag = *(unsigned int*)((char*)&((CourseModelLoadState*)self)->pendingFilename);
    unsigned int value;
    value = CourseFilenameLength(&((CourseModelLoadState*)self)->pendingFilename, flag);

    if (value != 0) return 0;
    return 1;
}
#pragma pop

#pragma push
// Compare a borrowed C filename against the currently recorded filename.
int CourseModel_MatchesFilename(void* self, const char* name) {
    return strcmp(((CourseModelLoadState*)self)->filename, name) == 0;
}
#pragma pop

#pragma push
// Replace the caller-owned MW string with a rank/hole archive name plus its selected variant suffix.
int CourseModel_BuildListArchiveName(void* self, void* string, int useRankVariant, unsigned int unused) {
    unsigned int replacedNameLength;
    const char* value;
    int variantIndex;
    const char* suffix;
    unsigned int currentNameLength;
    struct CharSlot { char c; char pad[3]; };
    // Allocator source/copy slots preserve the original empty-allocator ABI and
    // stack lifetimes. They carry no filename data; do not interpret their bytes.
    volatile CharSlot _u0; CharSlot _e0;
    volatile CharSlot _u1; CharSlot _e1;
    volatile CharSlot _u2; CharSlot _e2;
    volatile CharSlot _u3; CharSlot _e3;
    volatile CharSlot _u4; CharSlot _e4;
    // Phase 1: replace the entire old name with either <map>_rank or <course>_<hole>.
    if (useRankVariant) {
        void* item = AsCSharedDoc(g_sharedDoc)->selectedCourseData;
        value = (const char*)String_FormatTemporary(courseModelResourceStrings + kCourseModelResource_rankNameFormat,
            item ? ((IFF_STRUCT::sCourse*)item)->mapName : 0);
        if (!value)
            return 0;
        unsigned int flag = *(unsigned int*)string;
        ReadCourseFilenameLength(string, flag, replacedNameLength);
        _e0.c = _u0.c;
        unsigned int length = strlen(value);
        String_ReplaceRange(string, 0, replacedNameLength, value, value + length, &_e0.c);
    } else {
        char buffer[0x100];
        memset(buffer, 0, sizeof(buffer));
        void* manager = g_sharedDoc;
        void* item = (*(void**)&AsCSharedDoc(manager)->golfDoc);
        sprintf(buffer, courseModelResourceStrings + kCourseModelResource_holeNameFormat,
            ((CourseArchiveSelection*)manager)->courseName, ((CourseHoleSelection*)item)->holeNumber);
        unsigned int flag = *(unsigned int*)string;
        ReadCourseFilenameLength(string, flag, replacedNameLength);
        _e1.c = _u1.c;
        unsigned int length = strlen(buffer);
        String_ReplaceRange(string, 0, replacedNameLength, buffer, buffer + length, &_e1.c);
    }

    // Phase 2: append separator, variant label and archive suffix. Invalid variant returns
    // 0 after replacing the base name; success returns 1 with the complete name.
    void* manager = g_sharedDoc;
    variantIndex = ((CourseArchiveSelection*)manager)->variantIndex;
    struct Labels { const char* values[3]; };
    Labels labels = *(Labels*)s_courseModelVariantLabels;
    if (variantIndex < 0 || variantIndex >= 3)
        return 0;

    suffix = courseModelResourceStrings + kCourseModelResource_variantSeparator;
    unsigned int flag = *(unsigned int*)string;
    ReadCourseFilenameLength(string, flag, currentNameLength);
    _e2.c = _u2.c;
    unsigned int length = strlen(suffix);
    String_ReplaceRange(string, currentNameLength, 0, suffix, suffix + length, &_e2.c);

    const char* variantLabel = labels.values[variantIndex];
    unsigned int flag2 = *(unsigned int*)string;
    unsigned int variantAppendOffset;
    ReadCourseFilenameLength(string, flag2, variantAppendOffset);
    _e3.c = _u3.c;
    length = strlen(variantLabel);
    String_ReplaceRange(string, variantAppendOffset, 0, variantLabel, variantLabel + length, &_e3.c);

    suffix = courseModelResourceStrings + kCourseModelResource_listArchiveSuffix;
    unsigned int flag3 = *(unsigned int*)string;
    unsigned int suffixAppendOffset;
    ReadCourseFilenameLength(string, flag3, suffixAppendOffset);
    _e4.c = _u4.c;
    length = strlen(suffix);
    String_ReplaceRange(string, suffixAppendOffset, 0, suffix, suffix + length, &_e4.c);
    return 1;
}
#pragma pop

#pragma push
// Free the allocation stored at +4 and optionally delete this wrapper; payload type remains unresolved.
void* dtor_801C703C(void* self, int deleting) {
    if (self != 0) {
        void* ptr = *(void**)((char*)self + 4);
        if (ptr != 0)
            appFreeUni__FPv(ptr);
        if (deleting > 0)
            __dl__FPv(self);
    }
    return self;
}
#pragma pop

#pragma push
// Clear a data/count container, free its backing allocation and optionally delete the wrapper.
void* dtor_801C709C(void* self, int deleting) {
    if (self != 0) {
        ((DataCountView*)self)->count = 0;
        if (self != 0) {
            if (((DataCountView*)self)->data != 0) {
                ((DataCountView*)self)->count = 0;
                appFreeUni__FPv(((DataCountView*)self)->data);
            }
        }
        if (deleting > 0)
            __dl__FPv(self);
    }
    return self;
}
#pragma pop

#pragma push
// Destroy deque elements from the end, release backing storage and optionally delete the deque.
void* GtlRecordDeque_dtor(void* self, int deleting) {
    if (0 != self) {
        DequeElement28* end;
        DequeElement28* start = *(DequeElement28**)self + ((GtlRecordDeque*)self)->firstIndex;
        end = start + ((GtlRecordDeque*)self)->size;
        while (start < end) {
            --end;
            end->~DequeElement28();
        }
        ((GtlRecordDeque*)self)->size = 0;
        if (0 != self) {
            int count;
            DequeElement28* ptr = *(DequeElement28**)self;
            if (ptr) {
                count = ((GtlRecordDeque*)self)->size;
                ((GtlRecordDeque*)self)->size = 0;
                while (count != 0) {
                    --ptr;
                    ptr->~DequeElement28();
                    count = count - 1;
                }
                appFreeUni__FPv(*(void**)self);
            }
        }
        if (deleting > 0)
            __dl__FPv(self);
    }
    return self;
}
#pragma pop

#pragma push
// Initialize the generated-point geometry unit's runtime full-circle value in radians.
void GeneTexPointGeometry_InitFullTurnConstant() {
    s_geneTexFullTurnRadians = kFloat_6_2831855;
}
#pragma pop

#pragma push
// Initialize empty point/buffer ownership and construct the three available caustic-layer slots.
void* CGeneTex3D_Ctor(void* self) {
    Vec3_Zero(self);
    Vec3_Zero((char*)&((GeneTexPointBounds*)self)->maximum);
    float f0 = kFloat_1_GeneTex3D;
    (*(unsigned int*)&AsCGeneTex3D(self)->surfacePoints) = 0;
    (*(unsigned int*)&AsCGeneTex3D(self)->relativePoints) = 0;
    ((CausticBufferOwner*)self)->buffer.vertexCount = 0;
    ((CausticBufferOwner*)self)->buffer.vtable = __vt__10CGeneTex3D;
    *(unsigned int*)&((CausticBufferOwner*)self)->buffer.vertices = 0;
    *(unsigned int*)&((CausticBufferOwner*)self)->buffer.layerBuffers = 0;
    ((CausticBufferOwner*)self)->buffer.layerBufferCount = 0;
    ((CausticBufferOwner*)self)->buffer.layerBufferCapacity = 0;
    (*(unsigned int*)&AsCGeneTex3D(self)->pointCount) = 0;
    AsCGeneTex3D(self)->causticLayerCount = 0;
    AsCGeneTex3D(self)->unk_6C = f0;
    CausticLayer* entries = (CausticLayer*)__nwa__FUl(0x64);
    if (entries)
        entries = (CausticLayer*)__construct_new_array(entries, (void*)CausticLayer_InitPointers,
            (void*)CausticLayer_Dtor, sizeof(CausticLayer), kCausticLayerCapacity);
    PSMTX44Identity((char*)&AsCGeneTex3D(self)->pad_08C);
    (*(CausticLayer**)&AsCGeneTex3D(self)->causticLayers) = entries;
    return self;
}
#pragma pop

#pragma push
// Initialize the four owned layer-array/resource slots to null without touching scroll or tint.
void CausticLayer_InitPointers(void* self) {
    (*(unsigned int*)&((CausticLayer*)self)->texcoords) = 0;
    (*(unsigned int*)&((CausticLayer*)self)->drawRecordPointers) = 0;
    (*(unsigned int*)&((CausticLayer*)self)->drawRecords) = 0;
    (*(unsigned int*)&((CausticLayer*)self)->texture) = 0;
}
#pragma pop

#pragma push
// Release layer arrays and the resource-manager texture ID, then optionally delete the layer.
void* CausticLayer_Dtor(void* self, int deleting) {
    if (self != 0) {
        void* ptr = ((CausticLayer*)self)->texcoords;
        if (ptr != 0) {
            if (ptr != 0) {
                __dla__FPv((char*)ptr - 0x10);
            }
            ((CausticLayer*)self)->texcoords = 0;
        }
        ptr = ((CausticLayer*)self)->drawRecordPointers;
        if (ptr != 0) {
            __dla__FPv(ptr);
            ((CausticLayer*)self)->drawRecordPointers = 0;
        }
        ptr = ((CausticLayer*)self)->drawRecords;
        if (ptr != 0) {
            __dla__FPv(ptr);
            ((CausticLayer*)self)->drawRecords = 0;
        }
        void* global = g_resourceManager;
        if (global != 0) {
            ptr = ((CausticLayer*)self)->texture;
            if (ptr != 0) {
                ResourceManager_ReleaseId(global, ptr);
                ((CausticLayer*)self)->texture = 0;
            }
        }
        if (deleting > 0)
            __dl__FPv(self);
    }
    return self;
}
#pragma pop

#pragma push
// Reset a data/count container through volatile size accesses, free its storage and optionally delete it.
void* dtor_801C73B0(void* self, int deleting) {
    if (self != 0) {
        if (self != 0) {
            if (self != 0) {
                if (self != 0) {
                    if (self != 0) {
                        void* ptr = *(void**)self;
                        if (ptr != 0) {
                            volatile int* vp = &((volatile DataCountView*)self)->count;
                            int t = *vp;
                            *vp = t - t;
                            appFreeUni__FPv(ptr);
                        }
                    }
                }
            }
        }
        if (deleting > 0)
            __dl__FPv(self);
    }
    return self;
}
#pragma pop

#pragma push
#pragma use_lmw_stmw on
// Release retained layer resources, owned point arrays and caustic buffers, then optionally delete geometry.
void* GeneTexPointGeometry_Dtor(void* self, int deleting) {
    if (self) {
        // Per caustic layer: release its texture id and clear its layer-buffer reference slot.
        int i = 0;
        int offset = 0;                      // byte offset of layer i in causticLayers (sizeof(CausticLayer) == 0x1C)
        while (i < (*(int*)&AsCGeneTex3D(self)->causticLayerCount)) {
            void* global = g_resourceManager;
            if (global) {
                // +0x18 is CausticLayer::texture
                if (*(void**)((*(char**)&AsCGeneTex3D(self)->causticLayers) + offset + 0x18)) {
                    ResourceManager_ReleaseId(global, *(void**)((*(char**)&AsCGeneTex3D(self)->causticLayers) + offset + 0x18));
                    *(void**)((*(char**)&AsCGeneTex3D(self)->causticLayers) + offset + 0x18) = 0;
                }
            }
            int ptrOffset = 0;
            Release7430((Self7430*)self, i, 0);
            offset += 0x1c;
            ptrOffset = (ptrOffset + 4);
            // i = i + 1 (the temporaries are needed for identical register allocation)
            __typeof__(i + 1) _p139 = 1 + i; __typeof__(1 + i) _p650 = _p139; i = _p650;
        }
        ((Self7430*)self)->vec.clear();
        // Free the owned arrays. Point arrays came from new[], so the allocation starts 0x10 bytes
        // before the pointer (array cookie).
        void* p = AsCGeneTex3D(self)->causticLayers;
        if (p) { __destroy_new_array(p, CausticLayer_Dtor); AsCGeneTex3D(self)->causticLayers = 0; }
        p = AsCGeneTex3D(self)->surfacePoints;
        if (p) { if (p) __dla__FPv((char*)p - 0x10); AsCGeneTex3D(self)->surfacePoints = 0; }
        p = AsCGeneTex3D(self)->relativePoints;
        if (p) { if (p) __dla__FPv((char*)p - 0x10); AsCGeneTex3D(self)->relativePoints = 0; }
        // Destroy the layer-buffer vector member: frees its storage. (The five identical tests are the
        // compiler-matching expansion of the nested member destructor calls; read as one "if (data) free".)
        if ((char*)self + kCausticBufferVectorOffset) { if ((char*)self + kCausticBufferVectorOffset) { if ((char*)self + kCausticBufferVectorOffset) { if ((char*)self + kCausticBufferVectorOffset) { if ((char*)self + kCausticBufferVectorOffset) {
            p = ((Self7430*)self)->vec.data;
            if (p) { ((Self7430*)self)->vec.clear(); appFreeUni__FPv(p); }
        } } } } }
        // Destroy the caustic vertex buffer member: reset its vtable to the base and free the vertex array.
        if (((char*)&((CausticBufferOwner*)self)->buffer)) {
            p = ((CausticBufferOwner*)self)->buffer.vertices;
            ((CausticBufferOwner*)self)->buffer.vtable = __vt__10CGeneTex3D;
            if (p) { __dla__FPv(p); ((CausticBufferOwner*)self)->buffer.vertices = 0; }
        }
        if (deleting > 0) __dl__FPv(self);
    }
    return self;
}
#pragma pop

#pragma push
// Copy and flatten input points, derive lowered bounds and initialize the three caustic scroll layers.
void CGeneTex3D_InitializePointGeometry(void* self, void* pointInput, int renderMode) {
    // Phase 1: allocate owned world-space surface points and origin-relative points.
    // Input is borrowed for this call; coordinates use the same world units as origin.
    Point3New801C* p;
    int count;
    count = ((GeneTexPointInput*)pointInput)->count;
    AsCGeneTex3D(self)->pointCount = count;
    p = new Point3New801C[count];
    (*(Point3New801C**)&AsCGeneTex3D(self)->surfacePoints) = p;
    p = new Point3New801C[AsCGeneTex3D(self)->pointCount];
    (*(Point3New801C**)&AsCGeneTex3D(self)->relativePoints) = p;
    // Phase 2: initialize an empty AABB with +10000 minima and -10000 maxima.
    float bounds[6];
    float lowerBoundSeed[3];
    float lowerSeedOnes[3];
    float upperBoundSeed[3];
    float upperSeedOnes[3];
    upperSeedOnes[0] = 1.0f; upperSeedOnes[1] = 1.0f; upperSeedOnes[2] = 1.0f;
    __ml__FfRC7WVector(upperBoundSeed, upperSeedOnes, -kBoundsSeedMagnitude);
    lowerSeedOnes[0] = 1.0f; lowerSeedOnes[1] = 1.0f; lowerSeedOnes[2] = 1.0f;
    __ml__FfRC7WVector(lowerBoundSeed, lowerSeedOnes, kBoundsSeedMagnitude);
    __as__7WVectorFRC7WVector(bounds, lowerBoundSeed);
    __as__7WVectorFRC7WVector(bounds + 3, upperBoundSeed);
    AABB_Copy(self, bounds);
    unsigned int pointByteOffset = 0;
    unsigned int inputByteOffset = 0;
    // Phase 3: flatten every surface Y to INPUT POINT ZERO, not each point's own Y.
    // Relative points then subtract origin and one vertical world unit.
    for (int i = 0; i < AsCGeneTex3D(self)->pointCount; ++i) {
        __as__7WVectorFRC7WVector((char*)AsCGeneTex3D(self)->surfacePoints + pointByteOffset, (char*)((GeneTexPointInput*)pointInput)->points + inputByteOffset);
        *(float*)((char*)AsCGeneTex3D(self)->surfacePoints + pointByteOffset + 4) =
            *(float*)((char*)((GeneTexPointInput*)pointInput)->points + 4);
        float temp[3];
        __mi__FRC7WVectorRC7WVector(temp, (char*)AsCGeneTex3D(self)->surfacePoints + pointByteOffset, (char*)&((GeneTexPointBounds*)self)->origin);
        __as__7WVectorFRC7WVector((char*)AsCGeneTex3D(self)->relativePoints + pointByteOffset, temp);
        *(float*)((char*)AsCGeneTex3D(self)->relativePoints + pointByteOffset + 4) -= kPointLowering;
        float component, bound;
        bound = ((GeneTexPointBounds*)self)->minimum.x;
        component = *(float*)((char*)AsCGeneTex3D(self)->surfacePoints + pointByteOffset);
        ((GeneTexPointBounds*)self)->minimum.x = (component < bound) ? component : bound;
        bound = ((GeneTexPointBounds*)self)->maximum.x;
        component = *(float*)((char*)AsCGeneTex3D(self)->surfacePoints + pointByteOffset);
        ((GeneTexPointBounds*)self)->maximum.x = (bound < component) ? component : bound;
        bound = ((GeneTexPointBounds*)self)->minimum.y;
        component = *(float*)((char*)AsCGeneTex3D(self)->surfacePoints + pointByteOffset + 4);
        ((GeneTexPointBounds*)self)->minimum.y = (bound > component) ? component : bound;
        bound = ((GeneTexPointBounds*)self)->maximum.y;
        component = *(float*)((char*)AsCGeneTex3D(self)->surfacePoints + pointByteOffset + 4);
        ((GeneTexPointBounds*)self)->maximum.y = (bound < component) ? component : bound;
        bound = ((GeneTexPointBounds*)self)->minimum.z;
        component = *(float*)((char*)AsCGeneTex3D(self)->surfacePoints + pointByteOffset + 8);
        ((GeneTexPointBounds*)self)->minimum.z = (bound > component) ? component : bound;
        bound = ((GeneTexPointBounds*)self)->maximum.z;
        component = *(float*)((char*)AsCGeneTex3D(self)->surfacePoints + pointByteOffset + 8);
        ((GeneTexPointBounds*)self)->maximum.z = (bound < component) ? component : bound;
        pointByteOffset += 0xc;
        inputByteOffset += 0xc;
    }
    // Phase 4: replace the measured Y bounds with [surfaceMaxY - 2, surfaceMaxY - 1].
    // This deliberately describes a lowered one-unit slab, not the copied surface plane.
    AsCGeneTex3D(self)->renderMode = renderMode;
    ((GeneTexPointBounds*)self)->maximum.y = ((GeneTexPointBounds*)self)->maximum.y - kPointLowering;
    ((GeneTexPointBounds*)self)->minimum.y = ((GeneTexPointBounds*)self)->maximum.y - kPointLowering;
    // Phase 5: build three caustic layers, with course 13/mode 2 using quarter scale.
    // Scroll vectors are UV displacement per update time; angle literal is pi/6 radians.
    float scrollRadius = kDefaultScrollRadius;
    float causticScale = kBaseTextureScale;
    if ((unsigned char)SharedDoc_GetCourseId() == kCausticScaleCourse13 && AsCGeneTex3D(self)->renderMode == kCausticSpecialRenderMode) causticScale *= kCourse13TextureMultiplier;
    float y = -scrollRadius * Math_SinApprox(kScrollAngleRadians);
    CGeneTex3D_AddCausticLayer(self, kCausticFullAlpha, -scrollRadius * Math_CosApprox(kScrollAngleRadians), y, kFirstLayerScale * causticScale);
    y = -scrollRadius * Math_SinApprox(kScrollAngleRadians);
    CGeneTex3D_AddCausticLayer(self, kCausticFullAlpha, scrollRadius * Math_CosApprox(kScrollAngleRadians), y, kSecondLayerScale * causticScale);
    CGeneTex3D_AddCausticLayer(self, kCausticFullAlpha, 0.0f, scrollRadius, kThirdLayerScale * causticScale);
}
#pragma pop

#pragma push
// Create one owned layer of UV/draw arrays and retain its loaded caustic texture and buffer handle.
void CGeneTex3D_AddCausticLayer(void* self, int alpha, float scrollX, float scrollY, float textureScale) {
    // Phase 1: claim the next preallocated layer slot and allocate point-sized
    // coordinate/draw arrays. Caller supplies room for the layer and nonzero scale.
    int layerIndex = (*(int*)&AsCGeneTex3D(self)->causticLayerCount);
    CausticLayer* layer = (CausticLayer*)AsCGeneTex3D(self)->causticLayers + layerIndex;
    (*(int*)&AsCGeneTex3D(self)->causticLayerCount) = layerIndex + 1;
    layer->scrollX = scrollX;
    layer->scrollY = scrollY;
    layer->tint = ((unsigned int)alpha << 24) | s_sceneLighting[kCausticLightingSlot];
    Point801C* points = new Point801C[AsCGeneTex3D(self)->pointCount];
    layer->texcoords = points;
    layer->drawRecordPointers = __nwa__FUl((AsCGeneTex3D(self)->pointCount + 1) * 4);
    layer->drawRecords = __nwa__FUl(AsCGeneTex3D(self)->pointCount * kCausticDrawRecordBytes);
    // Phase 2: choose the course texture (variants 1,2,2...), falling back to the
    // generic variant. The ice alias applies only in render mode zero.
    int textureVariant = 2;
    int layerCount = (*(int*)&AsCGeneTex3D(self)->causticLayerCount);
    if (layerCount <= 2) textureVariant = layerCount;
    void* course = AsCSharedDoc(g_sharedDoc)->selectedCourseData;
    const char* courseName = course ? ((IFF_STRUCT::sCourse*)course)->mapName : 0;
    if (String_CompareCString(courseName, waterCausticStrings) == 0 && AsCGeneTex3D(self)->renderMode == 0)
        courseName = waterCausticStrings + kWaterCaustic_iceCourseAlias;
    const char* textureName = (const char*)String_FormatTemporary(waterCausticStrings + kWaterCaustic_courseTextureFormat, courseName, textureVariant);
    void* texture = ResourceManager_LoadBitmap(g_resourceManager, textureName, 0, 0, 0);
    layer->texture = texture;
    if (!texture) {
        textureName = (const char*)String_FormatTemporary(waterCausticStrings + kWaterCaustic_fallbackTextureFormat, textureVariant);
        layer->texture = ResourceManager_LoadBitmap(g_resourceManager, textureName, 0, 0, 0);
    }
    // Phase 3: map surface X/Z relative to the minimum corner into UV coordinates;
    // textureScale is world-space distance per UV unit. Terminate the draw list.
    int i;
    for (i = 0; i < AsCGeneTex3D(self)->pointCount; ++i) {
        ((Point801C*)layer->texcoords)[i].x =
            (((Point3New801C*)AsCGeneTex3D(self)->surfacePoints)[i].x - ((GeneTexPointBounds*)self)->minimum.x) / textureScale;
        ((Point801C*)layer->texcoords)[i].y =
            (((Point3New801C*)AsCGeneTex3D(self)->surfacePoints)[i].z - ((GeneTexPointBounds*)self)->minimum.z) / textureScale;
        ((unsigned int*)layer->drawRecordPointers)[i] =
            (unsigned int)((CausticDrawRecord*)layer->drawRecords + i);
        ((CausticDrawRecord*)layer->drawRecords)[i].tint = layer->tint;
    }
    ((unsigned int*)layer->drawRecordPointers)[i] = 0;
    unsigned int vertexBuffer;
    // Phase 4: create one vertex buffer and append its retained handle. Growth
    // copies handles, clears old slots, then transfers the old allocation to temp.
    VertexBuffer_Create(AsCGeneTex3D(self)->pointCount * kCausticVertexBytes, 0, kCausticVertexFormat, 0, &vertexBuffer, 0);
    Vec801C* buffers = (Vec801C*)(((char*)self + kCausticBufferVectorOffset));
    unsigned int size = buffers->size;
    if (size < buffers->capacity) {
        unsigned int next = 1 + size;
        buffers->size = next;
        buffers->data[next - 1] = (void*)vertexBuffer;
    } else {
        if (0x3FFFFFFFu - buffers->capacity < 1) VecFail801C(waterCausticStrings + kWaterCaustic_vectorLengthError);
        unsigned int growth1;
        unsigned int growth2;
        unsigned int need = buffers->size + 1 - buffers->capacity;
        unsigned int cap = buffers->capacity;
        VecTemp801C temp(&buffers->capacity);
        VecCheckLen801C(need, cap);
        if (cap < 0x15555555u) {
            growth1 = (cap + 1) * 3 / 5;
            cap += VecMax801C(growth1, need);
        } else if (cap < 0x2AAAAAAAu) {
            growth2 = (cap + 1) / 2;
            cap += VecMax801C(growth2, need);
        } else {
            cap = 0x3FFFFFFFu;
        }
        if (cap > 0x3FFFFFFFu) VecFail801C(waterCausticStrings + kWaterCaustic_vectorLengthError);
        void** p = (void**)appMallocDummy__FUl(cap * 4);
        unsigned int oldSize = buffers->size;
        temp.offset = oldSize;
        (p + oldSize)[temp.size] = (void*)vertexBuffer;
        ++temp.size;
        temp.data = p;
        temp.capacity = cap;
        void** old = buffers->data;
        unsigned int count = (unsigned int)(((char*)(old + buffers->size) - (char*)old) / 4);
        temp.offset -= count;
        memcpy(p + temp.offset, old, count * 4);
        memset(old, 0, count * 4);
        temp.size += count;
        VecSwap801C(buffers, temp);
    }
}
#pragma pop

#pragma push
// Advance every point UV by layer scroll multiplied first by seconds, then by course speed.
void CGeneTex3D_AdvanceLayerTexcoords(void* self, float deltaSeconds) {
    float courseSpeedScale = kFloat_1_GeneTex3D;
    if ((unsigned char)SharedDoc_GetCourseId() == kCausticSpeedCourse12)
        courseSpeedScale = kFloat_0_7_GeneTex3D;
    if ((unsigned char)SharedDoc_GetCourseId() == kCausticScaleCourse13 && AsCGeneTex3D(self)->renderMode == kCausticSpecialRenderMode)
        courseSpeedScale = kFloat_0_5_Collision;
    unsigned int layerByteOffset;
    unsigned int texcoordByteOffset;
    int layerIndex;
    layerIndex = 0;
    layerByteOffset = 0;
    while (layerIndex < (*(int*)&AsCGeneTex3D(self)->causticLayerCount)) {
        int pointIndex = 0;
        texcoordByteOffset = 0;
        while (pointIndex < AsCGeneTex3D(self)->pointCount) {

            unsigned int savedTexcoordOffset = texcoordByteOffset;
            float speedScale = courseSpeedScale;
            unsigned int texcoordOffset = savedTexcoordOffset;
            Vec2_AddAssign(
                CausticTexcoordsAt(self, layerByteOffset) + texcoordOffset,
                // Scale by elapsed seconds, then course speed; keep the two return-value temporaries.
                &(CursorVec2_Scale(
                    &(CursorVec2_Scale(CausticLayerAt(self, layerByteOffset), deltaSeconds)),
                    speedScale)));
            texcoordByteOffset = texcoordByteOffset + 8;
            ++pointIndex;
        }
        layerByteOffset += sizeof(CausticLayer);
        ++layerIndex;
    }
}
#pragma pop

}

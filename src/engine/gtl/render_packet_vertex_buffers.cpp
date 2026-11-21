typedef unsigned long new_size_t;
#include "MSL/vector.h"
#include <decomp/cw_bitfields.h>
// Return caller storage for placement construction.
inline void* operator new(new_size_t, void* p){
    return p;
}
// Translation unit 0x80262614-0x80264238 (18 functions; boundaries inferred by decomp-toolkit from extab).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".
extern "C" {
    // Limits/growth tiers depend on element byte size: texcoords=8, positions AND
    // batch triples=12, expanded vertices=24. They are counts, not byte capacities.
    static const unsigned int kTexCoordElementLimit = 0x1fffffffu;
    static const unsigned int kTwelveByteElementLimit = 0x15555555u;
    static const unsigned int kVertexElementLimit = 0x0aaaaaaau;
    static const unsigned int kTexCoordGrowthTier1 = 0x0aaaaaaau;
    static const unsigned int kTexCoordGrowthTier2 = 0x15555554u;
    static const unsigned int kTwelveByteGrowthTier1 = 0x071c71c7u;
    static const unsigned int kTwelveByteGrowthTier2 = 0x0e38e38eu;
    static const unsigned int kVertexGrowthTier1 = 0x038e38e3u;
    static const unsigned int kVertexGrowthTier2 = 0x071c71c6u;
    static const int kCameraViewMatrixOffset = 32, kCameraProjectionMatrixOffset = 96;
    static const int kCoursePacketVertexFormat = 0x11f001;
    static const int kTriangleListPrimitive = 0x90;
    enum PacketTransformSlot { kPacketView = 0, kPacketProjection = 1,
        kPacketTexture0 = 2, kPacketObject = 11 };
    // These are ROWS in the state/value list, not engine state IDs.
    // Data table maps row 15->state 33 (depth scale/offset), 19/20->states 23/24
    // (fog parameters; exact units depend on their consuming render-state callback).
    enum PacketDefaultStateRow { kDepthScaleOffsetDefaultRow = 15,
        kFogParameter23DefaultRow = 19, kFogParameter24DefaultRow = 20 };
    static const int kRenderStateCount = 21, kTextureStateCount = 8, kSamplerStateCount = 5;

    // Empty render-packet lifecycle hook.
    void RenderPacket_NoOpHook() {
    }

    // Position payload: +0/+4/+8 are XYZ component bits (12-byte element).
    // Copies intentionally use integer words; no numeric float conversion occurs.
    struct PositionEntry {
        union { unsigned int positionWords[3]; float position[3]; };
    };

    struct PacketVectorPrefix {
        PositionEntry* entries;
        unsigned int count;
    };

    // Return the vector prefix's stored element count.
    unsigned int CoursePacketVertexVector_GetCount(PacketVectorPrefix* self) {
        return self->count;
    }

    // Return the indexed three-word position entry without a bounds check.
    PositionEntry* CoursePacketPositionVector_GetElement(PacketVectorPrefix* self, int index) {
        return self->entries + index;
    }

    // Texture payload: +0/+4 are the two stored coordinate words (8-byte element).
    // This unit copies/zeroes them; normalization and texture-space scale are external.
    struct TexCoordEntry {
        union { unsigned int texCoordWords[2]; float uv[2]; };
    };

    // CourseRenderPacket_LoadSBINArchive fills +0 XYZ floats, +12 packed color,
    // +16 UV floats, then uploads each 24-byte element without transforming it.
    struct CourseVertexComponents_u80262614 {
        float position[3]; unsigned int packedColor; float uv[2];
    };
    struct CourseVertexEntry {
        union { unsigned int vertexWords[6]; CourseVertexComponents_u80262614 components; };
    };
    // Return the indexed two-word texture-coordinate entry without a bounds check.
    TexCoordEntry* CoursePacketTexCoordVector_GetElement(TexCoordEntry** self, int index) {
        return *self + index;
    }
    // Return the indexed six-word course vertex without a bounds check.
    CourseVertexEntry* CoursePacketVertexVector_GetElement(CourseVertexEntry** self, int index) {
        return *self + index;
    }

    // Vector ABI header: first=allocation address, second=element count,
    // third=capacity in elements. Copying this header does not duplicate ownership.
    struct Triple {
        unsigned int first;
        unsigned int second;
        unsigned int third;
        // Copy a three-word storage header without changing ownership.
        Triple(const Triple& other){
            *this = other;
        }
        // Preserve the three-word header cleanup behavior.
        ~Triple(){
        }
    };

    // Reset the three-word vector prefix to empty.
    void Triple_Clear(Triple* self) {
        self->first = 0;
        self->second = 0;
        self->third = 0;
    }

    // Reset texture-coordinate vector storage, size and capacity.
    void CoursePacketTexCoordVector_Init(Triple* self) {
        self->first = 0;
        self->second = 0;
        self->third = 0;
    }

    // Reset course-vertex vector storage, size and capacity.
    void CoursePacketVertexVector_Init(Triple* self) {
        self->first = 0;
        self->second = 0;
        self->third = 0;
    }
    void gtl_PixelCopy_CopyPackedColor(void* self, void* value);
    // Copy a packed color and return the destination for assignment chaining.
    void* gtl_PixelCopy_AssignPackedColor(void* self, void* value) {
        gtl_PixelCopy_CopyPackedColor(self, value);
        return self;
    }
    void appFreeUni__FPv(void*);
    void __dl__FPv(void*);

    // Out-of-line deleting destructors of the packet vectors (std::vector with the game allocator).
    typedef std::vector<PositionEntry, app::app_allocator<PositionEntry> > PositionVector;
    typedef std::vector<TexCoordEntry, app::app_allocator<TexCoordEntry> > TexCoordVector;
    typedef std::vector<CourseVertexEntry, app::app_allocator<CourseVertexEntry> > CourseVertexVector;
    // Destroy a position vector; free the object too when deleting > 0.
    PositionVector* dtor_80262A30(PositionVector* self, int deleting) {
        return std::__vector_deleting_dtor(self, deleting);
    }

    // Destroy a texture-coordinate vector; free the object too when deleting > 0.
    TexCoordVector* dtor_80263084(TexCoordVector* self, int deleting) {
        return std::__vector_deleting_dtor(self, deleting);
    }

    // Destroy a course-vertex vector; free the object too when deleting > 0.
    CourseVertexVector* dtor_802636C0(CourseVertexVector* self, int deleting) {
        return std::__vector_deleting_dtor(self, deleting);
    }
}
// extern "C"
extern "C" {
#pragma scheduling on
#pragma use_lmw_stmw on

    struct Resource {
        CW_BITS2(
            unsigned int refs : 16,
            unsigned int flags : 16)
    };

    struct DrawEntry {
        Resource* vertexBuffer;
        Resource* texture;
        unsigned int vertexCount;
    };

    struct DrawObject {
        DrawEntry* entries;
        unsigned int size;
        unsigned int capacity;
        char matrix[64];
        char state[104];
        unsigned int unused;
        unsigned int active;
    };
    void CubeResourceStorage_dtor(Resource*, int);
    void PSMTX44Identity(void*);
    void* memset(void*, int, unsigned long);

    // Release each batch's shared vertex/texture resources and reset packet transforms and state.
    void RenderPacket_ReleaseResources(DrawObject* self) {
        for (DrawEntry* it = self->entries; it != self->entries + self->size; ++it) {
            if (it->vertexBuffer) {
                Resource* resource = it->vertexBuffer;
                if (--resource->refs == 0) CubeResourceStorage_dtor(resource, 1);
                it->vertexBuffer = 0;
            }
            if (it->texture) {
                Resource* resource = it->texture;
                if (--resource->refs == 0) CubeResourceStorage_dtor(resource, 1);
                it->texture = 0;
            }
        }
        self->active = 0;
        DrawEntry* old = self->entries;
        self->capacity = 0;
        self->entries = 0;
        self->size = 0;
        if (old) appFreeUni__FPv(old);
        PSMTX44Identity(self->matrix);
        memset(self->state, 0, 104);
    }
}

extern "C" {

    struct StatePair {
        unsigned int key, value;
    };
    extern signed char s_coursePacketDefaultStatesInitialized;
    extern unsigned int kCoursePacketRenderState15Default, s_coursePacketRenderState19Default, kCoursePacketRenderState20Default;
    extern StatePair s_drawObjectRenderStateDefaults[21], kCoursePacketTextureStageStateDefaults[8], kCoursePacketSamplerStateDefaults[5];
    extern char kIdentityMatrix4x4[];
    void Memory_Copy(void*, const void*, unsigned long);
    void Engine_GetRenderState(unsigned int, unsigned int*);
    void Engine_SetCachedRenderState(unsigned int, unsigned int);
    void Engine_GetTextureStageState(int, unsigned int, unsigned int*);
    void Engine_SetTextureStageState(int, unsigned int, unsigned int);
    void Engine_GetRenderStateRowField(int, unsigned int, unsigned int*);
    void Engine_SetTextureStageParameter(int, unsigned int, unsigned int);
    char* Camera_GetRenderState(void*);
    void SetTransform(int, void*);
    void Engine_SetTextureStageResource(unsigned int, void*);
    void Engine_SetTexturePalette(unsigned int, void*);
    void Engine_SetFlexibleVertexFormat(unsigned int);
    void Engine_SetType7Resource(int);
    void Engine_SetVertexStream(int, void*, int, int);
    void DrawPrimitiveFromBoundBuffers(int, int, unsigned int);
}

// Save the render-state defaults, install packet values, and restore them on scope exit.
struct SavedRender {
    StatePair states[kRenderStateCount];
    unsigned int old[kRenderStateCount];
    // Save each render-state value and apply the packet defaults.
    SavedRender() {
        Memory_Copy(states, s_drawObjectRenderStateDefaults, sizeof(StatePair) * kRenderStateCount);
        for (unsigned int i = 0;i < 21;++i) {
            StatePair& state = states[i];
            unsigned int& saved = old[i];
            Engine_GetRenderState(state.key, &saved);
            Engine_SetCachedRenderState(state.key, state.value);
        }
    }
    // Restore the render-state values captured on scope entry.
    ~SavedRender() {
        for (unsigned int i = 0;i < 21;++i) Engine_SetCachedRenderState(states[i].key, old[i]);
    }
};

// Temporarily install the packet texture-stage state for stage zero.
struct SavedTexture {
    StatePair states[kTextureStateCount];
    unsigned int old[kTextureStateCount];
    // Save each texture-stage state and apply the packet defaults.
    SavedTexture() {
        Memory_Copy(states, kCoursePacketTextureStageStateDefaults, sizeof(StatePair) * kTextureStateCount);
        for (unsigned int i = 0;i < 8;++i) {
            StatePair& state = states[i];
            unsigned int& saved = old[i];
            Engine_GetTextureStageState(0, state.key, &saved);
            Engine_SetTextureStageState(0, state.key, state.value);
        }
    }
    // Restore the texture-stage states captured on scope entry.
    ~SavedTexture() {
        for (unsigned int i = 0;i < 8;++i) Engine_SetTextureStageState(0, states[i].key, old[i]);
    }
};

// Temporarily install the packet sampler parameters for stage zero.
struct SavedSampler {
    StatePair states[kSamplerStateCount];
    unsigned int old[kSamplerStateCount];
    // Save each sampler parameter and apply the packet defaults.
    SavedSampler() {
        Memory_Copy(states, kCoursePacketSamplerStateDefaults, sizeof(StatePair) * kSamplerStateCount);
        for (unsigned int i = 0;i < 5;++i) {
            StatePair& state = states[i];
            unsigned int& saved = old[i];
            Engine_GetRenderStateRowField(0, state.key, &saved);
            Engine_SetTextureStageParameter(0, state.key, state.value);
        }
    }
    // Restore the sampler parameters captured on scope entry.
    ~SavedSampler() {
        for (unsigned int i = 0;i < 5;++i) Engine_SetTextureStageParameter(0, states[i].key, old[i]);
    }
};

// Save render state, bind camera transforms and textures, draw each triangle batch, and restore state.
extern "C" void RenderPacket_DrawBatches(DrawObject* self, void* camera) {
    if (self->active && camera) {
        if (!s_coursePacketDefaultStatesInitialized) {
            unsigned int a = kCoursePacketRenderState15Default;
            unsigned int b = s_coursePacketRenderState19Default;
            unsigned int c = kCoursePacketRenderState20Default;
            s_drawObjectRenderStateDefaults[kDepthScaleOffsetDefaultRow].value = a;
            s_drawObjectRenderStateDefaults[kFogParameter23DefaultRow].value = b;
            s_drawObjectRenderStateDefaults[kFogParameter24DefaultRow].value = c;
            s_coursePacketDefaultStatesInitialized = 1;
        }
        SavedRender render;
        SavedTexture texture;
        SavedSampler sampler;
        char* view = Camera_GetRenderState(camera);
        SetTransform(kPacketView, view + kCameraViewMatrixOffset);
        SetTransform(kPacketObject, self->matrix);
        SetTransform(kPacketTexture0, kIdentityMatrix4x4);
        SetTransform(kPacketProjection, view + kCameraProjectionMatrixOffset);
        unsigned int i;
        for (i = 0;i < 4;++i) {
            Engine_SetTextureStageResource(i, 0);
            Engine_SetTexturePalette(i, 0);
        }
        Engine_SetFlexibleVertexFormat(kCoursePacketVertexFormat);
        Engine_SetType7Resource(0);
        i = 0;
        unsigned int offset = 0;
        for (;i < self->size;offset += sizeof(DrawEntry), ++i) {
            DrawEntry* entry = (DrawEntry*)((char*)self->entries+offset);
            Engine_SetTextureStageResource(0, entry->texture);
            Engine_SetTexturePalette(0, 0);
            Engine_SetVertexStream(0, entry->vertexBuffer, 0, sizeof(CourseVertexEntry));
            DrawPrimitiveFromBoundBuffers(kTriangleListPrimitive, 0, entry->vertexCount);
        }
    }
}

extern "C" {
    extern char packedResourcePathStrings[], __vt__Q23std11logic_error[], __vt__Q23std12length_error[], s_sharedCharDeleterVtable[];
    unsigned long strlen(const char*);
    char* strcpy(char*, const char*);
    void* __nwa__FUl(unsigned long);
    void* appMallocDummy__FUl(unsigned long);
    void __dla__FPv(void*);
    void dtor_800460BC(void*, int);
    void __dt__Q23std12length_errorFv(void*, int);
    void __throw(void*, void*, void (*)(void*, int)) __attribute__((noreturn));
}

struct SharedString {
    void* table;
    unsigned int refs, owners;
    char* data;
    // Create a shared character-buffer control block with initial counts.
    SharedString(char* p): refs(1), owners(1) {
        table = s_sharedCharDeleterVtable;
        data = p;
    }
};

struct StringGuard {
    char* ptr;
    char* allocator;
    // Guard the character array until shared ownership transfers.
    StringGuard(char* p, char* a):ptr(p), allocator(a){
    }
    // Delete the character array if it is still guarded.
    ~StringGuard(){
        if (ptr) __dla__FPv(ptr);
    }
};

struct EmptyAllocator {
};
#pragma global_optimizer off

struct PushAllocator {
    char state;
    // Initialize the batch allocator marker.
    PushAllocator():state(0){
    }
    // Preserve the empty batch allocator cleanup hook.
    ~PushAllocator(){
    }
};

struct PushPair {
    unsigned int first, second;
    // Copy two raw batch words.
    PushPair(const PushPair& p){
        unsigned int a = p.first;
        unsigned int b = p.second;
        first = a;
        second = b;
    }
    // Preserve the empty two-word batch cleanup hook.
    ~PushPair(){
    }
};

struct PushTriple {
    PushPair pair;
    unsigned int third;
    // Copy the three-word batch record with its original exception scope.
    PushTriple(const PushTriple& p):pair(p.pair){
        try{
            third = p.third;
        } catch (...){
        }
    }
    // Preserve the empty batch record cleanup hook.
    ~PushTriple(){
    }
};

struct ErrorBase {
    void* table;
    char* text;
    SharedString* shared;
    // Construct a guarded shared character buffer for an error message.
    ErrorBase(const char* str) {
        const char* source = str;
        table = __vt__Q23std11logic_error;
        PushAllocator alloc;
        unsigned int n = strlen(source)+1;
        char* p = (char*)__nwa__FUl(n*2-n);
        text = p;
        {
            StringGuard guard(p, (char*)&alloc);
            shared = new SharedString(p);
            guard.ptr = 0;
        }
        strcpy(text, source);
    }
    // Run the exception base cleanup without changing its ABI.
    ~ErrorBase(){
        dtor_800460BC(this, -1);
    }
};
namespace std {

    struct length_error : ErrorBase {
        // Construct the owned length-error message and select its exception type.
        length_error(const char* str):ErrorBase(str){
            table = __vt__Q23std12length_error;
        }
    };
}

struct ThrowLengthError {
    void* table;
    char* text;
    SharedString* shared;
    // Leave exception storage for explicit initialization by init.
    ThrowLengthError() {
    }

    // Initialize the owned error-message buffer and shared control block.
    void init(const char* str) {
        const char* source = str;
        table = __vt__Q23std11logic_error;
        PushAllocator alloc;
        unsigned int n = strlen(source)+1;
        char* p = (char*)__nwa__FUl(n*2-n);
        text = p;
        {
            StringGuard guard(p, (char*)&alloc);
            shared = new SharedString(p);
            guard.ptr = 0;
        }
        strcpy(text, source);
    }
};
// Construct a guarded length-error message and throw it through the game's exception runtime.
inline void throwLengthError() {
    ThrowLengthError storage;
    ThrowLengthError* error = &storage;
    storage.init(packedResourcePathStrings+0x17);
    error->table = __vt__Q23std12length_error;
    __throw(packedResourcePathStrings+0x2b, error, __dt__Q23std12length_errorFv);
}

// Batch entries are three raw words: buffer handle, texture handle, vertex count
// (see DrawEntry at submission). size/capacity/remaining count entries. Temporary
// buffers own their slot allocation only; they never release the resource handles.
// During growth the appended tail is built first, then the prefix is copied backward.
// remaining tracks untransferred old entries; size tracks constructed new entries.
// After swapping, the temporary owns the old allocation and frees it at scope exit.
struct GrowBuffer_u80262614 {
    PushTriple* entries;
    unsigned int size, capacity;
    unsigned int* oldCapacity;
    unsigned int remaining;
    // Initialize an empty temporary batch vector tied to owner capacity.
    GrowBuffer_u80262614(unsigned int* c):entries(0), size(0), capacity(0), oldCapacity(c), remaining(0){
    }
    // Free the allocation retained by the temporary after swap or unwind.
    ~GrowBuffer_u80262614(){
        if (entries){
            PushAllocator allocator;
            size -= size;
            appFreeUni__FPv(entries);
        }
    }
};
// Return the larger unsigned growth request by reference.
inline const unsigned int& larger(const unsigned int& a, const unsigned int& b){
    return a < b?b:a;
}
// Throw when a batch growth request exceeds the twelve-byte element limit.
inline void pushCheck(unsigned int old, unsigned int extra){
    PushAllocator checkAllocator;
    if (extra > kTwelveByteElementLimit-old){
        PushAllocator errorAllocator;
        throwLengthError();
    }
}
#pragma scheduling on

// Append a buffer/texture/count batch, reallocating and moving entries when capacity is exhausted.
extern "C" void CourseRenderPacket_AppendBatch(GrowBuffer_u80262614* self, const PushTriple* value) {
    // Phase 1: append directly when capacity permits; otherwise validate growth.
    unsigned int newCapacity;
    PushAllocator allocator;
    if (self->size < self->capacity){
        self->size += 1;
        self->entries[self->size-1] = *value;
        return;
    }
    unsigned int savedCapacity = self->capacity;
    PushAllocator growthAllocator;
    pushCheck(savedCapacity, 1);
    GrowBuffer_u80262614 replacement(&self->capacity);
    unsigned int tailCount = 1+self->size-self->capacity;
    unsigned int oldCapacity = self->capacity;
    pushCheck(oldCapacity, tailCount);
    if (oldCapacity < kTwelveByteGrowthTier1)newCapacity = oldCapacity+larger((oldCapacity+1)*3/5, tailCount);
    else if (oldCapacity < kTwelveByteGrowthTier2)newCapacity = oldCapacity+larger((oldCapacity+1)/2, tailCount);
    else newCapacity = kTwelveByteElementLimit;
    PushAllocator storageAllocator;
    if (newCapacity > kTwelveByteElementLimit){
        PushAllocator errorAllocator;
        throwLengthError();
    }
    // Phase 2: reserve replacement storage and construct the new tail before copying.
    replacement.entries = (PushTriple*)appMallocDummy__FUl((((12*newCapacity))));
    replacement.capacity = newCapacity;
    unsigned int length = self->size;
    unsigned int savedLength = length;
    PushTriple* destination = (((savedLength+replacement.entries)));
    destination[replacement.size++] = *value;
    PushAllocator insertionAllocator;
    replacement.remaining = length;
    PushTriple* sourceEnd = (self->size+self->entries);
    PushTriple* sourceBegin = self->entries;
    // Phase 3: copy the oldCapacity prefix in reverse, preserving its logical order.
    PushAllocator copyAllocator;
    while (sourceEnd > sourceBegin){
        sourceEnd -= 1;
        --destination;
        new(destination) PushTriple(*sourceEnd);
        replacement.remaining -= 1;
        replacement.size = 1 + replacement.size;
    }
    // Phase 4: install the complete replacement; temporary cleanup frees oldCapacity slots.
    PushAllocator swapAllocator;
    PushTriple* retiredEntries = self->entries;
    self->size = 0;
    PushTriple* newEntries = replacement.entries;
    replacement.entries = retiredEntries;
    int retiredCapacity = self->capacity;
    self->capacity = replacement.capacity;
    replacement.capacity = retiredCapacity;
    self->entries = newEntries;
    self->size = replacement.size;
    replacement.size = 0;
}
#pragma scheduling on
#pragma global_optimizer off

struct ResizePair {
    unsigned int first, second;
    // Initialize or copy the two raw words of a texcoord entry.
    ResizePair():first(0), second(0){
    }
    // Initialize or copy the two raw words of a texcoord entry.
    ResizePair(const ResizePair& p){
        unsigned int a = p.first;
        unsigned int b = p.second;
        first = a;
        second = b;
    }
    // Preserve the empty two-word entry cleanup hook.
    ~ResizePair(){
    }
};

struct PairAllocator {
    char state;
    // Initialize the texcoord allocator marker.
    PairAllocator():state(0){
    }
    // Preserve the empty texcoord allocator cleanup hook.
    ~PairAllocator(){
    }
};

struct PairErrorBase {
    void* table;
    char* text;
    SharedString* shared;
    // Construct the guarded shared message for a texcoord length error.
    PairErrorBase(const char* str) {
        const char* source = str;
        table = __vt__Q23std11logic_error;
        PairAllocator alloc;
        unsigned int n = strlen(source)+1;
        char* p = (char*)__nwa__FUl(n*2-n);
        text = p;
        {
            StringGuard guard(p, (char*)&alloc);
            shared = new SharedString(p);
            guard.ptr = 0;
        }
        strcpy(text, source);
    }
    // Run the texcoord error base cleanup.
    ~PairErrorBase(){
        dtor_800460BC(this, -1);
    }
};
// Validate texcoord growth against the eight-byte element limit.
inline void pairCheck(unsigned int old, unsigned int extra) {
    if (extra > kTexCoordElementLimit-old){
        PairAllocator errorAllocator;
        throwLengthError();
    }
}
// Compute the original growth-tier candidate even when the caller discards it.
inline unsigned int pairDiscardedGrowth(unsigned int old){
    if (old < kTexCoordGrowthTier1)return (old+1)*3/5;
    else if (old < kTexCoordGrowthTier2)return (old+1)/2;
    else return kTexCoordElementLimit;
}

struct BufferPair {
    unsigned int first, second;
    // Initialize or copy a two-word texcoord record.
    BufferPair():first(0), second(0){
    }
    // Initialize or copy a two-word texcoord record.
    BufferPair(const BufferPair& p){
        try {
            unsigned int a = p.first;
            unsigned int b = p.second;
            first = a;
            second = b;
        } catch (...) {
        }
    }
    // Preserve the empty texcoord record cleanup hook.
    ~BufferPair(){
    }
};

struct PairBuffer {
    BufferPair* entries;
    unsigned int size, capacity;
    unsigned int* oldCapacity;
    unsigned int remaining;
    // Initialize an empty temporary texcoord vector tied to owner capacity.
    PairBuffer(unsigned int* c):entries(0), size(0), capacity(0), oldCapacity(c), remaining(0){
    }
    // Free the texcoord allocation retained after swap or unwind.
    ~PairBuffer(){
        if (entries){
            PairAllocator allocator;
            size -= size;
            appFreeUni__FPv(entries);
        }
    }
};
#pragma scheduling on
#pragma global_optimizer off

// Resize the two-word coordinate vector, zero new entries and transfer storage safely on growth.
// count/size/capacity are 8-byte entry counts. Growth retains the old prefix and
// zero-initializes the new tail; shrinking changes size only, retaining capacity.
// tailCount=count-oldSize stays unchanged until tail construction.
// Test tailCount>capacity first to avoid unsigned underflow in capacity-tailCount.
// The temporary owns fresh storage until swap, then old storage; no ownership gaps.
extern "C" void CoursePacketTexCoordVector_Resize(PairBuffer* self, unsigned int count) {
    if (count > self->size){
        unsigned int savedSizeCopy = self->size;
        unsigned int savedSize = self->size;
        PairAllocator allocator;
        unsigned int tailCount = count-savedSizeCopy;
        unsigned int size = savedSize;
        unsigned int capacity = self->capacity;
        if (tailCount > capacity || size > capacity-tailCount){
            PairAllocator growthAllocator;
            unsigned int oldCapacity = self->capacity;
            pairCheck(oldCapacity, (((self->size+tailCount))-capacity));
            unsigned int savedOld = oldCapacity;
            pairDiscardedGrowth(savedOld);
        } else{
            PairAllocator insertionAllocator;
            BufferPair* destination = self->entries+size;
            while (tailCount){
                BufferPair value;
                new(destination) BufferPair(value);
                --tailCount;
                destination += 1;
                ++self->size;
            }
            return;
        }
        // Growth phase 1: select a capacity >= requested count and allocate 8*newCapacity.
        PairBuffer replacement(&self->capacity);
        unsigned int newCapacity;
        unsigned int capacityIncrease = self->size+tailCount-self->capacity;
        unsigned int oldCapacity = self->capacity;
        pairCheck(oldCapacity, capacityIncrease);
        if (oldCapacity < kTexCoordGrowthTier1)newCapacity = oldCapacity+larger((1+oldCapacity)*3/5, capacityIncrease);
        else if (oldCapacity < kTexCoordGrowthTier2)newCapacity = oldCapacity+larger((oldCapacity+1)/2, capacityIncrease);
        else newCapacity = kTexCoordElementLimit;
        PairAllocator storageAllocator;
        if (newCapacity > kTexCoordElementLimit){
            PairAllocator errorAllocator;
            throwLengthError();
        }
        unsigned int savedCap = newCapacity;
        replacement.entries = (BufferPair*)appMallocDummy__FUl(((newCapacity*8)));
        replacement.capacity = newCapacity;
        // Growth phase 2: build zero tail, then reverse-copy the existing prefix.
        int length = self->size;
        replacement.remaining = length;
        PairAllocator insertionAllocator;
        BufferPair* destination = replacement.entries+length+replacement.size;
        while (tailCount){
            BufferPair value;
            new(destination) BufferPair(value);
            tailCount = tailCount - 1;
            ++destination;
            ++replacement.size;
        }
        BufferPair* sourceEnd = ((self->size+self->entries));
        BufferPair* sourceBegin = self->entries;
        PairAllocator copyAllocator;
        destination = replacement.remaining+replacement.entries;
        while (sourceBegin < sourceEnd){
            --destination;
            sourceEnd -= 1;
            new(destination) BufferPair(*sourceEnd);
            replacement.remaining -= 1;
            ++replacement.size;
        }
        // Growth phase 3: publish pointer/count/capacity together; temporary frees oldCapacity data.
        PairAllocator swapAllocator;
        BufferPair* newEntries = replacement.entries;
        BufferPair* retiredEntries = self->entries;
        self->size = 0;
        unsigned int retiredCapacity = self->capacity;
        self->capacity = replacement.capacity;
        replacement.entries = retiredEntries;
        replacement.capacity = retiredCapacity;
        self->entries = newEntries;
        self->size = replacement.size;
        replacement.size = 0;
    } else if (count < self->size){
        PairAllocator shrinkAllocator;
        self->size -= self->size-count;
    }
}
#pragma global_optimizer off
#pragma exceptions on
#pragma scheduling on

struct TripleAllocator {
    char state;
    // Initialize the position allocator marker.
    TripleAllocator():state(0){
    }
    // Preserve the empty position allocator cleanup hook.
    ~TripleAllocator(){
    }
};

struct TripleErrorBase {
    void* table;
    char* text;
    SharedString* shared;
    // Construct the guarded shared message for a position length error.
    TripleErrorBase(const char* str) {
        const char* source = str;
        table = __vt__Q23std11logic_error;
        TripleAllocator alloc;
        unsigned int n = strlen(source)+1;
        char* p = (char*)__nwa__FUl(n*2-n);
        text = p;
        {
            StringGuard guard(p, (char*)&alloc);
            shared = new SharedString(p);
            guard.ptr = 0;
        }
        strcpy(text, source);
    }
    // Run the position error base cleanup.
    ~TripleErrorBase(){
        dtor_800460BC(this, -1);
    }
};

struct ResizeTriple {
    ResizePair pair;
    unsigned int third;
    // Initialize or copy a three-word position record.
    ResizeTriple():third(0){
    }
    // Initialize or copy a three-word position record.
    ResizeTriple(const ResizeTriple& p):pair(p.pair){
        try {
            third = p.third;
        } catch (...){
        }
    }
    // Preserve the empty position record cleanup hook.
    ~ResizeTriple(){
    }
};

struct TripleBuffer {
    ResizeTriple* entries;
    unsigned int size, capacity;
    unsigned int* oldCapacity;
    unsigned int remaining;
    // Initialize an empty temporary position vector tied to owner capacity.
    TripleBuffer(unsigned int* c):entries(0), size(0), capacity(0), oldCapacity(c), remaining(0){
    }
    // Free the position allocation retained after swap or unwind.
    ~TripleBuffer(){
        if (entries){
            TripleAllocator allocator;
            size -= size;
            appFreeUni__FPv(entries);
        }
    }
};
#pragma global_optimizer off
// Compute the original twelve-byte growth-tier candidate.
inline unsigned int discardedGrowth(unsigned int old){
    if (old < kTwelveByteGrowthTier1)return (old+1)*3/5;
    else if (old < kTwelveByteGrowthTier2)return (old+1)/2;
    else return kTwelveByteElementLimit;
}
// Validate position growth against the twelve-byte element limit.
inline void tripleCheck(unsigned int old, unsigned int extra){
    if (extra > kTwelveByteElementLimit-old){
        TripleAllocator errorAllocator;
        throwLengthError();
    }
}

// Resize the three-word position vector with zero-filled growth and guarded storage transfer.
extern "C" void CoursePacketPositionVector_Resize(TripleBuffer* self, unsigned int count) {
    if (count > self->size){
        TripleAllocator allocator;
        unsigned int tailCount = count-self->size;
        unsigned int size = self->size;
        int capacity = self->capacity;
        if (tailCount > capacity || size > capacity-tailCount){
            unsigned int oldCapacity = self->capacity;
            unsigned int capacityIncrease = self->size+tailCount-capacity;
            TripleAllocator growthAllocator;
            tripleCheck(oldCapacity, capacityIncrease);
            discardedGrowth(oldCapacity);
        } else{
            TripleAllocator insertionAllocator;
            ResizeTriple* destination = (size+self->entries);
            while (tailCount){
                ResizeTriple value;
                new(destination) ResizeTriple(value);
                tailCount = tailCount - 1;
                ++destination;
                ++self->size;
            }
            return;
        }
        // Grow as in texcoord resize: construct the zero tail, reverse-copy prefix,
        // then swap. Temporary cleanup owns the oldCapacity allocation after publication.
        TripleBuffer replacement(&self->capacity);
        unsigned int capacityIncrease = self->size+tailCount-self->capacity;
        unsigned int oldCapacity = self->capacity;
        tripleCheck(oldCapacity, capacityIncrease);
        unsigned int newCapacity;
        if (oldCapacity < kTwelveByteGrowthTier1)newCapacity = oldCapacity+larger((oldCapacity+1)*3/5, capacityIncrease);
        else if (oldCapacity < kTwelveByteGrowthTier2)newCapacity = oldCapacity+larger((oldCapacity+1)/2, capacityIncrease);
        else newCapacity = kTwelveByteElementLimit;
        TripleAllocator storageAllocator;
        if (newCapacity > kTwelveByteElementLimit){
            TripleAllocator errorAllocator;
            throwLengthError();
        }
        replacement.entries = (ResizeTriple*)appMallocDummy__FUl(newCapacity*12);
        replacement.capacity = newCapacity;
        unsigned int length = self->size;
        replacement.remaining = length;
        TripleAllocator insertionAllocator;
        ResizeTriple* destination = length+replacement.entries+replacement.size;
        while (tailCount){
            ResizeTriple value;
            new(destination) ResizeTriple(value);
            --tailCount;
            ++destination;
            ++replacement.size;
        }
        TripleAllocator copyAllocator;
        ResizeTriple* sourceEnd = self->entries+self->size;
        ResizeTriple* sourceBegin = self->entries;
        destination = replacement.entries+replacement.remaining;
        while (sourceEnd > sourceBegin){
            --sourceEnd;
            --destination;
            new(destination) ResizeTriple(*sourceEnd);
            --replacement.remaining;
            ++replacement.size;
        }
        TripleAllocator swapAllocator;
        self->size = 0;
        ResizeTriple* retiredEntries = self->entries;
        ResizeTriple* newEntries = replacement.entries;
        unsigned int retiredCapacity = self->capacity;
        replacement.entries = retiredEntries;
        self->capacity = replacement.capacity;
        replacement.capacity = retiredCapacity;
        self->entries = newEntries;
        self->size = replacement.size;
        replacement.size = 0;
    } else if (count < self->size){
        TripleAllocator shrinkAllocator;
        self->size -= self->size-count;
    }
}
#pragma global_optimizer off

struct SixAllocator {
    char state;
    // Initialize the expanded-vertex allocator marker.
    SixAllocator():state(0){
    }
    // Preserve the empty expanded-vertex allocator cleanup hook.
    ~SixAllocator(){
    }
};

struct SixErrorBase {
    void* table;
    char* text;
    SharedString* shared;
    // Construct the guarded shared message for an expanded-vertex length error.
    SixErrorBase(const char* str) {
        const char* source = str;
        table = __vt__Q23std11logic_error;
        SixAllocator alloc;
        unsigned int n = strlen(source)+1;
        char* p = (char*)__nwa__FUl(n*2-n);
        text = p;
        {
            StringGuard guard(p, (char*)&alloc);
            shared = new SharedString(p);
            guard.ptr = 0;
        }
        strcpy(text, source);
    }
    // Run the expanded-vertex error base cleanup.
    ~SixErrorBase(){
        dtor_800460BC(this, -1);
    }
};

struct ResizeSix {
    ResizePair a, b, c;
    // Initialize or copy the six-word expanded-vertex record.
    ResizeSix(){
    }
    // Initialize or copy the six-word expanded-vertex record.
    ResizeSix(const ResizeSix& p):a(p.a), b(p.b), c(p.c){
    }
    // Preserve the empty expanded-vertex record cleanup hook.
    ~ResizeSix(){
    }
};

struct SixBuffer {
    ResizeSix* entries;
    unsigned int size, capacity;
    unsigned int* oldCapacity;
    unsigned int remaining;
    // Initialize an empty temporary expanded-vertex vector tied to owner capacity.
    SixBuffer(unsigned int* c):entries(0), size(0), capacity(0), oldCapacity(c), remaining(0){
    }
    // Free the expanded-vertex allocation retained after swap or unwind.
    ~SixBuffer(){
        if (entries){
            SixAllocator allocator;
            size -= size;
            appFreeUni__FPv(entries);
        }
    }
};
#pragma global_optimizer off

// Resize the six-word course-vertex vector while preserving allocation and copy-unwind order.
extern "C" void CoursePacketVertexVector_Resize(SixBuffer* self, unsigned int count) {
    if (count > self->size){
        unsigned int newCapacity;
        unsigned int tailCount = count-self->size;
        int size = self->size;
        unsigned int length = self->size;
        SixAllocator allocator;
        if (tailCount > self->capacity || size > self->capacity-tailCount){
            int oldCapacity = self->capacity;
            unsigned int capacityIncrease = ((self->size+tailCount))-self->capacity;
            SixAllocator growthAllocator;
            if (capacityIncrease > kVertexElementLimit-oldCapacity) {
                SixAllocator errorAllocator;
                throwLengthError();
            }
            if (oldCapacity < kVertexGrowthTier1){
            } else if (oldCapacity < kVertexGrowthTier2){
            }
        } else{
            SixAllocator insertionAllocator;
            ResizeSix* destination = (self->entries+size);
            while (tailCount){
                ResizeSix value;
                new(destination) ResizeSix(value);
                --tailCount;
                destination = 1 + destination;
                ++self->size;
            }
            return;
        }
        // Grow as in texcoord resize, with 24-byte entries: build tail, reverse-copy
        // prefix, then swap ownership so cleanup releases the oldCapacity allocation.
        SixBuffer replacement(&self->capacity);
        unsigned int capacityIncrease = self->size+tailCount-self->capacity;
        unsigned int oldCapacity = self->capacity;
        if (capacityIncrease > kVertexElementLimit-oldCapacity){
            SixAllocator errorAllocator;
            throwLengthError();
        }
        if (oldCapacity < kVertexGrowthTier1)newCapacity = oldCapacity+larger((1+oldCapacity)*3/5, capacityIncrease);
        else if (oldCapacity < kVertexGrowthTier2)newCapacity = oldCapacity+larger((oldCapacity+1)/2, capacityIncrease);
        else newCapacity = kVertexElementLimit;
        SixAllocator storageAllocator;
        if (newCapacity > kVertexElementLimit){
            SixAllocator errorAllocator;
            throwLengthError();
        }
        replacement.entries = (ResizeSix*)appMallocDummy__FUl(((newCapacity*24)));
        replacement.remaining = length;
        replacement.capacity = newCapacity;
        SixAllocator insertionAllocator;
        ResizeSix* destination = replacement.entries+length+replacement.size;
        while (tailCount){
            ResizeSix value;
            new(destination) ResizeSix(value);
            tailCount -= 1;
            ++destination;
            replacement.size = replacement.size + 1;
        }
        __typeof__(self->entries+self->size) savedEntries = self->size+self->entries;
        SixAllocator copyAllocator;
        ResizeSix* sourceEnd = savedEntries;
        ResizeSix* sourceBegin = self->entries;
        destination = (replacement.remaining+replacement.entries);
        while (sourceBegin < sourceEnd){
            destination -= 1;
            sourceEnd = sourceEnd - 1;
            new(destination) ResizeSix(*sourceEnd);
            replacement.remaining = replacement.remaining - 1;
            replacement.size++;
        }
        ResizeSix* retiredEntries = self->entries;
        SixAllocator swapAllocator;
        ResizeSix* newEntries = replacement.entries;
        replacement.entries = retiredEntries;
        self->size = 0;
        unsigned int retiredCapacity = self->capacity;
        self->capacity = replacement.capacity;
        replacement.capacity = retiredCapacity;
        self->entries = newEntries;
        self->size = replacement.size;
        replacement.size = 0;
    } else if (count < self->size){
        SixAllocator shrinkAllocator;
        self->size -= self->size-count;
    }
}

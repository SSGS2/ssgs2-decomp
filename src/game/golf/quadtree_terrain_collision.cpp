// The terrain collision quadtree (CQuadTree): its object and collision mesh buffers,
// how they are cleared and freed, and the copy helpers for its vector records.

#include "MSL/vector.h"
// Element type of two vectors in this unit whose elements are plain data (nothing to destroy).
struct PlainVectorElement {
    int word;
};
typedef std::vector<PlainVectorElement, app::app_allocator<PlainVectorElement> > PlainElementVector;
typedef std::vector<void*, app::app_allocator<void*> > PointerVector;
#include "game/types.h"
#include "game/wvector.h"

#include "sdk_protos.h"
#include "classes/CProjectG.h"
#include "classes/CQuadTree.h"
#include <decomp/cw_bitfields.h>

extern "C" void* appMallocDummy__FUl(unsigned int);
extern "C" void appFreeUni__FPv(void*);
// Construct a copied vector entry in its already allocated destination slot.
inline void* operator new(size_t, void* p) {
    return p;
}
// Placement construction failure does not own or release the destination block.
inline void operator delete(void*, void*) {}

typedef Vec3f Triple;
// Copy-only payload views, not indices: the vector headers index WHOLE records.
// Entry28 and Entry36 have no nested allocations; freeing a vector block never frees a pointee.
// Field interpretations must come from producers/consumers in other units.
struct Entry28 {
    // Ground-region payload: vectors at +0/+12 and one word at +24.
    // This unit only copies the record; coordinate spaces and word semantics are unresolved.
    Triple a, b;
    unsigned int c;
    // Copy one complete 28-byte region record without allocating nested storage.
    Entry28(const Entry28& s) : a(s.a), b(s.b), c(s.c) {}
};
struct Entry36 {
    // Collision-event payload: byte +0, short +2, words +4/+8, vectors +12/+24.
    // No ownership or coordinate interpretation is established by this copy-only view.
    unsigned char a;
    short b;
    unsigned int c, d;
    Triple e, f;
    // Copy the event's fields in their original order, leaving ABI padding implicit.
    Entry36(const Entry36& s) : a(s.a), b(s.b), c(s.c), d(s.d), e(s.e), f(s.f) {}
};
struct Tag {
    volatile char c;
    // Write the retained volatile construction/cleanup marker; not a gameplay flag.
    void clear() {
        c = 0;
    }
};
// The two owned auxiliary arrays are at +0x2204/+0x2208, before objectSoundEnabled.
// Element types and their relationship to region/event indices are not established here.
// The separate +0x214c vector pair is initialized to zero; no coordinate role is inferred.
struct QuadTreeAuxiliaryBuffers {
    char prefix[0x2204];
    void* first;
    void* second;
};
struct RefHeader {
    CW_BITS2(unsigned int refs : 16, unsigned int flags : 16)
};
// Local three-word vector view: data owns the allocation, count is live entries,
// capacity is entries. Element payloads here need no destructor during block release.
// alloc() takes BYTES but hardcodes a 32-entry capacity; it is only for initial reserves.
// The empty C++ destructor is intentional: explicit destroy() releases storage after swap.
template <class T> struct LocalVector {
    T* data;
    unsigned int count, capacity;
    // Start an empty, non-owning vector before reserving its initial storage.
    LocalVector() : data(0), count(0), capacity(0) {}
    // Allocate the caller-calculated byte count for exactly 32 entries.
    void alloc(unsigned int allocationBytes) {
        T* block = (T*)appMallocDummy__FUl(allocationBytes);
        capacity = 32;
        data = block;
    }
    // No implicit release; the matched expansion performs explicit cleanup below.
    ~LocalVector() {}
    // Clear count and free the block, retaining stale data/capacity as a destructor-only view.
    void destroy(Tag& tag) {
        if (data) {
            tag.clear();
            count -= ((char*)this + count) - (char*)this;
            appFreeUni__FPv(data);
        }
    }
    // Exchange all three words, transferring block ownership without copying entries.
    void swap(LocalVector& other) {
        unsigned int cap = other.capacity;
        other.capacity = capacity;
        capacity = cap;
        T* ptr = other.data;
        other.data = data;
        data = ptr;
        unsigned int n = other.count;
        other.count = count;
        count = n;
    }
};

typedef LocalVector<unsigned int> OwnedVectorStorage;

extern "C" {
void appFreeUni__FPv(void*);
void __dl__FPv(void*);
void __dla__FPv(void*);
void __destroy_new_array(void*, void*);
void Buffer_dtor(void);
void CubeResourceStorage_dtor(void*, short);
extern void* g_quadTreeTriangleQueryCache;
extern char __vt__9CQuadTree[];

// Empty construction hook for triangle-query cache entries.
void QuadTreeQueryCacheEntry_ConstructionHook(void*) {}

void Vec3_Zero(void*);
void __as__7WVectorFRC7WVector(void*, void*);
void* GolfBall__Fv(void);
void* appMallocDummy__FUl(unsigned int);
void appFreeUni__FPv(void*);
void* __nwa__FUl(unsigned int);
void VertexBuffer_Create(unsigned int, int, unsigned int, int, void**, int);
void IndexBuffer_Create(unsigned int, int, int, int, void**, int, int);
void* memcpy(void*, const void*, unsigned int);
void* memset(void*, int, unsigned int);
extern float kFloat_1_QuadTree, kFloat_1_1_QuadTree, kFloat_0_2_QuadTree, kFloat_3_QuadTree, kFloat_255_QuadTree;
// CGolfBall::m_radius (game/golfball.h). Including that header renumbers the local array-class relocation symbols.
static const int kBallRadiusOffset = 0xc0;
static const int kInitialVectorCapacity = 32;
static const int kTriangleQueryCacheCount = 32;
static const int kTriangleQueryCacheEntryBytes = 0xf48;
static const int kProjectedPatchVertexBytes = 0x1800;
static const int kProjectedPatchIndexCount = 0x300;
static const int kProjectedPatchVertexFormat = 0x11f001;
extern unsigned int
    s_quadTreePerShotResetWord; // Shared per-shot simulation state, also reset by CQuadTree_ResetPerShotSimulationState.
extern unsigned int s_quadTreePatchCacheWriteIndex;

// Initialize collision state and reserve region, object, event, query-cache and mesh buffers.
// The tree owns its vector blocks after construction. The triangle-query cache is
// process-global, and the two mesh buffers use 16-bit reference counts.
CQuadTree::CQuadTree() {
    // Establish the vtable, empty vector headers, and default simulation/patch values.
    vtbl = (void*)__vt__9CQuadTree;
    courseData = 0;
    groundSurface = 0;
    groundRegions = 0;
    groundRegionCount = 0;
    groundRegionCapacity = 0;
    Vec3_Zero(&greenBoxMinX);
    Vec3_Zero(&greenBoxMaxX); // green box corners at +0x209C/+0x20A8
    Vec3_Zero(worldBounds);
    Vec3_Zero(worldBounds + sizeof(Triple));
    float greenScale, unitScale, slopeScale;
    unitScale = kFloat_1_QuadTree;

    slopeScale = kFloat_1_1_QuadTree;
    greenScale = kFloat_0_2_QuadTree;
    simulationMode = 0;
    objects = 0;
    objectCount = 0;
    objectCapacity = 0;
    collisionEvents = 0;
    collisionEventCount = 0;
    collisionEventCapacity = 0;
    objectSoundEnabled = (1);
    vertexBuffer = 0;
    indexBuffer = 0;
    cells = 0;
    slopeGravityScale = slopeScale;
    bounceScale = unitScale;
    frictionScale = unitScale;
    greenFactor = greenScale;
    s_quadTreePerShotResetWord = 0;
    void* ball = GolfBall__Fv();
    float white, cup, ballRadius;
    ballRadius = *(float*)((char*)ball + kBallRadiusOffset);
    cup = kFloat_3_QuadTree;
    white = kFloat_255_QuadTree;
    cup *= ballRadius;
    cupRadius = cup; // Same world-distance units as the ball radius.
    float firstPatchColor[3] = {white, white, white};
    *(WVector*)pad_218C = *(WVector*)firstPatchColor;
    float secondPatchColor[3] = {kFloat_255_QuadTree, kFloat_255_QuadTree, kFloat_255_QuadTree};
    *(WVector*)(pad_218C + sizeof(Triple)) = *(WVector*)secondPatchColor;
    unk_21A4 = 0; // Reserved trailing patch-state word; no reader is established here.
    // Reserve phase (three vectors, capacity measured in entries):
    //   regions: copy 28-byte records; objects: move pointer words; events: copy 36-byte records.
    // Each block below follows allocate -> populate -> clear old count -> swap -> free old block.
    // After swap the tree owns the new block; tmp owns only the old, now-empty allocation.
    // Redundant temporaries/markers preserve compiler-generated exception cleanup identities.
    if (groundRegionCapacity < kInitialVectorCapacity) {
        Tag t4;
        Tag t3;
        Tag t2;
        Tag t1;
        unsigned int start;
        unsigned int sourceCount;
        Entry28* src;
        Entry28* end;
        Entry28* dst;
        LocalVector<Entry28> tmp;
        t1.clear();
        tmp.alloc(kInitialVectorCapacity * sizeof(Entry28));
        // Descriptor aliases below all refer to tmp's empty destination (start == 0).
        // Their declaration count also identifies the later local ArrayItem cleanup symbols.
        __typeof__(tmp.count) savedCountCopy = tmp.count;
        sourceCount = groundRegionCount;
        __typeof__(tmp.data) savedDataCopyCopy = tmp.data;
        start = savedCountCopy;
        src = (Entry28*)groundRegions;
        end = src + sourceCount;
        __typeof__(savedDataCopyCopy) savedDataCopy = tmp.data;
        __typeof__(tmp.data) savedData = tmp.data;
        __typeof__(savedData) savedSavedDataCopy = savedDataCopy;
        unsigned int savedStartCopy = start;
        dst = savedSavedDataCopy + savedStartCopy;
        t2.clear();
        while (src < end) {
            new (dst) Entry28(*src);
            ++src;
            dst = dst + 1;
            ++tmp.count;
        }
        // Commit region ownership: tmp.count equals sourceCount; old count becomes zero.
        // Swap publishes the copied data/count/capacity together; destroy frees only the old block.
        groundRegionCount = 0;
        t3.clear();
        tmp.swap(*(LocalVector<Entry28>*)&groundRegions);
        tmp.destroy(t4);
    }
    // Object reserve: memcpy moves pointer words, memset removes the old aliases.
    // Pointees survive this reserve; only ClearObjects/FreeBuffers delete them.
    if (objectCapacity < kInitialVectorCapacity) {
        int n;
        unsigned int size;
        unsigned int* dest;
        unsigned int* end;
        unsigned int* src;
        Tag t4;
        Tag t3;
        Tag t2;
        Tag t1;
        LocalVector<unsigned int> tmp;
        t1.clear();

        __typeof__(tmp.count) savedCount = tmp.count;

        unsigned int* block = (unsigned int*)appMallocDummy__FUl(kInitialVectorCapacity * sizeof(unsigned int));
        size = objectCount;
        src = (unsigned int*)objects;
        end = src + size;
        tmp.capacity = kInitialVectorCapacity;
        tmp.data = block;

        n = end - src;
        t2.clear();
        dest = tmp.data + (savedCount);
        unsigned int bytes = n * 4;
        memcpy(dest, src, bytes);
        memset(src, 0, bytes);
        tmp.count += n;
        objectCount = 0;
        t3.clear();
        tmp.swap(*(LocalVector<unsigned int>*)&objects);
        tmp.destroy(t4);
    }
    // Event reserve: copy records forward into tmp, then transfer the complete descriptor.
    // No event destructor or nested allocation is involved in releasing the old backing block.
    if (collisionEventCapacity < kInitialVectorCapacity) {
        int start;
        Tag t4;
        Tag t3;
        Tag t2;
        Tag t1;

        int sourceCount;
        Entry36* src;
        Entry36* dst;
        Entry36* end;
        LocalVector<Entry36> tmp;
        t1.clear();
        tmp.alloc(kInitialVectorCapacity * sizeof(Entry36));
        sourceCount = collisionEventCount;
        src = (Entry36*)collisionEvents;
        start = tmp.count;
        end = src + sourceCount;
        __typeof__(tmp.data + start) savedStart = tmp.data + start;
        t2.clear();
        dst = savedStart;
        while (end > src) {
            new (dst) Entry36(*src);
            ++src;
            ++dst;
            ++tmp.count;
        }
        collisionEventCount = 0;
        t3.clear();
        tmp.swap(*(LocalVector<Entry36>*)&collisionEvents);
        tmp.destroy(t4);
    }
    // Allocate and zero the shared triangle-query cache, then reset its write cursor.
    // Array new prepends a 16-byte cookie; the destructor releases the adjusted allocation base.
    struct ArrayItem {
        char bytes[kTriangleQueryCacheEntryBytes];
        // Run the cache-entry construction hook; bulk zeroing follows array construction.
        ArrayItem() {
            QuadTreeQueryCacheEntry_ConstructionHook(this);
        }
        // Allocate cache-array bytes with the game's array allocator.
        static void* operator new[](size_t bytes) {
            return __nwa__FUl(bytes);
        }
        // Release an array allocation if construction unwinds.
        static void operator delete[](void* ptr) {
            __dla__FPv(ptr);
        }
    };
    enum {
        kProjectedPatchIndexFormatCode = 3
    }; // API code; interpretation unresolved here.
    void* arr = new ArrayItem[kTriangleQueryCacheCount];
    g_quadTreeTriangleQueryCache = arr;
    if (arr) {
        memset(arr, 0, kTriangleQueryCacheCount * kTriangleQueryCacheEntryBytes);
        s_quadTreePatchCacheWriteIndex = 0;
    }
    // Replace the projected-patch buffers, releasing a prior reference before each create call.
    // These headers were cleared above; the release paths retain the original shared-handle pattern.
    {
        void* sharedBuffer = vertexBuffer;
        if (sharedBuffer) {
            RefHeader* header = (RefHeader*)sharedBuffer;
            --header->refs;
            if (!header->refs)
                CubeResourceStorage_dtor(sharedBuffer, (short)1);
            vertexBuffer = 0;
        }
    }
    VertexBuffer_Create(kProjectedPatchVertexBytes, 0, kProjectedPatchVertexFormat, 0, (void**)&vertexBuffer, 0);
    {
        void* sharedBuffer = indexBuffer;
        if (sharedBuffer) {
            RefHeader* header = (RefHeader*)sharedBuffer;
            header->refs = header->refs - 1;
            if (header->refs == 0)
                CubeResourceStorage_dtor(sharedBuffer, (short)1);
            indexBuffer = 0;
        }
    }
    IndexBuffer_Create(kProjectedPatchIndexCount, 0, kProjectedPatchIndexFormatCode, 0, (void**)&indexBuffer, 0, 0);
}

static const int kArrayAllocationCookieBytes = 0x10;

// Clear and release one vector allocation, then optionally delete its wrapper.
// flag > 0 deletes self; null is accepted. Returns the original, possibly freed address; no elements deleted.
void* dtor_801CC7F8(void* self, int flag) {
    return ::std::__vector_deleting_dtor((PlainElementVector*)self, flag);
}

// Clear and release one vector allocation, then optionally delete its wrapper.
// flag > 0 deletes self; null is accepted. Returns the original, possibly freed address; no elements deleted.
void* dtor_801CC8F4(void* self, int flag) {
    return ::std::__vector_deleting_dtor((PlainElementVector*)self, flag);
}

// Clear and release the pointer-vector allocation, then optionally delete its wrapper.
// Does not delete pointees (ClearObjects does); flag > 0 deletes self. Null accepted; returns original address.
void* dtor_801CC874(void* self, int flag) {
    return ::std::__vector_deleting_dtor((PointerVector*)self, flag);
}

// Delete every owned collision object and clear the object count.
// Retains the pointer-vector allocation and capacity. Object storage uses scalar delete.
// The collision-mesh count callers pass is not read.
// Byte offsets and the count-minus-span expressions are the original's iterator/erase code.
void CQuadTree::ClearObjects(int collisionMeshCount) {
    int offset = 0;
    int index = 0;
    int zero = 0;
    while ((unsigned int)index < objectCount) { // 0x801CCC30.
        void** array = (void**)objects;
        void* item = *(void**)((char*)array + offset);
        if (item) {
            if (item) {
                __dl__FPv(item);
            }
            array = (void**)objects;
            *(int*)((char*)array + offset) = zero;
        }
        offset += 4;
        index++;
    }
    unsigned int* p = &objectCount;
    unsigned int count = *(volatile unsigned int*)p;
    *p = count - (((char*)p + count) - (char*)p);
}

// Free cell data, shared mesh buffers, query cache and vectors before optional deletion.
// Null accepted; flag > 0 deletes self. Returns the original, possibly freed address.
// FreeBuffers deletes object pointees first; the final vector cleanup releases only backing blocks.
CQuadTree::~CQuadTree() {
    CQuadTree* tree = this;
    *(void**)this = (void*)__vt__9CQuadTree;
    // Release cells, object pointees and auxiliary arrays, retaining now-empty vector blocks.
    FreeBuffers();
    // Drop each shared mesh reference; destroy its buffer only when the count reaches zero.
    void* sharedBuffer = tree->vertexBuffer;
    if (sharedBuffer) {
        RefHeader* header = (RefHeader*)sharedBuffer;
        header->refs--;
        if (header->refs == 0) {
            CubeResourceStorage_dtor(sharedBuffer, 1);
        }
        tree->vertexBuffer = 0;
    }
    sharedBuffer = tree->indexBuffer;
    if (sharedBuffer) {
        RefHeader* header = (RefHeader*)sharedBuffer;
        header->refs--;
        if (header->refs == 0) {
            CubeResourceStorage_dtor(sharedBuffer, 1);
        }
        tree->indexBuffer = 0;
    }
    // Release the singleton cache at its allocation base, before releasing local vector storage.
    void* queryCache = g_quadTreeTriangleQueryCache;
    if (queryCache) {
        if (queryCache) {
            __dla__FPv((char*)queryCache - kArrayAllocationCookieBytes);
        }
        g_quadTreeTriangleQueryCache = 0;
    }
    // Storage phase, in order: events -> object pointer table -> ground regions.
    // Object pointees are already gone. Count-minus-span is the matched clear-to-zero
    // expansion; data/capacity stay stale because this is destruction, not reusable clear.
    // Repeated header-address guards are compiler cleanup scopes, not separate ownership.
    char* eventHeaderGuard0 = (char*)&tree->collisionEvents;
    if (eventHeaderGuard0) {
        char* eventHeaderGuard1 = (char*)&tree->collisionEvents;
        if (eventHeaderGuard1) {
            char* eventHeaderGuard2 = (char*)&tree->collisionEvents;
            if (eventHeaderGuard2) {
                char* eventHeaderGuard3 = (char*)&tree->collisionEvents;
                if (eventHeaderGuard3) {
                    void* q = (void*)tree->collisionEvents;
                    if (q) {
                        tree->collisionEventCount -= ((char*)this + tree->collisionEventCount) - (char*)this;
                        appFreeUni__FPv(q);
                    }
                }
            }
        }
    }
    if (&tree->objects) {
        if (&tree->objects) {
            if (&tree->objects) {
                if (&tree->objects) {
                    if (&tree->objects) {
                        void* q = tree->objects;
                        if (q) {
                            tree->objectCount -= ((char*)this + tree->objectCount) - (char*)this;
                            appFreeUni__FPv(q);
                        }
                    }
                }
            }
        }
    }
    if (&tree->groundRegions) {
        if (&tree->groundRegions) {
            if (&tree->groundRegions) {
                if (&tree->groundRegions) {
                    void* q = (void*)tree->groundRegions;
                    if (q) {
                        tree->groundRegionCount -= ((char*)this + tree->groundRegionCount) - (char*)this;
                        appFreeUni__FPv(q);
                    }
                }
            }
        }
    }
}

// Release the quadtree cell/object allocations and both auxiliary arrays, preserving its vectors.
// Valid self required. Clears region/event/object counts but retains their backing blocks and capacities.
// Also retains the global query cache and shared mesh buffers; those are released only by the destructor.
void CQuadTree::FreeBuffers() {
    // Release the first cell's nested allocation, then destroy the complete owned cell array.
    void* cellArray = (void*)cells;
    if (cellArray) {
        void* p = *(void**)cellArray;
        if (p) {
            __dla__FPv(p);
            cellArray = (void*)cells;
            *(int*)cellArray = 0;
        }
    }
    cellArray = (void*)cells;
    if (cellArray) {
        __destroy_new_array(cellArray, (void*)Buffer_dtor);
        (*(int*)&cells) = 0;
    }
    int offset;
    int i;
    int zero;
    i = 0;
    offset = 0;
    zero = 0;
    // Erase region/event records without releasing their reserved blocks, then delete all objects.
    unsigned int regionCount = groundRegionCount;
    unsigned int eventCount = collisionEventCount;
    groundRegionCount = regionCount - (((char*)this + regionCount) - (char*)this);
    collisionEventCount = eventCount - (((char*)this + eventCount) - (char*)this);
    while ((unsigned int)i < objectCount) {
        void** array = (*(void***)&objects);
        void* item = *(void**)((char*)array + offset);
        if (item) {
            if (item) {
                __dl__FPv(item);
            }
            array = (*(void***)&objects);
            *(int*)((char*)array + offset) = zero;
        }
        offset += 4;
        i++;
    }
    unsigned int* count = (&objectCount);
    unsigned int cv = *(volatile unsigned int*)count;
    void* q = ((QuadTreeAuxiliaryBuffers*)this)->first;
    *count = cv - (((char*)this + cv) - (char*)this);
    // Release both owned auxiliary arrays and null their slots; other tree storage remains reserved.
    if (q) {
        __dla__FPv(q);
        (*(int*)&((QuadTreeAuxiliaryBuffers*)this)->first) = 0;
    }
    q = ((QuadTreeAuxiliaryBuffers*)this)->second;
    if (q) {
        __dla__FPv(q);
        (*(int*)&((QuadTreeAuxiliaryBuffers*)this)->second) = 0;
    }
    // Retained repeated cleanup of the now-null second slot; no third auxiliary array is implied.
    q = ((QuadTreeAuxiliaryBuffers*)this)->second;
    if (q) {
        __dla__FPv(q);
        (*(int*)&((QuadTreeAuxiliaryBuffers*)this)->second) = 0;
    }
}
} // extern "C"

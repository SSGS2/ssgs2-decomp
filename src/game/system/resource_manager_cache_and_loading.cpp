// The resource manager: its caches of models, fonts and bitmaps, texture reference counts and release,
// and loading of bitmaps (from files and archives) and puppet models.
//
// Also the hash tables and string-keyed entry vectors the caches use, and their destructors.

#include "classes/BitmapLayout.h"
#include "classes/CGsrResource.h"
#define AsBitmapLayout(p) ((BitmapLayout*)(p))
#include "classes/ResourceTextureRecord.h"
#define AsResourceTextureRecord(p) ((ResourceTextureRecord*)(p))
#include "classes/MWStringStorage.h"
#include "classes/ResourceManager.h"
#define AsResourceManager(p) ((ResourceManager*)(p))
#include "classes/ResourceEntryVector.h"
#define AsResourceEntryVector(p) ((ResourceEntryVector*)(p))
#include "classes/WView.h"
#include "classes/WPuppet.h"
#include "iff/IffStruct.h"
#define AsWView(p) ((WView*)(p))
#define GAME_NSTRING_RANGE_CONSTRUCTOR
#include "game/string.h"
// Compare iterator/pointer positions without extending either operand's lifetime.
template <class T> inline bool same_pos(const T& lhs, const T& rhs) {
    return lhs == rhs;
}

#include "sdk_protos.h"
#include "MSL/vector.h"
#include "classes/CString.h"
#include "classes/CHashPath.h"
#include "classes/CFileAsync.h"
#include "classes/WOverlay.h"
#include "classes/W3dSpr.h"

extern "C" {

// Common cache bookkeeping shared by fonts and renderable resource instances.
struct ManagedResourceHeader {
    char reserved[0xC];
    unsigned referenceCount;
    void* resourceManager;
};
struct CacheEntryStorage {
    void* entries;
    unsigned count;
};
struct RecordListLinks {
    void* previous;
    void* next;
};
struct RecordListState {
    int count;
    RecordListLinks sentinel;
};
struct AsyncFileResource {
    char reserved[0xC];
    void* vtable;
};
enum ResourceLoadFormat {
    kFormatPet = 0xFFF,
    kFormatMpet = 0x101B,
    kFormatApet = 0x166,
    kFormatBpet = 0x82,
    kFormatGmr = 0x2001
};
enum ResourceLoadState {
    kWaitingForOpen = 1,
    kAllocateReadBuffer = 2,
    kWaitingForRead = 3,
    kReadComplete = 4,
    kCachedModelReady = 5,
    kModelReturned = 6
};
enum {
    kOpaque = 0xFF,
    kImmortalReferenceCount = 0xFFFFFFFF,
    kMaxCacheEntries = 0xFFFFFFF,
    kFirstGrowthThreshold = 0x5555555,
    kSecondGrowthThreshold = 0xAAAAAAA,
    kDvdReadAlignment = 32,
    kTextureSkipInitializeMask = 0x10003000,
    kTexture24Bit = 0x80000000,
    kTextureExtraHalfStorage = 0x40000000,
    kTextureNameBang = 0x20000000,
    kTextureNameBrace = 0x800,
    kTextureNameBraceClearMask = 0xBFFFFFFF,
    kResourcePathCapacity = 0x400,
    kBitmapBasenameCapacity = 0x40,
    kArchiveEntryKeyCapacity = 0x104,
    kAsciiUpperToLower = 'a' - 'A'
};

// Resource/cache contracts:
// - Model/object vectors contain 16-byte {12-byte tagged key, value pointer}
//   entries. They own key allocations and manager-tracked values. Same names may
//   have different values; insertion deduplicates the exact (name,value) pair.
// - Tagged keys store inline bytes at +1 with length byte0&0x7F, or (bit31 set)
//   heap length at +4 and an owned pointer at +8. The tag is big-endian.
// - Model/font counters at object+0xC are tested BEFORE decrementing.
//   Release at count 1 only reaches zero; another release
//   while already zero erases/destroys it. Texture records instead die on --refs<=0.
// - Texture list header is {count, previous, next}; sentinel is manager+0xC.
//   Nodes are {previous,next,key,value}; value points to a separately allocated
//   ResourceTextureRecord followed by its normalized name. Texture IDs are
//   resource handles, not list indices. Lookup-only APIs return borrowed records.
// - Paths are copied without bounds checks: full paths must fit 0x400 bytes;
//   normalized basenames must fit 0x40 bytes, including the terminator.
// - Deleting destructors accept null; a positive flag also frees self.
//   Allocation helpers/guard type names are retained where cleanup relocations
//   depend on them. Volatile tags and duplicate cleanup layers preserve codegen.

// Both embedded hashes use the same block/free-list/bucket/mutex layout.
struct ResourceHashStorage {
    void* first;
    void* iterator;
    void* blocks;
    void* free;
    int blockNodeCount;
    unsigned mask;
    int bucketCount;
    void* buckets;
    char mutex[0x18];
};
struct BitmapDimensions {
    int format;
    int width;
    int height;
};
typedef char
    ManagedResourceHeader_reference_offset[(IFF_OFFSET(ManagedResourceHeader, referenceCount) == 0xC) ? 1 : -1];
typedef char
    ManagedResourceHeader_manager_offset[(IFF_OFFSET(ManagedResourceHeader, resourceManager) == 0x10) ? 1 : -1];
typedef char ResourceHashStorage_size[(sizeof(ResourceHashStorage) == 0x38) ? 1 : -1];
enum {
    kModelCacheOffset = IFF_OFFSET(ResourceManager, modelCacheVtable),
    kObjectCacheOffset = IFF_OFFSET(ResourceManager, objectCacheVtable)
};

void __dla__FPv(void*);
void appFreeUni__FPv(void*);
void* __dl__FPv(void*);

#pragma use_lmw_stmw on
struct WInstanceLock {
    void* lock;
    // Acquire the embedded recursive OS mutex for this scope.
    WInstanceLock(void* l) : lock(l) {
        OSLockMutex((OSMutex*)l);
    }
    // Release the mutex on normal return or exception unwinding.
    ~WInstanceLock() {
        OSUnlockMutex((OSMutex*)lock);
    }
};

struct ResourceDestructorInterface {
    virtual void destroy(int);
};
struct ResourceReleaseInterface {
    virtual void slot0();
    virtual void slot1();
    virtual void slot2();
    virtual void slot3();
    virtual void slot4();
    virtual void slot5();
    virtual void slot6();
    virtual void slot7();
    virtual void slot8();
    virtual void slot9();
    virtual void release(void*);
};
struct SizedResourceReleaseInterface {
    virtual void slot0();
    virtual void slot1();
    virtual void slot2();
    virtual void slot3();
    virtual void slot4();
    virtual void slot5();
    virtual void slot6();
    virtual void slot7();
    virtual void slot8();
    virtual void slot9();
    virtual void release(void*, int);
};
struct ResourceCleanupString {
    unsigned flags, length;
    void* data;
    // Release an owned heap key; inline bytes need no separate allocation cleanup.
    ~ResourceCleanupString() {
        if (flags >> 31)
            appFreeUni__FPv(data);
    }
};
struct ResourceCleanupEntry {
    ResourceCleanupString key;
    ResourceDestructorInterface* value;
};
struct TextureCleanupRecord {
    char pad[0x10];
    int refs;
    void* resource;
    int bytes;
};
struct TextureCleanupNode {
    TextureCleanupNode* prev;
    TextureCleanupNode* next;
    ResourceCleanupString key;
    TextureCleanupRecord* value;
};
struct ResourceHashLink {
    void* value;
    char pad[8];
    ResourceHashLink* prev;
    ResourceHashLink* next;
};
// Invoke deleting destruction on a nonnull manager-owned cached value.
inline void DestroyCachedResource(ResourceDestructorInterface* p) {
    if (p)
        p->destroy(1);
}
// Destroy object values, then their keys; retain the vector allocation.
inline void ClearCachedResources(ResourceCleanupEntry*& data, int& size) {
    ResourceCleanupEntry* it = data;
    int count;
    ResourceCleanupEntry* end;
    while ((end = data + (count = size)) != it) {
        if (it->value) {
            DestroyCachedResource(it->value);
            it->value = 0;
        }
        ++it;
    }
    size -= count;
    while (count != 0) {
        --end;
        end->~ResourceCleanupEntry();
        --count;
    }
}
// Advance the circular active-hash iterator while holding its mutex.
inline void* NextRegisteredHashValue(char* p) {
    void* lock = p + IFF_OFFSET(ResourceManager, hashMutex);
    OSLockMutex((OSMutex*)lock);
    ResourceHashLink* it = (*(ResourceHashLink**)&AsResourceManager(p)->hashIterator);
    if (it) {
        ResourceHashLink* next = it->next;
        void* result = it->value;
        if (next == (*(ResourceHashLink**)&AsResourceManager(p)->activeHashNodes))
            next = 0;
        (*(ResourceHashLink**)&AsResourceManager(p)->hashIterator) = next;
        OSUnlockMutex((OSMutex*)lock);
        return result;
    }
    OSUnlockMutex((OSMutex*)lock);
    return 0;
}
// Destroy model values and keys with the second cache's original iterator lifetimes.
inline void ClearCachedModels(ResourceCleanupEntry*& data, int& size) {
    // Keep the second vector's iterator and count lifetimes separate.
    int count;
    ResourceCleanupEntry* end;
    ResourceCleanupEntry* it = data;
    while ((end = data + (count = size)) != it) {
        if (it->value) {
            DestroyCachedResource(it->value);
            it->value = 0;
        }
        ++it;
    }
    size -= count;
    while (count != 0) {
        --end;
        end->~ResourceCleanupEntry();
        --count;
    }
}
// Detach one circular hash node under its mutex and return a surviving node.
inline ResourceHashLink* UnlinkHashNode(void* lock, ResourceHashLink* it) {
    OSLockMutex((OSMutex*)lock);
    it->next->prev = it->prev;
    it->prev->next = it->next;
    ResourceHashLink* next = it->next;
    if (next == it)
        next = 0;
    OSUnlockMutex((OSMutex*)lock);
    return next;
}
// Insert a hash node into a circular list under its mutex.
inline ResourceHashLink* LinkHashNode(ResourceHashLink* head, ResourceHashLink* it, void* lock) {
    OSLockMutex((OSMutex*)lock);
    if (!head) {
        it->prev = it;
        it->next = it;
        OSUnlockMutex((OSMutex*)lock);
        return it;
    }
    it->next = head;
    it->prev = head->prev;
    head->prev = it;
    it->prev->next = it;
    OSUnlockMutex((OSMutex*)lock);
    return head;
}
// Free hash allocation blocks and the bucket array under the embedded mutex.
inline void DestroyResourceHashStorage(char* p) {
    if (p) {
        WInstanceLock g(p + IFF_OFFSET(ResourceHashStorage, mutex));
        if (((ResourceHashStorage*)p)->blocks) {
            ResourceHashLink* it;
            ResourceHashLink* next;
            void* lock = p + IFF_OFFSET(ResourceHashStorage, mutex);
            while ((it = *(ResourceHashLink**)&((ResourceHashStorage*)p)->blocks) != 0) {
                OSLockMutex((OSMutex*)lock);
                it->next->prev = it->prev;
                it->prev->next = it->next;
                next = it->next;
                if (next == it)
                    next = 0;
                OSUnlockMutex((OSMutex*)lock);
                *(ResourceHashLink**)&((ResourceHashStorage*)p)->blocks = next;
                appFreeUni__FPv(it);
            }
        }
        if (((ResourceHashStorage*)p)->buckets) {
            appFreeUni__FPv(((ResourceHashStorage*)p)->buckets);
            ((ResourceHashStorage*)p)->buckets = 0;
        }
    }
}
// Destroy owned key strings and free the entry buffer through its original destructor layers.
inline void DestroyCacheEntries(ResourceCleanupEntry*& data, int& size) {
    // These guards correspond to the inlined container destructor layers.
    if (&data) {
        if (&data) {
            if (&data) {
                if (&data) {
                    if (data) {
                        ResourceCleanupEntry* end;
                        int count = size;
                        end = data + count;
                        size -= count;
                        while (count != 0) {
                            --end;
                            end->~ResourceCleanupEntry();
                            --count;
                        }
                        appFreeUni__FPv(data);
                    }
                }
            }
        }
    }
}
// Destroy an entry-vector subobject through its storage and count fields.
inline void DestroyCacheVector(char* p) {
    // Byte addresses retain the original inlined destructor's base-register lifetime.
    if (p) {
        if (p)
            DestroyCacheEntries(*(ResourceCleanupEntry**)(p + IFF_OFFSET(ResourceEntryVector, entries)),
                                *(int*)(p + IFF_OFFSET(ResourceEntryVector, count)));
    }
}
// Unlink the circular record range, then destroy and free its keyed nodes.
inline void DestroyTextureRecordList(char* p) {
    if (p) {
        if (p) {
            if (p) {
                if (p) {
                    TextureCleanupNode* it = (TextureCleanupNode*)((RecordListState*)p)->sentinel.next;
                    TextureCleanupNode* end = (TextureCleanupNode*)(p + IFF_OFFSET(RecordListState, sentinel));
                    if (it != end) {
                        TextureCleanupNode* last = end->prev;
                        it->prev->next = last->next;
                        last->next->prev = it->prev;
                        while (it != end) {
                            ResourceCleanupString* key = &it->key;
                            if (key)
                                key->~ResourceCleanupString();
                            TextureCleanupNode* old = it;
                            it = it->next;
                            appFreeUni__FPv(old);
                            --((RecordListState*)p)->count;
                        }
                    }
                }
            }
        }
    }
}
// Find the cleanup record whose resource handle matches the requested id.
inline TextureCleanupRecord* FindTextureCleanupRecord(char* p, int id, TextureCleanupNode* const& end) {
    TextureCleanupNode* search = (*(TextureCleanupNode**)&AsResourceManager(p)->firstRecord);
    while (search != end) {
        TextureCleanupRecord* obj = search->value;
        if (id == (int)obj->resource)
            return obj;
        search = search->next;
    }
    return 0;
}
// Release a texture reference, including owner storage and its list node when exhausted.
inline void ReleaseTextureCleanupRecord(char* p, void* id, TextureCleanupNode* const& end) {
    TextureCleanupRecord* obj = FindTextureCleanupRecord(p, (int)id, end);
    if (obj) {
        if (--obj->refs <= 0) {
            int bytes = obj->bytes;
            (*(int*)&AsResourceManager(p)->resourceBytes) -= bytes;
            (*(SizedResourceReleaseInterface**)p)->release(id, bytes);
            TextureCleanupNode* cur = (*(TextureCleanupNode**)&AsResourceManager(p)->firstRecord);
            TextureCleanupNode* finish = (TextureCleanupNode*)(p + IFF_OFFSET(ResourceManager, recordSentinel));
            while (cur != finish) {
                if (cur->value == obj) {
                    cur->prev->next = cur->next;
                    cur->next->prev = cur->prev;
                    ((ResourceCleanupEntry*)&cur->key)->~ResourceCleanupEntry();
                    appFreeUni__FPv(cur);
                    --AsResourceManager(p)->recordCount;
                    break;
                }
                cur = cur->next;
            }
            __dla__FPv(obj);
        }
    }
}
// Release owned resources, cached instances, texture records, hash nodes, and backing storage.
void* ResourceManager_dtor(void* self, int destroy) {
    if (self) {
        char* p = (char*)self;
        // Pending resource owned by the manager; release it through the manager's interface.
        void* resource = AsResourceManager(p)->ownedResource;
        if (resource)
            (*(ResourceReleaseInterface**)p)->release(resource);
        // Destroy cached values before their keys and storage disappear.
        ClearCachedResources((*(ResourceCleanupEntry**)&AsResourceManager(p)->cachedObjects),
                             (*(int*)&AsResourceManager(p)->cachedObjectCount));
        ClearCachedModels((*(ResourceCleanupEntry**)&AsResourceManager(p)->cachedModels),
                          (*(int*)&AsResourceManager(p)->cachedModelCount));
        TextureCleanupNode* it = (*(TextureCleanupNode**)&AsResourceManager(p)->firstRecord);
        TextureCleanupNode* end = (TextureCleanupNode*)(p + IFF_OFFSET(ResourceManager, recordSentinel));
        while (it != end) {
            ReleaseTextureCleanupRecord(p, it->value->resource, end);
            it = it->next;
        }
        // Delete active hash values while retaining the nodes for free-list recycling.
        void* value;
        {
            WInstanceLock g(p + IFF_OFFSET(ResourceManager, hashMutex));
            AsResourceManager(p)->hashIterator = AsResourceManager(p)->activeHashNodes;
            value = NextRegisteredHashValue(p);
        }
        if (value) {
            while (!same_pos(value, (void*)0)) {
                if (value)
                    __dl__FPv(value);
                value = NextRegisteredHashValue(p);
            }
        }
        {
            // Splice active nodes into the free ring, then clear bucket heads.
            WInstanceLock g(p + IFF_OFFSET(ResourceManager, hashMutex));
            if ((*(ResourceHashLink**)&AsResourceManager(p)->activeHashNodes) &&
                (*(ResourceHashLink**)&AsResourceManager(p)->freeHashNodes)) {
                ((*(ResourceHashLink**)&AsResourceManager(p)->activeHashNodes))->prev->next =
                    ((*(ResourceHashLink**)&AsResourceManager(p)->freeHashNodes))->next;
                ((*(ResourceHashLink**)&AsResourceManager(p)->freeHashNodes))->next->prev =
                    ((*(ResourceHashLink**)&AsResourceManager(p)->activeHashNodes))->prev;
                ((*(ResourceHashLink**)&AsResourceManager(p)->freeHashNodes))->next =
                    (*(ResourceHashLink**)&AsResourceManager(p)->activeHashNodes);
                ((*(ResourceHashLink**)&AsResourceManager(p)->activeHashNodes))->prev =
                    (*(ResourceHashLink**)&AsResourceManager(p)->freeHashNodes);
                (*(ResourceHashLink**)&AsResourceManager(p)->activeHashNodes) = 0;
            } else if ((*(ResourceHashLink**)&AsResourceManager(p)->activeHashNodes)) {
                ResourceHashLink* next;
                void* lock = p + IFF_OFFSET(ResourceManager, hashMutex);
                ResourceHashLink* node;
                while ((node = (*(ResourceHashLink**)&AsResourceManager(p)->activeHashNodes)) != 0) {
                    next = UnlinkHashNode(lock, node);
                    (*(ResourceHashLink**)&AsResourceManager(p)->freeHashNodes) =
                        LinkHashNode((*(ResourceHashLink**)&AsResourceManager(p)->freeHashNodes),
                                     (*(ResourceHashLink**)&AsResourceManager(p)->activeHashNodes), lock);
                    (*(ResourceHashLink**)&AsResourceManager(p)->activeHashNodes) = next;
                }
            }
            int i = 0;
            int offset = 0;
            while (i < AsResourceManager(p)->hashBucketCount) {
                *(void**)((char*)AsResourceManager(p)->hashBuckets + offset) = 0;
                ++i;
                offset += 4;
            }
        }
        // Values are gone; now release allocation blocks, bucket arrays, and keys.
        DestroyResourceHashStorage(p + IFF_OFFSET(ResourceManager, activeHashNodes));
        DestroyResourceHashStorage(p + IFF_OFFSET(ResourceManager, resourceHash));
        DestroyCacheVector(p + kObjectCacheOffset);
        DestroyCacheVector(p + kModelCacheOffset);
        DestroyTextureRecordList(p + IFF_OFFSET(ResourceManager, recordCount));
        if (destroy > 0)
            __dl__FPv(self);
    }
    return self;
}

struct ResourceDeleteInterface {
    virtual void destroy(int);
};

#pragma dont_inline on
// Resolve a path through the shared hash-path lookup.
void* CHashPath::FindPathForwarder(const char* path) {
    return FindPath(path);
}
#pragma dont_inline off

// Invoke the resource destructor with allocation deletion enabled.
void ResourceManager_DeleteResource(void*, void* object) {
    if (object != 0) {
        ((ResourceDeleteInterface*)object)->destroy(1);
    }
}

// Report whether a path is registered in the shared path table.
int CHashPath::Exists(const char* path) {
    return FindPath(path) != 0;
}

struct TextureLookupNode {
    char unk0[4];
    TextureLookupNode* next; // 0x4
    char unk8[12];
    void* obj; // 0x14
};

struct TextureLookupRecord {
    char unk0[20];
    int id; // 0x14
};

struct ResourceRecordNode {
    ResourceRecordNode* prev;
    ResourceRecordNode* next;
    unsigned flags;
    char reserved0C[4];
    void* payload;
    void* object;
};

struct RefCountedTextureRecord {
    char unk[0x10];
    int refs;
    int id;
    int size;
};

struct OwnedRecordKey {
    unsigned flags;
    char reserved04[4];
    void* payload;
    // Release the record node's owned tagged-string payload.
    inline ~OwnedRecordKey() {
        if (flags >> 31) {
            appFreeUni__FPv(payload);
        }
    }
};

struct ReleaseByIdInterface {
    virtual void v08();
    virtual void v0C();
    virtual void v10();
    virtual void v14();
    virtual void v18();
    virtual void v1C();
    virtual void v20();
    virtual void v24();
    virtual void v28();
    virtual void v2C();
    virtual void Release(int id);
};

// Borrow the first texture record matching a handle, or return null.
static inline RefCountedTextureRecord* FindResourceRecordById(void* self, int id) {
    ResourceRecordNode* cur = (*(ResourceRecordNode**)&AsResourceManager(self)->firstRecord);
    ResourceRecordNode* end = (ResourceRecordNode*)((char*)self + IFF_OFFSET(ResourceManager, recordSentinel));
    for (; cur != end; cur = cur->next) {
        RefCountedTextureRecord* obj = (RefCountedTextureRecord*)cur->object;
        if (id == obj->id) {
            return obj;
        }
    }
    return 0;
}

// Erase a matching list node/key; leave the separately allocated record to its caller.
static inline void RemoveResourceRecordNode(void* self, void* object) {
    ResourceRecordNode* node = (ResourceRecordNode*)((RecordListState*)self)->sentinel.next;
    ResourceRecordNode* end = (ResourceRecordNode*)&((RecordListState*)self)->sentinel;
    for (; node != end; node = node->next) {
        if (node->object == object) {
            node->prev->next = node->next;
            node->next->prev = node->prev;
            if ((OwnedRecordKey*)&node->flags != 0) {
                ((OwnedRecordKey*)&node->flags)->~OwnedRecordKey();
            }
            appFreeUni__FPv(node);
            (*(int*)self)--;
            return;
        }
    }
}

// Drop one texture reference and return its signed remaining count.
static inline int DecrementTextureReference(RefCountedTextureRecord* object) {
    return --object->refs;
}

// Drop a texture-record reference and release its resource and record when exhausted.
void ResourceManager_ReleaseId(void* self, int id) {
    RefCountedTextureRecord* object;
    object = FindResourceRecordById(self, id);
    if (object != 0) {
        if (DecrementTextureReference(object) <= 0) {
            (*(int*)&AsResourceManager(self)->resourceBytes) -= object->size;
            (*(ReleaseByIdInterface**)self)->Release(id);
            RemoveResourceRecordNode((char*)self + 8, object);
            __dla__FPv(object);
        }
    }
}

struct CachedObjectReference {
    char unk[0xc];
    int refs;
};

struct CachedObjectEntry {
    char unk[0xc];
    void* object;
};

struct Vtable3 {
    // Preserve the weak no-op destructor slot used by cached-object ABI views.
    virtual void dispose(int) {}
};

// Return 1 after decrementing a nonzero external count, 0 if already unreferenced.
static inline int DropCachedObjectReference(CachedObjectReference* object) {
    if (object->refs == 0)
        return 0;
    object->refs--;
    return 1;
}

// Drop a cached-object reference; erase and destroy an already-unreferenced object.
void ResourceManager_ReleaseCachedObject(void* self, void* arg) {
    CachedObjectEntry* begin = (*(CachedObjectEntry**)&AsResourceManager(self)->cachedObjects);
    unsigned count = AsResourceManager(self)->cachedObjectCount;
    CachedObjectEntry* end = begin + count;
    CachedObjectEntry* entry = begin;
    while (end != entry) {
        if (entry->object == arg)
            break;
        entry = (CachedObjectEntry*)((char*)entry + sizeof(CachedObjectEntry));
    }
    if (end != entry) {
        if (entry->object != 0) {
            CachedObjectReference* object = (CachedObjectReference*)entry->object;
            int retained = DropCachedObjectReference(object);
            if (retained == 0) {
                if (entry != 0 && entry != 0 && (*(unsigned*)entry >> 31)) {
                    appFreeUni__FPv((*(void**)&AsMWStringStorage(entry)->data));
                }
                unsigned currentCount = AsResourceManager(self)->cachedObjectCount;
                char* currentBase = (*(char**)&AsResourceManager(self)->cachedObjects);
                char* currentEnd = currentBase + (currentCount << 4);
                long remaining = (currentEnd - (char*)entry) / 16;
                memmove(entry, (char*)entry + sizeof(CachedObjectEntry), (remaining - 1) << 4);
                (AsResourceManager(self)->cachedObjectCount)--;
                if (arg != 0) {
                    if (arg != 0) {
                        ((Vtable3*)arg)->dispose(1);
                    }
                }
            }
        }
    }
}

// Apply the model cache's decrement-or-erase decision (zero is tested before decrement).
static inline int DropCachedModelReference(CachedObjectReference* object) {
    if (object->refs == 0)
        return 0;
    object->refs--;
    return 1;
}

// Drop a cached-model reference; erase and destroy an already-unreferenced model.
void ResourceManager_ReleaseObject(void* self, void* arg) {
    if (arg != 0) {
        CachedObjectEntry* begin = (*(CachedObjectEntry**)&AsResourceManager(self)->cachedModels);
        unsigned count = AsResourceManager(self)->cachedModelCount;
        CachedObjectEntry* end = begin + count;
        CachedObjectEntry* entry = begin;
        while (end != entry) {
            if (entry->object == arg)
                break;
            entry = (CachedObjectEntry*)((char*)entry + sizeof(CachedObjectEntry));
        }
        if (end != entry) {
            if (entry->object != 0) {
                CachedObjectReference* object = (CachedObjectReference*)entry->object;
                int retained = DropCachedModelReference(object);
                if (retained == 0) {
                    if (entry != 0 && entry != 0 && (*(unsigned*)entry >> 31)) {
                        appFreeUni__FPv((*(void**)&AsMWStringStorage(entry)->data));
                    }
                    unsigned currentCount = AsResourceManager(self)->cachedModelCount;
                    char* currentBase = (*(char**)&AsResourceManager(self)->cachedModels);
                    char* currentEnd = currentBase + (currentCount << 4);
                    long remaining = (currentEnd - (char*)entry) / 16;
                    memmove(entry, (char*)entry + sizeof(CachedObjectEntry), (remaining - 1) << 4);
                    (AsResourceManager(self)->cachedModelCount)--;
                    if (arg != 0) {
                        if (arg != 0) {
                            ((Vtable3*)arg)->dispose(1);
                        }
                    }
                }
            }
        }
    }
}

// Find a texture record by its resource handle, returning null if absent.
void* ObjectList_FindById(void* self, int id) {
    TextureLookupNode* end = (TextureLookupNode*)((char*)self + IFF_OFFSET(ResourceManager, recordSentinel));
    TextureLookupNode* cur = (*(TextureLookupNode**)&AsResourceManager(self)->firstRecord);
    for (; cur != end; cur = cur->next) {
        void* obj = cur->obj;
        if (id == ((TextureLookupRecord*)obj)->id) {
            return obj;
        }
    }
    return 0;
}

// Return the cached texture rectangle width for a resource handle, or zero.
int ResourceManager_GetBitmapWidth(void* self, int id) {
    TextureLookupNode* cur = (*(TextureLookupNode**)&AsResourceManager(self)->firstRecord);
    TextureLookupNode* end = (TextureLookupNode*)((char*)self + IFF_OFFSET(ResourceManager, recordSentinel));
    for (; cur != end; cur = cur->next) {
        void* obj = cur->obj;
        if (id == (*(int*)&AsResourceTextureRecord(obj)->resource)) {
            int left = AsResourceTextureRecord(obj)->left;
            int right = AsResourceTextureRecord(obj)->right;
            return right - left;
        }
    }
    return 0;
}

// Return the cached texture rectangle height for a resource handle, or zero.
int ResourceManager_GetBitmapHeight(void* self, int id) {
    TextureLookupNode* cur = (*(TextureLookupNode**)&AsResourceManager(self)->firstRecord);
    TextureLookupNode* end = (TextureLookupNode*)((char*)self + IFF_OFFSET(ResourceManager, recordSentinel));
    for (; cur != end; cur = cur->next) {
        void* obj = cur->obj;
        if (id == (*(int*)&AsResourceTextureRecord(obj)->resource)) {
            int top = AsResourceTextureRecord(obj)->top;
            int bottom = AsResourceTextureRecord(obj)->bottom;
            return bottom - top;
        }
    }
    return 0;
}

void WTitleFont_Ctor(void*);
void* __nw__FUl(unsigned long);
void appFreeUni__FPv(void*);

void* WView_ctor(void*);
extern char sharedResourceLoaderStrings[];
struct ResourceLoaderStringPool {
    char files_of_the_same_name_txt_0[27];                     // +0x0
    char viewToken[4];                                         // +0x1B
    char petExtension[5];                                      // +0x1F
    char mpetExtension[6];                                     // +0x24
    char apetExtension[6];                                     // +0x2A
    char bpetExtension[6];                                     // +0x30
    char gmrExtension[5];                                      // +0x36
    char vectorLengthError[20];                                // +0x3B
    char lengthErrorType[55];                                  // +0x4F
    char defaultFont[9];                                       // +0x86
    char bresExtension[6];                                     // +0x8F
    char gsrExtension[5];                                      // +0x95
    char unknownName[8];                                       // +0x9A
    char basic_string_reserve_length_error_A2[35];             // +0xA2
    char basic_string_out_of_range_C5[27];                     // +0xC5
    char std_exception_std_logic_error_std_out_of_rang_E0[55]; // +0xE0
    char basic_string_length_error_117[27];                    // +0x117
    char padding_132[2];                                       // +0x132
};

char* CString_Compare(char*, const char*);
int ResourceCache_RetainOrAppend(void*, const char*, void*);
void* ResourceEntryVector_FindByName(void*, const char*);
void Renderable_SetResourceManager(void*, void*);
extern "C" void __ct__10CFileAsyncFPCci(void*, void*, int);
void WPuppet_SetOpacityRecursive(void*, int);
void WPuppet_RefreshMatrixBuffers(void*);
void* Pointer_GetFirstPointer(void*);
void WPuppet_ctor(void*);
void Path_CopyWithForwardSlashes(void*, char*, const char*);

struct PuppetObjectStorage {
    char unk[0x23c];
    // Construct a puppet inside its ABI-sized allocation.
    PuppetObjectStorage() {
        WPuppet_ctor(this);
    }
};

struct ModelResourceInterface {
    virtual void slot0(int);
    virtual void* slot1(void*);
    virtual int slot2(const char*, void*, int);
    virtual void* slot3(void*);
    virtual int slot4(const char*, void*, int);
    virtual void slot5();
    virtual void slot6();
    virtual void slot7(int, int, int*);
    virtual void slot8();
    virtual void slot9(int, int, int*);
};

// Resolve, reuse/clone, or load a model using its extension-selected format flags.
// Returns a manager-tracked instance or null. loadOptions/cloneOptions are opaque
// arguments passed to loader/clone slots. instance is an optional input to the
// existing allocation policy, not a guaranteed destination: the condition below
// may replace it. Loading failure deletes the selected instance. Paths stay borrowed.
void* ResourceManager_LoadByExtension(char* self, const char* filename, void* loadOptions, void* cloneOptions,
                                      void* instance) {
    // Normalize separators, discard the first path component, and select file format.
    char path[kResourcePathCapacity];
    Path_CopyWithForwardSlashes(self, path, filename);
    char* pathAfterFirstSlash = CString::FindCharForwarder(path, '/');
    if (pathAfterFirstSlash != 0)
        pathAfterFirstSlash = CString::FindCharForwarder(path, '/') + 1;
    else
        pathAfterFirstSlash = path;

    char cacheKey[kResourcePathCapacity];
    strcpy(cacheKey, pathAfterFirstSlash);
    char* extension = CString::FindLastCharForwarder(cacheKey, '.');
    if (extension == 0)
        return 0;

    int format;
    if (CString_Compare(extension, sharedResourceLoaderStrings + IFF_OFFSET(ResourceLoaderStringPool, petExtension)) ==
        0)
        format = kFormatPet;
    else if (CString_Compare(extension,
                             sharedResourceLoaderStrings + IFF_OFFSET(ResourceLoaderStringPool, mpetExtension)) == 0)
        format = kFormatMpet;
    else if (CString_Compare(extension,
                             sharedResourceLoaderStrings + IFF_OFFSET(ResourceLoaderStringPool, apetExtension)) == 0)
        format = kFormatApet;
    else if (CString_Compare(extension,
                             sharedResourceLoaderStrings + IFF_OFFSET(ResourceLoaderStringPool, bpetExtension)) == 0)
        format = kFormatBpet;
    else if (CString_Compare(extension,
                             sharedResourceLoaderStrings + IFF_OFFSET(ResourceLoaderStringPool, gmrExtension)) == 0)
        format = kFormatGmr;
    else
        return 0;

    // Cached APET reuses its instance; the other formats clone before registration.
    void* cachedModel = ResourceEntryVector_FindByName(self + kModelCacheOffset, cacheKey);
    if (cachedModel != 0) {
        if (format == kFormatApet) {
            ResourceCache_RetainOrAppend(self + kModelCacheOffset, cacheKey, cachedModel);
            return cachedModel;
        }
        instance = ((ModelResourceInterface*)cachedModel)->slot1(cloneOptions);
        Renderable_SetResourceManager(instance, self);
        WPuppet_SetOpacityRecursive(instance, kOpaque);
        ResourceCache_RetainOrAppend(self + kModelCacheOffset, cacheKey, instance);
        if (format == kFormatPet)
            WPuppet_RefreshMatrixBuffers(instance);
        return instance;
    }

    // Cache miss: query allocation policy, load the selected instance, then cache it.
    int reusePolicy = 0;
    void* ownerInterface = Pointer_GetFirstPointer(self);
    ((ModelResourceInterface*)ownerInterface)->slot7(0x1f, 2, &reusePolicy);
    // Original 0x80060F04: create unless all four exclusion tests hold.
    if (instance != 0 || reusePolicy == 0 || format == kFormatApet || format == kFormatMpet) {
        PuppetObjectStorage* created = new PuppetObjectStorage();
        instance = created;
    }
    Renderable_SetResourceManager(instance, self);
    if (((ModelResourceInterface*)instance)->slot2(path, loadOptions, format) == 0) {
        WPuppet_SetOpacityRecursive(instance, kOpaque);
        ResourceCache_RetainOrAppend(self + kModelCacheOffset, cacheKey, instance);
        if (format == kFormatPet)
            WPuppet_RefreshMatrixBuffers(instance);
        return instance;
    }
    if (instance != 0) {
        ((ModelResourceInterface*)instance)->slot0(1);
    }
    return 0;
}

struct AsyncModelFileStorage {
    char unk[0x10];
    // Start an asynchronous file object from the borrowed resolved path.
    AsyncModelFileStorage(void* value) {
        __ct__10CFileAsyncFPCci(this, value, 0);
    }
};

struct CachedModelCloneInterface {
    virtual void slot0();
    virtual void* slot1(int);
};

#pragma auto_inline off
// Prepare a cached model or start an asynchronous read for the requested model path.
// Return 0 for a missing/unsupported suffix, 1 when prepared/requested. One pending
// load occupies manager state; callers must finish/clean up the previous request.
// Unlike LoadByExtension, this lookup does not normalize backslashes first.
int ResourceManager_RequestModelLoad(char* self, const char* filename) {
    void** object = &AsResourceManager(self)->resourceData;
    if (*object != 0) {
        __dla__FPv(*object);
        *object = 0;
    }

    // Drop old read bytes, then derive the cache key after the first slash.
    char path[0x400];
    char* slash = strchr(filename, '/');
    char* src;
    if (slash != 0) {
        slash = strchr(filename, '/');
        src = slash + 1;
    } else {
        src = (char*)filename;
    }
    strcpy(path, src);

    char* extension = CString::FindLastCharForwarder(path, '.');
    if (extension == 0)
        return 0;

    int* format = &AsResourceManager(self)->loadFormat;
    if (CString_Compare(extension, sharedResourceLoaderStrings + IFF_OFFSET(ResourceLoaderStringPool, petExtension)) ==
        0)
        *format = kFormatPet;
    else if (CString_Compare(extension,
                             sharedResourceLoaderStrings + IFF_OFFSET(ResourceLoaderStringPool, mpetExtension)) == 0)
        *format = kFormatMpet;
    else if (CString_Compare(extension,
                             sharedResourceLoaderStrings + IFF_OFFSET(ResourceLoaderStringPool, apetExtension)) == 0)
        *format = kFormatApet;
    else if (CString_Compare(extension,
                             sharedResourceLoaderStrings + IFF_OFFSET(ResourceLoaderStringPool, bpetExtension)) == 0)
        *format = kFormatBpet;
    else if (CString_Compare(extension,
                             sharedResourceLoaderStrings + IFF_OFFSET(ResourceLoaderStringPool, gmrExtension)) == 0)
        *format = kFormatGmr;
    else
        return 0;

    strncpy(AsResourceManager(self)->loadPath, path, sizeof(AsResourceManager(self)->loadPath));
    // Cache hits publish state 5; misses publish an async file and state 1.
    void* value = ResourceEntryVector_FindByName(self + kModelCacheOffset, path);
    if (value != 0) {
        int cachedFormat = *format;
        AsResourceManager(self)->loadState = kCachedModelReady;
        if (cachedFormat == kFormatMpet || cachedFormat == kFormatApet) {
            ResourceCache_RetainOrAppend(self + kModelCacheOffset, path, value);
            AsResourceManager(self)->loadedModel = value;
            return 1;
        }
        void* result = ((CachedModelCloneInterface*)value)->slot1(0);
        AsResourceManager(self)->loadedModel = result;
        Renderable_SetResourceManager(result, self);
        WPuppet_SetOpacityRecursive(AsResourceManager(self)->loadedModel, kOpaque);
        ResourceCache_RetainOrAppend(self + kModelCacheOffset, path, AsResourceManager(self)->loadedModel);
        if (*format == kFormatPet)
            WPuppet_RefreshMatrixBuffers(AsResourceManager(self)->loadedModel);
    } else {
        AsyncModelFileStorage* async = new AsyncModelFileStorage(((CHashPath*)self)->FindPathForwarder(filename));
        AsResourceManager(self)->asyncRead = async;
        AsResourceManager(self)->loadState = kWaitingForOpen;
    }
    return 1;
}

struct TitleFontObjectStorage {
    char unk[0xe4];
    // Construct a title font inside its ABI-sized allocation.
    TitleFontObjectStorage() {
        WTitleFont_Ctor(this);
    }
};

struct ViewObjectStorage {
    char unk[0x1ab8];
    // Construct a view inside its ABI-sized allocation.
    ViewObjectStorage() {
        WView_ctor(this);
    }
};

// Allocate a title font and associate it with its resource manager.
void* ResourceManager_CreateWTitleFont(void* arg0) {
    TitleFontObjectStorage* p = new TitleFontObjectStorage();
    ((ManagedResourceHeader*)p)->resourceManager = arg0;
    return p;
}

void Path_CopyWithForwardSlashes(void* self, char* dest, const char* src) {
    strcpy(dest, src);
    char* p;
    while ((p = strchr(dest, '\\')) != 0) {
        *p = '/';
    }
}

// Destroy heap-owned string storage and optionally delete the string object.
void* MWStringStorage_dtor(void* self, int flag) {
    if (self != 0) {
        if (self != 0) {
            if (*(unsigned*)self >> 31) {
                appFreeUni__FPv((*(void**)&AsMWStringStorage(self)->data));
            }
        }
        if (flag > 0) {
            __dl__FPv(self);
        }
    }
    return self;
}

// Destroy heap-owned string storage and optionally delete the string object.
void* dtor_80065054(void* self, int flag) {
    if (self != 0) {
        if (self != 0) {
            if (*(unsigned*)self >> 31) {
                appFreeUni__FPv((*(void**)&AsMWStringStorage(self)->data));
            }
        }
        if (flag > 0) {
            __dl__FPv(self);
        }
    }
    return self;
}

// Destroy heap-owned string storage and optionally delete the string object.
void* dtor_80065934(void* self, int flag) {
    if (self != 0) {
        if (self != 0) {
            if (*(unsigned*)self >> 31) {
                appFreeUni__FPv((*(void**)&AsMWStringStorage(self)->data));
            }
        }
        if (flag > 0) {
            __dl__FPv(self);
        }
    }
    return self;
}

// Create a view with the supplied target slot.
void* ResourceManager_CreateWView(void* self, void* arg) {
    if (arg != 0) {
        strstr((const char*)arg, sharedResourceLoaderStrings + IFF_OFFSET(ResourceLoaderStringPool, viewToken));
    }
    ViewObjectStorage* p = new ViewObjectStorage();
    AsWView(p)->targetSlot = self;
    return p;
}

int WOverlay_Load(void*, void*, void*);

struct BitmapSpriteStorage {
    char unk[0x12c];
    // Construct a bitmap sprite before attaching its resource manager.
    BitmapSpriteStorage() {
        ((W3dSpr*)this)->Init();
    }
};

// Keep the ABI type name of this emitted weak virtual stub.
struct Fn80061CACVtable {
    // Retain the original weak no-op deleting-destructor slot.
    virtual void dispose(int) {}
};
typedef Fn80061CACVtable BitmapSpriteDeleteInterface;

struct OverlayObjectStorage {
    char unk[0x30];
    // Construct an overlay before optional texture loading.
    OverlayObjectStorage() {
        ((WOverlay*)this)->__ct();
    }
};

// Keep the ABI type name of this emitted weak virtual stub.
struct Fn800604A4Vtable {
    // Retain the original weak no-op deleting-destructor slot.
    virtual void dispose(int) {}
};
typedef Fn800604A4Vtable OverlayDeleteInterface;

// Create a bitmap sprite; destroy it if bitmap loading fails.
void* W3dSpr_Create(void* self, void* arg1, void* arg2) {
    BitmapSpriteStorage* p = new BitmapSpriteStorage();
    ((ManagedResourceHeader*)p)->resourceManager = self;
    if (((W3dSpr*)p)->LoadSprite(arg1, arg2) == 0) {
        return p;
    }
    if (p != 0) {
        ((BitmapSpriteDeleteInterface*)p)->dispose(1);
    }
    return 0;
}

// Create an overlay and optionally load its texture, cleaning up on failure.
void* WOverlay_Create(void* self, void* arg1, void* arg2) {
    OverlayObjectStorage* p = new OverlayObjectStorage();
    ((ManagedResourceHeader*)p)->resourceManager = self;
    if (arg1 != 0) {
        if (WOverlay_Load(p, arg1, arg2) == 0) {
        } else {
            if (p != 0) {
                ((OverlayDeleteInterface*)p)->dispose(1);
            }
            p = 0;
        }
    }
    return p;
}

// Keep the ABI type name of this emitted weak virtual stub.
struct Fn800632B0Vtable {
    // Retain the original weak no-op deleting-destructor slot.
    virtual void dispose(int) {}
};
typedef Fn800632B0Vtable CachedModelDeleteInterface;

#pragma push
#pragma use_lmw_stmw on
// Destroy every cached model, then clear keys while retaining vector storage.
void ResourceManager_ClearCachedModels(void* self) {
    unsigned count;
    char* end;
    char* p;
    void* obj;
    int n;
    char* cur;

    cur = (*(char**)&AsResourceManager(self)->cachedModels);

    while (end = (*(char**)&AsResourceManager(self)->cachedModels) +
                 ((count = AsResourceManager(self)->cachedModelCount) << 4),
           !(end == cur)) {
        obj = *(void**)(cur + 0xc);
        if (obj != 0) {
            if (obj != 0) {
                ((CachedModelDeleteInterface*)obj)->dispose(1);
            }
            *(void**)(cur + 0xc) = (void*)(0);
        }
        cur += 0x10;
    }
    n = count;
    p = end;
    AsResourceManager(self)->cachedModelCount = count - n;
    count = n;
    while (n) {
        p = p - 0x10;
        if (p != 0) {
            if (p != 0) {
                if (*(unsigned*)p >> 31) {
                    appFreeUni__FPv((*(void**)&AsMWStringStorage(p)->data));
                }
            }
        }
        n = n - 1;
    }
}
#pragma pop

extern "C++" {
namespace std {
// The cache entry vectors hold 16-byte entries whose key string is destroyed with the entry.
template <> struct __vec_destroys_elements<ResourceCleanupEntry> {
    enum {
        value = 1
    };
    static void destroy(ResourceCleanupEntry* e) {
        if (e != 0)
            if (e != 0)
                if (e->key.flags >> 31)
                    appFreeUni__FPv(e->key.data);
    }
};
}
typedef std::vector<ResourceCleanupEntry, app::app_allocator<ResourceCleanupEntry> > CacheEntryVector;
}
// Destroy cached string keys and free the entry-vector allocation; free the vector when requested.
void* dtor_800641E8(void* self, int flag) {
    return std::__vector_deleting_dtor((CacheEntryVector*)self, flag);
}

// Destroy cached string keys and free the entry-vector allocation; free the vector when requested.
void* dtor_80064448(void* self, int flag) {
    return std::__vector_deleting_dtor((CacheEntryVector*)self, flag);
}

void W3dAniSpr_Ctor(void*);
int W3dAniSpr_LoadSpritesInOneTexture(void*, void*, void*, float, float);

struct AnimatedSpriteStorage {
    char unk[0x19c];
    // Construct an animated sprite inside its ABI-sized allocation.
    AnimatedSpriteStorage() {
        W3dAniSpr_Ctor(this);
    }
};

struct AnimatedSpriteDeleteInterface {
    virtual void dispose(int);
    char unk[0x198];
};

// Create an animated bitmap-grid sprite and clean up if its grid load fails.
void* W3dAniSpr_Create(void* self, float x, float y, void* arg4, void* arg5) {
    AnimatedSpriteStorage* p = new AnimatedSpriteStorage();
    ((ManagedResourceHeader*)p)->resourceManager = self;
    if (W3dAniSpr_LoadSpritesInOneTexture(p, arg4, arg5, x, y) == 0) {
        return p;
    }
    if (p != 0) {
        ((AnimatedSpriteDeleteInterface*)p)->dispose(1);
    }
    return 0;
}

struct CacheVectorStorage {
    void* data;
    unsigned count;
};

struct CacheVectorOwner {
    char pad[4];
    CacheVectorStorage vec;
};

// Destroy the embedded string-entry vector and optionally delete its owner.
void* ResourceCache_DestroyEmbeddedEntryVector_800642AC(CacheVectorOwner* self, int flag) {
    if (self != 0) {
        if (&self->vec != 0) {
            if (&self->vec != 0) {
                if (&self->vec != 0) {
                    if (&self->vec != 0) {
                        if (self->vec.data != 0) {
                            char* item;
                            unsigned count = self->vec.count;
                            unsigned zero = (unsigned)count - count;
                            unsigned shift = count << 4;
                            item = (char*)self->vec.data + shift;
                            self->vec.count = zero;
                            while (count != 0) {
                                item -= 0x10;
                                if (item != 0) {
                                    if (item != 0) {
                                        if (*(unsigned*)item >> 31) {
                                            appFreeUni__FPv((*(void**)&AsMWStringStorage(item)->data));
                                        }
                                    }
                                }
                                count--;
                            }
                            appFreeUni__FPv(self->vec.data);
                        }
                    }
                }
            }
        }
        if (flag > 0) {
            __dl__FPv(self);
        }
    }
    return self;
}

// Destroy the embedded string-entry vector and optionally delete its owner.
void* ResourceCache_DestroyEmbeddedEntryVector_8006450C(CacheVectorOwner* self, int flag) {
    if (self != 0) {
        if (&self->vec != 0) {
            if (&self->vec != 0) {
                if (&self->vec != 0) {
                    if (&self->vec != 0) {
                        if (self->vec.data != 0) {
                            char* item;
                            unsigned count = self->vec.count;
                            unsigned zero = (unsigned)count - count;
                            unsigned shift = (unsigned)count << 4;
                            item = (char*)self->vec.data + shift;
                            self->vec.count = zero;
                            while (count != 0) {
                                item -= 0x10;
                                if (item != 0) {
                                    if (item != 0) {
                                        if (*(unsigned*)item >> 31) {
                                            appFreeUni__FPv((*(void**)&AsMWStringStorage(item)->data));
                                        }
                                    }
                                }
                                count--;
                            }
                            appFreeUni__FPv(self->vec.data);
                        }
                    }
                }
            }
        }
        if (flag > 0) {
            __dl__FPv(self);
        }
    }
    return self;
}

// Destroy a wrapped string-entry vector, retaining the original destructor layers.
void* StringVector_dtor(CacheVectorOwner* self, int flag) {
    if (self != 0) {
        if (self != 0) {
            if (&self->vec != 0) {
                if (&self->vec != 0) {
                    if (&self->vec != 0) {
                        if (&self->vec != 0) {
                            if (self->vec.data != 0) {
                                char* item;
                                unsigned count = self->vec.count;
                                unsigned zero = (unsigned)count - count;
                                unsigned shift = count << 4;
                                item = (char*)self->vec.data + shift;
                                self->vec.count = zero;
                                while (count != 0) {
                                    item -= 0x10;
                                    if (item != 0) {
                                        if (item != 0) {
                                            if (*(unsigned*)item >> 31) {
                                                appFreeUni__FPv((*(void**)&AsMWStringStorage(item)->data));
                                            }
                                        }
                                    }
                                    count--;
                                }
                                appFreeUni__FPv(self->vec.data);
                            }
                        }
                    }
                }
            }
        }
        if (flag > 0) {
            __dl__FPv(self);
        }
    }
    return self;
}

// Destroy a wrapped string-entry vector, retaining the original destructor layers.
void* CacheVectorOwner_dtor(CacheVectorOwner* self, int flag) {
    if (self != 0) {
        if (self != 0) {
            if (&self->vec != 0) {
                if (&self->vec != 0) {
                    if (&self->vec != 0) {
                        if (&self->vec != 0) {
                            if (self->vec.data != 0) {
                                char* item;
                                unsigned count = self->vec.count;
                                unsigned zero = (unsigned)count - count;
                                unsigned shift = count << 4;
                                item = (char*)self->vec.data + shift;
                                self->vec.count = zero;
                                while (count != 0) {
                                    item -= 0x10;
                                    if (item != 0) {
                                        if (item != 0) {
                                            if (*(unsigned*)item >> 31) {
                                                appFreeUni__FPv((*(void**)&AsMWStringStorage(item)->data));
                                            }
                                        }
                                    }
                                    count--;
                                }
                                appFreeUni__FPv(self->vec.data);
                            }
                        }
                    }
                }
            }
        }
        if (flag > 0) {
            __dl__FPv(self);
        }
    }
    return self;
}

#pragma push
#pragma use_lmw_stmw on

// Unlink and free hash nodes and backing storage under the hash mutex.
void* dtor_8006599C(char* self, int flag) {
    if (self != 0) {
        {
            // Phase 1: release allocation blocks while the recursive hash mutex is held.
            WInstanceLock tableLock(self + IFF_OFFSET(ResourceHashStorage, mutex));
            ResourceHashLink* nextBlock;
            void* hashMutex;
            ResourceHashLink* block;
            if (((ResourceHashStorage*)self)->blocks != 0) {
                hashMutex = self + IFF_OFFSET(ResourceHashStorage, mutex);
                while ((block = (ResourceHashLink*)((ResourceHashStorage*)self)->blocks) != 0) {
                    OSLockMutex((OSMutex*)hashMutex);
                    block->next->prev = block->prev;
                    block->prev->next = block->next;
                    nextBlock = block->next;
                    if (nextBlock == block)
                        nextBlock = 0;
                    OSUnlockMutex((OSMutex*)hashMutex);
                    ((ResourceHashStorage*)self)->blocks = nextBlock;
                    appFreeUni__FPv(block);
                }
            }
            // Phase 2: release the bucket-pointer array under the same outer lock.
            void* bucketStorage = ((ResourceHashStorage*)self)->buckets;
            if (bucketStorage != 0) {
                appFreeUni__FPv(bucketStorage);
                ((ResourceHashStorage*)self)->buckets = 0;
            }
        }
        if (flag > 0) {
            __dl__FPv(self);
        }
    }
    return self;
}

// Unlink and free hash nodes and backing storage under the hash mutex.
void* ResourceHashStorage_dtor(char* self, int flag) {
    if (self != 0) {
        {
            // Phase 1: release allocation blocks while the recursive hash mutex is held.
            WInstanceLock tableLock(self + IFF_OFFSET(ResourceHashStorage, mutex));
            ResourceHashLink* nextBlock;
            void* hashMutex;
            ResourceHashLink* block;
            if (((ResourceHashStorage*)self)->blocks != 0) {
                hashMutex = self + IFF_OFFSET(ResourceHashStorage, mutex);
                while ((block = (ResourceHashLink*)((ResourceHashStorage*)self)->blocks) != 0) {
                    OSLockMutex((OSMutex*)hashMutex);
                    block->next->prev = block->prev;
                    block->prev->next = block->next;
                    nextBlock = block->next;
                    if (nextBlock == block)
                        nextBlock = 0;
                    OSUnlockMutex((OSMutex*)hashMutex);
                    ((ResourceHashStorage*)self)->blocks = nextBlock;
                    appFreeUni__FPv(block);
                }
            }
            // Phase 2: release the bucket-pointer array under the same outer lock.
            void* bucketStorage = ((ResourceHashStorage*)self)->buckets;
            if (bucketStorage != 0) {
                appFreeUni__FPv(bucketStorage);
                ((ResourceHashStorage*)self)->buckets = 0;
            }
        }
        if (flag > 0) {
            __dl__FPv(self);
        }
    }
    return self;
}

#pragma pop

void* Storage_ClearThreeWords(void*);
void NString_AssignCString(void*, void*);
void* String_GetCharPtrAt(void*, int);
void String_ReserveCapacity(void*, int);
void* __nwa__FUl(unsigned);
void* appMallocDummy__FUl(unsigned);
void String_ReplaceRange(void*, int, int, void*, void*, void*);
void appFreeUni__FPv(void*);
void __dt__Q23std12length_errorFv(void*);
extern char __vt__Q23std11logic_error[];
extern char s_sharedCharDeleterVtable[];
extern char __vt__Q23std12length_error[];

void* FixedFontCache_GetOrCreate(void*, void*);
void Renderable_SetResourceManager(void*, void*);
int WFixedFont_LoadMessageResources(void*, void*);
int FixedFontCache_RetainOrAppend(void*, void*, void*);
void WFixedFont_Ctor(void*);
// Destroy string keys and nodes in the circular resource-record list.
void* RecordList_dtor(void* self, int flag) {
    if (self != 0) {
        if (self != 0) {
            if (self != 0) {
                if (self != 0) {
                    void* cur = ((RecordListState*)self)->sentinel.next;
                    if (cur != (char*)&((RecordListState*)self)->sentinel) {
                        void* a = *(void**)((char*)&((RecordListState*)self)->sentinel);
                        unsigned t = *(unsigned*)&((RecordListLinks*)a)->next;
                        *(unsigned*)&((RecordListLinks*)((RecordListLinks*)cur)->previous)->next = t;
                        t = *(unsigned*)cur;
                        *(unsigned*)((RecordListLinks*)a)->next = t;
                        void* last = (char*)&((RecordListState*)self)->sentinel;
                        while (cur != last) {
                            void* p = (char*)&((TextureCleanupNode*)cur)->key;
                            if (p != 0) {
                                if (p != 0) {
                                    if (*(unsigned*)p >> 31) {
                                        appFreeUni__FPv((*(void**)&AsMWStringStorage(p)->data));
                                    }
                                }
                            }
                            void* node = cur;
                            cur = ((RecordListLinks*)cur)->next;
                            appFreeUni__FPv(node);
                            (*(int*)self)--;
                        }
                    }
                }
            }
        }
        if (flag > 0) {
            __dl__FPv(self);
        }
    }
    return self;
}

struct FixedFontVtable {
    void* slot0;
    void* slot1;
    void (*dispose)(void*, int);
};
struct FixedFontDeleteInterface {
    virtual void dispose(int);
};
struct FixedFontObjectStorage {
    char unk[0xb8];
    // Construct a fixed font before loading its message resources.
    FixedFontObjectStorage() {
        WFixedFont_Ctor(this);
    }
};

// Keep this ABI type name: exception cleanup relocations reference its destructor.
struct Fn80061BA8Local {
    char unk[0x10];
    // Initialize empty temporary storage for the default-font key.
    Fn80061BA8Local() {
        Storage_ClearThreeWords(this);
    }
    // Release the temporary key through the canonical string destructor.
    ~Fn80061BA8Local() {
        ((NString*)(this))->~NString();
    }
};
typedef Fn80061BA8Local DefaultFontNameStorage;

// Reuse a cached default font or load and cache its message resources.
void* ResourceManager_GetOrLoadDefaultFixedFont(void* self) {
    DefaultFontNameStorage local;
    NString_AssignCString(&local, sharedResourceLoaderStrings + IFF_OFFSET(ResourceLoaderStringPool, defaultFont));
    void* cachedFont;
    void* fontName = String_GetCharPtrAt(&local, 0);
    cachedFont = FixedFontCache_GetOrCreate(self, fontName);
    if (cachedFont == 0) {
        FixedFontObjectStorage* newFont = new FixedFontObjectStorage();
        Renderable_SetResourceManager(newFont, self);
        int loadResult = WFixedFont_LoadMessageResources(newFont, fontName);
        if (loadResult != 0) {
            if (newFont != 0) {
                ((FixedFontDeleteInterface*)newFont)->dispose(1);
            }
        } else {
            cachedFont = newFont;
            FixedFontCache_RetainOrAppend((char*)self + kObjectCacheOffset, fontName, newFont);
        }
    }
    return cachedFont;
}

int MSLString_LowercaseInPlace(const char*);
void* memcpy(void*, const void*, unsigned long);
void* memset(void*, int, unsigned long);
void* ResourceManager_GetOrLoadBitmapByPath(void*, const char*, void*, int, int);
void* ResourceManager_LoadBitmapFromArchive(void*, const char*, const char*);
void Bitmap_ctor(void*, void*);

struct TextureRegistrationNode {
    TextureRegistrationNode* next;
    TextureRegistrationNode* prev;
    unsigned flags;
    char reserved0C[4];
    void* payload;
    void* object;
};

struct TextureRegistrationRecord {
    char unk[0x10];
    int refs;
    void* path;
};

void String_ReserveCapacity(void*, int);
void String_ReplaceRange(void*, int, int, void*, void*, void*);
void* appMallocDummy__FUl(unsigned);
void* __nwa__FUl(unsigned);
void* appMallocUni__FUl(void*);
}
// Placement construction uses storage owned by the surrounding guard/container.
inline void* operator new(size_t, void* p) {
    return p;
}
extern "C" {
union BitmapKeyAllocatorByte {
    char c;
    unsigned align;
};
// Keep this ABI type name: exception cleanup relocations reference its destructor.
struct RegisterString62520 {
    unsigned words[3];
    // Deep-copy a C string into the tagged bitmap registration key.
    RegisterString62520(const char* s) {
        words[0] = 0;
        words[1] = 0;
        words[2] = 0;
        unsigned n = strlen(s);
        String_ReserveCapacity(this, n);
        volatile BitmapKeyAllocatorByte a;
        BitmapKeyAllocatorByte b;
        b.c = a.c;
        String_ReplaceRange(this, 0, 0, (void*)s, (void*)(s + n), &b);
    }
    // Copy inline key words or allocate a separate heap key.
    RegisterString62520(const RegisterString62520& s) {
        unsigned first = s.words[0];
        if (!(first >> 31)) {
            unsigned second = s.words[1];
            words[0] = first;
            words[1] = second;
            words[2] = s.words[2];
        } else {
            words[0] = 0;
            words[1] = 0;
            words[2] = 0;
            String_ReserveCapacity(this, s.words[1]);
            volatile BitmapKeyAllocatorByte a;
            BitmapKeyAllocatorByte b;
            b.c = a.c;
            String_ReplaceRange(this, 0, 0, (void*)s.words[2], (void*)(s.words[2] + s.words[1]), &b);
        }
    }
    // Release only a heap-backed key's owned character buffer.
    ~RegisterString62520() {
        if (words[0] >> 31)
            appFreeUni__FPv((void*)words[2]);
    }
};
typedef RegisterString62520 BitmapRegistrationKey;
// Keep this ABI type name: exception cleanup relocations reference its destructor.
struct RegisterPair62520 {
    BitmapRegistrationKey key;
    void* value;
    // Copy the key and store a non-owning pointer to its separate texture record.
    RegisterPair62520(const BitmapRegistrationKey& k, void* v) : key(k), value(v) {}
};
typedef RegisterPair62520 BitmapRegistrationEntry;
#pragma does_not_return __throw
// Keep this ABI type name: exception cleanup relocations reference its destructor.
struct RegisterGuard62520 {
    void* list;
    char* node;
    // Own an unlinked record node until its payload is constructed successfully.
    RegisterGuard62520(void* l, char* n) : list(l), node(n) {}
    // Free an uncommitted node; the linked list takes responsibility after commit.
    ~RegisterGuard62520() {
        if (node)
            appFreeUni__FPv(node);
    }
};
typedef RegisterGuard62520 BitmapNodeAllocationGuard;
struct BitmapHashNode {
    char* value;
    char* key;
    unsigned char owned;
    char pad[3];
    BitmapHashNode* prev;
    BitmapHashNode* next;
    BitmapHashNode* chain;
};
struct BitmapFactoryInterface {
    virtual void slot0();
    virtual void slot1();
    virtual void slot2();
    virtual void slot3();
    virtual void slot4();
    virtual void slot5();
    virtual void slot6();
    virtual void slot7();
    virtual void* create(void*, int);
    virtual void initialize(void*, void*, int);
};
// Hash the first/last key bytes and length into the configured bucket mask.
inline unsigned HashBitmapBasename(char* self, const char* key) {
    void* lock = self + IFF_OFFSET(ResourceManager, hashMutex);
    OSLockMutex((OSMutex*)lock);
    unsigned n = strlen(key);
    unsigned last = (n - 1) & (unsigned)((-(int)n & ~(int)n) >> 31);
    unsigned hash = (((unsigned char)key[0] & 0x2a) | ((unsigned char)key[last] & 0x15) | (n << 6)) &
                    AsResourceManager(self)->hashMask;
    OSUnlockMutex((OSMutex*)lock);
    return hash;
}
// Link a bitmap hash node into its circular allocation/free/active list.
inline BitmapHashNode* LinkBitmapHashNode(char* self, BitmapHashNode* head, BitmapHashNode* n) {
    void* lock = self + IFF_OFFSET(ResourceManager, hashMutex);
    OSLockMutex((OSMutex*)lock);
    if (!head) {
        n->prev = n;
        n->next = n;
        OSUnlockMutex((OSMutex*)lock);
        return n;
    }
    n->next = head;
    n->prev = head->prev;
    head->prev = n;
    n->prev->next = n;
    OSUnlockMutex((OSMutex*)lock);
    return head;
}
// Search the selected hash bucket for an exact bitmap basename.
inline char* FindBitmapHashValue(char* self, char* key) {
    WInstanceLock g(self + IFF_OFFSET(ResourceManager, hashMutex));
    if (!key || !AsResourceManager(self)->hashBuckets)
        return 0;
    unsigned h = HashBitmapBasename(self, key);
    BitmapHashNode* it = ((*(BitmapHashNode***)&AsResourceManager(self)->hashBuckets))[h];
    while (it) {
        if (!strcmp(it->key, key))
            return it->value;
        it = it->chain;
    }
    return 0;
}
// Copy a key/value pair into a new circular texture-record node.
inline void AppendTextureRecord(char* self, const BitmapRegistrationEntry& pair) {
    char* sentinel = (char*)self + IFF_OFFSET(ResourceManager, recordSentinel);
    char* node = (char*)appMallocDummy__FUl(0x18);
    BitmapNodeAllocationGuard guard(sentinel, node);
    BitmapRegistrationEntry* value = (BitmapRegistrationEntry*)(node + 8);
    if (value) {
        try {
            new (value) BitmapRegistrationEntry(pair);
        } catch (...) {
            __throw(0, 0, 0);
        }
    }
    node = guard.node;
    guard.node = 0;
    *(char**)(*(char**)sentinel + 4) = node;
    *(char**)node = *(char**)sentinel;
    *(char**)sentinel = node;
    (*(char**)&((TextureCleanupNode*)node)->next) = sentinel;
    ++AsResourceManager(self)->recordCount;
}
// Reuse a normalized texture handle or create/register it and account for its storage.
// bitmapDescriptor is borrowed BitmapLayout input. path may
// be null (registered under the fallback name). A hit retains the existing record;
// a miss calls the manager's factory, stores the record, and returns its handle.
// async is unused here. The descriptor's pixels/resource must provide dimensions.
void* ResourceManager_GetOrLoadBitmapByPath(void* selfArg, const char* path, void* bitmapDescriptor, int flags,
                                            int async) {
    char* self = (char*)selfArg;
    if (path) {
        char normalized[0x40];
        const char* base = strchr(path, '/') ? strrchr(path, '/') + 1 : path;
        int n = 0;
        char* dst = normalized;
        unsigned char ch;
        while ((signed char)(ch = base[n]) != 0) {
            *dst++ = ch + (((signed char)ch >= 'A' && (signed char)ch <= 'Z') ? kAsciiUpperToLower : 0);
            ++n;
        }
        normalized[n] = 0;
        char* sentinel = (char*)self + IFF_OFFSET(ResourceManager, recordSentinel);
        char* node = (*(char**)&AsResourceManager(self)->firstRecord);
        char* found = 0;
        while (node != sentinel) {
            unsigned savedCount = strlen(normalized);
            unsigned count = savedCount;
            unsigned first = (*(unsigned*)&((TextureCleanupNode*)node)->key.flags);
            unsigned len;
            if (first >> 31)
                len = (*(unsigned*)&((TextureCleanupNode*)node)->key.length);
            else
                len = (*(unsigned char*)&((TextureCleanupNode*)node)->key.flags) & 0x7f;
            unsigned savedLen = len;
            const char* data;
            unsigned dataLen;
            if (!((*(unsigned*)&((TextureCleanupNode*)node)->key.flags) >> 31)) {
                data = node + 9;
                dataLen = (*(unsigned char*)&((TextureCleanupNode*)node)->key.flags) & 0x7f;
            } else {
                data = (*(char**)&((TextureCleanupNode*)node)->key.data);
                dataLen = (*(unsigned*)&((TextureCleanupNode*)node)->key.length);
            }
            unsigned savedDataLen = dataLen;
            unsigned* p = &savedDataLen;
            if (len < dataLen)
                p = &savedLen;
            unsigned selected = *p;
            p = &selected;
            p = (count >= selected) ? p : &savedCount;
            int comparison = memcmp(data, normalized, *p);
            if (!comparison) {
                if (selected < count)
                    comparison = -1;
                else if (count != selected)
                    comparison = 1;
            }
            if (!comparison) {
                found = (*(char**)&((TextureCleanupNode*)node)->value);
                break;
            }
            node = (*(char**)&((TextureCleanupNode*)node)->next);
        }
        if (found) {
            ++AsResourceTextureRecord(found)->referenceCount;
            return AsResourceTextureRecord(found)->resource;
        }
    }
    // Cache miss: create/optionally initialize the handle and read descriptor dimensions.
    BitmapFactoryInterface* owner = *(BitmapFactoryInterface**)self;
    void* resource = owner->create(bitmapDescriptor, flags);
    if (!(flags & kTextureSkipInitializeMask)) {
        owner = *(BitmapFactoryInterface**)self;
        owner->initialize(resource, bitmapDescriptor, flags);
    }
    int rect[4] = {0, 0, 0, 0};
    char* desc = (char*)bitmapDescriptor;
    char* sub = (char*)AsBitmapLayout(desc)->pixels;
    rect[2] = sub ? ((BitmapDimensions*)sub)->width : ((CGsrResource*)AsBitmapLayout(desc)->resource)->textureInfo[0];
    sub = (char*)AsBitmapLayout(desc)->pixels;
    rect[3] = sub ? ((BitmapDimensions*)sub)->height : ((CGsrResource*)AsBitmapLayout(desc)->resource)->textureInfo[1];
    if (strchr(path ? path : sharedResourceLoaderStrings + IFF_OFFSET(ResourceLoaderStringPool, unknownName), '/'))
        path = strrchr(path ? path : sharedResourceLoaderStrings + IFF_OFFSET(ResourceLoaderStringPool, unknownName),
                       '/') +
               1;
    else
        path = path ? path : sharedResourceLoaderStrings + IFF_OFFSET(ResourceLoaderStringPool, unknownName);
    char normalized[0x40];
    int n = 0;
    char* dst = normalized;
    unsigned char ch;
    while ((signed char)(ch = path[n]) != 0) {
        *dst++ = ch + (((signed char)ch >= 'A' && (signed char)ch <= 'Z') ? kAsciiUpperToLower : 0);
        ++n;
    }
    normalized[n] = 0;
    // Store rectangle, reference count and trailing basename, then link a keyed node.
    char* record = (char*)__nwa__FUl(strlen(normalized) + 0x20);
    AsResourceTextureRecord(record)->referenceCount = 1;
    AsResourceTextureRecord(record)->resource = resource;
    AsResourceTextureRecord(record)->left = rect[0];
    AsResourceTextureRecord(record)->top = rect[1];
    AsResourceTextureRecord(record)->right = rect[2];
    AsResourceTextureRecord(record)->bottom = rect[3];
    AsResourceTextureRecord(record)->resourceBytes = 0;
    strcpy(record + sizeof(ResourceTextureRecord), normalized);
    AppendTextureRecord(self,
                        BitmapRegistrationEntry(BitmapRegistrationKey(record + sizeof(ResourceTextureRecord)), record));
    // Bookkeeping estimates bytes as width*height*bytesPerPixel (+half if flagged).
    int bits = (flags & kTexture24Bit) ? 24 : 16;
    AsResourceTextureRecord(record)->resourceBytes = (bits >> 3) * ((rect[2] - rect[0]) * (rect[3] - rect[1]));
    if (flags & kTextureExtraHalfStorage)
        AsResourceTextureRecord(record)->resourceBytes += AsResourceTextureRecord(record)->resourceBytes >> 1;
    AsResourceManager(self)->resourceBytes += AsResourceTextureRecord(record)->resourceBytes;
    // A null handle also registers its name in the separate hash. Allocation
    // blocks contain a ring header followed by free nodes; buckets chain active nodes.
    if (!AsResourceTextureRecord(record)->resource) {
        char* found = FindBitmapHashValue(self, record + sizeof(ResourceTextureRecord));
        if (!found) {
            unsigned len = strlen(record + sizeof(ResourceTextureRecord)) + 1;
            char* key = (char*)__nwa__FUl((len << 1) - len);
            strcpy(key, record + sizeof(ResourceTextureRecord));
            WInstanceLock outer(self + IFF_OFFSET(ResourceManager, hashMutex));
            BitmapHashNode* node;
            {
                WInstanceLock inner(self + IFF_OFFSET(ResourceManager, hashMutex));
                if (!AsResourceManager(self)->freeHashNodes) {
                    BitmapHashNode* block =
                        (BitmapHashNode*)appMallocUni__FUl((void*)(AsResourceManager(self)->hashBlockNodeCount * 0x18));
                    (*(BitmapHashNode**)&AsResourceManager(self)->hashBlocks) =
                        LinkBitmapHashNode(self, (*(BitmapHashNode**)&AsResourceManager(self)->hashBlocks), block);
                    ++block;
                    for (int i = 1; i < AsResourceManager(self)->hashBlockNodeCount; ++i, ++block) {
                        (*(BitmapHashNode**)&AsResourceManager(self)->freeHashNodes) = LinkBitmapHashNode(
                            self, (*(BitmapHashNode**)&AsResourceManager(self)->freeHashNodes), block);
                    }
                }
                node = (*(BitmapHashNode**)&AsResourceManager(self)->freeHashNodes);
                BitmapHashNode* next;
                {
                    WInstanceLock g(self + IFF_OFFSET(ResourceManager, hashMutex));
                    node->next->prev = node->prev;
                    node->prev->next = node->next;
                    next = node->next;
                    if (next == node)
                        next = 0;
                }
                (*(BitmapHashNode**)&AsResourceManager(self)->freeHashNodes) = next;
            }
            node->value = key;
            node->owned = 0;
            node->key = key;
            if (key) {
                unsigned h = HashBitmapBasename(self, key);
                WInstanceLock g(self + IFF_OFFSET(ResourceManager, hashMutex));
                node->chain = ((*(BitmapHashNode***)&AsResourceManager(self)->hashBuckets))[h];
                ((*(BitmapHashNode***)&AsResourceManager(self)->hashBuckets))[h] = node;
            }
            (*(BitmapHashNode**)&AsResourceManager(self)->activeHashNodes) =
                LinkBitmapHashNode(self, (*(BitmapHashNode**)&AsResourceManager(self)->activeHashNodes), node);
        }
    }
    return AsResourceTextureRecord(record)->resource;
}

struct BitmapResourceStorage {
    char data[0x3c];
};
struct BitmapObjectStorage {
    char data[0x18];
};

// Decode a texture-list node's tagged-key byte length.
inline unsigned TextureNodeKeyLength(const char* node) {
    if (!((*(unsigned*)&((TextureCleanupNode*)node)->key.flags) >> 31))
        return (*(unsigned char*)&((TextureCleanupNode*)node)->key.flags) & 0x7f;
    return (*(unsigned*)&((TextureCleanupNode*)node)->key.length);
}

// Return the three-way ordering of two unsigned key lengths.
inline int CompareTextureKeyLengths(unsigned a, unsigned b) {
    if (a < b)
        return -1;
    if (a == b)
        return 0;
    return 1;
}

// Wait through the bitmap interface without taking ownership of its wrapper.
static inline void WaitForTemporaryBitmap(void* bitmap) {
    void** vt = (*(void***)&AsBitmapLayout(bitmap)->vtable);
    ((void (*)(void*))vt[3])(bitmap);
}

// Resolve a bitmap/archive path, load the bitmap, and register extension/name flags.
// Returns a retained manager handle or null; it destroys temporary bitmap wrappers
// after registration. Existing .bres suffixes are kept; other suffixes become .gsr.
// With '?', text before it is the archive entry key and text after it is the archive
// filename. async==0 adds a pre-registration wait; the later wait is unconditional.
void* ResourceManager_LoadBitmap(void* self, const char* filename, int flags, int async) {
    char normalizedBasename[kBitmapBasenameCapacity];
    char lowercaseFailurePath[kResourcePathCapacity];
    char resourcePath[kResourcePathCapacity];
    void* registered = 0;
    // Phase 1: replace the extension and derive the case-folded basename cache key.
    strcpy(resourcePath, filename);
    char* extensionToReplace = strrchr(resourcePath, '.');
    if (extensionToReplace == 0)
        return 0;

    if (strcmp(extensionToReplace, sharedResourceLoaderStrings + IFF_OFFSET(ResourceLoaderStringPool, bresExtension))) {
        strcpy(extensionToReplace, sharedResourceLoaderStrings + IFF_OFFSET(ResourceLoaderStringPool, gsrExtension));
    }

    char* firstSlash = strchr(resourcePath, '/');
    char* basename;
    if (firstSlash != 0)
        basename = strrchr(resourcePath, '/') + 1;
    else
        basename = resourcePath;

    int basenameByteIndex = 0;
    while (basename[basenameByteIndex] != 0) {
        char ch = basename[basenameByteIndex];
        normalizedBasename[basenameByteIndex] = ch + ((ch >= 'A' && ch <= 'Z') ? kAsciiUpperToLower : 0);
        basenameByteIndex++;
    }
    normalizedBasename[basenameByteIndex] = 0;

    // Phase 2: retain an existing basename hit before opening any new resource.
    char* sentinel = (char*)self + IFF_OFFSET(ResourceManager, recordSentinel);
    char* cacheNode = (*(char**)&AsResourceManager(self)->firstRecord);
    char* cachedRecord = 0;
    while (cacheNode != sentinel) {
        unsigned savedLen;
        unsigned savedCount = strlen(normalizedBasename);
        unsigned count = savedCount;
        unsigned len = TextureNodeKeyLength(cacheNode);
        savedLen = len;
        const char* data;
        unsigned dataLen;
        if (!((*(unsigned*)&((TextureCleanupNode*)cacheNode)->key.flags) >> 31)) {
            data = cacheNode + 9;
            dataLen = (*(unsigned char*)&((TextureCleanupNode*)cacheNode)->key.flags) & 0x7f;
        } else {
            data = (*(char**)&((TextureCleanupNode*)cacheNode)->key.data);
            dataLen = (*(unsigned*)&((TextureCleanupNode*)cacheNode)->key.length);
        }
        unsigned savedDataLen = dataLen;
        unsigned* p = &savedDataLen;
        if (len < dataLen)
            p = &savedLen;
        unsigned selected = *p;
        p = &selected;
        if (count < selected)
            p = &savedCount;
        int comparison = memcmp(data, normalizedBasename, *p);
        if (!comparison) {
            if (selected < count)
                comparison = -1;
            else if (selected == count)
                comparison = 0;
            else
                comparison = 1;
        }
        if (!comparison) {
            cachedRecord = (*(char**)&((TextureCleanupNode*)cacheNode)->value);
            break;
        }
        cacheNode = (*(char**)&((TextureCleanupNode*)cacheNode)->next);
    }
    if (cachedRecord) {
        ++AsResourceTextureRecord(cachedRecord)->referenceCount;
        return AsResourceTextureRecord(cachedRecord)->resource;
    }

    // Phase 3: after a miss, construct an archive entry or a direct GSR resource.
    void* temporaryBitmap = 0;
    char* extension = strrchr(resourcePath, '.');
    char* archiveSeparator = strrchr(resourcePath, '?');
    if (archiveSeparator != 0) {
        char query[kArchiveEntryKeyCapacity] = {0};
        memcpy(query, resourcePath, (unsigned)(archiveSeparator - resourcePath));
        temporaryBitmap = ResourceManager_LoadBitmapFromArchive(self, archiveSeparator + 1, query);
    } else if (extension != 0 &&
               CString_Compare(extension,
                               sharedResourceLoaderStrings + IFF_OFFSET(ResourceLoaderStringPool, gsrExtension)) == 0) {
        if (CHashPath::FindPath(resourcePath) == 0)
            temporaryBitmap = 0;
        else if (CHashPath::FindPath(resourcePath) == 0)
            temporaryBitmap = 0;
        else {
            void* gsrResource = new BitmapResourceStorage();
            if (gsrResource != 0) {
                try {
                    ((CGsrResource*)gsrResource)->__ct(resourcePath);
                } catch (...) {
                    __dl__FPv(gsrResource);
                    throw;
                }
            }
            void* bitmapWrapper = new BitmapObjectStorage();
            if (bitmapWrapper != 0)
                Bitmap_ctor(bitmapWrapper, gsrResource);
            temporaryBitmap = bitmapWrapper;
        }
    }

    // Phase 4: wait, derive name flags, register, then destroy the temporary wrapper.
    // Keep the conditional pre-wait and unconditional wait as two distinct calls.
    if (async == 0 && temporaryBitmap != 0) {
        WaitForTemporaryBitmap(temporaryBitmap);
    }
    if (temporaryBitmap != 0) {
        WaitForTemporaryBitmap(temporaryBitmap);
        int parsedFlags = flags;
        if (strchr(resourcePath, '{') != 0)
            parsedFlags = (parsedFlags | kTextureNameBrace) & kTextureNameBraceClearMask;
        if (strchr(resourcePath, '!') != 0)
            parsedFlags |= kTextureNameBang;
        registered = ResourceManager_GetOrLoadBitmapByPath(self, resourcePath, temporaryBitmap, parsedFlags, 0);
        if (temporaryBitmap != 0) {
            void** dtor = (*(void***)&AsBitmapLayout(temporaryBitmap)->vtable);
            ((void (*)(void*, int))dtor[2])(temporaryBitmap, 1);
        }
    } else {
        strcpy(lowercaseFailurePath, resourcePath);
        MSLString_LowercaseInPlace(lowercaseFailurePath);
    }
    return registered;
}

struct CachedResourceKey {
    char repr[12];
    // Borrow character storage from this key's inline/heap representation.
    const char* data() const {
        if (*(unsigned*)repr >> 31)
            return (*(const char**)&AsMWStringStorage(repr)->data);
        return repr + 1;
    }
    // Decode the tagged key length without scanning its characters.
    unsigned int length() const {
        if (*(unsigned*)repr >> 31)
            return AsMWStringStorage(repr)->length;
        return *(unsigned char*)repr & 0x7f;
    }
};

struct NamedResourceEntry {
    CachedResourceKey key;
    void* value;
};

void String_ReserveCapacity(void*, int);
void String_ReplaceRange(void*, int, int, void*, void*, void*);
void* appMallocDummy__FUl(unsigned);

// Return a reference to the shorter comparison span (ties choose b).
inline const unsigned& MinimumKeyLength(const unsigned& a, const unsigned& b) {
    if (a < b)
        return a;
    return b;
}

// Decode a standalone tagged key's byte length.
inline unsigned CachedKeyLength(const char* repr) {
    return (*(unsigned*)repr >> 31) == 0 ? (*(unsigned char*)repr & 0x7f) : AsMWStringStorage(repr)->length;
}

// Return a reference to the shorter search span (ties choose a).
inline const unsigned& MinimumSearchLength(const unsigned& a, const unsigned& b) {
    if (b < a)
        return b;
    return a;
}
// Resolve an equal-prefix comparison using unsigned byte lengths.
inline int CompareSearchLengths(unsigned a, unsigned b, int result) {
    if (a < b)
        result = -1;
    else if (a == b)
        result = 0;
    else if (a > b)
        result = 1;
    return result;
}

// Linearly find an exact small-string key and return its cached value.
void* ResourceEntryVector_FindByName(void* self, const char* key) {
    NamedResourceEntry* entry = (*(NamedResourceEntry**)&AsResourceEntryVector(self)->entries);
    // Original 0x800646A8: search up to the vector's current end.
    while (!same_pos((*(NamedResourceEntry**)&AsResourceEntryVector(self)->entries) +
                         (*(unsigned int*)&AsResourceEntryVector(self)->count),
                     entry)) {
        unsigned savedEntryLength;
        unsigned savedKeyLength;
        unsigned savedDataLength;
        unsigned selectedStringLength;
        unsigned keyLength;
        unsigned dataLength;
        unsigned entryLength;
        const char* data;
        unsigned second;
        keyLength = strlen(key);
        savedKeyLength = keyLength;
        entryLength = CachedKeyLength(entry->key.repr);
        savedEntryLength = entryLength;
        second = *(unsigned*)entry->key.repr;
        if (!(second >> 31)) {
            data = entry->key.repr + 1;
            dataLength = *(unsigned char*)entry->key.repr & 0x7f;
        } else {
            data = (*(const char**)&AsMWStringStorage(entry->key.repr)->data);
            dataLength = AsMWStringStorage(entry->key.repr)->length;
        }
        savedDataLength = dataLength;
        selectedStringLength = MinimumSearchLength(savedDataLength, savedEntryLength);
        int comparison = memcmp(data, key, MinimumSearchLength(selectedStringLength, savedKeyLength));
        if (comparison == 0)
            comparison = CompareSearchLengths(selectedStringLength, keyLength, comparison);
        if (comparison == 0)
            return entry->value;
        entry++;
    }
    return 0;
}

void* __nw__FUl(unsigned long);
void* __nwa__FUl(unsigned int);
void __dt__Q23std12length_errorFv(void*);
extern char __vt__Q23std11logic_error[];
extern char s_sharedCharDeleterVtable[];
extern char __vt__Q23std12length_error[];

// Inlined std::length_error construction + throw (one stack object per site).
#define LENGTH_ERROR_SITE()                                                                                    \
    do {                                                                                                       \
        void* exc[3];                                                                                          \
        exc[0] = __vt__Q23std11logic_error;                                                                    \
        unsigned elen =                                                                                        \
            strlen(sharedResourceLoaderStrings + IFF_OFFSET(ResourceLoaderStringPool, vectorLengthError)) + 1; \
        char* ebuf = (char*)__nwa__FUl(elen);                                                                  \
        exc[1] = ebuf;                                                                                         \
        unsigned* rep = (unsigned*)__nw__FUl(0x10);                                                            \
        if (rep) {                                                                                             \
            rep[1] = 1;                                                                                        \
            rep[2] = 1;                                                                                        \
            rep[0] = (unsigned)s_sharedCharDeleterVtable;                                                      \
            rep[3] = (unsigned)ebuf;                                                                           \
        }                                                                                                      \
        exc[2] = rep;                                                                                          \
        strcpy(ebuf, sharedResourceLoaderStrings + IFF_OFFSET(ResourceLoaderStringPool, vectorLengthError));   \
        exc[0] = __vt__Q23std12length_error;                                                                   \
        __throw(sharedResourceLoaderStrings + IFF_OFFSET(ResourceLoaderStringPool, lengthErrorType), exc,      \
                (void*)__dt__Q23std12length_errorFv);                                                          \
    } while (0)

// Async-file entry points used by the model-load state machine below.
void* __nwa__FUl(unsigned int);

// Advance asynchronous model I/O through open, aligned read, and read completion.
// Return 1 only in states 4/5. Zero means pending, an I/O failure, or an unsupported
// state; callers must inspect state/own policy rather than treating zero as only busy.
// State 2 allocates/clears a 32-byte-rounded read buffer owned by the manager.
// States 1->2->3->4 may all advance during one call through intentional fallthrough.
int ResourceManager_PollResourceRead(void* self) {
    int ready;
    int state = AsResourceManager(self)->loadState;
    switch (state) {
    case kWaitingForOpen: {
        if (((CFileAsync*)(AsResourceManager(self)->asyncRead))->PollRead((int*)&ready, 0, 0) == 0)
            return 0;
        if (ready == 0)
            break;
        AsResourceManager(self)->loadState = kAllocateReadBuffer;
    }
    // Intentional fallthrough lets ready I/O advance multiple states in one poll.
    case kAllocateReadBuffer: {
        AsResourceManager(self)->resourceDataBytes = ((CFileAsync*)(AsResourceManager(self)->asyncRead))->GetSize();
        void* old = AsResourceManager(self)->resourceData;
        if (old != 0) {
            __dla__FPv(old);
            AsResourceManager(self)->resourceData = 0;
        }
        int rounded = (*(int*)&AsResourceManager(self)->resourceDataBytes);
        int remainder = (rounded % kDvdReadAlignment);
        if (remainder) {
            rounded += kDvdReadAlignment - remainder;
        }
        void* allocated = __nwa__FUl(rounded);
        AsResourceManager(self)->resourceData = allocated;
        int rounded2 = (*(int*)&AsResourceManager(self)->resourceDataBytes);
        remainder = (rounded2 % kDvdReadAlignment);
        if (remainder != 0)
            rounded2 += kDvdReadAlignment - remainder;
        Memory_Clear(allocated, rounded2);
        int rounded3 = (*(int*)&AsResourceManager(self)->resourceDataBytes);
        void* asyncRead = AsResourceManager(self)->asyncRead;
        void* resourceData = AsResourceManager(self)->resourceData;
        remainder = (rounded3 % kDvdReadAlignment);
        if (remainder != 0)
            rounded3 += kDvdReadAlignment - remainder;
        if (((CFileAsync*)asyncRead)->StartRead(resourceData, rounded3) == 0)
            return 0;
        AsResourceManager(self)->loadState = kWaitingForRead;
    }
    case kWaitingForRead: {
        if (((CFileAsync*)(AsResourceManager(self)->asyncRead))->PollRead((int*)&ready, 0, 0) == 0)
            return 0;
        if (ready == 0)
            break;
        ((CFileAsync*)(AsResourceManager(self)->asyncRead))->Close();
        void* resource = AsResourceManager(self)->asyncRead;
        if (resource != 0) {
            if (resource != 0) {
                ((void (**)(void*, int))(((AsyncFileResource*)resource)->vtable))[2](resource, 1);
            }
            AsResourceManager(self)->asyncRead = 0;
        }
        AsResourceManager(self)->loadState = kReadComplete;
    }
    case kReadComplete:
    case kCachedModelReady:
        return 1;
    default:
        return 0;
    }
    return 0;
}

struct NamedTextureRecordNode {
    NamedTextureRecordNode* previous;
    NamedTextureRecordNode* next;
    CachedResourceKey key;
    void* value;
};

// Resolve equal basename prefixes by unsigned byte length.
inline int CompareBasenameLengths(unsigned a, unsigned b, int result) {
    if (a < b)
        result = -1;
    else if (a == b)
        result = 0;
    else if (a > b)
        result = 1;
    return result;
}
// Test only ASCII A..Z, independent of locale.
inline int IsAsciiUppercase(int ch) {
    return ch >= 'A' && ch <= 'Z';
}
// Fold ASCII uppercase bytes; leave all other bytes unchanged.
inline int LowercaseAsciiByte(unsigned char ch) {
    int upper = IsAsciiUppercase((signed char)ch);
    return ch + (upper ? kAsciiUpperToLower : 0);
}
// Copy/fold the remaining basename and return length; caller writes the terminator.
inline int CopyLowercaseBasename(int length, const char* path, char* normalized) {
    while (*path != 0) {
        *normalized++ = LowercaseAsciiByte((unsigned char)*path);
        ++length;
        ++path;
    }
    return length;
}

// Find a texture record using the lowercased final path component.
// Return a borrowed record, not its resource handle; no reference is acquired.
// Only '/' separates components here. The final component must fit 64 bytes.
void* ResourceManager_FindByNormalizedBasename(void* self, const char* path) {
    char* slash = strchr(path, '/');
    if (slash != 0)
        path = strrchr(path, '/') + 1;
    char buffer[0x40];
    int length = CopyLowercaseBasename(0, path, buffer);
    buffer[length] = 0;
    NamedTextureRecordNode* sentinel =
        (NamedTextureRecordNode*)((char*)self + IFF_OFFSET(ResourceManager, recordSentinel));
    NamedTextureRecordNode* entry = (*(NamedTextureRecordNode**)&AsResourceManager(self)->firstRecord);
    void* result = 0;
    while (entry != sentinel) {
        unsigned int selectedLength;
        unsigned int savedSecondLength;
        unsigned int savedKeyLength;
        unsigned int savedEntryLength;
        savedKeyLength = strlen(buffer);
        unsigned int keyLength = savedKeyLength;
        unsigned int entryLength;
        if ((*(unsigned*)entry->key.repr >> 31) == 0) {
            entryLength = *(unsigned char*)entry->key.repr & 0x7f;
            savedEntryLength = entryLength;
        } else {
            entryLength = AsMWStringStorage(entry->key.repr)->length;
            savedEntryLength = entryLength;
        }
        savedEntryLength = entryLength;
        const char* data;
        unsigned int secondLength;
        if (!(*(unsigned*)entry->key.repr >> 31)) {
            data = entry->key.repr + 1;
            secondLength = *(unsigned char*)entry->key.repr & 0x7f;
        } else {
            data = (*(const char**)&AsMWStringStorage(entry->key.repr)->data);
            secondLength = AsMWStringStorage(entry->key.repr)->length;
        }
        savedSecondLength = secondLength;
        selectedLength = MinimumKeyLength(savedEntryLength, savedSecondLength);
        int comparison = memcmp(data, buffer, MinimumKeyLength(savedKeyLength, selectedLength));
        if (comparison == 0) {
            comparison = CompareBasenameLengths(selectedLength, keyLength, comparison);
        }
        if (comparison == 0) {
            result = entry->value;
            break;
        }
        entry = entry->next;
    }
    return result;
}
void String_ReserveCapacity(void*, int);
void String_ReplaceRange(void*, int, int, void*, void*, void*);
void ResourcePath_NormalizeImageExtension(void*);
void* SharedFile_CreateFromDvdPath(void*, void*, int, void*, int);
}

// Stack-local narrow-string view; the canonical string ABI remains external.
class LocalNString_u8006028C {
public:
    unsigned int flags;
    unsigned int length;
    char* data;

    // Initialize empty inline string storage.
    LocalNString_u8006028C() : flags(0), length(0), data(0) {}
#if defined(GAME_NSTRING_TEXT_CONSTRUCTOR)
    // Forward construction to the canonical string implementation.
    LocalNString_u8006028C(const char* text) {
        MWString_ConstructCString(this, text);
    }
#endif
#if defined(GAME_NSTRING_RANGE_CONSTRUCTOR)
    // Deep-copy a borrowed C string into inline or owned heap storage.
    LocalNString_u8006028C(const char* text) {
        LocalNString_u8006028C* object = this;
        unsigned int size;
        const char* source = text;
        object->flags = 0;
        object->length = 0;
        object->data = 0;
        size = nstring_detail::strlen(source);
        nstring_detail::String_ReserveCapacity(object, size);
        int allocator;
        char copy = *(char*)&allocator;
        nstring_detail::String_ReplaceRange(object, 0, 0, source, source + size, &copy);
    }
#endif
    // Release only the owned heap representation.
    ~LocalNString_u8006028C() {
        if (flags >> 31)
            appFreeUni__FPv(data);
    }
    // Decode byte length from the inline tag or heap length word.
    unsigned int size() const {
        return (flags >> 31) == 0 ? (*(const unsigned char*)this & 0x7f) : length;
    }
    // Borrow the terminated character span until this string changes or dies.
    const char* c_str() const {
        return (flags >> 31) == 0 ? (const char*)this + 1 : data;
    }
};

extern "C" {
// Normalize an optional image suffix and open an existing path through the shared-file API.
// First/fifth ABI arguments are unused. openArgument2/4 pass through to the file
// factory. Zero openFlags also normalizes the suffix. Null means unresolved path
// or factory failure; successful handles follow SharedFile's release contract.
void* Resource_OpenResolvedPath(void* unusedOwner, const char* path, void* openArgument2, void* openArgument4,
                                void* unusedArgument, int openFlags) {
    LocalNString_u8006028C local(path);
    if (openFlags == 0) {
        ResourcePath_NormalizeImageExtension(&local);
    }
    void* normalizedPath = (void*)local.c_str();
    void* pathExists = CHashPath::FindPath((const char*)normalizedPath);
    if (pathExists == 0) {
        return 0;
    }
    void* resolvedPath = (void*)local.c_str();
    void* file = SharedFile_CreateFromDvdPath(resolvedPath, openArgument2, 0, openArgument4, openFlags);
    return file;
}

void* appMallocUni__FUl(void*);
void Resource_Release(void*);
void* Archive_Open(void*, void*, int);
void Archive_GetEntryInfo(void*, int, void*);
void Archive_ReadEntry(void*, int, void*, void*, int);
void Archive_Close(void*);
int String_CompareCString(void*, const char*);
extern "C" void __ct__12CGsrResourceFPCvUi(void*, void*, void*);
void Bitmap_ctor(void*, void*);

union ArchiveStringAllocatorByte {
    mutable volatile char c;
    unsigned align;
    // Retain the original uninitialized allocator tag; it owns no storage.
    ArchiveStringAllocatorByte() {}
};
// Keep this ABI type name: exception cleanup relocations reference its destructor.
struct ArchiveString62E3C {
    unsigned words[3];
    // Initialize an empty archive-path string.
    ArchiveString62E3C() {
        words[0] = 0;
        words[1] = 0;
        words[2] = 0;
    }
    // Deep-copy the path while preserving the original allocator-tag lifetimes.
    ArchiveString62E3C(const char* filename, const ArchiveStringAllocatorByte& allocator = ArchiveStringAllocatorByte(),
                       const ArchiveStringAllocatorByte& term = ArchiveStringAllocatorByte()) {
        words[0] = 0;
        words[1] = 0;
        words[2] = 0;
        int len = strlen(filename);
        String_ReserveCapacity(this, len);
        term.c = allocator.c;
        String_ReplaceRange(this, 0, 0, (void*)filename, (void*)(filename + len), (void*)&term);
    }
    // Release the archive path's owned heap representation.
    ~ArchiveString62E3C() {
        if (words[0] >> 31)
            appFreeUni__FPv((void*)words[2]);
    }
    // Borrow inline or heap character storage.
    void* text() {
        return (words[0] >> 31) == 0 ? (char*)this + 1 : (void*)words[2];
    }
};
typedef ArchiveString62E3C ArchivePathString;
struct ArchiveFileInterface {
    virtual void slot0();
    virtual void slot1();
    virtual void read(void*, void*);
    void* size;
};
// Keep this ABI type name: exception cleanup relocations reference its destructor.
struct ArchiveGuard62E3C {
    ArchiveFileInterface** pointer;
    // Track a file variable so explicit release can also disarm this guard.
    ArchiveGuard62E3C(ArchiveFileInterface*& file) : pointer(&file) {}
    // Release the shared-file handle once and clear the tracked variable.
    void reset() {
        if (*pointer) {
            Resource_Release(*pointer);
            *pointer = 0;
        }
    }
    // Release a still-open file on normal exit or exception unwinding.
    ~ArchiveGuard62E3C() {
        if (*pointer)
            reset();
    }
};
typedef ArchiveGuard62E3C ArchiveFileGuard;
struct BitmapArchiveEntryInfo {
    int index;
    char name[0x124];
    void* size;
};
// Copy the selected archive entry name using the original string range endpoints.
inline void AssignArchiveEntryName(ArchivePathString& name, BitmapArchiveEntryInfo& entry) {
    unsigned len = strlen(entry.name);
    void* end = (char*)&entry + len;
    unsigned length = (name.words[0] >> 31) == 0 ? (*(unsigned char*)&name & 0x7f) : name.words[1];
    BitmapKeyAllocatorByte term;
    volatile BitmapKeyAllocatorByte allocator;
    term.c = allocator.c;
    String_ReplaceRange(&name, 0, length, entry.name, end, &term);
}
struct ArchiveBitmapResourceStorage {
    char bytes[0x3c];
    // Construct a GSR resource from entry bytes before the temporary bytes are freed.
    ArchiveBitmapResourceStorage(void* data, void* size) {
        __ct__12CGsrResourceFPCvUi(this, data, size);
    }
};
struct ArchiveBitmapStorage {
    char bytes[0x18];
    // Construct the bitmap wrapper around the newly created GSR resource.
    ArchiveBitmapStorage(void* resource) {
        Bitmap_ctor(this, resource);
    }
};

// Normalize and resolve an archive path before opening its shared-file handle.
inline ArchiveFileInterface* OpenBitmapArchiveFile(const char* filename) {
    ArchivePathString path(filename);
    ResourcePath_NormalizeImageExtension(&path);
    if (CHashPath::FindPath((const char*)path.text()) == 0) {
        return 0;
    }
    return (ArchiveFileInterface*)SharedFile_CreateFromDvdPath(path.text(), (void*)-1, 0, 0, 0);
}
// Read a named bitmap archive entry and release all temporary buffers and handles.
// filename/key are borrowed; key comparison is exact. Return a new bitmap wrapper
// or null for open/archive/missing-entry/unsupported-extension failures. Matching
// entries require a suffix: the existing code passes strrchr's result unchecked.
void* ResourceManager_LoadBitmapFromArchive(void* self, const char* filename, const char* key) {
    // Read the whole archive, then release the file before opening the memory archive.
    ArchiveFileInterface* file = OpenBitmapArchiveFile(filename);
    ArchiveFileGuard guard(file);
    if (!file)
        return 0;
    void* size = file->size;
    void* archiveData = appMallocUni__FUl(size);
    file->read(archiveData, size);
    if (file) {
        Resource_Release(file);
        file = 0;
    }
    void* archive = Archive_Open(archiveData, size, 3);
    if (archive == 0) {
        if (archiveData)
            appFreeUni__FPv(archiveData);
        return 0;
    }
    BitmapArchiveEntryInfo entry;
    Archive_GetEntryInfo(archive, -1, &entry);
    int count = entry.index;
    // Find and decode one entry; its bytes are temporary even on successful decode.
    for (int index = 0; index < count; ++index) {
        Archive_GetEntryInfo(archive, index, &entry);
        if (CString_Compare(entry.name, key) == 0) {
            void* data = appMallocUni__FUl(entry.size);
            Archive_ReadEntry(archive, index, data, entry.size, 3);
            char* ext = strrchr(entry.name, '.');
            void* result = 0;
            if (String_CompareCString(ext, sharedResourceLoaderStrings +
                                               IFF_OFFSET(ResourceLoaderStringPool, gsrExtension)) == 0) {
                ArchivePathString name;
                AssignArchiveEntryName(name, entry);
                ArchiveBitmapResourceStorage* resource = new ArchiveBitmapResourceStorage(data, entry.size);
                result = new ArchiveBitmapStorage(resource);
            }
            if (data)
                appFreeUni__FPv(data);
            Archive_Close(archive);
            if (archiveData)
                appFreeUni__FPv(archiveData);
            return result;
        }
    }
    Archive_Close(archive);
    if (archiveData)
        appFreeUni__FPv(archiveData);
    return 0;
}

int WPuppet_LoadNamedResourceFromChunks(void*, const char*, void*, void*, int, int);
void WPuppet_SetOpacityRecursive(void*, int);
void __throw(const char*, void*, void*) __attribute__((noreturn));
#pragma does_not_return __throw
#pragma push
#pragma placement_new_check off
struct LoadedPuppetStorage {
    char bytes[0x23c];
    // Construct a puppet within the allocation sized for its original ABI.
    LoadedPuppetStorage() {
        WPuppet_ctor(this);
    }
};
struct CacheEntryIterator {
    NamedResourceEntry* pointer;
    // Borrow a position in the contiguous entry allocation.
    CacheEntryIterator(NamedResourceEntry* p) : pointer(p) {}
    // Copy the borrowed position without copying an entry.
    CacheEntryIterator(const CacheEntryIterator& p) : pointer(p.pointer) {}
    // Compare positions only, including the one-past-end position.
    bool operator!=(const CacheEntryIterator& other) const {
        return other.pointer != pointer;
    }
};
// Choose the shorter comparison length by reference (ties retain a).
inline const unsigned& MinimumCacheKeyLength(const unsigned& a, const unsigned& b) {
    if (b < a)
        return b;
    return a;
}
// Order unsigned lengths after equal character prefixes.
inline int CompareCacheKeyLengths(unsigned a, unsigned b) {
    if (a < b)
        return -1;
    if (a == b)
        return 0;
    return 1;
}
struct LoadedPuppetDeleteInterface {
    virtual void destroy(short);
};
union CacheAllocatorByte {
    char value;
    unsigned align;
};
// Keep this ABI type name: exception cleanup relocations reference its destructor.
struct Load63788String {
    unsigned w[3];
    // Deep-copy a borrowed C-string key.
    Load63788String(const char* s) {
        w[0] = 0;
        w[1] = 0;
        w[2] = 0;
        unsigned len = strlen(s);
        String_ReserveCapacity(this, len);
        volatile CacheAllocatorByte a;
        CacheAllocatorByte t;
        t.value = a.value;
        String_ReplaceRange(this, 0, 0, (void*)s, (void*)(s + len), &t);
    }
    // Copy inline bytes or allocate an independent heap key.
    Load63788String(const Load63788String& s) {
        unsigned first = s.w[0];
        if (!(first >> 31)) {
            unsigned second = s.w[1];
            w[0] = first;
            w[1] = second;
            w[2] = s.w[2];
        } else {
            w[0] = 0;
            w[1] = 0;
            w[2] = 0;
            String_ReserveCapacity(this, s.w[1]);
            volatile CacheAllocatorByte a;
            CacheAllocatorByte t;
            t.value = a.value;
            String_ReplaceRange(this, 0, 0, (void*)s.w[2], (void*)(s.w[2] + s.w[1]), &t);
        }
    }
    // Release only a heap-backed key's owned characters.
    ~Load63788String() {
        if (w[0] >> 31)
            appFreeUni__FPv((void*)w[2]);
    }
};
typedef Load63788String CacheInsertionKey;
// Keep this ABI type name: exception cleanup relocations reference its destructor.
struct Load63788Pair : CacheInsertionKey {
    void* value;
    // Own a copy of the key and store the manager-tracked value pointer.
    Load63788Pair(const CacheInsertionKey& s, void* v) : CacheInsertionKey(s), value(v) {}
};
typedef Load63788Pair CacheInsertionEntry;
// Keep this ABI type name: exception cleanup relocations reference its destructor.
struct Load63788Guard {
    void* pointer;
    CacheAllocatorByte* allocator;
    // Free exception text if shared-control adoption did not complete.
    ~Load63788Guard() {
        if (pointer)
            __dla__FPv(pointer);
    }
};
typedef Load63788Guard CacheArrayAllocationGuard;
extern "C" void __dt__Q23std9exceptionFv(void*, int);
struct CacheLogicErrorStorage {
    void* vt;
    char* text;
    void* rep;
    // Deep-copy exception text and adopt it through a shared control block.
    CacheLogicErrorStorage(const char* s) {
        vt = __vt__Q23std11logic_error;
        CacheAllocatorByte a;
        a.value = 0;
        unsigned n = strlen(s) + 1;
        char* buffer = (char*)__nwa__FUl((n << 1) - n);
        text = buffer;
        {
            CacheArrayAllocationGuard guard = {buffer, &a};
            unsigned* r = (unsigned*)__nw__FUl(16);
            if (r) {
                r[1] = 1;
                r[2] = 1;
                r[0] = (unsigned)s_sharedCharDeleterVtable;
                r[3] = (unsigned)buffer;
            }
            rep = r;
            guard.pointer = 0;
        }
        strcpy(text, s);
    }
    // Preserve the original exception-base destruction entry point.
    ~CacheLogicErrorStorage() {
        __dt__Q23std9exceptionFv(this, -1);
    }
};
struct CacheLengthErrorStorage : CacheLogicErrorStorage {
    // Set the derived runtime type after constructing the exception message.
    CacheLengthErrorStorage(const char* s) : CacheLogicErrorStorage(s) {
        vt = __vt__Q23std12length_error;
    }
};
// Construct/throw in the caller's exception scope; helper extraction can alter extab.
#define LOAD63788_LENGTH_ERROR()                                                                             \
    do {                                                                                                     \
        unsigned storage[3];                                                                                 \
        CacheLengthErrorStorage* format = new (storage) CacheLengthErrorStorage(                             \
            sharedResourceLoaderStrings + IFF_OFFSET(ResourceLoaderStringPool, vectorLengthError));          \
        __throw(sharedResourceLoaderStrings + IFF_OFFSET(ResourceLoaderStringPool, lengthErrorType), format, \
                (void*)__dt__Q23std12length_errorFv);                                                        \
    } while (0)
// Keep this ABI type name: exception cleanup relocations reference its destructor.
struct Load63788Vector {
    NamedResourceEntry* data;
    unsigned size, capacity;
    unsigned* owner;
    unsigned offset;
    // Begin an empty growth transaction associated with the destination capacity.
    Load63788Vector(unsigned* p) : data(0), size(0), capacity(0), owner(p), offset(0) {}
    // Destroy guarded key payloads, then free the block (new or old after swap).
    ~Load63788Vector() {
        if (data) {
            unsigned n = size;
            NamedResourceEntry* p = data + n;
            size -= n;
            while (n != 0) {
                --p;
                if (p && p && (*(unsigned*)p >> 31))
                    appFreeUni__FPv((*(void**)&AsMWStringStorage(p)->data));
                --n;
            }
            appFreeUni__FPv(data);
        }
    }
};
typedef Load63788Vector CacheTransferBuffer;
}
namespace std {
class exception {
public:
    // Leave exception storage for the runtime's derived-type initialization.
    exception() {}
    virtual ~exception();
};
class logic_error : public exception {
public:
    char* text;
    void* rep;
    // Deep-copy text and transfer it to a shared ownership control block.
    logic_error(const char* s) {
        CacheAllocatorByte a;
        a.value = 0;
        unsigned n = strlen(s) + 1;
        char* buffer = (char*)__nwa__FUl((n << 1) - n);
        text = buffer;
        {
            CacheArrayAllocationGuard guard = {buffer, &a};
            unsigned* r = (unsigned*)__nw__FUl(16);
            if (r) {
                r[1] = 1;
                r[2] = 1;
                r[0] = (unsigned)s_sharedCharDeleterVtable;
                r[3] = (unsigned)buffer;
            }
            rep = r;
            guard.pointer = 0;
        }
        strcpy(text, s);
    }
    virtual ~logic_error();
};
class length_error : public logic_error {
public:
    // Construct the logic-error payload with the derived runtime exception type.
    length_error(const char* s) : logic_error(s) {}
    virtual ~length_error();
};
}
extern "C" {
struct FixedFontFactoryInterface {
    virtual void slot0();
    virtual void slot1();
    virtual void* create();
};
// Return three-way unsigned length ordering for equal font-key prefixes.
inline int CompareFontKeyLengths(unsigned a, unsigned b) {
    if (a < b)
        return -1;
    else if (a != b)
        return 1;
    return 0;
}
// Choose the larger growth increment by reference (ties retain a).
inline const unsigned& MaximumCacheGrowth(const unsigned& a, const unsigned& b) {
    if (a < b)
        return b;
    return a;
}
struct ResourceEntryIterator {
    NamedResourceEntry* pointer;
    // Borrow the supplied entry position.
    ResourceEntryIterator(NamedResourceEntry* const& p) : pointer(p) {}
    // Copy an entry position without copying its key or value.
    ResourceEntryIterator(const ResourceEntryIterator& other) : pointer(other.pointer) {}
    // Test whether two borrowed entry positions differ.
    bool operator!=(const ResourceEntryIterator& other) const {
        return other.pointer != pointer;
    }
};
// Decode a resource entry's tagged key length in bytes.
inline unsigned ResourceEntryKeyLength(const NamedResourceEntry* entry) {
    if (!(*(unsigned*)entry->key.repr >> 31))
        return *(unsigned char*)entry->key.repr & 0x7f;
    return AsMWStringStorage(entry->key.repr)->length;
}
struct FontCacheSnapshotIterator {
    NamedResourceEntry* pointer;
    // Snapshot the entry position with the original volatile store.
    FontCacheSnapshotIterator(NamedResourceEntry* p) : pointer(p) {
        *(NamedResourceEntry* volatile*)&pointer = p;
    }
    // Copy a position, preserving its original volatile snapshot.
    FontCacheSnapshotIterator(const FontCacheSnapshotIterator& other) : pointer(other.pointer) {
        *(NamedResourceEntry* volatile*)&pointer = other.pointer;
    }
    // Compare positions through the original temporary snapshot.
    bool operator!=(const FontCacheSnapshotIterator& other) const {
        FontCacheSnapshotIterator end(other);
        return end.pointer != pointer;
    }
};
// Keep this ABI type name: exception cleanup relocations reference its destructor.
struct Vector61204 {
    NamedResourceEntry* data;
    unsigned size, capacity;
    unsigned* owner;
    unsigned offset;
    // Begin an empty font-entry growth transaction.
    Vector61204(unsigned* p) : data(0), size(0), capacity(0), owner(p), offset(0) {}
    // Destroy guarded key payloads and free the allocation owned after any swap.
    ~Vector61204() {
        if (data) {
            unsigned n = size;
            NamedResourceEntry* p = data + n;
            volatile CacheAllocatorByte allocator;
            allocator.value = 0;
            size -= n;
            while (n != 0) {
                --p;
                if (p && p && (*(unsigned*)p >> 31))
                    appFreeUni__FPv((*(void**)&AsMWStringStorage(p)->data));
                --n;
            }
            appFreeUni__FPv(data);
        }
    }
};
typedef Vector61204 FontCacheTransferBuffer;
// A separate macro-local allocator is needed at each throw site; inline helpers alter .text/extab.
#define LENGTH_ERROR61204()                                                                        \
    do {                                                                                           \
        volatile CacheAllocatorByte errorAllocator;                                                \
        errorAllocator.value = 0;                                                                  \
        const char* messageValue =                                                                 \
            sharedResourceLoaderStrings + IFF_OFFSET(ResourceLoaderStringPool, vectorLengthError); \
        throw std::length_error(messageValue);                                                     \
    } while (0)
struct FixedFontCacheView {
    char reserved0[0x2c];
    NamedResourceEntry* entries;
    unsigned count, capacity;
};
// Borrow a font key span; inline descriptor bytes must never be read as a heap pointer.
static inline void BorrowCachedFontNameSpan(const NamedResourceEntry* entry, const char*& text, unsigned& dataLength) {
    if (!(*(unsigned*)entry->key.repr >> 31)) {
        text = entry->key.repr + 1;
        dataLength = *(unsigned char*)entry->key.repr & 0x7f;
    } else {
        text = (*(const char**)&AsMWStringStorage(entry->key.repr)->data);
        dataLength = AsMWStringStorage(entry->key.repr)->length;
    }
}

#pragma opt_lifetimes off
// Create from a cached font factory, then retain or append the returned font instance.
// nameArg is a borrowed C string. Return null for missing factory/create failure;
// otherwise return the manager-tracked instance. Exact (name,instance) hits gain
// a reference except at 0xFFFFFFFF. A shared name alone never replaces a value.
void* FixedFontCache_GetOrCreate(void* selfArg, void* nameArg) {
    FixedFontCacheView* self = (FixedFontCacheView*)selfArg;
    const char* key = (const char*)nameArg;
    void* factory;
    void* value;
    // Phase 1: find the first exact-name factory (name comparison only).
    {
        FontCacheSnapshotIterator first(self->entries);
        FontCacheSnapshotIterator position(first);
        // Original 0x80061204: assign the absent factory only at exhaustion.
        for (; position != FontCacheSnapshotIterator(self->entries + self->count) || (factory = 0, false);
             ++position.pointer) {
            NamedResourceEntry* entry = position.pointer;
            unsigned selected;
            unsigned savedDataLength;
            unsigned savedKeyLength;
            unsigned savedEntryLength;
            unsigned keyLength = strlen(key);
            savedKeyLength = keyLength;
            unsigned entryLength;
            if (*(unsigned*)entry->key.repr >> 31)
                entryLength = AsMWStringStorage(entry->key.repr)->length;
            else
                entryLength = *(unsigned char*)entry->key.repr & 0x7f;
            savedEntryLength = entryLength;
            const char* text;
            unsigned dataLength;
            BorrowCachedFontNameSpan(entry, text, dataLength);
            savedDataLength = dataLength;
            selected = MinimumCacheKeyLength(savedDataLength, savedEntryLength);
            int comparison = memcmp(text, key, MinimumCacheKeyLength(selected, savedKeyLength));
            if (comparison == 0) {
                if (selected < keyLength)
                    comparison = -1;
                else {
                    comparison = 0;
                    if (selected > keyLength)
                        comparison = 1;
                }
            }
            if (comparison == 0) {
                factory = entry->value;
                break;
            }
        }
    }
    if (factory) {
        // Phase 2: create an instance, then retain only an exact (name,instance) hit.
        value = ((FixedFontFactoryInterface*)factory)->create();
        if (value) {
            ((ManagedResourceHeader*)value)->resourceManager = self;
            FontCacheSnapshotIterator first(self->entries);
            FontCacheSnapshotIterator position(first);
            for (; position != FontCacheSnapshotIterator(self->entries + self->count); ++position.pointer) {
                NamedResourceEntry* entry = position.pointer;
                unsigned selected;
                unsigned savedDataLength;
                unsigned savedKeyLength;
                unsigned savedEntryLength;
                unsigned keyLength = strlen(key);
                savedKeyLength = keyLength;
                unsigned entryLength;
                if (*(unsigned*)entry->key.repr >> 31)
                    entryLength = AsMWStringStorage(entry->key.repr)->length;
                else
                    entryLength = *(unsigned char*)entry->key.repr & 0x7f;
                savedEntryLength = entryLength;
                const char* text;
                unsigned dataLength;
                BorrowCachedFontNameSpan(entry, text, dataLength);
                savedDataLength = dataLength;
                selected = MinimumCacheKeyLength(savedDataLength, savedEntryLength);
                int comparison = memcmp(text, key, MinimumCacheKeyLength(selected, savedKeyLength));
                if (comparison == 0) {
                    if (selected < keyLength)
                        comparison = -1;
                    else {
                        comparison = 0;
                        if (selected > keyLength)
                            comparison = 1;
                    }
                }
                if (comparison == 0 && entry->value == value) {
                    unsigned refs = ((ManagedResourceHeader*)value)->referenceCount;
                    if (refs != kImmortalReferenceCount)
                        ((ManagedResourceHeader*)value)->referenceCount = refs + 1;
                    // Skip insertion while retaining the original exception-cleanup boundaries.
                    goto fontInstanceReady;
                }
            }
            {
                // Phase 3: append a new pair; an equal name with a different value stays.
                // Copy its key directly into spare capacity, or reserve
                // a guarded allocation and construct its new tail there first.
                LocalNString_u8006028C str(key);
                CacheInsertionEntry pair(*(const CacheInsertionKey*)&str, value);
                volatile CacheAllocatorByte pushAllocator;
                pushAllocator.value = 0;
                unsigned size = self->count;
                unsigned capacity = self->capacity;
                if (size < capacity) {
                    unsigned* dst = (unsigned*)(self->entries + size);
                    if (dst) {
                        try {
                            if (!(pair.w[0] >> 31)) {
                                unsigned first = pair.w[0];
                                unsigned second = pair.w[1];
                                dst[0] = first;
                                dst[1] = second;
                                dst[2] = pair.w[2];
                            } else {
                                dst[0] = 0;
                                dst[1] = 0;
                                dst[2] = 0;
                                String_ReserveCapacity(dst, pair.w[1]);
                                volatile CacheAllocatorByte a;
                                CacheAllocatorByte t;
                                t.value = a.value;
                                String_ReplaceRange(dst, 0, 0, (void*)pair.w[2], (char*)pair.w[2] + pair.w[1], &t);
                            }
                            dst[3] = (unsigned)pair.value;
                        } catch (...) {
                            for (;;)
                                throw;
                        }
                    }
                    ++self->count;
                } else {
                    volatile CacheAllocatorByte insertAllocator;
                    insertAllocator.value = 0;
                    volatile CacheAllocatorByte vectorAllocator;
                    vectorAllocator.value = 0;
                    if (kMaxCacheEntries - capacity < 1)
                        LENGTH_ERROR61204();
                    unsigned need = self->count + 1 - self->capacity;
                    FontCacheTransferBuffer buffer(&self->capacity);
                    capacity = self->capacity;
                    unsigned newCapacity;
                    if (need > kMaxCacheEntries - capacity)
                        LENGTH_ERROR61204();
                    if (capacity < kFirstGrowthThreshold) {
                        unsigned add = (capacity + 1) * 3 / 5;
                        newCapacity = capacity + MaximumCacheGrowth(add, need);
                    } else if (capacity < kSecondGrowthThreshold) {
                        unsigned add = (capacity + 1) / 2;
                        newCapacity = capacity + MaximumCacheGrowth(add, need);
                    } else
                        newCapacity = kMaxCacheEntries;
                    volatile CacheAllocatorByte reserveAllocator;
                    reserveAllocator.value = 0;
                    if (newCapacity > kMaxCacheEntries)
                        LENGTH_ERROR61204();
                    buffer.data = (NamedResourceEntry*)appMallocDummy__FUl(newCapacity * 16);
                    buffer.capacity = newCapacity;
                    buffer.offset = self->count;
                    unsigned* dst = (unsigned*)(buffer.data + buffer.offset + buffer.size);
                    volatile CacheAllocatorByte constructAllocator;
                    constructAllocator.value = 0;
                    unsigned remaining = 1;
                    while (remaining != 0) {
                        if (dst) {
                            try {
                                if (!(pair.w[0] >> 31)) {
                                    unsigned first = pair.w[0];
                                    unsigned second = pair.w[1];
                                    dst[0] = first;
                                    dst[1] = second;
                                    dst[2] = pair.w[2];
                                } else {
                                    dst[0] = 0;
                                    dst[1] = 0;
                                    dst[2] = 0;
                                    String_ReserveCapacity(dst, pair.w[1]);
                                    volatile CacheAllocatorByte a;
                                    CacheAllocatorByte t;
                                    t.value = a.value;
                                    String_ReplaceRange(dst, 0, 0, (void*)pair.w[2], (char*)pair.w[2] + pair.w[1], &t);
                                }
                                dst[3] = (unsigned)pair.value;
                            } catch (...) {
                                for (;;)
                                    throw;
                            }
                        }
                        --remaining;
                        dst += 4;
                        ++buffer.size;
                    }
                    // Move old key representations as words and zero the source:
                    // this transfers heap ownership rather than copying characters.
                    NamedResourceEntry* begin = self->entries;
                    volatile CacheAllocatorByte moveAllocator;
                    moveAllocator.value = 0;
                    int n = ((int)(begin + self->count) - (int)begin) / 16;
                    buffer.offset -= n;
                    unsigned bytes = n * 16;
                    memcpy(buffer.data + buffer.offset, begin, bytes);
                    memset(begin, 0, bytes);
                    unsigned newSize = buffer.size + n;
                    begin = self->entries;
                    NamedResourceEntry* firstMoved = begin + buffer.offset;
                    NamedResourceEntry* storage = buffer.data;
                    unsigned oldSize = self->count;
                    buffer.size = newSize;
                    NamedResourceEntry* end = firstMoved + oldSize;
                    unsigned oldCap = self->capacity;
                    unsigned newCap = buffer.capacity;
                    buffer.data = begin;
                    volatile CacheAllocatorByte destroyAllocator;
                    destroyAllocator.value = 0;
                    // Publish the allocation swap. The guard now frees old storage.
                    self->capacity = newCap;
                    buffer.capacity = oldCap;
                    self->entries = storage;
                    self->count = newSize;
                    buffer.size = oldSize;
                    while (end > firstMoved) {
                        --end;
                        if (end && end && (*(unsigned*)end >> 31))
                            appFreeUni__FPv((*(void**)&AsMWStringStorage(end)->data));
                    }
                    buffer.size = 0;
                }
            }
        fontInstanceReady:
            return value;
        }
    }
    return 0;
}
#pragma opt_lifetimes reset
// Keep the allocator and temporary exception within each caller's original cleanup scope.
#define LENGTH_ERROR63788()                                                                        \
    do {                                                                                           \
        volatile CacheAllocatorByte errorAllocator;                                                \
        errorAllocator.value = 0;                                                                  \
        const char* messageValue =                                                                 \
            sharedResourceLoaderStrings + IFF_OFFSET(ResourceLoaderStringPool, vectorLengthError); \
        throw std::length_error(messageValue);                                                     \
    } while (0)
// Construct/cache a puppet after async reading, freeing read data on success or failure.
// Return the model address encoded as int once (state 4/5 -> 6), else zero.
// This does not poll I/O. Call PollResourceRead first. Loader result zero is
// success; nonzero deletes the failed model and read bytes, leaving state at 4.
int ResourceManager_PollPuppetResourceLoad(ResourceManager* self) {
    void* value;
    int state = self->loadState;
    if (state == kCachedModelReady) {
        self->loadState = kModelReturned;
        return (int)self->loadedModel;
    }
    if (self->resourceData == 0 || state != kReadComplete)
        return 0;
    // Decode the completed read into a new manager-associated puppet.
    void* obj = new LoadedPuppetStorage;
    self->loadedModel = obj;
    ((ManagedResourceHeader*)obj)->resourceManager = self;
    int result = WPuppet_LoadNamedResourceFromChunks(self->loadedModel, self->loadPath, self->resourceData,
                                                     (void*)self->resourceDataBytes, 0, self->loadFormat);
    // Original 0x80063788: successful loading owns the resource cleanup path.
    if (result == 0) {
        // Release source bytes and normalize opacity before exact-pair cache lookup.
        void* data = self->resourceData;
        if (data) {
            __dla__FPv(data);
            self->resourceData = 0;
        }
        WPuppet_SetOpacityRecursive(self->loadedModel, kOpaque);
        ResourceEntryIterator first(((NamedResourceEntry*)self->cachedModels));
        ResourceEntryIterator position(first);
        value = self->loadedModel;
        NamedResourceEntry* end;
        for (;
             end = ((NamedResourceEntry*)self->cachedModels) + self->cachedModelCount, !same_pos(end, position.pointer);
             ++position.pointer) {
            NamedResourceEntry* entry = position.pointer;
            unsigned selected;
            unsigned savedDataLength;
            unsigned savedKeyLength;
            unsigned savedEntryLength;
            unsigned keyLength = strlen(self->loadPath);
            savedKeyLength = keyLength;
            unsigned entryLength;
            if (*(unsigned*)entry->key.repr >> 31)
                entryLength = AsMWStringStorage(entry->key.repr)->length;
            else
                entryLength = *(unsigned char*)entry->key.repr & 0x7f;
            savedEntryLength = entryLength;
            const char* text;
            unsigned dataLength;
            BorrowCachedFontNameSpan(entry, text, dataLength);
            savedDataLength = dataLength;
            selected = MinimumCacheKeyLength(savedDataLength, savedEntryLength);
            int comparison = memcmp(text, self->loadPath, MinimumCacheKeyLength(selected, savedKeyLength));
            if (comparison == 0) {
                if (selected < keyLength)
                    comparison = -1;
                else {
                    comparison = 0;
                    if (selected > keyLength)
                        comparison = 1;
                }
            }
            if (comparison == 0 && entry->value == value) {
                unsigned refs = ((ManagedResourceHeader*)value)->referenceCount;
                if (refs != kImmortalReferenceCount)
                    ((ManagedResourceHeader*)value)->referenceCount = refs + 1;
                // Join PET matrix refresh and completion without entering the insertion cleanup scope.
                goto modelInstanceReady;
            }
        }
        {
            // Append the new pair in-place when possible; otherwise construct
            // a guarded tail, transfer the old prefix, then commit the swap.
            CacheInsertionKey str(self->loadPath);
            CacheInsertionEntry pair(str, value);
            volatile CacheAllocatorByte pushAllocator;
            pushAllocator.value = 0;
            unsigned size = self->cachedModelCount;
            unsigned capacity = self->cachedModelCapacity;
            if (size < capacity) {
                unsigned* dst = (unsigned*)(((NamedResourceEntry*)self->cachedModels) + size);
                if (dst) {
                    try {
                        if (!(pair.w[0] >> 31)) {
                            unsigned first = pair.w[0];
                            unsigned second = pair.w[1];
                            dst[0] = first;
                            dst[1] = second;
                            dst[2] = pair.w[2];
                        } else {
                            dst[0] = 0;
                            dst[1] = 0;
                            dst[2] = 0;
                            String_ReserveCapacity(dst, pair.w[1]);
                            volatile CacheAllocatorByte a;
                            CacheAllocatorByte t;
                            t.value = a.value;
                            String_ReplaceRange(dst, 0, 0, (void*)pair.w[2], (char*)pair.w[2] + pair.w[1], &t);
                        }
                        dst[3] = (unsigned)pair.value;
                    } catch (...) {
                        // Rethrowing never returns; keep the catch cleanup unreachable.
                        for (;;)
                            throw;
                    }
                }
                ++self->cachedModelCount;
            } else {
                volatile CacheAllocatorByte insertAllocator;
                insertAllocator.value = 0;
                volatile CacheAllocatorByte vectorAllocator;
                vectorAllocator.value = 0;
                if (kMaxCacheEntries - capacity < 1)
                    LENGTH_ERROR63788();
                unsigned need = self->cachedModelCount + 1 - self->cachedModelCapacity;
                CacheTransferBuffer buffer(&self->cachedModelCapacity);
                capacity = self->cachedModelCapacity;
                unsigned newCapacity;
                if (need > kMaxCacheEntries - capacity)
                    LENGTH_ERROR63788();
                if (capacity < kFirstGrowthThreshold) {
                    unsigned add = (capacity + 1) * 3 / 5;
                    unsigned* p = &add;
                    if (add < need)
                        p = &need;
                    newCapacity = capacity + *p;
                } else if (capacity < kSecondGrowthThreshold) {
                    unsigned add = (capacity + 1) / 2;
                    unsigned* p = &add;
                    if (add < need)
                        p = &need;
                    newCapacity = capacity + *p;
                } else
                    newCapacity = kMaxCacheEntries;
                volatile CacheAllocatorByte reserveAllocator;
                reserveAllocator.value = 0;
                if (newCapacity > kMaxCacheEntries)
                    LENGTH_ERROR63788();
                buffer.data = (NamedResourceEntry*)appMallocDummy__FUl(newCapacity * 16);
                buffer.capacity = newCapacity;
                buffer.offset = self->cachedModelCount;
                unsigned* dst = (unsigned*)(buffer.data + buffer.offset + buffer.size);
                volatile CacheAllocatorByte constructAllocator;
                constructAllocator.value = 0;
                unsigned remaining = 1;
                while (remaining != 0) {
                    if (dst) {
                        try {
                            if (!(pair.w[0] >> 31)) {
                                unsigned first = pair.w[0];
                                unsigned second = pair.w[1];
                                dst[0] = first;
                                dst[1] = second;
                                dst[2] = pair.w[2];
                            } else {
                                dst[0] = 0;
                                dst[1] = 0;
                                dst[2] = 0;
                                String_ReserveCapacity(dst, pair.w[1]);
                                volatile CacheAllocatorByte a;
                                CacheAllocatorByte t;
                                t.value = a.value;
                                String_ReplaceRange(dst, 0, 0, (void*)pair.w[2], (char*)pair.w[2] + pair.w[1], &t);
                            }
                            dst[3] = (unsigned)pair.value;
                        } catch (...) {
                            for (;;)
                                throw;
                        }
                    }
                    --remaining;
                    dst += 4;
                    ++buffer.size;
                }
                // Move tagged-key words and clear their former owners.
                NamedResourceEntry* begin = ((NamedResourceEntry*)self->cachedModels);
                volatile CacheAllocatorByte moveAllocator;
                moveAllocator.value = 0;
                int n = ((int)(begin + self->cachedModelCount) - (int)begin) / 16;
                buffer.offset -= n;
                unsigned byteCount = n * 16;
                memcpy(buffer.data + buffer.offset, begin, byteCount);
                memset(begin, 0, byteCount);
                unsigned newSize = buffer.size + n;
                begin = ((NamedResourceEntry*)self->cachedModels);
                NamedResourceEntry* first = begin + buffer.offset;
                NamedResourceEntry* storage = buffer.data;
                unsigned oldSize = self->cachedModelCount;
                buffer.size = newSize;
                NamedResourceEntry* end = first + oldSize;
                unsigned oldCap = self->cachedModelCapacity;
                unsigned newCap = buffer.capacity;
                buffer.data = begin;
                volatile CacheAllocatorByte destroyAllocator;
                destroyAllocator.value = 0;
                // Commit; old keys are now empty and the guard frees their block.
                self->cachedModelCapacity = newCap;
                buffer.capacity = oldCap;
                self->cachedModels = storage;
                self->cachedModelCount = newSize;
                buffer.size = oldSize;
                while (end > first) {
                    --end;
                    if (end && end && (*(unsigned*)end >> 31))
                        appFreeUni__FPv((*(void**)&AsMWStringStorage(end)->data));
                }
                buffer.size = 0;
            }
        }
    modelInstanceReady:
        if (self->loadFormat == kFormatPet)
            WPuppet_RefreshMatrixBuffers(self->loadedModel);
        self->loadState = kModelReturned;
        return (int)self->loadedModel;
    }
    // Decode failed: release the new puppet and input bytes without publishing it.
    void* old = self->loadedModel;
    if (old) {
        if (old)
            ((LoadedPuppetDeleteInterface*)old)->destroy(1);
        self->loadedModel = 0;
    }
    void* data = self->resourceData;
    if (data) {
        __dla__FPv(data);
        self->resourceData = 0;
    }
    return 0;
}

#undef LENGTH_ERROR63788
struct FontInsertionIterator {
    NamedResourceEntry* pointer;
    // Borrow a position and preserve the original volatile snapshot lifetime.
    FontInsertionIterator(NamedResourceEntry* p) : pointer(p) {
        NamedResourceEntry* volatile snapshot = p;
    }
    // Copy the position with the same snapshot side effect.
    FontInsertionIterator(const FontInsertionIterator& p) : pointer(p.pointer) {
        NamedResourceEntry* volatile snapshot = p.pointer;
    }
    // Compare positions after snapshotting the other position.
    bool equals(const FontInsertionIterator& other) const {
        NamedResourceEntry* volatile snapshot = other.pointer;
        return other.pointer == pointer;
    }
};
// Decode a font entry's tagged key length in bytes.
inline unsigned FontEntryKeyLength(const NamedResourceEntry* entry) {
    if (!(*(unsigned*)entry->key.repr >> 31))
        return *(unsigned char*)entry->key.repr & 0x7f;
    return AsMWStringStorage(entry->key.repr)->length;
}
// Keep the allocator and temporary exception within each caller's original cleanup scope.
#define LENGTH_ERROR650BC()                                                                        \
    do {                                                                                           \
        volatile CacheAllocatorByte errorAllocator;                                                \
        errorAllocator.value = 0;                                                                  \
        const char* messageValue =                                                                 \
            sharedResourceLoaderStrings + IFF_OFFSET(ResourceLoaderStringPool, vectorLengthError); \
        throw std::length_error(messageValue);                                                     \
    } while (0)
struct FixedFontEntryVector {
    void* vt;
    NamedResourceEntry* data;
    unsigned size, capacity;
};
#pragma opt_lifetimes off
// Retain an identical key/value pair or append a new cached font entry.
// Return 0 for an exact-pair hit (retained unless count=0xFFFFFFFF), 1 for append.
// keyArg is a borrowed C string copied on append; valueArg is manager-tracked.
// Despite the shared name, a matching key with a different value is not assigned.
int FixedFontCache_RetainOrAppend(void* selfArg, void* keyArg, void* valueArg) {
    FixedFontEntryVector* self = (FixedFontEntryVector*)selfArg;
    void* value;
    const char* key;
    NamedResourceEntry* firstEntry;
    value = valueArg;
    key = (const char*)keyArg;
    // Phase 1: exact-pair lookup. A key match alone never updates the existing value.
    firstEntry = self->data;
    FontInsertionIterator first(firstEntry);
    FontInsertionIterator position(first);
    for (; !position.equals(FontInsertionIterator(self->data + self->size)); ++position.pointer) {
        NamedResourceEntry* entry = position.pointer;
        unsigned selected;
        unsigned savedDataLength;
        unsigned savedKeyLength;
        unsigned savedEntryLength;
        unsigned keyLength = strlen(key);
        savedKeyLength = keyLength;
        unsigned entryLength;
        if (!(*(unsigned*)entry->key.repr >> 31))
            entryLength = *(unsigned char*)entry->key.repr & 0x7f;
        else
            entryLength = AsMWStringStorage(entry->key.repr)->length;
        savedEntryLength = entryLength;
        const char* text;
        unsigned dataLength;
        BorrowCachedFontNameSpan(entry, text, dataLength);
        savedDataLength = dataLength;
        selected = MinimumCacheKeyLength(savedDataLength, savedEntryLength);
        int comparison = memcmp(text, key, MinimumCacheKeyLength(selected, savedKeyLength));
        if (comparison == 0) {
            if (selected < keyLength)
                comparison = -1;
            else {
                comparison = 0;
                if (selected > keyLength)
                    comparison = 1;
            }
        }
        if (comparison == 0 && entry->value == value) {
            unsigned refs = ((ManagedResourceHeader*)value)->referenceCount;
            if (refs != kImmortalReferenceCount)
                ((ManagedResourceHeader*)value)->referenceCount = refs + 1;
            return 0;
        }
    }
    // Phase 2: no exact pair; construct an owning key and borrowed value.
    // Inline keys occupy all three descriptor words, including length/data storage.
    // Phase 3: append into spare capacity, or construct a guarded new tail.
    LocalNString_u8006028C str(key);
    CacheInsertionEntry pair(*(const CacheInsertionKey*)&str, value);
    volatile CacheAllocatorByte pushAllocator;
    pushAllocator.value = 0;
    unsigned size = self->size;
    unsigned capacity = self->capacity;
    if (size < capacity) {
        NamedResourceEntry* destination = (self->data + size);
        if (destination) {
            try {
                if (!(pair.w[0] >> 31)) {
                    unsigned first = pair.w[0];
                    unsigned second = pair.w[1];
                    AsMWStringStorage(destination)->flags = first;
                    AsMWStringStorage(destination)->length = second;
                    AsMWStringStorage(destination)->data = (char*)pair.w[2];
                } else {
                    AsMWStringStorage(destination)->flags = 0;
                    AsMWStringStorage(destination)->length = 0;
                    AsMWStringStorage(destination)->data = 0;
                    String_ReserveCapacity(destination, pair.w[1]);
                    volatile CacheAllocatorByte a;
                    CacheAllocatorByte t;
                    t.value = a.value;
                    String_ReplaceRange(destination, 0, 0, (void*)pair.w[2], (char*)pair.w[2] + pair.w[1], &t);
                }
                destination->value = pair.value;
            } catch (...) {
                for (;;)
                    throw;
            }
        }
        ++self->size;
    } else {
        volatile CacheAllocatorByte insertAllocator;
        insertAllocator.value = 0;
        volatile CacheAllocatorByte vectorAllocator;
        vectorAllocator.value = 0;
        if (kMaxCacheEntries - capacity < 1)
            LENGTH_ERROR650BC();
        unsigned newCapacity;
        unsigned need = self->size + 1 - self->capacity;
        FontCacheTransferBuffer buffer(&self->capacity);
        capacity = self->capacity;
        if (need > kMaxCacheEntries - capacity)
            LENGTH_ERROR650BC();
        if (capacity < kFirstGrowthThreshold) {
            unsigned add = (capacity + 1) * 3 / 5;
            newCapacity = capacity + MaximumCacheGrowth(add, need);
        } else if (capacity < kSecondGrowthThreshold) {
            unsigned add = (capacity + 1) / 2;
            newCapacity = capacity + MaximumCacheGrowth(add, need);
        } else
            newCapacity = kMaxCacheEntries;
        volatile CacheAllocatorByte reserveAllocator;
        reserveAllocator.value = 0;
        if (newCapacity > kMaxCacheEntries)
            LENGTH_ERROR650BC();
        buffer.data = (NamedResourceEntry*)appMallocDummy__FUl(newCapacity * 16);
        buffer.capacity = newCapacity;
        buffer.offset = self->size;
        NamedResourceEntry* destination = (buffer.data + buffer.offset + buffer.size);
        volatile CacheAllocatorByte constructAllocator;
        constructAllocator.value = 0;
        unsigned remaining = 1;
        while (remaining != 0) {
            if (destination) {
                try {
                    if (!(pair.w[0] >> 31)) {
                        unsigned first = pair.w[0];
                        unsigned second = pair.w[1];
                        AsMWStringStorage(destination)->flags = first;
                        AsMWStringStorage(destination)->length = second;
                        AsMWStringStorage(destination)->data = (char*)pair.w[2];
                    } else {
                        AsMWStringStorage(destination)->flags = 0;
                        AsMWStringStorage(destination)->length = 0;
                        AsMWStringStorage(destination)->data = 0;
                        String_ReserveCapacity(destination, pair.w[1]);
                        volatile CacheAllocatorByte a;
                        CacheAllocatorByte t;
                        t.value = a.value;
                        String_ReplaceRange(destination, 0, 0, (void*)pair.w[2], (char*)pair.w[2] + pair.w[1], &t);
                    }
                    destination->value = pair.value;
                } catch (...) {
                    for (;;)
                        throw;
                }
            }
            --remaining;
            ++destination;
            ++buffer.size;
        }
        unsigned bytes;
        NamedResourceEntry* begin;
        int n;
        NamedResourceEntry* end;
        NamedResourceEntry* firstMoved;
        // Phase 4: transfer old key ownership, then publish the new vector.
        begin = self->data;
        volatile CacheAllocatorByte moveAllocator;
        moveAllocator.value = 0;
        n = ((int)(begin + self->size) - (int)begin) / 16;
        buffer.offset -= n;
        bytes = n * 16;
        memcpy(buffer.data + buffer.offset, begin, bytes);
        memset(begin, 0, bytes);
        unsigned newSize = buffer.size + n;
        NamedResourceEntry* oldBegin = self->data;
        firstMoved = oldBegin + buffer.offset;
        NamedResourceEntry* storage = buffer.data;
        unsigned oldSize = self->size;
        buffer.size = newSize;
        end = firstMoved + oldSize;
        unsigned oldCap = self->capacity;
        unsigned newCap = buffer.capacity;
        buffer.data = oldBegin;
        volatile CacheAllocatorByte destroyAllocator;
        destroyAllocator.value = 0;
        self->capacity = newCap;
        buffer.capacity = oldCap;
        self->data = storage;
        self->size = newSize;
        buffer.size = oldSize;
        while (end > firstMoved) {
            --end;
            if (end && end && (*(unsigned*)end >> 31))
                appFreeUni__FPv((*(void**)&AsMWStringStorage(end)->data));
        }
        buffer.size = 0;
    }
    return 1;
}
#pragma opt_lifetimes reset

// Retain an identical key/value pair or append a new cached resource entry.
// Return 0 for an exact-pair hit (retained unless count=0xFFFFFFFF), 1 for append.
// key is borrowed and copied on append; value remains tracked by the manager.
// A matching key with a different value appends another entry, without assignment.
int ResourceCache_RetainOrAppend(void* self, const char* key, void* value) {
    ResourceEntryIterator first((*(NamedResourceEntry**)&AsResourceEntryVector(self)->entries));
    ResourceEntryIterator position(first);
    for (; position != ResourceEntryIterator((*(NamedResourceEntry**)&AsResourceEntryVector(self)->entries) +
                                             AsResourceEntryVector(self)->count);
         ++position.pointer) {
        NamedResourceEntry* entry = position.pointer;
        unsigned keyLength = strlen(key);
        unsigned savedKeyLength = keyLength;
        unsigned entryLength = ResourceEntryKeyLength(entry);
        unsigned savedEntryLength = entryLength;
        const char* text;
        unsigned dataLength;
        BorrowCachedFontNameSpan(entry, text, dataLength);
        unsigned savedDataLength = dataLength;
        unsigned selected = MinimumCacheKeyLength(savedDataLength, savedEntryLength);
        int comparison = memcmp(text, key, MinimumCacheKeyLength(selected, savedKeyLength));
        if (comparison == 0) {
            if (selected < keyLength)
                comparison = -1;
            else if (selected != keyLength)
                comparison = 1;
        }
        if (comparison == 0 && entry->value == value) {
            unsigned refs = ((ManagedResourceHeader*)value)->referenceCount;
            if (refs != kImmortalReferenceCount)
                ((ManagedResourceHeader*)value)->referenceCount = refs + 1;
            return 0;
        }
    }
    // Append directly if capacity permits; otherwise reserve a guarded new tail.
    CacheInsertionKey str(key);
    CacheInsertionEntry pair(str, value);
    volatile CacheAllocatorByte pushAllocator;
    pushAllocator.value = 0;
    unsigned size = AsResourceEntryVector(self)->count;
    unsigned capacity = AsResourceEntryVector(self)->capacity;
    if (size < capacity) {
        NamedResourceEntry* destination = ((*(NamedResourceEntry**)&AsResourceEntryVector(self)->entries) + size);
        if (destination) {
            try {
                if (!(pair.w[0] >> 31)) {
                    unsigned first = pair.w[0];
                    unsigned second = pair.w[1];
                    AsMWStringStorage(destination)->flags = first;
                    AsMWStringStorage(destination)->length = second;
                    AsMWStringStorage(destination)->data = (char*)pair.w[2];
                } else {
                    AsMWStringStorage(destination)->flags = 0;
                    AsMWStringStorage(destination)->length = 0;
                    AsMWStringStorage(destination)->data = 0;
                    String_ReserveCapacity(destination, pair.w[1]);
                    volatile CacheAllocatorByte a;
                    CacheAllocatorByte t;
                    t.value = a.value;
                    String_ReplaceRange(destination, 0, 0, (void*)pair.w[2], (char*)pair.w[2] + pair.w[1], &t);
                }
                destination->value = pair.value;
            } catch (...) {
                __throw(0, 0, 0);
            }
        }
        ++AsResourceEntryVector(self)->count;
    } else {
        volatile CacheAllocatorByte insertAllocator;
        insertAllocator.value = 0;
        volatile CacheAllocatorByte vectorAllocator;
        vectorAllocator.value = 0;
        if (kMaxCacheEntries - capacity < 1)
            LENGTH_ERROR61204();
        unsigned need = AsResourceEntryVector(self)->count + 1 - AsResourceEntryVector(self)->capacity;
        CacheTransferBuffer buffer(&AsResourceEntryVector(self)->capacity);
        capacity = AsResourceEntryVector(self)->capacity;
        unsigned newCapacity;
        if (need > kMaxCacheEntries - capacity)
            LENGTH_ERROR61204();
        if (capacity < kFirstGrowthThreshold) {
            unsigned add = (capacity + 1) * 3 / 5;
            unsigned* p = &add;
            if (add < need)
                p = &need;
            newCapacity = capacity + *p;
        } else if (capacity < kSecondGrowthThreshold) {
            unsigned add = (capacity + 1) / 2;
            unsigned* p = &add;
            if (add < need)
                p = &need;
            newCapacity = capacity + *p;
        } else
            newCapacity = kMaxCacheEntries;
        volatile CacheAllocatorByte reserveAllocator;
        reserveAllocator.value = 0;
        if (newCapacity > kMaxCacheEntries)
            LENGTH_ERROR61204();
        buffer.data = (NamedResourceEntry*)appMallocDummy__FUl(newCapacity * 16);
        buffer.capacity = newCapacity;
        buffer.offset = AsResourceEntryVector(self)->count;
        NamedResourceEntry* destination = (buffer.data + buffer.offset + buffer.size);
        volatile CacheAllocatorByte constructAllocator;
        constructAllocator.value = 0;
        unsigned remaining = 1;
        while (remaining != 0) {
            if (destination) {
                try {
                    if (!(pair.w[0] >> 31)) {
                        unsigned first = pair.w[0];
                        unsigned second = pair.w[1];
                        AsMWStringStorage(destination)->flags = first;
                        AsMWStringStorage(destination)->length = second;
                        AsMWStringStorage(destination)->data = (char*)pair.w[2];
                    } else {
                        AsMWStringStorage(destination)->flags = 0;
                        AsMWStringStorage(destination)->length = 0;
                        AsMWStringStorage(destination)->data = 0;
                        String_ReserveCapacity(destination, pair.w[1]);
                        volatile CacheAllocatorByte a;
                        CacheAllocatorByte t;
                        t.value = a.value;
                        String_ReplaceRange(destination, 0, 0, (void*)pair.w[2], (char*)pair.w[2] + pair.w[1], &t);
                    }
                    destination->value = pair.value;
                } catch (...) {
                    __throw(0, 0, 0);
                }
            }
            --remaining;
            ++destination;
            ++buffer.size;
        }
        // Transfer the old prefix as key/value words, clearing old key ownership.
        NamedResourceEntry* begin = (*(NamedResourceEntry**)&AsResourceEntryVector(self)->entries);
        volatile CacheAllocatorByte moveAllocator;
        moveAllocator.value = 0;
        int n = ((int)(begin + AsResourceEntryVector(self)->count) - (int)begin) / 16;
        buffer.offset -= n;
        memcpy(buffer.data + buffer.offset, begin, n * 16);
        memset(begin, 0, n * 16);
        unsigned newSize = buffer.size + n;
        begin = (*(NamedResourceEntry**)&AsResourceEntryVector(self)->entries);
        NamedResourceEntry* firstMoved = begin + buffer.offset;
        NamedResourceEntry* storage = buffer.data;
        unsigned oldSize = AsResourceEntryVector(self)->count;
        buffer.size = newSize;
        NamedResourceEntry* end = firstMoved + oldSize;
        unsigned oldCap = AsResourceEntryVector(self)->capacity;
        unsigned newCap = buffer.capacity;
        buffer.data = begin;
        volatile CacheAllocatorByte destroyAllocator;
        destroyAllocator.value = 0;
        // Commit the new vector; the guard releases only the emptied old allocation.
        AsResourceEntryVector(self)->capacity = newCap;
        buffer.capacity = oldCap;
        (*(NamedResourceEntry**)&AsResourceEntryVector(self)->entries) = storage;
        AsResourceEntryVector(self)->count = newSize;
        buffer.size = oldSize;
        while (end > firstMoved) {
            --end;
            if (end && end && (*(unsigned*)end >> 31))
                appFreeUni__FPv((*(void**)&AsMWStringStorage(end)->data));
        }
        buffer.size = 0;
    }
    return 1;
}

#pragma pop
} // extern "C"

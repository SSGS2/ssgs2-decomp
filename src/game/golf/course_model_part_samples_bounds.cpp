// Course / model bounds and per-part triangle samples (0x8005F290): the ABI name
// PuppetMesh_CreatePartSamplesAndBounds must not imply a complete collision triangle soup.
// Only the FIRST transformed triangle for each part ID is retained. The function
// is called by CourseModel_LoadFromFile (course_model_scene_loader.cpp) after installing the GBIN
// scale/rotation/translation and updating the puppet's bone matrices. It does not
// read GBIN or choose a course/model file: change that caller for placement/loading.
// Mod lookup:
// - Input geometry: namedBones (+0x13C) -> matching descriptor/bone names ->
//   IPetMesh groups -> MeshPart triangle-list vertices -> Node::transform (+0x78).
// - Bounds: kBoundsInitMin/Max in phase1_shared_data.cpp are +100000/-100000
//   empty-box sentinels, shared with WPuppet_ComputeMeshBounds (wpuppet_mesh_bounds.cpp).
//   They are not clipping distances. No vertices leaves this inverted box intact.
// - Constructor fallback bounds: kFloat_neg6_3999996_Bounds/+6_3999996_Bounds
//   in the data section are exact +/-6.3999996185 model-distance floats;
//   extraction immediately overwrites them with the empty-box sentinels.
// - Normals: storeNormals != 0 stores normalize((v2-v1) cross (v0-v1)); preserve
//   winding, float operation order, and the first-triangle-per-part-ID rule.
// Bounds include ALL candidates even when their duplicate part ID is discarded.
// The GBIN caller uses those bounds and samples to build its bitmap-name map,
// then releases the temporary with CourseObjectList_Dtor(owner,1) in
// course_object_and_selection.cpp: delete active Triangle payloads, recycle links, free blocks/slots,
// free owner. Do not invoke List::~List alone to release this returned owner.
// The course's full triangle/material arrays are built separately in course_model_scene_loader.cpp.
// Course callers use the transformed output as world geometry; this routine
// applies only the supplied bone matrix and makes no yards/metres conversion.
// Valid source, descriptor, node and mesh pointers and complete vertex triples
// are required. No allocation-failure recovery or malformed-mesh checks exist.
#include "game/types.h"
#include "game/wvector.h"
#include "classes/WPuppet.h"
#include "sdk_protos.h"

// Local collision model: Source is a borrowed puppet entry array; Entry pairs a
// bone node with a descriptor whose name begins at +0x2C. Resource is a temporary
// reference-counted mesh handle. Obj is the returned collision-data owner; its List
// owns pooled Link blocks. Triangle pointers are retained in live links; this
// local List destructor frees the allocation-block ring at +8 and its lookup
// allocation at +0x1C, not Triangle payloads (LockedNodeList_dtor in wpuppet_mesh_bounds.cpp).
// Payload teardown therefore belongs to the collision-data consumer.
// MeshPart vertices are borrowed triangle lists in bone-local coordinates.
typedef Vec3f Vec3;
struct VectorWork {
    Vec3 a, b, transformed, normal;
}; // a=v0-v1, b=v2-v1; normal=normalize(b cross a).
struct Triangle {
    Vec3 v[3];
    Vec3 normal;
    char pad30[12];
    int id;
    // Allocate without initializing vertex/normal data; extraction fills used members.
    Triangle() {}
};
struct Link {
    Triangle* value;
    int field4;
    unsigned char flag;
    char pad9[3];
    Link* prev;
    Link* next;
    int pad14;
};
// Runtime Wii records, not serialized GBIN: 32-bit pointers, native big-endian
// floats. MeshPart is 0x10 bytes, Mesh is 8; vertices are packed 12-byte XYZ.
struct MeshPart {
    int id;
    unsigned count;
    Vec3* vertices;
    int padC;
};
struct Mesh {
    unsigned count;
    MeshPart* parts;
};
struct Resource {
    int refs;
    virtual ~Resource();
};
struct Node {
    char pad0[8];
    char name[0x70];
    float transform[12];
    // Borrow the inline bone name; the source node retains storage.
    const char* getName() const {
        return name;
    }
    // Borrow the affine bone transform used to place extracted vertices.
    const float* getTransform() const {
        return transform;
    }
};
struct Entry {
    Node* node;
    char* other;
    // Borrow the bone node paired with this source descriptor.
    Node* getNode() const {
        return node;
    }
};
struct Source {
    char pad0[0x13c];
    Entry* entries;
    unsigned count;
};

enum CollisionStorage {
    kLookupSlotCount = 64,
    kLookupAllocationBytes = 256,
    kPooledLinkBytes = 24,
    kCollisionNameBytes = 32,
    kPositionBytes = 12,
    kVerticesPerTriangle = 3
};

extern "C" {
void* __nw__FUl(unsigned int);
void __dl__FPv(void*);
void* appMallocUni__FUl(unsigned int);
void LockedNodeList_dtor(void*, int);
void Vec3_Zero(void*);
void __as__7WVectorFRC7WVector(void*, const void*);
int CString_Compare(const void*, const void*);
int WPuppet_CreateMeshByBoneName(void*, Resource**, const void*);
unsigned short IPetMesh_GetGroupCount(Resource*);
Mesh* IPetMesh_GetGroupByIndex(Resource*, unsigned short);
void* Matrix_TransformPoint(void*, const void*, const void*);
void __mi__FRC7WVectorRC7WVector(void*, const void*, const void*);
void Vector_CrossProduct(void*, const void*, const void*);
void Normalize__7WVectorFv(void*);
void* memcpy(void*, const void*, unsigned int);
void* memset(void*, int, unsigned int);
extern const float kFloat_neg6_3999996_Bounds, kFloat_6_3999996_Bounds, kBoundsInitMin, kBoundsInitMax;
}

class InstanceLock_u8005F290 {
    OSMutex* mutex;

public:
    // Lock the borrowed instance mutex for this scope.
    InstanceLock_u8005F290(OSMutex* m) : mutex(m) {
        OSLockMutex(mutex);
    }
    // Unlock the same mutex on normal or exceptional scope exit.
    ~InstanceLock_u8005F290() {
        OSUnlockMutex(mutex);
    }
};

// Extend a minimum bound with one coordinate, preserving original NaN ordering.
inline float lower(float bound, float value) {
    return bound > value ? value : bound;
}
// Extend a maximum bound with one coordinate, preserving original NaN ordering.
inline float upper(float bound, float value) {
    return bound < value ? value : bound;
}

// Circular live/free rings use null for an empty ring. blocks links the first
// Link of each allocation (a block header, never a payload slot). clear recycles
// live links and resets lookup slots; it does not delete Triangle payloads.
struct List {
    Link* head;
    Link* cursor;
    Link* blocks;
    Link* free;
    int capacity;
    int mask;
    int size;
    int* slots;
    OSMutex mutex;

    // Initialize empty rings and a 64-slot lookup array under the instance mutex.
    List() {
        slots = 0;
        OSInitMutex(&mutex);
        InstanceLock_u8005F290 constructorLock(&mutex);
        head = 0;
        free = 0;
        blocks = 0;
        capacity = kLookupSlotCount;
        cursor = 0;
        size = kLookupSlotCount;
        if (size > 0) {
            slots = (int*)appMallocUni__FUl(kLookupAllocationBytes);
            mask = size - 1;
            for (int i = 0; i < size; ++i)
                slots[i] = 0;
        } else {
            slots = 0;
        }
    }
    // Delegate link/lookup teardown to LockedNodeList_dtor; this does not delete Triangle*.
    ~List() {
        LockedNodeList_dtor(this, -1);
    }

    // Splice node before into, or create a one-node ring; return the ring origin.
    Link* insert(Link* into, Link* node) {
        OSMutex* m = &mutex;
        OSLockMutex(m);
        if (!into) {
            node->prev = node;
            node->next = node;
            OSUnlockMutex(m);
            return node;
        }
        node->next = into;
        node->prev = into->prev;
        into->prev = node;
        node->prev->next = node;
        OSUnlockMutex(m);
        return into;
    }
    // Unlink a live node and return its successor, or null for the last node.
    Link* remove(Link* node) {
        OSMutex* m = &mutex;
        OSLockMutex(m);
        node->next->prev = node->prev;
        node->prev->next = node->next;
        Link* next = node->next;
        if (next == node)
            next = 0;
        OSUnlockMutex(m);
        return next;
    }
    // Unlink the current ring head without releasing its payload.
    Link* removeFirst() {
        return remove(head);
    }
    // Recycle all live links into the free ring; keep allocated blocks and slots.
    void clear() {
        InstanceLock_u8005F290 clearLock(&mutex);
        if (head && free) {
            head->prev->next = free->next;
            free->next->prev = head->prev;
            free->next = head;
            head->prev = free;
            head = 0;
        } else {
            Link* node;
            while ((node = head) != 0) {
                Link* next = removeFirst();
                free = insert(free, head);
                head = next;
            }
        }
        for (int i = 0; i < size; ++i)
            slots[i] = 0;
    }
    // Borrow the next live triangle and advance the cursor once around the ring.
    Triangle* next() {
        OSMutex* m = &mutex;
        OSLockMutex(m);
        if (cursor) {
            Triangle* value = cursor->value;
            Link* n = cursor->next;
            if (n == head)
                n = 0;
            cursor = n;
            OSUnlockMutex(m);
            return value;
        }
        OSUnlockMutex(m);
        return 0;
    }
    // Reset iteration to the live head and borrow its first triangle.
    Triangle* first() {
        InstanceLock_u8005F290 firstLock(&mutex);
        cursor = head;
        return next();
    }
    // Borrow a recycled link; on exhaustion allocate a block with one header link.
    Link* allocate() {
        InstanceLock_u8005F290 allocateLock(&mutex);
        if (!free) {
            Link* nodes = (Link*)appMallocUni__FUl(capacity * kPooledLinkBytes);
            blocks = insert(blocks, nodes);
            int i = 1;
            for (; i < capacity; ++i) {
                free = insert(free, nodes + i);
            }
        }
        Link* node = free;
        free = remove(node);
        return node;
    }
    // Retain triangle in a live link; list-node teardown does not delete the payload.
    void append(Triangle* triangle) {
        InstanceLock_u8005F290 appendLock(&mutex);
        Link* node = allocate();
        node->value = triangle;
        node->flag = 0;
        node->field4 = 0;
        head = insert(head, node);
    }
};

struct Obj {
    int candidateTriangleCount; // Candidate triangles, including candidates discarded by part ID.
    List list;
    Vec3 min;
    Vec3 max;
    char name[0x20];
    // Initialize empty collision data with default +/-6.4 bounds before extraction.
    Obj() : candidateTriangleCount(0) {
        Vec3_Zero(&min);
        Vec3_Zero(&max);
        list.clear();
        memset(name, 0, kCollisionNameBytes);
        Vec3 hi, lo;
        lo.x = kFloat_neg6_3999996_Bounds;
        lo.y = kFloat_neg6_3999996_Bounds;
        lo.z = kFloat_neg6_3999996_Bounds;
        __as__7WVectorFRC7WVector(&min, &lo);
        hi.x = kFloat_6_3999996_Bounds;
        hi.y = kFloat_6_3999996_Bounds;
        hi.z = kFloat_6_3999996_Bounds;
        __as__7WVectorFRC7WVector(&max, &hi);
    }
};

typedef char Obj_min_is_at_0x3C[((int)&((Obj*)0)->min == 0x3C) ? 1 : -1];
typedef char Obj_max_is_at_0x48[((int)&((Obj*)0)->max == 0x48) ? 1 : -1];
enum CollisionLayout {
    kDescriptorNameByteOffset = 0x2C,
    kMinimumBoundFloatWord = (int)&((Obj*)0)->min / sizeof(float),
    kMaximumBoundFloatWord = (int)&((Obj*)0)->max / sizeof(float)
};

// Collect transformed bounds and the first triangle per part ID for course setup.
// Returns a newly allocated owner to the caller. storeNormals requests stored unit normals;
// otherwise normal is left uninitialized. No allocation/mesh-count validation here.
// Obj::candidateTriangleCount counts candidate triangles, including duplicates discarded by part ID;
// bounds also include discarded candidates. Retained list length may be smaller.
// Input vertex counts must be multiples of three. The bone transform places
// vertices in the output space; its relationship to scene world space is external.
// In the GBIN caller the matrix has already been prepared for world placement.
// The zero-filled 32-byte name stores at most 31 basename bytes plus its terminator.
// All mesh handles acquired on successful resolution are released before returning;
// the source puppet is borrowed throughout.
// Failure/ownership ledger (there is no enclosing cleanup guard):
// - While new Obj constructs, C++ deletes its raw block on a constructor throw;
//   completed List is destroyed if the Obj body throws. List's lock guards unwind,
//   but its raw slots allocation is not guarded during List construction itself.
// - Once new Obj returns, collisionData is a raw owner until return transfers it
//   to CourseModel_LoadFromFile. A later throw does NOT call CourseObjectList_Dtor.
// - A fresh Triangle is raw/unpublished until append succeeds. Duplicate candidates
//   are deleted immediately; a throwing append leaves that candidate unowned.
// - WPuppet_CreateMeshByBoneName supplies a temporary counted mesh reference;
//   only the normal end-of-group path below decrements refs/deletes at zero.
//   A throw during traversal leaves that hold and the raw owner unreleased.
// Allocators must return usable storage or terminate/throw: a null owner,
// triangle, lookup array or link block is dereferenced, not a recoverable result.
extern "C" void* PuppetMesh_CreatePartSamplesAndBounds(Source* self, int storeNormals) {
    Obj* collisionData = new Obj;
    collisionData->min.x = kBoundsInitMin;
    collisionData->max.x = kBoundsInitMax;
    collisionData->min.y = kBoundsInitMin;
    collisionData->max.y = kBoundsInitMax;
    collisionData->min.z = kBoundsInitMin;
    collisionData->max.z = kBoundsInitMax;
    strncpy(collisionData->name, ((WPuppet*)self)->resourceName, sizeof(collisionData->name) - 1);
    Resource* resource = 0;
    unsigned entryIndex = 0;
    VectorWork vectors;
    // Phase 1: select entries whose descriptor name matches their bone name.
    while (entryIndex < self->count) {
        Entry* entry = &self->entries[entryIndex];
        if (entry) {
            Node* node = entry->getNode();
            if (CString_Compare(entry->other + kDescriptorNameByteOffset, node->getName()) == 0 &&
                WPuppet_CreateMeshByBoneName(self, &resource, node->getName())) {
                unsigned short meshCount = IPetMesh_GetGroupCount(resource);
                for (int meshGroupIndex = 0; meshGroupIndex < meshCount; ++meshGroupIndex) {
                    Mesh* mesh = IPetMesh_GetGroupByIndex(resource, (unsigned short)meshGroupIndex);
                    int partIndex = 0;
                    while (partIndex < mesh->count) {
                        for (int vertexIndex = 0; vertexIndex < mesh->parts[partIndex].count;
                             vertexIndex += kVerticesPerTriangle) {
                            Triangle* triangle = new Triangle;
                            // Phase 2: transform each local vertex and extend candidate bounds.
                            for (int cornerIndex = 0; cornerIndex < kVerticesPerTriangle; ++cornerIndex) {
                                Vec3* outputVertex = &triangle->v[cornerIndex];
                                memcpy(outputVertex, mesh->parts[partIndex].vertices + (vertexIndex + cornerIndex),
                                       kPositionBytes);
                                Matrix_TransformPoint(&vectors.transformed, outputVertex, node->getTransform());
                                *(WVector*)outputVertex = *(WVector*)(&vectors.transformed);
                                // Obj::min/max occupy float words 15..17 and 18..20;
                                // keep the word-address form for identical generated loads/stores.
                                for (int axis = 0; axis < 3; ++axis) {
                                    ((float*)collisionData)[kMinimumBoundFloatWord + axis] =
                                        lower(((float*)collisionData)[kMinimumBoundFloatWord + axis],
                                              ((float*)outputVertex)[axis]);
                                    ((float*)collisionData)[kMaximumBoundFloatWord + axis] =
                                        upper(((float*)collisionData)[kMaximumBoundFloatWord + axis],
                                              ((float*)outputVertex)[axis]);
                                }
                            }
                            // Phase 3: retain only the first triangle for each mesh-part ID.
                            Triangle* found = collisionData->list.first();
                            while (found) {
                                if (found->id == mesh->parts[partIndex].id)
                                    break;
                                found = collisionData->list.next();
                            }
                            if (found) {
                                if (triangle)
                                    __dl__FPv(triangle);
                            } else {
                                if (storeNormals) {
                                    // Zero-area triangles are still eligible as the FIRST sample
                                    // for an ID. Exact zero cross products normalize to (0,0,0),
                                    // not an up-vector and not rejection (WVector::Normalize,
                                    // wvector_affine_matrix_math.cpp). Near-zero/nonfinite inputs follow that
                                    // routine's retry policy; a unit normal is not guaranteed.
                                    // Bounds/count/ID retention are independent of normal length.
                                    __mi__FRC7WVectorRC7WVector(&vectors.a, &triangle->v[0], &triangle->v[1]);
                                    __mi__FRC7WVectorRC7WVector(&vectors.b, &triangle->v[2], &triangle->v[1]);
                                    Vector_CrossProduct(&vectors.normal, &vectors.b, &vectors.a);
                                    (*(WVector*)(&vectors.normal)).Normalize();
                                    *(WVector*)(&triangle->normal) = *(WVector*)(&vectors.normal);
                                }
                                triangle->id = mesh->parts[partIndex].id;
                                collisionData->list.append(triangle);
                            }
                            ++collisionData->candidateTriangleCount;
                        }
                        ++partIndex;
                    }
                }
                // Phase 4: release only the temporary mesh reference, not the source puppet.
                Resource* released = resource;
                if (released && --released->refs == 0) {
                    delete released;
                }
            }
        }
        ++entryIndex;
    }
    return collisionData;
}

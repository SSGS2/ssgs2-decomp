// Model / course bounds: WPuppet_ComputeMeshBounds collects transformed .pet mesh
// positions; it neither loads GBIN course objects nor performs a collision query.
// Mod lookup: entry transform = Entry::a + kEntryTransformOffset (0x78);
// position stream = SubMesh::verts, packed float XYZ, kPositionBytes (12) per vertex;
// optional inclusion box = clip, inclusive in world space after bone transform.
// Empty-result sentinels = kBoundsInitMin/Max (+/-100000) in the data section.
// Keep format offsets/strides and triangle counts together when changing a model
// reader. Changing the clip box filters vertices, so this cannot establish bounds
// of a geometrically clipped triangle or prove that course collision is absent.
// The other operation destroys a mutex-protected owned list/buffer; it has no
// resolution, aspect ratio, texture format, or frame rate setting.

#include "game/types.h"

// Target ABI: six consecutive floats, 24 bytes, min XYZ followed by max XYZ.
struct Box {
    Vec3f min;
    Vec3f max;
};
// Expand one axis: dest points at min[axis], and max[axis] is three floats later.
// Both pointers required and borrowed; changes only min[axis] and max[axis].
// The repeated source load and comparison order preserve NaN/alias behavior.
inline void Expand(float* dest, float* value) {
    float v = *value;
    *dest = *dest > v ? v : *dest;
    v = *value;
    dest[3] = dest[3] < v ? v : dest[3];
}

// Test inclusive bounds in the same space as the already-transformed vertex.
inline bool Contains(Box* clip, Vec3f* p) {
    bool inside = false;
    if (clip->min.x <= p->x && p->x <= clip->max.x && clip->min.y <= p->y && p->y <= clip->max.y &&
        clip->min.z <= p->z && p->z <= clip->max.z)
        inside = true;
    return inside;
}
struct RefObj;
// CodeWarrior places the vptr after refs: f1(1) is the final-reference delete hook.
struct RefObj {
    int refs;
    virtual void f1(int);
};

// These are live 32-bit target views, not serialized GBIN records. Pointers are
// borrowed for the acquired mesh reference's lifetime. SubMesh is 16 bytes:
// count +4, verts +8; MeshItem is 8 bytes; Entry is two pointers (8 bytes).
// Owner::entries/count are WPuppet::namedBones/namedBoneCount at 0x13c/0x140
// (classes/WPuppet.h). Entry::a is the resolved bone, Entry::b its bounds metadata:
// WPuppet_Share in wpuppet_model_bone_animation.cpp builds those two-pointer records.
// Bone name is +8; bounds name is +0x2c. Both must be NUL-terminated and nonnull.
struct SubMesh {
    int pad0;
    unsigned count;
    char* verts;
    int pad3;
};
struct MeshItem {
    unsigned count;
    SubMesh* subs;
};
struct Entry {
    char* a;
    char* b;
};
struct Owner {
    char pad[0x13c];
    Entry* entries;
    unsigned count;
};

// Entry::a supplies the name and transform; Entry::b supplies the comparison name.
// SubMesh::count counts positions, stored as packed 12-byte XYZ float triples.
enum {
    kEntryNameOffset = 8,
    kComparisonNameOffset = 0x2c,
    kEntryTransformOffset = 0x78,
    kVerticesPerTriangle = 3,
    kPositionBytes = 12
};

extern "C" {
extern const float kBoundsInitMin, kBoundsInitMax;
void Vec3_Zero(void*);
int strcmp(void*, void*);
int WPuppet_CreateMeshByBoneName(void*, RefObj**, void*);
unsigned short IPetMesh_GetGroupCount(RefObj*);
MeshItem* IPetMesh_GetGroupByIndex(RefObj*, unsigned short);
void* memcpy(void*, const void*, unsigned);
void* Matrix_TransformPoint(Vec3f*, Vec3f*, void*);
void __as__7WVectorFRC7WVector(Vec3f*, Vec3f*);
void OSLockMutex(void*);
void OSUnlockMutex(void*);
void appFreeUni__FPv(void*);
void __dl__FPv(void*);

// Bound transformed mesh vertices, optionally including only vertices inside clip.
// out and owner are required; clip is borrowed/optional and uses world coordinates.
// Entry +0x78 is the bone's affine world matrix, populated by
// WPuppet_UpdateBoneWorldMatrices -> WPuppetBone_ConcatMatrixAlongChain;
// call after updating bones to bound the current pose. This is vertex filtering,
// not triangle clipping: intersecting triangles with all vertices outside contribute nothing.
// Counts must describe complete triples. With no accepted vertices, +/-100000 sentinels remain.
// out must not overlap owner, mesh storage, or clip. There is no status result:
// failed mesh acquisition skips that entry, and the output may be partial/empty.
// Preserve name matching, per-entry transforms, inclusive tests, and reference
// release order. Distances use the supplied model/world transform's units; no
// conversion to display yards is performed. Empty sentinels limit practical
// coordinates to +/-100000 if a true finite AABB is required (not checked).
void WPuppet_ComputeMeshBounds(Box* out, Owner* owner, Box* clip) {
    // Seed an empty box; accepted positions expand each independent min/max axis.
    Vec3_Zero(out);
    Vec3_Zero(&out->max);
    out->min.x = kBoundsInitMin;
    out->max.x = kBoundsInitMax;
    out->min.y = kBoundsInitMin;
    out->max.y = kBoundsInitMax;
    out->min.z = kBoundsInitMin;
    out->max.z = kBoundsInitMax;
    RefObj* mesh = 0;
    MeshItem* group;
    Entry* entry;
    int unusedOffset;
    Vec3f* vertex;
    int groupIndex;
    unsigned submeshIndex;
    int corner;
    unsigned triangleStart;
    char* entryData;
    unsigned entryIndex;
    // Resolve only entries whose two stored names agree; acquire one mesh reference per entry.
    for (entryIndex = 0; entryIndex < owner->count; entryIndex++) {
        entry = &owner->entries[entryIndex];
        if (entry) {
            entryData = entry->a;
            if (!strcmp(entry->b + kComparisonNameOffset, kEntryNameOffset + entryData)) {
                if (WPuppet_CreateMeshByBoneName(owner, &mesh, entryData + kEntryNameOffset)) {
                    unsigned short groupCount = IPetMesh_GetGroupCount(mesh);
                    // Walk groups/submeshes/triples; transform each position before filtering it.
                    for (groupIndex = 0; groupCount > groupIndex; groupIndex++) {
                        group = IPetMesh_GetGroupByIndex(mesh, (unsigned short)groupIndex);
                        for (submeshIndex = 0; submeshIndex < group->count; submeshIndex++) {
                            for (triangleStart = 0; triangleStart < group->subs[submeshIndex].count;
                                 triangleStart += kVerticesPerTriangle) {
                                Vec3f triangle[kVerticesPerTriangle];
                                vertex = triangle;
                                for (corner = 0; corner < kVerticesPerTriangle; corner++, vertex++) {
                                    Vec3f transformed;
                                    memcpy(vertex,
                                           group->subs[submeshIndex].verts + (triangleStart + corner) * kPositionBytes,
                                           kPositionBytes);
                                    Matrix_TransformPoint(&transformed, vertex, entryData + kEntryTransformOffset);
                                    __as__7WVectorFRC7WVector(vertex, &transformed);
                                    if (!clip || (clip && Contains(clip, vertex))) {
                                        for (int axis = 0; axis < 3; axis++) {
                                            Expand(((float*)out) + axis, ((float*)vertex) + axis);
                                        }
                                    }
                                }
                            }
                        }
                    }
                    // Balance the mesh acquisition, deleting it only when the last reference drops.
                    RefObj* released = mesh;
                    if (released && --released->refs == 0) {
                        if (released)
                            released->f1(1);
                    }
                }
            }
        }
    }
}

// Circular intrusive list without a separate sentinel: a singleton links to itself.
// Holder owns every node and buf; a null head denotes an empty list.
// Target offsets: Node prev/next +0xc/+0x10; Holder head +8, buf +0x1c,
// mutex +0x20. The guard and the unlink loop acquire the SAME recursive mutex.
struct Node {
    char pad[0xc];
    Node* prev;
    Node* next;
};

struct InstanceLock_u8005FA9C {
    void* p;
    // Hold the instance mutex for this scope, including exception unwinding.
    InstanceLock_u8005FA9C(void* l) : p(l) {
        OSLockMutex(l);
    }
    // Release the mutex acquired by the guard.
    ~InstanceLock_u8005FA9C() {
        OSUnlockMutex(p);
    }
};
struct Holder {
    char pad0[8];
    Node* head;
    char pad1[0x10];
    void* buf;
    char lock[4];
};

#pragma push
#pragma use_lmw_stmw on
// Drain owned list nodes and buffer under the mutex; flag > 0 also deletes the holder.
// Null is accepted; returns the original address, dangling when flag is positive.
// flag <= 0 releases children only; flag > 0 also invokes operator delete.
// Preserve the nested recursive lock and unlock-before-node-free order.
Holder* LockedNodeList_dtor(Holder* self, short flag) {
    if (self) {
        {
            InstanceLock_u8005FA9C instanceLock(self->lock);
            Node* nextHead;
            void* mutex;
            Node* node;
            if (self->head) {
                mutex = self->lock;
                // Unlink under the nested mutex acquisition; turn the final self-link into null.
                while ((node = self->head) != 0) {
                    OSLockMutex(mutex);
                    Node* previous = node->prev;
                    node->next->prev = previous;
                    Node* next = node->next;
                    node->prev->next = next;
                    nextHead = node->next;
                    if (nextHead == node)
                        nextHead = 0;
                    OSUnlockMutex(mutex);
                    self->head = nextHead;
                    appFreeUni__FPv(node);
                }
            }
            // After the list is empty, release the separate buffer before dropping the outer lock.
            if (self->buf) {
                appFreeUni__FPv(self->buf);
                self->buf = 0;
            }
        }
        if (flag > 0)
            __dl__FPv(self);
    }
    return self;
}
}

#pragma pop

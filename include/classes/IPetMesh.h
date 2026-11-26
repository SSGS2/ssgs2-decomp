// IPetMesh: offset-preserving view; no constructors, bases, or virtual methods.
// Evidence: IPetMesh_Ctor (0x8021CDB4) initializes refcount/vtable/resource; IPetMesh_GetGroupByIndex bounds-checks index against groupCount and indexes groups.
#ifndef CLASSES_IPETMESH_H
#define CLASSES_IPETMESH_H

struct IPetMesh {
    unsigned int referenceCount; // 0x0
    void* vtbl;                  // 0x4
    unsigned int unk_8;          // 0x8
    void* resource;              // 0xC
    void* groups;                // 0x10
    unsigned int groupCount;     // 0x14
};

#define OFF(f, o) typedef char IPetMesh_off_##f[((int)&((IPetMesh*)0)->f == (o)) ? 1 : -1]
OFF(referenceCount, 0x0);
OFF(vtbl, 0x4);
OFF(unk_8, 0x8);
OFF(resource, 0xC);
OFF(groups, 0x10);
OFF(groupCount, 0x14);
#undef OFF
typedef char IPetMesh_size[(sizeof(IPetMesh) == 0x18) ? 1 : -1];
#define AsIPetMesh(p) ((IPetMesh*)(p))

#endif // CLASSES_IPETMESH_H

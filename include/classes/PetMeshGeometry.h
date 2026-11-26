// PetMeshGeometry: offset-preserving view; no constructors, bases, or virtual methods.
// Evidence: IPetMesh_BuildGeometry stores resultCount, allocates count*0x14, copies count*12 position bytes and count*8 UV bytes.
#ifndef CLASSES_PETMESHGEOMETRY_H
#define CLASSES_PETMESHGEOMETRY_H

struct PetMeshGeometry {
    char pad_000[0x4];
    unsigned int vertexCount; // 0x4
    void* positions;          // 0x8
    void* texcoords;          // 0xC
};

#define OFF(f, o) typedef char PetMeshGeometry_off_##f[((int)&((PetMeshGeometry*)0)->f == (o)) ? 1 : -1]
OFF(vertexCount, 0x4);
OFF(positions, 0x8);
OFF(texcoords, 0xC);
#undef OFF
typedef char PetMeshGeometry_size[(sizeof(PetMeshGeometry) == 0x10) ? 1 : -1];
#define AsPetMeshGeometry(p) ((PetMeshGeometry*)(p))

#endif // CLASSES_PETMESHGEOMETRY_H

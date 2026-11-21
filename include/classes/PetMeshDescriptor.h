// PetMeshDescriptor: offset-preserving view; no constructors, bases, or virtual methods.
// Evidence: IPetMesh_BuildGeometry requests resource channels 0/1 with +0x12/+0x10, allocates triangleCount*3 output vertices; IPetMesh_ExpandTriangleStrip indexes position data using vertexStride.
#ifndef CLASSES_PETMESHDESCRIPTOR_H
#define CLASSES_PETMESHDESCRIPTOR_H

struct PetMeshDescriptor {
    char pad_000[0xA];
    unsigned short vertexStride; // 0xA
    char pad_00C[0x2];
    unsigned short triangleCount; // 0xE
    unsigned short indexResource; // 0x10
    unsigned short vertexResource; // 0x12
};

#define OFF(f, o) typedef char PetMeshDescriptor_off_##f[((int)&((PetMeshDescriptor*)0)->f == (o)) ? 1 : -1]
OFF(vertexStride, 0xA);
OFF(triangleCount, 0xE);
OFF(indexResource, 0x10);
OFF(vertexResource, 0x12);
#undef OFF
typedef char PetMeshDescriptor_size[(sizeof(PetMeshDescriptor) == 0x14) ? 1 : -1];
#define AsPetMeshDescriptor(p) ((PetMeshDescriptor*)(p))

#endif // CLASSES_PETMESHDESCRIPTOR_H

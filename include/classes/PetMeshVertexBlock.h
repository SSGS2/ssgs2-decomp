// PetMeshVertexBlock: offset-preserving view; no constructors, bases, or virtual methods.
// Evidence: IPetMesh_ScaleVertexPositions adds dataOffset to the vertex-data base and divides byteSize by the format stride.
#ifndef CLASSES_PETMESHVERTEXBLOCK_H
#define CLASSES_PETMESHVERTEXBLOCK_H

struct PetMeshVertexBlock {
    void* format; // 0x0
    unsigned int byteSize; // 0x4
    unsigned int dataOffset; // 0x8
};

#define OFF(f, o) typedef char PetMeshVertexBlock_off_##f[((int)&((PetMeshVertexBlock*)0)->f == (o)) ? 1 : -1]
OFF(format, 0x0);
OFF(byteSize, 0x4);
OFF(dataOffset, 0x8);
#undef OFF
typedef char PetMeshVertexBlock_size[(sizeof(PetMeshVertexBlock) == 0xC) ? 1 : -1];
#define AsPetMeshVertexBlock(p) ((PetMeshVertexBlock*)(p))

#endif // CLASSES_PETMESHVERTEXBLOCK_H

// GtlPrimitive: partial layout for methods in the address-named translation unit.
// Field comments identify the function providing layout evidence.
#ifndef CLASSES_GTLPRIMITIVE_H
#define CLASSES_GTLPRIMITIVE_H

class GtlPrimitive {
public:
    char pad_000[0x4];
    void* vtable; // 0x4: gtl_IG3DXPrimitive_dtor
    void* vertexBuffer; // 0x8: gtl_IG3DXPrimitive_GetVertexData
    void* indexBuffer; // 0xC: gtl_IG3DXPrimitive_GetIndexData
    void* primitiveCount; // 0x10: gtl_IG3DXPrimitive_Draw
    void* vertexCount; // 0x14: gtl_IG3DXPrimitive_Draw
    char pad_018[0x4];
    unsigned int vertexDeclaration; // 0x1C: gtl_IG3DXPrimitive_dtor
    void* primitiveType; // 0x20: gtl_IG3DXPrimitive_Draw
};

#define OFF(f, o) typedef char GtlPrimitive_off_##f[((int)&((GtlPrimitive*)0)->f == (o)) ? 1 : -1]
OFF(vtable, 0x4);
OFF(vertexBuffer, 0x8);
OFF(indexBuffer, 0xC);
OFF(primitiveCount, 0x10);
OFF(vertexCount, 0x14);
OFF(vertexDeclaration, 0x1C);
OFF(primitiveType, 0x20);
#undef OFF

#endif

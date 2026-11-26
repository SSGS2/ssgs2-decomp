// gtl::IG3DXPrimitive: a vertex buffer and an index buffer with the primitive type and counts used to draw them.
#ifndef CLASSES_GTLPRIMITIVE_H
#define CLASSES_GTLPRIMITIVE_H

namespace gtl {

class IG3DXPrimitive {
public:
    IG3DXPrimitive* Init(int primitiveCount, int vertexCount, unsigned int creationFlags, void* declaration,
                         void* primitiveType);
    void* Destroy(volatile short freeFlag);
    int Draw(int unused);
    void GetVertexData(void* a, void* b);
    void GetIndexData(void* a, void* b);
    void FlushVertexBuffer();
    void FlushIndexBuffer();
    static int CreateFromFVF(void* argument0, void* argument1, void* argument2, void* vertexFormat, void* argument4,
                             void** outPrimitive);

    unsigned int referenceCount; // 0x0: set to 1 by Init
    void* vtable;                // 0x4
    void* vertexBuffer;          // 0x8
    void* indexBuffer;           // 0xC
    int primitiveCount;          // 0x10
    int vertexCount;             // 0x14
    unsigned int creationFlags;  // 0x18
    void* vertexDeclaration;     // 0x1C
    void* primitiveType;         // 0x20
};

} // namespace gtl

#define IG3DXPRIMITIVE_OFF(f, o) \
    typedef char IG3DXPrimitive_off_##f[((int)&((gtl::IG3DXPrimitive*)0)->f == (o)) ? 1 : -1]
IG3DXPRIMITIVE_OFF(vtable, 0x4);
IG3DXPRIMITIVE_OFF(vertexBuffer, 0x8);
IG3DXPRIMITIVE_OFF(indexBuffer, 0xC);
IG3DXPRIMITIVE_OFF(primitiveCount, 0x10);
IG3DXPRIMITIVE_OFF(vertexCount, 0x14);
IG3DXPRIMITIVE_OFF(creationFlags, 0x18);
IG3DXPRIMITIVE_OFF(vertexDeclaration, 0x1C);
IG3DXPRIMITIVE_OFF(primitiveType, 0x20);
#undef IG3DXPRIMITIVE_OFF

#endif

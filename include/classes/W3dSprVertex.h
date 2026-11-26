// W3dSpr's 0x38-byte vertex. W3dSpr::Init fills the texture coordinates from s_spriteQuadUvCornerSelectors;
// W3dSpr::SetColor writes color.
#ifndef CLASSES_W3DSPRVERTEX_H
#define CLASSES_W3DSPRVERTEX_H
struct W3dSprVertex {
    char pad_0[0x10];
    unsigned int color; // 0x10
    char pad_14[4];
    float textureU; // 0x18
    float textureV; // 0x1C
    char pad_20[0x18];
};
#define OFF(f, o) typedef char W3dSprVertex_off_##f[((int)&((W3dSprVertex*)0)->f == (o)) ? 1 : -1]
OFF(color, 0x10);
OFF(textureU, 0x18);
OFF(textureV, 0x1C);
#undef OFF
typedef char W3dSprVertex_size[(sizeof(W3dSprVertex) == 0x38) ? 1 : -1];
#define AsW3dSprVertex(p) ((W3dSprVertex*)(p))
#endif

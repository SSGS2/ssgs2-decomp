// CBpTex: one texture binding for the 2D sprites.
//
// A binding holds either a shared texture resource, loaded by path, or a
// direct texture resource. Sprites borrow a binding; the binding owns its
// texture source and releases it in its destructor.
#ifndef CLASSES_CBPTEX_H
#define CLASSES_CBPTEX_H

#include "game/types.h"

class CBpTex {
public:
    CBpTex();
    ~CBpTex();

    int vfunc_2();
    int IsLoaded();
    int Bind() const;
    int BindTexturePath(void* texturePath);

    void* vtbl;                // 0x00
    int sharedTextureResource; // 0x04  ISharedResource address as a word, or 0
    // Descriptor schema: seven native 32-bit words (0x1C bytes), 4-byte aligned.
    // +00 width pixels; +04 height pixels; +08 producer writes zero (role unknown);
    // +0C mip-level count; +10 platform-7 format ID; +14 producer writes tag 3
    // (role unknown); +18 opaque trailing word. The shared producer in gtl_texture_vertex_formats.cpp
    // writes only the FIRST SIX words; the copier forwards word 6 unchanged, even
    // if the producer left it uninitialized. Never interpret it as a valid dimension
    // or initialize it as part of a readability edit. Direct sources copy all seven
    // stored words instead of consulting a shared object. No endian/float conversion.
    int descriptorWord0;       // 0x08
    int descriptorWord1;       // 0x0C
    int descriptorWord2;       // 0x10
    int descriptorWord3;       // 0x14
    int descriptorWord4;       // 0x18
    int descriptorWord5;       // 0x1C
    int descriptorWord6;       // 0x20
    int directTextureResource; // 0x24  direct texture resource, with a packed shared refcount, or 0
};

#define CBPTEX_OFF(f, o) typedef char CBpTex_off_##f[((int)&((CBpTex*)0)->f == (o)) ? 1 : -1]
CBPTEX_OFF(sharedTextureResource, 0x4);
CBPTEX_OFF(descriptorWord0, 0x8);
CBPTEX_OFF(descriptorWord6, 0x20);
CBPTEX_OFF(directTextureResource, 0x24);
typedef char CBpTex_size[(sizeof(CBpTex) == 0x28) ? 1 : -1];
#undef CBPTEX_OFF

#endif // CLASSES_CBPTEX_H

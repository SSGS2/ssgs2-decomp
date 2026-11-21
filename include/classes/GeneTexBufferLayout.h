// GeneTexBufferLayout: layout view owned by club_trail_and_ring_effects.cpp.
// Evidence: __dt__13GeneTexBufferFv; CGeneTex3D_AllocVertexBuffer/3/4/5/6: vertex data has stride 0x18. No virtual declarations or runtime objects.
#ifndef CLASSES_GENETEXBUFFERLAYOUT_H
#define CLASSES_GENETEXBUFFERLAYOUT_H

class GeneTexBufferLayout {
public:
    char pad_0[0x4];
    void* vtable; // 0x4; __dt__13GeneTexBufferFv
    void* vertices; // 0x8; __dt__13GeneTexBufferFv
};

#define OFF(f, o) typedef char GeneTexBufferLayout_off_##f[((int)&((GeneTexBufferLayout*)0)->f == (o)) ? 1 : -1]
OFF(vtable, 0x4);
OFF(vertices, 0x8);
#undef OFF

#endif

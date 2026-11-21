// CSimpleSpd: layout-only view owned by player_status_hud.cpp (CSimpleSpd_Ctor, CSimpleSpd_dtor, CSimpleSpd_ApplyElementOpacity).
// No virtual methods, constructors, or generated RTTI.
#ifndef CLASSES_CSIMPLESPD_H
#define CLASSES_CSIMPLESPD_H

class CSimpleSpd {
public:
    void* resource; // 0x0: CSimpleSpd_Ctor creates shared file handle
    void* texture; // 0x4: CSimpleSpd_Ctor constructs 0x28-byte CBpTex
    void* layout; // 0x8: CSimpleSpd_Ctor constructs 0x30-byte ns0::CSpdCtrl
    unsigned char opacity; // 0xc: CSimpleSpd_ApplyElementOpacity places byte into low byte of color
    char pad_00D[0x3];
    void* vtbl; // 0x10: CSimpleSpd_Ctor installs __vt__10CSimpleSpd here
};

#define OFF(f, o) typedef char CSimpleSpd_off_##f[((int)&((CSimpleSpd*)0)->f == (o)) ? 1 : -1]
OFF(resource, 0x0);
OFF(texture, 0x4);
OFF(layout, 0x8);
OFF(opacity, 0xc);
OFF(vtbl, 0x10);
#undef OFF

#endif

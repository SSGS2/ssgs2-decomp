// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CGENETEX3D_H
#define CLASSES_CGENETEX3D_H

#include "game/types.h"

class CGeneTex3D {
public:
    void* vtbl;                         // 0x0: original vtable pointer
    void* unk_4;                        // 0x4: 3 uses in __dt__13GeneTexBufferFv, CSnow_Ctor, CTailEffect_dtor
    void* unk_8;                        // 0x8: 18 uses in CGeneTex3D_AllocVertexBuffer, CGeneTex3D_SetVertexColorARGB, __dt__13GeneTexBufferFv...
    void* unk_C;                        // 0xc: 11 uses in GroundGrid_Dtor, CFxSpray_Ctor, GroundGrid_Ctor...
    void* unk_10;                       // 0x10: 13 uses in GroundGrid_Dtor, ntRiver_Lava_dtor, CFxSpray_Ctor...
    void* unk_14;                       // 0x14: 6 uses in GroundGrid_Dtor, GroundGrid_Ctor, ntRiver_Lava_Ctor
    void* unk_18;                       // 0x18: 4 uses in GeneratedSurface_Destroy, ntRiver_Lava_Ctor, GeneratedSurface_Init
    float unk_1C;                       // 0x1c: 2 uses in ntRiver_Lava_Ctor, GeneratedSurface_Init
    void* unk_20;                       // 0x20: 10 uses in GeneratedSurface_Destroy, CSnow_dtor, ntRiver_Lava_Ctor... (CONFLICT sizes [1, 4])
    void* surfacePoints;                       // 0x24: 11 uses in CSnow_Ctor, CSnow_dtor, GeneTexPointGeometry_Dtor... (CONFLICT sizes [1, 4])
    void* relativePoints;                       // 0x28: 7 uses in GeneTexPointGeometry_Dtor, CGeneTex3D_Ctor, ntRiver_Lava_Ctor...
    int pointCount;                         // 0x2c: 6 uses in GeneratedSurface_Destroy, CGeneTex3D_Ctor, ntRiver_Lava_Ctor...
    void* causticLayers;                       // 0x30: 10 uses in GeneTexPointGeometry_Dtor, CGeneTex3D_Ctor, ntRiver_Lava_Ctor...
    unsigned int causticLayerCount;                // 0x34: 4 uses in CGeneTex3D_Ctor, GeneTexPointGeometry_Dtor, ntRiver_Lava_Ctor...
    float unk_38;                       // 0x38: 3 uses in ntRiver_Lava_Ctor, GeneratedSurface_Init, CSnow_Ctor
    unsigned char unk_3C;               // 0x3c: 2 uses in ntRiver_Lava_Ctor, CSnow_Ctor
    char pad_03D[0x3];                  // 0x3d
    void* unk_40;                       // 0x40: 3 uses in GroundGrid_Ctor, ntRiver_Lava_Ctor, CSnow_Ctor
    void* unk_44;                       // 0x44: 2 uses in GroundGrid_Ctor, ntRiver_Lava_Ctor
    void* unk_48;                       // 0x48: 5 uses in CTailEffect_dtor, GroundGrid_Ctor, ntRiver_Lava_Ctor...
    void* unk_4C;                       // 0x4c: 5 uses in CSnow_dtor, GroundGrid_Ctor, ntRiver_Lava_Ctor...
    int unk_50;                         // 0x50: 5 uses in CSnow_dtor, GroundGrid_Ctor, ntRiver_Lava_Ctor...
    int unk_54;                         // 0x54: 4 uses in CSnow_dtor, ntRiver_Lava_Ctor, CSnow_Ctor
    unsigned char unk_58;               // 0x58: 4 uses in ntRiver_Lava_Ctor, CSnow_Ctor, CSnow_dtor... (CONFLICT sizes [1, 4])
    char pad_059[0x3];                  // 0x59
    void* unk_5C;                       // 0x5c: 6 uses in CTailEffect_dtor, ntRiver_Lava_Ctor, ntRiver_Lava_dtor...
    void* unk_60;                       // 0x60: 7 uses in CSnow_dtor, ntRiver_Lava_dtor, ntRiver_Lava_Ctor...
    int unk_64;                         // 0x64: 4 uses in ntRiver_Lava_dtor, GroundGrid_Ctor, ntRiver_Lava_Ctor
    int unk_68;                         // 0x68: 1 use in ntRiver_Lava_Ctor
    float unk_6C;                       // 0x6c: 1 use in CGeneTex3D_Ctor
    int renderMode;                     // 0x70: CGeneTex3D_InitializePointGeometry input; CGeneTex3D_AddCausticLayer/801C8014 tests
    unsigned char causticVertexCount;               // 0x74: 1 use in CGeneTex3D_Ctor
    char pad_075[0x3];                  // 0x75
    void* causticBufferVtable;                       // 0x78: 3 uses in CFxSpray_Ctor, CGeneTex3D_Ctor, GeneTexPointGeometry_Dtor
    unsigned int causticVertices;                // 0x7c: 4 uses in GeneTexPointGeometry_Dtor, CFxSpray_Ctor, CGeneTex3D_Ctor
    unsigned int causticLayerBuffers;                // 0x80: 2 uses in CFxSpray_Ctor, CGeneTex3D_Ctor
    unsigned int causticLayerBufferCount;                // 0x84: 2 uses in CFxSpray_Ctor, CGeneTex3D_Ctor
    unsigned int causticLayerBufferCapacity;                // 0x88: 1 use in CGeneTex3D_Ctor
    char pad_08C[0x10];                 // 0x8c
    int unk_9C;                         // 0x9c: 1 use in GroundGrid_Ctor
    char pad_0A0[0x34];                 // 0xa0
    int unk_D4;                         // 0xd4: 1 use in GroundGrid_Ctor
    void* unk_D8;                       // 0xd8: 1 use in CGeneTex3D_Dtor
    void* unk_DC;                       // 0xdc: 2 uses in CGeneTex3D_Dtor
    char pad_0E0[0x2c];                 // 0xe0
    int unk_10C;                        // 0x10c: 1 use in GroundGrid_Ctor
    char pad_110[0x24];                 // 0x110
    void* unk_134;                      // 0x134: 4 uses in GroundGrid_Dtor, GroundGrid_Ctor
    char pad_138[0x8];                  // 0x138
    int surfaceFlags;                        // 0x140: 1 use in GroundGrid_Ctor
    unsigned char surfaceVertexCount;              // 0x144: 1 use in GroundGrid_Ctor
    char pad_145[0x3];                  // 0x145
    void* surfaceVertexBufferVtable;                      // 0x148: 2 uses in GroundGrid_Ctor, GroundGrid_Dtor
    void* surfaceVertices;                      // 0x14c: 4 uses in GroundGrid_Dtor, GroundGrid_Ctor
    char pad_150[0x10c];                // 0x150
    unsigned int unk_25C;               // 0x25c: 1 use in CFxSpray_Ctor
    void* unk_260;                      // 0x260: 1 use in CFxSpray_Ctor
    void* unk_264;                      // 0x264: 1 use in CFxSpray_Ctor
    char pad_268[0x30];                 // 0x268
    unsigned int unk_298;               // 0x298: 1 use in CFxSpray_Ctor
    void* unk_29C;                      // 0x29c: 1 use in CFxSpray_Ctor
    void* unk_2A0;                      // 0x2a0: 1 use in CFxSpray_Ctor
    unsigned int unk_2A4;               // 0x2a4: 1 use in CFxSpray_Ctor
    void* unk_2A8;                      // 0x2a8: 1 use in CFxSpray_Ctor
    void* unk_2AC;                      // 0x2ac: 1 use in CFxSpray_Ctor
    char pad_2B0[0x4];                  // 0x2b0
    void* unk_2B4;                      // 0x2b4: 1 use in CFxSpray_Ctor
    void* unk_2B8;                      // 0x2b8: 1 use in CFxSpray_Ctor
    unsigned int unk_2BC;               // 0x2bc: 1 use in CFxSpray_Ctor
    void* unk_2C0;                      // 0x2c0: 1 use in CFxSpray_Ctor
    void* unk_2C4;                      // 0x2c4: 1 use in CFxSpray_Ctor
    unsigned int unk_2C8;               // 0x2c8: 1 use in CFxSpray_Ctor
    void* unk_2CC;                      // 0x2cc: 1 use in CFxSpray_Ctor
    void* unk_2D0;                      // 0x2d0: 1 use in CFxSpray_Ctor
    unsigned int unk_2D4;               // 0x2d4: 1 use in CFxSpray_Ctor
    void* unk_2D8;                      // 0x2d8: 1 use in CFxSpray_Ctor
    void* unk_2DC;                      // 0x2dc: 1 use in CFxSpray_Ctor
    unsigned int unk_2E0;               // 0x2e0: 1 use in CFxSpray_Ctor
    void* unk_2E4;                      // 0x2e4: 1 use in CFxSpray_Ctor
    void* unk_2E8;                      // 0x2e8: 1 use in CFxSpray_Ctor
    unsigned int unk_2EC;               // 0x2ec: 1 use in CFxSpray_Ctor
    void* unk_2F0;                      // 0x2f0: 1 use in CFxSpray_Ctor
    void* unk_2F4;                      // 0x2f4: 1 use in CFxSpray_Ctor
    char pad_2F8[0xc];                  // 0x2f8
    unsigned int unk_304;               // 0x304: 1 use in CFxSpray_Ctor
    void* unk_308;                      // 0x308: 1 use in CFxSpray_Ctor
    void* unk_30C;                      // 0x30c: 1 use in CFxSpray_Ctor
    unsigned int unk_310;               // 0x310: 1 use in CFxSpray_Ctor
    void* unk_314;                      // 0x314: 1 use in CFxSpray_Ctor
    void* unk_318;                      // 0x318: 1 use in CFxSpray_Ctor
    unsigned int unk_31C;               // 0x31c: 1 use in CFxSpray_Ctor
    void* unk_320;                      // 0x320: 1 use in CFxSpray_Ctor
    void* unk_324;                      // 0x324: 1 use in CFxSpray_Ctor
    unsigned int unk_328;               // 0x328: 1 use in CFxSpray_Ctor
    void* unk_32C;                      // 0x32c: 1 use in CFxSpray_Ctor
    void* unk_330;                      // 0x330: 1 use in CFxSpray_Ctor
    char pad_334[0x8];                  // 0x334
    unsigned int unk_33C;               // 0x33c: 1 use in CFxSpray_Ctor
    void* unk_340;                      // 0x340: 1 use in CFxSpray_Ctor
    void* unk_344;                      // 0x344: 1 use in CFxSpray_Ctor
    unsigned int unk_348;               // 0x348: 1 use in CFxSpray_Ctor
    void* unk_34C;                      // 0x34c: 1 use in CFxSpray_Ctor
    void* unk_350;                      // 0x350: 1 use in CFxSpray_Ctor
    unsigned int unk_354;               // 0x354: 1 use in CFxSpray_Ctor
    void* unk_358;                      // 0x358: 1 use in CFxSpray_Ctor
    void* unk_35C;                      // 0x35c: 1 use in CFxSpray_Ctor
    char pad_360[0xc];                  // 0x360
    unsigned char sprayVertexCount;              // 0x36c: 2 uses in CFxSpray_Ctor
    char pad_36D[0x3];                  // 0x36d
    void* sprayVertexBufferVtable;                      // 0x370: 1 use in CFxSpray_Ctor
    unsigned int sprayVertices;               // 0x374: 1 use in CFxSpray_Ctor
    char pad_378[0x628];                // 0x378
    unsigned char effectVertexCount;              // 0x9a0: 1 use in CPvsFxBox_Init

    // offsets whose address is passed on (sub-objects or arrays, not placed): 0x4, 0x8, 0xc, 0x10, 0x14, 0x1c, 0x24, 0x28, 0x54, 0x58, 0x74, 0x80, 0x8c, 0xc4, 0xd4, 0xfc, 0x144, 0x150, 0x188, 0x18c, 0x190, 0x194, 0x198, 0x1a8
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define CGENETEX3D_OFF(f, o) typedef char CGeneTex3D_off_##f[((int)&((CGeneTex3D*)0)->f == (o)) ? 1 : -1]
CGENETEX3D_OFF(unk_4, 0x4);
CGENETEX3D_OFF(unk_8, 0x8);
CGENETEX3D_OFF(unk_C, 0xc);
CGENETEX3D_OFF(unk_10, 0x10);
CGENETEX3D_OFF(unk_14, 0x14);
CGENETEX3D_OFF(unk_18, 0x18);
CGENETEX3D_OFF(unk_1C, 0x1c);
CGENETEX3D_OFF(unk_20, 0x20);
CGENETEX3D_OFF(surfacePoints, 0x24);
CGENETEX3D_OFF(relativePoints, 0x28);
CGENETEX3D_OFF(pointCount, 0x2c);
CGENETEX3D_OFF(causticLayers, 0x30);
CGENETEX3D_OFF(causticLayerCount, 0x34);
CGENETEX3D_OFF(unk_38, 0x38);
CGENETEX3D_OFF(unk_3C, 0x3c);
CGENETEX3D_OFF(unk_40, 0x40);
CGENETEX3D_OFF(unk_44, 0x44);
CGENETEX3D_OFF(unk_48, 0x48);
CGENETEX3D_OFF(unk_4C, 0x4c);
CGENETEX3D_OFF(unk_50, 0x50);
CGENETEX3D_OFF(unk_54, 0x54);
CGENETEX3D_OFF(unk_58, 0x58);
CGENETEX3D_OFF(unk_5C, 0x5c);
CGENETEX3D_OFF(unk_60, 0x60);
CGENETEX3D_OFF(unk_64, 0x64);
CGENETEX3D_OFF(unk_68, 0x68);
CGENETEX3D_OFF(unk_6C, 0x6c);
CGENETEX3D_OFF(causticVertexCount, 0x74);
CGENETEX3D_OFF(causticBufferVtable, 0x78);
CGENETEX3D_OFF(causticVertices, 0x7c);
CGENETEX3D_OFF(causticLayerBuffers, 0x80);
CGENETEX3D_OFF(causticLayerBufferCount, 0x84);
CGENETEX3D_OFF(causticLayerBufferCapacity, 0x88);
CGENETEX3D_OFF(unk_9C, 0x9c);
CGENETEX3D_OFF(unk_D4, 0xd4);
CGENETEX3D_OFF(unk_D8, 0xd8);
CGENETEX3D_OFF(unk_DC, 0xdc);
CGENETEX3D_OFF(unk_10C, 0x10c);
CGENETEX3D_OFF(unk_134, 0x134);
CGENETEX3D_OFF(surfaceFlags, 0x140);
CGENETEX3D_OFF(surfaceVertexCount, 0x144);
CGENETEX3D_OFF(surfaceVertexBufferVtable, 0x148);
CGENETEX3D_OFF(surfaceVertices, 0x14c);
CGENETEX3D_OFF(unk_25C, 0x25c);
CGENETEX3D_OFF(unk_260, 0x260);
CGENETEX3D_OFF(unk_264, 0x264);
CGENETEX3D_OFF(unk_298, 0x298);
CGENETEX3D_OFF(unk_29C, 0x29c);
CGENETEX3D_OFF(unk_2A0, 0x2a0);
CGENETEX3D_OFF(unk_2A4, 0x2a4);
CGENETEX3D_OFF(unk_2A8, 0x2a8);
CGENETEX3D_OFF(unk_2AC, 0x2ac);
CGENETEX3D_OFF(unk_2B4, 0x2b4);
CGENETEX3D_OFF(unk_2B8, 0x2b8);
CGENETEX3D_OFF(unk_2BC, 0x2bc);
CGENETEX3D_OFF(unk_2C0, 0x2c0);
CGENETEX3D_OFF(unk_2C4, 0x2c4);
CGENETEX3D_OFF(unk_2C8, 0x2c8);
CGENETEX3D_OFF(unk_2CC, 0x2cc);
CGENETEX3D_OFF(unk_2D0, 0x2d0);
CGENETEX3D_OFF(unk_2D4, 0x2d4);
CGENETEX3D_OFF(unk_2D8, 0x2d8);
CGENETEX3D_OFF(unk_2DC, 0x2dc);
CGENETEX3D_OFF(unk_2E0, 0x2e0);
CGENETEX3D_OFF(unk_2E4, 0x2e4);
CGENETEX3D_OFF(unk_2E8, 0x2e8);
CGENETEX3D_OFF(unk_2EC, 0x2ec);
CGENETEX3D_OFF(unk_2F0, 0x2f0);
CGENETEX3D_OFF(unk_2F4, 0x2f4);
CGENETEX3D_OFF(unk_304, 0x304);
CGENETEX3D_OFF(unk_308, 0x308);
CGENETEX3D_OFF(unk_30C, 0x30c);
CGENETEX3D_OFF(unk_310, 0x310);
CGENETEX3D_OFF(unk_314, 0x314);
CGENETEX3D_OFF(unk_318, 0x318);
CGENETEX3D_OFF(unk_31C, 0x31c);
CGENETEX3D_OFF(unk_320, 0x320);
CGENETEX3D_OFF(unk_324, 0x324);
CGENETEX3D_OFF(unk_328, 0x328);
CGENETEX3D_OFF(unk_32C, 0x32c);
CGENETEX3D_OFF(unk_330, 0x330);
CGENETEX3D_OFF(unk_33C, 0x33c);
CGENETEX3D_OFF(unk_340, 0x340);
CGENETEX3D_OFF(unk_344, 0x344);
CGENETEX3D_OFF(unk_348, 0x348);
CGENETEX3D_OFF(unk_34C, 0x34c);
CGENETEX3D_OFF(unk_350, 0x350);
CGENETEX3D_OFF(unk_354, 0x354);
CGENETEX3D_OFF(unk_358, 0x358);
CGENETEX3D_OFF(unk_35C, 0x35c);
CGENETEX3D_OFF(sprayVertexCount, 0x36c);
CGENETEX3D_OFF(sprayVertexBufferVtable, 0x370);
CGENETEX3D_OFF(sprayVertices, 0x374);
CGENETEX3D_OFF(effectVertexCount, 0x9a0);
CGENETEX3D_OFF(renderMode, 0x70);
#undef CGENETEX3D_OFF

#endif  // CLASSES_CGENETEX3D_H

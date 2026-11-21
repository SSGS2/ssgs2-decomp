// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CRIVER_H
#define CLASSES_CRIVER_H

#include "game/types.h"

class CRiver {
public:
    void* vtbl;                         // 0x0: original vtable pointer
    char pad_004[0x4];                  // 0x4
    void* nodes;                        // 0x8: 1 use in CRiver_DrawTextureLayers
    unsigned int nodeCount;                 // 0xc: 2 uses in CRiver_DrawTextureLayers
    char pad_010[0x4];                  // 0x10
    float layerVOffset;                       // 0x14: 3 uses in CRiver_ScrollTextureLayers, CRiver_DrawTextureLayers
    float layerVScrollSpeed;                       // 0x18: 1 use in CRiver_ScrollTextureLayers
    void* layerBitmap;                       // 0x1c: 1 use in CRiver_DrawTextureLayers
    float layerVScale; // 0x20: CRiver_LoadTextureLayers initializes each layer to 1
    float layerUExtent;                       // 0x24: 1 use in CRiver_BuildTerrainStrips
    float layerHeightOffset; // 0x28: CRiver_LoadTextureLayers selects by texture/course variant
    unsigned char layerEnabled;               // 0x2c: 3 uses in CRiver_BuildTerrainStrips, CRiver_ScrollTextureLayers, CRiver_DrawTextureLayers
    char pad_02D[0x3];                  // 0x2d
    float secondLayerVOffset;                       // 0x30: 2 uses in CRiver_ScrollTextureLayers
    float secondLayerVScrollSpeed;                       // 0x34: 1 use in CRiver_ScrollTextureLayers
    char pad_038[0x10];                 // 0x38
    unsigned char secondLayerEnabled;               // 0x48: 1 use in CRiver_ScrollTextureLayers
    char pad_049[0x1b];                 // 0x49
    void* vertexBuffer;                       // 0x64: 3 uses in CRiver_DrawTextureLayers

    // offsets whose address is passed on (sub-objects or arrays, not placed): 0x4, 0x1c, 0x58
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define CRIVER_OFF(f, o) typedef char CRiver_off_##f[((int)&((CRiver*)0)->f == (o)) ? 1 : -1]
CRIVER_OFF(nodes, 0x8);
CRIVER_OFF(nodeCount, 0xc);
CRIVER_OFF(layerVOffset, 0x14);
CRIVER_OFF(layerVScrollSpeed, 0x18);
CRIVER_OFF(layerBitmap, 0x1c);
CRIVER_OFF(layerVScale, 0x20);
CRIVER_OFF(layerHeightOffset, 0x28);
CRIVER_OFF(layerUExtent, 0x24);
CRIVER_OFF(layerEnabled, 0x2c);
CRIVER_OFF(secondLayerVOffset, 0x30);
CRIVER_OFF(secondLayerVScrollSpeed, 0x34);
CRIVER_OFF(secondLayerEnabled, 0x48);
CRIVER_OFF(vertexBuffer, 0x64);
#undef CRIVER_OFF

#endif  // CLASSES_CRIVER_H

// CRiver: the surface of a river on the course.
//
// The river keeps a centreline of path nodes, two scrolling texture layers, an
// auxiliary point list, and an embedded generated-texture handle. Constructing
// it only clears the owned storage; the terrain strips and texture layers are
// filled in later from the course data.
#ifndef CLASSES_CRIVER_H
#define CLASSES_CRIVER_H

#include "game/types.h"

class CRiver {
public:
    CRiver();
    ~CRiver();

    static void BuildTerrainStrips(void* self_, void* centerlineArg, char* name);
    void LoadTextureLayers(char* name);
    void ScrollTextureLayers(float deltaSeconds);
    void DrawTextureLayers();

    void* vtbl;                       // 0x0: original vtable pointer
    float halfWidth;                  // 0x4
    void* nodes;                      // 0x8: path nodes
    unsigned int nodeCount;           // 0xc
    unsigned int nodeCapacity;        // 0x10
    float layerVOffset;               // 0x14
    float layerVScrollSpeed;          // 0x18
    void* layerBitmap;                // 0x1c
    float layerVScale;                // 0x20: the texture loader initializes each layer to 1
    float layerUExtent;               // 0x24
    float layerHeightOffset;          // 0x28: the texture loader selects by texture/course variant
    unsigned char layerEnabled;       // 0x2c
    char pad_02D[0x3];                // 0x2d
    float secondLayerVOffset;         // 0x30
    float secondLayerVScrollSpeed;    // 0x34
    void* secondLayerBitmap;          // 0x38
    float secondLayerVScale;          // 0x3c
    float secondLayerUExtent;         // 0x40
    float secondLayerHeightOffset;    // 0x44
    unsigned char secondLayerEnabled; // 0x48
    char pad_049[0x3];                // 0x49
    void* auxiliaryData;              // 0x4c: auxiliary point list
    int auxiliarySize;                // 0x50
    int auxiliaryCapacity;            // 0x54
    unsigned char materialFlags;      // 0x58
    char pad_059[0x3];                // 0x59
    void* materialVtable;             // 0x5c: the generated-texture handle
    void* materialData;               // 0x60
    void* vertexBuffer;               // 0x64
    void* secondVertexBuffer;         // 0x68
};

#define CRIVER_OFF(f, o) typedef char CRiver_off_##f[((int)&((CRiver*)0)->f == (o)) ? 1 : -1]
CRIVER_OFF(halfWidth, 0x4);
CRIVER_OFF(nodes, 0x8);
CRIVER_OFF(nodeCount, 0xc);
CRIVER_OFF(nodeCapacity, 0x10);
CRIVER_OFF(layerVOffset, 0x14);
CRIVER_OFF(layerVScrollSpeed, 0x18);
CRIVER_OFF(layerBitmap, 0x1c);
CRIVER_OFF(layerVScale, 0x20);
CRIVER_OFF(layerUExtent, 0x24);
CRIVER_OFF(layerHeightOffset, 0x28);
CRIVER_OFF(layerEnabled, 0x2c);
CRIVER_OFF(secondLayerVOffset, 0x30);
CRIVER_OFF(secondLayerVScrollSpeed, 0x34);
CRIVER_OFF(secondLayerBitmap, 0x38);
CRIVER_OFF(secondLayerVScale, 0x3c);
CRIVER_OFF(secondLayerUExtent, 0x40);
CRIVER_OFF(secondLayerHeightOffset, 0x44);
CRIVER_OFF(secondLayerEnabled, 0x48);
CRIVER_OFF(auxiliaryData, 0x4c);
CRIVER_OFF(auxiliarySize, 0x50);
CRIVER_OFF(auxiliaryCapacity, 0x54);
CRIVER_OFF(materialFlags, 0x58);
CRIVER_OFF(materialVtable, 0x5c);
CRIVER_OFF(materialData, 0x60);
CRIVER_OFF(vertexBuffer, 0x64);
CRIVER_OFF(secondVertexBuffer, 0x68);
#undef CRIVER_OFF
typedef char CRiver_size[(sizeof(CRiver) == 0x6C) ? 1 : -1];

#endif // CLASSES_CRIVER_H

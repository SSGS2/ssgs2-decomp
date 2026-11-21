// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_DEBUGTEXT800CA990_H
#define CLASSES_DEBUGTEXT800CA990_H

#include "game/types.h"

class DebugTextView {
public:
    char pad_000[4]; // 0x0: unmodelled bytes
    int glyphMapAddress;                // 0x4: DebugText_DrawFormattedAtWorldPosition original object access
    int atlasColumns;                   // 0x8: DebugText_DrawFormattedAtWorldPosition original object access
    float glyphUSize;                   // 0xc: DebugText_DrawFormattedAtWorldPosition original object access
    float glyphVSize;                   // 0x10: DebugText_DrawFormattedAtWorldPosition original object access
    int glyphWidth;                     // 0x14: DebugText_DrawFormattedAtWorldPosition original object access
    int glyphHeight;                    // 0x18: DebugText_DrawFormattedAtWorldPosition original object access
    float offsetX;                      // 0x1c: DebugText_DrawFormattedAtWorldPosition original object access
    float offsetY;                      // 0x20: DebugText_DrawFormattedAtWorldPosition original object access
    float worldPosition[3];             // 0x24: DebugText_SetWorldPosition copies; DebugText_DrawFormattedAtWorldPosition projects
    float vertex0X;                       // 0x30: DebugText_DrawFormattedAtWorldPosition original object access
    float vertex0Y;                       // 0x34: DebugText_DrawFormattedAtWorldPosition original object access
    float vertex0Z;                       // 0x38: DebugText_DrawFormattedAtWorldPosition original object access
    float vertex0Rhw;                       // 0x3c: DebugText_DrawFormattedAtWorldPosition original object access
    char pad_040[0x8];                  // 0x40
    float vertex0U;                       // 0x48: DebugText_DrawFormattedAtWorldPosition original object access
    float vertex0V;                       // 0x4c: DebugText_DrawFormattedAtWorldPosition original object access
    char pad_050[0x18];                 // 0x50
    float vertex1X;                       // 0x68: DebugText_DrawFormattedAtWorldPosition original object access
    float vertex1Y;                       // 0x6c: DebugText_DrawFormattedAtWorldPosition original object access
    float vertex1Z;                       // 0x70: DebugText_DrawFormattedAtWorldPosition original object access
    float vertex1Rhw;                       // 0x74: DebugText_DrawFormattedAtWorldPosition original object access
    char pad_078[0x8];                  // 0x78
    float vertex1U;                       // 0x80: DebugText_DrawFormattedAtWorldPosition original object access
    float vertex1V;                       // 0x84: DebugText_DrawFormattedAtWorldPosition original object access
    char pad_088[0x18];                 // 0x88
    float vertex2X;                       // 0xa0: DebugText_DrawFormattedAtWorldPosition original object access
    float vertex2Y;                       // 0xa4: DebugText_DrawFormattedAtWorldPosition original object access
    float vertex2Z;                       // 0xa8: DebugText_DrawFormattedAtWorldPosition original object access
    float vertex2Rhw;                       // 0xac: DebugText_DrawFormattedAtWorldPosition original object access
    char pad_0B0[0x8];                  // 0xb0
    float vertex2U;                       // 0xb8: DebugText_DrawFormattedAtWorldPosition original object access
    float vertex2V;                       // 0xbc: DebugText_DrawFormattedAtWorldPosition original object access
    char pad_0C0[0x18];                 // 0xc0
    float vertex3X;                       // 0xd8: DebugText_DrawFormattedAtWorldPosition original object access
    float vertex3Y;                       // 0xdc: DebugText_DrawFormattedAtWorldPosition original object access
    float vertex3Z;                       // 0xe0: DebugText_DrawFormattedAtWorldPosition original object access
    float vertex3Rhw;                       // 0xe4: DebugText_DrawFormattedAtWorldPosition original object access
    char pad_0E8[0x8];                  // 0xe8
    float vertex3U;                       // 0xf0: DebugText_DrawFormattedAtWorldPosition original object access
    float vertex3V;                       // 0xf4: DebugText_DrawFormattedAtWorldPosition original object access
    char pad_0F8[0x18];                 // 0xf8
    void* vertex0;                      // 0x110: DebugText_SetColor original object access
    void* vertex1;                      // 0x114: DebugText_SetColor original object access
    void* vertex2;                      // 0x118: DebugText_SetColor original object access
    void* vertex3;                      // 0x11c: DebugText_SetColor original object access
    unsigned int vertexListTerminator;               // 0x120: DebugText_Init original object access
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define DEBUGTEXT800CA990_OFF(f, o) typedef char DebugText800CA990_off_##f[((int)&((DebugTextView*)0)->f == (o)) ? 1 : -1]
DEBUGTEXT800CA990_OFF(glyphMapAddress, 0x4);
DEBUGTEXT800CA990_OFF(atlasColumns, 0x8);
DEBUGTEXT800CA990_OFF(glyphUSize, 0xc);
DEBUGTEXT800CA990_OFF(glyphVSize, 0x10);
DEBUGTEXT800CA990_OFF(glyphWidth, 0x14);
DEBUGTEXT800CA990_OFF(glyphHeight, 0x18);
DEBUGTEXT800CA990_OFF(offsetX, 0x1c);
DEBUGTEXT800CA990_OFF(offsetY, 0x20);
DEBUGTEXT800CA990_OFF(worldPosition, 0x24);
DEBUGTEXT800CA990_OFF(vertex0X, 0x30);
DEBUGTEXT800CA990_OFF(vertex0Y, 0x34);
DEBUGTEXT800CA990_OFF(vertex0Z, 0x38);
DEBUGTEXT800CA990_OFF(vertex0Rhw, 0x3c);
DEBUGTEXT800CA990_OFF(vertex0U, 0x48);
DEBUGTEXT800CA990_OFF(vertex0V, 0x4c);
DEBUGTEXT800CA990_OFF(vertex1X, 0x68);
DEBUGTEXT800CA990_OFF(vertex1Y, 0x6c);
DEBUGTEXT800CA990_OFF(vertex1Z, 0x70);
DEBUGTEXT800CA990_OFF(vertex1Rhw, 0x74);
DEBUGTEXT800CA990_OFF(vertex1U, 0x80);
DEBUGTEXT800CA990_OFF(vertex1V, 0x84);
DEBUGTEXT800CA990_OFF(vertex2X, 0xa0);
DEBUGTEXT800CA990_OFF(vertex2Y, 0xa4);
DEBUGTEXT800CA990_OFF(vertex2Z, 0xa8);
DEBUGTEXT800CA990_OFF(vertex2Rhw, 0xac);
DEBUGTEXT800CA990_OFF(vertex2U, 0xb8);
DEBUGTEXT800CA990_OFF(vertex2V, 0xbc);
DEBUGTEXT800CA990_OFF(vertex3X, 0xd8);
DEBUGTEXT800CA990_OFF(vertex3Y, 0xdc);
DEBUGTEXT800CA990_OFF(vertex3Z, 0xe0);
DEBUGTEXT800CA990_OFF(vertex3Rhw, 0xe4);
DEBUGTEXT800CA990_OFF(vertex3U, 0xf0);
DEBUGTEXT800CA990_OFF(vertex3V, 0xf4);
DEBUGTEXT800CA990_OFF(vertex0, 0x110);
DEBUGTEXT800CA990_OFF(vertex1, 0x114);
DEBUGTEXT800CA990_OFF(vertex2, 0x118);
DEBUGTEXT800CA990_OFF(vertex3, 0x11c);
DEBUGTEXT800CA990_OFF(vertexListTerminator, 0x120);
#undef DEBUGTEXT800CA990_OFF

#endif  // CLASSES_DEBUGTEXT800CA990_H

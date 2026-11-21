// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_WGENERIC3DR_H
#define CLASSES_WGENERIC3DR_H

#include "game/types.h"

class WGeneric3DR {
public:
    char pad_000[0x4];                  // 0x0: uninterpreted prefix
    char pad_004[0xc];                  // 0x4
    int textureStageCount;                         // 0x10: WGeneric3DR_DrawImmediatePrimitive (0x800879D0)
    void* viewportWidth;                       // 0x14: WGeneric3DR_vfunc_13 (0x80088DC8)
    int viewportHeight;                         // 0x18: WGeneric3DR_vfunc_14 (0x80088DC0)
    char pad_01C[0x4828];               // 0x1c
    int submissionState; // 0x4844: 0 idle, 1 pending (suppresses draws), 2 active
    char pad_4848[0x8];                 // 0x4848
    int unk_4850;                       // 0x4850: WGeneric3DR_GetProperty (0x80088CA8)
    char pad_4854[0xc];                 // 0x4854
    void* overrideTexture;              // 0x4860: WGeneric3DR_DrawImmediatePrimitive (0x800879D0)
    char pad_4864[0x4];                 // 0x4864
    int initialized;                       // 0x4868: WGeneric3DR_DispatchCommand (0x80088A38)
    char pad_486C[0xc];                 // 0x486c
    int windowFormatFlags;                       // 0x4878: WGeneric3DR_GetProperty (0x80088CA8)
    char pad_487C[0xc];                 // 0x487c
    int unk_4888;                       // 0x4888: WGeneric3DR_GetProperty (0x80088CA8)
    char pad_488C[0x38];                // 0x488c
    int defaultTextureWidth;                       // 0x48c4: WGeneric3DR_GetProperty (0x80088CA8)
    int defaultTextureHeight;                       // 0x48c8: WGeneric3DR_GetProperty (0x80088CA8)
    char pad_48CC[0x34];                // 0x48cc
    int unk_4900;                       // 0x4900: WGeneric3DR_GetProperty (0x80088CA8)
    int unk_4904;                       // 0x4904: WGeneric3DR_GetProperty (0x80088CA8)
    char pad_4908[0xc];                 // 0x4908
    int unk_4914;                       // 0x4914: WGeneric3DR_GetProperty (0x80088CA8)
    int unk_4918;                       // 0x4918: WGeneric3DR_GetProperty (0x80088CA8)
    float unk_491C;                     // 0x491c: WGeneric3DR_GetProperty (0x80088CA8)
    char pad_4920[0x10];                // 0x4920
    int unk_4930;                       // 0x4930: WGeneric3DR_GetProperty (0x80088CA8)
    char pad_4934[0x4];                 // 0x4934
    int unk_4938;                       // 0x4938: WGeneric3DR_GetProperty (0x80088CA8)
    char pad_493C[0x4];                 // 0x493c
    unsigned int boundTextureIndices;              // 0x4940: WGeneric3DR_DrawImmediatePrimitive (0x800879D0)
    unsigned int cachedRenderFlags;     // 0x4944: WGeneric3DR_ApplyRenderFlags (0x8008869C)
    int cachedBlendMode;                       // 0x4948: WGeneric3DR_SetBlendMode (0x8008884C)
    unsigned int drawMode;              // 0x494c: WGeneric3DR_DrawImmediatePrimitive (0x800879D0)
    unsigned int cachedDrawSettings;              // 0x4950: WGeneric3DR_DrawImmediatePrimitive (0x800879D0)
    char pad_4954[0xc];                 // 0x4954
    void* releasableSharedResource;                     // 0x4960: WGeneric3DR_DispatchCommand (0x80088A38)
    char pad_4964[0x8];                 // 0x4964
    unsigned int stage0ColorOperation;              // 0x496c: WGeneric3DR_DrawImmediatePrimitive (0x800879D0)
    char pad_4970[0x8];                 // 0x4970
    unsigned int stage0AlphaOperation;              // 0x4978: WGeneric3DR_DrawImmediatePrimitive (0x800879D0)
    char pad_497C[0x8];                 // 0x497c
    unsigned int stage1ColorOperation;              // 0x4984: WGeneric3DR_DrawImmediatePrimitive (0x800879D0)
    char pad_4988[0x14];                // 0x4988
    unsigned int cachedVertexFormat;    // 0x499c: WGeneric3DR_DrawImmediatePrimitive (0x800879D0)
    unsigned int vertexStrideBytes;            // 0x49a0: WGeneric3DR_DrawImmediatePrimitive (0x800879D0)
    char pad_49A4[0x4];                 // 0x49a4
    int textureResource;                       // 0x49a8: WGeneric3DR_DispatchCommand (0x80088A38)
    char pad_49AC[0x16008];             // 0x49ac
    int presentationTarget;                      // 0x1a9b4: WGeneric3DR_DispatchCommand (0x80088A38)
    char pad_1A9B8[0x20];               // 0x1a9b8
    unsigned char stage0ColorMultiplyEnabled;            // 0x1a9d8: WGeneric3DR_DrawImmediatePrimitive (0x800879D0)
    char pad_1A9D9[0x5f];               // 0x1a9d9
    int unk_1AA38;                      // 0x1aa38: WGeneric3DR_DispatchCommand (0x80088A38)
    unsigned char unk_1AA3C;            // 0x1aa3c: WGeneric3DR_DispatchCommand (0x80088A38)
    char pad_1AA3D[0x3];                // 0x1aa3d
    int unk_1AA40;                      // 0x1aa40: WGeneric3DR_DispatchCommand (0x80088A38)
    int unk_1AA44;                      // 0x1aa44: WGeneric3DR_DispatchCommand (0x80088A38)
    int unk_1AA48;                      // 0x1aa48: WGeneric3DR_DispatchCommand (0x80088A38)
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define WGENERIC3DR_OFF(f, o) typedef char WGeneric3DR_off_##f[((int)&((WGeneric3DR*)0)->f == (o)) ? 1 : -1]
WGENERIC3DR_OFF(textureStageCount, 0x10);
WGENERIC3DR_OFF(viewportWidth, 0x14);
WGENERIC3DR_OFF(viewportHeight, 0x18);
WGENERIC3DR_OFF(submissionState, 0x4844);
WGENERIC3DR_OFF(unk_4850, 0x4850);
WGENERIC3DR_OFF(overrideTexture, 0x4860);
WGENERIC3DR_OFF(initialized, 0x4868);
WGENERIC3DR_OFF(windowFormatFlags, 0x4878);
WGENERIC3DR_OFF(unk_4888, 0x4888);
WGENERIC3DR_OFF(defaultTextureWidth, 0x48c4);
WGENERIC3DR_OFF(defaultTextureHeight, 0x48c8);
WGENERIC3DR_OFF(unk_4900, 0x4900);
WGENERIC3DR_OFF(unk_4904, 0x4904);
WGENERIC3DR_OFF(unk_4914, 0x4914);
WGENERIC3DR_OFF(unk_4918, 0x4918);
WGENERIC3DR_OFF(unk_491C, 0x491c);
WGENERIC3DR_OFF(unk_4930, 0x4930);
WGENERIC3DR_OFF(unk_4938, 0x4938);
WGENERIC3DR_OFF(boundTextureIndices, 0x4940);
WGENERIC3DR_OFF(cachedRenderFlags, 0x4944);
WGENERIC3DR_OFF(cachedBlendMode, 0x4948);
WGENERIC3DR_OFF(drawMode, 0x494c);
WGENERIC3DR_OFF(cachedDrawSettings, 0x4950);
WGENERIC3DR_OFF(releasableSharedResource, 0x4960);
WGENERIC3DR_OFF(stage0ColorOperation, 0x496c);
WGENERIC3DR_OFF(stage0AlphaOperation, 0x4978);
WGENERIC3DR_OFF(stage1ColorOperation, 0x4984);
WGENERIC3DR_OFF(cachedVertexFormat, 0x499c);
WGENERIC3DR_OFF(vertexStrideBytes, 0x49a0);
WGENERIC3DR_OFF(textureResource, 0x49a8);
WGENERIC3DR_OFF(presentationTarget, 0x1a9b4);
WGENERIC3DR_OFF(stage0ColorMultiplyEnabled, 0x1a9d8);
WGENERIC3DR_OFF(unk_1AA38, 0x1aa38);
WGENERIC3DR_OFF(unk_1AA3C, 0x1aa3c);
WGENERIC3DR_OFF(unk_1AA40, 0x1aa40);
WGENERIC3DR_OFF(unk_1AA44, 0x1aa44);
WGENERIC3DR_OFF(unk_1AA48, 0x1aa48);
#undef WGENERIC3DR_OFF

#endif  // CLASSES_WGENERIC3DR_H

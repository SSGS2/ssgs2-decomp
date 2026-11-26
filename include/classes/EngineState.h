// Render-state cache reached through g_engineState by methods in gx_renderer_state_and_efb_copy.cpp.
#ifndef CLASSES_ENGINESTATE_H
#define CLASSES_ENGINESTATE_H

struct EngineState {
    char pad_0[0x60];
    int offscreenRenderingEnabled; // 0x60: gx_renderer_state_and_efb_copy.cpp render-state access
    char pad_64[0x20];
    int palettes[8];    // 0x84: Engine_SetTexturePalette retains palette objects and marks palette bindings dirty
    void* vertexBuffer; // 0xa4: gx_renderer_state_and_efb_copy.cpp render-state access
    char pad_a8[0xc];
    void* indexBuffer; // 0xb4: gx_renderer_state_and_efb_copy.cpp render-state access
    char pad_b8[0x4];
    int vertexFormat;        // 0xbc: gx_renderer_state_and_efb_copy.cpp render-state access
    int displayRenderTarget; // 0xc0: gx_renderer_state_and_efb_copy.cpp render-state access
    char pad_c4[0x4];
    void* offscreenRenderTarget; // 0xc8: gx_renderer_state_and_efb_copy.cpp render-state access
    char pad_cc[0x4];
    int vertexDeclaration; // 0xd0: gx_renderer_state_and_efb_copy.cpp render-state access
    int type6Resource;     // 0xd4: gx_renderer_state_and_efb_copy.cpp render-state access
    int type7Resource;     // 0xd8: gx_renderer_state_and_efb_copy.cpp render-state access
    int renderStates[42];  // 0xdc: Engine_GetRenderState and Engine_SetCachedRenderState index this cache
    int textureStageStates
        [8][25]; // 0x184: Engine_SetTextureStageState / Engine_GetTextureStageState use stage * 100 + state * 4
    char pad_4a4[0x120];
    unsigned int immediateStateUpdates; // 0x5c4: gx_renderer_state_and_efb_copy.cpp render-state access
    char pad_5c8[0x20];
    unsigned int copySourceX;      // 0x5e8: Engine_CopyFramebufferToTexture default bounds passed to GXSetTexCopySrc
    unsigned int copySourceY;      // 0x5ec: same call, y
    unsigned int copySourceWidth;  // 0x5f0: same call, width
    unsigned int copySourceHeight; // 0x5f4: same call, height
    char pad_5f8[8];
    char transforms[43][0x40]; // 0x600: SetTransform caches 4x4 matrices; states 0x0B..0x2A also get a transpose
    char pad_10c0[0x40c];
    unsigned int framebufferCopyParameter0; // 0x14cc: gx_renderer_state_and_efb_copy.cpp render-state access
    unsigned int framebufferCopyParameter1; // 0x14d0: gx_renderer_state_and_efb_copy.cpp render-state access
    int fogStateDirty;                      // 0x14d4: gx_renderer_state_and_efb_copy.cpp render-state access
    int lightingStateDirty;                 // 0x14d8: gx_renderer_state_and_efb_copy.cpp render-state access
    int depthStateDirty;                    // 0x14dc: gx_renderer_state_and_efb_copy.cpp render-state access
    int blendStateDirty;                    // 0x14e0: gx_renderer_state_and_efb_copy.cpp render-state access
    int alphaCompareStateDirty;             // 0x14e4: gx_renderer_state_and_efb_copy.cpp render-state access
    int colorWriteStateDirty;               // 0x14e8: gx_renderer_state_and_efb_copy.cpp render-state access
    int alphaWriteStateDirty;               // 0x14ec: gx_renderer_state_and_efb_copy.cpp render-state access
    int lightingChannelStateDirty;          // 0x14f0: gx_renderer_state_and_efb_copy.cpp render-state access
    int depthScaleOffsetStateDirty;         // 0x14f4: gx_renderer_state_and_efb_copy.cpp render-state access
    int pointSizeStateDirty;                // 0x14f8: gx_renderer_state_and_efb_copy.cpp render-state access
    int textureStageComponentDirty
        [8][2]; // 0x14fc: Engine_SetTextureStageState distinguishes state groups 0/1/2/14 and 3/4/5/15
    char pad_153c[0x8];
    int drawStateDirty;    // 0x1544: gx_renderer_state_and_efb_copy.cpp render-state access
    int textureStateDirty; // 0x1548: gx_renderer_state_and_efb_copy.cpp render-state access
    char pad_154c[0x4];
    int lightStateDirty[8]; // 0x1550: SetTransform state 0 invalidation
    char pad_1570[0x4];
    int textureStageDirty[8]; // 0x1574: Engine_SetTextureStageState sets one dirty word per stage
    int paletteDirty[8];      // 0x1594: Engine_SetTexturePalette invalidates a selected palette binding
    int transformDirty[43];   // 0x15b4: SetTransform marks the selected matrix dirty
    char pad_1660[0x4];
    int renderTargetRequiresCopy;    // 0x1664: gx_renderer_state_and_efb_copy.cpp render-state access
    int usesVertexDeclaration;       // 0x1668: gx_renderer_state_and_efb_copy.cpp render-state access
    int activeType6ResourceTypes[7]; // 0x166c: Engine_SetType6Resource clears seven words then marks an index
    int activeType7ResourceTypes[7]; // 0x1688: Engine_SetType7Resource clears seven words then marks an index
    char pad_16a4[0x34c];
    float fogNearPlane;                  // 0x19f0: gx_renderer_state_and_efb_copy.cpp render-state access
    float fogFarPlane;                   // 0x19f4: gx_renderer_state_and_efb_copy.cpp render-state access
    char transposedTransforms[32][0x40]; // 0x19f8: SetTransform transposes states 0x0B..0x2A
    char pad_21f8[0x70];
    void* scratchVertexDeclaration; // 0x2268: gx_renderer_state_and_efb_copy.cpp render-state access
};

#define OFF(f, o) typedef char EngineState_off_##f[((int)&((EngineState*)0)->f == (o)) ? 1 : -1]
OFF(offscreenRenderingEnabled, 0x60);
OFF(palettes, 0x84);
OFF(vertexBuffer, 0xa4);
OFF(indexBuffer, 0xb4);
OFF(vertexFormat, 0xbc);
OFF(displayRenderTarget, 0xc0);
OFF(offscreenRenderTarget, 0xc8);
OFF(vertexDeclaration, 0xd0);
OFF(type6Resource, 0xd4);
OFF(type7Resource, 0xd8);
OFF(renderStates, 0xdc);
OFF(textureStageStates, 0x184);
OFF(immediateStateUpdates, 0x5c4);
OFF(copySourceX, 0x5e8);
OFF(copySourceY, 0x5ec);
OFF(copySourceWidth, 0x5f0);
OFF(copySourceHeight, 0x5f4);
OFF(transforms, 0x600);
OFF(framebufferCopyParameter0, 0x14cc);
OFF(framebufferCopyParameter1, 0x14d0);
OFF(fogStateDirty, 0x14d4);
OFF(lightingStateDirty, 0x14d8);
OFF(depthStateDirty, 0x14dc);
OFF(blendStateDirty, 0x14e0);
OFF(alphaCompareStateDirty, 0x14e4);
OFF(colorWriteStateDirty, 0x14e8);
OFF(alphaWriteStateDirty, 0x14ec);
OFF(lightingChannelStateDirty, 0x14f0);
OFF(depthScaleOffsetStateDirty, 0x14f4);
OFF(pointSizeStateDirty, 0x14f8);
OFF(textureStageComponentDirty, 0x14fc);
OFF(drawStateDirty, 0x1544);
OFF(textureStateDirty, 0x1548);
OFF(lightStateDirty, 0x1550);
OFF(textureStageDirty, 0x1574);
OFF(paletteDirty, 0x1594);
OFF(transformDirty, 0x15b4);
OFF(renderTargetRequiresCopy, 0x1664);
OFF(usesVertexDeclaration, 0x1668);
OFF(activeType6ResourceTypes, 0x166c);
OFF(activeType7ResourceTypes, 0x1688);
OFF(fogNearPlane, 0x19f0);
OFF(fogFarPlane, 0x19f4);
OFF(transposedTransforms, 0x19f8);
OFF(scratchVertexDeclaration, 0x2268);
#undef OFF
#define AsEngineState(p) ((EngineState*)(p))

#endif

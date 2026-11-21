// RenderState80422F10. Render-state singleton g_engineState accessed by gx_render_state_and_texture_upload.cpp.
// Explicit layout view without inheritance or virtual methods.
#ifndef CLASSES_RENDERSTATE80422F10_H
#define CLASSES_RENDERSTATE80422F10_H

class RenderState80422F10 {
public:
    char pad_000[0x64];
    unsigned int textures[8]; // 0x64: Render_ApplyTevState loops over eight texture handles
    unsigned int palettes[8]; // 0x84: Render_ApplyTevState calls TexturePalette_GetDescription for the corresponding palette
    char pad_0A4[0x18];
    int flexibleVertexFormat; // 0xBC: Render_ApplyLightingChannels, Render_ApplyVertexInputState, Render_SetFlexibleVertexFormatState, VertexInput_CountTextureCoordinateGenerators, Render_ApplyTevState
    char pad_0C0[0x10];
    unsigned int vertexDeclaration; // 0xD0: Render_ApplyVertexInputState, VertexInput_CountTextureCoordinateGenerators
    char pad_0D4[0x4];
    int type7Resource; // 0xD8: Render_ApplyTevState
    char pad_0DC[0x4];
    int fogEnabled; // 0xE0: Render_ApplyFogState
    int lightingEnabled; // 0xE4: Render_ApplyLightingChannels
    int depthCompare; // 0xE8: Render_ApplyDepthState passes as GXCompare to GXSetZMode
    int alphaCompare; // 0xEC: Render_ApplyAlphaTestState
    unsigned int depthWrite; // 0xF0: Render_ApplyDepthState passes as update enable to GXSetZMode
    unsigned int depthTest; // 0xF4: Render_ApplyDepthState passes as compare enable to GXSetZMode
    int alphaTestEnabled; // 0xF8: Render_ApplyAlphaTestState
    char pad_0FC[0x8];
    int blendEnabled; // 0x104: Render_ApplyBlendState
    unsigned int alphaReference; // 0x108: Render_ApplyAlphaTestState
    int blendOperation; // 0x10C: Render_ApplyBlendState
    int sourceBlend; // 0x110: Render_ApplyBlendState passes as source GXBlendFactor
    int destinationBlend; // 0x114: Render_ApplyBlendState passes as destination GXBlendFactor
    unsigned int logicOperation; // 0x118: Render_ApplyBlendState
    char pad_11C[0x8];
    int materialColorSource; // 0x124: Render_ApplyLightingChannels
    int ambientColorSource; // 0x128: Render_ApplyLightingChannels, Render_ApplyTevState
    char pad_12C[0x4];
    void* fogType; // 0x130: Render_ApplyFogState
    unsigned char fogRed; // 0x134: Render_ApplyFogState
    unsigned char fogGreen; // 0x135: Render_ApplyFogState
    unsigned char fogBlue; // 0x136: Render_ApplyFogState
    unsigned char fogAlpha; // 0x137: Render_ApplyFogState
    float fogStart; // 0x138: Render_ApplyFogState passes as first float to GXSetFog
    float fogEnd; // 0x13C: Render_ApplyFogState passes as second float to GXSetFog
    char pad_140[0x14];
    unsigned int colorWriteMask; // 0x154: Render_ApplyColorWriteState, Render_ApplyAlphaWriteState
    int specularLightingEnabled; // 0x158: Render_ApplyLightingChannels, Render_ApplyTevState
    float depthScale; // 0x15C: Render_ApplyDepthScaleOffset
    float depthOffset; // 0x160: Render_ApplyDepthScaleOffset
    char pad_164[0x4];
    float normalizedPointSize; // 0x168: Render_ApplyPointSize scales by 255 before GXSetPointSize
    char pad_16C[0x12D4];
    void* lights[8]; // 0x1440: Render_ApplyLightingChannels builds GX light masks from eight non-null pointers
    char pad_1460[0x78];
    int lightingStateDirty; // 0x14D8: Render_SetFlexibleVertexFormatState
    char pad_14DC[0x68];
    int drawStateDirty; // 0x1544: Render_SetFlexibleVertexFormatState
    int textureStateDirty; // 0x1548: Render_ApplyTevState
    int materialStateDirty; // 0x154C: Render_SetFlexibleVertexFormatState
    char pad_1550[0x68];
    int projectionTransformDirty; // 0x15B8: Render_SetFlexibleVertexFormatState
    char pad_15BC[0xAC];
    int useVertexDeclaration; // 0x1668: Render_ApplyVertexInputState, VertexInput_CountTextureCoordinateGenerators
    char pad_166C[0x34];
    int replaceLightingTevStage; // 0x16A0: Render_ApplyTevState
    char pad_16A4[0x34C];
    float fogNearPlane; // 0x19F0: Render_ApplyFogState
    float fogFarPlane; // 0x19F4: Render_ApplyFogState
};

#define OFF(f, o) typedef char RenderState80422F10_off_##f[((int)&((RenderState80422F10*)0)->f == (o)) ? 1 : -1]
OFF(textures, 0x64);
OFF(palettes, 0x84);
OFF(flexibleVertexFormat, 0xBC);
OFF(vertexDeclaration, 0xD0);
OFF(type7Resource, 0xD8);
OFF(fogEnabled, 0xE0);
OFF(lightingEnabled, 0xE4);
OFF(depthCompare, 0xE8);
OFF(alphaCompare, 0xEC);
OFF(depthWrite, 0xF0);
OFF(depthTest, 0xF4);
OFF(alphaTestEnabled, 0xF8);
OFF(blendEnabled, 0x104);
OFF(alphaReference, 0x108);
OFF(blendOperation, 0x10C);
OFF(sourceBlend, 0x110);
OFF(destinationBlend, 0x114);
OFF(logicOperation, 0x118);
OFF(materialColorSource, 0x124);
OFF(ambientColorSource, 0x128);
OFF(fogType, 0x130);
OFF(fogRed, 0x134);
OFF(fogGreen, 0x135);
OFF(fogBlue, 0x136);
OFF(fogAlpha, 0x137);
OFF(fogStart, 0x138);
OFF(fogEnd, 0x13C);
OFF(colorWriteMask, 0x154);
OFF(specularLightingEnabled, 0x158);
OFF(depthScale, 0x15C);
OFF(depthOffset, 0x160);
OFF(normalizedPointSize, 0x168);
OFF(lightingStateDirty, 0x14D8);
OFF(drawStateDirty, 0x1544);
OFF(textureStateDirty, 0x1548);
OFF(materialStateDirty, 0x154C);
OFF(projectionTransformDirty, 0x15B8);
OFF(useVertexDeclaration, 0x1668);
OFF(replaceLightingTevStage, 0x16A0);
OFF(fogNearPlane, 0x19F0);
OFF(fogFarPlane, 0x19F4);
OFF(lights, 0x1440);
#undef OFF
#define AsRenderState80422F10(p) ((RenderState80422F10*)(p))

#endif

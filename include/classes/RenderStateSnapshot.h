// RenderStateSnapshot: s_renderStateSnapshot, saved by Engine_SaveRenderState and restored by Engine_RestoreRenderState.
// Array extents follow the next observed region. Calls retain their original loop bounds.
#ifndef CLASSES_RENDERSTATESNAPSHOT_H
#define CLASSES_RENDERSTATESNAPSHOT_H

class RenderStateSnapshot {
public:
    void* textureResources[8]; // 0x0: Engine_SaveRenderState, Engine_RestoreRenderState
    void* paletteResources[8]; // 0x20: Engine_SaveRenderState, Engine_RestoreRenderState
    unsigned int vertexStream; // 0x40: Engine_SaveRenderState, Engine_RestoreRenderState
    unsigned int vertexStride; // 0x44: Engine_SaveRenderState, Engine_RestoreRenderState
    void* indexBuffer;         // 0x48: Engine_SaveRenderState, Engine_RestoreRenderState
    unsigned int indexOffset;  // 0x4C: Engine_SaveRenderState, Engine_RestoreRenderState
    unsigned int vertexFormat; // 0x50: Engine_SaveRenderState, Engine_RestoreRenderState
    char pad_054[0xC];
    void* renderTarget;                     // 0x60: Engine_SaveRenderState, Engine_RestoreRenderState
    void* depthStencilSurface;              // 0x64: Engine_SaveRenderState, Engine_RestoreRenderState
    void* vertexDeclaration;                // 0x68: Engine_SaveRenderState, Engine_RestoreRenderState
    void* type6Resource;                    // 0x6C: Engine_SaveRenderState, Engine_RestoreRenderState
    void* type7Resource;                    // 0x70: Engine_SaveRenderState, Engine_RestoreRenderState
    void* renderStates[42];                 // 0x74: Engine_SaveRenderState, Engine_RestoreRenderState
    void* textureStageStates[200];          // 0x11C: Engine_SaveRenderState, Engine_RestoreRenderState
    void* samplerStates[72];                // 0x43C: Engine_SaveRenderState, Engine_RestoreRenderState
    char systemOptions[36];                 // 0x55C: Engine_SaveRenderState, Engine_RestoreRenderState
    char viewport[24];                      // 0x580: Engine_SaveRenderState, Engine_RestoreRenderState
    char transforms[2816];                  // 0x598: Engine_SaveRenderState, Engine_RestoreRenderState
    char lights[832];                       // 0x1098: Engine_SaveRenderState, Engine_RestoreRenderState
    void* lightEnabled[8];                  // 0x13D8: Engine_SaveRenderState, Engine_RestoreRenderState
    char material[92];                      // 0x13F8: Engine_SaveRenderState, Engine_RestoreRenderState
    unsigned int useIndexedVertexInput;     // 0x1454: Engine_SaveRenderState, Engine_RestoreRenderState
    unsigned int drawPrimitiveType;         // 0x1458: Engine_SaveRenderState, Engine_RestoreRenderState
    unsigned int drawUsesBoundVertexBuffer; // 0x145C: Engine_SaveRenderState, Engine_RestoreRenderState
    unsigned int drawUsesIndexDisplayList;  // 0x1460: Engine_SaveRenderState, Engine_RestoreRenderState
    unsigned int boundBufferDrawArgument;   // 0x1464: Engine_SaveRenderState, Engine_RestoreRenderState
    unsigned int formatDescriptor[8];       // 0x1468: Engine_SaveRenderState, Engine_RestoreRenderState
};

#define OFF(f, o) typedef char RenderStateSnapshot_off_##f[((int)&((RenderStateSnapshot*)0)->f == (o)) ? 1 : -1]
OFF(textureResources, 0x0);
OFF(paletteResources, 0x20);
OFF(vertexStream, 0x40);
OFF(vertexStride, 0x44);
OFF(indexBuffer, 0x48);
OFF(indexOffset, 0x4C);
OFF(vertexFormat, 0x50);
OFF(renderTarget, 0x60);
OFF(depthStencilSurface, 0x64);
OFF(vertexDeclaration, 0x68);
OFF(type6Resource, 0x6C);
OFF(type7Resource, 0x70);
OFF(renderStates, 0x74);
OFF(textureStageStates, 0x11C);
OFF(samplerStates, 0x43C);
OFF(systemOptions, 0x55C);
OFF(viewport, 0x580);
OFF(transforms, 0x598);
OFF(lights, 0x1098);
OFF(lightEnabled, 0x13D8);
OFF(material, 0x13F8);
OFF(useIndexedVertexInput, 0x1454);
OFF(drawPrimitiveType, 0x1458);
OFF(drawUsesBoundVertexBuffer, 0x145C);
OFF(drawUsesIndexDisplayList, 0x1460);
OFF(boundBufferDrawArgument, 0x1464);
OFF(formatDescriptor, 0x1468);
#undef OFF

#endif

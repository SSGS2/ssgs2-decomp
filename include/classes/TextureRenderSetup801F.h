// Layout view of textureCaptureRenderStates, used by ShadowModel_DrawWithProjectedTextureStages in shadow_model_cache_and_shared_doc.cpp.
// The render API calls identify these matrix, slot and state-list regions.
#ifndef CLASSES_TEXTURERENDERSETUP801F_H
#define CLASSES_TEXTURERENDERSETUP801F_H

class TextureRenderSetup801F {
public:
    char pad_000[0x1A8];
    float baseTransform[16];                   // 0x1A8: final PSMTX44Concat operand
    unsigned int transformSlots[8];            // 0x1E8: SetTransform slot IDs; next region begins at 0x208
    unsigned int baseTextureStageStates[12];   // 0x208: six Engine_SetTextureStageStateList pairs
    unsigned int baseStageParameters[10];      // 0x238: five Engine_SetTextureStageParameterList pairs
    unsigned int objectTextureStageStates[16]; // 0x260: eight stage-state pairs for list objects
    unsigned int extraTextureStageStates[16];  // 0x2A0: eight stage-state pairs for optional matrix
    unsigned int objectStageParameters[10];    // 0x2E0: five Engine_SetTextureStageParameterList pairs
};

#define OFF(f, o) typedef char TextureRenderSetup801F_off_##f[((int)&((TextureRenderSetup801F*)0)->f == (o)) ? 1 : -1]
OFF(baseTransform, 0x1A8);
OFF(transformSlots, 0x1E8);
OFF(baseTextureStageStates, 0x208);
OFF(baseStageParameters, 0x238);
OFF(objectTextureStageStates, 0x260);
OFF(extraTextureStageStates, 0x2A0);
OFF(objectStageParameters, 0x2E0);
#undef OFF
#define AsTextureRenderSetup801F(p) ((TextureRenderSetup801F*)(p))
#endif

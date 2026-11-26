// CFxBox: the effect box. It owns the live effect sequences and sprays together
// with the caches they are loaded from: the animated-sprite map, the spray and
// sequence template maps, and the two effect registries.
//
// The class is declared without virtual functions on purpose: its vtable is
// still taken from the original game data, so `vtbl` is spelled out as a member.
#ifndef CLASSES_CFXBOX_H
#define CLASSES_CFXBOX_H

#include "game/types.h"

struct Vec2;
class CFxSequence;
class CFxSpray;

class CFxBox {
public:
    // Advance every live spray and sequence by delta milliseconds. Returns 0 when
    // a sequence reports that it has expired, otherwise 1.
    int UpdateEffects(float delta);

    // Whether current is within tolerance of target.x, or the step from previous
    // is close enough to cross it.
    int TestHeightCrossing(float maximumStep, struct Vec2* previous, struct Vec2* current, struct Vec2* target);

    // Release every live effect and empty all resource caches.
    void ResetResources();

    // The live sequence and spray registries, keyed by effect address. Registering
    // or removing an effect also updates any subclass attachment cache through the
    // box's virtual hooks.
    void RegisterSequence(CFxSequence* sequence);
    void RegisterSpray(CFxSpray* spray);
    void RemoveSequence(CFxSequence* sequence, int mode);
    void RemoveSpray(CFxSpray* spray);
    // Erase the sequence whose registry node *iter names, with the child policy mode.
    void EraseSequenceAtIterator(struct FxRegistryNode** iter, int mode);
    // Remove every live effect. With a nonzero mode, effects whose +4 byte is
    // zero are preserved.
    void ClearEffects(int mode);

    // Release the cached resource handles and then the sprite-cache nodes.
    void ClearCachedResourceMap();
    // Delete the cached templates, then free their key nodes.
    void ClearSprayTemplateCache();
    void ClearSequenceTemplateCache();

    // Borrowed pointers to the resource manager and the camera transform.
    void SetResourceManager(void* manager);
    void SetCameraTransform(const void* transform);

    void* vtbl;                        // 0x00
    float nominalStepMilliseconds;     // 0x04
    float timeScale;                   // 0x08
    char unk_C;                        // 0x0C
    char pad_00D[0x103];               // 0x0D
    unsigned char pause;               // 0x110
    char pad_111[0x3];                 // 0x111
    int renderFlags;                   // 0x114
    unsigned char clampSimulationStep; // 0x118
    char pad_119[0x3];                 // 0x119
    int unk_11C;                       // 0x11C
    int unk_120;                       // 0x120
    char debugBoundsEnabled;           // 0x124
    char pad_125[0x3];                 // 0x125

    // Each cache below is a tree header: node count, root, and an iterator that
    // points at the root slot while the cache is empty.
    int sprayTemplateCacheCount;      // 0x128
    int sprayTemplateCacheRoot;       // 0x12C
    void* sprayTemplateCacheFirst;    // 0x130
    int sequenceTemplateCacheCount;   // 0x134
    int sequenceTemplateCacheRoot;    // 0x138
    void* sequenceTemplateCacheFirst; // 0x13C

    void* resourceManager;  // 0x140
    int spriteCacheCount;   // 0x144
    int spriteCacheRoot;    // 0x148
    char pad_14C[0x4];      // 0x14C
    void* spriteCacheFirst; // 0x150

    int sprayCount;          // 0x154
    int sprayTreeRoot;       // 0x158
    void* sprayTreeFirst;    // 0x15C
    int sequenceCount;       // 0x160
    int sequenceTreeRoot;    // 0x164
    void* sequenceTreeFirst; // 0x168

    const void* cameraTransformAddress; // 0x16C
    void* unk_170;                      // 0x170
    void* unk_174;                      // 0x174
    void* unk_178;                      // 0x178
    void* unk_17C;                      // 0x17C
    int unk_180;                        // 0x180
    char pad_184[0x10];                 // 0x184
    int unk_194;                        // 0x194
    char pad_198[0x4];                  // 0x198
    float unk_19C;                      // 0x19C
    float unk_1A0;                      // 0x1A0
    char pad_1A4[0x28];                 // 0x1A4
    int unk_1CC;                        // 0x1CC
    char pad_1D0[0x4];                  // 0x1D0
    float unk_1D4;                      // 0x1D4
    float unk_1D8;                      // 0x1D8
    char pad_1DC[0x28];                 // 0x1DC
    int unk_204;                        // 0x204
    char pad_208[0x4];                  // 0x208
    float unk_20C;                      // 0x20C
    float unk_210;                      // 0x210
    char pad_214[0x28];                 // 0x214
    int unk_23C;                        // 0x23C
    char pad_240[0x4];                  // 0x240
    float unk_244;                      // 0x244
    float unk_248;                      // 0x248

    void vfunc_1();
    void DrawSprays(void* configuration);

    void vfunc_6();
    void vfunc_4();
    void vfunc_3();
    void vfunc_5();
    void* GetOrLoadAnimatedSprite(const char* bitmapName, unsigned int cellWidth, unsigned int cellHeight);

    CFxBox();

    ~CFxBox();
};

#define CFXBOX_OFF(f, o) typedef char CFxBox_off_##f[((int)&((CFxBox*)0)->f == (o)) ? 1 : -1]
CFXBOX_OFF(vtbl, 0x0);
CFXBOX_OFF(nominalStepMilliseconds, 0x4);
CFXBOX_OFF(timeScale, 0x8);
CFXBOX_OFF(unk_C, 0xc);
CFXBOX_OFF(pause, 0x110);
CFXBOX_OFF(renderFlags, 0x114);
CFXBOX_OFF(clampSimulationStep, 0x118);
CFXBOX_OFF(unk_11C, 0x11c);
CFXBOX_OFF(unk_120, 0x120);
CFXBOX_OFF(debugBoundsEnabled, 0x124);
CFXBOX_OFF(sprayTemplateCacheCount, 0x128);
CFXBOX_OFF(sprayTemplateCacheRoot, 0x12c);
CFXBOX_OFF(sprayTemplateCacheFirst, 0x130);
CFXBOX_OFF(sequenceTemplateCacheCount, 0x134);
CFXBOX_OFF(sequenceTemplateCacheRoot, 0x138);
CFXBOX_OFF(sequenceTemplateCacheFirst, 0x13c);
CFXBOX_OFF(resourceManager, 0x140);
CFXBOX_OFF(spriteCacheCount, 0x144);
CFXBOX_OFF(spriteCacheRoot, 0x148);
CFXBOX_OFF(spriteCacheFirst, 0x150);
CFXBOX_OFF(sprayCount, 0x154);
CFXBOX_OFF(sprayTreeRoot, 0x158);
CFXBOX_OFF(sprayTreeFirst, 0x15c);
CFXBOX_OFF(sequenceCount, 0x160);
CFXBOX_OFF(sequenceTreeRoot, 0x164);
CFXBOX_OFF(sequenceTreeFirst, 0x168);
CFXBOX_OFF(cameraTransformAddress, 0x16c);
CFXBOX_OFF(unk_170, 0x170);
CFXBOX_OFF(unk_174, 0x174);
CFXBOX_OFF(unk_178, 0x178);
CFXBOX_OFF(unk_17C, 0x17c);
CFXBOX_OFF(unk_180, 0x180);
CFXBOX_OFF(unk_194, 0x194);
CFXBOX_OFF(unk_19C, 0x19c);
CFXBOX_OFF(unk_1A0, 0x1a0);
CFXBOX_OFF(unk_1CC, 0x1cc);
CFXBOX_OFF(unk_1D4, 0x1d4);
CFXBOX_OFF(unk_1D8, 0x1d8);
CFXBOX_OFF(unk_204, 0x204);
CFXBOX_OFF(unk_20C, 0x20c);
CFXBOX_OFF(unk_210, 0x210);
CFXBOX_OFF(unk_23C, 0x23c);
CFXBOX_OFF(unk_244, 0x244);
CFXBOX_OFF(unk_248, 0x248);
#undef CFXBOX_OFF
typedef char CFxBox_size[(sizeof(CFxBox) == 0x24C) ? 1 : -1];

#endif // CLASSES_CFXBOX_H

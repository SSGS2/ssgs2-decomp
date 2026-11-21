#include <decomp/static_init.h>
#include <decomp/offset_of.h>
// Sprite channels use signed fixed-point scale (1.0 = kBpScaleOne); packed colour unity is 128.
enum BpSpriteConstants {
    kBpInvalidSlotSourceLine = 0x84, kBpUnregisteredSlotSourceLine = 0x89,
    kBpModeFlag1 = 1, kBpModeFlag2 = 2,
    kBpScaleOne = 1024, kBpColorOne = 128, kBpSlotCount = 40,
    kBpChannelsPerSprite = 18, kBpChannelBytes = 0x18, kBpSpriteBytes = 0x1C8,
    kBpFormatMask = 0xF000, kBpTileFlipMask = 0xF0000, kBpBlendMask = 0xF0,
    kBpFlipHorizontal = 0x1000, kBpFlipVertical = 0x2000,
    kBpAlternateColumnX = 0x10000, kBpAlternateColumnY = 0x20000,
    kBpAlternateRowX = 0x40000, kBpAlternateRowY = 0x80000
};
enum BpSpriteChannelIndex {
    kImageChannel = 0, kSourceXChannel = 1, kSourceYChannel = 2,
    kWidthChannel = 3, kHeightChannel = 4, kClipChannel = 5,
    kPositionXChannel = 6, kPositionYChannel = 7,
    kScaleXChannel = 8, kScaleYChannel = 9, kOpacityChannel = 10,
    kAuxiliaryChannel = 11, kColumnsChannel = 12, kRowsChannel = 13,
    kPivotXChannel = 14, kPivotYChannel = 15, kRotationChannel = 16, kTintChannel = 17
};
struct BpSpriteChannel {
    int baseValue; int value; short mode; short padding;
    int animationState; int propertyAddress; int animationData;
};
struct BpSpriteState {
    short id; short flags; int key; int userData;
    int drawRectCallback; int drawQuadCallback; void* next;
    BpSpriteChannel channels[kBpChannelsPerSprite];
};
// Controller slots are 0x34 bytes, distinct from their 0x1C8-byte sprite pool.
// RegisterWork borrows one backing range: sprites first, then 0x10-byte work
// records. Reset invalidates keys/list links but never frees either pool.
struct BpAuxiliaryWorkView { int word0, word1, key, word3; };
struct BpSlotResetView {
    int slotId; void* backingRange; BpSpriteState* sprites;
    BpAuxiliaryWorkView* auxiliaryWork;
    int spriteCount, auxiliaryCount, word18;
    int positionX, positionY, opacity, scaleX, scaleY;
    BpSpriteState* activeHead;
};
typedef char BpSlotResetView_size[(sizeof(BpSlotResetView) == 0x34) ? 1 : -1];
typedef char BpSpriteState_size[(sizeof(BpSpriteState) == kBpSpriteBytes) ? 1 : -1];
typedef char BpSpriteState_imageOffset[((int)DECOMP_OFFSETOF(BpSpriteState, channels[0].value) == 0x1C) ? 1 : -1];
#define BP_STATE(p) ((BpSpriteState*)(p))
#define BP_STATE_OFFSET(field) ((int)DECOMP_OFFSETOF(BpSpriteState, field))

#include "classes/BpSpriteDrawView.h"
#define AsBpDrawObject800BC91C(p) ((BpSpriteDrawView*)(p))

// Translation unit 0x800BC91C-0x800BDF64 (14 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

extern "C" {

extern int Bp_GetDefaultRectDrawCallback();
extern int BpSprite_ComputeUnrotatedRect(int *, void *, void *);
extern unsigned int Color_ModulatePackedChannels(int, int);

#pragma push
#pragma use_lmw_stmw on
// Draw a fixed-point tile grid, optionally inheriting scale, colour, and opacity from another sprite.
void BpSprite_DrawTilesUnrotated(void *self, void *other, int columns, int rows, int x, int y,
                 int widthScale, int heightScale, int opacityScale);
#pragma pop

extern const float kFloat_6_2831855;
extern float s_bpSpriteFullTurnAngle;
extern const float kFloat_0_0078125_BpColorMultiply;
extern const float kBpColorChannelMaximum;
extern const float kFloat_0_BpColorMultiply;
extern const char kBpControllerSourceFilename[];
extern const char bpSlotRegistrationDiagnosticStrings[];

extern int BpCtrl_InvokeGlobalHook(int, const char *, int, const char *);
extern int BpObject_ClearWorkBuffers(int);
extern unsigned int Color_ModulatePackedChannels(int, int);
extern const char bpControllerCallbackDiagnosticStrings[];
extern unsigned int s_bpCachedRenderCallbackValue;
extern void *memset(void *, int, unsigned int);
extern void BpSprite_DrawTilesRotated(void *, void *, int, int, int, int, int, int, int);
extern void BpSprite_DrawTilesUnrotated(void *, void *, int, int, int, int, int, int, int);
extern int Math_RotateIntegerPoints2D(void *, int, int, int, int);

struct B2dCtl;
extern unsigned int s_bpObjectEntries[];
extern int g_valueAnimationUpdateMode;
extern int g_bpDefaultRectDrawCallback;
extern int g_bpDrawObjectCallback;
extern int g_bpTileDrawCallback;

// Clear all 40 controller slots, restore unused IDs and colour unity, and reset draw callbacks.
int CBpCtrl_InitializeObjectEntries();

// Compute tiled pixel bounds from sprite size, inherited scale, position, and pivot.
// out is four writable pixel integers [left,top,width,height]. parentSprite is
// optional/borrowed; only its position/scale are inherited. Scale and pivot use
// 1024 as unity; negative tile counts contribute their absolute magnitude.
// Multiply/divide stages intentionally truncate separately; combining them
// changes both fixed-point rounding and overflow behavior.
int BpSprite_ComputeUnrotatedRect(int *out, void *sprite, void *parentSprite);

// Multiply four packed colour channels with 128 as unity, clamping each result to a byte.
unsigned int Color_ModulatePackedChannels(int a, int b);
// Reset a valid slot's work records, draw objects, transforms, and cached render callback.
int BpObject_ClearWorkBuffers(int index);

extern int BpCtrl_FindOrAllocObject(void *, int, int *);
extern int bpChannelDefaultOverrides[];
extern void BpSprite_ComputeRotatedQuad(int *, void *, void *);
extern int BpCtrl_GetTexturedQuadCallback(void);

#pragma push
#pragma use_lmw_stmw on
// Subdivide the transformed quadrilateral into tiles, applying alternate-row/column flips.
void BpSprite_DrawTilesRotated(void *self, void *other, int columnCount, int rowCount,
                 int startX, int startY, int widthScale, int heightScale,
                 int opacityScale);
#pragma pop

 #pragma push
#pragma use_lmw_stmw on
// Allocate a sprite, reset its animation channels, initialize its image rectangle, and link it into the slot.
// All creation paths return the pooled sprite's short ID, or -1 for invalid
// slots/exhaustion. objectKey identifies an existing sprite; alreadyExists is
// true for reuse, NOT allocation failure (bp_layout_animation_and_sprite_draw.cpp). Only new sprites are
// appended to activeHead, following next at +0x14; the slot owns pool storage.
// Channel reset contract shared by the four paths: base/current=0, mode=0,
// animation state/property/data=0, then apply (channelIndex,mode) overrides
// until an override key<=0. The first two paths reset 18 channels as nine
// pairs, stepping 0x30; the others spell out one 0x18-byte channel per step.
int BpCtrl_CreateDrawObject(int index, int objectKey, int image, int sourceX, int sourceY, int sourceWidth, int sourceHeight, int auxiliaryValue, int userData);
#pragma pop

// Initialize a sprite with both special mode bits and reset all animation channels.
// Flags 1/2 select the existing data-driven/special paths; their complete draw
// semantics are not established here. This variant sets both, zero dimensions,
// auxiliary value 100 and clip-channel key=objectKey; do not treat 3 as an ID.
int BpCtrl_CreateDrawObjectFlag3(int index, int objectKey, int userData);

#pragma push
#pragma use_lmw_stmw on
// Initialize a data-driven sprite, set its dimensions, and append newly allocated objects to the slot.
// base is borrowed; its first word becomes sourceX under flag bit 1. Width and
// height are pixel dimensions; auxiliaryValue retains the undecoded channel-11 role.
int BpCtrl_CreateDrawObjectFromData(int index, int objectKey, int *base, int sourceWidth, int sourceHeight, int auxiliaryValue);

// Initialize a mode-2 sprite with dimensions and caller data, then link new objects into the slot.
// sourceWidth/sourceHeight are pixels; auxiliaryValue populates channel 11 and
// userData is the opaque +8 word. Flag bit 2 is preserved without guessing its renderer meaning.
int BpCtrl_CreateDrawObjectFlag2(int index, int objectKey, int sourceWidth, int sourceHeight, int auxiliaryValue, int userData);
#pragma pop

// Expand pixel bounds into four corners and rotate them around the sprite pivot.
// out holds [TL.x,TL.y,BL.x,BL.y,BR.x,BR.y,TR.x,TR.y]; pivot fractions use
// 1024 as unity. rotation uses the integer angle units of Math_RotateIntegerPoints2D.
void BpSprite_ComputeRotatedQuad(int *out, void *sprite, void *parentSprite);

// Initialize this sprite unit's full-turn angle constant.
void BpSprite_InitFullTurnConstant();

// Register the slot's backing draw-object and auxiliary-work ranges, then reset their state.
// start is a borrowed address represented as int; count is the sprite capacity.
// auxiliaryCountWord is also an integer count despite the legacy pointer type:
// callers in bp_object_sprite_resources.cpp pass animationWorkCount, and ClearWorkBuffers uses it as
// the auxiliary loop bound. This call neither allocates nor takes heap ownership.
int BpSlot_RegisterWork(int index, int start, int count, const char *auxiliaryCountWord);

// Dispatch tiled drawing using combined rotation; offsets are pixels, scales use 1024/128 unity.
int BpSprite_DrawTiles(void *self, void *other, int startX, int startY, int widthScale,
                int heightScale, int opacityScale, int unused);

} // extern "C"

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU80462088", _ctors_u800BC91C_0, BpSprite_InitFullTurnConstant);
#pragma pop


// Function definitions follow the original address order.


#pragma push
#pragma dont_inline on
#pragma use_lmw_stmw on
extern "C" {
// Create or reuse an image sprite, reset its channels and link a new allocation.
int BpCtrl_CreateDrawObject(int index, int objectKey, int image, int sourceX, int sourceY, int sourceWidth, int sourceHeight, int auxiliaryValue, int userData) {
    unsigned int z = 0;
    unsigned char * record;
    if (index < 0 || index >= kBpSlotCount) {
        BpCtrl_InvokeGlobalHook(index, kBpControllerSourceFilename, kBpInvalidSlotSourceLine, bpControllerCallbackDiagnosticStrings);
        record = 0;
    } else {
        record = (unsigned char *)s_bpObjectEntries + index * sizeof(BpSlotResetView);
        if (*(int *)record == -1) {
            BpCtrl_InvokeGlobalHook(index, kBpControllerSourceFilename, kBpUnregisteredSlotSourceLine, bpControllerCallbackDiagnosticStrings);
            record = 0;
        }
    }
    if (!record) return -1;
    int alreadyExists;
    unsigned char *obj = (unsigned char *)BpCtrl_FindOrAllocObject(record, objectKey, &alreadyExists);
    // 0x800BC91C: Initialize the allocated object before the shared result check.
    do {
        if (obj == 0) { obj = 0; break; }
        unsigned int z = 0;
        BP_STATE(obj)->flags = z;
        BP_STATE(obj)->key = objectKey;
        (*(int*)&BP_STATE(obj)->next) = z;
        BP_STATE(obj)->userData = userData;
        // Phase 1: reset both channels of each pair before assigning properties.
        int *entry;
        unsigned char *p = obj;
        for (int i = 0, ch = 0; i < kBpChannelsPerSprite / 2; ++i, p += 2 * kBpChannelBytes) {
            BP_STATE(p)->channels[0].animationState = z;
            BP_STATE(p)->channels[0].animationData = z;
            BP_STATE(p)->channels[0].propertyAddress = z;
            BP_STATE(p)->channels[0].baseValue = z;
            BP_STATE(p)->channels[0].value = z;
            BP_STATE(p)->channels[0].mode = z;
            int j;
            int *entry = bpChannelDefaultOverrides;
            for (j = 0; entry[0] > 0; entry += 2, ++j)
                if (ch == entry[0]) {
                    BP_STATE(p)->channels[0].mode = bpChannelDefaultOverrides[(2 * j) + 1];
                    break;
                }
            ++ch;
            BP_STATE(p)->channels[1].animationState = z;
            BP_STATE(p)->channels[1].animationData = z;
            BP_STATE(p)->channels[1].propertyAddress = z;
            BP_STATE(p)->channels[1].baseValue = z;
            BP_STATE(p)->channels[1].value = z;
            BP_STATE(p)->channels[1].mode = z;
            int j2;
            entry = bpChannelDefaultOverrides;
            for (j2 = 0; entry[0] > 0; entry += 2, ++j2)
                if (ch == entry[0]) {
                    BP_STATE(p)->channels[1].mode = bpChannelDefaultOverrides[(2 * j2) + 1];
                    break;
                }
            ++ch;
        }
    } while (0);

    if (!obj) return -1;
    // Phase 2: install current/base property pairs (source rectangle is pixels).
    AsBpDrawObject800BC91C(obj)->image = image;
    BP_STATE(obj)->channels[kImageChannel].baseValue = image;
    AsBpDrawObject800BC91C(obj)->sourceX = sourceX;
    BP_STATE(obj)->channels[kSourceXChannel].baseValue = sourceX;
    AsBpDrawObject800BC91C(obj)->sourceY = sourceY;
    BP_STATE(obj)->channels[kSourceYChannel].baseValue = sourceY;
    AsBpDrawObject800BC91C(obj)->sourceWidth = sourceWidth;
    BP_STATE(obj)->channels[kWidthChannel].baseValue = sourceWidth;
    AsBpDrawObject800BC91C(obj)->sourceHeight = sourceHeight;

    BP_STATE(obj)->channels[kHeightChannel].baseValue = sourceHeight;
    BP_STATE(obj)->channels[kAuxiliaryChannel].value = auxiliaryValue;
    BP_STATE(obj)->channels[kAuxiliaryChannel].baseValue = auxiliaryValue;
    AsBpDrawObject800BC91C(obj)->scaleX = kBpScaleOne;
    BP_STATE(obj)->channels[kScaleXChannel].baseValue = kBpScaleOne;
    AsBpDrawObject800BC91C(obj)->scaleY = kBpScaleOne; BP_STATE(obj)->channels[kScaleYChannel].baseValue = kBpScaleOne;
    AsBpDrawObject800BC91C(obj)->pivotX = 0;
    BP_STATE(obj)->channels[kPivotXChannel].baseValue = 0;
    AsBpDrawObject800BC91C(obj)->pivotY = 0; BP_STATE(obj)->channels[kPivotYChannel].baseValue = 0;
    BP_STATE(obj)->channels[kClipChannel].value = -1; BP_STATE(obj)->channels[kClipChannel].baseValue = -1;
    // Phase 3: append only new objects; reused objects already have a list entry.
    if (!alreadyExists) {
        void **head = (void **)(record + 0x30);
        if (!*head) *head = obj;
        else {
            void **node = (void **)*head;
            while (node[5]) node = (void **)node[5];
            node[5] = obj;
        }
    }
    return *(short *)obj;
}
}
#pragma pop

#pragma push
#pragma dont_inline on
#pragma use_lmw_stmw on
extern "C" {
// Create or reuse a data-driven sprite with pixel dimensions and default animation channels.
int BpCtrl_CreateDrawObjectFromData(int index, int objectKey, int *base, int sourceWidth, int sourceHeight, int auxiliaryValue) { unsigned char *record;
    int alreadyExists;

    if (0 > index || kBpSlotCount <= index) {
        BpCtrl_InvokeGlobalHook(index, kBpControllerSourceFilename, kBpInvalidSlotSourceLine, bpControllerCallbackDiagnosticStrings);
        record = 0;
    } else {
        record = (unsigned char *)s_bpObjectEntries + index * sizeof(BpSlotResetView);
        if (*(int *)record == -1) {
            BpCtrl_InvokeGlobalHook(index, kBpControllerSourceFilename, kBpUnregisteredSlotSourceLine, bpControllerCallbackDiagnosticStrings);
            record = 0;
        }
    }
    if (record == 0) return -1;
    int *obj = (int *)BpCtrl_FindOrAllocObject(record, objectKey, &alreadyExists);
    // 0x800BCB8C: Initialize the allocated object before the shared result check.
    do {
        if (obj == 0) { obj = 0; break; }
        unsigned int z = 0;
        BP_STATE(obj)->flags = z;
        BP_STATE(obj)->key = objectKey;
        (*(int*)&BP_STATE(obj)->next) = z;
        BP_STATE(obj)->userData = z;
        // 18 slots of 0x18 bytes (the original compiler unrolled this loop by two, which is
        // why it looks like 9 steps of 0x30): slot i takes its value from the table entry whose key is i.
        int i;
        char *p = (char *)obj;
        for (i = 0; i < kBpChannelsPerSprite; p += kBpChannelBytes, ++i) {
            BP_STATE(p)->channels[0].animationState = 0; BP_STATE(p)->channels[0].animationData = 0;
            BP_STATE(p)->channels[0].propertyAddress = 0; BP_STATE(p)->channels[0].baseValue = 0;
            BP_STATE(p)->channels[0].value = 0; BP_STATE(p)->channels[0].mode = 0;
            int j;
            int *entry = bpChannelDefaultOverrides;
            for (j = 0; entry[0] > 0; entry += 2, ++j)
                if (i == entry[0]) { BP_STATE(p)->channels[0].mode = bpChannelDefaultOverrides[2 * j + 1]; break; }
        }
    } while (0);

    if (obj == 0) return -1;
    int data = *base;
    BP_STATE(obj)->flags |= kBpModeFlag1;
    AsBpDrawObject800BC91C(obj)->sourceX = data;
    BP_STATE(obj)->channels[kSourceXChannel].baseValue = data;
    AsBpDrawObject800BC91C(obj)->sourceWidth = sourceWidth; BP_STATE(obj)->channels[kWidthChannel].baseValue = sourceWidth;
    AsBpDrawObject800BC91C(obj)->sourceHeight = sourceHeight; BP_STATE(obj)->channels[kHeightChannel].baseValue = sourceHeight;
    BP_STATE(obj)->channels[kAuxiliaryChannel].value = auxiliaryValue; BP_STATE(obj)->channels[kAuxiliaryChannel].baseValue = auxiliaryValue;
    AsBpDrawObject800BC91C(obj)->scaleX = kBpScaleOne; BP_STATE(obj)->channels[kScaleXChannel].baseValue = kBpScaleOne;
    AsBpDrawObject800BC91C(obj)->scaleY = kBpScaleOne;
    BP_STATE(obj)->channels[kScaleYChannel].baseValue = kBpScaleOne;
    AsBpDrawObject800BC91C(obj)->pivotX = 0; BP_STATE(obj)->channels[kPivotXChannel].baseValue = 0;
    AsBpDrawObject800BC91C(obj)->pivotY = 0; BP_STATE(obj)->channels[kPivotYChannel].baseValue = 0;
    BP_STATE(obj)->channels[kClipChannel].value = -1;
    BP_STATE(obj)->channels[kClipChannel].baseValue = -1;
    if (!alreadyExists) {
        void **head = (void **)(record + 0x30);
        if (!*head) *head = obj;
        else { void **node = (void **)*head; while (node[5]) node = (void **)node[5]; node[5] = obj; }
    }
    return *(short *)obj;
}
}
#pragma pop

#pragma push
#pragma dont_inline on
#pragma use_lmw_stmw on
extern "C" {
// Create or reuse a mode-2 sprite and apply its caller data and dimensions.
int BpCtrl_CreateDrawObjectFlag2(int index, int objectKey, int sourceWidth, int sourceHeight, int auxiliaryValue, int userData) {
    unsigned char *record;
    void *obj;
    int alreadyExists;

    if (0 > index || kBpSlotCount <= index) {
        BpCtrl_InvokeGlobalHook(index, kBpControllerSourceFilename, kBpInvalidSlotSourceLine, bpControllerCallbackDiagnosticStrings);
        record = 0;
    } else {
        record = (unsigned char *)s_bpObjectEntries + index * sizeof(BpSlotResetView);
        if (*(int *)record == -1) {
            BpCtrl_InvokeGlobalHook(index, kBpControllerSourceFilename, kBpUnregisteredSlotSourceLine, bpControllerCallbackDiagnosticStrings);
            record = 0;
        }
    }
    if (record == 0) return -1;

    obj = (void *)BpCtrl_FindOrAllocObject(record, objectKey, &alreadyExists);
    // 0x800BCDF0: Initialize the allocated object before the shared result check.
    do {
        if (0 == obj) { obj = 0; break; }
        BP_STATE(obj)->flags = 0;
        BP_STATE(obj)->key = objectKey;
        (*(int*)&BP_STATE(obj)->next) = 0;
        BP_STATE(obj)->userData = userData;
        int i;
        char *p = (char *)obj;
        for (i = 0; i < kBpChannelsPerSprite; p += kBpChannelBytes, ++i) {
            BP_STATE(p)->channels[0].animationState = 0;
            BP_STATE(p)->channels[0].animationData = 0;
            BP_STATE(p)->channels[0].propertyAddress = 0; BP_STATE(p)->channels[0].baseValue = 0;
            BP_STATE(p)->channels[0].value = 0;
            BP_STATE(p)->channels[0].mode = 0;
            int j;
            int *entry = bpChannelDefaultOverrides;
            for (j = 0; entry[0] > 0; entry += 2, ++j)
                if (i == entry[0]) { BP_STATE(p)->channels[0].mode = bpChannelDefaultOverrides[2 * j + 1]; break; }
        }
    } while (0);

    if (!obj) return -1;
    BP_STATE(obj)->flags |= kBpModeFlag2;
    AsBpDrawObject800BC91C(obj)->sourceWidth = sourceWidth; BP_STATE(obj)->channels[kWidthChannel].baseValue = sourceWidth;
    AsBpDrawObject800BC91C(obj)->sourceHeight = sourceHeight; BP_STATE(obj)->channels[kHeightChannel].baseValue = sourceHeight;
    BP_STATE(obj)->channels[kAuxiliaryChannel].value = auxiliaryValue;
    BP_STATE(obj)->channels[kAuxiliaryChannel].baseValue = auxiliaryValue;
    AsBpDrawObject800BC91C(obj)->scaleX = kBpScaleOne;
    BP_STATE(obj)->channels[kScaleXChannel].baseValue = kBpScaleOne;
    AsBpDrawObject800BC91C(obj)->scaleY = kBpScaleOne;
    BP_STATE(obj)->channels[kScaleYChannel].baseValue = kBpScaleOne;
    AsBpDrawObject800BC91C(obj)->pivotX = 0; BP_STATE(obj)->channels[kPivotXChannel].baseValue = 0;
    AsBpDrawObject800BC91C(obj)->pivotY = 0;
    BP_STATE(obj)->channels[kPivotYChannel].baseValue = 0;
    BP_STATE(obj)->channels[kClipChannel].value = -1; BP_STATE(obj)->channels[kClipChannel].baseValue = -1;
    if (alreadyExists == 0) {
        void **tail = (void **)(record + 0x30);
        if (!*tail) *tail = obj;
        else { void **p = (void **)*tail; while (p[5]) p = (void **)p[5]; p[5] = obj; }
    }
    return *(short *)obj;
}
}
#pragma pop

#pragma push
#pragma dont_inline on
#pragma use_lmw_stmw reset
extern "C" {
// Create or reuse a sprite with both special mode flags and zero dimensions.
int BpCtrl_CreateDrawObjectFlag3(int index, int objectKey, int userData) {
    unsigned char * record;
    if (0 > index || kBpSlotCount <= index) {
        BpCtrl_InvokeGlobalHook(index, kBpControllerSourceFilename, kBpInvalidSlotSourceLine, bpControllerCallbackDiagnosticStrings);
        record = 0;
    } else {
        record = (unsigned char *)s_bpObjectEntries + index * sizeof(BpSlotResetView);
        if (*(int *)record == -1) {
            BpCtrl_InvokeGlobalHook(index, kBpControllerSourceFilename, kBpUnregisteredSlotSourceLine, bpControllerCallbackDiagnosticStrings);
            record = 0;
        }
    }
    if (!record) return -1;

    int alreadyExists; unsigned char *obj = (unsigned char *)BpCtrl_FindOrAllocObject(record, objectKey, &alreadyExists);
    if (obj != 0) {
        BP_STATE(obj)->flags = 0;
    BP_STATE(obj)->key = objectKey;
        (*(int*)&BP_STATE(obj)->next) = 0;
    unsigned char *p = obj;
    BP_STATE(obj)->userData = userData;
    for (int i = 0; i < kBpChannelsPerSprite / 2; ++i, p += 2 * kBpChannelBytes) {

            BP_STATE(p)->channels[0].animationState = 0;
            BP_STATE(p)->channels[0].animationData = 0;
        BP_STATE(p)->channels[0].propertyAddress = 0;
            BP_STATE(p)->channels[0].baseValue = 0;
        BP_STATE(p)->channels[0].value = 0;
        BP_STATE(p)->channels[0].mode = 0;
        for (int j = 0; bpChannelDefaultOverrides[j * 2] > 0; ++j)
            if (2 * i == bpChannelDefaultOverrides[2 * j]) {
                BP_STATE(p)->channels[0].mode = bpChannelDefaultOverrides[((j * 2) + 1)];
                break;
            }
        BP_STATE(p)->channels[1].animationState = 0;
        BP_STATE(p)->channels[1].animationData = 0;
        BP_STATE(p)->channels[1].propertyAddress = 0;
        BP_STATE(p)->channels[1].baseValue = 0;
            BP_STATE(p)->channels[1].value = 0;
        BP_STATE(p)->channels[1].mode = 0;
        for (int j = 0; bpChannelDefaultOverrides[j * 2] > 0; ++j)
            if (2 * i + 1 == bpChannelDefaultOverrides[2 * j]) { BP_STATE(p)->channels[1].mode = bpChannelDefaultOverrides[2 * j + 1]; break; }
    }
    }
    if (obj == 0) return -1;
    *(short *)((obj + 2)) |= 3;
    AsBpDrawObject800BC91C(obj)->sourceWidth = 0; BP_STATE(obj)->channels[kWidthChannel].baseValue = 0;
    AsBpDrawObject800BC91C(obj)->sourceHeight = 0; BP_STATE(obj)->channels[kHeightChannel].baseValue = 0;
    BP_STATE(obj)->channels[kAuxiliaryChannel].value = 100; BP_STATE(obj)->channels[kAuxiliaryChannel].baseValue = 100;
    AsBpDrawObject800BC91C(obj)->scaleX = kBpScaleOne; BP_STATE(obj)->channels[kScaleXChannel].baseValue = kBpScaleOne;
    AsBpDrawObject800BC91C(obj)->scaleY = kBpScaleOne;
    BP_STATE(obj)->channels[kScaleYChannel].baseValue = kBpScaleOne;
    AsBpDrawObject800BC91C(obj)->pivotX = 0; BP_STATE(obj)->channels[kPivotXChannel].baseValue = 0;
    AsBpDrawObject800BC91C(obj)->pivotY = 0; BP_STATE(obj)->channels[kPivotYChannel].baseValue = 0;
    BP_STATE(obj)->channels[kClipChannel].value = BP_STATE(obj)->key; BP_STATE(obj)->channels[kClipChannel].baseValue = BP_STATE(obj)->key;
    if (!alreadyExists) {
        void **head = (void **)(record + 0x30);
        if (!*head) *head = obj;
        else {
            void **node = (void **)*head;
            while (node[5]) node = (void **)node[5];
            node[5] = obj;
        }
    }
    return *(short *)obj;
}
}
#pragma pop

#pragma push
#pragma dont_inline on
#pragma use_lmw_stmw reset
extern "C" {
// Reset one registered slot without freeing its borrowed sprite or auxiliary pools.
int BpObject_ClearWorkBuffers(int index) {
    unsigned char * record;
    if (index < 0 || index >= kBpSlotCount) {
        BpCtrl_InvokeGlobalHook(index, kBpControllerSourceFilename, kBpInvalidSlotSourceLine, bpControllerCallbackDiagnosticStrings);
        record = 0;
    } else {
        record = (unsigned char *)s_bpObjectEntries + index * sizeof(BpSlotResetView);
        if (*(int *)record == -1) {
            BpCtrl_InvokeGlobalHook(index, kBpControllerSourceFilename, kBpUnregisteredSlotSourceLine, bpControllerCallbackDiagnosticStrings);
            record = 0;
        }
    }
    if (record == 0) return -1;
    // Phase 1: invalidate auxiliary work records; the pool remains allocated.
    if (((BpSlotResetView*)record)->auxiliaryWork != 0) {
        int * entry;
        int off;
        int i;
        for (i = 0, off = 0; i < ((BpSlotResetView*)record)->auxiliaryCount; ++i, off += 0x10) {
            entry = (int *)((char*)((BpSlotResetView*)record)->auxiliaryWork + off);
            entry[2] = -1;
            entry[3] = 0;
            entry[1] = 0;
            entry[0] = 0;
        }
    }
    // Phase 2: mark every sprite unused without touching its animation backing.
    if (((BpSlotResetView*)record)->sprites != 0) {
        int off;
        int i;
        for (i = 0, off = 0; i < ((BpSlotResetView*)record)->spriteCount; ++i, off += kBpSpriteBytes) {
            ((BpSpriteState*)((char*)((BpSlotResetView*)record)->sprites + off))->key = -1;
            ((BpSpriteState*)((char*)((BpSlotResetView*)record)->sprites + off))->flags = 0;
        }
    }
    // Phase 3: detach the active list and restore the slot's transform defaults.
    (*(int*)&((BpSlotResetView*)record)->activeHead) = 0;
    ((BpSlotResetView*)record)->positionX = 0;
    ((BpSlotResetView*)record)->positionY = 0;
    ((BpSlotResetView*)record)->scaleX = kBpScaleOne;
    ((BpSlotResetView*)record)->scaleY = kBpScaleOne;
    s_bpCachedRenderCallbackValue = -1;
    return 0;
}
}
#pragma pop

#pragma push
#pragma dont_inline on
#pragma use_lmw_stmw reset
extern "C" {
// Register borrowed sprite and auxiliary storage, then reset the slot.
int BpSlot_RegisterWork(int index, int start, int count, const char *auxiliaryCountWord) {
    if (index < 0 || index >= kBpSlotCount) {
        BpCtrl_InvokeGlobalHook(-1, kBpControllerSourceFilename, 0x9F6, bpSlotRegistrationDiagnosticStrings);
        return -1;
    }
    unsigned int *work = (unsigned int *)((char *)s_bpObjectEntries + index * sizeof(BpSlotResetView));
    work[0] = index;
    work[1] = start;
    work[12] = 0;
    work[2] = start;
    work[3] = start + count * kBpSpriteBytes;
    work[4] = count;
    work[5] = (unsigned int)auxiliaryCountWord;
    work[6] = 0;
    BpObject_ClearWorkBuffers(index);
    return 0;
}
}
#pragma pop

#pragma push
#pragma dont_inline on
#pragma use_lmw_stmw reset
extern "C" {
// Reset all controller slots and global sprite draw callbacks.
int CBpCtrl_InitializeObjectEntries() {
    g_valueAnimationUpdateMode = 1;
    memset(s_bpObjectEntries, 0, 0x820);
    unsigned int *p = s_bpObjectEntries;
    for (int i = 0; i < 5; ++i) {
        p[0] = 0xFFFFFFFF;
        p[9] = 0x80;
        p[13] = 0xFFFFFFFF;
        p[22] = 0x80;
        p[26] = 0xFFFFFFFF;
        p[35] = 0x80;
        p[39] = 0xFFFFFFFF;
        p[48] = 0x80;
        p[52] = 0xFFFFFFFF;
        p[61] = 0x80;
        p[65] = 0xFFFFFFFF;
        p[74] = 0x80;
        p[78] = 0xFFFFFFFF;
        p[87] = 0x80;
        p[91] = 0xFFFFFFFF;
        p[100] = 0x80;
        p += 0x68;
    }
    g_bpDefaultRectDrawCallback = 0;
    g_bpDrawObjectCallback = 0;
    g_bpTileDrawCallback = 0;
    return 0;
}
}
#pragma pop

#pragma push
#pragma dont_inline on
#pragma use_lmw_stmw reset
extern "C" {
// Initialize the sprite angle constant to one full turn.
void BpSprite_InitFullTurnConstant() {
    s_bpSpriteFullTurnAngle = kFloat_6_2831855;
}
}
#pragma pop

#pragma push
#pragma dont_inline on
#pragma use_lmw_stmw reset
extern "C" {
// Compute pixel bounds with separately truncated sprite/parent scales and a full-grid pivot.
int BpSprite_ComputeUnrotatedRect(int *out, void *sprite, void *parentSprite) {
    int parentPositionX, parentPositionY;
    int scaledTileWidth, scaledTileHeight, tileColumns, tileRows;
    int parentScaleX, parentScaleY;
    int widthProduct, heightProduct;
    int spriteScaleX, spriteScaleY;
    parentPositionX = 0;
    parentPositionY = 0;
    parentScaleX = kBpScaleOne;
    parentScaleY = kBpScaleOne;
    // Phase 1: inherit parent translation in pixels and scale in 1/1024 units.
    if (parentSprite) {
        parentPositionX = AsBpDrawObject800BC91C(parentSprite)->positionX;
        parentPositionY = AsBpDrawObject800BC91C(parentSprite)->positionY;
        parentScaleX = AsBpDrawObject800BC91C(parentSprite)->scaleX;
        parentScaleY = AsBpDrawObject800BC91C(parentSprite)->scaleY;
    }
    out[0] = AsBpDrawObject800BC91C(sprite)->positionX + parentPositionX;
    out[1] = AsBpDrawObject800BC91C(sprite)->positionY + parentPositionY;
    scaledTileWidth = AsBpDrawObject800BC91C(sprite)->sourceWidth;
    scaledTileHeight = AsBpDrawObject800BC91C(sprite)->sourceHeight;
    spriteScaleX = AsBpDrawObject800BC91C(sprite)->scaleX;
    spriteScaleY = AsBpDrawObject800BC91C(sprite)->scaleY;
    // Phase 2: truncate after sprite scale, then after parent scale.
    // Preserve both divisions: one combined product changes integer rounding.
    widthProduct = scaledTileWidth * spriteScaleX;
    heightProduct = scaledTileHeight * spriteScaleY;
    scaledTileWidth = widthProduct / kBpScaleOne;
    scaledTileHeight = heightProduct / kBpScaleOne;
    widthProduct = scaledTileWidth * parentScaleX;
    heightProduct = scaledTileHeight * parentScaleY;
    scaledTileWidth = widthProduct / kBpScaleOne;
    scaledTileHeight = heightProduct / kBpScaleOne;
    // Phase 3: expand tile counts and apply the pivot to the complete pixel bounds.
    tileColumns = AsBpDrawObject800BC91C(sprite)->tileColumns;
    tileRows = AsBpDrawObject800BC91C(sprite)->tileRows;
    out[2] = tileColumns >= 0 ? scaledTileWidth * tileColumns : scaledTileWidth * -tileColumns;
    out[3] = tileRows >= 0 ? scaledTileHeight * tileRows : scaledTileHeight * -tileRows;
    out[0] -= (out[2] * AsBpDrawObject800BC91C(sprite)->pivotX) / kBpScaleOne;
    out[1] -= (out[3] * AsBpDrawObject800BC91C(sprite)->pivotY) / kBpScaleOne;
    return 0;
}
}
#pragma pop

#pragma push
#pragma dont_inline on
#pragma use_lmw_stmw reset
extern "C" {
// Expand pixel bounds into four corners and rotate around the sprite pivot.
void BpSprite_ComputeRotatedQuad(int *out, void *sprite, void *parentSprite) {
    int count = AsBpDrawObject800BC91C(sprite)->rotation;
    if (parentSprite) count += AsBpDrawObject800BC91C(parentSprite)->rotation;
    int r[4];
    BpSprite_ComputeUnrotatedRect(r, sprite, parentSprite);
    out[0] = r[0]; out[1] = r[1]; out[2] = r[0];
    out[3] = r[1] + r[3];
    out[4] = r[0] + r[2];
    out[5] = r[1] + r[3];
    out[6] = r[0] + r[2];
    out[7] = r[1];
    if (count)
        Math_RotateIntegerPoints2D(out, 4, r[0] + ((r[2] * AsBpDrawObject800BC91C(sprite)->pivotX) / kBpScaleOne), r[1] + ((r[3] * AsBpDrawObject800BC91C(sprite)->pivotY) / kBpScaleOne), count);
}
}
#pragma pop

#pragma push
#pragma dont_inline on
#pragma use_lmw_stmw reset
extern "C" {
// Multiply packed color channels with 128 as unity and clamp to bytes.
unsigned int Color_ModulatePackedChannels(int a, int b) {
    float value;
    int bits;
    unsigned int result = 0;
    int i;
    for (i = 0; i < 4; ++i) {
        int shift = (3 - i) * 8;
        int ca = (unsigned char)(a >> shift);
        int cb = (unsigned char)(b >> shift);
        value = (float)ca * (float)cb;
        value *= kFloat_0_0078125_BpColorMultiply;
        if (value >= kBpColorChannelMaximum) value = kBpColorChannelMaximum;
        else if (value <= kFloat_0_BpColorMultiply) value = kFloat_0_BpColorMultiply;
        bits = (int)value;
        result <<= 8;
        result |= (unsigned char)bits;
    }
    return result;
}
}
#pragma pop

#pragma push
#pragma dont_inline on
#pragma use_lmw_stmw on
extern "C" {
// Draw unrotated tiles in pixel space with inherited fixed-point scale and color.
void BpSprite_DrawTilesUnrotated(void *self, void *other, int columns, int rows, int x, int y,
                 int widthScale, int heightScale, int opacityScale) {
    int flipA, flipB, flipC, flipD;
    int drawOpacity, rowParity, col, stepX, baseX, baseY;
    int drawCallback, format, width2, height2, sx;
    int flags;
    int sy;
    unsigned char *s = (unsigned char *)self;
    format = *(int *)(AsBpDrawObject800BC91C(s)->formatProperty + 4) & kBpFormatMask;
    flags = *(int *)(AsBpDrawObject800BC91C(s)->tileFlipProperty + 4) & kBpTileFlipMask;
    drawCallback = AsBpDrawObject800BC91C(s)->drawRectCallback;
    width2 = AsBpDrawObject800BC91C(s)->scaleX;
    height2 = AsBpDrawObject800BC91C(s)->scaleY;
    sy = AsBpDrawObject800BC91C(s)->opacity;
    sx = AsBpDrawObject800BC91C(s)->tintColor;
    drawCallback = drawCallback ? drawCallback : Bp_GetDefaultRectDrawCallback();
    int bounds[4];
    BpSprite_ComputeUnrotatedRect(bounds, self, other);
    int w = (AsBpDrawObject800BC91C(s)->sourceWidth * AsBpDrawObject800BC91C(s)->scaleX) / kBpScaleOne;
    stepX = (w * widthScale) / kBpScaleOne;
    int h = (AsBpDrawObject800BC91C(s)->sourceHeight * AsBpDrawObject800BC91C(s)->scaleY) / kBpScaleOne;
    int stepY = (h * heightScale) / kBpScaleOne;
    if (other) {
        unsigned char *o = (unsigned char *)other;
        width2 = (width2 * AsBpDrawObject800BC91C(o)->scaleX) / kBpScaleOne;
        height2 = (height2 * AsBpDrawObject800BC91C(o)->scaleY) / kBpScaleOne;
        sy = (sy * AsBpDrawObject800BC91C(o)->opacity) / kBpColorOne;
        sx = Color_ModulatePackedChannels(sx, AsBpDrawObject800BC91C(o)->tintColor);
        stepX = (stepX * AsBpDrawObject800BC91C(o)->scaleX) / kBpScaleOne;
        stepY = (stepY * AsBpDrawObject800BC91C(o)->scaleY) / kBpScaleOne;
    }
    int tileProduct = sy * opacityScale;
    baseX = bounds[0] + x;
    baseY = bounds[1] + y;
    flipA = flags & kBpAlternateRowX;
    flipB = flags & kBpAlternateRowY;
    flipC = flags & kBpAlternateColumnX;
    flipD = flags & kBpAlternateColumnY;
    drawOpacity = tileProduct / kBpColorOne;
    typedef void (*Draw)(int, int, int, int, int, int, int, int, int, int, int, int);
    for (int row = 0; row < rows; ++row) {
        int startX = baseX;
        rowParity = ((row & 1) ^ (row < 0)) - (row < 0);
        for (col = 0; col < columns; ++col) {
            int f = format;
            if (rowParity == 1) {
                if (flipA) f ^= kBpFlipHorizontal;
                if (flipB) f ^= kBpFlipVertical;
            }
            int colParity = ((col & 1) ^ (col < 0)) - (col < 0);
            if (colParity == 1) {
                if (flipC) f ^= kBpFlipHorizontal;
                if (flipD) f ^= kBpFlipVertical;
            }
            ((Draw)drawCallback)(AsBpDrawObject800BC91C(s)->image, AsBpDrawObject800BC91C(s)->sourceX, AsBpDrawObject800BC91C(s)->sourceY,
                 AsBpDrawObject800BC91C(s)->sourceWidth, AsBpDrawObject800BC91C(s)->sourceHeight, baseX, baseY, width2, height2,
                 drawOpacity, sx, f | (*(int *)(AsBpDrawObject800BC91C(s)->blendProperty + 4) & 0xF0));
            baseX += stepX;
        }
        baseX = startX;
        baseY += stepY;
    }
}
}
#pragma pop

#pragma push
#pragma dont_inline on
#pragma use_lmw_stmw on
extern "C" {
// Subdivide a rotated quadrilateral and apply row/column flip policies.
void BpSprite_DrawTilesRotated(void *self, void *other, int columnCount, int rowCount,
                 int startX, int startY, int widthScale, int heightScale,
                 int opacityScale) {
    typedef void (*Draw)(int, int *, int *, int, int, int);
    int flipA;
    int dRY, dRX, LYr;
    int row, col, format, tintColor;
    unsigned char *s = (unsigned char *)self;
    int rowNext;
    int flipB, flipC, flipD, scaledColor, rowParity;
    Draw drawFn;
    int otherScale, flags;
    int LXr, LXr1, dXr1, LYr1, dYr1;
    int dX0, dX1, dY0, dY1;
    format = (*(int *)(AsBpDrawObject800BC91C(s)->formatProperty + 4)) & kBpFormatMask;
    flags = (*(int *)(AsBpDrawObject800BC91C(s)->tileFlipProperty + 4)) & kBpTileFlipMask;
    drawFn = *(Draw *)&AsBpDrawObject800BC91C(s)->drawQuadCallback;
    otherScale = AsBpDrawObject800BC91C(s)->opacity;
    tintColor = AsBpDrawObject800BC91C(s)->tintColor;
    drawFn = drawFn ? drawFn : (Draw)BpCtrl_GetTexturedQuadCallback();
    int bounds[8];
    BpSprite_ComputeRotatedQuad(bounds, self, other);
    bounds[0] = startX + (widthScale * bounds[0]) / kBpScaleOne;
    bounds[1] = startY + (heightScale * bounds[1]) / kBpScaleOne;
    bounds[2] = startX + (widthScale * bounds[2]) / kBpScaleOne;
    bounds[3] = startY + (heightScale * bounds[3]) / kBpScaleOne;
    bounds[4] = startX + (widthScale * bounds[4]) / kBpScaleOne;
    bounds[5] = startY + (heightScale * bounds[5]) / kBpScaleOne;
    bounds[6] = startX + (widthScale * bounds[6]) / kBpScaleOne;
    bounds[7] = startY + (heightScale * bounds[7]) / kBpScaleOne;
    if (other) {
        otherScale = (otherScale * AsBpDrawObject800BC91C(other)->opacity) / kBpColorOne;
        tintColor = Color_ModulatePackedChannels(tintColor, AsBpDrawObject800BC91C(other)->tintColor);
    }
    int colorProduct = otherScale * opacityScale;
    flipA = flags & kBpAlternateRowX;
    flipB = flags & kBpAlternateRowY;
    flipC = flags & kBpAlternateColumnX;
    flipD = flags & kBpAlternateColumnY;
    int w = AsBpDrawObject800BC91C(s)->sourceWidth;
    int x0 = AsBpDrawObject800BC91C(s)->sourceX;
    int h = AsBpDrawObject800BC91C(s)->sourceHeight;
    int dst[8];
    dst[0] = x0;
    int right = x0 + w;
    int y0 = AsBpDrawObject800BC91C(s)->sourceY;
    scaledColor = colorProduct / kBpColorOne;
    dst[1] = y0;
    dst[2] = x0;
    dst[3] = y0 + h;
    dst[4] = right;
    dst[5] = y0 + h;
    dst[6] = right;
    dst[7] = y0;
    for (row = 0; row < rowCount; ++row) {
        rowParity = ((row & 1) ^ (row < 0)) - (row < 0);
        rowNext = row + 1;
        for (col = 0; col < columnCount; ++col) {
            int src[8];
            dX0 = bounds[2] - bounds[0];
            dX1 = bounds[4];
            dX1 -= bounds[6];
            dY0 = bounds[3] - bounds[1];
            dY1 = bounds[5];
            dY1 -= bounds[7];
            LXr = bounds[0] + (dX0 * row) / rowCount;
            dRX = bounds[6] + (dX1 * row) / rowCount - LXr;
            LYr = bounds[1] + (dY0 * row) / rowCount;
            dRY = bounds[7] + (dY1 * row) / rowCount - LYr;
            LXr1 = bounds[0] + (dX0 * rowNext) / rowCount;
            dXr1 = bounds[6] + (dX1 * rowNext) / rowCount - LXr1;
            LYr1 = bounds[1] + (dY0 * rowNext) / rowCount;
            dYr1 = bounds[7] + (dY1 * rowNext) / rowCount - LYr1;
            src[0] = LXr + (dRX * col) / columnCount;
            src[1] = LYr + (dRY * col) / columnCount;
            src[2] = LXr1 + (dXr1 * col) / columnCount;
            src[3] = LYr1 + (dYr1 * col) / columnCount;
            src[4] = LXr1 + (dXr1 * (col + 1)) / columnCount;
            src[5] = LYr1 + (dYr1 * (col + 1)) / columnCount;
            src[7] = LYr + (dRY * (col + 1)) / columnCount;
            src[6] = LXr + (dRX * (col + 1)) / columnCount;
            int f = format;
            if (rowParity == 1) {
                if (flipA) f ^= kBpFlipHorizontal;
                if (flipB) f ^= kBpFlipVertical;
            }
            int colParity = ((col & 1) ^ (col < 0)) - (col < 0);
            if (colParity == 1) {
                if (flipC) f ^= kBpFlipHorizontal;
                if (flipD) f ^= kBpFlipVertical;
            }
            f |= (*(int *)(AsBpDrawObject800BC91C(s)->blendProperty + 4)) & 0xF0;
            drawFn(AsBpDrawObject800BC91C(s)->image, src, dst, scaledColor, tintColor, f);
        }
    }
}
}
#pragma pop

#pragma push
#pragma dont_inline on
#pragma use_lmw_stmw reset
extern "C" {
// Dispatch tile drawing with the combined sprite and parent rotation.
int BpSprite_DrawTiles(void *self, void *other, int startX, int startY, int widthScale,
                int heightScale, int opacityScale, int unused) {
    int rotation;
    int x;
    int y;
    int sy;
    int sx;
    rotation = AsBpDrawObject800BC91C(self)->rotation;
    if (other != 0)
        rotation += AsBpDrawObject800BC91C(other)->rotation;
    y = AsBpDrawObject800BC91C(self)->tileRows;
    x = AsBpDrawObject800BC91C(self)->tileColumns;
    sx = x >> 31;
    sy = y >> 31;
    x = (sx ^ x) - sx;
    y = (sy ^ y) - sy;
    if (rotation != 0)
        BpSprite_DrawTilesRotated(self, other, x, y,
                    startX, startY, widthScale, heightScale, opacityScale);
    else
        BpSprite_DrawTilesUnrotated(self, other, x, y,
                    startX, startY, widthScale, heightScale, opacityScale);
    return 0;
}
}
#pragma pop
